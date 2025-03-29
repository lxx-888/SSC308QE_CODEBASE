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
#include "ssos_io.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_enum.h"
#include "ptree_sur.h"
#include "ptree_sur_stdio.h"
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

PTREE_ENUM_DEFINE(PTREE_SUR_STDIO_OutMode_e,                                     // PTREE_SUR_STDIO_OutMode_e
                  {E_PTREE_SUR_STDIO_OUT_MODE_META_RGN_FRAME, "meta_rgn_frame"}, // META_RGN_FRAME
                  {E_PTREE_SUR_STDIO_OUT_MODE_RAW_VIDEO, "raw_video"},           // RAW_VIDEO
)

PTREE_ENUM_DEFINE(PTREE_SUR_STDIO_MetaRgnFrameLayout_e,                       // PTREE_SUR_STDIO_MetaRgnFrameLayout_e
                  {E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_GRID, "grid"},     // grid
                  {E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_STACK, "stack"},   // stack
                  {E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_PAGODA, "pagoda"}, // pagoda
                  {E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_RANDOM, "random"}, // random
)

static PARENA_Tag_t *_PTREE_SUR_STDIO_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_STDIO_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *info, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_STDIO_Free(PTREE_SUR_Obj_t *sur);
static const PTREE_SUR_Ops_t G_PTREE_SUR_STDIO_OPS = {
    .occupyOutInfo = _PTREE_SUR_STDIO_OccupyOutInfo,
    .loadOutDb     = _PTREE_SUR_STDIO_LoadOutDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_STDIO_HOOK = {
    .free = _PTREE_SUR_STDIO_Free,
};
static PARENA_Tag_t *_PTREE_SUR_STDIO_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_STDIO_OutInfo_t, base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_STDIO_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_STDIO_OutInfo_t *stdioInfo = CONTAINER_OF(info, PTREE_SUR_STDIO_OutInfo_t, base);
    (void)sur;
    stdioInfo->info.isUserTrigger = PTREE_DB_GetInt(db, "USER_TIRGGER");
    stdioInfo->info.mode          = PTREE_ENUM_FROM_STR(PTREE_SUR_STDIO_OutMode_e, PTREE_DB_GetStr(db, "MODE"));
    PTREE_DB_ProcessKey(db, "MODE_PARAM");
    if (E_PTREE_SUR_STDIO_OUT_MODE_META_RGN_FRAME == stdioInfo->info.mode)
    {
        stdioInfo->info.metaRgnFrameInfo.number = PTREE_DB_GetInt(db, "NUMBER");
        stdioInfo->info.metaRgnFrameInfo.layout =
            PTREE_ENUM_FROM_STR(PTREE_SUR_STDIO_MetaRgnFrameLayout_e, PTREE_DB_GetStr(db, "LAYOUT"));
    }
    else if (E_PTREE_SUR_STDIO_OUT_MODE_RAW_VIDEO == stdioInfo->info.mode)
    {
        stdioInfo->info.rawVideoInfo.rawData[0] = PTREE_DB_GetInt(db, "RAW_DATA_0");
        stdioInfo->info.rawVideoInfo.rawData[1] = PTREE_DB_GetInt(db, "RAW_DATA_1");
        stdioInfo->info.rawVideoInfo.rawInfo.fmt =
            PTREE_ENUM_FROM_STR(PTREE_PACKET_RAW_VideoFmt_e, PTREE_DB_GetStr(db, "FMT"));
        stdioInfo->info.rawVideoInfo.rawInfo.width  = PTREE_DB_GetInt(db, "VID_W");
        stdioInfo->info.rawVideoInfo.rawInfo.height = PTREE_DB_GetInt(db, "VID_H");
    }
    PTREE_DB_ProcessBack(db);
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_STDIO_Free(PTREE_SUR_Obj_t *sur)
{
    SSOS_MEM_Free(sur);
}

PTREE_SUR_Obj_t *PTREE_SUR_STDIO_New(void)
{
    PTREE_SUR_Obj_t *sur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sur, &G_PTREE_SUR_STDIO_OPS))
    {
        SSOS_MEM_Free(sur);
        return NULL;
    }
    PTREE_SUR_Register(sur, &G_PTREE_SUR_STDIO_HOOK);
    return sur;
}

PTREE_MAKER_SUR_INIT(STDIO, PTREE_SUR_STDIO_New);
