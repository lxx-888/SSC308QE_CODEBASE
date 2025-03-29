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

#include "ssos_io.h"
#include "mi_venc_datatype.h"
#include "ptree_log.h"
#include "ptree_sur_venc.h"
#include "ptree_enum.h"
#include "ptree_packet.h"
#include "ptree_packet_video.h"
#include "ssos_list.h"
#include "ssos_mem.h"
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

static PARENA_Tag_t *_PTREE_SUR_VENC_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_VENC_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_VENC_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_VENC_OPS = {
    .occupyInfo = _PTREE_SUR_VENC_OccupyInfo,
    .loadDb     = _PTREE_SUR_VENC_LoadDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_VENC_HOOK = {
    .free = _PTREE_SUR_VENC_Free,
};

static PARENA_Tag_t *_PTREE_SUR_VENC_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_VENC_Info_t, base.base, PTREE_SUR_Info_t);
}
static int _PTREE_SUR_VENC_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(sysInfo, PTREE_SUR_VENC_Info_t, base);
    const char *           str      = NULL;
    (void)sysSur;
    vencInfo->u32MaxWidth  = (unsigned int)PTREE_DB_GetInt(db, "MAX_STREAM_W");
    vencInfo->u32MaxHeight = (unsigned int)PTREE_DB_GetInt(db, "MAX_STREAM_H");
    vencInfo->u32Width     = (unsigned int)PTREE_DB_GetInt(db, "STREAM_W");
    vencInfo->u32Height    = (unsigned int)PTREE_DB_GetInt(db, "STREAM_H");

    /*Max strem cnt enable*/
    vencInfo->u8StreamCntEnable = (unsigned int)PTREE_DB_GetInt(db, "STREAM_CNT_EN");
    if (vencInfo->u8StreamCntEnable)
    {
        vencInfo->u32MaxStreamCnt = (unsigned int)PTREE_DB_GetInt(db, "MAX_STREAM_CNT");
    }
    str = PTREE_DB_GetStr(db, "EN_TYPE");
    if (str && strlen(str) > 1)
    {
        vencInfo->pEncodeType = PTREE_ENUM_FROM_STR(PTREE_PACKET_VIDEO_Fmt_e, str);
        str                   = NULL;
    }

    vencInfo->u32EncodeFps   = (unsigned int)PTREE_DB_GetInt(db, "EN_FPS");
    vencInfo->u32MultiSlice  = (unsigned int)PTREE_DB_GetInt(db, "MULTI_SLICE");
    vencInfo->u32SliceRowCnt = (unsigned int)PTREE_DB_GetInt(db, "SLICE_ROW_CNT");

    /*rc mode & pstParam*/
    str = PTREE_DB_GetStr(db, "RC_MODE");
    snprintf(vencInfo->pRcMode, 16, "%s", str);
    PTREE_DB_ProcessKey(db, "RC_MODE_PARAM");
    if (!strcmp(vencInfo->pRcMode, "cbr"))
    {
        vencInfo->stCbrCfg.u32BitRate = (unsigned int)PTREE_DB_GetInt(db, "BIT_RATE");
        vencInfo->stCbrCfg.u32Gop     = (unsigned int)PTREE_DB_GetInt(db, "GOP");
    }
    else if (!strcmp(vencInfo->pRcMode, "vbr"))
    {
        vencInfo->stVbrCfg.u32BitRate = (unsigned int)PTREE_DB_GetInt(db, "BIT_RATE");
        vencInfo->stVbrCfg.u32Gop     = (unsigned int)PTREE_DB_GetInt(db, "GOP");
        vencInfo->stVbrCfg.u32MinQp   = (unsigned int)PTREE_DB_GetInt(db, "MIN_QP");
        vencInfo->stVbrCfg.u32MaxQp   = (unsigned int)PTREE_DB_GetInt(db, "MAX_QP");
    }
    else if (!strcmp(vencInfo->pRcMode, "fixqp"))
    {
        vencInfo->stFixQpCfg.u32Gop     = (unsigned int)PTREE_DB_GetInt(db, "GOP");
        vencInfo->stFixQpCfg.u32IQp     = (unsigned int)PTREE_DB_GetInt(db, "IQP");
        vencInfo->stFixQpCfg.u32PQp     = (unsigned int)PTREE_DB_GetInt(db, "PQP");
        vencInfo->stFixQpCfg.u32Qfactor = (unsigned int)PTREE_DB_GetInt(db, "QFACTOR");
    }
    else if (!strcmp(vencInfo->pRcMode, "avbr"))
    {
        vencInfo->stAvbrCfg.u32BitRate = (unsigned int)PTREE_DB_GetInt(db, "BIT_RATE");
        vencInfo->stAvbrCfg.u32Gop     = (unsigned int)PTREE_DB_GetInt(db, "GOP");
        vencInfo->stAvbrCfg.u32MinQp   = (unsigned int)PTREE_DB_GetInt(db, "MIN_QP");
        vencInfo->stAvbrCfg.u32MaxQp   = (unsigned int)PTREE_DB_GetInt(db, "MAX_QP");
    }
    else
    {
        PTREE_ERR("RC Mode error!");
        PTREE_DB_ProcessBack(db);
        return -1;
    }
    PTREE_DB_ProcessBack(db);

    vencInfo->bYuvEnable = (unsigned char)PTREE_DB_GetInt(db, "YUV_EN");
    if (vencInfo->bYuvEnable)
    {
        PTREE_DB_ProcessKey(db, "YUV_PARAM");
        vencInfo->u32YuvWidth  = (unsigned int)PTREE_DB_GetInt(db, "VID_W");
        vencInfo->u32YuvHeight = (unsigned int)PTREE_DB_GetInt(db, "VID_H");
        PTREE_DB_ProcessBack(db);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_VENC_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_VENC_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_VENC_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_VENC_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(VENC, PTREE_SUR_VENC_New);
