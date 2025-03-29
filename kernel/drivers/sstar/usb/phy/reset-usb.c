/*
 * reset-usb.c - Sigmastar
 *
 * Copyright (c) [2019~2023] SigmaStar Technology.
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
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>

struct sstar_usb_reset_controller
{
    struct reset_controller_dev rst;
    struct regmap *             map;
};

#define to_sstar_usb_reset_controller(_rst) container_of(_rst, struct sstar_usb_reset_controller, rst)

static int sstar_usb_reset_assert(struct reset_controller_dev *rcdev, unsigned long idx)
{
    struct sstar_usb_reset_controller *rc     = to_sstar_usb_reset_controller(rcdev);
    unsigned int                       offset = idx >> 16;
    unsigned int                       mask   = BIT(idx & 0xffff);

    dev_info(rc->rst.dev, "usb reset assert\n");
    return regmap_update_bits(rc->map, (offset << 2), mask, mask);
}

static int sstar_usb_reset_deassert(struct reset_controller_dev *rcdev, unsigned long idx)
{
    struct sstar_usb_reset_controller *rc     = to_sstar_usb_reset_controller(rcdev);
    unsigned int                       offset = idx >> 16;
    unsigned int                       mask   = BIT(idx & 0xffff);

    dev_info(rc->rst.dev, "usb reset deassert\n");
    return regmap_clear_bits(rc->map, (offset << 2), mask);
}

static int sstar_usb_reset_dev(struct reset_controller_dev *rcdev, unsigned long idx)
{
    int err;

    err = sstar_usb_reset_assert(rcdev, idx);
    if (err)
        return err;

    return sstar_usb_reset_deassert(rcdev, idx);
}

static const struct reset_control_ops sstar_usb_reset_ops = {
    .reset    = sstar_usb_reset_dev,
    .assert   = sstar_usb_reset_assert,
    .deassert = sstar_usb_reset_deassert,
};

static int sstar_usb_reset_xlate(struct reset_controller_dev *rcdev, const struct of_phandle_args *reset_spec)
{
    unsigned int offset, bit;

    offset = reset_spec->args[0];
    bit    = reset_spec->args[1];

    return (offset << 16) | bit;
}

static int sstar_usb_reset_probe(struct platform_device *pdev)
{
    struct sstar_usb_reset_controller *rc;
    struct device_node *               np  = pdev->dev.of_node;
    struct device *                    dev = &pdev->dev;

    rc = devm_kzalloc(dev, sizeof(*rc), GFP_KERNEL);
    if (!rc)
        return -ENOMEM;

    rc->map = syscon_regmap_lookup_by_phandle(np, "rst-syscon");
    if (IS_ERR(rc->map))
    {
        dev_err(dev, "failed to get rst-syscon\n");
        return PTR_ERR(rc->map);
    }

    rc->rst.ops              = &sstar_usb_reset_ops;
    rc->rst.of_node          = np;
    rc->rst.of_reset_n_cells = 2;
    rc->rst.of_xlate         = sstar_usb_reset_xlate;
    rc->rst.dev              = dev;

    return reset_controller_register(&rc->rst);
}

static const struct of_device_id sstar_usb_reset_match[] = {
    {
        .compatible = "sstar,usb-reset",
    },
    {},
};
MODULE_DEVICE_TABLE(of, sstar_usb_reset_match);

static struct platform_driver sstar_usb_reset_driver = {
    .probe = sstar_usb_reset_probe,
    .driver =
        {
            .name           = "sstar-usb-reset",
            .of_match_table = sstar_usb_reset_match,
        },
};

static int __init sstar_usb_reset_init(void)
{
    return platform_driver_register(&sstar_usb_reset_driver);
}
arch_initcall(sstar_usb_reset_init);

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:reset-usb");
MODULE_AUTHOR("Zuhuang Zhang <zuhuang.zhang@sigmastar.com.cn>");
MODULE_DESCRIPTION("Sigmastar USB Reset Control driver");
