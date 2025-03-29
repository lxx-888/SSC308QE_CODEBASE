/*
 * hal_irqchip_ut.c- Sigmastar
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

#include <irqs.h>
#include <ms_platform.h>
#include <registers.h>

int main_intc_force_irq(int hwirq, int force)
{
    short irq;
    short fiq;

    irq = hwirq - GIC_SPI_ARM_INTERNAL_NR;
    fiq = hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if (fiq >= 0 && fiq < GIC_SPI_MS_FIQ_NR)
    {
        if (force)
            SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_40 + (fiq / 16) * 4), (1 << (fiq % 16)));
        else
            CLRREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_40 + (fiq / 16) * 4), (1 << (fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (irq >= 0 && irq < GIC_SPI_MS_IRQ_NR)
    {
        if (force)
            SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_50 + (irq / 16) * 4), (1 << (irq % 16)));
        else
            CLRREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_50 + (irq / 16) * 4), (1 << (irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}

int main_intc_mask_irq(int hwirq)
{
    short irq;
    short fiq;

    irq = hwirq - GIC_SPI_ARM_INTERNAL_NR;
    fiq = hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if (fiq >= 0 && fiq < GIC_SPI_MS_FIQ_NR)
    {
        SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_44 + (fiq / 16) * 4), (1 << (fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (irq >= 0 && irq < GIC_SPI_MS_IRQ_NR)
    {
        SETREG16_BIT_OP((BASE_REG_INTRCTL_PA + REG_ID_54 + (irq / 16) * 4), (1 << (irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}

int pm_main_intc_force_irq(int hwirq, int force)
{
    s16 irq;
    s16 fiq;

    irq = hwirq - PMSLEEP_FIQ_NR;
    fiq = hwirq;

    if (fiq >= 0 && fiq < PMSLEEP_FIQ_END)
    {
        if (force)
            SETREG16((BASE_REG_IRQ_PA + REG_ID_00 + (fiq / 16) * 4), (1 << (fiq % 16)));
        else
            CLRREG16((BASE_REG_IRQ_PA + REG_ID_00 + (fiq / 16) * 4), (1 << (fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (irq >= 0 && irq < PMSLEEP_IRQ_NR)
    {
        if (force)
            SETREG16((BASE_REG_IRQ_PA + REG_ID_10 + (irq / 16) * 4), (1 << (irq % 16)));
        else
            CLRREG16((BASE_REG_IRQ_PA + REG_ID_10 + (irq / 16) * 4), (1 << (irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}

int pm_main_intc_mask_irq(int hwirq)
{
    s16 irq;
    s16 fiq;

    irq = hwirq - PMSLEEP_FIQ_NR;
    fiq = hwirq;

    if (fiq >= 0 && fiq < PMSLEEP_FIQ_END)
    {
        SETREG16((BASE_REG_IRQ_PA + REG_ID_04 + (fiq / 16) * 4), (1 << (fiq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (irq >= 0 && irq < PMSLEEP_IRQ_NR)
    {
        SETREG16((BASE_REG_IRQ_PA + REG_ID_14 + (irq / 16) * 4), (1 << (irq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}

int gpi_intc_force_irq(int hwirq, int force)
{
    if (hwirq < GPI_FIQ_END)
    {
        if (force)
            SETREG16_BIT_OP((BASE_REG_GPI_INT_PA + REG_ID_10 + (hwirq / 16) * 4), (1 << (hwirq % 16)));
        else
            CLRREG16_BIT_OP((BASE_REG_GPI_INT_PA + REG_ID_10 + (hwirq / 16) * 4), (1 << (hwirq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (hwirq < GPI_GPIC_END)
    {
        if (force)
            SETREG16_BIT_OP((BASE_REG_GPI_INT2_PA + REG_ID_79 + ((hwirq - GPI_GPIC_START) / 16) * 4),
                            (1 << ((hwirq - GPI_GPIC_START) % 16)));
        else
            CLRREG16_BIT_OP((BASE_REG_GPI_INT2_PA + REG_ID_79 + ((hwirq - GPI_GPIC_START) / 16) * 4),
                            (1 << ((hwirq - GPI_GPIC_START) % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}

int gpi_intc_mask_irq(int hwirq)
{
    if (hwirq < GPI_FIQ_END)
    {
        SETREG16_BIT_OP((BASE_REG_GPI_INT_PA + REG_ID_00 + (hwirq / 16) * 4), (1 << (hwirq % 16)));
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else if (hwirq < GPI_GPIC_END)
    {
        SETREG16_BIT_OP((BASE_REG_GPI_INT2_PA + REG_ID_78 + ((hwirq - GPI_GPIC_START) / 16) * 4),
                        (1 << ((hwirq - GPI_GPIC_START) % 16)));
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}

int pm_gpi_intc_force_irq(int hwirq, int force)
{
    if (hwirq < PM_GPI_FIQ_END)
    {
        if (force)
            SETREG16(BASE_REG_PMGPIO_PA + ((hwirq - PM_GPI_FIQ_START) << 2), BIT5);
        else
            CLRREG16(BASE_REG_PMGPIO_PA + ((hwirq - PM_GPI_FIQ_START) << 2), BIT5);

        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}

int pm_gpi_intc_mask_irq(int hwirq)
{
    if (hwirq < PM_GPI_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + ((hwirq - PM_GPI_FIQ_START) << 2), BIT4);
        INREG16(BASE_REG_MAILBOX_PA); // read a register make ensure the previous write command was compeleted
    }
    else
    {
        pr_err("hirq:%d unknown\t\t\t---FAILED\n", hwirq);
        return -1;
    }

    return 0;
}
