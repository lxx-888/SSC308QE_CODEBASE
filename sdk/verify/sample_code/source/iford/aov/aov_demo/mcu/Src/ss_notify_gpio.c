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
#include "ss_notify_gpio.h"
#include "ss_pir_wifi_wakeup.h"

long long suspend_ini_num   = 0;
long long basetimer_ini_num = 0;
long long pir_done_ini_num  = 0;
long long pd11done_pd6high  = 0;

// All GPIO IRQ
void GPIO_IRQHandler(void)
{
    if (FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_14)) // pd11
    {
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_14);
        if (!((FL_GPIO_ReadInputPort(GPIOD) & 0x800) >> 11))
        {
            suspend_ini_num++;

            MF_Config_Init();
            FL_BSTIM32_WriteCounter(BSTIM32, 0);
            FL_BSTIM32_EnableIT_Update(BSTIM32);

            if (!FL_BSTIM32_IsEnabled(BSTIM32))
            {
                FL_BSTIM32_Enable(BSTIM32);
            }

            FL_GPIO_ResetOutputPin(GPIOC, FL_GPIO_PIN_1);
            FL_GPIO_ResetOutputPin(GPIOC, FL_GPIO_PIN_0);

            SS_Pir_Wifi_Done();
        }
    }

    if (FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2))
    { // key1
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2);
        DelayMs(8); // 8ms for key debounce
        SS_Pir_Wifi_Wakeup_Soc();
        SS_Pir_Comming();
    }
    if (FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_9))
    { // key4
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_9);
        DelayMs(8); // 8ms for key debounce
        SS_Pir_Wifi_Wakeup_Soc();
        SS_Wifi_Comming();
    }
    if (FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_13))
    {
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_13); // pd6

        pir_done_ini_num++;

        SS_Pir_Done();
        SS_Wifi_Done();
    }
}

void BSTIM_IRQHandler(void)
{
    if (FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        basetimer_ini_num++;
        FL_BSTIM32_ClearFlag_Update(BSTIM32);

        FL_GPIO_SetOutputPin(GPIOC, FL_GPIO_PIN_1);
        FL_GPIO_SetOutputPin(GPIOC, FL_GPIO_PIN_0);

        SS_Pir_Wifi_Wakeup_Soc();
        FL_BSTIM32_Disable(BSTIM32);
    }
}

// PD11 init notify gpio
void SS_Notify_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    // FL_RCC_EnableEXTIOnSleep();
    FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI); // EXTI Work clock

    stGpioType.pin        = FL_GPIO_PIN_11;
    stGpioType.mode       = FL_GPIO_MODE_INPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;
    FL_GPIO_Init(GPIOD, &stGpioType);
}

// notify gpio exit interrupt init
void SS_Notify_Gpio_Exit_Interrupt_Init(void)
{
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_14, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE); // disable edge select

    FL_GPIO_SetExtiLine14(GPIO, FL_GPIO_EXTI_LINE_14_PD11);
    FL_GPIO_EnableDigitalFilter(GPIO, FL_GPIO_EXTI_LINE_14);
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_14, FL_GPIO_EXTI_TRIGGER_EDGE_FALLING);
    FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_14);

    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);
    NVIC_EnableIRQ(GPIO_IRQn);
}