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
#include "uart.h"
#include <stdio.h>
#include "ss_timer_gpio.h"
#include "uart.h"
#include "ss_notify_gpio.h"
#include "ss_pir_wifi_wakeup.h"

extern long long suspend_ini_num;
extern long long basetimer_ini_num;
extern long long pir_done_ini_num;
extern long long pd11done_pd6high;
int              main(void)
{
    /* SHOULD BE KEPT!!! */
    MF_Clock_Init();

    /* Confiure the system clock */
    /* SHOULD BE KEPT!!! */
    MF_SystemClock_Config();
    /* user init */
    UserInit();
    Uart0_Init();
    printf("log uart init done\r\n");
    // Uart5_Init();
    Led12_Gpio_Init();
    SS_Key1_Gpio_Init();
    SS_Key1_Gpio_Interrupt_Init();
    SS_Key4_Gpio_Init();
    SS_Key4_Gpio_Interrupt_Init();
    SS_Wakeup_Soc_Gpio_Init();
    SS_Dram_pwr_Init();

    SS_Pir_Flags_Gpio_Init();
    SS_Wifi_Flags_Gpio_Init();
    SS_Pir_Done_Gpio_Init();
    SS_Pir_Done_Gpio_Exit_Interrupt_Init();
    SS_Notify_Gpio_Init();

    // do{

    //}while(!FL_GPIO_ReadInputPort(GPIOD) & 0x8000);

    SS_Notify_Gpio_Exit_Interrupt_Init();
    printf("mcu aov test\r\n");
    // FL_BSTIM32_Enable(BSTIM32);
    while (1)
    {
        printf("running\r\n");
        printf("suspend num %lld\n", suspend_ini_num);
        printf("base timer num %lld\n", basetimer_ini_num);
        printf("pir done num %lld\n", pir_done_ini_num);
        printf("pd6 %d\n", ((FL_GPIO_ReadInputPort(GPIOD) & 0x40) >> 6));
        printf("pd11done_pd6high %lld\n", pd11done_pd6high);
        DelayMs(500);
    }
}
