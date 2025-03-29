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
#include "scl_init.h"

#define HAL_SCL_MODULE_DEVICE_COMP_NAME "sigmastar,vpe"

extern MI_BOOL MI_SCL_CHECK_InputModeValid(HAL_SCL_COMMON_InModeType_e eInModeType);

void * HAL_SCL_MODULE_GetDevNodeHandle(void)
{
    return (void *)of_find_compatible_node(NULL, NULL, HAL_SCL_MODULE_DEVICE_COMP_NAME);
}

void HAL_SCL_MODULE_MatchIrqID(void *pDevNodeHandle,MI_U32 *pu32IrqID)
{
    HAL_SCL_COMMON_InModeType_e enHalInMode = E_HAL_SCL_COMMON_INMODE_ISP;
    MI_U8                       u8Idx       = 0;
    MI_U32                      u32IrqId    = 0;

    // Initial IRQ setting
    for (enHalInMode = E_HAL_SCL_COMMON_INMODE_ISP; enHalInMode < E_HAL_SCL_COMMON_INMODE_MAX; enHalInMode++)
    {
        if (MI_SCL_CHECK_InputModeValid(enHalInMode))
        {
            u32IrqId = CamOfIrqToResource(pDevNodeHandle, u8Idx, NULL);
            if (!u32IrqId)
            {
                printk("[%s @ %d] Can't get scl IRQ number\n", __FUNCTION__, __LINE__);
                return;
            }
            pu32IrqID[enHalInMode] = u32IrqId;
            u8Idx++;
        }
    }
}

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar");
