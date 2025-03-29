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

#include "ptree_db.h"
#include "ssos_def.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_iq.h"
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

static PARENA_Tag_t *_PTREE_SUR_IQ_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_IQ_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_IQ_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_IQ_OPS = {
    .occupyInfo = _PTREE_SUR_IQ_OccupyInfo,
    .loadDb     = _PTREE_SUR_IQ_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_IQ_HOOK = {
    .free = _PTREE_SUR_IQ_Free,
};

PTREE_ENUM_DEFINE(MI_ISP_IQ_CaliItem_e, {E_SS_CALI_ITEM_AWB, "awb"}, {E_SS_CALI_ITEM_OBC, "obc"},
                  {E_SS_CALI_ITEM_SDC, "sdc"}, {E_SS_CALI_ITEM_ALSC, "alsc"}, {E_SS_CALI_ITEM_LSC, "lsc"},
                  {E_SS_CALI_ITEM_AWB_EX, "awb_ex"}, {E_SS_CALI_ITEM_FPN, "fpn"}, {E_SS_CALI_ITEM_NE, "ne"},
                  {E_SS_CALI_ITEM_AIBNR, "aibnr"}, )
static PARENA_Tag_t *_PTREE_SUR_IQ_OccupyInfo(PTREE_SUR_SYS_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_IQ_Info_t, base.base, PTREE_SUR_Info_t);
}
static int _PTREE_SUR_IQ_LoadDb(PTREE_SUR_SYS_Obj_t *sur, PTREE_SUR_SYS_Info_t *info, PTREE_DB_Obj_t db)
{
    const char *         str         = NULL;
    const char *         strItem     = NULL;
    const char *         strCaliFile = NULL;
    PTREE_SUR_IQ_Info_t *iqInfo      = CONTAINER_OF(info, PTREE_SUR_IQ_Info_t, base);
    int                  i           = 0;
    char                 caliCfgIdx[32];

    (void)sur;

    iqInfo->u8OpenIqServer = PTREE_DB_GetInt(db, "DO_OPEN_IQ_SRV");
    iqInfo->u8InitCus3a    = PTREE_DB_GetInt(db, "CUS3A");
    iqInfo->u8Cus3aAe      = PTREE_DB_GetInt(db, "CUS3A_AE");
    iqInfo->u8Cus3aAwb     = PTREE_DB_GetInt(db, "CUS3A_AWB");
    iqInfo->u8Cus3aAf      = PTREE_DB_GetInt(db, "CUS3A_AF");
    iqInfo->u8Cus3aBlack   = PTREE_DB_GetInt(db, "CUS3A_BLOCK");
    iqInfo->u16Key         = PTREE_DB_GetInt(db, "IQ_USR_KEY");
    str                    = PTREE_DB_GetStr(db, "CUS3A_TYPE");
    snprintf(iqInfo->cus3aType, 16, "%s", str);
    iqInfo->u32CaliCfgCnt = PTREE_DB_GetInt(db, "CALI_CFG_CNT");
    if (iqInfo->u32CaliCfgCnt >= MAX_IQ_CALI_CFG)
    {
        PTREE_ERR("Get cali cfg max cnt error, get cnt is %d!", iqInfo->u32CaliCfgCnt);
        return SSOS_DEF_FAIL;
    }
    for (i = 0; i < iqInfo->u32CaliCfgCnt; i++)
    {
        snprintf(caliCfgIdx, 32, "CALI_CFG_%d", i);
        PTREE_DB_ProcessKey(db, caliCfgIdx);
        strItem     = PTREE_DB_GetStr(db, "IQ_CALI_ITEM");
        strCaliFile = PTREE_DB_GetStr(db, "IQ_CALI_FILE");
        if (!strItem || strlen(strItem) <= 1 || !strlen(strCaliFile))
        {
            PTREE_ERR("Get cali item or file error!");
            PTREE_DB_ProcessBack(db);
            continue;
        }
        iqInfo->arrCaliCfgs[i].caliItem = PTREE_ENUM_FROM_STR(MI_ISP_IQ_CaliItem_e, strItem);
        snprintf(iqInfo->arrCaliCfgs[i].caliFile, 64, "%s", strCaliFile);
        PTREE_DB_ProcessBack(db);
        PTREE_DBG("Cali item %s, file %s", strItem, strCaliFile);
    }
    snprintf(iqInfo->cus3aType, 16, "%s", str);

    return SSOS_DEF_OK;
}
static void _PTREE_SUR_IQ_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_IQ_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_IQ_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_IQ_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(IQ, PTREE_SUR_IQ_New);
