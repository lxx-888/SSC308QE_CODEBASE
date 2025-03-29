/*
 * drv_ive_clk.h- Sigmastar
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
#ifndef _DRV_IVE_CLK_H_
#define _DRV_IVE_CLK_H_
//---------------------------------------------------------------------------
// MACRO
//---------------------------------------------------------------------------
#define IVE_CLK_ENABLE_ON  (1)
#define IVE_CLK_ENABLE_OFF (0)

//---------------------------------------------------------------------------
// INCLUDE
//---------------------------------------------------------------------------
#include "drv_ive_datatype.h"

//---------------------------------------------------------------------------
// FUNCTION
//---------------------------------------------------------------------------
int  drv_ive_clock_init(ive_dev_data *dev_data);
void drv_ive_clock_release(ive_dev_data *dev_data);
int  drv_ive_clock_enable(ive_dev_data *dev_data);
void drv_ive_clock_disable(ive_dev_data *dev_data);
#endif //_DRV_IVE_CLK_H_
