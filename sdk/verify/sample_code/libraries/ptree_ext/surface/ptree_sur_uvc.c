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
#include "ptree_sur_uvc.h"
#include "ptree_maker.h"

static PARENA_Tag_t *_PTREE_SUR_UVC_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_UVC_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_UVC_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_UVC_OPS = {
    .occupyInfo = _PTREE_SUR_UVC_OccupyInfo,
    .loadDb     = _PTREE_SUR_UVC_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_UVC_HOOK = {
    .free = _PTREE_SUR_UVC_Free,
};
static PARENA_Tag_t *_PTREE_SUR_UVC_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_UVC_Info_t, base.base, PTREE_SUR_Info_t);
}
static int _PTREE_SUR_UVC_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_UVC_Info_t *uvcInfo = CONTAINER_OF(sysInfo, PTREE_SUR_UVC_Info_t, base);
    (void)sysSur;
    (void)db;
    (void)uvcInfo;

    return SSOS_DEF_OK;
}

static void _PTREE_SUR_UVC_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}
PTREE_SUR_Obj_t *PTREE_SUR_UVC_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_UVC_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_UVC_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(UVC, PTREE_SUR_UVC_New);
