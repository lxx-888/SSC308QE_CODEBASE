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

#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_pool.h"
#include "ptree_maker.h"
#include "mi_common_datatype.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_POOL_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_POOL_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_POOL_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_POOL_OPS = {
    .occupyInfo = _PTREE_SUR_POOL_OccupyInfo,
    .loadDb     = _PTREE_SUR_POOL_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_POOL_HOOK = {
    .free = _PTREE_SUR_POOL_Free,
};

static PARENA_Tag_t *_PTREE_SUR_POOL_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_POOL_Info_t, base.base, PTREE_SUR_Info_t);
}
static int _PTREE_SUR_POOL_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_POOL_Info_t *poolInfo = CONTAINER_OF(info, PTREE_SUR_POOL_Info_t, base);

    (void)sur;

    poolInfo->u32PoolDevMod = (unsigned int)PTREE_DB_GetInt(db, "DEV_MOD");
    poolInfo->u32PoolDevId  = (unsigned int)PTREE_DB_GetInt(db, "DEV_ID");
    poolInfo->u32PoolVidWid = (unsigned int)PTREE_DB_GetInt(db, "VID_W");
    poolInfo->u32PoolVidHei = (unsigned int)PTREE_DB_GetInt(db, "VID_H");
    poolInfo->u32RingLine   = (unsigned int)PTREE_DB_GetInt(db, "RING_LINE");

    return SSOS_DEF_OK;
}
static void _PTREE_SUR_POOL_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_POOL_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_POOL_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_POOL_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(POOL, PTREE_SUR_POOL_New);
