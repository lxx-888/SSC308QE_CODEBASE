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
#include <linux/mm.h>
#include "mi_common.h"
#include "mi_common_internal.h"

MI_U32 MI_Debug_FreeReservedArea(ss_phys_addr_t start, ss_phys_addr_t end, int poison, const char *s)
{
    return free_reserved_area(__va(start), __va(end), poison, s);
}

DECLEAR_MODULE_INIT_EXIT
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Sigmastar");
