/*
 * drv_timer.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <ms_msys.h>
#include <registers.h>
#include <ms_types.h>
#include <ms_platform.h>
#include <irqs.h>
#include <drv_timer.h>
#include <hal_timer.h>

#define ERR_TIMER_FIND   1
#define ERR_TIMER_USED   2
#define ERR_TIMER_INT    3
#define ERR_TIMER_BUSY   4
#define ERR_TIMER_NOUSED 5

#define OSC_CLK_12000000 12000000

#define TIMER_DEV_MAX 16 /* max 16 timer devices supported */

// #define SSTAR_TIMER_DEBUG 1
#ifdef SSTAR_TIMER_DEBUG
#define TIMER_LOG printk
#else
#define TIMER_LOG(fmt, ...)
#endif

struct sstar_timer
{
    struct hal_timer_data hal_data;
    unsigned int          id;
    char                  dev_name[20];
    unsigned int          irq;           // virtual irq number
    unsigned long         int_count;     // interrupt count
    bool                  used;          // whether timer is registered
    bool                  started;       // whether timer is started
    unsigned long long    exp_time;      // expire time (ms)
    struct clk *          clk;           // clock struct
    bool                  clk_on_demand; // control clk on demand
    unsigned int          clk_rate;      // clock rate for this timer
    struct spinlock       lock;
    enum sstar_timer_mode mode; // timer running mode
    void *                pdata;
    sstar_timer_callback  callback;
    struct device *       dev;
};

static DEFINE_IDR(sstar_timer_idr);
static dev_t timer_devt;

static irqreturn_t sstar_timer_interrupt(int irq, void *pdata)
{
    struct sstar_timer *timer_dev = pdata;

    /* Since timer0~7 share identical IRQ, each IRQ will trigger 8 ISRs.
     * It's possible to take 5.46ms in ISRs @ 12MHz due to regsiter access if corresponding clock was disabled.
     * So we skip those who's clock was disabled.
     */
    if (timer_dev->clk_on_demand && !__clk_is_enabled(timer_dev->clk))
    {
        return IRQ_HANDLED;
    }

    if (hal_timer_interrupt_status(&timer_dev->hal_data))
    {
        hal_timer_interrupt_clear(&timer_dev->hal_data);

        if (SSTAR_TIMER_MODE_ONESHOT == timer_dev->mode)
        {
            timer_dev->started = FALSE;
        }

        if (timer_dev->callback)
            timer_dev->callback(timer_dev->pdata); // put the timer id info to callback's pdata
        timer_dev->int_count++;
    }

    return IRQ_HANDLED;
}

/**
 * sstar_timer_register - try to take an idle timer and request irq
 * @timer_id: The timer id [1,max]
 * @mode: 0-timer counting one time (from 0 to max, then stop)
 *        1-enable timer counting rolled (from 0 to max, then rolled)
 * @callback: Function to be called when the IRQ occurs.
 * @pdata: Private data passed to callback function.
 * return: 0-success, negative value-fail
 *
 * This API can't be used in interrupt context
 */
sstar_timer_handle sstar_timer_register(unsigned int timer_id, enum sstar_timer_mode mode,
                                        sstar_timer_callback callback, void *pdata)
{
    sstar_timer_handle  ret       = NULL;
    struct sstar_timer *timer_dev = NULL;
    unsigned long       flags;

    timer_dev = idr_find(&sstar_timer_idr, timer_id);

    if (timer_dev)
    {
        spin_lock_irqsave(&timer_dev->lock, flags);
        if (FALSE == timer_dev->used)
        {
            timer_dev->used      = TRUE;
            timer_dev->started   = FALSE;
            timer_dev->exp_time  = 0;
            timer_dev->int_count = 0;
            timer_dev->mode      = mode;
            timer_dev->callback  = callback;
            timer_dev->pdata     = pdata;
            ret                  = timer_dev;
        }
        spin_unlock_irqrestore(&timer_dev->lock, flags);
    }
    else
    {
        TIMER_LOG(KERN_ERR "%s,line:%d can't find timer%d.\n", __FUNCTION__, __LINE__, timer_id);
    }

    return ret;
}
EXPORT_SYMBOL(sstar_timer_register);

/**
 * sstar_timer_unregister - stop timer and free irq
 * @handle: The timer device
 * return: 0-success, negative value-fail
 *
 * This API can't be used in interrupt context
 */
int sstar_timer_unregister(sstar_timer_handle handle)
{
    int                 ret       = 0;
    struct sstar_timer *timer_dev = handle;
    unsigned long       flags;

    if (timer_dev)
    {
        spin_lock_irqsave(&timer_dev->lock, flags);
        if (TRUE == timer_dev->used)
        {
            hal_timer_stop(&timer_dev->hal_data);

            if (!IS_ERR_OR_NULL(timer_dev->clk))
            {
                if (timer_dev->clk_on_demand && __clk_is_enabled(timer_dev->clk))
                {
                    clk_disable_unprepare(timer_dev->clk);
                }
            }

            timer_dev->used     = FALSE;
            timer_dev->started  = FALSE;
            timer_dev->callback = NULL;
        }
        spin_unlock_irqrestore(&timer_dev->lock, flags);
    }
    else
    {
        ret = -ERR_TIMER_FIND;
        TIMER_LOG(KERN_ERR "%s,line:%d can't find timer%d.\n", __FUNCTION__, __LINE__, timer_dev->id);
    }

    return ret;
}
EXPORT_SYMBOL(sstar_timer_unregister);

/**
 * sstar_timer_start - enable the timer and set the expire time
 * @handle: The timer device
 * @exp_time: The timer expire time in ms unit
 * return: 0-success, negative value-fail
 *
 * This API can be used in interrupt context
 */
int sstar_timer_start(sstar_timer_handle handle, unsigned long long exp_time)
{
    int                 ret       = 0;
    struct sstar_timer *timer_dev = handle;
    unsigned long       flags;
    unsigned long long  expire;

    if (timer_dev)
    {
        spin_lock_irqsave(&timer_dev->lock, flags);
        if (TRUE == timer_dev->used)
        {
            timer_dev->started  = TRUE;
            timer_dev->exp_time = exp_time;

            if (!IS_ERR_OR_NULL(timer_dev->clk))
            {
                if (!__clk_is_enabled(timer_dev->clk))
                {
                    clk_prepare_enable(timer_dev->clk);
                }
            }

            expire = timer_dev->exp_time * timer_dev->clk_rate / 1000;
            hal_timer_start(&timer_dev->hal_data, timer_dev->mode, expire);
            hal_timer_interrupt_enable(&timer_dev->hal_data, 1);
        }
        else
        {
            ret = -ERR_TIMER_NOUSED;
            TIMER_LOG(KERN_ERR "%s,line:%d %s has not been registered.\n", __FUNCTION__, __LINE__, timer_dev->dev_name);
        }
        spin_unlock_irqrestore(&timer_dev->lock, flags);
    }
    else
    {
        ret = -ERR_TIMER_FIND;
        TIMER_LOG(KERN_ERR "%s,line:%d can't find timer%d.\n", __FUNCTION__, __LINE__, timer_dev->id);
    }

    return ret;
}
EXPORT_SYMBOL(sstar_timer_start);

/**
 * sstar_timer_stop - stop the timer
 * @handle: The timer device
 * return: 0-success, negative value-fail
 *
 * This API can be used in interrupt context
 */
int sstar_timer_stop(sstar_timer_handle handle)
{
    int                 ret       = 0;
    struct sstar_timer *timer_dev = handle;
    unsigned long       flags;

    if (timer_dev)
    {
        spin_lock_irqsave(&timer_dev->lock, flags);
        if (TRUE == timer_dev->used)
        {
            timer_dev->started = FALSE;

            hal_timer_stop(&timer_dev->hal_data);

            if (!IS_ERR_OR_NULL(timer_dev->clk))
            {
                if (timer_dev->clk_on_demand && __clk_is_enabled(timer_dev->clk))
                {
                    clk_disable_unprepare(timer_dev->clk);
                }
            }
        }
        else
        {
            ret = -ERR_TIMER_NOUSED;
            TIMER_LOG(KERN_ERR "%s,line:%d %s has not been registered.\n", __FUNCTION__, __LINE__, timer_dev->dev_name);
        }
        spin_unlock_irqrestore(&timer_dev->lock, flags);
    }
    else
    {
        ret = -ERR_TIMER_FIND;
        TIMER_LOG(KERN_ERR "%s,line:%d can't find timer%d.\n", __FUNCTION__, __LINE__, timer_dev->id);
    }

    return ret;
}
EXPORT_SYMBOL(sstar_timer_stop);

/**
 * sstar_timer_get_current - get time (in ms unit) that has passed since timer start running
 * @handle: The timer device
 * @ptime: The time in ms unit
 * return: 0-success, negative value-fail
 *
 * This API can be used in interrupt context
 */
int sstar_timer_get_current(sstar_timer_handle handle, unsigned long long *ptime)
{
    int                 ret       = 0;
    struct sstar_timer *timer_dev = handle;
    unsigned long long  timer_count;

    *ptime = 0;

    if (timer_dev)
    {
        if (TRUE == timer_dev->used)
        {
            timer_count = hal_timer_count(&timer_dev->hal_data);
            if (0 != timer_dev->clk_rate)
            {
                *ptime = (timer_count / timer_dev->clk_rate) * 1000;
            }
        }
        else
        {
            ret = -ERR_TIMER_NOUSED;
            TIMER_LOG(KERN_ERR "%s,line:%d %s has not been registered.\n", __FUNCTION__, __LINE__, timer_dev->dev_name);
        }
    }
    else
    {
        ret = -ERR_TIMER_FIND;
        TIMER_LOG(KERN_ERR "%s,line:%d can't find timer%d.\n", __FUNCTION__, __LINE__, timer_dev->id);
    }

    return ret;
}
EXPORT_SYMBOL(sstar_timer_get_current);

/**
 * sstar_timer_device_count - count the quantity of hardware timers
 * return: the quantity of timers
 */
int sstar_timer_device_count(void)
{
    int                 id;
    struct sstar_timer *timer_dev;

    idr_for_each_entry(&sstar_timer_idr, timer_dev, id) {}

    return id;
}
EXPORT_SYMBOL(sstar_timer_device_count);

/**
 * sstar_timer_find_idle - find the first available timer
 * return: the timer id
 */
int sstar_timer_find_idle(void)
{
    int                 id;
    struct sstar_timer *timer_dev;

    idr_for_each_entry(&sstar_timer_idr, timer_dev, id)
    {
        if (FALSE == timer_dev->used)
        {
            break;
        }
    }

    return id;
}
EXPORT_SYMBOL(sstar_timer_find_idle);

static ssize_t sstar_timer_get_info(struct device *dev, struct device_attribute *attr, char *buf)
{
    int                 fill      = 0;
    struct sstar_timer *timer_dev = dev_get_drvdata(dev);

    fill += sprintf(buf + fill, "%s\t\t: %d\n", "id", timer_dev->id);
    fill += sprintf(buf + fill, "%s\t\t: %s\n", "name", timer_dev->dev_name);
    fill += sprintf(buf + fill, "%s\t\t: %d\n", "mode", timer_dev->mode);
    fill += sprintf(buf + fill, "%s\t\t: %d\n", "used", timer_dev->used);
    fill += sprintf(buf + fill, "%s\t\t: %d\n", "started", timer_dev->started);
    fill += sprintf(buf + fill, "%s\t\t: %lld\n", "time", timer_dev->exp_time);
    fill += sprintf(buf + fill, "%s\t\t: %d\n", "irq", timer_dev->irq);
    fill += sprintf(buf + fill, "%s\t: %ld\n", "int_count", timer_dev->int_count);
    fill += sprintf(buf + fill, "%s\t\t: %d\n", "clk", timer_dev->clk_rate);

    return fill;
}

static DEVICE_ATTR(info, 0444, sstar_timer_get_info, NULL);

static void sstar_timer_free_resource(struct device *dev, struct sstar_timer *timer_dev)
{
    if (NULL == timer_dev)
    {
        return;
    }

    // stop timer and release irq
    if (timer_dev->used)
    {
        if (!IS_ERR_OR_NULL(timer_dev->clk))
        {
            if (!__clk_is_enabled(timer_dev->clk))
            {
                clk_prepare_enable(timer_dev->clk);
            }
        }
        hal_timer_stop(&timer_dev->hal_data);

        if (!IS_ERR_OR_NULL(timer_dev->clk))
        {
            if (timer_dev->clk_on_demand && __clk_is_enabled(timer_dev->clk))
            {
                clk_disable_unprepare(timer_dev->clk);
            }
        }
    }

    if (timer_dev->id >= 0)
        idr_remove(&sstar_timer_idr, timer_dev->id);

    devm_kfree(dev, timer_dev);
}

static unsigned int sstar_timer_get_clk_rate(struct device *dev)
{
    struct clk * clk;
    unsigned int rate = 0;

    clk = of_clk_get(dev->of_node, 0);
    if (IS_ERR_OR_NULL(clk))
    {
        rate = 0;
    }
    else
    {
        rate = (unsigned int)clk_get_rate(clk);
        clk_put(clk);
    }

    if (rate == 0)
    {
        rate = OSC_CLK_12000000;
    }

    return rate;
}

static int sstar_timer_probe(struct platform_device *pdev)
{
    int                 ret = 0;
    struct device *     dev;
    struct sstar_timer *timer_dev = NULL;
    struct resource *   res       = NULL;
    dev_t               devt;

    dev = &pdev->dev;

    // alloc memory for timer device instance
    timer_dev = devm_kzalloc(dev, sizeof(struct sstar_timer), GFP_KERNEL);
    if (!timer_dev)
    {
        dev_err(dev, "%s,line:%d failed to alloc device memory.\n", __FUNCTION__, __LINE__);
        goto err_out;
    }

    timer_dev->id = idr_alloc(&sstar_timer_idr, timer_dev, 0, 0, GFP_KERNEL);

    if (timer_dev->id < 0)
    {
        goto err_out;
    }

    timer_dev->clk           = of_clk_get(dev->of_node, 0);
    timer_dev->clk_on_demand = of_property_read_bool(dev->of_node, "clock-on-demand");

    // parse the resource from dts
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
    {
        dev_err(dev, "%s,line:%d parse register fail.\n", __FUNCTION__, __LINE__);
        goto err_out;
    }

    timer_dev->hal_data.reg_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(timer_dev->hal_data.reg_base))
    {
        dev_err(dev, "%s,line:%d parse register fail.\n", __FUNCTION__, __LINE__);
        goto err_out;
    }

    ret = platform_get_irq(pdev, 0);
    if (ret)
    {
        timer_dev->irq = ret;
    }
    else
    {
        dev_err(dev, "%s,line:%d parse irq fail.\n", __FUNCTION__, __LINE__);
        goto err_out;
    }

    timer_dev->clk_rate = sstar_timer_get_clk_rate(dev);
    platform_set_drvdata(pdev, timer_dev);

    // lock init
    spin_lock_init(&timer_dev->lock);

    sprintf(timer_dev->dev_name, "timer%d", timer_dev->id);
    devt           = MKDEV(MAJOR(timer_devt), MINOR(timer_devt) + timer_dev->id);
    timer_dev->dev = device_create(msys_get_sysfs_class(), NULL, devt, NULL, timer_dev->dev_name);
    if (!timer_dev->dev)
    {
        dev_err(dev, "[%s][%d] create device file fail\n", __FUNCTION__, __LINE__);
        goto err_out;
    }

    dev_set_drvdata(timer_dev->dev, (void *)timer_dev);
    device_create_file(timer_dev->dev, &dev_attr_info);

    if (0 != request_irq(timer_dev->irq, sstar_timer_interrupt, IRQF_SHARED, timer_dev->dev_name, timer_dev))
    {
        goto err_out;
    }

    pr_debug("reg_base:0x%lx,irq:%u\n", (unsigned long)timer_dev->hal_data.reg_base, (unsigned int)timer_dev->irq);

    return 0;

err_out:
    sstar_timer_free_resource(dev, timer_dev);

    return -1;
}

static int sstar_timer_remove(struct platform_device *pdev)
{
    struct sstar_timer *timer_dev = platform_get_drvdata(pdev);

    free_irq(timer_dev->irq, timer_dev);
    device_remove_file(timer_dev->dev, &dev_attr_info);
    device_destroy(msys_get_sysfs_class(), timer_dev->dev->devt);
    sstar_timer_free_resource(&pdev->dev, timer_dev);

    return 0;
}

static void sstar_timer_shutdown(struct platform_device *dev) {}

#ifdef CONFIG_PM_SLEEP
static int sstar_timer_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct sstar_timer *timer_dev;

    timer_dev = platform_get_drvdata(pdev);
    if (timer_dev->started)
    {
        hal_timer_stop(&timer_dev->hal_data);

        if (!IS_ERR_OR_NULL(timer_dev->clk))
        {
            if (timer_dev->clk_on_demand && __clk_is_enabled(timer_dev->clk))
            {
                clk_disable_unprepare(timer_dev->clk);
            }
        }

        disable_irq(timer_dev->irq);
    }

    return 0;
}

static int sstar_timer_resume(struct platform_device *pdev)
{
    struct sstar_timer *timer_dev;
    unsigned long long  expire;

    timer_dev = platform_get_drvdata(pdev);
    if (timer_dev->started)
    {
        enable_irq(timer_dev->irq);

        if (!IS_ERR_OR_NULL(timer_dev->clk))
        {
            if (!__clk_is_enabled(timer_dev->clk))
            {
                clk_prepare_enable(timer_dev->clk);
            }
        }

        expire = timer_dev->exp_time * timer_dev->clk_rate / 1000;
        hal_timer_start(&timer_dev->hal_data, timer_dev->mode, expire);
        hal_timer_interrupt_enable(&timer_dev->hal_data, 1);
    }

    return 0;
}
#endif

static const struct of_device_id sstar_timer_of_match_table[] = {{.compatible = "sstar,timer"}, {}};

MODULE_DEVICE_TABLE(of, sstar_timer_of_match_table);

static struct platform_driver sstar_timer_driver = {
    .probe    = sstar_timer_probe,
    .remove   = sstar_timer_remove,
    .shutdown = sstar_timer_shutdown,
#ifdef CONFIG_PM_SLEEP
    .suspend = sstar_timer_suspend,
    .resume  = sstar_timer_resume,
#endif
    .driver =
        {
            .owner          = THIS_MODULE,
            .name           = "sstar,timer",
            .of_match_table = sstar_timer_of_match_table,
        },
};

// module_platform_driver(sstar_timer_driver);
static int __init sstar_timer_init(void)
{
    alloc_chrdev_region(&timer_devt, 0, TIMER_DEV_MAX, "timer");
    return platform_driver_register(&sstar_timer_driver);
}
module_init(sstar_timer_init);

static void __exit sstar_timer_exit(void)
{
    unregister_chrdev_region(timer_devt, TIMER_DEV_MAX);
    platform_driver_unregister(&sstar_timer_driver);
}
module_exit(sstar_timer_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sigmastar Timer Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sstar-timer");
