/*
 * sstar-rng.c- Sigmastar
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
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/hw_random.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/slab.h>
#include "drv_camclk_Api.h"

#include <halAESDMA.h>

#define to_sstar_rng(p) container_of(p, struct sstar_rng, rng)
extern void                    enableRngClock(void);
extern void                    disableRngClock(void);
extern struct platform_device *psg_mdrv_rng;

struct sstar_rng
{
    void __iomem *base;
    struct hwrng  rng;
};

static int sstar_rng_read(struct hwrng *rng, void *buf, size_t max, bool wait)
{
    // struct sstar_rng *hrng = to_sstar_rng(rng);
    u16 *data = buf;
    // printk("[%s,%d] max(%d)\n",__FUNCTION__,__LINE__, max);
    enableRngClock();
    *data = HAL_RNG_Read();
    disableRngClock();
    return 2;
}

int sstar_rng_probe(struct platform_device *pdev)
{
    struct sstar_rng *rng;
    // struct resource *res;
    int ret;
    // printk("[%s,%d]\n", __FUNCTION__, __LINE__);
    psg_mdrv_rng = pdev;
    rng          = devm_kzalloc(&pdev->dev, sizeof(*rng), GFP_KERNEL);
    if (!rng)
        return -ENOMEM;
    platform_set_drvdata(pdev, rng);
    // res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    // rng->base = devm_ioremap_resource(&pdev->dev, res);
    // if (IS_ERR(rng->base))
    //	return PTR_ERR(rng->base);

    rng->rng.name    = pdev->name;
    rng->rng.init    = NULL;
    rng->rng.cleanup = NULL;
    rng->rng.read    = sstar_rng_read;
    rng->rng.quality = 500;

    enableRngClock();
    ret = HAL_RNG_init();
    disableRngClock();

    if (!ret)
    {
        dev_err(&pdev->dev, "failed to init hwrng\n");
        return -ENOMEM;
    }
    ret = devm_hwrng_register(&pdev->dev, &rng->rng);
    if (ret)
    {
        dev_err(&pdev->dev, "failed to register hwrng\n");
        return ret;
    }
    return 0;
}

static const struct of_device_id sstar_rng_dt_ids[] = {{.compatible = "sstar,rng"}, {}};
MODULE_DEVICE_TABLE(of, sstar_rng_dt_ids);

static struct platform_driver sstar_rng_driver = {
    .probe = sstar_rng_probe,
    .driver =
        {
            .name           = "sstar-rng",
            .of_match_table = of_match_ptr(sstar_rng_dt_ids),
        },
};

module_platform_driver(sstar_rng_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SStar random number generator driver");
