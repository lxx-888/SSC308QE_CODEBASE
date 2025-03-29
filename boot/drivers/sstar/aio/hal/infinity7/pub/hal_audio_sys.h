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

// No exhaust space for write AO dma buffer, No exhaust data for read AI dma buffer.
#define NO_EXHAUST_DMA_BUF (0)
#define DMA_BUF_REMAINDER  (128) // 128 Bytes ( 8 MIU Line )

#define MIU_WORD_BYTE_SIZE (16) // bytes
#define DMA_LOCALBUF_SIZE  (64) // bytes
#define DMA_LOCALBUF_LINE  (DMA_LOCALBUF_SIZE / MIU_WORD_BYTE_SIZE)
#define DMA_EMPTY_THD      (4) // Later must check the HW empty threshold for I7

#define BACH_DPGA_GAIN_MAX_DB  (34)
#define BACH_DPGA_GAIN_MIN_DB  (-64) // actually -63.5 dB
#define BACH_DPGA_GAIN_MIN_IDX (0x7F)

//
#define CHIP_ADC_GAIN_STEP_TOTAL (22)
extern const U16 g_aMicInCombineGainTable[CHIP_ADC_GAIN_STEP_TOTAL][2];

#endif // __HAL_AUDIO_SYS_H__