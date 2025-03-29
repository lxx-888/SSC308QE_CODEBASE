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
#include "mi_vif_datatype.h"
#include "mi_vif.h"

EXPORT_SYMBOL(MI_VIF_CreateDevGroup);
EXPORT_SYMBOL(MI_VIF_DestroyDevGroup);
EXPORT_SYMBOL(MI_VIF_GetDevGroupAttr);
EXPORT_SYMBOL(MI_VIF_SetDevGroupAttr);
EXPORT_SYMBOL(MI_VIF_SetDevAttr);
EXPORT_SYMBOL(MI_VIF_GetDevAttr);
EXPORT_SYMBOL(MI_VIF_EnableDev);
EXPORT_SYMBOL(MI_VIF_DisableDev);
EXPORT_SYMBOL(MI_VIF_GetDevStatus);
EXPORT_SYMBOL(MI_VIF_SetOutputPortAttr);
EXPORT_SYMBOL(MI_VIF_GetOutputPortAttr);
EXPORT_SYMBOL(MI_VIF_EnableOutputPort);
EXPORT_SYMBOL(MI_VIF_DisableOutputPort);
EXPORT_SYMBOL(MI_VIF_Query);

EXPORT_SYMBOL(MI_VIF_CallBackTask_Register);
EXPORT_SYMBOL(MI_VIF_CallBackTask_UnRegister);
EXPORT_SYMBOL(MI_VIF_CustFunction);

