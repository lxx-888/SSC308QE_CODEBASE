/*
 * adclp_os.h- Sigmastar
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

#ifndef _ADCLP_OS_H_
#define _ADCLP_OS_H_

#include <linux/delay.h>
#include <sstar_types.h>

#define ADCLP_DBG 0
#if ADCLP_DBG
#define adclp_dbg(fmt, ...) printf("[%s]: " fmt, __func__, ##__VA_ARGS__);
#define adclp_err(fmt, ...) printf("[%s]: " fmt, __func__, ##__VA_ARGS__);
#else
#define adclp_dbg(fmt, ...)
#define adclp_err(fmt, ...) printf("[%s]: " fmt, __func__, ##__VA_ARGS__);
#endif

#define HAL_ADCLP_DELAY(_x) udelay(_x)

#endif
