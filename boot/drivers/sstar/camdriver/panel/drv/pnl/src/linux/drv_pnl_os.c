/*
 * drv_pnl_os.c- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#define _DRV_PNL_OS_C_

#include "drv_pnl_os.h"
#include "mhal_common.h"
#include "drv_padmux.h"
#include "pnl_debug.h"
#include "cam_clkgen.h"
#include "cam_sysfs.h"

#include "mhal_pnl_datatype.h"
#include "mhal_pnl.h"
#include "hal_pnl_chip.h"
#include "cam_fs_wrapper.h"

#include "drv_puse.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

struct device_node *g_gpstPnlDeviceNode = NULL;

//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------

void *DrvPnlOsMemAlloc(u32 u32Size)
{
    return CamOsMemAlloc(u32Size);
}

void DrvPnlOsMemRelease(void *pPtr)
{
    CamOsMemRelease(pPtr);
}

void DrvPnlOsMsSleep(u32 u32Msec)
{
    CamOsMsSleep(u32Msec);
}

void DrvPnlOsUsSleep(u32 u32Usec)
{
    CamOsUsSleep(u32Usec);
}

bool DrvPnlOsPadMuxActive(void)
{
    bool bRet = drv_padmux_active() ? 1 : 0;
    return bRet;
}

int DrvPnlOsPadMuxGetMode(void)
{
    return drv_padmux_getmode(MDRV_PUSE_TTL_CLK);
}

int DrvPnlOsGetMode(u16 u16LinkType, u16 u16OutputFormatBitMode)
{
    int ret     = 0;
    u32 u32Mode = 0;

    if (g_gpstPnlDeviceNode)
    {
        if (u16LinkType == DRV_PNL_OS_LINK_TTL)
        {
            if (u16OutputFormatBitMode == DRV_PNL_OS_OUTPUT_565BIT_MODE)
            {
                ret = CamofPropertyReadU32(g_gpstPnlDeviceNode, "ttl-16bit-mode", &u32Mode);
            }
            else
            {
                ret = CamofPropertyReadU32(g_gpstPnlDeviceNode, "ttl-24bit-mode", &u32Mode);
            }
        }
    }

    return (ret == 0) ? u32Mode : 0;
}

bool DrvPnlOsSetDeviceNode(void *pPlatFormDev)
{
    int ret             = 0;
    g_gpstPnlDeviceNode = ((struct platform_device *)pPlatFormDev)->dev.of_node;

    if (g_gpstPnlDeviceNode)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

#ifdef CONFIG_CAM_CLK
#include "drv_camclk_Api.h"
void **pvPnlclk     = NULL;
u32    PnlParentCnt = 1;

u8 Pnl_ClkRegister(struct device_node *device_node)
{
    u32 u32clknum;
    u32 PnlClk;
    u8  str[16];

    if (of_find_property(device_node, "camclk", &PnlParentCnt))
    {
        PnlParentCnt /= sizeof(int);
        if (PnlParentCnt < 0)
        {
            CamOsPrintf("[%s] Fail to get parent count! Error Number : %d\n", __FUNCTION__, PnlParentCnt);
            return 0;
        }
        pvPnlclk = CamOsMemAlloc((sizeof(void *) * PnlParentCnt));
        if (!pvPnlclk)
        {
            return 0;
        }
        for (u32clknum = 0; u32clknum < PnlParentCnt; u32clknum++)
        {
            PnlClk = 0;
            msys_of_property_read_u32_index(device_node, "camclk", u32clknum, &(PnlClk));
            if (!PnlClk)
            {
                printk("[%s] Fail to get clk!\n", __FUNCTION__);
            }
            else
            {
                CamOsSnprintf(str, 16, "Pnl_%d ", u32clknum);
                CamClkRegister(str, PnlClk, &(pvPnlclk[u32clknum]));
            }
        }
    }
    else
    {
        CamOsPrintf("[%s] W/O Camclk \n", __FUNCTION__);
    }
    return 1;
}
u8 Pnl_ClkUnregister(void)
{
    u32 u32clknum;

    for (u32clknum = 0; u32clknum < PnlParentCnt; u32clknum++)
    {
        if (pvPnlclk[u32clknum])
        {
            CamOsPrintf("[%s] %p\n", __FUNCTION__, pvPnlclk[u32clknum]);
            CamClkUnregister(pvPnlclk[u32clknum]);
            pvPnlclk[u32clknum] = NULL;
        }
    }
    CamOsMemRelease(pvPnlclk);

    return 1;
}
#endif

bool DrvPnlOsSetClkOn(void *pClkEn, void *pClkRate, u32 u32ClkRateSize)
{
    bool           bRet = TRUE;
    unsigned long  pulClkRate;
    unsigned long *pulClkRateBuf = (unsigned long *)pClkRate;
    bool *         pbClkEnBuf    = (bool *)pClkEn;

#ifdef CONFIG_CAM_CLK
    u32                  u32clknum = 0;
    CAMCLK_Set_Attribute stSetCfg;
    CAMCLK_Get_Attribute stGetCfg;

    if (g_gpstPnlDeviceNode && pClkRate && u32ClkRateSize == HAL_PNL_CLK_NUM)
    {
        Pnl_ClkRegister(g_gpstPnlDeviceNode);
        for (u32clknum = 0; u32clknum < PnlParentCnt; u32clknum++)
        {
            if (pvPnlclk[u32clknum])
            {
                pulClkRate = pulClkRateBuf[u32clknum];
                if (HAL_PNL_CLK_GET_MUX_ATTR(u32clknum) == 1)
                {
                    CamClkAttrGet(pvPnlclk[u32clknum], &stGetCfg);
                    CAMCLK_SETPARENT(stSetCfg, stGetCfg.u32Parent[pulClkRate]);
                    CamClkAttrSet(pvPnlclk[u32clknum], &stSetCfg);
                }
                else
                {
                    CAMCLK_SETRATE_ROUNDUP(stSetCfg, pulClkRate);
                    CamClkAttrSet(pvPnlclk[u32clknum], &stSetCfg);
                }

                if (!pbClkEnBuf[u32clknum])
                {
                    CamClkSetOnOff(pvPnlclk[u32clknum], 1);
                }
            }
        }
    }

#else
    u32            u32NumParents, idx;
    struct clk **  ppstPnlClks;
    struct clk_hw *pstHwParent;

    if (g_gpstPnlDeviceNode && pClkRate && u32ClkRateSize == HAL_PNL_CLK_NUM)
    {
        u32NumParents = CamOfClkGetParentCount(g_gpstPnlDeviceNode);
        for (idx = 0; idx < u32ClkRateSize; idx++)
        {
            PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d, CLK_%d = %ld\n", __FUNCTION__, __LINE__, idx, pulClkRateBuf[idx]);
        }

        PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d u32NumParents:%d \n", __FUNCTION__, __LINE__, u32NumParents);
        if (u32NumParents == HAL_PNL_CLK_NUM)
        {
            ppstPnlClks = CamOsMemAlloc((sizeof(struct clk *) * u32NumParents));

            if (ppstPnlClks == NULL)
            {
                PNL_ERR("%s %d Alloc ppstPnlClks is NULL\n", __FUNCTION__, __LINE__);
                return 0;
            }

            for (idx = 0; idx < u32NumParents; idx++)
            {
                ppstPnlClks[idx] = of_clk_get(g_gpstPnlDeviceNode, idx);
                if (IS_ERR(ppstPnlClks[idx]))
                {
                    PNL_ERR("%s %d, Fail to get [Pnl] %s\n", __FUNCTION__, __LINE__,
                            CamOfClkGetParentName(g_gpstPnlDeviceNode, idx));
                    CamOsMemRelease(ppstPnlClks);
                    return 0;
                }

                if (HAL_PNL_CLK_GET_MUX_ATTR(idx) == 1)
                {
                    pulClkRate  = pulClkRateBuf[idx];
                    pstHwParent = CamClkGetParentByIndex(__CamClkGetHw(ppstPnlClks[idx]), pulClkRate);
                    CamClkSetParent(ppstPnlClks[idx], pstHwParent->clk);
                }
                else
                {
                    pulClkRate = CamClkRoundRate(ppstPnlClks[idx], pulClkRateBuf[idx]);
                    CamClkSetRate(ppstPnlClks[idx], pulClkRate + 1000000);
                }

                if (!pbClkEnBuf[idx])
                {
                    CamClkPrepareEnable(ppstPnlClks[idx]);
                }

                PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d, [Pnl] u32NumParents:%d-%d %20s En:%d, Rate:%ld\n", __FUNCTION__,
                        __LINE__, u32NumParents, idx + 1, CamOfClkGetParentName(g_gpstPnlDeviceNode, idx),
                        pbClkEnBuf[idx], pulClkRate);

                clk_put(ppstPnlClks[idx]);
            }

            CamOsMemRelease(ppstPnlClks);
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, u32NumParents %d != %d\n", __FUNCTION__, __LINE__, u32NumParents, HAL_PNL_CLK_NUM);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Param Null, DeviceNode:%x, ClkRate:%x, ClkSize:%ld\n", __FUNCTION__, __LINE__,
                g_gpstPnlDeviceNode, pClkRate, u32ClkRateSize);
    }

#endif
    return bRet;
}

bool DrvPnlOsSetClkOff(void *pClkEn, void *pClkRate, u32 u32ClkRateSize)
{
    bool bRet = TRUE;
#ifdef CONFIG_CAM_CLK
    u32 u32clknum = 0;

    for (u32clknum = 0; u32clknum < PnlParentCnt; u32clknum++)
    {
        if (pvPnlclk[u32clknum])
        {
            CamClkSetOnOff(pvPnlclk[u32clknum], 0);
        }
    }
    Pnl_ClkUnregister();
#else
    u32            u32NumParents, idx;
    struct clk_hw *pstHwParent;
    struct clk **  ppstPnlClks;
    unsigned long  pulClkRate;
    unsigned long *pulClkRateBuf = (unsigned long *)pClkRate;
    bool *         pbClkEnBuf    = (bool *)pClkEn;

    if (g_gpstPnlDeviceNode)
    {
        u32NumParents = CamOfClkGetParentCount(g_gpstPnlDeviceNode);

        PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d u32NumParents:%d\n", __FUNCTION__, __LINE__, u32NumParents);
        if (u32NumParents == HAL_PNL_CLK_NUM)
        {
            ppstPnlClks = CamOsMemAlloc((sizeof(struct clk *) * u32NumParents));

            if (ppstPnlClks == NULL)
            {
                PNL_ERR("%s %d Alloc ppstPnlClks is NULL\n", __FUNCTION__, __LINE__);
                return 0;
            }

            for (idx = 0; idx < u32NumParents; idx++)
            {
                ppstPnlClks[idx] = of_clk_get(g_gpstPnlDeviceNode, idx);
                if (IS_ERR(ppstPnlClks[idx]))
                {
                    PNL_ERR("%s %d, Fail to get [Pnl] %s\n", __FUNCTION__, __LINE__,
                            CamOfClkGetParentName(g_gpstPnlDeviceNode, idx));
                    CamOsMemRelease(ppstPnlClks);
                    bRet = 0;
                    break;
                }

                if (HAL_PNL_CLK_GET_MUX_ATTR(idx) == 1)
                {
                    pulClkRate  = pulClkRateBuf[idx];
                    pstHwParent = CamClkGetParentByIndex(__CamClkGetHw(ppstPnlClks[idx]), pulClkRate);
                    CamClkSetParent(ppstPnlClks[idx], pstHwParent->clk);
                }
                else
                {
                    pulClkRate = CamClkRoundRate(ppstPnlClks[idx], pulClkRateBuf[idx]);
                    CamClkSetRate(ppstPnlClks[idx], pulClkRate + 1000000);
                }

                if (pbClkEnBuf[idx])
                {
                    CamClkDisableUnprepare(ppstPnlClks[idx]);
                }

                PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d, [Pnl] %d-%d, %20s En:%d  Rate:%ld\n", __FUNCTION__, __LINE__,
                        u32NumParents, idx + 1, CamOfClkGetParentName(g_gpstPnlDeviceNode, idx), pbClkEnBuf[idx],
                        pulClkRate);

                clk_put(ppstPnlClks[idx]);
            }
            CamOsMemRelease(ppstPnlClks);
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
    }

#endif
    return bRet;
}

u32 DrvPnlOsGetFileSize(DrvPnlOsFileConfig_t *pFileCfg)
{
    u32 filesize;

    filesize = CamFsSeek(pFileCfg->pFile, 0, SEEK_END);
    CamFsSeek(pFileCfg->pFile, 0, SEEK_SET);
    return filesize;
}

CamFsRet_e DrvPnlOsOpenFile(u8 *path, s32 flag, s32 mode, DrvPnlOsFileConfig_t *pstFileCfg)
{
    CamFsRet_e bRet = 1;

    pstFileCfg->pFile = NULL;
    bRet              = CamFsOpen(&pstFileCfg->pFile, path, flag, mode);
    return bRet;
}

s32 DrvPnlOsWriteFile(DrvPnlOsFileConfig_t *pFileCfg, u8 *buf, s32 writelen)
{
    s32 ret;

    ret = CamFsWrite(pFileCfg->pFile, buf, writelen);
    return ret;
}

s32 DrvPnlOsReadFile(DrvPnlOsFileConfig_t *pFileCfg, u8 *buf, s32 readlen)
{
    s32 ret;

    ret = CamFsRead(pFileCfg->pFile, buf, readlen);
    return ret;
}

s32 DrvPnlOsCloseFile(DrvPnlOsFileConfig_t *pFileCfg)
{
    CamFsClose(pFileCfg->pFile);
    return 0;
}

EXPORT_SYMBOL(MhalPnlCreateInstance);
EXPORT_SYMBOL(MhalPnlCreateInstanceEx);
EXPORT_SYMBOL(MhalPnlDestroyInstance);
EXPORT_SYMBOL(MhalPnlSetParamConfig);
EXPORT_SYMBOL(MhalPnlGetParamConfig);
EXPORT_SYMBOL(MhalPnlSetMipiDsiConfig);
EXPORT_SYMBOL(MhalPnlGetMipiDsiConfig);
EXPORT_SYMBOL(MhalPnlSetSscConfig);
EXPORT_SYMBOL(MhalPnlSetTimingConfig);
EXPORT_SYMBOL(MhalPnlGetTimingConfig);
EXPORT_SYMBOL(MhalPnlSetPowerConfig);
EXPORT_SYMBOL(MhalPnlGetPowerConfig);
EXPORT_SYMBOL(MhalPnlSetBackLightOnOffConfig);
EXPORT_SYMBOL(MhalPnlGetBackLightOnOffConfig);
EXPORT_SYMBOL(MhalPnlSetBackLightLevelConfig);
EXPORT_SYMBOL(MhalPnlGetBackLightLevelConfig);
EXPORT_SYMBOL(MhalPnlSetDrvCurrentConfig);
EXPORT_SYMBOL(MhalPnlSetTestPatternConfig);
EXPORT_SYMBOL(MhalPnlSetDebugLevel);
EXPORT_SYMBOL(MhalPnlSetUnifiedParamConfig);
EXPORT_SYMBOL(MhalPnlGetUnifiedParamConfig);
EXPORT_SYMBOL(DrvPnlOsMemRelease);
EXPORT_SYMBOL(DrvPnlOsMemAlloc);
EXPORT_SYMBOL(DrvPnlOsGetFileSize);
EXPORT_SYMBOL(DrvPnlOsOpenFile);
EXPORT_SYMBOL(DrvPnlOsWriteFile);
EXPORT_SYMBOL(DrvPnlOsReadFile);
EXPORT_SYMBOL(DrvPnlOsCloseFile);
