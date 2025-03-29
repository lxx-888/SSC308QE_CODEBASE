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
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "parena.h"
#include "ptree_mod.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ptree_mod_sys.h"
#include "ptree_sur_pool.h"
#include "ptree_mod_pool.h"
#include "ptree_maker.h"
#include "ssos_mem.h"

typedef struct PTREE_MOD_POOL_Obj_s PTREE_MOD_POOL_Obj_t;

struct PTREE_MOD_POOL_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};

static int                 _PTREE_MOD_POOL_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_POOL_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_POOL_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_POOL_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_POOL_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_POOL_SYS_OPS = {
    .init         = _PTREE_MOD_POOL_Init,
    .deinit       = _PTREE_MOD_POOL_Deinit,
    .createModIn  = _PTREE_MOD_POOL_CreateModIn,
    .createModOut = _PTREE_MOD_POOL_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_POOL_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_POOL_Free,
};

static int _PTREE_MOD_POOL_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_POOL_Info_t *       poolInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_POOL_Info_t, base.base);
    MI_SYS_GlobalPrivPoolConfig_t privPoolCfg;
    memset(&privPoolCfg, 0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
    privPoolCfg.bCreate                                         = TRUE;
    privPoolCfg.eConfigType                                     = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    privPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule      = (MI_ModuleId_e)poolInfo->u32PoolDevMod;
    privPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid     = poolInfo->u32PoolDevId;
    privPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth  = poolInfo->u32PoolVidWid;
    privPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = poolInfo->u32PoolVidHei;
    privPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16RingLine  = poolInfo->u32RingLine;
    if (MI_SUCCESS != MI_SYS_ConfigPrivateMMAPool(0, &privPoolCfg))
    {
        PTREE_ERR("MI_SYS_ConfigPrivateMMAPool failed");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_POOL_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_POOL_Info_t *       poolInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_POOL_Info_t, base.base);
    MI_SYS_GlobalPrivPoolConfig_t privPoolCfg;
    memset(&privPoolCfg, 0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
    privPoolCfg.bCreate                                     = FALSE;
    privPoolCfg.eConfigType                                 = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    privPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule  = (MI_ModuleId_e)poolInfo->u32PoolDevMod;
    privPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = poolInfo->u32PoolDevId;
    if (MI_SUCCESS != MI_SYS_ConfigPrivateMMAPool(0, &privPoolCfg))
    {
        PTREE_ERR("MI_SYS_ConfigPrivateMMAPool failed");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_POOL_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_POOL_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static void _PTREE_MOD_POOL_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_POOL_Obj_t *poolMod = CONTAINER_OF(sysMod, PTREE_MOD_POOL_Obj_t, base);
    SSOS_MEM_Free(poolMod);
}

PTREE_MOD_Obj_t *PTREE_MOD_POOL_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_POOL_Obj_t *poolMod = NULL;

    poolMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_POOL_Obj_t));
    if (!poolMod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(poolMod, 0, sizeof(PTREE_MOD_POOL_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&poolMod->base, &G_PTREE_MOD_POOL_SYS_OPS, tag, E_MI_MODULE_ID_MAX))
    {
        SSOS_MEM_Free(poolMod);
        return NULL;
    }
    PTREE_MOD_SYS_ObjRegister(&poolMod->base, &G_PTREE_MOD_POOL_SYS_HOOK);
    return &poolMod->base.base;
}

PTREE_MAKER_MOD_INIT(POOL, PTREE_MOD_POOL_New);
