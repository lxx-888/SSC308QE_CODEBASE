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
#include "mi_ao_datatype.h"
#include "mi_ao.h"
#include "cam_dev_wrapper.h"
#include "ssos_def.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_packer.h"
#include "ssos_task.h"
#include "ptree_linker.h"
#include "ptree_nlinker_async.h"
#include "ptree_packet.h"
#include "ptree_packet_audio.h"
#include "ptree_mod.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_ao.h"
#include "ptree_sur_ao.h"
#include "ptree_maker.h"

#define PTREE_MOD_AO_READER_TASK_NAME_LEN 32

typedef struct PTREE_MOD_AO_InObj_s PTREE_MOD_AO_InObj_t;
typedef struct PTREE_MOD_AO_Obj_s   PTREE_MOD_AO_Obj_t;

struct PTREE_MOD_AO_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
    PTREE_PACKER_Obj_t  packer;
};
struct PTREE_MOD_AO_InObj_s
{
    PTREE_MOD_InObj_t         base;
    PTREE_NLINKER_ASYNC_Obj_t asyncLinker;
    PTREE_LINKER_Obj_t        syncLinker;
    void *                    taskHandle;
};

static PTREE_PACKET_Obj_t *_PTREE_MOD_AO_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info);

static int                 _PTREE_MOD_AO_SyncLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet);
static PTREE_PACKET_Obj_t *_PTREE_MOD_AO_SyncLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms);

static int                 _PTREE_MOD_AO_InLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static int                 _PTREE_MOD_AO_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref);
static int                 _PTREE_MOD_AO_InGetType(PTREE_MOD_InObj_t *modIn);
static PTREE_LINKER_Obj_t *_PTREE_MOD_AO_InCreateNLinker(PTREE_MOD_InObj_t *modIn);
static PTREE_PACKER_Obj_t *_PTREE_MOD_AO_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast);
static void                _PTREE_MOD_AO_InDestruct(PTREE_MOD_InObj_t *modIn);
static void                _PTREE_MOD_AO_InFree(PTREE_MOD_InObj_t *modIn);
static PTREE_MOD_InObj_t * _PTREE_MOD_AO_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId);

static int                 _PTREE_MOD_AO_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_AO_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_AO_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_AO_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_AO_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static const PTREE_PACKER_Ops_t G_PTREE_MOD_AO_PACKER_OPS = {
    .make = _PTREE_MOD_AO_PackerMake,
};

static const PTREE_PACKER_Hook_t G_PTREE_MOD_AO_PACKER_HOOK = {};

static const PTREE_LINKER_Ops_t G_PTREE_MOD_AO_SYNC_LINKER_OPS = {
    .enqueue = _PTREE_MOD_AO_SyncLinkerEnqueue,
    .dequeue = _PTREE_MOD_AO_SyncLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_AO_SYNC_LINKER_HOOK = {};

static const PTREE_NLINKER_ASYNC_Hook_t G_PTREE_MOD_AO_ASYNC_LINKER_HOOK = {};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_AO_SYS_OPS = {
    .init         = _PTREE_MOD_AO_Init,
    .deinit       = _PTREE_MOD_AO_Deinit,
    .createModIn  = _PTREE_MOD_AO_CreateModIn,
    .createModOut = _PTREE_MOD_AO_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_AO_SYS_HOOK = {
    .free = _PTREE_MOD_AO_Free,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_AO_IN_OPS = {
    .linked        = _PTREE_MOD_AO_InLinked,
    .unlinked      = _PTREE_MOD_AO_InUnlinked,
    .getType       = _PTREE_MOD_AO_InGetType,
    .createNLinker = _PTREE_MOD_AO_InCreateNLinker,
    .createPacker  = _PTREE_MOD_AO_InCreatePacker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_AO_IN_HOOK = {
    .destruct = _PTREE_MOD_AO_InDestruct,
    .free     = _PTREE_MOD_AO_InFree,
};

static PTREE_PACKET_Obj_t *_PTREE_MOD_AO_PackerMake(PTREE_PACKER_Obj_t *packer, const PTREE_PACKET_Info_t *info)
{
    PTREE_PACKET_AUDIO_Info_t *audioPacketInfo = NULL;
    (void)packer;
    if (!PTREE_PACKET_InfoLikely(info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        PTREE_ERR("packet info type %s is not support", info->type);
        return NULL;
    }
    audioPacketInfo = CONTAINER_OF(info, PTREE_PACKET_AUDIO_Info_t, base);
    return PTREE_PACKET_AUDIO_NormalNew(&audioPacketInfo->packetInfo);
}
static int _PTREE_MOD_AO_SyncLinkerEnqueue(PTREE_LINKER_Obj_t *linker, PTREE_PACKET_Obj_t *packet)
{
    PTREE_MOD_AO_InObj_t *    aoModIn     = CONTAINER_OF(linker, PTREE_MOD_AO_InObj_t, syncLinker);
    PTREE_PACKET_AUDIO_Obj_t *audioPacket = NULL;
    void *                    pvBuffer    = NULL;
    unsigned int              byte        = 0;
    int                       sendRetry   = 0;
    unsigned int              i           = 0;

    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        return SSOS_DEF_FAIL;
    }

    audioPacket = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
    for (i = 0; i < audioPacket->info.packetInfo.pktCount; i++)
    {
        pvBuffer  = (void *)audioPacket->packetData.pktData[i].data;
        byte      = audioPacket->info.packetInfo.pktInfo[i].size;
        sendRetry = 20;
        do
        {
            if (MI_SUCCESS == MI_AO_Write((MI_AUDIO_DEV)aoModIn->base.thisMod->info->devId, pvBuffer, byte, 0, -1))
            {
                break;
            }
            sendRetry--;
        } while (sendRetry);
    }

    return 0;
}
static PTREE_PACKET_Obj_t *_PTREE_MOD_AO_SyncLinkerDequeue(PTREE_LINKER_Obj_t *linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}

static void *_PTREE_MOD_AO_ReceiverMonitor(SSOS_TASK_Buffer_t *taskBuf)
{
    void *                    pvBuffer    = NULL;
    unsigned int              byte        = 0;
    int                       sendRetry   = 0;
    unsigned int              i           = 0;
    PTREE_MOD_InObj_t *       modIn       = taskBuf->buf;
    PTREE_MOD_AO_InObj_t *    aoModIn     = CONTAINER_OF(modIn, PTREE_MOD_AO_InObj_t, base);
    PTREE_PACKET_Obj_t *      packet      = NULL;
    PTREE_PACKET_AUDIO_Obj_t *audioPacket = NULL;

    packet = PTREE_NLINKER_ASYNC_WaitPacket(&aoModIn->asyncLinker, 10);
    if (!packet)
    {
        return NULL;
    }
    if (!PTREE_PACKET_InfoLikely(packet->info, PTREE_PACKET_INFO_TYPE(audio)))
    {
        return NULL;
    }

    audioPacket = CONTAINER_OF(packet, PTREE_PACKET_AUDIO_Obj_t, base);
    for (i = 0; i < audioPacket->info.packetInfo.pktCount; i++)
    {
        pvBuffer  = (void *)audioPacket->packetData.pktData[i].data;
        byte      = audioPacket->info.packetInfo.pktInfo[i].size;
        sendRetry = 20;
        do
        {
            if (MI_SUCCESS == MI_AO_Write((MI_AUDIO_DEV)aoModIn->base.thisMod->info->devId, pvBuffer, byte, 0, -1))
            {
                break;
            }
            sendRetry--;
        } while (sendRetry);
    }
    return NULL;
}
static int _PTREE_MOD_AO_InLinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    char                  taskName[PTREE_MOD_AO_READER_TASK_NAME_LEN];
    PTREE_MOD_AO_InObj_t *aoModIn = CONTAINER_OF(modIn, PTREE_MOD_AO_InObj_t, base);
    PTREE_SUR_AO_Info_t * aoInfo  = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_AO_Info_t, base.base);
    SSOS_TASK_Attr_t      taskAttr;

    if (ref > 0)
    {
        return 0;
    }
    if (!aoInfo->u32SyncMode)
    {
        PTREE_MOD_InKeyStr(modIn, taskName, PTREE_MOD_AO_READER_TASK_NAME_LEN);
        memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
        taskAttr.doSignal         = NULL;
        taskAttr.doMonitor        = _PTREE_MOD_AO_ReceiverMonitor;
        taskAttr.monitorCycleSec  = 0;
        taskAttr.monitorCycleNsec = 0;
        taskAttr.isResetTimer     = 0;
        taskAttr.inBuf.buf        = (void *)modIn;
        taskAttr.inBuf.size       = 0;
        taskAttr.threadAttr.name  = taskName;
        aoModIn->taskHandle       = SSOS_TASK_Open(&taskAttr);
        if (!aoModIn->taskHandle)
        {
            PTREE_ERR("AO Task create error!");
            return SSOS_DEF_FAIL;
        }
        SSOS_TASK_StartMonitor(aoModIn->taskHandle);
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_AO_InUnlinked(PTREE_MOD_InObj_t *modIn, unsigned int ref)
{
    PTREE_MOD_AO_InObj_t *aoModIn = NULL;
    PTREE_SUR_AO_Info_t * aoInfo  = NULL;
    aoInfo                        = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_AO_Info_t, base.base);
    aoModIn                       = CONTAINER_OF(modIn, PTREE_MOD_AO_InObj_t, base);

    if (ref > 0)
    {
        return 0;
    }
    if (!aoModIn->taskHandle)
    {
        return SSOS_DEF_OK;
    }
    if (!aoInfo->u32SyncMode)
    {
        SSOS_TASK_Stop(aoModIn->taskHandle);
        SSOS_TASK_Close(aoModIn->taskHandle);
        aoModIn->taskHandle = NULL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_AO_InGetType(PTREE_MOD_InObj_t *modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}
static PTREE_LINKER_Obj_t *_PTREE_MOD_AO_InCreateNLinker(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_AO_InObj_t *aoModIn = CONTAINER_OF(modIn, PTREE_MOD_AO_InObj_t, base);
    PTREE_SUR_AO_Info_t * aoInfo  = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_AO_Info_t, base.base);

    if (aoInfo->u32SyncMode)
    {
        return PTREE_LINKER_Dup(&aoModIn->syncLinker);
    }
    return PTREE_LINKER_Dup(&aoModIn->asyncLinker.base);
}
static PTREE_PACKER_Obj_t *_PTREE_MOD_AO_InCreatePacker(PTREE_MOD_InObj_t *modIn, int *isFast)
{
    PTREE_MOD_AO_Obj_t *aoMod = CONTAINER_OF(modIn->thisMod, PTREE_MOD_AO_Obj_t, base.base);
    *isFast                   = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&aoMod->packer);
}
static void _PTREE_MOD_AO_InDestruct(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_AO_InObj_t *aoModIn = CONTAINER_OF(modIn, PTREE_MOD_AO_InObj_t, base);
    PTREE_SUR_AO_Info_t * aoInfo  = CONTAINER_OF(modIn->thisMod->info, PTREE_SUR_AO_Info_t, base.base);

    if (aoInfo->u32SyncMode)
    {
        PTREE_LINKER_Del(&aoModIn->syncLinker);
    }
    else
    {
        PTREE_LINKER_Del(&aoModIn->asyncLinker.base);
    }
}
static void _PTREE_MOD_AO_InFree(PTREE_MOD_InObj_t *modIn)
{
    PTREE_MOD_AO_InObj_t *aoModIn = CONTAINER_OF(modIn, PTREE_MOD_AO_InObj_t, base);
    SSOS_MEM_Free(aoModIn);
}
static PTREE_MOD_InObj_t *_PTREE_MOD_AO_InNew(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    PTREE_MOD_AO_InObj_t *aoModIn = NULL;
    PTREE_SUR_AO_Info_t * aoInfo  = NULL;

    aoModIn = SSOS_MEM_Alloc(sizeof(PTREE_MOD_AO_InObj_t));
    if (!aoModIn)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(aoModIn, 0, sizeof(PTREE_MOD_AO_InObj_t));

    if (SSOS_DEF_OK != PTREE_MOD_InObjInit(&aoModIn->base, &G_PTREE_MOD_AO_IN_OPS, mod, loopId))
    {
        goto ERR_MOD_IN_INIT;
    }
    aoInfo = CONTAINER_OF(aoModIn->base.thisMod->info, PTREE_SUR_AO_Info_t, base.base);

    if (aoInfo->u32SyncMode)
    {
        if (SSOS_DEF_OK != PTREE_LINKER_Init(&aoModIn->syncLinker, &G_PTREE_MOD_AO_SYNC_LINKER_OPS))
        {
            goto ERR_LINKER_INIT;
        }
        PTREE_LINKER_Register(&aoModIn->syncLinker, &G_PTREE_MOD_AO_SYNC_LINKER_HOOK);
    }
    else
    {
        if (SSOS_DEF_OK != PTREE_NLINKER_ASYNC_Init(&aoModIn->asyncLinker, 0, 10))
        {
            goto ERR_LINKER_INIT;
        }
        PTREE_NLINKER_ASYNC_Register(&aoModIn->asyncLinker, &G_PTREE_MOD_AO_ASYNC_LINKER_HOOK);
    }

    PTREE_MOD_InObjRegister(&aoModIn->base, &G_PTREE_MOD_AO_IN_HOOK);
    return &aoModIn->base;

ERR_LINKER_INIT:
    PTREE_MOD_InObjDel(&aoModIn->base);
ERR_MOD_IN_INIT:
    SSOS_MEM_Free(aoModIn);
ERR_MEM_ALLOC:
    return NULL;
}

static int _PTREE_MOD_AO_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    MI_S32               s32Ret = MI_SUCCESS;
    MI_AO_Attr_t         stAoSetAttr;
    PTREE_SUR_AO_Info_t *aoInfo = NULL;
    MI_AUDIO_I2sConfig_t stAoI2sACfg;

    aoInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_AO_Info_t, base.base);

    memset(&stAoSetAttr, 0x0, sizeof(MI_AO_Attr_t));
    memset(&stAoI2sACfg, 0x0, sizeof(stAoI2sACfg));
    stAoSetAttr.enFormat      = aoInfo->eAoFormat;
    stAoSetAttr.enSoundMode   = aoInfo->eAoSoundMode;
    stAoSetAttr.enSampleRate  = (MI_AUDIO_SampleRate_e)aoInfo->u32SampleRate;
    stAoSetAttr.enChannelMode = aoInfo->eChannelMode;
    stAoSetAttr.u32PeriodSize = (MI_U32)aoInfo->u32PeriodSize;
    s32Ret                    = MI_AO_Open((MI_AUDIO_DEV)aoInfo->base.base.devId, &stAoSetAttr);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("AO dev%d create fail", aoInfo->base.base.devId);
        return SSOS_DEF_FAIL;
    }

    if ((aoInfo->enAoIf & E_MI_AO_IF_I2S_A) || (aoInfo->enAoIf & E_MI_AO_IF_I2S_B))
    {
        stAoI2sACfg.enMode       = aoInfo->eAoI2sMode;
        stAoI2sACfg.enFormat     = aoInfo->eAoI2sFormat;
        stAoI2sACfg.enSampleRate = (MI_AUDIO_SampleRate_e)aoInfo->u32SampleRate;
        stAoI2sACfg.enMclk       = aoInfo->eAoI2sMclkE;
        stAoI2sACfg.bSyncClock   = (MI_BOOL)aoInfo->u32I2sSyncclock;
        stAoI2sACfg.u32TdmSlots  = (MI_U32)aoInfo->u32I2sTdmslots;
        stAoI2sACfg.enBitWidth   = aoInfo->eAoI2sBitWidth;
        if (aoInfo->enAoIf & E_MI_AO_IF_I2S_A)
        {
            s32Ret = MI_AO_SetI2SConfig(E_MI_AO_IF_I2S_A, &stAoI2sACfg);
            if (MI_SUCCESS != s32Ret)
            {
                PTREE_ERR("AO I2s set fail");
                return SSOS_DEF_FAIL;
            }
        }
        if (aoInfo->enAoIf & E_MI_AO_IF_I2S_B)
        {
            s32Ret = MI_AO_SetI2SConfig(E_MI_AO_IF_I2S_B, &stAoI2sACfg);
            if (MI_SUCCESS != s32Ret)
            {
                PTREE_ERR("AO I2s set fail");
                return SSOS_DEF_FAIL;
            }
        }
        PTREE_DBG("Interface 0x%x AO set I2S suecess!", aoInfo->enAoIf);
    }
    if (MI_SUCCESS != MI_AO_AttachIf((MI_AUDIO_DEV)aoInfo->base.base.devId, aoInfo->enAoIf, 0))
    {
        PTREE_ERR("AO dev%d attach interface fail", aoInfo->base.base.devId);
        return SSOS_DEF_FAIL;
    }

    if (MI_SUCCESS
        != MI_AO_SetVolume((MI_AUDIO_DEV)aoInfo->base.base.devId, aoInfo->s16Volume, aoInfo->s16Volume,
                           E_MI_AO_GAIN_FADING_OFF))
    {
        PTREE_ERR("AO dev%d SetVolume fail", aoInfo->base.base.devId);
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_AO_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    MI_S32               s32Ret = MI_SUCCESS;
    PTREE_SUR_AO_Info_t *info   = NULL;
    info                        = CONTAINER_OF(sysMod->base.info, PTREE_SUR_AO_Info_t, base.base);

    s32Ret = MI_AO_DetachIf((MI_AUDIO_DEV)info->base.base.devId, info->enAoIf);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("AO DetachIf fail");
        return -1;
    }

    s32Ret = MI_AO_Close((MI_AUDIO_DEV)info->base.base.devId);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_ERR("AO Close fail");
        return -1;
    }

    return 0;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_AO_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    return _PTREE_MOD_AO_InNew(mod, loopId);
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_AO_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static void _PTREE_MOD_AO_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_AO_Obj_t, base));
}

PTREE_MOD_Obj_t *PTREE_MOD_AO_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_AO_Obj_t *aoMod = NULL;

    aoMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_AO_Obj_t));
    if (!aoMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(aoMod, 0, sizeof(PTREE_MOD_AO_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&aoMod->base, &G_PTREE_MOD_AO_SYS_OPS, tag, E_MI_MODULE_ID_AO))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (SSOS_DEF_OK != PTREE_PACKER_Init(&aoMod->packer, &G_PTREE_MOD_AO_PACKER_OPS))
    {
        goto ERR_PACKER_INIT;
    }

    PTREE_PACKER_Register(&aoMod->packer, &G_PTREE_MOD_AO_PACKER_HOOK);
    PTREE_MOD_SYS_ObjRegister(&aoMod->base, &G_PTREE_MOD_AO_SYS_HOOK);
    return &aoMod->base.base;

ERR_PACKER_INIT:
    PTREE_MOD_ObjDel(&aoMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(aoMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(AO, PTREE_MOD_AO_New);
