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

#include "ms_platform.h"
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
#define BACH_REG_BANK_6    0x150600
#define BACH_REG_BANK_7    0x150700
#define BACH_PLL_BANK      0x141d00

typedef enum
{
    E_BACH_REG_BANK1,
    E_BACH_REG_BANK2,
    E_BACH_REG_BANK3,
    E_BACH_REG_BANK4,
    E_BACH_REG_BANK5,
    E_BACH_REG_BANK6,
    E_BACH_REG_BANK7,
    E_BACH_PLL_BANK,
    E_BACH_REG_BANK_TOTAL,
} BachRegBank_e;

enum
{
    // ----------------------------------------------
    // BANK6 BACH PLL
    // ----------------------------------------------
    E_BACH_BACH_PLL_SET0 = 0x0,
    E_BACH_BACH_PLL_SET1 = 0x02,
    E_BACH_BACH_PLL_SET2 = 0x10,
    E_BACH_BACH_PLL_SET3 = 0x16,
    E_BACH_BACH_PLL_SET4 = 0x18,
};
//=============================================================================
// Macro definition
//=============================================================================
#define AUDIO_IO_ADDRESS(x) ((long)(x) + MS_IO_OFFSET)

#define WRITE_BYTE(_reg, _val) (*((volatile U8*)((long)_reg))) = (U8)(_val)
#define WRITE_WORD(_reg, _val) (*((volatile U16*)((long)_reg))) = (U16)(_val)
#define WRITE_LONG(_reg, _val) (*((volatile U32*)((long)_reg))) = (U32)(_val)
#define READ_BYTE(_reg)        (*(volatile U8*)((long)(_reg)))
#define READ_WORD(_reg)        (*(volatile U16*)((long)_reg))
#define READ_LONG(_reg)        (*(volatile U32*)((long)_reg))

/****************************************************************************/
/*        AUDIO DIGITAL registers                                           */
/****************************************************************************/

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
#define REG_BP_ADC2_HPF2 (1 << 13)
#define REG_BP_ADC2_HPF1 (1 << 12)
#define REG_BP_ADC_HPF2  (1 << 11)
#define REG_BP_ADC_HPF1  (1 << 10)

/**
 * @brief Register 02h
 */
#define REG_CIC_1_SEL_POS  14
#define REG_CIC_1_SEL_MSK  (0x3 << REG_CIC_1_SEL_POS)
#define REG_CIC_2_SEL_POS  12
#define REG_CIC_2_SEL_MSK  (0x3 << REG_CIC_2_SEL_POS)
#define REG_CIC_3_SEL_POS  10
#define REG_CIC_3_SEL_MSK  (0x3 << REG_CIC_3_SEL_POS)
#define REG_WRITER_SEL_POS 8
#define REG_WRITER_SEL_MSK (0x3 << REG_WRITER_SEL_POS)
#define REG_SRC1_SEL_POS   4
#define REG_SRC1_SEL_MSK   (0xF << REG_SRC1_SEL_POS)
#define REG_SRC2_SEL_POS   0
#define REG_SRC2_SEL_MSK   (0xF << REG_SRC2_SEL_POS)

/**
 * @brief Register 04h
 */
#define REG_RESET_ASRC_POS 11
#define REG_RESET_ASRC_MSK (0x1 << REG_RESET_ASRC_POS)
#define REG_RESET_SDM_POS  10
#define REG_RESET_SDM_MSK  (0x1 << REG_RESET_SDM_POS)

/**
 * @brief Register 06h,
 */
#define REG_MMC2_SRC_SEL (1 << 4)
#define REG_MMC1_SRC_SEL (1 << 0)

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
 * @brief Register 30h
 */
#define REG_FREQ_DIFF_INT_CLEAR                     (1 << 15)
#define REG_HDMI_RX_ERROR_INT_CLEAR                 (1 << 14)
#define REG_HDMI_RX_MUTE_INT_CLEAR                  (1 << 13)
#define REG_HDMI_RX_UNMUTE_INT_CLEAR                (1 << 12)
#define REG_HDMI_RX_HEAD_DONE_INT_CLEAR             (1 << 11)
#define REG_HDMI_DMA1_2_POINTER_THRESHOLD_INT_CLEAR (1 << 10)
#define REG_DMA2_RW_POINTER_DIFF_TRIG               (1 << 9)
#define REG_DMA1_RW_POINTER_DIFF_TRIG               (1 << 8)
#define REG_DMA2_WR_THRESHOLD_INT_CLEAR             (1 << 7)
#define REG_DMA2_RD_THRESHOLD_INT_CLEAR             (1 << 6)
#define REG_DMA1_WR_THRESHOLD_INT_CLEAR             (1 << 5)
#define REG_DMA1_RD_THRESHOLD_INT_CLEAR             (1 << 4)
#define REG_DMA2_WR_OFFSET_ADDR_TRIG                (1 << 3)
#define REG_DMA2_RD_OFFSET_ADDR_TRIG                (1 << 2)
#define REG_DMA1_WR_OFFSET_ADDR_TRIG                (1 << 1)
#define REG_DMA1_RD_OFFSET_ADDR_TRIG                (1 << 0)

/**
 * @brief Register 36h
 */
#define REG_TX_FIFO_TX        (1 << 15)
#define REG_TX_FIFO_CLR       (1 << 14)
#define REG_CNT_RST           (1 << 13)
#define REG_SPDIF_FIX         (1 << 12)
#define REG_SPFIF_FIX_VAL     (1 << 11)
#define REG_SPDIF_CFG         (1 << 10)
#define REG_SPDIF_VALIDITY    (1 << 9)
#define REG_SPDIF_EN_ABS      (1 << 8)
#define REG_SPDIF_OUT_CS4_POS 0
#define REG_SPDIF_OUT_CS4_MSK (0xFF << 0)

/**
 * @brief Register 3Ch
 */
#define CODEC_DAAD_LOOP  (1 << 7)
#define CODEC_DAAD2_LOOP (1 << 11)

/**
 * @brief Register 40h,48h,50h,60h
 */
#define MUTE_2_ZERO (1 << 2)
#define FADING_EN   (1 << 1)
#define DPGA_EN     (1 << 0)

/**
 * @brief Register 42h
 */
#define STEP_POS 0
#define STEP_MSK (0x7 << STEP_POS)

/**
 * @brief Register 44h
 */
#define REG_OFFSET_R_POS 8
#define REG_OFFSET_R_MSK (0x7F << REG_OFFSET_R_POS)
#define REG_OFFSET_L_POS 0
#define REG_OFFSET_L_MSK (0x7F << REG_OFFSET_L_POS)

/**
 * @brief Register 42h,4Ah,52h,62h
 */
#define REG_GAIN_R_POS      4
#define REG_GAIN_R_MSK      (0x3FF << REG_GAIN_R_POS)
#define REG_GAIN_L_POS      4
#define REG_GAIN_L_MSK      (0x3FF << REG_GAIN_L_POS)
#define REG_GAIN_ENABLE_MSK 1 << 0

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
 * @brief Register A0h
 */
#define REG_DMA2_SW_RESET_MSK (1 << 0)

/**
 * @brief Register 9Eh
 */
#define REG_WR_LEVEL_CNT_POS 0
#define REG_WR_LEVEL_CNT_MSK (0xFFFF << REG_RD_LEVEL_CNT_POS)

/**
 * @brief Register C8h
 */
#define REG_FREQUENCE_LOWER_THR_POS 0
#define REG_FREQUENCE_LOWER_THR_MSK (0XFF << REG_FREQUENCE_LOWER_THR_POS)

#define REG_FREQUENCE_UPPER_THR_POS 8
#define REG_FREQUENCE_UPPER_THR_MSK (0XFF << REG_FREQUENCE_UPPER_THR_POS)

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
 * @brief Register CEh
 */
#define REG_DMA2_THRESHOLD_INT_EN (1 << 13)
#define REG_DMA1_THRESHOLD_INT_EN (1 << 12)

#define REG_DMA2_WR_TH_INT_EN_MASK (1 << 11)
#define REG_DMA2_RD_TH_INT_EN_MASK (1 << 10)
#define REG_DMA1_WR_TH_INT_EN_MASK (1 << 9)
#define REG_DMA1_RD_TH_INT_EN_MASK (1 << 8)
#define REG_DMA2_WR_AFTER_RD_POS   1
#define REG_DMA2_WR_AFTER_RD_MSK   (1 << REG_DMA2_WR_AFTER_RD_POS)
#define REG_DMA1_WR_AFTER_RD_POS   0
#define REG_DMA1_WR_AFTER_RD_MSK   (1 << REG_DMA1_WR_AFTER_RD_POS)

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
#define REG_AU2HDMI_EN           (1 << 15)
#define REG_AU2HDMI_MUX_SEL_POS  12
#define REG_AU2HDMI_MUX_SEL_MSK  (0x3 << REG_AU2HDMI_MUX_SEL_POS)
#define REG_HDMI_DPGA_GAIN_R_POS 0
#define REG_HDMI_DPGA_GAIN_R_MSK (0x3FF << REG_GAIN_R_POS)

/**F
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

/**
 * @brief Register F6h
 */
#define REG_HDMI_DPGA_GAIN_L_POS 6
#define REG_HDMI_DPGA_GAIN_L_MSK (0x3FF << REG_GAIN_R_POS)

/**
 * @brief Register FAh
 */
#define REG_DMA1_RD_L_SEL_POS 0
#define REG_DMA1_RD_L_SEL_MSK (0x7 << REG_DMA1_RD_L_SEL_POS)
#define REG_DMA1_RD_R_SEL_POS 3
#define REG_DMA1_RD_R_SEL_MSK (0X7 << REG_DMA1_RD_R_SEL_POS)
#define REG_DMA2_RD_L_SEL_POS 6
#define REG_DMA2_RD_L_SEL_MSK (0x7 << REG_DMA2_RD_L_SEL_POS)
#define REG_DMA2_RD_R_SEL_POS 9
#define REG_DMA2_RD_R_SEL_MSK (0x7 << REG_DMA2_RD_R_SEL_POS)

/****************************************************************************/
/*        AUDIO DIGITAL registers bank2                                     */
/****************************************************************************/
/**
 * @brief Register 0Ah
 */
#define BACH_DMA_WR2_CHAB_SEL_POS 12
#define BACH_DMA_WR2_CHAB_SEL_MSK (0xF << BACH_DMA_WR2_CHAB_SEL_POS)
#define BACH_DMA_WR2_CH89_SEL_POS 8
#define BACH_DMA_WR2_CH89_SEL_MSK (0xF << BACH_DMA_WR2_CH89_SEL_POS)
#define BACH_DMA_WR1_CHAB_SEL_POS 4
#define BACH_DMA_WR1_CHAB_SEL_MSK (0xF << BACH_DMA_WR1_CHAB_SEL_POS)
#define BACH_DMA_WR1_CH89_SEL_POS 0
#define BACH_DMA_WR1_CH89_SEL_MSK (0xF << BACH_DMA_WR1_CH89_SEL_POS)

/**
 * @brief Register 0Ch
 */
#define BACH_DMA_WR2_CHEF_SEL_POS 12
#define BACH_DMA_WR2_CHEF_SEL_MSK (0xF << BACH_DMA_WR2_CHEF_SEL_POS)
#define BACH_DMA_WR2_CHCD_SEL_POS 8
#define BACH_DMA_WR2_CHCD_SEL_MSK (0xF << BACH_DMA_WR2_CHCD_SEL_POS)
#define BACH_DMA_WR1_CHEF_SEL_POS 4
#define BACH_DMA_WR1_CHEF_SEL_MSK (0xF << BACH_DMA_WR1_CHEF_SEL_POS)
#define BACH_DMA_WR1_CHCD_SEL_POS 0
#define BACH_DMA_WR1_CHCD_SEL_MSK (0xF << BACH_DMA_WR1_CHCD_SEL_POS)

/**
 * @brief Register 0Eh
 */
#define REG_DMA_TH_INT_EN    (1 << 6)
#define REG_DMA_POINT_INT_EN (1 << 5)
#define REG_FREQ_DIF_INT_EN  (1 << 4)
#define REG_HDMI_RX_INT_EN   (1 << 3)
#define REG_DMA_INT_EN       (1 << 0)

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
 * @brief Register 4Ah
 */
#define REG_HDMI_RX_DPGA_MUTE_L           (1 << 15)
#define REG_HDMI_RX_DPGA_MUTE_R           (1 << 14)
#define REG_HDMI_RX_ERR_INT_STATUS        (1 << 12)
#define REG_HDMI_RX_MUTE_INT_STATUS       (1 << 11)
#define REG_HDMI_RX_UNMUTE_INT_STATUS     (1 << 10)
#define REG_HDMI_RX_HEAD_DONE_INT_STATUS  (1 << 9)
#define REG_FREQ_DIFF_INTERRUPT_STATUS    (1 << 8)
#define REG_DMA_2_WR_THRESHOLD_IRQ_STATUS (1 << 7)
#define REG_DMA_2_RD_THRESHOLD_IRQ_STATUS (1 << 6)
#define REG_DMA_1_WR_THRESHOLD_IRQ_STATUS (1 << 5)
#define REG_DMA_1_RD_THRESHOLD_IRQ_STATUS (1 << 4)
#define REG_DMA_2_AUD_DELAY_TH_IRQ_STATUS (1 << 1)
#define REG_DMA_1_AUD_DELAY_TH_IRQ_STATUS (1 << 0)

/**
 * @brief Register 4Eh
 */
#define REG_MMC1_L_MUX_POS 8
#define REG_MMC1_L_MUX_MSK (0x7F << REG_MMC1_L_MUX_POS)
#define REG_MMC1_R_MUX_POS 0
#define REG_MMC1_R_MUX_MSK (0x7F << REG_MMC1_R_MUX_POS)

/**
 * @brief Register 60h
 */
#define BACH_I2S_RX_MS_MODE        (1 << 14)
#define BACH_I2S_RX_CHLEN_POS      10
#define BACH_I2S_RX_CHLEN_MSK      (0x7 << BACH_I2S_RX_CHLEN_POS)
#define BACH_I2S_RX_ENC_WIDTH      (1 << 9)
#define BACH_I2S_RX_WS_FMT         (1 << 7)
#define BACH_I2S_RX_FMT            (1 << 6)
#define BACH_I2S_RX_WS_INV         (1 << 5)
#define BACH_I2S_RX_BCK_DG_EN      (1 << 3)
#define BACH_I2S_RX_BCK_DG_NUM_POS 0
#define BACH_I2S_RX_BCK_DG_NUM_MSK (0x7 << BACH_I2S_RX_BCK_DG_NUM_POS)

/**
 * @brief Register 62h
 */
#define BACH_I2S_TDM_TX_SEL_POS 10
#define BACH_I2S_TDM_TX_SEL_MSK (0X7 << BACH_I2S_TDM_TX_SEL_POS)
#define BACH_I2S_RX_WS_WDTH_POS 5
#define BACH_I2S_RX_WS_WDTH_MSK (0x1F << BACH_I2S_RX_WS_WDTH_POS)
#define BACH_I2S_TX_WS_WDTH_POS 0
#define BACH_I2S_TX_WS_WDTH_MSK (0x1F << BACH_I2S_TX_WS_WDTH_POS)

/**
 * @brief Register 64h
 */
/**
 * @brief Register 6Ah
 */
#define BACH_I2S_TX_CHLEN_2    (1 << 15)
#define BACH_I2S_TX_MS_MODE    (1 << 14)
#define BACH_I2S_TX_CHLEN_1    (1 << 13)
#define BACH_I2S_TX_4WIRE_MODE (1 << 12)
#define BACH_FIFO_STATUS_CLR   (1 << 11)
#define BACH_I2S_TX_ENC_WIDTH  (1 << 9)
#define BACH_I2S_TX_CHLEN      (1 << 8)
#define BACH_I2S_TX_WS_FMT     (1 << 7)
#define BACH_I2S_TX_FMT        (1 << 6)
#define BACH_I2S_TX_WS_INV     (1 << 5)
#define BACH_I2S_TX_OUT_MODE   (1 << 4) // Tx output from same edge of bck samp edge
#define BACH_I2S_TX_BCK_DG_EN  (1 << 3)
#define BACH_I2S_TX_BCK_DG_POS 0
#define BACH_I2S_TX_BCK_DG_MSK (0x7 << BACH_I2S_TX_BCK_DG_POS)

/**
 * @brief Register 6Ch
 */
#define BACH_I2S_TX_ACT_SLOT_POS 0
#define BACH_I2S_TX_ACT_SLOT_MSK (0xFFFF << BACH_I2S_TX_ACT_SLOT_POS)

/**
 * @brief Register 78h
 */
#define BACH_RX_DIV_SEL_POS     14
#define BACH_RX_DIV_SEL_MSK     (0x3 << BACH_RX_DIV_SEL_POS)
#define BACH_INV_CLK_RX_BCK_POS 13
#define BACH_INV_CLK_RX_BCK_MSK (1 << BACH_INV_CLK_RX_BCK_POS)
#define BACH_ENABLE_RX_BCK_POS  12
#define BACH_ENABLE_RX_BCK_MSK  (1 << BACH_ENABLE_RX_BCK_POS)
#define BACH_TX_DIV_SEL_POS     10
#define BACH_TX_DIV_SEL_MSK     (0x3 << BACH_TX_DIV_SEL_POS)
#define BACH_INV_CLK_TX_BCK_POS 9
#define BACH_INV_CLK_TX_BCK_MSK (1 << BACH_INV_CLK_TX_BCK_POS)
#define BACH_ENABLE_TX_BCK_POS  8
#define BACH_ENABLE_TX_BCK_MSK  (1 << BACH_ENABLE_TX_BCK_POS)
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
#define BACH_MCK_DIV_POS                    0
#define BACH_MCK_DIV_MSK                    (0xFF << BACH_MCK_DIV_POS)

/**
 * @brief Register 96h,98h
 */
#define BACH_DMA_WR_CH23_SEL_POS  12
#define BACH_DMA_WR_CH23_SEL_MSK  (0xF << BACH_DMA_WR_CH23_SEL_POS)
#define BACH_DMA_WR_CH01_SEL_POS  8
#define BACH_DMA_WR_CH01_SEL_MSK  (0xF << BACH_DMA_WR_CH01_SEL_POS)
#define BACH_DMA_WR_VALID_SEL_POS 5
#define BACH_DMA_WR_VALID_SEL_MSK (0x7 << BACH_DMA_WR_VALID_SEL_POS)
#define BACH_DMA_WR_CH_MODE_POS   2
#define BACH_DMA_WR_CH_MODE_MSK   (0x7 << BACH_DMA_WR_CH_MODE_POS)
#define BACH_DMA_WR_NEW_MODE_POS  0
#define BACH_DMA_WR_NEW_MODE_MSK  (0x3 << BACH_DMA_WR_NEW_MODE_POS)
/**
 * @brief Register 9Ah
 */
#define BACH_I2S_TDM_RX1_POS     8
#define BACH_I2S_TDM_RX1_MSK     (0x3 << BACH_I2S_TDM_RX1_POS)
#define BACH_DMA_RD1_TDM_SEL_POS 6
#define BACH_DMA_RD1_TDM_SEL_MSK (1 << BACH_DMA_RD1_TDM_SEL_POS)
#define BACH_DMA_RD2_TDM_SEL_POS 7
#define BACH_DMA_RD2_TDM_SEL_MSK (1 << BACH_DMA_RD2_TDM_SEL_POS)
#define BACH_DMA_RD2_SEL_POS     4
#define BACH_DMA_RD2_SEL_MSK     (0x3 << BACH_DMA_RD2_SEL_POS)
#define BACH_DMA_RD1_SEL_POS     2
#define BACH_DMA_RD1_SEL_MSK     (0x3 << BACH_DMA_RD1_SEL_POS)
#define BACH_I2S_TDM_RX_POS      0
#define BACH_I2S_TDM_RX_MSK      (0x3 << BACH_I2S_TDM_RX_POS)

#define BACH_DMA_MCH_DEBUG_MODE_1 (1 << 15)
#define BACH_DMA_W1_BIT_MODE      (1 << 14)
#define BACH_MCH_32B_EN_1         (1 << 13)
#define BACH_DMA_MCH_DEBUG_MODE_2 (1 << 12)
#define BACH_DMA_W2_BIT_MODE      (1 << 11)
#define BACH_MCH_32B_EN_2         (1 << 10)

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

/**
 * @brief Register B2h
 */
#define REG_SPDIF_RX_SEL_POS 14
#define REG_SPDIF_RX_SEL_MSK (0x3 << REG_SPDIF_RX_SEL_POS)

/**
 * @brief Register B4h
 */
#define REG_SPDIF_TX_CLK_SEL_POS 14
#define REG_SPDIF_TX_CLK_SEL_MSK (0x3 << REG_SPDIF_TX_CLK_SEL_POS)

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
#define REG_CHN_MODE_DMIC_MSK (0x7 << REG_CHN_MODE_DMIC_POS)
#define REG_PAUSE_DMA_WRITER  (0x1 << 2)

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
#define REG_SIN_PATH_SEL_POS    6
#define REG_SIN_PATH_SEL_MSK    (0x1 << REG_SIN_PATH_SEL_POS)
#define REG_SIN_PATH_SEL_LR_POS 4
#define REG_SIN_PATH_SEL_LR_MSK (0x3 << REG_SIN_PATH_SEL_LR_POS)
#define REG_DMIC_SIN_EN_DMIC67  (0x1 << 3)
#define REG_DMIC_SIN_EN_DMIC45  (0x1 << 2)
#define REG_DMIC_SIN_EN_DMIC23  (0x1 << 1)
#define REG_DMIC_SIN_EN_DMIC01  (0x1 << 0)

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
 * @brief Register 0Eh
 */
#define REG_DIG_MIC_BCK_DG_NUM_POS (4)
#define REG_DIG_MIC_BCK_DG_NUM_MSK (0x7 << REG_DIG_MIC_BCK_DG_NUM_POS)
#define REG_DIG_MIC_BCK_DG_EN      (1 << 3)
#define REG_DIG_MIC_CLK_SEL        (1 << 0)

/**
 * @brief Register 20h
 */
#define REG_TURN_OFF_DMIC4 (0x1 << 7)
#define REG_TURN_OFF_DMIC3 (0x1 << 6)
#define REG_TURN_OFF_DMIC2 (0x1 << 5)
#define REG_TURN_OFF_DMIC1 (0x1 << 4)

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
 * @brief Register 04h
 */
#define REG_HDMI_RX_EN_HWAUTO_MSK (1 << 5)
#define REG_HDMI_SAMPLE_GEN_MSK   (1 << 8)
#define REG_HDMI_RX_HEADDONE_POS  9
#define REG_HDMI_RX_HEADDONE_MSK  (1 << REG_HDMI_RX_HEADDONE_POS)
#define REG_HDMI_RX_UNMUTE_MSK    (1 << 10)
#define REG_HDMI_RX_MUTE_MSK      (1 << 11)
#define REG_HDMI_RX_ERROR_MSK     (1 << 12)

/**
 * @brief Register 60h
 */
#define REG_I2S_RX1_CHLEN_POS 8
#define REG_I2S_RX1_CHLEN_MSK (1 << REG_I2S_RX1_CHLEN_POS)
/**
 * @brief Register 62h
 */
#define REG_I2S_TDM_TX1_DATA_SEL_POS 10
#define REG_I2S_TDM_TX1_DATA_SEL_MSK (0x7 << REG_I2S_TDM_TX1_DATA_SEL_POS)

/**
 * @brief Register 04h
 */
#define REG_SELECT_SOURCE_DATA_OF_SPDIF (1 << 15)
#define REG_HDMI_RX_ERR_INT_MASK        (1 << 12)
#define REG_HDMI_RX_MUTE_INT_MASK       (1 << 11)
#define REG_HDMI_RX_UNMUTE_INT_MASK     (1 << 10)
#define REG_HDMI_RX_HEAD_DONE_INT_MASK  (1 << 9)
#define REG_HDMI_SAMPLE_GEN             (1 << 8)
#define REG_HDMI_RX_HWAUTO              (1 << 5)
#define REG_HDMI_RX_WRITE_LPCM_FLAG     (1 << 4)
#define REG_HDMI_RX_RENEW               (1 << 2)
#define REG_HDMI_RX_EN_POS              0
#define REG_HDMI_RX_EN_MSK              (0x3 << REG_HDMI_RX_EN_POS)

/**
 * @brief Register 12h
 */
#define BACH_SPDIF_TX_SOURCE_SEL_POS    14
#define BACH_SPDIF_TX_SOURCE_SEL_MSK    (0x3 << BACH_SPDIF_TX_SOURCE_SEL_POS)
#define BACH_FREQ_DIFF_TIME_POS         9
#define BACH_FREQ_DIFF_TIME_MSK         (0X1F << BACH_FREQ_DIFF_TIME_POS)
#define BACH_FREQ_DIFF_ENABLE_POS       8
#define BACH_FREQ_DIFF_ENABLE_MSK       (0x1 << BACH_FREQ_DIFF_ENABLE_POS)
#define BACH_SPDIF_TX_SRC_SEL_POS       7
#define BACH_SPDIF_TX_SRC_SEL_MSK       (0x1 << BACH_SPDIF_TX_SRC_SEL_POS)
#define BACH_AU2SPDIF_SAMPLE_SEL_POS    6
#define BACH_AU2SPDIF_SAMPLE_SEL_MSK    (0x1 << BACH_AU2SPDIF_SAMPLE_SEL_POS)
#define BACH_SPDIF_TX_NF_SYNTH_TRIG_MSK (0x1 << 5)
#define BACH_SPDIF_TX_EN_TIME_GEN_MSK   (0x1 << 4)
#define BACH_SEL_CLK_SYNTH_FREQ_POS     2
#define BACH_SEL_CLK_SYNTH_FREQ_MSK     (0x3 << BACH_SEL_CLK_SYNTH_FREQ_POS)
#define BACH_SEL_ClK_SYNTH_SPDIF_TX_POS 0
#define BACH_SEL_ClK_SYNTH_SPDIF_TX_MSK (0x3 << BACH_SEL_ClK_SYNTH_SPDIF_TX_POS)

/**
 * @brief Register 6Ah
 */
#define REG_I2S_TX1_CHLEN_POS 8
#define REG_I2S_TX1_CHLEN_MSK (1 << REG_I2S_TX1_CHLEN_POS)

/**
 * @brief Register 78h
 */
#define BACH_INV_CLK_RX1_BCK_POS       13
#define BACH_INV_CLK_RX1_BCK_MSK       (1 << BACH_INV_CLK_RX1_BCK_POS)
#define BACH_INV_CLK_TX1_BCK_POS       9
#define BACH_INV_CLK_TX1_BCK_MSK       (1 << BACH_INV_CLK_TX1_BCK_POS)
#define BACH_I2S_MCK_EN_POS            4
#define BACH_I2S_MCK_EN_MSK            (0x1 << BACH_I2S_MCK_EN_POS)
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
 * @brief Register B0h
 */
#define BACH_MMC2_R_MUX_SEL_POS 0
#define BACH_MMC2_R_MUX_SEL_MSK (0x7F << BACH_MMC2_R_MUX_SEL_POS)
#define BACH_MMC2_L_MUX_SEL_POS 8
#define BACH_MMC2_L_MUX_SEL_MSK (0x7F << BACH_MMC2_L_MUX_SEL_POS)

/**
 * @brief Register B2h
 */
#define BACH_SRC_MIXER2_L_SEL_POS 9
#define BACH_SRC_MIXER2_L_SEL_MSK (0x1F << BACH_SRC_MIXER2_L_SEL_POS)
#define BACH_SRC_MIXER2_R_SEL_POS 1
#define BACH_SRC_MIXER2_R_SEL_MSK (0x1F << BACH_SRC_MIXER2_R_SEL_POS)

/**
 * @brief Register B6h
 */
#define BACH_SRC_MIXER1_L_SEL_POS 9
#define BACH_SRC_MIXER1_L_SEL_MSK (0x1F << BACH_SRC_MIXER1_L_SEL_POS)
#define BACH_SRC_MIXER1_R_SEL_POS 1
#define BACH_SRC_MIXER1_R_SEL_MSK (0x1F << BACH_SRC_MIXER1_R_SEL_POS)

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
#define REG_DAC_SW_TEST     (1 << 9)
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
 * @brief Register 2Eh
 */
#define REG_VCM_BUF_EN (1 << 10)

/**
 * @brief Register 30h
 */

#define REG_SEL_CH_INMUX_M_POS 0
#define REG_SEL_CH_INMUX_M_MSK (0x7 << REG_SEL_CH_INMUX_M_POS)
#define REG_GAIN_INMUX_M_POS   12
#define REG_GAIN_INMUX_M_MSK   (0xF << REG_GAIN_INMUX_M_POS)

/**
 * @brief Register 34h
 */
#define REG_EN_TEST        (1 << 1)
#define REG_EN_SAR_LOGIC_M (1 << 2)
#define REG_PD_INMUX_M     (1 << 6)
#define REG_PD_INT_M       (1 << 7)
#define REG_PD_SAR_M       (1 << 8)

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

/****************************************************************************/
/*        AUDIO BACH PLL registers bank6                                    */
/****************************************************************************/
/**
 * @brief Register 00h
 */
#define REG_BACHPLL_PD_POS     1
#define REG_BACHPLL_PD_MSK     (1 << REG_BACHPLL_PD_POS)
#define REG_BACHPLL_ENXTAL_POS 7
#define REG_BACHPLL_ENXTAL_MSK (1 << REG_BACHPLL_ENXTAL_POS)
#define REG_BACHPLL_ICTRL_POS  12
#define REG_BACHPLL_ICTRL_MSK  (0x7 << REG_BACHPLL_ICTRL_POS)

/**
 * @brief Register 02h
 */
#define REG_BACHPLL_OUTPUT_DIV_POS     1
#define REG_BACHPLL_OUTPUT_DIV_MSK     (0x7 << REG_BACHPLL_OUTPUT_DIV_POS)
#define REG_BACHPLL_LOOPDIV_FIR_POS    4
#define REG_BACHPLL_LOOPDIV_FIR_MSK    (0x3 << REG_BACHPLL_LOOPDIV_FIR_POS)
#define REG_BACHPLL_EN_PRDT_POS        7
#define REG_BACHPLL_EN_PRDT_MSK        (1 << REG_BACHPLL_EN_PRDT_POS)
#define REG_BACHPLL_OUTPUT_DIV_MSK     (0x7 << REG_BACHPLL_OUTPUT_DIV_POS)
#define REG_BACHPLL_LOOPDIV_SECOND_POS 8
#define REG_BACHPLL_LOOPDIV_SECOND_MSK (0xFF << REG_BACHPLL_LOOPDIV_SECOND_POS)

/**
 * @brief Register 10h
 */
#define REG_BACHPLL_POST_DIV1_POS 0
#define REG_BACHPLL_POST_DIV1_MSK (0x3F << REG_BACHPLL_POST_DIV1_POS)
#define REG_BACHPLL_POST_DIV2_POS 6
#define REG_BACHPLL_POST_DIV2_MSK (0x3 << REG_BACHPLL_POST_DIV2_POS)

/**
 * @brief Register 16h
 */
#define REG_SYNTH_SET_0_POS 0
#define REG_SYNTH_SET_0_MSK (0xFFFF << REG_SYNTH_SET_0_POS)

/**
 * @brief Register 18h
 */
#define REG_SYNTH_SET_1_POS 0
#define REG_SYNTH_SET_1_MSK (0xFF << REG_SYNTH_SET_1_POS)

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
