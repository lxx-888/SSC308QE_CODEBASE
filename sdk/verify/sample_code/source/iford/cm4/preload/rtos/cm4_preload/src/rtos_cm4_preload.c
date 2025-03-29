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
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "sys_sys_isw_cli.h"
#include "sys_memmap.h"
#include "initcall.h"
#include "drv_dualos.h"
#include "mi_device.h"
#include "mi_sys.h"
#include "mi_vif.h"
#include "mi_scl.h"
#include "mi_venc.h"
#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"
#include "mi_sensor.h"
#include "mi_sensor_impl.h"
#include "sys_sys_boot_timestamp.h"
#include "sys_sys_early_param.h"
#include <mhal_earlyinit_para.h>
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
#include "earlyinit_preload_api.h"
#endif
#include "application_selector.h"


#define MAX_FRAME_HANDLE (32)
#define MI_CLICMD_PRELOAD 0

static MI_BOOL   bThreadExit = 1;

////////////////////////////////////////////////////////////////////////////////
extern int MI_DEVICE_Init(void* pMmaConfig);
extern int mi_debug_init(void);

bool RtosPreloadIsUseReduceMemCfg(void)
{
    return (SysMmapGetLimitDramSize() <= 0x04000000) ? TRUE : FALSE;
}

#define _PATH               "/misc/"
#define _EXT_BIN            ".bin"
#define _ISP_API            "isp_api"

/*=============================================================*/
// Global Variable definition
/*=============================================================*/
MI_U8 g_u8stoppipe = 0;
MI_U8 g_u8deintmoudle = 0;

#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        CamOsPrintf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }

typedef struct _MaxFrameSize {
    MI_U32 frameSizeI;
    MI_U32 frameSizeP;
} MaxFrameSize;

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

typedef struct ST_Stream_Attr_S
{
    MI_U32              u32InputChn;
    MI_U32              u32InputPort;
    MI_VENC_CHN         vencChn;
    MI_VENC_ModType_e   eType;
    MI_U32              u32Mbps;
    MI_U32              u32Width;
    MI_U32              u32Height;
    MI_U32              u32MaxWidth;
    MI_U32              u32MaxHeight;
    MI_U32              u32CropX;
    MI_U32              u32CropY;
    MI_U32              u32CropWidth;
    MI_U32              u32CropHeight;
    MI_SYS_BindType_e   eBindType;
    MI_U32              u32BindPara;
 }ST_Stream_Attr_T;


static ST_Stream_Attr_T g_stStreamAttr[] =
{
    {
        .u32InputChn = 0,
        .u32InputPort = 0,
        .vencChn = 0,
        .eType = E_MI_VENC_MODTYPE_H265E,
        .u32Mbps = 2097152,
        .u32Width = 3840,
        .u32Height = 2160,
        .u32MaxWidth = 3840,
        .u32MaxHeight = 2160,
        .u32CropX = 0,
        .u32CropY = 0,
        .u32CropWidth = 0,
        .u32CropHeight = 0,
        .eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE,
        .u32BindPara = 0,
    },
};

extern CamOsTsem_t tPreloadFileTsem;
extern CamOsTsem_t tIspReadFileTsem;

static MI_S32 STUB_GetVencConfig(MI_VENC_ModType_e eModType, MI_VENC_ChnAttr_t *pstVencChnAttr, MI_U8 u8ChnId)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_U32 u32Profile =0, u32BitRate = 0, u32Gop = 0, u32MaxQp = 0, u32MinQp = 0, u32SrcFrmRateNum = 0, u32MaxBitRate = 0;
    MI_U8 u8VideoH264RcMode = 0, u8VideoH265RcMode = 0;
    if(0 == u8ChnId)
    {
        u8VideoH264RcMode = 0;
        u8VideoH265RcMode = 0;
        u32Profile = 1;
        u32BitRate = g_stStreamAttr[0].u32Height * g_stStreamAttr[0].u32Width * 1.3;
        u32Gop = 30;
        u32MaxQp = 48;
        u32MinQp = 12;
        u32SrcFrmRateNum  = 30;
        u32MaxBitRate = g_stStreamAttr[0].u32Height * g_stStreamAttr[0].u32Width * 1.3;
    }

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
        }
        break;
        default:
            CamOsPrintf("unsupport eModType[%u]\n", eModType);
            return E_MI_ERR_FAILED;
    }
    return MI_SUCCESS;
}
CamOsThread PreloadMiPipe_tid;
CamOsThread PreloadFile_tid;




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

static MI_S32 ST_StartPipeLine(MI_U8 i, MI_U32 u32Width, MI_U32 u32Height, MI_U32 u32CropW, MI_U32 u32CropH, MI_U32 u32CropX, MI_U32 u32CropY)
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
    {
        u32FrameCnt = 3;
    }
    else
    {
        u32FrameCnt = 100;
    }

    MI_ISP_DEV IspDevId = 0;
    MI_SCL_DEV SclDevId = 0;
    MI_ISP_OutPortParam_t  stIspOutputParam;
    MI_SCL_OutPortParam_t  stSclOutputParam;
    MI_VENC_SuperFrameCfg_t stSuperFrameCfg = { E_MI_VENC_SUPERFRM_NONE, 0, 0, 0 };

    memset(&stIspOutputParam, 0x0, (size_t)sizeof(MI_ISP_OutPortParam_t));
    STCHECKRESULT(MI_ISP_GetInputPortCrop(IspDevId, 0, &stIspOutputParam.stCropRect));

    memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
    stSclOutputParam.stSCLOutCropRect.u16X = stIspOutputParam.stCropRect.u16X;
    stSclOutputParam.stSCLOutCropRect.u16Y = stIspOutputParam.stCropRect.u16Y;
    stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;
    stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
    stSclOutputParam.stSCLOutputSize.u16Width = u32Width;
    stSclOutputParam.stSCLOutputSize.u16Height = u32Height;
    stSclOutputParam.bMirror = TRUE;
    stSclOutputParam.bFlip = FALSE;

    if (pstStreamAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
        stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_IFC;
    else
        stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;

    stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, 0, 0, &stSclOutputParam));
    STCHECKRESULT(MI_ISP_SetOutputPortParam(IspDevId, 0, 0, &stIspOutputParam));

    /************************************************
    init VENC
    *************************************************/
    if(pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
        VencDevId = MI_VENC_DEV_ID_JPEG_0;
    else
        VencDevId = MI_VENC_DEV_ID_H264_H265_0;
    memset(&stVencChnAttr, 0x0, (size_t)sizeof(MI_VENC_ChnAttr_t));
    STUB_GetVencConfig(pstStreamAttr[i].eType, &stVencChnAttr, i);
    tVencParam.u32MaxWidth = pstStreamAttr->u32MaxWidth;
    tVencParam.u32MaxHeight = pstStreamAttr->u32MaxHeight;
    MI_VENC_CreateDev(VencDevId, &tVencParam);
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
        if(pstStreamAttr[i].u32BindPara == 0 || pstStreamAttr[i].u32BindPara == pstStreamAttr[i].u32Height)
        {
            pstStreamAttr[i].u32BindPara = pstStreamAttr[i].u32Height;
            stVencSourceCfg.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_ONE_FRM;
        }
        else if(pstStreamAttr[i].u32BindPara == pstStreamAttr[i].u32Height/2)
        {
            stVencSourceCfg.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_HALF_FRM;
        }
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
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = i;
    stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId = VencDevId;
    stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate = 30;
    u32DstFrmrate = 30;
    eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;//E_MI_SYS_BIND_TYPE_REALTIME

    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, 0, 0));
    STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, 0, 0));

    STCHECKRESULT(MI_VENC_StartRecvPic(VencDevId, pstStreamAttr[i].vencChn));
    CamOsPrintf("chn %d startPipeLine:vpeChn[%d],vpePort[%d],vencChn[%d],venc bindtype[%d]\n", i,
            pstStreamAttr[i].u32InputChn,pstStreamAttr[i].u32InputPort,pstStreamAttr[i].vencChn,pstStreamAttr[i].eBindType);

    return MI_SUCCESS;
}

static MI_S32 ST_ExitPipeLine(void)
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
    if(pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
        VencDevId = MI_VENC_DEV_ID_JPEG_0;
    else
        VencDevId = MI_VENC_DEV_ID_H264_H265_0;
    memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId = SclDevId;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = i;
    stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId = VencDevId;
    stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
    stDstChnPort.u32PortId = 0;
    MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
    MI_VENC_StopRecvPic(VencDevId, pstStreamAttr[i].vencChn);
    MI_VENC_DestroyChn(VencDevId, pstStreamAttr[i].vencChn);

    return MI_SUCCESS;
}
#if 0
static MI_VENC_ModType_e ST_GetVideoModType(MI_U8 u8VideoFormat)
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
#endif

static void ST_DeinitMoudle(void)
{
    MI_ISP_DEV IspDevId = 0;
    MI_SCL_DEV SclDevId = 0;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId = IspDevId;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId = SclDevId;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;
    MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);

    /************************************************
    Destroy SCL
    *************************************************/
    MI_SCL_DisableOutputPort(SclDevId, 0, 0);
    MI_SCL_StopChannel(SclDevId, 0);
    MI_SCL_DestroyChannel(SclDevId, 0);
    MI_SCL_DestroyDevice(SclDevId);

    memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId = IspDevId;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;
    MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);

    /************************************************
    Destroy ISP
    *************************************************/
    MI_ISP_StopChannel(IspDevId, 0);
    MI_ISP_DestroyChannel(IspDevId, 0);
    MI_ISP_DestoryDevice(IspDevId);
    /************************************************
    Destroy VIF
    *************************************************/
    MI_VIF_DisableOutputPort(0, 0);
    MI_VIF_DisableDev(0);

    MI_VENC_DestroyDev(MI_VENC_DEV_ID_H264_H265_0);
}

static void _Exit_PreloadForce(void)
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

static int _MI_Cli_GetDataL2R(void *data)
{
    char *string = (char *)data;
    int len = 0;
    len = strlen((const char *)string);
    if(len > 0)
    {
        printf("recv from linux data: %s\n", string);
        if(strncmp(data, "quit", 4) == 0)
        {
            // bThreadExit = 0;
            _Exit_PreloadForce();
        }
    }
    else
    {
        printf("recv from linux data is null\n");
    }

    return 0;
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

    MasterEarlyInitParam_t *pstEarlyInitParam;
    MI_U32 u32SensorNum = 1;
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
    MI_U32 u32snr_fps = 0;
    char stPatch[128] = {0};
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
    unsigned char earlyinit_enable = 0;
    const EarlyInitPreloadCfg_t *pearlyinit_cfg = NULL;
#endif

    // MI_DEVICE_RegMiSysExitCall(_Exit_PreloadForce);
    MI_DEVICE_RegMiCliGetDataL2R(MI_CLICMD_PRELOAD, _MI_Cli_GetDataL2R);
    u32SensorNum = 1;

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
    pearlyinit_cfg = DrvEarlyInitGetPreloadCfg();
    for(i = 0; i < pearlyinit_cfg->u32NumSnr; ++i)
    {
        earlyinit_enable |= DrvEarlyInitForPreloadIsEnabled(i);
    }

    u32SensorNum = (earlyinit_enable) ? pearlyinit_cfg->u32NumSnr : u32SensorNum;
    CamOsPrintf("[earlyinit] en:%u, snr num:%u.\n", earlyinit_enable, u32SensorNum);
#endif

    for(i=0;i<u32SensorNum;++i)
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

        u8SnrPad = i;
        IspChnId = i;
        SclChnId = i;

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            u8SnrPad = pearlyinit_cfg->ChCfg[i].u8SnrPad;
            bMirror = pearlyinit_cfg->ChCfg[i].bMirror;
            bFlip = pearlyinit_cfg->ChCfg[i].bFlip;

            if(bMirror == FALSE)
            {
                if(bFlip == FALSE)
                {
                    eSetType = E_MI_SYS_ROTATE_NONE;
                }
                else
                {
                    eSetType = E_MI_SYS_ROTATE_90;
                }
            }
            else
            {
                if(bFlip == FALSE)
                {
                    eSetType = E_MI_SYS_ROTATE_270;
                }
                else
                {
                    eSetType = E_MI_SYS_ROTATE_180;
                }
            }
        }
#endif

        /************************************************
        Step2:  init VIF(for IPC, only one dev)
        *************************************************/
        MI_SNR_SetPlaneMode(u8SnrPad,0);
        MI_SNR_QueryResCount(u8SnrPad, &u32GetSnrResCount);
        u32SnrResCount = u32GetSnrResCount;

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            if(pearlyinit_cfg->ChCfg[i].u8ResIdx >= u32SnrResCount)
            {
                CamOsPrintf("[earlyinit] snr res idx %d,%d err\n", pearlyinit_cfg->ChCfg[i].u8ResIdx, u32SnrResCount);
                continue;
            }

            tIndex = pearlyinit_cfg->ChCfg[i].u8ResIdx;
            u32snr_fps = pearlyinit_cfg->ChCfg[i].u32SensorFrameRate / 1000;
        }
#endif

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
        MI_SNR_Enable(u8SnrPad);
        /************************************************
        Step2:  init VIF
        *************************************************/
        u32VifDevId = i;
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
        STCHECKRESULT(MI_VIF_EnableOutputPort(u32VifDevId, u32VifPortId));

        /************************************************
        Step3:  Init Isp
        *************************************************/
        memset(&stIspDevAttr, 0x0, (size_t)sizeof(MI_ISP_DevAttr_t));
        memset(&stIspChnAttr, 0x0, (size_t)sizeof(MI_ISP_ChannelAttr_t));
        memset(&stIspChnParam, 0x0, (size_t)sizeof(MI_ISP_ChnParam_t));

        stIspDevAttr.u32DevStitchMask = E_MI_ISP_DEVICEMASK_ID0;
        if(u32SensorNum > 1)
        {
            stIspDevAttr.u32DevStitchMask |= E_MI_ISP_DEVICEMASK_ID1;
        }
        STCHECKRESULT(MI_ISP_CreateDevice(IspDevId, &stIspDevAttr));

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
        pstEarlyInitParam->u16SnrEarlyFlicker = 1;
        pstEarlyInitParam->u32SnrEarlyShutter = 24978;
        pstEarlyInitParam->u32SnrEarlyGainX1024 = 1280;
        pstEarlyInitParam->u32SnrEarlyDGain = 1024;
        pstEarlyInitParam->u16SnrEarlyAwbRGain = 1405;
        pstEarlyInitParam->u16SnrEarlyAwbGGain = 1024;
        pstEarlyInitParam->u16SnrEarlyAwbBGain = 2317;
        stIspChnAttr.stIspCustIqParam.stVersion.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
        stIspChnAttr.stIspCustIqParam.stVersion.u32Size = sizeof(MasterEarlyInitParam_t);

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            pstEarlyInitParam->u16SnrEarlyFlicker = pearlyinit_cfg->ChCfg[i].u16SnrEarlyFlicker;
            pstEarlyInitParam->u32SnrEarlyShutter = pearlyinit_cfg->ChCfg[i].u32ShutterLEF;
            pstEarlyInitParam->u32SnrEarlyGainX1024 = pearlyinit_cfg->ChCfg[i].u32SensorGainLEFx1024; //long frame, gain x 1024
        }
#endif

        STCHECKRESULT(MI_ISP_CreateChannel(IspDevId, IspChnId, &stIspChnAttr));
        STCHECKRESULT(MI_ISP_SetChnParam(IspDevId, IspChnId, &stIspChnParam));
        STCHECKRESULT(MI_ISP_StartChannel(IspDevId, IspChnId));

        sprintf(&stPatch[0], "%s%s%u%s", _PATH, _ISP_API, i, _EXT_BIN);

        MI_ISP_ApiCmdLoadBinFile(IspDevId, IspChnId, &stPatch[0], 1234);
        CamOsTsemUp(&tIspReadFileTsem);

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
        eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;//E_MI_SYS_BIND_TYPE_REALTIME
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, 0));

       /************************************************
        Step3:  init SCL
        *************************************************/
        memset(&stSclDevAttr, 0x0, (size_t)sizeof(MI_SCL_DevAttr_t));
        memset(&stSclChnAttr, 0x0, (size_t)sizeof(MI_SCL_ChannelAttr_t));
        memset(&stSclChnParam, 0x0, (size_t)sizeof(MI_SCL_ChnParam_t));

        stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1 | E_MI_SCL_HWSCL2;// | E_MI_SCL_HWSCL3 | E_MI_SCL_HWSCL4 | E_MI_SCL_HWSCL5;
        STCHECKRESULT(MI_SCL_CreateDevice(SclDevId, &stSclDevAttr));
        STCHECKRESULT(MI_SCL_CreateChannel(SclDevId, SclChnId, &stSclChnAttr));
        STCHECKRESULT(MI_SCL_SetChnParam(SclDevId, SclChnId, &stSclChnParam));
        STCHECKRESULT(MI_SCL_StartChannel(SclDevId, SclChnId));

        /************************************************
        Step5:  Bind ISP->SCL
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
    }

    ST_StartPipeLine(0, pstStreamAttr[0].u32Width, pstStreamAttr[0].u32Height, pstStreamAttr[0].u32CropWidth, pstStreamAttr[0].u32CropHeight, pstStreamAttr[0].u32CropX, pstStreamAttr[0].u32CropY);

    return MI_SUCCESS;
}



static MI_S32 ST_BasePipelineInit(void)
{
    STCHECKRESULT(STUB_BaseModuleInit());
    return 0;
}

CamOsTsem_t tPreloadFileTsem;
CamOsTsem_t tIspReadFileTsem;

static void* MI_PreloadFile(void* p)
{
    BootTimestampRecord(__LINE__, "PreloadFile Start");

    CamOsTsemUp(&tPreloadFileTsem);
    CamOsTsemDown(&tIspReadFileTsem);//wait Isp Load api bin
    if (application_selector_retreat() != 0)
    {
        CamOsPrintf("application selector stop failed.\n");
    }
    BootTimestampRecord(__LINE__, "PreloadFile end");
    return 0;
}

static void* MI_PreloadMiPipe(void* p)
{
    CamOsTimespec_t stSystemTimetemp;
    MI_U64 u64PtsBase;

    MI_SYS_Init(0);
    mi_debug_init();
    CamOsGetTimeOfDay(&stSystemTimetemp);
    u64PtsBase = stSystemTimetemp.nSec*1000000ULL+stSystemTimetemp.nNanoSec/1000;
    MI_SYS_InitPtsBase(0, u64PtsBase);

    BootTimestampRecord(__LINE__, "ST_BasePipelineInit Start");
    ST_BasePipelineInit();
    BootTimestampRecord(__LINE__, "ST_BasePipelineInit Done");

    CamOsTsemDown(&tPreloadFileTsem);

    return 0;
}

static void MI_PreloadTask(void)
{
    CamOsThreadAttrb_t threadPreloadFileAttr = {.nPriority = 10,.szName = "MI_PreloadFile",.nStackSize = 3072};
    CamOsThreadAttrb_t threadPreloadMiPipeAttr = {.nPriority = 99,.szName = "MI_PreloadMiPipe",.nStackSize = 6*1024};

    CamOsTsemInit(&tPreloadFileTsem,0);
    CamOsTsemInit(&tIspReadFileTsem,0);
    CamOsThreadCreate(&PreloadMiPipe_tid, &threadPreloadMiPipeAttr, MI_PreloadMiPipe, NULL);
    CamOsThreadCreate(&PreloadFile_tid, &threadPreloadFileAttr, MI_PreloadFile, NULL);
}

static int RtosAppMainEntry(int argc, char **argv)
{
    MI_DEVICE_Init(NULL);// mount misc partion

    MI_PreloadTask();
    return 0;
}
rtos_application_selector_initcall(cm4_preload, RtosAppMainEntry);

