/*
 * drv_gpio.c- Sigmastar
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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <drv_gpio.h>
#include <hal_gpio.h>

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// GPIO chiptop initialization
/// @return None
/// @note   Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void sstar_gpio_init(void)
{
    hal_gpio_init();
}

//-------------------------------------------------------------------------------------------------
/// select one pad to set to GPIO mode
/// @param  gpio_index              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void sstar_gpio_pad_set(u8 gpio_index)
{
    hal_gpio_pad_set(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// select one pad to clear GPIO mode
/// @param  gpio_index              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void sstar_gpio_pad_clr(u8 gpio_index)
{
    hal_gpio_pad_clr(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// set the specified pad mode( a set of GPIO pad will be effected)
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_padgroupmode_set(u32 pad_mode)
{
    return hal_gpio_padgroupmode_set(pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// set a pad to the specified mode
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pad_val_set(u8 gpio_index, u32 pad_mode)
{
    return hal_gpio_pad_val_set(gpio_index, pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// set a pad to the specified mode
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pad_val_get(u8 gpio_index, u32* pad_mode)
{
    return hal_gpio_pad_val_get(gpio_index, pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// set a mode to IO voltage
///@param  group  mode
///@return 1
///@note only for I7 , 3.3V is not allowed when MODE = 1
//-------------------------------------------------------------------------------------------------
void sstar_gpio_vol_val_set(u8 group, u32 mode)
{
    hal_gpio_vol_val_set(group, mode);
}

//-------------------------------------------------------------------------------------------------
/// set a pad to the specified mode
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pad_val_check(u8 gpio_index, u32 pad_mode)
{
    return hal_gpio_pad_val_check(gpio_index, pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// enable output for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pad_oen(u8 gpio_index)
{
    return hal_gpio_pad_oen(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// enable input for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pad_odn(u8 gpio_index)
{
    return hal_gpio_pad_odn(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// read data from selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pad_read(u8 gpio_index, u8* pad_level)
{
    return hal_gpio_pad_level(gpio_index, pad_level);
}

//-------------------------------------------------------------------------------------------------
/// read pad direction for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pad_in_out(u8 gpio_index, u8* pad_in_out)
{
    return hal_gpio_pad_in_out(gpio_index, pad_in_out);
}

//-------------------------------------------------------------------------------------------------
/// output pull high for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pull_high(u8 gpio_index)
{
    return hal_gpio_pull_high(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull low for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pull_low(u8 gpio_index)
{
    return hal_gpio_pull_low(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull up for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull up or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pull_up(u8 gpio_index)
{
    return hal_gpio_pull_up(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull down for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull down or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pull_down(u8 gpio_index)
{
    return hal_gpio_pull_down(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull off for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull off or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pull_off(u8 gpio_index)
{
    return hal_gpio_pull_off(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// get the gpio pull status
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_pull_status(u8 gpio_index, u8* pull_status)
{
    return hal_gpio_pull_status(gpio_index, pull_status);
}

//-------------------------------------------------------------------------------------------------
/// output set high for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_set_high(u8 gpio_index)
{
    return hal_gpio_set_high(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output set low for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_set_low(u8 gpio_index)
{
    return hal_gpio_set_low(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output set driving for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: this GPIO unsupport set driving or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_drv_set(u8 gpio_index, u8 level)
{
    return hal_gpio_drv_set(gpio_index, level);
}

//-------------------------------------------------------------------------------------------------
/// get driving for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: this GPIO unsupport set driving or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_drv_get(u8 gpio_index, u8* level)
{
    return hal_gpio_drv_get(gpio_index, level);
}

//-------------------------------------------------------------------------------------------------
/// enable GPIO int for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
int sstar_gpio_to_irq(u8 gpio_index)
{
    return hal_gpio_to_irq(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// get GPIO check info count
/// @param  None                     \b IN:  None
/// @return GPIO check info count
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_get_check_count(void)
{
    return hal_gpio_get_check_count();
}

//-------------------------------------------------------------------------------------------------
/// get GPIO check info point
/// @param  index                  \b IN:  Check info index
/// @return GPIO check info point
/// @note
//-------------------------------------------------------------------------------------------------
void* sstar_gpio_get_check_info(u8 index)
{
    return hal_gpio_get_check_info(index);
}

//-------------------------------------------------------------------------------------------------
/// return GPIO number index according to pad name
/// @param  p_name                  \b IN:  pad name to transform
/// @return GPIO number index
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_name_to_num(u8* p_name, u8* gpio_index)
{
    return hal_gpio_name_to_num(p_name, gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// return GPIO name according to pad number
/// @param  gpio_index              \b IN:  pad index
/// @return GPIO name or NULL
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_num_to_name(u8 gpio_index, const u8** p_name)
{
    return hal_gpio_num_to_name(gpio_index, p_name);
}

//-------------------------------------------------------------------------------------------------
/// return pinmux mode index according to pad name
/// @param  p_name                  \b IN:  pad name to transform
/// @return pinmux mode index
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_padmode_to_val(u8* p_mode, u32* mode_to_val)
{
    return hal_gpio_padmode_to_val(p_mode, mode_to_val);
}

//-------------------------------------------------------------------------------------------------
/// get pad index from pad mode
/// @param  u32 mode              \b IN:  pad mode
/// @return pad array
/// @note
//-------------------------------------------------------------------------------------------------
u32* sstar_gpio_padmode_to_padindex(u32 mode)
{
    return hal_gpio_padmode_to_padindex(mode);
}

//-------------------------------------------------------------------------------------------------
/// get pad reg info about gpio output
/// @param  u8 gpio_index
/// @return void*
/// @note
//-------------------------------------------------------------------------------------------------
u8 sstar_gpio_get_reg_cfg(u8 gpio_index, void* reg_cfg)
{
    return hal_gpio_get_reg_info(gpio_index, reg_cfg);
}

EXPORT_SYMBOL(sstar_gpio_init);
EXPORT_SYMBOL(sstar_gpio_pad_set);
EXPORT_SYMBOL(sstar_gpio_pad_clr);
EXPORT_SYMBOL(sstar_gpio_padgroupmode_set);
EXPORT_SYMBOL(sstar_gpio_pad_val_set);
EXPORT_SYMBOL(sstar_gpio_pad_val_get);
EXPORT_SYMBOL(sstar_gpio_vol_val_set);
EXPORT_SYMBOL(sstar_gpio_pad_val_check);
EXPORT_SYMBOL(sstar_gpio_pad_oen);
EXPORT_SYMBOL(sstar_gpio_pad_odn);
EXPORT_SYMBOL(sstar_gpio_pad_read);
EXPORT_SYMBOL(sstar_gpio_pad_in_out);
EXPORT_SYMBOL(sstar_gpio_pull_high);
EXPORT_SYMBOL(sstar_gpio_pull_low);
EXPORT_SYMBOL(sstar_gpio_pull_up);
EXPORT_SYMBOL(sstar_gpio_pull_down);
EXPORT_SYMBOL(sstar_gpio_pull_off);
EXPORT_SYMBOL(sstar_gpio_pull_status);
EXPORT_SYMBOL(sstar_gpio_set_high);
EXPORT_SYMBOL(sstar_gpio_set_low);
EXPORT_SYMBOL(sstar_gpio_drv_set);
EXPORT_SYMBOL(sstar_gpio_drv_get);
EXPORT_SYMBOL(sstar_gpio_to_irq);
EXPORT_SYMBOL(sstar_gpio_get_check_count);
EXPORT_SYMBOL(sstar_gpio_get_check_info);
EXPORT_SYMBOL(sstar_gpio_name_to_num);
EXPORT_SYMBOL(sstar_gpio_padmode_to_val);
EXPORT_SYMBOL(sstar_gpio_padmode_to_padindex);
EXPORT_SYMBOL(sstar_gpio_get_reg_cfg);
