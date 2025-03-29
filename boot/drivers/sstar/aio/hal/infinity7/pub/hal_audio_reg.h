/*
 * hal_audio_reg.h - Sigmastar
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

#ifndef __HAL_AUDIO_REG_H__
#define __HAL_AUDIO_REG_H__

//=============================================================================
// Include files
//=============================================================================

//=============================================================================
// Extern definition
//=============================================================================
#define BACH_RIU_BASE_ADDR 0x1f000000
#define BACH_REG_BANK_1    0x150200
#define BACH_REG_BANK_2    0x150300
#define BACH_REG_BANK_3    0x150400
#define BACH_REG_BANK_4    0x150500
#define BACH_REG_BANK_5    0x103400

typedef enum
{
    E_BACH_REG_BANK1,
    E_BACH_REG_BANK2,
    E_BACH_REG_BANK3,
    E_BACH_REG_BANK4,
    E_BACH_REG_BANK5
} BachRegBank_e;

//=============================================================================
// Macro definition
//=============================================================================
//#define AUDIO_IO_ADDRESS(x) ((long)(x) + MS_IO_OFFSET)

#define WRITE_BYTE(_reg, _val) (*((volatile U8*)((U64)_reg))) = (U8)(_val)
#define WRITE_WORD(_reg, _val) (*((volatile U16*)((U64)_reg))) = (U16)(_val)
#define WRITE_LONG(_reg, _val) (*((volatile U32*)((U64)_reg))) = (U32)(_val)
#define READ_BYTE(_reg)        (*(volatile U8*)((U64)(_reg)))
#define READ_WORD(_reg)        (*(volatile U16*)((U64)_reg))
#define READ_LONG(_reg)        (*(volatile U32*)((U64)_reg))

/****************************************************************************/
/*        AUDIO DIGITAL registers                                           */
/****************************************************************************/
enum
{
    // ----------------------------------------------
    // BANK1 SYS
    // ----------------------------------------------
    E_BACH_ENABLE_CTRL       = 0x00,
    E_BACH_SR0_SEL           = 0x02,
    E_BACH_MUX0_SEL          = 0x06,
    E_BACH_MIX1_SEL          = 0x0E,
    E_BACH_SDM_OFFSET        = 0x10,
    E_BACH_SDM_CTRL          = 0x12,
    E_BACH_CODEC_I2S_RX_CTRL = 0x24,
    E_BACH_CODEC_I2S_TX_CTRL = 0x26,
    E_BACH_SYNTH_CTRL        = 0x3A,
    E_BACH_PAD0_CFG          = 0x3C,
    E_BACH_MMC1_DPGA_CFG1    = 0x40,
    E_BACH_MMC1_DPGA_CFG2    = 0x42,
    E_BACH_MMC2_DPGA_CFG1    = 0x44,
    E_BACH_MMC2_DPGA_CFG2    = 0x46,
    E_BACH_ADC_DPGA_CFG1     = 0x48,
    E_BACH_ADC_DPGA_CFG2     = 0x4A,
    E_BACH_AEC1_DPGA_CFG1    = 0x50,
    E_BACH_AEC1_DPGA_CFG2    = 0x52,
    E_BACH_MMCDEC1_DPGA_CFG1 = 0x60,
    E_BACH_MMCDEC1_DPGA_CFG2 = 0x62,
    E_BACH_MMCDEC2_DPGA_CFG1 = 0x64,
    E_BACH_MMCDEC2_DPGA_CFG2 = 0x66,
    E_BACH_AU2HDMI_DPGA_CFG1 = 0xF6,
    E_BACH_AU2HDMI_DPGA_CFG2 = 0xF8
};

enum
{
    // ----------------------------------------------
    // BANK1 DMA
    // ----------------------------------------------
    E_BACH_DMA1_CTRL_0      = 0x80,
    E_BACH_DMA1_CTRL_1      = 0x82,
    E_BACH_DMA1_CTRL_2      = 0x84,
    E_BACH_DMA1_CTRL_3      = 0x86,
    E_BACH_DMA1_CTRL_4      = 0x88,
    E_BACH_DMA1_CTRL_5      = 0x8A,
    E_BACH_DMA1_CTRL_6      = 0x8C,
    E_BACH_DMA1_CTRL_7      = 0x8E,
    E_BACH_DMA1_CTRL_8      = 0x90,
    E_BACH_DMA1_CTRL_9      = 0x92,
    E_BACH_DMA1_CTRL_10     = 0x94,
    E_BACH_DMA1_CTRL_11     = 0x96,
    E_BACH_DMA1_CTRL_12     = 0x98,
    E_BACH_DMA1_CTRL_13     = 0x9A,
    E_BACH_DMA1_CTRL_14     = 0x9C,
    E_BACH_DMA1_CTRL_15     = 0x9E,
    E_BACH_DMA2_CTRL_0      = 0xA0, // BANK1_DMA2
    E_BACH_DMA2_CTRL_1      = 0xA2,
    E_BACH_DMA2_CTRL_2      = 0xA4,
    E_BACH_DMA2_CTRL_3      = 0xA6,
    E_BACH_DMA2_CTRL_4      = 0xA8,
    E_BACH_DMA2_CTRL_5      = 0xAA,
    E_BACH_DMA2_CTRL_6      = 0xAC,
    E_BACH_DMA2_CTRL_7      = 0xAE,
    E_BACH_DMA2_CTRL_8      = 0xB0,
    E_BACH_DMA2_CTRL_9      = 0xB2,
    E_BACH_DMA2_CTRL_10     = 0xB4,
    E_BACH_DMA2_CTRL_11     = 0xB6,
    E_BACH_DMA2_CTRL_12     = 0xB8,
    E_BACH_DMA2_CTRL_13     = 0xBA,
    E_BACH_DMA2_CTRL_14     = 0xBC,
    E_BACH_DMA2_CTRL_15     = 0xBE,
    E_BACH_EN_CTRL_AU2      = 0xC8,
    E_BACH_SR0_SEL_AU2      = 0xCA,
    E_BACH_MUX4_SEL         = 0xCC,
    E_BACH_DMA_TEST_CTRL5   = 0xEA,
    E_BACH_DMA_TEST_CTRL6   = 0xEC,
    E_BACH_DMA_TEST_CTRL7   = 0xEE,
    E_BACH_AU2HDMI_CTRL     = 0xF0,
    E_BACH_AU2HDMI_REF_FS   = 0xF2,
    E_BACH_DMA_TEST_CTRL_18 = 0xF4,
    E_BACH_DAM_TEST_CTRL8   = 0xF4, // 64 bit ext
};

enum
{
    // ----------------------------------------------
    // BANK2 SYS
    // ----------------------------------------------
    E_BACH_INT_EN            = 0x0E,
    E_BACH_MUX1_SEL          = 0x1C,
    E_BACH_AU_SYS_CTRL1      = 0x2C,
    E_BACH_AU_SYS_CTRL2      = 0x2E,
    E_BACH_DIG_MIC_CTRL0     = 0x3A,
    E_BACH_DIG_MIC_CTRL1     = 0x3C,
    E_BACH_I2S_TDM_CFG00     = 0x60,
    E_BACH_I2S_TDM_CFG01     = 0x62,
    E_BACH_I2S_TDM_CFG02     = 0x64,
    E_BACH_I2S_TDM_CFG05     = 0x6A,
    E_BACH_I2S_TDM_CFG06     = 0x6C,
    E_BACH_I2S_TDM_DPGA_CFG1 = 0x70,
    E_BACH_I2S_TDM_DPGA_CFG2 = 0x72,
    E_BACH_I2S_CFG00         = 0x78,
    E_BACH_NF_SYNTH_RX_BCK00 = 0x82,
    E_BACH_NF_SYNTH_RX_BCK01 = 0x84,
    E_BACH_NF_SYNTH_RX_BCK02 = 0x86,
    E_BACH_NF_SYNTH_TX_BCK00 = 0x88,
    E_BACH_NF_SYNTH_TX_BCK01 = 0x8A,
    E_BACH_NF_SYNTH_TX_BCK02 = 0x8C,
    E_BACH_NF_SYNTH_MCK00    = 0x8E,
    E_BACH_NF_SYNTH_MCK01    = 0x90,
    E_BACH_NF_SYNTH_MCK02    = 0x92,
    E_BACH_DMA_SRC_CFG00     = 0x96,
    E_BACH_DMA_SRC_CFG01     = 0x98,
    E_BACH_DMA_SRC_CFG02     = 0x9A,
    E_BACH_DMA_SRC_CFG03     = 0x9C,
    E_BACH_VREC_HPF_CFG00    = 0x9E
};

enum
{
    // ----------------------------------------------
    // BANK3 SYS
    // ----------------------------------------------
    E_BACH_VREC_CTRL00 = 0x00,
    E_BACH_VREC_CTRL01 = 0x02,
    E_BACH_VREC_CTRL02 = 0x04,
    E_BACH_VREC_CTRL03 = 0x06,
    E_BACH_VREC_CTRL04 = 0x08,
    E_BACH_VREC_CTRL05 = 0x0A,
    E_BACH_VREC_CTRL06 = 0x0C,
    E_BACH_VREC_CTRL07 = 0x0E,
    E_BACH_VREC_CTRL08 = 0x10,
    E_BACH_VREC_CTRL09 = 0x12,
    E_BACH_CIC_CTRL01  = 0x42,
    E_BACH_CIC_CTRL02  = 0x44
};

enum
{
    // ----------------------------------------------
    // BANK4 DMA
    // ----------------------------------------------
    E_BACH_DMA3_CTRL_0    = 0x00,
    E_BACH_DMA3_CTRL_1    = 0x02,
    E_BACH_DMA3_CTRL_2    = 0x04,
    E_BACH_DMA3_CTRL_3    = 0x06,
    E_BACH_DMA3_CTRL_4    = 0x08,
    E_BACH_DMA3_CTRL_5    = 0x0A,
    E_BACH_DMA3_CTRL_6    = 0x0C,
    E_BACH_DMA3_CTRL_7    = 0x0E,
    E_BACH_DMA3_CTRL_8    = 0x10,
    E_BACH_DMA3_CTRL_9    = 0x12,
    E_BACH_DMA3_CTRL_10   = 0x14,
    E_BACH_DMA3_CTRL_11   = 0x16,
    E_BACH_DMA3_CTRL_12   = 0x18,
    E_BACH_DMA3_CTRL_13   = 0x1A,
    E_BACH_DMA3_CTRL_14   = 0x1C,
    E_BACH_DMA3_CTRL_15   = 0x1E,
    E_BACH_DMA4_CTRL_0    = 0x20, // DMA4
    E_BACH_DMA4_CTRL_1    = 0x22,
    E_BACH_DMA4_CTRL_2    = 0x24,
    E_BACH_DMA4_CTRL_3    = 0x26,
    E_BACH_DMA4_CTRL_4    = 0x28,
    E_BACH_DMA4_CTRL_5    = 0x2A,
    E_BACH_DMA4_CTRL_6    = 0x2C,
    E_BACH_DMA4_CTRL_7    = 0x2E,
    E_BACH_DMA4_CTRL_8    = 0x30,
    E_BACH_DMA4_CTRL_9    = 0x32,
    E_BACH_DMA4_CTRL_00   = 0x34,
    E_BACH_DMA4_CTRL_01   = 0x36,
    E_BACH_DMA4_CTRL_02   = 0x38,
    E_BACH_DMA4_CTRL_03   = 0x3A,
    E_BACH_DMA4_CTRL_04   = 0x3C,
    E_BACH_DMA4_CTRL_05   = 0x3E,
    E_BACH_DMA5_CTRL_0    = 0x40, // DMA5
    E_BACH_DMA5_CTRL_1    = 0x42,
    E_BACH_DMA5_CTRL_2    = 0x44,
    E_BACH_DMA5_CTRL_3    = 0x46,
    E_BACH_DMA5_CTRL_4    = 0x48,
    E_BACH_DMA5_CTRL_5    = 0x4A,
    E_BACH_DMA5_CTRL_6    = 0x4C,
    E_BACH_DMA5_CTRL_7    = 0x4E,
    E_BACH_DMA5_CTRL_8    = 0x50,
    E_BACH_DMA5_CTRL_9    = 0x52,
    E_BACH_DMA5_CTRL_10   = 0x54,
    E_BACH_DMA5_CTRL_11   = 0x56,
    E_BACH_DMA5_CTRL_12   = 0x58,
    E_BACH_DMA5_CTRL_13   = 0x5A,
    E_BACH_DMA5_CTRL_14   = 0x5C,
    E_BACH_DMA5_CTRL_15   = 0x5E,
    E_BACH_DMA3_CTRL_16   = 0xE0, // DMA3
    E_BACH_DMA3_CTRL_17   = 0xE2,
    E_BACH_DMA3_TEST_CTRL = 0xE4,
    E_BACH_DMA4_CTRL_06   = 0xE8, // DMA4
    E_BACH_DMA4_CTRL_07   = 0xEA,
    E_BACH_DMA4_TEST_CTRL = 0xEC,
    E_BACH_DMA5_CTRL_16   = 0xF0, // DMA5
    E_BACH_DMA5_CTRL_17   = 0xF2,
    E_BACH_DMA5_TEST_CTRL = 0xF4,
};

enum
{
    // ----------------------------------------------
    // BANK4 SYS
    // ----------------------------------------------
    E_BACH_I2S_TDM1_CFG00     = 0x60,
    E_BACH_I2S_TDM1_CFG01     = 0x62,
    E_BACH_I2S_TDM1_CFG02     = 0x64,
    E_BACH_I2S_TDM1_CFG03     = 0x66,
    E_BACH_I2S_TDM1_CFG05     = 0x6A,
    E_BACH_I2S_TDM1_CFG06     = 0x6C,
    E_BACH_I2S_TDM1_DPGA_CFG1 = 0x70,
    E_BACH_I2S_TDM1_DPGA_CFG2 = 0x72,
    E_BACH_I2S1_CFG00         = 0x78,
    E_BACH_DMA3_SRC_CFG       = 0x7A,
    E_BACH_NF_SYNTH_RX1_BCK00 = 0x82,
    E_BACH_NF_SYNTH_RX1_BCK01 = 0x84,
    E_BACH_NF_SYNTH_RX1_BCK02 = 0x86,
    E_BACH_NF_SYNTH_TX1_BCK00 = 0x88,
    E_BACH_NF_SYNTH_TX1_BCK01 = 0x8A,
    E_BACH_NF_SYNTH_TX1_BCK02 = 0x8C,
    E_BACH_NF_SYNTH_MCK1_00   = 0x8E,
    E_BACH_NF_SYNTH_MCK1_01   = 0x90,
    E_BACH_NF_SYNTH_MCK1_02   = 0x92,
    E_BACH_DMA3_SRC_CFG00     = 0x94,
    E_BACH_DMA3_SRC_CFG01     = 0x96,
    E_BACH_DMA4_SRC_CFG00     = 0x98,
    E_BACH_DMA4_SRC_CFG01     = 0x9A,
    E_BACH_DMA5_SRC_CFG00     = 0x9C,
    E_BACH_DMA5_SRC_CFG01     = 0x9E,
    E_BACH_I2S_TDM2_CFG00     = 0xA0,
    E_BACH_I2S_TDM2_CFG01     = 0xA2,
    E_BACH_I2S_TDM2_CFG02     = 0xA4,
    E_BACH_I2S_TDM3_CFG00     = 0xC0,
    E_BACH_I2S_TDM3_CFG01     = 0xC2,
    E_BACH_I2S_TDM3_CFG02     = 0xC4
};

enum
{
    // ----------------------------------------------
    // BANK5 SYS ANALOG 0x1034 ausdm
    // ----------------------------------------------
    E_BACH_ANALOG_CTRL00 = 0x00,
    E_BACH_ANALOG_CTRL01 = 0x02,
    E_BACH_ANALOG_CTRL02 = 0x04,
    E_BACH_ANALOG_CTRL03 = 0x06,
    E_BACH_ANALOG_CTRL04 = 0x08,
    E_BACH_ANALOG_CTRL05 = 0x0A,
    E_BACH_ANALOG_CTRL06 = 0x0C,
    E_BACH_ANALOG_CTRL07 = 0x0E,
    E_BACH_ANALOG_CTRL08 = 0x10,
    E_BACH_ANALOG_CTRL09 = 0x12,
    E_BACH_ANALOG_CTRL0A = 0x14,
    E_BACH_ANALOG_CTRL0B = 0x16,
    E_BACH_ANALOG_CTRL0C = 0x18,
    E_BACH_ANALOG_CTRL0D = 0x1A,
    E_BACH_ANALOG_STAT03 = 0x24,
    E_BACH_ANALOG_CTRL30 = 0x60,
};

/****************************************************************************/
/*        AUDIO ChipTop                                     */
/****************************************************************************/
/**
 * @brief Register 101E10h,
 */
//#define REG_I2S_MODE                  (1<<10)
//#define REG_I2S_MODE                  (1<<12) // for SPI PAD
#define REG_I2S_MODE (2 << 12) // for LCD PAD

/****************************************************************************/
/*        AUDIO DIGITAL registers bank1                                     */
/****************************************************************************/
/**
 * @brief Register 00h,
 */
#define REG_BP_ADC_HPF2 (1 << 11)
#define REG_BP_ADC_HPF1 (1 << 10)

/**
 * @brief Register 02h
 */
#define REG_CIC_1_SEL_POS  14
#define REG_CIC_1_SEL_MSK  (0x3 << REG_CIC_1_SEL_POS)
#define REG_CIC_3_SEL_POS  10
#define REG_CIC_3_SEL_MSK  (0x3 << REG_CIC_3_SEL_POS)
#define REG_WRITER_SEL_POS 8
#define REG_WRITER_SEL_MSK (0x3 << REG_WRITER_SEL_POS)
#define REG_SRC1_SEL_POS   4
#define REG_SRC1_SEL_MSK   (0xF << REG_SRC1_SEL_POS)
#define REG_SRC2_SEL_POS   0
#define REG_SRC2_SEL_MSK   (0xF << REG_SRC2_SEL_POS)

/**
 * @brief Register 06h,
 */
#define REG_MMC2_SRC_SEL (1 << 4)
#define REG_MMC1_SRC_SEL (1 << 5)

/**
 * @brief Register 08h,
 */
#define REG_ADC_HPF_N_POS 12
#define REG_ADC_HPF_N_MSK (0xF << REG_ADC_HPF_N_POS)

/**
 * @brief Register 0Eh,
 */
#define REG_DAC_DIN_L_SEL_POS    14
#define REG_DAC_DIN_L_SEL_MSK    (0x3 << REG_DAC_DIN_L_SEL_POS)
#define REG_DAC_DIN_R_SEL_POS    12
#define REG_DAC_DIN_R_SEL_MSK    (0x3 << REG_DAC_DIN_R_SEL_POS)
#define REG_HD_DAC_DIN_L_SEL_POS 10
#define REG_HD_DAC_DIN_L_SEL_MSK (0x3 << REG_HD_DAC_DIN_L_SEL_POS)
#define REG_HD_DAC_DIN_R_SEL_POS 8
#define REG_HD_DAC_DIN_R_SEL_MSK (0x3 << REG_HD_DAC_DIN_R_SEL_POS)
#define REG_SRC_MIXER1_L_POS     6
#define REG_SRC_MIXER1_L_MSK     (0x3 << REG_SRC_MIXER1_L_POS)
#define REG_SRC_MIXER1_R_POS     4
#define REG_SRC_MIXER1_R_MSK     (0x3 << REG_SRC_MIXER1_R_POS)
#define REG_SRC_MIXER2_L_POS     2
#define REG_SRC_MIXER2_L_MSK     (0x3 << REG_SRC_MIXER2_L_POS)
#define REG_SRC_MIXER2_R_POS     0
#define REG_SRC_MIXER2_R_MSK     (0x3 << REG_SRC_MIXER2_R_POS)

/**
 * @brief Register 24h,26h
 */
#define RESETB_RX         (1 << 15)
#define REG_I2S_FMT       (1 << 14)
#define REG_I2S_FIFO_CLR  (1 << 13)
#define REG_I2S_BCK_INV   (1 << 11)
#define I2S_MUX_SEL_R_POS 6
#define I2S_MUX_SEL_R_MSK (0x3 << I2S_MUX_SEL_R_POS)
#define I2S_MUX_SEL_L_POS 4
#define I2S_MUX_SEL_L_MSK (0x3 << I2S_MUX_SEL_L_POS)

/**
 * @brief Register 3Ch
 */
#define CODEC_DAAD_LOOP  (1 << 7)
#define CODEC_DAAD2_LOOP (1 << 11)

/**
 * @brief Register 40h,48h,50h,60h
 */
#define REG_OFFSET_R_POS 11
#define REG_OFFSET_R_MSK (0x1F << REG_OFFSET_R_POS)
#define REG_OFFSET_L_POS 6
#define REG_OFFSET_L_MSK (0x1F << REG_OFFSET_L_POS)
#define STEP_POS         3
#define STEP_MSK         (0x7 << STEP_POS)
#define MUTE_2_ZERO      (1 << 2)
#define FADING_EN        (1 << 1)
#define DPGA_EN          (1 << 0)

/**
 * @brief Register 42h,4Ah,52h,62h
 */
#define REG_GAIN_R_POS 8
#define REG_GAIN_R_MSK (0xFF << REG_GAIN_R_POS)
#define REG_GAIN_L_POS 0
#define REG_GAIN_L_MSK (0xFF << REG_GAIN_L_POS)

/**
 * @brief Register 80h
 */
#define REG_WR_UNDERRUN_INT_EN     (1 << 15)
#define REG_WR_OVERRUN_INT_EN      (1 << 14)
#define REG_RD_UNDERRUN_INT_EN     (1 << 13)
#define REG_RD_OVERRUN_INT_EN      (1 << 12)
#define REG_WR_FULL_INT_EN         (1 << 11)
#define REG_RD_EMPTY_INT_EN        (1 << 10)
#define REG_WR_FULL_FLAG_CLR       (1 << 9)
#define REG_RD_EMPTY_FLAG_CLR      (1 << 8)
#define REG_SEL_TES_BUS            (1 << 6)
#define REG_RD_LR_SWAP_EN          (1 << 5)
#define REG_PRIORITY_KEEP_HIGH     (1 << 4)
#define REG_RD_BYTE_SWAP_EN        (1 << 3)
#define REG_RD_LEVEL_CNT_LIVE_MASK (1 << 2)
#define REG_ENABLE                 (1 << 1)
#define REG_SW_RST_DMA             (1 << 0)

/**
 * @brief Register 82h
 */
#define REG_RD_ENABLE         (1 << 15)
#define REG_RD_INIT           (1 << 14)
#define REG_RD_TRIG           (1 << 13)
#define REG_RD_LEVEL_CNT_MASK (1 << 12)

#define REG_RD_BASE_ADDR_LO_POS 0
#define REG_RD_BASE_ADDR_LO_MSK (0xFFF << REG_RD_BASE_ADDR_LO_POS)

/**
 * @brief Register 84h
 */
#define REG_RD_BASE_ADDR_HI_OFFSET 12
#define REG_RD_BASE_ADDR_HI_POS    0
#define REG_RD_BASE_ADDR_HI_MSK    (0xFFFF << REG_RD_BASE_ADDR_HI_POS)

/**
 * @brief Register 86h
 */
#define REG_RD_BUFF_SIZE_POS 0
#define REG_RD_BUFF_SIZE_MSK (0xFFFF << REG_RD_BUFF_SIZE_POS)

/**
 * @brief Register 88h
 */
#define REG_RD_SIZE_POS 0
#define REG_RD_SIZE_MSK (0xFFFF << REG_RD_SIZE_POS)

/**
 * @brief Register 8Ah
 */
#define REG_RD_OVERRUN_TH_POS 0
#define REG_RD_OVERRUN_TH_MSK (0xFFFF << REG_RD_OVERRUN_TH_POS)

/**
 * @brief Register 8Ch
 */
#define REG_RD_UNDERRUN_TH_POS 0
#define REG_RD_UNDERRUN_TH_MSK (0xFFFF << REG_RD_UNDERRUN_TH_POS)

/**
 * @brief Register 8Eh
 */
#define REG_RD_LEVEL_CNT_POS 0
#define REG_RD_LEVEL_CNT_MSK (0xFFFF << REG_RD_LEVEL_CNT_POS)

/**
 * @brief Register 90h
 */
#define REG_RD_LOCALBUF_EMPTY (1 << 7)
#define REG_WR_LOCALBUF_FULL  (1 << 6)
#define REG_WR_FULL_FLAG      (1 << 5)
#define REG_RD_EMPTY_FLAG     (1 << 4)
#define REG_RD_OVERRUN_FLAG   (1 << 3)
#define REG_RD_UNDERRUN_FLAG  (1 << 2)
#define REG_WR_OVERRUN_FLAG   (1 << 1)
#define REG_WR_UNDERRUN_FLAG  (1 << 0)

/**
 * @brief Register 92h
 */
#define REG_WR_ENABLE         (1 << 15)
#define REG_WR_INIT           (1 << 14)
#define REG_WR_TRIG           (1 << 13)
#define REG_WR_LEVEL_CNT_MASK (1 << 12)

#define REG_WR_BASE_ADDR_LO_POS 0
#define REG_WR_BASE_ADDR_LO_MSK (0xFFF << REG_WR_BASE_ADDR_LO_POS)

/**
 * @brief Register 94h
 */
#define REG_WR_BASE_ADDR_HI_OFFSET 12
#define REG_WR_BASE_ADDR_HI_POS    0
#define REG_WR_BASE_ADDR_HI_MSK    (0xFFFF << REG_WR_BASE_ADDR_HI_POS)

/**
 * @brief Register 96h
 */
#define REG_WR_BUFF_SIZE_POS 0
#define REG_WR_BUFF_SIZE_MSK (0xFFFF << REG_WR_BUFF_SIZE_POS)

/**
 * @brief Register 98h
 */
#define REG_WR_SIZE_POS 0
#define REG_WR_SIZE_MSK (0xFFFF << REG_WR_SIZE_POS)

/**
 * @brief Register 9Ah
 */
#define REG_WR_OVERRUN_TH_POS 0
#define REG_WR_OVERRUN_TH_MSK (0xFFFF << REG_WR_OVERRUN_TH_POS)

/**
 * @brief Register 9Ch
 */
#define REG_WR_UNDERRUN_TH_POS 0
#define REG_WR_UNDERRUN_TH_MSK (0xFFFF << REG_WR_UNDERRUN_TH_POS)

/**
 * @brief Register 9Eh
 */
#define REG_WR_LEVEL_CNT_POS 0
#define REG_WR_LEVEL_CNT_MSK (0xFFFF << REG_RD_LEVEL_CNT_POS)

/**
 * @brief Register C8h
 */
#define REG_BP_ADC2_HPF2 (1 << 11)
#define REG_BP_ADC2_HPF1 (1 << 10)
#define REG_EN_INT_1_AU2 (1 << 8)
#define REG_EN_INT_2_AU2 (1 << 7)
#define REG_EN_DEC_1_AU2 (1 << 6)
#define REG_EN_DEC_2_AU2 (1 << 5)
#define REG_EN_DEC_3_AU2 (1 << 4)
#define REG_EN_FIR_1_AU2 (1 << 3)
#define REG_EN_ASRC_AU2  (1 << 2)
#define REG_EN_CODEC_AU2 (1 << 0)

/**
 * @brief Register CAh
 */
#define REG_CIC_1_SEL_AU2_POS 14
#define REG_CIC_1_SEL_AU2_MSK (0x3 << REG_CIC_1_SEL_AU2_POS)
#define REG_CIC_2_SEL_AU2_POS 12
#define REG_CIC_2_SEL_AU2_MSK (0x3 << REG_CIC_2_SEL_AU2_POS)
#define REG_CIC_3_SEL_AU2_POS 10
#define REG_CIC_3_SEL_AU2_MSK (0x3 << REG_CIC_3_SEL_AU2_POS)
#define REG_CODEC_SEL_AU2_POS 8
#define REG_CODEC_SEL_AU2_MSK (0x3 << REG_CODEC_SEL_AU2_POS)

/**
 * @brief Register CCh
 */
#define REG_ADC2_HPF_N_POS   12
#define REG_ADC2_HPF_N_MSK   (0xF << REG_ADC2_HPF_N_POS)
#define REG_ADC2_SAT_SEL     (1 << 11)
#define REG_SDM_DINL_SEL_POS 8
#define REG_SDM_DINL_SEL_MSK (0x7 << REG_SDM_DINL_SEL_POS)
#define REG_SDM_DINL_INV     (1 << 7)
#define REG_SDM_DINR_SEL_POS 4
#define REG_SDM_DINR_SEL_MSK (0x7 << REG_SDM_DINR_SEL_POS)
#define REG_SDM_DINR_INV     (1 << 3)

/**
 * @brief Register EAh
 */
#define REG_SINE_GEN_EN       (1 << 15)
#define REG_SINE_GEN_L        (1 << 14)
#define REG_SINE_GEN_R        (1 << 13)
#define REG_SINE_GEN_RD_WR    (1 << 12)
#define REG_SINE_GEN_GAIN_POS 4
#define REG_SINE_GEN_GAIN_MSK (0xF << REG_SINE_GEN_GAIN_POS)
#define REG_SINE_GEN_FREQ_POS 0
#define REG_SINE_GEN_FREQ_MSK (0xF << REG_SINE_GEN_FREQ_POS)

/**
 * @brief Register EEh
 */
#define REG_DMA1_RD_MONO      (1 << 15)
#define REG_DMA1_WR_MONO      (1 << 14)
#define REG_DMA1_RD_MONO_COPY (1 << 13)
#define REG_DMA2_RD_MONO      (1 << 11)
#define REG_DMA2_WR_MONO      (1 << 10)
#define REG_DMA2_RD_MONO_COPY (1 << 9)
#define REG_DMA2_MIU_SEL1     (1 << 7)
#define REG_DMA2_MIU_SEL0     (1 << 6)
#define REG_DMA1_MIU_SEL1     (1 << 5)
#define REG_DMA1_MIU_SEL0     (1 << 4)

/**
 * @brief Register F0h
 */
#define REG_AU2HDMI_EN          (1 << 15)
#define REG_AU2HDMI_MUX_SEL_POS 12
#define REG_AU2HDMI_MUX_SEL_MSK (0x3 << REG_AU2HDMI_MUX_SEL_POS)

/**
 * @brief Register F2h
 */
#define REG_AU2HDMI_1K_POS 0
#define REG_AU2HDMI_1K_MSK (0xFFFF << REG_AU2HDMI_1K_POS)

/**
 * @brief Register F3h
 */
#define REG_WR_BASE_ADDR_ADDITION_OFFSET (REG_WR_BASE_ADDR_HI_OFFSET + 16)
#define REG_RD_BASE_ADDR_ADDITION_OFFSET (REG_WR_BASE_ADDR_HI_OFFSET + 16)
#define REG_WR2_BASE_ADDR_ADDITION_POS   12
#define REG_WR2_BASE_ADDR_ADDITION_MSK   (0xF << REG_WR2_BASE_ADDR_ADDITION_POS)
#define REG_RD2_BASE_ADDR_ADDITION_POS   8
#define REG_RD2_BASE_ADDR_ADDITION_MSK   (0xF << REG_RD2_BASE_ADDR_ADDITION_POS)
#define REG_WR1_BASE_ADDR_ADDITION_POS   4
#define REG_WR1_BASE_ADDR_ADDITION_MSK   (0xF << REG_WR1_BASE_ADDR_ADDITION_POS)
#define REG_RD1_BASE_ADDR_ADDITION_POS   0
#define REG_RD1_BASE_ADDR_ADDITION_MSK   (0xF << REG_RD1_BASE_ADDR_ADDITION_POS)

/****************************************************************************/
/*        AUDIO DIGITAL registers bank2                                     */
/****************************************************************************/
/**
 * @brief Register 0Eh
 */
#define REG_DMA_INT_EN (1 << 1)

/**
 * @brief Register 1Ch
 */
#define MUX_ASRC_ADC_SEL (1 << 1)

/**
 * @brief Register 2Ch
 */
#define REG_CODEC_SEL_POS    0
#define REG_CODEC_SEL_MSK    (0x3 << REG_CODEC_SEL_POS)
#define MUX_CODEC_TX_SEL_POS 2
#define MUX_CODEC_TX_SEL_MSK (0x3 << MUX_CODEC_TX_SEL_POS)

/**
 * @brief Register 2Eh
 */
#define REG_DMA2_MCH_EN    (1 << 15)
#define REG_DMA2_MCH_DBG   (1 << 14)
#define REG_DMA2_MCH_RESET (1 << 13)
#define REG_DMA2_MCH_BIT   (1 << 4)
#define REG_DMA2_MCH_POS   0
#define REG_DMA2_MCH_MSK   (0x3 << REG_DMA2_MCH_POS)

/**
 * @brief Register 3Ah
 */
#define REG_DIGMIC_EN       (1 << 15)
#define REG_DIGMIC_CLK_INV  (1 << 14)
#define REG_DIGMIC_CLK_MODE (1 << 13)
#define REG_DIGMIC_SEL_POS  0
#define REG_DIGMIC_SEL_MSK  (0x3 << REG_DIGMIC_SEL_POS)

/**
 * @brief Register 3Ch
 */
#define REG_CIC_SEL (1 << 15)

/**
 * @brief Register 60h
 */
#define BACH_I2S_RX_SHARE_MODE (1 << 15)
#define BACH_I2S_RX_MS_MODE    (1 << 14)
#define BACH_I2S_RX_TDM_MODE   (1 << 13)
#define BACH_I2S_RX_ENC_WIDTH  (1 << 9)
#define BACH_I2S_RX_CHLEN      (1 << 8)
#define BACH_I2S_RX_WS_FMT     (1 << 7)
#define BACH_I2S_RX_FMT        (1 << 6)
#define BACH_I2S_RX_WS_INV     (1 << 5)
#define BACH_I2S_RX_BCK_DG_EN  (1 << 3)
#define BACH_I2S_RX_BCK_DG_POS 0
#define BACH_I2S_RX_BCK_DG_MSK (0x7 << BACH_I2S_RX_BCK_DG_POS)

/**
 * @brief Register 62h
 */
#define BACH_I2S_RX_WS_WDTH_POS 5
#define BACH_I2S_RX_WS_WDTH_MSK (0x1F << BACH_I2S_RX_WS_WDTH_POS)

/**
 * @brief Register 64h
 */
#define BACH_I2S_TDM_RX_FIFO_CLR        (1 << 15)
#define BACH_I2S_TDM_TX_DPGA_STATUS_CLR (1 << 14)
#define BACH_I2S_RX_CH_SWAP_POS         0
#define BACH_I2S_RX_CH_SWAP_MSK         (0x7 << BACH_I2S_RX_CH_SWAP_POS)

/**
 * @brief Register 6Ah
 */
#define BACH_I2S_TX_MS_MODE    (1 << 14)
#define BACH_I2S_TX_TDM_MODE   (1 << 13)
#define BACH_I2S_TX_4WIRE_MODE (1 << 12)
#define BACH_FIFO_STATUS_CLR   (1 << 11)
#define BACH_I2S_TX_ENC_WIDTH  (1 << 9)
#define BACH_I2S_TX_CHLEN      (1 << 8)
#define BACH_I2S_TX_WS_FMT     (1 << 7)
#define BACH_I2S_TX_FMT        (1 << 6)
#define BACH_I2S_TX_WS_INV     (1 << 5)
#define BACH_I2S_TX_BCK_DG_EN  (1 << 3)
#define BACH_I2S_TX_BCK_DG_POS 0
#define BACH_I2S_TX_BCK_DG_MSK (0x7 << BACH_I2S_TX_BCK_DG_POS)

/**
 * @brief Register 6Ch
 */
#define BACH_I2S_TX_CH_SWAP_POS  13
#define BACH_I2S_TX_CH_SWAP_MSK  (0x7 << BACH_I2S_TX_CH_SWAP_POS)
#define BACH_I2S_TX_WS_WDTH_POS  8
#define BACH_I2S_TX_WS_WDTH_MSK  (0x1F << BACH_I2S_TX_WS_WDTH_POS)
#define BACH_I2S_TX_ACT_SLOT_POS 0
#define BACH_I2S_TX_ACT_SLOT_MSK (0xFF << BACH_I2S_TX_ACT_SLOT_POS)

/**
 * @brief Register 78h
 */
#define BACH_RX_DIV_SEL_POS     14
#define BACH_RX_DIV_SEL_MSK     (0x3 << BACH_RX_DIV_SEL_POS)
#define BACH_INV_CLK_RX_BCK_POS 13
#define BACH_INV_CLK_RX_BCK_MSK (1 << BACH_INV_CLK_RX_BCK_POS)
#define BACH_TX_DIV_SEL_POS     10
#define BACH_TX_DIV_SEL_MSK     (0x3 << BACH_TX_DIV_SEL_POS)
#define BACH_INV_CLK_TX_BCK_POS 9
#define BACH_INV_CLK_TX_BCK_MSK (1 << BACH_INV_CLK_TX_BCK_POS)
#define BACH_MCK_SEL_POS        6
#define BACH_MCK_SEL_MSK        (0x3 << BACH_MCK_SEL_POS)
#define BACH_I2S_TDM_TX_MUX     (1 << 3)
#define BACH_DMA_RD2_PATH_POS   1
#define BACH_DMA_RD2_PATH_MSK   (0x3 << BACH_DMA_RD2_PATH_POS)
#define BACH_MUX2_SEL           (1 << 0)

/**
 * @brief Register 82h,88h,8Eh
 */
#define BACH_SEL_CLK_NF_SYNTH_REF_POS 6
#define BACH_SEL_CLK_NF_SYNTH_REF_MSK (0x3 << BACH_SEL_CLK_NF_SYNTH_REF_POS)
#define BACH_INV_CLK_NF_SYNTH_REF     (1 << 5)
#define BACH_ENABLE_CLK_NF_SYNTH_REF  (1 << 4)
#define BACH_CODEC_BCK_EN_SYNTH_TRIG  (1 << 2)
#define BACH_CODEC_BCK_EN_TIME_GEN    (1 << 0)

/**
 * @brief Register 84h,8Ah,90h
 */
#define BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_LO (0xFFFF)
#define BACH_MCK_NF_VALUE_LO_MSK            (0xFFFF)

/**
 * @brief Register 86h,8Ch,92h
 */
#define BACH_CODEC_BCK_EN_SYNTH_NF_VALUE_HI (0xFFFF)
#define BACH_MCK_NF_VALUE_HI_OFFSET         16
#define BACH_MCK_NF_VALUE_HI_MSK            (0x00FF)
#define BACH_MCK_EXPAND_POS                 8
#define BACH_MCK_EXPAND_MSK                 (0xFF << BACH_MCK_EXPAND_POS)

/**
 * @brief Register 96h,98h
 */
#define BACH_DMA_WR_CH23_SEL_POS  12
#define BACH_DMA_WR_CH23_SEL_MSK  (0xF << BACH_DMA_WR_CH23_SEL_POS)
#define BACH_DMA_WR_CH01_SEL_POS  8
#define BACH_DMA_WR_CH01_SEL_MSK  (0xF << BACH_DMA_WR_CH01_SEL_POS)
#define BACH_DMA_WR_VALID_SEL_POS 6
#define BACH_DMA_WR_VALID_SEL_MSK (0x3 << BACH_DMA_WR_VALID_SEL_POS)
#define BACH_DMA_MCH_DEBUG_MODE   (1 << 5)
#define BACH_DMA_WR_BIT_MODE      (1 << 4)
#define BACH_DMA_WR_CH_MODE_POS   2
#define BACH_DMA_WR_CH_MODE_MSK   (0x3 << BACH_DMA_WR_CH_MODE_POS)
#define BACH_DMA_WR_NEW_MODE      (1 << 1)
#define BACH_MCH_32B_EN           (1 << 0)

/**
 * @brief Register 9Ah
 */
#define BACH_DMA_RD1_SEL         (1 << 3)
#define BACH_DMA_RD2_SEL         (1 << 2)
#define BACH_DMA_RD1_TDM_SEL_POS 6
#define BACH_DMA_RD1_TDM_SEL_MSK (0x3 << BACH_DMA_RD1_TDM_SEL_POS)
#define BACH_DMA_RD2_TDM_SEL_POS 4
#define BACH_DMA_RD2_TDM_SEL_MSK (0x3 << BACH_DMA_RD2_TDM_SEL_POS)
#define BACH_I2S_TDM_RX3_POS     12
#define BACH_I2S_TDM_RX3_MSK     (0x3 << BACH_I2S_TDM_RX3_POS)
#define BACH_I2S_TDM_RX2_POS     10
#define BACH_I2S_TDM_RX2_MSK     (0x3 << BACH_I2S_TDM_RX2_POS)
#define BACH_I2S_TDM_RX1_POS     8
#define BACH_I2S_TDM_RX1_MSK     (0x3 << BACH_I2S_TDM_RX1_POS)
#define BACH_I2S_TDM_RX_POS      0
#define BACH_I2S_TDM_RX_MSK      (0x3 << BACH_I2S_TDM_RX_POS)

/**
 * @brief Register 9Ch
 */
#define BACH_DMA_WR2_CH67_SEL_POS 12
#define BACH_DMA_WR2_CH67_SEL_MSK (0xF << BACH_DMA_WR2_CH67_SEL_POS)
#define BACH_DMA_WR2_CH45_SEL_POS 8
#define BACH_DMA_WR2_CH45_SEL_MSK (0xF << BACH_DMA_WR2_CH45_SEL_POS)
#define BACH_DMA_WR1_CH67_SEL_POS 4
#define BACH_DMA_WR1_CH67_SEL_MSK (0xF << BACH_DMA_WR1_CH67_SEL_POS)
#define BACH_DMA_WR1_CH45_SEL_POS 0
#define BACH_DMA_WR1_CH45_SEL_MSK (0xF << BACH_DMA_WR1_CH45_SEL_POS)

/**
 * @brief Register 9Eh
 */
#define REG_BP_VERC_HPF2      (1 << 5)
#define REG_BP_VERC_HPF1      (1 << 4)
#define REG_BP_VERC_HPF_N_POS (0)
#define REG_BP_VERC_HPF_N_MSK (0xF << REG_BP_VERC_HPF_N_POS)

/****************************************************************************/
/*        AUDIO DIGITAL registers bank3                                     */
/****************************************************************************/
/**
 * @brief Register 00h
 */
#define REG_RESET_ALL_DMIC_PATH (1 << 0)

/**
 * @brief Register 02h
 */
#define REG_CHN_MODE_DMIC_POS 4
#define REG_CHN_MODE_DMIC_MSK (0x3 << REG_CHN_MODE_DMIC_POS)

/**
 * @brief Register 04h
 */
#define REG_SEL_RECORD_SRC (1 << 1)

/**
 * @brief Register 06h
 */
#define REG_SIN_FREQ_SEL_POS    12
#define REG_SIN_FREQ_SEL_MSK    (0xF << REG_SIN_FREQ_SEL_POS)
#define REG_SIN_GAIN_POS        8
#define REG_SIN_GAIN_MSK        (0xF << REG_SIN_GAIN_POS)
#define REG_SIN_PATH_SEL_POS    4
#define REG_SIN_PATH_SEL_MSK    (0x3 << REG_SIN_PATH_SEL_POS)
#define REG_SIN_PATH_SEL_LR_POS 1
#define REG_SIN_PATH_SEL_LR_MSK (0x3 << REG_SIN_PATH_SEL_LR_POS)
#define REG_DMIC_SIN_EN         (1 << 0)

/**
 * @brief Register 08h
 */
#define REG_FS_SEL_POS 0
#define REG_FS_SEL_MSK (0x1F << REG_FS_SEL_POS)

/**
 * @brief Register 0Ah
 */
#define REG_CLK_CNT_BY       (1 << 15)
#define REG_SPL_RATE_REC_POS 8
#define REG_SPL_RATE_REC_MSK (0x3 << REG_SPL_RATE_REC_POS)
#define REG_DEC_RATE_POS     4
#define REG_DEC_RATE_MSK     (0xF << REG_DEC_RATE_POS)
#define REG_OUT_CLK_DMIC_POS 0
#define REG_OUT_CLK_DMIC_MSk (0xF << REG_OUT_CLK_DMIC_POS)

/**
 * @brief Register 42h, 44h
 */
#define REG_CIC_GAIN_R_POS 12
#define REG_CIC_GAIN_R_MSK (0x7 << REG_CIC_GAIN_R_POS)
#define REG_CIC_GAIN_L_POS 8
#define REG_CIC_GAIN_L_MSK (0x7 << REG_CIC_GAIN_L_POS)
#define REG_DEC_FILTER     (1 << 0)

/****************************************************************************/
/*        AUDIO DIGITAL registers bank4                                     */
/****************************************************************************/
/**
 * @brief Register 78h
 */
#define BACH_INV_CLK_RX1_BCK_POS       13
#define BACH_INV_CLK_RX1_BCK_MSK       (1 << BACH_INV_CLK_RX1_BCK_POS)
#define BACH_INV_CLK_TX1_BCK_POS       9
#define BACH_INV_CLK_TX1_BCK_MSK       (1 << BACH_INV_CLK_TX1_BCK_POS)
#define BACH_I2S_TDM2_TX_MUX           (1 << 3)
#define REG_I2S_1_TDM_TX1_DATA_SEL_POS 1
#define REG_I2S_1_TDM_TX1_DATA_SEL_MSK (0x3 << REG_I2S_1_TDM_TX1_DATA_SEL_POS)

/**
 * @brief Register 7A
 */

#define BACH_RDMA3_VALID_SEL_POS   4
#define BACH_RDMA3_VALID_SEL_MSK   (0x3 << BACH_RDMA3_VALID_SEL_POS)
#define BACH_RDMA3_SAMPLE_RATE_POS 0
#define BACH_RDMA3_SAMPLE_RATE_MSK (0xf << BACH_RDMA3_SAMPLE_RATE_POS)

/**
 * @brief Register 94h,96h,98h,9ch
 */
#define BACH_DMA_WR3_CH67_SEL_POS   12
#define BACH_DMA_WR3_CH67_SEL_MSK   (0xF << BACH_DMA_WR3_CH67_SEL_POS)
#define BACH_DMA_WR3_CH45_SEL_POS   8
#define BACH_DMA_WR3_CH45_SEL_MSK   (0xF << BACH_DMA_WR3_CH45_SEL_POS)
#define BACH_DMA_WR3_CH23_SEL_POS   4
#define BACH_DMA_WR3_CH23_SEL_MSK   (0xF << BACH_DMA_WR3_CH23_SEL_POS)
#define BACH_DMA_WR3_CH01_SEL_POS   0
#define BACH_DMA_WR3_CH01_SEL_MSK   (0xF << BACH_DMA_WR3_CH01_SEL_POS)
#define BACH_DMA_WR3_VALID_SEL_POS  6
#define BACH_DMA_WR3_VALID_SEL_MSK  (0x3 << BACH_DMA_WR3_VALID_SEL_POS)
#define BACH_DMA_WR3_MCH_DEBUG_MODE (1 << 5)
#define BACH_DMA_WR3_BIT_MODE       (1 << 4)
#define BACH_DMA_WR3_CH_MODE_POS    2
#define BACH_DMA_WR3_CH_MODE_MSK    (0x3 << BACH_DMA_WR3_CH_MODE_POS)
#define BACH_MCH_WR3_32B_EN         (1 << 0)

/**
 * @brief Register E2h,EAh,F2h
 */
#define REG_DMA3_RD_MONO      (1 << 6)
#define REG_DMA3_WR_MONO      (1 << 5)
#define REG_DMA3_RD_MONO_COPY (1 << 4)
#define REG_DMA3_MIU_SEL1     (1 << 3)
#define REG_DMA3_MIU_SEL0     (1 << 2)

/**
 * @brief Register E0H
 */
#define REG_WR_BASE_ADDR_ADD_POS 8
#define REG_WR_BASE_ADDR_ADD_MSK (0xF << REG_WR_BASE_ADDR_ADD_POS)
#define REG_RD_BASE_ADDR_ADD_POS 0
#define REG_RD_BASE_ADDR_ADD_MSK (0xF << REG_RD_BASE_ADDR_ADD_POS)

/****************************************************************************/
/*        AUDIO ANALOG registers bank5                                     */
/****************************************************************************/

/**
 * @brief Register 00h
 */
#define REG_EN_BYP_INMUX_POS 0
#define REG_EN_BYP_INMUX_MSK (0x3 << REG_EN_BYP_INMUX_POS)
#define REG_EN_CK_DAC_POS    4
#define REG_EN_CK_DAC_MSK    (0xF << REG_EN_CK_DAC_POS)
#define REG_EN_DAC_DISCH     (1 << 8)
#define REG_EN_EXT_CK        (1 << 9)
#define REG_EN_ITEST_DAC     (1 << 10)
#define REG_EN_MSP           (1 << 11)

/**
 * @brief Register 02h
 */
#define REG_EN_MUTE_INMUX_L    (1 << 0)
#define REG_EN_MUTE_INMUX_R    (1 << 1)
#define REG_EN_MUTE_MIC_STG1_L (1 << 2)
#define REG_EN_SHRT_L_ADC0     (1 << 6)
#define REG_EN_SHRT_R_ADC0     (1 << 7)
#define REG_IGEN_IBIAS_SEL_POS 8
#define REG_IGEN_IBIAS_SEL_MSK (0x3 << REG_IGEN_IBIAS_SEL_POS)
#define REG_EN_VREF_DISCH      (1 << 10)
#define REG_EN_VREF_SFTDCH_POS 11
#define REG_EN_VREF_SFTDCH_MSK (0x3 << REG_EN_VREF_SFTDCH_POS)

/**
 * @brief Register 04h
 */
#define REG_ADC_LDO_TEST_POS  3
#define REG_ADC_LDO_TEST_MSK  (0x3 << REG_ADC_LDO_TEST_POS)
#define REG_EN_SW_SW_TEST_POS 9
#define REG_EN_SW_SW_TEST_MSK (0xf << REG_EN_SW_SW_TEST_POS)

/**
 * @brief Register 06h
 */
#define REG_PD_BIAS_DAC (1 << 1)
#define REG_PD_INMUX_L  (1 << 2)
#define REG_PD_INMUX_R  (1 << 3)
#define REG_PD_L0_DAC   (1 << 4)
#define REG_PD_LDO_ADC  (1 << 5)
#define REG_PD_INT_L    (1 << 7)
#define REG_PD_INT_R    (1 << 8)
#define REG_PD_R0_DAC   (1 << 9)
#define REG_PD_REF_DAC  (1 << 10)
#define REG_PD_IGEN     (1 << 11)
#define REG_PD_VREF     (1 << 12)
#define REG_PD_SS_OP    (1 << 13)

/**
 * @brief Register 08h
 */
#define REG_RESET_ADC0    (1 << 0)
#define REG_RESET_ADC1    (1 << 1)
#define REG_RESET_DAC_POS 4
#define REG_RESET_DAC_MSK (0xF << REG_RESET_DAC_POS)

/**
 * @brief Register 0Ah
 */
#define REG_SEL_CH_INMUX_L_POS 0
#define REG_SEL_CH_INMUX_L_MSK (0x7 << REG_SEL_CH_INMUX_L_POS)
#define REG_SEL_CH_INMUX_R_POS 4
#define REG_SEL_CH_INMUX_R_MSK (0x7 << REG_SEL_CH_INMUX_R_POS)
#define REG_SEL_BIAS_DAC_POS   8
#define REG_SEL_BIAS_DAC_MSK   (0x3 << REG_SEL_BIAS_DAC_POS)
#define REG_SEL_CK_EDGE_DAC    (1 << 13)

/**
 * @brief Register 0Ch
 */
#define REG_SEL_GAIN_INMUX_L_POS 0
#define REG_SEL_GAIN_INMUX_L_MSK (0xF << REG_SEL_GAIN_INMUX_L_POS)
#define REG_SEL_GAIN_INMUX_R_POS 4
#define REG_SEL_GAIN_INMUX_R_MSK (0xF << REG_SEL_GAIN_INMUX_R_POS)
#define REG_SEL_CK_RTCPLL        (1 << 8)

/**
 * @brief Register 0Eh
 */
#define REG_SEL_IBIAS_INMUX_POS 8
#define REG_SEL_IBIAS_INMUX_MSK (0x3 << REG_SEL_IBIAS_INMUX_POS)

/**
 * @brief Register 10h
 */
#define REG_SEL_MICGAIN_STG1_L_POS 0
#define REG_SEL_MICGAIN_STG1_L_MSK (0x3 << REG_SEL_MICGAIN_STG1_L_POS)
#define REG_SEL_MICGAIN_STG1_R_POS 4
#define REG_SEL_MICGAIN_STG1_R_MSK (0x3 << REG_SEL_MICGAIN_STG1_R_POS)
#define REG_SEL_DAC_SW_POS         8
#define REG_SEL_DAC_SW_MSK         (0x3 << REG_SEL_DAC_SW_POS)

/**
 * @brief Register 12h
 */
#define REG_DAC_SW_TEST_POS 0
#define REG_DAC_SW_TEST_MSK (0xffff << REG_DAC_SW_TEST_POS)

/**
 * @brief Register 14h
 */
#define REG_DIT_ADC_L_POS 0
#define REG_DIT_ADC_L_MSK (0xf << REG_DIT_ADC_L_POS)
#define REG_DIT_ADC_R_POS 4
#define REG_DIT_ADC_R_MSK (0xf << REG_DIT_ADC_R_POS)
#define REG_DLF_DEM_L_POS 8
#define REG_DLF_DEM_L_MSK (0xf << REG_DLF_DEM_L_POS)
#define REG_DLF_DEM_R_POS 11
#define REG_DLF_DEM_R_MSK (0xf << REG_DLF_DEM_R_POS)

/**
 * @brief Register 16h
 */
#define REG_DLF_D1_L_POS 0
#define REG_DLF_D1_L_MSK (0x1f << REG_DLF_D1_L_POS)
#define REG_DLF_D1_R_POS 8
#define REG_DLF_D1_R_MSK (0x1f << REG_DLF_D1_R_POS)

/**
 * @brief Register 18h
 */
#define REG_DLF_D2_L_POS 0
#define REG_DLF_D2_L_MSK (0x1f << REG_DLF_D2_L_POS)
#define REG_DLF_D2_R_POS 8
#define REG_DLF_D2_R_MSK (0x1f << REG_DLF_D2_R_POS)

/**
 * @brief Register 1Ah
 */
#define REG_DLF_FZ_L_POS 0
#define REG_DLF_FZ_L_MSK (0x1f << REG_DLF_FZ_L_POS)
#define REG_DLF_FZ_R_POS 8
#define REG_DLF_FZ_R_MSK (0x1f << REG_DLF_FZ_R_POS)

/**
 * @brief Register 24h
 */
#define REG_ADC_LR_SWAP  (1 << 0)
#define REG_ADC2_LR_SWAP (1 << 6)

//
#define REG_SEL_MICGAIN_STG1_L_ALL_MSK (REG_SEL_MICGAIN_STG1_L_MSK | REG_SEL_MICGAIN_STG1_L_EXT_MSK)
#define REG_SEL_MICGAIN_STG1_R_ALL_MSK (REG_SEL_MICGAIN_STG1_R_MSK | REG_SEL_MICGAIN_STG1_R_EXT_MSK)

#define REG_SEL_MICGAIN_STG2_L_ALL_MSK (REG_SEL_MICGAIN_STG2_L_MSK | REG_SEL_MICGAIN_STG2_L_EXT_MSK)
#define REG_SEL_MICGAIN_STG2_R_ALL_MSK (REG_SEL_MICGAIN_STG2_R_MSK | REG_SEL_MICGAIN_STG2_R_EXT_MSK)

/**
 * @brief Register 60h
 */
#define REG_EN_SAR_LOFIC_L  (1 << 0)
#define REG_EN_SAR_LOFIC_R  (1 << 1)
#define REG_PD_SAR_L        (1 << 2)
#define REG_PD_SAR_R        (1 << 3)
#define REG_SAR_TIMER_L_POS 8
#define REG_SAR_TIMER_L_MSK (0x3 << REG_SAR_TIMER_L_POS)
#define REG_SAR_TIMER_R_POS 12
#define REG_SAR_TIMER_R_MSK (0x3 << REG_SAR_TIMER_R_POS)

//=============================================================================
// Global function definition
//=============================================================================
void HalBachSysSetBankBaseAddr(U64 nAddr);
void HalBachWriteRegByte(U32 nAddr, U8 regMsk, U8 nValue);
void HalBachWriteReg2Byte(U32 nAddr, U16 regMsk, U16 nValue);
void HalBachWriteReg(BachRegBank_e nBank, U8 nAddr, U16 regMsk, U16 nValue);
U16  HalBachReadReg2Byte(U32 nAddr);
U8   HalBachReadRegByte(U32 nAddr);
U16  HalBachReadReg(BachRegBank_e nBank, U8 nAddr);

#endif //__HAL_AUDIO_REG_H__
