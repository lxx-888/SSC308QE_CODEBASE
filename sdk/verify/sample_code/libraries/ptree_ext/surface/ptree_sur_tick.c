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
#include "ssos_mem.h"
#include "ssos_list.h"
#include "ssos_io.h"
#include "ptree_log.h"
#include "ptree_maker.h"
#include "ptree_sur_tick.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *        _PTREE_SUR_TICK_OccupyInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static int                   _PTREE_SUR_TICK_LoadDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_Info_t *info, PTREE_DB_Obj_t db);
static void                  _PTREE_SUR_TICK_Free(PTREE_SUR_Obj_t *sur);
static const PTREE_SUR_Ops_t G_PTREE_SUR_TICK_OPS = {
    .occupyInfo = _PTREE_SUR_TICK_OccupyInfo,
    .loadDb     = _PTREE_SUR_TICK_LoadDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_TICK_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_SUR_TICK_Free,
};

static PARENA_Tag_t *_PTREE_SUR_TICK_OccupyInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_TICK_Info_t, base, PTREE_SUR_Info_t);
}
static int _PTREE_SUR_TICK_LoadDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_Info_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_TICK_Info_t *tickInfo = CONTAINER_OF(info, PTREE_SUR_TICK_Info_t, base);
    (void)sur;
    tickInfo->sec           = PTREE_DB_GetInt(db, "SEC");
    tickInfo->nSec          = PTREE_DB_GetInt(db, "NSEC");
    tickInfo->frameCntLimit = PTREE_DB_GetInt(db, "LEFT_FRAME_CNT");
    snprintf(tickInfo->frcType, 16, "%s", PTREE_DB_GetStr(db, "FRC_TYPE"));
    snprintf(tickInfo->ioMode, 16, "%s", PTREE_DB_GetStr(db, "MODE"));
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_TICK_Free(PTREE_SUR_Obj_t *sur)
{
    SSOS_MEM_Free(sur);
}

PTREE_SUR_Obj_t *PTREE_SUR_TICK_New(void)
{
    PTREE_SUR_Obj_t *sur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sur, &G_PTREE_SUR_TICK_OPS))
    {
        SSOS_MEM_Free(sur);
        return NULL;
    }
    PTREE_SUR_Register(sur, &G_PTREE_SUR_TICK_HOOK);
    return sur;
}

PTREE_MAKER_SUR_INIT(TICK, PTREE_SUR_TICK_New);
