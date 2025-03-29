
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <algorithm>
#include <sys/time.h>
#include <unistd.h>

#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_scl_datatype.h"
#include "mi_scl.h"
#include "mi_vif_datatype.h"
#include "mi_vif.h"
#include "mi_sensor.h"
#include "isp_cus3a_if.h"
#include "mi_isp_cus3a_api.h"
#include "mi_isp_hw_dep_datatype.h"
#include "mi_isp_awb.h"

#include <poll.h>
#include "mi_isp.h"
#include "mi_isp_iq.h"
#include "mi_isp_datatype.h"
#include "color_aligner.h"

void *read_binary(char *fileName, void *pBuff, int size)
{
    FILE *fp;
    int c, rgb_comp_color;

    //open PPM file for reading
    fp = fopen(fileName, "rb");
    if(!fp)
    {
        CLRALIGN_ERR("Unable to open file '%s'\n", fileName);
        exit(1);
    }

    if(fread(pBuff,1,(size), fp) != (size))
    {
        CLRALIGN_ERR("Error loading image '%s'\n", fileName);
    }
    else
    {
        CLRALIGN_INFO("Load binary success '%s'\n", fileName);
    }

    fclose(fp);
    return pBuff;
}

void CalculateStaturationHSL(MI_U8 R, MI_U8 G, MI_U8 B, MI_U32* MaxRGB, MI_U32* MinRGB, MI_U32* Luminance, MI_U32* Saturation)
{
    *MaxRGB = MAX(MAX(R,G),B);
    *MinRGB = MIN(MIN(R,G),B);
    *Luminance = (*MaxRGB+*MinRGB)/2;
    if(*Luminance==0)
    {
        *Saturation = 0;
    }
    else if(*Luminance>0 && *Luminance <= 128)
    {
        *Saturation = (*MaxRGB-*MinRGB)*1024/(2*(*Luminance));
    }
    else
    {
        *Saturation = (*MaxRGB-*MinRGB)*1024/(2*255-2*(*Luminance));
    }
}

void CalculateStaturationHSV(MI_U8 R, MI_U8 G, MI_U8 B, MI_U32* MaxRGB, MI_U32* MinRGB, MI_U32* Luminance, MI_U32* Saturation)
{
    *MaxRGB = MAX(MAX(R,G),B);
    *MinRGB = MIN(MIN(R,G),B);
    *Luminance = *MaxRGB;
    if(*MaxRGB==0)
    {
        *Saturation = 0;
    }
    else
    {
        *Saturation = (*MaxRGB-*MinRGB)*1024/(*MaxRGB);
    }
}

ColorAligner::ColorAligner(Stitching_Dev_Ch_t camera_map[], MI_U32 valid_cam_num, MI_U32 image_width, MI_U32 image_height)
{
    uValidCameraNum = valid_cam_num;
    if(uValidCameraNum>MAX_SUPPORT_NUM)
    {
        CLRALIGN_ERR("Max stitching support number is %d, but %d cameras are set", MAX_SUPPORT_NUM, uValidCameraNum);
        return;
    }

    for(MI_U8 i = 0; i< uValidCameraNum; i++)
    {
        mCameraMap[i].uDev = camera_map[i].uDev;
        mCameraMap[i].uCh = camera_map[i].uCh;
        memcpy((char *)mCameraMap[i].mappingx_path,  (char *)camera_map[i].mappingx_path,  strlen((char *)camera_map[i].mappingx_path));
        memcpy((char *)mCameraMap[i].mappingy_path,  (char *)camera_map[i].mappingy_path,  strlen((char *)camera_map[i].mappingy_path));
    }
    u4ImageWidth = image_width;
    u4ImageHeight = image_height;
    u2CollectFrameNum = STITCHING_ALIGN_FRAME;
    u2STAInlierRatio = 60;
    u2STASaturationThd1 = 1024;
    u2STASaturationThd2 = 1024;
    u2STAMaxRGBThd1 = 180;
    u2STAMaxRGBThd2 = 190;
    u2STAMinRGBThd1 = 15;
    u2STAMinRGBThd2 = 20;
    u2LowLuxNotApplyThd = 1000;
    uChangeSceneMotionThd = 10;
    uMotionThd = 5;
    uApplyIIRPre = 5;
    uApplyIIRCur = 1;
    u2ConvergeRatio = 1;
    this->Init();
}

ColorAligner::~ColorAligner()
{
    this->UnInit();
}

unsigned int ColorAligner::Init()
{
    MI_U8 i = 0x0;

#ifdef STITCHING_USE_AE
    CusAeStatsPostProcessing_e AeStatistic = AE_STATS_PP_TYPE_SEP;
#else
    CusAwbStatsPostProcessing_e AwbStatistic = AWB_STATS_PP_TYPE_SEP;
    CusAWBSample_t tAwbSample;
    tAwbSample.SizeX    = ((u4ImageWidth/128) >> 1) << 1;
    tAwbSample.SizeY    = ((u4ImageHeight/128) >> 1) << 1;
    tAwbSample.IncRatio = 1;
    CLRALIGN_WARN("Set awb sample SizeX=%d, SizeY=%d, IncRatio=%d\n",tAwbSample.SizeX, tAwbSample.SizeY, tAwbSample.IncRatio);
#endif
    for(i=0; i < uValidCameraNum; i++)
    {
#ifdef STITCHING_USE_AE
        MI_ISP_CUS3A_SetAeStatsPostProcessing(mCameraMap[i].uDev, mCameraMap[i].uCh, &AeStatistic);
#else
        MI_ISP_CUS3A_SetAwbStatsPostProcessing(mCameraMap[i].uDev, mCameraMap[i].uCh, &AwbStatistic);
        MI_ISP_CUS3A_SetAWBSampling(mCameraMap[i].uDev, mCameraMap[i].uCh, &tAwbSample);
#endif
    }

    ldc_mapping_x = (float**)malloc(sizeof(float*) * uValidCameraNum);
    ldc_mapping_y = (float**)malloc(sizeof(float*) * uValidCameraNum);

    
    for(int i =0; i< uValidCameraNum-1; ++i)
    {
        ldc_mapping_x[i] = (float*) calloc(u4ImageWidth*u4ImageHeight, sizeof(float));
        ldc_mapping_y[i] = (float*) calloc(u4ImageWidth*u4ImageHeight, sizeof(float));
        ldc_mapping_x[i] = (float*)read_binary(mCameraMap[i].mappingx_path, (void*)ldc_mapping_x[i], sizeof(float) * u4ImageWidth * u4ImageHeight);
        ldc_mapping_y[i] = (float*)read_binary(mCameraMap[i].mappingy_path, (void*)ldc_mapping_y[i], sizeof(float) * u4ImageWidth * u4ImageHeight);
    }

    for(int i=0; i <MAX_CUST_3A_DEV_NUM ; i++)
    {
        for(int j=0; j <MAX_CUST_3A_DEV_NUM ; j++)
        {
            mStitchingApplyAdjust[i][j].u2AdjustR = STITCHING_ADJUST_BASE;
            mStitchingApplyAdjust[i][j].u2AdjustG = STITCHING_ADJUST_BASE;
            mStitchingApplyAdjust[i][j].u2AdjustB = STITCHING_ADJUST_BASE;
        }
    }

    bFirstCal = TRUE;
    mStitchingFrameCount = 0;
    bRetriggerFindMaxIndex = 0;
    return MI_SUCCESS;
}

unsigned int ColorAligner::UnInit()
{
    if(NULL != ldc_mapping_x)
    {
        for(int i =0; i< uValidCameraNum-1; ++i)
        {
            free(ldc_mapping_x[i]);
            ldc_mapping_x[i] = NULL;
        }
    }
    free(ldc_mapping_x);
    ldc_mapping_x = NULL;

    if(NULL != ldc_mapping_y)
    {
        for(int i =0; i< uValidCameraNum-1; ++i)
        {
            free(ldc_mapping_y[i]);
            ldc_mapping_y[i] = NULL;
        }
    }
    free(ldc_mapping_y);
    ldc_mapping_y = NULL;
    

    return MI_SUCCESS;
}

unsigned int ColorAligner::Run(unsigned int debug_level)
{
    MI_U8 i = 0x0;
    MI_U32 _invokeCMD = 0x0;
    MI_U32 u32SubChId = 0;
    MI_U32 u32SensorBindId = 0;
    MI_U8 bUseSubChId = 0;
    MI_U32 sret = 0;
    MI_U32 y, x;

    mMotionStable=1;
    for(i=0x0; i < uValidCameraNum; i++)
    {
        MI_U8 nDev = mCameraMap[i].uDev;
        MI_U8 nCh = mCameraMap[i].uCh;
        MI_U32 avg = 0;
#ifdef STITCHING_USE_AE
        sret = MI_ISP_AE_GetAeHwAvgStats(nDev, nCh, &tAeAvg[nDev][nCh]);
        MI_U32 nSTACol = tAeAvg[nDev][nCh].nBlkX;
        MI_U32 nSTARow = tAeAvg[nDev][nCh].nBlkY;
#else
        sret = MI_ISP_AWB_GetAwbHwAvgStats(nDev, nCh, &tAwbAvg[nDev][nCh]);
        MI_U32 nSTACol = tAwbAvg[nDev][nCh].nBlkX;
        MI_U32 nSTARow = tAwbAvg[nDev][nCh].nBlkY;
#endif
        for(y=0;y<nSTARow;y++)
        {
            MI_U32 RowIndexShift = y * nSTACol;
            for(x = 0; x<nSTACol; x++, RowIndexShift++)
            {
#ifdef STITCHING_USE_AE
                if(tAeAvg[nDev][nCh].nAvg[RowIndexShift].uAvgY > tAeAvgPre[nDev][nCh].nAvg[RowIndexShift].uAvgY)
                {
                    avg += tAeAvg[nDev][nCh].nAvg[RowIndexShift].uAvgY - tAeAvgPre[nDev][nCh].nAvg[RowIndexShift].uAvgY;
                }
                else
                {
                    avg += tAeAvgPre[nDev][nCh].nAvg[RowIndexShift].uAvgY - tAeAvg[nDev][nCh].nAvg[RowIndexShift].uAvgY;
                }
#else
                if(tAwbAvg[nDev][nCh].nAvg[RowIndexShift].uAvgG > tAwbAvgPre[nDev][nCh].nAvg[RowIndexShift].uAvgG)
                {
                    avg += tAwbAvg[nDev][nCh].nAvg[RowIndexShift].uAvgG - tAwbAvgPre[nDev][nCh].nAvg[RowIndexShift].uAvgG;
                }
                else
                {
                    avg += tAwbAvgPre[nDev][nCh].nAvg[RowIndexShift].uAvgG - tAwbAvg[nDev][nCh].nAvg[RowIndexShift].uAvgG;
                }
#endif
            }
        }
        avg /= (nSTARow*nSTACol);
        if(avg > uMotionThd && mMotionStable==1)
        {
            if(mStitchingFrameCount > 0)
            {
                if(debug_level>DBGLVL_ALIGNPROCESS)
                {
                    CLRALIGN_DBG("Motion Detected, Frame count reset, dev %d, ch %d, value %d, motion_thd %d,row %d, col %d\n", nDev, nCh, avg, uMotionThd, nSTARow, nSTACol);
                }
                mStitchingFrameCount = 0;
            }
            bRetriggerFindMaxIndex = 1;
            mMotionStable = 0;
        }
        else
        {
            mMotionStable = 1;
        }
#ifdef STITCHING_USE_AE
        memcpy(&tAeAvgPre[nDev][nCh], &tAeAvg[nDev][nCh], sizeof(MI_ISP_AE_HW_STATISTICS_t));
#else
        memcpy(&tAwbAvgPre[nDev][nCh], &tAwbAvg[nDev][nCh], sizeof(MI_ISP_AWB_HW_STATISTICS_t));
#endif
        if(mMotionStable == 0)
        {
            break;
        }
    }



    //[STICHING_ALIGN] Iterate all overlap area and set overlap statistics to buffer.
    MI_ISP_AWB_QueryInfoType_t tIspAwbInfo;
    MI_ISP_AWB_QueryInfo(0, 0, &tIspAwbInfo);
    if (tIspAwbInfo.bIsStable==1 && mMotionStable == 1)
    {
        MI_U32 overlapx;
        for(i=0x0; i < uValidCameraNum-1; i++)
        {
            MI_U8 indexRight = i;
            MI_U8 indexLeft = i+1;

            MI_U8 nDevLeft = mCameraMap[indexLeft].uDev;  
            MI_U8 nChLeft = mCameraMap[indexLeft].uCh;
            MI_U8 nDevRight = mCameraMap[indexRight].uDev;
            MI_U8 nChRight = mCameraMap[indexRight].uCh;

#ifdef STITCHING_USE_AE
            MI_U32 nSTACol = tAeAvg[nDevLeft][nChLeft].nBlkX;
            MI_U32 nSTARow = tAeAvg[nDevLeft][nChLeft].nBlkY;
#else
            MI_U32 nSTACol = tAwbAvg[nDevLeft][nChLeft].nBlkX;
            MI_U32 nSTARow = tAwbAvg[nDevLeft][nChLeft].nBlkY;
#endif

            MI_U32 AvgRLeft=0;
            MI_U32 AvgGLeft=0;
            MI_U32 AvgBLeft=0;
            MI_U32 AvgRRight=0;
            MI_U32 AvgGRight=0;
            MI_U32 AvgBRight=0;
            MI_U32 Saturation;
            MI_U32 Luminance;
            MI_U32 MaxRGB;
            MI_U32 MinRGB;
            MI_U32 R;
            MI_U32 G;
            MI_U32 B;
            MI_U32 AvgCntLeft=0;
            MI_U32 AvgCntRight=0;
            MI_U32 WeightBase=1024;
            MI_U32 CountBase=100;
            MI_U32 SaturationWeight;
            MI_U32 MaxRGBWeight;
            MI_U32 MinRGBWeight;
            MI_U32 FinalWeight;
            
            int mapping_index;
            MI_U32 y_step = 10;
            MI_U32 x_step = 10;
            MI_U32 Height_lb = u4ImageHeight*0/10;
            MI_U32 Height_ub = u4ImageHeight*10/10;
            for(y=Height_lb;y<Height_ub;y+=y_step)
            {
                for(x=0;x<u4ImageWidth;x+=x_step)
                {
                    mapping_index = y * u4ImageWidth + x;
                    float xp = ldc_mapping_x[i][mapping_index];
                    float yp = ldc_mapping_y[i][mapping_index];
                    if(xp > 0 && yp >0 && xp < u4ImageWidth && yp < u4ImageHeight)
                    {
                        MI_U32 LeftAE_X = xp * nSTACol / u4ImageWidth;
                        MI_U32 LeftAE_Y = yp * nSTARow / u4ImageHeight;
                        MI_U32 indexSTALeft = LeftAE_Y * nSTACol + LeftAE_X;

                        MI_U32 RightAE_X = x * nSTACol / u4ImageWidth;
                        MI_U32 RightAE_Y = y * nSTARow / u4ImageHeight;
                        MI_U32 indexSTARight = RightAE_Y * nSTACol + RightAE_X;

#ifdef STITCHING_USE_AE
                        R = MIN(tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgR*tIspAwbInfo.u16Rgain/1024, 255);
                        G = MIN(tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgG*tIspAwbInfo.u16Gbgain/1024, 255);
                        B = MIN(tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgB*tIspAwbInfo.u16Bgain/1024, 255);
#else
                        R = MIN(tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgR*tIspAwbInfo.u16Rgain/1024, 255);
                        G = MIN(tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgG*tIspAwbInfo.u16Gbgain/1024, 255);
                        B = MIN(tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgB*tIspAwbInfo.u16Bgain/1024, 255);
#endif
                        CalculateStaturationHSV(R,G,B, &MaxRGB, &MinRGB, &Luminance, &Saturation);
                        
                        if(Saturation <= u2STASaturationThd1 && MaxRGB <= u2STAMaxRGBThd1 && MinRGB >= u2STAMinRGBThd2)
                        {
#ifdef STITCHING_USE_AE
                            AvgRLeft += tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgR;
                            AvgGLeft += tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgG;
                            AvgBLeft += tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgB;
#else
                            AvgRLeft += tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgR;
                            AvgGLeft += tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgG;
                            AvgBLeft += tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgB;
#endif
                            AvgCntLeft += CountBase;
                        }
                        else if(Saturation >= u2STASaturationThd2 && MaxRGB >= u2STAMaxRGBThd2 && MinRGB <= u2STAMinRGBThd1)
                        {
                        }
                        else
                        {
                            if(Saturation > u2STASaturationThd1 && Saturation < u2STASaturationThd2)
                            {
                                SaturationWeight = WeightBase - (WeightBase * (Saturation-u2STASaturationThd1) / (u2STASaturationThd2-u2STASaturationThd1));
                            }
                            else if(Saturation <= u2STASaturationThd1)
                            {
                                SaturationWeight = WeightBase;
                            }
                            else
                            {
                                SaturationWeight = 0;
                            }


                            if(MaxRGB > u2STAMaxRGBThd1 && MaxRGB < u2STAMaxRGBThd2)
                            {
                                MaxRGBWeight = WeightBase - (WeightBase * (MaxRGB-u2STAMaxRGBThd1) / (u2STAMaxRGBThd2-u2STAMaxRGBThd1));
                            }
                            else if(MaxRGB <= u2STAMaxRGBThd1)
                            {
                                MaxRGBWeight = WeightBase;
                            }
                            else
                            {
                                MaxRGBWeight = 0;
                            }

                            if(MinRGB > u2STAMinRGBThd1 && MinRGB < u2STAMinRGBThd2)
                            {
                                MinRGBWeight = WeightBase * (MinRGB-u2STAMinRGBThd1) / (u2STAMinRGBThd2-u2STAMinRGBThd1);
                            }
                            else if(MinRGB <= u2STAMinRGBThd1)
                            {
                                MinRGBWeight = 0;
                            }
                            else
                            {
                                MinRGBWeight = WeightBase;
                            }

                            FinalWeight = (MinRGBWeight + MaxRGBWeight + SaturationWeight)/3;

#ifdef STITCHING_USE_AE
                            AvgRLeft += (tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgR*FinalWeight/WeightBase);
                            AvgGLeft += (tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgG*FinalWeight/WeightBase);
                            AvgBLeft += (tAeAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgB*FinalWeight/WeightBase);
#else
                            AvgRLeft += (tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgR*FinalWeight/WeightBase);
                            AvgGLeft += (tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgG*FinalWeight/WeightBase);
                            AvgBLeft += (tAwbAvg[nDevLeft][nChLeft].nAvg[indexSTALeft].uAvgB*FinalWeight/WeightBase);
#endif
                            AvgCntLeft += (CountBase * FinalWeight / WeightBase);
                        }

#ifdef STITCHING_USE_AE
                        R = MIN(tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgR*tIspAwbInfo.u16Rgain/1024, 255);
                        G = MIN(tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgG*tIspAwbInfo.u16Gbgain/1024, 255);
                        B = MIN(tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgB*tIspAwbInfo.u16Bgain/1024, 255);
#else
                        R = MIN(tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgR*tIspAwbInfo.u16Rgain/1024, 255);
                        G = MIN(tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgG*tIspAwbInfo.u16Gbgain/1024, 255);
                        B = MIN(tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgB*tIspAwbInfo.u16Bgain/1024, 255);
#endif
                        CalculateStaturationHSV(R,G,B, &MaxRGB, &MinRGB, &Luminance, &Saturation);
                        if(Saturation < u2STASaturationThd1 && MaxRGB < u2STAMaxRGBThd1 && MinRGB > u2STAMinRGBThd2)
                        {
#ifdef STITCHING_USE_AE
                            AvgRRight += tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgR;
                            AvgGRight += tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgG;
                            AvgBRight += tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgB;
#else
                            AvgRRight += tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgR;
                            AvgGRight += tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgG;
                            AvgBRight += tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgB;
#endif
                            AvgCntRight += CountBase;
                        }
                        else if(Saturation >= u2STASaturationThd2 && MaxRGB >= u2STAMaxRGBThd2 && MinRGB <= u2STAMinRGBThd1)
                        {
                        }
                        else
                        {
                            if(Saturation > u2STASaturationThd1 && Saturation < u2STASaturationThd2)
                            {
                                SaturationWeight = WeightBase - (WeightBase * (Saturation-u2STASaturationThd1) / (u2STASaturationThd2-u2STASaturationThd1));
                            }
                            else if(Saturation <= u2STASaturationThd1)
                            {
                                SaturationWeight = WeightBase;
                            }
                            else
                            {
                                SaturationWeight = 0;
                            }


                            if(MaxRGB > u2STAMaxRGBThd1 && MaxRGB < u2STAMaxRGBThd2)
                            {
                                MaxRGBWeight = WeightBase - (WeightBase * (MaxRGB-u2STAMaxRGBThd1) / (u2STAMaxRGBThd2-u2STAMaxRGBThd1));
                            }
                            else if(MaxRGB <= u2STAMaxRGBThd1)
                            {
                                MaxRGBWeight = WeightBase;
                            }
                            else
                            {
                                MaxRGBWeight = 0;
                            }

                            if(MinRGB > u2STAMinRGBThd1 && MinRGB < u2STAMinRGBThd2)
                            {
                                MinRGBWeight = WeightBase * (MinRGB-u2STAMinRGBThd1) / (u2STAMinRGBThd2-u2STAMinRGBThd1);
                            }
                            else if(MinRGB <= u2STAMinRGBThd1)
                            {
                                MinRGBWeight = 0;
                            }
                            else
                            {
                                MinRGBWeight = WeightBase;
                            }

                            FinalWeight = (MinRGBWeight + MaxRGBWeight + SaturationWeight)/3;

#ifdef STITCHING_USE_AE
                            AvgRRight += (tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgR*FinalWeight/WeightBase);
                            AvgGRight += (tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgG*FinalWeight/WeightBase);
                            AvgBRight += (tAeAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgB*FinalWeight/WeightBase);
#else
                            AvgRRight += (tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgR*FinalWeight/WeightBase);
                            AvgGRight += (tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgG*FinalWeight/WeightBase);
                            AvgBRight += (tAwbAvg[nDevRight][nChRight].nAvg[indexSTARight].uAvgB*FinalWeight/WeightBase);
#endif
                            AvgCntRight += (CountBase * FinalWeight / WeightBase);
                        }
                    }
                }
            }
            
            AvgRLeft = AvgRLeft * CountBase / AvgCntLeft;
            AvgRRight = AvgRRight * CountBase / AvgCntRight;
            AvgGLeft = AvgGLeft * CountBase / AvgCntLeft;
            AvgGRight = AvgGRight * CountBase / AvgCntRight;
            AvgBLeft = AvgBLeft * CountBase / AvgCntLeft;
            AvgBRight = AvgBRight * CountBase / AvgCntRight;
            tStitchingSTA[i][0][mStitchingFrameCount].u4AvgR = AvgRLeft;
            tStitchingSTA[i][1][mStitchingFrameCount].u4AvgR = AvgRRight;
            tStitchingSTA[i][0][mStitchingFrameCount].u4AvgG = AvgGLeft;
            tStitchingSTA[i][1][mStitchingFrameCount].u4AvgG = AvgGRight;
            tStitchingSTA[i][0][mStitchingFrameCount].u4AvgB = AvgBLeft;
            tStitchingSTA[i][1][mStitchingFrameCount].u4AvgB = AvgBRight;
        }
        mStitchingFrameCount ++;
        
    } 
    
    //[STITCHING_ALIGN] While all buffer is set, start to align IQ with statistics and awb gain.
    if (mStitchingFrameCount>=u2CollectFrameNum)
    {
        if (debug_level>DBGLVL_DUMPSTA)
        {
            CLRALIGN_DBG("Start dump STA\n");
            FILE *myFile = NULL;
            char filename[128];
            for(i=0x0; i < uValidCameraNum; i++)
            {
                 MI_U8 index = i;
                 MI_U8 nDev = mCameraMap[index].uDev;
                 MI_U8 nCh = mCameraMap[index].uCh;
#ifdef STITCHING_USE_AE
                 MI_U32 nSTACol = tAeAvg[nDev][nCh].nBlkX;
                 MI_U32 nSTARow = tAeAvg[nDev][nCh].nBlkY;
#else
                 MI_U32 nSTACol = tAwbAvg[nDev][nCh].nBlkX;
                 MI_U32 nSTARow = tAwbAvg[nDev][nCh].nBlkY;
#endif

                 sprintf(filename, "STA_Index%d.csv",index);
                 myFile = fopen(filename, "w+");
                 MI_U32 maxLength = nSTARow*nSTACol - 1;
                 for(int y=0;y<nSTARow;++y)
                 {
                     MI_U32 RowIndexShift = y * nSTACol;
                     for(int x=0;x<nSTACol;++x,++RowIndexShift)
                     {
#ifdef STITCHING_USE_AE
                         fprintf(myFile, "%d,%d,%d\n", tAeAvg[nDev][nCh].nAvg[RowIndexShift].uAvgR,  tAeAvg[nDev][nCh].nAvg[RowIndexShift].uAvgG, tAeAvg[nDev][nCh].nAvg[RowIndexShift].uAvgB);
#else
                         fprintf(myFile, "%d,%d,%d\n", tAwbAvg[nDev][nCh].nAvg[RowIndexShift].uAvgR,  tAwbAvg[nDev][nCh].nAvg[RowIndexShift].uAvgG, tAwbAvg[nDev][nCh].nAvg[RowIndexShift].uAvgB);
#endif
                     }
                 }
                 fclose(myFile);
            }
        }

        if(debug_level>DBGLVL_ALIGNPROCESS)
        {
            CLRALIGN_DBG("MaxRGB_THD1 = %d, MaxRGB_THD2 = %d, MinRGB_THD1 = %d, MinRGB_THD2 = %d, Saturation_THD1=%d, Saturation_THD2 = %d\n", u2STAMaxRGBThd1, u2STAMaxRGBThd2, u2STAMinRGBThd1, u2STAMinRGBThd2, u2STASaturationThd1, u2STASaturationThd2);
            CLRALIGN_DBG("Inlier ratio=%d\n", u2STAInlierRatio);
        }
        //Calculate median and inlier mean.
        for(i=0x0; i < uValidCameraNum-1; i++)
        {
            MI_U8 indexRight = i;
            MI_U8 indexLeft = i+1;

            MI_U8 nDevLeft = mCameraMap[indexLeft].uDev;  
            MI_U8 nChLeft = mCameraMap[indexLeft].uCh;
            MI_U8 nDevRight = mCameraMap[indexRight].uDev;
            MI_U8 nChRight = mCameraMap[indexRight].uCh;

            std::sort(tStitchingSTA[i][0],tStitchingSTA[i][0]+u2CollectFrameNum,Stitching_STA_t::compOnR);
            std::sort(tStitchingSTA[i][1],tStitchingSTA[i][1]+u2CollectFrameNum,Stitching_STA_t::compOnR);
            std::sort(tStitchingSTA[i][0],tStitchingSTA[i][0]+u2CollectFrameNum,Stitching_STA_t::compOnG);
            std::sort(tStitchingSTA[i][1],tStitchingSTA[i][1]+u2CollectFrameNum,Stitching_STA_t::compOnG);
            std::sort(tStitchingSTA[i][0],tStitchingSTA[i][0]+u2CollectFrameNum,Stitching_STA_t::compOnB);
            std::sort(tStitchingSTA[i][1],tStitchingSTA[i][1]+u2CollectFrameNum,Stitching_STA_t::compOnB);
            //median.
            tStitchingSTAMedian[i][0].u4AvgR = (tStitchingSTA[i][0][u2CollectFrameNum/2-1].u4AvgR+tStitchingSTA[i][0][u2CollectFrameNum/2].u4AvgR)/2;
            tStitchingSTAMedian[i][1].u4AvgR = (tStitchingSTA[i][1][u2CollectFrameNum/2-1].u4AvgR+tStitchingSTA[i][1][u2CollectFrameNum/2].u4AvgR)/2;
            tStitchingSTAMedian[i][0].u4AvgG = (tStitchingSTA[i][0][u2CollectFrameNum/2-1].u4AvgG+tStitchingSTA[i][0][u2CollectFrameNum/2].u4AvgG)/2;
            tStitchingSTAMedian[i][1].u4AvgG = (tStitchingSTA[i][1][u2CollectFrameNum/2-1].u4AvgG+tStitchingSTA[i][1][u2CollectFrameNum/2].u4AvgG)/2;
            tStitchingSTAMedian[i][0].u4AvgB = (tStitchingSTA[i][0][u2CollectFrameNum/2-1].u4AvgB+tStitchingSTA[i][0][u2CollectFrameNum/2].u4AvgB)/2;
            tStitchingSTAMedian[i][1].u4AvgB = (tStitchingSTA[i][1][u2CollectFrameNum/2-1].u4AvgB+tStitchingSTA[i][1][u2CollectFrameNum/2].u4AvgB)/2;

            //inlier mean
            tStitchingSTAMean[i][0].u4AvgR = 0;
            tStitchingSTAMean[i][1].u4AvgR = 0;
            tStitchingSTAMean[i][0].u4AvgG = 0;
            tStitchingSTAMean[i][1].u4AvgG = 0;
            tStitchingSTAMean[i][0].u4AvgB = 0;
            tStitchingSTAMean[i][1].u4AvgB = 0;
            for(int inlierIdx = u2CollectFrameNum*(STITCHING_STA_INLIER_RATIO_BASE - u2STAInlierRatio)/(STITCHING_STA_INLIER_RATIO_BASE*2);\
                inlierIdx<u2CollectFrameNum*(STITCHING_STA_INLIER_RATIO_BASE + u2STAInlierRatio)/(STITCHING_STA_INLIER_RATIO_BASE*2);\
                inlierIdx++)
            {
                tStitchingSTAMean[i][0].u4AvgR += tStitchingSTA[i][0][inlierIdx].u4AvgR;
                tStitchingSTAMean[i][1].u4AvgR += tStitchingSTA[i][1][inlierIdx].u4AvgR;
                tStitchingSTAMean[i][0].u4AvgG += tStitchingSTA[i][0][inlierIdx].u4AvgG;
                tStitchingSTAMean[i][1].u4AvgG += tStitchingSTA[i][1][inlierIdx].u4AvgG;
                tStitchingSTAMean[i][0].u4AvgB += tStitchingSTA[i][0][inlierIdx].u4AvgB;
                tStitchingSTAMean[i][1].u4AvgB += tStitchingSTA[i][1][inlierIdx].u4AvgB;
            }

            tStitchingSTAMean[i][0].u4AvgR = tStitchingSTAMean[i][0].u4AvgR*STITCHING_STA_INLIER_RATIO_BASE*100/(u2CollectFrameNum*u2STAInlierRatio);
            tStitchingSTAMean[i][1].u4AvgR = tStitchingSTAMean[i][1].u4AvgR*STITCHING_STA_INLIER_RATIO_BASE*100/(u2CollectFrameNum*u2STAInlierRatio);
            tStitchingSTAMean[i][0].u4AvgG = tStitchingSTAMean[i][0].u4AvgG*STITCHING_STA_INLIER_RATIO_BASE*100/(u2CollectFrameNum*u2STAInlierRatio);
            tStitchingSTAMean[i][1].u4AvgG = tStitchingSTAMean[i][1].u4AvgG*STITCHING_STA_INLIER_RATIO_BASE*100/(u2CollectFrameNum*u2STAInlierRatio);
            tStitchingSTAMean[i][0].u4AvgB = tStitchingSTAMean[i][0].u4AvgB*STITCHING_STA_INLIER_RATIO_BASE*100/(u2CollectFrameNum*u2STAInlierRatio);
            tStitchingSTAMean[i][1].u4AvgB = tStitchingSTAMean[i][1].u4AvgB*STITCHING_STA_INLIER_RATIO_BASE*100/(u2CollectFrameNum*u2STAInlierRatio);
        }

        //Find reference channel.
        MI_U8 MaxIndex = 0;
        MI_U32 MaxScale = 0;
        MI_U32 ChangeFlag = 0;
        MI_U32 CurScale = 1024;
        MI_U32 CurIndex = 0;
        MI_U32 PreSign = 0;

        if(bRetriggerFindMaxIndex==1)
        {
            for(int i=0x0; i < uValidCameraNum-1; i++)
            {
                if(i!=0)
                {
                    MI_U32 CurSign = tStitchingSTAMean[i][0].u4AvgG > tStitchingSTAMean[i][1].u4AvgG? 0:1;
                    ChangeFlag = CurSign!=PreSign?1:0;
                }

                if(ChangeFlag == 1)
                {
                    if(CurScale>MaxScale)
                    {
                        MaxScale = CurScale;
                        MaxIndex = CurIndex;
                    }
                    CurScale = 1024;
                    CurIndex = i;
                }

                if(tStitchingSTAMean[i][0].u4AvgG > tStitchingSTAMean[i][1].u4AvgG)
                {
                    CurScale = (tStitchingSTAMean[i][0].u4AvgG==0 || tStitchingSTAMean[i][1].u4AvgG==0)?CurScale:tStitchingSTAMean[i][0].u4AvgG*CurScale / tStitchingSTAMean[i][1].u4AvgG;
                    PreSign = 0;
                }else{
                    CurScale = (tStitchingSTAMean[i][0].u4AvgG==0 || tStitchingSTAMean[i][1].u4AvgG==0)?CurScale:tStitchingSTAMean[i][1].u4AvgG*CurScale / tStitchingSTAMean[i][0].u4AvgG;
                    PreSign = 1;
                    CurIndex = i+1;
                }

                if(i==uValidCameraNum-2)
                {
                    if(CurScale>MaxScale)
                    {
                        MaxScale = CurScale;
                        MaxIndex = CurIndex;
                    }
                }

                if (debug_level > DBGLVL_ALIGNPROCESS)
                {
                    CLRALIGN_DBG("CurScale=%d, CurIndex=%d, PreSign=%d, ChangeFlag=%d, MaxScale=%d, MaxIndex=%d\n", CurScale, CurIndex, PreSign, ChangeFlag, MaxScale, MaxIndex);
                }
            }
            bRetriggerFindMaxIndex = 0;
            uPreMaxIndex = MaxIndex;
        }
        else
        {
            MaxIndex = uPreMaxIndex;
        }

        if(debug_level>DBGLVL_ALIGNPROCESS)
        {
            CLRALIGN_DBG("RefIndex=%d\n", MaxIndex);
        }
        
        MI_U32 RadjustCur;
        MI_U32 GadjustCur;
        MI_U32 BadjustCur;
        MI_U32 RadjustAlign;
        MI_U32 GadjustAlign;
        MI_U32 BadjustAlign;
        // Align Ref from Left to Right.
        for(i=MaxIndex; i < uValidCameraNum-1; i++)
        {
            MI_U8 indexRight = i;
            MI_U8 indexLeft = i+1;
            MI_U8 nDevLeft = mCameraMap[indexLeft].uDev;  //mObjIqController[i]
            MI_U8 nChLeft = mCameraMap[indexLeft].uCh;
            MI_U8 nDevRight = mCameraMap[indexRight].uDev;
            MI_U8 nChRight = mCameraMap[indexRight].uCh;


            if(debug_level>DBGLVL_ALIGNPROCESS)
            {
                CLRALIGN_DBG("Align Right to Left\n");
                CLRALIGN_DBG("indexLeft =%d, indexRight=%d\n",indexLeft,indexRight);
                CLRALIGN_DBG("nDevLeft =%d, nChLeft=%d, nDevRight =%d, nChRight=%d\n",nDevLeft,nChLeft,nDevRight,nChRight);
            }

            if(i==MaxIndex)
            {
                RadjustCur = STITCHING_ADJUST_BASE;
                GadjustCur = STITCHING_ADJUST_BASE;
                BadjustCur = STITCHING_ADJUST_BASE;
                mStitchingRefAdjust[nDevRight][nChRight].u2AdjustR = RadjustCur;
                mStitchingRefAdjust[nDevRight][nChRight].u2AdjustG = GadjustCur;
                mStitchingRefAdjust[nDevRight][nChRight].u2AdjustB = BadjustCur;
                
                if(debug_level>DBGLVL_ALIGNPROCESS)
                {
                    CLRALIGN_DBG("RadjustCur =%d, GadjustCur =%d, BadjustCur=%d(HEAD)\n",RadjustCur,GadjustCur,BadjustCur);
                }
            }
            else 
            {
                if(debug_level>DBGLVL_ALIGNPROCESS)
                {
                    CLRALIGN_DBG("RadjustCur =%d, GadjustCur =%d, BadjustCur=%d(NODE)\n",RadjustCur,GadjustCur,BadjustCur);
                }
            }

            if(!(tStitchingSTAMean[i][0].u4AvgR  == 0 || tStitchingSTAMean[i][0].u4AvgG  == 0 || tStitchingSTAMean[i][0].u4AvgB == 0 ))
            {
                RadjustAlign = tStitchingSTAMean[i][1].u4AvgR * RadjustCur / tStitchingSTAMean[i][0].u4AvgR;
                GadjustAlign = tStitchingSTAMean[i][1].u4AvgG * GadjustCur / tStitchingSTAMean[i][0].u4AvgG;
                BadjustAlign = tStitchingSTAMean[i][1].u4AvgB * BadjustCur / tStitchingSTAMean[i][0].u4AvgB;
            }

            if(tStitchingSTAMean[i][0].u4AvgR < u2LowLuxNotApplyThd || tStitchingSTAMean[i][0].u4AvgG  < u2LowLuxNotApplyThd || tStitchingSTAMean[i][0].u4AvgB  < u2LowLuxNotApplyThd ||
               tStitchingSTAMean[i][1].u4AvgR  < u2LowLuxNotApplyThd || tStitchingSTAMean[i][1].u4AvgG  < u2LowLuxNotApplyThd || tStitchingSTAMean[i][1].u4AvgB < u2LowLuxNotApplyThd)
            {
                RadjustAlign = RadjustCur;
                GadjustAlign = GadjustCur;
                BadjustAlign = BadjustCur;
            }else
            {
                RadjustCur = RadjustAlign;
                GadjustCur = GadjustAlign;
                BadjustCur = BadjustAlign;
            }
            
            if(debug_level>DBGLVL_ALIGNPROCESS)
            {
                CLRALIGN_DBG("AvgRLeft =%d, AvgRRight=%d, AvgGLeft=%d ,AvgGRight=%d,AvgBLeft=%d ,AvgBRight=%d (%d)\n",   \
                          tStitchingSTAMean[i][0].u4AvgR,  \
                          tStitchingSTAMean[i][1].u4AvgR,  \
                          tStitchingSTAMean[i][0].u4AvgG,  \
                          tStitchingSTAMean[i][1].u4AvgG,  \
                          tStitchingSTAMean[i][0].u4AvgB,  \
                          tStitchingSTAMean[i][1].u4AvgB,  \
                          u2CollectFrameNum);
                CLRALIGN_DBG("RadjustAlign=%d, GadjustAlign=%d, BadjustAlign=%d\n", RadjustAlign,GadjustAlign,BadjustAlign);
            }

            mStitchingRefAdjust[nDevLeft][nChLeft].u2AdjustR = RadjustAlign;
            mStitchingRefAdjust[nDevLeft][nChLeft].u2AdjustG = GadjustAlign;
            mStitchingRefAdjust[nDevLeft][nChLeft].u2AdjustB = BadjustAlign;
        }

        // Align Ref from Right to Left.
        for(i=MaxIndex; i > 0; i--)
        {
            MI_U8 indexRight = i-1;
            MI_U8 indexLeft = i;
            MI_U8 nDevLeft = mCameraMap[indexLeft].uDev;  //mObjIqController[i]
            MI_U8 nChLeft = mCameraMap[indexLeft].uCh;
            MI_U8 nDevRight = mCameraMap[indexRight].uDev;
            MI_U8 nChRight = mCameraMap[indexRight].uCh;


            if(debug_level>DBGLVL_ALIGNPROCESS)
            {
                CLRALIGN_DBG("Align Left to Right\n");
                CLRALIGN_DBG("indexRight=%d, indexLeft =%d\n",indexRight,indexLeft);
                CLRALIGN_DBG("nDevRight =%d, nChRight=%d, nDevLeft =%d, nChLeft=%d\n",nDevRight,nChRight,nDevLeft,nChLeft);
            }

            if(i==MaxIndex)
            {
                RadjustCur = STITCHING_ADJUST_BASE;
                GadjustCur = STITCHING_ADJUST_BASE;
                BadjustCur = STITCHING_ADJUST_BASE;
                mStitchingRefAdjust[nDevLeft][nChLeft].u2AdjustR = RadjustCur;
                mStitchingRefAdjust[nDevLeft][nChLeft].u2AdjustG = GadjustCur;
                mStitchingRefAdjust[nDevLeft][nChLeft].u2AdjustB = BadjustCur;

                if(debug_level>DBGLVL_ALIGNPROCESS)
                {
                    CLRALIGN_DBG("RadjustCur =%d, GadjustCur =%d, BadjustCur=%d(HEAD)\n",RadjustCur,GadjustCur,BadjustCur);
                }
            }
            else 
            {
                if(debug_level>DBGLVL_ALIGNPROCESS)
                {
                    CLRALIGN_DBG("RadjustCur =%d, GadjustCur =%d, BadjustCur=%d(NODE)\n",RadjustCur,GadjustCur,BadjustCur);
                }
            }

            if(!(tStitchingSTAMean[i-1][1].u4AvgR  == 0 || tStitchingSTAMean[i-1][1].u4AvgG  == 0 || tStitchingSTAMean[i-1][1].u4AvgB == 0 ))
            {
                RadjustAlign = tStitchingSTAMean[i-1][0].u4AvgR * RadjustCur / tStitchingSTAMean[i-1][1].u4AvgR;
                GadjustAlign = tStitchingSTAMean[i-1][0].u4AvgG * GadjustCur / tStitchingSTAMean[i-1][1].u4AvgG;
                BadjustAlign = tStitchingSTAMean[i-1][0].u4AvgB * BadjustCur / tStitchingSTAMean[i-1][1].u4AvgB;
            }
            if(tStitchingSTAMean[i-1][0].u4AvgR < u2LowLuxNotApplyThd || tStitchingSTAMean[i-1][0].u4AvgG  < u2LowLuxNotApplyThd || tStitchingSTAMean[i-1][0].u4AvgB  < u2LowLuxNotApplyThd ||
               tStitchingSTAMean[i-1][1].u4AvgR  < u2LowLuxNotApplyThd || tStitchingSTAMean[i-1][1].u4AvgG  < u2LowLuxNotApplyThd || tStitchingSTAMean[i-1][1].u4AvgB < u2LowLuxNotApplyThd)
            {
                RadjustAlign = RadjustCur;
                GadjustAlign = GadjustCur;
                BadjustAlign = BadjustCur;
            }else
            {
                RadjustCur = RadjustAlign;
                GadjustCur = GadjustAlign;
                BadjustCur = BadjustAlign;
            }

            if(debug_level>DBGLVL_ALIGNPROCESS)
            {
                CLRALIGN_DBG("AvgRRight =%d, AvgRLeft=%d, AvgGRight=%d ,AvgGLeft=%d,AvgBRight=%d ,AvgBLeft=%d (%d)\n",   \
                          tStitchingSTAMean[i-1][1].u4AvgR,  \
                          tStitchingSTAMean[i-1][0].u4AvgR,  \
                          tStitchingSTAMean[i-1][1].u4AvgG,  \
                          tStitchingSTAMean[i-1][0].u4AvgG,  \
                          tStitchingSTAMean[i-1][1].u4AvgB,  \
                          tStitchingSTAMean[i-1][0].u4AvgB,  \
                          u2CollectFrameNum);
                CLRALIGN_DBG("RadjustAlign=%d, GadjustAlign=%d, BadjustAlign=%d\n", RadjustAlign,GadjustAlign,BadjustAlign);
            }

            mStitchingRefAdjust[nDevRight][nChRight].u2AdjustR = RadjustAlign;
            mStitchingRefAdjust[nDevRight][nChRight].u2AdjustG = GadjustAlign;
            mStitchingRefAdjust[nDevRight][nChRight].u2AdjustB = BadjustAlign;
        }

        mStitchingFrameCount = 0;
        
        MI_U32 bApply = FALSE;
        MI_U32 bAllNotEnable = TRUE;
        MI_U32 R;
        MI_U32 G;
        MI_U32 B;
        MI_U32 Max_converge_thd = STITCHING_CONVERGE_RATIO_BASE+u2ConvergeRatio;
        MI_U32 Min_converge_thd = STITCHING_CONVERGE_RATIO_BASE-u2ConvergeRatio;
        for(i=0; i <uValidCameraNum ; i++)
        {
            MI_U8 index = i;
            MI_U8 nDev = mCameraMap[index].uDev;  
            MI_U8 nCh = mCameraMap[index].uCh;

            if(bFirstCal == FALSE)
            {
                bAllNotEnable = FALSE;
                R = mStitchingRefAdjust[nDev][nCh].u2AdjustR * STITCHING_CONVERGE_RATIO_BASE / mStitchingApplyAdjust[nDev][nCh].u2AdjustR;
                G = mStitchingRefAdjust[nDev][nCh].u2AdjustG * STITCHING_CONVERGE_RATIO_BASE / mStitchingApplyAdjust[nDev][nCh].u2AdjustG;
                B = mStitchingRefAdjust[nDev][nCh].u2AdjustB * STITCHING_CONVERGE_RATIO_BASE / mStitchingApplyAdjust[nDev][nCh].u2AdjustB;

                bApply = (R>Max_converge_thd || R< Min_converge_thd||G>Max_converge_thd || G< Min_converge_thd||B>Max_converge_thd || B< Min_converge_thd) || bApply;
            }
            
        }

        if(bApply || bFirstCal==TRUE)
        {
            if(debug_level>DBGLVL_ALIGNPROCESS)
            {
                CLRALIGN_DBG("Align gain applied, %d, %d, %d\n", bApply, bAllNotEnable, bFirstCal);
            }
            for(i=0; i <uValidCameraNum ; i++)
            {
                MI_U8 index = i;
                MI_U8 nDev = mCameraMap[index].uDev;  
                MI_U8 nCh = mCameraMap[index].uCh;

                memcpy(&mStitchingApplyAdjust[nDev][nCh], &mStitchingRefAdjust[nDev][nCh], sizeof(Stitching_Adjust_t));
            }
            bFirstCal=FALSE;
            bAllNotEnable = FALSE;
        }
        else
        {
            if(debug_level>DBGLVL_ALIGNPROCESS)
            {
                CLRALIGN_DBG("Dicard align gain, %d, %d, %d\n", bApply, bAllNotEnable, bFirstCal);
            }
        }
        if(debug_level>DBGLVL_ALIGNPROCESS)
        {
            CLRALIGN_DBG("---------------------------------------------------------------------------------------------\n");
        }
    }

    MI_U32 bAllNotEnable = E_SS_IQ_TRUE;
    for(i=0; i <uValidCameraNum ; i++)
    {
        MI_U8 index = i;
        MI_U8 nDev = mCameraMap[index].uDev;  
        MI_U8 nCh = mCameraMap[index].uCh;
        mStitchingApplyGain[nDev][nCh].bEnable = bFirstCal == TRUE? E_SS_IQ_FALSE : E_SS_IQ_TRUE;
        mStitchingApplyGain[nDev][nCh].u16Rgain = (tIspAwbInfo.u16Rgain * mStitchingApplyAdjust[nDev][nCh].u2AdjustR)/STITCHING_ADJUST_BASE;
        mStitchingApplyGain[nDev][nCh].u16Grgain = (tIspAwbInfo.u16Grgain * mStitchingApplyAdjust[nDev][nCh].u2AdjustG)/STITCHING_ADJUST_BASE;
        mStitchingApplyGain[nDev][nCh].u16Gbgain = (tIspAwbInfo.u16Gbgain * mStitchingApplyAdjust[nDev][nCh].u2AdjustG)/STITCHING_ADJUST_BASE;
        mStitchingApplyGain[nDev][nCh].u16Bgain = (tIspAwbInfo.u16Bgain * mStitchingApplyAdjust[nDev][nCh].u2AdjustB)/STITCHING_ADJUST_BASE;
        MI_U16 IIR_SUM = uApplyIIRPre + uApplyIIRCur;
        
        mStitchingPreGain[nDev][nCh].bEnable = mStitchingApplyGain[nDev][nCh].bEnable;
        mStitchingPreGain[nDev][nCh].u16Rgain = (uApplyIIRPre*mStitchingPreGain[nDev][nCh].u16Rgain+uApplyIIRCur*mStitchingApplyGain[nDev][nCh].u16Rgain)/IIR_SUM;
        mStitchingPreGain[nDev][nCh].u16Grgain = (uApplyIIRPre*mStitchingPreGain[nDev][nCh].u16Grgain+uApplyIIRCur*mStitchingApplyGain[nDev][nCh].u16Grgain)/IIR_SUM;
        mStitchingPreGain[nDev][nCh].u16Gbgain = (uApplyIIRPre*mStitchingPreGain[nDev][nCh].u16Gbgain+uApplyIIRCur*mStitchingApplyGain[nDev][nCh].u16Gbgain)/IIR_SUM;
        mStitchingPreGain[nDev][nCh].u16Bgain = (uApplyIIRPre*mStitchingPreGain[nDev][nCh].u16Bgain+uApplyIIRCur*mStitchingApplyGain[nDev][nCh].u16Bgain)/IIR_SUM;
        mStitchingPreGain[nDev][nCh].u32YCompensate = 1024;
        if(debug_level>DBGLVL_APPLYINFO)
        {
            CLRALIGN_DBG("IIR Index=%d, Enable=%d, RgainAlign=%d, GgainAlign=%d, BgainAlign=%d\n", index, mStitchingPreGain[nDev][nCh].bEnable, mStitchingPreGain[nDev][nCh].u16Rgain,mStitchingPreGain[nDev][nCh].u16Grgain,mStitchingPreGain[nDev][nCh].u16Bgain);
        }
        MI_ISP_IQ_SetAwbAlign(nDev,nCh,&mStitchingPreGain[nDev][nCh]);
    }

    return MI_SUCCESS;
}

//ColorAligner C Wrap
ColorAligner* ColorAligner_create(Stitching_Dev_Ch_t camera_map[], MI_U32 valid_cam_num, MI_U32 image_width, MI_U32 image_height)
{
    return new ColorAligner(camera_map, valid_cam_num, image_width, image_height);
}


void ColorAligner_release(ColorAligner *self)
{
    delete self;
}

unsigned int ColorAligner_run(ColorAligner *self, unsigned int debug_level)
{
    return self->Run(debug_level);
}


