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

#ifndef __SSOS_TIME_H__
#define __SSOS_TIME_H__

#include "ssos_def.h"
#include "ssos_time_platform.h"

typedef struct SSOS_TIME_Spec_s
{
    unsigned long long tvSec;
    unsigned long long tvNSec;
} SSOS_TIME_Spec_t;

int  SSOS_TIME_Get(SSOS_TIME_Spec_t *ts);
void SSOS_TIME_Sleep(unsigned int sec);
void SSOS_TIME_MSleep(unsigned int mSec);
void SSOS_TIME_USleep(unsigned int uSec);

#endif /* ifndef __SSOS_TIME_H__ */
