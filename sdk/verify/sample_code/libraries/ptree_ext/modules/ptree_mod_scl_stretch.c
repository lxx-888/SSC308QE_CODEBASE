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

#include "mi_common_datatype.h"
#include "mi_scl_datatype.h"
#include "mi_scl.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_packet_raw.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_scl.h"
#include "ptree_mod_scl_base.h"
#include "ptree_mod_scl_stretch.h"
#include "ptree_sur_scl_stretch.h"
#include "ptree_sur_sys.h"
#include "ptree_maker.h"
#include "ptree_packer.h"
#include "ptree_mma_packet.h"
#include "ssos_task.h"
#include "ptree_linker.h"
#include "ptree_nlinker_async.h"
#include "ptree_nlinker_out.h"
#include "ptree_packet.h"

#define SCL_STRETCH_OSD_W       (16384)
#define SCL_STRETCH_OSD_H       (64)
#define SCL_STRETCH_OSD_PIXEL_T short

#define PTREE_MOD_SCL_STRETCH_READER_TASK_NAME_LEN 32

typedef struct PTREE_MOD_SCL_STRETCH_Obj_s    PTREE_MOD_SCL_STRETCH_Obj_t;
typedef struct PTREE_MOD_SCL_STRETCH_InObj_s  PTREE_MOD_SCL_STRETCH_InObj_t;
typedef struct PTREE_MOD_SCL_STRETCH_OutObj_s PTREE_MOD_SCL_STRETCH_OutObj_t;

struct PTREE_MOD_SCL_STRETCH_InObj_s
{
    PTREE_MOD_InObj_t         base;
    PTREE_NLINKER_ASYNC_Obj_t asyncLinker;
};
struct PTREE_MOD_SCL_STRETCH_OutObj_s
{
    PTREE_MOD_OutObj_t      base;
    PTREE_NLINKER_OUT_Obj_t outLinker;
    void *                  taskHandle;
};

struct PTREE_MOD_SCL_STRETCH_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
    // PTREE_PACKER_Obj_t  packer;
    PTREE_MMA_PACKET_Packer_t packer;
};

static int _PTREE_MOD_SCL_STRETCH_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static int _PTREE_MOD_SCL_STRETCH_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_SCL_STRETCH_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut,
                                                                             int                      ms);
static int _PTREE_MOD_SCL_STRETCH_FillPacket(PTREE_PACKET_Obj_t *snedPacket, PTREE_MOD_OutObj_t *modOut, int ms);
static int _PTREE_MOD_SCL_STRETCH_InGetType(PTREE_MOD_InObj_t *modIn);
static int _PTREE_MOD_SCL_STRETCH_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static PTREE_LINKER_Obj_t *_PTREE_MOD_SCL_STRETCH_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_SCL_STRETCH_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static PTREE_LINKER_Obj_t *_PTREE_MOD_SCL_STRETCH_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut);

static int _PTREE_MOD_SCL_STRETCH_OutGetType(PTREE_MOD_OutObj_t *modOut);

static int                 _PTREE_MOD_SCL_STRETCH_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_SCL_STRETCH_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_SCL_STRETCH_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_STRETCH_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_SCL_STRETCH_Destruct(PTREE_MOD_SYS_Obj_t *sysMod);
static void                _PTREE_MOD_SCL_STRETCH_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static int                _PTREE_MOD_SCL_STRETCH_InStart(PTREE_MOD_InObj_t *modIn);
static void               _PTREE_MOD_SCL_STRETCH_InDestruct(PTREE_MOD_InObj_t *modIn);
static void               _PTREE_MOD_SCL_STRETCH_InFree(PTREE_MOD_InObj_t *modIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_SCL_STRETCH_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static int _PTREE_MOD_SCL_STRETCH_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int _PTREE_MOD_SCL_STRETCH_OutUnLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
// static void                _PTREE_MOD_SCL_STRETCH_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *    sysModOut,
//                                                                    PTREE_PACKET_RAW_RawInfo_t *rawInfo);
static void                _PTREE_MOD_SCL_STRETCH_OutDestruct(PTREE_MOD_OutObj_t *modOut);
static void                _PTREE_MOD_SCL_STRETCH_OutFree(PTREE_MOD_OutObj_t *modOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_STRETCH_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static const PTREE_NLINKER_ASYNC_Hook_t G_PTREE_MOD_SCL_STRETCH_IN_ASYNC_LINKER_HOOK = {};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_MOD_SCL_STRETCH_OUT_LINKER_OPS = {
    .enqueue           = _PTREE_MOD_SCL_STRETCH_OutLinkerEnqueue,
    .dequeueOut        = _PTREE_MOD_SCL_STRETCH_OutLinkerDequeueOut,
    .dequeueFromInside = _PTREE_MOD_SCL_STRETCH_OutLinkerDequeueFromInside,
};

static const PTREE_NLINKER_OUT_Hook_t G_PTREE_MOD_SCL_STRETCH_OUT_LINKER_HOOK = {};

static const PTREE_MOD_InOps_t G_PTREE_MOD_SCL_STRETCH_IN_OPS = {
    .start         = _PTREE_MOD_SCL_STRETCH_InStart,
    .getType       = _PTREE_MOD_SCL_STRETCH_InGetType,
    .unlinked      = _PTREE_MOD_SCL_STRETCH_InUnlinked,
    .createNLinker = _PTREE_MOD_SCL_STRETCH_InCreateNLinker,
    .createPacker  = _PTREE_MOD_SCL_STRETCH_InCreatePacker,
};

static const PTREE_MOD_InHook_t G_PTREE_MOD_SCL_STRETCH_IN_HOOK = {
    .destruct = _PTREE_MOD_SCL_STRETCH_InDestruct,
    .free     = _PTREE_MOD_SCL_STRETCH_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_SCL_STRETCH_OUT_OPS = {
    .linked        = _PTREE_MOD_SCL_STRETCH_OutLinked,
    .unlinked      = _PTREE_MOD_SCL_STRETCH_OutUnLinked,
    .getType       = _PTREE_MOD_SCL_STRETCH_OutGetType,
    .createNLinker = _PTREE_MOD_SCL_STRETCH_OutCreateNLinker,
    //.getPacketInfo = _PTREE_MOD_SCL_STRETCH_OutGetPacketInfo,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_SCL_STRETCH_OUT_HOOK = {
    .destruct = _PTREE_MOD_SCL_STRETCH_OutDestruct,
    .free     = _PTREE_MOD_SCL_STRETCH_OutFree,
};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_SCL_STRETCH_SYS_OPS = {
    .init         = _PTREE_MOD_SCL_STRETCH_Init,
    .deinit       = _PTREE_MOD_SCL_STRETCH_Deinit,
    .createModIn  = _PTREE_MOD_SCL_STRETCH_CreateModIn,
    .createModOut = _PTREE_MOD_SCL_STRETCH_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_SCL_STRETCH_SYS_HOOK = {
    .destruct = _PTREE_MOD_SCL_STRETCH_Destruct,
    .free     = _PTREE_MOD_SCL_STRETCH_Free,
};

static MI_PHY g_phyOsdBuf = 0;

static int _PTREE_MOD_SCL_STRETCH_FillPacket(PTREE_PACKET_Obj_t *sendPacket, PTREE_MOD_OutObj_t *modOut, int ms)
{
    unsigned int              i         = 0;
    unsigned int              j         = 0;
    PTREE_PACKET_Obj_t *      packet    = NULL;
    PTREE_MMA_PACKET_RawPa_t *rawPa     = NULL;
    PTREE_MMA_PACKET_RawPa_t *sendRawPa = NULL;

    PTREE_MOD_SCL_STRETCH_InObj_t *sclModIn =
        CONTAINER_OF(*(modOut->thisMod->arrModIn), PTREE_MOD_SCL_STRETCH_InObj_t, base);
    PTREE_SUR_SCL_STRETCH_InInfo_t *sclInInfo =
        CONTAINER_OF(sclModIn->base.info, PTREE_SUR_SCL_STRETCH_InInfo_t, base.base);
    PTREE_SUR_SCL_STRETCH_OutInfo_t *sclOutInfo =
        CONTAINER_OF(modOut->info, PTREE_SUR_SCL_STRETCH_OutInfo_t, base.base);

    sendRawPa = CONTAINER_OF(sendPacket, PTREE_MMA_PACKET_RawPa_t, base);

    for (i = 0; i < sclOutInfo->u32RowNum; ++i)
    {
        for (j = 0; j < sclOutInfo->u32ColNum; ++j)
        {
            MI_SCL_DirectBuf_t  stSrcBuf;
            MI_SCL_DirectBuf_t  stDstBuf;
            MI_SYS_WindowRect_t stSrcRect;
            MI_SCL_FilterType_e eFilterType;
            unsigned int        yDiv[2] = {1, 1};

            memset(&stSrcBuf, 0, sizeof(MI_SCL_DirectBuf_t));
            memset(&stDstBuf, 0, sizeof(MI_SCL_DirectBuf_t));
            memset(&stSrcRect, 0, sizeof(MI_SYS_WindowRect_t));

            packet = PTREE_NLINKER_ASYNC_WaitPacket(&sclModIn->asyncLinker, ms);
            if (!packet)
            {
                // PTREE_ERR("packet not ok!");
                return -1;
            }
            // PTREE_DBG("get packet ok!");

            if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(raw_pa)))
            {
                PTREE_ERR("packet info type %s is not support", packet->info->type);
                PTREE_PACKET_Del(packet);
                packet = NULL;
                return -1;
            }

            rawPa = CONTAINER_OF(packet, PTREE_MMA_PACKET_RawPa_t, base);

            stSrcRect.u16X      = sclInInfo->u16CropX;
            stSrcRect.u16Y      = sclInInfo->u16CropY;
            stSrcRect.u16Width  = sclInInfo->u16CropW;
            stSrcRect.u16Height = sclInInfo->u16CropH;

            stSrcBuf.ePixelFormat = PTREE_MOD_SYS_PtreeFmtToSysFmt(rawPa->info.rawInfo.fmt);
            stSrcBuf.u32Width     = rawPa->info.rawInfo.width;
            stSrcBuf.u32Height    = rawPa->info.rawInfo.height;
            stSrcBuf.u32BuffSize  = rawPa->rawData.size[0] + rawPa->rawData.size[1];
            stSrcBuf.u32Stride[0] = rawPa->rawData.stride[0];
            stSrcBuf.u32Stride[1] = rawPa->rawData.stride[1];
            stSrcBuf.phyAddr[0]   = rawPa->rawData.phy[0];
            stSrcBuf.phyAddr[1]   = rawPa->rawData.phy[1];
            if (E_PTREE_PACKET_RAW_FORMAT_YUV420SP == rawPa->info.rawInfo.fmt
                || E_PTREE_PACKET_RAW_FORMAT_YUV420SP_NV21 == rawPa->info.rawInfo.fmt)
            {
                yDiv[0] = 1;
                yDiv[1] = 2;
            }
            stDstBuf.ePixelFormat = PTREE_MOD_SYS_PtreeFmtToSysFmt(sendRawPa->info.rawInfo.fmt);
            stDstBuf.u32Width     = sclInInfo->u16CropW;
            stDstBuf.u32Height    = sclInInfo->u16CropH;
            stDstBuf.u32BuffSize  = stSrcBuf.u32BuffSize;
            stDstBuf.u32Stride[0] = sendRawPa->rawData.stride[0];
            stDstBuf.u32Stride[1] = sendRawPa->rawData.stride[1];
            stDstBuf.phyAddr[0] = sendRawPa->rawData.phy[0] + stDstBuf.u32Stride[0] * sclInInfo->u16CropH * i / yDiv[0]
                                  + stSrcBuf.u32Stride[0] * j;
            stDstBuf.phyAddr[1] = sendRawPa->rawData.phy[1] + stDstBuf.u32Stride[1] * sclInInfo->u16CropH * i / yDiv[1]
                                  + stSrcBuf.u32Stride[1] * j;
            eFilterType = E_MI_SCL_FILTER_TYPE_AUTO;

            if (g_phyOsdBuf && i == 0)
            {
                MI_SCL_DirectOsdBuf_t stDstOsdBuf;
                memset(&stDstOsdBuf, 0, sizeof(MI_SCL_DirectOsdBuf_t));
                stDstOsdBuf.ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB1555;
                stDstOsdBuf.u8OsdBufCnt  = 1;
                stDstOsdBuf.astOsdBuf[0].phyAddr =
                    g_phyOsdBuf + sclInInfo->u16CropW * sizeof(SCL_STRETCH_OSD_PIXEL_T) * j;
                stDstOsdBuf.astOsdBuf[0].u32X      = 0;
                stDstOsdBuf.astOsdBuf[0].u32Y      = 0;
                stDstOsdBuf.astOsdBuf[0].u32Width  = SCL_STRETCH_OSD_W;
                stDstOsdBuf.astOsdBuf[0].u32Height = SCL_STRETCH_OSD_H;
                stDstOsdBuf.astOsdBuf[0].u32Stride = SCL_STRETCH_OSD_W * sizeof(SCL_STRETCH_OSD_PIXEL_T);
                stDstOsdBuf.astOsdBuf[0].stOsdAlphaAttr.eAlphaMode = E_MI_SCL_DIRECT_BUF_OSD_CONSTANT_ALPHT;
                stDstOsdBuf.astOsdBuf[0].stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 0xff;
                if (MI_SUCCESS != MI_SCL_StretchBufOsd(&stSrcBuf, &stSrcRect, &stDstBuf, &stDstOsdBuf, eFilterType))
                {
                    PTREE_ERR("MI_SCL_StretchBufOsd Failed\n");
                    return -1;
                }
            }
            else
            {
                if (MI_SUCCESS != MI_SCL_StretchBuf(&stSrcBuf, &stSrcRect, &stDstBuf, eFilterType))
                {
                    PTREE_ERR("MI_SCL_StretchBuf Failed\n");
                    return -1;
                }
            }
        }
    }
    PTREE_PACKET_Del(packet);
    packet = NULL;
    return 0;
}

static void *_PTREE_MOD_SCL_STRETCH_Reader(SSOS_TASK_Buffer_t *taskBuf)
{
    PTREE_PACKET_Obj_t *           sendPacket = NULL;
    PTREE_PACKET_RAW_Info_t        sendRawInfo;
    PTREE_PACKET_RAW_RawInfo_t     rawInfo;
    PTREE_MOD_OutObj_t *           modOut = (PTREE_MOD_OutObj_t *)taskBuf->buf;
    PTREE_MOD_SCL_STRETCH_InObj_t *sclModIn =
        CONTAINER_OF(*(modOut->thisMod->arrModIn), PTREE_MOD_SCL_STRETCH_InObj_t, base);
    PTREE_SUR_SCL_STRETCH_InInfo_t *sclInInfo =
        CONTAINER_OF(sclModIn->base.info, PTREE_SUR_SCL_STRETCH_InInfo_t, base.base);
    PTREE_SUR_SCL_STRETCH_OutInfo_t *sclOutInfo =
        CONTAINER_OF(modOut->info, PTREE_SUR_SCL_STRETCH_OutInfo_t, base.base);

    memset(&sendRawInfo, 0, sizeof(PTREE_PACKET_RAW_Info_t));
    rawInfo.fmt    = PTREE_MOD_SYS_SysFmtToPtreeFmt(sclOutInfo->u32VideoFormat);
    rawInfo.width  = sclInInfo->u16CropW * sclOutInfo->u32ColNum;
    rawInfo.height = sclInInfo->u16CropH * sclOutInfo->u32RowNum;
    PTREE_MMA_PACKET_RawPaInfoInit(&sendRawInfo, &rawInfo);
    sendPacket = PTREE_PACKER_Make(&modOut->packer.base, &sendRawInfo.base);
    if (!sendPacket)
    {
        PTREE_ERR("get out port buffer failed");
        return NULL;
    }

    // PTREE_DBG("get out port buffer ok");

    if (0 == _PTREE_MOD_SCL_STRETCH_FillPacket(sendPacket, modOut, 10))
    {
        PTREE_LINKER_Enqueue(&modOut->plinker.base, sendPacket);
    }
    PTREE_PACKET_Del(sendPacket);
    return NULL;
}

static int _PTREE_MOD_SCL_STRETCH_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_SCL_STRETCH_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}

static PTREE_PACKET_Obj_t *_PTREE_MOD_SCL_STRETCH_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut,
                                                                             int                      ms)
{
    PTREE_PACKET_Obj_t *            sendPacket = NULL;
    PTREE_PACKET_RAW_Info_t         sendRawInfo;
    PTREE_PACKET_RAW_RawInfo_t      rawInfo;
    PTREE_MOD_SCL_STRETCH_OutObj_t *sclModOut = CONTAINER_OF(nlinkerOut, PTREE_MOD_SCL_STRETCH_OutObj_t, outLinker);
    PTREE_MOD_OutObj_t *            modOut    = &(sclModOut->base);

    PTREE_MOD_SCL_STRETCH_InObj_t *sclModIn =
        CONTAINER_OF(*(modOut->thisMod->arrModIn), PTREE_MOD_SCL_STRETCH_InObj_t, base);
    PTREE_SUR_SCL_STRETCH_InInfo_t *sclInInfo =
        CONTAINER_OF(sclModIn->base.info, PTREE_SUR_SCL_STRETCH_InInfo_t, base.base);
    PTREE_SUR_SCL_STRETCH_OutInfo_t *sclOutInfo =
        CONTAINER_OF(modOut->info, PTREE_SUR_SCL_STRETCH_OutInfo_t, base.base);

    memset(&sendRawInfo, 0, sizeof(PTREE_PACKET_RAW_Info_t));
    rawInfo.fmt    = PTREE_MOD_SYS_SysFmtToPtreeFmt(sclOutInfo->u32VideoFormat);
    rawInfo.width  = sclInInfo->u16CropW * sclOutInfo->u32ColNum;
    rawInfo.height = sclInInfo->u16CropH * sclOutInfo->u32RowNum;
    PTREE_MMA_PACKET_RawPaInfoInit(&sendRawInfo, &rawInfo);
    sendPacket = PTREE_PACKER_Make(&modOut->packer.base, &sendRawInfo.base);
    if (!sendPacket)
    {
        PTREE_ERR("get out port buffer failed");
        return NULL;
    }
    // PTREE_DBG("get out port buffer ok");

    if (0 != _PTREE_MOD_SCL_STRETCH_FillPacket(sendPacket, modOut, ms))
    {
        PTREE_PACKET_Del(sendPacket);
        return NULL;
    }

    return sendPacket;
}

static int _PTREE_MOD_SCL_STRETCH_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static int _PTREE_MOD_SCL_STRETCH_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_SCL_STRETCH_InObj_t *sclModIn = CONTAINER_OF(modIn, PTREE_MOD_SCL_STRETCH_InObj_t, base);
    if (ref)
    {
        return SSOS_DEF_OK;
    }
    PTREE_NLINKER_ASYNC_Clear(&sclModIn->asyncLinker);
    return SSOS_DEF_OK;
}

static PTREE_LINKER_Obj_t *_PTREE_MOD_SCL_STRETCH_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SCL_STRETCH_InObj_t *sclModIn = CONTAINER_OF(modIn, PTREE_MOD_SCL_STRETCH_InObj_t, base);
    return PTREE_LINKER_Dup(&sclModIn->asyncLinker.base);
}

static PTREE_PACKER_Obj_t *_PTREE_MOD_SCL_STRETCH_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_SCL_STRETCH_Obj_t *sclMod = CONTAINER_OF(modIn->thisMod, PTREE_MOD_SCL_STRETCH_Obj_t, base.base);
    *isFast                             = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&sclMod->packer.base);
}

static int _PTREE_MOD_SCL_STRETCH_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static PTREE_LINKER_Obj_t *_PTREE_MOD_SCL_STRETCH_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SCL_STRETCH_OutObj_t *sclModOut = CONTAINER_OF(modOut, PTREE_MOD_SCL_STRETCH_OutObj_t, base);
    return PTREE_LINKER_Dup(&sclModOut->outLinker.base);
}

static int _PTREE_MOD_SCL_STRETCH_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SCL_STRETCH_Info_t *sclInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SCL_STRETCH_Info_t, base.base);
    int                           i = 0, j = 0;
    SCL_STRETCH_OSD_PIXEL_T *     virt = NULL;
    SCL_STRETCH_OSD_PIXEL_T       val  = 0;
    MI_SCL_DevAttr_t              sclDevAttr;
    memset(&sclDevAttr, 0, sizeof(MI_SCL_DevAttr_t));
    sclDevAttr.u32NeedUseHWOutPortMask = sclInfo->u32HwPortMode;

    if (SSOS_DEF_OK != PTREE_MOD_SCL_BASE_CreateDevice(sclInfo->base.base.devId, &sclDevAttr))
    {
        PTREE_ERR("MI_SCL_CreateDevice(%d) failed", sclInfo->base.base.devId);
        return SSOS_DEF_FAIL;
    }

    if (!sclInfo->u32OsdEn)
    {
        return SSOS_DEF_OK;
    }

    // Alloc and fill osd buffer
    if (MI_SUCCESS
        != MI_SYS_MMA_Alloc(0, NULL, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SCL_STRETCH_OSD_PIXEL_T),
                            &g_phyOsdBuf))
    {
        PTREE_ERR("MI_SYS_Mma_Alloc failed.\n");
        return SSOS_DEF_FAIL;
    }
    if (MI_SUCCESS
        != MI_SYS_Mmap(g_phyOsdBuf, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SCL_STRETCH_OSD_PIXEL_T),
                       (void **)&virt, TRUE))
    {
        PTREE_ERR("MI_SYS_Mmap failed\n");
        return SSOS_DEF_FAIL;
    }
    for (i = 0; i < SCL_STRETCH_OSD_H; ++i)
    {
        val = 0;
        for (j = 0; j < SCL_STRETCH_OSD_W; ++j)
        {
            virt[i * SCL_STRETCH_OSD_W + j] = val;
            val += 2;
        }
    }
    MI_SYS_FlushInvCache(virt, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SCL_STRETCH_OSD_PIXEL_T));
    MI_SYS_Munmap(virt, SCL_STRETCH_OSD_W * SCL_STRETCH_OSD_H * sizeof(SCL_STRETCH_OSD_PIXEL_T));

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SCL_STRETCH_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_SCL_STRETCH_Info_t *sclInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_SCL_STRETCH_Info_t, base.base);

    PTREE_MOD_SCL_BASE_DestroyDevice(sclInfo->base.base.devId);

    if (g_phyOsdBuf)
    {
        MI_SYS_MMA_Free(0, g_phyOsdBuf);
        g_phyOsdBuf = 0;
    }

    return SSOS_DEF_OK;
}

static PTREE_MOD_InObj_t *_PTREE_MOD_SCL_STRETCH_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    // PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    //(void)loopId;
    return _PTREE_MOD_SCL_STRETCH_InNew(mod, loopId);
}

static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_STRETCH_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    // PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    //(void)loopId;
    return _PTREE_MOD_SCL_STRETCH_OutNew(mod, loopId);
}

static void _PTREE_MOD_SCL_STRETCH_Destruct(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_MOD_SCL_STRETCH_Obj_t *sclMod = CONTAINER_OF(sysMod, PTREE_MOD_SCL_STRETCH_Obj_t, base);
    PTREE_PACKER_Del(&sclMod->packer.base);
}

static void _PTREE_MOD_SCL_STRETCH_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_SCL_STRETCH_Obj_t, base));
}

static int _PTREE_MOD_SCL_STRETCH_InStart(PTREE_MOD_InObj_t *modIn)
{
    PTREE_SUR_SCL_STRETCH_InInfo_t *sclInInfo = CONTAINER_OF(modIn->info, PTREE_SUR_SCL_STRETCH_InInfo_t, base.base);
    PTREE_SUR_SYS_InInfo_t *        sysInInfo = &sclInInfo->base;
    // PTREE_SUR_Info_t *      info      = sysModIn->base.thisMod->info;
    MI_SYS_WindowRect_t cropRect;

    if (sysInInfo->bindType != E_MI_SYS_BIND_TYPE_FRAME_BASE)
    {
        return SSOS_DEF_OK;
    }

    if (!sclInInfo->u16CropW || !sclInInfo->u16CropH)
    {
        return SSOS_DEF_OK;
    }

    memset(&cropRect, 0, sizeof(MI_SYS_WindowRect_t));
    cropRect.u16X      = sclInInfo->u16CropX;
    cropRect.u16Y      = sclInInfo->u16CropY;
    cropRect.u16Width  = sclInInfo->u16CropW;
    cropRect.u16Height = sclInInfo->u16CropH;

    return SSOS_DEF_OK;
}

static void _PTREE_MOD_SCL_STRETCH_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SCL_STRETCH_InObj_t *sclModIn = CONTAINER_OF(modIn, PTREE_MOD_SCL_STRETCH_InObj_t, base);

    PTREE_LINKER_Del(&sclModIn->asyncLinker.base);
    PTREE_MOD_InObjDel(&sclModIn->base);
}

static void _PTREE_MOD_SCL_STRETCH_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(CONTAINER_OF(modIn, PTREE_MOD_SCL_STRETCH_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_SCL_STRETCH_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SCL_STRETCH_InObj_t *sclModIn = NULL;

    sclModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SCL_STRETCH_InObj_t));
    if (!sclModIn)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(sclModIn, 0, sizeof(PTREE_MOD_SCL_STRETCH_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&sclModIn->base, &G_PTREE_MOD_SCL_STRETCH_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }

    if (SSOS_DEF_OK != PTREE_NLINKER_ASYNC_Init(&sclModIn->asyncLinker, 0, 10))
    {
        goto ERR_LINKER_INIT;
    }
    PTREE_NLINKER_ASYNC_Register(&sclModIn->asyncLinker, &G_PTREE_MOD_SCL_STRETCH_IN_ASYNC_LINKER_HOOK);

    PTREE_MOD_InObjRegister(&sclModIn->base, &G_PTREE_MOD_SCL_STRETCH_IN_HOOK);
    return &sclModIn->base;

ERR_LINKER_INIT:
    PTREE_MOD_InObjDel(&sclModIn->base);
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(sclModIn);
ERR_MEM_ALLOC:
    return NULL;
}

static int _PTREE_MOD_SCL_STRETCH_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    char             taskName[PTREE_MOD_SCL_STRETCH_READER_TASK_NAME_LEN];
    SSOS_TASK_Attr_t taskAttr;
    // PTREE_SUR_SCL_STRETCH_OutInfo_t *sclOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_SCL_STRETCH_OutInfo_t,
    // base.base);
    //  PTREE_SUR_Info_t *info = sysModOut->base.thisMod->info;
    PTREE_MOD_SCL_STRETCH_OutObj_t *sclModOut = CONTAINER_OF(modOut, PTREE_MOD_SCL_STRETCH_OutObj_t, base);

    if (ref)
    {
        return SSOS_DEF_OK;
    }

    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }

    PTREE_MOD_OutKeyStr(modOut, taskName, PTREE_MOD_SCL_STRETCH_READER_TASK_NAME_LEN);
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    taskAttr.doSignal         = NULL;
    taskAttr.doMonitor        = _PTREE_MOD_SCL_STRETCH_Reader;
    taskAttr.monitorCycleSec  = 0;
    taskAttr.monitorCycleNsec = 0;
    taskAttr.isResetTimer     = 0;
    taskAttr.inBuf.buf        = (void *)modOut;
    taskAttr.inBuf.size       = 0;
    taskAttr.threadAttr.name  = taskName;
    sclModOut->taskHandle     = SSOS_TASK_Open(&taskAttr);
    if (!sclModOut->taskHandle)
    {
        PTREE_ERR("SCL STRETCH Task create error!");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(sclModOut->taskHandle);
    PTREE_DBG("SCL STRETCH Task start!");

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SCL_STRETCH_OutUnLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    // PTREE_SUR_SCL_STRETCH_OutInfo_t *sclOutInfo = CONTAINER_OF(sysModOut->base.info, PTREE_SUR_SCL_STRETCH_OutInfo_t,
    // base.base); PTREE_SUR_Info_t *info = sysModOut->base.thisMod->info;
    PTREE_MOD_SCL_STRETCH_OutObj_t *sclModOut = NULL;
    sclModOut                                 = CONTAINER_OF(modOut, PTREE_MOD_SCL_STRETCH_OutObj_t, base);

    if (ref)
    {
        return SSOS_DEF_OK;
    }

    if (!sclModOut->taskHandle)
    {
        return SSOS_DEF_OK;
    }

    SSOS_TASK_Stop(sclModOut->taskHandle);
    SSOS_TASK_Close(sclModOut->taskHandle);
    sclModOut->taskHandle = NULL;

    return SSOS_DEF_OK;
}
/*
static void _PTREE_MOD_SCL_STRETCH_OutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *    sysModOut,
                                                    PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    PTREE_SUR_SCL_STRETCH_OutInfo_t *sclOutInfo =
        CONTAINER_OF(sysModOut->base.info, PTREE_SUR_SCL_STRETCH_OutInfo_t, base.base);
    rawInfo->fmt = PTREE_MOD_SYS_SysFmtToPtreeFmt(sclOutInfo->u32VideoFormat);
    // rawInfo->width                      = sclOutInfo->u16Width;
    // rawInfo->height                     = sclOutInfo->u16Height;
}
*/

static void _PTREE_MOD_SCL_STRETCH_OutDestruct(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SCL_STRETCH_OutObj_t *sclModOut = CONTAINER_OF(modOut, PTREE_MOD_SCL_STRETCH_OutObj_t, base);

    PTREE_LINKER_Del(&sclModOut->outLinker.base);
    PTREE_MOD_OutObjDel(&sclModOut->base);
}

static void _PTREE_MOD_SCL_STRETCH_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(CONTAINER_OF(modOut, PTREE_MOD_SCL_STRETCH_OutObj_t, base));
}

static PTREE_MOD_OutObj_t *_PTREE_MOD_SCL_STRETCH_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SCL_STRETCH_OutObj_t *sclModOut = NULL;

    sclModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SCL_STRETCH_OutObj_t));
    if (!sclModOut)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(sclModOut, 0, sizeof(PTREE_MOD_SCL_STRETCH_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&sclModOut->base, &G_PTREE_MOD_SCL_STRETCH_OUT_OPS, mod, loopId))
    {
        goto ERR_MOD_OUT_INIT;
    }

    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&sclModOut->outLinker, &G_PTREE_MOD_SCL_STRETCH_OUT_LINKER_OPS))
    {
        goto ERR_OUT_LINKER_INIT;
    }

    PTREE_MOD_OutObjRegister(&sclModOut->base, &G_PTREE_MOD_SCL_STRETCH_OUT_HOOK);
    PTREE_NLINKER_OUT_Register(&sclModOut->outLinker, &G_PTREE_MOD_SCL_STRETCH_OUT_LINKER_HOOK);

    return &sclModOut->base;

ERR_OUT_LINKER_INIT:
    PTREE_MOD_OutObjDel(&sclModOut->base);
ERR_MOD_OUT_INIT:
    SSOS_MEM_Free(sclModOut);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MOD_Obj_t *PTREE_MOD_SCL_STRETCH_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_SCL_STRETCH_Obj_t *sclMod = NULL;

    sclMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SCL_STRETCH_Obj_t));
    if (!sclMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(sclMod, 0, sizeof(PTREE_MOD_SCL_STRETCH_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&sclMod->base, &G_PTREE_MOD_SCL_STRETCH_SYS_OPS, tag, E_MI_MODULE_ID_SCL))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (sclMod->base.base.info->devId >= PTREE_MOD_SCL_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", sclMod->base.base.info->devId, PTREE_MOD_SCL_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    if (SSOS_DEF_OK != PTREE_MMA_PACKET_PackerInit(&sclMod->packer))
    {
        goto ERR_DEV_OUT_OF_RANGE;
    }

    PTREE_MOD_SYS_ObjRegister(&sclMod->base, &G_PTREE_MOD_SCL_STRETCH_SYS_HOOK);
    return &sclMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&sclMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(sclMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(SCL_STRETCH, PTREE_MOD_SCL_STRETCH_New);
