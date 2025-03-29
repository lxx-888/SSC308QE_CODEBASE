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
#include "mi_sys_datatype.h"
#include "mi_sys.h"
#include "mi_ai_datatype.h"
#include "mi_ai.h"
#include "cam_dev_wrapper.h"
#include "ssos_def.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ssos_task.h"
#include "ptree_linker.h"
#include "ptree_nlinker_out.h"
#include "ptree_packet.h"
#include "ptree_packet_audio.h"
#include "ptree_mod.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_ai.h"
#include "ptree_sur_ai.h"
#include "ptree_maker.h"

#define PTREE_MOD_AI_READER_TASK_NAME_LEN 32

typedef struct PTREE_MOD_AI_BufInfo_s   PTREE_MOD_AI_BufInfo_t;
typedef struct PTREE_MOD_AI_OutLinker_s PTREE_MOD_AI_OutLinker_t;
typedef struct PTREE_MOD_AI_Packet_s    PTREE_MOD_AI_Packet_t;
typedef struct PTREE_MOD_AI_OutObj_s    PTREE_MOD_AI_OutObj_t;
typedef struct PTREE_MOD_AI_Obj_s       PTREE_MOD_AI_Obj_t;

struct PTREE_MOD_AI_BufInfo_s
{
    unsigned int                  u32DevId;
    unsigned int                  u32ChnGrpIdx;
    unsigned int                  u32sampleRate;
    unsigned int                  u32SoundMode;
    unsigned char                 bEcho;
    enum PTREE_PACKET_AUDIO_Fmt_e fmt;
};
struct PTREE_MOD_AI_OutLinker_s
{
    PTREE_NLINKER_OUT_Obj_t base;
    PTREE_MOD_AI_BufInfo_t  stAiInfo;
    int                     fd;
};
struct PTREE_MOD_AI_Packet_s
{
    PTREE_PACKET_AUDIO_Obj_t base;
    unsigned int             u32DevId;
    unsigned int             u32ChnGrpIdx;
    MI_AI_Data_t             stAiChFrame;
    MI_AI_Data_t             stEchoFrame;
    PTREE_PACKET_Obj_t *     dupPacket;
};
struct PTREE_MOD_AI_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};
struct PTREE_MOD_AI_OutObj_s
{
    PTREE_MOD_OutObj_t base;
    int                fd;
    void *             taskHandle;
};

static PTREE_PACKET_Obj_t *_PTREE_MOD_AI_PacketDup(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_AI_PacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_AI_PacketDestruct(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_AI_PacketFree(PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_AI_PacketNew(int fd, const PTREE_MOD_AI_BufInfo_t *pstAiInfo, int ms);

static int _PTREE_MOD_AI_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_AI_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms);
static int  _PTREE_MOD_AI_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static void _PTREE_MOD_AI_OutLinkerDestruct(PTREE_NLINKER_OUT_Obj_t *nlinkerOut);
static void _PTREE_MOD_AI_OutLinkerFree(PTREE_NLINKER_OUT_Obj_t *nlinkerOut);
static PTREE_LINKER_Obj_t *_PTREE_MOD_AI_OutLinkerNew(MI_SYS_ChnPort_t *pstChnPort, PTREE_MOD_AI_BufInfo_t *pstAiInfo);

static int                  _PTREE_MOD_AI_OutStart(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_AI_OutStop(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_AI_OutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_LINKER_Obj_t * _PTREE_MOD_AI_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_AI_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_AI_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_AI_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static void                 _PTREE_MOD_AI_OutFree(PTREE_MOD_OutObj_t *modOut);
static PTREE_MOD_OutObj_t * _PTREE_MOD_AI_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static int                 _PTREE_MOD_AI_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_AI_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_AI_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_AI_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_AI_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static const PTREE_PACKET_ObjOps_t G_PTREE_MOD_AI_PACKET_OPS = {
    .dup             = _PTREE_MOD_AI_PacketDup,
    .convert         = NULL,
    .updateTimeStamp = _PTREE_MOD_AI_PacketUpdateTimeStamp,
};
static const PTREE_PACKET_ObjHook_t G_PTREE_MOD_AI_PACKET_HOOK = {
    .destruct = _PTREE_MOD_AI_PacketDestruct,
    .free     = _PTREE_MOD_AI_PacketFree,
};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_MOD_AI_OUT_LINKER_OPS = {
    .enqueue           = _PTREE_MOD_AI_OutLinkerEnqueue,
    .dequeueOut        = _PTREE_MOD_AI_OutLinkerDequeueOut,
    .dequeueFromInside = _PTREE_MOD_AI_OutLinkerDequeueFromInside,
};
static const PTREE_NLINKER_OUT_Hook_t G_PTREE_MOD_AI_OUT_LINKER_HOOK = {
    .destruct = _PTREE_MOD_AI_OutLinkerDestruct,
    .free     = _PTREE_MOD_AI_OutLinkerFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_AI_OUT_OPS = {
    .start         = _PTREE_MOD_AI_OutStart,
    .stop          = _PTREE_MOD_AI_OutStop,
    .getType       = _PTREE_MOD_AI_OutGetType,
    .createNLinker = _PTREE_MOD_AI_OutCreateNLinker,
    .getPacketInfo = _PTREE_MOD_AI_OutGetPacketInfo,
    .linked        = _PTREE_MOD_AI_OutLinked,
    .unlinked      = _PTREE_MOD_AI_OutUnlinked,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_AI_OUT_HOOK = {
    .free = _PTREE_MOD_AI_OutFree,
};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_AI_SYS_OPS = {
    .init         = _PTREE_MOD_AI_Init,
    .deinit       = _PTREE_MOD_AI_Deinit,
    .createModIn  = _PTREE_MOD_AI_CreateModIn,
    .createModOut = _PTREE_MOD_AI_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_AI_SYS_HOOK = {
    .free = _PTREE_MOD_AI_Free,
};

PTREE_PACKET_TYPE_DEFINE(ai);

static PTREE_PACKET_Obj_t *_PTREE_MOD_AI_PacketDup(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_AI_Packet_t *   aiPacket    = CONTAINER_OF(packet, PTREE_MOD_AI_Packet_t, base.base);
    PTREE_PACKET_AUDIO_Obj_t *dupAudioObj = NULL;
    if (aiPacket->dupPacket)
    {
        /* Using reference count for packet dup. */
        return NULL;
    }
    aiPacket->dupPacket = PTREE_PACKET_AUDIO_NormalNewClone(&aiPacket->base);
    SSOS_ASSERT(aiPacket->dupPacket);
    dupAudioObj = CONTAINER_OF(aiPacket->dupPacket, PTREE_PACKET_AUDIO_Obj_t, base);
    /* Copy address. */
    aiPacket->base.packetData = dupAudioObj->packetData;
    /* Release current address. */
    MI_AI_ReleaseData(aiPacket->u32DevId, aiPacket->u32ChnGrpIdx, &aiPacket->stAiChFrame, &aiPacket->stEchoFrame);
    return NULL;
}
static void _PTREE_MOD_AI_PacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_AI_Packet_t *  aiPacket = CONTAINER_OF(packet, PTREE_MOD_AI_Packet_t, base.base);
    PTREE_PACKET_TimeStamp_t stamp;

    stamp.tvSec  = aiPacket->stAiChFrame.u64Pts / 1000000;
    stamp.tvUSec = aiPacket->stAiChFrame.u64Pts % 1000000;
    PTREE_PACKET_SetTimeStamp(&aiPacket->base.base, &stamp);
}
static void _PTREE_MOD_AI_PacketDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_AI_Packet_t *aiPacket = CONTAINER_OF(packet, PTREE_MOD_AI_Packet_t, base.base);
    if (!aiPacket->dupPacket)
    {
        /* Packet did not dup before. */
        MI_AI_ReleaseData(aiPacket->u32DevId, aiPacket->u32ChnGrpIdx, &aiPacket->stAiChFrame, &aiPacket->stEchoFrame);
        return;
    }
    PTREE_PACKET_Del(aiPacket->dupPacket);
    aiPacket->dupPacket = NULL;
}
static void _PTREE_MOD_AI_PacketFree(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_AI_Packet_t *aiPacket = CONTAINER_OF(packet, PTREE_MOD_AI_Packet_t, base.base);
    SSOS_MEM_Free(aiPacket);
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_AI_PacketNew(int fd, const PTREE_MOD_AI_BufInfo_t *pstAiInfo, int ms)
{
    PTREE_MOD_AI_Packet_t *  aiPacket = NULL;
    struct pollfd            pollFd;
    int                      ret      = SSOS_DEF_OK;
    int                      readyNum = 0;
    PTREE_PACKET_TimeStamp_t stamp;

    aiPacket = SSOS_MEM_Alloc(sizeof(PTREE_MOD_AI_Packet_t));
    if (!aiPacket)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(aiPacket, 0, sizeof(PTREE_MOD_AI_Packet_t));

    if (SSOS_DEF_OK != PTREE_PACKET_AUDIO_Init(&aiPacket->base, &G_PTREE_MOD_AI_PACKET_OPS, PTREE_PACKET_TYPE(ai)))
    {
        PTREE_ERR("PTREE_PACKET_AUDIO_Init Failed");
        goto ERR_AUDIO_INIT;
    }

    aiPacket->u32DevId     = pstAiInfo->u32DevId;
    aiPacket->u32ChnGrpIdx = pstAiInfo->u32ChnGrpIdx;
    memset(&pollFd, 0, sizeof(struct pollfd));
    pollFd.fd     = fd;
    pollFd.events = POLLIN;
    readyNum      = CamDevPoll(&pollFd, 1, ms);
    if (readyNum < 0)
    {
        PTREE_ERR("poll ret = %d", readyNum);
        goto ERR_POLL_FD;
    }
    if (!(pollFd.revents & POLLIN))
    {
        goto ERR_POLL_FD;
    }
    ret = MI_AI_Read(aiPacket->u32DevId, aiPacket->u32ChnGrpIdx, &aiPacket->stAiChFrame, &aiPacket->stEchoFrame, -1);
    if (ret == MI_AI_ERR_NOBUF)
    {
        PTREE_ERR("dev%d, chn_group%d no buffer warning!", aiPacket->u32DevId, aiPacket->u32ChnGrpIdx);
        goto ERR_READ_BUF;
    }
    else if (ret != MI_SUCCESS)
    {
        PTREE_ERR("dev%d, chn_group%d get frame error!", aiPacket->u32DevId, aiPacket->u32ChnGrpIdx);
        goto ERR_READ_BUF;
    }

    aiPacket->base.info.packetInfo.fmt             = pstAiInfo->fmt;
    aiPacket->base.info.packetInfo.sampleRate      = pstAiInfo->u32sampleRate;
    aiPacket->base.info.packetInfo.sampleWidth     = 16;
    aiPacket->base.info.packetInfo.channels        = pstAiInfo->u32SoundMode;
    aiPacket->base.info.packetInfo.pktCount        = 1;
    aiPacket->base.packetData.pktData[0].data      = (char *)aiPacket->stAiChFrame.apvBuffer[0];
    aiPacket->base.info.packetInfo.pktInfo[0].size = aiPacket->stAiChFrame.u32Byte[0];

    if (1 == pstAiInfo->bEcho)
    {
        aiPacket->base.packetData.pktData[1].data      = (char *)aiPacket->stEchoFrame.apvBuffer[0];
        aiPacket->base.info.packetInfo.pktInfo[1].size = aiPacket->stEchoFrame.u32Byte[0];
    }
    stamp.tvSec  = aiPacket->stAiChFrame.u64Pts / 1000000;
    stamp.tvUSec = aiPacket->stAiChFrame.u64Pts % 1000000;
    PTREE_PACKET_SetTimeStamp(&aiPacket->base.base, &stamp);
    PTREE_PACKET_Register(&aiPacket->base.base, &G_PTREE_MOD_AI_PACKET_HOOK);
    return &aiPacket->base.base;

ERR_READ_BUF:
ERR_POLL_FD:
    PTREE_PACKET_Del(&aiPacket->base.base);
ERR_AUDIO_INIT:
    SSOS_MEM_Free(aiPacket);
ERR_MEM_ALLOC:
    return NULL;
}

static int _PTREE_MOD_AI_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_AI_OutLinker_t *aiLinker       = CONTAINER_OF(nlinkerOut, PTREE_MOD_AI_OutLinker_t, base);
    PTREE_PACKET_AUDIO_Obj_t *packetAudioSrc = NULL;
    PTREE_PACKET_AUDIO_Obj_t *packetAudioDst = NULL;
    PTREE_PACKET_Obj_t *      packetSrc      = NULL;

    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_ERR("Enqueue packet type is not audio.");
        return SSOS_DEF_FAIL;
    }

    packetSrc = _PTREE_MOD_AI_PacketNew(aiLinker->fd, &aiLinker->stAiInfo, 300);
    if (!packetSrc)
    {
        return SSOS_DEF_FAIL;
    }

    if (!PTREE_PACKET_InfoEqual(packet->info, packetSrc->info))
    {
        PTREE_ERR("Enqueue packet info is not matched.");
        PTREE_PACKET_Del(packetSrc);
        return SSOS_DEF_FAIL;
    }

    packetAudioSrc = CONTAINER_OF(packetSrc, PTREE_PACKET_AUDIO_Obj_t, base);
    packetAudioDst = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);

    PTREE_PACKET_AUDIO_Copy(packetAudioDst, packetAudioSrc);
    PTREE_PACKET_Del(packetSrc);
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_AI_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms)
{
    PTREE_MOD_AI_OutLinker_t *aiLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_AI_OutLinker_t, base);
    return _PTREE_MOD_AI_PacketNew(aiLinker->fd, &aiLinker->stAiInfo, ms);
}
static int _PTREE_MOD_AI_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_AI_OutLinkerDestruct(PTREE_NLINKER_OUT_Obj_t *nlinkerOut)
{
    PTREE_MOD_AI_OutLinker_t *aiLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_AI_OutLinker_t, base);
    if (aiLinker->fd > 0)
    {
        MI_SYS_CloseFd(aiLinker->fd);
    }
}
static void _PTREE_MOD_AI_OutLinkerFree(PTREE_NLINKER_OUT_Obj_t *nlinkerOut)
{
    PTREE_MOD_AI_OutLinker_t *aiLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_AI_OutLinker_t, base);
    SSOS_MEM_Free(aiLinker);
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_AI_OutLinkerNew(MI_SYS_ChnPort_t *pstChnPort, PTREE_MOD_AI_BufInfo_t *pstAiInfo)
{
    int                       ret      = SSOS_DEF_OK;
    PTREE_MOD_AI_OutLinker_t *aiLinker = NULL;

    aiLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_AI_OutLinker_t));
    if (!aiLinker)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(aiLinker, 0, sizeof(PTREE_MOD_AI_OutLinker_t));

    ret = PTREE_NLINKER_OUT_Init(&aiLinker->base, &G_PTREE_MOD_AI_OUT_LINKER_OPS);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_LINKER_Init Failed");
        goto ERR_NLINKER_OUT_INIT;
    }

    ret = MI_SYS_GetFd(pstChnPort, &aiLinker->fd);
    if (ret != MI_SUCCESS)
    {
        PTREE_ERR("MI_SYS_GetFd Failed ret = %d", ret);
        goto ERR_GET_FD;
    }
    aiLinker->stAiInfo = *pstAiInfo;

    PTREE_NLINKER_OUT_Register(&aiLinker->base, &G_PTREE_MOD_AI_OUT_LINKER_HOOK);
    return &aiLinker->base.base;

ERR_GET_FD:
    PTREE_LINKER_Del(&aiLinker->base.base);
ERR_NLINKER_OUT_INIT:
    SSOS_MEM_Free(aiLinker);
ERR_MEM_ALLOC:
    return NULL;
}

static void *_PTREE_MOD_AI_Reader(SSOS_TASK_Buffer_t *taskBuf)
{
    PTREE_MOD_OutObj_t *   pstModOutObj = taskBuf->buf;
    PTREE_MOD_AI_OutObj_t *pstAiOutObj  = CONTAINER_OF(pstModOutObj, PTREE_MOD_AI_OutObj_t, base);
    PTREE_PACKET_Obj_t *   pstPacket    = NULL;
    PTREE_MOD_AI_BufInfo_t stAiInfo;
    PTREE_SUR_AI_Info_t *  info = CONTAINER_OF(pstModOutObj->thisMod->info, PTREE_SUR_AI_Info_t, base.base);

    memset(&stAiInfo, 0, sizeof(PTREE_MOD_AI_BufInfo_t));
    stAiInfo.u32DevId      = pstModOutObj->thisMod->info->devId;
    stAiInfo.u32ChnGrpIdx  = pstModOutObj->thisMod->info->chnId;
    stAiInfo.bEcho         = info->u32Echo;
    stAiInfo.fmt           = E_PTREE_PACKET_AUDIO_STREAM_WAV;
    stAiInfo.u32sampleRate = (MI_AUDIO_SampleRate_e)info->u32SampleRate;
    stAiInfo.u32SoundMode  = info->eAiSoundMode;

    pstPacket = _PTREE_MOD_AI_PacketNew(pstAiOutObj->fd, &stAiInfo, 10);
    if (pstPacket != NULL)
    {
        PTREE_LINKER_Enqueue(&pstModOutObj->plinker.base, pstPacket);
        PTREE_PACKET_Del(pstPacket);
    }

    return NULL;
}
static int _PTREE_MOD_AI_OutStart(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_AI_OutObj_t *      aiModOut   = CONTAINER_OF(modOut, PTREE_MOD_AI_OutObj_t, base);
    PTREE_SUR_SYS_OutInfo_t *    sysOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_SYS_OutInfo_t, base);
    MI_SYS_ChnPort_t             chnPort;
    MI_SYS_FrameBufExtraConfig_t bufExtCfg;

    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = E_MI_MODULE_ID_AI;
    chnPort.u32DevId  = modOut->thisMod->info->devId;
    chnPort.u32ChnId  = modOut->thisMod->info->chnId;
    chnPort.u32PortId = modOut->info->port;

    if (sysOutInfo->depthEn)
    {
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &chnPort, sysOutInfo->depthUser, sysOutInfo->depthTotal))
        {
            PTREE_ERR("MI_SYS_SetChnOutputPortDepth Failed");
        }
    }
    if (sysOutInfo->extEn)
    {
        memset(&bufExtCfg, 0, sizeof(MI_SYS_FrameBufExtraConfig_t));

        bufExtCfg.u16BufHAlignment        = sysOutInfo->extHAlign;
        bufExtCfg.u16BufVAlignment        = sysOutInfo->extVAlign;
        bufExtCfg.u16BufChromaAlignment   = sysOutInfo->extChromaAlign;
        bufExtCfg.u16BufCompressAlignment = sysOutInfo->extCompressAlign;
        bufExtCfg.u16BufExtraSize         = sysOutInfo->extExtraSize;
        bufExtCfg.bClearPadding           = sysOutInfo->extClearPadding;
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortBufExtConf(&chnPort, &bufExtCfg))
        {
            PTREE_ERR("MI_SYS_SetChnOutputPortBufExtConf failed");
        }
    }

    if (MI_SUCCESS != MI_SYS_GetFd(&chnPort, &aiModOut->fd))
    {
        PTREE_ERR("MI_SYS_GetFd Failed");
        return SSOS_DEF_FAIL;
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_AI_OutStop(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_AI_OutObj_t *aiModOut = CONTAINER_OF(modOut, PTREE_MOD_AI_OutObj_t, base);
    if (aiModOut->fd > 0)
    {
        MI_SYS_CloseFd(aiModOut->fd);
        aiModOut->fd = -1;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_AI_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_AI_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    MI_SYS_ChnPort_t       chnPort;
    PTREE_MOD_AI_BufInfo_t aiBufInfo;
    PTREE_SUR_AI_Info_t *  aiInfo = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_AI_Info_t, base.base);

    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = E_MI_MODULE_ID_AI;
    chnPort.u32DevId  = modOut->thisMod->info->devId;
    chnPort.u32ChnId  = modOut->thisMod->info->chnId;
    chnPort.u32PortId = modOut->info->port;
    memset(&aiBufInfo, 0, sizeof(PTREE_MOD_AI_BufInfo_t));
    aiBufInfo.u32DevId      = modOut->thisMod->info->devId;
    aiBufInfo.u32ChnGrpIdx  = modOut->thisMod->info->chnId;
    aiBufInfo.bEcho         = aiInfo->u32Echo;
    aiBufInfo.fmt           = E_PTREE_PACKET_AUDIO_STREAM_WAV;
    aiBufInfo.u32sampleRate = (MI_AUDIO_SampleRate_e)aiInfo->u32SampleRate;

    return _PTREE_MOD_AI_OutLinkerNew(&chnPort, &aiBufInfo);
}
static PTREE_PACKET_Info_t *_PTREE_MOD_AI_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_SUR_AI_Info_t *           aiInfo = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_AI_Info_t, base.base);
    PTREE_PACKET_AUDIO_PacketInfo_t audioPacketInfo;
    memset(&audioPacketInfo, 0, sizeof(PTREE_PACKET_AUDIO_PacketInfo_t));
    audioPacketInfo.fmt         = E_PTREE_PACKET_AUDIO_STREAM_PCM;
    audioPacketInfo.channels    = aiInfo->eAiSoundMode;
    audioPacketInfo.sampleRate  = (MI_AUDIO_SampleRate_e)aiInfo->u32SampleRate;
    audioPacketInfo.sampleWidth = 16;
    return PTREE_PACKET_AUDIO_InfoNew(&audioPacketInfo);
}
static int _PTREE_MOD_AI_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_AI_OutObj_t *aiModOut = CONTAINER_OF(modOut, PTREE_MOD_AI_OutObj_t, base);
    SSOS_TASK_Attr_t       taskAttr;
    char                   taskName[PTREE_MOD_AI_READER_TASK_NAME_LEN];

    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }

    PTREE_MOD_OutKeyStr(modOut, taskName, PTREE_MOD_AI_READER_TASK_NAME_LEN);
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    taskAttr.doSignal         = NULL;
    taskAttr.doMonitor        = _PTREE_MOD_AI_Reader;
    taskAttr.monitorCycleSec  = 0;
    taskAttr.monitorCycleNsec = 0;
    taskAttr.isResetTimer     = 0;
    taskAttr.inBuf.buf        = (void *)modOut;
    taskAttr.inBuf.size       = 0;
    taskAttr.threadAttr.name  = taskName;
    aiModOut->taskHandle      = SSOS_TASK_Open(&taskAttr);
    if (!aiModOut->taskHandle)
    {
        PTREE_ERR("AI Task create error!");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(aiModOut->taskHandle);
    PTREE_DBG("Ai reader out port %d start", modOut->info->port);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_AI_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_AI_OutObj_t *aiModOut = CONTAINER_OF(modOut, PTREE_MOD_AI_OutObj_t, base);
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (!aiModOut->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    SSOS_TASK_Stop(aiModOut->taskHandle);
    SSOS_TASK_Close(aiModOut->taskHandle);
    aiModOut->taskHandle = NULL;
    PTREE_DBG("Ai reader out port %d stop", modOut->info->port);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_AI_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_AI_OutObj_t *aiModOut = CONTAINER_OF(modOut, PTREE_MOD_AI_OutObj_t, base);
    SSOS_MEM_Free(aiModOut);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_AI_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_AI_OutObj_t *aiModOut = NULL;

    aiModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_AI_OutObj_t));
    if (!aiModOut)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(aiModOut, 0, sizeof(PTREE_MOD_AI_OutObj_t));

    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&aiModOut->base, &G_PTREE_MOD_AI_OUT_OPS, mod, loopId))
    {
        goto ERR_MOD_OUT_INIT;
    }

    PTREE_MOD_OutObjRegister(&aiModOut->base, &G_PTREE_MOD_AI_OUT_HOOK);
    return &aiModOut->base;

ERR_MOD_OUT_INIT:
    SSOS_MEM_Free(aiModOut);
ERR_MEM_ALLOC:
    return NULL;
}
static int _PTREE_MOD_AI_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    MI_AI_Attr_t         stAiSetAttr;
    MI_SYS_ChnPort_t     stAiChnOutputPort;
    MI_AUDIO_I2sConfig_t stAiI2sACfg;
    MI_S32               s32Ret  = MI_SUCCESS;
    MI_BOOL              bEcho   = false;
    PTREE_SUR_AI_Info_t *info    = NULL;
    int                  i       = 0;
    signed short         gain[2] = {0};

    memset(&stAiSetAttr, 0x0, sizeof(MI_AI_Attr_t));
    memset(&stAiChnOutputPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stAiI2sACfg, 0x0, sizeof(stAiI2sACfg));

    info = CONTAINER_OF(sysMod->base.info, PTREE_SUR_AI_Info_t, base.base);

    stAiSetAttr.enFormat     = info->eAiFormat;
    stAiSetAttr.enSampleRate = (MI_AUDIO_SampleRate_e)info->u32SampleRate;
    stAiSetAttr.enSoundMode  = info->eAiSoundMode;

    stAiSetAttr.u32PeriodSize = (MI_U32)info->u32PeriodSize;
    stAiSetAttr.bInterleaved  = (MI_BOOL)info->u32InterLeaved;

    s32Ret = MI_AI_Open((MI_AUDIO_DEV)info->base.base.devId, &stAiSetAttr);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("MI_AI_Open error");
        return -1;
    }

    for (i = 0; i < MI_AI_MAX_CHN_NUM / 2; i++)
    {
        if (i >= MI_AI_MAX_CHN_NUM / 2)
        {
            break;
        }
        if (E_MI_AI_IF_ECHO_A == info->enAiIf[i])
        {
            bEcho         = true;
            info->u32Echo = 1;
        }
        if (info->enAiIf[i] > E_MI_AI_IF_DMIC_A_23 && info->enAiIf[i] < E_MI_AI_IF_ECHO_A)
        {
            stAiI2sACfg.enMode       = info->eAiI2sMode;
            stAiI2sACfg.enFormat     = info->eAiI2sFormat;
            stAiI2sACfg.enSampleRate = (MI_AUDIO_SampleRate_e)info->u32SampleRate;
            stAiI2sACfg.enMclk       = info->eAiI2sMclkE;
            stAiI2sACfg.bSyncClock   = (MI_BOOL)info->u32I2sSyncclock;
            stAiI2sACfg.u32TdmSlots  = (MI_U32)info->u32I2sTdmslots;
            stAiI2sACfg.enBitWidth   = info->eAiI2sBitWidth;
            s32Ret                   = MI_AI_SetI2SConfig(info->enAiIf[i], &stAiI2sACfg);
            if (MI_SUCCESS != s32Ret)
            {
                PTREE_ERR("AI I2s set fail");
                return -1;
            }
        }
    }

    s32Ret = MI_AI_AttachIf((MI_AUDIO_DEV)info->base.base.devId, info->enAiIf, info->u32IfCount);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("AI attach interface fail");
        return -1;
    }

    stAiChnOutputPort.eModId    = E_MI_MODULE_ID_AI;
    stAiChnOutputPort.u32DevId  = (MI_AUDIO_DEV)info->base.base.devId;
    stAiChnOutputPort.u32ChnId  = (MI_AI_CHN)info->base.base.chnId;
    stAiChnOutputPort.u32PortId = 0;

    s32Ret = MI_SYS_SetChnOutputPortDepth(0, &stAiChnOutputPort, 16, 32);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("AI set outbuffer fail");
        return -1;
    }

    s32Ret = MI_AI_EnableChnGroup((MI_AUDIO_DEV)info->base.base.devId, (MI_AI_CHN)info->base.base.chnId);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("AI Enable chn group%d fail\n", (MI_AI_CHN)info->base.base.chnId);
        return -1;
    }

    for (i = 0; i < MI_AI_MAX_CHN_NUM / 2; i++)
    {
        if (E_MI_AI_IF_NONE == info->enAiIf[i] || i >= MI_AI_MAX_CHN_NUM / 2)
        {
            break;
        }
        if (info->enAiIf[i] > E_MI_AI_IF_DMIC_A_23 && info->enAiIf[i] < E_MI_AI_IF_DMIC_A_45)
        {
            continue;
        }
        s32Ret = MI_AI_SetIfGain(info->enAiIf[i], (MI_S16)info->s16Volume, (MI_S16)info->s16Volume);
        if (MI_SUCCESS != s32Ret)
        {
            PTREE_ERR("AI interface %d Set Gain fail", info->enAiIf[i]);
            return -1;
        }
    }
    // echo sholud be set echo gain, default 0
    if (bEcho)
    {
        s32Ret = MI_AI_SetGain((MI_AUDIO_DEV)info->base.base.devId, MI_AI_ECHO_CHN_GROUP_ID, gain, 2);
        if (MI_SUCCESS != s32Ret)
        {
            PTREE_ERR("echo gain set fail");
            return -1;
        }
    }
    return 0;
}
static int _PTREE_MOD_AI_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    MI_S32               s32Ret = MI_SUCCESS;
    PTREE_SUR_AI_Info_t *info   = NULL;
    info                        = CONTAINER_OF(sysMod->base.info, PTREE_SUR_AI_Info_t, base.base);

    s32Ret = MI_AI_DisableChnGroup((MI_AUDIO_DEV)info->base.base.devId, (MI_AI_CHN)info->base.base.chnId);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("AI DisableChnGroup fail");
        return -1;
    }
    s32Ret = MI_AI_Close((MI_AUDIO_DEV)info->base.base.devId);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("MI_AI_Close fail");
        return -1;
    }

    return 0;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_AI_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_AI_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_AI_OutNew(mod, loopId);
}
static void _PTREE_MOD_AI_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_AI_Obj_t, base));
}
PTREE_MOD_Obj_t *PTREE_MOD_AI_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_AI_Obj_t *aiMod = NULL;

    aiMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_AI_Obj_t));
    if (!aiMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(aiMod, 0, sizeof(PTREE_MOD_AI_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&aiMod->base, &G_PTREE_MOD_AI_SYS_OPS, tag, E_MI_MODULE_ID_AI))
    {
        goto ERR_MOD_SYS_INIT;
    }

    PTREE_MOD_SYS_ObjRegister(&aiMod->base, &G_PTREE_MOD_AI_SYS_HOOK);
    return &aiMod->base.base;

ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(aiMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(AI, PTREE_MOD_AI_New);
