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
#include "cam_os_wrapper.h"
#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include <mi_common_datatype.h>

//==============================================================================
//
//                              EXTERN FUNCTION
//
//==============================================================================

//==============================================================================
//
//                              LOCAL FUNCTION
//
//==============================================================================

void * DRV_DIS_INIT_OfFindCompatibleNode(MI_U8 u8DevId)
{
    char                compatible[16];

    if (u8DevId)
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs%d", u8DevId);
    }
    else
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs");
    }

    return of_find_compatible_node(NULL, NULL, compatible);
}

//------------------------------------------------------------------------------
// Function    : DRV_DIS_INIT_GetResourceIrq
// Description :
//------------------------------------------------------------------------------
MI_U32 DRV_DIS_INIT_GetResourceIrq(MI_U8 u8DevId, MI_U32 *pu32IrqNum)
{
    struct device_node *devNode = NULL;
    char                compatible[16];

    if (pu32IrqNum)
    {
        CamOsSnprintf(compatible, sizeof(compatible), "sstar,cvs");
        devNode = of_find_compatible_node(NULL, NULL, compatible);

        if (!devNode)
        {
            return E_MI_ERR_UNEXIST;
        }

        *pu32IrqNum = (MI_U32)CamIrqOfParseAndMap(devNode, 0);

        return MI_SUCCESS;
    }

    return E_MI_ERR_UNEXIST;
}

//==============================================================================
//
//                              EXPORT SYMBOL
//
//==============================================================================
