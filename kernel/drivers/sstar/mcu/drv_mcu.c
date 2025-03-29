/*
 * drv_mcu.c- Sigmastar
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
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include <gpio.h>
#include <ms_msys.h>
#include <ms_platform.h>
#include <drv_gpio.h>
#include <registers.h>

#define IR_MODE_RC5 0
#define IR_MODE_NEC 1

/*
 * define wakeup source
 */
enum
{
    NON_WAKEUP = 0,
    NEC_WAKEUP,
    RC5_WAKEUP,
    GPIO_WAKEUP_START,
};

#define IR_KEY_NR CONFIG_IR_KEY_NR
#define GPIO_SIZE (ALIGN(PAD_PM_NR, 8) / 8)

struct mcu_cfg_t
{
    unsigned long base;
    u8            ir_mode;
    u8            ir_key[IR_KEY_NR];
    u8            gpio_enable[GPIO_SIZE];
};

static const char *mcu_wakeup_source[GPIO_WAKEUP_START] = {[NEC_WAKEUP] = "ir_nec", [RC5_WAKEUP] = "ir_rc5"};

static struct mcu_cfg_t mcu_cfg;

void mcu_write_config(void)
{
    u8            i;
    void __iomem *base = (void __iomem *)mcu_cfg.base;

    writeb(mcu_cfg.ir_mode, base++);
    for (i = 0; i < IR_KEY_NR; i++)
    {
        writeb(mcu_cfg.ir_key[i], base++);
        if (!((unsigned long)base % 2))
        {
            base += 2;
        }
    }
    for (i = 0; i < GPIO_SIZE; i++)
    {
        writeb(mcu_cfg.gpio_enable[i], base++);
        if (!((unsigned long)base % 2))
        {
            base += 2;
        }
    }
}
EXPORT_SYMBOL(mcu_write_config);

u8 mcu_get_wakeup(void)
{
    void __iomem *base = (void __iomem *)mcu_cfg.base;
    base += ((IR_KEY_NR + 1 + GPIO_SIZE) / 2) * 4 + ((IR_KEY_NR + 1 + GPIO_SIZE) % 2);
    return readb(base);
}
EXPORT_SYMBOL(mcu_get_wakeup);

const char *mcu_get_wakeup_name(void)
{
    const U8 *name;
    u8        index = mcu_get_wakeup();

    if (index < GPIO_WAKEUP_START)
    {
        return mcu_wakeup_source[index];
    }
    else
    {
        sstar_gpio_num_to_name(index - GPIO_WAKEUP_START + PAD_PM_START, &name);
        return name;
    }
}

static u8 parse_mcu_ir_mode(const char *mode)
{
    if (!strcmp(mode, "nec"))
    {
        return IR_MODE_NEC;
    }
    else if (!strcmp(mode, "rc5"))
    {
        return IR_MODE_RC5;
    }
    return IR_MODE_RC5;
}

static ssize_t wakeup_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    const char *src;
    char *      str = buf;
    char *      end = buf + PAGE_SIZE;

    src = mcu_get_wakeup_name();

    str += scnprintf(str, end - str, "wakeup event: %s\n", src);
    return (str - buf);
}
static DEVICE_ATTR(wakeup_event, 0444, wakeup_event_show, NULL);

static int sstar_mcu_probe(struct platform_device *pdev)
{
    u8    i;
    int   ret;
    dev_t dev;

    const char *     mode;
    struct device *  mcu_dev;
    struct resource *resource;
    u32              key[IR_KEY_NR]  = {0};
    u32              gpio[PAD_PM_NR] = {0};

    resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!resource)
    {
        return -ENOENT;
    }
    mcu_cfg.base = (unsigned long)(IO_ADDRESS(resource->start));

    ret = of_property_read_string(pdev->dev.of_node, "ir-mode", &mode);
    if (!ret)
    {
        mcu_cfg.ir_mode = parse_mcu_ir_mode(mode);
    }
    else
    {
        mcu_cfg.ir_mode = 0;
    }

    ret = of_property_count_elems_of_size(pdev->dev.of_node, "ir-key-table", sizeof(u32));
    if (ret > 0)
    {
        if (ret > IR_KEY_NR)
        {
            printk("number of ir key is greater than max number parse %d only\n", IR_KEY_NR);
            ret = IR_KEY_NR;
        }

        if (!of_property_read_u32_array(pdev->dev.of_node, "ir-key-table", key, ret))
        {
            for (i = 0; i < ret; i++)
            {
                if (key[i] <= U8_MAX)
                {
                    mcu_cfg.ir_key[i] = (u8)key[i];
                }
                else
                {
                    printk("abandon illegal ir key %d\n", key[i]);
                }
            }
        }
    }

    ret = of_property_count_elems_of_size(pdev->dev.of_node, "gpio-table", sizeof(u32));
    if (ret > 0)
    {
        if (ret > PAD_PM_NR)
        {
            printk("number of gpio is greater than max number parse %d only\n", PAD_PM_NR);
            ret = PAD_PM_NR;
        }

        if (!of_property_read_u32_array(pdev->dev.of_node, "gpio-table", gpio, ret))
        {
            for (i = 0; i < ret; i++)
            {
                if (gpio[i] >= PAD_PM_START && gpio[i] <= PAD_PM_END)
                {
                    mcu_cfg.gpio_enable[(gpio[i] - PAD_PM_START) / 8] |= (1 << ((gpio[i] - PAD_PM_START) % 8));
                }
                else
                {
                    printk("abandon illegal gpio index %d\n", gpio[i]);
                }
            }
        }
    }

    if (0 != (ret = alloc_chrdev_region(&dev, 0, 1, "mcu")))
        return ret;

    mcu_dev = device_create(msys_get_sysfs_class(), NULL, dev, NULL, "mcu");
    if (IS_ERR(mcu_dev))
        return -ENODEV;

    device_create_file(mcu_dev, &dev_attr_wakeup_event);

    return 0;
}

static int sstar_mcu_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id sstar_mcu_match[] = {{
                                                          .compatible = "sstar,mcu",
                                                      },
                                                      {}};
MODULE_DEVICE_TABLE(of, sstar_mcu_match);

static struct platform_driver sstar_mcu_driver = {
    .driver =
        {
            .name           = "mcu",
            .owner          = THIS_MODULE,
            .of_match_table = sstar_mcu_match,
        },
    .probe  = sstar_mcu_probe,
    .remove = sstar_mcu_remove,
};
module_platform_driver(sstar_mcu_driver);

MODULE_DESCRIPTION("MCU configure driver for SigmaStar");
MODULE_LICENSE("GPL v2");
