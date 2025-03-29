/*
 * ir_os.h- Sigmastar
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

#ifndef _IR_OS_H_
#define _IR_OS_H_

#include <cam_os_wrapper.h>

#define IR_DEBUG 0
#if IR_DEBUG
#define ir_err(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#define ir_dbg(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#else
#define ir_err(fmt, ...) CamOsPrintf("%s: " fmt, __func__, ##__VA_ARGS__)
#define ir_dbg(fmt, ...)
#endif

#endif
