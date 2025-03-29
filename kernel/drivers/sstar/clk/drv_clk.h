/*
 * drv_clk.h- Sigmastar
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
#ifndef _DRV_CLK_H_
#define _DRV_CLK_H_

#include <linux/of.h>
#include <linux/of_clk.h>

struct clk_dfs_divider
{
    struct clk_hw hw;
    void __iomem *reg;
    u8            enable;
    u8            shift;
    u8            width;
    u32           mask;
    u8            flags;
    u8            od_shift;
    u8            dfs_num;
    u8            dfs_den;
    void (*approximation)(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate, unsigned long *m,
                          unsigned long *n);
    spinlock_t *lock;
};

struct sstar_clk_mux
{
    struct clk_hw          hw;
    void __iomem *         reg;
    u32 *                  table;
    u32                    mask;
    u8                     shift;
    u8                     flags;
    u8                     glitch; // this is specific usage for MSTAR
    spinlock_t *           lock;
    struct clk_dfs_divider dd;
    const char *           name;
};

#ifdef CONFIG_PM_SLEEP
struct sstar_clk_str
{
    u8                    enable;
    u8                    parent_save;
    struct sstar_clk_mux *mux;
    struct clk_gate *     gate;
    struct list_head      list;
};
#endif

#define to_sstar_clk_mux(_hw) container_of(_hw, struct sstar_clk_mux, hw)
#define to_clk_dd(_hw)        container_of(_hw, struct clk_dfs_divider, hw)

#define CLK_DFS_DIVIDER_ZERO_BASED BIT(0)
#define CLK_DFS_DIVIDER_BIG_ENDIAN BIT(1)

extern const struct clk_ops clk_dfs_divider_ops;

int clk_dfs_clr_config(struct clk_dfs_divider *dd);

#endif
