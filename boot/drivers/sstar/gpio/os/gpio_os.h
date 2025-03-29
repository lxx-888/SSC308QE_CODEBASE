/*
 * gpio_os.h- Sigmastar
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

#ifndef __GPIO_OS__
#define __GPIO_OS__

#include <common.h>
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/sstar_types.h>
#include <asm/arch/mach/io.h>
#include <linux/string.h>

#define GPI_TYPE        1
#define GPIO_INT        0
#define BASE_REG_RIU_PA 0x1F000000

#define GPIO_IO_ADDRESS(x) IO_ADDRESS(x)

#define hal_pinmux_info(fmt, arg...) printf(fmt, ##arg)
#define gpio_print(fmt, ...)         printf(fmt, ##__VA_ARGS__)

#endif
