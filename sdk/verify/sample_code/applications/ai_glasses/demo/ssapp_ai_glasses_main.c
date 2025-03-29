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

#include <elf.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "mi_common_datatype.h"
#include "st_common_ai_glasses_protrcol.h"
#include "st_common_ai_glasses_uart.h"
#include "ssapp_glasses_factory.h"
#include "ssapp_glasses_signal.h"

// strong statement to disable light misc control register vif done cb
MI_BOOL g_bOnlyLightSensor = true;

SSAPP_GLASSES_FACTORY_DebugParam_t g_stDebugParam = {0};

MI_S32 SSAPP_AI_GLASSES_MAIN_StGlassesGetCmdlineParam(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "-t"))
        {
            g_stDebugParam.bDisableHeartBeat = TRUE;
            g_stDebugParam.bDisableSuspend   = TRUE;
        }
        else if (0 == strcmp(argv[i], "-nohb"))
        {
            g_stDebugParam.bDisableHeartBeat = TRUE;
        }
        else if (0 == strcmp(argv[i], "-nostr"))
        {
            g_stDebugParam.bDisableSuspend = TRUE;
        }
        else if (0 == strcmp(argv[i], "-nosig"))
        {
            g_stDebugParam.bDisbaleSignal = TRUE;
        }
        else if (0 == strcmp(argv[i], "-w"))
        {
            g_stDebugParam.bWaitDoingTimeout = TRUE;
        }
        else if (0 == strcmp(argv[i], "-dma"))
        {
            g_stDebugParam.bDmaSupport = TRUE;
        }
        else if (0 == strcmp(argv[i], "-d"))
        {
            g_stDebugParam.bDebugMode        = TRUE;
            g_stDebugParam.bDisableHeartBeat = TRUE;
        }
        else if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "--help")))
        {
            printf("Options are: \n");
            printf("    -h --help   : Display option list\n");
            printf("    -nohb       : no send heartbeat anymore\n");
            printf("    -nostr      : only go to sleep 10s rather then going to suspend\n");
            printf("    -nosig      : not catch system signal(2 9 15)\n");
            printf("    -dma        : fuart with dma mode, there is no dma support with fuart\n");
            printf("    -t          : nohb nostr\n");
            printf("    -w          : wait doing task 100s then force destory task\n");
            printf("    -d          : debug mode, user can add task by manual. nohb default\n");
            exit(0);
        }
    }
    return MI_SUCCESS;
}

int main(int argc, char **argv)
{
    MI_S32 ret = 0;

    SSAPP_AI_GLASSES_MAIN_StGlassesGetCmdlineParam(argc, argv);

    if (!g_stDebugParam.bDisbaleSignal)
    {
        SSAPP_GLASSES_SIGNAL_Init();
    }

    printf("sample_ai_glasses run!\n");
    ret = ST_Common_AiGlasses_Uart_Init(SSAPP_GLASSES_FACTORY_HandleTask, g_stDebugParam.bDmaSupport);
    if (ret != MI_SUCCESS)
    {
        return ret;
    }
    SSAPP_GLASSES_FACTORY_Init();

    while (!SSAPP_GLASSES_SIGNAL_ThreadCanExit())
    {
        sleep(1);
    }
    ret = ST_Common_AiGlasses_Uart_DeInit();
    printf("ready to factory deinit\n");
    SSAPP_GLASSES_FACTORY_DeInit();

    printf("sample_ai_glasses exit!\n");
    return ret;
}
