/*
 * hal_audio_sys.h - Sigmastar
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

#ifndef __HAL_AUDIO_SYS_H__
#define __HAL_AUDIO_SYS_H__

////////////////////////////////////////////////////////////////////////////
//
// Config by platform
//
////////////////////////////////////////////////////////////////////////////

// -----------------------
// Common Fucntion
// ------------------------
#include "audio_ausdm0_sw_reg_define.h"
#include "audio_bank1_sw_reg_define.h"
#include "audio_bank2_sw_reg_define.h"
#include "audio_bank3_sw_reg_define.h"
#include "audio_bank4_sw_reg_define.h"

// No exhaust space for write AO dma buffer, No exhaust data for read AI dma buffer.
#define NO_EXHAUST_DMA_BUF (0)
#define DMA_BUF_REMAINDER  (128) // 128 Bytes ( 8 MIU Line )

#define MIU_WORD_BYTE_SIZE (16) // bytes
#define DMA_LOCALBUF_SIZE  (64) // bytes
#define DMA_LOCALBUF_LINE  (DMA_LOCALBUF_SIZE / MIU_WORD_BYTE_SIZE)
#define DMA_EMPTY_THD      (4) // Later must check the HW empty threshold for Muffin

#define BACH_DPGA_GAIN_MAX_DB  (64)
#define BACH_DPGA_GAIN_MIN_DB  (-64) // actually -63.875 dB
#define BACH_DPGA_GAIN_MIN_IDX (0x1FF)

//
#define CHIP_ADC_GAIN_STEP_TOTAL (22)
extern const U16 g_aMicInCombineGainTable[CHIP_ADC_GAIN_STEP_TOTAL][2];

#define FREQ_48MHZ       (48000000)
#define FREQ_147_456MHZ  (147456000)
#define FREQ_186_6336MHZ (180633600)
#define FREQ_196_608MHZ  (196608000)
#define FREQ_384MHZ      (384000000)

#define I2S_8DIV_MAX_FREQ           (96000000)
#define I2S_MAX_FREQ                (49152000)
#define I2S_AUPLL_DIV_SUITABLE_FREQ 0 // 3072000
#define ADJUSTABLE_PLL              1

#define MAX_DIGMIC_CHN 8

#define SSTAR_FOR_FPGA_TEST 0
//------------------------------------------------------------------------------
#if SSTAR_FOR_FPGA_TEST
#define REF_FREQ 48 // 48MHz for FPGA
#else
#define REF_FREQ       384 // 384MHz
#define REF_FREQ_AUPLL 48  // AUPLL   48MHz
#endif

#define ADJUSTABLE_PLL 1

#endif // __HAL_AUDIO_SYS_H__
