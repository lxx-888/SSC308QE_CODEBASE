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
#ifndef __SSAPP_GLASSES_MISC_H
#define __SSAPP_GLASSES_MISC_H
#include "mi_common_datatype.h"

#define FTP_SERVER_PATH  "/customer/ai_glasses/"
#define FILE_NAME_LENGTH 256

MI_VIRT SSAPP_GLASSES_MISC_GetPts(void);

MI_S32 SSAPP_GLASSES_MISC_IsModuleLoaded(const char *moduleName);

MI_S32 SSAPP_GLASSES_MISC_CheckFtpPath(void);
MI_S32 SSAPP_GLASSES_MISC_Suspend(void);
MI_S32 SSAPP_GLASSES_MISC_Init(void);
MI_S32 SSAPP_GLASSES_MISC_DeInit(void);
#endif
