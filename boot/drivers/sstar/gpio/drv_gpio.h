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

#ifdef __cplusplus
extern "C"
{
#endif

#include "sstar_types.h"

    typedef struct
    {
        u32 reg_out;
        u16 mask_out;
    } drv_gpio_reg_info;

    void  sstar_gpio_init(void);
    void  sstar_gpio_pad_set(U8 gpio_index);
    void  sstar_gpio_pad_clr(U8 gpio_index);
    U8    sstar_gpio_padgroupmode_set(U32 pad_mode);
    U8    sstar_gpio_pad_val_set(U8 gpio_index, U32 pad_mode);
    U8    sstar_gpio_pad_val_get(U8 gpio_index, U32* pad_mode);
    void  sstar_gpio_vol_val_set(U8 group, U32 mode);
    U8    sstar_gpio_pad_val_check(U8 gpio_index, U32 pad_mode);
    U8    sstar_gpio_pad_oen(U8 gpio_index);
    U8    sstar_gpio_pad_odn(U8 gpio_index);
    U8    sstar_gpio_pad_read(U8 gpio_index, U8* pad_level);
    U8    sstar_gpio_pad_in_out(U8 gpio_index, U8* pad_in_out);
    U8    sstar_gpio_pull_high(U8 gpio_index);
    U8    sstar_gpio_pull_low(U8 gpio_index);
    U8    sstar_gpio_pull_up(U8 gpio_index);
    U8    sstar_gpio_pull_down(U8 gpio_index);
    U8    sstar_gpio_pull_off(U8 gpio_index);
    U8    sstar_gpio_pull_status(U8 gpio_index, U8* pull_status);
    U8    sstar_gpio_set_high(U8 gpio_index);
    U8    sstar_gpio_set_low(U8 gpio_index);
    U8    sstar_gpio_drv_set(U8 gpio_index, U8 level);
    U8    sstar_gpio_drv_get(U8 gpio_index, U8* level);
    int   sstar_gpio_to_irq(U8 gpio_index);
    U8    sstar_gpio_get_check_count(void);
    void* sstar_gpio_get_check_info(U8 index);
    U8    sstar_gpio_name_to_num(U8* p_name, U8* gpio_index);
    U8    sstar_gpio_num_to_name(U8 gpio_index, const U8** p_name);
    U8    sstar_gpio_padmode_to_val(U8* p_mode, U32* mode_to_val);
    U32*  sstar_gpio_padmode_to_padindex(U32 mode);
    u8    sstar_gpio_get_reg_cfg(u8 gpio_index, void* reg_cfg);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_GPIO_H_ */
