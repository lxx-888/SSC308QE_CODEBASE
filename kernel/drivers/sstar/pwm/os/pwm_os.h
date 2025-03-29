/*
 * pwm_os.h- Sigmastar
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

#ifndef _PWM_OS_H_
#define _PWM_OS_H_

#include <cam_os_wrapper.h>

#define PWM_DEBUG 0
#if PWM_DEBUG
#define pwm_err(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#define pwm_dbg(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#else
#define pwm_err(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#define pwm_dbg(fmt, ...)
#endif

#define HAL_PWM_DELAY_US(x) CamOsUsDelay(x)

#endif
