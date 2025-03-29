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
#include "jpd_init.h"
#include "mi_jpd_datatype.h"

void DrvJpdClkPut(void *pClk)
{
    clk_put((struct clk*)pClk);
}

void* DrvJpdOfClkGet(void *pDevNode, int s32ClkIdx)
{
    return of_clk_get((struct device_node *)pDevNode, s32ClkIdx);
}

void* DrvJpdOfFindCompatibleNode(void *pDevNode, const char *pu8Type, const char *pu8Compat)
{
    return of_find_compatible_node((struct device_node *)pDevNode, pu8Type, pu8Compat);
}

void* DrvJpdOfFindProperty(void *pDevNode, char *pu8Name, int *ps32Cnt)
{
    return of_find_property((struct device_node *)pDevNode, pu8Name, ps32Cnt);
}

int DrvJpdMsysOfPropertyReadU32Index(void *pDevNode, const char *pu8PropName, unsigned int u32ClkIdx, unsigned int *pu32Val)
{
    return msys_of_property_read_u32_index((struct device_node *)pDevNode, pu8PropName, u32ClkIdx, pu32Val);
}

void* DrvJpdCamClkGetHw(void *pClk)
{
    return __CamClkGetHw((struct clk*)pClk);
}

unsigned int DrvJpdCamOfClkGetParentCount(void *pDevNode)
{
    return CamOfClkGetParentCount((struct device_node *)pDevNode);
}

void* DrvJpdCamClkGetParentByIndex(void *pClkHw, unsigned int u32ClockIdx)
{
    struct clk_hw *pstClkHw = CamClkGetParentByIndex((struct clk_hw*)pClkHw, u32ClockIdx);
    if (pstClkHw == NULL)
    {
        return NULL;
    }
    return pstClkHw->clk;
}

int DrvJpdCamClkSetRate(void *pClk, unsigned int u32ClkRate)
{
    return CamClkSetRate((struct clk*)pClk, u32ClkRate);
}

unsigned int DrvJpdCamClkGetRate(void *pClk)
{
    return CamClkGetRate((struct clk *)pClk);
}

int DrvJpdCamClkSetParent(void *pClk, void *pParentClk)
{
    return CamClkSetParent((struct clk*)pClk, (struct clk*)pParentClk);
}

void DrvJpdCamClkPrepareEnable(void *pClk)
{
    CamClkPrepareEnable((struct clk*)pClk);
}

void DrvJpdCamClkDisableUnprepare(void *pClk)
{
    CamClkDisableUnprepare((struct clk*)pClk);
}

int DrvJpdCamOfAddressToResource(void *pDevNode, int s32DevIdx, void **ppBase, unsigned int *pu32Size)
{
    struct resource stRes;
    int             s32Ret = -1;

    s32Ret = CamOfAddressToResource((struct device_node *)pDevNode, s32DevIdx, &stRes);
    if (s32Ret)
    {
        printk("%s:%d get reg base err.\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (ppBase)
    {
        *ppBase = (void *)IO_ADDRESS(stRes.start);
    }
    if (pu32Size)
    {
        *pu32Size = stRes.end - stRes.start;
    }

    return 0;
}

unsigned int DrvJpdCamIrqOfParseAndMap(void *pDevNode, int s32DevIdx)
{
    return CamIrqOfParseAndMap((struct device_node *)pDevNode, s32DevIdx);
}

int DrvJpdCamofPropertyReadU32(void *pDevNode, const char *pPropName, unsigned int *pu32Value)
{
    return CamofPropertyReadU32((struct device_node *)pDevNode, pPropName, pu32Value);
}

DECLEAR_MODULE_INIT_EXIT
MI_MODULE_LICENSE();
MODULE_AUTHOR("Sigmastar"); // NOLINT
