/*
 * sstar_swla_export.c - Sigmastar
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
#include "sstar_swla.h"

EXPORT_SYMBOL(sys_swla_init);
EXPORT_SYMBOL(sys_swla_start);
EXPORT_SYMBOL(sys_swla_stop);
EXPORT_SYMBOL(sys_swla_log_add_event);
EXPORT_SYMBOL(sys_swla_log_add_irq);
EXPORT_SYMBOL(sys_swla_dump);
