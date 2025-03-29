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
#include <linux/of_platform.h>
#include <linux/of_irq.h>

#include "cam_sysfs.h"
#include "mi_common.h"
#include "mi_common_internal.h"
#include "common_print.h"

DECLEAR_MODULE_INIT_EXIT

MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
MODULE_IMPORT_NS(ANDROID_GKI_VFS_EXPORT_ONLY);
#endif
