/*
 * drv_adclp.c- Sigmastar
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

#include <ms_msys.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/of.h>
#include <drv_adclp.h>
#include <hal_adclp.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <ms_platform.h>
#include <linux/of_irq.h>
#include <linux/module.h>
#include <linux/iio/iio.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/moduleparam.h>
#include <linux/clk-provider.h>
#include <linux/platform_device.h>

enum adclp_vdd_type
{
    ADCLP_VDD_CPU = 0,
    ADCLP_VDD_IPU,
};

struct adclp_cb
{
    adclp_cb_t       cb_t;
    u8               channel;
    struct list_head cb_node;
};

struct iio_control
{
    struct iio_info      info;
    struct iio_chan_spec channels;
};

struct adclp_control
{
    int                      irq;
    struct file_operations   fops;
    struct device *          sysdev;
    bool                     int_en;
    u32                      channel;
    struct clk *             src_clk;
    u8                       en_flag;
    u8                       workrun;
    struct work_struct       adc_work;
    struct completion        adc_done;
    struct iio_control *     iio_ctrl;
    u32                      clock_en;
    struct hal_adclp_dev     adclp_dev;
    struct clk_hw *          parent_hw;
    struct cdev              adclp_cdev;
    dev_t                    adclp_dev_t;
    struct fasync_struct *   async_queue;
    char                     irq_name[16];
    char                     dev_name[16];
    struct list_head         channel_node;
    struct workqueue_struct *adc_workqueue;
};

static LIST_HEAD(adclp_channel_list);
static LIST_HEAD(adclp_callback_list);

static int sstar_adclp_iio_read(struct iio_dev *indio_dev, struct iio_chan_spec const *channel, int *val, int *val2,
                                long mask)
{
    int                   ret        = 0;
    struct adclp_control *adclp_ctrl = (struct adclp_control *)dev_get_drvdata(indio_dev->dev.parent);

    if (!adclp_ctrl)
    {
        return -ENOMEM;
    }

    switch (mask)
    {
        case IIO_CHAN_INFO_RAW:
            *val = hal_adclp_get_data(&adclp_ctrl->adclp_dev);
            ret  = IIO_VAL_INT;
            break;

        case IIO_CHAN_INFO_SCALE:
            *val  = adclp_ctrl->adclp_dev.ref_vol / 1023;
            *val2 = ((u64)(adclp_ctrl->adclp_dev.ref_vol % 1023) * 1000000000LL) / 1023;
            ret   = IIO_VAL_INT_PLUS_NANO;
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

static int sstar_adclp_open(struct inode *node, struct file *file)
{
    int                   ret;
    struct adclp_control *adclp_ctrl = (struct adclp_control *)container_of(file->f_op, struct adclp_control, fops);

    if (!adclp_ctrl->clock_en)
    {
        ret = clk_prepare_enable(adclp_ctrl->src_clk);
        if (ret)
        {
            adclp_err("fail to enable adclp%u clock\n", adclp_ctrl->channel);
            return ret;
        }
        adclp_ctrl->clock_en++;
    }
    adclp_ctrl->en_flag = 1;

    return 0;
}

static int sstar_adclp_release(struct inode *node, struct file *file)
{
    struct adclp_control *adclp_ctrl = (struct adclp_control *)container_of(file->f_op, struct adclp_control, fops);

    if (adclp_ctrl->clock_en)
    {
        clk_disable_unprepare(adclp_ctrl->src_clk);
        adclp_ctrl->clock_en--;
    }
    adclp_ctrl->en_flag = 0;

    return 0;
}

static int sstar_adclp_fasync(int fd, struct file *file, int on)
{
    struct adclp_control *adclp_ctrl = (struct adclp_control *)container_of(file->f_op, struct adclp_control, fops);

    return fasync_helper(fd, file, on, &adclp_ctrl->async_queue);
}

static long sstar_adclp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    u16                   value;
    int                   ret = 0;
    struct adclp_bound    adclp_bd;
    struct adclp_control *adclp_ctrl;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((ADCLP_IOC_MAGIC != _IOC_TYPE(cmd)) || (_IOC_NR(cmd) > ADCLP_IOC_MAXNR))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        ret = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
    }
    if (ret)
    {
        return -EFAULT;
    }

    adclp_ctrl = (struct adclp_control *)container_of(file->f_op, struct adclp_control, fops);

    switch (cmd)
    {
        case IOCTL_ADCLP_SET_BOUND:
            if (copy_from_user(&adclp_bd, (struct adclp_bound __user *)arg, sizeof(struct adclp_bound)))
            {
                return -EFAULT;
            }
            hal_adclp_set_bound(&adclp_ctrl->adclp_dev, adclp_bd.upper_bound, adclp_bd.lower_bound);
            break;

        case IOCTL_ADCLP_READ_VALUE:
            if (copy_from_user(&value, (u16 __user *)arg, sizeof(u16)))
            {
                return -EFAULT;
            }
            value = hal_adclp_get_data(&adclp_ctrl->adclp_dev);
            if (copy_to_user((u16 __user *)arg, &value, sizeof(u16)))
            {
                return -EFAULT;
            }
            break;

        default:
            adclp_err("unsupported command\n");
            return -EINVAL;
    }

    return 0;
}

static void sstar_adclp_sample_workquene(struct work_struct *work)
{
    struct adclp_cb *     cb_pos     = NULL;
    struct adclp_control *adclp_ctrl = container_of(work, struct adclp_control, adc_work);

    while (adclp_ctrl->workrun)
    {
        wait_for_completion(&adclp_ctrl->adc_done);

        if (!adclp_ctrl->workrun)
            return;

        list_for_each_entry(cb_pos, &adclp_callback_list, cb_node)
        {
            if ((cb_pos->channel == adclp_ctrl->channel) && (cb_pos->cb_t))
            {
                cb_pos->cb_t(cb_pos->channel);
            }
        }
    }
}

static irqreturn_t sstar_adclp_int_handler(int irq, void *dev_id)
{
    u16                   status;
    struct adclp_control *adclp_ctrl = (struct adclp_control *)dev_id;

    status = hal_adclp_int_status(&adclp_ctrl->adclp_dev);
    if (status)
    {
        hal_adclp_int_clear(&adclp_ctrl->adclp_dev);
        kill_fasync(&adclp_ctrl->async_queue, SIGIO, POLL_IN);
        complete(&adclp_ctrl->adc_done);
    }

    return IRQ_HANDLED;
}

static ssize_t threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *                str        = buf;
    char *                end        = buf + PAGE_SIZE;
    struct adclp_control *adclp_ctrl = (struct adclp_control *)dev_get_drvdata(dev);

    str += scnprintf(str, end - str,
                     "upper limit  :   %hu\n"
                     "lower limit  :   %hu\n",
                     adclp_ctrl->adclp_dev.upper_bound, adclp_ctrl->adclp_dev.lower_bound);

    return (str - buf);
}

static ssize_t threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                   ret        = 0;
    u16                   max        = 0;
    u16                   min        = 0;
    struct adclp_control *adclp_ctrl = (struct adclp_control *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%hu %hu", &max, &min);
    if (ret != 2)
    {
        adclp_err("invalid argument, correct format: echo [max] [min] > threshold\n");
        goto out;
    }

    hal_adclp_set_bound(&adclp_ctrl->adclp_dev, max, min);

out:
    return count;
}
static DEVICE_ATTR_RW(threshold);

static ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u16                   value      = 0;
    char *                str        = buf;
    char *                end        = buf + PAGE_SIZE;
    struct adclp_control *adclp_ctrl = (struct adclp_control *)dev_get_drvdata(dev);

    value = hal_adclp_get_data(&adclp_ctrl->adclp_dev);

    str += scnprintf(str, end - str, "%hu\n", value);

    return (str - buf);
}
static DEVICE_ATTR_RO(value);

static ssize_t enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    const char *          status     = "unknown";
    struct adclp_control *adclp_ctrl = (struct adclp_control *)dev_get_drvdata(dev);

    switch (adclp_ctrl->en_flag)
    {
        case 0:
            status = "disable";
            break;

        case 1:
            status = "enable";
            break;
    }

    return sprintf(buf, "%s\n", status);
}

static ssize_t enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int                   ret;
    u8                    enable;
    struct adclp_control *adclp_ctrl = (struct adclp_control *)dev_get_drvdata(dev);

    ret = kstrtou8(buf, 0, &enable);
    if (ret)
    {
        adclp_err("correct format: echo [1/0] > enable\n");
        return ret;
    }

    if (enable)
    {
        if (!adclp_ctrl->clock_en)
        {
            ret = clk_prepare_enable(adclp_ctrl->src_clk);
            if (ret)
            {
                adclp_err("fail to enable adclp%u clock\n", adclp_ctrl->channel);
                return ret;
            }
            adclp_ctrl->clock_en++;
        }
        adclp_ctrl->en_flag = 1;
    }
    else
    {
        if (adclp_ctrl->clock_en)
        {
            clk_disable_unprepare(adclp_ctrl->src_clk);
            adclp_ctrl->clock_en--;
        }
        adclp_ctrl->en_flag = 0;
    }

    return count;
}
static DEVICE_ATTR_RW(enable);

static int sstar_adclp_sysfs_file(struct adclp_control *adclp_ctrl)
{
    int ret = 0;

    snprintf(adclp_ctrl->dev_name, sizeof(adclp_ctrl->dev_name), "adclp%u", adclp_ctrl->channel);
    adclp_ctrl->sysdev =
        device_create(msys_get_sysfs_class(), NULL, adclp_ctrl->adclp_dev_t, NULL, "%s", adclp_ctrl->dev_name);
    if (IS_ERR_OR_NULL(adclp_ctrl->sysdev))
    {
        adclp_err("fail to creates adclp%u device and registers it with sysfs\n", adclp_ctrl->channel);
        return -ENODEV;
    }
    dev_set_drvdata(adclp_ctrl->sysdev, (void *)adclp_ctrl);

    ret = device_create_file(adclp_ctrl->sysdev, &dev_attr_enable);
    if (ret)
    {
        adclp_err("fail to add adclp%u enable sysfs file\n", adclp_ctrl->channel);
        device_destroy(msys_get_sysfs_class(), adclp_ctrl->adclp_dev_t);
        return ret;
    }

    ret = device_create_file(adclp_ctrl->sysdev, &dev_attr_value);
    if (ret)
    {
        adclp_err("fail to add adclp%u value sysfs file\n", adclp_ctrl->channel);
        device_destroy(msys_get_sysfs_class(), adclp_ctrl->adclp_dev_t);
        return ret;
    }

    ret = device_create_file(adclp_ctrl->sysdev, &dev_attr_threshold);
    if (ret)
    {
        adclp_err("fail to add adclp%u threshold sysfs file\n", adclp_ctrl->channel);
        device_destroy(msys_get_sysfs_class(), adclp_ctrl->adclp_dev_t);
        return ret;
    }

    return ret;
}

static void sstar_adclp_clock_release(struct adclp_control *adclp_ctrl)
{
    adclp_ctrl->parent_hw = clk_hw_get_parent_by_index(__clk_get_hw(adclp_ctrl->src_clk), 0);
    if (IS_ERR_OR_NULL(adclp_ctrl->parent_hw))
    {
        adclp_err("fail to get adclp%u parent\n", adclp_ctrl->channel);
        return;
    }
    clk_set_parent(adclp_ctrl->src_clk, adclp_ctrl->parent_hw->clk);
    clk_put(adclp_ctrl->src_clk);
}

static int sstar_adclp_clock_set(struct platform_device *pdev, struct adclp_control *adclp_ctrl)
{
    int ret = 0;

    adclp_ctrl->src_clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR_OR_NULL(adclp_ctrl->src_clk))
    {
        adclp_err("fail to allocate and link adclp%u clk to clk_core\n", adclp_ctrl->channel);
        return PTR_ERR(adclp_ctrl->src_clk);
    }

    adclp_ctrl->parent_hw = clk_hw_get_parent_by_index(__clk_get_hw(adclp_ctrl->src_clk), 0);
    if (IS_ERR_OR_NULL(adclp_ctrl->parent_hw))
    {
        adclp_err("fail to get adclp%u parent\n", adclp_ctrl->channel);
        return PTR_ERR(adclp_ctrl->parent_hw);
    }

    ret = clk_set_parent(adclp_ctrl->src_clk, adclp_ctrl->parent_hw->clk);
    if (ret)
    {
        adclp_err("fail to switch adclp%u clk\n", adclp_ctrl->channel);
        return ret;
    }

    return 0;
}

static int sstar_adclp_probe(struct platform_device *pdev)
{
    int                   ret        = 0;
    u32                   upper      = 0;
    u32                   lower      = 0;
    struct resource *     res        = NULL;
    struct iio_dev *      indio_dev  = NULL;
    struct adclp_control *adclp_ctrl = NULL;

    adclp_ctrl = devm_kzalloc(&pdev->dev, sizeof(struct adclp_control), GFP_KERNEL);
    if (!adclp_ctrl)
    {
        adclp_err("fail to allocate memory\n");
        return -ENOMEM;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if ((!res) || (!res->start))
    {
        adclp_err("fail to get a resource from adclp device\n");
        return PTR_ERR(res);
    }
    adclp_ctrl->adclp_dev.base = IO_ADDRESS(res->start);

    ret = of_property_read_u32(pdev->dev.of_node, "channel", &adclp_ctrl->channel);
    if (ret < 0)
    {
        adclp_err("fail to get adclp channel from dts\n");
        return ret;
    }
    else
    {
        adclp_ctrl->adclp_dev.channel = adclp_ctrl->channel;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "ref-voltage", &adclp_ctrl->adclp_dev.ref_vol);
    if (ret < 0)
    {
        adclp_dbg("fail to get adclp%u reference voltage from dts\n", adclp_ctrl->channel);
        adclp_ctrl->adclp_dev.ref_vol = 0;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "upper-bound", &upper);
    if (ret < 0)
    {
        adclp_dbg("fail to get adclp%u upper bound from dts\n", adclp_ctrl->channel);
        adclp_ctrl->adclp_dev.upper_bound = 0x3FF;
    }
    adclp_ctrl->adclp_dev.upper_bound = upper & 0x3FF;

    ret = of_property_read_u32(pdev->dev.of_node, "lower-bound", &lower);
    if (ret < 0)
    {
        adclp_dbg("fail to get adclp%u lower bound from dts\n", adclp_ctrl->channel);
        adclp_ctrl->adclp_dev.lower_bound = 0;
    }
    adclp_ctrl->adclp_dev.lower_bound = lower & 0x3FF;

    ret = sstar_adclp_clock_set(pdev, adclp_ctrl);
    if (ret)
        goto out1;

    snprintf(adclp_ctrl->dev_name, sizeof(adclp_ctrl->dev_name), "adclp-%hhu", adclp_ctrl->channel);
    ret = alloc_chrdev_region(&adclp_ctrl->adclp_dev_t, 0, 1, adclp_ctrl->dev_name);
    if (ret)
    {
        adclp_err("fail to alloc adclp%u char device\n", adclp_ctrl->channel);
        goto out2;
    }

    adclp_ctrl->fops.owner          = THIS_MODULE;
    adclp_ctrl->fops.open           = sstar_adclp_open;
    adclp_ctrl->fops.release        = sstar_adclp_release;
    adclp_ctrl->fops.fasync         = sstar_adclp_fasync;
    adclp_ctrl->fops.unlocked_ioctl = sstar_adclp_ioctl;
    cdev_init(&adclp_ctrl->adclp_cdev, &adclp_ctrl->fops);
    ret = cdev_add(&adclp_ctrl->adclp_cdev, MKDEV(MAJOR(adclp_ctrl->adclp_dev_t), MINOR(adclp_ctrl->adclp_dev_t)), 1);
    if (ret)
    {
        adclp_err("fail to add adclp%u char device to the system\n", adclp_ctrl->channel);
        goto out3;
    }

    ret = sstar_adclp_sysfs_file(adclp_ctrl);
    if (ret)
        goto out4;

    platform_set_drvdata(pdev, adclp_ctrl);

    indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(struct iio_control));
    if (!indio_dev)
    {
        adclp_err("fail to allocate an iio device for adclp%u\n", adclp_ctrl->channel);
        goto out5;
    }

    adclp_ctrl->iio_ctrl                                    = iio_priv(indio_dev);
    adclp_ctrl->iio_ctrl->info.read_raw                     = sstar_adclp_iio_read;
    adclp_ctrl->iio_ctrl->channels.type                     = IIO_VOLTAGE;
    adclp_ctrl->iio_ctrl->channels.channel                  = adclp_ctrl->adclp_dev.channel;
    adclp_ctrl->iio_ctrl->channels.info_mask_separate       = BIT(IIO_CHAN_INFO_RAW);
    adclp_ctrl->iio_ctrl->channels.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE);

    indio_dev->dev.parent   = &pdev->dev;
    indio_dev->modes        = INDIO_DIRECT_MODE;
    indio_dev->info         = &adclp_ctrl->iio_ctrl->info;
    indio_dev->channels     = &adclp_ctrl->iio_ctrl->channels;
    indio_dev->num_channels = 1;
    devm_iio_device_register(&pdev->dev, indio_dev);

    adclp_ctrl->int_en = of_property_read_bool(pdev->dev.of_node, "interrupt-enable");
    if (adclp_ctrl->int_en)
    {
        adclp_ctrl->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
        if (!adclp_ctrl->irq)
        {
            adclp_err("fail to parse and map adclp%u interrupt into linux virq space\n", adclp_ctrl->channel);
            goto out5;
        }

        init_completion(&adclp_ctrl->adc_done);
        adclp_ctrl->workrun       = 1;
        adclp_ctrl->adc_workqueue = create_workqueue("adclp_workqueue");
        INIT_WORK(&adclp_ctrl->adc_work, sstar_adclp_sample_workquene);
        queue_work(adclp_ctrl->adc_workqueue, &adclp_ctrl->adc_work);

        snprintf(adclp_ctrl->irq_name, sizeof(adclp_ctrl->irq_name), "adclp-%hhu", adclp_ctrl->channel);
        ret = devm_request_irq(&pdev->dev, adclp_ctrl->irq, sstar_adclp_int_handler, IRQF_SHARED,
                               (const char *)adclp_ctrl->irq_name, adclp_ctrl);
        if (ret < 0)
        {
            adclp_err("fail to request adclp%u irq\n", adclp_ctrl->channel);
            goto out6;
        }
    }

    INIT_LIST_HEAD(&adclp_ctrl->channel_node);
    list_add(&adclp_ctrl->channel_node, &adclp_channel_list);
    hal_adclp_init(&adclp_ctrl->adclp_dev);

    return 0;

out6:
    adclp_ctrl->workrun = 0;
    complete(&adclp_ctrl->adc_done);
    destroy_workqueue(adclp_ctrl->adc_workqueue);
out5:
    device_destroy(msys_get_sysfs_class(), adclp_ctrl->adclp_dev_t);
out4:
    cdev_del(&adclp_ctrl->adclp_cdev);
out3:
    unregister_chrdev_region(adclp_ctrl->adclp_dev_t, 1);
out2:
    sstar_adclp_clock_release(adclp_ctrl);
out1:
    return ret;
}

static int sstar_adclp_remove(struct platform_device *pdev)
{
    struct adclp_control *pos        = NULL;
    struct adclp_control *tmp        = NULL;
    struct adclp_control *adclp_ctrl = platform_get_drvdata(pdev);

    hal_adclp_deinit(&adclp_ctrl->adclp_dev);
    list_for_each_entry_safe(pos, tmp, &adclp_channel_list, channel_node)
    {
        list_del(&pos->channel_node);
    }

    if (adclp_ctrl->int_en)
    {
        adclp_ctrl->workrun = 0;
        complete(&adclp_ctrl->adc_done);
        destroy_workqueue(adclp_ctrl->adc_workqueue);
    }

    device_destroy(msys_get_sysfs_class(), adclp_ctrl->adclp_dev_t);
    cdev_del(&adclp_ctrl->adclp_cdev);
    unregister_chrdev_region(adclp_ctrl->adclp_dev_t, 1);
    if (adclp_ctrl->clock_en)
    {
        clk_disable_unprepare(adclp_ctrl->src_clk);
    }
    sstar_adclp_clock_release(adclp_ctrl);

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_adclp_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static int sstar_adclp_resume(struct platform_device *pdev)
{
    struct adclp_control *adclp_ctrl = platform_get_drvdata(pdev);

    hal_adclp_init(&adclp_ctrl->adclp_dev);

    return 0;
}
#endif /* CONFIG_PM_SLEEP */

int sstar_adclp_enable(u8 channel, u8 enable)
{
    int                   ret = 0;
    struct adclp_control *pos = NULL;

    list_for_each_entry(pos, &adclp_channel_list, channel_node)
    {
        if (pos->channel == channel)
        {
            if (enable && !pos->clock_en)
            {
                ret = clk_prepare_enable(pos->src_clk);
                if (ret)
                {
                    adclp_err("fail to enable adclp%u clock\n", pos->channel);
                    return ret;
                }
                pos->clock_en++;
                pos->en_flag = 1;
                return 0;
            }
            else if (!enable && pos->clock_en)
            {
                clk_disable_unprepare(pos->src_clk);
                pos->clock_en--;
                pos->en_flag = 0;
                return 0;
            }
            else
            {
                return 0;
            }
        }
    }

    adclp_err("unsupported channel%u\n", channel);
    return -EINVAL;
}
EXPORT_SYMBOL(sstar_adclp_enable);

int sstar_adclp_get_data(u8 channel, u16 *data)
{
    struct adclp_control *pos = NULL;

    list_for_each_entry(pos, &adclp_channel_list, channel_node)
    {
        if (pos->channel == channel)
        {
            *data = hal_adclp_get_data(&pos->adclp_dev);
            return 0;
        }
    }

    adclp_err("unsupported channel%u\n", channel);
    return -EINVAL;
}
EXPORT_SYMBOL(sstar_adclp_get_data);

int sstar_adclp_vdd_power(enum adclp_vdd_type type)
{
    u16                   value   = 0;
    u32                   channel = 0;
    struct adclp_control *pos     = NULL;

    switch (type)
    {
        case ADCLP_VDD_CPU:
            channel = HAL_ADCLP_VDD_CPU_CH;
            break;

        case ADCLP_VDD_IPU:
            channel = HAL_ADCLP_VDD_IPU_CH;
            break;

        default:
            adclp_err("unsupported vdd power type\n");
            return -EINVAL;
    }

    list_for_each_entry(pos, &adclp_channel_list, channel_node)
    {
        if (pos->channel == channel)
        {
            hal_adclp_muxsel_enbale(&pos->adclp_dev, 1);
            switch (type)
            {
                case ADCLP_VDD_CPU:
                    hal_adclp_vdd_cpu_enable(&pos->adclp_dev);
                    break;

                case ADCLP_VDD_IPU:
                    hal_adclp_vdd_ipu_enable(&pos->adclp_dev);
                    break;
            }
            value = hal_adclp_get_data(&pos->adclp_dev);
            hal_adclp_muxsel_enbale(&pos->adclp_dev, 0);
            return value;
        }
    }

    adclp_err("unsupported channel%u\n", channel);
    return -EINVAL;
}
EXPORT_SYMBOL(sstar_adclp_vdd_power);

int sstar_adclp_set_bound(u8 channel, u16 max, u16 min)
{
    struct adclp_control *pos = NULL;

    list_for_each_entry(pos, &adclp_channel_list, channel_node)
    {
        if (pos->channel == channel)
        {
            hal_adclp_set_bound(&pos->adclp_dev, max, min);
            return 0;
        }
    }

    adclp_err("unsupported channel%u\n", channel);
    return -EINVAL;
}
EXPORT_SYMBOL(sstar_adclp_set_bound);

int sstar_adclp_register_callback(u8 channel, adclp_cb_t cb_t)
{
    struct adclp_cb *cb_pos = NULL;
    struct adclp_cb *cb_hub = NULL;

    list_for_each_entry(cb_pos, &adclp_callback_list, cb_node)
    {
        if ((cb_pos->cb_t == cb_t) && (cb_pos->channel == channel))
        {
            adclp_err("adclp callback function has been registered\n");
            return -EINVAL;
        }
    }

    cb_hub = kzalloc(sizeof(struct adclp_cb), GFP_KERNEL);
    INIT_LIST_HEAD(&cb_hub->cb_node);
    list_add(&cb_hub->cb_node, &adclp_callback_list);
    cb_hub->channel = channel;
    cb_hub->cb_t    = cb_t;

    return 0;
}
EXPORT_SYMBOL(sstar_adclp_register_callback);

int sstar_adclp_unregister_callback(u8 channel, adclp_cb_t cb_t)
{
    struct adclp_cb *cb_pos = NULL;

    list_for_each_entry(cb_pos, &adclp_callback_list, cb_node)
    {
        if ((cb_pos->cb_t == cb_t) && (cb_pos->channel == channel))
        {
            list_del(&cb_pos->cb_node);
            cb_pos->cb_t = NULL;
            kfree(cb_pos);
            return 0;
        }
    }

    adclp_err("adclp callback function deregistration failed\n");
    return -EINVAL;
}
EXPORT_SYMBOL(sstar_adclp_unregister_callback);

static const struct of_device_id sstar_adclp_of_match_table[] = {{.compatible = "sstar,adclp"}, {}};
MODULE_DEVICE_TABLE(of, sstar_adclp_of_match_table);

static struct platform_driver sstar_adclp_driver = {
    .probe  = sstar_adclp_probe,
    .remove = sstar_adclp_remove,
#ifdef CONFIG_PM_SLEEP
    .suspend = sstar_adclp_suspend,
    .resume  = sstar_adclp_resume,
#endif
    .driver =
        {
            .owner          = THIS_MODULE,
            .name           = "sstar-adclp",
            .of_match_table = sstar_adclp_of_match_table,
        },
};

module_platform_driver(sstar_adclp_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sstar ADC Low Precision Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sstar-adclp");
