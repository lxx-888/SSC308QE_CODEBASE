/*
 * hal_gpio_common.h- Sigmastar
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

#ifndef _HAL_GPIO_COM_H_
#define _HAL_GPIO_COM_H_

#include <hal_pinmux.h>

struct gpio_setting
{
    u8  p_name[32];
    u32 r_oen;
    u16 m_oen;
    u32 r_out;
    u16 m_out;
    u32 r_in;
    u16 m_in;
    u32 r_drv;
    u16 m_drv;
    u32 r_pe;
    u16 m_pe;
    u32 r_ps;
    u16 m_ps;
};

extern const struct gpio_setting gpio_table[];
extern const u32                 gpio_table_size;

#endif
