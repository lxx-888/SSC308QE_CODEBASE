/*
 * hal_measure_clk.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <ms_platform.h>
#include <registers.h>
#include <ms_msys.h>

enum measure_clk_dev
{
    MEASURE_CLK_TEST = 0,
    MEASURE_CLK_MIU,
    MEASURE_CLK_MCU,
    MEASURE_CLK_IMI,
    MEASURE_CLK_RISC,
    MEASURE_CLK_PCIE0,
    MEASURE_CLK_PCIE1,
    MEASURE_CLK_SPI,
    MEASURE_CLK_NUM,
};

static int measure_clk_cm4(char *buf)
{
    char *str       = buf;
    char *end       = buf + PAGE_SIZE;
    u16   reg_value = 0;
    u32   freq      = 0;

    // choose source of clk_mcu_pm
    OUTREG8(BASE_REG_RIU_PA + (0x000e6c << 1), 0x02);
    OUTREG8(BASE_REG_RIU_PA + (0x000e6c << 1) + 1, 0x80);

    // test bus enable (add for power saving)
    OUTREG8(BASE_REG_RIU_PA + (0x10221a << 1) + 1, 0x80);
    udelay(1000);

    if ((INREG8(BASE_REG_RIU_PA + (0x102216 << 1)) & BIT0) != BIT0)
    {
        OUTREG8(BASE_REG_RIU_PA + (0x102216 << 1), 0x01);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEE << 1), 0x09);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEA << 1), 0x06);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEA << 1) + 1, 0x40);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEC << 1), 0x01);
        OUTREG8(BASE_REG_RIU_PA + (0x101EE0 << 1) + 1, 0x00);
        OUTREG8(BASE_REG_RIU_PA + (0x101EE0 << 1) + 1, 0x80);
        udelay(1000);
    }
    reg_value = (INREG8(BASE_REG_RIU_PA + (0x101EE2 << 1)) | (INREG8(BASE_REG_RIU_PA + (0x101EE2 << 1) + 1) << 8));
    freq      = reg_value * 12000;

    str += scnprintf(str, end - str, "CM4   CLK rate = %d Hz \n", freq);

    // disable testout_sel
    OUTREG8(BASE_REG_RIU_PA + (0x102216 << 1), 0x00);

    // test bus disable (add for power saving)
    OUTREG8(BASE_REG_RIU_PA + (0x10221a << 1) + 1, 0x00);

    return (int)(str - buf);
}

static int measure_clk_function(enum measure_clk_dev dev, char *buf)
{
    u32     calc_cnt  = 0;
    u32     speed     = 0;
    ktime_t timeout   = 5; // 5 msec
    ktime_t starttime = 0;

    if (dev >= MEASURE_CLK_NUM)
        return 0;

    OUTREGMSK16(BASE_REG_L3BRIDGE + REG_ID_7C, 0x0001, 0x0001);

    OUTREGMSK16(BASE_REG_L3BRIDGE + REG_ID_7F, 0x2000, 0xFF00);

    starttime = ktime_get();

    while (ktime_ms_delta(ktime_get(), starttime) < timeout)
    {
        if ((INREG16(BASE_REG_L3BRIDGE + REG_ID_7F) & 0xFF00) == 0x0000)
            break;
    }

    if ((INREG16(BASE_REG_L3BRIDGE + REG_ID_7F) & 0xFF00) != 0x0000)
    {
        pr_err("measure clock clear timeout!!!\n");
        print_hex_dump(KERN_CONT, "l3brodge:", DUMP_PREFIX_OFFSET, 32, 4,
                       (const void *)(unsigned long)IO_ADDRESS(BASE_REG_L3BRIDGE), 0x80 << 2, false);
        return 0;
    }

    OUTREGMSK16(BASE_REG_L3BRIDGE + REG_ID_7F, dev, 0x0007);

    OUTREGMSK16(BASE_REG_L3BRIDGE + REG_ID_7F, 0x4000, 0xFF00);

    starttime = ktime_get();

    while (ktime_ms_delta(ktime_get(), starttime) < timeout)
    {
        if ((INREG16(BASE_REG_L3BRIDGE + REG_ID_7F) & 0xFF00) == 0x8000)
            break;
    }

    if ((INREG16(BASE_REG_L3BRIDGE + REG_ID_7F) & 0xFF00) != 0x8000)
    {
        pr_err("measure clock timeout!!!\n");
        print_hex_dump(KERN_CONT, "l3brodge:", DUMP_PREFIX_OFFSET, 32, 4,
                       (const void *)(unsigned long)IO_ADDRESS(BASE_REG_L3BRIDGE), 0x80 << 2, false);
        return 0;
    }

    calc_cnt = INREG16(BASE_REG_L3BRIDGE + REG_ID_7D) | ((INREG16(BASE_REG_L3BRIDGE + REG_ID_7E) & 0x1) << 16);
    speed    = calc_cnt * 12000;

    switch (dev)
    {
        case MEASURE_CLK_TEST:
            return scnprintf(buf, PAGE_SIZE, "TEST  CLK rate = %d Hz \n", speed);
        case MEASURE_CLK_MIU:
            return scnprintf(buf, PAGE_SIZE, "MIU   CLK rate = %d Hz \n", speed);
        case MEASURE_CLK_MCU:
            return scnprintf(buf, PAGE_SIZE, "MCU   CLK rate = %d Hz \n", speed);
        case MEASURE_CLK_IMI:
            return scnprintf(buf, PAGE_SIZE, "IMI   CLK rate = %d Hz \n", speed);
        case MEASURE_CLK_RISC:
            return scnprintf(buf, PAGE_SIZE, "RISC  CLK rate = %d Hz \n", speed);
        case MEASURE_CLK_PCIE0:
            return scnprintf(buf, PAGE_SIZE, "PCIE0 CLK rate = %d Hz \n", speed);
        case MEASURE_CLK_PCIE1:
            return scnprintf(buf, PAGE_SIZE, "PCIE1 CLK rate = %d Hz \n", speed);
        case MEASURE_CLK_SPI:
            return scnprintf(buf, PAGE_SIZE, "SPI   CLK rate = %d Hz \n", speed);
        default:
            return 0;
    }

    return 0;
}

static ssize_t measure_clk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int     dev_clock = 0;
    int     size      = 0;
    ssize_t resize    = 0;

    do
    {
        size = measure_clk_function(dev_clock, buf);

        dev_clock++;
        buf += size;
        resize += size;
    } while (size);

    resize += measure_clk_cm4(buf);

    return resize;
}
static DEVICE_ATTR(rate, 0444, measure_clk_show, NULL);

static int measure_clk_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int measure_clk_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static struct file_operations measure_clk_fops = {
    .owner   = THIS_MODULE,
    .open    = measure_clk_open,
    .release = measure_clk_release,
};

static int __init measure_clk_init(void)
{
    int            ret;
    struct device *dev;

    ret = register_chrdev(0, "measure_clk", &measure_clk_fops);
    if (ret < 0)
    {
        pr_err("cannot register measure_clk  (err=%d)\n", ret);
    }

    dev = device_create(msys_get_sysfs_class(), NULL, CAM_DEVICE_MKDEV(ret, 0), NULL, "measure_clk");

    device_create_file(dev, &dev_attr_rate);

    return 0;
}

subsys_initcall(measure_clk_init);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("MEASURE CLK driver");
MODULE_LICENSE("SSTAR");
