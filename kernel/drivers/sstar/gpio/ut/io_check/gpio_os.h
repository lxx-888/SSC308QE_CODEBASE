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

#ifndef GPIO_OS_H
#define GPIO_OS_H

#include <stdio.h>
#include <mdrv_types.h>
#include <string.h>
#include <riu.h>

typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;
typedef unsigned long      uintptr_t;

#define hal_pinmux_info    printf
#define GPIO_IO_ADDRESS(x) 0

#define _GPIO_W_WORD(addr, val)            riu_write(addr, val, 0, 0)
#define _GPIO_W_WORD_MASK(addr, val, mask) riu_write(addr, val, mask, 1)
#define _GPIO_R_BYTE(addr)                 riu_read(addr, 0, 0)
#define _GPIO_R_WORD_MASK(addr, mask)      riu_read(addr, mask, 1)

#endif
