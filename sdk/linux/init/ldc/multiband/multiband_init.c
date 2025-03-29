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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/file.h>
#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include <mi_common_datatype.h>

//==============================================================================
//
//                              Structure
//
//==============================================================================

//==============================================================================
//
//                              Variable
//
//==============================================================================

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================
///------------------------------------------------------------------------------
// Function    : DRV_Multiband_OfFindCompatibleNode
// Description :
//------------------------------------------------------------------------------
void *  DRV_Multiband_OfFindCompatibleNode(MI_U8 u8DevId)
{
    char                as8Compatible[16];

    if (u8DevId)
    {
        CamOsSnprintf(as8Compatible, sizeof(as8Compatible), "sstar,cvs%d", u8DevId);
    }
    else
    {
        CamOsSnprintf(as8Compatible, sizeof(as8Compatible), "sstar,cvs");
    }

    return of_find_compatible_node(NULL, NULL, as8Compatible);
}
//------------------------------------------------------------------------------
// Function    : DRV_Multiband_DeviceGetResourceIrq
// Description :
//------------------------------------------------------------------------------
MI_U32 DRV_Multiband_DeviceGetResourceIrq(MI_U8 u8DevId, MI_U32 *pu32IrqNum)
{
    struct device_node *    devNode = NULL;
    char                    compatible[16];
    struct platform_device *pdev = NULL;
    UNUSED(u8DevId);

    if (pu32IrqNum)
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs");

        devNode = of_find_compatible_node(NULL, NULL, compatible);

        if (!devNode)
        {
            return E_MI_ERR_UNEXIST;
        }

        pdev = of_find_device_by_node(devNode);
        if (!pdev)
        {
            of_node_put(devNode);
            return E_MI_ERR_UNEXIST;
        }

        *pu32IrqNum = (MI_U32)CamIrqOfParseAndMap(pdev->dev.of_node, 0);
        return MI_SUCCESS;
    }

    return E_MI_ERR_FAILED;
}

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================
