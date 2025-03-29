/*
 * drv_wdt.c - Sigmastar
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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/clk-provider.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <watchdog_pretimeout.h>
#include <registers.h>
#include <ms_msys.h>
#include <ms_types.h>
#include <ms_platform.h>
#include <drv_camclk_Api.h>
#include <hal_wdt.h>
#ifdef CONFIG_SS_DUALOS
#include <drv_dualos.h>
#include <cam_inter_os.h>
#endif

// #define SSTAR_WDT_DEBUG 1

#ifdef SSTAR_WDT_DEBUG
#define wdt_dbg(fmt, ...) printk("[sstar-wdt] " fmt, ##__VA_ARGS__)
#define wdt_err(fmt, ...) printk("[sstar-wdt] " fmt, ##__VA_ARGS__)
#else
#define wdt_dbg(fmt, ...) \
    do                    \
    {                     \
    } while (0)
#define wdt_err(fmt, ...) printk("[sstar-wdt] " fmt, ##__VA_ARGS__)
#endif

#define OPTIONS (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_PRETIMEOUT)

#define WATCHDOG_DEFAULT_TIME (10) /* seconds */

struct sstar_wdt
{
    struct hal_wdt_data     hal_data;
    int                     irq;
    struct device *         dev;
    struct watchdog_device *wdd;
    spinlock_t              lock;
    struct clk *            clk;
    unsigned long           clk_rate;
    int                     opened;
    unsigned int            margin;      /* seconds */
    unsigned int            max_timeout; /* seconds */
#ifdef CONFIG_SSTAR_WATCHDOG_GOV
    dev_t                  dev_t;
    struct cdev            cdev;
    struct device *        wdt_dev;
    struct file_operations ops;
    struct fasync_struct * async_queue;
#endif
};

#ifdef CONFIG_SSTAR_WATCHDOG_GOV
static struct class *sstar_class;
#endif

#ifdef CONFIG_SS_DUALOS
static int wdt_disable_rtos = 0;

static int __init wdt_disable_rtos_func(char *str)
{
    wdt_disable_rtos = simple_strtol(str, NULL, 10);
    return 0;
}
early_param("disable_rtos", wdt_disable_rtos_func);

static int write_riu_via_rtos(u32 addr, u16 val)
{
    rtkinfo_t *rtk = NULL;

    rtk = get_rtkinfo();
    if (!rtk)
    {
        wdt_err("get RTOS handle fail\n");
        return -EINVAL;
    }

    snprintf(rtk->sbox, sizeof(rtk->sbox), "regset 0x%08X 0x%04X", (u32)addr, val);
    wdt_dbg("via rtos do: %s\n", rtk->sbox);

    return CamInterOsSignal(INTEROS_SC_L2R_RTK_CLI, 0, 0, 0);
}
#endif

static u32 __wdt_get_clk_rate(struct sstar_wdt *wdt)
{
#ifdef CONFIG_CAM_CLK
    u32 wdt_clk = 0;
#endif
    unsigned long rate = 0;

#ifdef CONFIG_CAM_CLK
    of_property_read_u32_index(wdt->dev.of_node, "camclk", 0, &wdt_clk);
    if (!wdt_clk)
    {
        wdt_dbg("failed to get clk!\n");
    }
    else
    {
        rate = CamClkRateGet(wdt_clk);
    }
#else
    if (IS_ERR(wdt->clk))
    {
        return 0;
    }

    rate = clk_get_rate(wdt->clk);
#endif

    return rate;
}

/* This API will get the WDT clock frequency as much as
 * possible and update it to wdt->clk_rate.
 */
static void wdt_clk_rate_update(struct sstar_wdt *wdt)
{
    wdt->clk_rate = __wdt_get_clk_rate(wdt);
    if (wdt->clk_rate == 0)
    {
        wdt->clk_rate = 12000000;
    }

    wdt_dbg("update clock rate: %lu\n", wdt->clk_rate);

    return;
}

static int sstar_wdt_stop(struct watchdog_device *wdd)
{
    struct sstar_wdt *wdt = watchdog_get_drvdata(wdd);

    wdt_dbg("stop\n");

    spin_lock(&wdt->lock);
    if (!wdt->opened)
    {
        goto unlock_out;
    }
    wdt->opened = 0;
    hal_wdt_stop(&wdt->hal_data);
    spin_unlock(&wdt->lock);

    if (!IS_ERR_OR_NULL(wdt->clk) && __clk_is_enabled(wdt->clk))
    {
        clk_disable_unprepare(wdt->clk);
    }

    return 0;

unlock_out:
    spin_unlock(&wdt->lock);
    return 0;
}

static int sstar_wdt_start(struct watchdog_device *wdd)
{
    struct sstar_wdt *wdt = watchdog_get_drvdata(wdd);
    unsigned long     lunch_time;

    if (!IS_ERR_OR_NULL(wdt->clk))
    {
        clk_prepare_enable(wdt->clk);
    }
    wdt_clk_rate_update(wdt);

    spin_lock(&wdt->lock);

    hal_wdt_stop(&wdt->hal_data);

    lunch_time = (wdt->margin <= wdt->max_timeout) ? wdt->margin : wdt->max_timeout;
    hal_wdt_start(&wdt->hal_data, -1ULL, lunch_time * wdt->clk_rate);

    wdt->opened = 1;

    spin_unlock(&wdt->lock);

    wdt_dbg("start, timeout: %lu\n", lunch_time);

    return 0;
}

static int sstar_wdt_set_timeout(struct watchdog_device *wdd, unsigned int timeout)
{
    unsigned long long time_tick;
    struct sstar_wdt * wdt = watchdog_get_drvdata(wdd);

    wdt_dbg("set timeout: %d\n", timeout);

    if (timeout < 5)
        timeout = 5;

    if (timeout > wdt->max_timeout)
        time_tick = wdt->clk_rate * wdt->max_timeout;
    else
        time_tick = wdt->clk_rate * timeout;

    spin_lock(&wdt->lock);

    wdt->margin  = timeout;
    wdd->timeout = timeout;

    hal_wdt_set_max(&wdt->hal_data, time_tick);

    spin_unlock(&wdt->lock);

    return 0;
}

static int sstar_wdt_set_pretimeout(struct watchdog_device *wdd, unsigned int timeout)
{
    unsigned long long time_tick;
    struct sstar_wdt * wdt = watchdog_get_drvdata(wdd);

    time_tick = wdt->clk_rate * timeout;

    spin_lock(&wdt->lock);

    wdd->pretimeout = timeout;

    wdt_dbg("set pretimeout: %d(%#llx)\n", timeout, time_tick);

    hal_wdt_set_int(&wdt->hal_data, time_tick);

    spin_unlock(&wdt->lock);

    return 0;
}

static int sstar_wdt_ping(struct watchdog_device *wdd)
{
    struct sstar_wdt *wdt = watchdog_get_drvdata(wdd);

    wdt_dbg("ping, current count:%#llx\n", hal_wdt_get_count(&wdt->hal_data));

#ifdef CONFIG_SS_DUALOS
    if (!wdt_disable_rtos)
    {
        write_riu_via_rtos(((u32)wdt->hal_data.reg_base - MS_IO_OFFSET) + REG_WDT_CLR, 1);
    }
    else
#endif
    {
        spin_lock(&wdt->lock);
        hal_wdt_ping(&wdt->hal_data);
        spin_unlock(&wdt->lock);
    }

    return 0;
}

static int sstar_wdt_set_heartbeat(struct watchdog_device *wdd, unsigned int timeout)
{
    struct sstar_wdt *wdt = watchdog_get_drvdata(wdd);

    if (timeout < 1)
        return -EINVAL;

    wdt_dbg("set heartbeat: %u\n", timeout);

    spin_lock(&wdt->lock);
    hal_wdt_ping(&wdt->hal_data);
    wdd->timeout = timeout;
    spin_unlock(&wdt->lock);

    return 0;
}

static irqreturn_t sstar_wdt_interrupt(int irq, void *dev_id)
{
    struct sstar_wdt *wdt = (struct sstar_wdt *)dev_id;

    watchdog_notify_pretimeout(wdt->wdd);

    return IRQ_HANDLED;
}

static const struct watchdog_info sstar_wdt_ident = {
    .options          = OPTIONS,
    .firmware_version = 0,
    .identity         = "SSTAR Watchdog",
};

static struct watchdog_ops sstar_wdt_ops = {
    .owner          = THIS_MODULE,
    .start          = sstar_wdt_start,
    .stop           = sstar_wdt_stop,
    .set_timeout    = sstar_wdt_set_timeout,
    .set_pretimeout = sstar_wdt_set_pretimeout,
    .ping           = sstar_wdt_ping,
};

#ifdef CONFIG_SSTAR_WATCHDOG_GOV
static void sstar_wdt_pretimeout(struct watchdog_device *wdd)
{
    struct sstar_wdt *wdt = watchdog_get_drvdata(wdd);

    wdt_dbg("pretimeout event\n");

    kill_fasync(&wdt->async_queue, SIGIO, POLL_IN);

    return;
}

static struct watchdog_governor sstar_watchdog_gov = {
    .name       = "sstar",
    .pretimeout = sstar_wdt_pretimeout,
};

static int sstar_watchdog_fasync(int fd, struct file *file, int on)
{
    struct sstar_wdt *wdt = container_of(file->f_op, struct sstar_wdt, ops);

    return fasync_helper(fd, file, on, &wdt->async_queue);
}
#endif

static int sstar_wdt_probe(struct platform_device *pdev)
{
    int                     ret = 0;
    struct device *         dev;
    struct sstar_wdt *      wdt;
    struct watchdog_device *wdd;
    int                     started = 0;
    struct resource *       res;

    wdt_dbg("watchdog probe\n");

    dev = &pdev->dev;
    wdt = devm_kzalloc(dev, sizeof(struct sstar_wdt), GFP_KERNEL);
    if (!wdt)
        return -ENOMEM;

    wdd = devm_kzalloc(dev, sizeof(struct watchdog_device), GFP_KERNEL);
    if (!wdd)
        return -ENOMEM;

    spin_lock_init(&wdt->lock);

    wdt->dev    = &pdev->dev;
    wdt->margin = WATCHDOG_DEFAULT_TIME;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
    {
        wdt_err("failed to get IORESOURCE_MEM\n");
        return -ENODEV;
    }
    wdt->hal_data.reg_base = (void *)IO_ADDRESS(res->start);

    wdt->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (wdt->irq == 0)
    {
        wdt_err("failed to get interrupts\n");
        return -ENOENT;
    }

    if (devm_request_irq(dev, wdt->irq, sstar_wdt_interrupt, 0, "WDT Interrupt", wdt))
    {
        wdt_err("failed to register interrupt\n");
        return -ENOENT;
    }

    wdt->clk = of_clk_get(wdt->dev->of_node, 0);
    wdt_clk_rate_update(wdt);

    wdd->ops                 = &sstar_wdt_ops;
    wdd->info                = &sstar_wdt_ident;
    wdt->max_timeout         = hal_wdt_get_max_count() / wdt->clk_rate;
    wdd->max_hw_heartbeat_ms = wdt->max_timeout * 1000;
    wdt->wdd                 = wdd;

    platform_set_drvdata(pdev, wdt);
    watchdog_set_drvdata(wdt->wdd, wdt);

    if (sstar_wdt_set_heartbeat(wdd, wdt->margin))
    {
        started = sstar_wdt_set_heartbeat(wdd, WATCHDOG_DEFAULT_TIME);

        if (started == 0)
            wdt_dbg("margin value is out of range, use default value %d instead\n", WATCHDOG_DEFAULT_TIME);
        else
            wdt_dbg("default time value is out of range, cannot start\n");
    }

    ret = watchdog_register_device(wdd);
    if (ret)
    {
        wdt_err("failed to register watchdog (%d)\n", ret);
        goto err;
    }

#ifdef CONFIG_SSTAR_WATCHDOG_GOV
    ret = alloc_chrdev_region(&wdt->dev_t, 0, 1, "wdt");
    if (ret)
    {
        wdt_err("failed to alloc watchdog char device (%d)\n", ret);
        goto err;
    }

    wdt->ops.owner  = THIS_MODULE;
    wdt->ops.fasync = sstar_watchdog_fasync;
    cdev_init(&wdt->cdev, &wdt->ops);

    ret = cdev_add(&wdt->cdev, MKDEV(MAJOR(wdt->dev_t), MINOR(wdt->dev_t)), 1);
    if (ret)
    {
        wdt_err("failed to create watchdog char device (%d)\n", ret);
        unregister_chrdev_region(wdt->dev_t, 1);
        goto err;
    }

    if (!sstar_class)
    {
        sstar_class = msys_get_sysfs_class();
    }

    wdt->wdt_dev = device_create(sstar_class, NULL, wdt->dev_t, NULL, "wdt");
    if (IS_ERR(wdt->wdt_dev))
    {
        wdt_err("failed to create watchdog char device (%d)\n", ret);
        cdev_del(&wdt->cdev);
        unregister_chrdev_region(wdt->dev_t, 1);
        ret = -ENODEV;
        goto err;
    }

    ret = watchdog_register_governor(&sstar_watchdog_gov);
    if (ret)
    {
        wdt_err("failed to register watchdog governor (%d)\n", ret);
        device_destroy(sstar_class, wdt->dev_t);
        cdev_del(&wdt->cdev);
        unregister_chrdev_region(wdt->dev_t, 1);
        goto err;
    }

    wdd->gov = &sstar_watchdog_gov;
#endif

    wdt_dbg("base:%p clk_rate:%lu max_timeout:%d, probed\n", wdt->hal_data.reg_base, wdt->clk_rate, wdt->max_timeout);

    return 0;
err:
    watchdog_unregister_device(wdd);

    return ret;
}

static int sstar_wdt_remove(struct platform_device *dev)
{
    struct sstar_wdt *wdt = platform_get_drvdata(dev);

    wdt_dbg("watchdog remove\n");

    sstar_wdt_stop(wdt->wdd);
#ifdef CONFIG_SSTAR_WATCHDOG_GOV
    watchdog_unregister_governor(&sstar_watchdog_gov);
    device_destroy(sstar_class, wdt->dev_t);
    cdev_del(&wdt->cdev);
    unregister_chrdev_region(wdt->dev_t, 1);
#endif
    watchdog_unregister_device(wdt->wdd);

    return 0;
}

static void sstar_wdt_shutdown(struct platform_device *dev)
{
    struct sstar_wdt *wdt = platform_get_drvdata(dev);

    wdt_dbg("watchdog shutdown\n");

    sstar_wdt_stop(wdt->wdd);

    return;
}

#ifdef CONFIG_PM_SLEEP

static int no_wdt_suspend = 0;
module_param_named(no_wdt_suspend, no_wdt_suspend, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(no_wdt_suspend, "you can choose: 0(disable), 1(enable)");

static int sstar_wdt_suspend(struct platform_device *dev, pm_message_t state)
{
    struct sstar_wdt *wdt = platform_get_drvdata(dev);

    wdt_dbg("watchdog suspend, no_wdt_suspend:%d\n", no_wdt_suspend);

    if (wdt->opened == 1)
    {
        if (no_wdt_suspend)
        {
            sstar_wdt_ping(wdt->wdd);
            msys_watchdog_notify(wdt->clk_rate * wdt->margin);
            return 0;
        }

        sstar_wdt_stop(wdt->wdd);
        wdt->opened = 1; // flag is cleared by sstar_wdt_stop, need to be set again for resume
    }

    return 0;
}

static int sstar_wdt_resume(struct platform_device *dev)
{
    struct sstar_wdt *wdt = platform_get_drvdata(dev);

    wdt_dbg("watchdog resume\n");

    /* Restore watchdog state */
    if (wdt->opened == 1)
    {
        if (no_wdt_suspend)
        {
            sstar_wdt_ping(wdt->wdd);
            msys_watchdog_notify(0);
            return 0;
        }

        sstar_wdt_start(wdt->wdd);
    }

    return 0;
}
#endif /* CONFIG_PM_SLEEP */

static const struct of_device_id sstar_wdt_of_match_table[] = {{.compatible = "sstar,wdt"}, {}};
MODULE_DEVICE_TABLE(of, sstar_wdt_of_match_table);

static struct platform_driver sstar_wdt_driver = {
    .probe    = sstar_wdt_probe,
    .remove   = sstar_wdt_remove,
    .shutdown = sstar_wdt_shutdown,
#ifdef CONFIG_PM_SLEEP
    .suspend = sstar_wdt_suspend,
    .resume  = sstar_wdt_resume,
#endif
    .driver =
        {
            .owner          = THIS_MODULE,
            .name           = "sstar-wdt",
            .of_match_table = sstar_wdt_of_match_table,
        },
};

module_platform_driver(sstar_wdt_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SSTAR Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS("platform:sstar-wdt");
