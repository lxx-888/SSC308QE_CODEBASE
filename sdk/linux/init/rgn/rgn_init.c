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
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include "mi_common_internal.h"

#define _RGN_INIT_C_

#define DRV_RGN_DEVICE_NAME  "rgn"

typedef struct DRV_RGN_MODULE_Device_s
{
    int refCnt;
} DRV_RGN_MODULE_Device_t;

static DRV_RGN_MODULE_Device_t g_tRgnDevice = {
    .refCnt = 0,
};

static struct platform_device g_stDrvRgnPlatformDevice = {
    .name = DRV_RGN_DEVICE_NAME,
    .id   = 0,
    .dev  = {
        .of_node = NULL,
    },
};

extern void *MHAL_RGN_GetInternalApis(void);
extern void *mi_rgn_GetInternalApis(void);
extern MI_U32 MHAL_RGN_GetGopNumByType(MI_U32 eTargetType);
extern MI_U32 MHAL_RGN_GetTargetMax(void);
extern MI_S32 MHAL_RGN_SetIrqNum(MI_U32 eTargetType, MI_U8 u8Id, MI_U32 u32IrqNum);
#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MHAL_RGN_GetInternalApis);
EXPORT_SYMBOL(mi_rgn_GetInternalApis);
#endif
void DRV_RGN_MoudleDeviceInit(void)
{
    MI_U32 s32Ret;
    MI_U32 u32IrqId = 0, u32Index = 0;
    MI_U8  i, j;

    if (g_tRgnDevice.refCnt == 0)
    {
        if (g_stDrvRgnPlatformDevice.dev.of_node == NULL)
        {
            g_stDrvRgnPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sstar,rgn");
        }
        if (g_stDrvRgnPlatformDevice.dev.of_node == NULL)
        {
            CamOsPrintf("[RGN INIT] Get Device mode Fail!!\n");
            return;
        }
    }
    g_tRgnDevice.refCnt++;

    for (i = 0; i < MHAL_RGN_GetTargetMax(); i++)
    {
        for (j = 0; j < MHAL_RGN_GetGopNumByType(i); j++)
        {
            u32IrqId = CamOfIrqToResource(g_stDrvRgnPlatformDevice.dev.of_node, u32Index, NULL);
            if (!u32IrqId)
            {
                CamOsPrintf("[RGN MODULE] Can't Get GOP_IRQ\n");
                return;
            }
            if (0 != MHAL_RGN_SetIrqNum(i, j, u32IrqId))
            {
                continue;
            }
            u32Index++;
        }
    }
}

void DRV_RGN_ModuleDeviceDeinit(void)
{
    if (g_tRgnDevice.refCnt)
    {
        g_tRgnDevice.refCnt--;
    }

    if (g_tRgnDevice.refCnt == 0)
    {
        g_stDrvRgnPlatformDevice.dev.of_node = NULL;
    }
}

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("malloc.peng@sigmastar.com.cn");
