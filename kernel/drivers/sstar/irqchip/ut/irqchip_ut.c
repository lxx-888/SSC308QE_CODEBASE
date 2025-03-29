/*
 * irqchip_ut.c- Sigmastar
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
#include <linux/random.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/irq.h>
#include <irqs.h>
#include <ms_platform.h>
#include <registers.h>
#include <gpi-irqs.h>
#include <gpio.h>
#include <hal_irqchip_ut.h>

// cmdline parament
static unsigned int para_hwirq = -1;
module_param(para_hwirq, uint, 0);

static char *para_intc = "";
module_param(para_intc, charp, 0);

static unsigned int para_pin = -1;
module_param(para_pin, uint, 0);

static int para_case = -1;
module_param(para_case, int, 0);

static char *g_case_name[4] = {"unmask test", "mask test", "polarity test", "no request irq, force irq test"};

enum
{
    IRQCHIP_UT_MAIN_INTC = 0x0,
    IRQCHIP_UT_PM_MAIN_INTC,
    IRQCHIP_UT_GPI_INTC,
    IRQCHIP_UT_PM_GPI_INTC,
};

enum
{
    IRQCHIP_UT_UNMASK = 0x0,
    IRQCHIP_UT_MASK,
    IRQCHIP_UT_POLARITY,
    IRQCHIP_UT_NO_REQUEST_IRQ_FORCE,
};

struct irqchip_ut
{
    bool be_trigger;
    u32  virq;
    u32  hwirq;
    u32  pin;
    u32  irq_be_trigger_count;
    u32  intc;
    u32  ut_case;
};

static struct irqchip_ut irqchip_ut_pattern;

static int irqchip_ut_force_irq(int hwirq, int force)
{
    switch (irqchip_ut_pattern.intc)
    {
        case IRQCHIP_UT_MAIN_INTC:
            return main_intc_force_irq(hwirq, force);
        case IRQCHIP_UT_PM_MAIN_INTC:
            return pm_main_intc_force_irq(hwirq, force);
        case IRQCHIP_UT_GPI_INTC:
            return gpi_intc_force_irq(hwirq, force);
        case IRQCHIP_UT_PM_GPI_INTC:
            return pm_gpi_intc_force_irq(hwirq, force);
        default:
            pr_err("hwirq:%d force trigger irq not implment\t\t\t---FAILED\n", hwirq);
    }

    return -EINVAL;
}

static int irqchip_ut_mask_irq(int hwirq)
{
    switch (irqchip_ut_pattern.intc)
    {
        case IRQCHIP_UT_MAIN_INTC:
            return main_intc_mask_irq(hwirq);
        case IRQCHIP_UT_PM_MAIN_INTC:
            return pm_main_intc_mask_irq(hwirq);
        case IRQCHIP_UT_GPI_INTC:
            return gpi_intc_mask_irq(hwirq);
        case IRQCHIP_UT_PM_GPI_INTC:
            return pm_gpi_intc_mask_irq(hwirq);
        default:
            pr_err("intc:%s mask irq not implment\t\t\t---FAILED\n\n", para_intc);
    }

    return -EINVAL;
}

irqreturn_t irqchip_ut_unmask_test_interrupt(int irq, void *data)
{
    int hwirq = irq_get_irq_data(irq)->hwirq;

    if (irqchip_ut_pattern.hwirq == hwirq)
        irqchip_ut_pattern.irq_be_trigger_count++;

    if (irqchip_ut_pattern.irq_be_trigger_count < 3)
    {
        pr_err("hwirq:%d triggered, de-force irq\n", hwirq);
        irqchip_ut_force_irq(hwirq, 0);
    }
    else
    {
        pr_err("hwirq:%d triggered by peripherals, mask irq\n", hwirq);
        irqchip_ut_mask_irq(hwirq);
    }

    return IRQ_HANDLED;
}

irqreturn_t irqchip_ut_mask_test_interrupt(int irq, void *data)
{
    int hwirq = irq_get_irq_data(irq)->hwirq;

    if (irqchip_ut_pattern.be_trigger && (irqchip_ut_pattern.hwirq == hwirq))
    {
        pr_err("hirq:%d virq:%d triggered unexcept\n", hwirq, irq);
        irqchip_ut_pattern.irq_be_trigger_count++;
    }
    else
    {
        pr_err("hirq:%d virq:%d triggered, mask irq\n", hwirq, irq);
        irqchip_ut_mask_irq(hwirq);
        irqchip_ut_pattern.be_trigger = true;
    }

    return IRQ_HANDLED;
}

irqreturn_t irqchip_ut_pol_test_interrupt(int irq, void *data)
{
    int hwirq = irq_get_irq_data(irq)->hwirq;

    // g_init_done not set fall this handler means irq trigger bettewn
    // "request_irq" and "gpio_set_value 1".
    // g_irq_handle_count > 0 means irq trigger more then one times after
    // "gpio_set_value 1".
    if (!irqchip_ut_pattern.be_trigger)
    {
        pr_err("hirq:%d virq:%d pin:%d triggered\n", hwirq, irq, irqchip_ut_pattern.pin);
        irqchip_ut_pattern.be_trigger = true;
    }
    else
    {
        pr_err("hirq:%d virq:%d pin:%d triggered by peripherals, set mask\n", hwirq, irq, irqchip_ut_pattern.pin);
        irqchip_ut_pattern.irq_be_trigger_count++;
        irqchip_ut_mask_irq(hwirq);
    }

    return IRQ_HANDLED;
}

static __init int irqchip_ut_init(void)
{
    struct device_node *intc_node;
    struct irq_domain * domain;
    struct irq_fwspec   fwspec;
    struct irq_desc *   irq_desc;
    int                 ret = 0;

    if (para_hwirq != -1)
    {
        pr_err("**********************************************************************\n");
        pr_err("** irqchip:%s hwirq:%d case:%s\n", para_intc, para_hwirq, g_case_name[para_case]);
        pr_err("**********************************************************************\n");

        intc_node = of_find_compatible_node(NULL, NULL, para_intc);
        domain    = irq_find_host(intc_node);

        if (!domain)
        {
            pr_err("irqchip:%s not found\t\t\t---FAILED\n", para_intc);
            ret = -ENXIO;
            goto fail_log_sync_free;
        }

        if (!strcmp(para_intc, "sstar,main-intc"))
        {
            irqchip_ut_pattern.intc  = IRQCHIP_UT_MAIN_INTC;
            irqchip_ut_pattern.hwirq = para_hwirq;
            fwspec.param_count       = 3;
            fwspec.param[0]          = 0;
            fwspec.param[1]          = irqchip_ut_pattern.hwirq;
            fwspec.param[2]          = IRQ_TYPE_LEVEL_HIGH;
        }
        else if (!strcmp(para_intc, "sstar,pm-main-intc"))
        {
            irqchip_ut_pattern.intc  = IRQCHIP_UT_PM_MAIN_INTC;
            irqchip_ut_pattern.hwirq = para_hwirq;
            fwspec.param_count       = 1;
            fwspec.param[0]          = irqchip_ut_pattern.hwirq;
        }
        else if (!strcmp(para_intc, "sstar,gpi-intc"))
        {
            irqchip_ut_pattern.intc  = IRQCHIP_UT_GPI_INTC;
            irqchip_ut_pattern.hwirq = para_hwirq;
            fwspec.param_count       = 1;
            fwspec.param[0]          = irqchip_ut_pattern.hwirq;
        }
        else if (!strcmp(para_intc, "sstar,pm-gpi-intc"))
        {
            irqchip_ut_pattern.intc  = IRQCHIP_UT_PM_GPI_INTC;
            irqchip_ut_pattern.hwirq = para_hwirq;
            fwspec.param_count       = 1;
            fwspec.param[0]          = irqchip_ut_pattern.hwirq;
        }
        else
        {
            pr_err("irq domain:%s not support\t\t\t---FAILED\n", domain->name);
            ret = -EINVAL;
            goto fail_log_sync_free;
        }

        fwspec.fwnode           = of_node_to_fwnode(intc_node);
        irqchip_ut_pattern.virq = irq_create_fwspec_mapping(&fwspec);
        pr_err("virq %d hwirq %d\n", irqchip_ut_pattern.virq, irqchip_ut_pattern.hwirq);

        if (!irqchip_ut_pattern.virq)
        {
            pr_err("hwirq:%d mapping fail.\t\t\t---FAILED\n", irqchip_ut_pattern.hwirq);
            ret = -EINVAL;
            goto fail_log_sync_free;
        }

        // check if irq already has binding handler
        irq_desc = irq_to_desc(irqchip_ut_pattern.virq);

        if (irq_desc->irq_data.domain != domain)
        {
            pr_err("hwirq %d has already request by %s\n", irqchip_ut_pattern.hwirq, irq_desc->irq_data.chip->name);
            irqchip_ut_pattern.virq = 0;
            goto fail_log_sync_free;
        }
        else if (irq_desc->action)
        {
            if (irq_desc->action->name)
                pr_err("hwirq %d has already request by %s\n", irqchip_ut_pattern.hwirq, irq_desc->action->name);
            else
                pr_err("hwirq %d has already request\n", irqchip_ut_pattern.hwirq);

            irqchip_ut_pattern.virq = 0;
            goto fail_log_sync_free;
        }
    }
    else if (para_pin != -1)
    {
        pr_err("**********************************************************************\n");
        pr_err("** irqchip:%s pin:%d case:%s\n", para_intc, para_pin, g_case_name[para_case]);
        pr_err("**********************************************************************\n");

        irqchip_ut_pattern.pin = para_pin;

        if ((PAD_PM_UART_TX == irqchip_ut_pattern.pin) || (PAD_PM_UART_RX == irqchip_ut_pattern.pin))
        {
            pr_err("pin:%d avoid to test UART_RX or UART_TX\t\t\t---FAILED\n", irqchip_ut_pattern.pin);
            ret = -EINVAL;
            goto fail_log_sync_free;
        }

        ret = gpio_request_one(irqchip_ut_pattern.pin, GPIOF_OUT_INIT_LOW, "irqchip_ut_pol_test");
        if (ret)
        {
            pr_err("pin:%d request fail. ret:%d\t\t\t---FAILED\n", irqchip_ut_pattern.pin, ret);
            ret = -EIO;
            goto fail_log_sync_free;
        }

        irqchip_ut_pattern.virq  = gpio_to_irq(irqchip_ut_pattern.pin);
        irqchip_ut_pattern.hwirq = irq_get_irq_data(irqchip_ut_pattern.virq)->hwirq;

        if (!strcmp(para_intc, "sstar,gpi-intc"))
        {
            irqchip_ut_pattern.intc = IRQCHIP_UT_GPI_INTC;
        }
        else if (!strcmp(para_intc, "sstar,pm-gpi-intc"))
        {
            irqchip_ut_pattern.intc = IRQCHIP_UT_PM_GPI_INTC;
        }
        else
        {
            pr_err("irq domain:%s not support\t\t\t---FAILED\n", domain->name);
            ret = -EINVAL;
            goto fail_log_sync_free;
        }
    }
    else
    {
        ret = -EINVAL;
        goto fail_log_sync_free;
    }

    irqchip_ut_pattern.ut_case              = para_case;
    irqchip_ut_pattern.be_trigger           = false;
    irqchip_ut_pattern.irq_be_trigger_count = 0;

    switch (irqchip_ut_pattern.ut_case)
    {
        case IRQCHIP_UT_UNMASK:

            local_irq_disable();
            ret = request_irq(irqchip_ut_pattern.virq, irqchip_ut_unmask_test_interrupt, 0, "irqchip_ut", NULL);
            if (ret)
            {
                pr_err("hwirq:%d, virq:%d request fail. ret:%d\t\t\t---FAILED\n", irqchip_ut_pattern.hwirq,
                       irqchip_ut_pattern.virq, ret);
                ret = -EBUSY;
                local_irq_enable();
                goto fail_log_sync_free;
            }

            ret = irqchip_ut_force_irq(irqchip_ut_pattern.hwirq, 1);
            if (ret)
            {
                ret = -ENONET;
                local_irq_enable();
                goto fail_irq_free;
            }
            local_irq_enable();
            break;
        case IRQCHIP_UT_MASK:
            local_irq_disable();
            ret = request_irq(irqchip_ut_pattern.virq, irqchip_ut_mask_test_interrupt, 0, "irqchip_ut", NULL);
            if (ret)
            {
                pr_err("hwirq:%d, virq:%d request fail. ret:%d\t\t\t---FAILED\n", irqchip_ut_pattern.hwirq,
                       irqchip_ut_pattern.virq, ret);
                ret = -EBUSY;
                local_irq_enable();
                goto fail_log_sync_free;
            }

            ret = irqchip_ut_force_irq(irqchip_ut_pattern.hwirq, 1);
            if (ret)
            {
                ret = -ENONET;
                local_irq_enable();
                goto fail_irq_free;
            }
            local_irq_enable();
            break;
        case IRQCHIP_UT_POLARITY:
            if (irqchip_ut_pattern.pin)
            {
                gpio_set_value(irqchip_ut_pattern.pin, 0);
                local_irq_disable();
                ret = request_irq(irqchip_ut_pattern.virq, irqchip_ut_pol_test_interrupt, IRQF_TRIGGER_RISING,
                                  "irqchip_ut", NULL);
                if (ret)
                {
                    pr_err("hirq:%d, virq:%d request fail. ret:%d\t\t\t---FAILED\n", irqchip_ut_pattern.hwirq,
                           irqchip_ut_pattern.virq, ret);
                    ret = -EBUSY;
                    goto fail_log_sync_free;
                }
                local_irq_enable();
                irqchip_ut_pattern.be_trigger           = false;
                irqchip_ut_pattern.irq_be_trigger_count = 0;
            }
            break;
        case IRQCHIP_UT_NO_REQUEST_IRQ_FORCE:
            irqchip_ut_pattern.virq = 0;
            ret                     = irqchip_ut_force_irq(irqchip_ut_pattern.hwirq, 1);
            if (ret)
            {
                goto fail_log_sync_free;
            }
            break;
        default:
            pr_err("case:%d not support\n", para_case);
            return -EINVAL;
    }

    return ret;

fail_irq_free:
    free_irq(irqchip_ut_pattern.virq, NULL);
fail_log_sync_free:
    return ret;
}

static __exit void irqchip_ut_exit(void)
{
    switch (irqchip_ut_pattern.ut_case)
    {
        case IRQCHIP_UT_UNMASK:
            irqchip_ut_force_irq(irqchip_ut_pattern.hwirq, 0);
            if (irqchip_ut_pattern.irq_be_trigger_count >= 1)
                pr_err("hwirq:%d virq:%d \t\t\t---%s\n", irqchip_ut_pattern.hwirq, irqchip_ut_pattern.virq, "SUCCEED");
            else
                pr_err("hirq:%d virq:%d irq_be_trigger_count %d be_trigger %d \t\t\t---%s\n", irqchip_ut_pattern.hwirq,
                       irqchip_ut_pattern.virq, irqchip_ut_pattern.irq_be_trigger_count, irqchip_ut_pattern.be_trigger,
                       "FAILED");
            break;
        case IRQCHIP_UT_MASK:
            irqchip_ut_force_irq(irqchip_ut_pattern.hwirq, 0);
            if (irqchip_ut_pattern.be_trigger && (irqchip_ut_pattern.irq_be_trigger_count == 0))
                pr_err("hwirq:%d virq:%d \t\t\t---%s\n", irqchip_ut_pattern.hwirq, irqchip_ut_pattern.virq, "SUCCEED");
            else
                pr_err("hirq:%d virq:%d irq_be_trigger_count %d be_trigger %d \t\t\t---%s\n", irqchip_ut_pattern.hwirq,
                       irqchip_ut_pattern.virq, irqchip_ut_pattern.irq_be_trigger_count, irqchip_ut_pattern.be_trigger,
                       "FAILED");
            break;
        case IRQCHIP_UT_POLARITY:
            gpio_set_value(irqchip_ut_pattern.pin, 1);
            msleep(200);
            if (irqchip_ut_pattern.be_trigger && (irqchip_ut_pattern.irq_be_trigger_count == 0))
                pr_err("hwirq:%d virq:%d \t\t\t---%s\n", irqchip_ut_pattern.hwirq, irqchip_ut_pattern.virq, "SUCCEED");
            else
                pr_err("hirq:%d virq:%d irq_be_trigger_count %d be_trigger %d \t\t\t---%s\n", irqchip_ut_pattern.hwirq,
                       irqchip_ut_pattern.virq, irqchip_ut_pattern.irq_be_trigger_count, irqchip_ut_pattern.be_trigger,
                       "FAILED");
            break;
            gpio_free(irqchip_ut_pattern.pin);
            break;
        case IRQCHIP_UT_NO_REQUEST_IRQ_FORCE:
            irqchip_ut_force_irq(irqchip_ut_pattern.hwirq, 0);
            if (!irqchip_ut_pattern.be_trigger && (irqchip_ut_pattern.irq_be_trigger_count == 0))
                pr_err("hwirq:%d virq:%d \t\t\t---%s\n", irqchip_ut_pattern.hwirq, irqchip_ut_pattern.virq, "SUCCEED");
            else
                pr_err("hirq:%d virq:%d irq_be_trigger_count %d be_trigger %d \t\t\t---%s\n", irqchip_ut_pattern.hwirq,
                       irqchip_ut_pattern.virq, irqchip_ut_pattern.irq_be_trigger_count, irqchip_ut_pattern.be_trigger,
                       "FAILED");
            break;
        default:
            break;
    }

    if (irqchip_ut_pattern.virq)
        free_irq(irqchip_ut_pattern.virq, NULL);
}

module_init(irqchip_ut_init);
module_exit(irqchip_ut_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("sstar irqchip ut driver");
MODULE_LICENSE("GPL");
