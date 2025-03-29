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

#ifndef _ST_GLASSES_UART_H
#define _ST_GLASSES_UART_H

#include "mi_common_datatype.h"

MI_S32 ST_Common_AiGlasses_Uart_Init(void* protcolTaskHandler, MI_BOOL bEnableDma);
MI_S32 ST_Common_AiGlasses_Uart_DeInit(void);
MI_S32 ST_Common_AiGlasses_Uart_ReadThread_CanExit(void);

MI_S32 ST_Common_AiGlasses_Uart_Write(char* buf, int len);

#endif
