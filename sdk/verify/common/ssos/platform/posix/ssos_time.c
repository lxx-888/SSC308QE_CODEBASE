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

#include <time.h>
#include <unistd.h>
#include "ssos_time.h"

int SSOS_TIME_Get(SSOS_TIME_Spec_t *ts)
{
    struct timespec tv;
    int             ret = SSOS_DEF_OK;
    if (!ts)
    {
        return SSOS_DEF_EINVAL;
    }
    ret        = clock_gettime(CLOCK_MONOTONIC, &tv);
    ts->tvSec  = tv.tv_sec;
    ts->tvNSec = tv.tv_nsec;
    return ret;
}
void SSOS_TIME_Sleep(unsigned int sec)
{
    sleep(sec);
}
void SSOS_TIME_MSleep(unsigned int mSec)
{
    usleep(mSec * 1000);
}
void SSOS_TIME_USleep(unsigned int uSec)
{
    usleep(uSec);
}
