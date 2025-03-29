/*
 * drv_gpio.h- Sigmastar
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
#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

#include <mdrv_types.h>

typedef struct drv_gpio_pad_check
{
    u16 base;
    u16 offset;
    u16 mask;
    u16 val;
    u16 regval;
} drv_gpio_pad_check_info;

typedef struct
{
    u32 reg_out;
    u16 mask_out;
} drv_gpio_reg_info;

void  sstar_gpio_init(void);
void  sstar_gpio_pad_set(u8 gpio_index);
void  sstar_gpio_pad_clr(u8 gpio_index);
u8    sstar_gpio_padgroupmode_set(u32 pad_mode);
u8    sstar_gpio_pad_val_set(u8 gpio_index, u32 pad_mode);
u8    sstar_gpio_pad_val_get(u8 gpio_index, u32* pad_mode);
void  sstar_gpio_vol_val_set(u8 group, u32 mode);
u8    sstar_gpio_pad_val_check(u8 gpio_index, u32 pad_mode);
u8    sstar_gpio_pad_oen(u8 gpio_index);
u8    sstar_gpio_pad_odn(u8 gpio_index);
u8    sstar_gpio_pad_read(u8 gpio_index, u8* pad_level);
u8    sstar_gpio_pad_in_out(u8 gpio_index, u8* pad_in_out);
u8    sstar_gpio_pull_high(u8 gpio_index);
u8    sstar_gpio_pull_low(u8 gpio_index);
u8    sstar_gpio_pull_up(u8 gpio_index);
u8    sstar_gpio_pull_down(u8 gpio_index);
u8    sstar_gpio_pull_off(u8 gpio_index);
u8    sstar_gpio_pull_status(u8 gpio_index, u8* pull_status);
u8    sstar_gpio_set_high(u8 gpio_index);
u8    sstar_gpio_set_low(u8 gpio_index);
u8    sstar_gpio_drv_set(u8 gpio_index, u8 level);
u8    sstar_gpio_drv_get(u8 gpio_index, u8* level);
int   sstar_gpio_to_irq(u8 gpio_index);
u8    sstar_gpio_get_check_count(void);
void* sstar_gpio_get_check_info(u8 index);
u8    sstar_gpio_name_to_num(u8* p_name, u8* gpio_index);
u8    sstar_gpio_num_to_name(u8 gpio_index, const u8** p_name);
u8    sstar_gpio_padmode_to_val(u8* p_mode, u32* mode_to_val);
u32*  sstar_gpio_padmode_to_padindex(u32 mode);
u8    sstar_gpio_get_reg_cfg(u8 gpio_index, void* reg_cfg);

#endif // _DRV_GPIO_H_
