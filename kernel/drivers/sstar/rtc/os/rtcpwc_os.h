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

#include <ms_platform.h>
#include <ms_types.h>
#include <ms_msys.h>
#include <cam_os_wrapper.h>

#define INTEROS_L2R_RTC_TIME_SET_INFO_SYNC 0xF20FF200
#define INTEROS_R2L_RTC_TIME_SET_INFO_SYNC 0xF20FF201

#define RTC_DEBUG 0

#if RTC_DEBUG
#define RTC_DBG(fmt, arg...) printk("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define RTC_DBG(fmt, arg...)
#endif
#define RTC_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

#define rtc_os_udelay(usec) CamOsUsDelay(usec)
#define rtc_os_usleep(usec) CamOsUsSleep(usec)
#define rtc_os_mdelay(msec) CamOsMsDelay(msec)
#define rtc_os_msleep(msec) CamOsMsSleep(msec)

#endif
