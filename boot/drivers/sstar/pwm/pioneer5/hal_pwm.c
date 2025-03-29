/*
 * hal_pwm.c- Sigmastar
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

#include "hal_pwm.h"

static u16 clk_pwm_div[7]       = {1, 2, 4, 8, 32, 64, 128};
static u32 pwm_duty_ns[PWM_NUM] = {0};

static void HalPwmGetGrpAddr(struct sstar_pwm_chip *chip, ulong *uladdr, u32 *u32PwmOffs, u8 u8Id)
{
    if (u8Id >= PWM_NUM)
        return;

    if (u8Id < 12)
    {
        *uladdr     = (uintptr_t)chip->base_addr;
        *u32PwmOffs = (u8Id < 4) ? (u8Id * 0x80) : ((4 * 0x80) + ((u8Id - 4) * 0x40));
    }
    else if (u8Id < 19)
    {
        *uladdr     = (uintptr_t)chip->base_addr;
        *u32PwmOffs = (u8Id < 13) ? ((12 * 0x80)) : ((12 * 0x80) + (0x20 * 4) + ((u8Id - 13) * 0x40));
    }
    else if (u8Id < 20)
    {
        *uladdr     = (uintptr_t)chip->base_addr;
        *u32PwmOffs = (0x50 * 4);
    }
    else if (u8Id < PWM_NUM)
    {
        *uladdr     = (uintptr_t)PWM_PM_BANK;
        *u32PwmOffs = (u8Id - 19) * 0x40;
    }
    else
        return;
}

void HalPwmSetPolarity(struct sstar_pwm_chip *chip, u8 u8Id, u8 u8Val)
{
    ulong ulPwmAddr  = 0;
    u32   u32PwmOffs = 0;
    HalPwmGetGrpAddr(chip, &ulPwmAddr, &u32PwmOffs, u8Id);
    OUTREGMSK16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_CTRL, u8Val ? POLARITY_BIT : 0, POLARITY_BIT);
}

void HalPwmSetConfig(struct sstar_pwm_chip *chip, u8 u8Id, u32 duty, u32 period)
{
    U8    i;
    U16   u16Div     = 0;
    U32   common     = 0;
    U32   pwmclk     = 0;
    U32   periodmax  = 0;
    U32   u32Period  = 0x00000000;
    U32   u32Duty    = 0x00000000;
    ulong ulPwmAddr  = 0;
    U32   u32PwmOffs = 0;

    if (u8Id >= PWM_NUM)
        return;

    HalPwmGetGrpAddr(chip, &ulPwmAddr, &u32PwmOffs, u8Id);
    switch (chip->clk_rate)
    {
        case 12000000:
            pwmclk = 3;
            common = 250;
            break;
        default:
            pwmclk = 3;
            common = 250;
    }

    /*      select   div       */
    for (i = 0; i < (sizeof(clk_pwm_div) / sizeof(U16)); i++)
    {
        periodmax = (clk_pwm_div[i] * 262144 / pwmclk) * common;
        if (period < periodmax)
        {
            u16Div = clk_pwm_div[i];
            break;
        }
    }

    /*      select   period       */
    if (period < (0xFFFFFFFF / pwmclk))
    {
        u32Period = (pwmclk * period) / (u16Div * common);
        if (((pwmclk * period) % (u16Div * common)) > (u16Div * common / 2))
        {
            u32Period++;
        }
    }
    else
    {
        u32Period = (period / u16Div) * pwmclk / common;
        u32Period++;
    }

    /*      select   duty       */
    pwm_duty_ns[u8Id] = duty;
    if (duty < (0xFFFFFFFF / pwmclk))
    {
        u32Duty = (pwmclk * duty) / (u16Div * common);
        if ((((pwmclk * duty) % (u16Div * common)) > (u16Div * common / 2)) || (u32Duty == 0))
        {
            u32Duty++;
        }
    }
    else
    {
        u32Duty = (duty / u16Div) * pwmclk / common;
        u32Duty++;
    }

    /*      set  div period duty       */
    u16Div--;
    u32Period--;
    u32Duty--;
    OUTREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_DIV, (u16Div & 0xFFFF));
    OUTREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_PERIOD_L, (u32Period & 0xFFFF));
    OUTREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_PERIOD_H, ((u32Period >> 16) & 0x3));
    OUTREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_DUTY_L, (u32Duty & 0xFFFF));
    OUTREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_DUTY_H, ((u32Duty >> 16) & 0x3));
}

void HalPwmEnable(struct sstar_pwm_chip *chip, u8 u8Id, u8 u8Val)
{
    ulong ulPwmAddr  = 0;
    U32   u32PwmOffs = 0;
    U32   u32DutyL   = 0;
    U32   u32DutyH   = 0;

    if (PWM_NUM <= u8Id)
        return;

    HalPwmGetGrpAddr(chip, &ulPwmAddr, &u32PwmOffs, u8Id);
    u32DutyL = INREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_DUTY_L);
    u32DutyH = INREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_DUTY_H);

    if (u8Val)
    {
        if (u8Id < 12)
            CLRREG16(ulPwmAddr + u16REG_SW_RESET_1, (BIT0 << (u8Id == 11 ? 14 : u8Id)));
        else if (u8Id < 20)
            CLRREG16(ulPwmAddr + u16REG_SW_RESET_2, (BIT0 << (u8Id - 12)));
        else
            CLRREG16(ulPwmAddr + u16REG_SW_RESET_1, (BIT0 << (u8Id - 20)));
    }
    else
    {
        if (u8Id < 12)
            SETREG16(ulPwmAddr + u16REG_SW_RESET_1, (BIT0 << (u8Id == 11 ? 14 : u8Id)));
        else if (u8Id < 20)
            SETREG16(ulPwmAddr + u16REG_SW_RESET_2, (BIT0 << (u8Id - 12)));
        else
            SETREG16(ulPwmAddr + u16REG_SW_RESET_1, (BIT0 << (u8Id - 20)));
    }
}

void HalPwmSetClk(struct sstar_pwm_chip *chip)
{
    switch (chip->clk_rate)
    {
        case 12000000:
            WRITE_WORD_MASK((ulong)0x1F2070E0, 0x0000, 0x0F00);
            break;
        default:
            WRITE_WORD_MASK((ulong)0x1F2070E0, 0x0000, 0x0F00);
    }
}

void HalPwmInit(struct sstar_pwm_chip *chip)
{
    ulong ulPwmAddr  = 0;
    u32   u32PwmOffs = 0;
    u8    u8PwmId    = 0;
    for (u8PwmId = 0; u8PwmId < PWM_NUM; u8PwmId++)
    {
        HalPwmGetGrpAddr(chip, &ulPwmAddr, &u32PwmOffs, u8PwmId);
        OUTREG16(ulPwmAddr + (u32PwmOffs) + u16REG_PWM_CTRL, (VDBEN_SW_BIT | DBEN_BIT));
    }
}
