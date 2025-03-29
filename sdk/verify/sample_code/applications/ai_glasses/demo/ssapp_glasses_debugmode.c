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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "list.h"
#include "cam_os_condition.h"
#include "ssapp_glasses_debugmode.h"
#include "ssapp_glasses_factory.h"
#include "ssapp_glasses_signal.h"

typedef struct SSAPP_GLASSES_DEBUGMODE_Work_s
{
    struct list_head workList;
    SS_TASK_e        eDebugTask;
    MI_U8            u8PhotoCnt;
} SSAPP_GLASSES_DEBUGMODE_Work_t;

extern SSAPP_GLASSES_FACTORY_DebugParam_t g_stDebugParam;

LIST_HEAD(st_Debug_WorkList);
CamOsCondition_t gp_debugCmdWaitqueue = {0};
MI_BOOL          g_bIsExit            = FALSE;

MI_BOOL SSAPP_GLASSES_DEBUGMODE_IsExist(void)
{
    return g_bIsExit;
}

MI_S32 SSAPP_GLASSES_DEBUGMODE_AddDebugTask(SS_TASK_e eTask, MI_U8 u8PhotoCnt)
{
    if (eTask < E_TASK_ERR)
    {
        if (eTask == E_TASK_STOP_REC || eTask == E_TASK_STOP_TRANS)
        {
            ST_Protrcol_Task_t stProtcolTask = {0};

            stProtcolTask.eTaskCmd  = eTask;
            stProtcolTask.eCmdType  = E_CMD_TYPE_REQ;
            stProtcolTask.eSocState = E_STATE_UNKOWN;
            stProtcolTask.isReject  = 0;

            if (SSAPP_GLASSES_FACTORY_HandleTask(&stProtcolTask) != MI_SUCCESS)
            {
                printf("debug task inject failed\n");
            }
            SSAPP_GLASSES_DEBUGMODE_WakeUp(FALSE);
            return MI_SUCCESS;
        }
        SSAPP_GLASSES_DEBUGMODE_Work_t* pstDebugWork = NULL;

        pstDebugWork = malloc(sizeof(SSAPP_GLASSES_DEBUGMODE_Work_t));
        memset(pstDebugWork, 0x0, sizeof(*pstDebugWork));
        pstDebugWork->eDebugTask = eTask;
        pstDebugWork->u8PhotoCnt = u8PhotoCnt;
        list_add_tail(&pstDebugWork->workList, &st_Debug_WorkList);
        SSAPP_GLASSES_DEBUGMODE_WakeUp(FALSE);
    }
    else
    {
        printf("debug eTask:%d err\n", eTask);
    }
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_DEBUGMODE_HandleUserCmd(void)
{
    char debugCmdBuf[256] = {0};

    while (!SSAPP_GLASSES_SIGNAL_ThreadCanExit())
    {
        if (fgets(debugCmdBuf, sizeof(debugCmdBuf), stdin) != NULL)
        {
            MI_U8 u8Length = 0;
            char* token    = NULL;
            char* buf      = NULL;

            debugCmdBuf[strcspn(debugCmdBuf, "\n")] = '\0';
            u8Length                                = strlen(debugCmdBuf);
            if (u8Length == 0)
            {
                printf("[debugmode] # ");
                continue;
            }
            buf = debugCmdBuf;
            if ((token = strsep(&buf, " ")) == NULL)
            {
                printf("cmd err:%s\n", buf);
                goto HELP;
            }

            if (strcmp(token, "add") == 0)
            {
                int eTaskId = -1;
                if ((token = strsep(&buf, " ")) == NULL)
                {
                    printf("add unknown task, please enter \"help add\" get support task\n");
                    continue;
                }
                eTaskId = atoi(token);
                if (eTaskId >= 0 && eTaskId < E_TASK_POWEROFF)
                {
                    MI_U8 u8PhotoCnt = 0;
                    printf("debugmode new add task:%d\n", eTaskId);
                    if (eTaskId == E_TASK_PHOTO)
                    {
                        if ((token = strsep(&buf, " ")) == NULL)
                        {
                            u8PhotoCnt = 1;
                        }
                        else
                        {
                            u8PhotoCnt = atoi(token);
                        }
                        printf("debug photo cnt:%d\n", u8PhotoCnt);
                    }
                    SSAPP_GLASSES_DEBUGMODE_AddDebugTask(eTaskId, u8PhotoCnt);
                }
                else
                {
                    printf("SS_TASK_e err:[%d]%s, please enter \"help add\" get support task\n", eTaskId, token);
                }
            }
            else if (strcmp(token, "quit") == 0)
            {
                SSAPP_GLASSES_DEBUGMODE_WakeUp(TRUE);
                return MI_SUCCESS;
            }
            else if (strcmp(token, "exit") == 0)
            {
                pid_t pid = getpid();
                kill(pid, SIGINT);
                return MI_SUCCESS;
            }
            else if (strcmp(token, "help") == 0)
            {
                if ((token = strsep(&buf, " ")) != NULL)
                {
                    if (strcmp(token, "add") == 0)
                    {
                        goto HELP_ADD;
                    }
                }
                goto HELP;
            }
            else
            {
                printf("cmd not find:%s, please enter \"help\" get support cmd\n", token);
            }
        }
        continue;

        {
        HELP:
            printf("Options are: \n");
            printf("    help              : Display debugmode option list\n");
            printf("    quit              : quit debug mode\n");
            printf("    exit              : exit app\n");
        HELP_ADD:
            printf("    add \"SS_TASK_e\" : new add debug task\n");
            printf("                        0        E_TASK_NONE\n");
            printf("                        1    n   E_TASK_PHOTO u8PhotoCnt\n");
            printf("                        2        E_TASK_START_REC\n");
            printf("                        3        E_TASK_STOP_REC\n");
            printf("                        4        E_TASK_TRANS\n");
            printf("                        5        E_TASK_STOP_TRANS\n");
        }
    }
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_DEBUGMODE_Init(void)
{
    if (g_stDebugParam.bDebugMode)
    {
        CamOsConditionInit(&gp_debugCmdWaitqueue);

        SSAPP_GLASSES_DEBUGMODE_HandleUserCmd();
    }
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_DEBUGMODE_DeInit(void)
{
    if (g_stDebugParam.bDebugMode)
    {
        CamOsConditionDeinit(&gp_debugCmdWaitqueue);
    }
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_DEBUGMODE_WakeUp(MI_BOOL isExit)
{
    if (g_stDebugParam.bDebugMode)
    {
        g_bIsExit = isExit;
        CamOsConditionWakeUpAll(&gp_debugCmdWaitqueue);
    }
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_DEBUGMODE_GetDebugTaskInjectRealWork(void)
{
    ST_Protrcol_Task_t stProtcolTask = {0};

    if (list_empty(&st_Debug_WorkList))
    {
        printf("\033[31m ===================wait task cmd 10s:=====================\033[0m\n");
    }
    CamOsConditionTimedWait(&gp_debugCmdWaitqueue, !list_empty(&st_Debug_WorkList) || SSAPP_GLASSES_DEBUGMODE_IsExist(),
                            10000UL);

    if (list_empty(&st_Debug_WorkList))
    {
        stProtcolTask.eTaskCmd = E_TASK_NONE;
    }
    else
    {
        SSAPP_GLASSES_DEBUGMODE_Work_t* pstDebugWork = NULL;

        pstDebugWork = list_first_entry(&st_Debug_WorkList, SSAPP_GLASSES_DEBUGMODE_Work_t, workList);
        if (pstDebugWork)
        {
            stProtcolTask.eTaskCmd      = pstDebugWork->eDebugTask;
            stProtcolTask.u16UserDefine = pstDebugWork->u8PhotoCnt << 8;
            list_del(&pstDebugWork->workList);
            free(pstDebugWork);
            printf("get a debug task:%d\n", stProtcolTask.eTaskCmd);
        }
        else
        {
            printf("pstDebugWork is NULL!!\n");
        }
    }

    stProtcolTask.eCmdType  = E_CMD_TYPE_REQ;
    stProtcolTask.eSocState = E_STATE_UNKOWN;
    stProtcolTask.isReject  = 0;

    if (SSAPP_GLASSES_FACTORY_HandleTask(&stProtcolTask) != MI_SUCCESS)
    {
        printf("debug task inject failed\n");
    }

    return MI_SUCCESS;
}
