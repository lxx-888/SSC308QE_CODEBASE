/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifdef __KERNEL__

#else /* LINUX USER */

#include "mi_common_datatype.h"
#include "mi_vdf_datatype.h"
#include "mi_vdf.h"
#include "mi_shadow_datatype.h"
#include "mi_shadow.h"
#include "parena.h"
#include "ssos_def.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ssos_task.h"
#include "ptree_mod.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_vdf.h"
#include "ptree_sur_vdf.h"
#include "ptree_maker.h"
#include "ptree_rgn_packet.h"

#define VDF_ALIGN_X_UP(x, align)   ((((x) + (align - 1)) / align) * align)
#define VDF_ALIGN_X_DOWN(x, align) ((x) / align * align)

#define PTREE_MOD_VDF_READER_TASK_NAME_LEN (32)

#define CHECK_DATA_ALIGN(min, max, checkData, destData, pName)                                                     \
    (                                                                                                              \
        {                                                                                                          \
            if (checkData < min || checkData > max)                                                                \
            {                                                                                                      \
                PTREE_WRN("check data err, please check %s: %d, must in: [%d, %d]\n", pName, checkData, min, max); \
            }                                                                                                      \
            *destData = (checkData) < (min) ? (min) : ((checkData) > (max) ? (max) : (checkData));                 \
        })

typedef struct PTREE_MOD_VDF_Obj_s PTREE_MOD_VDF_Obj_t;

struct PTREE_MOD_VDF_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
    void *              taskHandle;
    unsigned int        streamMapWidth, streamMapHeight;
    unsigned int        vdfMapStartX, vdfMapStartY;
    unsigned int        vdfMapEndX, vdfMapEndY;

    unsigned int       inPortWidth;
    unsigned int       inPortHeight;
    PTREE_MOD_OutObj_t modOut;
};

static unsigned char _PTREE_MOD_VDF_GetMapSizeAlign(MDMB_MODE_e mbSize)
{
    unsigned char mapSize = 4;
    switch (mbSize)
    {
        case MDMB_MODE_MB_4x4:
            mapSize = 4;
            break;
        case MDMB_MODE_MB_8x8:
            mapSize = 8;
            break;
        case MDMB_MODE_MB_16x16:
            mapSize = 16;
            break;
        default:
            PTREE_WRN("set mbsize(%d) error\n", mbSize);
            break;
    }
    return mapSize;
}

static int                  _PTREE_MOD_VDF_OutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_VDF_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_VDF_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_VDF_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);

static int                 _PTREE_MOD_VDF_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_VDF_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_VDF_Prepare(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_VDF_UnPrepare(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_OutObj_t *_PTREE_MOD_VDF_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_VDF_Destruct(PTREE_MOD_SYS_Obj_t *sysMod);
static void                _PTREE_MOD_VDF_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static const PTREE_MOD_OutOps_t G_PTREE_MOD_VDF_OUT_OPS = {
    .getType       = _PTREE_MOD_VDF_OutGetType,
    .getPacketInfo = _PTREE_MOD_VDF_OutGetPacketInfo,
    .linked        = _PTREE_MOD_VDF_OutLinked,
    .unlinked      = _PTREE_MOD_VDF_OutUnlinked,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_VDF_OUT_HOOK = {};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_VDF_SYS_OPS = {
    .init         = _PTREE_MOD_VDF_Init,
    .deinit       = _PTREE_MOD_VDF_Deinit,
    .prepare      = _PTREE_MOD_VDF_Prepare,
    .unprepare    = _PTREE_MOD_VDF_UnPrepare,
    .createModOut = _PTREE_MOD_VDF_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_VDF_SYS_HOOK = {
    .destruct = _PTREE_MOD_VDF_Destruct,
    .free     = _PTREE_MOD_VDF_Free,
};

static int g_vdfCreateDev                       = 0;
static int g_vdfRunning[E_MI_VDF_WORK_MODE_MAX] = {0};

static void _PTREE_MOD_VDF_SetMdAttr(MI_VDF_MdAttr_t *mdAttr, PTREE_MOD_VDF_Obj_t *vdfMod, PTREE_SUR_VDF_Info_t *mdInfo)
{
    unsigned char mapSize             = 0;
    mdAttr->stMdStaticParamsIn.width  = vdfMod->inPortWidth;
    mdAttr->stMdStaticParamsIn.height = vdfMod->inPortHeight;
    mdAttr->u8Enable                  = 1;
    mdAttr->u8MdBufCnt                = 4;
    mdAttr->u8VDFIntvl                = 0;
    mdAttr->ccl_ctrl.u16InitAreaThr   = 8;
    mdAttr->ccl_ctrl.u16Step          = 2;
    CHECK_DATA_ALIGN(10, 100, mdInfo->stMdAttr.u32Sensitivity, &mdAttr->stMdDynamicParamsIn.sensitivity, "sensitivity");
    CHECK_DATA_ALIGN(0, 90, mdInfo->stMdAttr.u32ObjNumMax, &mdAttr->stMdDynamicParamsIn.obj_num_max, "obj_num_max");
    mdAttr->stMdStaticParamsIn.width        = VDF_ALIGN_X_UP(vdfMod->inPortWidth, 16); // must < divp width & 16 align
    mdAttr->stMdStaticParamsIn.stride       = mdAttr->stMdStaticParamsIn.width;
    mdAttr->stMdStaticParamsIn.color        = 1;
    mdAttr->stMdStaticParamsIn.mb_size      = mdInfo->enMdMbMode;
    mdAttr->stMdStaticParamsIn.md_alg_mode  = mdInfo->enAlgMode;
    mdAttr->stMdStaticParamsIn.sad_out_ctrl = mdInfo->enMdSadOutMode;
    if (MDALG_MODE_FG == mdAttr->stMdStaticParamsIn.md_alg_mode)
    {
        CHECK_DATA_ALIGN(1000, 30000, mdInfo->stMdAttr.u32LearnRate, &mdAttr->stMdDynamicParamsIn.learn_rate,
                         "learn_rate");
        CHECK_DATA_ALIGN(0, 99, mdInfo->stMdAttr.u32Thr, &mdAttr->stMdDynamicParamsIn.md_thr, "md_thr");
    }
    else
    {
        CHECK_DATA_ALIGN(1, 255, mdInfo->stMdAttr.u32LearnRate, &mdAttr->stMdDynamicParamsIn.learn_rate, "learn_rate");
        CHECK_DATA_ALIGN(0, 255, mdInfo->stMdAttr.u32Thr, &mdAttr->stMdDynamicParamsIn.md_thr, "md_thr");
    }
    mapSize = _PTREE_MOD_VDF_GetMapSizeAlign(mdAttr->stMdStaticParamsIn.mb_size);
    CHECK_DATA_ALIGN(0, mdAttr->stMdStaticParamsIn.width - 1, mdInfo->stMdAttr.stPnt[0].u32x,
                     &mdAttr->stMdStaticParamsIn.roi_md.pnt[0].x, "MD point0 x");
    CHECK_DATA_ALIGN(0, mdAttr->stMdStaticParamsIn.height - 1, mdInfo->stMdAttr.stPnt[0].u32y,
                     &mdAttr->stMdStaticParamsIn.roi_md.pnt[0].y, "MD point0 y");
    CHECK_DATA_ALIGN(0, mdAttr->stMdStaticParamsIn.width - 1, mdInfo->stMdAttr.stPnt[2].u32x,
                     &mdAttr->stMdStaticParamsIn.roi_md.pnt[2].x, "MD point2 x");
    CHECK_DATA_ALIGN(0, mdAttr->stMdStaticParamsIn.height - 1, mdInfo->stMdAttr.stPnt[2].u32y,
                     &mdAttr->stMdStaticParamsIn.roi_md.pnt[2].y, "MD point2 y");
    mdAttr->stMdStaticParamsIn.roi_md.pnt[0].x = VDF_ALIGN_X_DOWN(mdAttr->stMdStaticParamsIn.roi_md.pnt[0].x, mapSize);
    mdAttr->stMdStaticParamsIn.roi_md.pnt[0].y = VDF_ALIGN_X_DOWN(mdAttr->stMdStaticParamsIn.roi_md.pnt[0].y, mapSize);
    mdAttr->stMdStaticParamsIn.roi_md.pnt[2].x =
        VDF_ALIGN_X_UP(mdAttr->stMdStaticParamsIn.roi_md.pnt[2].x, mapSize) - 1;
    mdAttr->stMdStaticParamsIn.roi_md.pnt[2].y =
        VDF_ALIGN_X_UP(mdAttr->stMdStaticParamsIn.roi_md.pnt[2].y, mapSize) - 1;
    mdAttr->stMdStaticParamsIn.roi_md.pnt[1].x = mdAttr->stMdStaticParamsIn.roi_md.pnt[2].x;
    mdAttr->stMdStaticParamsIn.roi_md.pnt[1].y = mdAttr->stMdStaticParamsIn.roi_md.pnt[0].y;
    mdAttr->stMdStaticParamsIn.roi_md.pnt[3].x = mdAttr->stMdStaticParamsIn.roi_md.pnt[0].x;
    mdAttr->stMdStaticParamsIn.roi_md.pnt[3].y = mdAttr->stMdStaticParamsIn.roi_md.pnt[2].y;

    mdAttr->stMdStaticParamsIn.roi_md.num = mdInfo->stMdAttr.u32PointNum;
    PTREE_DBG("MD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d), point num: %d",
              mdAttr->stMdStaticParamsIn.width, mdAttr->stMdStaticParamsIn.height,
              mdAttr->stMdStaticParamsIn.roi_md.pnt[0].x, mdAttr->stMdStaticParamsIn.roi_md.pnt[0].y,
              mdAttr->stMdStaticParamsIn.roi_md.pnt[1].x, mdAttr->stMdStaticParamsIn.roi_md.pnt[1].y,
              mdAttr->stMdStaticParamsIn.roi_md.pnt[2].x, mdAttr->stMdStaticParamsIn.roi_md.pnt[2].y,
              mdAttr->stMdStaticParamsIn.roi_md.pnt[3].x, mdAttr->stMdStaticParamsIn.roi_md.pnt[3].y,
              mdAttr->stMdStaticParamsIn.roi_md.num);
    PTREE_DBG("md model is %d", mdAttr->stMdStaticParamsIn.md_alg_mode);
}

static void _PTREE_MOD_VDF_SetOdAttr(MI_VDF_OdAttr_t *odAttr, PTREE_MOD_VDF_Obj_t *vdfMod, PTREE_SUR_VDF_Info_t *odInfo)
{
    int i;
    odAttr->u8OdBufCnt                           = 4;
    odAttr->u8VDFIntvl                           = 0;
    odAttr->stOdDynamicParamsIn.thd_tamper       = 3;
    odAttr->stOdStaticParamsIn.inImgW            = VDF_ALIGN_X_UP(vdfMod->inPortWidth, 16);
    odAttr->stOdStaticParamsIn.inImgH            = VDF_ALIGN_X_UP(vdfMod->inPortHeight, 2);
    odAttr->stOdStaticParamsIn.inImgStride       = odAttr->stOdStaticParamsIn.inImgW;
    odAttr->stOdStaticParamsIn.nClrType          = OD_Y;
    odAttr->stOdStaticParamsIn.div               = odInfo->enOdWindows;
    odAttr->stOdStaticParamsIn.alpha             = 2;
    odAttr->stOdStaticParamsIn.M                 = 120;
    odAttr->stOdStaticParamsIn.MotionSensitivity = odInfo->enMotionSensitivity;
    odAttr->stOdDynamicParamsIn.tamper_blk_thd   = 1;
    odAttr->stOdDynamicParamsIn.min_duration     = 15;
    if (!strcmp(odInfo->strSensitivity, "low"))
    {
        odAttr->stOdDynamicParamsIn.tamper_blk_thd = 8;
        odAttr->stOdDynamicParamsIn.min_duration   = 30;
    }
    if (!strcmp(odInfo->strSensitivity, "middle"))
    {
        odAttr->stOdDynamicParamsIn.tamper_blk_thd = 4;
        odAttr->stOdDynamicParamsIn.min_duration   = 15;
    }
    if (!strcmp(odInfo->strSensitivity, "high"))
    {
        odAttr->stOdDynamicParamsIn.tamper_blk_thd = 2; // if 1x1 subwindow, this value should be 1
        odAttr->stOdDynamicParamsIn.min_duration   = 5;
    }
    odAttr->stOdStaticParamsIn.roi_od.num = odInfo->stOdAttr.u32PointNum;

    for (i = 0; i < odInfo->stOdAttr.u32PointNum; i++)
    {
        CHECK_DATA_ALIGN(0, odAttr->stOdStaticParamsIn.inImgW - 1, odInfo->stOdAttr.stPnt[i].u32x,
                         &odAttr->stOdStaticParamsIn.roi_od.pnt[i].x, "OD pointNum x");
        CHECK_DATA_ALIGN(0, odAttr->stOdStaticParamsIn.inImgH - 1, odInfo->stOdAttr.stPnt[i].u32y,
                         &odAttr->stOdStaticParamsIn.roi_od.pnt[i].y, "OD pointNum y");
    }
    PTREE_DBG("OD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d)", odAttr->stOdStaticParamsIn.inImgW,
              odAttr->stOdStaticParamsIn.inImgH, odAttr->stOdStaticParamsIn.roi_od.pnt[0].x,
              odAttr->stOdStaticParamsIn.roi_od.pnt[0].y, odAttr->stOdStaticParamsIn.roi_od.pnt[1].x,
              odAttr->stOdStaticParamsIn.roi_od.pnt[1].y, odAttr->stOdStaticParamsIn.roi_od.pnt[2].x,
              odAttr->stOdStaticParamsIn.roi_od.pnt[2].y, odAttr->stOdStaticParamsIn.roi_od.pnt[3].x,
              odAttr->stOdStaticParamsIn.roi_od.pnt[3].y);
}

static void _PTREE_MOD_VDF_CalMdMap(PTREE_MOD_VDF_Obj_t *vdfMod, MI_VDF_MdAttr_t *mdAttr)
{
    unsigned char mapSize   = _PTREE_MOD_VDF_GetMapSizeAlign(mdAttr->stMdStaticParamsIn.mb_size);
    vdfMod->streamMapWidth  = vdfMod->inPortWidth / mapSize;
    vdfMod->streamMapHeight = vdfMod->inPortHeight / mapSize;
    vdfMod->vdfMapStartX    = mdAttr->stMdStaticParamsIn.roi_md.pnt[0].x / mapSize;
    vdfMod->vdfMapStartY    = mdAttr->stMdStaticParamsIn.roi_md.pnt[0].y / mapSize;
    vdfMod->vdfMapEndX      = mdAttr->stMdStaticParamsIn.roi_md.pnt[2].x / mapSize;
    vdfMod->vdfMapEndY      = mdAttr->stMdStaticParamsIn.roi_md.pnt[2].y / mapSize;
    if (vdfMod->vdfMapEndX >= vdfMod->streamMapWidth)
    {
        vdfMod->vdfMapEndX = vdfMod->streamMapWidth - 1;
    }
    if (vdfMod->vdfMapEndY >= vdfMod->streamMapHeight)
    {
        vdfMod->vdfMapEndY = vdfMod->streamMapHeight - 1;
    }
    PTREE_DBG("Set md area [(%d, %d), (%d, %d)] in map [%dx%d]", vdfMod->vdfMapStartX, vdfMod->vdfMapStartY,
              vdfMod->vdfMapEndX, vdfMod->vdfMapEndY, vdfMod->streamMapWidth, vdfMod->streamMapHeight);
}
static void _PTREE_MOD_VDF_CalOdMap(PTREE_MOD_VDF_Obj_t *vdfMod, MI_VDF_OdAttr_t *odAttr)
{
    unsigned char mapDiv = 1;
    unsigned int  subWindowW, subWindowH;
    switch (odAttr->stOdStaticParamsIn.div)
    {
        case OD_WINDOW_1X1:
            mapDiv = 1;
            break;
        case OD_WINDOW_2X2:
            mapDiv = 2;
            break;
        case OD_WINDOW_3X3:
            mapDiv = 3;
            break;
        default:
            PTREE_ERR("param (%d) error! please check config", odAttr->stOdStaticParamsIn.div);
            return;
    }
    subWindowW = (odAttr->stOdStaticParamsIn.roi_od.pnt[2].x - odAttr->stOdStaticParamsIn.roi_od.pnt[0].x) / mapDiv;
    subWindowH = (odAttr->stOdStaticParamsIn.roi_od.pnt[2].y - odAttr->stOdStaticParamsIn.roi_od.pnt[0].y) / mapDiv;
    vdfMod->streamMapWidth  = vdfMod->inPortWidth / subWindowW;
    vdfMod->streamMapHeight = vdfMod->inPortHeight / subWindowH;
    vdfMod->vdfMapStartX    = odAttr->stOdStaticParamsIn.roi_od.pnt[0].x / subWindowW;
    vdfMod->vdfMapStartY    = odAttr->stOdStaticParamsIn.roi_od.pnt[0].y / subWindowH;
    vdfMod->vdfMapEndX      = odAttr->stOdStaticParamsIn.roi_od.pnt[2].x / subWindowW;
    vdfMod->vdfMapEndY      = odAttr->stOdStaticParamsIn.roi_od.pnt[2].y / subWindowH;
    if (vdfMod->vdfMapEndX >= vdfMod->streamMapWidth)
    {
        vdfMod->vdfMapEndX = vdfMod->streamMapWidth - 1;
    }
    if (vdfMod->vdfMapEndY >= vdfMod->streamMapHeight)
    {
        vdfMod->vdfMapEndY = vdfMod->streamMapHeight - 1;
    }
    PTREE_DBG("Set md area [(%d, %d), (%d, %d)] in map [%dx%d]", vdfMod->vdfMapStartX, vdfMod->vdfMapStartY,
              vdfMod->vdfMapEndX, vdfMod->vdfMapEndY, vdfMod->streamMapWidth, vdfMod->streamMapHeight);
}
// md result
static PTREE_PACKET_Obj_t *_PTREE_MOD_VDF_GetMdResult(PTREE_MOD_VDF_Obj_t *vdfMod, MI_MD_Result_t *mdResult)
{
    PTREE_PACKET_Obj_t *    packet = NULL;
    PTREE_RGN_PACKET_Map_t *map    = NULL;
    unsigned int            row, col;
    unsigned int            offset;

    packet = PTREE_RGN_PACKET_MapNew(vdfMod->streamMapWidth, vdfMod->streamMapHeight);
    map    = CONTAINER_OF(packet, PTREE_RGN_PACKET_Map_t, base);

    offset = 0;
    for (row = vdfMod->vdfMapStartY; row <= vdfMod->vdfMapEndY; ++row)
    {
        for (col = vdfMod->vdfMapStartX; col <= vdfMod->vdfMapEndX; ++col)
        {
            if (offset >= mdResult->stSubResultSize.u32RstStatusLen)
            {
                return packet;
            }
            map->data[row * vdfMod->streamMapWidth + col] = mdResult->pstMdResultStatus->paddr[offset];
            ++offset;
        }
    }
    return packet;
}

// Get od result
static PTREE_PACKET_Obj_t *_PTREE_MOD_VDF_GetOdResult(PTREE_MOD_VDF_Obj_t *vdfMod, MI_OD_Result_t *odResult)
{
    PTREE_PACKET_Obj_t *    packet = NULL;
    PTREE_RGN_PACKET_Map_t *map    = NULL;
    unsigned int            row, col;

    packet = PTREE_RGN_PACKET_MapNew(vdfMod->streamMapWidth, vdfMod->streamMapHeight);
    map    = CONTAINER_OF(packet, PTREE_RGN_PACKET_Map_t, base);

    for (row = vdfMod->vdfMapStartY; row <= vdfMod->vdfMapEndY; ++row)
    {
        for (col = vdfMod->vdfMapStartX; col <= vdfMod->vdfMapEndX; ++col)
        {
            map->data[row * vdfMod->streamMapWidth + col] =
                odResult->u8RgnAlarm[row - vdfMod->vdfMapStartY][col - vdfMod->vdfMapStartX];
        }
    }
    return packet;
}

static void *_PTREE_MOD_VDF_VdfReader(SSOS_TASK_Buffer_t *threadBuf)
{
    PTREE_SUR_VDF_Info_t *vdfInfo = NULL;
    MI_S32                s32Ret  = MI_SUCCESS;

    PTREE_MOD_VDF_Obj_t *vdfMod = NULL;

    vdfMod = (PTREE_MOD_VDF_Obj_t *)threadBuf->buf;

    if (NULL == vdfMod)
    {
        return NULL;
    }
    vdfInfo = CONTAINER_OF(vdfMod->base.base.info, PTREE_SUR_VDF_Info_t, base.base);

    MI_VDF_Result_t     stVdfResult    = {(MI_VDF_WorkMode_e)0};
    PTREE_PACKET_Obj_t *stStreamPacket = NULL;
    memset(&stVdfResult, 0x00, sizeof(MI_VDF_Result_t));
    stVdfResult.enWorkMode = vdfInfo->enVdfMode;

    s32Ret = MI_VDF_GetResult(vdfInfo->base.base.chnId, &stVdfResult, 0);
    if (MI_SUCCESS != s32Ret)
    {
        return NULL;
    }
    switch (stVdfResult.enWorkMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
        {
            if (1 == stVdfResult.stMdResult.u8Enable)
            {
                stStreamPacket = _PTREE_MOD_VDF_GetMdResult(vdfMod, &stVdfResult.stMdResult);
            }
        }
        break;
        case E_MI_VDF_WORK_MODE_OD:
        {
            if (1 == stVdfResult.stOdResult.u8Enable)
            {
                stStreamPacket = _PTREE_MOD_VDF_GetOdResult(vdfMod, &stVdfResult.stOdResult);
            }
        }
        break;
        default:
            PTREE_ERR("vdf init failed! mode type: %d", stVdfResult.enWorkMode);
    }
    MI_VDF_PutResult(vdfInfo->base.base.chnId, &stVdfResult);
    if (NULL == stStreamPacket)
    {
        return NULL;
    }
    PTREE_LINKER_Enqueue(&vdfMod->modOut.plinker.base, stStreamPacket);
    PTREE_PACKET_Del(stStreamPacket);
    return NULL;
}

static int _PTREE_MOD_VDF_CreateChannel(PTREE_MOD_VDF_Obj_t *vdfMod, PTREE_SUR_VDF_Info_t *vdfInfo)
{
    MI_VDF_ChnAttr_t stVdfChnAttr;

    if (0 == vdfMod->inPortWidth || 0 == vdfMod->inPortHeight)
    {
        PTREE_ERR("start vdf chn failed, src w: %d,  src h: %d, rect w: %d, rect h: %d, all can't = 0",
                  vdfMod->inPortWidth, vdfMod->inPortHeight);
        return SSOS_DEF_FAIL;
    }
    memset(&stVdfChnAttr, 0, sizeof(MI_VDF_ChnAttr_t));
    stVdfChnAttr.enWorkMode = vdfInfo->enVdfMode;
    switch (vdfInfo->enVdfMode)
    {
        case E_MI_VDF_WORK_MODE_MD:
        {
            _PTREE_MOD_VDF_SetMdAttr(&stVdfChnAttr.stMdAttr, vdfMod, vdfInfo);
            _PTREE_MOD_VDF_CalMdMap(vdfMod, &stVdfChnAttr.stMdAttr);
        }
        break;

        case E_MI_VDF_WORK_MODE_OD:
        {
            _PTREE_MOD_VDF_SetOdAttr(&stVdfChnAttr.stOdAttr, vdfMod, vdfInfo);
            _PTREE_MOD_VDF_CalOdMap(vdfMod, &stVdfChnAttr.stOdAttr);
        }
        break;
        default:
            PTREE_ERR("vdf init failed! mode type: %d", vdfInfo->enVdfMode);
            return SSOS_DEF_FAIL;
    }
    if (MI_SUCCESS != MI_VDF_CreateChn(vdfInfo->base.base.chnId, &stVdfChnAttr))
    {
        PTREE_ERR("vdf init failed! mode type: %d", vdfInfo->enVdfMode);
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VDF_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_PACKET_Info_t *_PTREE_MOD_VDF_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return NULL;
}
static int _PTREE_MOD_VDF_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_VDF_Obj_t *   vdfMod = CONTAINER_OF(modOut, PTREE_MOD_VDF_Obj_t, modOut);
    struct SSOS_TASK_Attr_s taskAttr;
    char                    taskName[PTREE_MOD_VDF_READER_TASK_NAME_LEN];

    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }
    memset(&taskAttr, 0, sizeof(struct SSOS_TASK_Attr_s));
    PTREE_MOD_OutKeyStr(&vdfMod->modOut, taskName, PTREE_MOD_VDF_READER_TASK_NAME_LEN);
    taskAttr.doSignal         = NULL;
    taskAttr.doMonitor        = _PTREE_MOD_VDF_VdfReader;
    taskAttr.monitorCycleSec  = 0;
    taskAttr.monitorCycleNsec = 20000000;
    taskAttr.isResetTimer     = 0;
    taskAttr.inBuf.buf        = (void *)vdfMod;
    taskAttr.inBuf.size       = 0;
    taskAttr.threadAttr.name  = taskName;

    vdfMod->taskHandle = SSOS_TASK_Open(&taskAttr);
    if (!vdfMod->taskHandle)
    {
        PTREE_ERR("Vdf task open error!");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(vdfMod->taskHandle);
    PTREE_DBG("Vdf reader start");
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VDF_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_VDF_Obj_t *vdfMod = CONTAINER_OF(modOut, PTREE_MOD_VDF_Obj_t, modOut);
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (!vdfMod->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    SSOS_TASK_Stop(vdfMod->taskHandle);
    SSOS_TASK_Close(vdfMod->taskHandle);
    vdfMod->taskHandle = NULL;
    PTREE_DBG("Vdf reader stop");
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_VDF_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_VDF_Obj_t *vdfMod = CONTAINER_OF(sysMod, PTREE_MOD_VDF_Obj_t, base);

    (void)vdfMod;

    if (!g_vdfCreateDev)
    {
        MI_VDF_Init();
    }
    g_vdfCreateDev++;

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VDF_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_VDF_Obj_t *vdfMod = CONTAINER_OF(sysMod, PTREE_MOD_VDF_Obj_t, base);

    (void)vdfMod;

    g_vdfCreateDev--;
    if (!g_vdfCreateDev)
    {
        MI_VDF_Uninit();
        PTREE_DBG("deinit vdf");
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VDF_Prepare(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_VDF_Obj_t *    vdfMod        = CONTAINER_OF(sysMod, PTREE_MOD_VDF_Obj_t, base);
    PTREE_MOD_InObj_t *      modIn         = NULL;
    PTREE_SUR_VDF_Info_t *   vdfInfo       = CONTAINER_OF(sysMod->base.info, PTREE_SUR_VDF_Info_t, base.base);
    PTREE_PACKET_RAW_Info_t *rawPacketInfo = NULL;
    PTREE_PACKET_Info_t *    packetInfo    = NULL;

    if (sysMod->base.info->inCnt != 1)
    {
        PTREE_ERR("Input port count is not 1");
        return SSOS_DEF_FAIL;
    }

    modIn = sysMod->base.arrModIn[0];

    packetInfo = PTREE_MESSAGE_GetPacketInfo(&modIn->message);
    if (!packetInfo)
    {
        PTREE_ERR("Packet info is null");
        return SSOS_DEF_FAIL;
    }
    if (!PTREE_PACKET_InfoLikely(packetInfo, PTREE_PACKET_INFO_TYPE(raw)))
    {
        PTREE_ERR("Packet info type %s is not support", packetInfo->type);
        PTREE_PACKET_InfoDel(packetInfo);
        return SSOS_DEF_FAIL;
    }
    rawPacketInfo = CONTAINER_OF(packetInfo, PTREE_PACKET_RAW_Info_t, base);

    vdfMod->inPortWidth  = rawPacketInfo->rawInfo.width;
    vdfMod->inPortHeight = rawPacketInfo->rawInfo.height;

    rawPacketInfo = NULL;
    PTREE_PACKET_InfoDel(packetInfo);
    packetInfo = NULL;

    if (SSOS_DEF_OK != _PTREE_MOD_VDF_CreateChannel(vdfMod, vdfInfo))
    {
        return SSOS_DEF_FAIL;
    }

    if (!g_vdfRunning[vdfInfo->enVdfMode])
    {
        MI_VDF_Run(vdfInfo->enVdfMode);
    }
    ++g_vdfRunning[vdfInfo->enVdfMode];

    MI_VDF_EnableSubWindow(vdfInfo->base.base.chnId, 0, 0, 1);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VDF_UnPrepare(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_VDF_Obj_t * vdfMod  = CONTAINER_OF(sysMod, PTREE_MOD_VDF_Obj_t, base);
    PTREE_SUR_VDF_Info_t *vdfInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_VDF_Info_t, base.base);

    MI_VDF_EnableSubWindow(vdfMod->base.base.info->chnId, 0, 0, 0);

    --g_vdfRunning[vdfInfo->enVdfMode];
    if (!g_vdfRunning[vdfInfo->enVdfMode])
    {
        MI_VDF_Stop(vdfInfo->enVdfMode);
    }

    MI_VDF_DestroyChn(vdfMod->base.base.info->chnId);
    return SSOS_DEF_OK;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_VDF_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_VDF_Obj_t *vdfMod = CONTAINER_OF(mod, PTREE_MOD_VDF_Obj_t, base.base);
    if (loopId > 0)
    {
        return NULL;
    }
    return PTREE_MOD_OutObjDup(&vdfMod->modOut);
}
static void _PTREE_MOD_VDF_Destruct(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_VDF_Obj_t *vdfMod = CONTAINER_OF(sysMod, PTREE_MOD_VDF_Obj_t, base);
    PTREE_MOD_OutObjDel(&vdfMod->modOut);
}
static void _PTREE_MOD_VDF_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_VDF_Obj_t, base));
}

PTREE_MOD_Obj_t *PTREE_MOD_VDF_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_VDF_Obj_t *vdfMod = NULL;

    vdfMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VDF_Obj_t));
    if (!vdfMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(vdfMod, 0, sizeof(PTREE_MOD_VDF_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&vdfMod->base, &G_PTREE_MOD_VDF_SYS_OPS, tag, E_MI_MODULE_ID_VDF))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&vdfMod->modOut, &G_PTREE_MOD_VDF_OUT_OPS, &vdfMod->base.base, 0))
    {
        goto ERR_MOD_OUT_INIT;
    }

    PTREE_MOD_OutObjRegister(&vdfMod->modOut, &G_PTREE_MOD_VDF_OUT_HOOK);
    PTREE_MOD_SYS_ObjRegister(&vdfMod->base, &G_PTREE_MOD_VDF_SYS_HOOK);
    return &vdfMod->base.base;

ERR_MOD_OUT_INIT:
    PTREE_MOD_ObjDel(&vdfMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(vdfMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(VDF, PTREE_MOD_VDF_New);

#endif
