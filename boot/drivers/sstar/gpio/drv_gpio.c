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

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <common.h>
#include <command.h>
#include <drv_gpio.h>
#include <hal_gpio.h>
#include <asm/gpio.h>
#include <gpio.h>
#include <dm.h>

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
void sstar_gpio_pad_set(U8 gpio_index)
{
    hal_gpio_pad_set(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// select one pad to clear GPIO mode
/// @param  gpio_index              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void sstar_gpio_pad_clr(U8 gpio_index)
{
    hal_gpio_pad_clr(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// set the specified pad mode( a set of GPIO pad will be effected)
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_padgroupmode_set(U32 pad_mode)
{
    return hal_gpio_padgroupmode_set(pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// set a pad to the specified mode
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pad_val_set(U8 gpio_index, U32 pad_mode)
{
    return hal_gpio_pad_val_set(gpio_index, pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// set a pad to the specified mode
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pad_val_get(U8 gpio_index, U32* pad_mode)
{
    return hal_gpio_pad_val_get(gpio_index, pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// set a mode to IO voltage
///@param  group  mode
///@return 1
///@note only for I7 , 3.3V is not allowed when MODE = 1
//-------------------------------------------------------------------------------------------------
void sstar_gpio_vol_val_set(U8 group, U32 mode)
{
    hal_gpio_vol_val_set(group, mode);
}

//-------------------------------------------------------------------------------------------------
/// set a pad to the specified mode
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; 1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pad_val_check(U8 gpio_index, U32 pad_mode)
{
    return hal_gpio_pad_val_check(gpio_index, pad_mode);
}

//-------------------------------------------------------------------------------------------------
/// enable output for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pad_oen(U8 gpio_index)
{
    return hal_gpio_pad_oen(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// enable input for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pad_odn(U8 gpio_index)
{
    return hal_gpio_pad_odn(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// read data from selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pad_read(U8 gpio_index, U8* pad_level)
{
    return hal_gpio_pad_level(gpio_index, pad_level);
}

//-------------------------------------------------------------------------------------------------
/// read pad direction for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pad_in_out(U8 gpio_index, U8* pad_in_out)
{
    return hal_gpio_pad_in_out(gpio_index, pad_in_out);
}

//-------------------------------------------------------------------------------------------------
/// output pull high for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pull_high(U8 gpio_index)
{
    return hal_gpio_pull_high(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull low for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: fail, GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pull_low(U8 gpio_index)
{
    return hal_gpio_pull_low(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull up for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull up or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pull_up(U8 gpio_index)
{
    return hal_gpio_pull_up(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull down for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull down or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pull_down(U8 gpio_index)
{
    return hal_gpio_pull_down(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output pull off for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull off or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pull_off(U8 gpio_index)
{
    return hal_gpio_pull_off(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// get the gpio pull status
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: this GPIO unsupport pull or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_pull_status(U8 gpio_index, U8* pull_status)
{
    return hal_gpio_pull_status(gpio_index, pull_status);
}

//-------------------------------------------------------------------------------------------------
/// output set high for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_set_high(U8 gpio_index)
{
    return hal_gpio_set_high(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output set low for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0 success, return 1: GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_set_low(U8 gpio_index)
{
    return hal_gpio_set_low(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// output set driving for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: this GPIO unsupport set driving or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_drv_set(U8 gpio_index, U8 level)
{
    return hal_gpio_drv_set(gpio_index, level);
}

//-------------------------------------------------------------------------------------------------
/// get driving for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return 0: success, return 1: this GPIO unsupport set driving or GPIO_Index out of bounds
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_drv_get(U8 gpio_index, U8* level)
{
    return hal_gpio_drv_get(gpio_index, level);
}

//-------------------------------------------------------------------------------------------------
/// enable GPIO int for selected one pad
/// @param  gpio_index              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
int sstar_gpio_to_irq(U8 gpio_index)
{
    return hal_gpio_to_irq(gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// get GPIO check info count
/// @param  None                     \b IN:  None
/// @return GPIO check info count
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_get_check_count(void)
{
    return hal_gpio_get_check_count();
}

//-------------------------------------------------------------------------------------------------
/// get GPIO check info point
/// @param  index                  \b IN:  Check info index
/// @return GPIO check info point
/// @note
//-------------------------------------------------------------------------------------------------
void* sstar_gpio_get_check_info(U8 index)
{
    return hal_gpio_get_check_info(index);
}

//-------------------------------------------------------------------------------------------------
/// return GPIO number index according to pad name
/// @param  p_name                  \b IN:  pad name to transform
/// @return GPIO number index
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_name_to_num(U8* p_name, U8* gpio_index)
{
    return hal_gpio_name_to_num(p_name, gpio_index);
}

//-------------------------------------------------------------------------------------------------
/// return GPIO name according to pad number
/// @param  gpio_index              \b IN:  pad index
/// @return GPIO name or NULL
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_num_to_name(U8 gpio_index, const U8** p_name)
{
    return hal_gpio_num_to_name(gpio_index, p_name);
}

//-------------------------------------------------------------------------------------------------
/// return pinmux mode index according to pad name
/// @param  p_name                  \b IN:  pad name to transform
/// @return pinmux mode index
/// @note
//-------------------------------------------------------------------------------------------------
U8 sstar_gpio_padmode_to_val(U8* p_mode, U32* mode_to_val)
{
    return hal_gpio_padmode_to_val(p_mode, mode_to_val);
}

//-------------------------------------------------------------------------------------------------
/// get pad index from pad mode
/// @param  U32 mode              \b IN:  pad mode
/// @return pad array
/// @note
//-------------------------------------------------------------------------------------------------
U32* sstar_gpio_padmode_to_padindex(U32 mode)
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

#if !CONFIG_IS_ENABLED(DM_GPIO)
int gpio_set_value(unsigned gpio, int value)
{
    if (value == 0)
        hal_gpio_pull_low(gpio);
    else
        hal_gpio_pull_high(gpio);

    return 0;
}

int gpio_get_value(unsigned gpio)
{
    U8 Pad_Level;
    hal_gpio_pad_level(gpio, &Pad_Level);
    return Pad_Level;
}

int gpio_request(unsigned gpio, const char* label)
{
    hal_gpio_pad_set(gpio);
    return 0;
}

int gpio_free(unsigned gpio)
{
    hal_gpio_pad_clr(gpio);
    return 0;
}

int gpio_direction_input(unsigned gpio)
{
    hal_gpio_pad_set(gpio);
    hal_gpio_pad_odn(gpio);
    return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
    hal_gpio_pad_set(gpio);
    if (value == 0)
        hal_gpio_set_low(gpio);
    else
        hal_gpio_set_high(gpio);
    return 0;
}

#endif

#if CONFIG_IS_ENABLED(DM_GPIO)
static int sstar_gpio_direction_input(struct udevice* dev, unsigned offset)
{
    hal_gpio_pad_set(offset);
    hal_gpio_pad_odn(offset);
    return 0;
}
static int sstar_gpio_direction_output(struct udevice* dev, unsigned offset, int value)
{
    hal_gpio_pad_set(offset);
    if (value == 0)
        hal_gpio_set_low(offset);
    else
        hal_gpio_set_high(offset);
    return 0;
}

static int sstar_gpio_get_value(struct udevice* dev, unsigned offset)
{
    U8 Pad_Level;
    hal_gpio_pad_level(offset, &Pad_Level);
    return Pad_Level;
}

static int sstar_gpio_set_value(struct udevice* dev, unsigned offset, int value)
{
    if (value == 0)
        hal_gpio_pull_low(offset);
    else
        hal_gpio_pull_high(offset);

    return 0;
}

static int sstar_gpio_get_function(struct udevice* dev, unsigned offset)
{
    U8 Pad_InOut;
    hal_gpio_pad_in_out(offset, &Pad_InOut);
    return !Pad_InOut;
}

static const struct dm_gpio_ops gpio_sstar_ops = {
    .direction_input  = sstar_gpio_direction_input,
    .direction_output = sstar_gpio_direction_output,
    .get_value        = sstar_gpio_get_value,
    .set_value        = sstar_gpio_set_value,
    .get_function     = sstar_gpio_get_function,
};

static int sstar_gpio_probe(struct udevice* dev)
{
    struct gpio_dev_priv* uc_priv = dev_get_uclass_priv(dev);

    uc_priv->gpio_base  = 0;
    uc_priv->gpio_count = GPIO_NR;
    return 0;
}

static const struct udevice_id sstar_gpio_ids[] = {{.compatible = "sstar,gpio"}, {}};

U_BOOT_DRIVER(gpio_sstar) = {
    .name     = "gpio_sstar",
    .id       = UCLASS_GPIO,
    .ops      = &gpio_sstar_ops,
    .of_match = sstar_gpio_ids,
    .probe    = sstar_gpio_probe,
};
#endif
