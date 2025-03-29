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
#include "drv_sysdesc.h"
#if defined(CONFIG_CAM_CLK)
#include "drv_camclk_Api.h"
#endif

#include "drv_ive_clk.h"

//-------------------------------------------------------------------------------------------------
// MACRO only for this file.
//-------------------------------------------------------------------------------------------------
#if !defined(CONFIG_CAM_CLK)
#define DRV_IVE_CLK_BANK_BASE  (0x1F2071A8) // 0x1F000000 + 0x1038(bank num) * 0x200 + 0x6a(offset) * 0x4
#define DRV_IVE_CLK_SWITCH_BIT (0x1)

// 0x6a bit[12:8] for IVE
#define DRV_IVE_CLK_MUX_MASK  (0x1F)
#define DRV_IVE_CLK_MUX_SHIFT (8)

#define REG_SET(reg, val) ((*(volatile unsigned short *)IO_ADDRESS(reg)) |= val)
#define REG_CLR(reg, val) ((*(volatile unsigned short *)IO_ADDRESS(reg)) &= (~val))
#endif

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
    int ret = 0;
#if defined(CONFIG_CAM_CLK)
    u16 u16IveClkId = 0;

    if (0 != (ret = MDrv_SysDesc_Read_U16(SYSDESC_DEV_ive0, SYSDESC_PRO_camclk_u16, &u16IveClkId)))
    {
        IVE_MSG(IVE_MSG_ERR, "get camclk. err:%X\n", ret);
    }
    else
    {
        u8 u8ClkName[8] = "CLK_IVE";
        IVE_MSG(IVE_MSG_DBG, "get camclk pass. err:%X, camclk:%X\n", ret, u16IveClkId);

        CamClkRegister(u8ClkName, u16IveClkId, &dev_data->pvclk);
        if (NULL == dev_data->pvclk)
        {
            IVE_MSG(IVE_MSG_ERR, "[%s] Fail to get clk!\n", __func__);
            ret = -1;
        }

        dev_data->u32ClkRate = IVE_DEFAULT_CLK_RATE;
    }
#else
    REG_CLR(DRV_IVE_CLK_BANK_BASE, DRV_IVE_CLK_MUX_MASK << DRV_IVE_CLK_MUX_SHIFT);
    REG_SET(DRV_IVE_CLK_BANK_BASE, DRV_IVE_CLK_SWITCH_BIT << DRV_IVE_CLK_MUX_SHIFT);
#endif
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
#if defined(CONFIG_CAM_CLK)
    if (dev_data->pvclk)
    {
        CamClkUnregister(dev_data->pvclk);
        dev_data->pvclk = NULL;
    }
#else
    REG_CLR(DRV_IVE_CLK_BANK_BASE, DRV_IVE_CLK_MUX_MASK << DRV_IVE_CLK_MUX_SHIFT);
    REG_SET(DRV_IVE_CLK_BANK_BASE, DRV_IVE_CLK_SWITCH_BIT << DRV_IVE_CLK_MUX_SHIFT);
#endif
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
#if defined(CONFIG_CAM_CLK)
    if (dev_data->pvclk)
    {
        // only enable once.
        if (IVE_CLK_ENABLE_OFF
            == CamOsAtomicCompareAndSwap(&dev_data->bClkEnable, IVE_CLK_ENABLE_OFF, IVE_CLK_ENABLE_ON))
        {
            u8                   u8ParentNum = 0;
            CAMCLK_Set_Attribute stSetCfg    = {0};
            CAMCLK_Get_Attribute stGetCfg    = {0};

            CamClkAttrGet(dev_data->pvclk, &stGetCfg);
            if (0 < (u8ParentNum = stGetCfg.u32NodeCount))
            {
                int i = 0;
                for (i = 0; i < u8ParentNum; ++i)
                {
                    if (dev_data->u32ClkRate == CamClkRateGet(stGetCfg.u32Parent[i]))
                    {
                        CAMCLK_SETPARENT(stSetCfg, stGetCfg.u32Parent[i]);
                        if (CAMCLK_RET_OK != (ret = CamClkAttrSet(dev_data->pvclk, &stSetCfg)))
                        {
                            IVE_MSG(IVE_MSG_ERR, "can't set parent, ret=%d\n", ret);
                            break;
                        }

                        if (CAMCLK_RET_OK != (ret = CamClkSetOnOff(dev_data->pvclk, IVE_CLK_ENABLE_ON)))
                        {
                            IVE_MSG(IVE_MSG_ERR, "can't enable clock, ret=%d\n", ret);
                            break;
                        }

                        CamClkAttrGet(dev_data->pvclk, &stGetCfg);
                        IVE_MSG(IVE_MSG_DBG, "set clk rate=%lu\n", stGetCfg.u32Rate);
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
#else
    REG_CLR(DRV_IVE_CLK_BANK_BASE, DRV_IVE_CLK_SWITCH_BIT << DRV_IVE_CLK_MUX_SHIFT);
#endif
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
#if defined(CONFIG_CAM_CLK)
    if (dev_data->pvclk)
    {
        if (IVE_CLK_ENABLE_ON
            == CamOsAtomicCompareAndSwap(&dev_data->bClkEnable, IVE_CLK_ENABLE_ON, IVE_CLK_ENABLE_OFF))
        {
            CAMCLK_Set_Attribute stSetCfg = {0};
            CAMCLK_Get_Attribute stGetCfg = {0};

            // reset parent before disable_unprepare.
            CamClkAttrGet(dev_data->pvclk, &stGetCfg);
            CAMCLK_SETPARENT(stSetCfg, stGetCfg.u32Parent[0]);
            if (CAMCLK_RET_OK != CamClkAttrSet(dev_data->pvclk, &stSetCfg))
            {
                IVE_MSG(IVE_MSG_ERR, "can't set default parent\n");
            }
            CamClkSetOnOff(dev_data->pvclk, IVE_CLK_ENABLE_OFF);
        }
    }
#else
    REG_SET(DRV_IVE_CLK_BANK_BASE, DRV_IVE_CLK_SWITCH_BIT << DRV_IVE_CLK_MUX_SHIFT);
#endif
}
