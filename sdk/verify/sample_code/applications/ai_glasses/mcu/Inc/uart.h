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

#ifndef __UART_H__
#define __UART_H__
#include "main.h"

#define UART0_DEF 0
#define UART5_DEF 5
#define CONFIG_CMD_UART UART0_DEF

#if CONFIG_CMD_UART == UART0_DEF
#define SS_UART_HANDLE UART0
#elif CONFIG_CMD_UART == UART5_DEF
#define SS_UART_HANDLE UART5
#endif

void Test_Uart0(void);
//extern void Test_Uart1();
//extern void Test_Uart4();
//extern void Test_Uart5();
void Uart0_Init(void);
void Test1_Uart0(void);
void SS_Send_Cmd_to_Def_UART(uint8_t * TestTxData,int len);
#endif
