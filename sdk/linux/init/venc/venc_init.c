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
#include "venc_init.h"
#include "mi_venc_datatype.h"

#if defined(CONFIG_ARM64)
#define VOIDP2UINT(addr) ((u64)(addr))
#define UINT2VOIDP(addr) ((void*)(u64)(addr))
#else
#define VOIDP2UINT(addr) ((unsigned int)(addr))
#define UINT2VOIDP(addr) ((void*)(unsigned int)(addr))
#endif

#ifndef EXTRA_MODULE_NAME
#define EXTRA_MODULE_NAME venc
#endif

extern MI_VENC_UserRcCallback_t     g_stVencUserCb;
EXPORT_SYMBOL(g_stVencUserCb);

void* DrvVencOfFindCompatibleNode(void *pDevNode, const char *type, const char *compat)
{
    return of_find_compatible_node((struct device_node *)pDevNode, type, compat);
}

int DrvVencOfDeviceIsAvailable(void *pDevNode)
{
    return of_device_is_available((struct device_node *)pDevNode) ? 1 : 0;
}

void* DrvVencOfFindProperty(void *pDevNode, char *name, int *cnt)
{
    return of_find_property((struct device_node *)pDevNode, name, cnt);
}

int DrvVencMsysOfPropertyReadU32Index(void *pDevNode, const char *propname, unsigned int u32ClkIdx, unsigned int *val)
{
    return msys_of_property_read_u32_index((struct device_node *)pDevNode, propname, u32ClkIdx, val);
}

int DrvVencCallUserModeHelper(char *path, char **argv, char **envp)
{
    return call_usermodehelper(path, argv, envp, UMH_WAIT_PROC);
}

void* DrvVencCamClkGetHw(void *pstClk)
{
    return __CamClkGetHw((struct clk*)pstClk);
}

void* DrvVencCamClkGetParentByIndex(void *pstClkHw, unsigned int u32ClockIdx)
{
    struct clk_hw *pClkHw = CamClkGetParentByIndex((struct clk_hw*)pstClkHw, u32ClockIdx);
    if (pClkHw == NULL)
    {
        return NULL;
    }
    return pClkHw->clk;
}

int DrvVencCamOfIrqToResource(void *pDevNode, int s32DevIdx, void *pRes)
{
    return CamOfIrqToResource((struct device_node *)pDevNode, s32DevIdx, (struct resource *)pRes);
}

int DrvVencCamOfAddressToResource(void *pDevNode, int s32DevIdx, void **base, int *size)
{
    struct resource     stRes;
    int                 ret      = -1;

    ret = CamOfAddressToResource((struct device_node *)pDevNode, s32DevIdx, &stRes);
    if (ret)
    {
        printk("%s:%d get reg base err.\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (base)
    {
        *base = UINT2VOIDP(stRes.start);
    }
    if (size)
    {
        *size = stRes.end - stRes.start + 1;
    }

    return 0;
}

unsigned int DrvVencCamIrqOfParseAndMap(void *pDevNode, int s32DevIdx)
{
    return CamIrqOfParseAndMap((struct device_node *)pDevNode, s32DevIdx);
}

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar"); // NOLINT
