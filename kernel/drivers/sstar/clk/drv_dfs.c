/*
 * drv_dfs.c- Sigmastar
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

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/rational.h>

#include "drv_clk.h"

static inline u32 clk_dfs_readl(struct clk_dfs_divider *dd)
{
    if (dd->flags & CLK_DFS_DIVIDER_BIG_ENDIAN)
        return ioread32be(dd->reg);

    return readl(dd->reg);
}

static inline void clk_dfs_writel(struct clk_dfs_divider *dd, u32 val)
{
    if (dd->flags & CLK_DFS_DIVIDER_BIG_ENDIAN)
        iowrite32be(val, dd->reg);
    else
        writel(val, dd->reg);
}

int clk_dfs_clr_config(struct clk_dfs_divider *dd)
{
    unsigned long flags = 0;
    u32           val;

    if (dd->lock)
        spin_lock_irqsave(dd->lock, flags);
    else
        __acquire(dd->lock);

    val = clk_dfs_readl(dd);
    val &= ~(dd->mask | ((1 << dd->enable)));
    clk_dfs_writel(dd, val);

    if (dd->lock)
        spin_unlock_irqrestore(dd->lock, flags);
    else
        __release(dd->lock);

    return 0;
}

static unsigned long clk_dfs_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    struct clk_dfs_divider *dd    = to_clk_dd(hw);
    unsigned long           flags = 0;
    unsigned long           m, n;
    u32                     val;
    u64                     ret;

    if (dd->lock)
        spin_lock_irqsave(dd->lock, flags);
    else
        __acquire(dd->lock);

    val = clk_dfs_readl(dd);

    if (dd->lock)
        spin_unlock_irqrestore(dd->lock, flags);
    else
        __release(dd->lock);

    m = ((val & dd->mask) >> dd->shift) + 1;
    n = 1 << dd->width;

    if (!n || !m || !(val & (1 << dd->enable)))
        return parent_rate;

    ret = (u64)parent_rate * m;
    do_div(ret, n);
    pr_debug("%s, dfs, m:%lu, n:%lu, ret:%llu\n", __FUNCTION__, m, n, ret);

    return ret;
}

static void clk_dfs_general_approximation(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate,
                                          unsigned long *m, unsigned long *n)
{
    struct clk_dfs_divider *dd = to_clk_dd(hw);
    u64                     scale;
    unsigned long           tmp_m, tmp_n;

    *n = 1 << dd->width;
    if (rate >= *parent_rate)
    {
        *m = 1 << dd->width;
        return;
    }
    rational_best_approximation(*parent_rate, rate, GENMASK(dd->width - 0, 0), GENMASK(dd->width - 1, 0), &tmp_m,
                                &tmp_n);
    scale = (*n) * tmp_n;
    *m    = DIV_ROUND_UP(scale, tmp_m);
    pr_debug("%s %d r:%lu, p:%lu, tm:%lu, tn:%lu, m:%lu, n:%lu\n", __func__, __LINE__, rate, *parent_rate, tmp_m, tmp_n,
             *m, *n);
}

static long clk_dfs_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
    unsigned long m, n;
    u64           ret;

    if (!rate || (!clk_hw_can_set_rate_parent(hw) && rate >= *parent_rate))
        return *parent_rate;

    clk_dfs_general_approximation(hw, rate, parent_rate, &m, &n);

    ret = (u64)*parent_rate * m;
    do_div(ret, n);
    pr_debug("%s %d , dfs, m:%lu, n:%lu, ret:%llu\n", __func__, __LINE__, m, n, ret);

    return ret;
}

static int clk_dfs_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    struct clk_dfs_divider *dd    = to_clk_dd(hw);
    unsigned long           flags = 0;
    unsigned long           m, n;
    u32                     val;

    clk_dfs_general_approximation(hw, rate, &parent_rate, &m, &n);

    // dfs is zero based, so it start with zero
    m--;
    n--;

    if (dd->lock)
        spin_lock_irqsave(dd->lock, flags);
    else
        __acquire(dd->lock);

    val = clk_dfs_readl(dd);
    val &= ~dd->mask;
    val |= (1 << dd->enable);
    val |= (m << dd->shift);
    pr_debug("%s %d r:%lu, p:%lu, m:%lu, n:%lu, val:%d\n", __func__, __LINE__, rate, parent_rate, m, n, val);
    if (m == n)
    {
        clk_dfs_writel(dd, clk_dfs_readl(dd) & ~(dd->mask | (1 << dd->enable)));
    }
    else
    {
        clk_dfs_writel(dd, val);
    }

    if (dd->lock)
        spin_unlock_irqrestore(dd->lock, flags);
    else
        __release(dd->lock);

    dd->dfs_num = m + 1;
    dd->dfs_den = n + 1;
    return 0;
}

const struct clk_ops clk_dfs_divider_ops = {
    .recalc_rate = clk_dfs_recalc_rate,
    .round_rate  = clk_dfs_round_rate,
    .set_rate    = clk_dfs_set_rate,
};
EXPORT_SYMBOL_GPL(clk_dfs_divider_ops);
