/*
 * drv_ddrfreq.c - Sigmastar
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
#include <linux/clk.h>
#include <linux/devfreq.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include "hal_ddrfreq.h"
#include "drv_ddrfreq.h"

struct sstar_ddrfreq
{
    struct devfreq *           devfreq;
    struct devfreq_dev_profile profile;
#if IS_ENABLED(CONFIG_DEVFREQ_GOV_SIMPLE_ONDEMAND)
    struct devfreq_simple_ondemand_data ondemand_data;
#endif
};

struct blocking_notifier_head devfreq_notifier;
struct notifier_block         devfreq_nb;

static int devfreq_notifier_callback(struct notifier_block *this, unsigned long index, void *data)
{
    return NOTIFY_DONE;
}

int devfreq_notifier_register_nb(struct notifier_block *nb)
{
    if (nb == NULL)
        return -EINVAL;

    blocking_notifier_chain_register(&devfreq_notifier, nb);

    return 0;
}
EXPORT_SYMBOL(devfreq_notifier_register_nb);

int devfreq_notifier_unregister_nb(struct notifier_block *nb)
{
    if (nb == NULL)
        return -EINVAL;

    blocking_notifier_chain_unregister(&devfreq_notifier, nb);

    return 0;
}
EXPORT_SYMBOL(devfreq_notifier_unregister_nb);

static int drv_ddrfreq_target(struct device *dev, unsigned long *freq, u32 flags)
{
    struct dev_pm_opp *opp;
    unsigned long      rate, req_rate;
    int                ret;

    // printk("%s, %d freq=%lu flags=%d\r\n", __FUNCTION__, __LINE__, *freq, flags);
    if (*freq == hal_ddrfreq_get_freq())
        return 0;

    req_rate = *freq;
    opp      = devfreq_recommended_opp(dev, freq, flags);
    // printk("%s, %d freq=%lu\r\n", __FUNCTION__, __LINE__, *freq);
    if (IS_ERR(opp))
        return PTR_ERR(opp);

    // printk("%s, %d\r\n", __FUNCTION__, __LINE__);
    rate = dev_pm_opp_get_freq(opp);
    dev_pm_opp_put(opp);

    blocking_notifier_call_chain(&devfreq_notifier, MIU_NOTIFY_DQ_STOP, 0);
    ret = hal_ddrfreq_target(rate);
    blocking_notifier_call_chain(&devfreq_notifier, MIU_NOTIFY_DQ_START, 0);
    dev_info(dev, "[target] ret=%d %lu - %lu\n", ret, req_rate, rate);

    return ret;
}

static int drv_ddrfreq_get_dev_status(struct device *dev, struct devfreq_dev_status *stat)
{
    hal_ddrfreq_get_load(stat);
    dev_info(dev, "[get_status] busy-total: %lu-%lu,freq %lu\n", stat->busy_time, stat->total_time,
             stat->current_frequency);

    return 0;
}

static void drv_ddrfreq_exit(struct device *dev)
{
    dev_pm_opp_of_remove_table(dev);
}

static int drv_ddrfreq_get_freq(struct device *dev, unsigned long *freq)
{
    *freq = hal_ddrfreq_get_freq();
    dev_info(dev, "[get_cur_freq] %lu\n", *freq);

    return 0;
}

const unsigned char ddr2_data   = SSTAR_DDR2_SDRAM;
const unsigned char ddr3_data   = SSTAR_DDR3_SDRAM;
const unsigned char ddr4_data   = SSTAR_DDR4_SDRAM;
const unsigned char lpddr4_data = SSTAR_LPDDR4_SDRAM;

static const struct of_device_id sstar_ddrfreq_of_match[] = {
    {
        .compatible = "sstar,devfreq-ddr2",
        .data       = &ddr2_data,
    },
    {
        .compatible = "sstar,devfreq-ddr3",
        .data       = &ddr3_data,
    },
    {
        .compatible = "sstar,devfreq-ddr4",
        .data       = &ddr4_data,
    },
    {
        .compatible = "sstar,devfreq-lpddr4",
        .data       = &lpddr4_data,
    },
    {},
};
MODULE_DEVICE_TABLE(of, sstar_ddrfreq_of_match);

static int drv_ddrfreq_probe(struct platform_device *pdev)
{
    struct sstar_ddrfreq *     sstar;
    struct device *            dev = &pdev->dev;
    struct device_node *       np  = pdev->dev.of_node;
    struct dev_pm_opp *        opp;
    int                        err;
    const struct of_device_id *match;
    const unsigned char *      type;
    const char *               governor = NULL;
    void *                     data     = NULL;
    unsigned long              rate;

    match = of_match_device(sstar_ddrfreq_of_match, &pdev->dev);
    if (!match || !match->data)
        return -ENODEV;
    type = match->data;
    if (*type != hal_ddrfreq_get_sdram_type())
        return -ENODEV;

    sstar = devm_kzalloc(dev, sizeof(*sstar), GFP_KERNEL);
    if (!sstar)
        return -ENOMEM;

    err = dev_pm_opp_of_add_table(dev);
    if (err < 0)
    {
        dev_err(dev, "failed to get OPP table\n");
        return err;
    }

    hal_ddrfreq_init();
    platform_set_drvdata(pdev, sstar);

    sstar->profile.target         = drv_ddrfreq_target;
    sstar->profile.get_dev_status = drv_ddrfreq_get_dev_status;
    sstar->profile.exit           = drv_ddrfreq_exit;
    sstar->profile.get_cur_freq   = drv_ddrfreq_get_freq;
    drv_ddrfreq_get_freq(dev, &sstar->profile.initial_freq);
    if (of_property_read_u32(np, "polling_ms", &sstar->profile.polling_ms))
        sstar->profile.polling_ms = 500;

#if IS_ENABLED(CONFIG_DEVFREQ_GOV_SIMPLE_ONDEMAND)
    data = &sstar->ondemand_data;
    if (of_property_read_u32(np, "upthreshold", &sstar->ondemand_data.upthreshold))
        sstar->ondemand_data.upthreshold = 90;
    if (of_property_read_u32(np, "downdifferential", &sstar->ondemand_data.downdifferential))
        sstar->ondemand_data.downdifferential = 10;
#endif
    of_property_read_string(np, "governor-name", &governor);

    sstar->devfreq = devfreq_add_device(&pdev->dev, &sstar->profile, governor, data);
    if (IS_ERR(sstar->devfreq))
    {
        err = PTR_ERR(sstar->devfreq);
        dev_err(dev, "failed to add devfreq device\n");
        goto remove_opps;
    }

    /* NOTE: assume the initial frequency is the MAX frequency,
     *       all higher frequencies in OPP are disabled here */
    devm_devfreq_register_opp_notifier(dev, sstar->devfreq);
    for (rate = 0;; rate++)
    {
        //("%s, %d rate=%lu\r\n", __FUNCTION__, __LINE__, rate);
        opp = dev_pm_opp_find_freq_ceil(dev, &rate);
        if (IS_ERR(opp))
            break;

        // printk("%s, %d: \r\n", __FUNCTION__, __LINE__);
        dev_pm_opp_put(opp);
        // printk("%s, %d rate=%lu, init_rate=%lu\r\n", __FUNCTION__, __LINE__, rate, sstar->profile.initial_freq);
        if (rate > sstar->profile.initial_freq)
        {
            dev_pm_opp_disable(dev, rate);
            // printk("%s, %d disable rate=%lu\r\n", __FUNCTION__, __LINE__, rate);
        }
    }

    // build blocking notifier chain
    BLOCKING_INIT_NOTIFIER_HEAD(&devfreq_notifier);
    devfreq_nb.notifier_call = devfreq_notifier_callback;
    devfreq_notifier_register_nb(&devfreq_nb);

    return 0;

remove_opps:
    dev_pm_opp_of_remove_table(dev);

    return err;
}

static struct platform_driver sstar_ddrfreq_driver = {
    .probe = drv_ddrfreq_probe,
    .driver =
        {
            .name           = "sstar-ddrfreq",
            .of_match_table = of_match_ptr(sstar_ddrfreq_of_match),
        },
};
module_platform_driver(sstar_ddrfreq_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Sigmastar ddr devfreq driver");
