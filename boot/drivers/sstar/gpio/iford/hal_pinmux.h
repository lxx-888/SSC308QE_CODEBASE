/*
 * hal_pinmux.h- Sigmastar
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
#ifndef __HAL_PINMUX_H__
#define __HAL_PINMUX_H__

extern s32   hal_gpio_pad_set_val(u32 pad_id, u32 mode);
extern s32   hal_gpio_pad_clr_val(u32 pad_id, u32 mode);
extern s32   hal_gpio_pad_get_val(u32 pad_id, u32* mode);
extern s32   hal_gpio_pad_check_val(u32 pad_id, u32 mode);
extern s32   hal_gpio_pad_set_mode(u32 mode);
extern u8    hal_gpio_pad_check_info_count(void);
extern void* hal_gpio_pad_check_info_get(u8 index);
extern u8    hal_gpio_padmux_to_val(u8* p_mode, u32* mode_to_val);
extern void  hal_gpio_set_vol(u32 group, u32 mode);
extern u32*  hal_gpio_padmdoe_to_padindex(u32 mode);

#endif // __HAL_PINMUX_H__
