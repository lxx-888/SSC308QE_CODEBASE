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
#if ((defined CONFIG_MI_VDEC) && (CONFIG_MI_VDEC == 1))

#include <linux/module.h>
#include "mi_scl.h"

struct mi_scl_internal_apis_s;
struct mi_scl_internal_apis_s * mi_scl_GetInternalApis(void);

EXPORT_SYMBOL(mi_scl_GetInternalApis);

#endif
