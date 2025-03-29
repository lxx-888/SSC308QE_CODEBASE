/*
 * rtcpwc_os.h- Sigmastar
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

#ifndef __RTCPWC_OS__
#define __RTCPWC_OS__

#include <linux/delay.h>
#include <sstar_types.h>
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/io.h>

#define RTC_DEBUG 0

#if RTC_DEBUG
#define RTC_DBG(fmt, arg...) printk("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define RTC_DBG(fmt, arg...)
#endif
#define RTC_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

#define rtc_os_udelay(usec) udelay(usec)
#define rtc_os_usleep(usec)
#define rtc_os_mdelay(msec) mdelay(msec)
#define rtc_os_msleep(msec)

#endif
