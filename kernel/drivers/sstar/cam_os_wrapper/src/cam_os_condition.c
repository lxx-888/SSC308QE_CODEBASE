/*
 * cam_os_condition.c - Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_condition.c
/// @brief     Cam OS Condition Source File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK
#include "sys_MsWrapper_cus_os_sem.h"
#elif defined(CAM_OS_LINUX_USER)
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#elif defined(CAM_OS_LINUX_KERNEL)
#include "linux/semaphore.h"
#include "linux/jiffies.h"
#endif

#include "cam_os_condition.h"
#include "cam_os_wrapper.h"

typedef struct
{
    struct CamOsListHead_t tListHead;
    CamOsSpinlock_t        tListLock;
} _CamOsCondition_t;

typedef struct
{
    struct CamOsListHead_t tList;
#ifdef CAM_OS_RTK
    Ms_DynSemaphor_t tSem;
#elif defined(CAM_OS_LINUX_USER)
    sem_t tSem;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct semaphore tSem;
#endif
} _CamOsConditionToken_t;

#ifdef CAM_OS_RTK
#define _CAM_OS_CONDITION_LIST_LOCK_INIT(x)
#define _CAM_OS_CONDITION_LIST_LOCK(x)   taskENTER_CRITICAL();
#define _CAM_OS_CONDITION_LIST_UNLOCK(x) taskEXIT_CRITICAL();
#define _CAM_OS_CONDITION_LIST_LOCK_DEINIT(x)
#else
#define _CAM_OS_CONDITION_LIST_LOCK_INIT(x)   CamOsSpinInit(x);
#define _CAM_OS_CONDITION_LIST_LOCK(x)        CamOsSpinLockIrqSave(x);
#define _CAM_OS_CONDITION_LIST_UNLOCK(x)      CamOsSpinUnlockIrqRestore(x);
#define _CAM_OS_CONDITION_LIST_LOCK_DEINIT(x) CamOsSpinDeinit(x);
#endif

void CamOsConditionInit(CamOsCondition_t *ptCondition)
{
    _CamOsCondition_t *_ptCondition = CamOsMemCallocAtomic(sizeof(_CamOsCondition_t), 1);

    if (!_ptCondition)
    {
        CamOsPanic("Condition NULL handle\n");
    }

    _CAM_OS_CONDITION_LIST_LOCK_INIT(&(_ptCondition)->tListLock);
    CAM_OS_INIT_LIST_HEAD(&(_ptCondition)->tListHead);

    *ptCondition = _ptCondition;
    return;
}

void CamOsConditionDeinit(CamOsCondition_t *ptCondition)
{
    _CamOsCondition_t *_ptCondition = (_CamOsCondition_t *)*ptCondition;

    if (!_ptCondition)
    {
        CamOsPanic("Condition NULL handle\n");
    }

    _CAM_OS_CONDITION_LIST_LOCK_DEINIT(&(_ptCondition)->tListLock);
    CamOsMemRelease(*ptCondition);

    return;
}

void CamOsConditionWakeUpAll(CamOsCondition_t *ptCondition)
{
    _CamOsCondition_t *     _ptCondition = (_CamOsCondition_t *)*ptCondition;
    _CamOsConditionToken_t *_ptToken;

    if (!_ptCondition)
    {
        CamOsPanic("Condition NULL handle\n");
    }

    _CAM_OS_CONDITION_LIST_LOCK(&(_ptCondition)->tListLock);
    CAM_OS_LIST_FOR_EACH_ENTRY(_ptToken, &(_ptCondition)->tListHead, tList)
    {
#ifdef CAM_OS_RTK
        MsProduceDynSem(&(_ptToken)->tSem);
#elif defined(CAM_OS_LINUX_USER)
        sem_post(&(_ptToken)->tSem);
#elif defined(CAM_OS_LINUX_KERNEL)
        up(&(_ptToken)->tSem);
#endif
    }
    _CAM_OS_CONDITION_LIST_UNLOCK(&(_ptCondition)->tListLock);
}

CamOsConditionToken_t _CamOsConditionTokenAlloc(CamOsCondition_t *ptCondition)
{
    _CamOsCondition_t *     _ptCondition = (_CamOsCondition_t *)*ptCondition;
    _CamOsConditionToken_t *_ptToken     = CamOsMemCallocAtomic(sizeof(_CamOsConditionToken_t), 1);

    if (!_ptCondition)
    {
        CamOsPanic("Condition NULL handle\n");
    }

    if (!_ptToken)
    {
        CamOsPanic("Condition NULL token\n");
    }

    CAM_OS_INIT_LIST_HEAD(&(_ptToken)->tList);
#ifdef CAM_OS_RTK
    MsCreateDynSemExtend(&(_ptToken)->tSem, CAM_OS_MAX_INT - 1, 0);
#elif defined(CAM_OS_LINUX_USER)
    sem_init(&(_ptToken)->tSem, 1, 0);
#elif defined(CAM_OS_LINUX_KERNEL)
    sema_init(&(_ptToken)->tSem, 0);
#endif

    _CAM_OS_CONDITION_LIST_LOCK(&(_ptCondition)->tListLock);
    CAM_OS_LIST_ADD_TAIL(&(_ptToken)->tList, &(_ptCondition)->tListHead);
    _CAM_OS_CONDITION_LIST_UNLOCK(&(_ptCondition)->tListLock);

    return (CamOsConditionToken_t)_ptToken;
}

void _CamOsConditionTokenWait(CamOsConditionToken_t tToken)
{
    _CamOsConditionToken_t *_ptToken = (_CamOsConditionToken_t *)tToken;

    if (!_ptToken)
    {
        CamOsPanic("Condition NULL token\n");
    }

#ifdef CAM_OS_RTK
    MsConsumeDynSem(&(_ptToken)->tSem);
#elif defined(CAM_OS_LINUX_USER)
    sem_wait(&(_ptToken)->tSem);
#elif defined(CAM_OS_LINUX_KERNEL)
    down(&(_ptToken)->tSem);
#endif
}

s32 _CamOsConditionTokenTimedWait(CamOsConditionToken_t tToken, u32 nMsec)
{
    _CamOsConditionToken_t *_ptToken = (_CamOsConditionToken_t *)tToken;
    s32                     ret      = 0;

    if (!_ptToken)
    {
        CamOsPanic("Condition NULL token\n");
    }

#ifdef CAM_OS_RTK
    if (MS_NO_MESSAGE == MsConsumeDynSemDelay(&(_ptToken)->tSem, nMsec))
    {
        ret = -1;
    }
#elif defined(CAM_OS_LINUX_USER)
    struct timespec tFinalTime;
    s64             nNanoDelay = 0;

    clock_gettime(CLOCK_REALTIME, &tFinalTime);

    nNanoDelay = (nMsec * 1000000LL) + tFinalTime.tv_nsec;
    tFinalTime.tv_sec += (nNanoDelay / 1000000000LL);
    tFinalTime.tv_nsec = nNanoDelay % 1000000000LL;

    if (0 != sem_timedwait(&(_ptToken)->tSem, &tFinalTime))
    {
        if (errno == ETIMEDOUT)
        {
            ret = -1;
        }
        else
        {
            ret = -2;
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if (0 != down_timeout(&(_ptToken)->tSem, msecs_to_jiffies(nMsec)))
    {
        ret = -1;
    }
#endif

    return ret;
}

void _CamOsConditionTokenFree(CamOsCondition_t *ptCondition, CamOsConditionToken_t tToken)
{
    _CamOsCondition_t *     _ptCondition = (_CamOsCondition_t *)*ptCondition;
    _CamOsConditionToken_t *_ptToken     = (_CamOsConditionToken_t *)tToken;

    if (!_ptCondition)
    {
        CamOsPanic("Condition NULL handle\n");
    }

    if (!_ptToken)
    {
        CamOsPanic("Condition NULL token\n");
    }

    _CAM_OS_CONDITION_LIST_LOCK(&(_ptCondition)->tListLock);
    CAM_OS_LIST_DEL(&(_ptToken)->tList);
    _CAM_OS_CONDITION_LIST_UNLOCK(&(_ptCondition)->tListLock);
#ifdef CAM_OS_RTK
    MsDestroyDynSem(&(_ptToken)->tSem);
#elif defined(CAM_OS_LINUX_USER)
    sem_destroy(&(_ptToken)->tSem);
#elif defined(CAM_OS_LINUX_KERNEL)
    // Do nothing!
#endif
    CamOsMemRelease(_ptToken);
}
