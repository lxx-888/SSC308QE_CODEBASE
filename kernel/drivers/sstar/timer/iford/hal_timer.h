/*
 * hal_timer.h - Sigmastar
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

#ifndef __HAL_DATA_H__
#define __HAL_DATA_H__

#define READ_WORD(_addr)        (*(volatile unsigned short *)(_addr))
#define WRITE_WORD(_addr, _val) (*((volatile unsigned short *)(_addr))) = (unsigned short)(_val)
#define WRITE_WORD_MASK(_addr, _val, _mask) \
    (*((volatile unsigned short *)(_addr))) = ((READ_WORD(_addr) & ~(_mask)) | ((unsigned short)(_val) & (_mask)))

#define TIMER_EN_BIT     (1 << 0)
#define TIMER_TRIG_BIT   (1 << 1)
#define TIMER_INT_EN_BIT (1 << 8)
#define TIMER_HIT_BIT    (1 << 0)
#define TIMER_EN_REG     (0x0 << 2)
#define TIMER_HIT_REG    (0x1 << 2)
#define TIMER_MAX_L_REG  (0x2 << 2)
#define TIMER_MAX_H_REG  (0x3 << 2)
#define TIMER_CAP_L_REG  (0x4 << 2)
#define TIMER_CAP_H_REG  (0x5 << 2)

struct hal_timer_data
{
    void *reg_base;
};

void               hal_timer_start(struct hal_timer_data *data, int rolled, unsigned long long expire);
void               hal_timer_stop(struct hal_timer_data *data);
unsigned long long hal_timer_count(struct hal_timer_data *data);
int                hal_timer_expried(struct hal_timer_data *data);
void               hal_timer_interrupt_enable(struct hal_timer_data *data, int enable);
int                hal_timer_interrupt_status(struct hal_timer_data *data);
void               hal_timer_interrupt_clear(struct hal_timer_data *data);
#endif
