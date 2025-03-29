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
#include "mi_isp_ae.h"
#include "mi_isp_cus3a_api.h"
#include "ssos_def.h"
#include "ssos_list.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_message.h"
#include "ptree_mod.h"
#include "ptree_rgn_packet.h"
#include "ptree_sur.h"
#include "ssos_task.h"
#include "ptree_sur_aestable.h"
#include "ssos_time.h"
#include "ptree_mod_aestable.h"
#include "ptree_maker.h"
#include "ptree_mod_sys.h"
#include "ptree_packer_bypass.h"

#define AESTABLE_PIPELINE_DEBUG 0

typedef struct PTREE_MOD_AESTABLE_Fsm_s PTREE_MOD_AESTABLE_Fsm_t;
struct PTREE_MOD_AESTABLE_Fsm_s
{
    unsigned int                        ispDevId;
    unsigned int                        ispChnId;
    enum PTREE_SUR_AESTABLE_StartMode_e startMode;
    enum PTREE_SUR_AESTABLE_RunMode_e   runMode;
    unsigned int                        stableThreshold;
    unsigned int                        captureCount; /* Captrue only */
    unsigned int                        lastAeCount;
    unsigned int                        diffAeCount;
    unsigned int                        unstableCount;
    unsigned char (*stateMachineCb)(PTREE_MOD_AESTABLE_Fsm_t*);
};

typedef struct PTREE_MOD_AESTABLE_Obj_s PTREE_MOD_AESTABLE_Obj_t;
struct PTREE_MOD_AESTABLE_Obj_s
{
    PTREE_MOD_SYS_Obj_t       base;
    PTREE_LINKER_Obj_t        inLinker;
    PTREE_MOD_AESTABLE_Fsm_t  fsm;
    SSOS_THREAD_Mutex_t       fsmMutex;
    PTREE_MOD_InObj_t         modIn;
    PTREE_MOD_OutObj_t        modOut;
    void*                     taskHandle;
    unsigned char             usingLowPower;
    unsigned char             bPowerOn;
    PTREE_PACKER_BYPASS_Obj_t bypassPacker;
};

struct PTREE_MOD_AESTABLE_EventSharedData_s
{
    PTREE_MOD_InObj_t* modIn;
    unsigned char      bPowerOn;
};

enum PTREE_MOD_AESTABLE_PowerEvent_e
{
    E_PTREE_MOD_AESTABLE_POWER_OFF,
};

static void* _PTREE_MOD_AESTABLE_EventHandler(struct SSOS_TASK_Buffer_s* buf, struct SSOS_TASK_UserData_s* data);
static int   _PTREE_MOD_AESTABLE_EventPowerOff(PTREE_MOD_AESTABLE_Obj_t* aestableMod);
static int   _PTREE_MOD_AESTABLE_CreatePowerHandler(PTREE_MOD_AESTABLE_Obj_t* aestableMod);
static int   _PTREE_MOD_AESTABLE_DestroyPowerHandler(PTREE_MOD_AESTABLE_Obj_t* aestableMod);
static int   _PTREE_MOD_AESTABLE_PowerInit(PTREE_MOD_AESTABLE_Obj_t* aestableMod);
static int   _PTREE_MOD_AESTABLE_PowerDeinit(PTREE_MOD_AESTABLE_Obj_t* aestableMod);
static int   _PTREE_MOD_AESTABLE_PowerOn(PTREE_MOD_AESTABLE_Obj_t* aestableMod);
static int   _PTREE_MOD_AESTABLE_PowerOff(PTREE_MOD_AESTABLE_Obj_t* aestableMod);

static int _PTREE_MOD_AESTABLE_InputLinkerEnqueue(PTREE_LINKER_Obj_t* linker, PTREE_PACKET_Obj_t* packet);
static PTREE_PACKET_Obj_t* _PTREE_MOD_AESTABLE_InputLinkerDequeue(PTREE_LINKER_Obj_t* linker, int ms);

static int                 _PTREE_MOD_AESTABLE_Init(PTREE_MOD_SYS_Obj_t* sysMod);
static int                 _PTREE_MOD_AESTABLE_Prepare(PTREE_MOD_SYS_Obj_t* sysMod);
static int                 _PTREE_MOD_AESTABLE_Unprepare(PTREE_MOD_SYS_Obj_t* sysMod);
static int                 _PTREE_MOD_AESTABLE_OutLinked(PTREE_MOD_OutObj_t* modOut, unsigned int ref);
static int                 _PTREE_MOD_AESTABLE_OutUnlinked(PTREE_MOD_OutObj_t* modOut, unsigned int ref);
static PTREE_MOD_InObj_t*  _PTREE_MOD_AESTABLE_CreateModIn(PTREE_MOD_Obj_t* mod, unsigned int loopId);
static PTREE_MOD_OutObj_t* _PTREE_MOD_AESTABLE_CreateModOut(PTREE_MOD_Obj_t* mod, unsigned int loopId);
static void                _PTREE_MOD_AESTABLE_Destruct(PTREE_MOD_SYS_Obj_t* sysMod);
static void                _PTREE_MOD_AESTABLE_Free(PTREE_MOD_SYS_Obj_t* sysMod);

static int _PTREE_MOD_AESTABLE_UpdateAeCount(PTREE_MOD_AESTABLE_Obj_t* aestableMod);

static int                 _PTREE_MOD_AESTABLE_InGetType(PTREE_MOD_InObj_t* modIn);
static PTREE_PACKER_Obj_t* _PTREE_MOD_AESTABLE_InCreatePacker(PTREE_MOD_InObj_t* modIn, int* isFast);
static PTREE_LINKER_Obj_t* _PTREE_MOD_AESTABLE_InCreateNLinker(PTREE_MOD_InObj_t* modIn);

static int _PTREE_MOD_AESTABLE_OutGetType(PTREE_MOD_OutObj_t* modOut);

static const PTREE_LINKER_Ops_t G_PTREE_MOD_AESTABLE_LINKER_OPS = {
    .enqueue = _PTREE_MOD_AESTABLE_InputLinkerEnqueue,
    .dequeue = _PTREE_MOD_AESTABLE_InputLinkerDequeue,
};
static const PTREE_LINKER_Hook_t G_PTREE_MOD_AESTABLE_LINKER_HOOK = {};

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_AESTABLE_SYS_OPS = {
    .init         = _PTREE_MOD_AESTABLE_Init,
    .prepare      = _PTREE_MOD_AESTABLE_Prepare,
    .unprepare    = _PTREE_MOD_AESTABLE_Unprepare,
    .createModIn  = _PTREE_MOD_AESTABLE_CreateModIn,
    .createModOut = _PTREE_MOD_AESTABLE_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_AESTABLE_SYS_HOOK = {
    .destruct = _PTREE_MOD_AESTABLE_Destruct,
    .free     = _PTREE_MOD_AESTABLE_Free,
};

static const PTREE_MOD_InOps_t G_PTREE_MOD_AESTABLE_IN_OPS = {
    .getType       = _PTREE_MOD_AESTABLE_InGetType,
    .createPacker  = _PTREE_MOD_AESTABLE_InCreatePacker,
    .createNLinker = _PTREE_MOD_AESTABLE_InCreateNLinker,
};
static const PTREE_MOD_InHook_t G_PTREE_MOD_AESTABLE_IN_HOOK = {};

static const PTREE_MOD_OutOps_t G_PTREE_MOD_AESTABLE_OUT_OPS = {
    .getType  = _PTREE_MOD_AESTABLE_OutGetType,
    .linked   = _PTREE_MOD_AESTABLE_OutLinked,
    .unlinked = _PTREE_MOD_AESTABLE_OutUnlinked,
};
static const PTREE_MOD_OutHook_t G_PTREE_MOD_AESTABLE_OUT_HOOK = {};

static unsigned char _PTREE_MOD_AESTABLE_AeCountingStart(PTREE_MOD_AESTABLE_Fsm_t* pFsm);
static unsigned char _PTREE_MOD_AESTABLE_ForceModeUnstable(PTREE_MOD_AESTABLE_Fsm_t* pFsm);
static unsigned char _PTREE_MOD_AESTABLE_AutoModeUnstable(PTREE_MOD_AESTABLE_Fsm_t* pFsm);
static unsigned char _PTREE_MOD_AESTABLE_ModeStable(PTREE_MOD_AESTABLE_Fsm_t* pFsm);
static unsigned char _PTREE_MOD_AESTABLE_ShotModeIdle(PTREE_MOD_AESTABLE_Fsm_t* pFsm);
static unsigned char _PTREE_MOD_AESTABLE_RecordModeIdle(PTREE_MOD_AESTABLE_Fsm_t* pFsm);

static int _PTREE_MOD_AESTABLE_UpdateAeCount(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    SSOS_THREAD_MutexLock(&aestableMod->fsmMutex);
    if (aestableMod->usingLowPower || aestableMod->fsm.runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD)
    {
        aestableMod->fsm.lastAeCount = 0;
        SSOS_THREAD_MutexUnlock(&aestableMod->fsmMutex);
        return SSOS_DEF_OK;
    }
    if (MI_SUCCESS
        != MI_ISP_CUS3A_GetDoAeCount(aestableMod->fsm.ispDevId, aestableMod->fsm.ispChnId,
                                     (MI_U32*)&aestableMod->fsm.lastAeCount))
    {
        SSOS_THREAD_MutexUnlock(&aestableMod->fsmMutex);
        PTREE_ERR("MI_ISP_CUS3A_GetDoAeCount failed\n");
        return SSOS_DEF_FAIL;
    }
    SSOS_THREAD_MutexUnlock(&aestableMod->fsmMutex);
    return SSOS_DEF_OK;
}

static unsigned char _PTREE_MOD_AESTABLE_AeCountingStart(PTREE_MOD_AESTABLE_Fsm_t* pFsm)
{
    pFsm->diffAeCount   = 0;
    pFsm->unstableCount = 0;
#if AESTABLE_PIPELINE_DEBUG
    PTREE_DBG("ae counting start.");
#endif
    if (pFsm->startMode == E_PTREE_SUR_AESTABLE_START_MODE_AUTO)
    {
        pFsm->stateMachineCb = _PTREE_MOD_AESTABLE_AutoModeUnstable;
        return pFsm->stateMachineCb(pFsm);
    }
    if (pFsm->startMode == E_PTREE_SUR_AESTABLE_START_MODE_FORCE)
    {
        pFsm->stateMachineCb = _PTREE_MOD_AESTABLE_ForceModeUnstable;
        return pFsm->stateMachineCb(pFsm);
    }
    PTREE_ERR("Start mode %d error!\n", pFsm->startMode);
    return SSOS_DEF_FAIL;
}

unsigned char _PTREE_MOD_AESTABLE_ForceModeUnstable(PTREE_MOD_AESTABLE_Fsm_t* pFsm)
{
    pFsm->stateMachineCb = _PTREE_MOD_AESTABLE_ModeStable;
    return pFsm->stateMachineCb(pFsm);
}

unsigned char _PTREE_MOD_AESTABLE_AutoModeUnstable(PTREE_MOD_AESTABLE_Fsm_t* pFsm)
{
    MI_U32 aeCount = 0;

    if (MI_SUCCESS != MI_ISP_CUS3A_GetDoAeCount(pFsm->ispDevId, pFsm->ispChnId, &aeCount))
    {
        PTREE_ERR("MI_ISP_CUS3A_GetDoAeCount failed\n");
        return 0;
    }
    pFsm->diffAeCount = aeCount - pFsm->lastAeCount;
    if (pFsm->diffAeCount)
    {
        MI_ISP_AE_ExpoInfoType_t stAEExpoInfo;
        memset(&stAEExpoInfo, 0, sizeof(MI_ISP_AE_ExpoInfoType_t));
        if (MI_SUCCESS != MI_ISP_AE_QueryExposureInfo(pFsm->ispDevId, pFsm->ispChnId, &stAEExpoInfo))
        {
            PTREE_ERR("MI_ISP_AE_QueryExposureInfo failed");
            return 0;
        }
        if (stAEExpoInfo.bIsStable == E_SS_AE_TRUE)
        {
#if AESTABLE_PIPELINE_DEBUG
            PTREE_DBG("ae check stable.");
#endif
            pFsm->stateMachineCb = _PTREE_MOD_AESTABLE_ModeStable;
            return pFsm->stateMachineCb(pFsm);
        }
    }
#if 1 // AESTABLE_PIPELINE_DEBUG
    PTREE_LOG("ae not stable ae cout diff %d time %luus.", aeCount - pFsm->lastAeCount, PTREE_MOD_GetTimer());
#endif
    pFsm->unstableCount++;
    return 0;
}

unsigned char _PTREE_MOD_AESTABLE_ModeStable(PTREE_MOD_AESTABLE_Fsm_t* pFsm)
{
    if (pFsm->stableThreshold)
    {
#if AESTABLE_PIPELINE_DEBUG
        PTREE_DBG("stable threshold count %d", pFsm->stableThreshold);
#endif
        pFsm->stableThreshold--;
        return 0;
    }
    if (pFsm->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_SHOT)
    {
        if (pFsm->captureCount)
        {
            PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(pFsm, PTREE_MOD_AESTABLE_Obj_t, fsm);
#if AESTABLE_PIPELINE_DEBUG
            PTREE_DBG("capture count %d", pFsm->captureCount);
#endif
            pFsm->captureCount--;
            if (!pFsm->captureCount && aestableMod->usingLowPower && aestableMod->taskHandle)
            {
                PTREE_DBG("Aestable will stop previous task.");
                PTREE_MESSAGE_Suspend(&aestableMod->modIn.message);
                _PTREE_MOD_AESTABLE_EventPowerOff(aestableMod);
            }
            PTREE_LOG("start cauptrue left %d doAeCntDiff: %d, unstable count %d, time: %luus", pFsm->captureCount,
                      pFsm->diffAeCount, pFsm->unstableCount, PTREE_MOD_GetTimer());
            pFsm->diffAeCount   = 0;
            pFsm->unstableCount = 0;
            return 1;
        }
        pFsm->stateMachineCb = _PTREE_MOD_AESTABLE_ShotModeIdle;
        return pFsm->stateMachineCb(pFsm);
    }
    if (pFsm->runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD)
    {
#if AESTABLE_PIPELINE_DEBUG
        PTREE_DBG("recording start...\n");
#endif
        PTREE_LOG("start record doAeCntDiff: %d, unstable count %d, time: %luus", pFsm->diffAeCount,
                  pFsm->unstableCount, PTREE_MOD_GetTimer());
        pFsm->diffAeCount    = 0;
        pFsm->unstableCount  = 0;
        pFsm->stateMachineCb = _PTREE_MOD_AESTABLE_RecordModeIdle;
        return pFsm->stateMachineCb(pFsm);
    }
    PTREE_ERR("Run mode %d error!\n", pFsm->runMode);
    return 0;
}

unsigned char _PTREE_MOD_AESTABLE_ShotModeIdle(PTREE_MOD_AESTABLE_Fsm_t* pFsm)
{
    (void)pFsm;
#if AESTABLE_PIPELINE_DEBUG
    PTREE_DBG("capture idle...");
#endif
    return 0;
}

unsigned char _PTREE_MOD_AESTABLE_RecordModeIdle(PTREE_MOD_AESTABLE_Fsm_t* pFsm)
{
    (void)pFsm;
#if AESTABLE_PIPELINE_DEBUG
    PTREE_DBG("record idle...");
#endif
    return 1;
}

static int _PTREE_MOD_AESTABLE_Init(PTREE_MOD_SYS_Obj_t* sysMod)
{
    return PTREE_MOD_AESTABLE_Reset(&sysMod->base);
}

static int _PTREE_MOD_AESTABLE_Prepare(PTREE_MOD_SYS_Obj_t* sysMod)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(sysMod, PTREE_MOD_AESTABLE_Obj_t, base);
    return _PTREE_MOD_AESTABLE_PowerInit(aestableMod);
}

static int _PTREE_MOD_AESTABLE_Unprepare(PTREE_MOD_SYS_Obj_t* sysMod)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(sysMod, PTREE_MOD_AESTABLE_Obj_t, base);
    if (!aestableMod->taskHandle)
    {
        if (_PTREE_MOD_AESTABLE_PowerOff(aestableMod) != SSOS_DEF_OK)
        {
            return SSOS_DEF_FAIL;
        }
    }
    return _PTREE_MOD_AESTABLE_PowerDeinit(aestableMod);
}

static PTREE_MOD_InObj_t* _PTREE_MOD_AESTABLE_CreateModIn(PTREE_MOD_Obj_t* mod, unsigned int loopId)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(mod, PTREE_MOD_AESTABLE_Obj_t, base.base);
    (void)loopId;
    return PTREE_MOD_InObjDup(&aestableMod->modIn);
}

static void* _PTREE_MOD_AESTABLE_EventHandler(struct SSOS_TASK_Buffer_s* buf, struct SSOS_TASK_UserData_s* data)
{
    enum PTREE_MOD_AESTABLE_PowerEvent_e event       = E_PTREE_MOD_AESTABLE_POWER_OFF;
    PTREE_MOD_AESTABLE_Obj_t*            aestableMod = NULL;

    if (!buf | !data)
    {
        PTREE_ERR("Mod is null!");
        return NULL;
    }
    aestableMod = (PTREE_MOD_AESTABLE_Obj_t*)buf->buf;
    event       = (enum PTREE_MOD_AESTABLE_PowerEvent_e)(unsigned long)data->data;
    switch (event)
    {
        case E_PTREE_MOD_AESTABLE_POWER_OFF:
            _PTREE_MOD_AESTABLE_PowerOff(aestableMod);
            break;
        default:
            PTREE_ERR("ERR event : %d", event);
            break;
    }
    return NULL;
}

static int _PTREE_MOD_AESTABLE_EventPowerOff(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    SSOS_TASK_UserData_t eventData;

    if (!aestableMod->taskHandle)
    {
        PTREE_ERR("Event handler not exist.");
        return SSOS_DEF_FAIL;
    }
    eventData.size     = 0;
    eventData.realSize = 0;
    eventData.data     = (void*)E_PTREE_MOD_AESTABLE_POWER_OFF;
    if (SSOS_TASK_Send(aestableMod->taskHandle, &eventData) != 0)
    {
        PTREE_ERR("event send error.");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_AESTABLE_PowerInit(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    if (PTREE_MOD_CreateDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("create delay pass error, pls check you config whether has 'PASS' module in previous!");
        return SSOS_DEF_FAIL;
    }
    if (PTREE_MOD_InitDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("Init delay pass error!");
        PTREE_MOD_DestroyDelayPass(&aestableMod->modIn);
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}
static int _PTREE_MOD_AESTABLE_PowerDeinit(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    if (PTREE_MOD_DeinitDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("Deinit delay pass error!");
        return SSOS_DEF_FAIL;
    }
    if (PTREE_MOD_DestroyDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("destroy delay pass error!");
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_AESTABLE_PowerOn(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    if (aestableMod->bPowerOn)
    {
        return SSOS_DEF_OK;
    }
    if (PTREE_MOD_BindDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("Bind delay pass error!");
        return SSOS_DEF_FAIL;
    }
    if (PTREE_MOD_StartDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("Start delay pass error!");
        PTREE_MOD_UnbindDelayPass(&aestableMod->modIn);
        return SSOS_DEF_FAIL;
    }
    aestableMod->bPowerOn = 1;
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_AESTABLE_PowerOff(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    if (!aestableMod->bPowerOn)
    {
        return SSOS_DEF_OK;
    }
    if (PTREE_MOD_StopDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("Stop delay pass error!");
        return SSOS_DEF_FAIL;
    }
    if (PTREE_MOD_UnbindDelayPass(&aestableMod->modIn) != SSOS_DEF_OK)
    {
        PTREE_ERR("Unbind delay pass error!");
        return SSOS_DEF_FAIL;
    }
    aestableMod->bPowerOn = 0;
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_AESTABLE_CreatePowerHandler(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    SSOS_TASK_Attr_t taskAttr;

    // TO Do : powner on sensor.
    memset(&taskAttr, 0, sizeof(SSOS_TASK_Attr_t));
    taskAttr.doSignal       = _PTREE_MOD_AESTABLE_EventHandler;
    taskAttr.inBuf.buf      = (void*)aestableMod;
    aestableMod->taskHandle = SSOS_TASK_Open(&taskAttr);
    if (!aestableMod->taskHandle)
    {
        PTREE_ERR("Event task open fail!");
        return SSOS_DEF_FAIL;
    }
    aestableMod->usingLowPower = 1;
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_AESTABLE_DestroyPowerHandler(PTREE_MOD_AESTABLE_Obj_t* aestableMod)
{
    if (_PTREE_MOD_AESTABLE_EventPowerOff(aestableMod) != SSOS_DEF_OK)
    {
        PTREE_ERR("power off error.");
        return SSOS_DEF_FAIL;
    }
    if (SSOS_TASK_Close(aestableMod->taskHandle) != 0)
    {
        PTREE_ERR("event task error.");
        return SSOS_DEF_FAIL;
    }
    aestableMod->taskHandle    = NULL;
    aestableMod->usingLowPower = 0;
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_AESTABLE_OutLinked(PTREE_MOD_OutObj_t* modOut, unsigned int ref)
{
    PTREE_MOD_AESTABLE_Obj_t*  aestableMod = NULL;
    PTREE_SUR_AESTABLE_Info_t* info        = NULL;

    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    info        = CONTAINER_OF(modOut->thisMod->info, PTREE_SUR_AESTABLE_Info_t, base);
    aestableMod = CONTAINER_OF(modOut->thisMod, PTREE_MOD_AESTABLE_Obj_t, base.base);
    if (_PTREE_MOD_AESTABLE_PowerOn(aestableMod) != SSOS_DEF_OK)
    {
        PTREE_ERR("Handle error in powner on.");
        return SSOS_DEF_FAIL;
    }
    if (!info->usingLowPower)
    {
        PTREE_LOG("Low power mode is off, start counting %luus", PTREE_MOD_GetTimer());
        return SSOS_DEF_OK;
    }
    if (_PTREE_MOD_AESTABLE_CreatePowerHandler(aestableMod) != SSOS_DEF_OK)
    {
        return SSOS_DEF_FAIL;
    }
    PTREE_LOG("Start aestable counting... time %luus", PTREE_MOD_GetTimer());
    return SSOS_DEF_OK;
}

static int _PTREE_MOD_AESTABLE_OutUnlinked(PTREE_MOD_OutObj_t* modOut, unsigned int ref)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = NULL;
    if (ref > 0)
    {
        return SSOS_DEF_OK;
    }
    aestableMod = CONTAINER_OF(modOut->thisMod, PTREE_MOD_AESTABLE_Obj_t, base.base);
    if (_PTREE_MOD_AESTABLE_UpdateAeCount(aestableMod) != SSOS_DEF_OK)
    {
        PTREE_ERR("Handle error in update aecount.");
        return SSOS_DEF_FAIL;
    }
    if (aestableMod->fsm.runMode == E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD)
    {
        _PTREE_MOD_AESTABLE_PowerOff(aestableMod);
    }
    PTREE_MOD_AESTABLE_Reset(modOut->thisMod);
    if (!aestableMod->usingLowPower)
    {
        return SSOS_DEF_OK;
    }
    if (_PTREE_MOD_AESTABLE_DestroyPowerHandler(aestableMod) != SSOS_DEF_OK)
    {
        return SSOS_DEF_FAIL;
    }
    return SSOS_DEF_OK;
}

static PTREE_MOD_OutObj_t* _PTREE_MOD_AESTABLE_CreateModOut(PTREE_MOD_Obj_t* mod, unsigned int loopId)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(mod, PTREE_MOD_AESTABLE_Obj_t, base.base);
    (void)loopId;
    return PTREE_MOD_OutObjDup(&aestableMod->modOut);
}

static int _PTREE_MOD_AESTABLE_InputLinkerEnqueue(PTREE_LINKER_Obj_t* linker, PTREE_PACKET_Obj_t* packet)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(linker, PTREE_MOD_AESTABLE_Obj_t, inLinker);
    unsigned char             bSendPacket = 0;
    SSOS_THREAD_MutexLock(&aestableMod->fsmMutex);
    bSendPacket = aestableMod->fsm.stateMachineCb(&aestableMod->fsm);
    SSOS_THREAD_MutexUnlock(&aestableMod->fsmMutex);
    if (bSendPacket)
    {
        PTREE_LINKER_Enqueue(&aestableMod->modOut.plinker.base, packet);
    }
    return SSOS_DEF_OK;
}

static PTREE_PACKET_Obj_t* _PTREE_MOD_AESTABLE_InputLinkerDequeue(PTREE_LINKER_Obj_t* linker, int ms)
{
    (void)linker;
    (void)ms;
    return NULL;
}

static PTREE_LINKER_Obj_t* _PTREE_MOD_AESTABLE_InCreateNLinker(PTREE_MOD_InObj_t* modIn)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(modIn, PTREE_MOD_AESTABLE_Obj_t, modIn);
    return PTREE_LINKER_Dup(&aestableMod->inLinker);
}

static PTREE_PACKER_Obj_t* _PTREE_MOD_AESTABLE_InCreatePacker(PTREE_MOD_InObj_t* modIn, int* isFast)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(modIn, PTREE_MOD_AESTABLE_Obj_t, modIn);
    if (aestableMod->modOut.bindCount)
    {
        PTREE_PACKER_BYPASS_SetTarget(&aestableMod->bypassPacker, &aestableMod->modOut.packer.base);
    }
    *isFast = SSOS_DEF_FALSE;
    return PTREE_PACKER_Dup(&aestableMod->bypassPacker.base);
}

static int _PTREE_MOD_AESTABLE_InGetType(PTREE_MOD_InObj_t* modIn)
{
    (void)modIn;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static int _PTREE_MOD_AESTABLE_OutGetType(PTREE_MOD_OutObj_t* modOut)
{
    (void)modOut;
    return E_PTREE_MOD_BIND_TYPE_USER;
}

static void _PTREE_MOD_AESTABLE_Destruct(PTREE_MOD_SYS_Obj_t* sysMod)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = CONTAINER_OF(sysMod, PTREE_MOD_AESTABLE_Obj_t, base);
    SSOS_THREAD_MutexDeinit(&aestableMod->fsmMutex);
    PTREE_PACKER_Del(&aestableMod->bypassPacker.base);
    PTREE_LINKER_Del(&aestableMod->inLinker);
    PTREE_MOD_OutObjDel(&aestableMod->modOut);
    PTREE_MOD_InObjDel(&aestableMod->modIn);
}

static void _PTREE_MOD_AESTABLE_Free(PTREE_MOD_SYS_Obj_t* sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_AESTABLE_Obj_t, base));
}

static PTREE_MOD_Obj_t* _PTREE_MOD_AESTABLE_New(PARENA_Tag_t* tag)
{
    PTREE_MOD_AESTABLE_Obj_t* aestableMod = NULL;
    aestableMod                           = SSOS_MEM_Alloc(sizeof(PTREE_MOD_AESTABLE_Obj_t));
    if (!aestableMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(aestableMod, 0, sizeof(PTREE_MOD_AESTABLE_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&aestableMod->base, &G_PTREE_MOD_AESTABLE_SYS_OPS, tag, 0))
    {
        goto ERR_MOD_SYS_INIT;
    }
    if (aestableMod->base.base.info->inCnt != 1)
    {
        PTREE_ERR("Aestable mod input count need 1 but %d", aestableMod->base.base.info->inCnt);
        goto ERR_IO_COUNT;
    }
    if (aestableMod->base.base.info->outCnt != 1)
    {
        PTREE_ERR("Aestable mod output count need 1 but %d", aestableMod->base.base.info->outCnt);
        goto ERR_IO_COUNT;
    }
    if (SSOS_DEF_OK
        != PTREE_MOD_InObjInit(&aestableMod->modIn, &G_PTREE_MOD_AESTABLE_IN_OPS, &aestableMod->base.base, 0))
    {
        goto ERR_MOD_IN_INIT;
    }
    if (SSOS_DEF_OK
        != PTREE_MOD_OutObjInit(&aestableMod->modOut, &G_PTREE_MOD_AESTABLE_OUT_OPS, &aestableMod->base.base, 0))
    {
        goto ERR_MOD_OUT_INIT;
    }
    if (SSOS_DEF_OK != PTREE_LINKER_Init(&aestableMod->inLinker, &G_PTREE_MOD_AESTABLE_LINKER_OPS))
    {
        goto ERR_IN_LINKER_INIT;
    }
    if (SSOS_DEF_OK != PTREE_PACKER_BYPASS_Init(&aestableMod->bypassPacker))
    {
        goto ERR_PACKER_INIT;
    }
    PTREE_MOD_InObjRegister(&aestableMod->modIn, &G_PTREE_MOD_AESTABLE_IN_HOOK);
    PTREE_MOD_OutObjRegister(&aestableMod->modOut, &G_PTREE_MOD_AESTABLE_OUT_HOOK);
    PTREE_LINKER_Register(&aestableMod->inLinker, &G_PTREE_MOD_AESTABLE_LINKER_HOOK);
    PTREE_MOD_SYS_ObjRegister(&aestableMod->base, &G_PTREE_MOD_AESTABLE_SYS_HOOK);
    SSOS_THREAD_MutexInit(&aestableMod->fsmMutex);
    aestableMod->fsm.ispDevId = aestableMod->base.base.info->devId;
    aestableMod->fsm.ispChnId = aestableMod->base.base.info->chnId;
    return &aestableMod->base.base;

ERR_PACKER_INIT:
    PTREE_LINKER_Del(&aestableMod->inLinker);
ERR_IN_LINKER_INIT:
    PTREE_MOD_OutObjDel(&aestableMod->modOut);
ERR_MOD_OUT_INIT:
    PTREE_MOD_InObjDel(&aestableMod->modIn);
ERR_MOD_IN_INIT:
ERR_IO_COUNT:
    PTREE_MOD_ObjDel(&aestableMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(aestableMod);
ERR_MEM_ALLOC:
    return NULL;
}

int PTREE_MOD_AESTABLE_Reset(PTREE_MOD_Obj_t* mod)
{
    PTREE_SUR_AESTABLE_Info_t* info        = CONTAINER_OF(mod->info, PTREE_SUR_AESTABLE_Info_t, base);
    PTREE_MOD_AESTABLE_Obj_t*  aestableMod = CONTAINER_OF(mod, PTREE_MOD_AESTABLE_Obj_t, base.base);

    SSOS_THREAD_MutexLock(&aestableMod->fsmMutex);
    aestableMod->fsm.captureCount    = info->captureCount;
    aestableMod->fsm.stableThreshold = info->stableCount;
    aestableMod->fsm.startMode       = info->startMode;
    aestableMod->fsm.runMode         = info->runMode;
    aestableMod->fsm.stateMachineCb  = _PTREE_MOD_AESTABLE_AeCountingStart;
    SSOS_THREAD_MutexUnlock(&aestableMod->fsmMutex);
    return SSOS_DEF_OK;
}

PTREE_MAKER_MOD_INIT(AESTABLE, _PTREE_MOD_AESTABLE_New);
