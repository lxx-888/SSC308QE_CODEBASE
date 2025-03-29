/*
 * sstar_dwc3_of_simple.c - Sigmastar
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
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <dm/lists.h>
#include <regmap.h>
#include <syscon.h>
#include <usb.h>
#include <linux/bitops.h>
#include <clk.h>
#include <dm/device_compat.h>

struct sstar_dwc3_of_simple
{
    struct clk_bulk clks;
    phys_addr_t     mac_syscfg_base;
    phys_addr_t     drd_syscfg_base;
    u32             x2a_off[4];
};

__maybe_unused static int sstar_dwc3_of_simple_drd_init(struct sstar_dwc3_of_simple *simple)
{
#if CONFIG_IS_ENABLED(SSTAR_USB_DWC3_HOST)
    {
        int i;
        int offset;

        offset = 0x50;
        for (i = 0; i < ARRAY_SIZE(simple->x2a_off); i++)
        {
            writew(simple->x2a_off[i] & 0xffff, simple->mac_syscfg_base + (offset << 2));
            writew((simple->x2a_off[i] >> 16) & 0xffff, simple->mac_syscfg_base + ((offset + 1) << 2));
            offset += 2;
        }
    }
#endif
    return 0;
}

static int sstar_of_simple_to_plat(struct udevice *dev)
{
    struct sstar_dwc3_of_simple *simple = dev_get_plat(dev);
    struct regmap *              regmap;
    int                          ret;

    ret = ofnode_read_u32_array(dev_ofnode(dev), "sstar,x2a-addr-off", simple->x2a_off, ARRAY_SIZE(simple->x2a_off));
    if (ret)
    {
        pr_err("unable to find sstar,x2a-addr-off property(%d)\n", ret);
        return ret;
    }
    /* get syscfg-reg base address */
    regmap = syscon_regmap_lookup_by_phandle(dev, "sstar,usb3drd-mac-syscon");
    if (!regmap)
    {
        pr_err("unable to find regmap\n");
        return -ENODEV;
    }
    simple->mac_syscfg_base = regmap->ranges[0].start;
    /* get syscfg-reg base address */
    regmap = syscon_regmap_lookup_by_phandle(dev, "sstar,usb3drd-gp-syscon");
    if (!regmap)
    {
        pr_err("unable to find regmap\n");
        return -ENODEV;
    }
    simple->drd_syscfg_base = regmap->ranges[0].start;
    dev_dbg(dev, "%s end.\r\n", __func__);
    return ret;
};

static int sstar_dwc3_of_simple_bind(struct udevice *dev)
{
    ofnode node, dwc3_node;

    /* Find snps,dwc3 node from subnode */
    ofnode_for_each_subnode(node, dev_ofnode(dev))
    {
        if (ofnode_device_is_compatible(node, "snps,dwc3"))
            dwc3_node = node;
    }

    if (!ofnode_valid(dwc3_node))
    {
        pr_err("Can't find dwc3 subnode for %s\n", dev->name);
        return -ENODEV;
    }

#if CONFIG_IS_ENABLED(USB_DWC3_GADGET)
    {
        struct udevice *pdev;
        int             ret;

        ret = device_bind_driver_to_node(dev, "dwc3-generic-peripheral", ofnode_get_name(dwc3_node), dwc3_node, &pdev);
        if (ret)
        {
            dev_err(dev, "not able to bind usb device mode\n");
            return ret;
        }
    }
    return 0;
#endif
    return dm_scan_fdt_dev(dev);
}

static int sstar_dwc3_of_simple_probe(struct udevice *dev)
{
    struct sstar_dwc3_of_simple *simple = dev_get_plat(dev);
    int                          ret;
    struct reset_ctl_bulk *      reset;

    dev_dbg(dev, "%s start\r\n", __func__);

    reset = devm_reset_bulk_get_optional(dev);
    if (reset)
        reset_deassert_bulk(reset);

    ret = clk_get_bulk(dev, &simple->clks);
    if (ret == -ENOSYS || ret == -ENOENT)
        goto out;
    if (ret)
        return ret;

#if CONFIG_IS_ENABLED(SSTAR_CLK)
    dev_dbg(dev, "%s clk_enable_bulk\r\n", __func__);

    ret = clk_enable_bulk(&simple->clks);
    if (ret)
    {
        clk_release_bulk(&simple->clks);
        return ret;
    }
#endif
out:
    if (ret)
    {
        dev_dbg(dev, "%s clk_enable manually\r\n", __func__);
        /* 1. turn on CLK_sof_usb30_drd
           2. it will be removed and replaced by clk api next release
        */
        dev_dbg(dev, "%s disable clk\r\n", __func__);
        setbits_16(phys_to_virt(0x1f207E00 + (0x51 << 2)), BIT(0));
        dev_dbg(dev, "%s config clk\r\n", __func__);
        clrbits_16(phys_to_virt(0x1f207E00 + (0x51 << 2)), BIT(2) | BIT(3));
        setbits_16(phys_to_virt(0x1f207E00 + (0x51 << 2)), BIT(2) | BIT(3));
        dev_dbg(dev, "%s enable clk\r\n", __func__);
        clrbits_16(phys_to_virt(0x1f207E00 + (0x51 << 2)), BIT(0));
    }

    sstar_dwc3_of_simple_drd_init(simple);
    dev_dbg(dev, "%s end\r\n", __func__);
    return 0;
}

static int sstar_dwc3_of_simple_remove(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(SSTAR_CLK)
    struct sstar_dwc3_of_simple *simple = dev_get_plat(dev);

    clk_release_bulk(&simple->clks);
#endif
    return 0;
}

static const struct udevice_id sstar_dwc3_glue_ids[] = {
    {.compatible = "sstar,infinity7-dwc3"}, {.compatible = "sstar,generic-dwc3"}, {}};

U_BOOT_DRIVER(sstar_dwc3_of_simple) = {
    .name       = "sstar-dwc3-of-simple",
    .id         = UCLASS_NOP,
    .of_match   = sstar_dwc3_glue_ids,
    .of_to_plat = sstar_of_simple_to_plat,
    .probe      = sstar_dwc3_of_simple_probe,
    .remove     = sstar_dwc3_of_simple_remove,
    .bind       = sstar_dwc3_of_simple_bind,
    .plat_auto  = sizeof(struct sstar_dwc3_of_simple),
    .flags      = DM_FLAG_ALLOC_PRIV_DMA,
};
