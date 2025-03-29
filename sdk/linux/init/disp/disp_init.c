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

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include "cam_sysfs.h"
#include "drv_clock.h"
#include "cam_clkgen.h"
#include "mi_common_internal.h"
#include "mi_common.h"
#include "mi_common_datatype.h"
#include "cam_os_wrapper.h"
#include "ms_msys.h"

#define DRV_DISP_DEVICE_COUNT 1
#define DRV_DISP_DEVICE_NAME  "mdisp"
#define DISP_ERR CamOsPrintf

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
static MI_U64              u64DispDmaMask = 0xffffffffUL;
static struct device_node *g_pstDispDeviceNode = NULL;

typedef struct DRV_DISP_MODULE_Device_s
{
    int refCnt;
} DRV_DISP_MODULE_Device_t;

static DRV_DISP_MODULE_Device_t g_tDispDevice = {
    .refCnt = 0,
};

static struct platform_device g_stDrvDispPlatformDevice = {
    .name = DRV_DISP_DEVICE_NAME,
    .id   = 0,
    .dev  = {
        .of_node = NULL,
        .dma_mask = &u64DispDmaMask,
        .coherent_dma_mask = 0xffffffffUL
    },
};

struct MHAL_DISP_DisplayInfo_s;

extern MI_U8 DRV_DISP_IF_DeviceGetInstance(MI_U32 u32DeviceId, void **pDevCtx);
extern MI_U8 DRV_DISP_IF_DeviceGetHwCount(MI_U32 *pu32Count);
extern MI_U8 DRV_DISP_IF_DeviceGetDisplayInfo(void *pDevCtx, struct MHAL_DISP_DisplayInfo_s *pstDisplayInfo);
extern MI_U8 DRV_DISP_IRQ_GetIsrNumByDevId(MI_U32 u32DevId, MI_U32 *pu32IsrNum);
extern void DRV_DISP_IRQ_SetIsrNum(MI_U32 u32DevId, MI_U32 u32IsrNum);
extern MI_U16 DRV_DISP_IRQ_GetIrqCount(void);
extern MI_U8 DRV_DISP_IF_ClkOn(void);
extern MI_U8 DRV_DISP_IF_ClkOff(void);
extern MI_U8 DRV_DISP_CTX_Init(void);
extern MI_U8 DRV_DISP_CTX_DeInit(void);
extern MI_U8 HAL_DISP_CLK_GetClkMuxAttr(MI_U32 u32Idx);
extern MI_U32 g_u32DispClkNum;
extern MI_U32 g_u32DispClkDispPllId;
//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------

static void DRV_DISP_GetIrqNum(struct platform_device *pDev, MI_U8 u8Idx, MI_U8 u8DevId)
{
    MI_U32 u32DispIrqId = 0; // INT_IRQ_AU_SYSTEM;

    u32DispIrqId = CamOfIrqToResource(pDev->dev.of_node, u8Idx, NULL);

    if (!u32DispIrqId)
    {
        DISP_ERR("[DISPMODULE] Can't Get SCL_IRQ, Idx=%d, DevId=%d\n", u8Idx, u8DevId);
    }
    else
    {
        DRV_DISP_IRQ_SetIsrNum(u8DevId, u32DispIrqId);
    }
}

static MI_U8 DRV_DISP_OS_SetDeviceNode(void *pPlatFormDev)
{
    g_pstDispDeviceNode = ((struct platform_device *)pPlatFormDev)->dev.of_node;

    if (!g_pstDispDeviceNode)
    {
        DISP_ERR("%s %d DeviceNode is NULL!!\n", __FUNCTION__, __LINE__);
        return false;
    }
    return true;
}

#ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
void **g_pvDispclk        = NULL;
MI_U32 g_u32DispParentCnt = 1;

static MI_U8 DRV_DISP_OS_ClkRegister(struct device_node *device_node)
{
    MI_U32 u32clknum;
    MI_U32 u32DispClk;
    MI_U8  a8str[16];

    if (of_find_property(device_node, "camclk", &g_u32DispParentCnt))
    {
        g_u32DispParentCnt /= sizeof(MI_U32);
        if (g_u32DispParentCnt < 0)
        {
            CamOsPrintf("[%s] Fail to get parent count! Error Number : %d\n", __func__, g_u32DispParentCnt);
            return 0;
        }
        g_pvDispclk = CamOsMemAlloc((sizeof(void *) * g_u32DispParentCnt));
        if (!g_pvDispclk)
        {
            return 0;
        }
        for (u32clknum = 0; u32clknum < g_u32DispParentCnt; u32clknum++)
        {
            u32DispClk = 0;
            msys_of_property_read_u32_index(device_node, "camclk", u32clknum, &(u32DispClk));
            if (!u32DispClk)
            {
                CamOsPrintf("[%s] Fail to get clk!\n", __func__);
            }
            else
            {
                CamOsSnprintf(a8str, 16, "Disp_%d ", u32clknum);
                CamClkRegister(a8str, u32DispClk, &(g_pvDispclk[u32clknum]));
            }
        }
    }
    else
    {
        CamOsPrintf("[%s] W/O Camclk \n", __func__);
    }
    return 1;
}

static MI_U8 DRV_DISP_OS_ClkUnregister(void)
{
    MI_U32 u32clknum;

    for (u32clknum = 0; u32clknum < g_u32DispParentCnt; u32clknum++)
    {
        if (g_pvDispclk[u32clknum])
        {
            CamOsPrintf(KERN_DEBUG "[%s] %px\n", __func__, g_pvDispclk[u32clknum]);
            CamClkUnregister(g_pvDispclk[u32clknum]);
            g_pvDispclk[u32clknum] = NULL;
        }
    }
    CamOsMemRelease(g_pvDispclk);

    return 1;
}
#endif

MI_U8 DRV_DISP_OS_SetClkOn(void *pClkEn, void *pClkRate, MI_U32 u32ClkRateSize)
{
    MI_U8   bRet          = true;
    MI_U32  u32Clkrate    = 0;
    MI_U32 *p32ClkRatebuf = (MI_U32 *)pClkRate;
    MI_U8 * p8Clkenbuf    = (MI_U8 *)pClkEn;

    MI_U32 u32Numparents = 0, u32Idx = 0;
    void * pstDispclk       = NULL;
    void * pstDispclkParent = NULL;

    if (g_pstDispDeviceNode && pClkRate && u32ClkRateSize == g_u32DispClkNum)
    {
        u32Numparents = sstar_clk_get_num_in_dev_node(g_pstDispDeviceNode);

        if (u32Numparents == g_u32DispClkNum)
        {
            for (u32Idx = 0; u32Idx < u32Numparents; u32Idx++)
            {
                pstDispclk = sstar_clk_get(g_pstDispDeviceNode, u32Idx);
                if (IS_ERR(pstDispclk))
                {
                    DISP_ERR("%s %d, Fail to get [Disp] %d\n", __FUNCTION__, __LINE__, u32Idx);
                    return 0;
                }

                if (HAL_DISP_CLK_GetClkMuxAttr(u32Idx) == 1)
                {
                    u32Clkrate  = p32ClkRatebuf[u32Idx];
                    pstDispclkParent = sstar_clk_get_parent_by_index(pstDispclk, u32Clkrate);
                    sstar_clk_set_parent(pstDispclk, pstDispclkParent);
                }
                else if(!p8Clkenbuf[u32Idx])
                {
                    u32Clkrate = sstar_clk_round_rate(pstDispclk, p32ClkRatebuf[u32Idx]);
                    sstar_clk_set_rate(pstDispclk, u32Clkrate + 1000000);
                }

                sstar_clk_prepare_enable(pstDispclk);
                sstar_clk_put(pstDispclk);
            }
        }
        else
        {
            bRet = false;
            DISP_ERR("%s %d, u32Numparents %d != %d\n", __FUNCTION__, __LINE__, u32Numparents, g_u32DispClkNum);
        }
    }
    else
    {
        bRet = false;
        DISP_ERR("%s %d, Param Null, DeviceNode:%x, ClkRate:%x, ClkSize:%d\n", __FUNCTION__, __LINE__,
                 g_pstDispDeviceNode, pClkRate, u32ClkRateSize);
    }
    return bRet;
}

MI_U8 DRV_DISP_OS_SetClkOff(void *pClkEn, void *pClkRate, MI_U32 u32ClkRateSize)
{
    MI_U8   bRet          = true;
    MI_U32  u32Clkrate    = 0;
    MI_U32 *p32ClkRatebuf = (MI_U32 *)pClkRate;
//    MI_U8 * p8Clkenbuf    = (MI_U8 *)pClkEn;
    MI_U32  u32Numparents = 0, u32Idx = 0;
    void *  pstDispclk       = NULL;
    void *  pstDispclkParent = NULL;

    UNUSED(pClkRate);
    UNUSED(u32ClkRateSize);

    if (g_pstDispDeviceNode)
    {
        u32Numparents = sstar_clk_get_num_in_dev_node(g_pstDispDeviceNode);
        if (u32Numparents == g_u32DispClkNum)
        {
            for (u32Idx = 0; u32Idx < u32Numparents; u32Idx++)
            {
                pstDispclk = sstar_clk_get(g_pstDispDeviceNode, u32Idx);
                if (IS_ERR(pstDispclk))
                {
                    DISP_ERR("%s %d, Fail to get [Disp] %d\n", __FUNCTION__, __LINE__, u32Idx);
                    return 0;
                }

                if (HAL_DISP_CLK_GetClkMuxAttr(u32Idx) == 1)
                {
                    u32Clkrate  = p32ClkRatebuf[u32Idx];
                    pstDispclkParent = sstar_clk_get_parent_by_index(pstDispclk, u32Clkrate);
                    sstar_clk_set_parent(pstDispclk, pstDispclkParent);
                }
                else
                {
                    u32Clkrate = sstar_clk_round_rate(pstDispclk, p32ClkRatebuf[u32Idx]);
                    sstar_clk_set_rate(pstDispclk, u32Clkrate + 1000000);
                }

                sstar_clk_disable_unprepare(pstDispclk);
                sstar_clk_put(pstDispclk);
            }
        }
        else
        {
            bRet = false;
            DISP_ERR("%s %d, u32Numparents %d != %d\n", __FUNCTION__, __LINE__, u32Numparents, g_u32DispClkNum);
        }
    }
    else
    {
        bRet = false;
        DISP_ERR("%s %d, Param Null, DeviceNode:%x\n", __FUNCTION__, __LINE__, g_pstDispDeviceNode);
    }
    return bRet;
}

MI_U8 DRV_DISP_OS_SetLpllClkConfig(MI_U64 u64Dclk, MI_U8 bDsi)
{
    MI_U8  bRet          = true;
    void * pstLpllclk    = NULL;
    MI_U32 u32Numparents = 0;

    if (g_pstDispDeviceNode)
    {
        u32Numparents = sstar_clk_get_num_in_dev_node(g_pstDispDeviceNode);
        if (u32Numparents == g_u32DispClkNum)
        {
            pstLpllclk = sstar_clk_get(g_pstDispDeviceNode, g_u32DispClkDispPllId);
            if (IS_ERR(pstLpllclk))
            {
                DISP_ERR("%s %d, Fail to get [Disp] %d\n", __FUNCTION__, __LINE__, g_u32DispClkDispPllId);
                return false;
            }

            if (!sstar_clk_is_enabled(pstLpllclk))
            {
                sstar_clk_prepare_enable(pstLpllclk);
            }
            if (bDsi)
            {
                sstar_clk_set_rate(pstLpllclk, (MI_U32)(u64Dclk / 8));
            }
            else
            {
                sstar_clk_set_rate(pstLpllclk, (MI_U32)(u64Dclk));
            }
            sstar_clk_put(pstLpllclk);
        }
        else
        {
            bRet = false;
            DISP_ERR("%s %d, u32Numparents %d != %d\n", __FUNCTION__, __LINE__, u32Numparents, g_u32DispClkNum);
        }
    }
    return bRet;
}

MI_U8 DRV_DISP_GetDeviceInstance(MI_U32 u32DeviceId, void **pDevCtx)
{
    return DRV_DISP_IF_DeviceGetInstance(u32DeviceId, pDevCtx);
}
EXPORT_SYMBOL(DRV_DISP_GetDeviceInstance);

void DRV_DISP_ModuleDeviceInit(void)
{
    MI_U32 s32Ret;
    MI_U8  i, u8IrqCnt;

    if (g_tDispDevice.refCnt == 0)
    {
        g_tDispDevice.refCnt++;

        if (g_stDrvDispPlatformDevice.dev.of_node == NULL)
        {
            g_stDrvDispPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sstar,disp");
        }
        if (g_stDrvDispPlatformDevice.dev.of_node == NULL)
        {
            DISP_ERR("[DISP INIT] Get Device mode Fail!!\n");
        }
    }

    // Get IRQ
    u8IrqCnt = DRV_DISP_IRQ_GetIrqCount();
    for (i = 0; i < u8IrqCnt; i++)
    {
        DRV_DISP_GetIrqNum(&g_stDrvDispPlatformDevice, i, i);
    }

    DRV_DISP_IF_ClkOn();
    DRV_DISP_CTX_Init();
    DRV_DISP_OS_SetDeviceNode(&g_stDrvDispPlatformDevice);
}

void DRV_DISP_ModuleDeviceDeInit(void)
{
    if (g_tDispDevice.refCnt)
    {
        g_tDispDevice.refCnt--;
    }

    if (g_tDispDevice.refCnt == 0)
    {
        g_stDrvDispPlatformDevice.dev.of_node = NULL;
    }

    DRV_DISP_IF_ClkOff();
    DRV_DISP_CTX_DeInit();
}

#if CONFIG_MI_DISP
DECLEAR_MODULE_INIT_EXIT
#else
#pragma message("compile only mhal")
int __init mhal_disp_insmod(void)
{
    CamOsPrintf("mhal module [%s] init\n", MACRO_TO_STRING(EXTRA_MODULE_NAME));
    DRV_DISP_ModuleDeviceInit();
    return 0;
}

void __exit mhal_disp_rmmod(void)
{
    CamOsPrintf("mhal module [%s] deinit\n", MACRO_TO_STRING(EXTRA_MODULE_NAME));
    DRV_DISP_ModuleDeviceDeInit();
}

module_init(mhal_disp_insmod);
module_exit(mhal_disp_rmmod);
#endif

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(DRV_DISP_IF_DeviceGetInstance);
EXPORT_SYMBOL(DRV_DISP_IF_DeviceGetHwCount);
EXPORT_SYMBOL(DRV_DISP_IF_DeviceGetDisplayInfo);
EXPORT_SYMBOL(DRV_DISP_IRQ_GetIsrNumByDevId);
#endif

MI_MODULE_LICENSE();
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
MODULE_AUTHOR("SigmaStar");
