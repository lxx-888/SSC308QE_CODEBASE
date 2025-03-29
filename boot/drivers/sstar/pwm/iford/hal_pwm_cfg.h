/*
 * hal_pwm_cfg.h- Sigmastar
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

#ifndef _HAL_PWM_CFG_H_
#define _HAL_PWM_CFG_H_

#include <hal_pwm.h>

#define HAL_PWM_CHANNEL 12
#define BASE_REG_RIU_PA (0x1F000000)

struct hal_pwm_reg
{
    u64 bank_base;
    u8  reg_offset;
    u8  bit_shift;
    u32 bit_mask;
};

struct hal_pwm_config
{
    u8  clk_mod;
    u32 clk_freq;
};

#ifndef CONFIG_SSTAR_CLK
struct hal_pwm_reg pwm_reg[HAL_PWM_CHANNEL] = {
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0x1038 << 9)), .reg_offset = 0x38, .bit_shift = 8, .bit_mask = 0x1F00},
    {.bank_base = (BASE_REG_RIU_PA + (0xE << 9)), .reg_offset = 0x1C, .bit_shift = 10, .bit_mask = 0x7C00},
    {.bank_base = (BASE_REG_RIU_PA + (0xE << 9)), .reg_offset = 0x1C, .bit_shift = 10, .bit_mask = 0x7C00},
    {.bank_base = (BASE_REG_RIU_PA + (0xE << 9)), .reg_offset = 0x1C, .bit_shift = 10, .bit_mask = 0x7C00},
    {.bank_base = (BASE_REG_RIU_PA + (0xE << 9)), .reg_offset = 0x1C, .bit_shift = 10, .bit_mask = 0x7C00},
};

struct hal_pwm_config pwm_cfg[HAL_PWM_CHANNEL] = {
    {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000},
    {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000},
    {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000},
    {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000}, {.clk_mod = 0, .clk_freq = 12000000},
};
#endif

#endif
