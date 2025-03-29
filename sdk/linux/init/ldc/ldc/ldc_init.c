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
#include <linux/err.h>
#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include <mi_common_datatype.h>
#include <mi_common_internal.h>
#include "drv_clock.h"


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

//------------------------------------------------------------------------------
// Function    : DRV_LDC_OfFindCompatibleNode
// Description :
//------------------------------------------------------------------------------
void * DRV_LDC_OfFindCompatibleNode(MI_U8 u8DevId)
{
    char                compatible[16];
    if (u8DevId)
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,ldc%d", u8DevId);
    }
    else
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,ldc");
    }

    return of_find_compatible_node(NULL, NULL, compatible);
}

//------------------------------------------------------------------------------
// Function    : DRV_LDC_DeviceGetResourceIrq
// Description :
//------------------------------------------------------------------------------
MI_U32 DRV_LDC_DeviceGetResourceIrq(MI_U8 u8DevId, MI_U32 *pu32IrqNum)
{
    struct device_node *devNode = NULL;
    char                compatible[16];

    if (pu32IrqNum)
    {
        if (u8DevId)
        {
            CamOsSnprintf(compatible, sizeof(compatible), "sstar,ldc%d", u8DevId);
        }
        else
        {
            CamOsSnprintf(compatible, sizeof(compatible), "sstar,ldc");
        }

        devNode = of_find_compatible_node(NULL, NULL, compatible);
        if (!devNode)
        {
            return E_MI_ERR_UNEXIST;
        }

        *pu32IrqNum = (MI_U32)CamIrqOfParseAndMap(devNode, 0);

        return MI_SUCCESS;
    }

    return E_MI_ERR_FAILED;
}

DECLEAR_MODULE_INIT_EXIT

MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar");

