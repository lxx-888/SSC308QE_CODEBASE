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
#include <stdint.h>
#include "ss_task_factory.h"
#include "uart.h"

SS_STATE_e eCurrentSocState = E_STATE_UNKOWN;

uint8_t SS_Factory_Update_Soc_State_And_Alive_State(SS_STATE_e status)
{
    eCurrentSocState = status;
    return 0;
}

uint8_t SS_Factory_Send_Task_Protrcol_To_Uart(SS_TaskList_t* sendTask)
{
    SS_TASK_e eTaskCmd                          = E_TASK_NONE;
    uint8_t   taskId                            = -1;
    uint8_t   msg_buf[SS_TASK_CMD_PROTRCOL + 1] = {0};
    uint8_t   u8PhotoCnt                        = 0;

    if (sendTask != NULL)
    {
        eTaskCmd   = sendTask->task;
        taskId     = sendTask->taskId;
        u8PhotoCnt = sendTask->photoCnt;
    }

    memset(msg_buf, 0x5A, 4 * sizeof(uint8_t));
    msg_buf[4] = E_CMD_TYPE_ACK;
    msg_buf[5] = eTaskCmd;
    msg_buf[7] = E_STATE_UNKOWN;
    msg_buf[8] = taskId;
    msg_buf[9] = u8PhotoCnt;
    memset(&msg_buf[10], 0xA5, 4 * sizeof(uint8_t));

    msg_buf[SS_TASK_CMD_PROTRCOL] = 0xD; // to linux uart must enter CR !!

    SS_Send_Cmd_to_Def_UART(msg_buf, SS_TASK_CMD_PROTRCOL + 1);
    return 0;
}

int8_t _SS_Factory_Handle_Mcu_Msg(uint8_t cmdType, uint8_t currentCmd, uint8_t isAccept, uint8_t status,
                                  uint16_t userDefine)
{
    BOOL isMcuAck = FALSE;
    if (cmdType >= E_CMD_TYPE_ERR)
    {
        return -2;
    }
    if (currentCmd >= E_TASK_ERR)
    {
        return -3;
    }
    if (isAccept != 0 && isAccept != 1)
    {
        return -4;
    }
    if (status >= E_STATE_UNKOWN)
    {
        return -5;
    }

    if (cmdType == E_CMD_TYPE_REQ && (currentCmd == E_TASK_REQUEST || currentCmd == E_TASK_POWEROFF_OK))
    {
        if (currentCmd == E_TASK_POWEROFF_OK)
        {
            // we can power off soc now!!!
            DEBUG_INFO("power off soc now\r\n");
            SS_GPIO_PowerOff_Soc();
        }
        else
        {
            SS_TaskList_t* doingTask = SS_TaskList_Get_a_Doning_Task();
            if (doingTask)
            {
                // maybe something worong that remains a doing task
                SS_Factory_Send_Task_Protrcol_To_Uart(doingTask);
                return 0;
            }
            // add a pending task to doing task list && send to soc
            if (SS_TaskList_Deal_a_Pending_Task() >= 0)
            {
                // there is no pending task now, send none task back
                doingTask = SS_TaskList_Get_a_Doning_Task();
                if (!doingTask)
                {
                    printf(ERR_LOG_TITLE "something not correct, no way!\r\n");
                }
            }
            SS_Factory_Send_Task_Protrcol_To_Uart(doingTask);
        }
    }
    else if (cmdType == E_CMD_TYPE_ACK && currentCmd == E_TASK_DONE)
    {
        // dask done, we can remove the task from doing task list
        uint8_t taskId = userDefine & 0xFF;
        if (SS_TaskList_Task_Had_Done(taskId) < 0)
        {
            printf(ERR_LOG_TITLE "task id:%d not fount\r\n", taskId);
            return -8;
        }
    }
    else if (currentCmd == E_TASK_HEARTBEAT)
    {
        // there is will be updata alive state !!!
        SS_Factory_Update_Soc_State_And_Alive_State(status);
    }
    else
    {
        printf(ERR_LOG_TITLE "soc cmd err, cmdType:%d currentCmd:%d\r\n", cmdType, currentCmd);
        return -6;
    }

    return 0;
}

int8_t SS_Factory_Uart_Data_Handle(uint8_t* pUartData)
{
    uint8_t  cmdType    = 0;
    uint8_t  currentCmd = 0;
    uint8_t  isAccept   = 0;
    uint8_t  status     = 0;
    uint16_t userDefine = 0;
    int8_t   ret        = 0;
    if (!pUartData)
    {
        printf(ERR_LOG_TITLE "pUartData is NULL\r\n");
        return -1;
    }
    cmdType    = pUartData[4];
    currentCmd = pUartData[5];
    isAccept   = pUartData[6];
    status     = pUartData[7];
    memcpy(&userDefine, &pUartData[8], sizeof(userDefine));

    ret = _SS_Factory_Handle_Mcu_Msg(cmdType, currentCmd, isAccept, status, userDefine);
    if (ret < 0)
    {
        printf(ERR_LOG_TITLE "cmdType:%x currentCmd:%x isAccept:%x status:%x userDefine:%x recived msg error:%d\r\n",
               cmdType, currentCmd, isAccept, status, userDefine, ret);
        return -1;
    }
    return 0;
}
