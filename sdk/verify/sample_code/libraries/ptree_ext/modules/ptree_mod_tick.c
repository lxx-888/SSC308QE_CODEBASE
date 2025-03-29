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

#include "ssos_def.h"
#include "ssos_mem.h"
#include "ptree_message.h"
#include "ptree_nlinker_async.h"
#include "ssos_time.h"
#include "ssos_io.h"
#include "ssos_list.h"
#include "ptree_mod.h"
#include "ptree_log.h"
#include "ssos_task.h"
#include "ptree_sur_tick.h"
#include "ptree_packet.h"
#include "ptree_linker.h"
#include "ptree_linker_bypass.h"
#include "ptree_nlinker_out.h"
#include "ptree_packer.h"
#include "ptree_packer_bypass.h"
#include "ptree_maker.h"

#define PTREE_MOD_TICK_ADD_TIMER(__dst_time, __diff_time) \
    do                                                    \
    {                                                     \
        __dst_time.tvSec += __diff_time.tvSec;            \
        __dst_time.tvNSec += __diff_time.tvNSec;          \
        if (__dst_time.tvNSec > 1000000000)               \
        {                                                 \
            __dst_time.tvNSec %= 1000000000;              \
            __dst_time.tvSec++;                           \
        }                                                 \
    } while (0)

enum PTREE_MOD_TICK_FrcType_e
{
    E_PTREE_MOD_TICK_FRC_TYPE_SYNC,
    E_PTREE_MOD_TICK_FRC_TYPE_ASYNC,
    E_PTREE_MOD_TICK_FRC_TYPE_RATIO
};

typedef struct PTREE_MOD_TICK_FrcObject_s
{
    enum PTREE_MOD_TICK_FrcType_e frcType;
    SSOS_TIME_Spec_t              startTime;
    SSOS_TIME_Spec_t              diffTime;
    SSOS_THREAD_Mutex_t           frcLock;
    SSOS_THREAD_Cond_t            frcCond;
    unsigned short                srcFps;
    unsigned short                dstFps;
    unsigned short                frameStep;
    int                           frameCntLimit;
} PTREE_MOD_TICK_FrcObject_t;

typedef struct PTREE_MOD_TICK_Obj_s
{
    PTREE_MOD_Obj_t            base;
    PTREE_MOD_TICK_FrcObject_t frcObject;
} PTREE_MOD_TICK_Obj_t;

/* Tick input linker. */
typedef struct PTREE_MOD_TICK_InputLinker_s
{
    PTREE_LINKER_Obj_t          base;
    PTREE_LINKER_Obj_t*         holdLinker;
    PTREE_MOD_TICK_FrcObject_t* frcObj;
} PTREE_MOD_TICK_InputLinker_t;

/* Tick input async linker */
typedef struct PTREE_MOD_TICK_InputAsyncLinker_s
{
    PTREE_NLINKER_ASYNC_Obj_t base;
    PTREE_LINKER_Obj_t*       holdLinker;
} PTREE_MOD_TICK_InputAsyncLinker_t;

/* Tick output linker */
typedef struct PTREE_MOD_TICK_OutputLinker_s
{
    PTREE_LINKER_Obj_t          base;
    PTREE_LINKER_Obj_t*         holdLinker;
    PTREE_MOD_TICK_FrcObject_t* frcObj;
} PTREE_MOD_TICK_OutputLinker_t;

/* Tick output fifo linker */
typedef struct PTREE_MOD_TICK_OutputFifoLinker_s
{
    PTREE_NLINKER_OUT_Obj_t     base;
    PTREE_NLINKER_ASYNC_Obj_t*  holdLinker;
    PTREE_MOD_TICK_FrcObject_t* frcObj;
} PTREE_MOD_TICK_OutputFifoLinker_t;

static unsigned char _PTREE_MOD_TICK_FrcControl(PTREE_MOD_TICK_FrcObject_t* frcObj);

static int                 _PTREE_MOD_TICK_InputLinkerEnqueue(PTREE_LINKER_Obj_t* linker, PTREE_PACKET_Obj_t* packet);
static PTREE_PACKET_Obj_t* _PTREE_MOD_TICK_InputLinkerDequeue(PTREE_LINKER_Obj_t* linker, int ms);
static void                _PTREE_MOD_TICK_InputLinkerDestruct(PTREE_LINKER_Obj_t* linker);
static void                _PTREE_MOD_TICK_InputLinkerFree(PTREE_LINKER_Obj_t* linker);

static void _PTREE_MOD_TICK_InputAsyncLinkerDestruct(PTREE_NLINKER_ASYNC_Obj_t* nlinkerAsync);
static void _PTREE_MOD_TICK_InputAsyncLinkerFree(PTREE_NLINKER_ASYNC_Obj_t* nlinkerAsync);

static int                 _PTREE_MOD_TICK_OutputLinkerEnqueue(PTREE_LINKER_Obj_t* linker, PTREE_PACKET_Obj_t* packet);
static PTREE_PACKET_Obj_t* _PTREE_MOD_TICK_OutputLinkerDequeue(PTREE_LINKER_Obj_t* linker, int ms);
static void                _PTREE_MOD_TICK_OutputLinkerDestruct(PTREE_LINKER_Obj_t* linker);
static void                _PTREE_MOD_TICK_OutputLinkerFree(PTREE_LINKER_Obj_t* linker);

static int _PTREE_MOD_TICK_OutputFifoLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t* nlinkerOut, PTREE_PACKET_Obj_t* packet);
static PTREE_PACKET_Obj_t* _PTREE_MOD_TICK_OutputFifoLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t* nlinkerOut,
                                                                             int                      ms);
static int  _PTREE_MOD_TICK_OutputFifoLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t* nlinkerOut, PTREE_PACKET_Obj_t* packet);
static void _PTREE_MOD_TICK_OutputFifoLinkerDestruct(PTREE_NLINKER_OUT_Obj_t* nlinkerOut);
static void _PTREE_MOD_TICK_OutputFifoLinkerFree(PTREE_NLINKER_OUT_Obj_t* nlinkerOut);

static int                 _PTREE_MOD_TICK_InLinked(PTREE_MOD_InObj_t* modIn, unsigned int ref);
static int                 _PTREE_MOD_TICK_InGetType(PTREE_MOD_InObj_t* modIn);
static int                 _PTREE_MOD_TICK_InIsPostReader(PTREE_MOD_InObj_t* modIn);
static PTREE_LINKER_Obj_t* _PTREE_MOD_TICK_InCreateNLinker(PTREE_MOD_InObj_t* modIn);
static PTREE_PACKER_Obj_t* _PTREE_MOD_TICK_InCreatePacker(PTREE_MOD_InObj_t* modIn, int* isFast);
static void                _PTREE_MOD_TICK_InFree(PTREE_MOD_InObj_t* modIn);

static int                  _PTREE_MOD_TICK_OutGetType(PTREE_MOD_OutObj_t* modOut);
static PTREE_LINKER_Obj_t*  _PTREE_MOD_TICK_OutCreateNLinker(PTREE_MOD_OutObj_t* modOut);
static PTREE_PACKET_Info_t* _PTREE_MOD_TICK_OutGetPacketInfo(PTREE_MOD_OutObj_t* modOut);
static int                  _PTREE_MOD_TICK_OutLinkedTransfer(PTREE_MOD_OutObj_t* modOut);
static int                  _PTREE_MOD_TICK_OutUnlinkedTransfer(PTREE_MOD_OutObj_t* modOut);
static int                  _PTREE_MOD_TICK_OutSuspendTransfer(PTREE_MOD_OutObj_t* modOut);
static int                  _PTREE_MOD_TICK_OutResumeTransfer(PTREE_MOD_OutObj_t* modOut);
static void                 _PTREE_MOD_TICK_OutFree(PTREE_MOD_OutObj_t* modOut);

static int                 _PTREE_MOD_TICK_Init(PTREE_MOD_Obj_t* mod);
static int                 _PTREE_MOD_TICK_Deinit(PTREE_MOD_Obj_t* mod);
static PTREE_MOD_InObj_t*  _PTREE_MOD_TICK_CreateModIn(PTREE_MOD_Obj_t* mod, unsigned int loopId);
static PTREE_MOD_OutObj_t* _PTREE_MOD_TICK_CreateModOut(PTREE_MOD_Obj_t* mod, unsigned int loopId);
static void                _PTREE_MOD_TICK_Destruct(PTREE_MOD_Obj_t* mod);
static void                _PTREE_MOD_TICK_Free(PTREE_MOD_Obj_t* mod);

static const PTREE_LINKER_Ops_t G_PTREE_MOD_TICK_INPUT_LINKER_OPS = {
    .enqueue = _PTREE_MOD_TICK_InputLinkerEnqueue,
    .dequeue = _PTREE_MOD_TICK_InputLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_TICK_INPUT_LINKER_HOOK = {
    .destruct = _PTREE_MOD_TICK_InputLinkerDestruct,
    .free     = _PTREE_MOD_TICK_InputLinkerFree,
};

static const PTREE_NLINKER_ASYNC_Hook_t G_PTREE_MOD_TICK_INPUT_ASYNC_LINKER_HOOK = {
    .destruct = _PTREE_MOD_TICK_InputAsyncLinkerDestruct,
    .free     = _PTREE_MOD_TICK_InputAsyncLinkerFree,
};

static const PTREE_LINKER_Ops_t G_PTREE_MOD_TICK_OUTPUT_LINKER_OPS = {
    .enqueue = _PTREE_MOD_TICK_OutputLinkerEnqueue,
    .dequeue = _PTREE_MOD_TICK_OutputLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_TICK_OUTPUT_LINKER_HOOK = {
    .destruct = _PTREE_MOD_TICK_OutputLinkerDestruct,
    .free     = _PTREE_MOD_TICK_OutputLinkerFree,
};

static const PTREE_NLINKER_OUT_Ops_t G_PTREE_MOD_TICK_OUTPUT_FIFO_LINKER_OPS = {
    .enqueue           = _PTREE_MOD_TICK_OutputFifoLinkerEnqueue,
    .dequeueOut        = _PTREE_MOD_TICK_OutputFifoLinkerDequeueOut,
    .dequeueFromInside = _PTREE_MOD_TICK_OutputFifoLinkerDequeueFromInside,
};
static const PTREE_NLINKER_OUT_Hook_t G_PTREE_MOD_TICK_OUTPUT_FIFO_LINKER_HOOK = {
    .destruct = _PTREE_MOD_TICK_OutputFifoLinkerDestruct,
    .free     = _PTREE_MOD_TICK_OutputFifoLinkerFree,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_TICK_INPUT_OPS = {
    .linked        = _PTREE_MOD_TICK_InLinked,
    .getType       = _PTREE_MOD_TICK_InGetType,
    .isPostReader  = _PTREE_MOD_TICK_InIsPostReader,
    .createNLinker = _PTREE_MOD_TICK_InCreateNLinker,
    .createPacker  = _PTREE_MOD_TICK_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_TICK_INPUT_HOOK = {
    .free = _PTREE_MOD_TICK_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_TICK_OUTPUT_OPS = {
    .getType          = _PTREE_MOD_TICK_OutGetType,
    .createNLinker    = _PTREE_MOD_TICK_OutCreateNLinker,
    .getPacketInfo    = _PTREE_MOD_TICK_OutGetPacketInfo,
    .linkedTransfer   = _PTREE_MOD_TICK_OutLinkedTransfer,
    .unlinkedTransfer = _PTREE_MOD_TICK_OutUnlinkedTransfer,
    .suspendTransfer  = _PTREE_MOD_TICK_OutSuspendTransfer,
    .resumeTransfer   = _PTREE_MOD_TICK_OutResumeTransfer,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_TICK_OUTPUT_HOOK = {
    .free = _PTREE_MOD_TICK_OutFree,
};

static const PTREE_MOD_Ops_t G_PTREE_MOD_TICK_OPS = {
    .init         = _PTREE_MOD_TICK_Init,
    .deinit       = _PTREE_MOD_TICK_Deinit,
    .createModIn  = _PTREE_MOD_TICK_CreateModIn,
    .createModOut = _PTREE_MOD_TICK_CreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_TICK_HOOK = {
    .destruct = _PTREE_MOD_TICK_Destruct,
    .free     = _PTREE_MOD_TICK_Free,
};

void PTREE_MOD_TICK_SetBlockTime(PTREE_MOD_Obj_t* mod, unsigned int sec, unsigned int nsec)
{
    PTREE_MOD_TICK_Obj_t* tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);
    SSOS_THREAD_MutexLock(&tickMod->frcObject.frcLock);

    tickMod->frcObject.diffTime.tvSec  = sec;
    tickMod->frcObject.diffTime.tvNSec = nsec;
    tickMod->frcObject.frcType         = E_PTREE_MOD_TICK_FRC_TYPE_SYNC;

    SSOS_THREAD_MutexUnlock(&tickMod->frcObject.frcLock);
}

void PTREE_MOD_TICK_SetRateFps(PTREE_MOD_Obj_t* mod, unsigned short srcFps, unsigned short dstFps)
{
    PTREE_MOD_TICK_Obj_t* tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);
    SSOS_THREAD_MutexLock(&tickMod->frcObject.frcLock);

    tickMod->frcObject.srcFps  = srcFps;
    tickMod->frcObject.dstFps  = dstFps;
    tickMod->frcObject.frcType = E_PTREE_MOD_TICK_FRC_TYPE_RATIO;

    SSOS_THREAD_MutexUnlock(&tickMod->frcObject.frcLock);
}

void PTREE_MOD_TICK_SetDestFps(PTREE_MOD_Obj_t* mod, unsigned short dstFps)
{
    PTREE_MOD_TICK_Obj_t* tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);
    SSOS_THREAD_MutexLock(&tickMod->frcObject.frcLock);

    if (0 == dstFps)
    {
        PTREE_WRN("tick set dest type fps: %d failed, not support this fps.\n", dstFps);
        return;
    }

    tickMod->frcObject.frcType = E_PTREE_MOD_TICK_FRC_TYPE_ASYNC;
    tickMod->frcObject.dstFps  = dstFps;

    SSOS_THREAD_MutexUnlock(&tickMod->frcObject.frcLock);
}

void PTREE_MOD_TICK_SetLeftFrame(PTREE_MOD_Obj_t* mod, unsigned int frameCnt)
{
    PTREE_MOD_TICK_Obj_t* tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);
    SSOS_THREAD_MutexLock(&tickMod->frcObject.frcLock);

    tickMod->frcObject.frameCntLimit = frameCnt;

    SSOS_THREAD_MutexUnlock(&tickMod->frcObject.frcLock);
}

int PTREE_MOD_TICK_WaitFrame(PTREE_MOD_Obj_t* mod, unsigned int waitMs)
{
    SSOS_TIME_Spec_t      time;
    PTREE_MOD_TICK_Obj_t* tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);

    SSOS_THREAD_MutexLock(&tickMod->frcObject.frcLock);
    time.tvSec  = waitMs / 1000000;
    time.tvNSec = (waitMs % 1000000) * 1000;
    if (!SSOS_THREAD_COND_TIME_WAIT(&tickMod->frcObject.frcCond, tickMod->frcObject.frameCntLimit, time))
    {
        PTREE_ERR(" Wait timeout");
        return 0;
    }
    SSOS_THREAD_MutexUnlock(&tickMod->frcObject.frcLock);
    return 1;
}

static int _PTREE_MOD_TICK_InputLinkerEnqueue(PTREE_LINKER_Obj_t* linker, PTREE_PACKET_Obj_t* packet)
{
    PTREE_MOD_TICK_InputLinker_t* tickLinker = CONTAINER_OF(linker, PTREE_MOD_TICK_InputLinker_t, base);
    if (_PTREE_MOD_TICK_FrcControl(tickLinker->frcObj))
    {
        return PTREE_LINKER_Enqueue(tickLinker->holdLinker, packet);
    }
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t* _PTREE_MOD_TICK_InputLinkerDequeue(PTREE_LINKER_Obj_t* linker, int ms)
{
    (void)linker;
    (void)ms;
    PTREE_ERR("Tick should not call here");
    return NULL;
}
static void _PTREE_MOD_TICK_InputLinkerDestruct(PTREE_LINKER_Obj_t* linker)
{
    PTREE_MOD_TICK_InputLinker_t* tickLinker = CONTAINER_OF(linker, PTREE_MOD_TICK_InputLinker_t, base);
    tickLinker->frcObj                       = NULL;
    tickLinker->holdLinker                   = NULL;
}
static void _PTREE_MOD_TICK_InputLinkerFree(PTREE_LINKER_Obj_t* linker)
{
    PTREE_MOD_TICK_InputLinker_t* tickLinker = CONTAINER_OF(linker, PTREE_MOD_TICK_InputLinker_t, base);
    SSOS_MEM_Free(tickLinker);
}
static void _PTREE_MOD_TICK_InputAsyncLinkerDestruct(PTREE_NLINKER_ASYNC_Obj_t* nlinkerAsync)
{
    PTREE_MOD_TICK_InputAsyncLinker_t* tickAsyncLinker =
        CONTAINER_OF(nlinkerAsync, PTREE_MOD_TICK_InputAsyncLinker_t, base);
    tickAsyncLinker->holdLinker = NULL;
}
static void _PTREE_MOD_TICK_InputAsyncLinkerFree(PTREE_NLINKER_ASYNC_Obj_t* nlinkerAsync)
{
    PTREE_MOD_TICK_InputAsyncLinker_t* tickAsyncLinker =
        CONTAINER_OF(nlinkerAsync, PTREE_MOD_TICK_InputAsyncLinker_t, base);
    SSOS_MEM_Free(tickAsyncLinker);
}
static PTREE_LINKER_Obj_t* _PTREE_MOD_TICK_InputAsyncLinkerNew(PTREE_LINKER_Obj_t* linker)
{
    PTREE_MOD_TICK_InputAsyncLinker_t* tickAsyncLinker = NULL;

    tickAsyncLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_TICK_InputAsyncLinker_t));
    if (!tickAsyncLinker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(tickAsyncLinker, 0, sizeof(PTREE_MOD_TICK_InputAsyncLinker_t));
    if (SSOS_DEF_OK != PTREE_NLINKER_ASYNC_Init(&tickAsyncLinker->base, 0, 8))
    {
        SSOS_MEM_Free(tickAsyncLinker);
        return NULL;
    }
    tickAsyncLinker->holdLinker = linker;
    PTREE_NLINKER_ASYNC_Register(&tickAsyncLinker->base, &G_PTREE_MOD_TICK_INPUT_ASYNC_LINKER_HOOK);
    return &tickAsyncLinker->base.base;
}
static PTREE_LINKER_Obj_t* _PTREE_MOD_TICK_InputLinkerNew(PTREE_LINKER_Obj_t*         linker,
                                                          PTREE_MOD_TICK_FrcObject_t* frcObject)
{
    PTREE_MOD_TICK_InputLinker_t* tickLinker = NULL;

    tickLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_TICK_InputLinker_t));
    if (!tickLinker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(tickLinker, 0, sizeof(PTREE_MOD_TICK_InputLinker_t));
    if (SSOS_DEF_OK != PTREE_LINKER_Init(&tickLinker->base, &G_PTREE_MOD_TICK_INPUT_LINKER_OPS))
    {
        SSOS_MEM_Free(tickLinker);
        return NULL;
    }
    tickLinker->holdLinker = linker;
    tickLinker->frcObj     = frcObject;
    PTREE_LINKER_Register(&tickLinker->base, &G_PTREE_MOD_TICK_INPUT_LINKER_HOOK);
    return &tickLinker->base;
}
static int _PTREE_MOD_TICK_OutputLinkerEnqueue(PTREE_LINKER_Obj_t* linker, PTREE_PACKET_Obj_t* packet)
{
    (void)linker;
    (void)packet;
    PTREE_ERR("Tick should not call here");
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t* _PTREE_MOD_TICK_OutputLinkerDequeue(PTREE_LINKER_Obj_t* linker, int ms)
{
    PTREE_MOD_TICK_OutputLinker_t* tickLinker = CONTAINER_OF(linker, PTREE_MOD_TICK_OutputLinker_t, base);
    if (_PTREE_MOD_TICK_FrcControl(tickLinker->frcObj))
    {
        return PTREE_LINKER_Dequeue(tickLinker->holdLinker, ms);
    }
    return NULL;
}
static void _PTREE_MOD_TICK_OutputLinkerDestruct(PTREE_LINKER_Obj_t* linker)
{
    PTREE_MOD_TICK_OutputLinker_t* tickLinker = CONTAINER_OF(linker, PTREE_MOD_TICK_OutputLinker_t, base);
    tickLinker->frcObj                        = NULL;
    tickLinker->holdLinker                    = NULL;
}
static void _PTREE_MOD_TICK_OutputLinkerFree(PTREE_LINKER_Obj_t* linker)
{
    PTREE_MOD_TICK_OutputLinker_t* tickLinker = CONTAINER_OF(linker, PTREE_MOD_TICK_OutputLinker_t, base);
    SSOS_MEM_Free(tickLinker);
}
static PTREE_LINKER_Obj_t* _PTREE_MOD_TICK_OutputLinkerNew(PTREE_LINKER_Obj_t*         linker,
                                                           PTREE_MOD_TICK_FrcObject_t* frcObject)
{
    PTREE_MOD_TICK_OutputLinker_t* tickLinker = NULL;

    tickLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_TICK_OutputLinker_t));
    if (!tickLinker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(tickLinker, 0, sizeof(PTREE_MOD_TICK_OutputLinker_t));
    if (SSOS_DEF_OK != PTREE_LINKER_Init(&tickLinker->base, &G_PTREE_MOD_TICK_OUTPUT_LINKER_OPS))
    {
        SSOS_MEM_Free(tickLinker);
        return NULL;
    }
    tickLinker->holdLinker = linker;
    tickLinker->frcObj     = frcObject;
    PTREE_LINKER_Register(&tickLinker->base, &G_PTREE_MOD_TICK_OUTPUT_LINKER_HOOK);
    return &tickLinker->base;
}
static int _PTREE_MOD_TICK_OutputFifoLinkerEnqueue(PTREE_NLINKER_OUT_Obj_t* nlinkerOut, PTREE_PACKET_Obj_t* packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}
static PTREE_PACKET_Obj_t* _PTREE_MOD_TICK_OutputFifoLinkerDequeueFromInside(PTREE_NLINKER_OUT_Obj_t* nlinkerOut,
                                                                             int                      ms)
{
    PTREE_MOD_TICK_OutputFifoLinker_t* fifoLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_TICK_OutputFifoLinker_t, base);
    if (_PTREE_MOD_TICK_FrcControl(fifoLinker->frcObj))
    {
        return PTREE_NLINKER_ASYNC_WaitPacket(fifoLinker->holdLinker, ms);
    }
    return NULL;
}
static int _PTREE_MOD_TICK_OutputFifoLinkerDequeueOut(PTREE_NLINKER_OUT_Obj_t* nlinkerOut, PTREE_PACKET_Obj_t* packet)
{
    (void)nlinkerOut;
    (void)packet;
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_TICK_OutputFifoLinkerDestruct(PTREE_NLINKER_OUT_Obj_t* nlinkerOut)
{
    PTREE_MOD_TICK_OutputFifoLinker_t* fifoLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_TICK_OutputFifoLinker_t, base);
    fifoLinker->frcObj                            = NULL;
    fifoLinker->holdLinker                        = NULL;
}
static void _PTREE_MOD_TICK_OutputFifoLinkerFree(PTREE_NLINKER_OUT_Obj_t* nlinkerOut)
{
    PTREE_MOD_TICK_OutputFifoLinker_t* fifoLinker = CONTAINER_OF(nlinkerOut, PTREE_MOD_TICK_OutputFifoLinker_t, base);
    SSOS_MEM_Free(fifoLinker);
}
static PTREE_LINKER_Obj_t* _PTREE_MOD_TICK_OutputFifoLinkerNew(PTREE_NLINKER_ASYNC_Obj_t*  holdLinker,
                                                               PTREE_MOD_TICK_FrcObject_t* frcObj)
{
    PTREE_MOD_TICK_OutputFifoLinker_t* fifoLinker = NULL;

    fifoLinker = SSOS_MEM_Alloc(sizeof(PTREE_MOD_TICK_OutputFifoLinker_t));
    if (!fifoLinker)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(fifoLinker, 0, sizeof(PTREE_MOD_TICK_OutputFifoLinker_t));

    if (SSOS_DEF_OK != PTREE_NLINKER_OUT_Init(&fifoLinker->base, &G_PTREE_MOD_TICK_OUTPUT_FIFO_LINKER_OPS))
    {
        SSOS_MEM_Free(fifoLinker);
        return NULL;
    }

    fifoLinker->holdLinker = holdLinker;
    fifoLinker->frcObj     = frcObj;
    PTREE_NLINKER_OUT_Register(&fifoLinker->base, &G_PTREE_MOD_TICK_OUTPUT_FIFO_LINKER_HOOK);
    return &fifoLinker->base.base;
}

static unsigned char _PTREE_MOD_TICK_FrcStepFrame(PTREE_MOD_TICK_FrcObject_t* frcObj)
{
    SSOS_THREAD_MutexLock(&frcObj->frcLock);
    frcObj->frameStep += frcObj->dstFps;
    if (frcObj->frameStep < frcObj->srcFps)
    {
        SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
        return SSOS_DEF_FALSE;
    }
    frcObj->frameStep -= frcObj->srcFps;
    SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
    return SSOS_DEF_TRUE;
}

static unsigned char _PTREE_MOD_TICK_FrcStepAsync(PTREE_MOD_TICK_FrcObject_t* frcObj)
{
    SSOS_TIME_Spec_t curTime;

    SSOS_THREAD_MutexLock(&frcObj->frcLock);
    if (!frcObj->diffTime.tvSec && !frcObj->diffTime.tvNSec)
    {
        SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
        return SSOS_DEF_TRUE;
    }
    SSOS_TIME_Get(&curTime);
    if (!frcObj->startTime.tvSec || !frcObj->startTime.tvNSec)
    {
        frcObj->startTime = curTime;
    }
    if (curTime.tvSec > frcObj->startTime.tvSec
        || (curTime.tvSec == frcObj->startTime.tvSec && curTime.tvNSec > frcObj->startTime.tvNSec))
    {
        PTREE_MOD_TICK_ADD_TIMER(frcObj->startTime, frcObj->diffTime);
        SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
        return SSOS_DEF_TRUE;
    }
    SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
    return SSOS_DEF_FALSE;
}
static unsigned char _PTREE_MOD_TICK_FrcStepSync(PTREE_MOD_TICK_FrcObject_t* frcObj)
{
    SSOS_TIME_Spec_t curTime;
    SSOS_TIME_Spec_t tmpDiffTime;

    SSOS_THREAD_MutexLock(&frcObj->frcLock);
    if (frcObj->diffTime.tvSec == -1)
    {
        SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
        SSOS_TIME_USleep(10 * 1000);
        return SSOS_DEF_TRUE;
    }
    if (!frcObj->diffTime.tvSec && !frcObj->diffTime.tvNSec)
    {
        SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
        return SSOS_DEF_TRUE;
    }
    SSOS_TIME_Get(&curTime);
    if (!frcObj->startTime.tvSec || !frcObj->startTime.tvNSec)
    {
        frcObj->startTime = curTime;
    }
    PTREE_MOD_TICK_ADD_TIMER(frcObj->startTime, frcObj->diffTime);
    if (curTime.tvSec > frcObj->startTime.tvSec
        || (curTime.tvSec == frcObj->startTime.tvSec && curTime.tvNSec > frcObj->startTime.tvNSec))
    {
        PTREE_WRN(
            "Take timer exceeded! current time sec: %lld nsec:%lld, next sec: %lld nsec: %lld, step sec: %lld nsec: "
            "%lld",
            curTime.tvSec, curTime.tvNSec, frcObj->startTime.tvSec, frcObj->startTime.tvNSec, frcObj->diffTime.tvSec,
            frcObj->diffTime.tvNSec);
        frcObj->startTime = curTime;
        PTREE_MOD_TICK_ADD_TIMER(frcObj->startTime, frcObj->diffTime);
    }
    tmpDiffTime.tvSec = frcObj->startTime.tvSec - curTime.tvSec;
    if (curTime.tvNSec > frcObj->startTime.tvNSec)
    {
        tmpDiffTime.tvNSec = 1000000000 - (curTime.tvNSec - frcObj->startTime.tvNSec);
        tmpDiffTime.tvSec--;
    }
    else
    {
        tmpDiffTime.tvNSec = frcObj->startTime.tvNSec - curTime.tvNSec;
    }
    SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
    SSOS_THREAD_COND_TIME_WAIT(&frcObj->frcCond, 0, tmpDiffTime);
    return SSOS_DEF_TRUE;
}

static unsigned char _PTREE_MOD_TICK_FrcCntControl(PTREE_MOD_TICK_FrcObject_t* frcObj)
{
    SSOS_THREAD_MutexLock(&frcObj->frcLock);

    if (frcObj->frameCntLimit == 0)
    {
        SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
        return SSOS_DEF_TRUE;
    }
    if (frcObj->frameCntLimit > 0)
    {
        frcObj->frameCntLimit--;
    }
    SSOS_THREAD_MutexUnlock(&frcObj->frcLock);
    return SSOS_DEF_FALSE;
}

static unsigned char _PTREE_MOD_TICK_FrcControl(PTREE_MOD_TICK_FrcObject_t* frcObj)
{
    if (_PTREE_MOD_TICK_FrcCntControl(frcObj))
    {
        return SSOS_DEF_FALSE;
    }

    if (frcObj->frcType == E_PTREE_MOD_TICK_FRC_TYPE_RATIO)
    {
        return _PTREE_MOD_TICK_FrcStepFrame(frcObj);
    }
    if (frcObj->frcType == E_PTREE_MOD_TICK_FRC_TYPE_ASYNC)
    {
        return _PTREE_MOD_TICK_FrcStepAsync(frcObj);
    }
    return _PTREE_MOD_TICK_FrcStepSync(frcObj);
}

static int _PTREE_MOD_TICK_IsSyncMode(const PTREE_MOD_InObj_t* modIn)
{
    PTREE_SUR_TICK_Info_t* tickInfo = NULL;

    tickInfo = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_TICK_Info_t, base);
    if (!strcmp(tickInfo->ioMode, "push"))
    {
        return SSOS_DEF_TRUE;
    }
    if (!strcmp(tickInfo->ioMode, "fifo") || !strcmp(tickInfo->ioMode, "pull"))
    {
        return SSOS_DEF_FALSE;
    }
    return SSOS_DEF_TRUE;
}
static int _PTREE_MOD_TICK_InLinked(PTREE_MOD_InObj_t* modIn, unsigned int ref)
{
    PTREE_MOD_TICK_Obj_t* tickMod    = NULL;
    PTREE_MOD_OutObj_t*   modOut     = NULL;
    PTREE_SUR_OutInfo_t*  outInfo    = NULL;
    int                   isSync     = 0;
    int                   receivable = 0;

    if (ref)
    {
        return SSOS_DEF_OK;
    }

    SSOS_ASSERT(modIn && modIn->thisMod);
    if (modIn->thisMod->info->outCnt != 1)
    {
        PTREE_ERR("Not found out port 0");
        return -1;
    }
    /* Find current output port param. */
    outInfo = modIn->thisMod->arrModOut[0]->info;
    if (!outInfo || outInfo->port != 0)
    {
        PTREE_ERR("Not found out port 0");
        return -1;
    }
    tickMod = CONTAINER_OF(modIn->thisMod, PTREE_MOD_TICK_Obj_t, base);
    /* Previous module's output parameter. */
    outInfo                   = modIn->prevModOut->info;
    tickMod->frcObject.srcFps = outInfo->fps;
    tickMod->frcObject.dstFps = modIn->info->fps;
    modOut                    = modIn->thisMod->arrModOut[0];
    SSOS_ASSERT(modOut);
    isSync     = _PTREE_MOD_TICK_IsSyncMode(modIn);
    receivable = modOut->nlinker != NULL;
    if (receivable && isSync)
    {
        PTREE_ERR("======== Tick input port %d is sync mode and it's next is post reader.", modIn->info->port);
        PTREE_ERR("======== Stream pipeline will slow");
        PTREE_ERR("======== Modify %s -> IN_%d -> MODE to fifo/pull", modIn->thisMod->info->sectionName,
                  modIn->info->port);
        SSOS_ASSERT(0);
    }
    if (!receivable && !isSync)
    {
        PTREE_ERR("======== Tick input port %d is not sync mode and it's next is not post reader.", modIn->info->port);
        PTREE_ERR("======== Stream pipeline will pending");
        PTREE_ERR("======== Modify %s -> IN_%d -> MODE to push", modIn->thisMod->info->sectionName, modIn->info->port);
        SSOS_ASSERT(0);
    }
    return 0;
}
static int _PTREE_MOD_TICK_InGetType(PTREE_MOD_InObj_t* modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static int _PTREE_MOD_TICK_InIsPostReader(PTREE_MOD_InObj_t* modIn)
{
    PTREE_SUR_TICK_Info_t* tickInfo = NULL;

    tickInfo = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_TICK_Info_t, base);
    if (!strcmp(tickInfo->ioMode, "push") || !strcmp(tickInfo->ioMode, "fifo"))
    {
        return SSOS_DEF_FALSE;
    }
    if (!strcmp(tickInfo->ioMode, "pull"))
    {
        return SSOS_DEF_TRUE;
    }
    return SSOS_DEF_FALSE;
}
static PTREE_LINKER_Obj_t* _PTREE_MOD_TICK_InCreateNLinker(PTREE_MOD_InObj_t* modIn)
{
    PTREE_SUR_TICK_Info_t* tickInfo = NULL;
    PTREE_SUR_OutInfo_t*   outInfo  = NULL;
    PTREE_MOD_OutObj_t*    modOut   = NULL;

    if (modIn->thisMod->info->outCnt != 1)
    {
        PTREE_ERR("Not found out port 0");
        return NULL;
    }
    outInfo = modIn->thisMod->arrModOut[0]->info;
    if (!outInfo || outInfo->port != 0)
    {
        PTREE_ERR("Not found out port 0");
        return NULL;
    }
    tickInfo = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_TICK_Info_t, base);

    if (!strcmp(tickInfo->ioMode, "pull"))
    {
        return NULL;
    }
    if (!strcmp(tickInfo->ioMode, "push"))
    {
        PTREE_MOD_TICK_Obj_t* tickMod = NULL;
        modOut                        = modIn->thisMod->arrModOut[0];
        tickMod                       = CONTAINER_OF(modIn->thisMod, PTREE_MOD_TICK_Obj_t, base);
        SSOS_ASSERT(modOut);
        return _PTREE_MOD_TICK_InputLinkerNew(&modOut->plinker.base, &tickMod->frcObject);
    }
    if (!strcmp(tickInfo->ioMode, "fifo"))
    {
        return _PTREE_MOD_TICK_InputAsyncLinkerNew(&modOut->plinker.base);
    }
    PTREE_ERR("Tick's 'MODE' is not specified, MODE:%s", tickInfo->ioMode);
    return NULL;
}
static PTREE_PACKER_Obj_t* _PTREE_MOD_TICK_InCreatePacker(PTREE_MOD_InObj_t* modIn, int* isFast)
{
    PTREE_SUR_OutInfo_t*       outInfo      = NULL;
    PTREE_MOD_OutObj_t*        modOut       = NULL;
    PTREE_PACKER_BYPASS_Obj_t* packerBypass = NULL;
    PTREE_PACKER_Obj_t*        packer       = NULL;
    if (modIn->thisMod->info->outCnt != 1)
    {
        PTREE_ERR("Not found out port 0");
        return NULL;
    }
    outInfo = modIn->thisMod->arrModOut[0]->info;
    if (!outInfo || outInfo->port != 0)
    {
        PTREE_ERR("Not found out port 0");
        return NULL;
    }
    modOut = modIn->thisMod->arrModOut[0];
    packer = PTREE_PACKER_BYPASS_New();
    if (!packer)
    {
        return NULL;
    }
    packerBypass = CONTAINER_OF(packer, PTREE_PACKER_BYPASS_Obj_t, base);
    PTREE_PACKER_BYPASS_SetTarget(packerBypass, &modOut->packer.base);
    *isFast = SSOS_DEF_FALSE;
    return packer;
}
static void _PTREE_MOD_TICK_InFree(PTREE_MOD_InObj_t* modIn)
{
    SSOS_MEM_Free(modIn);
}

static int _PTREE_MOD_TICK_OutGetType(PTREE_MOD_OutObj_t* modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t* _PTREE_MOD_TICK_OutCreateNLinker(PTREE_MOD_OutObj_t* modOut)
{
    PTREE_SUR_TICK_Info_t* tickInfo = NULL;
    PTREE_SUR_InInfo_t*    inInfo   = NULL;
    PTREE_MOD_InObj_t*     modIn    = NULL;

    if (modOut->thisMod->info->outCnt != 1)
    {
        PTREE_ERR("Not found in port 0");
        return NULL;
    }
    inInfo = modOut->thisMod->arrModIn[0]->info;
    if (!inInfo || inInfo->port != 0)
    {
        PTREE_ERR("Not found out port 0");
        return NULL;
    }
    tickInfo = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_TICK_Info_t, base);

    if (!strcmp(tickInfo->ioMode, "pull"))
    {
        PTREE_MOD_TICK_Obj_t* tickMod = NULL;
        modIn                         = modOut->thisMod->arrModIn[0];
        tickMod                       = CONTAINER_OF(modIn->thisMod, PTREE_MOD_TICK_Obj_t, base);
        SSOS_ASSERT(modIn);
        return _PTREE_MOD_TICK_OutputLinkerNew(&modIn->plinker.base, &tickMod->frcObject);
    }
    if (!strcmp(tickInfo->ioMode, "fifo"))
    {
        PTREE_MOD_TICK_Obj_t*      tickMod     = NULL;
        PTREE_NLINKER_ASYNC_Obj_t* asyncLinker = NULL;
        modIn                                  = modOut->thisMod->arrModIn[0];
        tickMod                                = CONTAINER_OF(modIn->thisMod, PTREE_MOD_TICK_Obj_t, base);
        SSOS_ASSERT(modIn);
        asyncLinker = CONTAINER_OF(modIn->nlinker, PTREE_NLINKER_ASYNC_Obj_t, base);
        return _PTREE_MOD_TICK_OutputFifoLinkerNew(asyncLinker, &tickMod->frcObject);
    }
    PTREE_ERR("Tick's 'MODE' is not specified, MODE:%s", tickInfo->ioMode);
    return NULL;
}
static PTREE_PACKET_Info_t* _PTREE_MOD_TICK_OutGetPacketInfo(PTREE_MOD_OutObj_t* modOut)
{
    PTREE_SUR_InInfo_t* inInfo = NULL;
    PTREE_MOD_InObj_t*  modIn  = NULL;

    if (modOut->thisMod->info->inCnt != 1)
    {
        PTREE_ERR("Not found in port 0");
        return NULL;
    }
    modIn = modOut->thisMod->arrModIn[0];
    if (!inInfo || modIn->info->port != 0)
    {
        PTREE_ERR("Not found out port 0");
        return NULL;
    }
    return PTREE_MESSAGE_GetPacketInfo(&modIn->message);
}
static int _PTREE_MOD_TICK_OutLinkedTransfer(PTREE_MOD_OutObj_t* modOut)
{
    PTREE_MOD_InObj_t* modIn = NULL;

    SSOS_ASSERT(modOut);
    if (modOut->thisMod->info->inCnt != 1)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    modIn = modOut->thisMod->arrModIn[0];
    if (modIn->info->port != 0)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    SSOS_ASSERT(modIn);
    PTREE_MESSAGE_Access(&modIn->message);
    return 0;
}
static int _PTREE_MOD_TICK_OutUnlinkedTransfer(PTREE_MOD_OutObj_t* modOut)
{
    PTREE_MOD_InObj_t* modIn = NULL;

    SSOS_ASSERT(modOut);
    if (modOut->thisMod->info->inCnt != 1)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    modIn = modOut->thisMod->arrModIn[0];
    if (modIn->info->port != 0)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    SSOS_ASSERT(modIn);
    PTREE_MESSAGE_Leave(&modIn->message);
    return 0;
}
static int _PTREE_MOD_TICK_OutSuspendTransfer(PTREE_MOD_OutObj_t* modOut)
{
    PTREE_MOD_InObj_t* modIn = NULL;

    SSOS_ASSERT(modOut);
    if (modOut->thisMod->info->inCnt != 1)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    modIn = modOut->thisMod->arrModIn[0];
    if (modIn->info->port != 0)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    SSOS_ASSERT(modIn);
    PTREE_MESSAGE_Suspend(&modIn->message);
    return 0;
}
static int _PTREE_MOD_TICK_OutResumeTransfer(PTREE_MOD_OutObj_t* modOut)
{
    PTREE_MOD_InObj_t* modIn = NULL;

    SSOS_ASSERT(modOut);
    if (modOut->thisMod->info->inCnt != 1)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    modIn = modOut->thisMod->arrModIn[0];
    if (modIn->info->port != 0)
    {
        PTREE_ERR("Not found in port 0");
        return -1;
    }
    SSOS_ASSERT(modIn);
    PTREE_MESSAGE_Resume(&modIn->message);
    return 0;
}
static void _PTREE_MOD_TICK_OutFree(PTREE_MOD_OutObj_t* modOut)
{
    SSOS_MEM_Free(modOut);
}

static int _PTREE_MOD_TICK_Init(PTREE_MOD_Obj_t* mod)
{
    PTREE_MOD_TICK_Obj_t*  tickMod  = NULL;
    PTREE_SUR_TICK_Info_t* tickInfo = NULL;

    tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);
    if (!tickMod)
    {
        PTREE_ERR("tickMod is null");
        return -1;
    }
    tickInfo = CONTAINER_OF(tickMod->base.info, PTREE_SUR_TICK_Info_t, base);
    if (!tickInfo)
    {
        PTREE_ERR("tickInfo is null");
        return -1;
    }
    memset(&tickMod->frcObject, 0, sizeof(PTREE_MOD_TICK_FrcObject_t));
    tickMod->frcObject.diffTime.tvSec  = tickInfo->sec;
    tickMod->frcObject.diffTime.tvNSec = tickInfo->nSec;
    if (!strcmp(tickInfo->frcType, "dest"))
    {
        tickMod->frcObject.frcType = E_PTREE_MOD_TICK_FRC_TYPE_ASYNC;
    }
    else if (!strcmp(tickInfo->frcType, "rate"))
    {
        tickMod->frcObject.frcType = E_PTREE_MOD_TICK_FRC_TYPE_RATIO;
    }
    else
    {
        /* block  */
        tickMod->frcObject.frcType = E_PTREE_MOD_TICK_FRC_TYPE_SYNC;
    }
    SSOS_THREAD_MutexInit(&tickMod->frcObject.frcLock);
    SSOS_THREAD_COND_INIT(&tickMod->frcObject.frcCond);
    PTREE_MOD_TICK_SetLeftFrame(mod, tickInfo->frameCntLimit);
    return 0;
}
static int _PTREE_MOD_TICK_Deinit(PTREE_MOD_Obj_t* mod)
{
    PTREE_MOD_TICK_Obj_t* tickMod = NULL;

    tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);

    SSOS_THREAD_COND_DEINIT(&tickMod->frcObject.frcCond);
    SSOS_THREAD_MutexDeinit(&tickMod->frcObject.frcLock);
    return 0;
}
static PTREE_MOD_InObj_t* _PTREE_MOD_TICK_CreateModIn(PTREE_MOD_Obj_t* mod, unsigned int loopId)
{
    PTREE_MOD_InObj_t* modIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_InObj_t));
    if (!modIn)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(modIn, 0, sizeof(PTREE_MOD_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(modIn, &G_PTREE_MOD_TICK_INPUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(modIn);
        return NULL;
    }
    PTREE_MOD_InObjRegister(modIn, &G_PTREE_MOD_TICK_INPUT_HOOK);
    return modIn;
}
static PTREE_MOD_OutObj_t* _PTREE_MOD_TICK_CreateModOut(PTREE_MOD_Obj_t* mod, unsigned int loopId)
{
    PTREE_MOD_OutObj_t* modOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_OutObj_t));
    if (!modOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(modOut, 0, sizeof(PTREE_MOD_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(modOut, &G_PTREE_MOD_TICK_OUTPUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(modOut);
        return NULL;
    }
    PTREE_MOD_OutObjRegister(modOut, &G_PTREE_MOD_TICK_OUTPUT_HOOK);
    return modOut;
}
static void _PTREE_MOD_TICK_Destruct(PTREE_MOD_Obj_t* mod)
{
    PTREE_MOD_TICK_Obj_t* tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);
    SSOS_THREAD_COND_DEINIT(&tickMod->frcObject.frcCond);
    SSOS_THREAD_MutexDeinit(&tickMod->frcObject.frcLock);
}
static void _PTREE_MOD_TICK_Free(PTREE_MOD_Obj_t* mod)
{
    PTREE_MOD_TICK_Obj_t* tickMod = CONTAINER_OF(mod, PTREE_MOD_TICK_Obj_t, base);
    SSOS_MEM_Free(tickMod);
}
PTREE_MOD_Obj_t* PTREE_MOD_TICK_New(PARENA_Tag_t* tag)
{
    PTREE_MOD_TICK_Obj_t* tickMod = NULL;

    tickMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_TICK_Obj_t));
    if (!tickMod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(tickMod, 0, sizeof(PTREE_MOD_TICK_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&tickMod->base, &G_PTREE_MOD_TICK_OPS, tag))
    {
        SSOS_MEM_Free(tickMod);
        return NULL;
    }
    SSOS_THREAD_MutexInit(&tickMod->frcObject.frcLock);
    SSOS_THREAD_COND_INIT(&tickMod->frcObject.frcCond);
    PTREE_MOD_ObjRegister(&tickMod->base, &G_PTREE_MOD_TICK_HOOK);
    return &tickMod->base;
}

PTREE_MAKER_MOD_INIT(TICK, PTREE_MOD_TICK_New);
