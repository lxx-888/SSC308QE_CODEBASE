/*
 * hal_timer.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#include <hal_timer.h>

void hal_timer_start(struct hal_timer_data *data, int rolled, unsigned long long expire)
{
    unsigned int timer_count_low;
    unsigned int timer_count_high;

    timer_count_low  = (expire & 0xFFFF);
    timer_count_high = (expire & 0xFFFF0000) >> 16;

    WRITE_WORD(data->reg_base + TIMER_EN_REG, 0);
    WRITE_WORD(data->reg_base + TIMER_MAX_L_REG, timer_count_low);
    WRITE_WORD(data->reg_base + TIMER_MAX_H_REG, timer_count_high);
    if (rolled)
        WRITE_WORD_MASK(data->reg_base + TIMER_EN_REG, TIMER_EN_BIT, TIMER_EN_BIT);
    else
        WRITE_WORD_MASK(data->reg_base + TIMER_EN_REG, TIMER_TRIG_BIT, TIMER_TRIG_BIT);

    return;
}

void hal_timer_stop(struct hal_timer_data *data)
{
    WRITE_WORD(data->reg_base + TIMER_EN_REG, 0);
    WRITE_WORD(data->reg_base + TIMER_MAX_L_REG, 0x0);
    WRITE_WORD(data->reg_base + TIMER_MAX_H_REG, 0x0);

    return;
}

unsigned long long hal_timer_count(struct hal_timer_data *data)
{
    unsigned long long timer_count;
    unsigned int       timer_count_low;
    unsigned int       timer_count_high;

    timer_count_low  = READ_WORD(data->reg_base + TIMER_CAP_L_REG);
    timer_count_high = READ_WORD(data->reg_base + TIMER_CAP_H_REG);
    timer_count      = (timer_count_high << 16) | timer_count_low;

    return timer_count;
}

int hal_timer_expried(struct hal_timer_data *data)
{
    return (READ_WORD(data->reg_base + TIMER_CAP_L_REG) == READ_WORD(data->reg_base + TIMER_MAX_L_REG))
           && (READ_WORD(data->reg_base + TIMER_CAP_H_REG) == READ_WORD(data->reg_base + TIMER_MAX_H_REG));
}

void hal_timer_interrupt_enable(struct hal_timer_data *data, int enable)
{
    if (enable)
        WRITE_WORD_MASK(data->reg_base + TIMER_EN_REG, TIMER_INT_EN_BIT, TIMER_INT_EN_BIT);
    else
        WRITE_WORD_MASK(data->reg_base + TIMER_EN_REG, ~TIMER_INT_EN_BIT, TIMER_INT_EN_BIT);

    return;
}

int hal_timer_interrupt_status(struct hal_timer_data *data)
{
    return (READ_WORD(data->reg_base + TIMER_HIT_REG) & TIMER_HIT_BIT) ? 1 : 0;
}

void hal_timer_interrupt_clear(struct hal_timer_data *data)
{
    WRITE_WORD_MASK(data->reg_base + TIMER_HIT_REG, TIMER_HIT_BIT, TIMER_HIT_BIT);
    WRITE_WORD_MASK(data->reg_base + TIMER_HIT_REG, ~TIMER_HIT_BIT, TIMER_HIT_BIT);

    return;
}
