/*
 * reset-usb.c - Sigmastar
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

#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <dt-bindings/reset/ti-syscon.h>
#include <reset-uclass.h>
#include <linux/bitops.h>

struct sstar_usb_signal
{
    unsigned short offset, bit;
};

struct sstar_usb_reset_priv
{
    void __iomem *           base;
    struct sstar_usb_signal *signals;
};

static struct sstar_usb_signal sstar_usb_signals[] = {
    [0] = {0x30, 8},
};

static int sstar_usb_reset_deassert(struct reset_ctl *rst)
{
    struct sstar_usb_reset_priv *priv = dev_get_priv(rst->dev);
    u32                          val;
    struct sstar_usb_signal *    sig;

    if (rst->id >= ARRAY_SIZE(sstar_usb_signals))
        return -EINVAL;

    sig = &priv->signals[rst->id];
    val = readw(priv->base + (sig->offset << 2));
    val &= ~BIT(sig->bit);
    writew(val, priv->base + (sig->offset << 2));
    debug("sstar usb deassert base:%p offset:0x%x BIT(%d)\n", priv->base, sig->offset, sig->bit);
    return 0;
}

static int sstar_usb_reset_assert(struct reset_ctl *rst)
{
    struct sstar_usb_reset_priv *priv = dev_get_priv(rst->dev);
    u32                          val;
    struct sstar_usb_signal *    sig;

    if (rst->id >= ARRAY_SIZE(sstar_usb_signals))
        return -EINVAL;

    sig = &priv->signals[rst->id];
    val = readw(priv->base + (sig->offset << 2));
    val |= BIT(sig->bit);
    writew(val, priv->base + (sig->offset << 2));
    debug("sstar usb assert base:%p offset:0x%x BIT(%d)\n", priv->base, sig->offset, sig->bit);
    return 0;
}

static int sstar_usb_reset_free(struct reset_ctl *rst)
{
    return 0;
}

static int sstar_usb_reset_request(struct reset_ctl *rst)
{
    return 0;
}

static const struct reset_ops sstar_usb_reset_reset_ops = {
    .request      = sstar_usb_reset_request,
    .rfree        = sstar_usb_reset_free,
    .rst_assert   = sstar_usb_reset_assert,
    .rst_deassert = sstar_usb_reset_deassert,
};

static const struct udevice_id sstar_usb_reset_ids[] = {{.compatible = "sstar,usb-reset"}, {}};

static int sstar_usb_reset_probe(struct udevice *dev)
{
    struct sstar_usb_reset_priv *priv = dev_get_priv(dev);

    priv->base = dev_remap_addr(dev);
    if (!priv->base)
        return -ENOMEM;

    priv->signals = sstar_usb_signals;
    return 0;
}

U_BOOT_DRIVER(sstar_usb_reset) = {
    .name      = "sstar_usb_reset",
    .id        = UCLASS_RESET,
    .of_match  = sstar_usb_reset_ids,
    .ops       = &sstar_usb_reset_reset_ops,
    .probe     = sstar_usb_reset_probe,
    .priv_auto = sizeof(struct sstar_usb_reset_priv),
};
