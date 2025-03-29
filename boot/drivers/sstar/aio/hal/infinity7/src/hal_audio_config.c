/*
 * hal_audio_config.c - Sigmastar
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
#include "hal_audio_config.h"
#include "hal_audio_reg.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_pri_api.h"

////////////////////////////////////////////////////////////////////////////
//
// Just config it !
//
////////////////////////////////////////////////////////////////////////////
const U16 g_aMicInCombineGainTable[CHIP_ADC_GAIN_STEP_TOTAL][2] = {
    //  {Pre_gain, ADC_gain},
    {0, 0},   //  0: 0dB
    {1, 0},   //  1: 3dB
    {2, 0},   //  2: 6dB
    {3, 0},   //  3: 9dB
    {4, 0},   //  4: 12dB
    {5, 0},   //  5: 15dB
    {6, 0},   //  6: 18dB
    {7, 0},   //  7: 21dB
    {8, 0},   // 8: 24db
    {9, 0},   // 9: 27db
    {0xA, 0}, // 10: 30db
    {0xB, 0}, // 11: 33db
    {0xC, 0}, // 12: 36db
    {0xD, 0}, // 13: 39db
    {8, 3},   // 14: 24 + 18 = 42db
    {9, 3},   // 15: 27 + 18 = 45db
    {0xA, 3}, // 16: 30 + 18 = 48db
    {0xB, 3}, // 17: 33 + 18 = 51db
    {0xC, 3}, // 18: 36 + 18 = 54db
    {0xD, 3}, // 19: 39 + 18 = 57db
};
