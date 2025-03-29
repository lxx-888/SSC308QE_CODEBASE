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

#ifndef __SS_TASK_FACTORY_H
#define __SS_TASK_FACTORY_H

#include "list/list.h"
#include "ss_task_list.h"

uint8_t SS_Factory_Send_Task_Protrcol_To_Uart(SS_TaskList_t* sendTask);
int8_t  SS_Factory_Uart_Data_Handle(uint8_t* pUartData);
#endif
