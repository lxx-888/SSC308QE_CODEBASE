/*
 * drv_ive_clk.c- Sigmastar
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
#include <linux/clk.h>
#include <linux/clk-provider.h>

#include "drv_ive_clk.h"

//-------------------------------------------------------------------------------------------------
// File operations
//-------------------------------------------------------------------------------------------------

/*******************************************************************************************************************
 * drv_ive_clock_init
 *   init device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   0: OK, othes: failed
 */
int drv_ive_clock_init(ive_dev_data *dev_data)
{
    int ret      = 0;
    u8  u8ClkNum = 0;

    u8ClkNum = of_clk_get_parent_count(dev_data->pdev->dev.of_node);
    if (u8ClkNum <= 0)
    {
        IVE_MSG(IVE_MSG_ERR, "incorrect clock number (%d)\n", u8ClkNum);
    }
    else
    {
        IVE_MSG(IVE_MSG_DBG, "dev_data->clk_num = %d\n", u8ClkNum);

        if (NULL == (dev_data->pstClk = of_clk_get(dev_data->pdev->dev.of_node, 0)))
        {
            IVE_MSG(IVE_MSG_ERR, "%s, can't get clock\n", __FUNCTION__);
            ret = -1;
        }

        dev_data->u32ClkRate = IVE_DEFAULT_CLK_RATE;
        CamOsAtomicSet(&dev_data->bClkEnable, IVE_CLK_ENABLE_OFF);
    }
    return ret;
}

/*******************************************************************************************************************
 * drv_ive_clock_release
 *   release device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   none
 */
void drv_ive_clock_release(ive_dev_data *dev_data)
{
    if (dev_data->pstClk)
    {
        clk_put(dev_data->pstClk);
    }
}

/*******************************************************************************************************************
 * drv_ive_clock_enable
 *   enable device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   0: OK, othes: failed
 */
int drv_ive_clock_enable(ive_dev_data *dev_data)
{
    int ret = 0;

    if (dev_data->pstClk)
    {
        // only enable once.
        if (IVE_CLK_ENABLE_OFF
            == CamOsAtomicCompareAndSwap(&dev_data->bClkEnable, IVE_CLK_ENABLE_OFF, IVE_CLK_ENABLE_ON))
        {
            u8 u8ParentNum = 0;
            if (0 < (u8ParentNum = clk_hw_get_num_parents(__clk_get_hw(dev_data->pstClk))))
            {
                int         i            = 0;
                struct clk *pstClkParent = NULL;

                for (i = 0; i < u8ParentNum; ++i)
                {
                    pstClkParent = clk_hw_get_parent_by_index(__clk_get_hw(dev_data->pstClk), i)->clk;
                    if (NULL != pstClkParent && dev_data->u32ClkRate == clk_get_rate(pstClkParent))
                    {
                        if (0 != (ret = clk_set_parent(dev_data->pstClk, pstClkParent)))
                        {
                            IVE_MSG(IVE_MSG_ERR, "can't set parent, ret=%d\n", ret);
                            break;
                        }

                        if (0 != (ret = clk_prepare_enable(dev_data->pstClk)))
                        {
                            IVE_MSG(IVE_MSG_ERR, "can't enable clock, ret=%d\n", ret);
                            break;
                        }

                        IVE_MSG(IVE_MSG_DBG, "clk rate=%u", clk_get_rate(dev_data->pstClk));
                        break;
                    }
                }

                if (i == u8ParentNum)
                {
                    IVE_MSG(IVE_MSG_ERR, "err clk_rete(%u).\n", dev_data->u32ClkRate);
                    ret = -1;
                }
            }
            else
            {
                IVE_MSG(IVE_MSG_ERR, "err clk number(%d).\n", u8ParentNum);
                ret = -1;
            }

            // enable clock failed.
            if (0 != ret)
            {
                CamOsAtomicSet(&dev_data->bClkEnable, IVE_CLK_ENABLE_OFF);
            }
        }
    }

    return ret;
}

/*******************************************************************************************************************
 * drv_ive_clock_disable
 *   disable device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   0: OK, othes: failed
 */
void drv_ive_clock_disable(ive_dev_data *dev_data)
{
    if (dev_data->pstClk)
    {
        if (IVE_CLK_ENABLE_ON
            == CamOsAtomicCompareAndSwap(&dev_data->bClkEnable, IVE_CLK_ENABLE_ON, IVE_CLK_ENABLE_OFF))
        {
            struct clk *pstClkParent = clk_hw_get_parent_by_index(__clk_get_hw(dev_data->pstClk), 0)->clk;
            // reset parent before disable_unprepare.
            if (NULL != pstClkParent && 0 != (clk_set_parent(dev_data->pstClk, pstClkParent)))
            {
                IVE_MSG(IVE_MSG_ERR, "can't set default parent\n");
            }
            clk_disable_unprepare(dev_data->pstClk);
        }
    }
}
