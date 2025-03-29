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
#include "mi_fb.h"

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_FB_Init);
EXPORT_SYMBOL(MI_FB_DeInit);
EXPORT_SYMBOL(MI_FB_Open);
EXPORT_SYMBOL(MI_FB_Close);
EXPORT_SYMBOL(MI_FB_Mmap);
EXPORT_SYMBOL(MI_FB_PanDisplay);
EXPORT_SYMBOL(MI_FB_WaitForVsync);
EXPORT_SYMBOL(MI_FB_GetVarScreenInfo);
EXPORT_SYMBOL(MI_FB_SetVarScreenInfo);
EXPORT_SYMBOL(MI_FB_GetFixScreenInfo);
EXPORT_SYMBOL(MI_FB_Munmap);
EXPORT_SYMBOL(MI_FB_GetScreenLocation);
EXPORT_SYMBOL(MI_FB_SetScreenLocation);
EXPORT_SYMBOL(MI_FB_GetDisplayLayerAttr);
EXPORT_SYMBOL(MI_FB_SetDisplayLayerAttr);
EXPORT_SYMBOL(MI_FB_GetCursorAttr);
EXPORT_SYMBOL(MI_FB_SetCursorAttr);
EXPORT_SYMBOL(MI_FB_GetAlpha);
EXPORT_SYMBOL(MI_FB_SetAlpha);
EXPORT_SYMBOL(MI_FB_GetColorKey);
EXPORT_SYMBOL(MI_FB_SetColorKey);
EXPORT_SYMBOL(MI_FB_SetCmap);
EXPORT_SYMBOL(MI_FB_GetCmap);
EXPORT_SYMBOL(MI_FB_GetShow);
EXPORT_SYMBOL(MI_FB_SetShow);
EXPORT_SYMBOL(MI_FB_GetCompressionInfo);
EXPORT_SYMBOL(MI_FB_SetCompressionInfo);
#endif
