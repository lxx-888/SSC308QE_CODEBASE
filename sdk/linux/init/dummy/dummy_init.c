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
#include <linux/delay.h>

#include "cam_sysfs.h"
#include "mi_common.h"
#include "mi_common_internal.h"

extern int event_cmd;
extern int cmdqtimeout_gap;

module_param(event_cmd, int, S_IRUGO | S_IWUSR);
module_param(cmdqtimeout_gap, int, S_IRUGO | S_IWUSR);

extern int ramSize;
extern int eRamTypeCfg;
extern int ramSizeInDev0;
extern int ramSizeInDev1;
extern int eRamInDev0;
extern int eRamInDev1;

module_param(ramSize, int, S_IRUGO | S_IWUSR);
module_param(eRamTypeCfg, int, S_IRUGO | S_IWUSR);
module_param(ramSizeInDev0, int, S_IRUGO | S_IWUSR);
module_param(ramSizeInDev1, int, S_IRUGO | S_IWUSR);
module_param(eRamInDev0, int, S_IRUGO | S_IWUSR);
module_param(eRamInDev1, int, S_IRUGO | S_IWUSR);

DECLEAR_MODULE_INIT_EXIT
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Sigmastar");
