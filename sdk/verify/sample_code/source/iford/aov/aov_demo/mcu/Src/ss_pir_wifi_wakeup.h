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

#ifndef __SS_PIR_WIFI_WAKEUP_H
#define __SS_PIR_WIFI_WAKEUP_H

void SS_Key1_Gpio_Init(void);
void SS_Key1_Gpio_Interrupt_Init(void);
void SS_Key4_Gpio_Init(void);
void SS_Key4_Gpio_Interrupt_Init(void);
void SS_Wakeup_Soc_Gpio_Init(void);
void SS_Pir_Wifi_Wakeup_Soc(void);
void SS_Pir_Wifi_Done(void);
void SS_Pir_Flags_Gpio_Init(void);
void SS_Wifi_Flags_Gpio_Init(void);
void SS_Pir_Comming(void);
void SS_Pir_Done(void);
void SS_Wifi_Comming(void);
void SS_Wifi_Done(void);
void SS_Pir_Done_Gpio_Init(void);
void SS_Pir_Done_Gpio_Exit_Interrupt_Init(void);

#endif
