/*
 * drv_composite_clk.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/cpu.h>
#ifdef CONFIG_PM_SLEEP
#include <linux/syscore_ops.h>
#endif
#include "ms_types.h"
#include "ms_platform.h"
#include "drv_clk.h"

#ifdef CONFIG_PM_SLEEP
static u8 _is_syscore_register = 0;
LIST_HEAD(sstar_clk_str_list);
#endif

#if IS_ENABLED(CONFIG_PPC)
static inline u32 clk_readl(u32 __iomem *reg)
{
    return ioread32be(reg);
}

static inline void clk_writel(u32 val, u32 __iomem *reg)
{
    iowrite32be(val, reg);
}
#else /* platform dependent I/O accessors */
static inline u32 clk_readl(u32 __iomem *reg)
{
    return readl(reg);
}

static inline void clk_writel(u32 val, u32 __iomem *reg)
{
    writel(val, reg);
}
#endif

static u8 sstar_clk_mux_get_parent(struct clk_hw *hw)
{
    struct sstar_clk_mux *mux         = to_sstar_clk_mux(hw);
    int                   num_parents = clk_hw_get_num_parents(hw);
    u32                   val;

    val = clk_readl(mux->reg) >> mux->shift;
    val &= mux->mask;

    if (val >= num_parents)
        return -EINVAL;

    if (mux->glitch)
    {
        if (!(clk_readl(mux->reg) & (1 << mux->glitch)))
        {
            val = mux->mask + 1;
        }
    }

    return val;
}

static int sstar_clk_mux_set_parent(struct clk_hw *hw, u8 index)
{
    struct sstar_clk_mux *mux = to_sstar_clk_mux(hw);
    u32                   val;
    unsigned long         flags = 0;

    if (mux->table)
        index = mux->table[index];
    else
    {
        if (mux->flags & CLK_MUX_INDEX_BIT)
            index = (1 << ffs(index));

        if (mux->flags & CLK_MUX_INDEX_ONE)
            index++;
    }

    if (mux->lock)
        spin_lock_irqsave(mux->lock, flags);

    if (mux->flags & CLK_MUX_HIWORD_MASK)
    {
        val = mux->mask << (mux->shift + 16);
    }
    else
    {
        val = clk_readl(mux->reg);
        val &= ~(mux->mask << mux->shift);
    }

    // switch to glitch-free mux(set 0)
    if (mux->glitch)
    {
        val &= ~(1 << mux->glitch);
        clk_writel(val, mux->reg);
    }

    if (index <= mux->mask)
    {
        val |= index << mux->shift;
        clk_writel(val, mux->reg);

        // switch back to original mux(set 1)
        if (mux->glitch)
        {
            val |= 1 << mux->glitch;
            clk_writel(val, mux->reg);
        }
    }
    if (mux->lock)
        spin_unlock_irqrestore(mux->lock, flags);

    return 0;
}

static unsigned long sstar_clk_mux_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    struct sstar_clk_mux *mux = to_sstar_clk_mux(hw);

    if (mux->glitch)
    {
        if (clk_readl(mux->reg) & (1 << mux->glitch))
        {
            pr_debug("\n <%s> parent_rate=%lu, glitch-mux=1\n\n", hw->init->name, parent_rate);
            return parent_rate;
        }
        else
        {
            pr_debug("\n <%s> parent_rate=%lu, glitch-mux=0\n\n", hw->init->name, parent_rate);
            return 12000000;
        }
    }

    pr_debug("\n <%s> parent_rate=%lu, no glitch-mux\n\n", hw->init->name, parent_rate);
    return parent_rate;
}

struct clk_ops sstar_clk_mux_ops = {
    .get_parent     = sstar_clk_mux_get_parent,
    .set_parent     = sstar_clk_mux_set_parent,
    .determine_rate = __clk_mux_determine_rate,
    .recalc_rate    = sstar_clk_mux_recalc_rate,
};

#ifdef CONFIG_PM_SLEEP
static int sstar_clk_mux_suspend(void)
{
    struct sstar_clk_str *clk_str;

    // keep parent index for restoring clocks in resume
    list_for_each_entry(clk_str, &sstar_clk_str_list, list)
    {
        // clock already disabled, not need to restore it in resume
        if (clk_str->gate)
        {
            clk_str->enable = clk_gate_ops.is_enabled(&clk_str->gate->hw);
        }
        if (clk_str->mux)
        {
            clk_str->parent_save = sstar_clk_mux_ops.get_parent(&clk_str->mux->hw);
        }
    }
    // pr_debug("sstar_clk_mux_suspend\n");

    return 0;
}

static void sstar_clk_mux_resume(void)
{
    struct sstar_clk_str *clk_str;

    // restore auto-enable clocks in list
    list_for_each_entry(clk_str, &sstar_clk_str_list, list)
    {
        if (clk_str->gate)
        {
            if (clk_str->enable)
            {
                clk_gate_ops.enable(&clk_str->gate->hw);
            }
            else
            {
                clk_gate_ops.disable(&clk_str->gate->hw);
            }
        }
        if (clk_str->mux)
        {
            sstar_clk_mux_ops.set_parent(&clk_str->mux->hw, clk_str->parent_save);
        }
    }

    // pr_debug("sstar_clk_mux_resume\n");
}

struct syscore_ops sstar_clk_mux_syscore_ops = {
    .suspend = sstar_clk_mux_suspend,
    .resume  = sstar_clk_mux_resume,
};
#endif

static void __init sstar_clk_composite_init(struct device_node *node)
{
    const char *            clk_name = node->name;
    int                     num_parents;
    const char **           parent_names;
    struct sstar_clk_mux *  mux  = NULL;
    struct clk_gate *       gate = NULL;
    struct clk_dfs_divider *div  = NULL;
    void __iomem *          reg, *reg1 = NULL;
    u32 i, mux_shift, mux_width, mux_glitch, div_width, div_shift, bit_idx, auto_enable, ignore, gate_flag, propagate;
    struct clk * clk;
    unsigned int flag = 0;
    u32          od_shift, nonod_shift;
#ifdef CONFIG_PM_SLEEP
    struct sstar_clk_str *clk_str = NULL;
#endif

    num_parents = of_clk_get_parent_count(node);
    if (num_parents < 0)
        num_parents = 0;
    parent_names = kzalloc((sizeof(char *) * num_parents), GFP_KERNEL);
    mux          = kzalloc(sizeof(*mux), GFP_KERNEL);
    gate         = kzalloc(sizeof(*gate), GFP_KERNEL);
    div          = kzalloc(sizeof(*div), GFP_KERNEL);
#ifdef CONFIG_PM_SLEEP
    clk_str = kzalloc(sizeof(*clk_str), GFP_KERNEL);
#endif

    if (!parent_names || !mux || !gate)
    {
        pr_err("<%s> allocate mem fail\n", clk_name);
        goto fail;
    }

    reg = of_iomap(node, 0);
    if (!reg)
    {
        pr_err("<%s> map region fail\n", clk_name);
        goto fail;
    }

    reg1 = of_iomap(node, 1);
    if (!reg) // dfs en
    {
        pr_err("<%s> map region div fail\n", clk_name);
    }

    for (i = 0; i < num_parents; i++)
        parent_names[i] = of_clk_get_parent_name(node, i);

    mux->reg  = reg;
    gate->reg = reg;
    div->reg  = reg1;

    if (!of_property_read_u32(node, "gate-enable", &gate_flag))
    {
        if (gate_flag)
        {
            pr_debug("<%s> gate flag of CLK_GATE_SET_TO_DISABLE is not set\n", clk_name);
            gate->flags = 0;
        }
        else
            gate->flags = CLK_GATE_SET_TO_DISABLE;
    }
    else
        gate->flags = CLK_GATE_SET_TO_DISABLE;

    // flag = CLK_GET_RATE_NOCACHE | CLK_IGNORE_UNUSED; //remove ignore_unused flag when all drivers use clk framework,
    // so some clks will be gated
    if (!of_property_read_u32(node, "ignore", &ignore))
    {
        if (ignore)
        {
            pr_debug("<%s> ignore gate clock\n", clk_name);
            flag = CLK_IGNORE_UNUSED;
        }
    }
    if (!of_property_read_u32(node, "propagate", &propagate))
    {
        if (propagate)
        {
            pr_debug("<%s> propagate rate change to parent\n", clk_name);
            flag |= CLK_SET_RATE_PARENT;
        }
    }
    if (of_property_read_u32(node, "mux-shift", &mux_shift))
    {
        pr_debug("<%s> no mux-shift, treat as gate clock\n", clk_name);
        mux->shift = 0xFF;
    }
    else
        mux->shift = (u8)mux_shift;

    if (of_property_read_u32(node, "mux-width", &mux_width))
    {
        pr_debug("<%s> no mux-width, set to default 2 bits\n", clk_name);
        mux->mask = BIT(2) - 1;
    }
    else
        mux->mask = BIT((u8)mux_width) - 1;

    if (of_property_read_u32(node, "glitch-shift", &mux_glitch))
    {
        mux->glitch = 0;
    }
    else
    {
        mux->glitch = (u8)mux_glitch;
    }

    if (of_property_read_u32(node, "gate-shift", &bit_idx))
    {
        pr_debug("<%s> no gate-shift, can not be gated\n", clk_name);
        gate->bit_idx = 0xFF;
    }
    else
        gate->bit_idx = (u8)bit_idx;

    if (reg1)
    {
        flag |= CLK_SET_RATE_UNGATE;
        of_property_read_u32(node, "dfs-shift", &div_shift);
        div->enable = div_shift;
        div->shift  = div_shift + 1;

        of_property_read_u32(node, "dfs-width", &div_width);
        div->width = div_width;

        div->mask = GENMASK(div->width - 1, 0) << div->shift;

        of_property_read_u32(node, "od-shift", &od_shift);
        div->od_shift = od_shift;
    }

    mux->name = node->name;

    pr_debug("[%s]\nmux->reg=0x%08lX\nmux->shift=%d\nmux->width=%d\nmux->glitch=%d\ngate->bit_idx=%d\n", clk_name,
             (unsigned long)mux->reg, mux->shift, mux_width, mux->glitch, gate->bit_idx);

    pr_debug("clk_name:%s dif: width:%d shift:%d od_shift:%d\n", clk_name, div->width, div->shift, div->od_shift);
    if (mux->shift != 0xFF && gate->bit_idx != 0xFF && reg1 != NULL)
    {
        clk = clk_register_composite(NULL, clk_name, parent_names, num_parents, &mux->hw, &sstar_clk_mux_ops, &div->hw,
                                     &clk_dfs_divider_ops, &gate->hw, &clk_gate_ops, flag);
        memcpy(&mux->dd, div, sizeof(*div));
    }
    else if (mux->shift != 0xFF && gate->bit_idx != 0xFF)
    {
        clk = clk_register_composite(NULL, clk_name, parent_names, num_parents, &mux->hw, &sstar_clk_mux_ops, &mux->hw,
                                     &sstar_clk_mux_ops, &gate->hw, &clk_gate_ops, flag);
        kfree(div);
        div = NULL;
    }
    else if (mux->shift != 0xFF)
    {
        clk = clk_register_composite(NULL, clk_name, parent_names, num_parents, &mux->hw, &sstar_clk_mux_ops, &mux->hw,
                                     &sstar_clk_mux_ops, NULL, NULL, flag);
        kfree(gate);
        kfree(div);
        div  = NULL;
        gate = NULL;
    }
    else if (gate->bit_idx != 0xFF)
    {
        clk = clk_register_composite(NULL, clk_name, parent_names, num_parents, NULL, NULL, NULL, NULL, &gate->hw,
                                     &clk_gate_ops, flag);
        kfree(mux);
        kfree(div);
        div = NULL;
        mux = NULL;
    }
    else
    {
        pr_err("clock <%s> info err\n", clk_name);
        goto fail;
    }

    if (IS_ERR(clk))
    {
        pr_err("%s: register clock <%s> fail\n", __func__, clk_name);
        goto fail;
    }

    of_clk_add_provider(node, of_clk_src_simple_get, clk);
    clk_register_clkdev(clk, clk_name, NULL);

    if (!of_property_read_u32(node, "nonod-shift", &nonod_shift))
    {
        clk_hw_set_rate_range(__clk_get_hw(clk), 0,
                              clk_get_rate(clk_hw_get_parent_by_index(__clk_get_hw(clk), nonod_shift)->clk));
        pr_debug("set %s max rate to %ld\n", clk_name,
                 clk_get_rate(clk_hw_get_parent_by_index(__clk_get_hw(clk), nonod_shift)->clk));
    }

    if (!of_property_read_u32(node, "auto-enable", &auto_enable))
    {
        if (auto_enable)
        {
            clk_prepare_enable(clk);
            pr_debug("clk_prepare_enable <%s>\n", clk_name);
        }
    }

#ifdef CONFIG_PM_SLEEP
    if (!of_property_read_bool(node, "str-ignore"))
    {
        if (mux)
        {
            clk_str->mux = mux;
        }
        if (gate)
        {
            clk_str->gate = gate;
        }
        list_add_tail(&clk_str->list, &sstar_clk_str_list);
    }
    else
    {
        kfree(clk_str);
        clk_str = NULL;
    }
    if (!_is_syscore_register)
    {
        register_syscore_ops(&sstar_clk_mux_syscore_ops);
        _is_syscore_register = 1;
    }
#endif
    kfree(parent_names);
    return;

fail:
    kfree(parent_names);
    kfree(mux);
    kfree(gate);
    kfree(div);
    return;
}

CLK_OF_DECLARE(sstar_clk_composite, "sstar,composite-clock", sstar_clk_composite_init);
