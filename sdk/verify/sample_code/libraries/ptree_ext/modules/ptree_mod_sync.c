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
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_packer_bypass.h"
#include "ptree_packet.h"
#include "ssos_task.h"
#include "ptree_mod.h"
#include "ptree_maker.h"
#include "ptree_sur_sync.h"
#include "ptree_mod_sync.h"
#include "ssos_time.h"

typedef struct PTREE_MOD_SYNC_Obj_s    PTREE_MOD_SYNC_Obj_t;
typedef struct PTREE_MOD_SYNC_InObj_s  PTREE_MOD_SYNC_InObj_t;
typedef struct PTREE_MOD_SYNC_OutObj_s PTREE_MOD_SYNC_OutObj_t;

struct PTREE_MOD_SYNC_Obj_s
{
    PTREE_MOD_Obj_t base;
    void *          taskHandle;
};

struct PTREE_MOD_SYNC_InObj_s
{
    PTREE_MOD_InObj_t         base;
    PTREE_LINKER_BYPASS_Obj_t bypassLinker;
    PTREE_PACKER_BYPASS_Obj_t bypassPacker;
    PTREE_MOD_OutObj_t *      targetModOut;
};

struct PTREE_MOD_SYNC_OutObj_s
{
    PTREE_MOD_OutObj_t base;
    PTREE_MOD_InObj_t *targetModIn;
};

static int                 _PTREE_MOD_SYNC_InGetType(PTREE_MOD_InObj_t *modIn);
static int                 _PTREE_MOD_SYNC_InIsPostReader(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYNC_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_SYNC_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static void                _PTREE_MOD_SYNC_InDestruct(PTREE_MOD_InObj_t *modIn);
static void                _PTREE_MOD_SYNC_InFree(PTREE_MOD_InObj_t *modIn);

static int                  _PTREE_MOD_SYNC_OutGetType(PTREE_MOD_OutObj_t *modOut);
static PTREE_PACKET_Info_t *_PTREE_MOD_SYNC_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYNC_OutLinkedTransfer(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYNC_OutUnlinkedTransfer(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYNC_OutSuspendTransfer(PTREE_MOD_OutObj_t *modOut);
static int                  _PTREE_MOD_SYNC_OutResumeTransfer(PTREE_MOD_OutObj_t *modOut);
static void                 _PTREE_MOD_SYNC_OutFree(PTREE_MOD_OutObj_t *modOut);

static int                 _PTREE_MOD_SYNC_Init(PTREE_MOD_Obj_t *mod);
static int                 _PTREE_MOD_SYNC_Start(PTREE_MOD_Obj_t *mod);
static int                 _PTREE_MOD_SYNC_Stop(PTREE_MOD_Obj_t *mod);
static PTREE_MOD_InObj_t * _PTREE_MOD_SYNC_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_SYNC_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_SYNC_Free(PTREE_MOD_Obj_t *mod);
static PTREE_MOD_InObj_t * _PTREE_MOD_SYNC_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut);

static const PTREE_MOD_Ops_t G_PTREE_MOD_SYNC_OPS = {
    .init         = _PTREE_MOD_SYNC_Init,
    .start        = _PTREE_MOD_SYNC_Start,
    .stop         = _PTREE_MOD_SYNC_Stop,
    .createModIn  = _PTREE_MOD_SYNC_CreateModIn,
    .createModOut = _PTREE_MOD_SYNC_CreateModOut,
};
static const PTREE_MOD_Hook_t G_PTREE_MOD_SYNC_HOOK = {
    .free = _PTREE_MOD_SYNC_Free,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_SYNC_IN_OPS = {
    .getType       = _PTREE_MOD_SYNC_InGetType,
    .isPostReader  = _PTREE_MOD_SYNC_InIsPostReader,
    .createNLinker = _PTREE_MOD_SYNC_InCreateNLinker,
    .createPacker  = _PTREE_MOD_SYNC_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_SYNC_IN_HOOK = {
    .destruct = _PTREE_MOD_SYNC_InDestruct,
    .free     = _PTREE_MOD_SYNC_InFree,
};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_SYNC_OUT_OPS = {
    .getType             = _PTREE_MOD_SYNC_OutGetType,
    .getPacketInfo       = _PTREE_MOD_SYNC_OutGetPacketInfo,
    .linkedTransfer      = _PTREE_MOD_SYNC_OutLinkedTransfer,
    .unlinkedTransfer    = _PTREE_MOD_SYNC_OutUnlinkedTransfer,
    .suspendTransfer     = _PTREE_MOD_SYNC_OutSuspendTransfer,
    .resumeTransfer      = _PTREE_MOD_SYNC_OutResumeTransfer,
    .resetStreamTraverse = _PTREE_MOD_SYNC_ResetStreamTraverse,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_SYNC_OUT_HOOK = {
    .free = _PTREE_MOD_SYNC_OutFree,
};

static int _PTREE_MOD_SYNC_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static int _PTREE_MOD_SYNC_InIsPostReader(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return SSOS_DEF_TRUE;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_SYNC_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYNC_InObj_t *syncInObj = CONTAINER_OF(modIn, PTREE_MOD_SYNC_InObj_t, base);

    if (syncInObj->targetModOut)
    {
        PTREE_LINKER_BYPASS_SetTarget(&syncInObj->bypassLinker, &syncInObj->targetModOut->plinker.base);
    }
    return PTREE_LINKER_Dup(&syncInObj->bypassLinker.base);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_SYNC_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_SYNC_InObj_t *syncInObj = CONTAINER_OF(modIn, PTREE_MOD_SYNC_InObj_t, base);

    if (syncInObj->targetModOut)
    {
        PTREE_PACKER_BYPASS_SetTarget(&syncInObj->bypassPacker, &syncInObj->targetModOut->packer.base);
    }
    *isFast = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&syncInObj->bypassPacker.base);
}
static void _PTREE_MOD_SYNC_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYNC_InObj_t *syncInObj = CONTAINER_OF(modIn, PTREE_MOD_SYNC_InObj_t, base);
    PTREE_LINKER_Del(&syncInObj->bypassLinker.base);
}
static void _PTREE_MOD_SYNC_InFree(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_SYNC_InObj_t *syncInObj = CONTAINER_OF(modIn, PTREE_MOD_SYNC_InObj_t, base);
    SSOS_MEM_Free(syncInObj);
}

static int _PTREE_MOD_SYNC_OutGetType(PTREE_MOD_OutObj_t *modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_PACKET_Info_t *_PTREE_MOD_SYNC_OutGetPacketInfo(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYNC_OutObj_t *syncModOut = CONTAINER_OF(modOut, PTREE_MOD_SYNC_OutObj_t, base);
    if (!syncModOut->targetModIn)
    {
        return NULL;
    }
    return PTREE_MESSAGE_GetPacketInfo(&syncModOut->targetModIn->message);
}
static int _PTREE_MOD_SYNC_OutLinkedTransfer(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYNC_OutObj_t *syncModOut = CONTAINER_OF(modOut, PTREE_MOD_SYNC_OutObj_t, base);
    if (syncModOut->targetModIn)
    {
        PTREE_MESSAGE_Access(&syncModOut->targetModIn->message);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYNC_OutUnlinkedTransfer(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYNC_OutObj_t *syncModOut = CONTAINER_OF(modOut, PTREE_MOD_SYNC_OutObj_t, base);
    if (syncModOut->targetModIn)
    {
        PTREE_MESSAGE_Leave(&syncModOut->targetModIn->message);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYNC_OutSuspendTransfer(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYNC_OutObj_t *syncModOut = CONTAINER_OF(modOut, PTREE_MOD_SYNC_OutObj_t, base);
    if (syncModOut->targetModIn)
    {
        PTREE_MESSAGE_Suspend(&syncModOut->targetModIn->message);
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYNC_OutResumeTransfer(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYNC_OutObj_t *syncModOut = CONTAINER_OF(modOut, PTREE_MOD_SYNC_OutObj_t, base);
    if (syncModOut->targetModIn)
    {
        PTREE_MESSAGE_Resume(&syncModOut->targetModIn->message);
    }
    return SSOS_DEF_OK;
}
static void _PTREE_MOD_SYNC_OutFree(PTREE_MOD_OutObj_t *modOut)
{
    PTREE_MOD_SYNC_OutObj_t *syncModOut = CONTAINER_OF(modOut, PTREE_MOD_SYNC_OutObj_t, base);
    SSOS_MEM_Free(syncModOut);
}

static int _PTREE_MOD_SYNC_Init(PTREE_MOD_Obj_t *mod)
{
    PTREE_SUR_SYNC_InInfo_t *syncInInfo = NULL;
    PTREE_MOD_InObj_t *      modIn      = NULL;
    PTREE_MOD_OutObj_t *     modOut     = NULL;
    PTREE_MOD_SYNC_InObj_t * syncModIn  = NULL;
    PTREE_MOD_SYNC_OutObj_t *syncModOut = NULL;

    int i = 0;

    for (i = 0; i < mod->info->inCnt; ++i)
    {
        syncInInfo = CONTAINER_OF(mod->arrModIn[i]->info, PTREE_SUR_SYNC_InInfo_t, base);
        modIn      = mod->arrModIn[i];
        if (syncInInfo->u16OutIndex >= mod->info->outCnt)
        {
            PTREE_ERR("Sync input %d link to output %d err, output need less than %d", i, syncInInfo->u16OutIndex,
                      mod->info->outCnt);
            continue;
        }
        modOut     = mod->arrModOut[syncInInfo->u16OutIndex];
        syncModIn  = CONTAINER_OF(modIn, PTREE_MOD_SYNC_InObj_t, base);
        syncModOut = CONTAINER_OF(modOut, PTREE_MOD_SYNC_OutObj_t, base);

        syncModIn->targetModOut = modOut;
        syncModOut->targetModIn = modIn;
    }

    return SSOS_DEF_OK;
}
static void *_PTREE_MOD_SYNC_PtreeSyncReaderTask(struct SSOS_TASK_Buffer_s *pstBuf)
{
    int                     idx       = 0;
    PTREE_PACKET_Obj_t *    packet    = NULL;
    PTREE_MOD_Obj_t *       mod       = (PTREE_MOD_Obj_t *)pstBuf->buf;
    PTREE_MOD_InObj_t *     modIn     = NULL;
    PTREE_MOD_SYNC_InObj_t *syncModIn = NULL;
    unsigned char           bTransfer = 0;
    for (idx = 0; idx < mod->info->inCnt; idx++)
    {
        modIn     = mod->arrModIn[idx];
        syncModIn = CONTAINER_OF(modIn, PTREE_MOD_SYNC_InObj_t, base);
        if (!syncModIn->targetModOut)
        {
            continue;
        }
        if (!PTREE_MESSAGE_Check(&syncModIn->targetModOut->message))
        {
            continue;
        }
        packet = PTREE_LINKER_Dequeue(&modIn->plinker.base, 1000);
        if (NULL == packet)
        {
            continue;
        }
        if (!bTransfer)
        {
            bTransfer = 1;
        }
        PTREE_LINKER_Enqueue(&syncModIn->targetModOut->plinker.base, packet);
        PTREE_PACKET_Del(packet);
    }
    if (!bTransfer)
    {
        SSOS_TIME_MSleep(20);
    }
    return NULL;
}
static int _PTREE_MOD_SYNC_Start(PTREE_MOD_Obj_t *mod)
{
    SSOS_TASK_Attr_t      stTaskAttr;
    PTREE_MOD_SYNC_Obj_t *syncMod = CONTAINER_OF(mod, PTREE_MOD_SYNC_Obj_t, base);
    memset(&stTaskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    stTaskAttr.inBuf.buf       = (void *)mod;
    stTaskAttr.doMonitor       = _PTREE_MOD_SYNC_PtreeSyncReaderTask;
    stTaskAttr.threadAttr.name = mod->info->sectionName;
    syncMod->taskHandle        = SSOS_TASK_Open(&stTaskAttr);
    if (!syncMod->taskHandle)
    {
        PTREE_ERR("Monitor return error!");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_StartMonitor(syncMod->taskHandle);
    PTREE_DBG("Sync reader %s start", mod->info->sectionName);
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_SYNC_Stop(PTREE_MOD_Obj_t *mod)
{
    PTREE_MOD_SYNC_Obj_t *syncMod = CONTAINER_OF(mod, PTREE_MOD_SYNC_Obj_t, base);
    if (!syncMod->taskHandle)
    {
        PTREE_ERR("Task handle null!");
        return SSOS_DEF_FAIL;
    }
    SSOS_TASK_Close(syncMod->taskHandle);
    PTREE_DBG("Sync reader %s exit", mod->info->sectionName);
    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_SYNC_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYNC_InObj_t *syncModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYNC_InObj_t));
    if (!syncModIn)
    {
        PTREE_ERR("Alloc err");
        goto ERR_ALLOC_MEM;
    }
    memset(syncModIn, 0, sizeof(PTREE_MOD_SYNC_InObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&syncModIn->base, &G_PTREE_MOD_SYNC_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }
    if (SSOS_DEF_OK != PTREE_LINKER_BYPASS_Init(&syncModIn->bypassLinker))
    {
        PTREE_ERR("PTREE_LINKER_BYPASS_Init Failed");
        goto ERR_LINKER_BYPASS_INIT;
    }
    if (SSOS_DEF_OK != PTREE_PACKER_BYPASS_Init(&syncModIn->bypassPacker))
    {
        PTREE_ERR("PTREE_PACKER_BYPASS_Init Failed");
        goto ERR_PACKER_BYPASS_INIT;
    }
    PTREE_MOD_InObjRegister(&syncModIn->base, &G_PTREE_MOD_SYNC_IN_HOOK);
    return &syncModIn->base;

ERR_PACKER_BYPASS_INIT:
    PTREE_LINKER_Del(&syncModIn->bypassLinker.base);
ERR_LINKER_BYPASS_INIT:
    PTREE_MOD_InObjDel(&syncModIn->base);
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(syncModIn);
ERR_ALLOC_MEM:
    return NULL;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_SYNC_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_SYNC_OutObj_t *syncModOut = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYNC_OutObj_t));
    if (!syncModOut)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(syncModOut, 0, sizeof(PTREE_MOD_SYNC_OutObj_t));
    if (SSOS_DEF_OK != PTREE_MOD_OutObjInit(&syncModOut->base, &G_PTREE_MOD_SYNC_OUT_OPS, mod, loopId))
    {
        SSOS_MEM_Free(syncModOut);
        return NULL;
    }
    PTREE_MOD_OutObjRegister(&syncModOut->base, &G_PTREE_MOD_SYNC_OUT_HOOK);
    return &syncModOut->base;
}
static void _PTREE_MOD_SYNC_Free(PTREE_MOD_Obj_t *mod)
{
    SSOS_MEM_Free(mod);
}

static PTREE_MOD_InObj_t *_PTREE_MOD_SYNC_ResetStreamTraverse(PTREE_MOD_OutObj_t *modOut)
{
    unsigned char            i          = 0;
    PTREE_SUR_SYNC_InInfo_t *syncInInfo = 0;

    for (i = 0; i < modOut->thisMod->info->inCnt; i++)
    {
        syncInInfo = CONTAINER_OF(modOut->thisMod->arrModIn[i]->info, PTREE_SUR_SYNC_InInfo_t, base);
        if (modOut->thisMod->info->outCnt <= syncInInfo->u16OutIndex)
        {
            PTREE_ERR("Out index not matched");
            return NULL;
        }
        if (modOut->thisMod->arrModOut[syncInInfo->u16OutIndex] == modOut)
        {
            return modOut->thisMod->arrModIn[i];
        }
    }
    return NULL;
}

PTREE_MOD_Obj_t *PTREE_MOD_SYNC_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_SYNC_Obj_t *syncMod = NULL;

    syncMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_SYNC_Obj_t));
    if (!syncMod)
    {
        PTREE_ERR("Alloc err");
        return NULL;
    }
    memset(syncMod, 0, sizeof(PTREE_MOD_SYNC_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_ObjInit(&syncMod->base, &G_PTREE_MOD_SYNC_OPS, tag))
    {
        SSOS_MEM_Free(syncMod);
        return NULL;
    }
    PTREE_MOD_ObjRegister(&syncMod->base, &G_PTREE_MOD_SYNC_HOOK);
    return &syncMod->base;
}

PTREE_MAKER_MOD_INIT(SYNC, PTREE_MOD_SYNC_New);
