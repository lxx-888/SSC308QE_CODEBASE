/*
 * drv_mcu.c - Sigmastar
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
#ifdef CONFIG_DM
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#endif
#include "asm/types.h"

#include <gpio.h>
#include <drv_gpio.h>
#include <asm/io.h>
#include <asm/arch/mach/io.h>
#include <asm/arch/mach/platform.h>

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

struct mcu_cfg_t * mcu_cfg;
static const char *mcu_wakeup_source[GPIO_WAKEUP_START] = {[NEC_WAKEUP] = "ir_nec", [RC5_WAKEUP] = "ir_rc5"};

void sstar_mcu_write_config(void)
{
    u8            i;
    void __iomem *base = (void __iomem *)mcu_cfg->base;

    writeb(mcu_cfg->ir_mode, base++);
    for (i = 0; i < IR_KEY_NR; i++)
    {
        writeb(mcu_cfg->ir_key[i], base++);
        if (!((unsigned long)base % 2))
        {
            base += 2;
        }
    }
    for (i = 0; i < GPIO_SIZE; i++)
    {
        writeb(mcu_cfg->gpio_enable[i], base++);
        if (!((unsigned long)base % 2))
        {
            base += 2;
        }
    }
}
EXPORT_SYMBOL(sstar_mcu_write_config);

u8 sstar_mcu_get_wakeup(void)
{
    void __iomem *base = (void __iomem *)mcu_cfg->base;
    base += ((IR_KEY_NR + 1 + GPIO_SIZE) / 2) * 4 + ((IR_KEY_NR + 1 + GPIO_SIZE) % 2);
    return readb(base);
}
EXPORT_SYMBOL(sstar_mcu_get_wakeup);

const char *mcu_get_wakeup_name(void)
{
    const U8 *name;
    u8        index = sstar_mcu_get_wakeup();

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

static int sstar_mcu_of_to_plat(struct udevice *dev)
{
    int               i;
    int               size;
    fdt_addr_t        base;
    const char *      mode;
    u32               key[IR_KEY_NR]  = {0};
    u32               gpio[PAD_PM_NR] = {0};
    struct mcu_cfg_t *cfg             = dev_get_plat(dev);

    base = dev_read_addr(dev);
    if (base == FDT_ADDR_T_NONE)
    {
        return -EINVAL;
    }
    cfg->base = (unsigned long)base;
    dev_dbg(dev, "base of configuration regster is %lx\n", cfg->base);

    mode = dev_read_string(dev, "ir-mode");
    if (mode)
    {
        cfg->ir_mode = parse_mcu_ir_mode(mode);
    }
    else
    {
        cfg->ir_mode = 0;
    }
    dev_dbg(dev, "ir mode is %d\n", cfg->ir_mode);

    if (dev_read_prop(dev, "ir-key-table", &size))
    {
        size /= sizeof(u32);
        if (size > IR_KEY_NR)
        {
            dev_warn(dev, "number of ir key %d is greater than max number parse %d only\n", size, IR_KEY_NR);
            size = IR_KEY_NR;
        }

        if (!dev_read_u32_array(dev, "ir-key-table", key, size))
        {
            for (i = 0; i < size; i++)
            {
                if (key[i] <= U8_MAX)
                {
                    cfg->ir_key[i] = (u8)key[i];
                    dev_dbg(dev, "add ir key %02x\n", cfg->ir_key[i]);
                }
                else
                {
                    dev_warn(dev, "abandon illegal ir key %02x\n", key[i]);
                }
            }
        }
    }

    if (dev_read_prop(dev, "gpio-table", &size))
    {
        size /= sizeof(u32);
        if (size > PAD_PM_NR)
        {
            dev_warn(dev, "number of gpio %d is greater than max number parse %d only\n", size, PAD_PM_NR);
            size = PAD_PM_NR;
        }

        if (!dev_read_u32_array(dev, "gpio-table", gpio, size))
        {
            for (i = 0; i < size; i++)
            {
                if (gpio[i] >= PAD_PM_START && gpio[i] <= PAD_PM_END)
                {
                    dev_dbg(dev, "add gpio %d to enable list\n", gpio[i]);
                    cfg->gpio_enable[(gpio[i] - PAD_PM_START) / 8] |= (1 << ((gpio[i] - PAD_PM_START) % 8));
                }
                else
                {
                    dev_warn(dev, "abandon illegal gpio index %d\n", gpio[i]);
                }
            }
        }
    }

    mcu_cfg = cfg;

    return 0;
}

static const struct udevice_id sstar_mcu_ids[] = {{.compatible = "sstar,mcu"}, {}};

U_BOOT_DRIVER(sstar_mcu) = {
    .name       = "sstar,mcu",
    .id         = UCLASS_NOP,
    .of_match   = sstar_mcu_ids,
    .of_to_plat = sstar_mcu_of_to_plat,
    .plat_auto  = sizeof(struct mcu_cfg_t),
    .flags      = DM_FLAG_PRE_RELOC,
};

void sstar_mcu_init(void)
{
    struct udevice *dev;
    int             ret;

    for (uclass_first_device(UCLASS_NOP, &dev); dev; uclass_find_next_device(&dev))
    {
        if (dev->driver == DM_DRIVER_GET(sstar_mcu))
        {
            ret = device_probe(dev);
            if (ret)
            {
                printf("Failed to probe device %s err: %d\n", dev->name, ret);
            }
        }
    }
}
