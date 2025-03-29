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

#ifdef CAM_OS_RTK
#include "time.h"
#include "sys_arch_timer.h"
#elif defined(__KERNEL__)
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/math64.h>
#else /* LINUX USER */
#include <time.h>
#endif

#include "ssos_time.h"

#ifdef CAM_OS_RTK
int SSOS_TIME_Get(SSOS_TIME_Spec_t *ts)
{
    if (!ts)
    {
        return -1;
    }
    u64 ticks  = arch_timer_get_counter();
    ts->tvSec  = ticks / arch_timer_get_cntfrq();
    ts->tvNSec = ((ticks % arch_timer_get_cntfrq()) * 1000000000 / arch_timer_get_cntfrq());
    return 0;
}
#elif defined(__KERNEL__)
int SSOS_TIME_Get(SSOS_TIME_Spec_t *ts)
{
    struct timespec64 tv64;
    if (!ts)
    {
        return -1;
    }
    ktime_get_raw_ts64(&tv64);
    ts->tvSec  = tv64.tv_sec;
    ts->tvNSec = tv64.tv_nsec;
    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TIME_Get);
#else /* LINUX USER */
int SSOS_TIME_Get(SSOS_TIME_Spec_t *ts)
{
    int ret = 0;
    if (!ts)
    {
        return -1;
    }
    struct timespec tv;
    ret        = clock_gettime(CLOCK_MONOTONIC, &tv);
    ts->tvSec  = tv.tv_sec;
    ts->tvNSec = tv.tv_nsec;
    return ret;
}
#endif

void SSOS_TIME_Sleep(unsigned int sec)
{
    CamOsMsSleep(sec * 1000);
}
SSOS_EXPORT_SYMBOL(SSOS_TIME_Sleep);

void SSOS_TIME_MSleep(unsigned int mSec)
{
    CamOsMsSleep(mSec);
}
SSOS_EXPORT_SYMBOL(SSOS_TIME_MSleep);

void SSOS_TIME_USleep(unsigned int uSec)
{
    CamOsUsSleep(uSec);
}
SSOS_EXPORT_SYMBOL(SSOS_TIME_USleep);
