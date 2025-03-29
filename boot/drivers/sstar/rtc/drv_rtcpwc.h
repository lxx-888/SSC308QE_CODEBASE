/*
 * drv_rtcpwc.h - Sigmastar
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

#ifndef _DRV_RTCPWC_H_
#define _DRV_RTCPWC_H_

extern void sstar_rtc_init(void);
extern void sstar_rtc_poweroff(void);
extern void sstar_rtc_sw3_set(u32 val);
extern u32  sstar_rtc_sw3_get(void);

#endif
