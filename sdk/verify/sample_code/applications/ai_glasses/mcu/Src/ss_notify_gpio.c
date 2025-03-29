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

#include "main.h"
#include "ss_gpio.h"
#include "ss_task_list.h"

//Òý½ÅÖÐ¶Ï
void GPIO_IRQHandler(void)
{
    // junqiang.bi take photo button(PA2)
    if (FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_0))
    {
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_0);
        SS_TaskList_NewAdd_Pending_Task(E_TASK_PHOTO);
    }
    // junqiang.bi record picture button(PA4)
    if (FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_1))
    {
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_1);
        SS_TaskList_CheckCmd_Doing_Or_Replace_New_Cmd(E_TASK_START_REC, E_TASK_STOP_REC);
    }

    // junqiang.bi wifi grab button(PA11)
    if (FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2))
    {
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2);
        SS_TaskList_CheckCmd_Doing_Or_Replace_New_Cmd(E_TASK_TRANS, E_TASK_STOP_TRANS);
    }
}
