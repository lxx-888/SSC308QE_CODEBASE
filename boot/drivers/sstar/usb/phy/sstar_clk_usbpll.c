/*
 * sstar_clk_usbpll.c - Sigmastar
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
#include <dm/device_compat.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>

struct sstar_clk_usbpll_priv
{
    void __iomem *regs;
};

static int sstar_clk_usbpll_enable(struct clk *clk)
{
    struct sstar_clk_usbpll_priv *priv = dev_get_priv(clk->dev);
    void __iomem *                base = priv->regs;

    dev_dbg(clk->dev, "%s start.\r\n", __func__);

    if (ofnode_device_is_compatible(dev_ofnode(clk->dev), "sstar,generic-usbpll"))
    {
#if 0
        if (INREG16(usbpll->base + (0xe << 2)) & BIT(0))
        {
            return 0;
        }
#endif
        clrbits_16(base + (0x0 << 2), BIT(1));
        clrbits_16(base + (0x0 << 2), BIT(4));
        clrbits_16(base + (0x0 << 2), BIT(5));
        setbits_16(base + (0xe << 2), BIT(0));
        dev_dbg(clk->dev, "%s sstar,generic-usbpll\r\n", __func__);
    }
    else
    {
        // enable UPLL; wait around 100us for PLL to be stable

        /*  setting scalar div for 240 MHZ
            reg[14:6] = 4'hb
            scalar_div[6:2] = 2, scalar_div[1:0] = 4, scalar_div = 2*4 = 8
        */
        writew(0x2c0, base + (0x22 << 2));

        // enable UPLL d2s & vco_en
        writeb(0xbc, base + (0x27 << 2));

        // enable UPLL div clock
        // writew(base + (0x20 << 2), 0x8f00);
        // writew(base + (0x21 << 2), 0x022b);

        // UPLL power on
        writew(0x00c0, base + (0x00 << 2));
        writeb(0x11, base + (0x07 << 2));
        mdelay(1);
    }
    dev_dbg(clk->dev, "%s end.\r\n", __func__);
    return 0;
}

static int sstar_clk_usbpll_disable(struct clk *clk)
{
    return 0;
}

static struct clk_ops sstar_clk_usbpll_ops = {
    .disable = sstar_clk_usbpll_disable,
    .enable  = sstar_clk_usbpll_enable,
};

static const struct udevice_id sstar_clk_usbpll_ids[] = {
    {.compatible = "sstar,infinity7-usbpll"}, {.compatible = "sstar,generic-usbpll"}, {}, {/* sentinel */}};

static int sstar_clk_usbpll_probe(struct udevice *dev)
{
    struct sstar_clk_usbpll_priv *priv = dev_get_priv(dev);
    dev_dbg(dev, "%s start.\r\n", __func__);
    priv->regs = dev_remap_addr(dev);
    if (!priv->regs)
        return -EINVAL;
    dev_dbg(dev, "%s end.\r\n", __func__);
    return 0;
}

U_BOOT_DRIVER(sstar_clk_usbpll) = {
    .name      = "sstar-clk-usbpll",
    .id        = UCLASS_CLK,
    .of_match  = sstar_clk_usbpll_ids,
    .ops       = &sstar_clk_usbpll_ops,
    .probe     = sstar_clk_usbpll_probe,
    .priv_auto = sizeof(struct sstar_clk_usbpll_priv),
};
