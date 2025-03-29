/*
 * drv_riu_dbg.c- Sigmastar
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

#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/dma-mapping.h>

#include <registers.h>
#include <ms_msys.h>
#include <hal_riu_dbg.h>
#include "drv_riu_dbg.h"
#include "cam_os_wrapper.h"
#include "ms_platform.h"

#define RIU_BIN_ATTR_MAX_SIZE 0x100000

enum action_t
{
    ACTION_NONE   = 0x00,
    ACTION_PRINT  = 0x01,
    ACTION_BUG_ON = 0x02,
};

struct timeout_t
{
    u8 action;
};

struct record_t
{
    u8                  action;
    bool                enable;
    bool                dirty;
    bool                setup;
    u32                 setup_size;
    struct riu_irq_info irq_info;
};

struct riu_dbg_t
{
    struct cdev           cdev;
    struct fasync_struct *async;
    struct mutex          mutex;
    struct clk *          clock;

    struct timeout_t     timeout;
    struct record_t      record;
    struct hal_riu_dbg_t hal;
};

#if defined(CONFIG_SSTAR_RIU_TIMEOUT) || defined(CONFIG_SSTAR_RIU_RECORDER)
static struct riu_dbg_t riu_dbg;
#endif

#ifdef CONFIG_SSTAR_RIU_TIMEOUT
static irqreturn_t sstar_riu_timeout_interrupt(int irq, void *dev_id)
{
    u64                           base = *((u64 *)dev_id);
    struct riu_timeout_snapshot_t snapshot;

    hal_riu_timeout_get_snapshot(base, &snapshot);

    if (riu_dbg.timeout.action & ACTION_PRINT)
    {
        pr_alert("xiu timeout at bank 0x%04X offset 0x%02X\n", snapshot.bank, snapshot.offset);
    }
    if (riu_dbg.timeout.action & ACTION_BUG_ON)
    {
        BUG();
    }

    return IRQ_HANDLED;
}
#endif

#ifdef CONFIG_SSTAR_RIU_RECORDER
static irqreturn_t sstar_riu_record_interrupt(int irq, void *dev_id)
{
    char                         bridge[10]  = {0};
    char                         capture[10] = {0};
    struct riu_record_snapshot_t snapshot;

    hal_riu_record_interrupt(&riu_dbg.hal);
    hal_riu_record_get_snapshot(&riu_dbg.hal, &snapshot);
    if (snapshot.irq_type == RIU_RECORD_IRQ)
    {
        if (riu_dbg.record.action & ACTION_PRINT)
        {
            if (snapshot.bridge >= HAL_RIU_BRIDGE_NUM)
            {
                snprintf(bridge, sizeof(bridge), "[unknow]");
            }
            else
            {
                snprintf(bridge, sizeof(bridge), "%d", snapshot.bridge);
            }

            if (snapshot.capture >= HAL_RIU_CAPTUER_NUM)
            {
                snprintf(capture, sizeof(capture), "[unknow]");
            }
            else
            {
                snprintf(capture, sizeof(capture), "%d", snapshot.capture);
            }
            pr_alert("[riu debug] bridge %s capture %s\n", bridge, capture);
        }

        if (riu_dbg.record.action & ACTION_BUG_ON)
        {
            BUG();
        }
        riu_dbg.record.irq_info.bridge  = snapshot.bridge;
        riu_dbg.record.irq_info.capture = snapshot.capture;
        kill_fasync(&riu_dbg.async, SIGIO, POLL_IN);
    }
    else if (snapshot.irq_type == RIU_RECORD_WDMA_BUSY_IRQ)
    {
        if (riu_dbg.record.action & ACTION_PRINT)
        {
            if (snapshot.bridge >= HAL_RIU_BRIDGE_NUM)
            {
                snprintf(bridge, sizeof(bridge), "[unknow]");
            }
            else
            {
                snprintf(bridge, sizeof(bridge), "%d", snapshot.bridge);
            }

            pr_alert("[riu debug] bridge %s wdma busy\n", bridge);
        }

        if (riu_dbg.record.action & ACTION_BUG_ON)
        {
            BUG();
        }
    }

    return IRQ_HANDLED;
}

static int __init sstar_riu_record_setup(char *str)
{
    char *token = 0;

    while ((token = strsep(&str, ",")))
    {
        if (!strcmp(token, "enable"))
        {
            riu_dbg.record.enable = true;
        }
        else if (sscanf(token, "0x%X", &riu_dbg.record.setup_size) == 1)
        {
            riu_dbg.record.setup = true;
        }
    }

    return 0;
}
__setup("riu_record=", sstar_riu_record_setup);

static bool riu_filter_check(u8 bridge, u16 bank_start, u16 bank_end)
{
    u8 i;

    for (i = 0; i < riu_dbg.hal.record.filt_num[bridge]; i++)
    {
        if ((bank_start == riu_dbg.hal.record.filt_start[bridge][i])
            && (bank_end == riu_dbg.hal.record.filt_end[bridge][i]))
        {
            return false;
        }
    }

    return true;
}

static int __init sstar_riu_filter_setup(char *str)
{
    int   ret;
    char *token = 0;
    u16   bank_start;
    u16   bank_end;

    while ((token = strsep(&str, ",")))
    {
        sscanf(token, "0x%hx-0x%hx", &bank_start, &bank_end);
        ret = hal_riu_bank_to_bridge(bank_start);
        if ((ret >= 0) && (ret == hal_riu_bank_to_bridge(bank_end)))
        {
            if (riu_dbg.hal.record.filt_num[ret] < HAL_RIU_FILTER_NUM)
            {
                if (riu_filter_check(ret, bank_start, bank_end))
                {
                    riu_dbg.hal.record.filt_start[ret][riu_dbg.hal.record.filt_num[ret]] = bank_start;
                    riu_dbg.hal.record.filt_end[ret][riu_dbg.hal.record.filt_num[ret]]   = bank_end;
                    riu_dbg.hal.record.filt_num[ret]++;
                }
            }
        }
    }

    return 0;
}
__setup("filter=", sstar_riu_filter_setup);

static bool riu_capture_check(u8 bridge, u16 bank, u8 offset, u32 data, u8 mask)
{
    u8 i;

    for (i = 0; i < riu_dbg.hal.record.cap_num[bridge]; i++)
    {
        if ((bank == riu_dbg.hal.record.cap_bank[bridge][i]) && (offset == riu_dbg.hal.record.cap_offset[bridge][i])
            && (data == riu_dbg.hal.record.cap_data[bridge][i]) && (mask == riu_dbg.hal.record.cap_mask[bridge][i]))
        {
            return false;
        }
    }

    return true;
}

static int __init sstar_riu_capture_setup(char *str)
{
    int   ret;
    u16   bank;
    u8    offset;
    u32   data;
    u8    mask;
    char *token = 0;

    while ((token = strsep(&str, ",")))
    {
        sscanf(token, "0x%hx-0x%hhx-0x%x-0x%hhx", &bank, &offset, &data, &mask);
        ret = hal_riu_bank_to_bridge(bank);
        if (ret >= 0)
        {
            if (riu_dbg.hal.record.cap_num[ret] < HAL_RIU_CAPTUER_NUM)
            {
                if (riu_capture_check(ret, bank, offset, data, mask))
                {
                    riu_dbg.hal.record.cap_bank[ret][riu_dbg.hal.record.cap_num[ret]]   = bank;
                    riu_dbg.hal.record.cap_offset[ret][riu_dbg.hal.record.cap_num[ret]] = offset;
                    riu_dbg.hal.record.cap_data[ret][riu_dbg.hal.record.cap_num[ret]]   = data;
                    riu_dbg.hal.record.cap_mask[ret][riu_dbg.hal.record.cap_num[ret]]   = mask;
                    riu_dbg.hal.record.cap_num[ret]++;
                }
            }
        }
    }

    return 0;
}
__setup("capture=", sstar_riu_capture_setup);

static ssize_t filter_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u8 i;
    u8 bridge;
    u8 header_showed = 0;

    char *str = buf;
    char *end = buf + PAGE_SIZE;

    mutex_lock(&riu_dbg.mutex);

    for (bridge = 0; bridge < HAL_RIU_BRIDGE_NUM; bridge++)
    {
        for (i = 0; i < riu_dbg.hal.record.filt_num[bridge]; i++)
        {
            if (!header_showed)
            {
                str += scnprintf(str, end - str, " bridge numb  from   to\n");
                str += scnprintf(str, end - str, " -----------------------\n");
                header_showed = 1;
            }

            str += scnprintf(str, end - str, "  [%1d]   [%1d]   %04x  %04x\n", bridge, i,
                             riu_dbg.hal.record.filt_start[bridge][i], riu_dbg.hal.record.filt_end[bridge][i]);
        }
    }

    mutex_unlock(&riu_dbg.mutex);

    return (str - buf);
}

static ssize_t filter_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int ret;
    u16 bank_start;
    u16 bank_end;

    if (buf != NULL)
    {
        ret = sscanf(buf, "0x%hx 0x%hx", &bank_start, &bank_end);
        if (ret == 2)
        {
            ret = hal_riu_bank_to_bridge(bank_start);
            if ((ret >= 0) && (ret == hal_riu_bank_to_bridge(bank_end)))
            {
                mutex_lock(&riu_dbg.mutex);

                if (riu_dbg.hal.record.filt_num[ret] < HAL_RIU_FILTER_NUM)
                {
                    if (riu_filter_check(ret, bank_start, bank_end))
                    {
                        riu_dbg.hal.record.filt_start[ret][riu_dbg.hal.record.filt_num[ret]] = bank_start;
                        riu_dbg.hal.record.filt_end[ret][riu_dbg.hal.record.filt_num[ret]]   = bank_end;
                        riu_dbg.hal.record.filt_num[ret]++;
                        riu_dbg.record.dirty = true;
                    }
                }

                mutex_unlock(&riu_dbg.mutex);
            }
            return n;
        }
    }

    return -EINVAL;
}

DEVICE_ATTR_RW(filter);

static ssize_t capture_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u8 i;
    u8 bridge;
    u8 header_showed = 0;

    char *str = buf;
    char *end = buf + PAGE_SIZE;

    mutex_lock(&riu_dbg.mutex);

    for (bridge = 0; bridge < HAL_RIU_BRIDGE_NUM; bridge++)
    {
        for (i = 0; i < riu_dbg.hal.record.cap_num[bridge]; i++)
        {
            if (!header_showed)
            {
                str += scnprintf(str, end - str, " bridge numb  bank offset  data   mask\n");
                str += scnprintf(str, end - str, " --------------------------------------\n");
                header_showed = 1;
            }

            str += scnprintf(str, end - str, "  [%1d]   [%1d]   %04x   %02x  %08x  %1x\n", bridge, i,
                             riu_dbg.hal.record.cap_bank[bridge][i], riu_dbg.hal.record.cap_offset[bridge][i],
                             riu_dbg.hal.record.cap_data[bridge][i], riu_dbg.hal.record.cap_mask[bridge][i]);
        }
    }

    mutex_unlock(&riu_dbg.mutex);

    return (str - buf);
}

static ssize_t capture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int ret;
    u16 bank;
    u8  offset;
    u32 data;
    u8  mask;

    if (buf != NULL)
    {
        ret = sscanf(buf, "0x%hx 0x%hhx 0x%x 0x%hhx", &bank, &offset, &data, &mask);
        if (ret == 4)
        {
            ret = hal_riu_bank_to_bridge(bank);
            if (ret >= 0)
            {
                mutex_lock(&riu_dbg.mutex);

                if (riu_dbg.hal.record.cap_num[ret] < HAL_RIU_CAPTUER_NUM)
                {
                    if (riu_capture_check(ret, bank, offset, data, mask))
                    {
                        riu_dbg.hal.record.cap_bank[ret][riu_dbg.hal.record.cap_num[ret]]   = bank;
                        riu_dbg.hal.record.cap_offset[ret][riu_dbg.hal.record.cap_num[ret]] = offset;
                        riu_dbg.hal.record.cap_data[ret][riu_dbg.hal.record.cap_num[ret]]   = data;
                        riu_dbg.hal.record.cap_mask[ret][riu_dbg.hal.record.cap_num[ret]]   = mask;
                        riu_dbg.hal.record.cap_num[ret]++;
                        riu_dbg.record.dirty = true;
                    }
                }

                mutex_unlock(&riu_dbg.mutex);
            }
            return n;
        }
    }

    return -EINVAL;
}

DEVICE_ATTR_RW(capture);

static ssize_t clear_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    u8    i;
    int   ret;
    u8    bridge;
    char *cmd;

    if (buf != NULL)
    {
        cmd = vzalloc(n);
        ret = sscanf(buf, "%s %hhd", cmd, &bridge);
        if (ret == 1)
        {
            if (strcmp(cmd, "all") == 0)
            {
                for (i = 0; i < HAL_RIU_BRIDGE_NUM; i++)
                {
                    mutex_lock(&riu_dbg.mutex);
                    riu_dbg.hal.record.filt_num[i] = 0;
                    riu_dbg.hal.record.cap_num[i]  = 0;
                    riu_dbg.record.dirty           = true;
                    mutex_unlock(&riu_dbg.mutex);
                }
                vfree(cmd);
                return n;
            }
            else if (strcmp(cmd, "filter") == 0)
            {
                for (i = 0; i < HAL_RIU_BRIDGE_NUM; i++)
                {
                    mutex_lock(&riu_dbg.mutex);
                    riu_dbg.hal.record.filt_num[i] = 0;
                    riu_dbg.record.dirty           = true;
                    mutex_unlock(&riu_dbg.mutex);
                }
                vfree(cmd);
                return n;
            }
            else if (strcmp(cmd, "capture") == 0)
            {
                for (i = 0; i < HAL_RIU_BRIDGE_NUM; i++)
                {
                    mutex_lock(&riu_dbg.mutex);
                    riu_dbg.hal.record.cap_num[i] = 0;
                    riu_dbg.record.dirty          = true;
                    mutex_unlock(&riu_dbg.mutex);
                }
                vfree(cmd);
                return n;
            }
        }
        else if (ret == 2)
        {
            if (bridge < HAL_RIU_BRIDGE_NUM)
            {
                if (strcmp(cmd, "filter") == 0)
                {
                    mutex_lock(&riu_dbg.mutex);
                    riu_dbg.hal.record.filt_num[bridge] = 0;
                    riu_dbg.record.dirty                = true;
                    mutex_unlock(&riu_dbg.mutex);
                    vfree(cmd);
                    return n;
                }
                else if (strcmp(cmd, "capture") == 0)
                {
                    mutex_lock(&riu_dbg.mutex);
                    riu_dbg.hal.record.cap_num[bridge] = 0;
                    riu_dbg.record.dirty               = true;
                    mutex_unlock(&riu_dbg.mutex);
                    vfree(cmd);
                    return n;
                }
            }
        }
        vfree(cmd);
    }

    return -EINVAL;
}

DEVICE_ATTR_WO(clear);

static ssize_t state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    mutex_lock(&riu_dbg.mutex);

    if (riu_dbg.record.enable)
    {
        if (riu_dbg.record.dirty)
        {
            str += scnprintf(str, end - str, " enable (dity)\n");
        }
        else
        {
            str += scnprintf(str, end - str, " enable\n");
        }
    }
    else
    {
        str += scnprintf(str, end - str, " disable\n");
    }

    mutex_unlock(&riu_dbg.mutex);

    return (str - buf);
}

static ssize_t state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int ret;
    u8  enable;

    if (buf != NULL)
    {
        ret = sscanf(buf, "%hhd", &enable);
        if (ret == 1)
        {
            mutex_lock(&riu_dbg.mutex);

            if (enable == 1)
            {
                if (riu_dbg.record.enable)
                {
                    if (riu_dbg.record.dirty)
                    {
                        hal_riu_record_enable(&riu_dbg.hal, false);
                        hal_riu_record_init(&riu_dbg.hal);
                        hal_riu_record_enable(&riu_dbg.hal, true);
                        riu_dbg.record.dirty = false;
                    }
                }
                else
                {
                    hal_riu_record_init(&riu_dbg.hal);
                    hal_riu_record_enable(&riu_dbg.hal, true);
                    riu_dbg.record.dirty  = false;
                    riu_dbg.record.enable = true;
                }
            }
            else if (enable == 0)
            {
                hal_riu_record_enable(&riu_dbg.hal, false);
                riu_dbg.record.enable = false;
            }

            mutex_unlock(&riu_dbg.mutex);
            return n;
        }
    }

    return -EINVAL;
}

DEVICE_ATTR_RW(state);

static ssize_t record_read(struct file *filp, struct kobject *kobj, struct bin_attribute *attr, char *buf,
                           loff_t offset, size_t n)
{
    char *data;

    if (offset >= riu_dbg.hal.record.addr_size)
        return 0;
    else if ((offset + n) > riu_dbg.hal.record.addr_size)
        n = riu_dbg.hal.record.addr_size - offset;

    mutex_lock(&riu_dbg.mutex);

    data = riu_dbg.hal.record.addr_base_virt + offset;
    Chip_Inv_Cache_Range(data, n);
    memcpy(buf, data, n);

    mutex_unlock(&riu_dbg.mutex);

    return n;
}

BIN_ATTR_RO(record, RIU_BIN_ATTR_MAX_SIZE);

ssize_t sstar_riu_dev_read(struct file *file, char __user *buf, size_t n, loff_t *offset)
{
    char *data;

    mutex_lock(&riu_dbg.mutex);

    data = riu_dbg.hal.record.addr_base_virt + *offset;
    Chip_Inv_Cache_Range(data, n);

    mutex_unlock(&riu_dbg.mutex);

    return copy_to_user(buf, data, n);
}

static long sstar_riu_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int          ret  = 0;
    void __user *argp = (void __user *)arg;

    mutex_lock(&riu_dbg.mutex);

    switch (cmd)
    {
        case RIU_GET_IRQ_INFO:
            ret = copy_to_user(argp, &riu_dbg.record.irq_info, sizeof(struct riu_irq_info)) ? -EFAULT : 0;
            break;
    }

    mutex_unlock(&riu_dbg.mutex);

    return ret;
}

static int sstar_riu_dev_fasync(int fd, struct file *file, int on)
{
    return fasync_helper(fd, file, on, &riu_dbg.async);
}

static struct file_operations sstar_riu_ops = {
    .owner          = THIS_MODULE,
    .read           = sstar_riu_dev_read,
    .unlocked_ioctl = sstar_riu_dev_ioctl,
    .fasync         = sstar_riu_dev_fasync,
};

static int riu_node_create(void)
{
    int            ret;
    dev_t          dev;
    struct device *device;

    if (0 != (ret = alloc_chrdev_region(&dev, 0, 1, "riu")))
    {
        return ret;
    }

    cdev_init(&riu_dbg.cdev, &sstar_riu_ops);
    ret = cdev_add(&riu_dbg.cdev, dev, 1);
    if (ret)
    {
        return ret;
    }

    device = device_create(msys_get_sysfs_class(), NULL, dev, NULL, "riu");
    if (IS_ERR(device))
    {
        return -ENODEV;
    }

    ret = device_create_file(device, &dev_attr_filter);
    if (ret)
    {
        return ret;
    }
    ret = device_create_file(device, &dev_attr_capture);
    if (ret)
    {
        return ret;
    }
    ret = device_create_file(device, &dev_attr_clear);
    if (ret)
    {
        return ret;
    }
    ret = device_create_file(device, &dev_attr_state);
    if (ret)
    {
        return ret;
    }
    ret = device_create_bin_file(device, &bin_attr_record);
    if (ret)
    {
        return ret;
    }

    return 0;
}
#endif

static int sstar_riu_probe(struct platform_device *pdev)
{
#if defined(CONFIG_SSTAR_RIU_TIMEOUT) || defined(CONFIG_SSTAR_RIU_RECORDER)
    int ret = 0;
#ifdef CONFIG_SSTAR_RIU_RECORDER
    char *irq_record = 0;
#endif
#ifdef CONFIG_SSTAR_RIU_TIMEOUT
    char **irq_timeout = 0;
#endif

    struct device_node *child;
    struct device_node *parent = pdev->dev.of_node;

    for_each_available_child_of_node(parent, child)
    {
        u32             i;
        u32             num_reg = 0;
        struct resource res;

#ifdef CONFIG_SSTAR_RIU_TIMEOUT
        if (strcmp(child->name, "timeout") == 0)
        {
            while (of_address_to_resource(child, num_reg, &res) == 0)
                num_reg++;

            riu_dbg.hal.timeout.num_bank = num_reg;
            riu_dbg.hal.timeout.banks    = devm_kzalloc(&pdev->dev, num_reg * sizeof(u64), GFP_KERNEL);

            irq_timeout = devm_kzalloc(&pdev->dev, num_reg * sizeof(char *), GFP_KERNEL);

            for (i = 0; i < num_reg; i++)
            {
                ret = of_address_to_resource(child, i, &res);
                if (ret == 0)
                {
                    riu_dbg.hal.timeout.banks[i] = (u64)IO_ADDRESS(res.start);
                }
                else
                {
                    return ret;
                }

                ret = of_irq_get(child, i);
                if (ret <= 0)
                {
                    return ret;
                }

                irq_timeout[i] = devm_kzalloc(&pdev->dev, 32 * sizeof(char), GFP_KERNEL);
                snprintf(irq_timeout[i], 32 * sizeof(char), "riu timeout %d", i);

                ret = request_irq(ret, sstar_riu_timeout_interrupt, IRQF_TRIGGER_HIGH, irq_timeout[i],
                                  &riu_dbg.hal.timeout.banks[i]);
                if (ret)
                {
                    return ret;
                }
            }

            if (of_property_read_bool(child, "print-on"))
            {
                riu_dbg.timeout.action = ACTION_PRINT;
            }

            if (of_property_read_bool(child, "bug-on"))
            {
                riu_dbg.timeout.action |= ACTION_BUG_ON;
            }

            hal_riu_timeout_enable(&riu_dbg.hal);
        }
#endif

#ifdef CONFIG_SSTAR_RIU_RECORDER
        if (strcmp(child->name, "recorder") == 0)
        {
            dma_addr_t dma_addr;

            while (of_address_to_resource(child, num_reg, &res) == 0)
                num_reg++;

            riu_dbg.hal.record.num_bank = num_reg;
            riu_dbg.hal.record.banks    = devm_kzalloc(&pdev->dev, num_reg * sizeof(u64), GFP_KERNEL);

            irq_record = devm_kzalloc(&pdev->dev, 32 * sizeof(char), GFP_KERNEL);
            for (i = 0; i < num_reg; i++)
            {
                ret = of_address_to_resource(child, i, &res);
                if (ret == 0)
                {
                    riu_dbg.hal.record.banks[i] = (u64)IO_ADDRESS(res.start);
                }
                else
                {
                    return ret;
                }
            }

            ret = of_irq_get(child, 0);
            if (ret <= 0)
            {
                return ret;
            }

            snprintf(irq_record, 32 * sizeof(char), "riu record");
            ret = devm_request_irq(&pdev->dev, ret, sstar_riu_record_interrupt, IRQF_TRIGGER_HIGH, irq_record, NULL);
            if (ret)
            {
                goto record_err;
            }

            if (riu_dbg.record.setup)
            {
                riu_dbg.hal.record.addr_size = riu_dbg.record.setup_size;
            }
            else
            {
                riu_dbg.hal.record.addr_size = RIU_BIN_ATTR_MAX_SIZE;
            }
            riu_dbg.hal.record.addr_base_virt =
                dma_alloc_coherent(&pdev->dev, riu_dbg.hal.record.addr_size, &dma_addr, GFP_KERNEL);
            if (!riu_dbg.hal.record.addr_base_virt)
            {
                pr_err("riu recorder: alloc failed\r\n");
                ret = -ENOMEM;
                goto record_err;
            }
            riu_dbg.hal.record.addr_base = (u32)(dma_addr - MIU0_BASE);
            // print out the MIU address of RIU recorder for user convenience
            dev_info(&pdev->dev, "recorder 0x%X@0x%X\r\n", riu_dbg.hal.record.addr_size, riu_dbg.hal.record.addr_base);

            if (riu_dbg.hal.record.addr_size > RIU_BIN_ATTR_MAX_SIZE)
            {
                pr_warn("memory for riu recorder is too large limit to 0x%x\n", RIU_BIN_ATTR_MAX_SIZE);
                riu_dbg.hal.record.addr_size = RIU_BIN_ATTR_MAX_SIZE;
            }

            if (of_property_read_bool(child, "print-on"))
            {
                riu_dbg.record.action = ACTION_PRINT;
            }

            if (of_property_read_bool(child, "bug-on"))
            {
                riu_dbg.record.action |= ACTION_BUG_ON;
            }

            riu_dbg.clock = of_clk_get(child, 0);
            if (IS_ERR(riu_dbg.clock))
            {
                return PTR_ERR(riu_dbg.clock);
            }
            ret = clk_prepare_enable(riu_dbg.clock);
            if (ret)
            {
                goto record_err;
            }

            hal_riu_record_init(&riu_dbg.hal);

            if (riu_dbg.record.enable)
            {
                hal_riu_record_enable(&riu_dbg.hal, true);
            }

            ret = riu_node_create();
        record_err:
            if (ret)
            {
                if (riu_dbg.hal.record.addr_base_virt)
                {
                    dma_free_coherent(&pdev->dev, riu_dbg.hal.record.addr_size, riu_dbg.hal.record.addr_base_virt,
                                      dma_addr);
                }
                if (!IS_ERR(riu_dbg.clock))
                {
                    clk_disable_unprepare(riu_dbg.clock);
                }
                if (riu_dbg.record.enable)
                {
                    hal_riu_record_enable(&riu_dbg.hal, false);
                    riu_dbg.record.enable = false;
                }
                return ret;
            }
        }
#endif
    }
#endif

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_riu_suspend(struct device *dev)
{
#ifdef CONFIG_SSTAR_RIU_RECORDER
    clk_disable_unprepare(riu_dbg.clock);
#endif
    return 0;
}

static int sstar_riu_resume(struct device *dev)
{
#ifdef CONFIG_SSTAR_RIU_RECORDER
    int ret;

    ret = clk_prepare_enable(riu_dbg.clock);
    if (ret)
    {
        return ret;
    }
#endif
#ifdef CONFIG_SSTAR_RIU_TIMEOUT
    hal_riu_timeout_enable(&riu_dbg.hal);
#endif
#ifdef CONFIG_SSTAR_RIU_RECORDER
    if (riu_dbg.record.enable)
    {
        hal_riu_record_init(&riu_dbg.hal);
        hal_riu_record_enable(&riu_dbg.hal, true);
        riu_dbg.record.dirty = false;
    }
#endif
    return 0;
}
#else
#define sstar_riu_suspend NULL
#define sstar_riu_resume  NULL
#endif

static const struct of_device_id riu_of_device_id[] = {{
                                                           .compatible = "sstar,riu",
                                                       },
                                                       {}};
MODULE_DEVICE_TABLE(of, riu_of_device_id);

static const struct dev_pm_ops riu_dev_pm_ops = {
    .suspend_noirq = sstar_riu_suspend,
    .resume_noirq  = sstar_riu_resume,
};

static struct platform_driver riu_platform_driver = {
    .driver =
        {
            .name           = "sstar,riu",
            .owner          = THIS_MODULE,
            .pm             = &riu_dev_pm_ops,
            .of_match_table = riu_of_device_id,
        },
    .probe = sstar_riu_probe,
};

static int __init sstar_riu_driver_init(void)
{
    return platform_driver_register(&riu_platform_driver);
}

core_initcall(sstar_riu_driver_init);
