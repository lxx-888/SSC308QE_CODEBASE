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

#include "mi_common_internal.h"
#include "cam_sysfs.h"
#include "cam_clkgen.h"

#ifndef EXTRA_MODULE_NAME
#define EXTRA_MODULE_NAME ive
#endif

// for irq/bankbase
void* DrvIveOfFindCompatibleNode(void *pDevNode, const char *pu8Type, const char *pu8Compat)
{
    return of_find_compatible_node((struct device_node *)pDevNode, pu8Type, pu8Compat);
}

int DrvIveCamOfAddressToResource(void *pDevNode, int s32ResourceIdx, unsigned long long *pBankBase)
{
    struct resource stRes  = {0};

    if (CamOfAddressToResource((struct device_node *)pDevNode, s32ResourceIdx, &stRes))
    {
        printk("%s:%d get reg base err.\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (pBankBase)
    {
        *pBankBase = stRes.start;
    }

    return 0;
}

int DrvIveCamOfIrqToResource(void *pDevNode, int s32ResourceIdx, void *pRes)
{
    return CamOfIrqToResource((struct device_node *)pDevNode, s32ResourceIdx, (struct resource *)pRes);
}

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar"); // NOLINT
