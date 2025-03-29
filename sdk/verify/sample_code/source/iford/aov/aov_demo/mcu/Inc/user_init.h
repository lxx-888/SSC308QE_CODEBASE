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

#ifndef __USER_INIT_H__
#define __USER_INIT_H__

#include "main.h"
#ifndef MFANG
#include <stdio.h>
#endif

#ifndef MFANG

#define LED0_GPIO GPIOC
#define LED0_PIN  FL_GPIO_PIN_0

#define LED0_ON()  FL_GPIO_ResetOutputPin(LED0_GPIO, LED0_PIN)
#define LED0_OFF() FL_GPIO_SetOutputPin(LED0_GPIO, LED0_PIN)
#define LED0_TOG() FL_GPIO_ToggleOutputPin(LED0_GPIO, LED0_PIN)

#endif

void UserInit(void);
void FoutInit(void);
void DelayUs(uint32_t count);
void DelayMs(uint32_t count);

#endif
