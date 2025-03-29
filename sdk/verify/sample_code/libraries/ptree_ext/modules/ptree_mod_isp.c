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

#include "mi_common_datatype.h"
#include "mi_isp_datatype.h"
#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"
#include "mi_sys_datatype.h"
#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_packet_raw.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_isp.h"
#include "ptree_sur_isp.h"
#include "ptree_sur_sys.h"
#include "ptree_maker.h"

#ifdef CAM_OS_RTK
#include <mhal_earlyinit_para.h>
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
#include "earlyinit_preload_api.h"
#include "ptree_mod_snr_datatype.h"
#endif

#endif

#define PTREE_MOD_ISP_DEV_NUM (2)

PTREE_ENUM_DEFINE(MI_ISP_Overlap_e,              // MI_ISP_Overlap_e
                  {E_MI_ISP_OVERLAP_128, "128"}, // 128
                  {E_MI_ISP_OVERLAP_256, "256"}, // 256
)

typedef struct PTREE_MOD_ISP_Obj_s    PTREE_MOD_ISP_Obj_t;
typedef struct PTREE_MOD_ISP_InObj_s  PTREE_MOD_ISP_InObj_t;
typedef struct PTREE_MOD_ISP_OutObj_s PTREE_MOD_ISP_OutObj_t;

struct PTREE_MOD_ISP_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};
struct PTREE_MOD_ISP_InObj_s
{
    PTREE_MOD_SYS_InObj_t base;
};
struct PTREE_MOD_ISP_OutObj_s
{
    PTREE_MOD_SYS_OutObj_t base;
};

static int                 _PTREE_MOD_ISP_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_ISP_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_ISP_Start(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_ISP_Stop(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_ISP_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_ISP_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_ISP_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static int                _PTREE_MOD_ISP_InStart(PTREE_MOD_SYS_InObj_t *sysModIn);
static void               _PTREE_MOD_ISP_InFree(PTREE_MOD_SYS_InObj_t *sysModIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_ISP_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static int  _PTREE_MOD_ISP_OutStart(PTREE_MOD_SYS_OutObj_t *sysModOut);
static int  _PTREE_MOD_ISP_OutStop(PTREE_MOD_SYS_OutObj_t *sysModOut);
static void _PTREE_MOD_ISP_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo);
static void _PTREE_MOD_ISP_OutFree(PTREE_MOD_SYS_OutObj_t *sysModOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_ISP_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_ISP_SYS_OPS = {
    .init         = _PTREE_MOD_ISP_Init,
    .deinit       = _PTREE_MOD_ISP_Deinit,
    .start        = _PTREE_MOD_ISP_Start,
    .stop         = _PTREE_MOD_ISP_Stop,
    .createModIn  = _PTREE_MOD_ISP_CreateModIn,
    .createModOut = _PTREE_MOD_ISP_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_ISP_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_ISP_Free,
};

static const PTREE_MOD_SYS_InOps_t G_PTREE_MOD_ISP_SYS_IN_OPS = {
    .start = _PTREE_MOD_ISP_InStart,
};
static const PTREE_MOD_SYS_InHook_t G_PTREE_MOD_ISP_SYS_IN_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_ISP_InFree,
};

static const PTREE_MOD_SYS_OutOps_t G_PTREE_MOD_ISP_SYS_OUT_OPS = {
    .start         = _PTREE_MOD_ISP_OutStart,
    .stop          = _PTREE_MOD_ISP_OutStop,
    .getPacketInfo = _PTREE_MOD_ISP_OutGetPacketInfo,
};
static const PTREE_MOD_SYS_OutHook_t G_PTREE_MOD_ISP_SYS_OUT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_ISP_OutFree,
};

static unsigned int g_ispCreateDev[PTREE_MOD_ISP_DEV_NUM] = {0};

static int _PTREE_MOD_ISP_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_ISP_Info_t *ispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_ISP_Info_t, base.base);
    MI_ISP_ChannelAttr_t  ispChnAttr;
    MI_ISP_ChnParam_t     ispChnParam;
    MI_ISP_LdcAttr_t      ispLdcAttr;
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
    PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *snrDrvInfo        = NULL;
    MI_U8                                   u8EarlyinitEnable = 0;
    const EarlyInitPreloadCfg_t *           earlyinitCfg      = NULL;
    int                                     EarlyinitId       = 0;
    MI_U8                                   snrPad            = 0;
    int                                     i                 = 0;
#define MI_SENSOR_MAX_SENSORID 10
#endif

    if (!g_ispCreateDev[ispInfo->base.base.devId])
    {
        MI_ISP_DevAttr_t ispDevAttr;
        memset(&ispDevAttr, 0, sizeof(MI_ISP_DevAttr_t));
        if (MI_SUCCESS != MI_ISP_CreateDevice(ispInfo->base.base.devId, &ispDevAttr))
        {
            PTREE_ERR("MI_ISP_CreateDevice failed");
            return SSOS_DEF_FAIL;
        }
    }
    ++g_ispCreateDev[ispInfo->base.base.devId];

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
    earlyinitCfg = DrvEarlyInitGetPreloadCfg();
    if (earlyinitCfg != NULL)
    {
        for (i = 0; i < MI_SENSOR_MAX_SENSORID; i++)
        {
            if (ispInfo->u32SnrMask & (1 << i))
            {
                snrPad = i;
                PTREE_DBG("from snrmask %d,get sndpad %d", ispInfo->u32SnrMask, snrPad);
                break;
            }
        }

        for (EarlyinitId = 0; EarlyinitId < earlyinitCfg->u32NumSnr; EarlyinitId++)
        {
            if (earlyinitCfg->ChCfg[EarlyinitId].u8SnrPad == snrPad)
            {
                u8EarlyinitEnable = DrvEarlyInitForPreloadIsEnabled(EarlyinitId);
                break;
            }
        }
    }
#endif

    memset(&ispChnAttr, 0, sizeof(MI_ISP_ChannelAttr_t));
    ispChnAttr.u32Sync3AType   = ispInfo->u32Sync3aType;
    ispChnAttr.u32SensorBindId = ispInfo->u32SnrMask;

#ifdef CAM_OS_RTK
    if (ispInfo->u8CustIqEn)
    {
        MasterEarlyInitParam_t *earlyInitParam;
        earlyInitParam                     = (MasterEarlyInitParam_t *)&ispChnAttr.stIspCustIqParam.stVersion.u8Data[0];
        earlyInitParam->u16SnrEarlyFps     = ispInfo->stEarlyInitParam.u16SnrEarlyFps;
        earlyInitParam->u16SnrEarlyFlicker = ispInfo->stEarlyInitParam.u16SnrEarlyFlicker;
        earlyInitParam->u32SnrEarlyShutter = ispInfo->stEarlyInitParam.u32SnrEarlyShutter;
        earlyInitParam->u32SnrEarlyGainX1024              = ispInfo->stEarlyInitParam.u32SnrEarlyGainX1024;
        earlyInitParam->u32SnrEarlyDGain                  = ispInfo->stEarlyInitParam.u32SnrEarlyDGain;
        earlyInitParam->u16SnrEarlyAwbRGain               = ispInfo->stEarlyInitParam.u16SnrEarlyAwbRGain;
        earlyInitParam->u16SnrEarlyAwbGGain               = ispInfo->stEarlyInitParam.u16SnrEarlyAwbGGain;
        earlyInitParam->u16SnrEarlyAwbBGain               = ispInfo->stEarlyInitParam.u16SnrEarlyAwbBGain;
        ispChnAttr.stIspCustIqParam.stVersion.u32Revision = ispInfo->stEarlyInitParam.u32Revision;
        ispChnAttr.stIspCustIqParam.stVersion.u32Size     = sizeof(MasterEarlyInitParam_t);

        PTREE_DBG("stEarlyInitParam u32Revision %d, Size = %d", ispChnAttr.stIspCustIqParam.stVersion.u32Revision,
                  ispChnAttr.stIspCustIqParam.stVersion.u32Size);

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
        if (u8EarlyinitEnable)
        {
            earlyInitParam->u16SnrEarlyFlicker   = earlyinitCfg->ChCfg[EarlyinitId].u16SnrEarlyFlicker;
            earlyInitParam->u32SnrEarlyShutter   = earlyinitCfg->ChCfg[EarlyinitId].u32ShutterLEF;
            earlyInitParam->u32SnrEarlyGainX1024 = earlyinitCfg->ChCfg[EarlyinitId].u32SensorGainLEFx1024;
            PTREE_DBG("[earlyinit] snr pad:%u, shutter %d, gain %d", snrPad, earlyInitParam->u32SnrEarlyShutter,
                      earlyInitParam->u32SnrEarlyGainX1024);
        }
#endif
    }
#endif

    if (ispInfo->u8MutichnEn)
    {
        if (MI_SUCCESS
            != MI_ISP_CreateChannel((MI_ISP_DEV)ispInfo->base.base.devId,
                                    MI_ISP_MULTI_CHN_MASK | (MI_ISP_CHANNEL)ispInfo->base.base.chnId, &ispChnAttr))
        {
            PTREE_ERR("MI_ISP_CreateChannel error");
            return SSOS_DEF_FAIL;
        }
    }
    else
    {
        if (MI_SUCCESS
            != MI_ISP_CreateChannel((MI_ISP_DEV)ispInfo->base.base.devId, (MI_ISP_CHANNEL)ispInfo->base.base.chnId,
                                    &ispChnAttr))
        {
            PTREE_ERR("MI_ISP_CreateChannel error");
            return SSOS_DEF_FAIL;
        }
    }

    MI_ISP_ApiCmdLoadBinFile((MI_ISP_DEV)ispInfo->base.base.devId, (MI_ISP_CHANNEL)ispInfo->base.base.chnId,
                             &ispInfo->apiBinpath[0], 1234);

    memset(&ispChnParam, 0, sizeof(MI_ISP_ChnParam_t));

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
    snrDrvInfo = PTREE_MOD_SNR_DATATYPE_GetSensorInfo(snrPad);
    if (!snrDrvInfo)
    {
        PTREE_ERR("pstSrcSnrDrvInfo is NULL, intSensorId:%d may not fpInit", snrPad);
        return SSOS_DEF_FAIL;
    }
    ispChnParam.eHDRType = (snrDrvInfo->bHdrEnable == 0)                               ? E_MI_ISP_HDR_TYPE_OFF
                           : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_OFF) ? E_MI_ISP_HDR_TYPE_OFF
                           : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_VC)
                               ? E_MI_ISP_HDR_TYPE_VC
                               : // virtual channel mode HDR,vc0->long, vc1->short
                               (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_DOL) ? E_MI_ISP_HDR_TYPE_DOL
                           : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_COMP)  ? E_MI_ISP_HDR_TYPE_COMP
                                                                                         : // compressed HDR mode
                               (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_LI) ? E_MI_ISP_HDR_TYPE_LI
                                                                                         : // Line interlace HDR
                               (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_COMPVS) ? E_MI_ISP_HDR_TYPE_COMPVS
                            : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_DCG)      ? E_MI_ISP_HDR_TYPE_DCG
                                                                                        : // Dual conversion gain HDR
                               E_MI_ISP_HDR_TYPE_OFF;

    ispChnParam.eHDRFusionType     = (snrDrvInfo->bHdrEnable == 0)
                                         ? E_MI_ISP_HDR_FUSION_TYPE_NONE
                                         : (MI_ISP_HDRFusionType_e)snrDrvInfo->stPadInfo.eHDRFusionType;
    ispChnParam.u16HDRExposureMask = (snrDrvInfo->bHdrEnable == 0)
                                         ? E_MI_ISP_HDR_EXPOSURE_TYPE_NONE
                                         : (MI_ISP_HDRExposureType_e)ispInfo->u32HdrExposureMask;
    PTREE_DBG("[earlyinit] snrPad %d HdrEnable %d, Type %d, fusiontype %d, HDRExposureMask %d", snrPad,
              snrDrvInfo->bHdrEnable, ispChnParam.eHDRType, ispChnParam.eHDRFusionType, ispChnParam.u16HDRExposureMask);
#else
    ispChnParam.eHDRType           = (MI_ISP_HDRType_e)ispInfo->u32HdrType;
    ispChnParam.eHDRFusionType     = (MI_ISP_HDRFusionType_e)ispInfo->u32HdrFusionType;
    ispChnParam.u16HDRExposureMask = (MI_ISP_HDRExposureType_e)ispInfo->u32HdrExposureMask;
#endif
    ispChnParam.e3DNRLevel = (MI_ISP_3DNR_Level_e)ispInfo->u32level3dnr;
    ispChnParam.bMirror    = (MI_BOOL)ispInfo->u8Mirror;
    ispChnParam.bFlip      = (MI_BOOL)ispInfo->u8Flip;
    ispChnParam.eRot       = (MI_SYS_Rotate_e)ispInfo->u32Rotation;
    ispChnParam.bLdcEnable = ispInfo->u8IspLdcEn ? TRUE : FALSE;
    if (MI_SUCCESS
        != MI_ISP_SetChnParam((MI_ISP_DEV)ispInfo->base.base.devId, (MI_ISP_CHANNEL)ispInfo->base.base.chnId,
                              &ispChnParam))
    {
        PTREE_ERR("MI_ISP_SetChnParam error");
        return SSOS_DEF_FAIL;
    }

    if (ispInfo->u8IspOverlapEn)
    {
        MI_ISP_Overlap_e eOverlap;
        eOverlap = PTREE_ENUM_FROM_STR(MI_ISP_Overlap_e, ispInfo->stOverLapParam.chOverlap);
        MI_ISP_SetChnOverlapAttr((MI_ISP_DEV)ispInfo->base.base.devId, (MI_ISP_CHANNEL)ispInfo->base.base.chnId,
                                 eOverlap);
    }

    if (ispInfo->u8IspLdcEn)
    {
        ispLdcAttr.u32CenterXOffset = ispInfo->stIspLdcParam.centerX;
        ispLdcAttr.u32CenterYOffset = ispInfo->stIspLdcParam.centerY;
        ispLdcAttr.s32Alpha         = ispInfo->stIspLdcParam.alpha;
        ispLdcAttr.s32Beta          = ispInfo->stIspLdcParam.beta;
        ispLdcAttr.u32CropLeft      = ispInfo->stIspLdcParam.cropL;
        ispLdcAttr.u32CropRight     = ispInfo->stIspLdcParam.cropR;
        MI_ISP_SetLdcAttr((MI_ISP_DEV)ispInfo->base.base.devId, (MI_ISP_CHANNEL)ispInfo->base.base.chnId, &ispLdcAttr);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_ISP_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_ISP_Info_t *ispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_ISP_Info_t, base.base);

    MI_ISP_DestroyChannel((MI_ISP_DEV)ispInfo->base.base.devId, (MI_ISP_CHANNEL)ispInfo->base.base.chnId);

    --g_ispCreateDev[ispInfo->base.base.devId];
    if (!g_ispCreateDev[ispInfo->base.base.devId])
    {
        MI_ISP_DestoryDevice(ispInfo->base.base.devId);
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_ISP_Start(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_ISP_Info_t *ispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_ISP_Info_t, base.base);

    if (MI_SUCCESS != MI_ISP_StartChannel(ispInfo->base.base.devId, ispInfo->base.base.chnId))
    {
        PTREE_ERR("MI_ISP_StartChannel failed");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_ISP_Stop(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_ISP_Info_t *ispInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_ISP_Info_t, base.base);

    MI_ISP_StopChannel(ispInfo->base.base.devId, ispInfo->base.base.chnId);

    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_ISP_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    (void)loopId;
    return _PTREE_MOD_ISP_InNew(sysMod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_ISP_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    (void)loopId;
    return _PTREE_MOD_ISP_OutNew(sysMod, loopId);
}
static void _PTREE_MOD_ISP_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_ISP_Obj_t, base));
}

static int _PTREE_MOD_ISP_InStart(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    PTREE_SUR_ISP_InInfo_t *ispInInfo = CONTAINER_OF(sysModIn->base.info, PTREE_SUR_ISP_InInfo_t, base.base);
    PTREE_SUR_SYS_InInfo_t *sysInInfo = &ispInInfo->base;
    PTREE_SUR_Info_t *      info      = sysModIn->base.thisMod->info;
    MI_SYS_WindowRect_t     cropRect;

    if (sysInInfo->bindType != E_MI_SYS_BIND_TYPE_FRAME_BASE)
    {
        return SSOS_DEF_OK;
    }

    if (!ispInInfo->u16CropW || !ispInInfo->u16CropH)
    {
        return SSOS_DEF_OK;
    }

    memset(&cropRect, 0, sizeof(MI_SYS_WindowRect_t));
    cropRect.u16X      = ispInInfo->u16CropX;
    cropRect.u16Y      = ispInInfo->u16CropY;
    cropRect.u16Width  = ispInInfo->u16CropW;
    cropRect.u16Height = ispInInfo->u16CropH;
    if (MI_SUCCESS != MI_ISP_SetInputPortCrop(info->devId, info->chnId, &cropRect))
    {
        PTREE_ERR("MI_ISP_SetInputPortCrop failed");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static void _PTREE_MOD_ISP_InFree(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModIn, PTREE_MOD_ISP_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_ISP_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_ISP_InObj_t *ispModIn = NULL;

    ispModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_ISP_InObj_t));
    if (!ispModIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(ispModIn, 0, sizeof(PTREE_MOD_ISP_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_InObjInit(&ispModIn->base, &G_PTREE_MOD_ISP_SYS_IN_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(ispModIn);
        return NULL;
    }
    PTREE_MOD_SYS_InObjRegister(&ispModIn->base, &G_PTREE_MOD_ISP_SYS_IN_HOOK);
    return &ispModIn->base.base;
}

static int _PTREE_MOD_ISP_OutStart(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    PTREE_SUR_ISP_OutInfo_t *ispOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_ISP_OutInfo_t, base.base);
    PTREE_SUR_Info_t *       info       = sysModOut->base.thisMod->info;

    MI_ISP_OutPortParam_t ispOutputParam;

    memset(&ispOutputParam, 0, sizeof(MI_ISP_OutPortParam_t));
    ispOutputParam.stCropRect.u16X      = (MI_U16)ispOutInfo->u16CropX;
    ispOutputParam.stCropRect.u16Y      = (MI_U16)ispOutInfo->u16CropY;
    ispOutputParam.stCropRect.u16Width  = (MI_U16)ispOutInfo->u16CropW;
    ispOutputParam.stCropRect.u16Height = (MI_U16)ispOutInfo->u16CropH;
    ispOutputParam.ePixelFormat         = (MI_SYS_PixelFormat_e)ispOutInfo->u32VideoFormat;
    ispOutputParam.eCompressMode        = (MI_SYS_CompressMode_e)ispOutInfo->u32CompressMode;
    /* Fixme: The current bw tool doesn't support this yet!Keep the default value 0 */
    // isp_output_param.eBufLayout
    if (MI_SUCCESS
        != MI_ISP_SetOutputPortParam((MI_ISP_DEV)info->devId, (MI_ISP_CHANNEL)info->chnId,
                                     (MI_ISP_PORT)ispOutInfo->base.base.port, &ispOutputParam))
    {
        PTREE_ERR("MI_ISP_SetOutputPortParam error");
        return SSOS_DEF_FAIL;
    }

    if (MI_SUCCESS
        != MI_ISP_EnableOutputPort((MI_ISP_DEV)info->devId, (MI_ISP_CHANNEL)info->chnId,
                                   (MI_ISP_PORT)ispOutInfo->base.base.port))
    {
        PTREE_ERR("MI_ISP_EnableOutputPort error");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_ISP_OutStop(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    PTREE_SUR_ISP_OutInfo_t *ispOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_ISP_OutInfo_t, base.base);
    PTREE_SUR_Info_t *       info       = sysModOut->base.thisMod->info;

    MI_ISP_DisableOutputPort(info->devId, info->chnId, ispOutInfo->base.base.port);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_ISP_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    PTREE_SUR_ISP_OutInfo_t *ispOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_ISP_OutInfo_t, base.base);
    rawInfo->fmt                        = PTREE_MOD_SYS_SysFmtToPtreeFmt(ispOutInfo->u32VideoFormat);
    rawInfo->width                      = ispOutInfo->u16CropW;
    rawInfo->height                     = ispOutInfo->u16CropH;
}
static void _PTREE_MOD_ISP_OutFree(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModOut, PTREE_MOD_ISP_OutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_ISP_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_ISP_OutObj_t *ispModOut = NULL;

    ispModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_ISP_OutObj_t));
    if (!ispModOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(ispModOut, 0, sizeof(PTREE_MOD_ISP_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_OutObjInit(&ispModOut->base, &G_PTREE_MOD_ISP_SYS_OUT_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(ispModOut);
        return NULL;
    }
    PTREE_MOD_SYS_OutObjRegister(&ispModOut->base, &G_PTREE_MOD_ISP_SYS_OUT_HOOK);
    return &ispModOut->base.base;
}

PTREE_MOD_Obj_t *PTREE_MOD_ISP_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_ISP_Obj_t *ispMod = NULL;

    ispMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_ISP_Obj_t));
    if (!ispMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(ispMod, 0, sizeof(PTREE_MOD_ISP_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&ispMod->base, &G_PTREE_MOD_ISP_SYS_OPS, tag, E_MI_MODULE_ID_ISP))
    {
        goto ERR_MOD_SYS_INIT;
    }
    if (ispMod->base.base.info->devId >= PTREE_MOD_ISP_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", ispMod->base.base.info->devId, PTREE_MOD_ISP_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    PTREE_MOD_SYS_ObjRegister(&ispMod->base, &G_PTREE_MOD_ISP_SYS_HOOK);
    return &ispMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&ispMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(ispMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(ISP, PTREE_MOD_ISP_New);
