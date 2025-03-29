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

#include <ssos_mem.h>
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "mi_sensor.h"
#include "ssos_def.h"
#include "ptree_mod.h"
#include "ptree_log.h"
#include "ssos_list.h"
#include "ptree_mod_sys.h"
#include "ptree_sur_snr.h"
#include "ptree_mod_snr.h"
#include "ptree_mod_snr_datatype.h"
#include "ptree_maker.h"

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
#include "earlyinit_preload_api.h"
#endif

#define MI_SENSOR_MAX_SENSORID 10

typedef struct PTREE_MOD_SNR_Obj_s PTREE_MOD_SNR_Obj_t;

struct PTREE_MOD_SNR_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};

static int                 _PTREE_MOD_SNR_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_SNR_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_SNR_Start(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_SNR_Stop(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_SNR_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_SNR_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_SNR_Free(PTREE_MOD_SYS_Obj_t *sysMod);
static void                _PTREE_MOD_SNR_ClearSensorInfo(MI_U32 u32SnrId);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_SNR_SYS_OPS = {
    .init         = _PTREE_MOD_SNR_Init,
    .deinit       = _PTREE_MOD_SNR_Deinit,
    .start        = _PTREE_MOD_SNR_Start,
    .stop         = _PTREE_MOD_SNR_Stop,
    .createModIn  = _PTREE_MOD_SNR_CreateModIn,
    .createModOut = _PTREE_MOD_SNR_CreateModOut,
};

static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_SNR_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SNR_Free,
};

static PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *g_pastSensorDevInfo[MI_SENSOR_MAX_SENSORID] = {0};

static void _PTREE_MOD_SNR_GetEarlyInitCfg(PTREE_SUR_SNR_Info_t *snrInfo, MI_BOOL *hdrEnable)
{
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
    const EarlyInitPreloadCfg_t *earlyinitCfg      = NULL;
    MI_U8                        u8EarlyinitEnable = 0;
    int                          earlyInitId       = 0;

    earlyinitCfg = DrvEarlyInitGetPreloadCfg();
    if (earlyinitCfg != NULL)
    {
        for (earlyInitId = 0; earlyInitId < earlyinitCfg->u32NumSnr; earlyInitId++)
        {
            if (earlyinitCfg->ChCfg[earlyInitId].u8SnrPad == snrInfo->s32SensorId)
            {
                u8EarlyinitEnable = DrvEarlyInitForPreloadIsEnabled(earlyInitId);
                break;
            }
        }
    }
    PTREE_DBG("[earlyinit] snr pad:%d, earlyinit_enable:%d", earlyinitCfg->ChCfg[earlyInitId].u8SnrPad,
              u8EarlyinitEnable);

    if (u8EarlyinitEnable)
    {
        snrInfo->s32SensorMirror = earlyinitCfg->ChCfg[earlyInitId].bMirror;
        snrInfo->s32SensorFlip   = earlyinitCfg->ChCfg[earlyInitId].bFlip;
        *hdrEnable               = earlyinitCfg->ChCfg[earlyInitId].bHDREn;
        snrInfo->s32SensorRes    = earlyinitCfg->ChCfg[earlyInitId].u8ResIdx;
        if (earlyinitCfg->ChCfg[earlyInitId].u32SensorFrameRate / 1000 != 0)
        {
            if (snrInfo->s32SensorFps == 0)
            {
                snrInfo->s32SensorFps = earlyinitCfg->ChCfg[earlyInitId].u32SensorFrameRate / 1000;
            }
        }
        PTREE_DBG("[earlyinit] Mirror %d, Flip %d, HdrEnable %d Res %d, fps %d", snrInfo->s32SensorMirror,
                  snrInfo->s32SensorFlip, *hdrEnable, snrInfo->s32SensorRes, snrInfo->s32SensorFps);
    }
#else
    (void)snrInfo;
    (void)hdrEnable;
#endif
}
static int _PTREE_MOD_SNR_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SNR_Info_t *                  snrInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SNR_Info_t, base.base);
    PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *sensorDrvInfo = PTREE_MOD_SNR_DATATYPE_GetSensorInfo(snrInfo->s32SensorId);
    MI_U32                                  u32ResCount   = 0;
    MI_U8                                   u8ResIndex    = 0;
    MI_SNR_Res_t                            stRes;
    if (!sensorDrvInfo)
    {
        PTREE_ERR("pastSensorDrvInfo[%d] is null, maybe had some thing wrong", snrInfo->s32SensorId);
        return SSOS_DEF_FAIL;
    }
    MI_SNR_GetPadInfo(snrInfo->s32SensorId, &sensorDrvInfo->stPadInfo);
    sensorDrvInfo->bHdrEnable =
        !(snrInfo->s32HdrType == E_MI_SNR_HDR_TYPE_OFF || snrInfo->s32HdrType == E_MI_SNR_HDR_TYPE_LI);
    _PTREE_MOD_SNR_GetEarlyInitCfg(snrInfo, &sensorDrvInfo->bHdrEnable);

    MI_SNR_SetPlaneMode(snrInfo->s32SensorId, sensorDrvInfo->bHdrEnable);
    MI_SNR_QueryResCount(snrInfo->s32SensorId, &u32ResCount);
    memset(&stRes, 0x0, sizeof(MI_SNR_Res_t));
    for (u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
    {
        MI_SNR_GetRes(snrInfo->s32SensorId, u8ResIndex, &stRes);
        PTREE_DBG("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s", u8ResIndex,
                  stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width, stRes.stCropRect.u16Height,
                  stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height, stRes.u32MaxFps, stRes.u32MinFps,
                  stRes.strResDesc);
    }

    if (snrInfo->s32SensorRes >= u32ResCount)
    {
        PTREE_ERR("choice err res %d > =cnt %d", snrInfo->s32SensorRes, u32ResCount);
        return SSOS_DEF_FAIL;
    }
    PTREE_DBG("You choose sensor res is %d", snrInfo->s32SensorRes);

    MI_SNR_SetRes(snrInfo->s32SensorId, snrInfo->s32SensorRes);
    MI_SNR_GetRes(snrInfo->s32SensorId, snrInfo->s32SensorRes, &stRes);
    if (snrInfo->s32SensorFps > stRes.u32MinFps && snrInfo->s32SensorFps < stRes.u32MaxFps)
    {
        MI_SNR_SetFps(snrInfo->s32SensorId, snrInfo->s32SensorFps);
    }
    if (snrInfo->s32SensorMirror || snrInfo->s32SensorFlip)
    {
        MI_SNR_SetOrien(snrInfo->s32SensorId, snrInfo->s32SensorMirror, snrInfo->s32SensorFlip);
    }
    MI_SNR_GetPlaneInfo(snrInfo->s32SensorId, 0, &sensorDrvInfo->stSnrPlaneInfo);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SNR_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SNR_Info_t *snrInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SNR_Info_t, base.base);

    PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *sensorDrvInfo = PTREE_MOD_SNR_DATATYPE_GetSensorInfo(snrInfo->s32SensorId);
    if (!sensorDrvInfo)
    {
        PTREE_ERR("pastSensorDrvInfo[%d] is null, maybe had some thing wrong", snrInfo->s32SensorId);
        return SSOS_DEF_FAIL;
    }
    _PTREE_MOD_SNR_ClearSensorInfo(snrInfo->s32SensorId);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SNR_Start(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SNR_Info_t *snrInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SNR_Info_t, base.base);

    MI_SNR_Enable(snrInfo->s32SensorId);

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SNR_Stop(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SNR_Info_t *snrInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SNR_Info_t, base.base);

    MI_SNR_Disable((MI_SNR_PADID)snrInfo->s32SensorId);

    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_SNR_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_SNR_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static void _PTREE_MOD_SNR_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_SNR_Obj_t *snrMod = CONTAINER_OF(sysMod, PTREE_MOD_SNR_Obj_t, base);
    SSOS_MEM_Free(snrMod);
}

static void _PTREE_MOD_SNR_ClearSensorInfo(MI_U32 u32SnrId)
{
    if (u32SnrId >= MI_SENSOR_MAX_SENSORID)
    {
        PTREE_ERR("u32SnrId:%d >= maxSensorid:%d", u32SnrId, MI_SENSOR_MAX_SENSORID);
        return;
    }
    if (!g_pastSensorDevInfo[u32SnrId])
    {
        PTREE_ERR("Sensor info had not been created!\n");
        return;
    }
    SSOS_MEM_Free(g_pastSensorDevInfo[u32SnrId]);
    g_pastSensorDevInfo[u32SnrId] = NULL;
}

PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t *PTREE_MOD_SNR_DATATYPE_GetSensorInfo(MI_U32 u32SnrId)
{
    if (u32SnrId >= MI_SENSOR_MAX_SENSORID)
    {
        PTREE_ERR("u32SnrId:%d >= MI_SENSOR_MAX_SENSORID:%d failed", u32SnrId, MI_SENSOR_MAX_SENSORID);
        return NULL;
    }
    if (!g_pastSensorDevInfo[u32SnrId])
    {
        g_pastSensorDevInfo[u32SnrId] = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SNR_DATATYPE_SensorDrvInfo_t));
        if (!g_pastSensorDevInfo[u32SnrId])
        {
            PTREE_ERR("alloc pstSnrDrvInfoBak for sensor %d failed", u32SnrId);
            return NULL;
        }
    }
    return g_pastSensorDevInfo[u32SnrId];
}

PTREE_MOD_Obj_t *PTREE_MOD_SNR_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_SNR_Obj_t *snrMod = NULL;

    snrMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SNR_Obj_t));
    if (!snrMod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(snrMod, 0, sizeof(PTREE_MOD_SNR_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&snrMod->base, &G_PTREE_MOD_SNR_SYS_OPS, tag, E_MI_MODULE_ID_SNR))
    {
        SSOS_MEM_Free(snrMod);
        return NULL;
    }
    PTREE_MOD_SYS_ObjRegister(&snrMod->base, &G_PTREE_MOD_SNR_SYS_HOOK);
    return &snrMod->base.base;
}

PTREE_MAKER_MOD_INIT(SNR, PTREE_MOD_SNR_New);
