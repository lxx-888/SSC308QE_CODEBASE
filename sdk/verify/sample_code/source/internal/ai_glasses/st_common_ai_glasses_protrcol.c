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
#include <sys/time.h>

#include "mi_common_datatype.h"
#include "st_common_ai_glasses_protrcol.h"

char protHeader[] = {0x5A, 0x5A, 0x5A, 0x5A};
char protTail[]   = {0xA5, 0xA5, 0xA5, 0xA5};

/* the length of msg_buf must be SS_TASK_CMD_PROTRCOL */
MI_S32 ST_Common_AiGlasses_Prot_Make_Cmd_To_Protrcol(char *msg_buf, ST_Protrcol_Task_t *pstProtTask)
{
    if (!msg_buf)
    {
        printf("msg_buf is NULL\n");
        return -1;
    }
    if (!pstProtTask)
    {
        printf("pstProtTask is NULL\n");
        return -1;
    }
    if (pstProtTask->eCmdType >= E_CMD_TYPE_ERR)
    {
        printf("eCmdType:%d err\n", pstProtTask->eCmdType);
        return -1;
    }
    if (pstProtTask->eTaskCmd >= E_TASK_ERR)
    {
        printf("eTaskCmd:%d err\n", pstProtTask->eTaskCmd);
        return -1;
    }
    if (pstProtTask->eSocState >= E_STATE_UNKOWN)
    {
        printf("eSocState:%d err\n", pstProtTask->eSocState);
        return -1;
    }

    memcpy(msg_buf, protHeader, sizeof(protHeader));
    msg_buf[4] = pstProtTask->eCmdType;
    msg_buf[5] = pstProtTask->eTaskCmd;
    msg_buf[6] = pstProtTask->isReject;
    msg_buf[7] = pstProtTask->eSocState;
    memcpy(&msg_buf[8], &pstProtTask->u16UserDefine, sizeof(pstProtTask->u16UserDefine));
    memcpy(&msg_buf[10], protTail, sizeof(protTail));

    return 0;
}

MI_S32 _ST_Common_AiGlasses_Prot_Get_Task_From_Msg(char *msg_buf, ST_Protrcol_Task_t *pstProtTask)
{
    if (!msg_buf)
    {
        printf("msg_buf is NULL\n");
        return -1;
    }
    if (!pstProtTask)
    {
        printf("pstProtTask is NULL\n");
        return -1;
    }
    pstProtTask->eCmdType  = msg_buf[4];
    pstProtTask->eTaskCmd  = msg_buf[5];
    pstProtTask->isReject  = msg_buf[6];
    pstProtTask->eSocState = msg_buf[7];
    memcpy(&pstProtTask->u16UserDefine, &msg_buf[8], sizeof(pstProtTask->u16UserDefine));
    return MI_SUCCESS;
}

MI_S32 _ST_Common_AiGlasses_Prot_Find_Msg(const char *buf, MI_U32 buf_size, char *msg, BOOL useDma)
{
    char *start       = NULL;
    char  startBuf[5] = {0};
    int   value       = 0;

    memcpy(startBuf, protHeader, sizeof(protHeader));
    startBuf[4] = '\0';

    start = strstr(buf, startBuf);
    if (start == NULL)
    {
        if (useDma)
        {
            printf("cat't find protHeader\n");
        }
        return -1;
    }

    value = memcmp(start + SS_TASK_CMD_PROTRCOL - sizeof(protTail), protTail, sizeof(protTail));
    if (value != 0)
    {
        printf("cat't find protTail, buf:%lx start:%lx cmp:%d\n", (unsigned long)buf, (unsigned long)start, value);
        return -1;
    }

    if (msg)
    {
        memcpy(msg, start, SS_TASK_CMD_PROTRCOL);
    }
    else
    {
        return -2;
    }

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiGlasses_Prot_Deal_Uart_Buffer(char *uartBuf, MI_U32 len, BOOL useDma,
                                        MI_S32 (*protcolTaskHandler)(ST_Protrcol_Task_t *pstProtcolTask))
{
    char protMsg[SS_TASK_CMD_PROTRCOL] = {0};
    int  ret                           = MI_SUCCESS;
    if (uartBuf && len > 0)
    {
        int cnt = 0;
        ret     = _ST_Common_AiGlasses_Prot_Find_Msg(uartBuf, len, (char *)&protMsg, useDma);
        if (ret == MI_SUCCESS)
        {
            ST_Protrcol_Task_t stProtcolTask = {0};
            struct timeval     tv;
            long               milliseconds;

            gettimeofday(&tv, NULL);
            milliseconds = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            printf("[%ld]get mcu protMsg:\n", milliseconds);
            for (cnt = 0; cnt < SS_TASK_CMD_PROTRCOL; cnt++)
            {
                printf("%x ", protMsg[cnt]);
            }
            printf("\n");
            _ST_Common_AiGlasses_Prot_Get_Task_From_Msg((char *)&protMsg, &stProtcolTask);
            // handle msg to deal task stProtcolTask
            /* ret = ST_GLASSES_FACTORY_HandleTask(&stProtcolTask); */
            ret = protcolTaskHandler(&stProtcolTask);
        }
        else if (ret == -1 && useDma)
        {
            printf("Received:%s\n", uartBuf);
            /*             while (cnt < len) */
            /* { */
            /*     printf("received buf[%d]:%x\n", cnt, uartBuf[cnt]); */
            /*     cnt++; */
            /* } */
        }
    }
    return ret;
}
