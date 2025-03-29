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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "cam_fs_wrapper.h"
#include "sys_memmap.h"
#include "mi_device.h"
#include "mi_sys.h"
#include "mi_vif.h"
#include "mi_scl.h"
#include "mi_venc.h"
#include "mi_rgn.h"
#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"
#include "mi_isp_iq.h"
#include "mi_sensor.h"
#include "mi_ai.h"
#include "mi_ao.h"
#include "sys_sys_boot_timestamp.h"
#include "mi_ipu.h"
#include "sstar_det_api.h"
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "preload_sample_rtos.h"
#include <mhal_earlyinit_para.h>
#if defined(CONFIG_SENSOR_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
#include "earlyinit_preload_api.h"
#endif
#if defined(CONFIG_APPLICATION_SELECTOR)
#include "application_selector.h"
#endif

#include "preload_sample_rtos_common.h"
#include "light_sensor.h"
/*=============================================================*/
// Global Variable definition
/*=============================================================*/
static const MaxFrameSize _maxFrameSize[8] =
{
    {84 * 8 * 1024 * 8,  63 * 8 * 1024 * 8 },
    {63 * 1024 * 8,  47 * 1024 * 8 },
    {42 * 1024 * 8,  32 * 1024 * 8 },
    {32 * 1024 * 8,  24 * 1024 * 8 },
    {21 * 1024 * 8,  16 * 1024 * 8 },
    {21 * 1024 * 8,  16 * 1024 * 8 },
    {20 * 1024 * 8,  15 * 1024 * 8 },
    {20 * 1024 * 8,  15 * 1024 * 8 }
};

static ST_Stream_Attr_T g_stStreamAttr[] =
{
    {
        .bEnable = TRUE,
        .u32InputChn = 0,
        .u32InputPort = 0,
        .vencChn = 0,
        .eType = E_MI_VENC_MODTYPE_H265E,
        .u32Mbps = 2097152,
        .u32Width = 2560,
        .u32Height = 1440,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1440,
        .u32CropX = 0,
        .u32CropY = 0,
        .u32CropWidth = 0,
        .u32CropHeight = 0,
        .eBindType = E_MI_SYS_BIND_TYPE_HW_RING,
        .u32BindPara = 0,
    },
    {
        .bEnable = TRUE,
        .u32InputChn = 1,
        .u32InputPort = 0,
        .vencChn = 1,
        .eType = E_MI_VENC_MODTYPE_H265E,
        .u32Mbps = 2097152,
        .u32Width = 2560,
        .u32Height = 1440,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1440,
        .u32CropX = 0,
        .u32CropY = 0,
        .u32CropWidth = 0,
        .u32CropHeight = 0,
        .eBindType = E_MI_SYS_BIND_TYPE_HW_RING,
        .u32BindPara = 0,
    },
};

MI_BOOL                      bThreadExit = 1;
MI_BOOL                      bInitAlready = 0;
MI_U8                        g_u8stoppipe = 0;
MI_U8                        g_u8deintmoudle = 0;
MI_S32                       s32Aov_Get_Status = AOV_DEFAULT_STATUS;
void*                        pDetHandle;
InputAttr_t                  stDetModelAttr;
static MI_RGN_ChnPortParam_t gstRgnChnPortParam;
MI_RGN_HANDLE                hFrame[MAX_FRAME_HANDLE] = {0};
MI_S32                       s32ThreadRetval;
MI_SYS_WindowRect_t          get_sensor_size;
MI_U32                       g_u32SensorNum = 1;

InitPreloadCfg_t             g_st_init_setting;
CamOsThread PreloadMiPipe_tid;
CamOsThread PreloadFile_tid;
CamOsThread PreloadDet_tid;
CamOsThread PreloadSecond_tid;
CamOsTsem_t tPreloadFileTsem;
CamOsTsem_t tIspReadFileTsem;
////////////////////////////////////////////////////////////////////////////////

int ST_DET_Init(void);
void *ST_DetectThread(void *p);

bool RtosPreloadIsUseReduceMemCfg(void)
{
    return (SysMmapGetLimitDramSize() <= 0x04000000) ? TRUE : FALSE;
}

static MI_S32 STUB_GetVencConfig(MI_VENC_ModType_e eModType, MI_VENC_ChnAttr_t *pstVencChnAttr, MI_U8 u8ChnId)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_U32 u32Profile =0, u32BitRate = 0, u32Gop = 0, u32MaxQp = 0, u32MinQp = 0, u32SrcFrmRateNum = 0, u32MaxBitRate = 0;
    MI_U8 u8VideoH264RcMode = 0, u8VideoH265RcMode = 0;

    u8VideoH264RcMode = 0;
    u8VideoH265RcMode = 0;
    u32Profile = 1;
    u32BitRate = g_stStreamAttr[0].u32Height * g_stStreamAttr[0].u32Width * 1.3;
    u32Gop = 30;
    u32MaxQp = 48;
    u32MinQp = 12;
    u32SrcFrmRateNum  = 15;
    u32MaxBitRate = g_stStreamAttr[0].u32Height * g_stStreamAttr[0].u32Width * 1.3;

    switch(eModType)
    {
        case E_MI_VENC_MODTYPE_H264E:
        {
            pstVencChnAttr->stVeAttr.eType = eModType;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32PicWidth = pstStreamAttr[u8ChnId].u32Width;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32PicHeight = pstStreamAttr[u8ChnId].u32Height;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicWidth = pstStreamAttr[u8ChnId].u32MaxWidth;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicHeight = pstStreamAttr[u8ChnId].u32MaxHeight;
            pstVencChnAttr->stVeAttr.stAttrH264e.bByFrame = TRUE;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32Profile = u32Profile;
            if(0 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32BitRate = u32BitRate;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32StatTime = 0;
            }
            else if(1 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264VBR;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32MinQp = u32MinQp;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32MaxBitRate = u32MaxBitRate;
            }
            else if(2 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32IQp = 30;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32PQp = 30;
            }
            else if(3 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264AVBR;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32MinQp = u32MinQp;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32MaxBitRate = u32MaxBitRate;
            }
        }
        break;
        case E_MI_VENC_MODTYPE_H265E:
        {
            pstVencChnAttr->stVeAttr.eType = eModType;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32PicWidth = pstStreamAttr[u8ChnId].u32Width;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32PicHeight = pstStreamAttr[u8ChnId].u32Height;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicWidth = pstStreamAttr[u8ChnId].u32MaxWidth;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicHeight = pstStreamAttr[u8ChnId].u32MaxHeight;
            pstVencChnAttr->stVeAttr.stAttrH265e.bByFrame = TRUE;

            if(0 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32BitRate = u32BitRate;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 1;
            }
            else if(1 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32MaxBitRate = u32MaxBitRate;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32MinQp = u32MinQp;

            }
            else if(2 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32IQp = 30;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32PQp = 30;
            }
            else if(3 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265AVBR;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32MaxBitRate = u32MaxBitRate;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32MinQp = u32MinQp;
            }
        }
        break;
        case E_MI_VENC_MODTYPE_JPEGE:
        {
            pstVencChnAttr->stVeAttr.eType = eModType;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicWidth = pstStreamAttr[u8ChnId].u32MaxWidth;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicHeight = pstStreamAttr[u8ChnId].u32MaxHeight;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicWidth = pstStreamAttr[u8ChnId].u32Width;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicHeight = pstStreamAttr[u8ChnId].u32Height;
            pstVencChnAttr->stVeAttr.stAttrJpeg.bByFrame = TRUE;
            pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            pstVencChnAttr->stRcAttr.stAttrMjpegFixQp.u32Qfactor = 50;
            pstVencChnAttr->stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = u32SrcFrmRateNum;
        }
        break;
        default:
            CamOsPrintf("unsupport eModType[%u]\n", eModType);
            return E_MI_ERR_FAILED;
    }
    return MI_SUCCESS;
}

static inline MI_S32 GetVifPadMapGroup(MI_SNR_PADID sPad, MI_VIF_GROUP *mGroup)
{
    MI_S32 nRet = 0x0;
    if(NULL == mGroup)
    {
        return -1;
    }

    if(0 == sPad)
    {
        *mGroup = 0;
    }
    else if(1 == sPad)
    {
        *mGroup = 2;
    }
    else if(2 == sPad)
    {
        *mGroup = 1;
    }
    else if(3 == sPad)
    {
        *mGroup = 3;
    }
    else
    {
     nRet = -1;
    }
    return nRet;
}

static int ST_AI_Autorun(void)
{
    MI_AUDIO_DEV AiDevId     = 0;
    MI_AI_Attr_t stAiSetAttr = {0};
    MI_AI_Attr_t stAiGetAttr = {0};
    MI_AI_If_e enAiIf[]      = {E_MI_AI_IF_ADC_AB, E_MI_AI_IF_ECHO_A};
    MI_U8 u8ChnGrpId         = 0;
    MI_S16 s8DpgaGain[]      = {-10};
    MI_U32 u32FrameCnt       = 5;
    MI_AUDIO_SampleRate_e eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_8000;
    //set output port buffer depth
    MI_SYS_ChnPort_t stAiChnOutputPort;

    CamOsPrintf("Func %s start\n", __FUNCTION__);
    memset(&stAiSetAttr, 0, (size_t)sizeof(MI_AI_Attr_t));
    memset(&stAiGetAttr, 0, (size_t)sizeof(MI_AI_Attr_t));

    stAiSetAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAiSetAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiSetAttr.enSampleRate = eAISetSamplerate;
    stAiSetAttr.u32PeriodSize = 1024;
    stAiSetAttr.bInterleaved = TRUE;
    /* open ai device */
    STCHECKRESULT(MI_AI_Open(AiDevId, &stAiSetAttr));

    /* get ai device */
    STCHECKRESULT(MI_AI_GetAttr(AiDevId, &stAiGetAttr));

    /* attach ai interface */
    STCHECKRESULT(MI_AI_AttachIf(AiDevId, enAiIf, sizeof(enAiIf) / sizeof(enAiIf[0])));

    /* set ai interface gain */
    STCHECKRESULT(MI_AI_SetIfGain(E_MI_AI_IF_ADC_AB, 10, 10));

    /* set output depth */
    memset(&stAiChnOutputPort, 0, (size_t)sizeof(stAiChnOutputPort));
    stAiChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stAiChnOutputPort.u32DevId = AiDevId;
    stAiChnOutputPort.u32ChnId = u8ChnGrpId;
    stAiChnOutputPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stAiChnOutputPort, u32FrameCnt, u32FrameCnt));

    /* set ai dpga gain */
    STCHECKRESULT(MI_AI_SetGain(AiDevId, u8ChnGrpId, s8DpgaGain, sizeof(s8DpgaGain) / sizeof(s8DpgaGain[0])));

    /* enable ai device channel */
    STCHECKRESULT(MI_AI_EnableChnGroup(AiDevId, u8ChnGrpId));

    CamOsPrintf("Func %s end\n", __FUNCTION__);
    return 0;
}

static int ST_AO_Autorun(void)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_S8 s8LeftVolume, s8RightVolume;
    MI_AO_GainFading_e eGainFading;
    MI_AUDIO_SampleRate_e eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_8000;
    MI_AO_Attr_t stAoSetAttr, stAoGetAttr;
    MI_S32 s32AoVolume = 0;

    CamOsPrintf("Func %s start\n", __FUNCTION__);
    memset(&stAoSetAttr, 0x0, (size_t)sizeof(MI_AO_Attr_t));

    stAoSetAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAoSetAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAoSetAttr.enSampleRate = eOutSampleRate;
    stAoSetAttr.u32PeriodSize = 1024;
    stAoSetAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_DOUBLE_MONO;
    /* open ao device */
    STCHECKRESULT(MI_AO_Open(AoDevId, &stAoSetAttr));
    /* get ao device */
    STCHECKRESULT(MI_AO_GetAttr(AoDevId, &stAoGetAttr));
    /* attach ao device interface */
    STCHECKRESULT(MI_AO_AttachIf(AoDevId, E_MI_AO_IF_DAC_AB, 0));

    //STCHECKRESULT(MI_AO_SetChannelMode(AoDevId, E_MI_AO_CHANNEL_MODE_ONLY_LEFT));
    s8LeftVolume = s32AoVolume;
    s8RightVolume = s32AoVolume;
    eGainFading = E_MI_AO_GAIN_FADING_OFF;
    /* set ao device volume */
    STCHECKRESULT(MI_AO_SetVolume(AoDevId, s8LeftVolume, s8RightVolume, eGainFading));
    CamOsPrintf("Func %s end\n", __FUNCTION__);
    return 0;
}

MI_S32 ST_StartPipeLine(MI_U8 i, MI_U32 u32Width, MI_U32 u32Height, MI_U32 u32CropW, MI_U32 u32CropH, MI_U32 u32CropX, MI_U32 u32CropY)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_VENC_DEV VencDevId = 0;
    MI_VENC_InitParam_t tVencParam;
    MI_VENC_ChnAttr_t stVencChnAttr;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32FrameCnt = 0;
    MI_U32 u32SrcFrmrate = 0;
    MI_U32 u32DstFrmrate = 0;
    MI_U32 u32BindParam = 0;
    MI_SYS_BindType_e eBindType;

    if (RtosPreloadIsUseReduceMemCfg())
        u32FrameCnt = 3;
    else
        u32FrameCnt = 30;

    MI_ISP_DEV IspDevId = 0;
    MI_SCL_DEV SclDevId = 0;
    MI_ISP_OutPortParam_t  stIspOutputParam;
    MI_SCL_OutPortParam_t  stSclOutputParam;
    MI_VENC_SuperFrameCfg_t stSuperFrameCfg = { E_MI_VENC_SUPERFRM_NONE, 0, 0, 0 };

    memset(&stIspOutputParam, 0x0, (size_t)sizeof(MI_ISP_OutPortParam_t));
    STCHECKRESULT(MI_ISP_GetInputPortCrop(IspDevId, i, &stIspOutputParam.stCropRect));

    memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
    stSclOutputParam.stSCLOutCropRect.u16X = stIspOutputParam.stCropRect.u16X;
    stSclOutputParam.stSCLOutCropRect.u16Y = stIspOutputParam.stCropRect.u16Y;
    stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;
    stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
    stSclOutputParam.stSCLOutputSize.u16Width = u32Width;
    stSclOutputParam.stSCLOutputSize.u16Height = u32Height;
    stSclOutputParam.bMirror = FALSE;
    stSclOutputParam.bFlip = FALSE;
    stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
    stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, i, 0, &stSclOutputParam));
    STCHECKRESULT(MI_ISP_SetOutputPortParam(IspDevId, i, 0, &stIspOutputParam));

    /************************************************
    init VENC
    *************************************************/
    if(pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
        VencDevId = MI_VENC_DEV_ID_JPEG_0;
    else
        VencDevId = MI_VENC_DEV_ID_H264_H265_0;
    memset(&stVencChnAttr, 0x0, (size_t)sizeof(MI_VENC_ChnAttr_t));
    STUB_GetVencConfig(pstStreamAttr[i].eType, &stVencChnAttr, i);

    tVencParam.u32MaxWidth = pstStreamAttr[i].u32MaxWidth;
    tVencParam.u32MaxHeight = pstStreamAttr[i].u32MaxHeight;

    MI_VENC_CreateDev(MI_VENC_DEV_ID_H264_H265_0, &tVencParam);
    STCHECKRESULT(MI_VENC_CreateChn(VencDevId, pstStreamAttr[i].vencChn, &stVencChnAttr));
    STCHECKRESULT(MI_VENC_SetMaxStreamCnt(VencDevId, pstStreamAttr[i].vencChn, u32FrameCnt));
    if(pstStreamAttr[i].eType != E_MI_VENC_MODTYPE_JPEGE && pstStreamAttr[i].eBindType != E_MI_SYS_BIND_TYPE_HW_RING)
    {
        stSuperFrameCfg.u32SuperIFrmBitsThr = _maxFrameSize[0].frameSizeI;
        stSuperFrameCfg.u32SuperPFrmBitsThr = _maxFrameSize[0].frameSizeP;
        STCHECKRESULT(MI_VENC_SetSuperFrameCfg(VencDevId, pstStreamAttr[i].u32InputChn, &stSuperFrameCfg));
    }
    if (pstStreamAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
    {
        MI_VENC_InputSourceConfig_t stVencSourceCfg;
        memset(&stVencSourceCfg, 0, (size_t)sizeof(MI_VENC_InputSourceConfig_t));
        stVencSourceCfg.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA;
        MI_VENC_SetInputSourceConfig(VencDevId, pstStreamAttr[i].vencChn, &stVencSourceCfg);
    }
    else
    {
        MI_VENC_InputSourceConfig_t stVencSourceCfg;
        memset(&stVencSourceCfg, 0, (size_t)sizeof(MI_VENC_InputSourceConfig_t));
        stVencSourceCfg.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_NORMAL_FRMBASE;
        MI_VENC_SetInputSourceConfig(VencDevId, pstStreamAttr[i].vencChn, &stVencSourceCfg);
    }

    /************************************************
    Bind VPE->VENC
    *************************************************/
    memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId = SclDevId;
    stSrcChnPort.u32ChnId = i;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId = VencDevId;
    stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate = 15;
    u32DstFrmrate = 15;
    eBindType = E_MI_SYS_BIND_TYPE_HW_RING;

    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, i, 0));
    STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, i, 0));

    STCHECKRESULT(MI_VENC_StartRecvPic(VencDevId, pstStreamAttr[i].vencChn));
    CamOsPrintf("chn %d startPipeLine:vpeChn[%d],vpePort[%d],vencChn[%d],venc bindtype[%d]\n", i,
            pstStreamAttr[i].u32InputChn,pstStreamAttr[i].u32InputPort,pstStreamAttr[i].vencChn,pstStreamAttr[i].eBindType);


    return MI_SUCCESS;
}

MI_S32 ST_ExitPipeLine(void)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_SCL_DEV SclDevId = 0;
    MI_VENC_DEV VencDevId = 0;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U8 i = 0;

    /************************************************
    Destroy VENC
    *************************************************/
    for(i = 0; i < g_u32SensorNum; i++)
    {
        if(pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
            VencDevId = MI_VENC_DEV_ID_JPEG_0;
        else
            VencDevId = MI_VENC_DEV_ID_H264_H265_0;
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId = SclDevId;
        stSrcChnPort.u32ChnId = i;
        stSrcChnPort.u32PortId = 0;
        stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId = VencDevId;
        stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
        stDstChnPort.u32PortId = 0;
        MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
        MI_VENC_StopRecvPic(VencDevId, pstStreamAttr[i].vencChn);
        MI_VENC_DestroyChn(VencDevId, pstStreamAttr[i].vencChn);
    }

    return MI_SUCCESS;
}

MI_VENC_ModType_e ST_GetVideoModType(MI_U8 u8VideoFormat)
{
    MI_VENC_ModType_e eVideoModType = E_MI_VENC_MODTYPE_H264E;
    switch(u8VideoFormat)
    {
        case 0:
            eVideoModType = E_MI_VENC_MODTYPE_H264E;
            break;
        case 1:
            eVideoModType = E_MI_VENC_MODTYPE_H265E;
            break;
        case 2:
            eVideoModType = E_MI_VENC_MODTYPE_JPEGE;
            break;
        default:
            CamOsPrintf("unsupport VideoFormat[%u]\n", u8VideoFormat);
            eVideoModType = E_MI_VENC_MODTYPE_H264E;
            break;
    }
    return eVideoModType;
}

void ST_DeinitMoudle(void)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_ISP_DEV             IspDevId = 0;
    MI_SCL_DEV             SclDevId = 0;
    MI_VENC_DEV           VencDevId = 0;
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32VencChnId = 0;
    MI_U16                 u16SocId = 0;
    MI_RGN_ChnPort_t   stRgnChnPort;
    MI_U32                        i = 0;
    MI_AUDIO_DEV            AiDevId = 0;
    MI_AUDIO_DEV            AoDevId = 0;

    if(pstStreamAttr[0].eType == E_MI_VENC_MODTYPE_JPEGE)
        VencDevId = MI_VENC_DEV_ID_JPEG_0;
    else
        VencDevId = MI_VENC_DEV_ID_H264_H265_0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    for (i = 0; i < MAX_FRAME_HANDLE; i++)
    {
        /************************************************
        step1 :detach rgn->venc
        *************************************************/
        MI_RGN_DetachFromChn(u16SocId, hFrame[i], &stRgnChnPort);
        MI_RGN_Destroy(u16SocId, hFrame[i]);
    }
    MI_RGN_DeInit(u16SocId);

    MI_AI_DisableChnGroup(AiDevId, 0);
    MI_AI_Close(AiDevId);
    MI_AO_DetachIf(AoDevId, E_MI_AO_IF_DAC_AB);
    MI_AO_Close(AoDevId);

    for(i = 0; i < g_u32SensorNum; i++)
    {
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId = IspDevId;
        stSrcChnPort.u32ChnId = i;
        stSrcChnPort.u32PortId = 0;
        stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId = SclDevId;
        stDstChnPort.u32ChnId = i;
        stDstChnPort.u32PortId = 0;
        MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
    }

    /************************************************
    Destroy SCL
    *************************************************/
    for(i = 0; i < g_u32SensorNum; i++)
    {
        MI_SCL_DisableOutputPort(SclDevId, i, 0);
        MI_SCL_DisableOutputPort(SclDevId, i, 1);
        MI_SCL_StopChannel(SclDevId, i);
        MI_SCL_DestroyChannel(SclDevId, i);
    }
    MI_SCL_DestroyDevice(SclDevId);

    for(i = 0; i < g_u32SensorNum; i++)
    {
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId = i * 4;
        stSrcChnPort.u32ChnId = 0;
        stSrcChnPort.u32PortId = 0;
        stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId = IspDevId;
        stDstChnPort.u32ChnId = i;
        stDstChnPort.u32PortId = 0;
        MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
    }

    /************************************************
    Destroy ISP
    *************************************************/
    for(i = 0; i < g_u32SensorNum; i++)
    {
        MI_ISP_StopChannel(IspDevId, i);
        MI_ISP_DestroyChannel(IspDevId, i);
    }
    MI_ISP_DestoryDevice(IspDevId);
    /************************************************
    Destroy VIF
    *************************************************/
    for(i = 0; i < g_u32SensorNum; i++)
    {
        MI_VIF_DisableOutputPort(i * 4, 0);
        MI_VIF_DisableDev(i * 4);
    }

    MI_VENC_DestroyDev(MI_VENC_DEV_ID_H264_H265_0);
}

int _MI_Cli_GetDataL2R(void *data)
{
    char *string = (char *)data;
    int len = 0;
    len = strlen((const char *)string);
    if(len > 0)
    {
        // printf("recv from linux data: %s\n", string);
        if(strncmp(data, AOV_GET_STATUS, strlen(AOV_GET_STATUS)) == 0)
        {
            return s32Aov_Get_Status;
        }
        else if(strncmp(data, "quit", 4) == 0)
        {
            // bThreadExit = 0;
        }
        else if(strncmp(data, AOV_GET_SENSORNUM, strlen(AOV_GET_SENSORNUM)) == 0)
        {
            return g_u32SensorNum;
        }
    }
    else
    {
        printf("recv from linux data is null\n");
    }

    return 0;
}

void _Exit_PreloadForce(void)
{
    if(bThreadExit)
    {
        bThreadExit = 0;
        CamOsMsSleep(50);
    }

    if(g_u8stoppipe == 0)
    {
        ST_ExitPipeLine();
    }

    if(g_u8deintmoudle == 0)
    {
        ST_DeinitMoudle();
    }

    MI_DEVICE_RegMiSysExitCall(NULL);
    MI_DEVICE_RegMiCliGetDataL2R(MI_CLICMD_PRELOAD, NULL);
    CamOsTsemDeinit(&tPreloadFileTsem);
    CamOsTsemDeinit(&tIspReadFileTsem);
    CamOsThreadStop(PreloadFile_tid);
    CamOsThreadStop(PreloadMiPipe_tid);
}

static MI_S32 STUB_BaseModuleInit(void)
{
    MI_VIF_DevAttr_t stVifDevAttr;
    MI_U32 u32VifDevId;
    MI_U32 u32VifChnId;
    MI_U32 u32VifPortId;

    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
    MI_SYS_BindType_e eBindType;

    MI_U32 i = 0;
    MI_BOOL bMirror = FALSE, bFlip = FALSE;
    MI_SYS_Rotate_e eSetType = E_MI_SYS_ROTATE_NONE;
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_SNR_PADID u8SnrPad = 0;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_VIF_GroupAttr_t stGroupAttr;
    MI_VIF_OutputPortAttr_t stVifPortAttr;
    MI_SNR_PADInfo_t  stSnrPadInfo;
    MI_VIF_GROUP nVCapGroup = 0;
    MI_SCL_DEV SclDevId = 0;
    MI_SCL_CHANNEL SclChnId = 0;
    MI_SCL_DevAttr_t stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t  stSclChnParam;
    MI_ISP_DEV IspDevId = 0;
    MI_ISP_CHANNEL IspChnId = 0;
    MI_ISP_DevAttr_t stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t stIspChnParam;
    MI_U32 u32snr_fps = 15;
    MI_U32 u32LightShutter    = 0;
    MI_U32 u32LightSensorGain = 0;
    MI_U32 u32LightIspGain    = 0;
    char stPatch[128] = {0};
    MasterEarlyInitParam_t *pstEarlyInitParam;

    memset(&g_st_init_setting,0,sizeof(InitPreloadCfg_t));
    g_st_init_setting.u32NumSnr = g_u32SensorNum;
    if(ST_GetRtosPreloadInitParam(&g_st_init_setting) < 0)
    {
        CamOsPrintf("[error]:%s:%d init cfg param is fail!\n", __func__, __LINE__);
        return -1;
    }
    g_u32SensorNum = g_st_init_setting.u32NumSnr? g_st_init_setting.u32NumSnr:g_u32SensorNum;
    MI_DEVICE_RegMiSysExitCall(_Exit_PreloadForce);
    MI_DEVICE_RegMiCliGetDataL2R(MI_CLICMD_PRELOAD, _MI_Cli_GetDataL2R);

   for(i=0; i<g_u32SensorNum; ++i)
   {
        MI_U32 u32SnrResCount = 0;
        MI_U32 u32GetSnrResCount = 0;
        MI_U8 index = 0x0, l=0x0, j = 0x0, k = 0x0;
        MI_U8 tIndex = (MI_U8)(-1);
        MI_SNR_Res_t stSensorRes[20], stSensorCurRes;
        MI_U8 tRecordIndexj[20] = {0x0}, tRecordIndexk[20] = {0x0};
        MI_U32  sensorDvalue = 0;
        MI_U32 minSensorDvalue = -1;
        MI_BOOL  statue = false;
        MI_U32  Dvalue = 0;
        MI_U32 minDvalue = -1;
        MI_U32 sFps = 30;  
        if(g_st_init_setting.ChCfg[i].u8SnrPad < 0)
        {
            CamOsPrintf("[warning]:%s:%d sensor[%d] no open!\n", __func__, __LINE__, i);
            continue;
        }
        u8SnrPad = g_st_init_setting.ChCfg[i].u8SnrPad;//i*2;//2+2lane
        IspChnId = i;
        SclChnId = i;
        tIndex = g_st_init_setting.ChCfg[i].u8ResIdx;
        u32snr_fps = g_st_init_setting.ChCfg[i].u32SensorFrameRate/1000;
        u8SnrPad = g_st_init_setting.ChCfg[i].u8SnrPad;
        bMirror = g_st_init_setting.ChCfg[i].bMirror;
        bFlip = g_st_init_setting.ChCfg[i].bFlip;        

        /************************************************
        Step2:  init VIF(for IPC, only one dev)
        *************************************************/
        MI_SNR_SetPlaneMode(u8SnrPad,0);
        MI_SNR_QueryResCount(u8SnrPad, &u32GetSnrResCount);
        u32SnrResCount = u32GetSnrResCount;

        if(tIndex == (MI_U8)(-1)) //Not set by earlyinit yet
        {
            for(index = 0; index < u32SnrResCount; index++)
            {
                if(MI_SUCCESS != MI_SNR_GetRes(u8SnrPad, index, &stSensorRes[index]))
                {
                    CamOsPrintf("%s:%d Get sensor resolution index %d error!\n", __func__, __LINE__, index);
                    continue;
                }

                CamOsPrintf("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
                            index, stSensorRes[index].stCropRect.u16X, stSensorRes[index].stCropRect.u16Y,
                            stSensorRes[index].stCropRect.u16Width, stSensorRes[index].stCropRect.u16Height,
                            stSensorRes[index].stOutputSize.u16Width, stSensorRes[index].stOutputSize.u16Height,
                            stSensorRes[index].u32MaxFps, stSensorRes[index].u32MinFps, stSensorRes[index].strResDesc);
            }

            for(k = 0x0, j = 0x0, index = 0; index < u32SnrResCount; index++)
            {
                if((stSensorRes[index].stCropRect.u16Width == g_stStreamAttr[0].u32Width) &&
                    (stSensorRes[index].stCropRect.u16Height == g_stStreamAttr[0].u32Height))
                {
                    tRecordIndexj[j] = index;
                    j++;
                    CamOsPrintf("find res, j:%d, index:%d.\n", j, index);
                }
                else if((stSensorRes[index].stCropRect.u16Width > g_stStreamAttr[0].u32Width) &&
                    (stSensorRes[index].stCropRect.u16Height > g_stStreamAttr[0].u32Height))
                {
                    sensorDvalue = stSensorRes[index].stCropRect.u16Width * stSensorRes[index].stCropRect.u16Height - g_stStreamAttr[0].u32Width * g_stStreamAttr[0].u32Height;
                    if(minSensorDvalue > sensorDvalue)
                    {
                        minSensorDvalue = sensorDvalue;

                        k = 0x0;
                        tRecordIndexk[k] = index;
                        k++;
                    }
                    else if(minSensorDvalue == sensorDvalue)
                    {
                        tRecordIndexk[k] = index;
                        k++;
                    }
                }
            }

            //set snr resolution
            if(0x0 != j)
            {
                for(l = 0; l < j; l++)
                {
                    index = tRecordIndexj[l];
                    if(sFps == stSensorRes[index].u32MaxFps)
                    {
                        statue = true;
                        tIndex = index;
                        CamOsPrintf("index:%d.\n", tRecordIndexj[l]);
                        break;
                    }
                    else if(sFps < stSensorRes[index].u32MaxFps)
                    {
                        Dvalue = stSensorRes[index].u32MaxFps - sFps;
                        if(Dvalue < minDvalue)
                        {
                            minDvalue = Dvalue;
                            statue = true;
                            tIndex = index;
                        }
                        continue;
                    }
                    else
                    {
                        CamOsPrintf("target fps is larger than sensor maxfps.\n");
                        statue = true;
                        tIndex = index;
                    }
                }
            }
            else if(0x0 != k)
            {
                for(l = 0; l < k; l++)
                {
                    index = tRecordIndexk[l];
                    if(sFps == stSensorRes[index].u32MaxFps)
                    {
                        statue = true;
                        tIndex = index;
                        CamOsPrintf("index:%d.\n", tRecordIndexk[l]);
                        break;
                    }
                    else if(sFps < stSensorRes[index].u32MaxFps)
                    {
                        Dvalue = stSensorRes[index].u32MaxFps - sFps;
                        if(Dvalue < minDvalue)
                        {
                            minDvalue = Dvalue;
                            statue = true;
                            tIndex = index;
                        }
                        continue;
                    }
                    else
                    {
                        CamOsPrintf("target fps is larger than sensor maxfps.\n");
                        statue = true;
                        tIndex = index;
                    }
                }
            }

            if(statue == false)
            {
                CamOsPrintf("can not find res, index user 0.\n");
                tIndex = 0x0;
            }
            else
            {
                CamOsPrintf("find res. index:%d.\n", tIndex);
            }
        }

        CamOsPrintf("%s:set SnrRes idx = %d\n", __func__, tIndex);
        MI_SNR_SetRes(u8SnrPad, tIndex);
        memset(&stSensorCurRes, 0x00, (size_t)sizeof(MI_SNR_Res_t));
        MI_SNR_GetRes(u8SnrPad, tIndex, &stSensorCurRes);
        if(stSensorCurRes.u32MinFps <= u32snr_fps && u32snr_fps <= stSensorCurRes.u32MaxFps)
        {
            MI_SNR_SetFps(u8SnrPad, u32snr_fps);
            CamOsPrintf("%s:%d current sensor fps(min:%d, max:%d), Set new sensor fps:%d\n", __func__, __LINE__,
                stSensorCurRes.u32MinFps, stSensorCurRes.u32MaxFps, u32snr_fps);
        }
        MI_SNR_SetOrien(u8SnrPad, bMirror, bFlip);
        //MI_SNR_Enable(u8SnrPad); will move to after isp init

        /************************************************
        Step2:  init VIF
        *************************************************/
        switch(u8SnrPad)
        {
            case 0:
                u32VifDevId = 0;
                break;
            case 1:
                u32VifDevId = 8;
                break;
            case 2:
                u32VifDevId = 4;
                break;
            case 3:
                u32VifDevId = 12;
                break;
            default:
                u32VifDevId = 0;
        }
        u32VifChnId = 0;
        u32VifPortId = 0;
        memset(&stVifDevAttr, 0x0, (size_t)sizeof(MI_VIF_DevAttr_t));

        memset(&stGroupAttr, 0x0, (size_t)sizeof(MI_VIF_GroupAttr_t));
        memset(&stSnrPadInfo, 0x0, (size_t)sizeof(MI_SNR_PADInfo_t));

        STCHECKRESULT(MI_SNR_GetPadInfo((MI_SNR_PADID)u8SnrPad, &stSnrPadInfo));

        STCHECKRESULT(GetVifPadMapGroup(u8SnrPad, &nVCapGroup));

        stGroupAttr.eWorkMode = E_MI_VIF_WORK_MODE_1MULTIPLEX;
        stGroupAttr.eHDRType = E_MI_VIF_HDR_TYPE_OFF;
        stGroupAttr.eIntfMode = (MI_VIF_IntfMode_e)stSnrPadInfo.eIntfMode;
        if(E_MI_VIF_MODE_BT656 == stGroupAttr.eIntfMode)
        {
            stGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stSnrPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
        }
        else
        {
            stGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
        }
        stGroupAttr.eScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        STCHECKRESULT(MI_VIF_CreateDevGroup(nVCapGroup, &stGroupAttr));
        memset(&stSnrPlane0Info, 0x0, (size_t)sizeof(MI_SNR_PlaneInfo_t));
        MI_SNR_GetPlaneInfo(u8SnrPad, 0, &stSnrPlane0Info);
        stVifDevAttr.stInputRect.u16X = stSnrPlane0Info.stCapRect.u16X;
        stVifDevAttr.stInputRect.u16Y = stSnrPlane0Info.stCapRect.u16Y;
        stVifDevAttr.stInputRect.u16Width = stSnrPlane0Info.stCapRect.u16Width;
        stVifDevAttr.stInputRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
        get_sensor_size.u16Width = stSnrPlane0Info.stCapRect.u16Width;
        get_sensor_size.u16Height = stSnrPlane0Info.stCapRect.u16Height;

        if(stSnrPlane0Info.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
        {
            stVifDevAttr.eInputPixel = stSnrPlane0Info.ePixel;
        }
        else
        {
            stVifDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
        }
        STCHECKRESULT(MI_VIF_SetDevAttr(u32VifDevId, &stVifDevAttr));
        STCHECKRESULT(MI_VIF_EnableDev(u32VifDevId));

        memset(&stVifPortAttr, 0, (size_t)sizeof(stVifPortAttr));
        stVifPortAttr.stCapRect.u16X = stSnrPlane0Info.stCapRect.u16X;
        stVifPortAttr.stCapRect.u16Y = stSnrPlane0Info.stCapRect.u16Y;
        stVifPortAttr.stCapRect.u16Width = stSnrPlane0Info.stCapRect.u16Width;
        stVifPortAttr.stCapRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
        stVifPortAttr.stDestSize.u16Width = stSnrPlane0Info.stCapRect.u16Width;
        stVifPortAttr.stDestSize.u16Height = stSnrPlane0Info.stCapRect.u16Height;
        stVifPortAttr.eFrameRate = E_MI_VIF_FRAMERATE_FULL;
        if(stSnrPlane0Info.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
            stVifPortAttr.ePixFormat = stSnrPlane0Info.ePixel;
        else
            stVifPortAttr.ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
        stVifPortAttr.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        STCHECKRESULT(MI_VIF_SetOutputPortAttr(u32VifDevId, u32VifPortId, &stVifPortAttr));
        //STCHECKRESULT(MI_VIF_EnableOutputPort(u32VifDevId, u32VifPortId)); //realtime need move to pipeline init ready

        /************************************************
        Step3:  Init Isp
        *************************************************/
        memset(&stIspDevAttr, 0x0, (size_t)sizeof(MI_ISP_DevAttr_t));
        memset(&stIspChnAttr, 0x0, (size_t)sizeof(MI_ISP_ChannelAttr_t));
        memset(&stIspChnParam, 0x0, (size_t)sizeof(MI_ISP_ChnParam_t));

        stIspDevAttr.u32DevStitchMask = E_MI_ISP_DEVICEMASK_ID0;
        if(g_u32SensorNum > 1)
        {
            stIspDevAttr.u32DevStitchMask |= E_MI_ISP_DEVICEMASK_ID1;
        }
        STCHECKRESULT(MI_ISP_CreateDevice(IspDevId, &stIspDevAttr));
        BootTimestampRecord(__LINE__, "Isp create dev");

        switch(u8SnrPad)
        {
            case 0:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR0;
                break;
            case 1:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR1;
                break;
            case 2:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR2;
                break;
            case 3:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR3;
                break;
            default:
                CamOsPrintf("Invalid Snr pad id:%d\n", (int)u8SnrPad);
                return -1;
        }

        stIspChnParam.eHDRType = E_MI_ISP_HDR_TYPE_OFF;

        stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL2;

        stIspChnParam.bMirror = FALSE;
        stIspChnParam.bFlip = FALSE;
        stIspChnParam.eRot = eSetType;
        pstEarlyInitParam = (MasterEarlyInitParam_t*) &stIspChnAttr.stIspCustIqParam.stVersion.u8Data[0];
        pstEarlyInitParam->u16SnrEarlyFps = u32snr_fps;
        pstEarlyInitParam->u16SnrEarlyFlicker = g_st_init_setting.ChCfg[i].u16SnrEarlyFlicker;
        pstEarlyInitParam->u32SnrEarlyShutter =  g_st_init_setting.ChCfg[i].u32ShutterLEF;
        pstEarlyInitParam->u32SnrEarlyGainX1024 = g_st_init_setting.ChCfg[i].u32SensorGainLEFx1024;
        pstEarlyInitParam->u32SnrEarlyDGain = 1024;
        pstEarlyInitParam->u16SnrEarlyAwbRGain = 2263;
        pstEarlyInitParam->u16SnrEarlyAwbGGain = 1024;
        pstEarlyInitParam->u16SnrEarlyAwbBGain = 2134;

        stIspChnAttr.stIspCustIqParam.stVersion.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
        stIspChnAttr.stIspCustIqParam.stVersion.u32Size = sizeof(MasterEarlyInitParam_t);

        STCHECKRESULT(MI_ISP_CreateChannel(IspDevId, IspChnId, &stIspChnAttr));
        BootTimestampRecord(__LINE__, "Isp create channel");

        STCHECKRESULT(MI_ISP_SetChnParam(IspDevId, IspChnId, &stIspChnParam));
        STCHECKRESULT(MI_ISP_StartChannel(IspDevId, IspChnId));

        CamOsSprintf(&stPatch[0], "%s/%s%u%s", application_selector_get_rofile_path(), _ISP_API, u8SnrPad, _EXT_BIN);

        MI_ISP_ApiCmdLoadBinFile(IspDevId, IspChnId, &stPatch[0], 1234);
        if (i == g_u32SensorNum - 1)
        {
            CamOsTsemUp(&tIspReadFileTsem);
        }

        {
            CusAESource_e eAeSource = AE_SOURCE_FROM_SE_ALSC_AF_HDR;
            MI_ISP_CUS3A_SetAESource(IspDevId, IspChnId, &eAeSource);
        }
        BootTimestampRecord(__LINE__, "Isp load api done");

        MI_SNR_Enable(u8SnrPad);

        /************************************************
        Step4:  Bind VIF->ISP
        *************************************************/
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId = u32VifDevId;
        stSrcChnPort.u32ChnId = u32VifChnId;
        stSrcChnPort.u32PortId = u32VifPortId;
        stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId = IspDevId;
        stDstChnPort.u32ChnId = IspChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate = u32snr_fps;
        u32DstFrmrate = u32snr_fps;

        eBindType = g_st_init_setting.ChCfg[i].eBindMode;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, 0));

        /************************************************
        Step5:  init SCL
        *************************************************/
        memset(&stSclDevAttr, 0x0, (size_t)sizeof(MI_SCL_DevAttr_t));
        memset(&stSclChnAttr, 0x0, (size_t)sizeof(MI_SCL_ChannelAttr_t));
        memset(&stSclChnParam, 0x0, (size_t)sizeof(MI_SCL_ChnParam_t));

        stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1;
        STCHECKRESULT(MI_SCL_CreateDevice(SclDevId, &stSclDevAttr));

        STCHECKRESULT(MI_SCL_CreateChannel(SclDevId, SclChnId, &stSclChnAttr));
        STCHECKRESULT(MI_SCL_SetChnParam(SclDevId, SclChnId, &stSclChnParam));
        STCHECKRESULT(MI_SCL_StartChannel(SclDevId, SclChnId));

        if(i == 0)
        {
            MI_SYS_GlobalPrivPoolConfig_t stConfig;
            memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

            stConfig.eConfigType                                     = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
            stConfig.bCreate                                         = TRUE;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule      = E_MI_MODULE_ID_SCL;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid     = SclDevId;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth  = pstStreamAttr[i].u32Width;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = pstStreamAttr[i].u32Height;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine  = pstStreamAttr[i].u32Height;
            STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));
        }

        /************************************************
        Step6:  Bind ISP->SCL
        *************************************************/
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId = IspDevId;
        stSrcChnPort.u32ChnId = IspChnId;
        stSrcChnPort.u32PortId = 0;
        stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId = SclDevId;
        stDstChnPort.u32ChnId = SclChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate = u32snr_fps;
        u32DstFrmrate = u32snr_fps;
        eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, 0));

        // STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 2, 4));

        stDstChnPort.u32PortId = 1;
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 2, 4));
    }

    for(i = 0; i < g_u32SensorNum; i++)
    {
        ST_StartPipeLine(i, pstStreamAttr[i].u32Width, pstStreamAttr[i].u32Height, pstStreamAttr[i].u32CropWidth, pstStreamAttr[i].u32CropHeight, pstStreamAttr[i].u32CropX, pstStreamAttr[i].u32CropY);

        u8SnrPad = i*2;
        IspChnId = i;
        switch(u8SnrPad)
        {
            case 0:
                u32VifDevId = 0;
                break;
            case 1:
                u32VifDevId = 8;
                break;
            case 2:
                u32VifDevId = 4;
                break;
            case 3:
                u32VifDevId = 12;
                break;
            default:
                u32VifDevId = 0;
        }
        u32VifPortId = 0;

        //set shutter & gain from light sensor
        if(i == 0)
        {
            //get lux and light sensor shutter and gain;
            LightTable_t stLightParam;
            memset(&stLightParam, 0x0, (size_t)sizeof(LightTable_t));
            if(Preload_LightSensorGetLux(&stLightParam) == 0)
            {
                u32LightShutter    = stLightParam.u32Shutter;
                u32LightSensorGain = stLightParam.u32Sensorgain;
                u32LightIspGain    = stLightParam.u32Ispgain;
                Preload_LightSensorPowerOnOff(0); //low power mode, after get lux,power off light
            }
        }
        //if have day and night api bin, move load api bin here judge by lux
        //MI_ISP_ApiCmdLoadBinFile(IspDevId, IspChnId, &stPatch[0], 1234);

        if(u32LightShutter != 0 && u32LightSensorGain != 0)
        {
            MI_VIF_ShutterGainParams_t stSnrParam;
            stSnrParam.u32ShutterTimeUs = u32LightShutter;
            stSnrParam.u32AeGain        = u32LightSensorGain;
            MI_VIF_CustFunction(u32VifDevId, E_MI_VIF_CUSTCMD_SHUTTER_GAIN_SET, sizeof(MI_VIF_ShutterGainParams_t), &stSnrParam);

            CusAEResult_t   AEResult;
            memset(&AEResult, 0x0, sizeof(CusAEResult_t));
            AEResult.Size   = sizeof(CusAEResult_t);
            AEResult.Change     = 1;
            AEResult.Shutter    = u32LightShutter;
            AEResult.SensorGain = u32LightSensorGain;
            AEResult.IspGain    = u32LightIspGain;
            CamOsPrintf("AEResult>>>>Shutter:%d,gain:%d,ispgain:%d\n", AEResult.Shutter, AEResult.SensorGain, AEResult.IspGain);
            MI_ISP_CUS3A_SetAeParam(IspDevId,IspChnId, &AEResult);
        }

        STCHECKRESULT(MI_VIF_EnableOutputPort(u32VifDevId, u32VifPortId));
     }

    return MI_SUCCESS;
}

int ST_RGN_Init(void)
{
    MI_U16                u16SocId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRegionAttr;
    ST_Stream_Attr_T *    pstStreamAttr = g_stStreamAttr;

    MI_RGN_ChnPort_t stRgnChnPort;
    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = 0;
    stRgnChnPort.s32ChnId  = 0;
    stRgnChnPort.s32PortId = 0;

    gstRgnChnPortParam.u32Layer                   = 0;
    gstRgnChnPortParam.bShow                      = FALSE;
    gstRgnChnPortParam.stFrameChnPort.u8Thickness = pstStreamAttr[0].u32Height * 0.005;//get_sensor_size.u16Height * 0.005;
    gstRgnChnPortParam.stFrameChnPort.u32Color    = 0x00ff00;

    //actual width/screen width * 8192
    gstRgnChnPortParam.stFrameChnPort.stRect.s32X      = 240 * 8192 / pstStreamAttr[0].u32Width;
    gstRgnChnPortParam.stFrameChnPort.stRect.s32Y      = 135 * 8192 / pstStreamAttr[0].u32Height;
    gstRgnChnPortParam.stFrameChnPort.stRect.u32Width  = 960 * 8192 / pstStreamAttr[0].u32Width;
    gstRgnChnPortParam.stFrameChnPort.stRect.u32Height = 540 * 8192 / pstStreamAttr[0].u32Height;

    STCHECKRESULT(MI_RGN_Init(u16SocId, &stPaletteTable));

    memset(&stRegionAttr, 0x00, sizeof(MI_RGN_Attr_t));
    stRegionAttr.eType = E_MI_RGN_TYPE_OSD; // or E_MI_RGN_TYPE_COVER
    // Just OSD need to set stOsdInitParam
    stRegionAttr.stOsdInitParam.ePixelFmt        = E_MI_RGN_PIXEL_FORMAT_I4; // or others
    stRegionAttr.stOsdInitParam.stSize.u32Width  = 100; // u32Width <= 3840
    stRegionAttr.stOsdInitParam.stSize.u32Height = 100; // u32Height <= 2160

    stRegionAttr.eType = E_MI_RGN_TYPE_FRAME;
    for (int j = 0; j < 32; j++)
    {
        hFrame[j] = j;
        STCHECKRESULT(MI_RGN_Create(u16SocId, hFrame[j], &stRegionAttr));
        /************************************************
        step10 :attch handle Frame to venc
        *************************************************/
        STCHECKRESULT(MI_RGN_AttachToChn(u16SocId, hFrame[j], &stRgnChnPort, &gstRgnChnPortParam));
    }

    return MI_SUCCESS;
}

void *ST_MiPipeSecondInit(void *p)
{
    ST_AI_Autorun();
    ST_AO_Autorun();

    ST_RGN_Init();

    return NULL;
}

int ST_DET_Init(void)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;

    /************************************************
    init det
    *************************************************/
    DetectionInfo_t stDetectionInfo;

    memset(&stDetectionInfo, 0x0, sizeof(DetectionInfo_t));

    CamOsSprintf(stDetectionInfo.ipu_firmware_path, "%s", application_selector_get_rofile_path());
    CamOsSprintf(stDetectionInfo.model, "%s/spdy48.img", stDetectionInfo.ipu_firmware_path);
    stDetectionInfo.threshold        = 0.5;
    stDetectionInfo.disp_size.width  = pstStreamAttr[0].u32Width;//get_sensor_size.u16Width;
    stDetectionInfo.disp_size.height = pstStreamAttr[0].u32Height;//get_sensor_size.u16Height;
    stDetectionInfo.had_create_device = false;

    STCHECKRESULT(ALGO_DET_CreateHandle(&pDetHandle));
    STCHECKRESULT(ALGO_DET_InitHandle(pDetHandle, &stDetectionInfo));

    CamOsTsemUp(&tPreloadFileTsem); // det algo flash get file done

    MI_S32   s32Tk_type = 0;
    MI_S32   s32Md_type = 0;
    bool     bStable    = true;
    MI_FLOAT fThreshold = 0.5;

    STCHECKRESULT(ALGO_DET_SetTracker(pDetHandle, s32Tk_type, s32Md_type));
    STCHECKRESULT(ALGO_DET_SetStableBox(pDetHandle, bStable));
    STCHECKRESULT(ALGO_DET_SetThreshold(pDetHandle, fThreshold));

    STCHECKRESULT(ALGO_DET_GetInputAttr(pDetHandle, &stDetModelAttr));

    MI_SCL_DEV SclDevId = 0;
    MI_SCL_OutPortParam_t  stSclDetOutputParam;
    MI_U32 u32SclScaledPortId = 1;

    memset(&stSclDetOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));

    stSclDetOutputParam.bFlip = FALSE;
    stSclDetOutputParam.bMirror = FALSE;
    stSclDetOutputParam.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stSclDetOutputParam.ePixelFormat  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    /*default not use scl portcrop */
    stSclDetOutputParam.stSCLOutCropRect.u16X = 0;
    stSclDetOutputParam.stSCLOutCropRect.u16Y = 0;
    stSclDetOutputParam.stSCLOutCropRect.u16Width  = 0;
    stSclDetOutputParam.stSCLOutCropRect.u16Height = 0;
    stSclDetOutputParam.stSCLOutputSize.u16Width = stDetModelAttr.width;
    stSclDetOutputParam.stSCLOutputSize.u16Height = stDetModelAttr.height;

    STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, 0, u32SclScaledPortId, &stSclDetOutputParam));
    STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, 0, u32SclScaledPortId));

    return 0;
}

MI_S32 ST_BasePipelineInit(void)
{
    CamFsFd tFd;
    MI_S32  s32Ret;
    char IpuFilePath[64] = {0};
    STCHECKRESULT(STUB_BaseModuleInit());

    CamOsThreadAttrb_t threadPreloadSecondInit = {.nPriority = 90,.szName = "MI_PreloadSecondInit",.nStackSize = 6*1024};
    CamOsThreadCreate(&PreloadSecond_tid, &threadPreloadSecondInit, ST_MiPipeSecondInit, NULL);

    CamOsSprintf(IpuFilePath, "%s", application_selector_get_rofile_path());
    strcat(IpuFilePath, "/spdy48.img");
    s32Ret = CamFsOpen(&tFd, IpuFilePath, CAM_FS_O_RDONLY, 0644);
    if(s32Ret == CAM_FS_OK)/*if have ipu file enter AOV mode*/
    {
        CamFsClose(tFd);
        CamOsThreadAttrb_t threadPreloadDetAttr = {.nPriority = 80,.szName = "MI_PreloadDet",.nStackSize = 15*1024};
        CamOsThreadCreate(&PreloadDet_tid, &threadPreloadDetAttr, ST_DetectThread, NULL);
    }
    else
    {
        CamOsPrintf("IPU file does not exist failed\n");
        CamOsTsemUp(&tPreloadFileTsem);
    }
    return 0;
}

void* MI_PreloadFile(void* p)
{
    BootTimestampRecord(__LINE__, "PreloadFile Start");

    CamOsTsemDown(&tPreloadFileTsem);
    CamOsTsemDown(&tIspReadFileTsem);//wait Isp Load api bin

    if (application_selector_retreat() != 0)
    {
        CamOsPrintf("application selector stop failed.\n");
    }
    BootTimestampRecord(__LINE__, "PreloadFile end");

    return 0;
}

void* MI_PreloadMiPipe(void* p)
{
    CamOsTimespec_t stSystemTimetemp;
    MI_U64 u64PtsBase;

    MI_SYS_Init(0);
    mi_debug_init();
    CamOsGetTimeOfDay(&stSystemTimetemp);
    u64PtsBase = stSystemTimetemp.nSec*1000000ULL+stSystemTimetemp.nNanoSec/1000;
    MI_SYS_InitPtsBase(0, u64PtsBase);

    BootTimestampRecord(__LINE__, "ST_BasePipelineInit Start");
    if(ST_BasePipelineInit() != MI_SUCCESS)
    {
        CamOsTsemUp(&tPreloadFileTsem);
        CamOsTsemUp(&tIspReadFileTsem);
    }
    BootTimestampRecord(__LINE__, "ST_BasePipelineInit Done");

    return 0;
}

void *ST_DetectThread(void *p)
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stScaledBufInfo;
    MI_SYS_BUF_HANDLE ScaledHandle;
    MI_U32            u32SclScaledPortId = 1;
    MI_U16            u16SocId = 0;
    MI_RGN_ChnPort_t  stRgnChnPort;
    ST_Stream_Attr_T  *pstStreamAttr = g_stStreamAttr;
    static MI_U8      IPU_Not_People_Count = 0;

    memset(&stChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId   = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = 0;
    stRgnChnPort.s32ChnId  = 0;
    stRgnChnPort.s32PortId = 0;

    Box_t  astTargetBoxes[MAX_DET_OBJECT] = {0};
    MI_S32 s32BoxsNum                     = 0;

    if(ST_DET_Init() != MI_SUCCESS)
    {
        CamOsTsemUp(&tPreloadFileTsem); // det algo flash get file done
        return NULL;
    }

    while (bThreadExit)
    {
        CamOsMsSleep(50);
        // Do Detect
        stChnPort.u32PortId = u32SclScaledPortId;
        s32ThreadRetval = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stScaledBufInfo, &ScaledHandle);
        if (s32ThreadRetval != MI_SUCCESS)
        {
            CamOsMsSleep(10);
            continue;
        }

        ALGO_Input_t stDetInputBufInfo;
        stDetInputBufInfo.buf_size   = stScaledBufInfo.stFrameData.u32BufSize;
        stDetInputBufInfo.p_vir_addr = stScaledBufInfo.stFrameData.pVirAddr[0];
        stDetInputBufInfo.phy_addr   = stScaledBufInfo.stFrameData.phyAddr[0];

        CHECK_DLA_RESULT(ALGO_DET_Run(pDetHandle, &stDetInputBufInfo, astTargetBoxes, &s32BoxsNum), s32ThreadRetval);
        if(s32BoxsNum > 0)
        {
            s32Aov_Get_Status = AOV_IPU_GET_PEOPLE;
            IPU_Not_People_Count = 0;
        }
        else
        {
            IPU_Not_People_Count++;
            if(IPU_Not_People_Count >= 10)
            {
                s32Aov_Get_Status = AOV_POWER_DOWN;//10 frame no people send powerdown
            }
        }

        // Draw Frame
        for (MI_U32 i = 0; i < MAX_FRAME_HANDLE; i++)
        {
            if (i < s32BoxsNum)
            {
                gstRgnChnPortParam.stFrameChnPort.u32Color = 0XFF00;

                MI_S32 x = ((astTargetBoxes[i].x) * 8192) / pstStreamAttr[0].u32Width;
                MI_S32 y = ((astTargetBoxes[i].y) * 8192) / pstStreamAttr[0].u32Height;
                MI_S32 w = ((astTargetBoxes[i].width) * 8192) / pstStreamAttr[0].u32Width;
                MI_S32 h = ((astTargetBoxes[i].height) * 8192) / pstStreamAttr[0].u32Height;

                gstRgnChnPortParam.stFrameChnPort.stRect.s32X      = x;
                gstRgnChnPortParam.stFrameChnPort.stRect.s32Y      = y;
                gstRgnChnPortParam.stFrameChnPort.stRect.u32Width  = w;
                gstRgnChnPortParam.stFrameChnPort.stRect.u32Height = h;

                gstRgnChnPortParam.bShow = TRUE;
            }
            else
            {
                gstRgnChnPortParam.bShow = FALSE;
            }
            CHECK_DLA_RESULT(MI_RGN_SetDisplayAttr(u16SocId, hFrame[i], &stRgnChnPort, &gstRgnChnPortParam), s32ThreadRetval);
        }

        CHECK_DLA_RESULT(MI_SYS_ChnOutputPortPutBuf(ScaledHandle), s32ThreadRetval);
    }
    ALGO_DET_DeinitHandle(pDetHandle);
    ALGO_DET_ReleaseHandle(pDetHandle);
    return NULL;
}


void MI_PreloadTask(void)
{
    CamOsThreadAttrb_t threadPreloadFileAttr = {.nPriority = 10,.szName = "MI_PreloadFile",.nStackSize = 3072};
    CamOsThreadAttrb_t threadPreloadMiPipeAttr = {.nPriority = 99,.szName = "MI_PreloadMiPipe",.nStackSize = 8*1024};

    CamOsTsemInit(&tPreloadFileTsem,0);
    CamOsTsemInit(&tIspReadFileTsem,0);
    CamOsThreadCreate(&PreloadMiPipe_tid, &threadPreloadMiPipeAttr, MI_PreloadMiPipe, NULL);
    CamOsThreadCreate(&PreloadFile_tid, &threadPreloadFileAttr, MI_PreloadFile, NULL);
}

static int RtosAppMainEntry(int argc, char **argv)
{
    MI_DEVICE_Init(NULL);

    MI_PreloadTask();
    return 0;
}

rtos_application_selector_initcall(preload_sample, RtosAppMainEntry);
