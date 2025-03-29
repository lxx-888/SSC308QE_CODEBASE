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
#include "ptree_sur_scl_stretch.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_SCL_STRETCH_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_SCL_STRETCH_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_SCL_STRETCH_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int _PTREE_SUR_SCL_STRETCH_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int _PTREE_SUR_SCL_STRETCH_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo,
                                           PTREE_DB_Obj_t db);
static int _PTREE_SUR_SCL_STRETCH_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo,
                                            PTREE_DB_Obj_t db);
static void _PTREE_SUR_SCL_STRETCH_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_SCL_STRETCH_OPS = {
    .occupyInfo    = _PTREE_SUR_SCL_STRETCH_OccupyInfo,
    .occupyInInfo  = _PTREE_SUR_SCL_STRETCH_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_SCL_STRETCH_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_SCL_STRETCH_LoadDb,
    .loadInDb      = _PTREE_SUR_SCL_STRETCH_LoadInDb,
    .loadOutDb     = _PTREE_SUR_SCL_STRETCH_LoadOutDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_SCL_STRETCH_HOOK = {
    .free = _PTREE_SUR_SCL_STRETCH_Free,
};

static PARENA_Tag_t *_PTREE_SUR_SCL_STRETCH_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_SCL_STRETCH_Info_t, base.base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_SCL_STRETCH_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_SCL_STRETCH_InInfo_t, base.base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_SCL_STRETCH_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_SCL_STRETCH_OutInfo_t, base.base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_SCL_STRETCH_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SCL_STRETCH_Info_t *sclInfo = CONTAINER_OF(sysInfo, PTREE_SUR_SCL_STRETCH_Info_t, base);
    (void)sysSur;

    sclInfo->u32HwPortMode = (unsigned int)PTREE_DB_GetInt(db, "PORT_MODE");
    sclInfo->u32OsdEn      = (unsigned int)PTREE_DB_GetInt(db, "OSD_EN");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_SCL_STRETCH_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo,
                                           PTREE_DB_Obj_t db)
{
    PTREE_SUR_SCL_STRETCH_InInfo_t *sclInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_SCL_STRETCH_InInfo_t, base);
    (void)sysSur;
    sclInInfo->u16CropX = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_X");
    sclInInfo->u16CropY = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_Y");
    sclInInfo->u16CropW = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_W");
    sclInInfo->u16CropH = (unsigned short)PTREE_DB_GetInt(db, "VID_CROP_H");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_SCL_STRETCH_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo,
                                            PTREE_DB_Obj_t db)
{
    PTREE_SUR_SCL_STRETCH_OutInfo_t *sclOutInfo = CONTAINER_OF(sysOutInfo, PTREE_SUR_SCL_STRETCH_OutInfo_t, base);
    const char *                     pOutFmt    = NULL;
    (void)sysSur;

    sclOutInfo->u32RowNum = (unsigned int)PTREE_DB_GetInt(db, "ROW_NUM");
    sclOutInfo->u32ColNum = (unsigned int)PTREE_DB_GetInt(db, "COL_NUM");

    pOutFmt = PTREE_DB_GetStr(db, "OUT_FMT");
    if (pOutFmt && strlen(pOutFmt) > 1)
    {
        sclOutInfo->u32VideoFormat = PTREE_ENUM_FROM_STR(MI_SYS_PixelFormat_e, pOutFmt);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_SCL_STRETCH_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_SCL_STRETCH_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_SCL_STRETCH_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_SCL_STRETCH_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(SCL_STRETCH, PTREE_SUR_SCL_STRETCH_New);
