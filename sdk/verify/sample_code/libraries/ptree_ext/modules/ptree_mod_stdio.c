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
#include "ssos_io.h"
#include "ptree_linker.h"
#include "ptree_maker.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ssos_list.h"
#include "ptree_mod.h"
#include "ptree_packer.h"
#include "ptree_packet.h"
#include "ptree_packet_raw.h"
#include "ptree_packet_video.h"
#include "ptree_packet_audio.h"
#include "ssos_task.h"
#include "ptree_mod_rgn_metadata.h"
#include "ptree_sur_stdio.h"
#include "ptree_mod_stdio.h"
#include "ssos_time.h"
#include "ptree_rgn_packet.h"

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

#define PTREE_MOD_STDIO_READER_TASK_NAME_LEN (32)

typedef struct PTREE_MOD_STDIO_Linker_s PTREE_MOD_STDIO_Linker_t;
typedef struct PTREE_MOD_STDIO_Obj_s    PTREE_MOD_STDIO_Obj_t;
typedef struct PTREE_MOD_STDIO_InObj_s  PTREE_MOD_STDIO_InObj_t;
typedef struct PTREE_MOD_STDIO_OutObj_s PTREE_MOD_STDIO_OutObj_t;

struct PTREE_MOD_STDIO_Linker_s
{
    PTREE_LINKER_Obj_t base;
    unsigned int       port;
};

struct PTREE_MOD_STDIO_Obj_s
{
    PTREE_MOD_Obj_t    base;
    PTREE_PACKER_Obj_t packer;
};

struct PTREE_MOD_STDIO_InObj_s
{
    PTREE_MOD_InObj_t base;
};

struct PTREE_MOD_STDIO_OutObj_s
{
    PTREE_MOD_OutObj_t base;
    void *             taskHandle;
    unsigned int       randSeed;
};

static void         _PTREE_MOD_STDIO_SRand(unsigned int *seed);
static unsigned int _PTREE_MOD_STDIO_Rand(unsigned int *seed);

static PTREE_MOD_InObj_t * _PTREE_MOD_STDIO_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_STDIO_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_STDIO_Free(PTREE_MOD_Obj_t *mod);

static int                 _PTREE_MOD_STDIO_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_STDIO_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_STDIO_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static void                _PTREE_MOD_STDIO_InFree(PTREE_MOD_InObj_t *modIn);
static PTREE_MOD_InObj_t * _PTREE_MOD_STDIO_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static void *_PTREE_MOD_STDIO_RawVideoReader(struct SSOS_TASK_Buffer_s *taskBuf);
static void *_PTREE_MOD_STDIO_MetaRgnRectAreaRandomReader(struct SSOS_TASK_Buffer_s *taskBuf);

static int                  _PTREE_MOD_STDIO_OutStart(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_STDIO_OutStop(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_STDIO_OutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_STDIO_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_STDIO_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_STDIO_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static void                 _PTREE_MOD_STDIO_OutFree(PTREE_MOD_OutObj_t *modOut);
static PTREE_MOD_OutObj_t * _PTREE_MOD_STDIO_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static int                 _PTREE_MOD_STDIO_LinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_STDIO_LinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);
static void                _PTREE_MOD_STDIO_LinkerFree(PTREE_LINKER_Obj_t *linker);

static PTREE_PACKET_Obj_t *_PTREE_MOD_STDIO_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info);

static const PTREE_PACKER_Ops_t G_STDIO_PACKER_OPS = {
    .make = _PTREE_MOD_STDIO_PackerMake,
};

static const PTREE_PACKER_Hook_t G_STDIO_PACKER_HOOK = {};

static const PTREE_LINKER_Ops_t G_STDIO_LINKER_OPS = {
    .enqueue = _PTREE_MOD_STDIO_LinkerEnqueue,
    .dequeue = _PTREE_MOD_STDIO_LinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_STDIO_LINKER_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_STDIO_LinkerFree,
};

static const PTREE_MOD_Ops_t G_PTREE_MOD_STDIO_OPS = {
    .init   = NULL,
    .deinit = NULL,
    .start  = NULL,
    .stop   = NULL,

    .createModIn  = _PTREE_MOD_STDIO_CreateModIn,
    .createModOut = _PTREE_MOD_STDIO_CreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_STDIO_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_STDIO_Free,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_STDIO_IN_OPS = {
    .start = NULL,
    .stop  = NULL,

    .directBind   = NULL,
    .directUnbind = NULL,

    .getType = _PTREE_MOD_STDIO_InGetType,

    .isPostReader = NULL,
    .isDelayLink  = NULL,

    .createNLinker = _PTREE_MOD_STDIO_InCreateNLinker,
    .createPacker  = _PTREE_MOD_STDIO_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_STDIO_IN_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_STDIO_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_STDIO_OUT_OPS = {
    .start = _PTREE_MOD_STDIO_OutStart,
    .stop  = _PTREE_MOD_STDIO_OutStop,

    .getType = _PTREE_MOD_STDIO_OutGetType,

    .createNLinker = NULL,

    .getPacketInfo = _PTREE_MOD_STDIO_OutGetPacketInfo,

    .linked           = _PTREE_MOD_STDIO_OutLinked,
    .unlinked         = _PTREE_MOD_STDIO_OutUnlinked,
    .linkedTransfer   = NULL,
    .unlinkedTransfer = NULL,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_STDIO_OUT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_STDIO_OutFree,
};

static void _PTREE_MOD_STDIO_SRand(unsigned int *seed)
{
    SSOS_TIME_Spec_t tv;
    SSOS_TIME_Get(&tv);
    *seed = tv.tvSec * 1000000000 + tv.tvNSec;
}
static unsigned int _PTREE_MOD_STDIO_Rand(unsigned int *seed)
{
    *seed = (*seed * 1103515245U + 12345U) & 0x7fffffffU;
    return *seed;
}

static PTREE_PACKET_Obj_t *_PTREE_MOD_STDIO_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info)
{
    (void)packer;
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(raw)))
    {
        PTREE_PACKET_RAW_Info_t *rawInfo = CONTAINER_OF(info, PTREE_PACKET_RAW_Info_t, base);
        return PTREE_PACKET_RAW_NormalNew(&rawInfo->rawInfo);
    }
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(video)))
    {
        PTREE_PACKET_VIDEO_Info_t *videoInfo = CONTAINER_OF(info, PTREE_PACKET_VIDEO_Info_t, base);
        return PTREE_PACKET_VIDEO_NormalNew(&videoInfo->packetInfo);
    }
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_PACKET_AUDIO_Info_t *audioInfo = CONTAINER_OF(info, PTREE_PACKET_AUDIO_Info_t, base);
        return PTREE_PACKET_AUDIO_NormalNew(&audioInfo->packetInfo);
    }
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(rects)))
    {
        PTREE_RGN_PACKET_RectsInfo_t *rectsInfo = CONTAINER_OF(info, PTREE_RGN_PACKET_RectsInfo_t, base);
        return PTREE_RGN_PACKET_RectsNew(rectsInfo->count);
    }
    return NULL;
}
static int _PTREE_MOD_STDIO_LinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    int                       i           = 0;
    PTREE_MOD_STDIO_Linker_t *stdioLinker = CONTAINER_OF(linker, PTREE_MOD_STDIO_Linker_t, base);
    SSOS_IO_Printf("================ STDIO [%d] >>>>>>>>>>>>>>>>\n", stdioLinker->port);
    SSOS_IO_Printf("[%p-%s]-", packet->info->type, packet->info->type);
    SSOS_IO_Printf("{ ");
    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(raw)))
    {
        PTREE_PACKET_RAW_Info_t *rawInfo = CONTAINER_OF(packet->info, PTREE_PACKET_RAW_Info_t, base);
        SSOS_IO_Printf("[%s]-[%dx%d]", PTREE_ENUM_TO_STR(PTREE_PACKET_RAW_VideoFmt_e, rawInfo->rawInfo.fmt),
                       rawInfo->rawInfo.width, rawInfo->rawInfo.height);
    }
    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(video)))
    {
        PTREE_PACKET_VIDEO_Info_t *videoInfo = CONTAINER_OF(packet->info, PTREE_PACKET_VIDEO_Info_t, base);
        SSOS_IO_Printf("[%s]-[%dx%d]-", PTREE_ENUM_TO_STR(PTREE_PACKET_VIDEO_Fmt_e, videoInfo->packetInfo.fmt),
                       videoInfo->packetInfo.width, videoInfo->packetInfo.height);
        for (i = 0; i < videoInfo->packetInfo.pktCount; ++i)
        {
            SSOS_IO_Printf("([%d]-%d)", i, videoInfo->packetInfo.pktInfo[i].size);
        }
    }
    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_PACKET_AUDIO_Info_t *audioInfo = CONTAINER_OF(packet->info, PTREE_PACKET_AUDIO_Info_t, base);
        SSOS_IO_Printf("[%s]-[%d,%d,%d]-", PTREE_ENUM_TO_STR(PTREE_PACKET_AUDIO_Fmt_e, audioInfo->packetInfo.fmt),
                       audioInfo->packetInfo.channels, audioInfo->packetInfo.sampleRate,
                       audioInfo->packetInfo.sampleWidth);
        for (i = 0; i < audioInfo->packetInfo.pktCount; ++i)
        {
            SSOS_IO_Printf("([%d]-%d)", i, audioInfo->packetInfo.pktInfo[i].size);
        }
    }
    if (PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(rects)))
    {
        PTREE_RGN_PACKET_RectsInfo_t *rectsInfo = CONTAINER_OF(packet->info, PTREE_RGN_PACKET_RectsInfo_t, base);
        SSOS_IO_Printf("[%d]", rectsInfo->count);
    }
    SSOS_IO_Printf(" }\n");
    SSOS_IO_Printf("================ STDIO [%d] <<<<<<<<<<<<<<<<\n", stdioLinker->port);
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_STDIO_LinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}
static void _PTREE_MOD_STDIO_LinkerFree(PTREE_LINKER_Obj_t *linker)
{
    PTREE_MOD_STDIO_Linker_t *stdioLinker = CONTAINER_OF(linker, PTREE_MOD_STDIO_Linker_t, base);
    SSOS_MEM_Free(stdioLinker);
}

static PTREE_MOD_InObj_t *_PTREE_MOD_STDIO_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_STDIO_InNew(mod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_STDIO_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_STDIO_OutNew(mod, loopId);
}
static void _PTREE_MOD_STDIO_Free(PTREE_MOD_Obj_t *mod)
{
    SSOS_MEM_Free(CONTAINER_OF(mod, PTREE_MOD_STDIO_Obj_t, base));
}

static int _PTREE_MOD_STDIO_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_STDIO_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_STDIO_Linker_t *stdioLinker = NULL;

    stdioLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_STDIO_Linker_t));
    if (!stdioLinker)
    {
        PTREE_ERR("ALloc err");
        return NULL;
    }
    memset(stdioLinker, 0, sizeof(PTREE_MOD_STDIO_Linker_t));
    if (SSOS_DEF_OK != PTREE_LINKER_Init(&stdioLinker->base, &G_STDIO_LINKER_OPS))
    {
        SSOS_MEM_Free(stdioLinker);
        return NULL;
    }

    stdioLinker->port = modIn->info->port;
    PTREE_LINKER_Register(&stdioLinker->base, &G_STDIO_LINKER_HOOK);
    return &stdioLinker->base;
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_STDIO_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_STDIO_Obj_t *stdioMod = CONTAINER_OF(modIn->thisMod, PTREE_MOD_STDIO_Obj_t, base);

    *isFast = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&stdioMod->packer);
}
static void _PTREE_MOD_STDIO_InFree(PTREE_MOD_InObj_t *modIn)
{
    SSOS_MEM_Free(CONTAINER_OF(modIn, PTREE_MOD_STDIO_InObj_t, base));
}
static PTREE_MOD_InObj_t *_PTREE_MOD_STDIO_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_STDIO_InObj_t *stdioModIn = NULL;

    stdioModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_STDIO_InObj_t));
    if (!stdioModIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(stdioModIn, 0, sizeof(PTREE_MOD_STDIO_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&stdioModIn->base, &G_PTREE_MOD_STDIO_IN_OPS, mod, loopId))
    {
        SSOS_MEM_Free(stdioModIn);
        return NULL;
    }
    PTREE_MOD_InObjRegister(&stdioModIn->base, &G_PTREE_MOD_STDIO_IN_HOOK);
    return &stdioModIn->base;
}

static void *_PTREE_MOD_STDIO_RawVideoReader(struct SSOS_TASK_Buffer_s *taskBuf)
{
    PTREE_MOD_OutObj_t *       modOut       = taskBuf->buf;
    PTREE_SUR_STDIO_OutInfo_t *stdioOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_STDIO_OutInfo_t, base);
    PTREE_PACKET_Obj_t *       packet       = NULL;
    PTREE_PACKET_RAW_Obj_t *   rawPacket    = NULL;
    PTREE_PACKET_RAW_Info_t    rawInfo;

    memset(&rawInfo, 0, sizeof(PTREE_PACKET_RAW_Info_t));

    PTREE_PACKET_RAW_InfoInit(&rawInfo, &stdioOutInfo->info.rawVideoInfo.rawInfo);
    packet = PTREE_PACKER_Make(&modOut->packer.base, &rawInfo.base);
    if (!packet)
    {
        return NULL;
    }
    rawPacket = CONTAINER_OF(packet, PTREE_PACKET_RAW_Obj_t, base);

    if (rawPacket->rawData.data[0])
    {
        memset(rawPacket->rawData.data[0], stdioOutInfo->info.rawVideoInfo.rawData[0], rawPacket->rawData.size[0]);
    }
    if (rawPacket->rawData.data[1])
    {
        memset(rawPacket->rawData.data[1], stdioOutInfo->info.rawVideoInfo.rawData[1], rawPacket->rawData.size[1]);
    }

    PTREE_LINKER_Enqueue(&modOut->plinker.base, packet);
    PTREE_PACKET_Del(packet);
    return NULL;
}

static void *_PTREE_MOD_STDIO_MetaRgnRectAreaRandomReader(struct SSOS_TASK_Buffer_s *taskBuf)
{
    PTREE_MOD_OutObj_t *       modOut       = taskBuf->buf;
    PTREE_MOD_STDIO_OutObj_t * stdioOut     = CONTAINER_OF(modOut, PTREE_MOD_STDIO_OutObj_t, base);
    PTREE_SUR_STDIO_OutInfo_t *stdioOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_STDIO_OutInfo_t, base);

    PTREE_PACKET_Obj_t *         packet = NULL;
    PTREE_RGN_PACKET_Rects_t *   rects  = NULL;
    PTREE_RGN_PACKET_RectsInfo_t rectsInfo;
    int                          i = 0;

    memset(&rectsInfo, 0, sizeof(PTREE_RGN_PACKET_RectsInfo_t));
    PTREE_RGN_PACKET_RectsInfoInit(&rectsInfo, stdioOutInfo->info.metaRgnFrameInfo.number);

    packet = PTREE_PACKER_Make(&modOut->packer.base, &rectsInfo.base);
    if (!packet)
    {
        return NULL;
    }
    rects = CONTAINER_OF(packet, PTREE_RGN_PACKET_Rects_t, base);

    for (i = 0; i < rects->info.count; ++i)
    {
        rects->rects[i].x = _PTREE_MOD_STDIO_Rand(&stdioOut->randSeed) % (PTREE_MOD_RGN_METADATA_COORDINATE_MAX_W - 1);
        rects->rects[i].y = _PTREE_MOD_STDIO_Rand(&stdioOut->randSeed) % (PTREE_MOD_RGN_METADATA_COORDINATE_MAX_H - 1);
        rects->rects[i].w =
            _PTREE_MOD_STDIO_Rand(&stdioOut->randSeed) % (PTREE_MOD_RGN_METADATA_COORDINATE_MAX_W - rects->rects[i].x)
            + 1;
        rects->rects[i].h =
            _PTREE_MOD_STDIO_Rand(&stdioOut->randSeed) % (PTREE_MOD_RGN_METADATA_COORDINATE_MAX_H - rects->rects[i].y)
            + 1;
    }
    PTREE_LINKER_Enqueue(&modOut->plinker.base, packet);
    PTREE_PACKET_Del(packet);
    return NULL;
}
static int _PTREE_MOD_STDIO_OutStart(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_STDIO_OutObj_t *stdioOut = CONTAINER_OF(modOut, PTREE_MOD_STDIO_OutObj_t, base);
    _PTREE_MOD_STDIO_SRand(&stdioOut->randSeed);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_STDIO_OutStop(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_STDIO_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_PACKET_Info_t *_PTREE_MOD_STDIO_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_SUR_STDIO_OutInfo_t *stdioOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_STDIO_OutInfo_t, base);
    if (stdioOutInfo->info.mode == E_PTREE_SUR_STDIO_OUT_MODE_META_RGN_FRAME)
    {
        return PTREE_RGN_PACKET_RectsInfoNew(0);
    }
    if (stdioOutInfo->info.mode == E_PTREE_SUR_STDIO_OUT_MODE_RAW_VIDEO)
    {
        return PTREE_PACKET_RAW_InfoNew(&stdioOutInfo->info.rawVideoInfo.rawInfo);
    }
    return NULL;
}
static int _PTREE_MOD_STDIO_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_STDIO_OutObj_t * stdioOut     = CONTAINER_OF(modOut, PTREE_MOD_STDIO_OutObj_t, base);
    PTREE_SUR_STDIO_OutInfo_t *stdioOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_STDIO_OutInfo_t, base);
    SSOS_TASK_Attr_t           taskAttr;
    char                       taskName[PTREE_MOD_STDIO_READER_TASK_NAME_LEN];

    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }

    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    memset(taskName, 0, PTREE_MOD_STDIO_READER_TASK_NAME_LEN);
    if (stdioOutInfo->info.mode == E_PTREE_SUR_STDIO_OUT_MODE_META_RGN_FRAME)
    {
        if (stdioOutInfo->info.metaRgnFrameInfo.layout == E_PTREE_SUR_STDIO_META_RGN_FRAME_LAYOUT_RANDOM)
        {
            taskAttr.doMonitor = _PTREE_MOD_STDIO_MetaRgnRectAreaRandomReader;
        }
    }
    else if (stdioOutInfo->info.mode == E_PTREE_SUR_STDIO_OUT_MODE_RAW_VIDEO)
    {
        taskAttr.doMonitor = _PTREE_MOD_STDIO_RawVideoReader;
    }

    if (stdioOutInfo->info.isUserTrigger)
    {
        taskAttr.monitorCycleNsec = 0;
    }
    else
    {
        taskAttr.monitorCycleNsec = 1000000000 / modOut->info->fps;
    }

    if (!taskAttr.doMonitor)
    {
        PTREE_ERR("No matched reader");
        return SSOS_DEF_FAIL;
    }

    taskAttr.inBuf.buf       = (void *)modOut;
    taskAttr.inBuf.size      = 0;
    taskAttr.threadAttr.name = PTREE_MOD_OutKeyStr(modOut, taskName, PTREE_MOD_STDIO_READER_TASK_NAME_LEN);

    stdioOut->taskHandle = SSOS_TASK_Open(&taskAttr);
    if (!stdioOutInfo->info.isUserTrigger)
    {
        SSOS_TASK_StartMonitor(stdioOut->taskHandle);
    }
    PTREE_DBG("STDIO reader %d start as %s mode", stdioOut->base.info->port,
              stdioOutInfo->info.isUserTrigger ? "user tirgger" : "monitor");
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_STDIO_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_STDIO_OutObj_t * stdioOut     = CONTAINER_OF(modOut, PTREE_MOD_STDIO_OutObj_t, base);
    PTREE_SUR_STDIO_OutInfo_t *stdioOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_STDIO_OutInfo_t, base);
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (!stdioOut->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    if (!stdioOutInfo->info.isUserTrigger)
    {
        SSOS_TASK_Stop(stdioOut->taskHandle);
    }
    SSOS_TASK_Close(stdioOut->taskHandle);
    stdioOut->taskHandle = NULL;
    PTREE_DBG("STDIO reader %d stop", stdioOut->base.info->port);
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_STDIO_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    SSOS_MEM_Free(CONTAINER_OF(modOut, PTREE_MOD_STDIO_OutObj_t, base));
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_STDIO_OutNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_STDIO_OutObj_t *stdioModOut = NULL;

    stdioModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_STDIO_OutObj_t));
    if (!stdioModOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(stdioModOut, 0, sizeof(PTREE_MOD_STDIO_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&stdioModOut->base, &G_PTREE_MOD_STDIO_OUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(stdioModOut);
        return NULL;
    }
    stdioModOut->taskHandle = NULL;
    _PTREE_MOD_STDIO_SRand(&stdioModOut->randSeed);
    PTREE_MOD_OutObjRegister(&stdioModOut->base, &G_PTREE_MOD_STDIO_OUT_HOOK);
    return &stdioModOut->base;
}

int PTREE_MOD_STDIO_UserTirgger(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_STDIO_OutObj_t *stdioOut = CONTAINER_OF(modOut, PTREE_MOD_STDIO_OutObj_t, base);
    return SSOS_TASK_OneShot(stdioOut->taskHandle);
}

PTREE_MOD_Obj_t *PTREE_MOD_STDIO_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_STDIO_Obj_t *stdioMod = NULL;

    stdioMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_STDIO_Obj_t));
    if (!stdioMod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(stdioMod, 0, sizeof(PTREE_MOD_STDIO_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&stdioMod->base, &G_PTREE_MOD_STDIO_OPS, tag))
    {
        SSOS_MEM_Free(stdioMod);
        return NULL;
    }
    if (SSOS_DEF_OK != PTREE_PACKER_Init(&stdioMod->packer, &G_STDIO_PACKER_OPS))
    {
        PTREE_MOD_ObjDel(&stdioMod->base);
        SSOS_MEM_Free(stdioMod);
        return NULL;
    }
    PTREE_PACKER_Register(&stdioMod->packer, &G_STDIO_PACKER_HOOK);
    PTREE_MOD_ObjRegister(&stdioMod->base, &G_PTREE_MOD_STDIO_HOOK);
    return &stdioMod->base;
}

PTREE_MAKER_MOD_INIT(STDIO, PTREE_MOD_STDIO_New);
