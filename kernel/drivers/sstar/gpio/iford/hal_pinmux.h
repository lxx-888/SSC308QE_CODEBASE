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
#ifndef __MHAL_PINMUX_H__
#define __MHAL_PINMUX_H__

#include <gpio_os.h>

typedef struct hal_gpio_st_padmux
{
    u16 pad_id;
    u32 base;
    u16 offset;
    u16 mask;
    u16 val;
    u16 mode;
} hal_gpio_st_padmux_info;

typedef struct hal_gpio_st_padmode
{
    u8  pad_name[32];
    u64 mode_riu;
    u16 mode_mask;
    u16 mode_val;
} hal_gpio_st_padmode_info;

typedef struct hal_gpio_st_pad_check
{
    u16 base;
    u16 offset;
    u16 mask;
    u16 val;
    u16 regval;
} hal_gpio_st_pad_check_info;

typedef struct hal_gpio_st_pad_checkVal
{
    u8                           infocount;
    struct hal_gpio_st_pad_check infos[64];
} hal_gpio_st_pad_check_v;

typedef struct hal_gpio_st_padmux_entry
{
    u32                            size;
    const hal_gpio_st_padmux_info* padmux;
} hal_gpio_st_padmux_en;

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

extern const hal_gpio_st_padmux_en m_hal_gpio_st_padmux_entry[];
extern const u32                   hal_gpio_st_padmux_size;

extern const hal_gpio_st_padmode_info m_hal_gpio_st_padmode_info_tbl[];
extern const u32                      hal_gpio_st_padmode_size;

extern hal_gpio_st_pad_check_v m_hal_gpio_st_pad_checkVal;

#endif // __MHAL_PINMUX_H__
