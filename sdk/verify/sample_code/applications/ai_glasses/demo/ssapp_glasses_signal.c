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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "st_common_ai_glasses_uart.h"
#include "ssapp_glasses_signal.h"
#include "ssapp_glasses_factory.h"
#include "ssapp_glasses_debugmode.h"

MI_BOOL g_bExit = FALSE;

void SSAPP_GLASSES_SIGNAL_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit normally\n");
    }
    else if (signo == SIGTERM)
    {
        printf("catch SIGTERM, exit normally\n");
    }
    if (!g_bExit)
    {
        g_bExit = TRUE;

        ST_Common_AiGlasses_Uart_ReadThread_CanExit();
        SSAPP_GLASSES_FACTORY_WakeUpMainByResume();
        SSAPP_GLASSES_DEBUGMODE_WakeUp(TRUE);
    }
}

MI_S32 SSAPP_GLASSES_SIGNAL_Init(void)
{
    struct sigaction sigAction = {0};

    sigAction.sa_handler = SSAPP_GLASSES_SIGNAL_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);  //-2
    sigaction(SIGKILL, &sigAction, NULL); //-9
    sigaction(SIGTERM, &sigAction, NULL); //-15

    return MI_SUCCESS;
}

MI_BOOL SSAPP_GLASSES_SIGNAL_ThreadCanExit(void)
{
    return g_bExit;
}
