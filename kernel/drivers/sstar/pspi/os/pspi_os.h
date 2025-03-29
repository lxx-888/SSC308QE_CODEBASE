/*
 * pspi_os.h- Sigmastar
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

#ifndef _PSPI_OS_H_
#define _PSPI_OS_H_

#include <cam_os_wrapper.h>
#include <linux/types.h>
#if (defined CONFIG_SSTAR_GPIO) || (defined CONFIG_GPIO_SUPPORT)
#include <drv_gpio.h>
#include <hal_pinmux.h>
#include <hal_gpio.h>

#endif

#define pspi_bdma_param       hal_bdma_param
#define pspi_gpio_pull_up     sstar_gpio_pull_up
#define pspi_gpio_pad_set     hal_gpio_pad_set
#define pspi_gpio_pad_set_val hal_gpio_pad_set_val
#define pspi_gpio_pad_get_val hal_gpio_pad_get_val
#define OS_BASEADDR           0xfd000000

#define PSPI_BDMA_CM4_IMI_TO_MIU0 HAL_BDMA_CM4_IMI_TO_MIU0
#define PSPI_BDMA_MIU0_TO_CM4_IMI HAL_BDMA_MIU0_TO_CM4_IMI
#define PSPI_BDMA2_CH1            HAL_BDMA2_CH1

#endif
