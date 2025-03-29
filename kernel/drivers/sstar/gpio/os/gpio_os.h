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

#include <linux/mm.h>
#include <ms_platform.h>
#include <mdrv_types.h>
#include <cam_os_wrapper.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <linux/irqdomain.h>
#include <irqs.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <generated/autoconf.h>

#define GPI_TYPE           1
#define GPIO_IO_ADDRESS(x) IO_ADDRESS(x)

#define hal_pinmux_info(fmt, arg...) CamOsPrintf(KERN_INFO fmt, ##arg)
#define gpio_print(fmt, ...)         CamOsPrintf(fmt, ##__VA_ARGS__)

#define _GPIO_W_WORD(addr, val)                           \
    {                                                     \
        (*(volatile u16*)(uintptr_t)(addr)) = (u16)(val); \
    }
#define _GPIO_W_WORD_MASK(addr, val, mask)                                                                             \
    {                                                                                                                  \
        (*(volatile u16*)(uintptr_t)(addr)) = ((*(volatile u16*)(uintptr_t)(addr)) & ~(mask)) | ((u16)(val) & (mask)); \
    }
#define _GPIO_R_BYTE(addr)            (*(volatile u8*)(uintptr_t)(addr))
#define _GPIO_R_WORD_MASK(addr, mask) ((*(volatile u16*)(uintptr_t)(addr)) & (mask))

#endif
