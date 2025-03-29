/*
 * irq-gpi.c- Sigmastar
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

struct sstar_gpi
{
    U16                gpi_polarity[(GPI_FIQ_NUM + 15) >> 4];
    U16                gpi_edge[(GPI_FIQ_NUM + 15) >> 4];
    U16                gpi_mask[(GPI_FIQ_NUM + 15) >> 4];
    U16                gpic_polarity[(GPI_GPIC_NUM + 15) >> 4];
    U16                gpic_mask[(GPI_GPIC_NUM + 15) >> 4];
    u32                parent_gpi_hwirq;
    u32                parent_gpi_virq;
    struct irq_domain *domain;
};

static struct sstar_gpi sstar_gpi_data;

static void sstar_gpi_irq_ack(struct irq_data *d)
{
    U16 gpi_irq;

    gpi_irq = d->hwirq;
    pr_debug("[%s] hw:%d\n", __FUNCTION__, gpi_irq);

    if (gpi_irq >= 0 && gpi_irq < (GPI_FIQ_NUM + GPI_GPIC_NUM))
    {
        if (gpi_irq < GPI_GPIC_START)
        {
            OUTREG16((BASE_REG_GPI_INT_PA + REG_ID_20 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
            INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
        }
        else
        {
            OUTREG16((BASE_REG_GPI_INT2_PA + REG_ID_7A + ((gpi_irq - GPI_GPIC_START) / 16) * 4),
                     (1 << ((gpi_irq - GPI_GPIC_START) % 16)));
            INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
        }
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_gpi_irq_eoi(struct irq_data *d)
{
    sstar_gpi_irq_ack(d);
}

static void sstar_gpi_irq_mask(struct irq_data *d)
{
    U16 gpi_irq;

    gpi_irq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, gpi_irq);

    if (gpi_irq >= 0 && gpi_irq < (GPI_FIQ_NUM + GPI_GPIC_NUM))
    {
        if (gpi_irq < GPI_GPIC_START)
        {
            SETREG16((BASE_REG_GPI_INT_PA + REG_ID_00 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
            INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
        }
        else
        {
            SETREG16((BASE_REG_GPI_INT2_PA + REG_ID_78 + ((gpi_irq - GPI_GPIC_START) / 16) * 4),
                     (1 << ((gpi_irq - GPI_GPIC_START) % 16)));
            INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
        }
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_gpi_irq_unmask(struct irq_data *d)
{
    U16 gpi_irq;

    gpi_irq = d->hwirq;
    pr_debug("[%s] hw:%d \n", __FUNCTION__, gpi_irq);

    sstar_gpi_irq_ack(d);
    if (gpi_irq >= 0 && gpi_irq < (GPI_FIQ_NUM + GPI_GPIC_NUM))
    {
        if (gpi_irq < GPI_GPIC_START)
        {
            CLRREG16((BASE_REG_GPI_INT_PA + REG_ID_00 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
        }
        else
        {
            CLRREG16((BASE_REG_GPI_INT2_PA + REG_ID_78 + ((gpi_irq - GPI_GPIC_START) / 16) * 4),
                     (1 << ((gpi_irq - GPI_GPIC_START) % 16)));
        }
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static int sstar_gpi_irq_set_type(struct irq_data *d, unsigned int type)
{
    U16 gpi_irq;
    pr_debug("%s %d type:0x%08x\n", __FUNCTION__, __LINE__, type);

    gpi_irq = d->hwirq;

    if (gpi_irq >= 0 && gpi_irq < (GPI_FIQ_NUM + GPI_GPIC_NUM))
    {
        switch (type)
        {
            case IRQ_TYPE_EDGE_FALLING:
                if (gpi_irq < GPI_GPIC_START)
                {
                    SETREG16((BASE_REG_GPI_INT_PA + REG_ID_30 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                    CLRREG16((BASE_REG_GPI_INT_PA + REG_ID_40 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                    CLRREG16((BASE_REG_GPI_INT2_PA + REG_ID_20 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                }
                else
                {
                    SETREG16((BASE_REG_GPI_INT2_PA + REG_ID_7B + ((gpi_irq - GPI_GPIC_START) / 16) * 4),
                             (1 << ((gpi_irq - GPI_GPIC_START) % 16)));
                }
                break;
            case IRQ_TYPE_EDGE_RISING:
                if (gpi_irq < GPI_GPIC_START)
                {
                    CLRREG16((BASE_REG_GPI_INT_PA + REG_ID_30 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                    CLRREG16((BASE_REG_GPI_INT_PA + REG_ID_40 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                    CLRREG16((BASE_REG_GPI_INT2_PA + REG_ID_20 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                }
                else
                {
                    CLRREG16((BASE_REG_GPI_INT2_PA + REG_ID_7B + ((gpi_irq - GPI_GPIC_START) / 16) * 4),
                             (1 << ((gpi_irq - GPI_GPIC_START) % 16)));
                }
                break;
            case IRQ_TYPE_EDGE_BOTH:
                if (gpi_irq < GPI_GPIC_START)
                {
                    SETREG16((BASE_REG_GPI_INT_PA + REG_ID_40 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                    CLRREG16((BASE_REG_GPI_INT2_PA + REG_ID_20 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                }
                else
                {
                    return -EINVAL;
                }
                break;
            case IRQ_TYPE_LEVEL_LOW:
                if (gpi_irq < GPI_GPIC_START)
                {
                    SETREG16((BASE_REG_GPI_INT_PA + REG_ID_30 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                    SETREG16((BASE_REG_GPI_INT2_PA + REG_ID_20 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                }
                else
                {
                    return -EINVAL;
                }
                break;
            case IRQ_TYPE_LEVEL_HIGH:
                if (gpi_irq < GPI_GPIC_START)
                {
                    CLRREG16((BASE_REG_GPI_INT_PA + REG_ID_30 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                    SETREG16((BASE_REG_GPI_INT2_PA + REG_ID_20 + (gpi_irq / 16) * 4), (1 << (gpi_irq % 16)));
                }
                else
                {
                    return -EINVAL;
                }
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

/**
 * For linux, there are two features of "lazy disable" and "delayed interrupt disable" which meaning:
 * 1)only disabled current interrupt in software layer and maintain unmasked in hardware layer when an interrupt
 * triggered; 2)mask current interrupt in hardware layer if the current interrupt triggered once when it's status is
 * disabled in software layer; 3)check whether need to retrigger the interrupt and retrigger it or not; There are two
 * retrigger methods of hardware retrigger implemented by "xxx_INTC retrigger" and software retrigger implemented by
 * "resend_tasklet", and the two methods use infomation in linux 4.9/5.10 is:
 *
 * linux version   INTC_name       retrigger_method
 * 4.9.227         GPI_INTC        resend_tasklet
 * 4.9.227         MS_MSIN_INTC    resend_tasklet
 * 4.9.227         GIC_INTC        gic_retrigger
 *
 * 5.10.61         GPI_INTC        not support irq-retrigger, need modify, use resend_tasklet
 * 5.10.61         MS_MAIN_INTC    gic_retrigger
 * 5.10.61         GIC_INTC        gic_retrigger
 *
 * So, add an irq_retrigger interface to make GPI_INTC's irq-retrigger branch to resend_tasklet.
 */
static int sstar_gpi_irq_retrigger(struct irq_data *d)
{
    /* return error number(for irq_retrigger, 0:fail, 1:success) to make the judge of function try_retrigger() failed */
    return 0;
}

struct irq_chip sstar_gpi_chip = {
    .name          = "SSTAR_GPI_INTC",
    .irq_ack       = sstar_gpi_irq_ack,
    .irq_eoi       = sstar_gpi_irq_eoi,
    .irq_mask      = sstar_gpi_irq_mask,
    .irq_unmask    = sstar_gpi_irq_unmask,
    .irq_set_type  = sstar_gpi_irq_set_type,
    .irq_retrigger = sstar_gpi_irq_retrigger,
};

static void sstar_gpi_handle_irq(struct irq_desc *desc)
{
    unsigned int     i;
    unsigned int     cascade_irq = 0xFFFFFFFF;
    struct irq_chip *chip        = irq_desc_get_chip(desc);
    unsigned int     virq        = desc->irq_data.irq;
    unsigned int     final_status;

    for (i = GPI_FIQ_START; i < GPI_FIQ_END; i += 16)
    {
        final_status = INREG16(BASE_REG_GPI_INT_PA + REG_ID_50 + (i / 16) * 4);
        if (final_status)
        {
            cascade_irq = 0;
            while (!((final_status >> cascade_irq) & 0x1))
            {
                cascade_irq++;
            }
            cascade_irq += i;
            goto handle_int;
        }
    }

    final_status = INREG16(BASE_REG_GPI_INT2_PA + REG_ID_7C) & 0x3;
    if (final_status)
    {
        cascade_irq = 0;
        while (!((final_status >> cascade_irq) & 0x1))
        {
            cascade_irq++;
        }
        cascade_irq += GPI_GPIC_START;
        goto handle_int;
    }

    if (0xFFFFFFFF == cascade_irq)
    {
        pr_err("gpi status:%d 0x%04X virq:%d\n", cascade_irq, final_status, virq);
        // dump_stack();
        goto err;
    }

handle_int:
    virq = irq_find_mapping(sstar_gpi_data.domain, cascade_irq);
    if (!virq)
    {
        pr_err("gpi cascade_irq:%d\n", cascade_irq);
        // dump_stack();
        goto err;
    }

    pr_debug("%s %d final_status:%d 0x%04X virq:%d\n", __FUNCTION__, __LINE__, cascade_irq, final_status, virq);

    chained_irq_enter(chip, desc);
    generic_handle_irq(virq);
    chained_irq_exit(chip, desc);
    return;
err:
    chained_irq_enter(chip, desc);
    chained_irq_exit(chip, desc);
    return;
}

static int sstar_gpi_irq_domain_translate(struct irq_domain *domain, struct irq_fwspec *fwspec, unsigned long *hwirq,
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

static int sstar_gpi_irq_domain_alloc(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs, void *data)
{
    struct irq_fwspec *fwspec = data;
    irq_hw_number_t    hwirq;
    unsigned int       i;

    if (fwspec->param_count != 1)
        return -EINVAL;

    hwirq = fwspec->param[0];

    for (i = 0; i < nr_irqs; i++)
    {
        irq_domain_set_info(domain, virq + i, hwirq + i, &sstar_gpi_chip, NULL, handle_edge_irq, NULL, NULL);
        pr_debug("[%s] hw:%d -> v:%d\n", __FUNCTION__, (unsigned int)hwirq + i, virq + i);
        /*
         * GPIOs don't have an equivalent interrupt in the parent
         * controller (sstar_main_intc), so its interrupt hierarchy
         * stops at the gpi level
         */
        if (domain->parent)
            irq_domain_disconnect_hierarchy(domain->parent, virq + i);
    }

    return 0;
}

static void sstar_gpi_irq_domain_free(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs)
{
    unsigned int i;

    for (i = 0; i < nr_irqs; i++)
    {
        struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
        irq_domain_reset_irq_data(d);
    }
}

struct irq_domain_ops sstar_gpi_irq_domain_ops = {
    .translate = sstar_gpi_irq_domain_translate,
    .alloc     = sstar_gpi_irq_domain_alloc,
    .free      = sstar_gpi_irq_domain_free,
};

#ifdef CONFIG_PM_SLEEP
static int sstar_gpi_suspend(void)
{
    unsigned int i, num;

    num = (GPI_FIQ_NUM + 15) >> 4;
    for (i = 0; i < num; i++)
    {
        sstar_gpi_data.gpi_polarity[i] = INREG16(BASE_REG_GPI_INT_PA + REG_ID_30 + (i << 2));
        sstar_gpi_data.gpi_edge[i]     = INREG16(BASE_REG_GPI_INT_PA + REG_ID_40 + (i << 2));
        sstar_gpi_data.gpi_mask[i]     = INREG16(BASE_REG_GPI_INT_PA + REG_ID_00 + (i << 2));
    }

    num = (GPI_GPIC_NUM + 15) >> 4;
    for (i = 0; i < num; i++)
    {
        sstar_gpi_data.gpic_polarity[i] = INREG16(BASE_REG_GPI_INT2_PA + REG_ID_7B + (i << 2));
        sstar_gpi_data.gpic_mask[i]     = INREG16(BASE_REG_GPI_INT2_PA + REG_ID_78 + (i << 2));
    }

    pr_debug("sstar_gpi_suspend\n\n");
    return 0;
}

static void sstar_gpi_resume(void)
{
    unsigned int i, num;

    num = (GPI_FIQ_NUM + 15) >> 4;
    for (i = 0; i < num; i++)
    {
        SETREG16(BASE_REG_GPI_INT_PA + REG_ID_20 + (i << 2), 0xFFFF);
        OUTREG16(BASE_REG_GPI_INT_PA + REG_ID_30 + (i << 2), sstar_gpi_data.gpi_polarity[i]);
        OUTREG16(BASE_REG_GPI_INT_PA + REG_ID_40 + (i << 2), sstar_gpi_data.gpi_edge[i]);
        OUTREG16(BASE_REG_GPI_INT_PA + REG_ID_00 + (i << 2), sstar_gpi_data.gpi_mask[i]);
    }

    num = (GPI_GPIC_NUM + 15) >> 4;
    for (i = 0; i < num; i++)
    {
        SETREG16(BASE_REG_GPI_INT2_PA + REG_ID_7A + (i << 2), 0xFFFF);
        OUTREG16(BASE_REG_GPI_INT2_PA + REG_ID_7B + (i << 2), sstar_gpi_data.gpic_polarity[i]);
        OUTREG16(BASE_REG_GPI_INT2_PA + REG_ID_78 + (i << 2), sstar_gpi_data.gpic_mask[i]);
    }

    pr_debug("sstar_gpi_resume\n\n");
}

struct syscore_ops sstar_gpi_syscore_ops = {
    .suspend = sstar_gpi_suspend,
    .resume  = sstar_gpi_resume,
};
#endif

static int __init sstar_gpi_init(struct device_node *np, struct device_node *interrupt_parent)
{
    int                    parent_gpi_virq = 0;
    struct irq_domain *    parent_domain;
    struct of_phandle_args irq_handle = {0};

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

    sstar_gpi_data.domain =
        irq_domain_add_hierarchy(parent_domain, 0, GPI_FIQ_NUM + GPI_GPIC_NUM, np, &sstar_gpi_irq_domain_ops, NULL);

    if (!sstar_gpi_data.domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }

    if (of_irq_parse_one(np, 0, &irq_handle))
        pr_err("%s: %s of_irq_parse_one fail\n", __func__, np->name);

    sstar_gpi_data.parent_gpi_hwirq = irq_handle.args[1];

    parent_gpi_virq = irq_create_of_mapping(&irq_handle);
    if (!parent_gpi_virq)
    {
        pr_err("Get irq err from DTS\n");
        return -EPROBE_DEFER;
    }
    sstar_gpi_data.parent_gpi_virq = parent_gpi_virq;
    irq_set_chained_handler_and_data(parent_gpi_virq, sstar_gpi_handle_irq, sstar_gpi_data.domain);

#ifdef CONFIG_PM_SLEEP
    register_syscore_ops(&sstar_gpi_syscore_ops);
#endif
    return 0;
}

IRQCHIP_DECLARE(sstar_gpi_intc, "sstar,gpi-intc", sstar_gpi_init);
