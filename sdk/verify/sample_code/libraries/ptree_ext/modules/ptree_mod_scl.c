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
#include "mi_scl_datatype.h"
#include "mi_scl.h"
#include "mi_sys_datatype.h"
#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_packet_raw.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_scl.h"
#include "ptree_mod_scl_base.h"
#include "ptree_sur_scl.h"
#include "ptree_sur_sys.h"
#include "ptree_maker.h"

typedef struct PTREE_MOD_SCL_Obj_s    PTREE_MOD_SCL_Obj_t;
typedef struct PTREE_MOD_SCL_InObj_s  PTREE_MOD_SCL_InObj_t;
typedef struct PTREE_MOD_SCL_OutObj_s PTREE_MOD_SCL_OutObj_t;

struct PTREE_MOD_SCL_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};
struct PTREE_MOD_SCL_InObj_s
{
    PTREE_MOD_SYS_InObj_t base;
};
struct PTREE_MOD_SCL_OutObj_s
{
    PTREE_MOD_SYS_OutObj_t base;
};

static int                 _PTREE_MOD_SCL_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_SCL_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_SCL_Start(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_SCL_Stop(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_SCL_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_SCL_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static int                _PTREE_MOD_SCL_InStart(PTREE_MOD_SYS_InObj_t *sysModIn);
static void               _PTREE_MOD_SCL_InFree(PTREE_MOD_SYS_InObj_t *sysModIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_SCL_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static int  _PTREE_MOD_SCL_OutStart(PTREE_MOD_SYS_OutObj_t *sysModOut);
static int  _PTREE_MOD_SCL_OutStop(PTREE_MOD_SYS_OutObj_t *sysModOut);
static void _PTREE_MOD_SCL_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo);
static void _PTREE_MOD_SCL_OutFree(PTREE_MOD_SYS_OutObj_t *sysModOut);
static int  _PTREE_MOD_SCL_ResetStreamOut(PTREE_MOD_SYS_OutObj_t *modOut, unsigned int width, unsigned int height);

static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_SCL_SYS_OPS = {
    .init         = _PTREE_MOD_SCL_Init,
    .deinit       = _PTREE_MOD_SCL_Deinit,
    .start        = _PTREE_MOD_SCL_Start,
    .stop         = _PTREE_MOD_SCL_Stop,
    .createModIn  = _PTREE_MOD_SCL_CreateModIn,
    .createModOut = _PTREE_MOD_SCL_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_SCL_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SCL_Free,
};

static const PTREE_MOD_SYS_InOps_t G_PTREE_MOD_SCL_SYS_IN_OPS = {
    .start = _PTREE_MOD_SCL_InStart,
};
static const PTREE_MOD_SYS_InHook_t G_PTREE_MOD_SCL_SYS_IN_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SCL_InFree,
};

static const PTREE_MOD_SYS_OutOps_t G_PTREE_MOD_SCL_SYS_OUT_OPS = {
    .start          = _PTREE_MOD_SCL_OutStart,
    .stop           = _PTREE_MOD_SCL_OutStop,
    .getPacketInfo  = _PTREE_MOD_SCL_OutGetPacketInfo,
    .resetStreamOut = _PTREE_MOD_SCL_ResetStreamOut,
};
static const PTREE_MOD_SYS_OutHook_t G_PTREE_MOD_SCL_SYS_OUT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SCL_OutFree,
};

static int _PTREE_MOD_SCL_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SCL_Info_t *sclInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SCL_Info_t, base.base);
    MI_SCL_ChannelAttr_t  sclChnAttr;
    MI_SCL_ChnParam_t     sclChnParam;
    MI_SCL_DevAttr_t      sclDevAttr;
    memset(&sclDevAttr, 0, sizeof(MI_SCL_DevAttr_t));
    memset(&sclChnAttr, 0, sizeof(MI_SCL_ChannelAttr_t));
    sclDevAttr.u32NeedUseHWOutPortMask = sclInfo->u32HwPortMode;

    if (SSOS_DEF_OK != PTREE_MOD_SCL_BASE_CreateDevice(sclInfo->base.base.devId, &sclDevAttr))
    {
        PTREE_ERR("MI_SCL_CreateDevice(%d) failed", sclInfo->base.base.devId);
        return SSOS_DEF_FAIL;
    }

    if (MI_SUCCESS
        != MI_SCL_CreateChannel((MI_SCL_DEV)sclInfo->base.base.devId, (MI_SCL_CHANNEL)sclInfo->base.base.chnId,
                                &sclChnAttr))
    {
        PTREE_ERR("MI_SCL_CreateChannel error");
        return SSOS_DEF_FAIL;
    }

    memset(&sclChnParam, 0, sizeof(MI_SCL_ChnParam_t));
    sclChnParam.eRot = (MI_SYS_Rotate_e)sclInfo->u32Rotation;
    if (MI_SUCCESS
        != MI_SCL_SetChnParam((MI_SCL_DEV)sclInfo->base.base.devId, (MI_SCL_CHANNEL)sclInfo->base.base.chnId,
                              &sclChnParam))
    {
        PTREE_ERR("MI_SCL_SetChnParam error");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SCL_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SCL_Info_t *sclInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SCL_Info_t, base.base);

    MI_SCL_DestroyChannel((MI_SCL_DEV)sclInfo->base.base.devId, (MI_SCL_CHANNEL)sclInfo->base.base.chnId);

    PTREE_MOD_SCL_BASE_DestroyDevice(sclInfo->base.base.devId);

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SCL_Start(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SCL_Info_t *sclInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SCL_Info_t, base.base);

    if (MI_SUCCESS != MI_SCL_StartChannel(sclInfo->base.base.devId, sclInfo->base.base.chnId))
    {
        PTREE_ERR("MI_SCL_StartChannel failed");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SCL_Stop(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SCL_Info_t *sclInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SCL_Info_t, base.base);

    MI_SCL_StopChannel(sclInfo->base.base.devId, sclInfo->base.base.chnId);

    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_SCL_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    (void)loopId;
    return _PTREE_MOD_SCL_InNew(sysMod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    (void)loopId;
    return _PTREE_MOD_SCL_OutNew(sysMod, loopId);
}
static void _PTREE_MOD_SCL_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_SCL_Obj_t, base));
}

static int _PTREE_MOD_SCL_InStart(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    PTREE_SUR_SCL_InInfo_t *sclInInfo = CONTAINER_OF(sysModIn->base.info, PTREE_SUR_SCL_InInfo_t, base.base);
    PTREE_SUR_SYS_InInfo_t *sysInInfo = &sclInInfo->base;
    PTREE_SUR_Info_t *      info      = sysModIn->base.thisMod->info;
    MI_SYS_WindowRect_t     cropRect;

    if (sysInInfo->bindType != E_MI_SYS_BIND_TYPE_FRAME_BASE)
    {
        return SSOS_DEF_OK;
    }

    if (!sclInInfo->u16CropW || !sclInInfo->u16CropH)
    {
        return SSOS_DEF_OK;
    }

    memset(&cropRect, 0, sizeof(MI_SYS_WindowRect_t));
    cropRect.u16X      = sclInInfo->u16CropX;
    cropRect.u16Y      = sclInInfo->u16CropY;
    cropRect.u16Width  = sclInInfo->u16CropW;
    cropRect.u16Height = sclInInfo->u16CropH;
    if (MI_SUCCESS != MI_SCL_SetInputPortCrop(info->devId, info->chnId, &cropRect))
    {
        PTREE_ERR("MI_SCL_SetInputPortCrop failed");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static void _PTREE_MOD_SCL_InFree(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModIn, PTREE_MOD_SCL_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_SCL_InNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_SCL_InObj_t *sclModIn = NULL;

    sclModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SCL_InObj_t));
    if (!sclModIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sclModIn, 0, sizeof(PTREE_MOD_SCL_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_InObjInit(&sclModIn->base, &G_PTREE_MOD_SCL_SYS_IN_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(sclModIn);
        return NULL;
    }
    PTREE_MOD_SYS_InObjRegister(&sclModIn->base, &G_PTREE_MOD_SCL_SYS_IN_HOOK);
    return &sclModIn->base.base;
}

static int _PTREE_MOD_SCL_OutStart(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    PTREE_SUR_SCL_OutInfo_t *sclOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_SCL_OutInfo_t, base.base);
    PTREE_SUR_Info_t *       info       = sysModOut->base.thisMod->info;

    MI_SCL_OutPortParam_t sclOutputParam;

    memset(&sclOutputParam, 0, sizeof(MI_SCL_OutPortParam_t));
    sclOutputParam.bMirror                    = (MI_BOOL)sclOutInfo->bMirror;
    sclOutputParam.bFlip                      = (MI_BOOL)sclOutInfo->bFlip;
    sclOutputParam.stSCLOutCropRect.u16X      = (MI_U16)sclOutInfo->u16CropX;
    sclOutputParam.stSCLOutCropRect.u16Y      = (MI_U16)sclOutInfo->u16CropY;
    sclOutputParam.stSCLOutCropRect.u16Width  = (MI_U16)sclOutInfo->u16CropW;
    sclOutputParam.stSCLOutCropRect.u16Height = (MI_U16)sclOutInfo->u16CropH;
    sclOutputParam.ePixelFormat               = (MI_SYS_PixelFormat_e)sclOutInfo->u32VideoFormat;
    sclOutputParam.stSCLOutputSize.u16Width   = (MI_U16)sclOutInfo->u16Width;
    sclOutputParam.stSCLOutputSize.u16Height  = (MI_U16)sclOutInfo->u16Height;
    sclOutputParam.eCompressMode              = (MI_SYS_CompressMode_e)sclOutInfo->u32CompressMode;
    /* Fixme: The current bw tool doesn't support this yet!Keep the default value 0 */
    // scl_output_param.eBufLayout
    if (MI_SUCCESS
        != MI_SCL_SetOutputPortParam((MI_SCL_DEV)info->devId, (MI_SCL_CHANNEL)info->chnId,
                                     (MI_SCL_PORT)sclOutInfo->base.base.port, &sclOutputParam))
    {
        PTREE_ERR("MI_SCL_SetOutputPortParam error");
        return SSOS_DEF_FAIL;
    }

    if (MI_SUCCESS
        != MI_SCL_EnableOutputPort((MI_SCL_DEV)info->devId, (MI_SCL_CHANNEL)info->chnId,
                                   (MI_SCL_PORT)sclOutInfo->base.base.port))
    {
        PTREE_ERR("MI_SCL_EnableOutputPort error");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SCL_OutStop(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    PTREE_SUR_SCL_OutInfo_t *sclOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_SCL_OutInfo_t, base.base);
    PTREE_SUR_Info_t *       info       = sysModOut->base.thisMod->info;

    MI_SCL_DisableOutputPort(info->devId, info->chnId, sclOutInfo->base.base.port);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_SCL_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    PTREE_SUR_SCL_OutInfo_t *sclOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_SCL_OutInfo_t, base.base);
    rawInfo->fmt                        = PTREE_MOD_SYS_SysFmtToPtreeFmt(sclOutInfo->u32VideoFormat);
    rawInfo->width                      = sclOutInfo->u16Width;
    rawInfo->height                     = sclOutInfo->u16Height;
}
static void _PTREE_MOD_SCL_OutFree(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModOut, PTREE_MOD_SCL_OutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_OutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_SCL_OutObj_t *sclModOut = NULL;

    sclModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SCL_OutObj_t));
    if (!sclModOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sclModOut, 0, sizeof(PTREE_MOD_SCL_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_OutObjInit(&sclModOut->base, &G_PTREE_MOD_SCL_SYS_OUT_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(sclModOut);
        return NULL;
    }
    PTREE_MOD_SYS_OutObjRegister(&sclModOut->base, &G_PTREE_MOD_SCL_SYS_OUT_HOOK);
    return &sclModOut->base.base;
}

static int _PTREE_MOD_SCL_ResetStreamOut(PTREE_MOD_SYS_OutObj_t *modOut, unsigned int width, unsigned int height)
{
    PTREE_SUR_SCL_OutInfo_t *sclOutInfo = CONTAINER_OF(modOut->base.info, PTREE_SUR_SCL_OutInfo_t, base.base);

    if (modOut->base.hasStart)
    {
        MI_SCL_OutPortParam_t stSclOutputParam;

        memset(&stSclOutputParam, 0, sizeof(MI_SCL_OutPortParam_t));
        MI_SCL_GetOutputPortParam((MI_SCL_DEV)modOut->base.thisMod->info->devId,
                                  (MI_SCL_CHANNEL)modOut->base.thisMod->info->chnId,
                                  (MI_SCL_PORT)modOut->base.info->port, &stSclOutputParam);
        sclOutInfo->u16Width = stSclOutputParam.stSCLOutputSize.u16Width = width;
        sclOutInfo->u16Height = stSclOutputParam.stSCLOutputSize.u16Height = height;
        MI_SCL_SetOutputPortParam((MI_SCL_DEV)modOut->base.thisMod->info->devId,
                                  (MI_SCL_CHANNEL)modOut->base.thisMod->info->chnId,
                                  (MI_SCL_PORT)modOut->base.info->port, &stSclOutputParam);
        return SSOS_DEF_OK;
    }
    sclOutInfo->u16Width  = width;
    sclOutInfo->u16Height = height;
    return SSOS_DEF_OK;
}

PTREE_MOD_Obj_t *PTREE_MOD_SCL_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_SCL_Obj_t *sclMod = NULL;

    sclMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SCL_Obj_t));
    if (!sclMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(sclMod, 0, sizeof(PTREE_MOD_SCL_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&sclMod->base, &G_PTREE_MOD_SCL_SYS_OPS, tag, E_MI_MODULE_ID_SCL))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (sclMod->base.base.info->devId >= PTREE_MOD_SCL_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", sclMod->base.base.info->devId, PTREE_MOD_SCL_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    PTREE_MOD_SYS_ObjRegister(&sclMod->base, &G_PTREE_MOD_SCL_SYS_HOOK);
    return &sclMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&sclMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(sclMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(SCL, PTREE_MOD_SCL_New);
