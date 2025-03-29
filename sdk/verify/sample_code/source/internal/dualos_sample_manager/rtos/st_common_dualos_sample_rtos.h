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
#ifndef _ST_COMMON_DUALOS_SAMPLE_RTOS_H_
#define _ST_COMMON_DUALOS_SAMPLE_RTOS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cam_os_wrapper.h"
#include "initcall.h"

typedef int (*Dualos_Sample_CallbackFunc)(int menu_argc, char** menu_argv);
typedef struct
{
    char                  identifier[64];
    Dualos_Sample_CallbackFunc function;
} CallbackItem;

typedef struct
{
    int                        use_flag;
    int                        index;
    int                        argc_demo;
    char                     **argv_demo;
    char                       common[256];
    Dualos_Sample_CallbackFunc function;
} FuncInputParam;

void ST_DUALOS_SAMPLE_RegisterCallback(const char* identifier, Dualos_Sample_CallbackFunc function);

#define DUALOS_SAMPLE_MODULE_AUTO_INITCALLBACK(__name, __func) \
static void __DualosSampleModuleAutoInitCallback(void)    \
{                                                              \
    ST_DUALOS_SAMPLE_RegisterCallback(__name,__func);  \
    CamOsPrintf("module %s called.\n", __name);                \
}                                                              \
rtos_application_initcall(__DualosSampleModuleAutoInitCallback, 0);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DUALOS_SAMPLE_RTOS_H_
