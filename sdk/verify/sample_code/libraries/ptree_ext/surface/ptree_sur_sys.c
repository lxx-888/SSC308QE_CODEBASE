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

#include "parena.h"
#include "ssos_def.h"
#include "ptree_enum.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ptree_sur.h"
#include "ptree_sur_sys.h"
#include "mi_sys_datatype.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

static PARENA_Tag_t *_PTREE_SUR_SYS_OccupyInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_SYS_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static PARENA_Tag_t *_PTREE_SUR_SYS_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena);
static int           _PTREE_SUR_SYS_LoadDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_Info_t *info, PTREE_DB_Obj_t db);
static int           _PTREE_SUR_SYS_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *inInfo, PTREE_DB_Obj_t db);
static int           _PTREE_SUR_SYS_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *outInfo, PTREE_DB_Obj_t db);
static void          _PTREE_SUR_SYS_Destruct(PTREE_SUR_Obj_t *sur);
static void          _PTREE_SUR_SYS_Free(PTREE_SUR_Obj_t *sur);

static const PTREE_SUR_Ops_t G_PTREE_SUR_SYS_OPS = {
    .occupyInfo    = _PTREE_SUR_SYS_OccupyInfo,
    .occupyInInfo  = _PTREE_SUR_SYS_OccupyInInfo,
    .occupyOutInfo = _PTREE_SUR_SYS_OccupyOutInfo,
    .loadDb        = _PTREE_SUR_SYS_LoadDb,
    .loadInDb      = _PTREE_SUR_SYS_LoadInDb,
    .loadOutDb     = _PTREE_SUR_SYS_LoadOutDb,
};
static const PTREE_SUR_Hook_t G_PTREE_SUR_SYS_HOOK = {
    .destruct = _PTREE_SUR_SYS_Destruct,
    .free     = _PTREE_SUR_SYS_Free,
};

static PARENA_Tag_t *_PTREE_SUR_SYS_OccupyInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    PTREE_SUR_SYS_Obj_t *sysSur = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    if (sysSur->ops->occupyInfo)
    {
        return sysSur->ops->occupyInfo(sysSur, pArena);
    }
    return PARENA_GET(pArena, PTREE_SUR_SYS_Info_t, base, PTREE_SUR_Info_t);
}
static PARENA_Tag_t *_PTREE_SUR_SYS_OccupyInInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    PTREE_SUR_SYS_Obj_t *sysSur = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    if (sysSur->ops->occupyInInfo)
    {
        return sysSur->ops->occupyInInfo(sysSur, pArena);
    }
    return PARENA_GET(pArena, PTREE_SUR_SYS_InInfo_t, base, PTREE_SUR_InInfo_t);
}
static PARENA_Tag_t *_PTREE_SUR_SYS_OccupyOutInfo(PTREE_SUR_Obj_t *sur, void *pArena)
{
    PTREE_SUR_SYS_Obj_t *sysSur = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    if (sysSur->ops->occupyOutInfo)
    {
        return sysSur->ops->occupyOutInfo(sysSur, pArena);
    }
    return PARENA_GET(pArena, PTREE_SUR_SYS_OutInfo_t, base, PTREE_SUR_OutInfo_t);
}
static int _PTREE_SUR_SYS_LoadDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_Info_t *info, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SYS_Obj_t * sysSur  = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    PTREE_SUR_SYS_Info_t *sysInfo = CONTAINER_OF(info, PTREE_SUR_SYS_Info_t, base);

    return sysSur->ops->loadDb ? sysSur->ops->loadDb(sysSur, sysInfo, db) : SSOS_DEF_OK;
}
static int _PTREE_SUR_SYS_LoadInDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_InInfo_t *inInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SYS_Obj_t *   sysSur    = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    PTREE_SUR_SYS_InInfo_t *sysInInfo = CONTAINER_OF(inInfo, PTREE_SUR_SYS_InInfo_t, base);

    sysInInfo->bindType  = PTREE_DB_GetInt(db, "BIND_TYPE");
    sysInInfo->bindParam = PTREE_DB_GetInt(db, "BIND_PARAM");

    return sysSur->ops->loadInDb ? sysSur->ops->loadInDb(sysSur, sysInInfo, db) : SSOS_DEF_OK;
}
static int _PTREE_SUR_SYS_LoadOutDb(PTREE_SUR_Obj_t *sur, PTREE_SUR_OutInfo_t *outInfo, PTREE_DB_Obj_t db)
{
    PTREE_SUR_SYS_Obj_t *    sysSur     = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    PTREE_SUR_SYS_OutInfo_t *sysOutInfo = CONTAINER_OF(outInfo, PTREE_SUR_SYS_OutInfo_t, base);

    sysOutInfo->depthEn = PTREE_DB_GetInt(db, "DEPTH_EN");
    if (sysOutInfo->depthEn)
    {
        PTREE_DB_ProcessKey(db, "DEPTH_PARAM");
        sysOutInfo->depthUser  = PTREE_DB_GetInt(db, "DEPTH_USER");
        sysOutInfo->depthTotal = PTREE_DB_GetInt(db, "DEPTH_TOTAL");
        PTREE_DB_ProcessBack(db);
    }
    sysOutInfo->extEn = PTREE_DB_GetInt(db, "EXT_EN");
    if (sysOutInfo->extEn)
    {
        PTREE_DB_ProcessKey(db, "EXT_PARAM");
        sysOutInfo->extHAlign       = PTREE_DB_GetInt(db, "EXT_H_ALIGN");
        sysOutInfo->extVAlign       = PTREE_DB_GetInt(db, "EXT_V_ALIGN");
        sysOutInfo->extChromaAlign  = PTREE_DB_GetInt(db, "EXT_CHROMA_ALIGN");
        sysOutInfo->extChromaAlign  = PTREE_DB_GetInt(db, "EXT_COMPRESS_ALIGN");
        sysOutInfo->extExtraSize    = PTREE_DB_GetInt(db, "EXT_EXTRA_SIZE");
        sysOutInfo->extClearPadding = PTREE_DB_GetInt(db, "EXT_CLEAR_PADDING");
        PTREE_DB_ProcessBack(db);
    }
    sysOutInfo->userFrc = PTREE_DB_GetInt(db, "USER_FRC_EN");
    return sysSur->ops->loadOutDb ? sysSur->ops->loadOutDb(sysSur, sysOutInfo, db) : SSOS_DEF_OK;
}
static void _PTREE_SUR_SYS_Destruct(PTREE_SUR_Obj_t *sur)
{
    PTREE_SUR_SYS_Obj_t *sysSur = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    if (sysSur->hook && sysSur->hook->destruct)
    {
        sysSur->hook->destruct(sysSur);
    }
}
static void _PTREE_SUR_SYS_Free(PTREE_SUR_Obj_t *sur)
{
    PTREE_SUR_SYS_Obj_t *sysSur = CONTAINER_OF(sur, PTREE_SUR_SYS_Obj_t, base);
    if (sysSur->hook && sysSur->hook->free)
    {
        sysSur->hook->free(sysSur);
    }
}

int PTREE_SUR_SYS_Init(PTREE_SUR_SYS_Obj_t *sysSur, const PTREE_SUR_SYS_Ops_t *ops)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(sysSur, return SSOS_DEF_EINVAL);
    CHECK_POINTER(ops, return SSOS_DEF_EINVAL);
    ret = PTREE_SUR_Init(&sysSur->base, &G_PTREE_SUR_SYS_OPS);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_SUR_Init Failed");
        return ret;
    }
    sysSur->ops = ops;
    PTREE_SUR_Register(&sysSur->base, &G_PTREE_SUR_SYS_HOOK);
    return SSOS_DEF_OK;
}

void PTREE_SUR_SYS_Register(PTREE_SUR_SYS_Obj_t *sysSur, const PTREE_SUR_SYS_Hook_t *hook)
{
    CHECK_POINTER(sysSur, return );
    sysSur->hook = hook;
}

PTREE_ENUM_DEFINE(
    MI_SYS_PixelFormat_e,                    // MI_SYS_PixelFormat_e
    {E_MI_SYS_PIXEL_FRAME_FORMAT_MAX, "NA"}, // FORMAT_MAX

    {E_MI_SYS_PIXEL_FRAME_YUV422_YUYV, "yuyv"},  // YUV422_YUYV
    {E_MI_SYS_PIXEL_FRAME_ARGB8888, "argb8888"}, // ARGB8888
    {E_MI_SYS_PIXEL_FRAME_ABGR8888, "abgr8888"}, // ABGR8888
    {E_MI_SYS_PIXEL_FRAME_BGRA8888, "bgra8888"}, // BGRA8888

    {E_MI_SYS_PIXEL_FRAME_RGB565, "rgb565"},     // RGB565
    {E_MI_SYS_PIXEL_FRAME_ARGB1555, "argb1555"}, // ARGB1555
    {E_MI_SYS_PIXEL_FRAME_ARGB4444, "argb4444"}, // ARGB4444
    {E_MI_SYS_PIXEL_FRAME_I2, "i2"},             // I2
    {E_MI_SYS_PIXEL_FRAME_I4, "i4"},             // I4
    {E_MI_SYS_PIXEL_FRAME_I8, "i8"},             // I8

    {E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422, "yuv422sp"},  // YUV_SEMIPLANAR_422
    {E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420, "nv12"},      // YUV_SEMIPLANAR_420
    {E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21, "nv21"}, // YUV_SEMIPLANAR_420_NV21
    {E_MI_SYS_PIXEL_FRAME_YUV_TILE_420, "tile420"},         // YUV_TILE_420
    {E_MI_SYS_PIXEL_FRAME_YUV422_UYVY, "uyvy"},             // YUV422_UYVY
    {E_MI_SYS_PIXEL_FRAME_YUV422_YVYU, "yvyu"},             // YUV422_YVYU
    {E_MI_SYS_PIXEL_FRAME_YUV422_VYUY, "vyuy"},             // YUV422_VYUY

    {E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR, "yuv422p"}, // YUV422_PLANAR
    {E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR, "yuv420p"}, // YUV420_PLANAR

    {E_MI_SYS_PIXEL_FRAME_FBC_420, "fbc420"}, // FBC_420

    {E_MI_SYS_PIXEL_FRAME_RGB888, "rgb888"},         // RGB888
    {E_MI_SYS_PIXEL_FRAME_BGR888, "bgr888"},         // BGR888
    {E_MI_SYS_PIXEL_FRAME_GRAY8, "gray8"},           // GRAY8
    {E_MI_SYS_PIXEL_FRAME_RGB101010, "rgb101010"},   // RGB101010
    {E_MI_SYS_PIXEL_FRAME_RGB888_PLANAR, "rgb888p"}, // RGB888_PLANAR

    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_BAYERID_RG), "bayer 8bpp rg"}, // 8bpp bayerid_rg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_BAYERID_GR), "bayer 8bpp gr"}, // 8bpp bayerid_gr
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_BAYERID_BG), "bayer 8bpp bg"}, // 8bpp bayerid_bg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_BAYERID_GB), "bayer 8bpp gb"}, // 8bpp bayerid_gb
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_R0), "bayer 8bpp r0"},   // 8bpp rgbir_r0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_G0), "bayer 8bpp g0"},   // 8bpp rgbir_g0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_B0), "bayer 8bpp b0"},   // 8bpp rgbir_b0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_G1), "bayer 8bpp g1"},   // 8bpp rgbir_g1
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_G2), "bayer 8bpp g2"},   // 8bpp rgbir_g2
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_I0), "bayer 8bpp i0"},   // 8bpp rgbir_i0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_G3), "bayer 8bpp g3"},   // 8bpp rgbir_g3
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_RGBIR_I1), "bayer 8bpp i1"},   // 8bpp rgbir_i1

    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_BAYERID_RG), "bayer 10bpp rg"}, // 10bpp bayerid_rg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_BAYERID_GR), "bayer 10bpp gr"}, // 10bpp bayerid_gr
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_BAYERID_BG), "bayer 10bpp bg"}, // 10bpp bayerid_bg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_BAYERID_GB), "bayer 10bpp gb"}, // 10bpp bayerid_gb
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_R0), "bayer 10bpp r0"},   // 10bpp rgbir_r0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_G0), "bayer 10bpp g0"},   // 10bpp rgbir_g0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_B0), "bayer 10bpp b0"},   // 10bpp rgbir_b0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_G1), "bayer 10bpp g1"},   // 10bpp rgbir_g1
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_G2), "bayer 10bpp g2"},   // 10bpp rgbir_g2
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_I0), "bayer 10bpp i0"},   // 10bpp rgbir_i0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_G3), "bayer 10bpp g3"},   // 10bpp rgbir_g3
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_10BPP, E_MI_SYS_PIXEL_RGBIR_I1), "bayer 10bpp i1"},   // 10bpp rgbir_i1

    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_BAYERID_RG), "bayer 12bpp rg"}, // 12bpp bayerid_rg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_BAYERID_GR), "bayer 12bpp gr"}, // 12bpp bayerid_gr
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_BAYERID_BG), "bayer 12bpp bg"}, // 12bpp bayerid_bg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_BAYERID_GB), "bayer 12bpp gb"}, // 12bpp bayerid_gb
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_R0), "bayer 12bpp r0"},   // 12bpp rgbir_r0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_G0), "bayer 12bpp g0"},   // 12bpp rgbir_g0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_B0), "bayer 12bpp b0"},   // 12bpp rgbir_b0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_G1), "bayer 12bpp g1"},   // 12bpp rgbir_g1
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_G2), "bayer 12bpp g2"},   // 12bpp rgbir_g2
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_I0), "bayer 12bpp i0"},   // 12bpp rgbir_i0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_G3), "bayer 12bpp g3"},   // 12bpp rgbir_g3
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_12BPP, E_MI_SYS_PIXEL_RGBIR_I1), "bayer 12bpp i1"},   // 12bpp rgbir_i1

    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_BAYERID_RG), "bayer 14bpp rg"}, // 14bpp bayerid_rg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_BAYERID_GR), "bayer 14bpp gr"}, // 14bpp bayerid_gr
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_BAYERID_BG), "bayer 14bpp bg"}, // 14bpp bayerid_bg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_BAYERID_GB), "bayer 14bpp gb"}, // 14bpp bayerid_gb
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_R0), "bayer 14bpp r0"},   // 14bpp rgbir_r0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_G0), "bayer 14bpp g0"},   // 14bpp rgbir_g0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_B0), "bayer 14bpp b0"},   // 14bpp rgbir_b0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_G1), "bayer 14bpp g1"},   // 14bpp rgbir_g1
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_G2), "bayer 14bpp g2"},   // 14bpp rgbir_g2
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_I0), "bayer 14bpp i0"},   // 14bpp rgbir_i0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_G3), "bayer 14bpp g3"},   // 14bpp rgbir_g3
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_14BPP, E_MI_SYS_PIXEL_RGBIR_I1), "bayer 14bpp i1"},   // 14bpp rgbir_i1

    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_BAYERID_RG), "bayer 16bpp rg"}, // 16bpp bayerid_rg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_BAYERID_GR), "bayer 16bpp gr"}, // 16bpp bayerid_gr
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_BAYERID_BG), "bayer 16bpp bg"}, // 16bpp bayerid_bg
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_BAYERID_GB), "bayer 16bpp gb"}, // 16bpp bayerid_gb
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_R0), "bayer 16bpp r0"},   // 16bpp rgbir_r0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_G0), "bayer 16bpp g0"},   // 16bpp rgbir_g0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_B0), "bayer 16bpp b0"},   // 16bpp rgbir_b0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_G1), "bayer 16bpp g1"},   // 16bpp rgbir_g1
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_G2), "bayer 16bpp g2"},   // 16bpp rgbir_g2
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_I0), "bayer 16bpp i0"},   // 16bpp rgbir_i0
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_G3), "bayer 16bpp g3"},   // 16bpp rgbir_g3
    {RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_16BPP, E_MI_SYS_PIXEL_RGBIR_I1), "bayer 16bpp i1"},   // 16bpp rgbir_i1
)

PTREE_ENUM_DEFINE(MI_SYS_BayerId_e, {E_MI_SYS_PIXEL_BAYERID_RG, "rg"}, {E_MI_SYS_PIXEL_BAYERID_GR, "gr"},
                  {E_MI_SYS_PIXEL_BAYERID_BG, "bg"}, {E_MI_SYS_PIXEL_BAYERID_GB, "gb"}, {E_MI_SYS_PIXEL_RGBIR_R0, "r0"},
                  {E_MI_SYS_PIXEL_RGBIR_G0, "g0"}, {E_MI_SYS_PIXEL_RGBIR_B0, "b0"}, {E_MI_SYS_PIXEL_RGBIR_G1, "g1"},
                  {E_MI_SYS_PIXEL_RGBIR_G2, "g2"}, {E_MI_SYS_PIXEL_RGBIR_I0, "i0"}, {E_MI_SYS_PIXEL_RGBIR_G3, "g3"},
                  {E_MI_SYS_PIXEL_RGBIR_I1, "i1"}, )

PTREE_ENUM_DEFINE(MI_SYS_DataPrecision_e, {E_MI_SYS_DATA_PRECISION_8BPP, "8bpp"},
                  {E_MI_SYS_DATA_PRECISION_10BPP, "10bpp"}, {E_MI_SYS_DATA_PRECISION_12BPP, "12bpp"},
                  {E_MI_SYS_DATA_PRECISION_14BPP, "14bpp"}, {E_MI_SYS_DATA_PRECISION_16BPP, "16bpp"}, )
