/*
 * drv_timer.h - Sigmastar
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

#ifndef _DRV_TIMER_H_
#define _DRV_TIMER_H_

typedef void (*sstar_timer_callback)(void *pdata);
typedef void *sstar_timer_handle;

enum sstar_timer_mode
{
    SSTAR_TIMER_MODE_ONESHOT,
    SSTAR_TIMER_MODE_RUNLOOP,
};

sstar_timer_handle sstar_timer_register(unsigned int timer_id, enum sstar_timer_mode mode,
                                        sstar_timer_callback callback, void *pdata);
int                sstar_timer_unregister(sstar_timer_handle handle);
int                sstar_timer_start(sstar_timer_handle handle, unsigned long long exp_time);
int                sstar_timer_stop(sstar_timer_handle handle);
int                sstar_timer_get_current(sstar_timer_handle handle, unsigned long long *ptime);
int                sstar_timer_device_count(void);
int                sstar_timer_find_idle(void);

#endif /* _DRV_TIMER_H_ */
