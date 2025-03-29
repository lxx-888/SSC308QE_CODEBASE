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
#ifndef __SS_AUTO_LOCK__
#define __SS_AUTO_LOCK__
#include <iostream>
#include <time.h>
#include <pthread.h>

class ss_auto_lock
{
public:
#define add_time(_ts, _sec, _nsec)     \
    do                                 \
    {                                  \
        _ts.tv_sec += _sec;            \
        _ts.tv_nsec += _nsec;          \
        if (_ts.tv_nsec > 1000000000)  \
        {                              \
            _ts.tv_nsec %= 1000000000; \
            _ts.tv_sec++;              \
        }                              \
    } while(0);
    explicit ss_auto_lock(pthread_mutex_t &mutex, pthread_cond_t &cond)
        : mutex_lock(&mutex), event_cond(&cond)
    {
        pthread_mutex_lock(mutex_lock);
    }
    explicit ss_auto_lock(pthread_mutex_t &mutex)
        : mutex_lock(&mutex), event_cond(NULL)
    {
        pthread_mutex_lock(mutex_lock);
    }
    bool wait(unsigned int wait_time_ms = 0xFFFFFFFF)
    {
        if (!event_cond)
        {
            std::cout << "ERR: Event Cond is null" << std::endl;
            return false;
        }
        if (wait_time_ms == 0xFFFFFFFF)
        {
            pthread_cond_wait(event_cond, mutex_lock);
            return true;
        }
        struct timespec out_time;
        clock_gettime(CLOCK_MONOTONIC, &out_time);
        add_time(out_time, (wait_time_ms / 1000), (wait_time_ms % 1000) * 1000000);
        int ret = pthread_cond_timedwait(event_cond, mutex_lock, &out_time);
        return ret != ETIMEDOUT;
    }
    void notify()
    {
        if (!event_cond)
        {
            std::cout << "ERR: Event Cond is null" << std::endl;
            return;
        }
        pthread_cond_signal(event_cond);
    }
    virtual ~ss_auto_lock()
    {
        pthread_mutex_unlock(mutex_lock);
    }
private:
    pthread_mutex_t *mutex_lock;
    pthread_cond_t  *event_cond;
};
class ss_auto_rdlock
{
public:
    explicit ss_auto_rdlock(pthread_rwlock_t &lock): rd_lock(lock)
    {
        pthread_rwlock_rdlock(&rd_lock);
    }
    virtual ~ss_auto_rdlock()
    {
        pthread_rwlock_unlock(&rd_lock);
    }
private:
    pthread_rwlock_t &rd_lock;
};
class ss_auto_rwlock
{
public:
    explicit ss_auto_rwlock(pthread_rwlock_t &lock): rw_lock(lock)
    {
        pthread_rwlock_wrlock(&rw_lock);
    }
    virtual ~ss_auto_rwlock()
    {
        pthread_rwlock_unlock(&rw_lock);
    }
private:
    pthread_rwlock_t &rw_lock;
};
#endif
