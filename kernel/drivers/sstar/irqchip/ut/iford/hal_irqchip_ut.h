/*
 * hal_irqchip_ut.h- Sigmastar
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

#ifndef _HAL_IRQCHIP_UT_H_
#define _HAL_IRQCHIP_UT_H_

int main_intc_force_irq(int hwirq, int force);
int main_intc_mask_irq(int hwirq);

int pm_main_intc_force_irq(int hwirq, int force);
int pm_main_intc_mask_irq(int hwirq);

int gpi_intc_force_irq(int hwirq, int force);
int gpi_intc_mask_irq(int hwirq);

int pm_gpi_intc_force_irq(int hwirq, int force);
int pm_gpi_intc_mask_irq(int hwirq);

#endif /* _HAL_IRQCHIP_UT_H_ */
