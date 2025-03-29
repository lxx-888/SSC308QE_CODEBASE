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

#define HAL_PWM_REG_CFG
#include <hal_pwm.h>

#ifdef CONFIG_PM_SLEEP
static u8  duty_qe0                        = 0;
static u8  duty0_pol                       = 0;
static u32 hold_mode1                      = 0;
static u32 goup_enable[HAL_PWM_MAX_GROUP]  = {0};
static u32 goup_stop[HAL_PWM_MAX_GROUP]    = {0};
static u32 goup_reset[HAL_PWM_MAX_GROUP]   = {0};
static u32 pwm_polar[HAL_PWM_MAX_CHANNEL]  = {0};
static u32 pwm_div[HAL_PWM_MAX_CHANNEL]    = {0};
static u32 pwm_sync[HAL_PWM_MAX_CHANNEL]   = {0};
static u32 pwm_reset[HAL_PWM_MAX_CHANNEL]  = {0};
static u64 pwm_period[HAL_PWM_MAX_CHANNEL] = {0};
static u64 pwm_duty[HAL_PWM_MAX_CHANNEL]   = {0};
static u64 pwm_shift[HAL_PWM_MAX_CHANNEL]  = {0};
#endif
static const u32 pwm_clk_div[17] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 23768, 65536};

void hal_pwm_get_duty0_pol_status(struct hal_pwm_cfg *pwm_cfg, u8 *duty0_pol)
{
    u8 i;
    for (i = 0; i < sizeof(pwm_duty0_pol) / sizeof(pwm_duty0_pol[0]); i++)
    {
        *duty0_pol |= (HAL_PWM_READ_REG(pwm_cfg->addr_base + (pwm_duty0_pol[i].bank << 9), pwm_duty0_pol[i].offset)
                       & pwm_duty0_pol[i].bit)
                      << i;
    }
}

static void hal_pwm_duty0_pol_enable(struct hal_pwm_cfg *pwm_cfg, u8 enable)
{
    u8 i;
    for (i = 0; i < sizeof(pwm_duty0_pol) / sizeof(pwm_duty0_pol[0]); i++)
    {
        if (enable)
            HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_duty0_pol[i].bank << 9), pwm_duty0_pol[i].offset,
                                   pwm_duty0_pol[i].bit, pwm_duty0_pol[i].bit);
        else
            HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_duty0_pol[i].bank << 9), pwm_duty0_pol[i].offset, 0,
                                   pwm_duty0_pol[i].bit);
    }
}

void hal_pwm_get_duty0_status(struct hal_pwm_cfg *pwm_cfg, u8 *duty_qe0)
{
    u8 i;

    for (i = 0; i < sizeof(pwm_duty_qe0) / sizeof(pwm_duty_qe0[0]); i++)
    {
        *duty_qe0 |= (HAL_PWM_READ_REG(pwm_cfg->addr_base + (pwm_duty_qe0[i].bank << 9), pwm_duty_qe0[i].offset)
                      & pwm_duty_qe0[i].bit)
                     << i;
    }
}

static void hal_pwm_duty0_enable(struct hal_pwm_cfg *pwm_cfg, u8 enable)
{
    u8 i;

    for (i = 0; i < sizeof(pwm_duty_qe0) / sizeof(pwm_duty_qe0[0]); i++)
    {
        if (enable)
            HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_duty_qe0[i].bank << 9), pwm_duty_qe0[i].offset,
                                   pwm_duty_qe0[i].bit, pwm_duty_qe0[i].bit);
        else
            HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_duty_qe0[i].bank << 9), pwm_duty_qe0[i].offset, 0,
                                   pwm_duty_qe0[i].bit);
    }
}

u32 hal_pwm_get_group_reset(u64 addr, u32 group)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(addr + (pwm_group_reset[group].bank << 9), pwm_group_reset[group].offset)
          & pwm_group_reset[group].bit;

    return ret ? 1 : 0;
}

void hal_pwm_set_group_reset(u64 addr, u32 group, u8 reset)
{
    if (reset)
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_group_reset[group].bank << 9), pwm_group_reset[group].offset,
                               pwm_group_reset[group].bit, pwm_group_reset[group].bit);
    else
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_group_reset[group].bank << 9), pwm_group_reset[group].offset, 0,
                               pwm_group_reset[group].bit);
}

u32 hal_pwm_get_group_status(u64 addr, u32 group)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(addr + (pwm_group_enable[group].bank << 9), pwm_group_enable[group].offset)
          & pwm_group_enable[group].bit;

    return ret ? 1 : 0;
}

void hal_pwm_group_enbale(u64 addr, u32 group, u8 enable)
{
    if (enable)
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_group_enable[group].bank << 9), pwm_group_enable[group].offset,
                               pwm_group_enable[group].bit, pwm_group_enable[group].bit);
    else
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_group_enable[group].bank << 9), pwm_group_enable[group].offset, 0,
                               pwm_group_enable[group].bit);
}

u32 hal_pwm_get_hold_int(u64 addr, u32 group)
{
    u32 ret;

    ret =
        HAL_PWM_READ_REG(addr + (pwm_hold_int[group].bank << 9), pwm_hold_int[group].offset) & pwm_hold_int[group].bit;

    return ret ? 1 : 0;
}

u32 hal_pwm_get_round_int(u64 addr, u32 group)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(addr + (pwm_round_int[group].bank << 9), pwm_round_int[group].offset)
          & pwm_round_int[group].bit;

    return ret ? 1 : 0;
}

u32 hal_pwm_get_hold_mode1_status(u64 addr)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(addr + (pwm_hold_mode1[0].bank << 9), pwm_hold_mode1[0].offset) & pwm_hold_mode1[0].bit;

    return ret ? 1 : 0;
}

void hal_pwm_hold_mode1_enable(u64 addr, u8 enable)
{
    u8 i;

    for (i = 0; i < sizeof(pwm_hold_mode1) / sizeof(pwm_hold_mode1[0]); i++)
    {
        if (enable)
            HAL_PWM_WRITE_REG_MASK(addr + (pwm_hold_mode1[i].bank << 9), pwm_hold_mode1[i].offset,
                                   pwm_hold_mode1[i].bit, pwm_hold_mode1[i].bit);
        else
            HAL_PWM_WRITE_REG_MASK(addr + (pwm_hold_mode1[i].bank << 9), pwm_hold_mode1[i].offset, 0,
                                   pwm_hold_mode1[i].bit);
    }
}

u32 hal_pwm_get_hold_mode_status(u64 addr, u32 group)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(addr + (pwm_hold_mode[group].bank << 9), pwm_hold_mode[group].offset)
          & pwm_hold_mode[group].bit;

    return ret ? 1 : 0;
}

void hal_pwm_hold_mode_enable(u64 addr, u32 group, u8 enable)
{
    if (enable)
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_hold_mode[group].bank << 9), pwm_hold_mode[group].offset,
                               pwm_hold_mode[group].bit, pwm_hold_mode[group].bit);
    else
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_hold_mode[group].bank << 9), pwm_hold_mode[group].offset, 0,
                               pwm_hold_mode[group].bit);
}

u32 hal_pwm_get_round_num(u64 addr, u32 group)
{
    return HAL_PWM_READ_REG(addr + (pwm_round_mode[group].bank << 9), pwm_round_mode[group].offset);
}

void hal_pwm_set_round_num(u64 addr, u32 group, u32 value)
{
    HAL_PWM_WRITE_REG(addr + (pwm_round_mode[group].bank << 9), pwm_round_mode[group].offset, value);
}

u32 hal_pwm_get_stop_mode_status(u64 addr, u32 group)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(addr + (pwm_stop_mode[group].bank << 9), pwm_stop_mode[group].offset)
          & pwm_stop_mode[group].bit;

    return ret ? 1 : 0;
}

void hal_pwm_stop_mode_enable(u64 addr, u32 group, u8 enable)
{
    if (enable)
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_stop_mode[group].bank << 9), pwm_stop_mode[group].offset,
                               pwm_stop_mode[group].bit, pwm_stop_mode[group].bit);
    else
        HAL_PWM_WRITE_REG_MASK(addr + (pwm_stop_mode[group].bank << 9), pwm_stop_mode[group].offset, 0,
                               pwm_stop_mode[group].bit);
}

u32 hal_pwm_get_sync_mode_status(struct hal_pwm_cfg *pwm_cfg)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(pwm_cfg->addr_base + (pwm_sync_mode[pwm_cfg->channel].bank << 9),
                           pwm_sync_mode[pwm_cfg->channel].offset)
          & pwm_sync_mode[pwm_cfg->channel].bit;

    return ret ? 1 : 0;
}

void hal_pwm_sync_mode_enbale(struct hal_pwm_cfg *pwm_cfg, u8 enable)
{
    u32 ret;

    if (pwm_cfg->channel >= HAL_PWM_MAX_ADD_GROUP_CH)
    {
        pwm_dbg("pwm channel with group property over the maximum\n");
        return;
    }

    ret = hal_pwm_get_sync_mode_status(pwm_cfg);
    if (enable && ret)
    {
        pwm_dbg("channel already join group\n");
        return;
    }

    if (enable)
        HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_sync_mode[pwm_cfg->channel].bank << 9),
                               pwm_sync_mode[pwm_cfg->channel].offset, pwm_sync_mode[pwm_cfg->channel].bit,
                               pwm_sync_mode[pwm_cfg->channel].bit);
    else
        HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_sync_mode[pwm_cfg->channel].bank << 9),
                               pwm_sync_mode[pwm_cfg->channel].offset, 0, pwm_sync_mode[pwm_cfg->channel].bit);
}

u32 hal_pwm_get_channel_reset(struct hal_pwm_cfg *pwm_cfg)
{
    u32 ret;

    ret = HAL_PWM_READ_REG(pwm_cfg->addr_base + (pwm_channel_reset[pwm_cfg->channel].bank << 9),
                           pwm_channel_reset[pwm_cfg->channel].offset)
          & pwm_channel_reset[pwm_cfg->channel].bit;

    return ret ? 1 : 0;
}

void hal_pwm_set_channel_reset(struct hal_pwm_cfg *pwm_cfg, u8 reset)
{
    if (reset)
        HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_channel_reset[pwm_cfg->channel].bank << 9),
                               pwm_channel_reset[pwm_cfg->channel].offset, pwm_channel_reset[pwm_cfg->channel].bit,
                               pwm_channel_reset[pwm_cfg->channel].bit);
    else
        HAL_PWM_WRITE_REG_MASK(pwm_cfg->addr_base + (pwm_channel_reset[pwm_cfg->channel].bank << 9),
                               pwm_channel_reset[pwm_cfg->channel].offset, 0, pwm_channel_reset[pwm_cfg->channel].bit);

#ifdef CONFIG_PM_SLEEP
    pwm_reset[pwm_cfg->channel] = reset;
#endif
}

u32 hal_pwm_get_div(struct hal_pwm_cfg *pwm_cfg)
{
    u32 div;
    div = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_DIVIDER);

    return div;
}

int hal_pwm_set_div(struct hal_pwm_cfg *pwm_cfg, u64 period_wf, u8 sync)
{
    u8  i;
    u32 div_reg    = 1;
    u64 period_max = 0;

#ifdef CONFIG_SSTAR_PWM_EXTEND

    if (period_wf == 0)
    {
        div_reg = pwm_clk_div[0];
        goto div_set;
    }

    for (i = 0; i < (sizeof(pwm_clk_div) / sizeof(pwm_clk_div[0])); i++)
    {
        period_max = ((u64)pwm_clk_div[i] * 262144 / pwm_cfg->clk_freq) * pwm_cfg->hz_to_ns;
        if (period_wf < period_max)
        {
            div_reg = pwm_clk_div[i];
            break;
        }
    }

#else

    if (period_wf == 0)
    {
        div_reg = pwm_clk_div[0];
        goto div_set;
    }

    for (i = 0; i < (sizeof(pwm_clk_div) / sizeof(pwm_clk_div[0])); i++)
    {
        period_max = pwm_cfg->clk_freq / (pwm_clk_div[i] * 262144);
        if (period_wf > period_max)
        {
            div_reg = pwm_clk_div[i];
            break;
        }
    }

#endif

div_set:
    div_reg--;
    pwm_cfg->pwm_attr.divider = div_reg;

    if (!sync)
    {
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DIVIDER, (div_reg & 0xFFFF));
#ifdef CONFIG_PM_SLEEP
        pwm_div[pwm_cfg->channel] = div_reg;
#endif
    }

    return 0;
}

u64 hal_pwm_get_period(struct hal_pwm_cfg *pwm_cfg)
{
    u32 div;
    u64 period;
    u64 period_reg;

    period_reg = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L)
                 | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H) & 0x3) << 16);

    div = hal_pwm_get_div(pwm_cfg);
    div++;

    if (period_reg)
    {
        period_reg++;
#ifdef CONFIG_SSTAR_PWM_EXTEND
        period = (HAL_PWM_HIGH_PRECISION * (u64)pwm_cfg->clk_freq) / (period_reg * div * pwm_cfg->hz_to_ns);
        return period;
#else
        period = pwm_cfg->clk_freq / period_reg * div;
        return period;
#endif
    }

    return 0;
}

int hal_pwm_set_period(struct hal_pwm_cfg *pwm_cfg, u64 period_wf, u8 sync)
{
    u32 div_reg;
    u64 duty_wf;
    u64 shift_wf;
    u64 period_reg;
#ifdef CONFIG_SSTAR_PWM_EXTEND
    u64 period_store;
#endif

    hal_pwm_set_div(pwm_cfg, period_wf, sync);
    div_reg = pwm_cfg->pwm_attr.divider;
    div_reg++;

#ifdef CONFIG_SSTAR_PWM_EXTEND

    if (period_wf == 0)
    {
        period_reg = 1;
        goto period_set;
    }

    if (period_wf < (0xFFFFFFFF / pwm_cfg->clk_freq))
    {
        period_reg = (pwm_cfg->clk_freq * period_wf) / (div_reg * pwm_cfg->hz_to_ns);
    }
    else
    {
        period_reg = (period_wf / div_reg) * pwm_cfg->clk_freq / pwm_cfg->hz_to_ns;
        period_reg++;
    }

#else

    if (period_wf == 0)
    {
        period_reg = 1;
        goto period_set;
    }
    else
    {
        period_reg = pwm_cfg->clk_freq / (period_wf * div_reg);
    }

#endif

    if (period_reg < 2)
    {
        pwm_err("pwm%u period setting over the maximun\n", pwm_cfg->channel);
        return -HAL_PWM_PERIOD_ERR;
    }
    else if (period_reg & 0xFFFC0000)
    {
        pwm_err("pwm%u period setting below the minimun\n", pwm_cfg->channel);
        return -HAL_PWM_PERIOD_ERR;
    }

period_set:
    period_reg--;
    pwm_cfg->pwm_attr.period = period_reg;

    pwm_cfg->pwm_attr.period_sync = sync;
    if (!sync)
    {
        if ((!pwm_cfg->pwm_attr.duty) && (pwm_cfg->pwm_attr.period == 0x1))
            hal_pwm_duty0_enable(pwm_cfg, 0);
        else if ((!pwm_cfg->pwm_attr.duty) && (pwm_cfg->pwm_attr.period >= 0x1))
            hal_pwm_duty0_enable(pwm_cfg, 1);

        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L, (period_reg & 0xFFFF));
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H, ((period_reg >> 16) & 0x3));
#ifdef CONFIG_PM_SLEEP
        pwm_period[pwm_cfg->channel] = period_reg;
#endif
    }

#ifdef CONFIG_SSTAR_PWM_EXTEND
    period_store = pwm_cfg->pwm_attr.period_wf;
#endif
    pwm_cfg->pwm_attr.period_wf = period_wf;
    if (pwm_cfg->pwm_attr.duty)
    {
#ifdef CONFIG_SSTAR_PWM_EXTEND
        if (period_store)
            duty_wf = (period_wf * (pwm_cfg->pwm_attr.duty_wf * 100 / period_store) / 100);
        else
            duty_wf = 0;
#else
        duty_wf  = pwm_cfg->pwm_attr.duty_wf;
#endif
        hal_pwm_set_duty(pwm_cfg, duty_wf, sync);
    }

    if (pwm_cfg->pwm_attr.shift)
    {
#ifdef CONFIG_SSTAR_PWM_EXTEND
        if (period_store)
            shift_wf = (period_wf * (pwm_cfg->pwm_attr.shift_wf * 100 / period_store) / 100);
        else
            shift_wf = 0;
#else
        shift_wf = pwm_cfg->pwm_attr.shift_wf;
#endif
        hal_pmw_set_shift(pwm_cfg, shift_wf, sync);
    }

    return 0;
}

u64 hal_pwm_get_duty(struct hal_pwm_cfg *pwm_cfg)
{
    u32 div;
    u64 duty_reg;
    u64 period_reg;

    duty_reg = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_L)
               | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_H) & 0x3) << 16);

    div = hal_pwm_get_div(pwm_cfg);
    div++;

    if (duty_reg)
    {
        duty_reg++;
        period_reg = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L)
                     | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H) & 0x3) << 16);
        if (period_reg != 0)
            return (duty_reg * div * 100) / ((period_reg + 1) * div);
    }

    return 0;
}

int hal_pwm_set_duty(struct hal_pwm_cfg *pwm_cfg, u64 duty_wf, u8 sync)
{
    u32 div_reg;
    u64 duty_reg;
    u64 period_reg;

    if (duty_wf > pwm_cfg->pwm_attr.period_wf)
    {
        pwm_err("pwm%u duty setting over the period\n", pwm_cfg->channel);
        return -HAL_PWM_DUTY_ERR;
    }

    div_reg = pwm_cfg->pwm_attr.divider;
    div_reg++;

    period_reg = pwm_cfg->pwm_attr.period;
    period_reg++;

#ifdef CONFIG_SSTAR_PWM_EXTEND

    if (duty_wf == 0)
    {
        duty_reg = 1;
        goto duty_set;
    }

    if (duty_wf < (0xFFFFFFFF / pwm_cfg->clk_freq))
    {
        duty_reg = (pwm_cfg->clk_freq * duty_wf) / (div_reg * pwm_cfg->hz_to_ns);
    }
    else
    {
        duty_reg = (duty_wf / div_reg) * pwm_cfg->clk_freq / pwm_cfg->hz_to_ns;
        duty_reg++;
    }

#else

    if (duty_wf == 0)
    {
        duty_reg = 1;
        goto duty_set;
    }
    else
    {
        duty_reg = ((period_reg * div_reg * duty_wf) / 100) / div_reg;
    }

#endif

    if (duty_reg < 1)
    {
        pwm_err("pwm%u duty setting below the minimun\n", pwm_cfg->channel);
        return -HAL_PWM_DUTY_ERR;
    }
    else if (duty_reg & 0xFFFC0000)
    {
        pwm_err("pwm%u duty setting over the maximun\n", pwm_cfg->channel);
        return -HAL_PWM_DUTY_ERR;
    }

duty_set:
    duty_reg--;
    pwm_cfg->pwm_attr.duty    = duty_reg;
    pwm_cfg->pwm_attr.duty_wf = duty_wf;

    pwm_cfg->pwm_attr.duty_sync = sync;
    if (!sync)
    {
        if ((!pwm_cfg->pwm_attr.duty) && (pwm_cfg->pwm_attr.period == 0x1))
            hal_pwm_duty0_enable(pwm_cfg, 0);
        else if ((!pwm_cfg->pwm_attr.duty) && (pwm_cfg->pwm_attr.period >= 0x1))
            hal_pwm_duty0_enable(pwm_cfg, 1);

        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_L, (duty_reg & 0xFFFF));
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_H, ((duty_reg >> 16) & 0x3));
#ifdef CONFIG_PM_SLEEP
        pwm_duty[pwm_cfg->channel] = duty_reg;
#endif
    }

    return 0;
}

u64 hal_pwm_get_shift(struct hal_pwm_cfg *pwm_cfg)
{
    u32 div;
    u64 shift_reg;
    u64 period_reg;

    shift_reg = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_L)
                | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_H) & 0x3) << 16);

    div = hal_pwm_get_div(pwm_cfg);
    div++;

    if (shift_reg)
    {
        shift_reg++;
        period_reg = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L)
                     | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H) & 0x3) << 16);
        if (period_reg != 0)
            return (shift_reg * div * 100) / ((period_reg + 1) * div);
    }

    return 0;
}

int hal_pmw_set_shift(struct hal_pwm_cfg *pwm_cfg, u64 shift_wf, u8 sync)
{
    u32 div_reg;
    u64 shift_reg;
    u64 period_reg;

    if (shift_wf > pwm_cfg->pwm_attr.period_wf)
    {
        pwm_err("pwm%u shift_reg setting over the period\n", pwm_cfg->channel);
        return -HAL_PWM_SHIFT_ERR;
    }

    div_reg = pwm_cfg->pwm_attr.divider;
    div_reg++;

    period_reg = pwm_cfg->pwm_attr.period;
    period_reg++;

#ifdef CONFIG_SSTAR_PWM_EXTEND

    if (shift_wf == 0)
    {
        shift_reg = 1;
        goto shift_set;
    }

    if (shift_wf < (0xFFFFFFFF / pwm_cfg->clk_freq))
    {
        shift_reg = (pwm_cfg->clk_freq * shift_wf) / (div_reg * pwm_cfg->hz_to_ns);
    }
    else
    {
        shift_reg = (shift_wf / div_reg) * pwm_cfg->clk_freq / pwm_cfg->hz_to_ns;
        shift_reg++;
    }

#else

    if (shift_wf == 0)
    {
        shift_reg = 1;
        goto shift_set;
    }
    else
    {
        shift_reg = ((period_reg * div_reg * shift_wf) / 100) / div_reg;
    }

#endif

    if (shift_reg < 1)
    {
        pwm_err("pwm%u shift_reg setting below the minimun\n", pwm_cfg->channel);
        return -HAL_PWM_SHIFT_ERR;
    }
    else if (shift_reg & 0xFFFC0000)
    {
        pwm_err("pwm%u shift_reg setting over the maximun\n", pwm_cfg->channel);
        return -HAL_PWM_SHIFT_ERR;
    }

shift_set:
    shift_reg--;
    pwm_cfg->pwm_attr.shift    = shift_reg;
    pwm_cfg->pwm_attr.shift_wf = shift_wf;

    pwm_cfg->pwm_attr.shift_sync = sync;
    if (!sync)
    {
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_L, (shift_reg & 0xFFFF));
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_H, ((shift_reg >> 16) & 0x3));
    }

    return 0;
}

u32 hal_pwm_get_polarity(struct hal_pwm_cfg *pwm_cfg)
{
    u32 polar;

    polar = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_POLARITY) & HAL_PWM_POLARITY_MASK;

    return polar ? 1 : 0;
}

void hal_pwm_set_polarity(struct hal_pwm_cfg *pwm_cfg, u8 value, u8 sync)
{
    pwm_cfg->pwm_attr.polar      = value;
    pwm_cfg->pwm_attr.polar_sync = sync;
    if (!sync)
    {
        HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_POLARITY, (value << HAL_PWM_POLARITY_BIT),
                               HAL_PWM_POLARITY_MASK);
#ifdef CONFIG_PM_SLEEP
        pwm_polar[pwm_cfg->channel] = value;
#endif
    }
}

u8 hal_pwm_sync_config(struct hal_pwm_cfg *pwm_cfg)
{
    u8 sync = 0;

    if ((!pwm_cfg->pwm_attr.duty) && (pwm_cfg->pwm_attr.period == 1))
        hal_pwm_duty0_enable(pwm_cfg, 0);
    else if ((!pwm_cfg->pwm_attr.duty) && (pwm_cfg->pwm_attr.period >= 0x1))
        hal_pwm_duty0_enable(pwm_cfg, 1);

    if (pwm_cfg->pwm_attr.period_sync)
    {
        sync++;
        pwm_cfg->pwm_attr.period_sync = 0;
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DIVIDER, (pwm_cfg->pwm_attr.divider & 0xFFFF));
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L, (pwm_cfg->pwm_attr.period & 0xFFFF));
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H, ((pwm_cfg->pwm_attr.period >> 16) & 0x3));
    }
    if (pwm_cfg->pwm_attr.duty_sync)
    {
        sync++;
        pwm_cfg->pwm_attr.duty_sync = 0;
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_L, (pwm_cfg->pwm_attr.duty & 0xFFFF));
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_H, ((pwm_cfg->pwm_attr.duty >> 16) & 0x3));
    }
    if (pwm_cfg->pwm_attr.shift_sync)
    {
        sync++;
        pwm_cfg->pwm_attr.shift_sync = 0;
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_L, (pwm_cfg->pwm_attr.shift & 0xFFFF));
        HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_H, ((pwm_cfg->pwm_attr.shift >> 16) & 0x3));
    }
    if (pwm_cfg->pwm_attr.polar_sync)
    {
        sync++;
        pwm_cfg->pwm_attr.polar_sync = 0;
        HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_POLARITY,
                               (pwm_cfg->pwm_attr.polar << HAL_PWM_POLARITY_BIT), HAL_PWM_POLARITY_MASK);
    }

    return sync;
}

#ifdef CONFIG_PM_SLEEP
void hal_pwm_suspend_channel(struct hal_pwm_cfg *pwm_cfg)
{
    pwm_period[pwm_cfg->channel] = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L)
                                   | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H) & 0x3) << 16);
    pwm_duty[pwm_cfg->channel] = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_L)
                                 | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_H) & 0x3) << 16);
    pwm_shift[pwm_cfg->channel] = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_L)
                                  | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_H) & 0x3) << 16);
    pwm_polar[pwm_cfg->channel] = hal_pwm_get_polarity(pwm_cfg);
    pwm_div[pwm_cfg->channel]   = hal_pwm_get_div(pwm_cfg);
    pwm_sync[pwm_cfg->channel]  = hal_pwm_get_sync_mode_status(pwm_cfg);
    pwm_reset[pwm_cfg->channel] = hal_pwm_get_channel_reset(pwm_cfg);
    hal_pwm_get_duty0_status(pwm_cfg, &duty_qe0);
    hal_pwm_get_duty0_pol_status(pwm_cfg, &duty0_pol);
}

void hal_pwm_suspend_group(u64 addr, u32 group)
{
    if (group >= HAL_PWM_MAX_GROUP)
    {
        pwm_err("pwm group over the maximum\n");
        return;
    }
    goup_enable[group] = hal_pwm_get_group_status(addr, group);
    goup_stop[group]   = hal_pwm_get_stop_mode_status(addr, group);
    goup_reset[group]  = hal_pwm_get_group_reset(addr, group);
    hold_mode1         = hal_pwm_get_hold_mode1_status(addr);
}

void hal_pwm_resume_channel(struct hal_pwm_cfg *pwm_cfg)
{
#ifdef CONFIG_SSTAR_PWM_EXTEND
    u32 gcd;
    u32 divisor;
    u32 dividend;

    divisor  = HAL_PWM_HIGH_PRECISION;
    dividend = pwm_cfg->clk_freq;
    while ((gcd = dividend % divisor))
    {
        dividend = divisor;
        divisor  = gcd;
    }
    gcd = divisor;
    pwm_cfg->clk_freq /= gcd;
    pwm_cfg->hz_to_ns = HAL_PWM_HIGH_PRECISION / gcd;
#endif

    HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_VDBEN_SW, (0X1 << HAL_PWM_VDBEN_SW_BIT),
                           HAL_PWM_VDBEN_SW_MASK);
    HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_DBEN, (0X1 << HAL_PWM_DBEN_BIT), HAL_PWM_DBEN_MASK);
    HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L, (pwm_period[pwm_cfg->channel] & 0xFFFF));
    HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H, ((pwm_period[pwm_cfg->channel] >> 16) & 0x3));
    HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_L, (pwm_duty[pwm_cfg->channel] & 0xFFFF));
    HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_H, ((pwm_duty[pwm_cfg->channel] >> 16) & 0x3));
    HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_L, (pwm_shift[pwm_cfg->channel] & 0xFFFF));
    HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_H, ((pwm_shift[pwm_cfg->channel] >> 16) & 0x3));
    HAL_PWM_WRITE_REG(pwm_cfg->bank_base, HAL_PWM_REG_DIVIDER, (pwm_div[pwm_cfg->channel] & 0xFFFF));
    hal_pwm_set_polarity(pwm_cfg, pwm_polar[pwm_cfg->channel], 0);
    hal_pwm_sync_mode_enbale(pwm_cfg, pwm_sync[pwm_cfg->channel]);
    hal_pwm_set_channel_reset(pwm_cfg, pwm_reset[pwm_cfg->channel]);
    hal_pwm_duty0_enable(pwm_cfg, duty_qe0);
    hal_pwm_duty0_pol_enable(pwm_cfg, duty0_pol);
}

void hal_pwm_resume_group(u64 addr, u32 group)
{
    if (group >= HAL_PWM_MAX_GROUP)
    {
        pwm_err("pwm group over the maximum\n");
        return;
    }
    hal_pwm_group_enbale(addr, group, goup_enable[group]);
    hal_pwm_stop_mode_enable(addr, group, goup_stop[group]);
    hal_pwm_set_group_reset(addr, group, goup_reset[group]);
    hal_pwm_hold_mode1_enable(addr, hold_mode1);
}
#endif

void hal_pwm_deinit(struct hal_pwm_cfg *pwm_cfg)
{
    if ((pwm_cfg->group >= 0) && (pwm_cfg->group < HAL_PWM_MAX_GROUP))
    {
        hal_pwm_sync_mode_enbale(pwm_cfg, 0);
        hal_pwm_set_group_reset(pwm_cfg->addr_base, pwm_cfg->group, 1);
        hal_pwm_group_enbale(pwm_cfg->addr_base, pwm_cfg->group, 0);
        hal_pwm_stop_mode_enable(pwm_cfg->addr_base, pwm_cfg->group, 0);
        hal_pwm_set_round_num(pwm_cfg->addr_base, pwm_cfg->group, 0);
        hal_pwm_hold_mode_enable(pwm_cfg->addr_base, pwm_cfg->group, 0);
    }
    hal_pwm_set_channel_reset(pwm_cfg, 1);
    hal_pwm_set_duty(pwm_cfg, 0, 0);
    hal_pmw_set_shift(pwm_cfg, 0, 0);
    hal_pwm_set_period(pwm_cfg, 0, 0);

    HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_VDBEN_SW, (0 << HAL_PWM_VDBEN_SW_BIT),
                           HAL_PWM_VDBEN_SW_MASK);
    HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_DBEN, (0 << HAL_PWM_DBEN_BIT), HAL_PWM_DBEN_MASK);
}

int hal_pwm_init(struct hal_pwm_cfg *pwm_cfg)
{
#ifdef CONFIG_SSTAR_PWM_EXTEND
    u32 gcd;
    u32 divisor;
    u32 dividend;
#endif

    if (pwm_cfg->channel >= HAL_PWM_MAX_CHANNEL)
    {
        pwm_err("pwm channel over the maximum\n");
        return -HAL_PWM_CHANNEL_ERR;
    }
    if ((pwm_cfg->group >= HAL_PWM_MAX_GROUP) && (pwm_cfg->group != -1))
    {
        pwm_err("pwm group over the maximum\n");
        return -HAL_PWM_GROUP_ERR;
    }

#ifdef CONFIG_SSTAR_PWM_EXTEND
    divisor  = HAL_PWM_HIGH_PRECISION;
    dividend = pwm_cfg->clk_freq;
    while ((gcd = dividend % divisor))
    {
        dividend = divisor;
        divisor  = gcd;
    }
    gcd = divisor;
    pwm_cfg->clk_freq /= gcd;
    pwm_cfg->hz_to_ns = HAL_PWM_HIGH_PRECISION / gcd;
#endif

    HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_VDBEN_SW, (0X1 << HAL_PWM_VDBEN_SW_BIT),
                           HAL_PWM_VDBEN_SW_MASK);
    HAL_PWM_WRITE_REG_MASK(pwm_cfg->bank_base, HAL_PWM_REG_DBEN, (0X1 << HAL_PWM_DBEN_BIT), HAL_PWM_DBEN_MASK);
    hal_pwm_duty0_pol_enable(pwm_cfg, 1);
    hal_pwm_hold_mode1_enable(pwm_cfg->addr_base, 1);

    if (pwm_cfg->group >= 0)
    {
        pwm_dbg("pwm%u sync mode enable\n", pwm_cfg->channel);
        hal_pwm_sync_mode_enbale(pwm_cfg, 1);
        hal_pwm_group_enbale(pwm_cfg->addr_base, pwm_cfg->group, 1);
        hal_pwm_set_group_reset(pwm_cfg->addr_base, pwm_cfg->group, 0);
    }
    else
    {
        pwm_dbg("pwm%u sync mode disable\n", pwm_cfg->channel);
        hal_pwm_sync_mode_enbale(pwm_cfg, 0);
    }

    pwm_cfg->pwm_attr.reset = hal_pwm_get_channel_reset(pwm_cfg);

    pwm_cfg->pwm_attr.period = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_L)
                               | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_PERIOD_H) & 0x3) << 16);

    if ((pwm_cfg->pwm_attr.reset == 0) && (pwm_cfg->pwm_attr.period))
    {
        pwm_cfg->pwm_attr.polar   = hal_pwm_get_polarity(pwm_cfg);
        pwm_cfg->pwm_attr.divider = hal_pwm_get_div(pwm_cfg);

        pwm_cfg->pwm_attr.duty = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_L)
                                 | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_DUTY_H) & 0x3) << 16);
        pwm_cfg->pwm_attr.shift = HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_L)
                                  | ((HAL_PWM_READ_REG(pwm_cfg->bank_base, HAL_PWM_REG_SHIFT_H) & 0x3) << 16);

        pwm_cfg->pwm_attr.divider++;
        pwm_cfg->pwm_attr.shift++;
        pwm_cfg->pwm_attr.duty++;

#ifdef CONFIG_SSTAR_PWM_EXTEND
        pwm_cfg->pwm_attr.duty_wf =
            (pwm_cfg->pwm_attr.duty * pwm_cfg->pwm_attr.divider * pwm_cfg->hz_to_ns) / pwm_cfg->clk_freq;
        pwm_cfg->pwm_attr.shift_wf =
            (pwm_cfg->pwm_attr.shift * pwm_cfg->pwm_attr.divider * pwm_cfg->hz_to_ns) / pwm_cfg->clk_freq;
#else
        pwm_cfg->pwm_attr.period++;
        pwm_cfg->pwm_attr.duty_wf = (pwm_cfg->pwm_attr.duty * pwm_cfg->pwm_attr.divider * 100)
                                    / (pwm_cfg->pwm_attr.period * pwm_cfg->pwm_attr.divider);
        pwm_cfg->pwm_attr.shift_wf = (pwm_cfg->pwm_attr.shift * pwm_cfg->pwm_attr.divider * 100)
                                     / (pwm_cfg->pwm_attr.period * pwm_cfg->pwm_attr.divider);

#endif
    }
    else
    {
        pwm_cfg->pwm_attr.divider = pwm_clk_div[0];
        pwm_cfg->pwm_attr.divider--;
        hal_pmw_set_shift(pwm_cfg, 0, 0);
        hal_pwm_set_channel_reset(pwm_cfg, 1);
    }

    return 0;
}
