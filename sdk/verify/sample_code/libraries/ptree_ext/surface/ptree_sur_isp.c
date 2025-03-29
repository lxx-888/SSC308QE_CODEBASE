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
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_maker.h"
#include "ptree_sur_isp.h"
#include "ptree_sur_sys.h"
#include "mi_common_datatype.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_ISP_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_ISP_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_ISP_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_ISP_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_ISP_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_ISP_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo,
                                     PTREE_DB_Obj_t db);
static void _PTREE_SUR_ISP_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_ISP_OPS = {
    .occupyInfo    = _PTREE_SUR_ISP_OccupyInfo,
    .occupyInInfo  = _PTREE_SUR_ISP_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_ISP_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_ISP_LoadDb,
    .loadInDb      = _PTREE_SUR_ISP_LoadInDb,
    .loadOutDb     = _PTREE_SUR_ISP_LoadOutDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_ISP_HOOK = {
    .free = _PTREE_SUR_ISP_Free,
};

static PARENA_Tag_t *_PTREE_SUR_ISP_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_ISP_Info_t, base.base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_ISP_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_ISP_InInfo_t, base.base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_ISP_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_ISP_OutInfo_t, base.base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_ISP_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_ISP_Info_t *ispInfo = CONTAINER_OF(sysInfo, PTREE_SUR_ISP_Info_t, base);
    const char *          str     = NULL;
    unsigned int          u32Idx  = 0;
    char                  strTableName[16];
    (void)sysSur;

    ispInfo->u32HdrType         = PTREE_DB_GetInt(db, "HDR_TYPE");
    ispInfo->u32HdrFusionType   = PTREE_DB_GetInt(db, "HDR_FUSION_TYPE");
    ispInfo->u32HdrExposureMask = PTREE_DB_GetInt(db, "HDR_EXPOSURE_MASK");
    ispInfo->u32SnrMask         = PTREE_DB_GetInt(db, "SNR_MASK");
    ispInfo->u32Rotation        = PTREE_DB_GetInt(db, "ROT");
    ispInfo->u32level3dnr       = PTREE_DB_GetInt(db, "3DNR_LV");
    ispInfo->u32Sync3aType      = PTREE_DB_GetInt(db, "SYNC_3A_TYPE");
    ispInfo->u32StitchMask      = PTREE_DB_GetInt(db, "STITCH_MASK");
    ispInfo->u8Mirror           = PTREE_DB_GetInt(db, "IS_MIRROR");
    ispInfo->u8Flip             = PTREE_DB_GetInt(db, "IS_FLIP");
    ispInfo->u8CustIqEn         = PTREE_DB_GetInt(db, "CUST_IQ_EN");
    ispInfo->u8ZoomEn           = PTREE_DB_GetInt(db, "ZOOM_EN");
    ispInfo->u8SubChnIqEn       = PTREE_DB_GetInt(db, "SUB_CHN_IQ_EN");
    ispInfo->u8IspLdcEn         = PTREE_DB_GetInt(db, "ISP_LDC_EN");
    ispInfo->u8MutichnEn        = PTREE_DB_GetInt(db, "MUTI_CHN_EN");
    ispInfo->u8IspOverlapEn     = PTREE_DB_GetInt(db, "OVER_LAP_EN");
    str                         = PTREE_DB_GetStr(db, "IQ_API_FILE");
    snprintf(ispInfo->apiBinpath, 64, "%s", str);
    if (ispInfo->u8ZoomEn)
    {
        PTREE_DB_ProcessKey(db, "ZOOM_PARAM");
        ispInfo->stZoomParam.u8FromEntryIndex = PTREE_DB_GetInt(db, "FROM_ENTRY_INDEX");
        ispInfo->stZoomParam.u8ToEntryIndex   = PTREE_DB_GetInt(db, "TO_ENTRY_INDEX");
        ispInfo->stZoomParam.u8TableNum       = PTREE_DB_GetInt(db, "ZOOM_TABLE_NUM");
        if (ispInfo->stZoomParam.u8TableNum > ST_MAX_PTREE_SUR_ISP_TABLE_NUM)
        {
            PTREE_ERR("ZoomTableNum error!");
            return -1;
        }
        for (u32Idx = 0; u32Idx < ispInfo->stZoomParam.u8TableNum; u32Idx++)
        {
            snprintf(strTableName, 16, "TABLE_%d", u32Idx);
            PTREE_DB_ProcessKey(db, strTableName);
            ispInfo->stZoomParam.stTable[u32Idx].snrId  = PTREE_DB_GetInt(db, "SNR_ID");
            ispInfo->stZoomParam.stTable[u32Idx].tableX = PTREE_DB_GetInt(db, "TABLE_X");
            ispInfo->stZoomParam.stTable[u32Idx].tableY = PTREE_DB_GetInt(db, "TABLE_Y");
            ispInfo->stZoomParam.stTable[u32Idx].tableW = PTREE_DB_GetInt(db, "TABLE_W");
            ispInfo->stZoomParam.stTable[u32Idx].tableH = PTREE_DB_GetInt(db, "TABLE_H");
            PTREE_DB_ProcessBack(db);
        }
        PTREE_DB_ProcessBack(db);
    }
    if (ispInfo->u8SubChnIqEn)
    {
        PTREE_DB_ProcessKey(db, "SUB_CHN_IQ_PARAM");
        ispInfo->stSubChnIqParam.dev = PTREE_DB_GetInt(db, "DEV");
        ispInfo->stSubChnIqParam.chn = PTREE_DB_GetInt(db, "CHN");
        str                          = PTREE_DB_GetStr(db, "API_FILE");
        snprintf(ispInfo->stSubChnIqParam.apiFile, 64, "%s", str);
        PTREE_DB_ProcessBack(db);
    }
    if (ispInfo->u8CustIqEn)
    {
        PTREE_DB_ProcessKey(db, "CUST_IQ_PARAM");
        ispInfo->stEarlyInitParam.u32Revision          = (unsigned int)PTREE_DB_GetInt(db, "REVISION");
        ispInfo->stEarlyInitParam.u16SnrEarlyFps       = (unsigned short)PTREE_DB_GetInt(db, "FPS");
        ispInfo->stEarlyInitParam.u16SnrEarlyFlicker   = (unsigned short)PTREE_DB_GetInt(db, "FLICKER");
        ispInfo->stEarlyInitParam.u32SnrEarlyShutter   = (unsigned long)PTREE_DB_GetInt(db, "SHUTTER");
        ispInfo->stEarlyInitParam.u32SnrEarlyGainX1024 = (unsigned long)PTREE_DB_GetInt(db, "GAIN_X1024");
        ispInfo->stEarlyInitParam.u32SnrEarlyDGain     = (unsigned long)PTREE_DB_GetInt(db, "DGAIN");
        ispInfo->stEarlyInitParam.u16SnrEarlyAwbRGain  = (unsigned short)PTREE_DB_GetInt(db, "AWB_RGAIN");
        ispInfo->stEarlyInitParam.u16SnrEarlyAwbGGain  = (unsigned short)PTREE_DB_GetInt(db, "AWB_GGAIN");
        ispInfo->stEarlyInitParam.u16SnrEarlyAwbBGain  = (unsigned short)PTREE_DB_GetInt(db, "AWB_BGAIN");
        PTREE_DB_ProcessBack(db);
    }
    if (ispInfo->u8IspLdcEn)
    {
        PTREE_DB_ProcessKey(db, "ISP_LDC_PARAM");
        ispInfo->stIspLdcParam.centerX = (unsigned int)PTREE_DB_GetInt(db, "CENTER_X");
        ispInfo->stIspLdcParam.centerY = (unsigned short)PTREE_DB_GetInt(db, "CENTER_Y");
        ispInfo->stIspLdcParam.alpha   = (unsigned short)PTREE_DB_GetInt(db, "ALPHA");
        ispInfo->stIspLdcParam.beta    = (unsigned long)PTREE_DB_GetInt(db, "BETA");
        ispInfo->stIspLdcParam.cropL   = (unsigned long)PTREE_DB_GetInt(db, "CROP_L");
        ispInfo->stIspLdcParam.cropR   = (unsigned long)PTREE_DB_GetInt(db, "CROP_R");
        PTREE_DB_ProcessBack(db);
    }
    if (ispInfo->u8IspOverlapEn)
    {
        PTREE_DB_ProcessKey(db, "OVER_LAP_PARAM");
        snprintf(ispInfo->stOverLapParam.chOverlap, 16, "%s", PTREE_DB_GetStr(db, "OVER_LAP_TYPE"));
        PTREE_DB_ProcessBack(db);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_ISP_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_ISP_InInfo_t *ispInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_ISP_InInfo_t, base);
    (void)sysSur;
    ispInInfo->u16CropX = PTREE_DB_GetInt(db, "VID_CROP_X");
    ispInInfo->u16CropY = PTREE_DB_GetInt(db, "VID_CROP_Y");
    ispInInfo->u16CropW = PTREE_DB_GetInt(db, "VID_CROP_W");
    ispInInfo->u16CropH = PTREE_DB_GetInt(db, "VID_CROP_H");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_ISP_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_ISP_OutInfo_t *ispOutInfo = CONTAINER_OF(sysOutInfo, PTREE_SUR_ISP_OutInfo_t, base);
    const char *             pOutFmt    = NULL;
    (void)sysSur;

    ispOutInfo->u16CropX        = PTREE_DB_GetInt(db, "VID_CROP_X");
    ispOutInfo->u16CropY        = PTREE_DB_GetInt(db, "VID_CROP_Y");
    ispOutInfo->u16CropW        = PTREE_DB_GetInt(db, "VID_CROP_W");
    ispOutInfo->u16CropH        = PTREE_DB_GetInt(db, "VID_CROP_H");
    ispOutInfo->u32CompressMode = PTREE_DB_GetInt(db, "COMPRESS_MODE");
    pOutFmt                     = PTREE_DB_GetStr(db, "OUT_FMT");
    if (pOutFmt && strlen(pOutFmt) > 1)
    {
        ispOutInfo->u32VideoFormat = PTREE_ENUM_FROM_STR(MI_SYS_PixelFormat_e, pOutFmt);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_ISP_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_ISP_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_ISP_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_ISP_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(ISP, PTREE_SUR_ISP_New);
