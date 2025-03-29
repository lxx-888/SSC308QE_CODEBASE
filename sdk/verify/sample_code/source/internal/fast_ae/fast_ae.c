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

#include <stdio.h>
#include "mi_common_datatype.h"
#include "mi_sensor.h"
#include "mi_vif.h"
#include "mi_isp.h"
#include "mi_scl.h"
#include "mi_venc.h"
#include "mi_isp_awb.h"
#include "mi_isp_cus3a_api.h"
#include "isp_cus3a_if.h"
#include "fast_ae.h"

static ST_Pipeline_Param    gstPipeLineParam;
static ST_Convergence_Param gstConvergenceParam;
static MI_U32               gu32LastAeCnt;

static MI_S64 __FastAE_CalcDiffTime_MS(struct timeval *pstBeforeStamp, struct timeval *pstAfterStamp)
{
    return (((pstAfterStamp->tv_sec - pstBeforeStamp->tv_sec) * 1000 * 1000)
            + (pstAfterStamp->tv_usec - pstBeforeStamp->tv_usec))
           / 1000;
}

static MI_S32 __FastAE_IspSetIqBin(MI_U32 IspDev, MI_U32 IspChn, char *pConfigPath)
{
    MI_S32              s32Ret = MI_SUCCESS;
    CUS3A_ALGO_STATUS_t stCus3aStatus;
    MI_ISP_IQ_ParamInitInfoType_t stIqstatus;
    memset(&stCus3aStatus, 0, sizeof(CUS3A_ALGO_STATUS_t));
    memset(&stIqstatus, 0, sizeof(MI_ISP_IQ_ParamInitInfoType_t));

    MI_U8 u8ispreadycnt = 0;
    if (strlen(pConfigPath) == 0)
    {
        printf("IQ Bin File path NULL!\n");
        return -1;
    }

#ifdef LINUX_FLOW_ON_DUAL_OS
    printf("Ready to load IQ bin :%s u8ispreadycnt:%d\n", pConfigPath, u8ispreadycnt);
    s32Ret |= MI_ISP_ApiCmdLoadBinFile(IspDev, IspChn, (char *)pConfigPath, 1234);
#else
    while (1)
    {
        if (u8ispreadycnt > 100)
        {
            printf("ISP ready time out!\n");
            u8ispreadycnt = 0;
            break;
        }

        CUS3A_GetAlgoStatus((CUS3A_ISP_DEV_e)IspDev, (CUS3A_ISP_CH_e)IspChn, &stCus3aStatus);
        MI_ISP_IQ_GetParaInitStatus(IspDev, IspChn, &stIqstatus);

        if ((stCus3aStatus.Ae == E_ALGO_STATUS_RUNNING) && (stCus3aStatus.Awb == E_ALGO_STATUS_RUNNING)
            && (stIqstatus.stParaAPI.bFlag == E_SS_IQ_TRUE))
        {
            printf("Ready to load IQ bin :%s u8ispreadycnt:%d\n", pConfigPath, u8ispreadycnt);
            s32Ret |= MI_ISP_ApiCmdLoadBinFile(IspDev, IspChn, (char *)pConfigPath, 1234);

            usleep(10 * 1000);

            u8ispreadycnt = 0;
            break;
        }
        else
        {
            usleep(10 * 1000);
            u8ispreadycnt++;
        }
    }
    UNUSED(u8ispreadycnt);
#endif
    return s32Ret;
}


MI_S32 ST_Common_FastAE_CheckDNChange(MI_U32 u32IspDevId, MI_U32 u32IspChnId, FASTAE_EnvBrightnessType_e *peCurrentLight,
                                      char *pu8IqApiBinDarkPath, char *pu8IqApiBinBrightPath, MI_BOOL *pbDNChange)
{
    FAST_AE_CHECK_POINTER(peCurrentLight);
    FAST_AE_CHECK_POINTER(pu8IqApiBinDarkPath);
    FAST_AE_CHECK_POINTER(pu8IqApiBinBrightPath);

    MI_ISP_IQ_ColorToGrayType_t  stColorToGray;
    MI_ISP_IQ_DaynightInfoType_t stDayNightInfo;

    memset(&stColorToGray, 0x0, sizeof(MI_ISP_IQ_ColorToGrayType_t));
    memset(&stDayNightInfo, 0x0, sizeof(MI_ISP_IQ_DaynightInfoType_t));

    MI_ISP_IQ_QueryDayNightInfo(u32IspDevId, u32IspChnId, &stDayNightInfo);
    if (E_FASTAE_LIGHT_BRIGHT == *peCurrentLight)
    {
        printf("Check BV: stDayNightInfo.bD2N = ");
        if (TRUE == stDayNightInfo.bD2N)
        {
            *pbDNChange = TRUE;
            printf("TRUE\n");
            *peCurrentLight = E_FASTAE_LIGHT_DARK;

            __FastAE_IspSetIqBin(u32IspDevId, u32IspChnId, pu8IqApiBinDarkPath);

            stColorToGray.bEnable = TRUE;
            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

            // Add light control logic here if necessary
        }
        else
        {
            *pbDNChange = FALSE;
            printf("FALSE\n");
            goto EXIT;
        }
    }
    else
    {
        printf("Check AWB: stDayNightInfo.bN2D = ");
        if (TRUE == stDayNightInfo.bN2D)
        {
            *pbDNChange = TRUE;
            printf("TRUE\n");
            *peCurrentLight = E_FASTAE_LIGHT_BRIGHT;

            if (strlen(pu8IqApiBinBrightPath) != 0)
            {
                __FastAE_IspSetIqBin(u32IspDevId, u32IspChnId, pu8IqApiBinBrightPath);
            }

            stColorToGray.bEnable = FALSE;
            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

            // Add light control logic here if necessary
        }
        else
        {
            *pbDNChange = FALSE;
            printf("FALSE\n");
            goto EXIT;
        }
    }

EXIT:
    return MI_SUCCESS;
}


MI_S32 ST_Common_FastAE_SetFroceStreamOutCnt(MI_U32 u32VifDevId, MI_U32 u32StreamOutCnt, MI_U32 u32DropCnt)
{
    MI_VIF_ForceFrameParams_t stVifSleepDisableParam;
    MI_S32 s32Ret = MI_SUCCESS;

    stVifSleepDisableParam.u32ForceFrameCnt = u32StreamOutCnt;
    stVifSleepDisableParam.u32DropCnt       = u32DropCnt;
    s32Ret = MI_VIF_CustFunction(u32VifDevId, E_MI_VIF_CUSTCMD_FORCEFRAMEPARAM_SET, sizeof(MI_VIF_ForceFrameParams_t),
                        &stVifSleepDisableParam);

    PRINT_FAST_AE_DEBUG("SetFroceStreamOutCnt: u32StreamOutCnt:%d, u32DropCnt:%d, ret %d\n",
                            u32StreamOutCnt, u32DropCnt, s32Ret);

    return s32Ret;
}


MI_S32 ST_Common_FastAE_SetFastAEStatus(MI_U32 u32VifDevId, MI_BOOL bRunning)
{
    MI_S32 s32Ret = MI_SUCCESS;
#if 0//wait vif sync code
    MI_VIF_FastAEStatus_t stFastAEStatus;

    stFastAEStatus.bFastAERunning = bRunning;
    s32Ret = MI_VIF_CustFunction(u32VifDevId, E_MI_VIF_CUSTCMD_FASTAE_STATUS, sizeof(MI_VIF_FastAEStatus_t),
                        &stFastAEStatus);
#endif
    return s32Ret;
}


MI_S32 ST_Common_FastAE_DisableSensorSleepMode(MI_U32 u32VifDevId)
{
    MI_VIF_SleepModeParams_t stVifSleepParam;
    MI_S32 s32Ret = MI_SUCCESS;

    stVifSleepParam.bSleepEnable = FALSE;
    s32Ret = MI_VIF_CustFunction(u32VifDevId, E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(MI_VIF_SleepModeParams_t),
                        &stVifSleepParam);

    return s32Ret;
}

MI_S32 ST_Common_FastAE_EnableSensorSleepMode(MI_U32 u32VifDevId, MI_U32 u32FrameCntBeforeSleep)
{
    MI_VIF_SleepModeParams_t stVifSleepParam;
    MI_S32 s32Ret = MI_SUCCESS;

    stVifSleepParam.bSleepEnable           = TRUE;
    stVifSleepParam.u32FrameCntBeforeSleep = u32FrameCntBeforeSleep;
    s32Ret = MI_VIF_CustFunction(u32VifDevId, E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(MI_VIF_SleepModeParams_t),
                        &stVifSleepParam);

    return s32Ret;
}

MI_S32 ST_Common_FastAE_QueryExpoInfo(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_ISP_AE_ExpoInfoType_t *pstAeExpoInfo)
{
    FAST_AE_CHECK_POINTER(pstAeExpoInfo);

    MI_ISP_AE_QueryExposureInfo(u32IspDevId, u32IspChnId, pstAeExpoInfo);

    PRINT_FAST_AE_DEBUG(
        "==> stAeExpoInfo :\nLumY = %d\nSceneTarget = %d\nbStable = %d\nbIsReachBoundary = %d\nExpoValueLong.u32US = "
        "%d\nExpoValueLong.u32SensorGain = %d\nExpoValueLong.u32ISPGain = %d\nstHistWeightY.u32AvgY = %d\n",
        pstAeExpoInfo->stHistWeightY.u32LumY, pstAeExpoInfo->u32SceneTarget, pstAeExpoInfo->bIsStable,
        pstAeExpoInfo->bIsReachBoundary, pstAeExpoInfo->stExpoValueLong.u32US,
        pstAeExpoInfo->stExpoValueLong.u32SensorGain, pstAeExpoInfo->stExpoValueLong.u32ISPGain,
        pstAeExpoInfo->stHistWeightY.u32AvgY);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_WaitIspAeDone(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_U32 u32IntervalTimeMS,
                                      MI_U32 u32TimeoutMS)
{
    MI_S32         s32Ret           = -1;
    MI_U32  u32CurrentAeCnt         = 0;
    MI_U32  u32LastAeCnt            = 0;
    struct timeval stBefore         = {0, 0};
    struct timeval stAfter          = {0, 0};

    u32LastAeCnt = gu32LastAeCnt;
    PRINT_FAST_AE_DEBUG("LastAECnt = %d\n", u32LastAeCnt);

    // wait isp ae update done every u32IntervalTimeMS ms. timeout is u32TimeoutMS ms.
    gettimeofday(&stBefore, NULL);
    do
    {
        CHECK_FAST_AE_RESULT(MI_ISP_CUS3A_GetDoAeCount(u32IspDevId, u32IspChnId, &u32CurrentAeCnt), s32Ret, EXIT);
        if (u32CurrentAeCnt != u32LastAeCnt)
        {
            gu32LastAeCnt = u32CurrentAeCnt;
            PRINT_FAST_AE_DEBUG("CurrentAECnt = %d\n", u32CurrentAeCnt);
            break;
        }

        gettimeofday(&stAfter, NULL);

        if (__FastAE_CalcDiffTime_MS(&stBefore, &stAfter) > u32TimeoutMS)
        {
            s32Ret = -1;
            printf("ST_Common_FastAE_WaitIspAeDone timeout, LastAECnt = %d, CurrentAECnt = %d\n", u32LastAeCnt, u32CurrentAeCnt);
            break;
        }

        usleep(1000 * u32IntervalTimeMS);

    } while (1);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_WaitSensorSleep(MI_U32 u32VifDevId, MI_SNR_PADID u32PADId, MI_U32 u32IntervalTimeMS,
                                        MI_U32 u32TimeoutMS)
{
    MI_S32              s32Ret = -1;
    ST_SensorCusSleep_t stSensorSleep;
    struct timeval      stBefore = {0, 0};
    struct timeval      stAfter  = {0, 0};

    // wait sensor entry sleep mode every u32IntervalTimeMS ms. timeout is u32TimeoutMS ms.
    gettimeofday(&stBefore, NULL);
    do
    {
        CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_GET_SENSOR_SLEEPMODE,
                                                 sizeof(ST_SensorCusSleep_t), &(stSensorSleep),
                                                 E_MI_SNR_CUSTDATA_TO_USER),
                             s32Ret, EXIT);

        if (stSensorSleep.sleepmode == 1)
        {
            PRINT_FAST_AE_DEBUG("Sensor is in sleep mode\n");
            break;
        }

        gettimeofday(&stAfter, NULL);

        if (__FastAE_CalcDiffTime_MS(&stBefore, &stAfter) > u32TimeoutMS)
        {
            s32Ret = -1;
            printf("ST_Common_FastAE_WaitSensorSleep timeout\n");
            break;
        }

        usleep(1000 * u32IntervalTimeMS);

    } while (1);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_WaitIspConvToStable(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                            MI_U32 u32KeepStableCntWhileReachBoundary, MI_U32 u32WaitIspStableCnt,
                                            MI_ISP_AE_ExpoInfoType_t *pstAeExpoInfo)
{
    FAST_AE_CHECK_POINTER(pstAeExpoInfo);

    MI_S32               s32Ret = -1;
    MI_ISP_AE_ModeType_e eAEMode;

    // wait isp conv bstable to true
    PRINT_FAST_AE_DEBUG("==> Wait ISP stable :wait cnt=%d\n", u32WaitIspStableCnt);

    for (MI_U8 i = 0; i < u32WaitIspStableCnt; i++)
    {
        // wait isp ae update done
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitIspAeDone(u32IspDevId, u32IspChnId, 2, 800), s32Ret, EXIT);

        if (i == 0)
        {
            eAEMode = E_SS_AE_OP_TYP_AUTO;
            MI_ISP_AE_SetExpoMode(u32IspDevId, u32IspChnId, &eAEMode);
        }

        MI_ISP_AE_QueryExposureInfo(u32IspDevId, u32IspChnId, pstAeExpoInfo);

        PRINT_FAST_AE_DEBUG(
            "ST_Common_FastAE_WaitIspConvToStable:\nLumY = %d\nSceneTarget = %d\nbStable = %d\nbIsReachBoundary = %d\nExpoValueLong.u32US = "
            "%d\nExpoValueLong.u32SensorGain = %d\nExpoValueLong.u32ISPGain = %d\nstHistWeightY.u32AvgY = "
            "%d\n",
            pstAeExpoInfo->stHistWeightY.u32LumY, pstAeExpoInfo->u32SceneTarget, pstAeExpoInfo->bIsStable,
            pstAeExpoInfo->bIsReachBoundary, pstAeExpoInfo->stExpoValueLong.u32US,
            pstAeExpoInfo->stExpoValueLong.u32SensorGain, pstAeExpoInfo->stExpoValueLong.u32ISPGain,
            pstAeExpoInfo->stHistWeightY.u32AvgY);

        if (TRUE == pstAeExpoInfo->bIsStable)
        {
            // If beyond the AE adjustment range
            if ((TRUE == pstAeExpoInfo->bIsReachBoundary) && (u32KeepStableCntWhileReachBoundary > 0))
            {
                u32KeepStableCntWhileReachBoundary--;
                continue;
            }

            break;
        }
        else
        {
            PRINT_FAST_AE_DEBUG("bIsStable = %d\n", pstAeExpoInfo->bIsStable);
        }
    }

    printf("FastAE>>>Wait stable cnt=%d, u32AvgY:%d, bStable:%d, bIsReachBoundary:%d\n",
        u32WaitIspStableCnt, pstAeExpoInfo->stHistWeightY.u32AvgY, pstAeExpoInfo->bIsStable, pstAeExpoInfo->bIsReachBoundary);
EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_PresetForSwitchOnFastAe(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                                MI_ISP_AE_ExpoInfoType_t *    pstExpoInfo,
                                                ST_Sensor_OS04D10_AeSwitch_t *pstAeSwitch)
{
    FAST_AE_CHECK_POINTER(pstExpoInfo);
    FAST_AE_CHECK_POINTER(pstAeSwitch);

    pstAeSwitch->SensorAEswitch = 1;
    pstAeSwitch->SensorAEfpsmax = 48;

    CusAEInfo_t      stCusAeInfo;
    CusAEInitParam_t stCusAeInitParam;

    MI_ISP_CUS3A_GetAeStatus(u32IspDevId, u32IspChnId, &stCusAeInfo);
    MI_ISP_CUS3A_GetAeInitStatus(u32IspDevId, u32IspChnId, &stCusAeInitParam);//need get shutter/gain from sensor driver


    // sensor
    MI_U64 U64AETH = (MI_U64)stCusAeInitParam.shutter * stCusAeInitParam.sensor_gain * stCusAeInfo.IspGain
                     * pstExpoInfo->u32SceneTarget / pstExpoInfo->stHistWeightY.u32LumY;

    //samll picture not support auto change lcg -> hcg, again lcg 1x~3x;hcg 3x~47.8x
    //u32SNRAgainMin * 3 base on hcg
    MI_U32 u32SNRShutterMax = (1022 * 22629) / 1000; // get from sensor
    MI_U32 u32SNRShutterMin = 100;                   // get from sensor
    MI_U32 u32SNRAgainMin   = 1024;                  // get from sensor
    MI_U32 u32SNRAgainMax   = 49725;                 // get from sensor
    MI_U32 u32SNRDgainMin   = 1024;                  // get from sensor
    MI_U32 u32SNRDgainMax   = 32 * 1024;             // get from sensor
    MI_U64 u64SNRTH1        = (MI_U64)u32SNRShutterMax * u32SNRAgainMin * u32SNRDgainMin;
    MI_U64 u64SNRTH2        = (MI_U64)u32SNRShutterMax * u32SNRAgainMax * u32SNRDgainMin;

    //printf("FastAE>>>SwitchOnFastAe :U64AETH:%lld\n", (MI_U64)U64AETH);

    if (U64AETH >= u64SNRTH1)
    {
        pstAeSwitch->SensorAEshutter = MAX(u32SNRShutterMin, u32SNRShutterMax / 4);
        pstAeSwitch->SensorAEdgain =
            MINMAX(4 * u32SNRDgainMin * U64AETH / u64SNRTH2, 4 * u32SNRDgainMin, u32SNRDgainMax);
        pstAeSwitch->SensorAEagain =
            MINMAX(U64AETH / pstAeSwitch->SensorAEshutter / pstAeSwitch->SensorAEdgain, u32SNRAgainMin * 3, u32SNRAgainMax);
    }
    else if(U64AETH <= u64SNRTH1 / 4)
    {
        pstAeSwitch->SensorAEagain   = u32SNRAgainMin; //use lcg
        pstAeSwitch->SensorAEdgain   = u32SNRDgainMin;
        pstAeSwitch->SensorAEshutter = MINMAX(U64AETH / pstAeSwitch->SensorAEagain / pstAeSwitch->SensorAEdgain,
                                              u32SNRShutterMin, u32SNRShutterMax);
    }
    else
    {
        pstAeSwitch->SensorAEagain   = u32SNRAgainMin * 3;//use hcg
        pstAeSwitch->SensorAEdgain   = u32SNRDgainMin;
        pstAeSwitch->SensorAEshutter = MINMAX(U64AETH / pstAeSwitch->SensorAEagain / pstAeSwitch->SensorAEdgain,
                                              u32SNRShutterMin, u32SNRShutterMax);
    }

    pstAeSwitch->SensorAEtarget = pstExpoInfo->u32SceneTarget / 10;

    //printf("FastAE>>>SwitchOnFastAe :AEtarget:%d,shutter:%d,again:%d,dgain:%d\n", pstAeSwitch->SensorAEtarget, pstAeSwitch->SensorAEshutter,
    //            pstAeSwitch->SensorAEagain, pstAeSwitch->SensorAEdgain);
    PRINT_FAST_AE_DEBUG(
        "==> ST_Common_FastAE_PresetForSwitchOnFastAe :\npstAeSwitch->SensorAEswitch = %d\npstAeSwitch->SensorAEtarget "
        "= %d\npstAeSwitch->SensorAEshutter = %d\npstAeSwitch->SensorAEagain = %d\npstAeSwitch->SensorAEdgain = "
        "%d\npstAeSwitch->SensorAEled = %d\npstAeSwitch->SensorAEfpsmax = %d\n",
        pstAeSwitch->SensorAEswitch, pstAeSwitch->SensorAEtarget, pstAeSwitch->SensorAEshutter,
        pstAeSwitch->SensorAEagain, pstAeSwitch->SensorAEdgain, pstAeSwitch->SensorAEled, pstAeSwitch->SensorAEfpsmax);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_PresetForSwitchOffFaseAe(MI_U32 u32SensorFps, ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull,
                                                 ST_Sensor_OS04D10_AeSwitch_t *pstAeSwitch)
{
    FAST_AE_CHECK_POINTER(pstAeStateFull);
    FAST_AE_CHECK_POINTER(pstAeSwitch);

    pstAeSwitch->SensorAEswitch  = 0;
    pstAeSwitch->SensorAEshutter = pstAeStateFull->shutter;
    pstAeSwitch->SensorAEagain   = pstAeStateFull->again;
    pstAeSwitch->SensorAEfpsmax  = u32SensorFps;

    PRINT_FAST_AE_DEBUG(
        "==> ST_Common_FastAE_PresetForSwitchOffFaseAe :\npstAeSwitch->SensorAEswitch = "
        "%d\npstAeSwitch->SensorAEtarget = "
        "%d\npstAeSwitch->SensorAEshutter = %d\npstAeSwitch->SensorAEagain = "
        "%d\npstAeSwitch->SensorAEdgain = "
        "%d\npstAeSwitch->SensorAEled = %d\npstAeSwitch->SensorAEfpsmax = %d\n",
        pstAeSwitch->SensorAEswitch, pstAeSwitch->SensorAEtarget, pstAeSwitch->SensorAEshutter,
        pstAeSwitch->SensorAEagain, pstAeSwitch->SensorAEdgain, pstAeSwitch->SensorAEled, pstAeSwitch->SensorAEfpsmax);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_ConvertFastAeResult(MI_U32 u32SensorFps, MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                            MI_ISP_AE_ExpoInfoType_t *pstExpoInfo, ST_Sensor_OS04D10_CusAeState_t *pstAeState,
                                            CusAEResult_t *pstAeResult, ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull)
{
    FAST_AE_CHECK_POINTER(pstExpoInfo);
    FAST_AE_CHECK_POINTER(pstAeState);
    FAST_AE_CHECK_POINTER(pstAeResult);
    FAST_AE_CHECK_POINTER(pstAeStateFull);

    MI_ISP_AE_ExpoTableType_t stExpoTable;

    // u32MaxShutter : max shutter in us. It will be set by user app.
    MI_U32 u32IspGainMin = 1024;
    MI_U32 u32IspGainMax = 1024;                   //get from AE table
    MI_U32 u32MaxShutter = 1000000 / u32SensorFps; //get from AE table
    MI_U32 u32SNRGainMin = 1024;                   // get from sensor
    MI_U32 u32SNRGainMax = 49725 * 32;             //get from AE table
    MI_U64 TH            = u32IspGainMin * (MI_U64)pstAeState->shutter * pstAeState->again * pstExpoInfo->u32SceneTarget
                / MAX((pstAeState->ymean * 10), 1);

    memset(&stExpoTable, 0x0, sizeof(MI_ISP_AE_ExpoTableType_t));
    MI_ISP_AE_GetPlainLongExpoTable(u32IspDevId, u32IspChnId, &stExpoTable);
    u32MaxShutter = stExpoTable.stExpoTbl[stExpoTable.u32NumPoints-1].u32Shutter;
    u32SNRGainMax = stExpoTable.stExpoTbl[stExpoTable.u32NumPoints-1].u32SensorGain;
    u32IspGainMax = ((MI_U64)(stExpoTable.stExpoTbl[stExpoTable.u32NumPoints-1].u32TotalGain * 1024)) / u32SNRGainMax;
    printf("FastAE>>>MaxExpoTable:US:%d, SG:%d, TG:%d\n", u32MaxShutter,
            stExpoTable.stExpoTbl[stExpoTable.u32NumPoints-1].u32SensorGain, stExpoTable.stExpoTbl[stExpoTable.u32NumPoints-1].u32TotalGain);

    if (TH >= (MI_U64)u32MaxShutter * u32SNRGainMin * u32IspGainMin)
    {
        pstAeStateFull->shutter = u32MaxShutter;
        pstAeStateFull->dgain   = MINMAX(TH / ((MI_U64)u32MaxShutter * u32SNRGainMax), u32IspGainMin, u32IspGainMax);
        pstAeStateFull->again   = MIN(TH / ((MI_U64)u32MaxShutter * pstAeStateFull->dgain), u32SNRGainMax);
    }
    else
    {
        pstAeStateFull->again   = u32SNRGainMin;
        pstAeStateFull->dgain   = u32IspGainMin;
        pstAeStateFull->shutter = MIN(TH / ((MI_U64)u32SNRGainMin * u32IspGainMin), u32SNRGainMax);
    }

    if ((0 != pstAeState->ymean) || (0 != pstAeState->again) || (0 != pstAeState->shutter))
    {
        pstAeResult->Size               = sizeof(CusAEResult_t);
        pstAeResult->Change             = 2;
        pstAeResult->Shutter            = pstAeStateFull->shutter;
        pstAeResult->SensorGain         = pstAeStateFull->again;
        pstAeResult->IspGain            = pstAeStateFull->dgain;
        pstAeResult->ShutterHdrShort    = 100;
        pstAeResult->SensorGainHdrShort = 1024;
        pstAeResult->IspGainHdrShort    = 1024;
        pstAeResult->i4BVx16384         = 0;
        pstAeResult->AvgY               = pstExpoInfo->u32SceneTarget;
        pstAeResult->HdrRatio           = 1024;
        pstAeResult->HdrRatio1          = 1024;
        pstAeResult->FNx10              = pstExpoInfo->stExpoValueLong.u32FNx10;
        pstAeResult->DebandFPS          = 30000;
        pstAeResult->WeightY            = pstExpoInfo->u32SceneTarget;
        pstAeResult->GMBlendRatio       = 512;
    }
    else
    {
        printf("fast ae result is 0, will use the last AE parameter\n");

        pstAeResult->Size               = sizeof(CusAEResult_t);
        pstAeResult->Change             = 2;
        pstAeResult->Shutter            = pstExpoInfo->stExpoValueLong.u32US;
        pstAeResult->SensorGain         = pstExpoInfo->stExpoValueLong.u32SensorGain;
        pstAeResult->IspGain            = pstExpoInfo->stExpoValueLong.u32ISPGain;
        pstAeResult->ShutterHdrShort    = 100;
        pstAeResult->SensorGainHdrShort = 1024;
        pstAeResult->IspGainHdrShort    = 1024;
        pstAeResult->i4BVx16384         = 0;
        pstAeResult->AvgY               = pstExpoInfo->u32SceneTarget;
        pstAeResult->HdrRatio           = 1024;
        pstAeResult->HdrRatio1          = 1024;
        pstAeResult->FNx10              = pstExpoInfo->stExpoValueLong.u32FNx10;
        pstAeResult->DebandFPS          = 30000;
        pstAeResult->WeightY            = pstExpoInfo->u32SceneTarget;
        pstAeResult->GMBlendRatio       = 512;
    }

    printf("FastAE>>>ConvertFastAeResult :Shutter = %d, SensorGain = %d, IspGain = %d, u32SceneTarget = %d\n",
        pstAeResult->Shutter, pstAeResult->SensorGain, pstAeResult->IspGain, pstAeResult->AvgY);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_SetToISP(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_ISP_AE_ExpoInfoType_t *pstExpoInfo,
                                 CusAEResult_t *pstAeResult)
{
    FAST_AE_CHECK_POINTER(pstExpoInfo);

    MI_S32                    s32Ret = -1;
    MI_ISP_AE_ModeType_e      eAEMode;
    MI_ISP_AE_ExpoValueType_t stExpoVal;

    CHECK_FAST_AE_RESULT(MI_ISP_CUS3A_SetAeParam(u32IspDevId, u32IspChnId, pstAeResult), s32Ret, EXIT);

    eAEMode = E_SS_AE_MODE_M;
    CHECK_FAST_AE_RESULT(MI_ISP_AE_SetExpoMode(u32IspDevId, u32IspChnId, &eAEMode), s32Ret, EXIT);

    stExpoVal.u32FNx10      = pstExpoInfo->stExpoValueLong.u32FNx10;
    stExpoVal.u32SensorGain = pstAeResult->SensorGain;
    stExpoVal.u32ISPGain    = pstAeResult->IspGain;
    stExpoVal.u32US         = pstAeResult->Shutter;
    CHECK_FAST_AE_RESULT(MI_ISP_AE_SetManualExpo(u32IspDevId, u32IspChnId, &stExpoVal), s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_ConfigToSmallPic(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_SNR_PADID u32PADId,
                                         MI_ISP_AE_ExpoInfoType_t *pstExpoInfo)
{
    FAST_AE_CHECK_POINTER(pstExpoInfo);

    MI_S32                       s32Ret = -1;
    ST_Sensor_OS04D10_AeSwitch_t stFastAeSwitch;

    // preset parameter for fast ae switch on (samll pic)
    ST_Common_FastAE_PresetForSwitchOnFastAe(u32IspDevId, u32IspChnId, pstExpoInfo, &stFastAeSwitch);

    // Configure Sensor to switch on fast ae
    CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_SWITCH_SENSOR_AE,
                                             sizeof(ST_Sensor_OS04D10_AeSwitch_t), &stFastAeSwitch,
                                             E_MI_SNR_CUSTDATA_TO_DRIVER),
                         s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_ConfigToFullPic(MI_SNR_PADID u32PADId, MI_U32 u32SensorFps,
                                        ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull)
{
    FAST_AE_CHECK_POINTER(pstAeStateFull);

    MI_S32                       s32Ret = -1;
    ST_Sensor_OS04D10_AeSwitch_t stFastAeSwitch;

    // Configure Sensor to switch off fast ae
    ST_Common_FastAE_PresetForSwitchOffFaseAe(u32SensorFps, pstAeStateFull, &stFastAeSwitch);
    CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_SWITCH_SENSOR_AE,
                                             sizeof(ST_Sensor_OS04D10_AeSwitch_t), &stFastAeSwitch,
                                             E_MI_SNR_CUSTDATA_TO_DRIVER),
                         s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_GetSmallPicAeResult(MI_SNR_PADID u32PADId, MI_U32 u32SensorFps,
                                            ST_Sensor_OS04D10_CusAeState_t *pstAeState,
                                            MI_ISP_AE_ExpoInfoType_t *pstExpoInfo, CusAEResult_t *pstAeResult,
                                            MI_U32 u32IntervalTimeMS, MI_U32 u32TimeoutMS)
{
    MI_S32                s32Ret   = -1;
    struct timeval        stBefore = {0, 0};
    struct timeval        stAfter  = {0, 0};
    ST_SensorCusAeAbort_t stSensorCusAbort;

    // wait fast ae done every u32IntervalTimeMS ms. timeout is u32TimeoutMS ms.
    gettimeofday(&stBefore, NULL);
    do
    {
        CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_GET_SENSOR_AE_STATE,
                                                 sizeof(ST_Sensor_OS04D10_CusAeState_t), pstAeState,
                                                 E_MI_SNR_CUSTDATA_TO_USER),
                             s32Ret, EXIT);

        if (pstAeState->state == 1)
        {
            PRINT_FAST_AE_DEBUG("Run fast ae done\n");
            break;
        }
        else if(pstAeState->state == 2)
        {
            printf("fast ae not working\n");
            s32Ret = -1;
            break;
        }

        gettimeofday(&stAfter, NULL);

        if (__FastAE_CalcDiffTime_MS(&stBefore, &stAfter) > u32TimeoutMS)
        {
            printf("abort fast ae while run timeout\n");

            // abort while fast ae timeout
            stSensorCusAbort.abort = 1;
            CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_GET_ABORT_SENSOR_AE,
                                                     sizeof(ST_SensorCusAeAbort_t), &stSensorCusAbort,
                                                     E_MI_SNR_CUSTDATA_TO_DRIVER),
                                 s32Ret, EXIT);

            s32Ret = -1;
            break;
        }

        usleep(1000 * u32IntervalTimeMS);

    } while (1);

    printf(
        "FastAE>>>GetSmallPicAeResult :state = %d, again = %d, dgain = %d, shutter = %d, ymean = %d\n",
        pstAeState->state, pstAeState->again, pstAeState->dgain, pstAeState->shutter, pstAeState->ymean);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_InitPipelineParam(ST_Pipeline_Param *pstPipelineParam)
{
    FAST_AE_CHECK_POINTER(pstPipelineParam);

    memcpy(&gstPipeLineParam, pstPipelineParam, sizeof(ST_Pipeline_Param));

    gu32LastAeCnt     = 0;
    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_InitIspConvParam(MI_U32 u32StreamOutCnt, MI_U32 u32DropCnt, MI_U32 u32RunThreshold)
{
    gstConvergenceParam.u32StreamOutCnt = u32StreamOutCnt;
    gstConvergenceParam.u32DropCnt      = u32DropCnt;
    gstConvergenceParam.u32RunThreshold = u32RunThreshold;

    printf("ST_Common_FastAE_InitIspConvParam[u32StreamOutCnt:%d, u32DropCnt:%d, u32RunThreshold:%d]\n",
            gstConvergenceParam.u32StreamOutCnt, gstConvergenceParam.u32DropCnt, gstConvergenceParam.u32RunThreshold);

    return MI_SUCCESS;
}

MI_S32 ST_Common_DoFastAE(MI_U32 u32VifDevId, MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_SNR_PADID u32PADId,
                            MI_U32 u32SensorFps, MI_U32 u32VencDevId, MI_U32 u32VencChnId,MI_U32 u32SclDevId,MI_U32 u32SclChnId,
                            MI_ISP_AE_ExpoInfoType_t *pstAeExpoInfo, MI_BOOL bDropFifo, MI_U32 u32StreamOutCnt, MI_U32 u32DropCnt)
{
    MI_S32                         s32Ret = MI_SUCCESS;
    ST_Sensor_OS04D10_AeSwitch_t   stFastAESwitch;
    ST_Sensor_OS04D10_CusAeState_t stAeStateFull;
    CusAEResult_t                  stAeResult;
    ST_Sensor_OS04D10_CusAeState_t stAeState;
    MI_BOOL                        bInstant  = bDropFifo;

    memset(&stFastAESwitch, 0x00, sizeof(ST_Sensor_OS04D10_AeSwitch_t));
    memset(&stAeStateFull, 0x00, sizeof(ST_Sensor_OS04D10_CusAeState_t));
    memset(&stAeState, 0x00, sizeof(ST_Sensor_OS04D10_CusAeState_t));

    // configure sensor to enable sleep mode
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_EnableSensorSleepMode(u32VifDevId, 1), s32Ret, EXIT);

    // wait sensor entry sleep mode
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitSensorSleep(u32VifDevId, u32PADId, 10, 800), s32Ret, EXIT);

    // request IDR to to clear VENC save stream
    CHECK_FAST_AE_RESULT(MI_VENC_RequestIdr(u32VencDevId, u32VencChnId, bInstant), s32Ret, EXIT);

    // config to small pic (switch on fast ae)
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_ConfigToSmallPic(u32IspDevId, u32IspChnId, u32PADId, pstAeExpoInfo),
                         s32Ret, EXIT);

    // update ae cnt
    CHECK_FAST_AE_RESULT(MI_ISP_CUS3A_GetDoAeCount(u32IspDevId, u32IspChnId, &gu32LastAeCnt), s32Ret, EXIT);

    //set stream out cnt after disable sleep mode
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_SetFroceStreamOutCnt(u32VifDevId, u32StreamOutCnt ,u32DropCnt), s32Ret, EXIT);

    // get small pic (fast ae) result
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_GetSmallPicAeResult(u32PADId, u32SensorFps, &stAeState, pstAeExpoInfo,
                                                              &stAeResult, 10, 800),
                         s32Ret, EXIT);

    // convert small pic parameters to full pic parameters
    CHECK_FAST_AE_RESULT(
        ST_Common_FastAE_ConvertFastAeResult(u32SensorFps, u32IspDevId, u32IspChnId, pstAeExpoInfo, &stAeState, &stAeResult, &stAeStateFull),
        s32Ret, EXIT);

    // set fast ae result to isp
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_SetToISP(u32IspDevId, u32IspChnId, pstAeExpoInfo, &stAeResult), s32Ret,
                         EXIT);

    // config to full pic (switch off fast ae)
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_ConfigToFullPic(u32PADId, u32SensorFps, &stAeStateFull), s32Ret, EXIT);

    // wait isp conv bstable to true
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitIspConvToStable(u32IspDevId, u32IspChnId, 1, u32StreamOutCnt, pstAeExpoInfo),
                         s32Ret, EXIT);

    //wait sensor stream out done
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitSensorSleep(u32VifDevId, u32PADId, 10, 800), s32Ret, EXIT);

EXIT:
    return s32Ret;
}


MI_S32 ST_Common_FastAE_Run(MI_BOOL *pbIsDoFastAE)
{
    MI_S32                         s32Ret = MI_SUCCESS;
    MI_ISP_AE_ExpoInfoType_t       stAeExpoInfo;
    MI_U32                         u32VifDevId     = gstPipeLineParam.u32VifDevId;
    MI_U32                         u32IspDevId     = gstPipeLineParam.u32IspDevId;
    MI_U32                         u32IspChnId     = gstPipeLineParam.u32IspChnId;;
    MI_U32                         u32PADId        = gstPipeLineParam.u32SnrPadId;
    MI_U32                         u32SensorFps    = gstPipeLineParam.u32SensorFps;
    MI_U32                         u32VencDevId    = gstPipeLineParam.u32VencDevId;
    MI_U32                         u32VencChnId    = gstPipeLineParam.u32VencChnId;
    MI_U32                         u32SclDevId     = gstPipeLineParam.u32SclDevId;
    MI_U32                         u32SclChnId     = gstPipeLineParam.u32SclChnId;
    MI_U32                         u32StreamOutCnt = gstConvergenceParam.u32StreamOutCnt;
    MI_U32                         u32DropCnt      = gstConvergenceParam.u32DropCnt;
    MI_BOOL                        bDropFifo       = FAST_AE_DROP_FIFO;

    memset(&stAeExpoInfo, 0x0, sizeof(MI_ISP_AE_ExpoInfoType_t));

    ST_Common_FastAE_SetFastAEStatus(u32VifDevId, TRUE);
    // wait isp ae update done
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitIspAeDone(u32IspDevId, u32IspChnId, 2, 800), s32Ret, EXIT);

    // check whether ae is stable
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_QueryExpoInfo(u32IspDevId, u32IspChnId, &stAeExpoInfo), s32Ret, EXIT);

    printf("FastAE>>>ae bstable %d, bIsReachBoundary %d\n", stAeExpoInfo.bIsStable, stAeExpoInfo.bIsReachBoundary);
    printf("FastAE>>>s32BV:%d, avgY:%d, u32SceneTarget %d\n", stAeExpoInfo.s32BV, stAeExpoInfo.stHistWeightY.u32AvgY, stAeExpoInfo.u32SceneTarget);

    if (FALSE == stAeExpoInfo.bIsStable && stAeExpoInfo.bIsReachBoundary == FALSE
            && DIFF(stAeExpoInfo.stHistWeightY.u32AvgY, stAeExpoInfo.u32SceneTarget) > gstConvergenceParam.u32RunThreshold)
    {
        CHECK_FAST_AE_RESULT(ST_Common_DoFastAE(u32VifDevId, u32IspDevId, u32IspChnId, u32PADId, u32SensorFps,
                            u32VencDevId, u32VencChnId, u32SclDevId, u32SclChnId,
                            &stAeExpoInfo, bDropFifo, u32StreamOutCnt, u32DropCnt), s32Ret, EXIT);

        *pbIsDoFastAE = TRUE;
    }
    else
    {
        *pbIsDoFastAE = FALSE;
    }

EXIT:
    gu32LastAeCnt = 0;
    ST_Common_FastAE_SetFastAEStatus(u32VifDevId, FALSE);
    return s32Ret;
}

MI_S32 ST_Common_FastAE_UpdateDNIqBin(FASTAE_EnvBrightnessType_e *peCurrentLight,char *pu8IqApiBinDarkPath, char *pu8IqApiBinBrightPath)
{
    MI_S32  s32Ret = MI_SUCCESS;
    MI_BOOL bDNChange;
    MI_U32  u32VifDevId     = gstPipeLineParam.u32VifDevId;
    MI_U32  u32IspDevId     = gstPipeLineParam.u32IspDevId;
    MI_U32  u32IspChnId     = gstPipeLineParam.u32IspChnId;;
    MI_U32  u32PADId        = gstPipeLineParam.u32SnrPadId;
    MI_U32  u32SensorFps    = gstPipeLineParam.u32SensorFps;
    MI_U32  u32VencDevId    = gstPipeLineParam.u32VencDevId;
    MI_U32  u32VencChnId    = gstPipeLineParam.u32VencChnId;
    MI_U32  u32SclDevId     = gstPipeLineParam.u32SclDevId;
    MI_U32  u32SclChnId     = gstPipeLineParam.u32SclChnId;
    MI_U32  u32StreamOutCnt = 1;
    MI_U32  u32DropCnt      = 0;
    MI_BOOL bDropFifo       = FAST_AE_DROP_FIFO;

    // check D2N or N2D
    ST_Common_FastAE_CheckDNChange(u32IspDevId, u32IspChnId, peCurrentLight, pu8IqApiBinDarkPath,
                              pu8IqApiBinBrightPath, &bDNChange);
    if (TRUE == bDNChange)
    {
        MI_ISP_AE_ExpoInfoType_t       stAeExpoInfo;
        memset(&stAeExpoInfo, 0x0, sizeof(MI_ISP_AE_ExpoInfoType_t));
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_QueryExpoInfo(u32IspDevId, u32IspChnId, &stAeExpoInfo), s32Ret, EXIT);
        CHECK_FAST_AE_RESULT(ST_Common_DoFastAE(u32VifDevId, u32IspDevId, u32IspChnId, u32PADId, u32SensorFps,
                                                u32VencDevId, u32VencChnId, u32SclDevId, u32SclChnId,
                                                &stAeExpoInfo, bDropFifo, u32StreamOutCnt, u32DropCnt), s32Ret, EXIT);
    }

EXIT:
    return s32Ret;
}

