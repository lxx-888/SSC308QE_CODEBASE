/*
 * hal_spdif.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#include "hal_audio_common.h"
#include "hal_audio_sys.h"
#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"

#include <drv_padmux.h>
#include <drv_puse.h>
#include <drv_gpio.h>

int HalAudSpdifEnable(CHIP_AI_SPDIF_e eSpdif, BOOL bEn)
{
    return AIO_OK;
}

int HalAudSpdifSetRate(CHIP_AI_SPDIF_e eSpdif, U32 nSampleRate)
{
    return AIO_OK;
}

int HalAudSpdifSetPadmux(CHIP_AI_SPDIF_e eSpdif)
{
    return AIO_OK;
}
