/*
 * drv_ive_io.h- Sigmastar
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

// this file only used for rtk.
#ifndef _DRV_IVE_IO_H_
#define _DRV_IVE_IO_H_

#include "drv_ive_io_st.h"

void* drv_ive_module_open(void);
int   drv_ive_module_release(void* pfile);
long  drv_ive_module_ioctl(void* pfile, ive_ioc_config ioc_config);
int   drv_ive_module_poll(void* pfile, int mSec);

#endif // _DRV_IVE_IO_H_
