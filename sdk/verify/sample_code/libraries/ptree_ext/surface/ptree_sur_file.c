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
#include "ssos_list.h"
#include "ssos_io.h"
#include "ptree_log.h"
#include "ptree_maker.h"
#include "ssos_mem.h"

#include "ptree_sur.h"
#include "ptree_sur_file.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_FILE_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_FILE_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_FILE_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *info, PTREE_DB_Obj_t db);
static int           _PTREE_SUR_FILE_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *info, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_FILE_Free(PTREE_SUR_Obj_t *sur);

static const PTREE_SUR_Ops_t G_PTREE_SUR_FILE_OPS = {
    .occupyInInfo  = _PTREE_SUR_FILE_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_FILE_OccupyOutInfo,
    .loadInDb      = _PTREE_SUR_FILE_LoadInDb,
    .loadOutDb     = _PTREE_SUR_FILE_LoadOutDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_FILE_HOOK = {
    .free = _PTREE_SUR_FILE_Free,
};

static PARENA_Tag_t *_PTREE_SUR_FILE_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_FILE_InInfo_t, base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_FILE_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    (void)sur;
    return PARENA_GET(pArena, PTREE_SUR_FILE_OutInfo_t, base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_FILE_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_FILE_InInfo_t *fileInInfo = CONTAINER_OF(info, PTREE_SUR_FILE_InInfo_t, base);
    (void)sur;
    snprintf(fileInInfo->fileName, 256, "%s", PTREE_DB_GetStr(db, "FILE_WRITE_PATH"));
    fileInInfo->frameCntLimit = PTREE_DB_GetInt(db, "FRAME_CNT_LIMIT");
    fileInInfo->bAddHead      = (unsigned char)PTREE_DB_GetInt(db, "HEAD");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_FILE_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_FILE_OutInfo_t *fileOutInfo = CONTAINER_OF(info, PTREE_SUR_FILE_OutInfo_t, base);
    (void)sur;
    snprintf(fileOutInfo->fileName, 256, "%s", PTREE_DB_GetStr(db, "FILE_READ_PATH"));
    fileOutInfo->frameCntLimit = PTREE_DB_GetInt(db, "FRAME_CNT_LIMIT");
    snprintf(fileOutInfo->outType, 16, "%s", PTREE_DB_GetStr(db, "OUT_TYPE"));
    if (!strcmp(fileOutInfo->outType, "raw") || !strcmp(fileOutInfo->outType, "video"))
    {
        if (!strcmp(fileOutInfo->outType, "raw"))
        {
            fileOutInfo->planeNum = (unsigned char)PTREE_DB_GetInt(db, "VID_PLANE_NUM");
        }
        fileOutInfo->videoWidth  = (unsigned short)PTREE_DB_GetInt(db, "VID_W");
        fileOutInfo->videoHeight = (unsigned short)PTREE_DB_GetInt(db, "VID_H");
    }
    snprintf(fileOutInfo->outFmt, 16, "%s", PTREE_DB_GetStr(db, "OUT_FMT"));
    if (0 == strcmp(fileOutInfo->outFmt, "bayer"))
    {
        PTREE_DB_ProcessKey(db, "BAYER_PARAM");
        snprintf(fileOutInfo->bayerId, 16, "%s", PTREE_DB_GetStr(db, "BAYER_ID"));
        snprintf(fileOutInfo->precision, 16, "%s", PTREE_DB_GetStr(db, "PRECISION"));
        PTREE_DB_ProcessBack(db);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_FILE_Free(PTREE_SUR_Obj_t *sur)
{
    SSOS_MEM_Free(sur);
}

PTREE_SUR_Obj_t *PTREE_SUR_FILE_New(void)
{
    PTREE_SUR_Obj_t *sur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_Obj_t));
    if (!sur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sur, 0, sizeof(PTREE_SUR_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_Init(sur, &G_PTREE_SUR_FILE_OPS))
    {
        SSOS_MEM_Free(sur);
        return NULL;
    }
    PTREE_SUR_Register(sur, &G_PTREE_SUR_FILE_HOOK);
    return sur;
}

PTREE_MAKER_SUR_INIT(FILE, PTREE_SUR_FILE_New);
