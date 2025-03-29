/*
 * drv_gpio_io.c- Sigmastar
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
//#include "MsCommon.h"
//#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>

#include <linux/err.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm.h>

#include <drv_gpio_io.h>
#include <hal_gpio.h>
#include <drv_gpio.h>
#include <ms_platform.h>
#include <gpio.h>

#define GPIO_DBG_ENABLE 0

#if GPIO_DBG_ENABLE
#define GPIO_DBG(_f) (_f)
#else
#define GPIO_DBG(_f)
#endif

#if 0
#define LINE_DBG() printf("GPIO %d\n", __LINE__)
#else
#define LINE_DBG()
#endif

#define GPIO_PRINT(fmt, args...) // printk("\n[GPIO][%05d] " fmt, __LINE__, ## args)

typedef struct
{
    s32                    s32MajorGPIO;
    s32                    s32MinorGPIO;
    struct cdev            cDevice;
    struct file_operations GPIOFop;
    struct fasync_struct * async_queue; /* asynchronous readers */
} GPIO_ModHandle_t;

#ifdef CONFIG_PM_SLEEP
typedef enum
{
    GPIO_INVAILD = 0,
    GPIO_OUT,
    GPIO_IN,
} GPIO_Dir_e;

typedef enum
{
    GPIO_HIZ = 0,
    GPIO_UP,
    GPIO_DOWN,
} GPIO_Pull_e;

typedef struct
{
    u8 isreq;
    u8 dir;
    u8 val;
    u8 drv;
    u8 pull;
} GPIO_State_t;
#endif

#define MOD_GPIO_DEVICE_COUNT 1
#define MOD_GPIO_NAME         "ModGPIO"

#define MDRV_NAME_GPIO  "gpio"
#define MDRV_MAJOR_GPIO 0x9b
#define MDRV_MINOR_GPIO 0x00

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

static struct device *dev;

#ifdef CONFIG_PM_SLEEP
static GPIO_State_t gpio_state_table[GPIO_NR];
#endif

// static struct class *gpio_class;

int camdriver_gpio_request(GPIO_Chip_t *chip, unsigned offset)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].isreq = TRUE;
#endif
    sstar_gpio_pad_set(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_request offset=%d\n", offset);
    return 0;
}

void camdriver_gpio_free(GPIO_Chip_t *chip, unsigned offset)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].isreq = FALSE;
#endif
    sstar_gpio_pad_clr(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_free\n");
}

void camdriver_gpio_set(GPIO_Chip_t *chip, unsigned offset, int value)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].val = value;
#endif
    if (value == 0)
        sstar_gpio_pull_low(offset);
    else
        sstar_gpio_pull_high(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_set\n");
}

int camdriver_gpio_get(GPIO_Chip_t *chip, unsigned offset)
{
    u8 pad_level;
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_get\n");
    sstar_gpio_pad_read(offset, &pad_level);
    return pad_level;
}

int camdriver_gpio_direction_input(GPIO_Chip_t *chip, unsigned offset)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].dir = GPIO_IN;
#endif
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_direction_input\n");
    sstar_gpio_pad_odn(offset);
    return 0;
}

int camdriver_gpio_direction_output(GPIO_Chip_t *chip, unsigned offset, int value)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].val = value;
    gpio_state_table[offset].dir = GPIO_OUT;
#endif
    if (value == 0)
        sstar_gpio_pull_low(offset);
    else
        sstar_gpio_pull_high(offset);
    sstar_gpio_pad_oen(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_direction_output\n");
    return 0;
}

int camdriver_gpio_drv_set(GPIO_Chip_t *chip, unsigned offset, int value)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].drv = value;
#endif
    return sstar_gpio_drv_set(offset, value);
}

int camdriver_gpio_pull_up(GPIO_Chip_t *chip, unsigned offset)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].pull = GPIO_UP;
#endif
    return sstar_gpio_pull_up(offset);
}

int camdriver_gpio_pull_down(GPIO_Chip_t *chip, unsigned offset)
{
#ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].pull = GPIO_DOWN;
#endif
    return sstar_gpio_pull_down(offset);
}

int camdriver_gpio_to_irq(GPIO_Chip_t *chip, unsigned offset)
{
    int virq;

    virq = sstar_gpio_to_irq(offset);
    if (virq < 0)
        return -ENXIO;

    GPIO_PRINT("%s virq:%d \n", __FUNCTION__, virq);
    return virq;
}

int camdriver_gpio_get_direction(GPIO_Chip_t *chip, unsigned offset)
{
    u8 status;
    sstar_gpio_pad_in_out(offset, &status);
    return status;
}

int camdriver_gpio_get_reg_cfg(GPIO_Chip_t *chip, unsigned offset, void *cfg)
{
    return sstar_gpio_get_reg_cfg(offset, cfg);
}

static int _camdriver_gpio_request(struct gpio_chip *chip, unsigned offset)
{
    return camdriver_gpio_request(chip, offset);
}

static void _camdriver_gpio_free(struct gpio_chip *chip, unsigned offset)
{
    return camdriver_gpio_free(chip, offset);
}

static int _camdriver_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
    return camdriver_gpio_direction_input(chip, offset);
}

static int _camdriver_gpio_get(struct gpio_chip *chip, unsigned offset)
{
    return camdriver_gpio_get(chip, offset);
}

static int _camdriver_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
    return camdriver_gpio_direction_output(chip, offset, value);
}

static void _camdriver_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
    return camdriver_gpio_set(chip, offset, value);
}

static int _camdriver_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
    return camdriver_gpio_to_irq(chip, offset);
}

static int _camdriver_gpio_get_direction(struct gpio_chip *chip, unsigned offset)
{
    return camdriver_gpio_get_direction(chip, offset);
}

static struct gpio_chip camdriver_gpio_chip = {
    .label            = "gpio",
    .request          = _camdriver_gpio_request,
    .free             = _camdriver_gpio_free,
    .direction_input  = _camdriver_gpio_direction_input,
    .get              = _camdriver_gpio_get,
    .direction_output = _camdriver_gpio_direction_output,
    .set              = _camdriver_gpio_set,
    .to_irq           = _camdriver_gpio_to_irq,
    .get_direction    = _camdriver_gpio_get_direction,
    .base             = 0,
};

static const struct of_device_id camdriver_gpio_of_match[] = {
    {.compatible = "sstar,gpio"},
    {},
};

static int camdriver_gpio_probe(struct platform_device *pdev)
{
    const struct of_device_id *match;
    int                        ret;

    dev = &pdev->dev;
    GPIO_PRINT("\n++[camdriver-gpio]camdriver_gpio_probe start\n");

    match = of_match_device(camdriver_gpio_of_match, &pdev->dev);
    if (!match)
    {
        printk("Err:[gpio] No dev found\n");
        return -ENODEV;
    }

    camdriver_gpio_chip.ngpio   = GPIO_NR;
    camdriver_gpio_chip.of_node = pdev->dev.of_node;
    ret                         = gpiochip_add(&camdriver_gpio_chip);
    if (ret < 0)
    {
        printk("[gpio] add err\n");
        return ret;
    }

    GPIO_PRINT("--[camdriver-gpio]camdriver_gpio_probe end\n");

    sstar_gpio_init();
    printk(KERN_WARNING "GPIO: probe end");
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int camdriver_gpio_suspend(struct device *dev)
{
    return 0;
}

static int camdriver_gpio_resume(struct device *dev)
{
    int i;
    for (i = 0; i < GPIO_NR; i++)
    {
        if (gpio_state_table[i].isreq == TRUE)
        {
            sstar_gpio_pad_set(i);
            if (gpio_state_table[i].dir == GPIO_IN)
            {
                sstar_gpio_pad_odn(i);
            }
            else if (gpio_state_table[i].dir == GPIO_OUT)
            {
                if (gpio_state_table[i].val == 0)
                {
                    sstar_gpio_pull_low(i);
                }
                else
                {
                    sstar_gpio_pull_high(i);
                }
                sstar_gpio_pad_oen(i);
            }
            sstar_gpio_drv_set(i, gpio_state_table[i].drv);
            if (gpio_state_table[i].pull == GPIO_UP)
            {
                sstar_gpio_pull_up(i);
            }
            else if (gpio_state_table[i].pull == GPIO_DOWN)
            {
                sstar_gpio_pull_down(i);
            }
        }
    }
    return 0;
}
#else
#define camdriver_gpio_suspend NULL
#define camdriver_gpio_resume  NULL
#endif

static const struct dev_pm_ops camdriver_gpio_pm_ops = {
    .suspend = camdriver_gpio_suspend,
    .resume  = camdriver_gpio_resume,
};

static struct platform_driver camdriver_gpio_driver = {
    .driver =
        {
            .name           = "gpio",
            .owner          = THIS_MODULE,
            .of_match_table = camdriver_gpio_of_match,
            .pm             = &camdriver_gpio_pm_ops,
        },
    .probe = camdriver_gpio_probe,
};

void __mod_gpio_init(void)
{
    // GPIO chiptop initialization
    sstar_gpio_init();
}

static int __init camdriver_gpio_init(void)
{
    return platform_driver_register(&camdriver_gpio_driver);
}
postcore_initcall(camdriver_gpio_init);

EXPORT_SYMBOL(camdriver_gpio_to_irq);
EXPORT_SYMBOL(camdriver_gpio_direction_output);
EXPORT_SYMBOL(camdriver_gpio_request);
EXPORT_SYMBOL(camdriver_gpio_free);
EXPORT_SYMBOL(camdriver_gpio_set);
EXPORT_SYMBOL(camdriver_gpio_get);
EXPORT_SYMBOL(camdriver_gpio_direction_input);
EXPORT_SYMBOL(camdriver_gpio_drv_set);
EXPORT_SYMBOL(camdriver_gpio_pull_up);
EXPORT_SYMBOL(camdriver_gpio_pull_down);
EXPORT_SYMBOL(camdriver_gpio_get_reg_cfg);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("GPIO driver");
MODULE_LICENSE("GPL");
