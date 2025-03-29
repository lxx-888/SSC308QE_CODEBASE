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

#include "gpi-irqs.h"
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

struct sstar_pm_gpi
{
    u16                fiq_flags[PM_GPI_FIQ_NUM];
    u32                parent_gpi_hwirq;
    struct irq_domain *domain;
};

static struct sstar_pm_gpi sstar_pm_gpi_data;
static DEFINE_SPINLOCK(sstar_pm_gpi_irq_controller_lock);

static void sstar_pm_gpi_irq_ack(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);

    if (pmsleep_fiq < PM_GPI_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_CLEAR);
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_pm_gpi_irq_eoi(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);

    if (pmsleep_fiq < PM_GPI_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_CLEAR);
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    irq_chip_eoi_parent(d);
}

static void sstar_pm_gpi_irq_mask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);

    if (pmsleep_fiq < PM_GPI_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_MASK);
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_pm_gpi_irq_unmask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, pmsleep_fiq);

    if (pmsleep_fiq < PM_GPI_FIQ_END)
    {
        CLRREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_MASK);
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    irq_chip_unmask_parent(d);
}

static int sstar_pm_gpi_irq_set_type(struct irq_data *d, unsigned int type)
{
    U16 pmsleep_fiq;
    pr_debug("%s %d type:0x%08x\n", __FUNCTION__, __LINE__, type);

    pmsleep_fiq = d->hwirq;

    if (pmsleep_fiq < PM_GPI_FIQ_END)
    {
        switch (type)
        {
            case IRQ_TYPE_EDGE_FALLING:
                SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_74 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_EDGE_RISING:
                CLRREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_74 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_EDGE_BOTH:
                SETREG16((BASE_REG_PMGPIO_PA + REG_ID_74 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
                CLRREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_LEVEL_LOW:
                SETREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                SETREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
                break;
            case IRQ_TYPE_LEVEL_HIGH:
                CLRREG16(BASE_REG_PMGPIO_PA + ((pmsleep_fiq - PM_GPI_FIQ_START) << 2), PMGPIO_FIQ_POLARITY);
                SETREG16((BASE_REG_PMGPIO_PA + REG_ID_70 + ((pmsleep_fiq - PM_GPI_FIQ_START) / 16) * 4),
                         (1 << ((pmsleep_fiq - PM_GPI_FIQ_START) % 16)));
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

struct irq_chip sstar_pm_gpi_chip = {
    .name         = "SSTAR_PM_GPI_INTC",
    .irq_ack      = sstar_pm_gpi_irq_ack,
    .irq_eoi      = sstar_pm_gpi_irq_eoi,
    .irq_mask     = sstar_pm_gpi_irq_mask,
    .irq_unmask   = sstar_pm_gpi_irq_unmask,
    .irq_set_type = sstar_pm_gpi_irq_set_type,
};

static void sstar_pm_gpi_handle_irq(struct irq_desc *desc)
{
    unsigned int     i;
    unsigned int     cascade_irq = 0xFFFFFFFF;
    unsigned int     virq        = desc->irq_data.irq;
    struct irq_data *parent_data = NULL;
    struct irq_data *data        = NULL;
    unsigned int     final_status;

    data        = irq_domain_get_irq_data(sstar_pm_gpi_data.domain, virq);
    parent_data = data->parent_data;

    spin_lock(&sstar_pm_gpi_irq_controller_lock);
    for (i = PM_GPI_FIQ_START; i < PM_GPI_FIQ_END; i++)
    {
        final_status = INREG16(BASE_REG_PMGPIO_PA + REG_ID_00 + (i << 2));
        if (final_status & PMGPIO_FIQ_FINAL_STATUS)
        {
            cascade_irq = i;
            break;
        }
    }
    spin_unlock(&sstar_pm_gpi_irq_controller_lock);

    if (0xFFFFFFFF == cascade_irq)
    {
        pr_err("[%s:%d] error final_status:%d 0x%04X virq:%d\n", __FUNCTION__, __LINE__, cascade_irq, final_status,
               virq);
        panic("[%s] error %d \n", __FUNCTION__, __LINE__);
    }

    virq = irq_find_mapping(sstar_pm_gpi_data.domain, cascade_irq);
    if (!virq)
    {
        printk("[%s] err %d cascade_irq:%d\n", __FUNCTION__, __LINE__, cascade_irq);
        panic("[%s] error %d \n", __FUNCTION__, __LINE__);
    }

    pr_debug("%s %d final_status:%d 0x%04X virq:%d\n", __FUNCTION__, __LINE__, cascade_irq, final_status, virq);
    handle_fasteoi_irq(irq_to_desc(virq));
}

static int sstar_pm_gpi_irq_domain_translate(struct irq_domain *domain, struct irq_fwspec *fwspec, unsigned long *hwirq,
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

static int sstar_pm_gpi_irq_domain_alloc(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs, void *data)
{
    struct irq_fwspec *fwspec = data;
    struct irq_fwspec  parent_fwspec;
    irq_hw_number_t    hwirq;
    unsigned int       i;

    if (fwspec->param_count != 1)
        return -EINVAL;

    //*alloc parent*//
    parent_fwspec             = *fwspec;
    parent_fwspec.fwnode      = domain->parent->fwnode;
    parent_fwspec.param_count = 3;
    parent_fwspec.param[0]    = GIC_SPI;
    parent_fwspec.param[1]    = sstar_pm_gpi_data.parent_gpi_hwirq;
    parent_fwspec.param[2]    = IRQ_TYPE_EDGE_BOTH;

    //*alloc parent*//
    irq_domain_alloc_irqs_parent(domain, virq, nr_irqs, &parent_fwspec);

    hwirq = fwspec->param[0];

    for (i = 0; i < nr_irqs; i++)
    {
        irq_domain_set_info(domain, virq + i, hwirq + i, &sstar_pm_gpi_chip, NULL, sstar_pm_gpi_handle_irq, NULL, NULL);
        pr_err("[SSTAR_PM_GPI_INTC] hw:%d -> v:%d\n", (unsigned int)hwirq + i, virq + i);
    }

    parent_fwspec = *fwspec;

    return 0;
}

static void sstar_pm_gpi_irq_domain_free(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs)
{
    unsigned int i;

    for (i = 0; i < nr_irqs; i++)
    {
        struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
        irq_domain_reset_irq_data(d);
    }
}

struct irq_domain_ops sstar_pm_gpi_irq_domain_ops = {
    .translate = sstar_pm_gpi_irq_domain_translate,
    .alloc     = sstar_pm_gpi_irq_domain_alloc,
    .free      = sstar_pm_gpi_irq_domain_free,
};

#ifdef CONFIG_PM_SLEEP
static int sstar_pm_gpi_suspend(void)
{
    unsigned int i;

    for (i = 0; i < PM_GPI_FIQ_NUM; i++)
    {
        sstar_pm_gpi_data.fiq_flags[i] = INREG16(BASE_REG_PMGPIO_PA + (i << 2));
        sstar_pm_gpi_data.fiq_flags[i] |= PMGPIO_FIQ_MASK; // always set mask here, unmask will be set by irq/pm.c
    }

    pr_debug("sstar_pm_gpi_suspend\n\n");
    return 0;
}

static void sstar_pm_gpi_resume(void)
{
    unsigned int i;

    for (i = 0; i < PM_GPI_FIQ_NUM; i++)
    {
        OUTREG16(BASE_REG_PMGPIO_PA + (i << 2), sstar_pm_gpi_data.fiq_flags[i]);
    }
    pr_debug("sstar_pm_gpi_resume\n\n");
}

struct syscore_ops sstar_pm_gpi_syscore_ops = {
    .suspend = sstar_pm_gpi_suspend,
    .resume  = sstar_pm_gpi_resume,
};
#endif

static int __init sstar_pm_gpi_init(struct device_node *np, struct device_node *interrupt_parent)
{
    struct irq_domain *    parent_domain;
    struct of_phandle_args oirq;

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

    sstar_pm_gpi_data.domain =
        irq_domain_add_hierarchy(parent_domain, 0, PM_GPI_FIQ_NUM, np, &sstar_pm_gpi_irq_domain_ops, NULL);

    if (!sstar_pm_gpi_data.domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }

    if (of_irq_parse_one(np, 0, &oirq))
        pr_err("%s: %s of_irq_parse_one fail\n", __func__, np->name);

    sstar_pm_gpi_data.parent_gpi_hwirq = oirq.args[1];

#ifdef CONFIG_PM_SLEEP
    register_syscore_ops(&sstar_pm_gpi_syscore_ops);
#endif
    return 0;
}

IRQCHIP_DECLARE(sstar_pm_gpi_intc, "sstar,pm-gpi-intc", sstar_pm_gpi_init);
