/*
 * drv_rgn_os.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#define __DRV_RGN_OS_C__

#include "drv_rgn_os.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "rgn_debug.h"
#include "hal_rgn_chip.h"
#include "hal_rgn_gop_reg.h"
#include "hal_rgn_st.h"
#include "hal_rgn_if.h"
#include "hal_rgn_gop.h"

#include "cam_clkgen.h"
#include "cam_sysfs.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
bool DrvRgnOsSemInit(DrvRgnOsTsemConfig_t *pCfg)
{
    return (CamOsTsemInit(&pCfg->tSemCfg, 1) != CAM_OS_OK) ? RGN_RET_FAILURE : RGN_RET_SUCCESS;
}

bool DrvRgnOsSemDeInit(DrvRgnOsTsemConfig_t *pCfg)
{
    return (CamOsTsemDeinit(&pCfg->tSemCfg) != CAM_OS_OK) ? RGN_RET_FAILURE : RGN_RET_SUCCESS;
}

bool DrvRgnOsSemUp(DrvRgnOsTsemConfig_t *pCfg)
{
    CamOsTsemUp(&pCfg->tSemCfg);
    return RGN_RET_SUCCESS;
}

bool DrvRgnOsSemDown(DrvRgnOsTsemConfig_t *pCfg)
{
    CamOsTsemDown(&pCfg->tSemCfg);
    return RGN_RET_SUCCESS;
}

void *DrvRgnOsMemAlloc(u32 u32Size)
{
    return CamOsMemAlloc(u32Size);
}

void DrvRgnOsMemRelease(void *pPtr)
{
    CamOsMemRelease(pPtr);
}

bool DrvRgnOsSetGopClkEnable(void *pCtx)
{
#ifndef CONFIG_CAM_CLK
    u32                 u32clknum;
    struct device_node *pstDevNode = NULL;
    u32                 u32NumParents;
    struct clk **       ppstIpClks;

    HalGopGetClkIdx((DrvRgnCtxConfig_t *)pCtx, &u32clknum);

    pstDevNode = of_find_compatible_node(NULL, NULL, "sstar,rgn");
    if (pstDevNode)
    {
        u32NumParents = CamOfClkGetParentCount(pstDevNode);
        if (u32NumParents > u32clknum)
        {
            ppstIpClks = CamOsMemAlloc((sizeof(struct clk *) * u32NumParents));
            if (ppstIpClks == NULL)
            {
                return RGN_RET_FAILURE;
            }

            // enable all clk
            ppstIpClks[u32clknum] = of_clk_get(pstDevNode, u32clknum);
            if (IS_ERR(ppstIpClks[u32clknum]))
            {
                RGN_ERR("Fail to get [RGN] clk!\n");
                CamOsMemRelease(ppstIpClks);
                return RGN_RET_FAILURE;
            }
            RGN_DBG(RGN_DBG_LEVEL_CLK, "%s %d, clk on u32NumParents:%d! %s\n", __FUNCTION__, __LINE__, u32NumParents,
                    CamOfClkGetParentName(pstDevNode, u32clknum));

            CamClkPrepareEnable(ppstIpClks[u32clknum]);
            clk_put(ppstIpClks[u32clknum]);
            CamOsMemRelease(ppstIpClks);
        }
        else
        {
            RGN_DBG(RGN_DBG_LEVEL_CLK, "%s %d, u32NumParents(%d) > u32ClkNum(%d)\n", __FUNCTION__, __LINE__,
                    u32NumParents, u32clknum);
        }
    }
    else
    {
        RGN_ERR("%s %d, dev node is null\n", __FUNCTION__, __LINE__);
    }
    return RGN_RET_SUCCESS;
#else
    return DrvGopClkEnable(pCtx);
#endif
}

bool DrvRgnOsSetGopClkDisable(void *pCtx, bool bEn)
{
#ifndef CONFIG_CAM_CLK
    u32                 u32clknum;
    struct device_node *pstDevNode = NULL;
    u32                 u32NumParents;
    struct clk **       ppstIpClks;

    if (!bEn)
    {
        return RGN_RET_FAILURE;
    }

    HalGopGetClkIdx((DrvRgnCtxConfig_t *)pCtx, &u32clknum);

    pstDevNode = of_find_compatible_node(NULL, NULL, "sstar,rgn");
    if (pstDevNode)
    {
        u32NumParents = CamOfClkGetParentCount(pstDevNode);
        // SCL_DBGERR( "[SCL] u32NumParents:%d! %s\n" ,u32NumParents
        // ,CamOfClkGetParentName(pstDevNode,u32clknumu32clknum));
        if (u32NumParents > u32clknum)
        {
            ppstIpClks = CamOsMemAlloc((sizeof(struct clk *) * u32NumParents));
            if (ppstIpClks == NULL)
            {
                RGN_ERR("%s %d, kzalloc failed!\n", __FUNCTION__, __LINE__);
                return RGN_RET_FAILURE;
            }

            // enable all clk
            ppstIpClks[u32clknum] = of_clk_get(pstDevNode, u32clknum);
            if (IS_ERR(ppstIpClks[u32clknum]))
            {
                RGN_ERR("Fail to get [RGN] clk!\n");
                CamOsMemRelease(ppstIpClks);
                return RGN_RET_FAILURE;
            }
            CamClkDisableUnprepare(ppstIpClks[u32clknum]);
            clk_put(ppstIpClks[u32clknum]);
            CamOsMemRelease(ppstIpClks);
        }
        else
        {
            RGN_DBG(RGN_DBG_LEVEL_CLK, "%s %d, u32NumParents(%d) > u32ClkNum(%d)\n", __FUNCTION__, __LINE__,
                    u32NumParents, u32clknum);
        }
    }
    else
    {
        RGN_ERR("%s %d, dev node is null\n", __FUNCTION__, __LINE__);
    }
    return bEn == 1 ? RGN_RET_SUCCESS : RGN_RET_FAILURE;
#else
    return DrvGopClkDisable(pCtx, bEn);
#endif
}

u32 DrvRgnOsGetChipRevision(void)
{
    return CamOsChipRevision();
}
