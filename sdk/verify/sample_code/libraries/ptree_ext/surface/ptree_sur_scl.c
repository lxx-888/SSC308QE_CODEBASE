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
#include "ssos_mem.h"
#include "ptree_log.h"
#include "mi_common_datatype.h"
#include "ptree_sur_sys.h"
#include "ptree_enum.h"
#include "ptree_packet.h"
#include "ptree_maker.h"
#include "ptree_sur_scl.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_SCL_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_SCL_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_SCL_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_SCL_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_SCL_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_SCL_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo,
                                     PTREE_DB_Obj_t db);
static void _PTREE_SUR_SCL_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_SCL_OPS = {
    .occupyInfo    = _PTREE_SUR_SCL_OccupyInfo,
    .occupyInInfo  = _PTREE_SUR_SCL_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_SCL_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_SCL_LoadDb,
    .loadInDb      = _PTREE_SUR_SCL_LoadInDb,
    .loadOutDb     = _PTREE_SUR_SCL_LoadOutDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_SCL_HOOK = {
    .free = _PTREE_SUR_SCL_Free,
};

static PARENA_Tag_t *_PTREE_SUR_SCL_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_SCL_Info_t, base.base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_SCL_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_SCL_InInfo_t, base.base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_SCL_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_SCL_OutInfo_t, base.base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_SCL_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SCL_Info_t *sclInfo = CONTAINER_OF(sysInfo, PTREE_SUR_SCL_Info_t, base);
    (void)sysSur;

    sclInfo->u32HwPortMode = (unsigned int)PTREE_DB_GetInt(db, "PORT_MODE");
    sclInfo->u32Rotation   = (unsigned int)PTREE_DB_GetInt(db, "ROT");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_SCL_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SCL_InInfo_t *sclInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_SCL_InInfo_t, base);
    (void)sysSur;
    sclInInfo->u16CropX = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_X");
    sclInInfo->u16CropY = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_Y");
    sclInInfo->u16CropW = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_W");
    sclInInfo->u16CropH = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_H");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_SCL_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SCL_OutInfo_t *sclOutInfo = CONTAINER_OF(sysOutInfo, PTREE_SUR_SCL_OutInfo_t, base);
    const char *             pOutFmt    = NULL;
    (void)sysSur;

    sclOutInfo->u16CropX        = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_X");
    sclOutInfo->u16CropY        = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_Y");
    sclOutInfo->u16CropW        = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_W");
    sclOutInfo->u16CropH        = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_H");
    sclOutInfo->bMirror         = (unsigned char)PTREE_DB_GetInt(db, "IS_MIRROR");
    sclOutInfo->bFlip           = (unsigned char)PTREE_DB_GetInt(db, "IS_FLIP");
    sclOutInfo->u32CompressMode = PTREE_DB_GetInt(db, "COMPRESS_MODE");
    sclOutInfo->u16Width        = (unsigned short)PTREE_DB_GetInt(db, "VID_W");
    sclOutInfo->u16Height       = (unsigned short)PTREE_DB_GetInt(db, "VID_H");

    pOutFmt = PTREE_DB_GetStr(db, "OUT_FMT");
    if (pOutFmt && strlen(pOutFmt) > 1)
    {
        sclOutInfo->u32VideoFormat = PTREE_ENUM_FROM_STR(MI_SYS_PixelFormat_e, pOutFmt);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_SCL_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_SCL_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_SCL_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_SCL_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(SCL, PTREE_SUR_SCL_New);
