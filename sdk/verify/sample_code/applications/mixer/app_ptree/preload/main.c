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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ptree_preload.h"
#include "ssapp_mixer_preload.h"

int main(int argc, char **argv)
{
    char           cGetVal   = 0;
    unsigned long  ulTimerS0 = SSAPP_MIXER_PRELOAD_GetTimer();
    unsigned long  ulTimerS1 = 0;
    void *         pIns      = NULL;
    int            s32Ret    = 0;
    PTREE_Config_t stConfig;

    PTREE_PRELOAD_Setup();
    s32Ret = SSAPP_MIXER_PRELOAD_PtreeTakeOff(&stConfig, argc, argv);
    if (s32Ret == -1)
    {
        goto RET;
    }
    pIns = PTREE_CreateInstance(&stConfig);
    if (!pIns)
    {
        printf("Create instance fail!\n");
        s32Ret = -1;
        goto RET;
    }
    PTREE_ConstructPipeline(pIns);
    ulTimerS1 = SSAPP_MIXER_PRELOAD_GetTimer();
    printf("--------------> Stage0->Load Param : time diff %ld us\n", ulTimerS1 - ulTimerS0);
    PTREE_StartPipeline(pIns);
    printf("--------------> Stage1->Module init: time diff %ld us\n", SSAPP_MIXER_PRELOAD_GetTimer() - ulTimerS1);
    while (1)
    {
        printf("Type '0' to enter the console of pipeline control!\n");
        printf("Type '1' to enter the console of ptree's module command!\n");
        printf("Type 'q' to exit!\n");
        cGetVal = getchar();
        if (cGetVal == 'q')
        {
            break;
        }
        if (cGetVal == '0')
        {
            SSAPP_MIXER_PRELOAD_PipelineControlConsole(pIns);
            continue;
        }
        if (cGetVal == '1')
        {
            SSAPP_MIXER_PRELOAD_PipelineModuleCommandConsole();
            continue;
        }
    }
    PTREE_StopPipeline(pIns);
    PTREE_DestructPipeline(pIns);
    PTREE_DestroyInstance(pIns);
    SSAPP_MIXER_PRELOAD_PtreeDropDown(s32Ret, &stConfig);
RET:
    PTREE_PRELOAD_CLear();

    return s32Ret;
}
