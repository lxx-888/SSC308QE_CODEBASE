/*
 * hal_gpio.h- Sigmastar
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
#ifndef _MHAL_GPIO_H_
#define _MHAL_GPIO_H_

typedef enum
{
    MHAL_PULL_UP = 0,
    MHAL_PULL_DOWN,
    MHAL_PULL_OFF,
} hal_pad_pull;

extern void  hal_gpio_init(void);
extern void  hal_gpio_pad_set(u8 gpio_index);
extern void  hal_gpio_pad_clr(u8 gpio_index);
extern u8    hal_gpio_padgroupmode_set(u32 pad_mode);
extern u8    hal_gpio_pad_val_set(u8 gpio_index, u32 pad_mode);
extern u8    hal_gpio_pad_val_get(u8 gpio_index, u32 *pad_mode);
extern void  hal_gpio_vol_val_set(u8 group, u32 mode);
extern u8    hal_gpio_pad_val_check(u8 gpio_index, u32 pad_mode);
extern u8    hal_gpio_pad_oen(u8 gpio_index);
extern u8    hal_gpio_pad_odn(u8 gpio_index);
extern u8    hal_gpio_pad_level(u8 gpio_index, u8 *pad_level);
extern u8    hal_gpio_pad_in_out(u8 gpio_index, u8 *pad_in_out);
extern u8    hal_gpio_pull_high(u8 gpio_index);
extern u8    hal_gpio_pull_low(u8 gpio_index);
extern u8    hal_gpio_pull_up(u8 gpio_index);
extern u8    hal_gpio_pull_down(u8 gpio_index);
extern u8    hal_gpio_pull_off(u8 gpio_index);
extern u8    hal_gpio_pull_status(u8 gpio_index, u8 *pull_status);
extern u8    hal_gpio_set_high(u8 gpio_index);
extern u8    hal_gpio_set_low(u8 gpio_index);
extern u8    hal_gpio_drv_set(u8 gpio_index, u8 level);
extern u8    hal_gpio_drv_get(u8 gpio_index, u8 *u8level);
extern int   hal_gpio_to_irq(u8 gpio_index);
extern u8    hal_gpio_get_check_count(void);
extern void *hal_gpio_get_check_info(u8 index);
extern u8    hal_gpio_name_to_num(u8 *p_name, u8 *gpio_index);
extern u8    hal_gpio_num_to_name(u8 gpio_index, const u8 **p_name);
extern u8    hal_gpio_padmode_to_val(u8 *p_mode, u32 *mode_to_val);
extern int   hal_gpio_gpi_to_irq(u8 gpio_index);
extern u32 * hal_gpio_padmode_to_padindex(u32 mode);

#endif // _MHAL_GPIO_H_
