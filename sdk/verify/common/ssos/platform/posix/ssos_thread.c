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

#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "ssos_thread.h"

int SSOS_THREAD_Create(SSOS_THREAD_Handle_t *pHandle, const SSOS_THREAD_Attr_t *pAttr, void *(*threadFunc)(void *),
                       void *arg)
{
    pthread_t          tid;
    struct sched_param sched;

    if (!pHandle)
    {
        return SSOS_DEF_EINVAL;
    }
    if (pAttr != NULL)
    {
        pthread_attr_t tAttr;
        pthread_attr_init(&tAttr);
        if (pAttr->priority > 70 && pAttr->priority <= 100)
        {
            pthread_attr_getschedparam(&tAttr, &sched);
            pthread_attr_setinheritsched(&tAttr, PTHREAD_EXPLICIT_SCHED);
            pthread_attr_setschedpolicy(&tAttr, SCHED_RR);
            if (pAttr->priority < 95) // nPriority 71~94 mapping to Linux PrioRT 1~94
            {
                sched.sched_priority = (pAttr->priority - 71) * 93 / 23 + 1;
            }
            else // nPriority 95~99 mapping to Linux PrioRT 95~99
            {
                sched.sched_priority = (pAttr->priority < 100) ? pAttr->priority : 99;
            }
            if (0 != pthread_attr_setschedparam(&tAttr, &sched))
            {
                pthread_attr_destroy(&tAttr);
                return SSOS_DEF_FAIL;
            }
        }
        if (0 != pAttr->stackSize)
        {
            if (0 != pthread_attr_setstacksize(&tAttr, (size_t)pAttr->stackSize))
            {
                pthread_attr_destroy(&tAttr);
                return SSOS_DEF_FAIL;
            }
        }
        if (pthread_create(&tid, &tAttr, threadFunc, arg) != 0)
        {
            pthread_attr_destroy(&tAttr);
            return SSOS_DEF_FAIL;
        }
        if (pAttr->name)
        {
            pthread_setname_np(tid, pAttr->name);
        }
        *pHandle = (SSOS_THREAD_Handle_t)tid;
        pthread_attr_destroy(&tAttr);
        return SSOS_DEF_OK;
    }
    if (pthread_create(&tid, NULL, threadFunc, arg) != 0)
    {
        return SSOS_DEF_FAIL;
    }
    *pHandle = (SSOS_THREAD_Handle_t)tid;
    return SSOS_DEF_OK;
}
int SSOS_THREAD_Join(SSOS_THREAD_Handle_t tHandle)
{
    pthread_t tid = (pthread_t)tHandle;
    pthread_join(tid, NULL);
    return SSOS_DEF_OK;
}
int SSOS_THREAD_GetName(SSOS_THREAD_Handle_t tHandle, char *name, unsigned long len)
{
    pthread_t *pTid = tHandle;
    (void)len;
    if (!pTid)
    {
        return SSOS_DEF_EINVAL;
    }
    if (pthread_self() != *pTid)
    {
        return SSOS_DEF_EBUSY;
    }
    return prctl(PR_GET_NAME, name);
}
int SSOS_THREAD_SetName(SSOS_THREAD_Handle_t tHandle, char *name)
{
    pthread_t *pTid = tHandle;
    if (!pTid)
    {
        return SSOS_DEF_EINVAL;
    }
    if (pthread_self() != *pTid)
    {
        return SSOS_DEF_EBUSY;
    }
    return prctl(PR_SET_NAME, name);
}
int SSOS_THREAD_GetPid(void)
{
    return pthread_self();
}

int SSOS_THREAD_MutexInit(SSOS_THREAD_Mutex_t *mutex)
{
    return pthread_mutex_init((pthread_mutex_t *)mutex, NULL);
}
int SSOS_THREAD_MutexLock(SSOS_THREAD_Mutex_t *mutex)
{
    return pthread_mutex_lock((pthread_mutex_t *)mutex);
}
int SSOS_THREAD_MutexTryLock(SSOS_THREAD_Mutex_t *mutex)
{
    return pthread_mutex_trylock((pthread_mutex_t *)mutex);
}
int SSOS_THREAD_MutexUnlock(SSOS_THREAD_Mutex_t *mutex)
{
    return pthread_mutex_unlock((pthread_mutex_t *)mutex);
}
int SSOS_THREAD_MutexDeinit(SSOS_THREAD_Mutex_t *mutex)
{
    return pthread_mutex_destroy((pthread_mutex_t *)mutex);
}
int SSOS_THREAD_RwLockInit(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_init((pthread_rwlock_t *)lock, NULL);
}
int SSOS_THREAD_ReadLock(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_rdlock((pthread_rwlock_t *)lock);
}
int SSOS_THREAD_TryReadLock(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_tryrdlock((pthread_rwlock_t *)lock);
}
int SSOS_THREAD_ReadUnLock(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_unlock((pthread_rwlock_t *)lock);
}
int SSOS_THREAD_TryWriteLock(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_trywrlock((pthread_rwlock_t *)lock);
}
int SSOS_THREAD_WriteLock(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_wrlock((pthread_rwlock_t *)lock);
}
int SSOS_THREAD_WriteUnlock(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_unlock((pthread_rwlock_t *)lock);
}
int SSOS_THREAD_RwLockDeinit(SSOS_THREAD_RwLock_t *lock)
{
    return pthread_rwlock_destroy((pthread_rwlock_t *)lock);
}
