/* SigmaStar trade secret */
/* Copyright (c) [2021~2022] SigmaStar Technology.
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

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/file.h>
#include "cam_clkgen.h"
#include "cam_sysfs.h"
#include <ms_platform.h>
#include "cam_os_wrapper.h"

#include <mi_common_datatype.h>
#include <mi_common_internal.h>

MI_S32 DRV_NIR_GetIrqNum(MI_U8 u8DevId, MI_U32* pu32IrqNum)
{
    struct device_node* devNode = NULL;
    char                compatible[16];

    if (pu32IrqNum)
    {
        if (u8DevId)
        {
            CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs%d", u8DevId);
        }
        else
        {
            CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs");
        }

        devNode = of_find_compatible_node(NULL, NULL, compatible);

        if (!devNode)
        {
            return -1;
        }

        *pu32IrqNum = (MI_U32)CamIrqOfParseAndMap(devNode, 0);

        return 0;
    }

    return -1;
}

MI_S32 DRV_NIR_SetClk(MI_U8 u8DevId, MI_U32 u32ClkIndex, MI_BOOL bOnOff)
{
    struct device_node* devNode = NULL;
    struct clk*         clock   = NULL;
    char                compatible[16];

    if (u8DevId)
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs%d", u8DevId);
    }
    else
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs");
    }

    devNode = of_find_compatible_node(NULL, NULL, compatible);
    if (!devNode)
    {
        return -1;
    }

    clock = CamClkGetParentByIndex(__CamClkGetHw(of_clk_get(devNode, 0)), u32ClkIndex)->clk;
    if (!clock)
    {
        return -1;
    }

    if (CamClkSetParent(of_clk_get(devNode, 0), clock))
    {
        return -1;
    }

    if (bOnOff)
    {
        CamClkPrepareEnable(of_clk_get(devNode, 0));
    }
    else
    {
        CamClkDisableUnprepare(of_clk_get(devNode, 0));
    }

    return MI_SUCCESS;
}

DECLEAR_MODULE_INIT_EXIT

MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar");

