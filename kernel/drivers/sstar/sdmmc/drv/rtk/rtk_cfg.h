/*
 * rtk_cfg.h- Sigmastar
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
 * FileName rtk_cfg.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#ifndef __RTK_CFG_H
#define __RTK_CFG_H

#include "cam_os_wrapper.h"

#if defined(__KERNEL__)
#define ONLY_FOR_LNX
#endif

// power_mode
#define MMC_POWER_OFF       0
#define MMC_POWER_UP        1
#define MMC_POWER_ON        2
#define MMC_POWER_UNDEFINED 3

// bus_width
#define MMC_BUS_WIDTH_1 0
#define MMC_BUS_WIDTH_4 2
#define MMC_BUS_WIDTH_8 3

// timing
#define MMC_TIMING_LEGACY     0
#define MMC_TIMING_MMC_HS     1
#define MMC_TIMING_SD_HS      2
#define MMC_TIMING_UHS_SDR12  3
#define MMC_TIMING_UHS_SDR25  4
#define MMC_TIMING_UHS_SDR50  5
#define MMC_TIMING_UHS_SDR104 6
#define MMC_TIMING_UHS_DDR50  7
#define MMC_TIMING_MMC_DDR52  8
#define MMC_TIMING_MMC_HS200  9
#define MMC_TIMING_MMC_HS400  10

struct msSt_SD_IOS
{
    unsigned int  clock;      /* clock rate */
    unsigned char power_mode; /* power supply mode */
    unsigned char bus_width;  /* data bus width */
    unsigned char timing;     /* timing specification used */
    unsigned char pad_volt;   /* pad voltage (1.8V or 3.3V) */

#define SD_PAD_VOLT_330 0
#define SD_PAD_VOLT_180 1

    unsigned char drv_type; /* driver type (A, B, C, D) */

#define SD_DRV_TYPE_B 0
#define SD_DRV_TYPE_A 1
#define SD_DRV_TYPE_C 2
#define SD_DRV_TYPE_D 3
};

struct msSt_SD_SlotInfo
{
    unsigned int  currClk;     // Current Clock
    unsigned int  currRealClk; // Current Real Clock
    unsigned char currPowrMode;
    unsigned char currWidth;   // Current Bus Width
    unsigned char currTiming;  // Current Bus Timning
    unsigned char currPadVolt; // Current Pad Voltage

    unsigned char read_only; // WP
    unsigned char card_det;  // Card Detect
};

#endif // __RTK_CFG_H