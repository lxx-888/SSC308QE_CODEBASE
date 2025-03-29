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
#include "ssos_def.h"
#include "ptree_maker.h"
#include "ptree_enum.h"
#include "ptree_log.h"
#include "ptree_sur.h"
#include "ptree_sur_aestable.h"

PTREE_ENUM_DEFINE(PTREE_SUR_AESTABLE_FuncMode_e,                    /* PTREE_SUR_AESTABLE_FuncMode_e */
                  {E_PTREE_SUR_AESTABLE_RUN_MODE_SHOT, "shot"},     /* SHOT */
                  {E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD, "record"}, /* RECORD */
)

PTREE_ENUM_DEFINE(PTREE_SUR_AESTABLE_StartMode_e,                   /* PTREE_SUR_AESTABLE_StartMode_e */
                  {E_PTREE_SUR_AESTABLE_START_MODE_FORCE, "force"}, /* FORCE */
                  {E_PTREE_SUR_AESTABLE_START_MODE_AUTO, "auto"},   /* AUTO */
)

static PARENA_Tag_t *_PTREE_SUR_AESTABLE_OccupyInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_AESTABLE_LoadDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_Info_t *info, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_AESTABLE_Free(PTREE_SUR_Obj_t *sur);

static const PTREE_SUR_Ops_t G_PTREE_SUR_AESTABLE_OPS = {
    .occupyInfo = _PTREE_SUR_AESTABLE_OccupyInfo,
    .loadDb     = _PTREE_SUR_AESTABLE_LoadDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_AESTABLE_HOOK = {
    .free = _PTREE_SUR_AESTABLE_Free,
};
static PARENA_Tag_t *_PTREE_SUR_AESTABLE_OccupyInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_AESTABLE_Info_t, base, PTREE_SUR_Info_t);
}

static int _PTREE_SUR_AESTABLE_LoadDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_Info_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_AESTABLE_Info_t *aestableInfo = CONTAINER_OF(info, PTREE_SUR_AESTABLE_Info_t, base);
    (void)sur;
    aestableInfo->runMode   = PTREE_ENUM_FROM_STR(PTREE_SUR_AESTABLE_FuncMode_e, PTREE_DB_GetStr(db, "RUN_MODE"));
    aestableInfo->startMode = PTREE_ENUM_FROM_STR(PTREE_SUR_AESTABLE_StartMode_e, PTREE_DB_GetStr(db, "START_MODE"));
    aestableInfo->debugMode = (unsigned char)PTREE_DB_GetInt(db, "DEBUG_MODE");
    if (aestableInfo->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_SHOT)
    {
        PTREE_DB_ProcessKey(db, "RUN_MODE_PARAM");
        aestableInfo->stableCount   = (unsigned int)PTREE_DB_GetInt(db, "STABLE_COUNT");
        aestableInfo->captureCount  = (unsigned int)PTREE_DB_GetInt(db, "CAPTURE_COUNT");
        aestableInfo->usingLowPower = (unsigned char)PTREE_DB_GetInt(db, "USING_LOW_POWER");
        PTREE_DB_ProcessBack(db);
        return SSOS_DEF_OK;
    }
    if (aestableInfo->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD)
    {
        PTREE_DB_ProcessKey(db, "RUN_MODE_PARAM");
        aestableInfo->stableCount = (unsigned int)PTREE_DB_GetInt(db, "STABLE_COUNT");
        PTREE_DB_ProcessBack(db);
        return SSOS_DEF_OK;
    }
    PTREE_ERR("Get wrong running mode -> %d,\n", aestableInfo->runMode);
    return SSOS_DEF_FAIL;
}

static void _PTREE_SUR_AESTABLE_Free(PTREE_SUR_Obj_t *sur)
{
    SSOS_MEM_Free(sur);
}

PTREE_SUR_Obj_t *PTREE_SUR_AESTABLE_New(void)
{
    PTREE_SUR_Obj_t *sur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sur, &G_PTREE_SUR_AESTABLE_OPS))
    {
        SSOS_MEM_Free(sur);
        return NULL;
    }
    PTREE_SUR_Register(sur, &G_PTREE_SUR_AESTABLE_HOOK);
    return sur;
}

PTREE_MAKER_SUR_INIT(AESTABLE, PTREE_SUR_AESTABLE_New);
