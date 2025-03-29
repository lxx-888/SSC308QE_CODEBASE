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
#include "mf_config.h"
#include "ss_gpio.h"
#include "ss_datatype.h"

volatile BOOL bWaitToPowerDone = FALSE;

uint8_t setPowerOffCnt = 0;

// junqiang.bi TEST something wrong that timer will call twice when first run
void BSTIM_IRQHandler(void)
{
    if (FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        FL_BSTIM32_ClearFlag_Update(BSTIM32);

        if (setPowerOffCnt > 0)
        {
            // 50ms to set poweroff done
            bWaitToPowerDone = FALSE;
            DEBUG_INFO("timer set bWaitToPowerDone to FALSE\r\n");
        }
        else
        {
            setPowerOffCnt++;
        }
        FL_BSTIM32_Disable(BSTIM32);
    }
}

void _SS_GPIO_Timer_To_Wait_PowerOff_Done(void)
{
    //  FL_BSTIM32_WriteCounter(BSTIM32,0);
    FL_BSTIM32_EnableIT_Update(BSTIM32);

    if (!FL_BSTIM32_IsEnabled(BSTIM32))
    {
        FL_BSTIM32_Enable(BSTIM32);
    }
}

// junqiang.bi  photo button  (PA2)
void _SS_GPIO_Photo_Init(void)
{
    FL_GPIO_InitTypeDef gpio_type;

    FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI); // EXTI工作时钟使能
    gpio_type.pin        = FL_GPIO_PIN_2;
    gpio_type.mode       = FL_GPIO_MODE_INPUT;
    gpio_type.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpio_type.pull       = DISABLE;
    gpio_type.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOA, &gpio_type);
}

// junqiang.bi photo button  (PA2) interrupt config
void _SS_GPIO_Photo_Interrupt_Init(void)
{
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_0, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE); // disable edge select

    //每条LINE 只能选一个引脚
    FL_GPIO_SetExtiLine10(GPIO, FL_GPIO_EXTI_LINE_0_PA2);                                //中断引脚选择
    FL_GPIO_EnableDigitalFilter(GPIO, FL_GPIO_EXTI_LINE_0);                              // EXTI数字滤波功能
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_0, FL_GPIO_EXTI_TRIGGER_EDGE_RISING); //边沿选择
    FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_0);                                   //清除中断标志

    /*NVIC中断配置*/
    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2); //中断优先级配置
    NVIC_EnableIRQ(GPIO_IRQn);
}

// junqiang.bi  Recording button  (PA4)
void _SS_GPIO_Recording_Init(void)
{
    FL_GPIO_InitTypeDef gpio_type;

    FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI); // EXTI工作时钟使能
    gpio_type.pin        = FL_GPIO_PIN_4;
    gpio_type.mode       = FL_GPIO_MODE_INPUT;
    gpio_type.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpio_type.pull       = DISABLE;
    gpio_type.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOA, &gpio_type);
}

// junqiang.bi Recording button  (PA4) interrupt config
void _SS_GPIO_Recording_Interrupt_Init(void)
{
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_1, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE); // disable edge select

    //每条LINE 只能选一个引脚
    FL_GPIO_SetExtiLine1(GPIO, FL_GPIO_EXTI_LINE_1_PA4);                                 //中断引脚选择
    FL_GPIO_EnableDigitalFilter(GPIO, FL_GPIO_EXTI_LINE_1);                              // EXTI数字滤波功能
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_1, FL_GPIO_EXTI_TRIGGER_EDGE_RISING); //边沿选择
    FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_1);                                   //清除中断标志

    /*NVIC中断配置*/
    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2); //中断优先级配置
    NVIC_EnableIRQ(GPIO_IRQn);
}

// junqiang.bi  Wifi_Grab button  (PA11)
void _SS_GPIO_Wifi_Grab_Init(void)
{
    FL_GPIO_InitTypeDef gpio_type;

    FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI); // EXTI工作时钟使能
    gpio_type.pin        = FL_GPIO_PIN_11;
    gpio_type.mode       = FL_GPIO_MODE_INPUT;
    gpio_type.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpio_type.pull       = DISABLE;
    gpio_type.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOA, &gpio_type);
}

// junqiang.bi Wifi_Grab button  (PA11) interrupt config
void _SS_GPIO_Wifi_Grab_Interrupt_Init(void)
{
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_2, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE); // disable edge select

    //每条LINE 只能选一个引脚
    FL_GPIO_SetExtiLine1(GPIO, FL_GPIO_EXTI_LINE_2_PA11);                                //中断引脚选择
    FL_GPIO_EnableDigitalFilter(GPIO, FL_GPIO_EXTI_LINE_2);                              // EXTI数字滤波功能
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_2, FL_GPIO_EXTI_TRIGGER_EDGE_RISING); //边沿选择
    FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2);                                   //清除中断标志

    /*NVIC中断配置*/
    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2); //中断优先级配置
    NVIC_EnableIRQ(GPIO_IRQn);
}

// junqiang.bi power_on soc (PA15)
void _SS_GPIO_PowerOn_Soc_Init(void)
{
    FL_GPIO_InitTypeDef gpio_type;

    gpio_type.pin        = FL_GPIO_PIN_15;
    gpio_type.mode       = FL_GPIO_MODE_OUTPUT;
    gpio_type.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpio_type.pull       = DISABLE;
    gpio_type.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOA, &gpio_type);
    // poweron soc when mcu power on
    FL_GPIO_SetOutputPin(GPIOA, FL_GPIO_PIN_15);
    // FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_15);
}

void SS_GPIO_PowerOn_Soc(void)
{
    FL_GPIO_SetOutputPin(GPIOA, FL_GPIO_PIN_15);
}

void SS_GPIO_PowerOff_Soc(void)
{
    FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_15);

    bWaitToPowerDone = TRUE; // timer 50ms to poweroff done
    _SS_GPIO_Timer_To_Wait_PowerOff_Done();
}

void SS_GPIO_Init_Config(void)
{
    // take photo
    _SS_GPIO_Photo_Init();
    _SS_GPIO_Photo_Interrupt_Init();

    // record pictures
    _SS_GPIO_Recording_Init();
    _SS_GPIO_Recording_Interrupt_Init();

    // grab photo/pictures
    _SS_GPIO_Wifi_Grab_Init();
    _SS_GPIO_Wifi_Grab_Interrupt_Init();

    // poweron soc
    _SS_GPIO_PowerOn_Soc_Init();
}
