/*
 * drv_clk.c- Sigmastar
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

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include "sstar_clk.h"

extern const sstar_clk_data* hal_clk_data_list(int* count);

struct sstar_clk_priv
{
    const sstar_clk_data* list;
    int                   count;
};

static inline void drv_clk_fixed_rate(ulong id, const sstar_fixed_rate* fr)
{
    clk_dm(id, clk_register_fixed_rate(NULL, fr->name, fr->rate));
}

static inline void drv_clk_fixed_factor(ulong id, const sstar_fixed_factor* ff)
{
    clk_dm(id, clk_register_fixed_factor(NULL, ff->name, ff->parent, 0, ff->mult, ff->div));
}

static void drv_clk_composite(ulong id, const sstar_composite* cp)
{
    struct clk*      clk  = NULL;
    struct clk_gate* gate = NULL;
    struct clk_mux*  mux  = NULL;

    mux = kzalloc(sizeof(struct clk_mux), GFP_KERNEL);
    if (!mux)
        goto fail;

    mux->reg          = (void __iomem*)cp->reg;
    mux->shift        = cp->mux_shift;
    mux->mask         = clk_div_mask(cp->mux_width);
    mux->flags        = cp->flags;
    mux->parent_names = cp->parents;
    mux->num_parents  = cp->num_parents;

    if (cp->gate_shift != 0xFF)
    {
        gate = kzalloc(sizeof(struct clk_gate), GFP_KERNEL);
        if (!gate)
            goto fail;
        gate->reg     = (void __iomem*)cp->reg;
        gate->bit_idx = cp->gate_shift;
        gate->flags   = CLK_GATE_SET_TO_DISABLE;
    }

    clk = clk_register_composite(NULL, cp->name, cp->parents, cp->num_parents, &mux->clk, &clk_mux_ops, &mux->clk,
                                 &clk_mux_ops, gate ? &gate->clk : NULL, gate ? &clk_gate_ops : NULL, cp->flags);
    if (IS_ERR(clk))
        goto fail;

    clk_dm(id, clk);
    return;

fail:
    kfree(gate);
    kfree(mux);
}

static struct clk* _drv_clk_get_by_id(ulong id)
{
    struct clk* clk;

    if (clk_get_by_id(id, &clk))
        return NULL;

    return clk;
}

static int drv_clk_disable(struct clk* clk)
{
    struct clk* c;

    c = _drv_clk_get_by_id(clk->id);
    if (!c)
        return -1;

    debug("drv_clk_disable: id %lu\r\n", clk->id);
    return clk_disable(c);
}

static int drv_clk_enable(struct clk* clk)
{
    struct clk* c;

    c = _drv_clk_get_by_id(clk->id);
    if (!c)
        return -1;

    debug("drv_clk_enable: id %lu\r\n", clk->id);
    return clk_enable(c);
}

static ulong drv_clk_get_rate(struct clk* clk)
{
    struct clk* c;

    c = _drv_clk_get_by_id(clk->id);
    if (!c)
        return 0;

    debug("drv_clk_get_rate: id %lu, rate %lu\r\n", clk->id, clk_get_rate(c));
    return clk_get_rate(c);
}

static ulong _drv_clk_composite_set_rate(struct sstar_clk_priv* priv, ulong id, ulong rate)
{
    int                   i           = 0;
    ulong                 best_rate   = 0;
    ulong                 parent_rate = 0;
    ulong                 min_diff    = ULONG_MAX;
    ulong                 diff        = ULONG_MAX;
    struct udevice*       dev;
    struct clk*           clkp;
    struct clk*           best_parent = NULL;
    const sstar_clk_data* data;

    data = &priv->list[id];
    if (data->type != SSTAR_CLK_TYPE_COMPOSITE)
        return 0;

    debug("composite parents:\r\n");
    for (i = 0; i < data->clk.composite.num_parents; i++)
    {
        debug("%s ", data->clk.composite.parents[i]);
        if (uclass_get_device_by_name(UCLASS_CLK, data->clk.composite.parents[i], &dev) == 0)
        {
            clkp = dev_get_clk_ptr(dev);
            if (priv->list[clkp->id].type == SSTAR_CLK_TYPE_COMPOSITE)
            {
                /* parent clock is also a composite clock */
                parent_rate = _drv_clk_composite_set_rate(priv, clkp->id, rate);
                if (parent_rate)
                    drv_clk_enable(clkp);
            }
            else
            {
                parent_rate = clk_get_rate(clkp);
            }

            diff = parent_rate >= rate ? parent_rate - rate : rate - parent_rate;
            if (diff < min_diff)
            {
                min_diff    = diff;
                best_rate   = parent_rate;
                best_parent = clkp;
            }
            debug("[id %lu], rate %lu\r\n", clkp->id, parent_rate);

            if (min_diff == 0)
                break;
        }
        else
        {
            debug("\r\n");
        }
    }
    debug("set %lu parent as %lu\r\n", id, best_parent->id);
    if (best_parent)
        clk_set_parent(_drv_clk_get_by_id(id), best_parent);

    return best_rate;
}

static ulong drv_clk_set_rate(struct clk* clk, ulong rate)
{
    struct clk*            c;
    struct sstar_clk_priv* priv = dev_get_priv(clk->dev);
    const sstar_clk_data*  data;
    ulong                  best_rate;

    debug("drv_clk_set_rate: id %lu, rate %lu\r\n", clk->id, rate);
    c = _drv_clk_get_by_id(clk->id);
    if (!c)
    {
        debug("can't find clk id %lu\r\n", clk->id);
        return 0;
    }

    best_rate = clk_get_rate(c);
    if (rate == best_rate)
        return rate;

    if (!priv || (clk->id >= priv->count))
        return best_rate;

    data = &priv->list[clk->id];
    if (data->type == SSTAR_CLK_TYPE_COMPOSITE)
        best_rate = _drv_clk_composite_set_rate(priv, clk->id, rate);

    return best_rate;
}

static struct clk_ops sstar_clk_ops = {
    .set_rate = drv_clk_set_rate,
    .get_rate = drv_clk_get_rate,
    .enable   = drv_clk_enable,
    .disable  = drv_clk_disable,
};

static int sstar_clk_probe(struct udevice* dev)
{
    struct sstar_clk_priv* priv = dev_get_priv(dev);
    const sstar_clk_data*  clks;
    int                    i;

    priv->list = hal_clk_data_list(&priv->count);
    clks       = priv->list;

    debug("sstar_clk_probe, clk count %d\r\n", priv->count);
    for (i = 0; i < priv->count; i++)
    {
        switch (clks[i].type)
        {
            case SSTAR_CLK_TYPE_FIXED_RATE:
                drv_clk_fixed_rate(clks[i].id, &clks[i].clk.fixed_rate);
                break;
            case SSTAR_CLK_TYPE_FIXED_FACTOR:
                drv_clk_fixed_factor(clks[i].id, &clks[i].clk.fixed_factor);
                break;
            case SSTAR_CLK_TYPE_COMPOSITE:
                drv_clk_composite(clks[i].id, &clks[i].clk.composite);
                break;
            default:
                pr_err("unknown clk type %d\r\n", clks[i].type);
                break;
        }
    }

    return 0;
}

static const struct udevice_id sstar_clk_ids[] = {
    {.compatible = "sstar,clk"},
    {},
};

U_BOOT_DRIVER(sstar_clk) = {
    .name      = "clk_sstar",
    .id        = UCLASS_CLK,
    .of_match  = sstar_clk_ids,
    .ops       = &sstar_clk_ops,
    .probe     = sstar_clk_probe,
    .priv_auto = sizeof(struct sstar_clk_priv),
    .flags     = DM_FLAG_PRE_RELOC,
};
