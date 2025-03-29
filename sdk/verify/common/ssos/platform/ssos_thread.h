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

#ifndef __SSOS_THREAD_H__
#define __SSOS_THREAD_H__

#include "ssos_def.h"
#include "ssos_thread_platform.h"

typedef void *SSOS_THREAD_Handle_t;
typedef struct SSOS_THREAD_ThreadAttr_s
{
    unsigned int priority;  /* From 1(lowest) to 99(highest), use OS default priority if set 0 */
    unsigned int stackSize; /* If nStackSize is zero, use OS default value */
    char *       name;
} SSOS_THREAD_Attr_t;

typedef SSOS_THREAD_PLATFORM_Mutex_t  SSOS_THREAD_Mutex_t;
typedef SSOS_THREAD_PLATFORM_RwLock_t SSOS_THREAD_RwLock_t;
typedef SSOS_THREAD_PLATFORM_Cond_t   SSOS_THREAD_Cond_t;

int SSOS_THREAD_Create(SSOS_THREAD_Handle_t *thread, const SSOS_THREAD_Attr_t *attr, void *(*threadHandle)(void *),
                       void *arg);
int SSOS_THREAD_Join(SSOS_THREAD_Handle_t thread);
int SSOS_THREAD_GetName(SSOS_THREAD_Handle_t thread, char *name, unsigned long len);
int SSOS_THREAD_SetName(SSOS_THREAD_Handle_t thread, char *name);
int SSOS_THREAD_GetPid(void);

int SSOS_THREAD_MutexInit(SSOS_THREAD_Mutex_t *mutex);
int SSOS_THREAD_MutexLock(SSOS_THREAD_Mutex_t *mutex);
int SSOS_THREAD_MutexTryLock(SSOS_THREAD_Mutex_t *mutex);
int SSOS_THREAD_MutexUnlock(SSOS_THREAD_Mutex_t *mutex);
int SSOS_THREAD_MutexDeinit(SSOS_THREAD_Mutex_t *mutex);

int SSOS_THREAD_RwLockInit(SSOS_THREAD_RwLock_t *lock);
int SSOS_THREAD_ReadLock(SSOS_THREAD_RwLock_t *lock);
int SSOS_THREAD_TryReadLock(SSOS_THREAD_RwLock_t *lock);
int SSOS_THREAD_ReadUnLock(SSOS_THREAD_RwLock_t *lock);
int SSOS_THREAD_WriteLock(SSOS_THREAD_RwLock_t *lock);
int SSOS_THREAD_TryWriteLock(SSOS_THREAD_RwLock_t *lock);
int SSOS_THREAD_WriteUnlock(SSOS_THREAD_RwLock_t *lock);
int SSOS_THREAD_RwLockDeinit(SSOS_THREAD_RwLock_t *lock);

#define SSOS_THREAD_COND_INIT(cond)                       SSOS_THREAD_PLATFORM_COND_INIT(cond)
#define SSOS_THREAD_COND_WAIT(cond, condition)            SSOS_THREAD_PLATFORM_COND_WAIT(cond, condition)
#define SSOS_THREAD_COND_TIME_WAIT(cond, condition, time) SSOS_THREAD_PLATFORM_COND_TIME_WAIT(cond, condition, time)
#define SSOS_THREAD_COND_WAKE(cond)                       SSOS_THREAD_PLATFORM_COND_WAKE(cond)
#define SSOS_THREAD_COND_DEINIT(cond)                     SSOS_THREAD_PLATFORM_COND_DEINIT(cond)

#endif /* ifndef __SSOS_THREAD_H__ */
