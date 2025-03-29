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

#include "mi_common.h"
#include "mi_vif.h"
#include "mi_vif_datatype.h"
#include "parena.h"
#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_maker.h"
#include "ptree_mod.h"
#include "ptree_sur_sys.h"
#include "ptree_mod_sys.h"
#include "ptree_log.h"
#include "ssos_list.h"
#include "ptree_sur_vif.h"
#include "ptree_mod_snr_datatype.h"

#define PTREE_MOD_VIF_GROUP_NUM  (4)
#define PTREE_MOD_VIF_GROUP_SIZE (4)
#define PTREE_MOD_VIF_DEV_NUM    (PTREE_MOD_VIF_GROUP_NUM * PTREE_MOD_VIF_GROUP_SIZE)

typedef struct PTREE_MOD_VIF_Obj_s    PTREE_MOD_VIF_Obj_t;
typedef struct PTREE_MOD_VIF_OutObj_s PTREE_MOD_VIF_OutObj_t;

struct PTREE_MOD_VIF_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};
struct PTREE_MOD_VIF_OutObj_s
{
    PTREE_MOD_SYS_OutObj_t base;
};

static int                 _PTREE_MOD_VIF_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_VIF_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_VIF_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_VIF_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_VIF_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static int  _PTREE_MOD_VIF_OutStart(PTREE_MOD_SYS_OutObj_t *modOut);
static int  _PTREE_MOD_VIF_OutStop(PTREE_MOD_SYS_OutObj_t *modOut);
static void _PTREE_MOD_VIF_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *modOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo);
static void _PTREE_MOD_VIF_OutFree(PTREE_MOD_SYS_OutObj_t *modOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_VIF_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_VIF_SYS_OPS = {
    .init         = _PTREE_MOD_VIF_Init,
    .deinit       = _PTREE_MOD_VIF_Deinit,
    .createModIn  = _PTREE_MOD_VIF_CreateModIn,
    .createModOut = _PTREE_MOD_VIF_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_VIF_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_VIF_Free,
};

static const PTREE_MOD_SYS_OutOps_t G_PTREE_MOD_VIF_SYS_OUT_OPS = {
    .start         = _PTREE_MOD_VIF_OutStart,
    .stop          = _PTREE_MOD_VIF_OutStop,
    .getPacketInfo = _PTREE_MOD_VIF_OutGetPacketInfo,
};
static const PTREE_MOD_SYS_OutHook_t G_PTREE_MOD_VIF_SYS_OUT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_VIF_OutFree,
};

static int g_vifCreateDev[PTREE_MOD_VIF_DEV_NUM]     = {0};
static int g_vifGroupEnable[PTREE_MOD_VIF_GROUP_NUM] = {0};

static int _PTREE_MOD_VIF_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_VIF_Info_t *                  vifInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_VIF_Info_t, base.base);
    PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *snrDrvInfo = NULL;
    unsigned int                            groupId    = 0;

    snrDrvInfo = PTREE_MOD_SNR_DATATYPE_GetSensorInfo(vifInfo->s32SensorId);
    if (!snrDrvInfo)
    {
        PTREE_ERR("pstSrcSnrDrvInfo is NULL, intSensorId:%d may not fpInit", vifInfo->s32SensorId);
        return SSOS_DEF_FAIL;
    }

    groupId = vifInfo->base.base.devId / 4;

    if (!g_vifGroupEnable[groupId])
    {
        MI_VIF_GroupAttr_t groupAttr;
        memset(&groupAttr, 0, sizeof(MI_VIF_GroupAttr_t));
        groupAttr.eIntfMode = (MI_VIF_IntfMode_e)snrDrvInfo->stPadInfo.eIntfMode;
        groupAttr.eWorkMode = (MI_VIF_WorkMode_e)vifInfo->s32WorkMode;
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
        groupAttr.eHDRType = (snrDrvInfo->bHdrEnable == 0)                               ? E_MI_VIF_HDR_TYPE_OFF
                             : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_OFF) ? E_MI_VIF_HDR_TYPE_OFF
                             : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_VC)
                                 ? E_MI_VIF_HDR_TYPE_VC
                                 : // virtual channel mode HDR,vc0->long, vc1->short
                                 (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_DOL) ? E_MI_VIF_HDR_TYPE_DOL
                             : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_COMP)  ? E_MI_VIF_HDR_TYPE_COMP
                                                                                           : // compressed HDR mode
                                 (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_LI) ? E_MI_VIF_HDR_TYPE_LI
                                                                                           : // Line interlace HDR
                                 (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_COMPVS) ? E_MI_VIF_HDR_TYPE_COMPVS
                              : (snrDrvInfo->stPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_DCG)      ? E_MI_VIF_HDR_TYPE_DCG
                                                                                          : // Dual conversion gain HDR
                                 E_MI_VIF_HDR_TYPE_OFF;
        PTREE_DBG("[earlyinit] snrPad %d, HdrEnable %d, Type %d fusiontype %d", vifInfo->s32SensorId,
                  snrDrvInfo->bHdrEnable, groupAttr.eHDRType, snrDrvInfo->stPadInfo.eHDRFusionType);
#else
        groupAttr.eHDRType = (MI_VIF_HDRType_e)vifInfo->s32HdrType;
#endif
        if (groupAttr.eHDRType > E_MI_VIF_HDR_TYPE_OFF && groupAttr.eHDRType < E_MI_VIF_HDR_TYPE_MAX)
        {
            groupAttr.eHDRFusionTpye    = (MI_VIF_HDRFusionType_e)snrDrvInfo->stPadInfo.eHDRFusionType;
            groupAttr.u8HDRExposureMask = vifInfo->s32HdrExposureMask;
        }
        switch (snrDrvInfo->stPadInfo.eIntfMode)
        {
            case E_MI_VIF_MODE_BT656:
            {
                groupAttr.eClkEdge = (MI_VIF_ClkEdge_e)snrDrvInfo->stPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
            }
            break;
            case E_MI_VIF_MODE_MIPI:
            case E_MI_VIF_MODE_DIGITAL_CAMERA:
            case E_MI_VIF_MODE_BT1120_STANDARD:
            case E_MI_VIF_MODE_LVDS:
            {
                groupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
            }
            break;
            default:
                return SSOS_DEF_FAIL;
        }
        groupAttr.u32GroupStitchMask = vifInfo->u32StitchMask;
        MI_VIF_CreateDevGroup((MI_VIF_GROUP)groupId, &groupAttr);
        PTREE_DBG("Create Group %d", groupId);
    }
    ++g_vifGroupEnable[groupId];

    if (!g_vifCreateDev[sysMod->base.info->devId])
    {
        MI_VIF_DevAttr_t devAttr;

        memset(&devAttr, 0, sizeof(MI_VIF_DevAttr_t));

        devAttr.stInputRect.u16X      = snrDrvInfo->stSnrPlaneInfo.stCapRect.u16X;
        devAttr.stInputRect.u16Y      = snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Y;
        devAttr.stInputRect.u16Width  = snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Width;
        devAttr.stInputRect.u16Height = snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Height;
        PTREE_DBG("u16X:%d u16Y:%d u16Width:%d u16Height:%d\n", devAttr.stInputRect.u16X, devAttr.stInputRect.u16Y,
                  devAttr.stInputRect.u16Width, devAttr.stInputRect.u16Height);
        if (snrDrvInfo->stSnrPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
        {
            /* Is not bayer format */
            devAttr.eInputPixel = snrDrvInfo->stSnrPlaneInfo.ePixel;
        }
        else
        {
            /* bayer format */
            devAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(snrDrvInfo->stSnrPlaneInfo.ePixPrecision,
                                                                        snrDrvInfo->stSnrPlaneInfo.eBayerId);
        }
        PTREE_DBG("VIF Dev Crop: X: %d, Y: %d, W: %d, H: %d\n", devAttr.stInputRect.u16X, devAttr.stInputRect.u16Y,
                  devAttr.stInputRect.u16Width, devAttr.stInputRect.u16Height);
        MI_VIF_SetDevAttr((MI_VIF_DEV)vifInfo->base.base.devId, &devAttr);
        MI_VIF_EnableDev((MI_VIF_DEV)vifInfo->base.base.devId);
    }
    ++g_vifCreateDev[sysMod->base.info->devId];
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VIF_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_VIF_Info_t *vifInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_VIF_Info_t, base.base);
    unsigned int          groupId = 0;

    groupId = vifInfo->base.base.devId / 4;

    --g_vifCreateDev[vifInfo->base.base.devId];
    if (!g_vifCreateDev[vifInfo->base.base.devId])
    {
        MI_VIF_DisableDev((MI_VIF_DEV)vifInfo->base.base.devId);
    }

    --g_vifGroupEnable[groupId];
    if (!g_vifGroupEnable[groupId])
    {
        MI_VIF_DestroyDevGroup((MI_VIF_GROUP)groupId);
        PTREE_DBG("Destrouy Group %d", groupId);
    }

    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_VIF_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_VIF_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    return _PTREE_MOD_VIF_OutNew(sysMod, loopId);
}
static void _PTREE_MOD_VIF_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_VIF_Obj_t, base));
}
static int _PTREE_MOD_VIF_OutStart(PTREE_MOD_SYS_OutObj_t *modOut)
{
    PTREE_SUR_VIF_Info_t *   vifInfo    = CONTAINER_OF(modOut->thisSysMod->base.info, PTREE_SUR_VIF_Info_t, base.base);
    PTREE_SUR_VIF_OutInfo_t *vifOutInfo = CONTAINER_OF(modOut->base.info, PTREE_SUR_VIF_OutInfo_t, base.base);

    PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *snrDrvInfo = NULL;
    MI_VIF_OutputPortAttr_t                 stVifPortInfo;

    snrDrvInfo = PTREE_MOD_SNR_DATATYPE_GetSensorInfo(vifInfo->s32SensorId);
    if (!snrDrvInfo)
    {
        PTREE_ERR("pstSrcSnrDrvInfo is NULL, intSensorId:%d may not fpInit", vifInfo->s32SensorId);
        return SSOS_DEF_FAIL;
    }

    memset(&stVifPortInfo, 0, sizeof(MI_VIF_OutputPortAttr_t));
    stVifPortInfo.stCapRect.u16X = (MI_U16)(vifOutInfo->u32CropX < snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Width
                                                ? vifOutInfo->u32CropX
                                                : snrDrvInfo->stSnrPlaneInfo.stCapRect.u16X);
    stVifPortInfo.stCapRect.u16Y = (MI_U16)(vifOutInfo->u32CropY < snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Height
                                                ? vifOutInfo->u32CropY
                                                : snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Y);
    stVifPortInfo.stCapRect.u16Width =
        (MI_U16)(vifOutInfo->u32CropW && vifOutInfo->u32CropW < snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Width
                     ? vifOutInfo->u32CropW
                     : snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Width);
    stVifPortInfo.stCapRect.u16Height =
        (MI_U16)(vifOutInfo->u32CropH && vifOutInfo->u32CropH < snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Height
                     ? vifOutInfo->u32CropH
                     : snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Height);
    if (vifOutInfo->s32IsUseSnrFmt)
    {
        if (snrDrvInfo->stSnrPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
        {
            /* YUV Sensor */
            stVifPortInfo.ePixFormat = snrDrvInfo->stSnrPlaneInfo.ePixel;
        }
        else
        {
            stVifPortInfo.ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(snrDrvInfo->stSnrPlaneInfo.ePixPrecision,
                                                                             snrDrvInfo->stSnrPlaneInfo.eBayerId);
        }
        stVifPortInfo.stDestSize.u16Width  = stVifPortInfo.stCapRect.u16Width;
        stVifPortInfo.stDestSize.u16Height = stVifPortInfo.stCapRect.u16Height;
    }
    else
    {
        stVifPortInfo.ePixFormat = vifOutInfo->eOutFmt;
        if (stVifPortInfo.ePixFormat == E_MI_SYS_PIXEL_FRAME_FORMAT_MAX)
        {
            stVifPortInfo.ePixFormat = snrDrvInfo->stSnrPlaneInfo.ePixel;
        }
        stVifPortInfo.stDestSize.u16Width =
            (MI_U16)(vifOutInfo->u32Width && vifOutInfo->u32Width < snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Width
                         ? vifOutInfo->u32Width
                         : snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Width);
        stVifPortInfo.stDestSize.u16Height =
            (MI_U16)(vifOutInfo->u32Height && vifOutInfo->u32Height < snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Height
                         ? vifOutInfo->u32Height
                         : snrDrvInfo->stSnrPlaneInfo.stCapRect.u16Height);
    }
    stVifPortInfo.eFrameRate    = E_MI_VIF_FRAMERATE_FULL;
    stVifPortInfo.eCompressMode = (MI_SYS_CompressMode_e)vifOutInfo->s32CompressMode;
    PTREE_DBG("VIF Output %d Crop: X: %d, Y: %d, W: %d, H: %d\n", vifOutInfo->base.base.port,
              stVifPortInfo.stCapRect.u16X, stVifPortInfo.stCapRect.u16Y, stVifPortInfo.stCapRect.u16Width,
              stVifPortInfo.stCapRect.u16Height);
    MI_VIF_SetOutputPortAttr((MI_VIF_DEV)vifInfo->base.base.devId, vifOutInfo->base.base.port, &stVifPortInfo);
    MI_VIF_EnableOutputPort((MI_VIF_DEV)vifInfo->base.base.devId, vifOutInfo->base.base.port);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VIF_OutStop(PTREE_MOD_SYS_OutObj_t *modOut)
{
    PTREE_SUR_VIF_Info_t *   vifInfo    = CONTAINER_OF(modOut->thisSysMod->base.info, PTREE_SUR_VIF_Info_t, base.base);
    PTREE_SUR_VIF_OutInfo_t *vifOutInfo = CONTAINER_OF(modOut->base.info, PTREE_SUR_VIF_OutInfo_t, base.base);

    MI_VIF_DisableOutputPort((MI_VIF_DEV)vifInfo->base.base.devId, vifOutInfo->base.base.port);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_VIF_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *modOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    PTREE_SUR_VIF_OutInfo_t *vifOutInfo = CONTAINER_OF(modOut->base.info, PTREE_SUR_VIF_OutInfo_t, base.base);
    rawInfo->fmt                        = PTREE_MOD_SYS_SysFmtToPtreeFmt(vifOutInfo->eOutFmt);
    rawInfo->width                      = vifOutInfo->u32Width;
    rawInfo->height                     = vifOutInfo->u32Height;
}
static void _PTREE_MOD_VIF_OutFree(PTREE_MOD_SYS_OutObj_t *modOut)
{
    SSOS_MEM_Free(CONTAINER_OF(modOut, PTREE_MOD_VIF_OutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_VIF_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_VIF_OutObj_t *vifModOut = NULL;

    vifModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VIF_OutObj_t));
    if (!vifModOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(vifModOut, 0, sizeof(PTREE_MOD_VIF_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_OutObjInit(&vifModOut->base, &G_PTREE_MOD_VIF_SYS_OUT_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(vifModOut);
        return NULL;
    }
    PTREE_MOD_SYS_OutObjRegister(&vifModOut->base, &G_PTREE_MOD_VIF_SYS_OUT_HOOK);
    return &vifModOut->base.base;
}

PTREE_MOD_Obj_t *PTREE_MOD_VIF_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_VIF_Obj_t *vifMod = NULL;

    vifMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VIF_Obj_t));
    if (!vifMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(vifMod, 0, sizeof(PTREE_MOD_VIF_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&vifMod->base, &G_PTREE_MOD_VIF_SYS_OPS, tag, E_MI_MODULE_ID_VIF))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (vifMod->base.base.info->devId >= PTREE_MOD_VIF_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", vifMod->base.base.info->devId, PTREE_MOD_VIF_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    PTREE_MOD_SYS_ObjRegister(&vifMod->base, &G_PTREE_MOD_VIF_SYS_HOOK);
    return &vifMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&vifMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(vifMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(VIF, PTREE_MOD_VIF_New);
