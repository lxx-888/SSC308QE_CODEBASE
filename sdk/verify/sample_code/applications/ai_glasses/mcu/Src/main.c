#include "main.h"
#include "uart.h"
#include <stdio.h>
#include "ss_timer_gpio.h"
#include "uart.h"
#include "ss_gpio.h"
#include "ss_task_list.h"


/**
  ****************************************************************************************************
  * @file    main.c
  * @author  FMSH Application Team
  * @brief   Header file of  LL Module
  ****************************************************************************************************
  * @attention
  *
  * Copyright (c) [2019] [Fudan Microelectronics]
  * THIS SOFTWARE is licensed under the Mulan PSL v1.
  * can use this software according to the terms and conditions of the Mulan PSL v1.
  * You may obtain a copy of Mulan PSL v1 at:
  * http://license.coscl.org.cn/MulanPSL
  * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
  * PURPOSE.
  * See the Mulan PSL v1 for more details.
  *
  ****************************************************************************************************
  */


extern volatile BOOL bWaitToPowerDone;
int main(void)
{


    /* SHOULD BE KEPT!!! */
    MF_Clock_Init();

    /* Confiure the system clock */
    /* SHOULD BE KEPT!!! */
    MF_SystemClock_Config();
    /* user init */
    UserInit();

     // junqiang.bi init timer clk
    MF_Config_Init();
    //junqiang.bi init ring_buffer for uart rx
    SS_Ring_Buffer_Init();
#if CONFIG_CMD_UART == UART0_DEF
    Uart0_Init();
#endif
#if CONFIG_CMD_UART == UART5_DEF
    Uart5_Init();
#endif
    DEBUG_ERROR("log uart init done\r\n");


    //junqiang.bi init tasklist
    SS_TaskList_Init();
    //junqiang.bi init gpio

    SS_GPIO_Init_Config();
    //SS_Uart0_Rx_Dma_Init();

    while(1)
    {
       // SS_Uart0_Rx_Dma_BufferFull_Cp_To_RingBuffer();
       SS_Deal_Ring_Buffer_Data();
       if (!SS_TaskList_Is_Empty() && !bWaitToPowerDone)
       {
           //只要还有剩余task soc就应该一直上电
           SS_GPIO_PowerOn_Soc();
       }
       SS_TaskList_Auto_Send_Stop_Cmd_Task();
    }

}


