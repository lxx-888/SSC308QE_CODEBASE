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
#include <linux/module.h>
#include "mi_jpd.h"
#include "mi_jpd_datatype.h"

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_JPD_CreateDev);
EXPORT_SYMBOL(MI_JPD_DestroyDev);
EXPORT_SYMBOL(MI_JPD_CreateChn);
EXPORT_SYMBOL(MI_JPD_DestroyChn);
EXPORT_SYMBOL(MI_JPD_GetChnAttr);
EXPORT_SYMBOL(MI_JPD_StartChn);
EXPORT_SYMBOL(MI_JPD_StopChn);
EXPORT_SYMBOL(MI_JPD_GetChnStatus);
EXPORT_SYMBOL(MI_JPD_ResetChn);
EXPORT_SYMBOL(MI_JPD_GetStreamBuf);
EXPORT_SYMBOL(MI_JPD_PutStreamBuf);
EXPORT_SYMBOL(MI_JPD_DropStreamBuf);
EXPORT_SYMBOL(MI_JPD_QueryStreamInfo);
EXPORT_SYMBOL(MI_JPD_DirectBufDecode);
EXPORT_SYMBOL(MI_JPD_PauseChn);
EXPORT_SYMBOL(MI_JPD_RefreshChn);
EXPORT_SYMBOL(MI_JPD_ResumeChn);
EXPORT_SYMBOL(MI_JPD_StepChn);
#endif
