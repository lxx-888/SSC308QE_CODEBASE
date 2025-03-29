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
#include "ptree_maker.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_hseg.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_HSEG_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_HSEG_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *info, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_HSEG_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_HSEG_OPS = {
    .occupyInfo = _PTREE_SUR_HSEG_OccupyInfo,
    .loadDb     = _PTREE_SUR_HSEG_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_HSEG_HOOK = {
    .free = _PTREE_SUR_HSEG_Free,
};

static PARENA_Tag_t *_PTREE_SUR_HSEG_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_HSEG_Info_t, base.base, PTREE_SUR_Info_t);
}

static int _PTREE_SUR_HSEG_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_HSEG_Info_t *pstHsegInfo = CONTAINER_OF(info, PTREE_SUR_HSEG_Info_t, base);
    (void)sur;
    snprintf(pstHsegInfo->chHsegMode, 32, "%s", PTREE_DB_GetStr(db, "HSEG_MODE"));
    snprintf(pstHsegInfo->chMaskOp, 32, "%s", PTREE_DB_GetStr(db, "MASK_OP"));
    snprintf(pstHsegInfo->chIpuPath, 64, "%s", PTREE_DB_GetStr(db, "IPU_PATH"));
    snprintf(pstHsegInfo->chModelPath, 64, "%s", PTREE_DB_GetStr(db, "MODEL_PATH"));
    pstHsegInfo->uiMaskThr      = (unsigned int)PTREE_DB_GetInt(db, "MASK_THR");
    pstHsegInfo->uiBlurLv       = (unsigned int)PTREE_DB_GetInt(db, "BLUR_LV");
    pstHsegInfo->uiScalingStage = (unsigned int)PTREE_DB_GetInt(db, "SCAL_STAGE");
    return 0;
}
static void _PTREE_SUR_HSEG_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_HSEG_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_HSEG_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_HSEG_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(HSEG, PTREE_SUR_HSEG_New);
