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

#ifndef __SSAPP_GLASSES_MEDIA_H
#define __SSAPP_GLASSES_MEDIA_H

#include "mi_common_datatype.h"

MI_S32 SSAPP_GLASSES_MEDIA_Init(void);
MI_S32 SSAPP_GLASSES_MEDIA_DeInit(void);
MI_S32 SSAPP_GLASSES_MEDIA_TakePhoto(MI_U8 u8PhotoCnt);
MI_S32 SSAPP_GLASSES_MEDIA_StartRec(void);
MI_S32 SSAPP_GLASSES_MEDIA_StopRec(void);

#endif /* ifndef __SSAPP_GLASSES_MEDIA_H */
