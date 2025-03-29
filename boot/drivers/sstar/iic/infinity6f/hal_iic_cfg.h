/*
 * hal_iic_cfg.h- Sigmastar
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

#ifndef _HAL_IIC_CFG_H_
#define _HAL_IIC_CFG_H_

#include <iic_os.h>

#define HAL_I2C_BUSNUM     8
#define BASE_REG_RIU_PA    (0x1F000000)
#define HAL_I2C_MAX_SRCCLK 72000000

struct hal_i2c_reg
{
    u64 bank_base;
    u8  reg_offset;
    u8  bit_shift;
    u32 bit_mask;
};

struct hal_i2c_config
{
    u8  clk_mod;
    u32 clk_freq;
};

#ifndef CONFIG_SSTAR_CLK
struct hal_i2c_reg i2c_clk_reg[HAL_I2C_BUSNUM] = {
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x37 << 2, .bit_shift = 0, .bit_mask = 0x000F},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x37 << 2, .bit_shift = 8, .bit_mask = 0x0F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x37 << 2, .bit_shift = 12, .bit_mask = 0xF000},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x39 << 2, .bit_shift = 0, .bit_mask = 0x000F},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x39 << 2, .bit_shift = 8, .bit_mask = 0x0F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x39 << 2, .bit_shift = 12, .bit_mask = 0xF000},
    {.bank_base = (BASE_REG_RIU_PA + (0x000E << 9)), .reg_offset = 0x26 << 2, .bit_shift = 0, .bit_mask = 0x000F},
    {.bank_base = (BASE_REG_RIU_PA + (0x000E << 9)), .reg_offset = 0x26 << 2, .bit_shift = 12, .bit_mask = 0xF000},
};

struct hal_i2c_config i2c_clk_cfg[HAL_I2C_BUSNUM] = {
    {.clk_mod = 0, .clk_freq = 72000000}, {.clk_mod = 0, .clk_freq = 72000000}, {.clk_mod = 0, .clk_freq = 72000000},
    {.clk_mod = 0, .clk_freq = 72000000}, {.clk_mod = 0, .clk_freq = 72000000}, {.clk_mod = 0, .clk_freq = 72000000},
    {.clk_mod = 4, .clk_freq = 24000000}, {.clk_mod = 4, .clk_freq = 24000000},
};
#endif

#endif
