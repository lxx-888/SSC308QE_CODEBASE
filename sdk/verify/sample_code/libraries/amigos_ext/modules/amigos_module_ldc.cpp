/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include "amigos_surface_base.h"
#include "mi_ldc.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "mi_sys.h"
#include "amigos_module_init.h"
#include "amigos_module_ldc.h"
#include "mi_sys_datatype.h"
#include "ss_enum_cast.hpp"

#define MMA_ALLOC_BUF(pphy, ppvaddr, size, __exit_func__)              \
    do                                                                 \
    {                                                                  \
        *pphy = 0UL;                                                   \
        *ppvaddr = NULL;                                               \
        ret = MI_SYS_MMA_Alloc(0, NULL, size, pphy);                   \
        if (ret)                                                       \
        {                                                              \
            AMIGOS_ERR("failed to MI_SYS_MMA_Alloc Buf: %d \n", size); \
            goto __exit_func__;                                        \
        }                                                              \
        ret = MI_SYS_Mmap(*pphy, size, ppvaddr, false);                \
        if (ret)                                                       \
        {                                                              \
            AMIGOS_ERR("failed to map Buf\n");                         \
            goto __exit_func__;                                        \
        }                                                              \
    } while (0)

#define MMA_FREE_BUF(phy, pvaddr, size)  \
    do                                   \
    {                                    \
        if (pvaddr && size)              \
            MI_SYS_Munmap(pvaddr, size); \
        if (phy)                         \
            MI_SYS_MMA_Free(0, phy);     \
        phy = 0UL;                       \
        pvaddr = NULL;                   \
    } while (0)

#ifndef STCHECKRESULT
#define STCHECKRESULT(_func_)\
        do{ \
            MI_S32 s32Ret = MI_SUCCESS; \
            s32Ret = _func_; \
            if (s32Ret != MI_SUCCESS)\
            { \
                printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
                return s32Ret; \
            } \
            else \
            { \
                printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__); \
            } \
        } while(0)
#endif
#ifndef STCHECKNORET
#define STCHECKNORET(_func_)\
        do{ \
            MI_S32 s32Ret = MI_SUCCESS; \
            s32Ret = _func_; \
            if (s32Ret != MI_SUCCESS)\
            { \
                printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            } \
            else \
            { \
                printf("(%s %d)exec function pass\n", __FUNCTION__,__LINE__); \
            } \
        } while(0)
#endif

SS_ENUM_CAST_STR(MI_LDC_WorkMode_e,
{
    {MI_LDC_WORKMODE_LDC,            "ldc"},
    {MI_LDC_WORKMODE_LUT,            "lut"},
    {MI_LDC_WORKMODE_DIS,            "dis"},
    {MI_LDC_WORKMODE_PMF,            "pmf"},
    {MI_LDC_WORKMODE_STITCH,         "stitch"},
    {MI_LDC_WORKMODE_NIR,            "nir"},
    {MI_LDC_WORKMODE_DPU,            "dpu"},
    {MI_LDC_WORKMODE_LDC_HORIZONTAL, "ldc_horizontal"},
    {MI_LDC_WORKMODE_DIS_LDC,        "dis_ldc"},
});

std::map<unsigned int, unsigned int> AmigosModuleLdc::mapLdcCreateDev;

static inline int GetFileSize(const char *path)
{
    FILE *fp        = NULL;
    int   totalSize = 0;

    fp = fopen(path, "rb");
    if (!fp)
    {
        AMIGOS_ERR("failed to open:%s \n", path);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    totalSize = ftell(fp);
    fclose(fp);
    return totalSize;
}
static inline int ReadFileToBuf(const char *path, void *pBuf, unsigned int Length)
{
    int   ret = -1, readSize = 0, totalSize = 0, buflen = (int)Length;
    FILE *fp = NULL;

    if (!pBuf)
    {
        return -1;
    }

    fp = fopen(path, "rb");
    if (!fp)
    {
        AMIGOS_ERR("failed to open:%s \n", path);
        goto __exit_err__;
    }

    fseek(fp, 0, SEEK_END);
    totalSize = ftell(fp);
    if (totalSize > buflen)
    {
        AMIGOS_ERR("%s file content is less need:%d %d\n", path, totalSize, buflen);
        goto __exit_err__;
    }

    fseek(fp, 0, SEEK_SET);

    readSize = fread(pBuf, 1, totalSize, fp);
    if (readSize != totalSize)
    {
        AMIGOS_ERR("failed to read buf:%s %p %d %d!\n", path, pBuf, readSize, buflen);
        goto __exit_err__;
    }

    fclose(fp);
    return 0;

__exit_err__:
    if (fp)
        fclose(fp);
    return ret;
}

static void *pVirMap2BinX[LDC_MAX_REGION_NUM], *pVirMap2BinY[LDC_MAX_REGION_NUM];
static MI_PHY phyMap2BinX[LDC_MAX_REGION_NUM],phyMap2BinY[LDC_MAX_REGION_NUM];

int AmigosModuleLdc::Ldc_Init()
{
    int                  ret;
    MI_LDC_ChnLDCAttr_t stChnLDCAttr;
    memset(&stChnLDCAttr,0,sizeof(MI_LDC_ChnLDCAttr_t));
    stChnLDCAttr.bBgColor                                        = stLdcInfo.LdcCfg.EnBgColor==1?TRUE:FALSE;
    stChnLDCAttr.u32BgColor                                      = stLdcInfo.LdcCfg.BgColor;
    stChnLDCAttr.eMountMode                                      = (MI_LDC_MountMode_e)stLdcInfo.LdcCfg.MountMode;
    stChnLDCAttr.stCalibInfo.u32CalibPolyBinSize                 = stLdcInfo.CalibInfo.len;
    stChnLDCAttr.stCalibInfo.pCalibPolyBinAddr                   = stLdcInfo.CalibInfo.addr;
    stChnLDCAttr.stCalibInfo.s32CenterXOffset                    = stLdcInfo.LdcCfg.Center_X_Off;
    stChnLDCAttr.stCalibInfo.s32CenterYOffset                    = stLdcInfo.LdcCfg.Center_Y_Off;
    stChnLDCAttr.stCalibInfo.s32FisheyeRadius                    = stLdcInfo.LdcCfg.FisheyeRadius;
    stChnLDCAttr.u32RegionNum                                    = stLdcInfo.LdcCfg.RegionNum;
    for(unsigned int i = 0; i < stChnLDCAttr.u32RegionNum && i < LDC_MAX_REGION_NUM; i++)
    {
        stChnLDCAttr.stRegionAttr[i].eRegionMode                     = (MI_LDC_RegionMode_e)stLdcInfo.LdcCfg.Region[i].RegionMode;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16X                  = stLdcInfo.LdcCfg.Region[i].OutRect.x;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Y                  = stLdcInfo.LdcCfg.Region[i].OutRect.y;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Width              = stLdcInfo.LdcCfg.Region[i].OutRect.width;
        stChnLDCAttr.stRegionAttr[i].stOutRect.u16Height             = stLdcInfo.LdcCfg.Region[i].OutRect.height;
        if(stChnLDCAttr.stRegionAttr[i].eRegionMode == MI_LDC_REGION_MAP2BIN)
        {
            stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32Grid         = stLdcInfo.LdcCfg.Region[i].map2binPara.grid;
            stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32XmapSize     = GetFileSize(stLdcInfo.LdcCfg.Region[i].map2binPara.mapX.c_str());
            stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32YmapSize     = GetFileSize(stLdcInfo.LdcCfg.Region[i].map2binPara.mapY.c_str());
            if(stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32XmapSize == (unsigned int)-1 || stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32YmapSize == (unsigned int)-1)
            {
                AMIGOS_ERR("region[%u] Xmap OR Ymap file err\n",i);
                goto __exit_ldc_err;
            }
            MMA_ALLOC_BUF(&phyMap2BinX[i], &pVirMap2BinX[i], stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32XmapSize, __exit_ldc_err);
            MMA_ALLOC_BUF(&phyMap2BinY[i], &pVirMap2BinY[i], stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32YmapSize, __exit_ldc_err);
            //AMIGOS_ERR("Xmap bufsize=%d.Ymap bufsize=%d\n",stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32XmapSize,stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32YmapSize);
            stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.pXmapAddr       = pVirMap2BinX[i];
            stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.pYmapAddr       = pVirMap2BinY[i];
        }
        else
        {
            stChnLDCAttr.stRegionAttr[i].stRegionPara.eCropMode          = (MI_LDC_RegionCropMode_e)stLdcInfo.LdcCfg.Region[i].Para.CropMode;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32DistortionRatio = stLdcInfo.LdcCfg.Region[i].Para.DistortionRatio;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32FocalRatio      = stLdcInfo.LdcCfg.Region[i].Para.FocalRatio;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32InRadius        = stLdcInfo.LdcCfg.Region[i].Para.InRadius;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRadius       = stLdcInfo.LdcCfg.Region[i].Para.OutRadius;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32OutRotate       = stLdcInfo.LdcCfg.Region[i].Para.OutRot;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Pan             = stLdcInfo.LdcCfg.Region[i].Para.Pan;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32Tilt            = stLdcInfo.LdcCfg.Region[i].Para.Tilt;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomH           = stLdcInfo.LdcCfg.Region[i].Para.ZoomH;
            stChnLDCAttr.stRegionAttr[i].stRegionPara.s32ZoomV           = stLdcInfo.LdcCfg.Region[i].Para.ZoomV;
        }
    }
    STCHECKRESULT(MI_LDC_SetChnLDCAttr(stModInfo.devId, stModInfo.chnId, &stChnLDCAttr));
    return MI_SUCCESS;
__exit_ldc_err:
    AMIGOS_ERR("__exit_ldc_err\n");
    for(unsigned int i = 0;i < stChnLDCAttr.u32RegionNum && i < LDC_MAX_REGION_NUM; i++)
    {
        MMA_FREE_BUF(phyMap2BinX[i], pVirMap2BinX[i], stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32XmapSize);
        MMA_FREE_BUF(phyMap2BinY[i], pVirMap2BinY[i], stChnLDCAttr.stRegionAttr[i].stRegionMapInfo.u32YmapSize);
    }
    return -1;
}
/*
//TODO 应该禁止两个LDC同时使用Lut
static MI_LDC_DirectBuf_t  stTableX, stTableY, stResult, stTableWeight;
static void *pVirTableX, *pVirTableY, *pVirTableWeight, *pVirResult;

int AmigosModuleLdc::Lut_Init()
{
    int                  ret;
    unsigned int         u32WeightSize, u32TableSize;
    MI_LDC_LutTaskAttr_t stLutTask;

#define LUT_TABLE_Init(table, w, h)                   \
    do                                                \
    {                                                 \
        table.ePixelFormat = E_MI_SYS_PIXEL_FRAME_I8; \
        table.u32Width     = w;                       \
        table.u32Height    = h;                       \
        table.u32Stride[0] = w;                       \
    } while (0)

    do
    {
        u32WeightSize = GetFileSize(LutCfg.Table_Weight);
        if (u32WeightSize <= 0)
            return -1;

        LUT_TABLE_Init(stTableX, stLdcInfo.LutCfg.Width, stLdcInfo.LutCfg.Height);
        LUT_TABLE_Init(stTableY, stLdcInfo.LutCfg.Width, stLdcInfo.LutCfg.Height);
        LUT_TABLE_Init(stTableWeight, stLdcInfo.LutCfg.Width, stLdcInfo.LutCfg.Height);
        LUT_TABLE_Init(stResult, stLdcInfo.LutCfg.Width, stLdcInfo.LutCfg.Height);

        u32TableSize = stLdcInfo.LutCfg.Width * stLdcInfo.LutCfg.Height;

        MMA_ALLOC_BUF(&stTableX.phyAddr[0], &pVirTableX, u32TableSize, __exit_lut_err);
        MMA_ALLOC_BUF(&stTableY.phyAddr[0], &pVirTableY, u32TableSize, __exit_lut_err);
        MMA_ALLOC_BUF(&stTableWeight.phyAddr[0], &pVirTableWeight, u32WeightSize, __exit_lut_err);
        MMA_ALLOC_BUF(&stResult.phyAddr[0], &pVirResult, u32TableSize, __exit_lut_err);

        ReadFileToBuf(stLdcInfo.LutCfg.Table_X, pVirTableX, u32TableSize);
        ReadFileToBuf(stLdcInfo.LutCfg.Table_Y, pVirTableY, u32TableSize);
        ReadFileToBuf(stLdcInfo.LutCfg.Table_Weight, pVirTableWeight, u32WeightSize);

        stLutTask.stSrcBuf.stTableX      = stTableX;
        stLutTask.stSrcBuf.stTableY      = stTableY;
        stLutTask.stSrcBuf.stTableWeight = stTableWeight;
        stLutTask.stDstBuf               = stResult;

        ret = MI_LDC_DoLutDirectTask(stModInfo.devId, stModInfo.chnId, &stLutTask);
        if (ret)
        {
            AMIGOS_ERR("MI_LDC_DoLutDirectTask failed: 0x%x \n", ret);
            goto __exit_lut_err;
        }
        break;

    __exit_lut_err:
        MMA_FREE_BUF(stTableX.phyAddr[0], pVirTableX, u32TableSize);
        MMA_FREE_BUF(stTableY.phyAddr[0], pVirTableY, u32TableSize);
        MMA_FREE_BUF(stTableWeight.phyAddr[0], pVirTableWeight, u32WeightSize);
        MMA_FREE_BUF(stResult.phyAddr[0], pVirResult, u32TableSize);
        return -1;
    } while (0);
    return MI_SUCCESS;
}*/

int AmigosModuleLdc::Dis_Init()
{
    MI_LDC_ChnDISAttr_t stChnDISAttr;
    memset(&stChnDISAttr,0,sizeof(MI_LDC_ChnDISAttr_t));
    stChnDISAttr.eMode           = (MI_LDC_DISMode_e)stLdcInfo.DisCfg.DisMode;
    if(MI_LDC_DIS_GME_6DOF == stChnDISAttr.eMode || MI_LDC_DIS_GME_8DOF == stChnDISAttr.eMode)
    {
        stChnDISAttr.eSceneType = (MI_LDC_DISSceneType_e)stLdcInfo.DisCfg.SceneType;
        stChnDISAttr.eMotionLevel = (MI_LDC_DISMotionLevel_e)stLdcInfo.DisCfg.MotionLevel;
        stChnDISAttr.u8CropRatio = stLdcInfo.DisCfg.CropRatio;
    }
    else if(MI_LDC_DIS_GYRO == stChnDISAttr.eMode)
    {
        std::string tmpVal;
        int i = 0;
        AMIGOS_DIV_STR_FOR_EACH(stLdcInfo.DisCfg.RotationMatrix.c_str(), ",", tmpVal)
        {
            stChnDISAttr.as32RotationMatrix[i] = atoi(tmpVal.c_str());
            i++;
        }
        stChnDISAttr.u32UserSliceNum = stLdcInfo.DisCfg.UserSliceNum;
        stChnDISAttr.u32FocalLengthX = stLdcInfo.DisCfg.FocalLengthX;
        stChnDISAttr.u32FocalLengthY = stLdcInfo.DisCfg.FocalLengthY;
    }
    STCHECKRESULT(MI_LDC_SetChnDISAttr(stModInfo.devId, stModInfo.chnId, &stChnDISAttr));
    return MI_SUCCESS;
}

int AmigosModuleLdc::Pmf_Init()
{
    MI_LDC_ChnPMFAttr_t stChnPMFAttr;
    memset(&stChnPMFAttr,0,sizeof(MI_LDC_ChnPMFAttr_t));
    std::string tmpVal;
    int i = 0;
    AMIGOS_DIV_STR_FOR_EACH(stLdcInfo.PmfCfg.PmfCoef.c_str(), ",", tmpVal)
    {
        stChnPMFAttr.as64PMFCoef[i] = atoi(tmpVal.c_str());
        i++;
    }
    STCHECKRESULT(MI_LDC_SetChnPMFAttr(stModInfo.devId, stModInfo.chnId, &stChnPMFAttr));
    return MI_SUCCESS;
}

int AmigosModuleLdc::Stitch_Init()
{
    MI_LDC_ChnStitchAttr_t stChnStitchAttr;
    memset(&stChnStitchAttr,0,sizeof(MI_LDC_ChnStitchAttr_t));
    stChnStitchAttr.eProjType                = (MI_LDC_ProjectionMode_e)stLdcInfo.StitchCfg.ProjType;
    stChnStitchAttr.s32Distance              = stLdcInfo.StitchCfg.Distance;
    stChnStitchAttr.stCalCfg.u32CalibCfgSize = stLdcInfo.CalibInfo.len;
    stChnStitchAttr.stCalCfg.pCalibCfgAddr   = stLdcInfo.CalibInfo.addr;

    STCHECKRESULT(MI_LDC_SetChnStitchAttr(stModInfo.devId, stModInfo.chnId, &stChnStitchAttr));
    return MI_SUCCESS;
}

int AmigosModuleLdc::Nir_Init()
{
    MI_LDC_ChnNIRAttr_t stChnNIRAttr;
    memset(&stChnNIRAttr,0,sizeof(MI_LDC_ChnNIRAttr_t));
    stChnNIRAttr.s32Distance              = stLdcInfo.NirCfg.Distance;
    stChnNIRAttr.stCalCfg.u32CalibCfgSize = stLdcInfo.CalibInfo.len;
    stChnNIRAttr.stCalCfg.pCalibCfgAddr   = stLdcInfo.CalibInfo.addr;

    STCHECKRESULT(MI_LDC_SetChnNIRAttr(stModInfo.devId, stModInfo.chnId, &stChnNIRAttr));
    return MI_SUCCESS;
}
/*
int AmigosModuleLdc::Dpu_Init()
{
    MI_LDC_ChnDPUAttr_t stChnDPUAttr;
    stChnDPUAttr.s32Distance              = stLdcInfo.DpuCfg.Distance;
    stChnDPUAttr.stCalCfg.u32CalibCfgSize = stLdcInfo.CalibInfo.len;
    stChnDPUAttr.stCalCfg.pCalibCfgAddr   = stLdcInfo.CalibInfo.addr;

    MI_LDC_SetChnDPUAttr(stLdcInfo.stModInfo.devId, stLdcInfo.stModInfo.chnId, &stChnDPUAttr);
    return MI_SUCCESS;
}

int AmigosModuleLdc::LdcHorizontal_Init()
{
    MI_LDC_ChnLDCHorAttr_t stChnLDCHorAttr;
    stChnLDCHorAttr.bCropEn                  = true;
    stChnLDCHorAttr.s32DistortionRatio       = stLdcInfo.LdcHorizontalCfg.DistortionRatio;
    stChnLDCHorAttr.stCalCfg.u32CalibCfgSize = stLdcInfo.CalibInfo.len;
    stChnLDCHorAttr.stCalCfg.pCalibCfgAddr   = stLdcInfo.CalibInfo.addr;

    MI_LDC_SetChnLDCHorAttr(stModInfo.devId, stModInfo.chnId, &stChnLDCHorAttr);
    return MI_SUCCESS;
}
*/
AmigosModuleLdc::AmigosModuleLdc(const std::string &strInSection)
    : AmigosSurfaceLdc(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleLdc::~AmigosModuleLdc()
{
}
unsigned int AmigosModuleLdc::GetModId() const
{
    return E_MI_MODULE_ID_LDC;
}
unsigned int AmigosModuleLdc::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleLdc::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
void AmigosModuleLdc::_ResourceInit()
{
    std::map<unsigned int, unsigned int>::iterator itMapLdcCreateDev;
    itMapLdcCreateDev = mapLdcCreateDev.find(stModInfo.devId);
    if (itMapLdcCreateDev == mapLdcCreateDev.end())
    {
        mapLdcCreateDev[stModInfo.devId] = 0;
    }
    mapLdcCreateDev[stModInfo.devId]++;
}
void AmigosModuleLdc::_ResourceDeinit()
{
    auto itMapLdcCreateDev = mapLdcCreateDev.find(stModInfo.devId);
    if (itMapLdcCreateDev != mapLdcCreateDev.end())
    {
       itMapLdcCreateDev->second--;
       if(!itMapLdcCreateDev->second)
       {
           mapLdcCreateDev.erase(itMapLdcCreateDev);
       }
    }
}
void AmigosModuleLdc::_Init()
{
    MI_LDC_DevAttr_t        stDevAttr;
    MI_LDC_ChnAttr_t        stChnAttr;
    MI_LDC_InputPortAttr_t  InputAttr;
    MI_LDC_OutputPortAttr_t OutputAttr;
    MI_SYS_ChnPort_t        stChnPort;
    std::map<unsigned int, LdcOutInfo>::iterator itMapLdcOut;
    std::map<unsigned int, LdcInInfo>::iterator itMapLdcIn;
    std::map<unsigned int, unsigned int>::iterator itMapLdcCreateDev;
    AmigosSurfaceBase::ModPortInInfo stInPortInfo;

    memset(&stDevAttr, 0, sizeof(MI_LDC_DevAttr_t));
    memset(&stChnAttr, 0, sizeof(MI_LDC_ChnAttr_t));
    memset(&InputAttr, 0, sizeof(MI_LDC_InputPortAttr_t));

    itMapLdcCreateDev = mapLdcCreateDev.find(stModInfo.devId);
    if (itMapLdcCreateDev == mapLdcCreateDev.end())
    {
        STCHECKNORET(MI_LDC_CreateDevice(stModInfo.devId, &stDevAttr));
        mapLdcCreateDev[stModInfo.devId] = 0;
    }
    mapLdcCreateDev[stModInfo.devId]++;
    this->GetPortInInfo(0, stInPortInfo);
    stChnAttr.eWorkMode = ss_enum_cast<MI_LDC_WorkMode_e>::from_str(stLdcInfo.strWorkMode);
    stChnAttr.eInputBindType = (MI_SYS_BindType_e)stInPortInfo.bindType;
    STCHECKNORET(MI_LDC_CreateChannel(stModInfo.devId, stModInfo.chnId, &stChnAttr));

    for (itMapLdcOut = mapLdcOut.begin(); itMapLdcOut != mapLdcOut.end(); itMapLdcOut++)
    {
        OutputAttr.u16Width  =  itMapLdcOut->second.width;
        OutputAttr.u16Height =  itMapLdcOut->second.height;
        OutputAttr.ePixelFmt = ss_enum_cast<MI_SYS_PixelFormat_e>::from_str(itMapLdcOut->second.strOutFmt);
        STCHECKNORET(MI_LDC_SetOutputPortAttr(stModInfo.devId, stModInfo.chnId, &OutputAttr));
    }
    itMapLdcIn = mapLdcIn.begin();
    InputAttr.u16Width  = itMapLdcIn->second.width;
    InputAttr.u16Height = itMapLdcIn->second.height;
    STCHECKNORET(MI_LDC_SetInputPortAttr(stModInfo.devId, stModInfo.chnId, &InputAttr));
    if (!stLdcInfo.CalibInfo.path.empty())
    {
        //use callib
        stLdcInfo.CalibInfo.len = GetFileSize(stLdcInfo.CalibInfo.path.c_str());
        if (stLdcInfo.CalibInfo.len == (unsigned int)-1)
        {
            AMIGOS_ERR("Open file error!\n");
            return;
        }
        stLdcInfo.CalibInfo.addr = (void *)malloc(stLdcInfo.CalibInfo.len);
        if(stLdcInfo.CalibInfo.addr == NULL)
        {
            AMIGOS_ERR("No Memory to alloc, ERR!\n");
            return;
        }
        ReadFileToBuf(stLdcInfo.CalibInfo.path.c_str(), stLdcInfo.CalibInfo.addr,stLdcInfo.CalibInfo.len);
    }
    switch (stChnAttr.eWorkMode)
    {
        case MI_LDC_WORKMODE_LDC:
            Ldc_Init();
            break;
        /*
        case MI_LDC_WORKMODE_LUT:
            Lut_Init();
            break;
        */
        case MI_LDC_WORKMODE_DIS:
            Dis_Init();
            break;
        case MI_LDC_WORKMODE_PMF:
            Pmf_Init();
            break;
        case MI_LDC_WORKMODE_STITCH:
            Stitch_Init();
            break;
        case MI_LDC_WORKMODE_NIR:
            Nir_Init();
            break;
        /*
        case MI_LDC_WORKMODE_DPU:
            Dpu_Init();
            break;
        case MI_LDC_WORKMODE_LDC_HORIZONTAL:
            LdcHorizontal_Init();
            break;
        */
        case MI_LDC_WORKMODE_DIS_LDC:
            Ldc_Init();
            Dis_Init();
            break;
        default:
            AMIGOS_ERR("Work Mode ERR!\n");
            return;
    }
    STCHECKNORET(MI_LDC_StartChannel(stModInfo.devId, stModInfo.chnId));

    free(stLdcInfo.CalibInfo.addr);
    stLdcInfo.CalibInfo.addr = NULL;
    AMIGOS_INFO("LDC ENABLE dev%d chn %d \n", stModInfo.devId, stModInfo.chnId);

    stChnPort.eModId = E_MI_MODULE_ID_LDC;
    stChnPort.u32DevId = stModInfo.devId;
    stChnPort.u32ChnId = stModInfo.chnId;
    stChnPort.u32PortId = 0;
    STCHECKNORET(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 2, 4));
    AMIGOS_INFO("MI_SYS_SetChnOutputPortDepth dev%d chn%d 2 4\n",stChnPort.u32DevId,stChnPort.u32ChnId);
}
void AmigosModuleLdc::_Deinit()
{
    STCHECKNORET(MI_LDC_StopChannel(stModInfo.devId, stModInfo.chnId));
    STCHECKNORET(MI_LDC_DestroyChannel(stModInfo.devId, stModInfo.chnId));
    auto itMapLdcCreateDev = mapLdcCreateDev.find(stModInfo.devId);
    if (itMapLdcCreateDev != mapLdcCreateDev.end())
    {
       itMapLdcCreateDev->second--;
       if(!itMapLdcCreateDev->second)
       {
           STCHECKNORET(MI_LDC_DestroyDevice(stModInfo.devId));
           mapLdcCreateDev.erase(itMapLdcCreateDev);
       }
    }
#if 0
    if(WorkMode == MI_LDC_WORKMODE_LUT)
    {
        unsigned int u32TableSize = LutCfg.Width * LutCfg.Height;
        unsigned int u32WeightSize = GetFileSize(LutCfg.Table_Weight);
        MMA_FREE_BUF(stTableX.phyAddr[0], pVirTableX, u32TableSize);
        MMA_FREE_BUF(stTableY.phyAddr[0], pVirTableY, u32TableSize);
        MMA_FREE_BUF(stTableWeight.phyAddr[0], pVirTableWeight, u32WeightSize);
        MMA_FREE_BUF(stResult.phyAddr[0], pVirResult, u32TableSize);
    }
#endif
}
AMIGOS_MODULE_INIT("LDC", AmigosModuleLdc);
