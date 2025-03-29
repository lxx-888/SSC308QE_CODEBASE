/*
 * drv_sdmmc_debug.h- Sigmastar
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

/***************************************************************************************************************
 *
 * FileName drv_sdmmc_debug.h
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 * 	   This file is the header file of ms_sdmmc_verify.c.
 *
 ***************************************************************************************************************/

#ifndef __SS_SDMMC_DEBUG_H
#define __SS_SDMMC_DEBUG_H

#include "hal_sdmmc_base.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define EXT_CSD_PART_CONF      179 /* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH 177
#define EXT_CSD_RST_n_FUNCTION 162

#define EXT_CSD_BOOT_ACK(x)         (x << 6)
#define EXT_CSD_BOOT_PART_NUM(x)    (x << 3)
#define EXT_CSD_PARTITION_ACCESS(x) (x << 0)

#define EXT_CSD_BOOT_BUS_WIDTH_MODE(x)  (x << 3)
#define EXT_CSD_BOOT_BUS_WIDTH_RESET(x) (x << 2)
#define EXT_CSD_BOOT_BUS_WIDTH_WIDTH(x) (x)

#define EXT_CSD_TEM_DISABLED 0
#define EXT_CSD_PER_ENABLED  BIT00_T
#define EXT_CSD_PER_DISABLED BIT01_T
#endif // End of __SS_SDMMC_DEBUG_H
