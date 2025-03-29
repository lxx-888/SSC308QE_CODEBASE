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
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_sur_sync.h"
#include "ssos_mem.h"
#include "ssos_list.h"
#include "ptree_maker.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_SYNC_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_SYNC_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *info, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_SYNC_Free(PTREE_SUR_Obj_t *sur);
static const PTREE_SUR_Ops_t G_PTREE_SUR_SYNC_OPS = {
    .occupyInInfo = _PTREE_SUR_SYNC_OccupyInInfo,
    .loadInDb     = _PTREE_SUR_SYNC_LoadInDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_SYNC_HOOK = {
    .free = _PTREE_SUR_SYNC_Free,
};
static PARENA_Tag_t *_PTREE_SUR_SYNC_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_SYNC_InInfo_t, base, PTREE_SUR_InInfo_t);
}
static int _PTREE_SUR_SYNC_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SYNC_InInfo_t *syncInfo = CONTAINER_OF(info, PTREE_SUR_SYNC_InInfo_t, base);
    (void)sur;
    syncInfo->u16OutIndex = (unsigned short)PTREE_DB_GetInt(db, "OUTPUT_PORT_ID");
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_SYNC_Free(PTREE_SUR_Obj_t *sur)
{
    SSOS_MEM_Free(sur);
}

PTREE_SUR_Obj_t *PTREE_SUR_SYNC_New(void)
{
    PTREE_SUR_Obj_t *sur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sur, &G_PTREE_SUR_SYNC_OPS))
    {
        SSOS_MEM_Free(sur);
        return NULL;
    }
    PTREE_SUR_Register(sur, &G_PTREE_SUR_SYNC_HOOK);
    return sur;
}

PTREE_MAKER_SUR_INIT(SYNC, PTREE_SUR_SYNC_New);
