/*
 * drv_clock.c- Sigmastar
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
#define DRV_CLOCK_C
#include <cam_os_wrapper.h>

#ifdef CAM_OS_RTK
#include "drv_camclk_Api.h"
#ifdef CONFIG_SYSDESC_SUPPORT
#include <drv_sysdesc.h>
#endif
#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/clk.h>
#include <linux/clk-provider.h>
#endif
#include "drv_clk.h"

void *sstar_clk_get(void *dev, u16 index)
{
#ifdef CAM_OS_RTK
    u16   size            = 0;
    u16   remainder       = 0;
    u16   camclk_name[16] = {0};
    u16   camclk_list[20];
    void *clk_handler = NULL;
    u16   dev_id      = *(int *)dev;

    if (dev_id >= SYSDESC_DEV_MIN && dev_id <= SYSDESC_DEV_MAX)
    {
        MDrv_SysDesc_GetElementCount(dev_id, SYSDESC_PRO_camclk_u16, &size, &remainder);
        if (size == 0)
        {
            return NULL;
        }
        MDrv_SysDesc_Read_MultiTypes(dev_id, SYSDESC_PRO_camclk_u16, (void *)camclk_list, sizeof(u16),
                                     sizeof(u16) * size);
        CamOsSnprintf((char *)camclk_name, sizeof(camclk_name), "Dev%d_%d", dev_id, index);
        if (CamClkRegister((u8 *)camclk_name, camclk_list[index], &clk_handler) != 0)
        {
            return NULL;
        }
    }

    return clk_handler;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *sstar_clk;
    void *      clk_handler = NULL;

    sstar_clk   = of_clk_get((struct device_node *)dev, index);
    clk_handler = (void *)sstar_clk;
    return clk_handler;
#endif
    return NULL;
}
EXPORT_SYMBOL(sstar_clk_get);

int sstar_clk_prepare(void *clk_handler)
{
#ifdef CAM_OS_RTK
    return CAM_OS_OK;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return clk_prepare(clk);
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_prepare);

int sstar_clk_enable(void *clk_handler)
{
#ifdef CAM_OS_RTK
    return CamClkSetOnOff(clk_handler, 1);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return clk_enable(clk);
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_enable);

int sstar_clk_prepare_enable(void *clk_handler)
{
#ifdef CAM_OS_RTK
    u32 ret;
    ret = sstar_clk_prepare(clk_handler);
    ret = sstar_clk_enable(clk_handler);
    return ret;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return clk_prepare_enable(clk);
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_prepare_enable);

int sstar_clk_is_enabled(void *clk_handler)
{
#ifdef CAM_OS_RTK
    u8 enable;
    CamClkGetOnOff(clk_handler, &enable);
    if (enable)
    {
        return 1;
    }
    else
    {
        return 0;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return __clk_is_enabled(clk);
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_is_enabled);

int sstar_clk_set_rate(void *clk_handler, unsigned long rate)
{
#ifdef CAM_OS_RTK
    CAMCLK_Set_Attribute clk_cfg;
    CAMCLK_SETRATE_ROUNDUP(clk_cfg, rate);
    return CamClkAttrSet(clk_handler, &clk_cfg);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return clk_set_rate(clk, rate);
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_set_rate);

u32 sstar_clk_get_rate(void *clk_handler)
{
#ifdef CAM_OS_RTK
    CAMCLK_Handler *pCfg = (CAMCLK_Handler *)clk_handler;
    return CamClkRateGet(pCfg->u32ClkId);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return clk_get_rate(clk);
#endif
    return 0;
}
EXPORT_SYMBOL(sstar_clk_get_rate);

u32 sstar_clk_round_rate(void *clk_handler, unsigned long rate)
{
#ifdef CAM_OS_RTK
    CAMCLK_Handler *     pCfg = (CAMCLK_Handler *)clk_handler;
    CAMCLK_Set_Attribute clk_cfg;
    clk_cfg.eRoundType        = CAMCLK_ROUNDRATE_ROUND;
    clk_cfg.attribute.u32Rate = rate;
    return CamClkRoundRateGet(pCfg, &clk_cfg);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return clk_round_rate(clk, rate);
#endif
    return 0;
}
EXPORT_SYMBOL(sstar_clk_round_rate);

int sstar_clk_set_parent(void *clk_handler, void *clk_handler_parent)
{
#ifdef CAM_OS_RTK
    CAMCLK_Set_Attribute set_cfg;
    CAMCLK_Handler *     pcfg;
    pcfg = (CAMCLK_Handler *)clk_handler_parent;
    CAMCLK_SETPARENT(set_cfg, pcfg->u32ClkId);
    return CamClkAttrSet(clk_handler, &set_cfg);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk        = (struct clk *)clk_handler;
    struct clk *clk_parent = (struct clk *)clk_handler_parent;
    return clk_set_parent(clk, clk_parent);
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_set_parent);

void *sstar_clk_get_parent(void *clk_handler)
{
#ifdef CAM_OS_RTK
    u16                  camclk_name[16] = {0};
    CAMCLK_Get_Attribute get_cfg;
    void *               clk_handler_parent;
    CamClkAttrGet(clk_handler, &get_cfg);
    CamOsSnprintf((char *)camclk_name, sizeof(camclk_name), "Parent%d", get_cfg.u32CurrentParent);
    CamClkRegister((u8 *)camclk_name, get_cfg.u32CurrentParent, &clk_handler_parent);
    return clk_handler_parent;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    struct clk *clk_parent;
    clk_parent = clk_get_parent(clk);
    return (void *)clk_parent;
#endif
    return NULL;
}
EXPORT_SYMBOL(sstar_clk_get_parent);

u32 sstar_clk_get_num_parents(void *clk_handler)
{
#ifdef CAM_OS_RTK
    CAMCLK_Get_Attribute get_cfg;
    CamClkAttrGet(clk_handler, &get_cfg);
    return get_cfg.u32NodeCount;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    return clk_hw_get_num_parents(__clk_get_hw(clk));
#endif
    return 0;
}
EXPORT_SYMBOL(sstar_clk_get_num_parents);

void *sstar_clk_get_parent_by_index(void *clk_handler, u16 index)
{
#ifdef CAM_OS_RTK
    u16                  camclk_name[16] = {0};
    CAMCLK_Get_Attribute get_cfg;
    void *               clk_handler_parent;
    CamClkAttrGet(clk_handler, &get_cfg);
    CamOsSnprintf((char *)camclk_name, sizeof(camclk_name), "Parent%d", get_cfg.u32Parent[index]);
    CamClkRegister((u8 *)camclk_name, get_cfg.u32Parent[index], &clk_handler_parent);
    return clk_handler_parent;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    struct clk *clk_parent;
    void *      clk_handler_parent;
    clk_parent         = clk_hw_get_parent_by_index(__clk_get_hw(clk), index)->clk;
    clk_handler_parent = (void *)clk_parent;
    return clk_handler_parent;
#endif
    return NULL;
}
EXPORT_SYMBOL(sstar_clk_get_parent_by_index);

void sstar_clk_unprepare(void *clk_handler)
{
#ifdef CAM_OS_RTK
    return;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    clk_unprepare(clk);
#endif
}
EXPORT_SYMBOL(sstar_clk_unprepare);

void sstar_clk_disable(void *clk_handler)
{
#ifdef CAM_OS_RTK
    CamClkSetOnOff(clk_handler, 0);
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    clk_disable(clk);
#endif
}
EXPORT_SYMBOL(sstar_clk_disable);

void sstar_clk_disable_unprepare(void *clk_handler)
{
#ifdef CAM_OS_RTK
    sstar_clk_disable(clk_handler);
    sstar_clk_unprepare(clk_handler);
    return;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    clk_disable_unprepare(clk);
#endif
}
EXPORT_SYMBOL(sstar_clk_disable_unprepare);

void sstar_clk_put(void *clk_handler)
{
#ifdef CAM_OS_RTK
#ifndef CONFIG_RPMSG_SUPPORT
    CamClkUnregister(clk_handler);
#endif
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    clk_put(clk);
#endif
}
EXPORT_SYMBOL(sstar_clk_put);

u32 sstar_clk_get_num_in_dev_node(void *dev)
{
    u32 count;
#ifdef CAM_OS_RTK
    u16 size      = 0;
    u16 remainder = 0;
    u16 dev_id    = *(int *)dev;
    if (dev_id >= SYSDESC_DEV_MIN && dev_id <= SYSDESC_DEV_MAX)
    {
        MDrv_SysDesc_GetElementCount(dev_id, SYSDESC_PRO_camclk_u16, &size, &remainder);
    }
    count = size;
    return count;
#elif defined(CAM_OS_LINUX_KERNEL)
    count = of_count_phandle_with_args((struct device_node *)dev, "clocks", "#clock-cells");
    if (count < 0)
    {
        return 0;
    }
    return count;
#endif
    return 0;
}
EXPORT_SYMBOL(sstar_clk_get_num_in_dev_node);

int sstar_clk_get_od_shift(void *clk_handler)
{
#ifdef CAM_OS_RTK
    // TODO
    return CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    u32                     od_shift;
    struct clk *            clk       = (struct clk *)clk_handler;
    struct clk_composite *  composite = to_clk_composite(__clk_get_hw(clk));
    struct clk_dfs_divider *hw        = to_clk_dd(composite->rate_hw);
    od_shift                          = hw->od_shift;
    return od_shift;
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_get_od_shift);

int sstar_clk_get_dfs_config(void *clk_handler, u8 *numerator, u8 *denominator)
{
#ifdef CAM_OS_RTK
    // TODO
    return CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *            clk       = (struct clk *)clk_handler;
    struct clk_composite *  composite = to_clk_composite(__clk_get_hw(clk));
    struct clk_dfs_divider *hw        = to_clk_dd(composite->rate_hw);
    *numerator                        = hw->dfs_num;
    *denominator                      = hw->dfs_den;
    return CAM_OS_OK;
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_get_dfs_config);

int sstar_clk_set_max_rate(void *clk_handler, unsigned long rate)
{
#ifdef CAM_OS_RTK
    // TODO
    return CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *clk = (struct clk *)clk_handler;
    clk_hw_set_rate_range(__clk_get_hw(clk), 0, rate);
    return CAM_OS_OK;
#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_set_max_rate);

int sstar_clk_clr_dfs(void *clk_handler)
{
#ifdef CAM_OS_RTK
    // TODO
    return CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct clk *          clk       = (struct clk *)clk_handler;
    struct clk_composite *composite = to_clk_composite(__clk_get_hw(clk));
    struct sstar_clk_mux *hw        = to_sstar_clk_mux(composite->mux_hw);
    if (hw->dd.reg)
    {
        clk_prepare_enable(clk);
        clk_dfs_clr_config(&hw->dd);
        clk_disable_unprepare(clk);
        return CAM_OS_OK;
    }
    else
    {
        return CAM_OS_FAIL;
    }

#endif
    return CAM_OS_FAIL;
}
EXPORT_SYMBOL(sstar_clk_clr_dfs);
