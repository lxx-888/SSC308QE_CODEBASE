/*
 * hal_wdt.c - Sigmastar
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

#include <hal_wdt.h>

unsigned long long hal_wdt_get_count(struct hal_wdt_data *data)
{
    unsigned long long count = 0;

    count |= (unsigned long long)READ_WORD(data->reg_base + REG_WDT_CNT_39_32) << 32;
    count |= (unsigned long long)READ_WORD(data->reg_base + REG_WDT_CNT_31_16) << 16;
    count |= (unsigned long long)READ_WORD(data->reg_base + REG_WDT_CNT_15_00);

    return count;
}

unsigned long long hal_wdt_get_max_count(void)
{
    return (1ULL << 40) - 1;
}

void hal_wdt_set_int(struct hal_wdt_data *data, unsigned long long int_ticks)
{
    WRITE_WORD_MASK(data->reg_base + REG_WDT_INT_23_16, (int_ticks >> 32) << REG_WDT_INT_23_16_LSB,
                    REG_WDT_INT_23_16_MASK);
    WRITE_WORD(data->reg_base + REG_WDT_INT_15_00, (int_ticks >> 16));
    return;
}

void hal_wdt_set_max(struct hal_wdt_data *data, unsigned long long max_ticks)
{
    /* Set register MSB first then LSB to start WDT safely. */
    WRITE_WORD_MASK(data->reg_base + REG_WDT_MAX_39_32, (max_ticks >> 32) << REG_WDT_MAX_39_32_LSB,
                    REG_WDT_MAX_39_32_MASK);
    WRITE_WORD(data->reg_base + REG_WDT_MAX_31_16, (max_ticks >> 16));
    WRITE_WORD(data->reg_base + REG_WDT_MAX_15_00, max_ticks);
    return;
}

void hal_wdt_ping(struct hal_wdt_data *data)
{
    WRITE_WORD(data->reg_base + REG_WDT_CLR, 1);
    return;
}

void hal_wdt_start(struct hal_wdt_data *data, unsigned long long int_ticks, unsigned long long max_ticks)
{
    hal_wdt_set_int(data, int_ticks);
    hal_wdt_set_max(data, max_ticks);
    hal_wdt_ping(data);
    return;
}

void hal_wdt_stop(struct hal_wdt_data *data)
{
    WRITE_WORD(data->reg_base + REG_WDT_CLR, 0);
    /* Clear register LSB first then MSB to stop WDT safely.
     * Do not replace the code below with 'hal_wdt_set_max'.
     */
    WRITE_WORD(data->reg_base + REG_WDT_MAX_15_00, 0x0000);
    WRITE_WORD(data->reg_base + REG_WDT_MAX_31_16, 0x0000);
    WRITE_WORD_MASK(data->reg_base + REG_WDT_MAX_39_32, 0x0000, REG_WDT_MAX_39_32_MASK);
    return;
}
