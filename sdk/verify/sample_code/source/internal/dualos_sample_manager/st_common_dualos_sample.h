/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized diSclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _ST_COMMON_DUALOS_SAMPLE_H_
#define _ST_COMMON_DUALOS_SAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "st_common.h"

int  ST_DUALOS_SAMPLE_StartDemo(char *common);
void ST_DUALOS_SAMPLE_StopDemo(void);

#define START_RTOS_DEMO(__name) \
ST_DUALOS_SAMPLE_StartDemo(__name);

#define STOP_RTOS_DEMO() \
ST_DUALOS_SAMPLE_StopDemo();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DUALOS_SAMPLE_H_
