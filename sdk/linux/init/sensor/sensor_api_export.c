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
#include "mi_sensor.h"
#include "mi_sensor_datatype.h"

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_SNR_Enable);
EXPORT_SYMBOL(MI_SNR_Disable);
EXPORT_SYMBOL(MI_SNR_GetPadInfo);
EXPORT_SYMBOL(MI_SNR_GetPlaneInfo);
EXPORT_SYMBOL(MI_SNR_GetFps);
EXPORT_SYMBOL(MI_SNR_SetFps);
EXPORT_SYMBOL(MI_SNR_GetAnadecSrcAttr);
EXPORT_SYMBOL(MI_SNR_SetAnadecSrcAttr);
EXPORT_SYMBOL(MI_SNR_QueryResCount);
EXPORT_SYMBOL(MI_SNR_GetRes);
EXPORT_SYMBOL(MI_SNR_GetCurRes);
EXPORT_SYMBOL(MI_SNR_SetRes);
EXPORT_SYMBOL(MI_SNR_SetOrien);
EXPORT_SYMBOL(MI_SNR_GetOrien);
EXPORT_SYMBOL(MI_SNR_SetPlaneMode);
EXPORT_SYMBOL(MI_SNR_GetPlaneMode);
EXPORT_SYMBOL(MI_SNR_CustFunction);
#endif

