/*
 * hal_card_platform_pri_config.h- Sigmastar
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

// PADMUX_SET
#define PADMUX_SET_BY_FUNC (0)
#define PADMUX_SET_BY_REG  (1)

// GPIO_SET
#define GPIO_SET_BY_FUNC (0)
#define GPIO_SET_BY_REG  (1)

#if 1 //(D_OS == D_OS__LINUX)
#define PADMUX_SET (PADMUX_SET_BY_FUNC)
#define GPIO_SET   (GPIO_SET_BY_FUNC)

#define FORCE_SWITCH_PAD (FALSE)
#else
#define PADMUX_SET (PADMUX_SET_BY_REG)
#define GPIO_SET   (GPIO_SET_BY_REG)

#define FORCE_SWITCH_PAD (TRUE)
#endif

#define PHYS_TO_MIU_USE_FUNC 0

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
