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
#include "ptree_enum.h"
#include "ptree_sur_sys.h"
#include "ptree_sur_vif.h"
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
#define CHECK_INTVALUE(value) (value == -1 ? 0 : value)

static PARENA_Tag_t *_PTREE_SUR_VIF_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_VIF_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_VIF_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_VIF_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo,
                                     PTREE_DB_Obj_t db);
static void _PTREE_SUR_VIF_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_VIF_OPS = {
    .occupyInfo    = _PTREE_SUR_VIF_OccupyInfo,
    .occupyOutInfo = _PTREE_SUR_VIF_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_VIF_LoadDb,
    .loadOutDb     = _PTREE_SUR_VIF_LoadOutDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_VIF_HOOK = {
    .free = _PTREE_SUR_VIF_Free,
};

static PARENA_Tag_t *_PTREE_SUR_VIF_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_VIF_Info_t, base.base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_VIF_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_VIF_OutInfo_t, base.base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_VIF_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_VIF_Info_t *vifInfo = CONTAINER_OF(sysInfo, PTREE_SUR_VIF_Info_t, base);
    (void)sysSur;

    vifInfo->s32HdrType         = CHECK_INTVALUE(PTREE_DB_GetInt(db, "HDR_TYPE"));
    vifInfo->s32SensorId        = CHECK_INTVALUE(PTREE_DB_GetInt(db, "SNR_ID"));
    vifInfo->s32HdrExposureMask = CHECK_INTVALUE(PTREE_DB_GetInt(db, "HDR_EXPOSURE_MASK"));
    vifInfo->s32WorkMode        = CHECK_INTVALUE(PTREE_DB_GetInt(db, "WORK_MOD"));
    vifInfo->u32StitchMask      = CHECK_INTVALUE(PTREE_DB_GetInt(db, "GROUP_STITCH_MASK"));
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_VIF_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_VIF_OutInfo_t *vifOutInfo = CONTAINER_OF(sysOutInfo, PTREE_SUR_VIF_OutInfo_t, base);
    const char *             pOutFmt    = NULL;
    const char *             pBayerId   = NULL;
    const char *             pPrecision = NULL;
    (void)sysSur;
    vifOutInfo->s32IsUseSnrFmt  = CHECK_INTVALUE(PTREE_DB_GetInt(db, "USE_SNR_FMT"));
    vifOutInfo->s32CompressMode = CHECK_INTVALUE(PTREE_DB_GetInt(db, "COMPRESS_MODE"));
    vifOutInfo->u32CropX        = CHECK_INTVALUE(PTREE_DB_GetInt(db, "VID_CROP_X"));
    vifOutInfo->u32CropY        = CHECK_INTVALUE(PTREE_DB_GetInt(db, "VID_CROP_Y"));
    vifOutInfo->u32CropW        = CHECK_INTVALUE(PTREE_DB_GetInt(db, "VID_CROP_W"));
    vifOutInfo->u32CropH        = CHECK_INTVALUE(PTREE_DB_GetInt(db, "VID_CROP_H"));
    PTREE_DBG("u32CropX:%d intCropY:%d intCropW:%d intCropH:%d", vifOutInfo->u32CropX, vifOutInfo->u32CropY,
              vifOutInfo->u32CropW, vifOutInfo->u32CropH);
    if (!vifOutInfo->s32IsUseSnrFmt)
    {
        vifOutInfo->u32Width  = CHECK_INTVALUE(PTREE_DB_GetInt(db, "VID_W"));
        vifOutInfo->u32Height = CHECK_INTVALUE(PTREE_DB_GetInt(db, "VID_H"));
        pOutFmt               = PTREE_DB_GetStr(db, "OUT_FMT");
        if (0 == strcmp(pOutFmt, "bayer"))
        {
            PTREE_DB_ProcessKey(db, "BAYER_PARAM");
            pBayerId            = PTREE_DB_GetStr(db, "BAYER_ID");
            pPrecision          = PTREE_DB_GetStr(db, "PRECISION");
            vifOutInfo->eOutFmt = RGB_BAYER_PIXEL(PTREE_ENUM_FROM_STR(MI_SYS_DataPrecision_e, pPrecision),
                                                  PTREE_ENUM_FROM_STR(MI_SYS_BayerId_e, pBayerId));
            PTREE_DB_ProcessBack(db);
        }
        else if (pOutFmt && strlen(pOutFmt) > 1)
        {
            vifOutInfo->eOutFmt = PTREE_ENUM_FROM_STR(MI_SYS_PixelFormat_e, pOutFmt);
        }
    }
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_VIF_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_VIF_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_VIF_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_VIF_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(VIF, PTREE_SUR_VIF_New);
