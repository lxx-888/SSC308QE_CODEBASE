/*
 * hal_wdt.h - Sigmastar
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

#ifndef __HAL_WDT_H__
#define __HAL_WDT_H__

#define READ_WORD(_addr)        (*(volatile unsigned short *)(_addr))
#define WRITE_WORD(_addr, _val) (*((volatile unsigned short *)(_addr))) = (unsigned short)(_val)
#define WRITE_WORD_MASK(_addr, _val, _mask) \
    (*((volatile unsigned short *)(_addr))) = ((READ_WORD(_addr) & ~(_mask)) | ((unsigned short)(_val) & (_mask)))

#define REG_WDT_CLR            (0x00 << 2)
#define REG_WDT_INT_15_00      (0x03 << 2)
#define REG_WDT_INT_23_16      (0x09 << 2)
#define REG_WDT_INT_23_16_LSB  (0x00)
#define REG_WDT_INT_23_16_MASK (0xFF << REG_WDT_INT_23_16_LSB)
#define REG_WDT_MAX_15_00      (0x04 << 2)
#define REG_WDT_MAX_31_16      (0x05 << 2)
#define REG_WDT_MAX_39_32      (0x09 << 2)
#define REG_WDT_MAX_39_32_LSB  (0x08)
#define REG_WDT_MAX_39_32_MASK (0xFF << REG_WDT_MAX_39_32_LSB)
#define REG_WDT_CNT_15_00      (0x06 << 2)
#define REG_WDT_CNT_31_16      (0x07 << 2)
#define REG_WDT_CNT_39_32      (0x08 << 2)

struct hal_wdt_data
{
    void *reg_base;
};

unsigned long long hal_wdt_get_count(struct hal_wdt_data *data);
unsigned long long hal_wdt_get_max_count(void);
void               hal_wdt_set_int(struct hal_wdt_data *data, unsigned long long int_ticks);
void               hal_wdt_set_max(struct hal_wdt_data *data, unsigned long long max_ticks);
void               hal_wdt_ping(struct hal_wdt_data *data);
void               hal_wdt_start(struct hal_wdt_data *data, unsigned long long int_ticks, unsigned long long max_ticks);
void               hal_wdt_stop(struct hal_wdt_data *data);
#endif /* __HAL_WDT_H__ */
