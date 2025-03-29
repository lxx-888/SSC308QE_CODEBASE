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

#include <pthread.h>
#include "ssos_time.h"

typedef pthread_mutex_t  SSOS_THREAD_PLATFORM_Mutex_t;
typedef pthread_rwlock_t SSOS_THREAD_PLATFORM_RwLock_t;

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

#endif /* ifndef __SSOS_THREAD_PLATFORM_H__ */
