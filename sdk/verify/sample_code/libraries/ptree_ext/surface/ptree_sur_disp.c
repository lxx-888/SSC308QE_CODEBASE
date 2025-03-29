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
#include "ptree_sur_disp.h"
#include "mi_common_datatype.h"
#include "ptree_sur_sys.h"
#include "ptree_enum.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_maker.h"

typedef struct PTREE_SUR_DISP_Obj_s PTREE_SUR_DISP_Obj_t;

struct PTREE_SUR_DISP_Obj_s
{
    PTREE_SUR_SYS_Obj_t base;
    unsigned int        layerPortId[PTREE_SUR_DISP_LAYER_MAX];
};

static PARENA_Tag_t *_PTREE_SUR_DISP_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_DISP_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_DISP_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_DISP_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_DISP_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_DISP_OPS = {
    .occupyInfo   = _PTREE_SUR_DISP_OccupyInfo,
    .occupyInInfo = _PTREE_SUR_DISP_OccupyInInfo,
    .loadDb       = _PTREE_SUR_DISP_LoadDb,
    .loadInDb     = _PTREE_SUR_DISP_LoadInDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_DISP_HOOK = {
    .free = _PTREE_SUR_DISP_Free,
};

static PARENA_Tag_t *_PTREE_SUR_DISP_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_DISP_Info_t, base.base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_DISP_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_DISP_InInfo_t, base.base, PTREE_SUR_InInfo_t);
}

int _PTREE_SUR_DISP_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_DISP_Obj_t * dispSur     = CONTAINER_OF(sysSur, PTREE_SUR_DISP_Obj_t, base);
    PTREE_SUR_DISP_Info_t *pstDispInfo = CONTAINER_OF(sysInfo, PTREE_SUR_DISP_Info_t, base);
    int                    s32Idx      = 0;
    char                   chLayerName[32];
    memset(dispSur->layerPortId, 0, sizeof(dispSur->layerPortId));
    snprintf(pstDispInfo->chDevType, 16, "%s", PTREE_DB_GetStr(db, "DEV_TYPE"));
    snprintf(pstDispInfo->chBackGroundColor, 16, "%s", PTREE_DB_GetStr(db, "BK_COLOR"));
    snprintf(pstDispInfo->chOutTiming, 16, "%s", PTREE_DB_GetStr(db, "DISP_OUT_TIMING"));
    pstDispInfo->uintLayerCount = (unsigned int)PTREE_DB_GetInt(db, "IN_LAYER_CNT");
    if (pstDispInfo->uintLayerCount >= PTREE_SUR_DISP_LAYER_MAX)
    {
        PTREE_ERR("layer count(%d) set over max support, please check\n", pstDispInfo->uintLayerCount);
        return SSOS_DEF_FAIL;
    }
    memset(pstDispInfo->chPnlLinkType, 0, 16);
    if (0 == strcmp("panel", pstDispInfo->chDevType))
    {
        PTREE_DB_ProcessKey(db, "MODE_PANEL_PARAM");
        snprintf(pstDispInfo->chPnlLinkType, 16, "%s", PTREE_DB_GetStr(db, "PNL_LINK_TYPE"));
        PTREE_DB_ProcessBack(db);
        PTREE_DBG("PNL_LINK_TYPE : %s\n", pstDispInfo->chPnlLinkType);
    }
    PTREE_DBG("DEV_TYPE : %s", pstDispInfo->chDevType);
    PTREE_DBG("BK_COLOR : %s", pstDispInfo->chBackGroundColor);
    PTREE_DBG("LAYER_CNT : %d", pstDispInfo->uintLayerCount);
    memset(&pstDispInfo->stDispLayerInfo, 0, 4 * sizeof(PTREE_SUR_DISP_LayerInfo_t));
    for (s32Idx = 0; s32Idx < pstDispInfo->uintLayerCount; s32Idx++)
    {
        snprintf(chLayerName, 32, "IN_LAYER_%d", s32Idx);
        PTREE_DB_ProcessKey(db, chLayerName);
        pstDispInfo->stDispLayerInfo[s32Idx].uintId         = PTREE_DB_GetInt(db, "LAYER_ID");
        pstDispInfo->stDispLayerInfo[s32Idx].uintRot        = PTREE_DB_GetInt(db, "LAYER_ROT");
        pstDispInfo->stDispLayerInfo[s32Idx].uintWidth      = PTREE_DB_GetInt(db, "LAYER_WIDTH");
        pstDispInfo->stDispLayerInfo[s32Idx].uintHeight     = PTREE_DB_GetInt(db, "LAYER_HEIGHT");
        pstDispInfo->stDispLayerInfo[s32Idx].uintDispWidth  = PTREE_DB_GetInt(db, "LAYER_DISP_WIDTH");
        pstDispInfo->stDispLayerInfo[s32Idx].uintDispHeight = PTREE_DB_GetInt(db, "LAYER_DISP_HEIGHT");
        pstDispInfo->stDispLayerInfo[s32Idx].uintDispXpos   = PTREE_DB_GetInt(db, "LAYER_DISP_XPOS");
        pstDispInfo->stDispLayerInfo[s32Idx].uintDispYpos   = PTREE_DB_GetInt(db, "LAYER_DISP_YPOS");
        PTREE_DB_ProcessBack(db);
        PTREE_DBG("Layer [%d]: canvas (%dx%d) - disp (%d, %d, %dx%d) - rot %d",
                  pstDispInfo->stDispLayerInfo[s32Idx].uintId, pstDispInfo->stDispLayerInfo[s32Idx].uintWidth,
                  pstDispInfo->stDispLayerInfo[s32Idx].uintHeight, pstDispInfo->stDispLayerInfo[s32Idx].uintDispXpos,
                  pstDispInfo->stDispLayerInfo[s32Idx].uintDispYpos, pstDispInfo->stDispLayerInfo[s32Idx].uintDispWidth,
                  pstDispInfo->stDispLayerInfo[s32Idx].uintDispHeight, pstDispInfo->stDispLayerInfo[s32Idx].uintRot);
    }
    return SSOS_DEF_OK;
}

static int _PTREE_SUR_DISP_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_DISP_Obj_t *   dispSur    = CONTAINER_OF(sysSur, PTREE_SUR_DISP_Obj_t, base);
    PTREE_SUR_DISP_InInfo_t *dispInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_DISP_InInfo_t, base);

    dispInInfo->uintLayId = PTREE_DB_GetInt(db, "IN_LAYER_ID");
    if (dispInInfo->uintLayId >= PTREE_SUR_DISP_LAYER_MAX)
    {
        PTREE_ERR("Layer [%d] is out of range [0, %d)", dispInInfo->uintLayId, PTREE_SUR_DISP_LAYER_MAX);
        return SSOS_DEF_FAIL;
    }
    dispInInfo->uintSrcWidth  = PTREE_DB_GetInt(db, "SRC_WIDTH");
    dispInInfo->uintSrcHeight = PTREE_DB_GetInt(db, "SRC_HEIGHT");
    dispInInfo->uintDstWidth  = PTREE_DB_GetInt(db, "DST_WIDTH");
    dispInInfo->uintDstHeight = PTREE_DB_GetInt(db, "DST_HEIGHT");
    dispInInfo->uintDstXpos   = PTREE_DB_GetInt(db, "DST_XPOS");
    dispInInfo->uintDstYpos   = PTREE_DB_GetInt(db, "DST_YPOS");
    dispInInfo->uintSysChn    = PTREE_DB_GetInt(db, "SYS_CHN");
    dispInInfo->uintLayPortId = dispSur->layerPortId[dispInInfo->uintLayId]++;
    PTREE_DBG("Layer [%d], port [%d]: chn %d, src (%dx%d), dst (%d, %d, %dx%d)", dispInInfo->uintLayId,
              dispInInfo->base.base.port, dispInInfo->uintSysChn, dispInInfo->uintSrcWidth, dispInInfo->uintSrcHeight,
              dispInInfo->uintDstXpos, dispInInfo->uintDstYpos, dispInInfo->uintDstWidth, dispInInfo->uintDstHeight);
    return SSOS_DEF_OK;
}

static void _PTREE_SUR_DISP_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_DISP_New(void)
{
    PTREE_SUR_DISP_Obj_t *dispSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_DISP_Obj_t));
    if (!dispSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(dispSur, 0, sizeof(PTREE_SUR_DISP_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(&dispSur->base, &G_PTREE_SUR_DISP_OPS))
    {
        SSOS_MEM_Free(dispSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(&dispSur->base, &G_PTREE_SUR_DISP_HOOK);
    return &dispSur->base.base;
}

PTREE_MAKER_SUR_INIT(DISP, PTREE_SUR_DISP_New);
