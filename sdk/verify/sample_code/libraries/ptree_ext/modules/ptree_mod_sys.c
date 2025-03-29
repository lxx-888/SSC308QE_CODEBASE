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
#include "mi_sys.h"
#include "ssos_def.h"
#include "ptree_log.h"
#include "ssos_list.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_mod_sys.h"
#include "ptree_packer.h"
#include "ptree_sur_sys.h"
#include "ssos_task.h"
#include "ptree_linker.h"
#include "ptree_nlinker_out.h"
#include "ptree_packet.h"
#include "ssos_time.h"
#include "ptree_packet_raw.h"
#include "ptree_dma_packet.h"
#include "ptree_mma_packet.h"

#include "cam_dev_wrapper.h"
#ifdef CAM_OS_RTK
#include "mi_device.h"
#include "mi_sys_internal.h"
#include "mi_common_internal.h"
#include "sys_sys_boot_timestamp.h"
#endif

#define CHECK_POINTER(_ptr, _err)            \
    (                                        \
        {                                    \
            if (!(_ptr))                     \
            {                                \
                PTREE_ERR("%s NULL", #_ptr); \
                _err;                        \
            }                                \
        })

#define PTREE_MOD_SYS_READER_TASK_NAME_LEN (32)

typedef struct PTREE_MOD_SYS_Packer_s
{
    PTREE_PACKER_Obj_t base;
    MI_SYS_ChnPort_t   chnPort;
} PTREE_MOD_SYS_Packer_t;

typedef struct PTREE_MOD_SYS_InLinker_s
{
    PTREE_LINKER_Obj_t base;
    MI_SYS_ChnPort_t   chnPort;
} PTREE_MOD_SYS_InLinker_t;

typedef struct PTREE_MOD_SYS_OutLinker_s
{
    PTREE_NLINKER_OUT_Obj_t base;
    MI_SYS_ChnPort_t        chnPort;
} PTREE_MOD_SYS_OutLinker_t;

typedef struct PTREE_MOD_SYS_InRawPacket_s
{
    PTREE_MMA_PACKET_RawPa_t base;
    MI_SYS_BufInfo_t         bufInfo;
    MI_SYS_BUF_HANDLE        bufHandle;
} PTREE_MOD_SYS_InRawPacket_t;

typedef struct PTREE_MOD_SYS_OutRawPacket_s
{
    PTREE_MMA_PACKET_RawPa_t base;
    MI_SYS_BufInfo_t         bufInfo;
    MI_SYS_BUF_HANDLE        bufHandle;
} PTREE_MOD_SYS_OutRawPacket_t;

static void *_PTREE_MOD_SYS_Reader(SSOS_TASK_Buffer_t *taskBuf);

static int                 _PTREE_MOD_SYS_Init(PTREE_MOD_Obj_t *mod);
static int                 _PTREE_MOD_SYS_Deinit(PTREE_MOD_Obj_t *mod);
static int                 _PTREE_MOD_SYS_Prepare(PTREE_MOD_Obj_t *mod);
static int                 _PTREE_MOD_SYS_Unprepare(PTREE_MOD_Obj_t *mod);
static int                 _PTREE_MOD_SYS_Start(PTREE_MOD_Obj_t *mod);
static int                 _PTREE_MOD_SYS_Stop(PTREE_MOD_Obj_t *mod);
static PTREE_MOD_InObj_t * _PTREE_MOD_SYS_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_SYS_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_SYS_Destruct(PTREE_MOD_Obj_t *mod);
static void                _PTREE_MOD_SYS_Free(PTREE_MOD_Obj_t *mod);

static int                 _PTREE_MOD_SYS_InStart(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_SYS_InStop(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_SYS_InLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static int                 _PTREE_MOD_SYS_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static int                 _PTREE_MOD_SYS_InDirectBind(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_SYS_InDirectUnbind(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_SYS_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYS_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_SYS_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static void                _PTREE_MOD_SYS_InDestruct(PTREE_MOD_InObj_t *modIn);
static void                _PTREE_MOD_SYS_InFree(PTREE_MOD_InObj_t *modIn);

static int                  _PTREE_MOD_SYS_OutStart(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYS_OutStop(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYS_OutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_LINKER_Obj_t * _PTREE_MOD_SYS_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_SYS_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYS_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_SYS_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref);
static int                  _PTREE_MOD_SYS_OutSuspend(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYS_OutResume(PTREE_MOD_OutObj_t *modOut);
static void                 _PTREE_MOD_SYS_OutDestruct(PTREE_MOD_OutObj_t *modOut);
static void                 _PTREE_MOD_SYS_OutFree(PTREE_MOD_OutObj_t *modOut);

static PTREE_MOD_InObj_t *_PTREE_MOD_SYS_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut);
static int _PTREE_MOD_SYS_ResetStreamOut(PTREE_MOD_OutObj_t *modOut, unsigned int width, unsigned int height);

static void               _PTREE_MOD_SYS_InDefaultFree(PTREE_MOD_SYS_InObj_t *sysModIn);
static PTREE_MOD_InObj_t *_PTREE_MOD_SYS_InDefaultNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static void                _PTREE_MOD_SYS_OutDefaultFree(PTREE_MOD_SYS_OutObj_t *sysModOut);
static PTREE_MOD_OutObj_t *_PTREE_MOD_SYS_OutDefaultNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId);

static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info);
static void                _PTREE_MOD_SYS_PackerFree(PTREE_PACKER_Obj_t *packer);
static PTREE_PACKER_Obj_t *_PTREE_MOD_SYS_PackerNew(MI_SYS_ChnPort_t *chnPort);

static int                 _PTREE_MOD_SYS_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);
static void                _PTREE_MOD_SYS_InLinkerFree(PTREE_LINKER_Obj_t *linker);
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYS_InLinkerNew(MI_SYS_ChnPort_t *chnPort);

static int _PTREE_MOD_SYS_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static int _PTREE_MOD_SYS_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms);
static void                _PTREE_MOD_SYS_OutLinkerFree(PTREE_NLINKER_OUT_Obj_t *nlinkerOut);
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYS_OutLinkerNew(MI_SYS_ChnPort_t *chnPort);

static void                _PTREE_MOD_SYS_InRawPacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_SYS_InRawPacketDestruct(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_SYS_InRawPacketFree(PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_InRawPacketNew(const PTREE_PACKET_RAW_Info_t *rawInfo,
                                                         MI_SYS_ChnPort_t *             chnPort);

static void                _PTREE_MOD_SYS_OutRawPacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_SYS_OutRawPacketDestruct(PTREE_PACKET_Obj_t *packet);
static void                _PTREE_MOD_SYS_OutRawPacketFree(PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_OutRawPacketNew(MI_SYS_ChnPort_t *chnPort, int ms);

PTREE_PACKET_TYPE_DEFINE(sys_in);
PTREE_PACKET_TYPE_DEFINE(sys_out);

static const PTREE_MOD_Ops_t G_PTREE_MOD_SYS_OPS = {
    .init         = _PTREE_MOD_SYS_Init,
    .deinit       = _PTREE_MOD_SYS_Deinit,
    .prepare      = _PTREE_MOD_SYS_Prepare,
    .unprepare    = _PTREE_MOD_SYS_Unprepare,
    .start        = _PTREE_MOD_SYS_Start,
    .stop         = _PTREE_MOD_SYS_Stop,
    .createModIn  = _PTREE_MOD_SYS_CreateModIn,
    .createModOut = _PTREE_MOD_SYS_CreateModOut,

};
static const PTREE_MOD_Hook_t G_PTREE_MOD_SYS_HOOK = {
    .destruct = _PTREE_MOD_SYS_Destruct,
    .free     = _PTREE_MOD_SYS_Free,
};
static const PTREE_MOD_InOps_t G_PTREE_MOD_SYS_IN_OPS = {
    .start         = _PTREE_MOD_SYS_InStart,
    .stop          = _PTREE_MOD_SYS_InStop,
    .directBind    = _PTREE_MOD_SYS_InDirectBind,
    .directUnbind  = _PTREE_MOD_SYS_InDirectUnbind,
    .getType       = _PTREE_MOD_SYS_InGetType,
    .createNLinker = _PTREE_MOD_SYS_InCreateNLinker,
    .createPacker  = _PTREE_MOD_SYS_InCreatePacker,
    .linked        = _PTREE_MOD_SYS_InLinked,
    .unlinked      = _PTREE_MOD_SYS_InUnlinked,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_SYS_IN_HOOK = {
    .destruct = _PTREE_MOD_SYS_InDestruct,
    .free     = _PTREE_MOD_SYS_InFree,
};
static const PTREE_MOD_OutOps_t G_PTREE_MOD_SYS_OUT_OPS = {
    .start               = _PTREE_MOD_SYS_OutStart,
    .stop                = _PTREE_MOD_SYS_OutStop,
    .getType             = _PTREE_MOD_SYS_OutGetType,
    .createNLinker       = _PTREE_MOD_SYS_OutCreateNLinker,
    .getPacketInfo       = _PTREE_MOD_SYS_OutGetPacketInfo,
    .linked              = _PTREE_MOD_SYS_OutLinked,
    .unlinked            = _PTREE_MOD_SYS_OutUnlinked,
    .suspend             = _PTREE_MOD_SYS_OutSuspend,
    .resume              = _PTREE_MOD_SYS_OutResume,
    .resetStreamOut      = _PTREE_MOD_SYS_ResetStreamOut,
    .resetStreamTraverse = _PTREE_MOD_SYS_ResetStreamTraverse,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_SYS_OUT_HOOK = {
    .destruct = _PTREE_MOD_SYS_OutDestruct,
    .free     = _PTREE_MOD_SYS_OutFree,
};

static const PTREE_MOD_SYS_InOps_t G_PTREE_MOD_SYS_IN_DEFAULT_OPS = {};

static const PTREE_MOD_SYS_OutOps_t G_PTREE_MOD_SYS_OUT_DEFAULT_OPS = {};

static const PTREE_MOD_SYS_InHook_t G_PTREE_MOD_SYS_IN_DEFAULT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SYS_InDefaultFree,
};

static const PTREE_MOD_SYS_OutHook_t G_PTREE_MOD_SYS_OUT_DEFAULT_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SYS_OutDefaultFree,
};

static const PTREE_PACKER_Ops_t G_PTREE_SYS_PACKER_OPS = {
    .make = _PTREE_MOD_SYS_PackerMake,
};
static const PTREE_PACKER_Hook_t G_PTREE_SYS_PACKER_HOOK = {
    .free = _PTREE_MOD_SYS_PackerFree,
};

static const PTREE_LINKER_Ops_t G_PTREE_SYS_IN_LINKER_OPS = {
    .enqueue = _PTREE_MOD_SYS_InLinkerEnqueue,
    .dequeue = _PTREE_MOD_SYS_InLinkerDequeue,
};

static const PTREE_LINKER_Hook_t G_PTREE_SYS_IN_LINKER_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SYS_InLinkerFree,
};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_SYS_OUT_LINKER_OPS = {
    .enqueue           = _PTREE_MOD_SYS_OutLinkerEnqueue,
    .dequeueFromInside = _PTREE_MOD_SYS_OutLinkerDequeueFromInside,
    .dequeueOut        = _PTREE_MOD_SYS_OutLinkerDequeueOut,
};

static const PTREE_NLINKER_OUT_Hook_t G_PTREE_SYS_OUT_LINKER_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_SYS_OutLinkerFree,
};

static const PTREE_PACKET_ObjHook_t G_PTREE_SYS_IN_PACKET_HOOK = {
    .destruct = _PTREE_MOD_SYS_InRawPacketDestruct,
    .free     = _PTREE_MOD_SYS_InRawPacketFree,
};

static const PTREE_MMA_PACKET_RawPaOps_t G_PTREE_SYS_IN_MMA_PACKET_OPS = {
    .updateTimeStamp = _PTREE_MOD_SYS_InRawPacketUpdateTimeStamp,
};

static const PTREE_PACKET_ObjHook_t G_PTREE_SYS_OUT_PACKET_HOOK = {
    .destruct = _PTREE_MOD_SYS_OutRawPacketDestruct,
    .free     = _PTREE_MOD_SYS_OutRawPacketFree,
};

static const PTREE_MMA_PACKET_RawPaOps_t G_PTREE_SYS_OUT_MMA_PACKET_OPS = {
    .updateTimeStamp = _PTREE_MOD_SYS_OutRawPacketUpdateTimeStamp,
};

static unsigned int g_sysInitCount;

#ifdef CAM_OS_RTK
extern int MI_DEVICE_Init(void *pMmaConfig);
extern int mi_debug_init(void);
#endif

static void *_PTREE_MOD_SYS_Reader(SSOS_TASK_Buffer_t *taskBuf)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = taskBuf->buf;
    PTREE_PACKET_Obj_t *    packet    = NULL;
    MI_SYS_ChnPort_t        chnPort;
    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = sysModOut->thisSysMod->modId;
    chnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    chnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    chnPort.u32PortId = sysModOut->base.info->port;
    packet            = _PTREE_MOD_SYS_OutRawPacketNew(&chnPort, 10);
    if (packet)
    {
        PTREE_LINKER_Enqueue(&sysModOut->base.plinker.base, packet);
        PTREE_PACKET_Del(packet);
    }
    return NULL;
}

static int _PTREE_MOD_SYS_Init(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    int                  ret    = SSOS_DEF_OK;

    if (sysMod->ops->init)
    {
        ret = sysMod->ops->init(sysMod);
    }
    return ret;
}
static int _PTREE_MOD_SYS_Deinit(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    int                  ret    = SSOS_DEF_OK;

    if (sysMod->ops->deinit)
    {
        ret = sysMod->ops->deinit(sysMod);
    }
    return ret;
}
static int _PTREE_MOD_SYS_Prepare(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    int                  ret    = SSOS_DEF_OK;
    if (sysMod->ops->prepare)
    {
        ret = sysMod->ops->prepare(sysMod);
    }
    return ret;
}
static int _PTREE_MOD_SYS_Unprepare(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    int                  ret    = SSOS_DEF_OK;
    if (sysMod->ops->unprepare)
    {
        ret = sysMod->ops->unprepare(sysMod);
    }
    return ret;
}
static int _PTREE_MOD_SYS_Start(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    int                  ret    = SSOS_DEF_OK;
    if (sysMod->ops->start)
    {
        ret = sysMod->ops->start(sysMod);
    }
    return ret;
}
static int _PTREE_MOD_SYS_Stop(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    int                  ret    = SSOS_DEF_OK;
    if (sysMod->ops->stop)
    {
        ret = sysMod->ops->stop(sysMod);
    }
    return ret;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_SYS_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    if (sysMod->ops->createModIn)
    {
        return sysMod->ops->createModIn(mod, loopId);
    }
    return _PTREE_MOD_SYS_InDefaultNew(sysMod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_SYS_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    if (sysMod->ops->createModOut)
    {
        return sysMod->ops->createModOut(mod, loopId);
    }
    return _PTREE_MOD_SYS_OutDefaultNew(sysMod, loopId);
}
static void _PTREE_MOD_SYS_Destruct(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    if (sysMod->hook && sysMod->hook->destruct)
    {
        sysMod->hook->destruct(sysMod);
    }
    --g_sysInitCount;
    if (0 == g_sysInitCount)
    {
        MI_SYS_Exit(0);
        PTREE_DBG("MI_SYS_Exit\n");
    }
}
static void _PTREE_MOD_SYS_Free(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYS_Obj_t *sysMod = CONTAINER_OF(mod, PTREE_MOD_SYS_Obj_t, base);
    if (sysMod->hook && sysMod->hook->free)
    {
        sysMod->hook->free(sysMod);
    }
}

static int _PTREE_MOD_SYS_InStart(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    if (sysModIn->ops->start)
    {
        return sysModIn->ops->start(sysModIn);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_InStop(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    if (sysModIn->ops->stop)
    {
        return sysModIn->ops->stop(sysModIn);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_InLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    if (sysModIn->ops->linked)
    {
        return sysModIn->ops->linked(sysModIn, ref);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    if (sysModIn->ops->unlinked)
    {
        return sysModIn->ops->unlinked(sysModIn, ref);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_InDirectBind(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYS_InObj_t * sysModIn  = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modIn->prevModOut, PTREE_MOD_SYS_OutObj_t, base);
    PTREE_SUR_SYS_InInfo_t *sysInInfo = CONTAINER_OF(modIn->info, PTREE_SUR_SYS_InInfo_t, base);

    MI_S32 ret = MI_SUCCESS;

    MI_SYS_ChnPort_t srcChnPort;
    MI_SYS_ChnPort_t dstChnPort;

    MI_U32 srcFrmRate;
    MI_U32 dstFrmRate;

    MI_SYS_BindType_e bindType;
    MI_U32            bindParam;

    if (sysModIn->ops->directBind)
    {
        return sysModIn->ops->directBind(sysModIn);
    }

    memset(&srcChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&dstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));

    srcChnPort.eModId    = sysModOut->thisSysMod->modId;
    srcChnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    srcChnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    srcChnPort.u32PortId = sysModOut->base.info->port;

    dstChnPort.eModId    = sysModIn->thisSysMod->modId;
    dstChnPort.u32DevId  = sysModIn->base.thisMod->info->devId;
    dstChnPort.u32ChnId  = sysModIn->base.thisMod->info->chnId;
    dstChnPort.u32PortId = sysModIn->base.info->port;

    srcFrmRate = sysModOut->base.info->fps;
    dstFrmRate = sysModIn->base.info->fps;
    bindType   = sysInInfo->bindType;
    bindParam  = sysInInfo->bindParam;

    ret = MI_SYS_BindChnPort2(0, &srcChnPort, &dstChnPort, srcFrmRate, dstFrmRate, bindType, bindParam);
    if (ret != MI_SUCCESS)
    {
        PTREE_ERR("MI_SYS_BindChnPort2 ret %d, %d-%d-%d-%d@%d --[%d, %d]--> %d-%d-%d-%d@%d", ret, srcChnPort.eModId,
                  srcChnPort.u32DevId, srcChnPort.u32ChnId, srcChnPort.u32PortId, srcFrmRate, bindType, bindParam,
                  dstChnPort.eModId, dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId, dstFrmRate);
        return SSOS_DEF_FAIL;
    }

    PTREE_DBG("Bind %d-%d-%d-%d@%d --[%d, %d]--> %d-%d-%d-%d@%d", srcChnPort.eModId, srcChnPort.u32DevId,
              srcChnPort.u32ChnId, srcChnPort.u32PortId, srcFrmRate, bindType, bindParam, dstChnPort.eModId,
              dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId, dstFrmRate);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_InDirectUnbind(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYS_InObj_t * sysModIn  = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modIn->prevModOut, PTREE_MOD_SYS_OutObj_t, base);

    MI_S32 ret = MI_SUCCESS;

    MI_SYS_ChnPort_t srcChnPort;
    MI_SYS_ChnPort_t dstChnPort;

    if (sysModIn->ops->directUnbind)
    {
        return sysModIn->ops->directUnbind(sysModIn);
    }

    memset(&srcChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&dstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));

    srcChnPort.eModId    = sysModOut->thisSysMod->modId;
    srcChnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    srcChnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    srcChnPort.u32PortId = sysModOut->base.info->port;

    dstChnPort.eModId    = sysModIn->thisSysMod->modId;
    dstChnPort.u32DevId  = sysModIn->base.thisMod->info->devId;
    dstChnPort.u32ChnId  = sysModIn->base.thisMod->info->chnId;
    dstChnPort.u32PortId = sysModIn->base.info->port;

    ret = MI_SYS_UnBindChnPort(0, &srcChnPort, &dstChnPort);
    if (ret != MI_SUCCESS)
    {
        PTREE_ERR("MI_SYS_UnBindChnPort ret %d, %d-%d-%d-%d --> %d-%d-%d-%d", ret, srcChnPort.eModId,
                  srcChnPort.u32DevId, srcChnPort.u32ChnId, srcChnPort.u32PortId, dstChnPort.eModId,
                  dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId);
        return SSOS_DEF_FAIL;
    }

    PTREE_DBG("Unbind %d-%d-%d-%d -/ /-> %d-%d-%d-%d", srcChnPort.eModId, srcChnPort.u32DevId, srcChnPort.u32ChnId,
              srcChnPort.u32PortId, dstChnPort.eModId, dstChnPort.u32DevId, dstChnPort.u32ChnId, dstChnPort.u32PortId);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER | E_PTREE_MOD_BIND_TYPE_KERNEL;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYS_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    MI_SYS_ChnPort_t       chnPort;

    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = sysModIn->thisSysMod->modId;
    chnPort.u32DevId  = sysModIn->base.thisMod->info->devId;
    chnPort.u32ChnId  = sysModIn->base.thisMod->info->chnId;
    chnPort.u32PortId = sysModIn->base.info->port;

    return _PTREE_MOD_SYS_InLinkerNew(&chnPort);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_SYS_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    MI_SYS_ChnPort_t       chnPort;

    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = sysModIn->thisSysMod->modId;
    chnPort.u32DevId  = sysModIn->base.thisMod->info->devId;
    chnPort.u32ChnId  = sysModIn->base.thisMod->info->chnId;
    chnPort.u32PortId = sysModIn->base.info->port;

    *isFast = SSOS_DEF_TRUE;
    return _PTREE_MOD_SYS_PackerNew(&chnPort);
}
static void _PTREE_MOD_SYS_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    if (sysModIn->hook && sysModIn->hook->destruct)
    {
        sysModIn->hook->destruct(sysModIn);
    }
}
static void _PTREE_MOD_SYS_InFree(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = CONTAINER_OF(modIn, PTREE_MOD_SYS_InObj_t, base);
    if (sysModIn->hook && sysModIn->hook->free)
    {
        sysModIn->hook->free(sysModIn);
    }
}

static int _PTREE_MOD_SYS_OutStart(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t * sysModOut  = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    PTREE_SUR_SYS_OutInfo_t *sysOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_SYS_OutInfo_t, base);

    MI_SYS_ChnPort_t             chnPort;
    MI_SYS_FrameBufExtraConfig_t bufExtCfg;

    int ret = SSOS_DEF_OK;

    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = sysModOut->thisSysMod->modId;
    chnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    chnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    chnPort.u32PortId = sysModOut->base.info->port;

    if (sysOutInfo->depthEn)
    {
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &chnPort, sysOutInfo->depthUser, sysOutInfo->depthTotal))
        {
            PTREE_ERR("MI_SYS_SetChnOutputPortDepth Failed, mod: %d dev %d chn %d", chnPort.eModId, chnPort.u32DevId,
                      chnPort.u32ChnId);
            return SSOS_DEF_FAIL;
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
            return SSOS_DEF_FAIL;
        }
    }

    if (sysModOut->ops->start)
    {
        ret = sysModOut->ops->start(sysModOut);
        if (ret != SSOS_DEF_OK)
        {
            PTREE_ERR("start out failed");
            return ret;
        }
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_OutStop(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t * sysModOut  = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    PTREE_SUR_SYS_OutInfo_t *sysOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_SYS_OutInfo_t, base);

    int ret = SSOS_DEF_OK;

    if (sysModOut->ops->stop)
    {
        ret = sysModOut->ops->stop(sysModOut);
        if (ret != SSOS_DEF_OK)
        {
            PTREE_ERR("stop out failed");
            return SSOS_DEF_FAIL;
        }
    }

    if (sysOutInfo->depthEn)
    {
        MI_SYS_ChnPort_t chnPort;
        memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
        chnPort.eModId    = sysModOut->thisSysMod->modId;
        chnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
        chnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
        chnPort.u32PortId = sysModOut->base.info->port;
        if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &chnPort, 0, 1))
        {
            PTREE_ERR("MI_SYS_SetChnOutputPortDepth Failed, mod: %d dev %d chn %d", chnPort.eModId, chnPort.u32DevId,
                      chnPort.u32ChnId);
            return SSOS_DEF_FAIL;
        }
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER | E_PTREE_MOD_BIND_TYPE_KERNEL;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYS_OutCreateNLinker(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    MI_SYS_ChnPort_t        chnPort;

    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = sysModOut->thisSysMod->modId;
    chnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    chnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    chnPort.u32PortId = sysModOut->base.info->port;

    return _PTREE_MOD_SYS_OutLinkerNew(&chnPort);
}
static PTREE_PACKET_Info_t *_PTREE_MOD_SYS_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t *   sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    PTREE_PACKET_RAW_RawInfo_t rawInfo;
    if (!sysModOut->ops->getPacketInfo)
    {
        return NULL;
    }
    memset(&rawInfo, 0, sizeof(PTREE_PACKET_RAW_RawInfo_t));
    sysModOut->ops->getPacketInfo(sysModOut, &rawInfo);
    return PTREE_PACKET_RAW_InfoNew(&rawInfo);
}
static int _PTREE_MOD_SYS_OutLinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_SYS_OutObj_t * sysModOut  = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    PTREE_SUR_SYS_OutInfo_t *sysOutInfo = CONTAINER_OF(modOut->info, PTREE_SUR_SYS_OutInfo_t, base);
    MI_SYS_ChnPort_t         chnPort;
    SSOS_TASK_Attr_t         taskAttr;
    char                     taskName[PTREE_MOD_SYS_READER_TASK_NAME_LEN];
    unsigned int             userDepth = 3, totalDepth = 5;

    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }

    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = sysModOut->thisSysMod->modId;
    chnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    chnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    chnPort.u32PortId = sysModOut->base.info->port;

    if (sysOutInfo->depthEn)
    {
        userDepth  = sysOutInfo->depthUser;
        totalDepth = sysOutInfo->depthTotal;
    }
    if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &chnPort, userDepth, totalDepth))
    {
        PTREE_ERR("MI_SYS_SetChnOutputPortDepth Failed, mod: %d dev %d chn %d", chnPort.eModId, chnPort.u32DevId,
                  chnPort.u32ChnId);
        return SSOS_DEF_FAIL;
    }
    if (sysOutInfo->userFrc)
    {
        MI_SYS_SetChnOutputPortUserFrc(&chnPort, -1, sysOutInfo->base.fps);
    }

    if (PTREE_PLINKER_GROUP_Empty(&modOut->plinker) || modOut->nlinker)
    {
        return SSOS_DEF_OK;
    }
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    PTREE_MOD_OutKeyStr(modOut, taskName, PTREE_MOD_SYS_READER_TASK_NAME_LEN);
    taskAttr.doSignal         = NULL;
    taskAttr.doMonitor        = _PTREE_MOD_SYS_Reader;
    taskAttr.monitorCycleSec  = 0;
    taskAttr.monitorCycleNsec = 0;
    taskAttr.isResetTimer     = 0;
    taskAttr.inBuf.buf        = (void *)sysModOut;
    taskAttr.inBuf.size       = 0;
    taskAttr.threadAttr.name  = taskName;
    sysModOut->taskHandle     = SSOS_TASK_Open(&taskAttr);
    if (!sysModOut->taskHandle)
    {
        PTREE_ERR("Task create error!");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(sysModOut->taskHandle);
    PTREE_DBG("Sys reader %d start", sysModOut->base.info->port);
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_SYS_OutSuspend(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    if (!sysModOut->taskHandle)
    {
        PTREE_ERR("Task null!");
        return SSOS_DEF_FAIL;
    }
    PTREE_DBG("Sys reader %d suspend", sysModOut->base.info->port);
    return SSOS_TASK_Stop(sysModOut->taskHandle);
}

static int _PTREE_MOD_SYS_OutResume(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    if (!sysModOut->taskHandle)
    {
        PTREE_ERR("Task null!");
        return SSOS_DEF_FAIL;
    }
    PTREE_DBG("Sys reader %d resume", sysModOut->base.info->port);
    return SSOS_TASK_StartMonitor(sysModOut->taskHandle);
}

static int _PTREE_MOD_SYS_OutUnlinked(PTREE_MOD_OutObj_t *modOut, unsigned int ref)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    MI_SYS_ChnPort_t        chnPort;
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    if (!sysModOut->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    SSOS_TASK_Close(sysModOut->taskHandle);
    sysModOut->taskHandle = NULL;
    memset(&chnPort, 0, sizeof(MI_SYS_ChnPort_t));
    chnPort.eModId    = sysModOut->thisSysMod->modId;
    chnPort.u32DevId  = sysModOut->base.thisMod->info->devId;
    chnPort.u32ChnId  = sysModOut->base.thisMod->info->chnId;
    chnPort.u32PortId = sysModOut->base.info->port;
    if (MI_SUCCESS != MI_SYS_SetChnOutputPortDepth(0, &chnPort, 0, 1))
    {
        PTREE_ERR("MI_SYS_SetChnOutputPortDepth Failed, mod: %d dev %d chn %d", chnPort.eModId, chnPort.u32DevId,
                  chnPort.u32ChnId);
        return SSOS_DEF_FAIL;
    }
    PTREE_DBG("Sys reader %d stop", sysModOut->base.info->port);
    return 0;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_SYS_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_InObj_t * sysModIn  = NULL;
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);

    if (!sysModOut->ops->resetStreamTraverse)
    {
        return NULL;
    }
    sysModIn = sysModOut->ops->resetStreamTraverse(sysModOut);
    return &sysModIn->base;
}
static int _PTREE_MOD_SYS_ResetStreamOut(PTREE_MOD_OutObj_t *modOut, unsigned int width, unsigned int height)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    if (!sysModOut->ops->resetStreamOut)
    {
        return SSOS_DEF_FAIL;
    }
    return sysModOut->ops->resetStreamOut(sysModOut, width, height);
}
static void _PTREE_MOD_SYS_OutDestruct(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    if (sysModOut->hook && sysModOut->hook->destruct)
    {
        sysModOut->hook->destruct(sysModOut);
    }
}
static void _PTREE_MOD_SYS_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = CONTAINER_OF(modOut, PTREE_MOD_SYS_OutObj_t, base);
    if (sysModOut->hook && sysModOut->hook->free)
    {
        sysModOut->hook->free(sysModOut);
    }
}

static void _PTREE_MOD_SYS_InDefaultFree(PTREE_MOD_SYS_InObj_t *sysModIn)
{
    SSOS_MEM_Free(sysModIn);
}
static void _PTREE_MOD_SYS_OutDefaultFree(PTREE_MOD_SYS_OutObj_t *sysModOut)
{
    SSOS_MEM_Free(sysModOut);
}

static PTREE_MOD_InObj_t *_PTREE_MOD_SYS_InDefaultNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_SYS_InObj_t *sysModIn = NULL;

    sysModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYS_InObj_t));
    if (!sysModIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysModIn, 0, sizeof(PTREE_MOD_SYS_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_InObjInit(sysModIn, &G_PTREE_MOD_SYS_IN_DEFAULT_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(sysModIn);
        return NULL;
    }
    PTREE_MOD_SYS_InObjRegister(sysModIn, &G_PTREE_MOD_SYS_IN_DEFAULT_HOOK);
    return &sysModIn->base;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_SYS_OutDefaultNew(PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    PTREE_MOD_SYS_OutObj_t *sysModOut = NULL;

    sysModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYS_OutObj_t));
    if (!sysModOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysModOut, 0, sizeof(PTREE_MOD_SYS_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_OutObjInit(sysModOut, &G_PTREE_MOD_SYS_OUT_DEFAULT_OPS, sysMod, loopId))
    {
        SSOS_MEM_Free(sysModOut);
        return NULL;
    }
    PTREE_MOD_SYS_OutObjRegister(sysModOut, &G_PTREE_MOD_SYS_OUT_DEFAULT_HOOK);
    return &sysModOut->base;
}

static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info)
{
    PTREE_MOD_SYS_Packer_t *sysPacker = CONTAINER_OF(packer, PTREE_MOD_SYS_Packer_t, base);
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(raw_pa)))
    {
        PTREE_PACKET_RAW_Info_t *rawInfo = CONTAINER_OF(info, PTREE_PACKET_RAW_Info_t, base);
        return _PTREE_MOD_SYS_InRawPacketNew(rawInfo, &sysPacker->chnPort);
    }
    if (PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(raw)))
    {
        PTREE_PACKET_RAW_Info_t *rawInfo        = CONTAINER_OF(info, PTREE_PACKET_RAW_Info_t, base);
        PTREE_PACKET_Obj_t *     sysInRawPacket = _PTREE_MOD_SYS_InRawPacketNew(rawInfo, &sysPacker->chnPort);
        PTREE_PACKET_Obj_t *     rawVaPacket    = NULL;
        if (!sysInRawPacket)
        {
            return NULL;
        }
        rawVaPacket =
            PTREE_MMA_PACKET_RawVaNew(&CONTAINER_OF(sysInRawPacket, PTREE_MOD_SYS_InRawPacket_t, base.base)->base);
        PTREE_PACKET_Del(sysInRawPacket);
        return rawVaPacket;
    }
    PTREE_ERR("packet info type %s is not support\n", info->type);
    return NULL;
}

static void _PTREE_MOD_SYS_PackerFree(PTREE_PACKER_Obj_t *packer)
{
    PTREE_MOD_SYS_Packer_t *sysPacker = CONTAINER_OF(packer, PTREE_MOD_SYS_Packer_t, base);
    SSOS_MEM_Free(sysPacker);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_SYS_PackerNew(MI_SYS_ChnPort_t *chnPort)
{
    PTREE_MOD_SYS_Packer_t *sysPacker = NULL;

    sysPacker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYS_Packer_t));
    if (!sysPacker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysPacker, 0, sizeof(PTREE_MOD_SYS_Packer_t));

    if (SSOS_DEF_OK != PTREE_PACKER_Init(&sysPacker->base, &G_PTREE_SYS_PACKER_OPS))
    {
        SSOS_MEM_Free(sysPacker);
        return NULL;
    }

    sysPacker->chnPort = *chnPort;

    PTREE_PACKER_Register(&sysPacker->base, &G_PTREE_SYS_PACKER_HOOK);
    return &sysPacker->base;
}

static int _PTREE_MOD_SYS_InLinkerInjectBuf(PTREE_MOD_SYS_InLinker_t *sysInLinker, MI_SYS_BUF_HANDLE bufHandle)
{
    MI_SYS_BUF_HANDLE bufHandleDup = 0;
    /* Dup */
    if (MI_SUCCESS != MI_SYS_DupBuf(bufHandle, &bufHandleDup))
    {
        PTREE_ERR("MI_SYS_DupBuf Failed");
        return SSOS_DEF_FAIL;
    }
    /* Inject */
    if (MI_SUCCESS != MI_SYS_ChnPortInjectBuf(bufHandleDup, &sysInLinker->chnPort))
    {
        PTREE_ERR("MI_SYS_ChnPortInjectBuf Failed");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_InLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_InLinker_t *sysInLinker = CONTAINER_OF(linker, PTREE_MOD_SYS_InLinker_t, base);
    PTREE_PACKET_Obj_t *      rawPaPacket = NULL;
    int                       ret;

    rawPaPacket = PTREE_PACKET_Convert(packet, PTREE_PACKET_INFO_TYPE(raw_pa));
    if (!rawPaPacket)
    {
        PTREE_ERR("packet info type %s is not support", packet->info->type);
        return SSOS_DEF_EINVAL;
    }

    if (PTREE_PACKET_Likely(rawPaPacket, PTREE_PACKET_TYPE(sys_in)))
    {
        PTREE_MOD_SYS_InRawPacket_t *sysInRawPacket = CONTAINER_OF(rawPaPacket, PTREE_MOD_SYS_InRawPacket_t, base.base);
        ret = _PTREE_MOD_SYS_InLinkerInjectBuf(sysInLinker, sysInRawPacket->bufHandle);
    }
    else if (PTREE_PACKET_Likely(rawPaPacket, PTREE_PACKET_TYPE(sys_out)))
    {
        PTREE_MOD_SYS_OutRawPacket_t *sysOutRawPacket =
            CONTAINER_OF(rawPaPacket, PTREE_MOD_SYS_OutRawPacket_t, base.base);
        ret = _PTREE_MOD_SYS_InLinkerInjectBuf(sysInLinker, sysOutRawPacket->bufHandle);
    }
    else if (PTREE_PACKET_Likely(rawPaPacket, PTREE_PACKET_TYPE(sys_mma)))
    {
        PTREE_MMA_PACKET_RawPa_t *   rawPa          = CONTAINER_OF(rawPaPacket, PTREE_MMA_PACKET_RawPa_t, base);
        PTREE_MOD_SYS_InRawPacket_t *sysInRawPacket = CONTAINER_OF(
            _PTREE_MOD_SYS_InRawPacketNew(&rawPa->info, &sysInLinker->chnPort), PTREE_MOD_SYS_InRawPacket_t, base.base);
        PTREE_MMA_PACKET_RawPaCopy(&sysInRawPacket->base, rawPa);
        PTREE_PACKET_Del(&sysInRawPacket->base.base);
        ret = SSOS_DEF_OK;
    }
    else
    {
        PTREE_ERR("packet type %s is not support", packet->type);
        ret = SSOS_DEF_FAIL;
    }
    PTREE_PACKET_Del(rawPaPacket);
    return ret;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_InLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}
static void _PTREE_MOD_SYS_InLinkerFree(PTREE_LINKER_Obj_t *linker)
{
    PTREE_MOD_SYS_InLinker_t *sysInLinker = CONTAINER_OF(linker, PTREE_MOD_SYS_InLinker_t, base);
    SSOS_MEM_Free(sysInLinker);
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYS_InLinkerNew(MI_SYS_ChnPort_t *chnPort)
{
    PTREE_MOD_SYS_InLinker_t *sysInLinker = NULL;

    sysInLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYS_InLinker_t));
    if (!sysInLinker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysInLinker, 0, sizeof(PTREE_MOD_SYS_InLinker_t));

    if (SSOS_DEF_OK != PTREE_LINKER_Init(&sysInLinker->base, &G_PTREE_SYS_IN_LINKER_OPS))
    {
        SSOS_MEM_Free(sysInLinker);
        return NULL;
    }

    sysInLinker->chnPort = *chnPort;

    PTREE_LINKER_Register(&sysInLinker->base, &G_PTREE_SYS_IN_LINKER_HOOK);
    return &sysInLinker->base;
}

static int _PTREE_MOD_SYS_OutLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_OutLinker_t *sysOutLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_SYS_OutLinker_t, base);
    PTREE_DMA_PACKET_Obj_t *   dmaPacket    = NULL;
    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(sys_dma)))
    {
        PTREE_ERR("packet info %s is not support", packet->info->type);
        return SSOS_DEF_FAIL;
    }
    dmaPacket = CONTAINER_OF(packet, PTREE_DMA_PACKET_Obj_t, base);
    if (MI_SUCCESS != MI_SYS_ChnOutputPortEnqueueDmabuf(&sysOutLinker->chnPort, &dmaPacket->info.dmaBufInfo))
    {
        PTREE_ERR("MI_SYS_ChnOutputPortEnqueueDmabu failed");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYS_OutLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_OutLinker_t *sysOutLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_SYS_OutLinker_t, base);
    PTREE_DMA_PACKET_Obj_t *   dmaPacket    = NULL;
    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(sys_dma)))
    {
        PTREE_ERR("packet info %s is not support", packet->info->type);
        return SSOS_DEF_FAIL;
    }
    dmaPacket = CONTAINER_OF(packet, PTREE_DMA_PACKET_Obj_t, base);
    if (MI_SUCCESS != MI_SYS_ChnOutputPortDequeueDmabuf(&sysOutLinker->chnPort, &dmaPacket->info.dmaBufInfo))
    {
        PTREE_ERR("MI_SYS_ChnOutputPortDequeueDmabuf failed");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_OutLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t *nlinkerOut, int ms)
{
    PTREE_MOD_SYS_OutLinker_t *sysOutLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_SYS_OutLinker_t, base);
    return _PTREE_MOD_SYS_OutRawPacketNew(&sysOutLinker->chnPort, ms);
}
static void _PTREE_MOD_SYS_OutLinkerFree(PTREE_NLINKER_OUT_Obj_t *nlinkerOut)
{
    PTREE_MOD_SYS_OutLinker_t *sysOutLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_SYS_OutLinker_t, base);
    SSOS_MEM_Free(sysOutLinker);
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYS_OutLinkerNew(MI_SYS_ChnPort_t *chnPort)
{
    PTREE_MOD_SYS_OutLinker_t *sysOutLinker = NULL;

    sysOutLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYS_OutLinker_t));
    if (!sysOutLinker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(sysOutLinker, 0, sizeof(PTREE_MOD_SYS_OutLinker_t));

    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&sysOutLinker->base, &G_PTREE_SYS_OUT_LINKER_OPS))
    {
        SSOS_MEM_Free(sysOutLinker);
        return NULL;
    }

    sysOutLinker->chnPort = *chnPort;

    PTREE_NLINKER_OUT_Register(&sysOutLinker->base, &G_PTREE_SYS_OUT_LINKER_HOOK);
    return &sysOutLinker->base.base;
}

static void _PTREE_MOD_SYS_InRawPacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_InRawPacket_t *sysInRawPacket = CONTAINER_OF(packet, PTREE_MOD_SYS_InRawPacket_t, base.base);
    PTREE_PACKET_TimeStamp_t     stamp;
    stamp.tvSec  = sysInRawPacket->bufInfo.u64Pts / 1000000;
    stamp.tvUSec = sysInRawPacket->bufInfo.u64Pts % 1000000;
    PTREE_PACKET_SetTimeStamp(packet, &stamp);
}

static void _PTREE_MOD_SYS_InRawPacketDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_InRawPacket_t *sysInRawPacket = CONTAINER_OF(packet, PTREE_MOD_SYS_InRawPacket_t, base.base);
    if (MI_SUCCESS != MI_SYS_ChnInputPortPutBufPa(sysInRawPacket->bufHandle, &sysInRawPacket->bufInfo, TRUE))
    {
        PTREE_ERR("MI_SYS_ChnInputPortPutBuf Failed");
    }
}
static void _PTREE_MOD_SYS_InRawPacketFree(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_InRawPacket_t *sysInRawPacket = CONTAINER_OF(packet, PTREE_MOD_SYS_InRawPacket_t, base.base);
    SSOS_MEM_Free(sysInRawPacket);
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_InRawPacketNew(const PTREE_PACKET_RAW_Info_t *rawInfo,
                                                         MI_SYS_ChnPort_t *             chnPort)
{
    PTREE_MOD_SYS_InRawPacket_t *sysInRawPacket = NULL;
    MI_SYS_BufConf_t             bufConf;
    SSOS_TIME_Spec_t             stamp;

    sysInRawPacket = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYS_InRawPacket_t));
    if (!sysInRawPacket)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(sysInRawPacket, 0, sizeof(PTREE_MOD_SYS_InRawPacket_t));

    if (SSOS_DEF_OK
        != PTREE_MMA_PACKET_RawPaInit(&sysInRawPacket->base, &G_PTREE_SYS_IN_MMA_PACKET_OPS, PTREE_PACKET_TYPE(sys_in)))
    {
        goto ERR_RAW_INIT;
    }

    memset(&sysInRawPacket->bufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&bufConf, 0, sizeof(MI_SYS_BufConf_t));
    SSOS_TIME_Get(&stamp);
    bufConf.u64TargetPts              = stamp.tvSec * 1000000 + stamp.tvNSec / 1000;
    bufConf.eBufType                  = E_MI_SYS_BUFDATA_FRAME;
    bufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    bufConf.stFrameCfg.eCompressMode  = E_MI_SYS_COMPRESS_MODE_NONE;
    bufConf.stFrameCfg.eFormat        = PTREE_MOD_SYS_PtreeFmtToSysFmt(rawInfo->rawInfo.fmt);
    bufConf.stFrameCfg.u16Width       = rawInfo->rawInfo.width;
    bufConf.stFrameCfg.u16Height      = rawInfo->rawInfo.height;
    if (MI_SUCCESS
        != MI_SYS_ChnInputPortGetBufPa(chnPort, &bufConf, &sysInRawPacket->bufInfo, &sysInRawPacket->bufHandle, 0))
    {
        PTREE_ERR("MI_SYS_ChnInputPortGetBufPa Failed");
        goto ERR_GET_BUF;
    }
    sysInRawPacket->base.info.rawInfo      = rawInfo->rawInfo;
    sysInRawPacket->base.rawData.stride[0] = sysInRawPacket->bufInfo.stFrameData.u32Stride[0];
    sysInRawPacket->base.rawData.phy[0]    = sysInRawPacket->bufInfo.stFrameData.phyAddr[0];
    sysInRawPacket->base.rawData.stride[1] = sysInRawPacket->bufInfo.stFrameData.u32Stride[1];
    sysInRawPacket->base.rawData.phy[1]    = sysInRawPacket->bufInfo.stFrameData.phyAddr[1];
    PTREE_PACKET_RAW_NormalSize(&sysInRawPacket->base.info.rawInfo, sysInRawPacket->base.rawData.stride,
                                sysInRawPacket->base.rawData.size);

    PTREE_PACKET_Register(&sysInRawPacket->base.base, &G_PTREE_SYS_IN_PACKET_HOOK);
    return &sysInRawPacket->base.base;

ERR_GET_BUF:
    PTREE_PACKET_Del(&sysInRawPacket->base.base);
ERR_RAW_INIT:
    SSOS_MEM_Free(sysInRawPacket);
ERR_MEM_ALLOC:
    return NULL;
}

static void _PTREE_MOD_SYS_OutRawPacketUpdateTimeStamp(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_OutRawPacket_t *sysOutRawPacket = CONTAINER_OF(packet, PTREE_MOD_SYS_OutRawPacket_t, base.base);
    PTREE_PACKET_TimeStamp_t      stamp;
    stamp.tvSec  = sysOutRawPacket->bufInfo.u64Pts / 1000000;
    stamp.tvUSec = sysOutRawPacket->bufInfo.u64Pts % 1000000;
    PTREE_PACKET_SetTimeStamp(packet, &stamp);
}

static void _PTREE_MOD_SYS_OutRawPacketDestruct(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_OutRawPacket_t *sysOutRawPacket = CONTAINER_OF(packet, PTREE_MOD_SYS_OutRawPacket_t, base.base);
    if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBufPa(sysOutRawPacket->bufHandle))
    {
        PTREE_ERR("MI_SYS_ChnOutputPortPutBufPa Failed");
    }
}
static void _PTREE_MOD_SYS_OutRawPacketFree(PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_SYS_OutRawPacket_t *sysOutRawPacket = CONTAINER_OF(packet, PTREE_MOD_SYS_OutRawPacket_t, base.base);
    SSOS_MEM_Free(sysOutRawPacket);
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_SYS_OutRawPacketNew(MI_SYS_ChnPort_t *chnPort, int ms)
{
    PTREE_MOD_SYS_OutRawPacket_t *sysOutRawPacket = NULL;
    struct pollfd                 pollFd;
    int                           readyNum = 0;

    sysOutRawPacket = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYS_OutRawPacket_t));
    if (!sysOutRawPacket)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(sysOutRawPacket, 0, sizeof(PTREE_MOD_SYS_OutRawPacket_t));

    if (SSOS_DEF_OK
        != PTREE_MMA_PACKET_RawPaInit(&sysOutRawPacket->base, &G_PTREE_SYS_OUT_MMA_PACKET_OPS,
                                      PTREE_PACKET_TYPE(sys_out)))
    {
        goto ERR_RAW_INIT;
    }

    memset(&pollFd, 0, sizeof(struct pollfd));

    if (MI_SUCCESS != MI_SYS_GetFd(chnPort, &pollFd.fd))
    {
        PTREE_ERR("MI_SYS_GetFd Failed");
        goto ERR_GET_FD;
    }

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

    memset(&sysOutRawPacket->bufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    if (MI_SUCCESS != MI_SYS_ChnOutputPortGetBufPa(chnPort, &sysOutRawPacket->bufInfo, &sysOutRawPacket->bufHandle))
    {
        PTREE_ERR("MI_SYS_ChnOutputPortGetBufPa failed");
        goto ERR_GET_BUF;
    }

    MI_SYS_CloseFd(pollFd.fd);

    sysOutRawPacket->base.info.rawInfo.fmt =
        PTREE_MOD_SYS_SysFmtToPtreeFmt(sysOutRawPacket->bufInfo.stFrameData.ePixelFormat);
    sysOutRawPacket->base.info.rawInfo.width  = sysOutRawPacket->bufInfo.stFrameData.u16Width;
    sysOutRawPacket->base.info.rawInfo.height = sysOutRawPacket->bufInfo.stFrameData.u16Height;
    sysOutRawPacket->base.rawData.stride[0]   = sysOutRawPacket->bufInfo.stFrameData.u32Stride[0];
    sysOutRawPacket->base.rawData.phy[0]      = sysOutRawPacket->bufInfo.stFrameData.phyAddr[0];
    sysOutRawPacket->base.rawData.stride[1]   = sysOutRawPacket->bufInfo.stFrameData.u32Stride[1];
    sysOutRawPacket->base.rawData.phy[1]      = sysOutRawPacket->bufInfo.stFrameData.phyAddr[1];
    PTREE_PACKET_RAW_NormalSize(&sysOutRawPacket->base.info.rawInfo, sysOutRawPacket->base.rawData.stride,
                                sysOutRawPacket->base.rawData.size);

    PTREE_PACKET_Register(&sysOutRawPacket->base.base, &G_PTREE_SYS_OUT_PACKET_HOOK);
    return &sysOutRawPacket->base.base;

ERR_GET_BUF:
ERR_POLL_FD:
    MI_SYS_CloseFd(pollFd.fd);
ERR_GET_FD:
    PTREE_PACKET_Del(&sysOutRawPacket->base.base);
ERR_RAW_INIT:
    SSOS_MEM_Free(sysOutRawPacket);
ERR_MEM_ALLOC:
    return NULL;
}

int PTREE_MOD_SYS_ObjInit(PTREE_MOD_SYS_Obj_t *sysMod, const PTREE_MOD_SYS_Ops_t *ops, PARENA_Tag_t *tag,
                          MI_ModuleId_e modId)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(sysMod, return SSOS_DEF_EINVAL);
    CHECK_POINTER(ops, return SSOS_DEF_EINVAL);
    CHECK_POINTER(tag, return SSOS_DEF_EINVAL);

    sysMod->ops   = ops;
    sysMod->modId = modId;

    ret = PTREE_MOD_ObjInit(&sysMod->base, &G_PTREE_MOD_SYS_OPS, tag);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_MOD_ObjInit failed");
        return ret;
    }

    PTREE_MOD_ObjRegister(&sysMod->base, &G_PTREE_MOD_SYS_HOOK);
    if (0 == g_sysInitCount)
    {
#ifdef CAM_OS_RTK
        MI_DEVICE_Init(NULL); // mount misc partion
#endif
        // all scenes need to do sys init
        MI_SYS_Init(0);
        PTREE_DBG("MI_SYS_Init\n");

#ifdef CAM_OS_RTK
        mi_debug_init();
        BootTimestampRecord(__LINE__, "PTREE_SysReady");
#endif
    }
    ++g_sysInitCount;
    return ret;
}
int PTREE_MOD_SYS_InObjInit(PTREE_MOD_SYS_InObj_t *sysModIn, const PTREE_MOD_SYS_InOps_t *ops,
                            PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(sysModIn, return SSOS_DEF_EINVAL);
    CHECK_POINTER(ops, return SSOS_DEF_EINVAL);
    CHECK_POINTER(sysMod, return SSOS_DEF_EINVAL);
    ret = PTREE_MOD_InObjInit(&sysModIn->base, &G_PTREE_MOD_SYS_IN_OPS, &sysMod->base, loopId);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_MOD_InObjInit failed");
        return ret;
    }

    sysModIn->ops        = ops;
    sysModIn->thisSysMod = sysMod;

    PTREE_MOD_InObjRegister(&sysModIn->base, &G_PTREE_MOD_SYS_IN_HOOK);
    return ret;
}
int PTREE_MOD_SYS_OutObjInit(PTREE_MOD_SYS_OutObj_t *sysModOut, const PTREE_MOD_SYS_OutOps_t *ops,
                             PTREE_MOD_SYS_Obj_t *sysMod, unsigned int loopId)
{
    int ret = SSOS_DEF_OK;
    CHECK_POINTER(sysModOut, return SSOS_DEF_EINVAL);
    CHECK_POINTER(ops, return SSOS_DEF_EINVAL);
    CHECK_POINTER(sysMod, return SSOS_DEF_EINVAL);

    ret = PTREE_MOD_OutObjInit(&sysModOut->base, &G_PTREE_MOD_SYS_OUT_OPS, &sysMod->base, loopId);
    if (ret != SSOS_DEF_OK)
    {
        PTREE_ERR("PTREE_MOD_OutObjInit failed");
        return ret;
    }

    sysModOut->ops        = ops;
    sysModOut->taskHandle = NULL;
    sysModOut->thisSysMod = sysMod;

    PTREE_MOD_OutObjRegister(&sysModOut->base, &G_PTREE_MOD_SYS_OUT_HOOK);
    return ret;
}
void PTREE_MOD_SYS_ObjRegister(PTREE_MOD_SYS_Obj_t *sysMod, const PTREE_MOD_SYS_Hook_t *hook)
{
    CHECK_POINTER(sysMod, return );
    sysMod->hook = hook;
}
void PTREE_MOD_SYS_InObjRegister(PTREE_MOD_SYS_InObj_t *sysModIn, const PTREE_MOD_SYS_InHook_t *hook)
{
    CHECK_POINTER(sysModIn, return );
    sysModIn->hook = hook;
}
void PTREE_MOD_SYS_OutObjRegister(PTREE_MOD_SYS_OutObj_t *sysModOut, const PTREE_MOD_SYS_OutHook_t *hook)
{
    CHECK_POINTER(sysModOut, return );
    sysModOut->hook = hook;
}
enum PTREE_PACKET_RAW_VideoFmt_e PTREE_MOD_SYS_SysFmtToPtreeFmt(MI_SYS_PixelFormat_e fmt)
{
    switch (fmt)
    {
        case E_MI_SYS_PIXEL_FRAME_YUV422_YUYV:
            return E_PTREE_PACKET_RAW_FORMAT_YUV422_YUYV;
        case E_MI_SYS_PIXEL_FRAME_YUV422_UYVY:
            return E_PTREE_PACKET_RAW_FORMAT_YUV422_UYVY;
        case E_MI_SYS_PIXEL_FRAME_YUV422_YVYU:
            return E_PTREE_PACKET_RAW_FORMAT_YUV422_YVYU;
        case E_MI_SYS_PIXEL_FRAME_YUV422_VYUY:
            return E_PTREE_PACKET_RAW_FORMAT_YUV422_VYUY;
        case E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR:
            return E_PTREE_PACKET_RAW_FORMAT_YUV422SP;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420:
            return E_PTREE_PACKET_RAW_FORMAT_YUV420SP;
        case E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21:
            return E_PTREE_PACKET_RAW_FORMAT_YUV420SP_NV21;
        case E_MI_SYS_PIXEL_FRAME_RGB888:
            return E_PTREE_PACKET_RAW_FORMAT_RGB888;
        case E_MI_SYS_PIXEL_FRAME_BGR888:
            return E_PTREE_PACKET_RAW_FORMAT_BGR888;
        case E_MI_SYS_PIXEL_FRAME_ARGB8888:
            return E_PTREE_PACKET_RAW_FORMAT_ARGB8888;
        case E_MI_SYS_PIXEL_FRAME_ABGR8888:
            return E_PTREE_PACKET_RAW_FORMAT_ABGR8888;
        case E_MI_SYS_PIXEL_FRAME_BGRA8888:
            return E_PTREE_PACKET_RAW_FORMAT_BGRA8888;
        case E_MI_SYS_PIXEL_FRAME_RGB565:
            return E_PTREE_PACKET_RAW_FORMAT_RGB565;
        case E_MI_SYS_PIXEL_FRAME_ARGB1555:
            return E_PTREE_PACKET_RAW_FORMAT_ARGB1555;
        case E_MI_SYS_PIXEL_FRAME_ARGB4444:
            return E_PTREE_PACKET_RAW_FORMAT_ARGB4444;
        case E_MI_SYS_PIXEL_FRAME_I2:
            return E_PTREE_PACKET_RAW_FORMAT_I2;
        case E_MI_SYS_PIXEL_FRAME_I4:
            return E_PTREE_PACKET_RAW_FORMAT_I4;
        case E_MI_SYS_PIXEL_FRAME_I8:
            return E_PTREE_PACKET_RAW_FORMAT_I8;
        case E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE ... E_MI_SYS_PIXEL_FRAME_RGB_BAYER_NUM:
            return (enum PTREE_PACKET_RAW_VideoFmt_e)(E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE + fmt
                                                      - E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE);
        default:
            break;
    }
    return E_PTREE_PACKET_RAW_FORMAT_MAX;
}
MI_SYS_PixelFormat_e PTREE_MOD_SYS_PtreeFmtToSysFmt(enum PTREE_PACKET_RAW_VideoFmt_e fmt)
{
    switch (fmt)
    {
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_YUYV:
            return E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_UYVY:
            return E_MI_SYS_PIXEL_FRAME_YUV422_UYVY;
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_YVYU:
            return E_MI_SYS_PIXEL_FRAME_YUV422_YVYU;
        case E_PTREE_PACKET_RAW_FORMAT_YUV422_VYUY:
            return E_MI_SYS_PIXEL_FRAME_YUV422_VYUY;
        case E_PTREE_PACKET_RAW_FORMAT_YUV422SP:
            return E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR;
        case E_PTREE_PACKET_RAW_FORMAT_YUV420SP:
            return E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        case E_PTREE_PACKET_RAW_FORMAT_YUV420SP_NV21:
            return E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21;
        case E_PTREE_PACKET_RAW_FORMAT_RGB888:
            return E_MI_SYS_PIXEL_FRAME_RGB888;
        case E_PTREE_PACKET_RAW_FORMAT_BGR888:
            return E_MI_SYS_PIXEL_FRAME_BGR888;
        case E_PTREE_PACKET_RAW_FORMAT_ARGB8888:
            return E_MI_SYS_PIXEL_FRAME_ARGB8888;
        case E_PTREE_PACKET_RAW_FORMAT_ABGR8888:
            return E_MI_SYS_PIXEL_FRAME_ABGR8888;
        case E_PTREE_PACKET_RAW_FORMAT_BGRA8888:
            return E_MI_SYS_PIXEL_FRAME_BGRA8888;
        case E_PTREE_PACKET_RAW_FORMAT_RGB565:
            return E_MI_SYS_PIXEL_FRAME_RGB565;
        case E_PTREE_PACKET_RAW_FORMAT_ARGB1555:
            return E_MI_SYS_PIXEL_FRAME_ARGB1555;
        case E_PTREE_PACKET_RAW_FORMAT_ARGB4444:
            return E_MI_SYS_PIXEL_FRAME_ARGB4444;
        case E_PTREE_PACKET_RAW_FORMAT_I2:
            return E_MI_SYS_PIXEL_FRAME_I2;
        case E_PTREE_PACKET_RAW_FORMAT_I4:
            return E_MI_SYS_PIXEL_FRAME_I4;
        case E_PTREE_PACKET_RAW_FORMAT_I8:
            return E_MI_SYS_PIXEL_FRAME_I8;
        case E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE ... E_PTREE_PACKET_RAW_FORMAT_BAYER_NUM:
            return (MI_SYS_PixelFormat_e)(E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE + fmt
                                          - E_PTREE_PACKET_RAW_FORMAT_BAYER_BASE);
        default:
            break;
    }
    return E_MI_SYS_PIXEL_FRAME_FORMAT_MAX;
}
