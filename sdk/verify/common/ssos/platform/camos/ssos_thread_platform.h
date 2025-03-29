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

#ifndef __SSOS_THREAD_PLATFORM_H__
#define __SSOS_THREAD_PLATFORM_H__

#if defined(__KERNEL__) || defined(CAM_OS_RTK)
#include "ms_platform.h"
#endif
#include "cam_os_wrapper.h"

typedef CamOsMutex_t SSOS_THREAD_PLATFORM_Mutex_t;
typedef CamOsRwsem_t SSOS_THREAD_PLATFORM_RwLock_t;

#ifdef CAM_OS_RTK

#include "cam_os_util_list.h"
#include "ms_platform.h"
#include "cam_os_wrapper.h"
#include "sys_MsWrapper_cus_os_sem.h"
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_arm_arch_timer.h"

typedef struct
{
    struct CamOsListHead_t listHead;
    Ms_Spinlock_t          flags;
} SSOS_THREAD_PLATFORM_Cond_t;

typedef struct
{
    struct CamOsListHead_t list;
    Ms_DynSemaphor_t       dynSem;
} SSOS_THREAD_PLATFORM_CondSem_t;

#define SSOS_THREAD_PLATFORM_COND_INIT(_cond)          \
    (                                                  \
        {                                              \
            MsInitSpinlock(&(_cond)->flags);           \
            CAM_OS_INIT_LIST_HEAD(&(_cond)->listHead); \
        })

#define SSOS_THREAD_PLATFORM_COND_WAIT(_cond, _condition)                 \
    (                                                                     \
        {                                                                 \
            int                            _ret = SSOS_DEF_OK;            \
            SSOS_THREAD_PLATFORM_CondSem_t waitSem;                       \
            CAM_OS_INIT_LIST_HEAD(&waitSem.list);                         \
            MsCreateDynSemExtend(&waitSem.dynSem, CAM_OS_MAX_INT - 1, 0); \
            MsSpinLockIrqSave(&(_cond)->flags);                           \
            CAM_OS_LIST_ADD_TAIL(&waitSem.list, &(_cond)->listHead);      \
            MsSpinUnlockIrqRestore(&(_cond)->flags);                      \
            while (!(_condition))                                         \
            {                                                             \
                MsConsumeDynSem(&waitSem.dynSem);                         \
            }                                                             \
            MsSpinLockIrqSave(&(_cond)->flags);                           \
            CAM_OS_LIST_DEL(&waitSem.list);                               \
            MsDestroyDynSem(&waitSem.dynSem);                             \
            MsSpinUnlockIrqRestore(&(_cond)->flags);                      \
            _ret;                                                         \
        })
#define __SSOS_THREAD_PLATFORM_COND_TIME_WAIT(_cond, _condition, _time)                                    \
    (                                                                                                      \
        {                                                                                                  \
            unsigned long long _timeOutNs = (_time).tvSec * 1000000000 + (_time).tvNSec;                   \
            unsigned long long _timeDiff  = _timeOutNs;                                                    \
            unsigned long long _targetTime =                                                               \
                arch_timer_get_counter() * 1000000000 / arch_timer_get_cntfrq() + _timeOutNs;              \
            int                            _ret = SSOS_DEF_TRUE;                                           \
            SSOS_THREAD_PLATFORM_CondSem_t waitSem;                                                        \
            CAM_OS_INIT_LIST_HEAD(&waitSem.list);                                                          \
            MsCreateDynSemExtend(&waitSem.dynSem, CAM_OS_MAX_INT - 1, 0);                                  \
            MsSpinLockIrqSave(&(_cond)->flags);                                                            \
            CAM_OS_LIST_ADD_TAIL(&waitSem.list, &(_cond)->listHead);                                       \
            MsSpinUnlockIrqRestore(&(_cond)->flags);                                                       \
            while (!(_condition))                                                                          \
            {                                                                                              \
                if (MS_NO_MESSAGE == MsConsumeDynSemDelay(&waitSem.dynSem, _timeDiff / 1000000))           \
                {                                                                                          \
                    _ret = (_condition);                                                                   \
                    break;                                                                                 \
                }                                                                                          \
                _timeDiff = _targetTime - arch_timer_get_counter() * 1000000000 / arch_timer_get_cntfrq(); \
                if (_timeDiff > _timeOutNs)                                                                \
                {                                                                                          \
                    _ret = (_condition);                                                                   \
                    break;                                                                                 \
                }                                                                                          \
            }                                                                                              \
            MsSpinLockIrqSave(&(_cond)->flags);                                                            \
            CAM_OS_LIST_DEL(&waitSem.list);                                                                \
            MsDestroyDynSem(&waitSem.dynSem);                                                              \
            MsSpinUnlockIrqRestore(&(_cond)->flags);                                                       \
            _ret;                                                                                          \
        })

#define SSOS_THREAD_PLATFORM_COND_TIME_WAIT(_cond, _condition, _time)             \
    (                                                                             \
        {                                                                         \
            int _ret = SSOS_DEF_OK;                                               \
            if (!__SSOS_THREAD_PLATFORM_COND_TIME_WAIT(_cond, _condition, _time)) \
            {                                                                     \
                _ret = SSOS_DEF_ETIMEOUT;                                         \
            }                                                                     \
            _ret;                                                                 \
        })
#define SSOS_THREAD_PLATFORM_COND_WAKE(_cond)                           \
    (                                                                   \
        {                                                               \
            SSOS_THREAD_PLATFORM_CondSem_t *ptSem;                      \
            MsSpinLockIrqSave(&(_cond)->flags);                         \
            CAM_OS_LIST_FOR_EACH_ENTRY(ptSem, &(_cond)->listHead, list) \
            {                                                           \
                MsProduceDynSem(&(ptSem->dynSem));                      \
            }                                                           \
            MsSpinUnlockIrqRestore(&(_cond)->flags);                    \
        })
#define SSOS_THREAD_PLATFORM_COND_DEINIT(_cond) ({ (void)_cond; })

#elif defined(__KERNEL__)

#include "linux/wait.h"
#include "ms_platform.h"
#include "cam_os_wrapper.h"
typedef wait_queue_head_t SSOS_THREAD_PLATFORM_Cond_t;

#define SSOS_THREAD_PLATFORM_COND_INIT(_cond) init_waitqueue_head(_cond);
#define SSOS_THREAD_PLATFORM_COND_WAIT(_cond, _condition) \
    (                                                     \
        {                                                 \
            int _ret = SSOS_DEF_OK;                       \
            wait_event((*(_cond)), _condition);           \
            _ret;                                         \
        })
#define SSOS_THREAD_PLATFORM_COND_TIME_WAIT(_cond, _condition, _time)                               \
    (                                                                                               \
        {                                                                                           \
            int _ret = SSOS_DEF_OK;                                                                 \
            if (!wait_event_timeout((*(_cond)), _condition,                                         \
                                    CamOsNsToJiffies((_time).tvSec * 1000000000 + (_time).tvNSec))) \
            {                                                                                       \
                _ret = SSOS_DEF_ETIMEOUT;                                                           \
            }                                                                                       \
            _ret;                                                                                   \
        })
#define SSOS_THREAD_PLATFORM_COND_WAKE(_cond) wake_up_all(_cond)
#define SSOS_THREAD_PLATFORM_COND_DEINIT(_cond)

#else /* LINUX USER */

#include <pthread.h>
#include "ssos_time.h"

typedef struct
{
    pthread_mutex_t    mutex;
    pthread_cond_t     cond;
    pthread_condattr_t attr;
} SSOS_THREAD_PLATFORM_Cond_t;

#define SSOS_THREAD_PLATFORM_COND_INIT(_cond)                       \
    (                                                               \
        {                                                           \
            pthread_condattr_init(_cond.attr);                      \
            pthread_condattr_setclock(_cond.attr, CLOCK_MONOTONIC); \
            pthread_cond_init(_cond.cond, _cond.attr);              \
            pthread_mutex_init(_cond.mutex, NULL);                  \
        })

#define SSOS_THREAD_PLATFORM_COND_WAIT(_cond, _condition)   \
    (                                                       \
        {                                                   \
            int _ret = SSOS_DEF_OK;                         \
            pthread_mutex_lock(_cond.mutex);                \
            while (!(_condition))                           \
                pthread_cond_wait(_cond.cond, _cond.mutex); \
            pthread_mutex_unlock(_cond.mutex);              \
            _ret;                                           \
        })

#define __SSOS_THREAD_PLATFORM_COND_TIME_WAIT(_cond, _condition, _time)             \
    (                                                                               \
        {                                                                           \
            int             _ret = 1;                                               \
            struct timespec maxWait;                                                \
            clock_gettime(CLOCK_MONOTONIC, &maxWait);                               \
            maxWait.tv_sec += (_time).tvSec;                                        \
            maxWait.tv_nsec += (_time).tvNSec;                                      \
            if (maxWait.tv_nsec > 1000000000)                                       \
            {                                                                       \
                ++maxWait.tv_sec;                                                   \
                maxWait.tv_nsec %= 1000000000;                                      \
            }                                                                       \
            pthread_mutex_lock(_cond.mutex);                                        \
            while (!(_condition))                                                   \
            {                                                                       \
                if (0 != pthread_cond_timedwait(_cond.cond, _cond.mutex, &maxWait)) \
                {                                                                   \
                    _ret = (_condition);                                            \
                    break;                                                          \
                }                                                                   \
            }                                                                       \
            pthread_mutex_unlock(_cond.mutex);                                      \
            _ret;                                                                   \
        })

#define SSOS_THREAD_PLATFORM_COND_TIME_WAIT(_cond, _condition, _time)             \
    (                                                                             \
        {                                                                         \
            int _ret = SSOS_DEF_OK;                                               \
            if (!__SSOS_THREAD_PLATFORM_COND_TIME_WAIT(_cond, _condition, _time)) \
            {                                                                     \
                _ret = SSOS_DEF_ETIMEOUT;                                         \
            }                                                                     \
            _ret;                                                                 \
        })

#define SSOS_THREAD_PLATFORM_COND_WAKE(_cond)   \
    (                                           \
        {                                       \
            pthread_mutex_lock(_cond.mutex);    \
            pthread_cond_broadcast(_cond.cond); \
            pthread_mutex_unlock(_cond.mutex);  \
        })

#define SSOS_THREAD_PLATFORM_COND_DEINIT(_cond)   \
    (                                             \
        {                                         \
            pthread_condattr_destroy(_cond.attr); \
            pthread_cond_destroy(_cond.cond);     \
            pthread_mutex_destroy(_cond.mutex);   \
        })

#endif /* endif PLATFORM type */

#endif /* ifndef __SSOS_THREAD_PLATFORM_H__ */
