/*
 * hal_pwm.h- Sigmastar
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

#ifndef _HAL_PWM_H_
#define _HAL_PWM_H_
#include "linux/types.h"
#include "asm/arch/mach/sstar_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include <linux/compat.h>
#include <linux/delay.h>
#include <cpu_func.h>
#define READ_WORD(_reg)        (*(volatile u16 *)(unsigned long)(_reg))
#define WRITE_WORD(_reg, _val) ((*(volatile u16 *)(unsigned long)(_reg)) = (u16)(_val))
#define WRITE_WORD_MASK(_reg, _val, _mask)      \
    ((*(volatile u16 *)(unsigned long)(_reg)) = \
         ((*(volatile u16 *)(unsigned long)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))

#define PWM_GROUP_NUM (5)
#define PWM_PER_GROUP (4)
#define PWM_NUM       (22)
#define PWM_PM_BANK   GET_BASE_ADDR_BY_BANK(0x1F000000, 0x001A00)

#define u16REG_PWM_SHIFT_L  (0x0 << 2)
#define u16REG_PWM_SHIFT_H  (0x1 << 2)
#define u16REG_PWM_DUTY_L   (0x2 << 2)
#define u16REG_PWM_DUTY_H   (0x3 << 2)
#define u16REG_PWM_PERIOD_L (0x4 << 2)
#define u16REG_PWM_PERIOD_H (0x5 << 2)
#define u16REG_PWM_DIV      (0x6 << 2)
#define u16REG_PWM_CTRL     (0x7 << 2)
#define VDBEN_SW_BIT        0x1
#define DBEN_BIT            0x2
#define DIFF_P_EN_BIT       0x4
#define SHIFT_GAT_BIT       0x8
#define POLARITY_BIT        0x10

#define u16REG_SW_RESET_2       (0x7C << 2)
#define REG_GROUP_SW_RST_SHFT_2 (0x8)

#define u16REG_SW_RESET_1       (0x7F << 2)
#define REG_GROUP_SW_RST_SHFT_1 (0xB)

struct sstar_pwm_chip
{
    u64 base_addr;
    u64 clk_rate;
    u32 padmux[PWM_NUM];
};

void HalPwmSetPolarity(struct sstar_pwm_chip *chip, u8 u8Id, u8 u8Val);
void HalPwmSetConfig(struct sstar_pwm_chip *chip, u8 u8Id, u32 duty, u32 period);
void HalPwmEnable(struct sstar_pwm_chip *chip, u8 u8Id, u8 u8Val);
void HalPwmSetClk(struct sstar_pwm_chip *chip);
void HalPwmInit(struct sstar_pwm_chip *chip);

#endif
