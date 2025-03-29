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

#if defined(__KERNEL__) || defined(CAM_OS_RTK)
#include "ms_platform.h"
#endif
#include "cam_os_wrapper.h"
#include "ssos_thread.h"
#ifndef __KERNEL__
#include <string.h>
#endif

int SSOS_THREAD_Create(SSOS_THREAD_Handle_t *pHandle, const SSOS_THREAD_Attr_t *pAttr, void *(*threadFunc)(void *),
                       void *arg)
{
    CamOsThreadAttrb_t stAttr;
    if (!pAttr)
    {
        return CamOsThreadCreate(pHandle, NULL, threadFunc, arg);
    }
    memset(&stAttr, 0, sizeof(CamOsThreadAttrb_t));
    stAttr.szName     = pAttr->name;
    stAttr.nPriority  = pAttr->priority;
    stAttr.nStackSize = pAttr->stackSize;
#ifdef CAM_OS_RTK
    if (stAttr.nStackSize == 0)
    {
        /* In RTOS default stackSize can not live up with the ALGO's request. */
        stAttr.nStackSize = 16384;
    }
#endif
    return CamOsThreadCreate(pHandle, &stAttr, threadFunc, arg);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_Create);

int SSOS_THREAD_Join(SSOS_THREAD_Handle_t tHandle)
{
    return CamOsThreadJoin(tHandle);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_Join);

int SSOS_THREAD_GetName(SSOS_THREAD_Handle_t tHandle, char *name, unsigned long len)
{
    return CamOsThreadGetName(tHandle, name, len);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_GetName);

int SSOS_THREAD_SetName(SSOS_THREAD_Handle_t tHandle, char *name)
{
#ifdef CAM_OS_RTK
    // return CamOsThreadSetName(tHandle, name);
    // Fixme: Rtos do not allow it in thread callback.
    (void)tHandle;
    (void)name;
    return 0;
#else
    return CamOsThreadSetName(tHandle, name);
#endif
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_SetName);

int SSOS_THREAD_GetPid(void)
{
    return CamOsThreadGetPID();
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_GetPid);

int SSOS_THREAD_MutexInit(SSOS_THREAD_Mutex_t *mutex)
{
    return CamOsMutexInit((CamOsMutex_t *)mutex);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_MutexInit);

int SSOS_THREAD_MutexLock(SSOS_THREAD_Mutex_t *mutex)
{
    return CamOsMutexLock((CamOsMutex_t *)mutex);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_MutexLock);

int SSOS_THREAD_MutexTryLock(SSOS_THREAD_Mutex_t *mutex)
{
    return CamOsMutexTryLock((CamOsMutex_t *)mutex);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_MutexTryLock);

int SSOS_THREAD_MutexUnlock(SSOS_THREAD_Mutex_t *mutex)
{
    return CamOsMutexUnlock((CamOsMutex_t *)mutex);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_MutexUnlock);

int SSOS_THREAD_MutexDeinit(SSOS_THREAD_Mutex_t *mutex)
{
    return CamOsMutexDestroy((CamOsMutex_t *)mutex);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_MutexDeinit);

int SSOS_THREAD_RwLockInit(SSOS_THREAD_RwLock_t *lock)
{
    return CamOsRwsemInit((CamOsRwsem_t *)lock);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_RwLockInit);

int SSOS_THREAD_ReadLock(SSOS_THREAD_RwLock_t *lock)
{
    CamOsRwsemDownRead((CamOsRwsem_t *)lock);
    return SSOS_DEF_OK;
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_ReadLock);

int SSOS_THREAD_TryReadLock(SSOS_THREAD_RwLock_t *lock)
{
    return CamOsRwsemTryDownRead((CamOsRwsem_t *)lock);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_TryReadLock);

int SSOS_THREAD_ReadUnLock(SSOS_THREAD_RwLock_t *lock)
{
    CamOsRwsemUpRead((CamOsRwsem_t *)lock);
    return SSOS_DEF_OK;
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_ReadUnLock);

int SSOS_THREAD_TryWriteLock(SSOS_THREAD_RwLock_t *lock)
{
    return CamOsRwsemTryDownWrite((CamOsRwsem_t *)lock);
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_TryWriteLock);

int SSOS_THREAD_WriteLock(SSOS_THREAD_RwLock_t *lock)
{
    CamOsRwsemDownWrite((CamOsRwsem_t *)lock);
    return SSOS_DEF_OK;
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_WriteLock);

int SSOS_THREAD_WriteUnlock(SSOS_THREAD_RwLock_t *lock)
{
    CamOsRwsemUpWrite((CamOsRwsem_t *)lock);
    return SSOS_DEF_OK;
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_WriteUnlock);

int SSOS_THREAD_RwLockDeinit(SSOS_THREAD_RwLock_t *lock)
{
    CamOsRwsemDeinit((CamOsRwsem_t *)lock);
    return SSOS_DEF_OK;
}
SSOS_EXPORT_SYMBOL(SSOS_THREAD_RwLockDeinit);
