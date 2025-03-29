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
#include "ssos_list.h"
#include "ssos_io.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "parena.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_det.h"
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

static PARENA_Tag_t *_PTREE_SUR_DET_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_DET_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *info, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_DET_Free(PTREE_SUR_SYS_Obj_t *sur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_DET_OPS = {
    .occupyInfo = _PTREE_SUR_DET_OccupyInfo,
    .loadDb     = _PTREE_SUR_DET_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_DET_HOOK = {
    .free = _PTREE_SUR_DET_Free,
};

static PARENA_Tag_t *_PTREE_SUR_DET_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_DET_Info_t, base.base, PTREE_SUR_SYS_Info_t);
}
static int _PTREE_SUR_DET_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_DET_Info_t *pstInfo = CONTAINER_OF(info, PTREE_SUR_DET_Info_t, base);
    const char *          tempStr = NULL;

    (void)sur;

    tempStr = PTREE_DB_GetStr(db, "FW_PATH");
    if (tempStr)
    {
        strncpy(pstInfo->fwPath, tempStr, PTREE_SUR_DET_PATH_LEN_MAX - 1);
    }

    tempStr = PTREE_DB_GetStr(db, "MODEL_PATH");
    if (tempStr)
    {
        strncpy(pstInfo->modelPath, tempStr, PTREE_SUR_DET_PATH_LEN_MAX - 1);
    }

    pstInfo->threshold = PTREE_DB_GetInt(db, "THRESHOLD");
    return SSOS_DEF_OK;
}

static void _PTREE_SUR_DET_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_DET_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_DET_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_DET_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(DET, PTREE_SUR_DET_New);
