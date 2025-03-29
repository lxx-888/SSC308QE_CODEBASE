/*
 * hal_sdmmc_platform_pri_config.h- Sigmastar
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

#ifndef __HAL_SDMMC_PLATFORM_PRI_CONFIG_H
#define __HAL_SDMMC_PLATFORM_PRI_CONFIG_H

#define SDMMC_NUM_TOTAL (3)

#define IP_0_TYPE IP_TYPE_SDIO
#define IP_1_TYPE IP_TYPE_SDIO
#define IP_2_TYPE IP_TYPE_FCIE

typedef enum
{
    IP_SD   = IP_ORDER_0,
    IP_SDIO = IP_ORDER_1,
    IP_FCIE = IP_ORDER_2,
    IP_TOTAL,

} IpSelect;

typedef enum
{
    PAD_SD_MD1 = PAD_ORDER_0, // PAD_SD_MD1
    PAD_SD_MD2 = PAD_ORDER_1, // PAD_SD_MD2
    PAD_SD_MD3 = PAD_ORDER_2, // PAD_SD_MD3
    PAD_SD_MD4 = PAD_ORDER_3, // PAD_SD_MD4
    PAD_SD_MD5 = PAD_ORDER_4, // PAD_SD_MD5

} PadSelect;

#endif // End of __HAL_SDMMC_PLATFORM_PRI_CONFIG_H
