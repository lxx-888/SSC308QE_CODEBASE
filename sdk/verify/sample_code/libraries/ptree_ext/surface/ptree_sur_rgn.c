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
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_enum.h"
#include "ssos_list.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_rgn.h"
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

PTREE_ENUM_DEFINE(MI_ModuleId_e,                 /* MI_ModuleId_e */
                  {E_MI_MODULE_ID_MAX, "NA"},    /* NA */
                  {E_MI_MODULE_ID_SCL, "SCL"},   /* SCL */
                  {E_MI_MODULE_ID_VENC, "VENC"}, /* VENC */
                  {E_MI_MODULE_ID_DISP, "DISP"}, /* DISP */
);

PTREE_ENUM_DEFINE(PTREE_SUR_RGN_InType_e,                                     /* PTREE_SUR_RGN_InType_e */
                  {E_PTREE_SUR_RGN_IN_MODE_FRAME, "frame"},                   /* FRAME */
                  {E_PTREE_SUR_RGN_IN_MODE_OSD_FRAME, "osd_frame"},           /* OSD FRAME */
                  {E_PTREE_SUR_RGN_IN_MODE_OSD_DOT_MATRIX, "osd_dot_matrix"}, /* OSD DOT MATRIX */
)

PTREE_ENUM_DEFINE(PTREE_SUR_RGN_Thickness_e,                    /* PTREE_SUR_RGN_Thickness_e */
                  {E_PTREE_SUR_RGN_THICKNESS_NORMAL, "normal"}, /* NORMAL */
                  {E_PTREE_SUR_RGN_THICKNESS_THIN, "thin"},     /* THIN */
                  {E_PTREE_SUR_RGN_THICKNESS_THICK, "thick"},   /* THICK */
)

PTREE_ENUM_DEFINE(PTREE_SUR_RGN_Size_e,                    /* PTREE_SUR_RGN_Size_e */
                  {E_PTREE_SUR_RGN_SIZE_NORMAL, "normal"}, /* NORMAL */
                  {E_PTREE_SUR_RGN_SIZE_TINY, "tiny"},     /* TINY */
                  {E_PTREE_SUR_RGN_SIZE_SMALL, "small"},   /* SMALL */
                  {E_PTREE_SUR_RGN_SIZE_LARGE, "large"},   /* LARGE */
                  {E_PTREE_SUR_RGN_SIZE_HUGE, "huge"},     /* HUGE */
)

PTREE_ENUM_DEFINE(MI_RGN_PixelFormat_e,                         /* MI_RGN_PixelFormat_e */
                  {E_MI_RGN_PIXEL_FORMAT_MAX, "NA"},            /* MAX */
                  {E_MI_RGN_PIXEL_FORMAT_ARGB1555, "argb1555"}, /* ARGB1555 */
                  {E_MI_RGN_PIXEL_FORMAT_ARGB4444, "argb4444"}, /* ARGB4444 */
                  {E_MI_RGN_PIXEL_FORMAT_I2, "i2"},             /* I2 */
                  {E_MI_RGN_PIXEL_FORMAT_I4, "i4"},             /* I4 */
                  {E_MI_RGN_PIXEL_FORMAT_I8, "i8"},             /* I8 */
                  {E_MI_RGN_PIXEL_FORMAT_RGB565, "rgb565"},     /* RGB565 */
                  {E_MI_RGN_PIXEL_FORMAT_ARGB8888, "argb8888"}, /* ARGB8888 */
)

static PARENA_Tag_t *_PTREE_SUR_RGN_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_RGN_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_RGN_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_RGN_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
static void _PTREE_SUR_RGN_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_RGN_OPS = {
    .occupyInfo   = _PTREE_SUR_RGN_OccupyInfo,
    .occupyInInfo = _PTREE_SUR_RGN_OccupyInInfo,
    .loadDb       = _PTREE_SUR_RGN_LoadDb,
    .loadInDb     = _PTREE_SUR_RGN_LoadInDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_RGN_HOOK = {
    .free = _PTREE_SUR_RGN_Free,
};

static unsigned int _PTREE_SUR_RGN_SplitFillTokens(const char *inputChar, char delimiter, char tokens[][8],
                                                   unsigned int maxTokens)
{
    unsigned int tokenCount = 0;
    unsigned int tokenIndex = 0;
    unsigned int i          = 0;
    for (i = 0; inputChar[i] != '\0' && tokenCount < maxTokens; i++)
    {
        if (inputChar[i] == delimiter)
        {
            tokenCount++;
            tokenIndex = 0;
        }
        else
        {
            tokens[tokenCount][tokenIndex] = inputChar[i];
            tokenIndex++;
        }
    }
    tokenCount++;

    return tokenCount;
}
static PARENA_Tag_t *_PTREE_SUR_RGN_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_RGN_Info_t, base.base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_RGN_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_RGN_InInfo_t, base.base, PTREE_SUR_InInfo_t);
}
static int _PTREE_SUR_RGN_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_RGN_Info_t *rgnInfo = CONTAINER_OF(sysInfo, PTREE_SUR_RGN_Info_t, base);

    int i = 0;
    (void)sysSur;

    rgnInfo->attachCnt = PTREE_DB_GetInt(db, "ATTACH_COUNT");
    rgnInfo->attachCnt = rgnInfo->attachCnt > PTREE_SUR_RGN_ATTACH_MAX ? PTREE_SUR_RGN_ATTACH_MAX : rgnInfo->attachCnt;
    for (i = 0; i < rgnInfo->attachCnt; ++i)
    {
        unsigned int tokenCount;
        char         tokens[4][8];
        const char * attachModule  = NULL;
        char         attachKey[32] = "";
        snprintf(attachKey, 32, "ATTACH_%d", i);
        PTREE_DB_ProcessKey(db, attachKey);
        attachModule = PTREE_DB_GetStr(db, "MODULE");
        if (!attachModule || !strlen(attachModule))
        {
            PTREE_DB_ProcessBack(db);
            PTREE_ERR("RGN Attach %d MODULE Err", i);
            continue;
        }
        memset(tokens, 0, sizeof(tokens));
        tokenCount = _PTREE_SUR_RGN_SplitFillTokens(attachModule, '_', tokens, sizeof(tokens) / sizeof(tokens[0]));
        if (tokenCount < 3)
        {
            PTREE_DB_ProcessBack(db);
            PTREE_ERR("RGN Attach %d MODULE %s Err", i, attachModule);
            continue;
        }
        rgnInfo->astAttachInfo[i].stChnPort.eModId     = PTREE_ENUM_FROM_STR(MI_ModuleId_e, tokens[0]);
        rgnInfo->astAttachInfo[i].stChnPort.s32DevId   = SSOS_IO_Atoi(tokens[1]);
        rgnInfo->astAttachInfo[i].stChnPort.s32ChnId   = SSOS_IO_Atoi(tokens[2]);
        rgnInfo->astAttachInfo[i].stChnPort.s32PortId  = PTREE_DB_GetInt(db, "PORT");
        rgnInfo->astAttachInfo[i].stChnPort.bInputPort = PTREE_DB_GetInt(db, "IS_INPORT");
        rgnInfo->astAttachInfo[i].timingW              = PTREE_DB_GetInt(db, "TIMING_W");
        rgnInfo->astAttachInfo[i].timingH              = PTREE_DB_GetInt(db, "TIMING_H");
        PTREE_DB_ProcessBack(db);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_RGN_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_RGN_InInfo_t *rgnInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_RGN_InInfo_t, base);

    (void)sysSur;

    rgnInInfo->mode = PTREE_ENUM_FROM_STR(PTREE_SUR_RGN_InType_e, PTREE_DB_GetStr(db, "MODE"));
    PTREE_DB_ProcessKey(db, "MODE_PARAM");
    if (E_PTREE_SUR_RGN_IN_MODE_FRAME == rgnInInfo->mode)
    {
        rgnInInfo->info.stFrameInfo.color = PTREE_DB_GetInt(db, "COLOR");
        rgnInInfo->info.stFrameInfo.thickness =
            PTREE_ENUM_FROM_STR(PTREE_SUR_RGN_Thickness_e, PTREE_DB_GetStr(db, "THICKNESS"));
    }
    else if (E_PTREE_SUR_RGN_IN_MODE_OSD_FRAME == rgnInInfo->mode)
    {
        rgnInInfo->info.stOsdFrameInfo.ePixelFormat =
            PTREE_ENUM_FROM_STR(MI_RGN_PixelFormat_e, PTREE_DB_GetStr(db, "PIXEL_FORMAT"));
        rgnInInfo->info.stOsdFrameInfo.color = PTREE_DB_GetInt(db, "COLOR");
        rgnInInfo->info.stOsdFrameInfo.thickness =
            PTREE_ENUM_FROM_STR(PTREE_SUR_RGN_Thickness_e, PTREE_DB_GetStr(db, "THICKNESS"));
    }
    else if (E_PTREE_SUR_RGN_IN_MODE_OSD_DOT_MATRIX == rgnInInfo->mode)
    {
        rgnInInfo->info.stOsdDotMatrixInfo.ePixelFormat =
            PTREE_ENUM_FROM_STR(MI_RGN_PixelFormat_e, PTREE_DB_GetStr(db, "PIXEL_FORMAT"));
        rgnInInfo->info.stOsdDotMatrixInfo.color = PTREE_DB_GetInt(db, "COLOR");
        rgnInInfo->info.stOsdDotMatrixInfo.size =
            PTREE_ENUM_FROM_STR(PTREE_SUR_RGN_Size_e, PTREE_DB_GetStr(db, "SIZE"));
    }
    PTREE_DB_ProcessBack(db);
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_RGN_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_RGN_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_RGN_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_RGN_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(RGN, PTREE_SUR_RGN_New);
