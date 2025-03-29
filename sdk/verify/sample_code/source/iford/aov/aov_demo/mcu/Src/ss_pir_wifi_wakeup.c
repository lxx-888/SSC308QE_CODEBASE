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

#include "ss_pir_wifi_wakeup.h"
#include "main.h"

// PA10 : KEY1 gpio init
void SS_Key1_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI);
    stGpioType.pin        = FL_GPIO_PIN_10;
    stGpioType.mode       = FL_GPIO_MODE_INPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOA, &stGpioType);
}

// PA10 key1 interrupt config
void SS_Key1_Gpio_Interrupt_Init(void)
{
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_2, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE);

    FL_GPIO_SetExtiLine14(GPIO, FL_GPIO_EXTI_LINE_2_PA10);
    FL_GPIO_EnableDigitalFilter(GPIO, FL_GPIO_EXTI_LINE_2);
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_2, FL_GPIO_EXTI_TRIGGER_EDGE_RISING);
    FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2);

    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);
    NVIC_EnableIRQ(GPIO_IRQn);
}

// PC6 : KEY4 gpio init
void SS_Key4_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI);
    stGpioType.pin        = FL_GPIO_PIN_6;
    stGpioType.mode       = FL_GPIO_MODE_INPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOC, &stGpioType);
}

// PC6 : key4 interrupt config
void SS_Key4_Gpio_Interrupt_Init(void)
{
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_9, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE); // disable edge select

    FL_GPIO_SetExtiLine14(GPIO, FL_GPIO_EXTI_LINE_9_PC6);
    FL_GPIO_EnableDigitalFilter(GPIO, FL_GPIO_EXTI_LINE_9);
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_9, FL_GPIO_EXTI_TRIGGER_EDGE_RISING);
    FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_9);

    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);
    NVIC_EnableIRQ(GPIO_IRQn);
}

// PA0 : wakeup soc gpio init
void SS_Wakeup_Soc_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    stGpioType.pin        = FL_GPIO_PIN_0;
    stGpioType.mode       = FL_GPIO_MODE_OUTPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOA, &stGpioType);

    FL_GPIO_SetOutputPin(GPIOA, FL_GPIO_PIN_0);
}

void SS_Pir_Wifi_Wakeup_Soc(void)
{
    FL_GPIO_SetOutputPin(GPIOA, FL_GPIO_PIN_0);
}

void SS_Pir_Wifi_Done(void)
{
    FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_0);
}

// PD3 : pir flags gpio init
void SS_Pir_Flags_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    stGpioType.pin        = FL_GPIO_PIN_3;
    stGpioType.mode       = FL_GPIO_MODE_OUTPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOD, &stGpioType);

    FL_GPIO_ResetOutputPin(GPIOD, FL_GPIO_PIN_3);
}

void SS_Pir_Comming(void)
{
    FL_GPIO_SetOutputPin(GPIOD, FL_GPIO_PIN_3);
}

void SS_Pir_Done(void)
{
    FL_GPIO_ResetOutputPin(GPIOD, FL_GPIO_PIN_3);
}

// PD4 : wifi flags gpio init
void SS_Wifi_Flags_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    stGpioType.pin        = FL_GPIO_PIN_4;
    stGpioType.mode       = FL_GPIO_MODE_OUTPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOD, &stGpioType);

    FL_GPIO_ResetOutputPin(GPIOD, FL_GPIO_PIN_4);
}

void SS_Wifi_Comming(void)
{
    FL_GPIO_SetOutputPin(GPIOD, FL_GPIO_PIN_4);
}

void SS_Wifi_Done(void)
{
    FL_GPIO_ResetOutputPin(GPIOD, FL_GPIO_PIN_4);
}

// PD6 init notify pir done gpio
void SS_Pir_Done_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    // FL_RCC_EnableEXTIOnSleep();

    stGpioType.pin        = FL_GPIO_PIN_6;
    stGpioType.mode       = FL_GPIO_MODE_INPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;
    FL_GPIO_Init(GPIOD, &stGpioType);
}

// notify pir done gpio exit interrupt init
void SS_Pir_Done_Gpio_Exit_Interrupt_Init(void)
{
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_13, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE);

    FL_GPIO_SetExtiLine14(GPIO, FL_GPIO_EXTI_LINE_13_PD6);
    FL_GPIO_EnableDigitalFilter(GPIO, FL_GPIO_EXTI_LINE_13);
    FL_GPIO_SetTriggerEdge(GPIO, FL_GPIO_EXTI_LINE_13, FL_GPIO_EXTI_TRIGGER_EDGE_FALLING);
    FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_13);

    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);
    NVIC_EnableIRQ(GPIO_IRQn);
}