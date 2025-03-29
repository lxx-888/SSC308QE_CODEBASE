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
#include "mi_venc_datatype.h"
#include "mi_venc.h"
#include "cam_dev_wrapper.h"
#include "ssos_def.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ssos_task.h"
#include "ptree_packet.h"
#include "ptree_packet_raw.h"
#include "ptree_packet_video.h"
#include "ptree_mma_packet.h"
#include "ptree_linker.h"
#include "ptree_nlinker_out.h"
#include "ptree_mod.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_venc.h"
#include "ptree_sur_venc.h"
#include "ptree_maker.h"

#define PTREE_MOD_VENC_DEV_NUM              (16)
#define PTREE_MOD_VENC_READER_TASK_NAME_LEN (32)

typedef struct PTREE_MOD_VENC_VideoPacket_s PTREE_MOD_VENC_VideoPacket_t;
typedef struct PTREE_MOD_VENC_OutLinker_s   PTREE_MOD_VENC_OutLinker_t;
typedef struct PTREE_MOD_VENC_RawOutObj_s   PTREE_MOD_VENC_RawOutObj_t;
typedef struct PTREE_MOD_VENC_VideoOutObj_s PTREE_MOD_VENC_VideoOutObj_t;
typedef struct PTREE_MOD_VENC_Obj_s         PTREE_MOD_VENC_Obj_t;
typedef struct PTREE_MOD_VENC_DevInfo_s     PTREE_MOD_VENC_DevInfo_t;

struct PTREE_MOD_VENC_VideoPacket_s
{
    PTREE_MMA_PACKET_VideoPa_t base;
    unsigned int               devId;
    unsigned int               chnId;
    MI_VENC_Stream_t           stStream;
    MI_VENC_ChnStat_t          stStat;
    MI_VENC_Pack_t             astPack[PTREE_PACKET_VIDEO_SLICE_COUNT_MAX];
    PTREE_PACKET_Obj_t *       dupPacket;
};
struct PTREE_MOD_VENC_OutLinker_s
{
    PTREE_NLINKER_OUT_Obj_t base;
    unsigned int            devId;
    unsigned int            chnId;
};
struct PTREE_MOD_VENC_RawOutObj_s
{
    PTREE_MOD_SYS_OutObj_t base;
};
struct PTREE_MOD_VENC_VideoOutObj_s
{
    PTREE_MOD_OutObj_t base;
    void *             taskHandle;
};
struct PTREE_MOD_VENC_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};
struct PTREE_MOD_VENC_DevInfo_s
{
    unsigned int maxWidth;
    unsigned int maxHeight;
};

static PTREE_PACKET_Obj_t *_PTREE_MOD_VENC_OutPacketDup(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_VENC_OutPacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_VENC_OutPacketDestruct(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_VENC_OutPacketFree(PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_VENC_OutPacketNew(unsigned int devId, unsigned int chnId, int ms);

static int _PTREE_MOD_VENC_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_VENC_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms);
static int  _PTREE_MOD_VENC_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static void _PTREE_MOD_VENC_OutLinkerFree(PTREE_NLINKER_OUT_Obj_t *nlinkerOut);
static PTREE_LINKER_Obj_t *_PTREE_MOD_VENC_OutLinkerNew(unsigned int devId, unsigned int chnId);

static int  _PTREE_MOD_VENC_RawOutStart(PTREE_MOD_SYS_OutObj_t *sysModOut);
static int  _PTREE_MOD_VENC_RawOutStop(PTREE_MOD_SYS_OutObj_t *sysModOut);
static void _PTREE_MOD_VENC_RawOutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo);
static void _PTREE_MOD_VENC_RawOutFree(PTREE_MOD_SYS_OutObj_t *sysModOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_VENC_RawOutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static int                  _PTREE_MOD_VENC_VideoOutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_LINKER_Obj_t * _PTREE_MOD_VENC_VideoOutCreateNLinker(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_VENC_VideoOutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_VENC_VideoOutStart(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_VENC_VideoOutStop(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_VENC_VideoOutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_VENC_VideoOutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_VENC_VideoOutSuspend(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_VENC_VideoOutResume(PTREE_MOD_OutObj_t *modOut);
static void                 _PTREE_MOD_VENC_VideoOutDestruct(PTREE_MOD_OutObj_t *modOut);
static void                 _PTREE_MOD_VENC_VideoOutFree(PTREE_MOD_OutObj_t *modOut);
static PTREE_MOD_OutObj_t * _PTREE_MOD_VENC_VideoOutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static PTREE_MOD_SYS_InObj_t *_PTREE_MOD_VENC_ResetStreamTraverseSys(PTREE_MOD_SYS_OutObj_t *modOut);
static int _PTREE_MOD_VENC_ResetStreamOutSys(PTREE_MOD_SYS_OutObj_t *modOut, unsigned int width, unsigned int height);
static PTREE_MOD_InObj_t *_PTREE_MOD_VENC_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut);
static int _PTREE_MOD_VENC_ResetStreamOut(PTREE_MOD_OutObj_t *modOut, unsigned int width, unsigned int height);

static int                 _PTREE_MOD_VENC_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_VENC_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_OutObj_t *_PTREE_MOD_VENC_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_VENC_Free(PTREE_MOD_SYS_Obj_t *sysMod);

PTREE_PACKET_TYPE_DEFINE(venc);

static const PTREE_PACKET_ObjOps_t G_PTREE_VENC_PACKET_OPS = {
    .dup             = _PTREE_MOD_VENC_OutPacketDup,
    .convert         = NULL,
    .updateTimeStamp = _PTREE_MOD_VENC_OutPacketUpdateTimeStamp,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_VENC_PACKET_HOOK = {
    .destruct = _PTREE_MOD_VENC_OutPacketDestruct,
    .free     = _PTREE_MOD_VENC_OutPacketFree,
};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_VENC_OUT_LINKER_OPS = {
    .enqueue           = _PTREE_MOD_VENC_OutLinkerEnqueue,
    .dequeueOut        = _PTREE_MOD_VENC_OutLinkerDequeueOut,
    .dequeueFromInside = _PTREE_MOD_VENC_OutLinkerDequeueFromInside,
};
static const PTREE_NLINKER_OUT_Hook_t G_PTREE_VENC_OUT_LINKER_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_VENC_OutLinkerFree,
};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_VENC_OPS = {
    .init         = _PTREE_MOD_VENC_Init,
    .deinit       = _PTREE_MOD_VENC_Deinit,
    .createModOut = _PTREE_MOD_VENC_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_VENC_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_VENC_Free,
};

static const PTREE_MOD_SYS_OutOps_t G_PTREE_MOD_VENC_RAW_OUT_OPS = {
    .start               = _PTREE_MOD_VENC_RawOutStart,
    .stop                = _PTREE_MOD_VENC_RawOutStop,
    .getPacketInfo       = _PTREE_MOD_VENC_RawOutGetPacketInfo,
    .resetStreamOut      = _PTREE_MOD_VENC_ResetStreamOutSys,
    .resetStreamTraverse = _PTREE_MOD_VENC_ResetStreamTraverseSys,
};
static const PTREE_MOD_SYS_OutHook_t G_PTREE_MOD_VENC_RAW_OUT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_VENC_RawOutFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_VENC_VIDEO_OUT_OPS = {
    .getType             = _PTREE_MOD_VENC_VideoOutGetType,
    .createNLinker       = _PTREE_MOD_VENC_VideoOutCreateNLinker,
    .getPacketInfo       = _PTREE_MOD_VENC_VideoOutGetPacketInfo,
    .start               = _PTREE_MOD_VENC_VideoOutStart,
    .stop                = _PTREE_MOD_VENC_VideoOutStop,
    .linked              = _PTREE_MOD_VENC_VideoOutLinked,
    .unlinked            = _PTREE_MOD_VENC_VideoOutUnlinked,
    .suspend             = _PTREE_MOD_VENC_VideoOutSuspend,
    .resume              = _PTREE_MOD_VENC_VideoOutResume,
    .resetStreamOut      = _PTREE_MOD_VENC_ResetStreamOut,
    .resetStreamTraverse = _PTREE_MOD_VENC_ResetStreamTraverse,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_VENC_VIDEO_OUT_HOOK = {
    .destruct = _PTREE_MOD_VENC_VideoOutDestruct,
    .free     = _PTREE_MOD_VENC_VideoOutFree,
};

static PTREE_MOD_VENC_DevInfo_t g_vencDevInfo[PTREE_MOD_VENC_DEV_NUM] = {0};

static PTREE_PACKET_Obj_t *_PTREE_MOD_VENC_OutPacketDup(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_VENC_VideoPacket_t *vencPacket  = CONTAINER_OF(packet, PTREE_MOD_VENC_VideoPacket_t, base.base.base);
    PTREE_PACKET_VIDEO_Obj_t *    dupVideoObj = NULL;

    if (vencPacket->dupPacket)
    {
        /* Using reference count for packet dup. */
        return NULL;
    }
    vencPacket->dupPacket = PTREE_PACKET_VIDEO_NormalNewClone(&vencPacket->base.base);
    SSOS_ASSERT(vencPacket->dupPacket);
    dupVideoObj = CONTAINER_OF(vencPacket->dupPacket, PTREE_PACKET_VIDEO_Obj_t, base);
    /* Copy address. */
    vencPacket->base.base.packetData = dupVideoObj->packetData;
    /* Release current address. */
    MI_VENC_ReleaseStream(vencPacket->devId, vencPacket->chnId, &vencPacket->stStream);
    return NULL;
}
static void _PTREE_MOD_VENC_OutPacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_VENC_VideoPacket_t *vencPacket = CONTAINER_OF(packet, PTREE_MOD_VENC_VideoPacket_t, base.base.base);
    PTREE_PACKET_TimeStamp_t      stamp;

    stamp.tvSec  = vencPacket->stStream.pstPack[0].u64PTS / 1000000;
    stamp.tvUSec = vencPacket->stStream.pstPack[0].u64PTS % 1000000;
    PTREE_PACKET_SetTimeStamp(&vencPacket->base.base.base, &stamp);
}
static void _PTREE_MOD_VENC_OutPacketDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_VENC_VideoPacket_t *vencPacket = CONTAINER_OF(packet, PTREE_MOD_VENC_VideoPacket_t, base.base.base);

    if (!vencPacket->dupPacket)
    {
        /* Packet did not dup before. */
        MI_VENC_ReleaseStream(vencPacket->devId, vencPacket->chnId, &vencPacket->stStream);
        return;
    }
    PTREE_PACKET_Del(vencPacket->dupPacket);
    vencPacket->dupPacket = NULL;
}
static void _PTREE_MOD_VENC_OutPacketFree(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_VENC_VideoPacket_t *vencPacket = CONTAINER_OF(packet, PTREE_MOD_VENC_VideoPacket_t, base.base.base);
    SSOS_MEM_Free(vencPacket);
}
static int _PTREE_MOD_VENC_PollStream(PTREE_MOD_VENC_VideoPacket_t *vencPacket, int ms)
{
    struct pollfd     pollFd;
    int               readyNum = 0;
    int               ret;
    unsigned int      packetCount = 0;
    MI_VENC_ChnAttr_t stAttr;

    memset(&pollFd, 0, sizeof(struct pollfd));
    pollFd.fd = MI_VENC_GetFd(vencPacket->devId, vencPacket->chnId);
    if (pollFd.fd < 0)
    {
        return SSOS_DEF_FAIL;
    }

    pollFd.events = POLLIN;
    readyNum      = CamDevPoll(&pollFd, 1, ms);
    if (readyNum < 0)
    {
        MI_VENC_CloseFd(vencPacket->devId, vencPacket->chnId);
        return SSOS_DEF_FAIL;
    }
    if (!(pollFd.revents & POLLIN))
    {
        MI_VENC_CloseFd(vencPacket->devId, vencPacket->chnId);
        return SSOS_DEF_FAIL;
    }

    memset(&vencPacket->stStat, 0, sizeof(MI_VENC_ChnStat_t));
    ret = MI_VENC_Query(vencPacket->devId, vencPacket->chnId, &vencPacket->stStat);
    if (ret != MI_SUCCESS || vencPacket->stStat.u32CurPacks == 0)
    {
        PTREE_ERR("MI_VENC_Query failed, ret = %d, CurPacks = %d", ret, vencPacket->stStat.u32CurPacks);
        return SSOS_DEF_FAIL;
    }
    memset(&vencPacket->stStream, 0, sizeof(MI_VENC_Stream_t));
    memset(vencPacket->astPack, 0, sizeof(MI_VENC_Pack_t) * PTREE_PACKET_VIDEO_SLICE_COUNT_MAX);
    memset(&stAttr, 0, sizeof(MI_VENC_ChnAttr_t));

    if (MI_SUCCESS != MI_VENC_GetChnAttr(vencPacket->devId, vencPacket->chnId, &stAttr))
    {
        PTREE_ERR("MI_VENC_GetChnAttr Failed");
        return SSOS_DEF_FAIL;
    }

    vencPacket->stStream.pstPack      = vencPacket->astPack;
    vencPacket->stStream.u32PackCount = vencPacket->stStat.u32CurPacks;

    if (MI_SUCCESS != MI_VENC_GetStream(vencPacket->devId, vencPacket->chnId, &vencPacket->stStream, 300))
    {
        PTREE_ERR("MI_VENC_GetStream Failed");
        return SSOS_DEF_FAIL;
    }

    if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        vencPacket->base.base.info.packetInfo.fmt    = E_PTREE_PACKET_VIDEO_STREAM_H264;
        vencPacket->base.base.info.packetInfo.bHead  = false;
        vencPacket->base.base.info.packetInfo.width  = stAttr.stVeAttr.stAttrH264e.u32PicWidth;
        vencPacket->base.base.info.packetInfo.height = stAttr.stVeAttr.stAttrH264e.u32PicHeight;
    }
    else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        vencPacket->base.base.info.packetInfo.fmt    = E_PTREE_PACKET_VIDEO_STREAM_H265;
        vencPacket->base.base.info.packetInfo.bHead  = false;
        vencPacket->base.base.info.packetInfo.width  = stAttr.stVeAttr.stAttrH265e.u32PicWidth;
        vencPacket->base.base.info.packetInfo.height = stAttr.stVeAttr.stAttrH265e.u32PicHeight;
    }
    else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        vencPacket->base.base.info.packetInfo.fmt    = E_PTREE_PACKET_VIDEO_STREAM_JPEG;
        vencPacket->base.base.info.packetInfo.bHead  = false;
        vencPacket->base.base.info.packetInfo.width  = stAttr.stVeAttr.stAttrJpeg.u32PicWidth;
        vencPacket->base.base.info.packetInfo.height = stAttr.stVeAttr.stAttrJpeg.u32PicHeight;
    }
    else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_AV1)
    {
        vencPacket->base.base.info.packetInfo.fmt    = E_PTREE_PACKET_VIDEO_STREAM_AV1;
        vencPacket->base.base.info.packetInfo.bHead  = false;
        vencPacket->base.base.info.packetInfo.width  = stAttr.stVeAttr.stAttrAv1.u32PicWidth;
        vencPacket->base.base.info.packetInfo.height = stAttr.stVeAttr.stAttrAv1.u32PicHeight;
    }
    else
    {
        PTREE_ERR("Bad en_type %d", stAttr.stVeAttr.eType);
        MI_VENC_ReleaseStream(vencPacket->devId, vencPacket->chnId, &vencPacket->stStream);
        return SSOS_DEF_FAIL;
    }

    vencPacket->base.base.info.packetInfo.pktCount = vencPacket->stStream.u32PackCount;
    for (packetCount = 0; packetCount < vencPacket->stStream.u32PackCount; ++packetCount)
    {
        vencPacket->base.pa[packetCount] = vencPacket->stStream.pstPack[packetCount].phyAddr;
        vencPacket->base.base.packetData.pktData[packetCount].data =
            (char *)vencPacket->stStream.pstPack[packetCount].pu8Addr;
        vencPacket->base.base.info.packetInfo.pktInfo[packetCount].size =
            vencPacket->stStream.pstPack[packetCount].u32Len;
        vencPacket->base.base.info.packetInfo.pktInfo[packetCount].bEnd =
            vencPacket->stStream.pstPack[packetCount].bFrameEnd;
    }
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_VENC_OutPacketNew(unsigned int devId, unsigned int chnId, int ms)
{
    PTREE_MOD_VENC_VideoPacket_t *vencPacket = NULL;

    vencPacket = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VENC_VideoPacket_t));
    if (!vencPacket)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(vencPacket, 0, sizeof(PTREE_MOD_VENC_VideoPacket_t));

    if (SSOS_DEF_OK
        != PTREE_PACKET_VIDEO_Init(&vencPacket->base.base, &G_PTREE_VENC_PACKET_OPS, PTREE_PACKET_TYPE(venc)))
    {
        PTREE_ERR("PTREE_PACKET_VIDEO_Init Failed");
        goto ERR_VIDEO_INIT;
    }

    vencPacket->devId     = devId;
    vencPacket->chnId     = chnId;
    vencPacket->dupPacket = NULL;

    if (SSOS_DEF_OK != _PTREE_MOD_VENC_PollStream(vencPacket, ms))
    {
        goto ERR_POLL_STREAM;
    }

    PTREE_PACKET_Register(&vencPacket->base.base.base, &G_PTREE_VENC_PACKET_HOOK);
    return &vencPacket->base.base.base;

ERR_POLL_STREAM:
    PTREE_PACKET_Del(&vencPacket->base.base.base);
ERR_VIDEO_INIT:
    SSOS_MEM_Free(vencPacket);
ERR_MEM_ALLOC:
    return NULL;
}

static int _PTREE_MOD_VENC_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    PTREE_PACKET_Obj_t *        packetSrc      = NULL;
    PTREE_PACKET_VIDEO_Obj_t *  packetVideoSrc = NULL;
    PTREE_PACKET_VIDEO_Obj_t *  packetVideoDst = NULL;
    PTREE_MOD_VENC_OutLinker_t *vencLinker     = NULL;
    PTREE_PACKET_Obj_t *        packetConvert  = NULL;
    int                         ret            = SSOS_DEF_FAIL;

    packetConvert = PTREE_PACKET_Convert(packet, PTREE_PACKET_INFO_TYPE(video));
    if (!packetConvert)
    {
        PTREE_ERR("Enqueue packet type is not video.");
        return SSOS_DEF_FAIL;
    }
    vencLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_VENC_OutLinker_t, base);
    packetSrc  = _PTREE_MOD_VENC_OutPacketNew(vencLinker->devId, vencLinker->chnId, 300);
    if (!packetSrc)
    {
        goto PACKET_ERR0;
    }
    if (!PTREE_PACKET_InfoEqual(packet->info, packetSrc->info))
    {
        PTREE_ERR("Enqueue packet info is not matched.");
        goto PACKET_ERR1;
    }
    packetVideoSrc = CONTAINER_OF(packetSrc, PTREE_PACKET_VIDEO_Obj_t, base);
    packetVideoDst = CONTAINER_OF(packet, PTREE_PACKET_VIDEO_Obj_t, base);
    PTREE_PACKET_VIDEO_Copy(packetVideoDst, packetVideoSrc);
    PTREE_PACKET_SetTimeStamp(packet, PTREE_PACKET_GetTimeStamp(&packetVideoSrc->base));
    ret = SSOS_DEF_OK;
PACKET_ERR1:
    PTREE_PACKET_Del(packetSrc);
PACKET_ERR0:
    PTREE_PACKET_Del(packetConvert);
    return ret;
}

static int _PTREE_MOD_VENC_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_VENC_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms)
{
    PTREE_MOD_VENC_OutLinker_t *vencLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_VENC_OutLinker_t, base);
    return _PTREE_MOD_VENC_OutPacketNew(vencLinker->devId, vencLinker->chnId, ms);
}
static void _PTREE_MOD_VENC_OutLinkerFree(PTREE_NLINKER_OUT_Obj_t *nlinkerOut)
{
    PTREE_MOD_VENC_OutLinker_t *vencLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_VENC_OutLinker_t, base);
    SSOS_MEM_Free(vencLinker);
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_VENC_OutLinkerNew(unsigned int devId, unsigned int chnId)
{
    PTREE_MOD_VENC_OutLinker_t *vencLinker = NULL;

    vencLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VENC_OutLinker_t));
    if (!vencLinker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(vencLinker, 0, sizeof(PTREE_MOD_VENC_OutLinker_t));

    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&vencLinker->base, &G_PTREE_VENC_OUT_LINKER_OPS))
    {
        SSOS_MEM_Free(vencLinker);
        return NULL;
    }

    vencLinker->devId = devId;
    vencLinker->chnId = chnId;
    PTREE_NLINKER_OUT_Register(&vencLinker->base, &G_PTREE_VENC_OUT_LINKER_HOOK);
    return &vencLinker->base.base;
}
static int _PTREE_MOD_VENC_RawOutStart(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(sysModOut->base.thisMod->info, PTREE_SUR_VENC_Info_t, base.base);
    if (vencInfo->bYuvEnable)
    {
        MI_VENC_OutPortParam_t outPortParam;
        memset(&outPortParam, 0, sizeof(MI_VENC_OutPortParam_t));
        outPortParam.u32Width  = vencInfo->u32YuvWidth;
        outPortParam.u32Height = vencInfo->u32YuvHeight;
        MI_VENC_SetOutputPortParam(sysModOut->thisSysMod->base.info->devId, sysModOut->thisSysMod->base.info->chnId, 0,
                                   &outPortParam);
        MI_VENC_EnableOutputPort(sysModOut->thisSysMod->base.info->devId, sysModOut->thisSysMod->base.info->chnId, 0);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VENC_RawOutStop(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(sysModOut->base.thisMod->info, PTREE_SUR_VENC_Info_t, base.base);
    if (vencInfo->bYuvEnable)
    {
        MI_VENC_DisableOutputPort(sysModOut->thisSysMod->base.info->devId, sysModOut->thisSysMod->base.info->chnId, 0);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_VENC_RawOutGetPacketInfo(PTREE_MOD_SYS_OutObj_t *sysModOut, PTREE_PACKET_RAW_RawInfo_t *rawInfo)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(sysModOut->base.thisMod->info, PTREE_SUR_VENC_Info_t, base.base);
    rawInfo->fmt                    = E_PTREE_PACKET_RAW_FORMAT_YUV420SP;
    if (vencInfo->bYuvEnable)
    {
        rawInfo->width  = vencInfo->u32YuvWidth;
        rawInfo->height = vencInfo->u32YuvHeight;
    }
}
static void _PTREE_MOD_VENC_RawOutFree(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    SSOS_MEM_Free(CONTAINER_OF(sysModOut, PTREE_MOD_VENC_RawOutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_VENC_RawOutNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_VENC_RawOutObj_t *vencRawOut = NULL;

    vencRawOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VENC_RawOutObj_t));
    if (!vencRawOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(vencRawOut, 0, sizeof(PTREE_MOD_VENC_RawOutObj_t));

    if (SSOS_DEF_OK != PTREE_MOD_SYS_OutObjInit(&vencRawOut->base, &G_PTREE_MOD_VENC_RAW_OUT_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(vencRawOut);
        return NULL;
    }
    PTREE_MOD_SYS_OutObjRegister(&vencRawOut->base, &G_PTREE_MOD_VENC_RAW_OUT_HOOK);
    return &vencRawOut->base.base;
}

static void *_PTREE_MOD_VENC_Reader(SSOS_TASK_Buffer_t *taskBuf)
{
    PTREE_MOD_OutObj_t *modOut = (PTREE_MOD_OutObj_t *)taskBuf->buf;
    PTREE_PACKET_Obj_t *packet = NULL;
    packet = _PTREE_MOD_VENC_OutPacketNew(modOut->thisMod->info->devId, modOut->thisMod->info->chnId, 300);
    if (packet != NULL)
    {
        PTREE_LINKER_Enqueue(&modOut->plinker.base, packet);
        PTREE_PACKET_Del(packet);
    }

    return NULL;
}
static int _PTREE_MOD_VENC_VideoOutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_VENC_VideoOutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    return _PTREE_MOD_VENC_OutLinkerNew(modOut->thisMod->info->devId, modOut->thisMod->info->chnId);
}
static PTREE_PACKET_Info_t *_PTREE_MOD_VENC_VideoOutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_SUR_VENC_Info_t *         pstVencInfo = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_VENC_Info_t, base.base);
    PTREE_PACKET_VIDEO_PacketInfo_t streamInfo;
    memset(&streamInfo, 0, sizeof(PTREE_PACKET_Info_t));
    streamInfo.fmt    = pstVencInfo->pEncodeType;
    streamInfo.width  = pstVencInfo->u32Width;
    streamInfo.height = pstVencInfo->u32Height;
    return PTREE_PACKET_VIDEO_InfoNew(&streamInfo);
}
static int _PTREE_MOD_VENC_VideoOutStart(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_VENC_Info_t, base.base);
    if (MI_SUCCESS
        != MI_VENC_StartRecvPic((MI_VENC_DEV)vencInfo->base.base.devId, (MI_VENC_CHN)vencInfo->base.base.chnId))
    {
        PTREE_ERR("MI_VENC_StartRecvPic failed");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VENC_VideoOutStop(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_VENC_Info_t, base.base);
    MI_VENC_StopRecvPic((MI_VENC_DEV)vencInfo->base.base.devId, (MI_VENC_CHN)vencInfo->base.base.chnId);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VENC_VideoOutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_VENC_VideoOutObj_t *vencVideoOut = CONTAINER_OF(modOut, PTREE_MOD_VENC_VideoOutObj_t, base);
    PTREE_SUR_VENC_Info_t *       vencInfo     = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_VENC_Info_t, base.base);
    SSOS_TASK_Attr_t              taskAttr;
    char                          taskName[PTREE_MOD_VENC_READER_TASK_NAME_LEN];

    if (ref > 0)
    {
        if (!vencVideoOut->taskHandle)
        {
            return SSOS_DEF_OK;
        }
        if (vencInfo->pEncodeType != E_PTREE_PACKET_VIDEO_STREAM_JPEG)
        {
            MI_VENC_RequestIdr(vencInfo->base.base.devId, vencInfo->base.base.chnId, TRUE);
        }
        return SSOS_DEF_OK;
    }

    if (vencInfo->pEncodeType != E_PTREE_PACKET_VIDEO_STREAM_JPEG)
    {
        MI_VENC_RequestIdr(vencInfo->base.base.devId, vencInfo->base.base.chnId, TRUE);
    }

    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }

    PTREE_MOD_OutKeyStr(modOut, taskName, PTREE_MOD_VENC_READER_TASK_NAME_LEN);
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    taskAttr.doSignal        = NULL;
    taskAttr.doMonitor       = _PTREE_MOD_VENC_Reader;
    taskAttr.inBuf.buf       = (void *)modOut;
    taskAttr.threadAttr.name = taskName;
    vencVideoOut->taskHandle = SSOS_TASK_Open(&taskAttr);
    if (!vencVideoOut->taskHandle)
    {
        PTREE_ERR("Task create error!");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(vencVideoOut->taskHandle);
    PTREE_DBG("Venc reader %d start", modOut->info->port);
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_VENC_VideoOutSuspend(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_VENC_VideoOutObj_t *vencVideoOut = CONTAINER_OF(modOut, PTREE_MOD_VENC_VideoOutObj_t, base);
    if (!vencVideoOut->taskHandle)
    {
        PTREE_ERR("Task null!");
        return SSOS_DEF_FAIL;
    }
    PTREE_DBG("Venc reader %d suspend", vencVideoOut->base.info->port);
    return SSOS_TASK_Stop(vencVideoOut->taskHandle);
}

static int _PTREE_MOD_VENC_VideoOutResume(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_VENC_VideoOutObj_t *vencVideoOut = CONTAINER_OF(modOut, PTREE_MOD_VENC_VideoOutObj_t, base);
    if (!vencVideoOut->taskHandle)
    {
        PTREE_ERR("Task null!");
        return SSOS_DEF_FAIL;
    }
    PTREE_DBG("Venc reader %d resume", vencVideoOut->base.info->port);
    return SSOS_TASK_StartMonitor(vencVideoOut->taskHandle);
}

static int _PTREE_MOD_VENC_VideoOutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_VENC_VideoOutObj_t *vencVideoOut = CONTAINER_OF(modOut, PTREE_MOD_VENC_VideoOutObj_t, base);
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (!vencVideoOut->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    SSOS_TASK_Close(vencVideoOut->taskHandle);
    vencVideoOut->taskHandle = NULL;
    PTREE_DBG("Venc reader %d stop", modOut->info->port);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_VENC_VideoOutDestruct(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_VENC_VideoOutObj_t *vencVideoOut = CONTAINER_OF(modOut, PTREE_MOD_VENC_VideoOutObj_t, base);
    vencVideoOut->taskHandle                   = NULL;
}
static void _PTREE_MOD_VENC_VideoOutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(CONTAINER_OF(modOut, PTREE_MOD_VENC_VideoOutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_VENC_VideoOutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_VENC_VideoOutObj_t *vencVideoOut = NULL;

    vencVideoOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VENC_VideoOutObj_t));
    if (!vencVideoOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(vencVideoOut, 0, sizeof(PTREE_MOD_VENC_VideoOutObj_t));

    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&vencVideoOut->base, &G_PTREE_MOD_VENC_VIDEO_OUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(vencVideoOut);
        return NULL;
    }

    vencVideoOut->taskHandle = NULL;

    PTREE_MOD_OutObjRegister(&vencVideoOut->base, &G_PTREE_MOD_VENC_VIDEO_OUT_HOOK);
    return &vencVideoOut->base;
}

static int _PTREE_MOD_VENC_CreateChannel(PTREE_SUR_VENC_Info_t *vencInfo)
{
    MI_VENC_ChnAttr_t vencChnAttr;
    memset(&vencChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

    switch (vencInfo->pEncodeType)
    {
        case E_PTREE_PACKET_VIDEO_STREAM_H264:
        {
            vencChnAttr.stVeAttr.eType                       = E_MI_VENC_MODTYPE_H264E;
            vencChnAttr.stVeAttr.stAttrH264e.u32PicWidth     = (MI_U32)vencInfo->u32Width;
            vencChnAttr.stVeAttr.stAttrH264e.u32PicHeight    = (MI_U32)vencInfo->u32Height;
            vencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth  = (MI_U32)vencInfo->u32MaxWidth;
            vencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = (MI_U32)vencInfo->u32MaxHeight;
            vencChnAttr.stVeAttr.stAttrH264e.bByFrame =
                (vencInfo->u32MultiSlice != -1) ? !vencInfo->u32MultiSlice : TRUE;
            vencChnAttr.stVeAttr.stAttrH264e.u32Profile = 2;
            if (!strcmp(vencInfo->pRcMode, "cbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                  = E_MI_VENC_RC_MODE_H264CBR;
                vencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = (MI_U32)vencInfo->stCbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop           = vencInfo->stCbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime      = 0;
            }
            else if (!strcmp(vencInfo->pRcMode, "vbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                     = E_MI_VENC_RC_MODE_H264VBR;
                vencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = (MI_U32)vencInfo->stVbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp      = vencInfo->stVbrCfg.u32MaxQp;
                vencChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp      = vencInfo->stVbrCfg.u32MinQp;
                vencChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH264Vbr.u32Gop           = vencInfo->stVbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrH264Vbr.u32StatTime      = 0;
            }
            else if (!strcmp(vencInfo->pRcMode, "fixqp"))
            {
                vencChnAttr.stRcAttr.eRcMode                = E_MI_VENC_RC_MODE_H264FIXQP;
                vencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = vencInfo->stFixQpCfg.u32IQp;
                vencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = vencInfo->stFixQpCfg.u32PQp;
                vencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop           = vencInfo->stFixQpCfg.u32Gop;
            }
            else if (!strcmp(vencInfo->pRcMode, "avbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                      = E_MI_VENC_RC_MODE_H264AVBR;
                vencChnAttr.stRcAttr.stAttrH264Avbr.u32MaxBitRate = (MI_U32)vencInfo->stAvbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrH264Avbr.u32MaxQp      = vencInfo->stAvbrCfg.u32MaxQp;
                vencChnAttr.stRcAttr.stAttrH264Avbr.u32MinQp      = vencInfo->stAvbrCfg.u32MinQp;
                vencChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH264Avbr.u32Gop           = vencInfo->stAvbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrH264Avbr.u32StatTime      = 0;
            }
            else
            {
                PTREE_ERR("RC Mode error!");
                return SSOS_DEF_FAIL;
            }
        }
        break;
        case E_PTREE_PACKET_VIDEO_STREAM_H265:
        {
            vencChnAttr.stVeAttr.eType                       = E_MI_VENC_MODTYPE_H265E;
            vencChnAttr.stVeAttr.stAttrH265e.u32PicWidth     = (MI_U32)vencInfo->u32Width;
            vencChnAttr.stVeAttr.stAttrH265e.u32PicHeight    = (MI_U32)vencInfo->u32Height;
            vencChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth  = (MI_U32)vencInfo->u32MaxWidth;
            vencChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = (MI_U32)vencInfo->u32MaxHeight;
            vencChnAttr.stVeAttr.stAttrH265e.bByFrame =
                (vencInfo->u32MultiSlice != -1) ? !vencInfo->u32MultiSlice : TRUE;
            if (!strcmp(vencInfo->pRcMode, "cbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                  = E_MI_VENC_RC_MODE_H265CBR;
                vencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = (MI_U32)vencInfo->stCbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH265Cbr.u32Gop           = vencInfo->stCbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime      = 0;
            }
            else if (!strcmp(vencInfo->pRcMode, "vbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                     = E_MI_VENC_RC_MODE_H265VBR;
                vencChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate = (MI_U32)vencInfo->stVbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp      = vencInfo->stVbrCfg.u32MaxQp;
                vencChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp      = vencInfo->stVbrCfg.u32MinQp;
                vencChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH265Vbr.u32Gop           = vencInfo->stVbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrH265Vbr.u32StatTime      = 0;
            }
            else if (!strcmp(vencInfo->pRcMode, "fixqp"))
            {
                vencChnAttr.stRcAttr.eRcMode                = E_MI_VENC_RC_MODE_H265FIXQP;
                vencChnAttr.stRcAttr.stAttrH265FixQp.u32IQp = vencInfo->stFixQpCfg.u32IQp;
                vencChnAttr.stRcAttr.stAttrH265FixQp.u32PQp = vencInfo->stFixQpCfg.u32PQp;
                vencChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH265FixQp.u32Gop           = vencInfo->stFixQpCfg.u32Gop;
            }
            else if (!strcmp(vencInfo->pRcMode, "avbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                      = E_MI_VENC_RC_MODE_H265AVBR;
                vencChnAttr.stRcAttr.stAttrH265Avbr.u32MaxBitRate = (MI_U32)vencInfo->stAvbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrH265Avbr.u32MaxQp      = vencInfo->stAvbrCfg.u32MaxQp;
                vencChnAttr.stRcAttr.stAttrH265Avbr.u32MinQp      = vencInfo->stAvbrCfg.u32MinQp;
                vencChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrH265Avbr.u32Gop           = vencInfo->stAvbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrH265Avbr.u32StatTime      = 0;
            }
            else
            {
                PTREE_ERR("RC Mode error!");
                return SSOS_DEF_FAIL;
            }
        }
        break;
        case E_PTREE_PACKET_VIDEO_STREAM_JPEG:
        {
            vencChnAttr.stVeAttr.eType                      = E_MI_VENC_MODTYPE_JPEGE;
            vencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth     = (MI_U32)vencInfo->u32Width;
            vencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight    = (MI_U32)vencInfo->u32Height;
            vencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth  = (MI_U32)vencInfo->u32MaxWidth;
            vencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = (MI_U32)vencInfo->u32MaxHeight;
            if (!strcmp(vencInfo->pRcMode, "cbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                   = E_MI_VENC_RC_MODE_MJPEGCBR;
                vencChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate = (MI_U32)vencInfo->stCbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateDen = 1;
            }
            else if (!strcmp(vencInfo->pRcMode, "vbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                      = E_MI_VENC_RC_MODE_MJPEGVBR;
                vencChnAttr.stRcAttr.stAttrMjpegVbr.u32MaxBitRate = (MI_U32)vencInfo->stVbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateDen = 1;
            }
            else if (!strcmp(vencInfo->pRcMode, "fixqp"))
            {
                vencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
                vencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor       = vencInfo->stFixQpCfg.u32Qfactor;
            }
            else
            {
                PTREE_ERR("RC Mode error!");
                return SSOS_DEF_FAIL;
            }
        }
        break;
        case E_PTREE_PACKET_VIDEO_STREAM_AV1:
        {
            vencChnAttr.stVeAttr.eType                  = E_MI_VENC_MODTYPE_AV1;
            vencChnAttr.stVeAttr.stAttrAv1.u32PicWidth  = (MI_U32)vencInfo->u32Width;
            vencChnAttr.stVeAttr.stAttrAv1.u32PicHeight = (MI_U32)vencInfo->u32Height;
            vencChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth =
                vencInfo->u32MaxWidth > 4096 ? 4096 : vencInfo->u32MaxWidth;
            vencChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight =
                vencInfo->u32MaxHeight > 4096 ? 4096 : vencInfo->u32MaxHeight;
            vencChnAttr.stVeAttr.stAttrAv1.bByFrame = TRUE;
            if (!strcmp(vencInfo->pRcMode, "cbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                 = E_MI_VENC_RC_MODE_AV1CBR;
                vencChnAttr.stRcAttr.stAttrAv1Cbr.u32BitRate = (MI_U32)vencInfo->stCbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrAv1Cbr.u32Gop           = vencInfo->stCbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrAv1Cbr.u32StatTime      = 0;
            }
            else if (!strcmp(vencInfo->pRcMode, "vbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                    = E_MI_VENC_RC_MODE_AV1VBR;
                vencChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxBitRate = (MI_U32)vencInfo->stVbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxQp      = vencInfo->stVbrCfg.u32MaxQp;
                vencChnAttr.stRcAttr.stAttrAv1Vbr.u32MinQp      = vencInfo->stVbrCfg.u32MinQp;
                vencChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrAv1Vbr.u32Gop           = vencInfo->stVbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrAv1Vbr.u32StatTime      = 0;
            }
            else if (!strcmp(vencInfo->pRcMode, "fixqp"))
            {
                vencChnAttr.stRcAttr.eRcMode               = E_MI_VENC_RC_MODE_AV1FIXQP;
                vencChnAttr.stRcAttr.stAttrAv1FixQp.u32IQp = vencInfo->stFixQpCfg.u32IQp;
                vencChnAttr.stRcAttr.stAttrAv1FixQp.u32PQp = vencInfo->stFixQpCfg.u32PQp;
                vencChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrAv1FixQp.u32Gop           = vencInfo->stFixQpCfg.u32Gop;
            }
            else if (!strcmp(vencInfo->pRcMode, "avbr"))
            {
                vencChnAttr.stRcAttr.eRcMode                     = E_MI_VENC_RC_MODE_AV1AVBR;
                vencChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxBitRate = (MI_U32)vencInfo->stAvbrCfg.u32BitRate * 1000;
                vencChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxQp      = vencInfo->stAvbrCfg.u32MaxQp;
                vencChnAttr.stRcAttr.stAttrAv1Avbr.u32MinQp      = vencInfo->stAvbrCfg.u32MinQp;
                vencChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateNum =
                    (MI_U32)((vencInfo->u32EncodeFps != -1) ? vencInfo->u32EncodeFps : 30);
                vencChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateDen = 1;
                vencChnAttr.stRcAttr.stAttrAv1Avbr.u32Gop           = vencInfo->stAvbrCfg.u32Gop;
                vencChnAttr.stRcAttr.stAttrAv1Avbr.u32StatTime      = 0;
            }
            else
            {
                PTREE_ERR("RC Mode error!");
                return SSOS_DEF_FAIL;
            }
        }
        break;

        default:
            PTREE_ERR("not support encode type %s", vencInfo->pEncodeType);
            return SSOS_DEF_FAIL;
    }

    if (MI_SUCCESS
        != MI_VENC_CreateChn((MI_VENC_DEV)vencInfo->base.base.devId, (MI_VENC_CHN)vencInfo->base.base.chnId,
                             &vencChnAttr))
    {
        PTREE_ERR("MI_VENC_CreateChannel error");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VENC_SetInputSourceConfig(PTREE_MOD_SYS_Obj_t *sysMod, PTREE_SUR_VENC_Info_t *vencInfo)
{
    MI_VENC_InputSourceConfig_t stVenInSrc;
    MI_SYS_BindType_e           eBindType    = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    MI_U32                      u32BindParam = 0;
    PTREE_SUR_SYS_InInfo_t *    sysInInfo    = NULL;
    MI_S32                      s32Ret;

    if (sysMod->base.info->inCnt != 1)
    {
        PTREE_ERR("Not found input object!");
        return SSOS_DEF_FAIL;
    }
    sysInInfo = CONTAINER_OF(sysMod->base.arrModIn[0]->info, PTREE_SUR_SYS_InInfo_t, base);

    memset(&stVenInSrc, 0, sizeof(MI_VENC_InputSourceConfig_t));
    eBindType = (MI_SYS_BindType_e)sysInInfo->bindType;
    if (eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
    {
        u32BindParam = sysInInfo->bindParam;
        if (u32BindParam == (MI_U32)vencInfo->u32Width)
        {
            stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_ONE_FRM;
            s32Ret                         = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)vencInfo->base.base.devId,
                                                                          (MI_VENC_CHN)vencInfo->base.base.chnId, &stVenInSrc);
            PTREE_DBG("Set ring one frame mode! Chn %d height %d s32Ret %d", vencInfo->base.base.chnId,
                      vencInfo->u32Height, s32Ret);
        }
        else if (u32BindParam == (MI_U32)vencInfo->u32Height / 2)
        {
            stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_HALF_FRM;
            s32Ret                         = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)vencInfo->base.base.devId,
                                                                          (MI_VENC_CHN)vencInfo->base.base.chnId, &stVenInSrc);
            PTREE_DBG("Set ring half frame mode! Chn %d height %d s32Ret %d", vencInfo->base.base.chnId,
                      vencInfo->u32Height, s32Ret);
        }
        else
        {
            if (u32BindParam == 1)
            {
                stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA;
                s32Ret                         = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)vencInfo->base.base.devId,
                                                                              (MI_VENC_CHN)vencInfo->base.base.chnId, &stVenInSrc);
                PTREE_DBG("Set dma ring mode! Chn %d height %d s32Ret %d", vencInfo->base.base.chnId,
                          vencInfo->u32Height, s32Ret);
            }
        }
    }
    else if (eBindType == E_MI_SYS_BIND_TYPE_FRAME_BASE)
    {
        stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_NORMAL_FRMBASE;
        s32Ret                         = MI_VENC_SetInputSourceConfig((MI_VENC_DEV)vencInfo->base.base.devId,
                                                                      (MI_VENC_CHN)vencInfo->base.base.chnId, &stVenInSrc);
        PTREE_DBG("Set frame mode! s32Ret %d", s32Ret);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VENC_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_VENC_Info_t, base.base);
    int                    ret;

    MI_VENC_InitParam_t vencInitParam;
    memset(&vencInitParam, 0, sizeof(MI_VENC_InitParam_t));
    vencInitParam.u32MaxWidth  = g_vencDevInfo[vencInfo->base.base.devId].maxWidth;
    vencInitParam.u32MaxHeight = g_vencDevInfo[vencInfo->base.base.devId].maxHeight;
    if (MI_SUCCESS != MI_VENC_CreateDev(vencInfo->base.base.devId, &vencInitParam))
    {
        PTREE_ERR("MI_VENC_CreateDevice failed");
        return SSOS_DEF_FAIL;
    }

    ret = _PTREE_MOD_VENC_CreateChannel(vencInfo);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }

    if (vencInfo->u8StreamCntEnable)
    {
        MI_VENC_SetMaxStreamCnt((MI_VENC_DEV)vencInfo->base.base.devId, (MI_VENC_CHN)vencInfo->base.base.chnId,
                                vencInfo->u32MaxStreamCnt);
    }

    ret = _PTREE_MOD_VENC_SetInputSourceConfig(sysMod, vencInfo);
    if (ret != SSOS_DEF_OK)
    {
        return ret;
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_VENC_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_VENC_Info_t, base.base);

    MI_VENC_DestroyChn((MI_VENC_DEV)vencInfo->base.base.devId, (MI_VENC_CHN)vencInfo->base.base.chnId);

    MI_VENC_DestroyDev(vencInfo->base.base.devId);

    return SSOS_DEF_OK;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_VENC_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    if (loopId == 0 && mod->info->outCnt > 1)
    {
        return _PTREE_MOD_VENC_RawOutNew(sysMod, loopId);
    }
    return _PTREE_MOD_VENC_VideoOutNew(mod, loopId);
}
static void _PTREE_MOD_VENC_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_VENC_Obj_t, base));
}

static PTREE_MOD_SYS_InObj_t *_PTREE_MOD_VENC_ResetStreamTraverseSys(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    (void)sysModOut;
    /* If current venc output is using mi sys bind, then do not traverse reset stream signal to previous. */
    return NULL;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_VENC_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut)
{
    if (!modOut->thisMod->info->inCnt)
    {
        return NULL;
    }
    /* Because it was handled by '_PTREE_MOD_VENC_ResetStreamTraverseSys',
     * PTREE do not need to judge bindType here.
     */
    return modOut->thisMod->arrModIn[0];
}

static int _PTREE_MOD_VENC_ResetStreamOutSys(PTREE_MOD_SYS_OutObj_t *sysModOut, unsigned int width, unsigned int height)
{
    return _PTREE_MOD_VENC_ResetStreamOut(&sysModOut->base, width, height);
}

static int _PTREE_MOD_VENC_ResetStreamOut(PTREE_MOD_OutObj_t *modOut, unsigned int width, unsigned int height)
{
    PTREE_SUR_VENC_Info_t *vencInfo = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_VENC_Info_t, base.base);

    if (modOut->thisMod->status != E_PTREE_MOD_STATUS_CONSTRUCT)
    {
        MI_VENC_ChnAttr_t stAttr;
        memset(&stAttr, 0, sizeof(MI_VENC_ChnAttr_t));
        if (MI_SUCCESS != MI_VENC_GetChnAttr(modOut->thisMod->info->devId, modOut->thisMod->info->chnId, &stAttr))
        {
            PTREE_ERR("MI_VENC_GetChnAttr Failed.");
            return SSOS_DEF_FAIL;
        }
        if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
        {
            vencInfo->u32Width = stAttr.stVeAttr.stAttrH264e.u32PicWidth = width;
            vencInfo->u32Height = stAttr.stVeAttr.stAttrH264e.u32PicHeight = height;
        }
        else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
        {
            vencInfo->u32Width = stAttr.stVeAttr.stAttrH265e.u32PicWidth = width;
            vencInfo->u32Height = stAttr.stVeAttr.stAttrH265e.u32PicHeight = height;
        }
        else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
        {
            vencInfo->u32Width = stAttr.stVeAttr.stAttrJpeg.u32PicWidth = width;
            vencInfo->u32Height = stAttr.stVeAttr.stAttrJpeg.u32PicHeight = height;
        }
        else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_AV1)
        {
            vencInfo->u32Width = stAttr.stVeAttr.stAttrAv1.u32PicWidth = width;
            vencInfo->u32Height = stAttr.stVeAttr.stAttrAv1.u32PicHeight = height;
        }
        else
        {
            PTREE_ERR("Enc type %d is not support.", stAttr.stVeAttr.eType);
            return SSOS_DEF_FAIL;
        }
        if (MI_SUCCESS != MI_VENC_SetChnAttr(modOut->thisMod->info->devId, modOut->thisMod->info->chnId, &stAttr))
        {
            PTREE_ERR("MI_VENC_SetChnAttr Failed\n");
            return SSOS_DEF_FAIL;
        }
        return SSOS_DEF_OK;
    }
    vencInfo->u32Width  = width;
    vencInfo->u32Height = height;
    return SSOS_DEF_OK;
}
PTREE_MOD_Obj_t *PTREE_MOD_VENC_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_VENC_Obj_t * vencMod  = NULL;
    PTREE_SUR_VENC_Info_t *vencInfo = NULL;

    vencMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_VENC_Obj_t));
    if (!vencMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(vencMod, 0, sizeof(PTREE_MOD_VENC_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&vencMod->base, &G_PTREE_MOD_VENC_OPS, tag, E_MI_MODULE_ID_VENC))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (vencMod->base.base.info->devId >= PTREE_MOD_VENC_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", vencInfo->base.base.devId, PTREE_MOD_VENC_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    vencInfo = CONTAINER_OF(vencMod->base.base.info, PTREE_SUR_VENC_Info_t, base.base);
    if (g_vencDevInfo[vencMod->base.base.info->devId].maxWidth < vencInfo->u32MaxWidth)
    {
        g_vencDevInfo[vencMod->base.base.info->devId].maxWidth = vencInfo->u32MaxWidth;
    }
    if (g_vencDevInfo[vencMod->base.base.info->devId].maxHeight < vencInfo->u32MaxHeight)
    {
        g_vencDevInfo[vencMod->base.base.info->devId].maxHeight = vencInfo->u32MaxHeight;
    }

    PTREE_MOD_SYS_ObjRegister(&vencMod->base, &G_PTREE_MOD_VENC_HOOK);
    return &vencMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&vencMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(vencMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(VENC, PTREE_MOD_VENC_New);
