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
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_sur_ldc.h"
#include "ptree_sur_sys.h"
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

PTREE_ENUM_DEFINE(MI_LDC_WorkMode_e, {MI_LDC_WORKMODE_LDC, "ldc"}, {MI_LDC_WORKMODE_LUT, "lut"},
                  {MI_LDC_WORKMODE_DIS, "dis"}, {MI_LDC_WORKMODE_PMF, "pmf"}, {MI_LDC_WORKMODE_STITCH, "stitch"},
                  {MI_LDC_WORKMODE_NIR, "nir"}, {MI_LDC_WORKMODE_DPU, "dpu"},
                  {MI_LDC_WORKMODE_LDC_HORIZONTAL, "ldc_horizontal"}, {MI_LDC_WORKMODE_DIS_LDC, "dis_ldc"}, );

static PARENA_Tag_t *_PTREE_SUR_LDC_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_LDC_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_LDC_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena);
static int  _PTREE_SUR_LDC_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_LDC_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db);
static int  _PTREE_SUR_LDC_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo,
                                     PTREE_DB_Obj_t db);
static void _PTREE_SUR_LDC_Free(PTREE_SUR_SYS_Obj_t *sysSur);

static const PTREE_SUR_SYS_Ops_t G_PTREE_SUR_LDC_OPS = {
    .occupyInfo    = _PTREE_SUR_LDC_OccupyInfo,
    .occupyInInfo  = _PTREE_SUR_LDC_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_LDC_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_LDC_LoadDb,
    .loadInDb      = _PTREE_SUR_LDC_LoadInDb,
    .loadOutDb     = _PTREE_SUR_LDC_LoadOutDb,
};
static const PTREE_SUR_SYS_Hook_t G_PTREE_SUR_LDC_HOOK = {
    .free = _PTREE_SUR_LDC_Free,
};

static unsigned int _PTREE_SUR_LDC_SplitFillTokens(const char *inputChar, char delimiter, char tokens[][8],
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

static PARENA_Tag_t *_PTREE_SUR_LDC_OccupyInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_LDC_Info_t, base.base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_LDC_OccupyInInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_LDC_InInfo_t, base.base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_LDC_OccupyOutInfo(PTREE_SUR_SYS_Obj_t *sysSur, void *pArena)
{
    (void)sysSur;
    return PARENA_GET(pArena, PTREE_SUR_LDC_OutInfo_t, base.base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_LDC_LoadDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_Info_t *sysInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_LDC_Info_t *ldcInfo = CONTAINER_OF(sysInfo, PTREE_SUR_LDC_Info_t, base);
    const char *          str     = NULL;
    unsigned int          u32Idx  = 0;
    char                  strLayerName[16];
    unsigned int          tokenCount;
    char                  tokens[LDC_MAXTRIX_NUM][8];
    memset(tokens, 0, sizeof(tokens));

    (void)sysSur;
    str               = PTREE_DB_GetStr(db, "WORK_MODE");
    ldcInfo->workMode = PTREE_ENUM_FROM_STR(MI_LDC_WorkMode_e, str);
    PTREE_DB_ProcessKey(db, "WORK_MODE_PARAM");
    if (strcmp(str, "ldc") == 0)
    {
        str = PTREE_DB_GetStr(db, "CALIB_PATH");
        snprintf(ldcInfo->calib.path, 32, "%s", str);
        ldcInfo->ldcCfg.fisheyeRadius = PTREE_DB_GetInt(db, "FISHEYE_RADIUS");
        ldcInfo->ldcCfg.centerXOff    = PTREE_DB_GetInt(db, "CENTER_X_OFFSET");
        ldcInfo->ldcCfg.centerYOff    = PTREE_DB_GetInt(db, "CENTER_Y_OFFSET");
        ldcInfo->ldcCfg.enBgColor     = PTREE_DB_GetInt(db, "ENABLE_BGCOLOR");
        ldcInfo->ldcCfg.bgColor       = PTREE_DB_GetInt(db, "BGCOLOR");
        ldcInfo->ldcCfg.mountMode     = PTREE_DB_GetInt(db, "MOUNT_MODE");
        ldcInfo->ldcCfg.regionNum     = PTREE_DB_GetInt(db, "REGION_NUM");
        if (ldcInfo->ldcCfg.regionNum == (unsigned int)-1
            || ldcInfo->ldcCfg.regionNum > ST_MAX_PTREE_SUR_LDC_REGION_NUM) // LDC_MAX_regionNum
        {
            return -1;
        }
        for (u32Idx = 0; u32Idx < ldcInfo->ldcCfg.regionNum; u32Idx++)
        {
            snprintf(strLayerName, 16, "REGION_%d", u32Idx);
            PTREE_DB_ProcessKey(db, strLayerName);
            ldcInfo->ldcCfg.region[u32Idx].regionMode = PTREE_DB_GetInt(db, "REGION_MODE");
            ldcInfo->ldcCfg.region[u32Idx].x          = PTREE_DB_GetInt(db, "OUT_RECT_X");
            ldcInfo->ldcCfg.region[u32Idx].y          = PTREE_DB_GetInt(db, "OUT_RECT_Y");
            ldcInfo->ldcCfg.region[u32Idx].width      = PTREE_DB_GetInt(db, "OUT_RECT_WIDTH");
            ldcInfo->ldcCfg.region[u32Idx].height     = PTREE_DB_GetInt(db, "OUT_RECT_HEIGHT");
            if (4 == ldcInfo->ldcCfg.region[u32Idx].regionMode) // MI_PTREE_SUR_LDC_LdcRegion_s_MAP2BIN
            {
                str = PTREE_DB_GetStr(db, "MAP_X");
                snprintf(ldcInfo->ldcCfg.region[u32Idx].map2bin.mapX, 16, "%s", str);
                str = PTREE_DB_GetStr(db, "MAP_Y");
                snprintf(ldcInfo->ldcCfg.region[u32Idx].map2bin.mapY, 16, "%s", str);
                ldcInfo->ldcCfg.region[u32Idx].map2bin.grid = PTREE_DB_GetInt(db, "GRID");
            }
            else
            {
                ldcInfo->ldcCfg.region[u32Idx].para.cropMode        = PTREE_DB_GetInt(db, "CROP_MODE");
                ldcInfo->ldcCfg.region[u32Idx].para.pan             = PTREE_DB_GetInt(db, "PAN");
                ldcInfo->ldcCfg.region[u32Idx].para.tilt            = PTREE_DB_GetInt(db, "TILT");
                ldcInfo->ldcCfg.region[u32Idx].para.zoomV           = PTREE_DB_GetInt(db, "ZOOM_V");
                ldcInfo->ldcCfg.region[u32Idx].para.zoomH           = PTREE_DB_GetInt(db, "ZOOM_H");
                ldcInfo->ldcCfg.region[u32Idx].para.inRadius        = PTREE_DB_GetInt(db, "IN_RADIUS");
                ldcInfo->ldcCfg.region[u32Idx].para.outRadius       = PTREE_DB_GetInt(db, "OUT_RADIUS");
                ldcInfo->ldcCfg.region[u32Idx].para.focalRatio      = PTREE_DB_GetInt(db, "FOCAL_RATIO");
                ldcInfo->ldcCfg.region[u32Idx].para.distortionRatio = PTREE_DB_GetInt(db, "DISTORTION_RATIO");
                ldcInfo->ldcCfg.region[u32Idx].para.outRot          = PTREE_DB_GetInt(db, "OUT_ROT");
                ldcInfo->ldcCfg.region[u32Idx].para.rot             = PTREE_DB_GetInt(db, "ROT");
            }
            PTREE_DB_ProcessBack(db);
        }
    }
    else if (strcmp(str, "lut") == 0)
    {
        ldcInfo->lutCfg.width  = PTREE_DB_GetInt(db, "WIDTH");
        ldcInfo->lutCfg.height = PTREE_DB_GetInt(db, "HEIGHT");
        str                    = PTREE_DB_GetStr(db, "TABLE_WEIGHT_PATH");
        snprintf(ldcInfo->lutCfg.tableW, 16, "%s", str);
        str = PTREE_DB_GetStr(db, "TABLE_X_PATH");
        snprintf(ldcInfo->lutCfg.tableX, 16, "%s", str);
        str = PTREE_DB_GetStr(db, "TABLE_Y_PATH");
        snprintf(ldcInfo->lutCfg.tableY, 16, "%s", str);
    }
    else if (strcmp(str, "dis") == 0)
    {
        ldcInfo->disCfg.disMode      = PTREE_DB_GetInt(db, "DIS_MODE");
        ldcInfo->disCfg.userSliceNum = PTREE_DB_GetInt(db, "USER_SLICE_NUM");
        ldcInfo->disCfg.focalLengthX = PTREE_DB_GetInt(db, "FOCAL_LEN_X");
        ldcInfo->disCfg.focalLengthY = PTREE_DB_GetInt(db, "FOCAL_LEN_Y");
        ldcInfo->disCfg.sceneType    = PTREE_DB_GetInt(db, "SCENE_TYPE");
        ldcInfo->disCfg.motionLevel  = PTREE_DB_GetInt(db, "MOTION_LEVEL");
        ldcInfo->disCfg.cropRatio    = PTREE_DB_GetInt(db, "CROP_RATIO");
        str                          = PTREE_DB_GetStr(db, "ROTATION_MATRIX");

        tokenCount = _PTREE_SUR_LDC_SplitFillTokens(str, ',', tokens, sizeof(tokens) / sizeof(tokens[0]));
        if (tokenCount < LDC_MAXTRIX_NUM)
        {
            PTREE_ERR("LDC get %s Err", str);
            return -1;
        }
        for (u32Idx = 0; u32Idx < LDC_MAXTRIX_NUM; u32Idx++)
        {
            ldcInfo->disCfg.rotationMatrix[u32Idx] = SSOS_IO_Atoi(tokens[u32Idx]);
        }
    }
    else if (strcmp(str, "pmf") == 0)
    {
        str = PTREE_DB_GetStr(db, "PMF_COEf");
        snprintf(ldcInfo->pmfCfg.pmfCoef, 16, "%s", str);
    }
    else if (strcmp(str, "stitch") == 0)
    {
        str = PTREE_DB_GetStr(db, "CALIB_PATH");
        snprintf(ldcInfo->calib.path, 32, "%s", str);
        ldcInfo->stitchCfg.distance = PTREE_DB_GetInt(db, "DISTANCE");
        ldcInfo->stitchCfg.projType = PTREE_DB_GetInt(db, "PROJ_TYPE");
    }
    else if (strcmp(str, "nir") == 0)
    {
        str = PTREE_DB_GetStr(db, "CALIB_PATH");
        snprintf(ldcInfo->calib.path, 32, "%s", str);
        ldcInfo->nirCfg.distance = PTREE_DB_GetInt(db, "DISTANCE");
    }
    else if (strcmp(str, "dpu") == 0)
    {
        str = PTREE_DB_GetStr(db, "CALIB_PATH");
        snprintf(ldcInfo->calib.path, 32, "%s", str);
        ldcInfo->dpuCfg.distance = PTREE_DB_GetInt(db, "DISTANCE");
    }
    else if (strcmp(str, "ldc_horizontal") == 0)
    {
        str = PTREE_DB_GetStr(db, "CALIB_PATH");
        snprintf(ldcInfo->calib.path, 32, "%s", str);
        ldcInfo->ldcHorizontalCfg.distortionRatio = PTREE_DB_GetInt(db, "DISTORTION_RATIO");
    }
    else if (strcmp(str, "dis_ldc") == 0)
    {
        str = PTREE_DB_GetStr(db, "CALIB_PATH");
        snprintf(ldcInfo->calib.path, 32, "%s", str);
        ldcInfo->ldcCfg.fisheyeRadius = PTREE_DB_GetInt(db, "FISHEYE_RADIUS");
        ldcInfo->ldcCfg.centerXOff    = PTREE_DB_GetInt(db, "CENTER_X_OFFSET");
        ldcInfo->ldcCfg.centerYOff    = PTREE_DB_GetInt(db, "CENTER_Y_OFFSET");
        ldcInfo->ldcCfg.enBgColor     = PTREE_DB_GetInt(db, "ENABLE_BGCOLOR");
        ldcInfo->ldcCfg.bgColor       = PTREE_DB_GetInt(db, "BGCOLOR");
        ldcInfo->ldcCfg.mountMode     = PTREE_DB_GetInt(db, "MOUNT_MODE");
        ldcInfo->ldcCfg.regionNum     = PTREE_DB_GetInt(db, "REGION_NUM");
        if (ldcInfo->ldcCfg.regionNum == (unsigned int)-1
            || ldcInfo->ldcCfg.regionNum > ST_MAX_PTREE_SUR_LDC_REGION_NUM) // LDC_MAX_regionNum
        {
            return -1;
        }
        for (u32Idx = 0; u32Idx < ldcInfo->ldcCfg.regionNum; u32Idx++)
        {
            snprintf(strLayerName, 16, "REGION_%d", u32Idx);
            PTREE_DB_ProcessKey(db, strLayerName);
            ldcInfo->ldcCfg.region[u32Idx].regionMode = PTREE_DB_GetInt(db, "REGION_MODE");
            ldcInfo->ldcCfg.region[u32Idx].x          = PTREE_DB_GetInt(db, "OUT_RECT_X");
            ldcInfo->ldcCfg.region[u32Idx].y          = PTREE_DB_GetInt(db, "OUT_RECT_Y");
            ldcInfo->ldcCfg.region[u32Idx].width      = PTREE_DB_GetInt(db, "OUT_RECT_WIDTH");
            ldcInfo->ldcCfg.region[u32Idx].height     = PTREE_DB_GetInt(db, "OUT_RECT_HEIGHT");
            if (4 == ldcInfo->ldcCfg.region[u32Idx].regionMode) // MI_PTREE_SUR_LDC_LdcRegion_s_MAP2BIN
            {
                str = PTREE_DB_GetStr(db, "MAP_X");
                snprintf(ldcInfo->ldcCfg.region[u32Idx].map2bin.mapX, 16, "%s", str);
                str = PTREE_DB_GetStr(db, "MAP_Y");
                snprintf(ldcInfo->ldcCfg.region[u32Idx].map2bin.mapY, 16, "%s", str);
                ldcInfo->ldcCfg.region[u32Idx].map2bin.grid = PTREE_DB_GetInt(db, "GRID");
            }
            else
            {
                ldcInfo->ldcCfg.region[u32Idx].para.cropMode        = PTREE_DB_GetInt(db, "CROP_MODE");
                ldcInfo->ldcCfg.region[u32Idx].para.pan             = PTREE_DB_GetInt(db, "PAN");
                ldcInfo->ldcCfg.region[u32Idx].para.tilt            = PTREE_DB_GetInt(db, "TILT");
                ldcInfo->ldcCfg.region[u32Idx].para.zoomV           = PTREE_DB_GetInt(db, "ZOOM_V");
                ldcInfo->ldcCfg.region[u32Idx].para.zoomH           = PTREE_DB_GetInt(db, "ZOOM_H");
                ldcInfo->ldcCfg.region[u32Idx].para.inRadius        = PTREE_DB_GetInt(db, "IN_RADIUS");
                ldcInfo->ldcCfg.region[u32Idx].para.outRadius       = PTREE_DB_GetInt(db, "OUT_RADIUS");
                ldcInfo->ldcCfg.region[u32Idx].para.focalRatio      = PTREE_DB_GetInt(db, "FOCAL_RATIO");
                ldcInfo->ldcCfg.region[u32Idx].para.distortionRatio = PTREE_DB_GetInt(db, "DISTORTION_RATIO");
                ldcInfo->ldcCfg.region[u32Idx].para.outRot          = PTREE_DB_GetInt(db, "OUT_ROT");
                ldcInfo->ldcCfg.region[u32Idx].para.rot             = PTREE_DB_GetInt(db, "ROT");
            }
            PTREE_DB_ProcessBack(db);
        }
        ldcInfo->disCfg.disMode      = PTREE_DB_GetInt(db, "DIS_MODE");
        ldcInfo->disCfg.userSliceNum = PTREE_DB_GetInt(db, "USER_SLICE_NUM");
        ldcInfo->disCfg.focalLengthX = PTREE_DB_GetInt(db, "FOCAL_LEN_X");
        ldcInfo->disCfg.focalLengthY = PTREE_DB_GetInt(db, "FOCAL_LEN_Y");
        ldcInfo->disCfg.sceneType    = PTREE_DB_GetInt(db, "SCENE_TYPE");
        ldcInfo->disCfg.motionLevel  = PTREE_DB_GetInt(db, "MOTION_LEVEL");
        ldcInfo->disCfg.cropRatio    = PTREE_DB_GetInt(db, "CROP_RATIO");
        str                          = PTREE_DB_GetStr(db, "ROTATION_MATRIX");

        tokenCount = _PTREE_SUR_LDC_SplitFillTokens(str, ',', tokens, sizeof(tokens) / sizeof(tokens[0]));
        if (tokenCount < LDC_MAXTRIX_NUM)
        {
            PTREE_ERR("LDC get %s Err", str);
            return -1;
        }
        for (u32Idx = 0; u32Idx < LDC_MAXTRIX_NUM; u32Idx++)
        {
            ldcInfo->disCfg.rotationMatrix[u32Idx] = SSOS_IO_Atoi(tokens[u32Idx]);
        }
    }
    PTREE_DB_ProcessBack(db);
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_LDC_LoadInDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_InInfo_t *sysInInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_LDC_InInfo_t *ldcInInfo = CONTAINER_OF(sysInInfo, PTREE_SUR_LDC_InInfo_t, base);
    (void)sysSur;
    ldcInInfo->width  = PTREE_DB_GetInt(db, "VID_W");
    ldcInInfo->height = PTREE_DB_GetInt(db, "VID_H");
    return SSOS_DEF_OK;
}
static int _PTREE_SUR_LDC_LoadOutDb(PTREE_SUR_SYS_Obj_t *sysSur, PTREE_SUR_SYS_OutInfo_t *sysOutInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_LDC_OutInfo_t *ldcOutInfo = CONTAINER_OF(sysOutInfo, PTREE_SUR_LDC_OutInfo_t, base);
    const char *             pOutFmt    = NULL;
    (void)sysSur;

    ldcOutInfo->width  = PTREE_DB_GetInt(db, "VID_W");
    ldcOutInfo->height = PTREE_DB_GetInt(db, "VID_H");
    pOutFmt            = PTREE_DB_GetStr(db, "OUT_FMT");
    if (pOutFmt && strlen(pOutFmt) > 1)
    {
        ldcOutInfo->videoFormat = PTREE_ENUM_FROM_STR(MI_SYS_PixelFormat_e, pOutFmt);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_SUR_LDC_Free(PTREE_SUR_SYS_Obj_t *sysSur)
{
    SSOS_MEM_Free(sysSur);
}

PTREE_SUR_Obj_t *PTREE_SUR_LDC_New(void)
{
    PTREE_SUR_SYS_Obj_t *sysSur = SSOS_MEM_Alloc(sizeof(PTREE_SUR_SYS_Obj_t));
    if (!sysSur)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysSur, 0, sizeof(PTREE_SUR_SYS_Obj_t));
    if (SSOS_DEF_OK != PTREE_SUR_SYS_Init(sysSur, &G_PTREE_SUR_LDC_OPS))
    {
        SSOS_MEM_Free(sysSur);
        return NULL;
    }
    PTREE_SUR_SYS_Register(sysSur, &G_PTREE_SUR_LDC_HOOK);
    return &sysSur->base;
}

PTREE_MAKER_SUR_INIT(LDC, PTREE_SUR_LDC_New);
