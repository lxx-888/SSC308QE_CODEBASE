/*
 * os_uart.h- Sigmastar
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

#ifndef __OS_UART__
#define __OS_UART__

#include <include/fdtdec.h>
#include <include/linux/delay.h>

#ifdef BASE_REG_RIU_PA
#define UART_BASE_RIU_PA BASE_REG_RIU_PA
#else
#define UART_BASE_RIU_PA (0x1F000000)
#endif

#ifdef BASE_REG_IMI_PA
#define UART_BASE_IMI_PA BASE_REG_IMI_PA
#else
#define UART_BASE_IMI_PA (0xA0000000)
#endif

#define uart_mem_flush(buf, size)
#define uart_mem_invalidate(buf, size)
#define uart_miu_pipe_flush()

#define HAL_UART_IMPL_USDELAY(x) udelay(x)

typedef fdt_addr_t uart_addr_t;

#endif
