/*
 * hal_mspireg.h- Sigmastar
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

#ifndef __MHAL_MSPI_REG_H__
#define __MHAL_MSPI_REG_H__

#define HAL_MSPI_REG_WRITE_BUF 0x40
#define HAL_MSPI_REG_READ_BUF  0x44
#define HAL_MSPI_REG_WBF_SIZE  0x48
#define HAL_MSPI_REG_RBF_SIZE  0x48
// read/ write buffer size
#define HAL_MSPI_REG_RWSIZE_MASK   0xFF
#define HAL_MSPI_REG_RSIZE_BIT     0x8
#define HAL_MSPI_REG_READ_BUF_MAX  0x8
#define HAL_MSPI_REG_WRITE_BUF_MAX 0x8
// CLK config
#define HAL_MSPI_CTRL                  0x49
#define HAL_MSPI_REG_CLK_CLOCK         0x49
#define HAL_MSPI_REG_CLK_CLOCK_BIT     0x08
#define HAL_MSPI_REG_CLK_CLOCK_MASK    0xFF
#define HAL_MSPI_REG_CLK_PHASE_MASK    0x40
#define HAL_MSPI_REG_3WIREMODE_MASK    0x10
#define HAL_MSPI_REG_CLK_PHASE_BIT     0x06
#define HAL_MSPI_REG_CLK_POLARITY_MASK 0x80
#define HAL_MSPI_REG_CLK_POLARITY_BIT  0x07
#define HAL_MSPI_REG_CLK_PHASE_MAX     0x1
#define HAL_MSPI_REG_CLK_POLARITY_MAX  0x1
#define HAL_MSPI_REG_CLK_CLOCK_MAX     0x7
// DC config
#define HAL_MSPI_REG_DC_MASK        0xFF
#define HAL_MSPI_REG_DC_BIT         0x08
#define HAL_MSPI_REG_DC_TR_START    0x4A
#define HAL_MSPI_REG_DC_TRSTART_MAX 0xFF
#define HAL_MSPI_REG_DC_TR_END      0x4A
#define HAL_MSPI_REG_DC_TREND_MAX   0xFF
#define HAL_MSPI_REG_DC_TB          0x4B
#define HAL_MSPI_REG_DC_TB_MAX      0xFF
#define HAL_MSPI_REG_DC_TRW         0x4B
#define HAL_MSPI_REG_DC_TRW_MAX     0xFF
// Frame Config
#define HAL_MSPI_REG_FRAME_WBIT      0x4C
#define HAL_MSPI_REG_FRAME_RBIT      0x4E
#define HAL_MSPI_REG_FRAME_BIT_MAX   0x07
#define HAL_MSPI_REG_FRAME_BIT_MASK  0x07
#define HAL_MSPI_REG_FRAME_BIT_FIELD 0x03
#define HAL_MSPI_REG_RLSB_FIRST      0x34
#define HAL_MSPI_REG_WLSB_FIRST      0x35
#define HAL_MSPI_REG_RLSB_FIRST_BIT  0x01
#define HAL_MSPI_REG_WLSB_FIRST_BIT  0x01
#define HAL_MSPI_REG_TRIGGER         0x5A
#define HAL_MSPI_REG_DONE            0x5B
#define HAL_MSPI_REG_DONE_CLEAR      0x5C
#define HAL_MSPI_REG_CHIP_CS         0x5F

#define HAL_MSPI_REG_CLK_NOT_GATED      0x76
#define HAL_MSPI_REG_CLK_NOT_GATED_MASK 0x100

#define HAL_MSPI_REG_FULL_DEPLUX 0x78

// chip select bit map
#define HAL_MSPI_REG_CHIP_CS_MAX 0x07
// control bit
#define HAL_MSPI_REG_DONE_FLAG  0x01
#define HAL_MSPI_REG_TRIG       0x01
#define HAL_MSPI_REG_CLEAR_DONE 0x01
#define HAL_MSPI_REG_INT_ENABLE 0x04
#define HAL_MSPI_REG_RESET      0x02
#define HAL_MSPI_REG_ENABLE     0x01

// spi dma
#define HAL_MSPI_REG_DMA_DATA_LEN_L 0x30
#define HAL_MSPI_REG_DMA_DATA_LEN_H 0x31
#define HAL_MSPI_REG_DMA_ENABLE     0x32
#define HAL_MSPI_REG_DMA_RW_MODE    0x33
#define HAL_MSPI_REG_DMA_WRITE      0x00
#define HAL_MSPI_REG_DMA_READ       0x01

#define HAL_MSPI_CLK_DIV_VAL          \
    {                                 \
        2, 4, 8, 16, 32, 64, 128, 256 \
    }

#endif
