/*
 * drv_ive_common.h- Sigmastar
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
#ifndef _DRV_IVE_COMMON_H_
#define _DRV_IVE_COMMON_H_
//---------------------------------------------------------------------------
// INCLUDE
//---------------------------------------------------------------------------
#include "drv_ive_datatype.h"

//---------------------------------------------------------------------------
// MACRO
//---------------------------------------------------------------------------
#define DRV_IVE_DEVICE_COUNT    (1)
#define DRV_IVE_MINOR           (0)
#define DRV_IVE_MAX_QUEUE_COUNT (16)

//---------------------------------------------------------------------------
// FUNCTION
//---------------------------------------------------------------------------
void drv_ive_isr(u32 irq, void *data);
void drv_ive_isr_pre_proc(ive_dev_data *dev_data);
void drv_ive_isr_post_proc(void *pDevData);
void drv_ive_reset_status(ive_file_data *file_data);

#endif //_DRV_IVE_COMMON_H_
