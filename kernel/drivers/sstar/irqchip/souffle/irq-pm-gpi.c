/*
 * irq-pm-gpi.c- Sigmastar
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
#include <linux/init.h>
#include <linux/io.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/syscore_ops.h>
#include <asm/mach/irq.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/irqchip.h>
#include <linux/irqdesc.h>

#include <dt-bindings/interrupt-controller/arm-gic.h>

#include "irqs.h"
#include "registers.h"

#include "ms_platform.h"
#include "ms_types.h"

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

#define PMGPIO_OEN              BIT0
#define PMGPIO_OUTPUT           BIT1
#define PMGPIO_INPUT            BIT2
#define PMGPIO_FIQ_MASK         BIT4
#define PMGPIO_FIQ_FROCE        BIT5
#define PMGPIO_FIQ_CLEAR        BIT6
#define PMGPIO_FIQ_POLARITY     BIT7
#define PMGPIO_FIQ_FINAL_STATUS BIT8
#define PMGPIO_FIQ_RAW_STATUS   BIT9

#ifdef CONFIG_PM_SLEEP
#define PM_FIQ_NUM (INT_PMSLEEP_PAD_PM_USB3_ID - PMSLEEP_FIQ_START + 1)

/**
 * struct ms_pm_irq_priv - private pm interrupt data
 * @fiq_flags:      Flags of each pm-gpio fiq
 * @irq_polarity:   Polarity of pm-sleep irq
 */
struct sstar_pm_irq_priv
{
    U16 fiq_mask[PM_FIQ_NUM];
    U16 fiq_polarity[PM_FIQ_NUM];
    U8  irq_polarity;
};

static struct sstar_pm_irq_priv pm_irq_priv;
#endif

static void sstar_pm_irq_ack(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;

    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);

    if (pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_CLEAR);
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_pm_irq_eoi(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);

    if (pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_CLEAR);
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_pm_irq_mask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);

    if (pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_MASK);
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (pmsleep_fiq < PMSLEEP_IRQ_END)
    {
        SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_08, 1 << (pmsleep_fiq - PMSLEEP_IRQ_START));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_pm_irq_unmask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);
    sstar_pm_irq_ack(d);

    if (pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        CLRREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_MASK);
    }
    else if (pmsleep_fiq < PMSLEEP_IRQ_END)
    {
        CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_08, 1 << (pmsleep_fiq - PMSLEEP_IRQ_START));
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    CLRREG16(BASE_REG_INTRCTL_PA + REG_ID_54, BIT2);
    INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
}

static int sstar_pm_irq_set_type(struct irq_data *d, unsigned int type)
{
    U16 pmsleep_fiq;
    pr_debug("%s %d type:0x%08x\n", __FUNCTION__, __LINE__, type);

    pmsleep_fiq = d->hwirq;

    if (pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        switch (type)
        {
            case IRQ_TYPE_EDGE_FALLING:
                SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_74 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_EDGE_RISING:
                CLRREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_74 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_EDGE_BOTH:
                SETREG16((BASE_REG_PMGPIO_PA + REG_ID_74 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_LEVEL_LOW:
                SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                SETREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_LEVEL_HIGH:
                CLRREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PMSLEEP_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                SETREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PMSLEEP_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PMSLEEP_FIQ_START) % 16)));
                break;
            default:
                return -EINVAL;
        }
    }
    else if (pmsleep_fiq < PMSLEEP_IRQ_END)
    {
        switch (type)
        {
            case IRQ_TYPE_EDGE_FALLING:
            case IRQ_TYPE_LEVEL_LOW:
                SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_09, 1 << (pmsleep_fiq - PMSLEEP_IRQ_START));
                break;
            case IRQ_TYPE_EDGE_RISING:
            case IRQ_TYPE_LEVEL_HIGH:
                CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_09, 1 << (pmsleep_fiq - PMSLEEP_IRQ_START));
                break;
            default:
                return -EINVAL;
        }
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return -EINVAL;
    }

    return 0;
}

struct irq_chip sstar_pm_intc_irqchip = {
    .name         = "SS_PM_INTC",
    .irq_ack      = sstar_pm_irq_ack,
    .irq_eoi      = sstar_pm_irq_eoi,
    .irq_mask     = sstar_pm_irq_mask,
    .irq_unmask   = sstar_pm_irq_unmask,
    .irq_set_type = sstar_pm_irq_set_type,
};
EXPORT_SYMBOL(sstar_pm_intc_irqchip);

static DEFINE_SPINLOCK(sstar_irq_controller_lock);

static void sstar_handle_cascade_pm_irq(struct irq_desc *desc)
{
    unsigned int       cascade_irq = 0xFFFFFFFF, i;
    unsigned int       virq;
    struct irq_chip *  chip   = irq_desc_get_chip(desc);
    struct irq_domain *domain = irq_desc_get_handler_data(desc);
    unsigned int       final_status;

    if (!domain)
    {
        printk("[%s] err %d \n", __FUNCTION__, __LINE__);
        goto exit;
    }

    spin_lock(&sstar_irq_controller_lock);
    for (i = PMSLEEP_FIQ_START; i < PMSLEEP_FIQ_END; i++)
    {
        final_status = INREG16(BASE_REG_PMGPIO_PA + (i - PMSLEEP_FIQ_START) * 4);
        if (final_status & PMGPIO_FIQ_FINAL_STATUS)
        {
            cascade_irq = i;
            pr_debug("[%s] Get hwirq:%d, Reg:0x%04x\n", __FUNCTION__, cascade_irq, final_status);
            break;
        }
    }
    spin_unlock(&sstar_irq_controller_lock);

    if (0xFFFFFFFF == cascade_irq)
    {
        pr_err("[%s:%d] error final_status:%d 0x%04X\n", __FUNCTION__, __LINE__, cascade_irq, final_status);
        panic("%s(%d) invalid cascade_irq: %d\n", __FUNCTION__, __LINE__, cascade_irq);
        chained_irq_exit(chip, desc);
        goto exit;
    }

    virq = irq_find_mapping(domain, cascade_irq);
    if (!virq)
    {
        printk("[%s] err %d \n", __FUNCTION__, __LINE__);
        goto exit;
    }
    pr_debug("%s %d final_status:%d 0x%04X virq:%d\n", __FUNCTION__, __LINE__, cascade_irq, final_status, virq);
    chained_irq_enter(chip, desc);
    generic_handle_irq(virq);

exit:
    chained_irq_exit(chip, desc);
}

static int sstar_pm_intc_domain_translate(struct irq_domain *domain, struct irq_fwspec *fwspec, unsigned long *hwirq,
                                          unsigned int *type)
{
    if (is_of_node(fwspec->fwnode))
    {
        if (fwspec->param_count != 1)
            return -EINVAL;
        *hwirq = fwspec->param[0];
        return 0;
    }

    return -EINVAL;
}

static int sstar_pm_intc_domain_alloc(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs, void *data)
{
    struct irq_fwspec *fwspec = data;
    irq_hw_number_t    hwirq;
    unsigned int       i;

    if (fwspec->param_count != 1)
        return -EINVAL;

    hwirq = fwspec->param[0];

    for (i = 0; i < nr_irqs; i++)
    {
        irq_domain_set_info(domain, virq + i, hwirq + i, &sstar_pm_intc_irqchip, NULL, handle_fasteoi_irq, NULL, NULL);
        pr_debug("[MS_PM_INTC] hw:%d -> v:%d\n", (unsigned int)hwirq + i, virq + i);
        /*
         * GPIOs don't have an equivalent interrupt in the parent
         * controller (ms_main_intc), so its interrupt hierarchy
         * stops at the gpi level
         */
        if (domain->parent)
            irq_domain_disconnect_hierarchy(domain->parent, virq + i);
    }
    return 0;
}

static void sstar_pm_intc_domain_free(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs)
{
    unsigned int i;

    for (i = 0; i < nr_irqs; i++)
    {
        struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
        irq_domain_reset_irq_data(d);
    }
}

struct irq_domain_ops sstar_pm_intc_domain_ops = {
    .translate = sstar_pm_intc_domain_translate,
    .alloc     = sstar_pm_intc_domain_alloc,
    .free      = sstar_pm_intc_domain_free,
};

#ifdef CONFIG_PM_SLEEP
static int sstar_pm_intc_suspend(void)
{
    unsigned int i;

    for (i = 0; i < PM_FIQ_NUM; i++)
    {
        pm_irq_priv.fiq_mask[i]     = INREG16(BASE_REG_PMGPIO_PA + (i << 2)) & PMGPIO_FIQ_MASK;
        pm_irq_priv.fiq_polarity[i] = INREG16(BASE_REG_PMGPIO_PA + (i << 2)) & PMGPIO_FIQ_POLARITY;
    }
    pm_irq_priv.irq_polarity = INREG8(BASE_REG_PMSLEEP_PA + REG_ID_09);

    pr_debug("ms_pm_intc_suspend\n\n");
    return 0;
}

static void sstar_pm_intc_resume(void)
{
    unsigned int i;

    for (i = 0; i < PM_FIQ_NUM; i++)
    {
        SETREG16(BASE_REG_PMGPIO_PA + (i << 2), PMGPIO_FIQ_CLEAR);
        OUTREGMSK16(BASE_REG_PMGPIO_PA + (i << 2), pm_irq_priv.fiq_mask[i], PMGPIO_FIQ_MASK);
        OUTREGMSK16(BASE_REG_PMGPIO_PA + (i << 2), pm_irq_priv.fiq_polarity[i], PMGPIO_FIQ_POLARITY);
    }
    OUTREG8(BASE_REG_PMSLEEP_PA + REG_ID_09, pm_irq_priv.irq_polarity);
    pr_debug("ms_pm_intc_resume\n\n");
}

struct syscore_ops sstar_pm_intc_syscore_ops = {
    .suspend = sstar_pm_intc_suspend,
    .resume  = sstar_pm_intc_resume,
};
#endif

static int __init sstar_init_pm_intc(struct device_node *np, struct device_node *interrupt_parent)
{
    struct irq_domain *parent_domain, *sstar_pm_irq_domain;
    int                irq = 0;

    if (!interrupt_parent)
    {
        pr_err("%s: %s no parent\n", __func__, np->name);
        return -ENODEV;
    }

    pr_err("%s: np->name=%s, parent=%s\n", __func__, np->name, interrupt_parent->name);

    parent_domain = irq_find_host(interrupt_parent);
    if (!parent_domain)
    {
        pr_err("%s: %s unable to obtain parent domain\n", __func__, np->name);
        return -ENXIO;
    }

    sstar_pm_irq_domain =
        irq_domain_add_hierarchy(parent_domain, 0, PMSLEEP_FIQ_NR, np, &sstar_pm_intc_domain_ops, NULL);

    if (!sstar_pm_irq_domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }

    irq = irq_of_parse_and_map(np, 0);
    if (!irq)
    {
        pr_err("Get irq err from DTS\n");
        return -EPROBE_DEFER;
    }

    irq_set_chained_handler_and_data(irq, sstar_handle_cascade_pm_irq, sstar_pm_irq_domain);

#ifdef CONFIG_PM_SLEEP
    register_syscore_ops(&sstar_pm_intc_syscore_ops);
#endif
    return 0;
}

IRQCHIP_DECLARE(sstar_pm_gpi_intc, "sstar,pm-gpi-intc", sstar_init_pm_intc);
