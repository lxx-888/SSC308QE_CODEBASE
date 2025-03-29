/*
 * irq-pm-main.c- Sigmastar
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
#include <linux/irq.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <irqs.h>
#include <pmsleep-irqs.h>
#include <registers.h>
#include <ms_platform.h>
#include <ms_types.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

/**
 * struct sstar_pm_main - private pm interrupt data
 * @fiq_flags:      Flags of each pm-gpio fiq
 * @irq_polarity:   Polarity of pm-sleep irq
 */
struct sstar_pm_main
{
    u16                irq_mask[(PMSLEEP_IRQ_NR) >> 4];
    u16                fiq_mask[(PMSLEEP_FIQ_NR) >> 4];
    u16                irq_polarity[(PMSLEEP_IRQ_NR) >> 4];
    u16                fiq_polarity[(PMSLEEP_FIQ_NR) >> 4];
    u32                parent_hwirq_irq;
    u32                parent_hwirq_fiq;
    u32                parent_virq_irq;
    u32                parent_virq_fiq;
    struct irq_domain *domain;
};

static struct sstar_pm_main sstar_pm_main_data;
static DEFINE_SPINLOCK(sstar_pm_main_irq_controller_lock);

static void sstar_pm_main_irq_ack(struct irq_data *d)
{
    s16 ms_fiq;

    ms_fiq = d->hwirq;

    if (ms_fiq >= 0 && ms_fiq < PMSLEEP_FIQ_END)
    {
        OUTREG16((BASE_REG_IRQ_PA + REG_ID_0C + (ms_fiq / 16) * 4), (1 << (ms_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (ms_fiq >= PMSLEEP_FIQ_NR)
    {
        pr_err("[%s] Unknown hwirq %lu, fiq %d\n", __func__, d->hwirq, ms_fiq);
        return;
    }
}

static void sstar_pm_main_irq_eoi(struct irq_data *d)
{
    s16 ms_fiq;

    ms_fiq = d->hwirq;

    if (ms_fiq >= 0 && ms_fiq < PMSLEEP_FIQ_END)
    {
        OUTREG16((BASE_REG_IRQ_PA + REG_ID_0C + (ms_fiq / 16) * 4), (1 << (ms_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (ms_fiq >= PMSLEEP_IRQ_END)
    {
        pr_err("[%s] Unknown hwirq %lu, fiq %d\n", __func__, d->hwirq, ms_fiq);
        return;
    }
}

static void sstar_pm_main_irq_mask(struct irq_data *d)
{
    s16 ms_irq;
    s16 ms_fiq;

    ms_irq = d->hwirq - PMSLEEP_FIQ_NR;
    ms_fiq = d->hwirq;

    if (ms_fiq >= 0 && ms_fiq < PMSLEEP_FIQ_END)
    {
        SETREG16_BIT_OP((BASE_REG_IRQ_PA + REG_ID_04 + (ms_fiq / 16) * 4), (1 << (ms_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (ms_irq >= 0 && ms_irq < PMSLEEP_IRQ_NR)
    {
        SETREG16_BIT_OP((BASE_REG_IRQ_PA + REG_ID_14 + (ms_irq / 16) * 4), (1 << (ms_irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void sstar_pm_main_irq_unmask(struct irq_data *d)
{
    s16 ms_irq;
    s16 ms_fiq;

    ms_irq = d->hwirq - PMSLEEP_FIQ_NR;
    ms_fiq = d->hwirq;

    if (ms_fiq >= 0 && ms_fiq < PMSLEEP_FIQ_END)
    {
        CLRREG16_BIT_OP((BASE_REG_IRQ_PA + REG_ID_04 + (ms_fiq / 16) * 4), (1 << (ms_fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (ms_irq >= 0 && ms_irq < PMSLEEP_IRQ_NR)
    {
        CLRREG16_BIT_OP((BASE_REG_IRQ_PA + REG_ID_14 + (ms_irq / 16) * 4), (1 << (ms_irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static int sstar_pm_main_irq_set_type(struct irq_data *data, unsigned int flow_type)
{
    s16 ms_irq;
    s16 ms_fiq;

    if ((flow_type & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH)
    {
        pr_err("Not support IRQ_TYPE_EDGE_BOTH mode 0x%x\n", flow_type);
        return 0;
    }

    ms_irq = data->hwirq - PMSLEEP_FIQ_NR;
    ms_fiq = data->hwirq;

    if (ms_fiq >= 0 && ms_fiq < PMSLEEP_FIQ_END)
    {
        if (flow_type & IRQ_TYPE_EDGE_FALLING)
            SETREG16((BASE_REG_IRQ_PA + REG_ID_08 + (ms_fiq / 16) * 4), (1 << (ms_fiq % 16)));
        else
            CLRREG16((BASE_REG_IRQ_PA + REG_ID_08 + (ms_fiq / 16) * 4), (1 << (ms_fiq % 16)));
    }
    else if (ms_irq >= 0 && ms_irq < PMSLEEP_IRQ_END)
    {
        if (flow_type & IRQ_TYPE_LEVEL_LOW)
            SETREG16((BASE_REG_IRQ_PA + REG_ID_18 + (ms_irq / 16) * 4), (1 << (ms_irq % 16)));
        else
            CLRREG16((BASE_REG_IRQ_PA + REG_ID_18 + (ms_irq / 16) * 4), (1 << (ms_irq % 16)));
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, data->hwirq);
        return -EINVAL;
    }
    return 0;
}

static struct irq_chip sstar_pm_main_chip = {
    .name         = "SSTAR_PM_MAIN_INTC",
    .irq_ack      = sstar_pm_main_irq_ack,
    .irq_eoi      = sstar_pm_main_irq_eoi,
    .irq_mask     = sstar_pm_main_irq_mask,
    .irq_unmask   = sstar_pm_main_irq_unmask,
    .irq_set_type = sstar_pm_main_irq_set_type,
};

static void sstar_pm_main_handle_irq(struct irq_desc *desc)
{
    unsigned int     i;
    unsigned int     cascade_irq  = 0xFFFFFFFF;
    struct irq_chip *chip         = irq_desc_get_chip(desc);
    unsigned int     virq         = desc->irq_data.irq;
    unsigned short   final_status = 0;

    spin_lock(&sstar_pm_main_irq_controller_lock);
    for (i = PMSLEEP_FIQ_START; i < PMSLEEP_FIQ_END; i += 16)
    {
        final_status = INREG16(BASE_REG_IRQ_PA + REG_ID_0C + (i / 16) * 4);
        if (final_status)
        {
            cascade_irq = 0;
            while (!((final_status >> cascade_irq) & 0x1))
            {
                cascade_irq++;
            }
            cascade_irq += i;
            pr_debug("[%s] Get hwirq:%d, Reg:0x%04x\n", __FUNCTION__, cascade_irq, final_status);
            break;
        }
    }
    if (cascade_irq == 0xFFFFFFFF)
    {
        for (i = PMSLEEP_IRQ_START; i < PMSLEEP_IRQ_END; i += 16)
        {
            final_status = INREG16(BASE_REG_IRQ_PA + REG_ID_1C + ((i - PMSLEEP_IRQ_START) / 16) * 4);
            if (final_status)
            {
                cascade_irq = 0;
                while (!((final_status >> cascade_irq) & 0x1))
                {
                    cascade_irq++;
                }
                cascade_irq += i;
                pr_debug("[%s] Get hwirq:%d, Reg:0x%04x\n", __FUNCTION__, cascade_irq, final_status);
                break;
            }
        }
    }
    spin_unlock(&sstar_pm_main_irq_controller_lock);

    if (cascade_irq == 0xFFFFFFFF)
    {
        pr_err("[%s:%d] err final status: 0x%04X virq:%d\n", __FUNCTION__, __LINE__, final_status, virq);
        // dump_stack();
        goto err;
    }

    virq = irq_find_mapping(sstar_pm_main_data.domain, cascade_irq);
    if (!virq)
    {
        pr_err("[%s] err %d cascade_irq:%d\n", __FUNCTION__, __LINE__, cascade_irq);
        dump_stack();
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

static int sstar_pm_main_irq_domain_translate(struct irq_domain *domain, struct irq_fwspec *fwspec,
                                              unsigned long *hwirq, unsigned int *type)
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

static int sstar_pm_main_irq_domain_alloc(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs,
                                          void *data)
{
    struct irq_fwspec *fwspec = data;
    irq_hw_number_t    hwirq;
    unsigned int       i;

    if (fwspec->param_count != 1)
        return -EINVAL;

    hwirq = fwspec->param[0];

    for (i = 0; i < nr_irqs; i++)
    {
        irq_domain_set_info(domain, virq + i, hwirq + i, &sstar_pm_main_chip, NULL, handle_fasteoi_irq, NULL, NULL);
        pr_debug("[SSTAR_PM_MAIN_INTC] hw:%d -> v:%d\n", (unsigned int)hwirq + i, virq + i);
        if (domain->parent)
            irq_domain_disconnect_hierarchy(domain->parent, virq + i);
    }

    return 0;
}

static void sstar_pm_main_irq_domain_free(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs)
{
    unsigned int i;

    for (i = 0; i < nr_irqs; i++)
    {
        struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
        irq_domain_reset_irq_data(d);
    }
}

struct irq_domain_ops sstar_pm_main_irq_domain_ops = {
    .translate = sstar_pm_main_irq_domain_translate,
    .alloc     = sstar_pm_main_irq_domain_alloc,
    .free      = sstar_pm_main_irq_domain_free,
};

#ifdef CONFIG_PM_SLEEP
static int sstar_pm_main_suspend(void)
{
    unsigned int i, num;

    num = (PMSLEEP_IRQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        if (i >= 0)
        {
            sstar_pm_main_data.irq_mask[i]     = INREG16(BASE_REG_IRQ_PA + REG_ID_14 + (i << 2));
            sstar_pm_main_data.irq_polarity[i] = INREG16(BASE_REG_IRQ_PA + REG_ID_18 + (i << 2));
        }
    }
    num = (GIC_SPI_MS_FIQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        sstar_pm_main_data.fiq_mask[i]     = INREG16(BASE_REG_IRQ_PA + REG_ID_04 + (i << 2));
        sstar_pm_main_data.fiq_polarity[i] = INREG16(BASE_REG_IRQ_PA + REG_ID_08 + (i << 2));
    }

    pr_debug("sstar_pm_main_suspend\n\n");
    return 0;
}

static void sstar_pm_main_resume(void)
{
    unsigned int i, num;

    num = (PMSLEEP_IRQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        if (i >= 0)
        {
            OUTREG16(BASE_REG_IRQ_PA + REG_ID_14 + (i << 2), sstar_pm_main_data.irq_mask[i]);
            OUTREG16(BASE_REG_IRQ_PA + REG_ID_18 + (i << 2), sstar_pm_main_data.irq_polarity[i]);
        }
    }

    num = (PMSLEEP_FIQ_NR) >> 4;
    for (i = 0; i < num; i++)
    {
        OUTREG16(BASE_REG_IRQ_PA + REG_ID_04 + (i << 2), sstar_pm_main_data.fiq_mask[i]);
        OUTREG16(BASE_REG_IRQ_PA + REG_ID_08 + (i << 2), sstar_pm_main_data.fiq_polarity[i]);
    }

    pr_debug("sstar_pm_main_resume\n\n");
}

struct syscore_ops sstar_pm_main_syscore_ops = {
    .suspend = sstar_pm_main_suspend,
    .resume  = sstar_pm_main_resume,
};
#endif

static int __init sstar_pm_main_init(struct device_node *np, struct device_node *interrupt_parent)
{
    int                    parent_virtirq = 0;
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

    sstar_pm_main_data.domain = irq_domain_add_hierarchy(parent_domain, 0, PMSLEEP_FIQ_NR + PMSLEEP_IRQ_NR, np,
                                                         &sstar_pm_main_irq_domain_ops, NULL);

    if (!sstar_pm_main_data.domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }

    if (of_irq_parse_one(np, 0, &irq_handle))
        pr_err("%s: %s of_irq_parse_one fail\n", __func__, np->name);

    sstar_pm_main_data.parent_hwirq_fiq = irq_handle.args[1];
    parent_virtirq                      = irq_create_of_mapping(&irq_handle);
    if (!parent_virtirq)
    {
        pr_err("Get virq err\n");
        return -EPROBE_DEFER;
    }
    sstar_pm_main_data.parent_virq_fiq = parent_virtirq;
    irq_set_chained_handler_and_data(parent_virtirq, sstar_pm_main_handle_irq, sstar_pm_main_data.domain);

    if (of_irq_parse_one(np, 1, &irq_handle))
        pr_err("%s: %s of_irq_parse_one fail\n", __func__, np->name);

    sstar_pm_main_data.parent_hwirq_irq = irq_handle.args[1];
    parent_virtirq                      = irq_create_of_mapping(&irq_handle);
    if (!parent_virtirq)
    {
        pr_err("Get virq err\n");
        return -EPROBE_DEFER;
    }
    sstar_pm_main_data.parent_virq_irq = parent_virtirq;
    irq_set_chained_handler_and_data(parent_virtirq, sstar_pm_main_handle_irq, sstar_pm_main_data.domain);

#ifdef CONFIG_PM_SLEEP
    register_syscore_ops(&sstar_pm_main_syscore_ops);
#endif
    return 0;
}

IRQCHIP_DECLARE(sstar_pm_main_intc, "sstar,pm-main-intc", sstar_pm_main_init);
