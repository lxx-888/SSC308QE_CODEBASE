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
#include "ss_timer_gpio.h"

void Led12_Gpio_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    stGpioType.pin        = FL_GPIO_PIN_0 | FL_GPIO_PIN_1;
    stGpioType.mode       = FL_GPIO_MODE_OUTPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOC, &stGpioType);

    FL_GPIO_SetOutputPin(GPIOC, FL_GPIO_PIN_1);
    FL_GPIO_SetOutputPin(GPIOC, FL_GPIO_PIN_0);
}

void SS_Dram_pwr_Init(void)
{
    FL_GPIO_InitTypeDef stGpioType = {0};

    stGpioType.pin        = FL_GPIO_PIN_13 | FL_GPIO_PIN_14;
    stGpioType.mode       = FL_GPIO_MODE_OUTPUT;
    stGpioType.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    stGpioType.pull       = DISABLE;
    stGpioType.remapPin   = DISABLE;

    FL_GPIO_Init(GPIOB, &stGpioType);

    FL_GPIO_SetOutputPin(GPIOB, FL_GPIO_PIN_13);
    FL_GPIO_SetOutputPin(GPIOB, FL_GPIO_PIN_14);
}