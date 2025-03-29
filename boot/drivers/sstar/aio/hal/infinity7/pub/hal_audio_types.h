/*
 * hal_audio_types.h - Sigmastar
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

#ifndef __HAL_AUD_TYPES_H__
#define __HAL_AUD_TYPES_H__

// -------------------------------------------------------------------------------
// DMA
#define CHIP_AI_DMA_VALID(x) ((unsigned int)x < (E_CHIP_AIO_DMA_AI_END - E_CHIP_AIO_DMA_AI_START + 1))
#define CHIP_AO_DMA_VALID(x) ((unsigned int)x < (E_CHIP_AIO_DMA_AO_END - E_CHIP_AIO_DMA_AO_START + 1))

// AI
#define CHIP_AI_ADC_IDX_BY_DEV(x)    (AI_DEV_ADC_IDX(x))
#define CHIP_AI_DMIC_IDX_BY_DEV(x)   (AI_DEV_DMIC_IDX(x))
#define CHIP_AI_SRC_IDX_BY_DEV(x)    (AI_DEV_ECHO_IDX(x))
#define CHIP_AI_I2S_RX_IDX_BY_DEV(x) (AI_DEV_I2S_RX_IDX(x))

#define CHIP_AI_ADC_VALID(x)    ((unsigned int)x < E_CHIP_AI_ADC_TOTAL)
#define CHIP_AI_DMIC_VALID(x)   ((unsigned int)x < E_CHIP_AI_DMIC_TOTAL)
#define CHIP_AI_SRC_VALID(x)    ((unsigned int)x < E_CHIP_AI_SRC_TOTAL)
#define CHIP_AI_I2S_RX_VALID(x) ((unsigned int)x < (E_CHIP_AIO_I2S_RX_END - E_CHIP_AIO_I2S_RX_START + 1))

typedef enum
{
    E_CHIP_AI_ADC_A,
    E_CHIP_AI_ADC_B,
    E_CHIP_AI_ADC_TOTAL,
    E_CHIP_AI_ADC_NONE,

} CHIP_AI_ADC_e;

typedef enum
{
    E_CHIP_AI_DMIC_A,
    E_CHIP_AI_DMIC_TOTAL,
    E_CHIP_AI_DMIC_NONE,

} CHIP_AI_DMIC_e;

typedef enum
{
    E_CHIP_AI_SRC_A,
    E_CHIP_AI_SRC_TOTAL,
    E_CHIP_AI_SRC_NONE,

} CHIP_AI_SRC_e;

// -------------------------------------------------------------------------------

// AO
#define CHIP_AO_DAC_IDX_BY_DEV(x)    (AO_DEV_DAC_IDX(x))
#define CHIP_AO_HDMI_IDX_BY_DEV(x)   (AO_DEV_HDMI_IDX(x))
#define CHIP_AO_SRC_IDX_BY_DEV(x)    (AO_DEV_ECHO_IDX(x))
#define CHIP_AO_I2S_TX_IDX_BY_DEV(x) (AO_DEV_I2S_TX_IDX(x))

#define CHIP_AO_DAC_VALID(x)    ((unsigned int)x < E_CHIP_AO_DAC_TOTAL)
#define CHIP_AO_HDMI_VALID(x)   ((unsigned int)x < E_CHIP_AO_HDMI_TOTAL)
#define CHIP_AO_SRC_VALID(x)    ((unsigned int)x < E_CHIP_AO_SRC_TOTAL)
#define CHIP_AO_I2S_TX_VALID(x) ((unsigned int)x < (E_CHIP_AIO_I2S_TX_END - E_CHIP_AIO_I2S_TX_START + 1))

typedef enum
{
    E_CHIP_AO_DAC_A,
    E_CHIP_AO_DAC_TOTAL,
    E_CHIP_AO_DAC_NONE,

} CHIP_AO_DAC_e;

typedef enum
{
    E_CHIP_AO_HDMI_A,
    E_CHIP_AO_HDMI_TOTAL,
    E_CHIP_AO_HDMI_NONE,

} CHIP_AO_HDMI_e;

typedef enum
{
    E_CHIP_AO_SRC_A,
    E_CHIP_AO_SRC_TOTAL,
    E_CHIP_AO_SRC_NONE,

} CHIP_AO_SRC_e;

// -------------------------------------------------------------------------------

// AIO
#define CHIP_AIO_DMA_IDX_BY_AI_DMA(x) ((x))
#define CHIP_AIO_DMA_IDX_BY_AO_DMA(x) ((x) + (E_CHIP_AIO_DMA_AI_END - E_CHIP_AIO_DMA_AI_START + 1))
#define CHIP_AIO_DMA_IDX_VALID(x)     ((unsigned int)x < E_CHIP_AIO_DMA_TOTAL)

typedef enum
{
    E_CHIP_AIO_DMA_AI_START,
    E_CHIP_AIO_DMA_AI_A = E_CHIP_AIO_DMA_AI_START,
    E_CHIP_AIO_DMA_AI_B,
    E_CHIP_AIO_DMA_AI_C,
    E_CHIP_AIO_DMA_AI_D,
    E_CHIP_AIO_DMA_AI_E,
    E_CHIP_AIO_DMA_AI_DIRECT_A,
    E_CHIP_AIO_DMA_AI_DIRECT_B,
    E_CHIP_AIO_DMA_AI_END = E_CHIP_AIO_DMA_AI_DIRECT_B,
    E_CHIP_AIO_DMA_AO_START,
    E_CHIP_AIO_DMA_AO_A = E_CHIP_AIO_DMA_AO_START,
    E_CHIP_AIO_DMA_AO_B,
    E_CHIP_AIO_DMA_AO_C,
    E_CHIP_AIO_DMA_AO_D,
    E_CHIP_AIO_DMA_AO_E,
    E_CHIP_AIO_DMA_AO_DIRECT_A,
    E_CHIP_AIO_DMA_AO_DIRECT_B,
    E_CHIP_AIO_DMA_AO_END = E_CHIP_AIO_DMA_AO_DIRECT_B,
    E_CHIP_AIO_DMA_TOTAL,

} CHIP_AIO_DMA_e;

#define CHIP_AIO_I2S_IDX_BY_AI_DEV_I2S_RX(x) (AI_DEV_I2S_RX_IDX(x))
#define CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(x) \
    (AO_DEV_I2S_TX_IDX(x) + (E_CHIP_AIO_I2S_RX_END - E_CHIP_AIO_I2S_RX_START + 1))
#define CHIP_AIO_I2S_IDX_VALID(x) ((unsigned int)x < E_CHIP_AIO_I2S_TOTAL)

typedef enum
{
    E_CHIP_AIO_I2S_RX_START,
    E_CHIP_AIO_I2S_RX_A = E_CHIP_AIO_I2S_RX_START,
    E_CHIP_AIO_I2S_RX_B,
    E_CHIP_AIO_I2S_RX_C,
    E_CHIP_AIO_I2S_RX_D,
    E_CHIP_AIO_I2S_RX_END = E_CHIP_AIO_I2S_RX_D,
    E_CHIP_AIO_I2S_TX_START,
    E_CHIP_AIO_I2S_TX_A = E_CHIP_AIO_I2S_TX_START,
    E_CHIP_AIO_I2S_TX_B,
    E_CHIP_AIO_I2S_TX_END = E_CHIP_AIO_I2S_TX_B,
    E_CHIP_AIO_I2S_TOTAL,
    E_CHIP_AIO_I2S_NONE,

} CHIP_AIO_I2S_e;

typedef enum
{
    E_AUD_HPF_ADC1_DMIC_2CH = 0,
    E_AUD_HPF_DMIC_4CH,
    E_AUD_HPF_DEV_NUM,
    E_AUD_HPF_DEV_NULL = 0xff
} AudHpfDev_e;

#define CHIP_AIO_ATOP_IDX_BY_AI_DEV_ADC(x) ((AI_DEV_ADC_IDX(x)) / 2)
#define CHIP_AIO_ATOP_IDX_BY_AO_DEV_DAC(x) ((AO_DEV_DAC_IDX(x) + E_CHIP_AI_ADC_TOTAL) / 2)
#define CHIP_AIO_ATOP_IDX_VALID(x)         ((unsigned int)x < E_CHIP_AIO_ATOP_TOTAL)

typedef enum
{
    E_CHIP_AIO_ATOP_ADC_AB, // E_CHIP_AI_ADC_A + E_CHIP_AI_ADC_B
    E_CHIP_AIO_ATOP_DAC_AB, // E_CHIP_AO_DAC_A + E_CHIP_AO_DAC_B
    E_CHIP_AIO_ATOP_TOTAL,
    E_CHIP_AIO_ATOP_NONE,

} CHIP_AIO_ATOP_e;

// -------------------------------------------------------------------------------

// DPGA
#define CHIP_DPAG_IDX_VALID(x) ((unsigned int)x < E_CHIP_DPGA_TOTAL)

typedef enum
{
    E_CHIP_DPGA_A, // E_AUD_DPGA_MMC1
    E_CHIP_DPGA_B, // E_AUD_DPGA_MMC2
    E_CHIP_DPGA_C, // E_AUD_DPGA_ADC
    E_CHIP_DPGA_D, // E_AUD_DPGA_AEC1
    E_CHIP_DPGA_E, // E_AUD_DPGA_DEC1
    E_CHIP_DPGA_F, // E_AUD_DPGA_DEC2
    E_CHIP_DPGA_G, // E_AUD_DPGA_TDM0
    E_CHIP_DPGA_H, // E_AUD_DPGA_TDM1
    E_CHIP_DPGA_I, // AUD_DPGA_HDMI
    E_CHIP_DPGA_TOTAL,
    E_CHIP_DPGA_NONE,
} EN_CHIP_DPGA;

// ADC_MUX
typedef enum
{
    E_CHIP_ADC_MUX_MICIN,
    E_CHIP_ADC_MUX_LINEIN,
    E_CHIP_ADC_MUX_TOTAL,
    E_CHIP_ADC_MUX_NONE,

} CHIP_ADC_MUX_e;

// MCH
typedef enum
{
    E_CHIP_AI_MCH_A,
    E_CHIP_AI_MCH_B,
    E_CHIP_AI_MCH_C,
    E_CHIP_AI_MCH_D,
    E_CHIP_AI_MCH_E,
    E_CHIP_AI_MCH_TOTAL,
    E_CHIP_AI_MCH_NONE,

} EN_CHIP_AI_MCH;

// -------------------------------------------------------------------------------
typedef enum
{
    E_AUD_RATE_SLAVE,
    E_AUD_RATE_8K,
    E_AUD_RATE_11K,
    E_AUD_RATE_12K,
    E_AUD_RATE_16K,
    E_AUD_RATE_22K,
    E_AUD_RATE_24K,
    E_AUD_RATE_32K,
    E_AUD_RATE_44K,
    E_AUD_RATE_48K,
    E_AUD_RATE_96K,
    E_AUD_RATE_192K,
    E_AUD_RATE_NUM,
    E_AUD_RATE_NULL = 0xff,

} AudRate_e;

typedef enum
{
    E_AUD_BITWIDTH_16,
    E_AUD_BITWIDTH_32
} AudBitWidth_e;

typedef enum
{
    E_AUD_I2S_MODE_I2S,
    E_AUD_I2S_MODE_TDM,
} AudI2sMode_e;

typedef enum
{
    E_AUD_I2S_MSMODE_MASTER,
    E_AUD_I2S_MSMODE_SLAVE,
} AudI2sMsMode_e;

typedef enum
{
    E_AUD_I2S_FMT_I2S,
    E_AUD_I2S_FMT_LEFT_JUSTIFY
} AudI2sFmt_e;

typedef enum
{
    E_AUD_I2S_MCK_NULL,    // Disable
    E_AUD_I2S_MCK_12_288M, // 12.288MHz
    E_AUD_I2S_MCK_16_384M, // 16.384MHz
    E_AUD_I2S_MCK_18_432M, // 18.432MHz
    E_AUD_I2S_MCK_24_576M, // 24.576MHz
    E_AUD_I2S_MCK_24M,     // 24MHz
    E_AUD_I2S_MCK_48M,     // 48MHz
} AudI2sMck_e;

typedef enum
{
    E_AUD_I2S_WIRE_4,
    E_AUD_I2S_WIRE_6,
    E_AUD_I2S_WIRE_NULL = 0xff
} AudWireMode_e;

typedef enum
{
    E_AUD_TDM_CHN_MAP0,
    E_AUD_TDM_CHN_MAP1,
    E_AUD_TDM_CHN_MAP2,
    E_AUD_TDM_CHN_MAP_NULL = 0xff
} AudTdmChnMap_e;

typedef enum
{
    E_AUD_DMA_WRITER1,
    E_AUD_DMA_WRITER2,
    E_AUD_DMA_READER1,
    E_AUD_DMA_READER2,
    E_AUD_DMA_NUM,
    E_AUD_DMA_NULL = 0xff
} AudDmaChn_e;

typedef enum
{
    E_AUD_ADC_LINEIN,
    E_AUD_DAC_LINEOUT,
    E_AUD_ATOP_NUM,
} AudAtopPath_e;

typedef enum
{
    E_AUD_ADC_SEL_LINEIN,
    E_AUD_ADC_SEL_MICIN,
    E_AUD_ADC_SEL_MICIN2,
    E_AUD_ADC_SEL_MICIN_4CH,
    E_AUD_ADC_SEL_NUM,
    E_AUD_ADC_SEL_NULL = 0xFF,
} AudAdcSel_e;

/**
 * \brief Audio Synthesizer
 */
typedef struct
{
    AudI2sMode_e   enI2sMode;
    AudI2sMsMode_e eMsMode;
    AudI2sFmt_e    eFormat;
    AudBitWidth_e  enI2sWidth;
    AudI2sMck_e    eMck;
    U16            nChannelNum;
    AudRate_e      eRate;
    AudWireMode_e  eWireMode;
    void*          pPriData;
} AudI2sCfg_t;

typedef enum
{
    E_AUD_I2S_CLK_REF_DMIC,
    E_AUD_I2S_CLK_REF_ADC,
    E_AUD_I2S_CLK_REF_I2S_TDM_RX,
    E_AUD_I2S_CLK_REF_SRC,
    E_AUD_I2S_CLK_REF_NUM,
    E_AUD_I2S_CLK_REF_NULL = 0xff,
} AudI2sClkRef_e;

/**
 * \brief Audio MUX
 */
typedef enum
{
    E_AUD_MUX_MMC1,
    E_AUD_MUX_MMC2,
    E_AUD_MUX_DMAWR1,
    E_AUD_MUX_DMAWR1_MCH,
    E_AUD_MUX_DMAWR2_MCH,
    E_AUD_MUX_DMARD1_TDM_SEL,
    E_AUD_MUX_DMARD2_TDM_SEL,
    E_AUD_MUX_I2STDM_RX,
    E_AUD_MUX_I2S_TX,
    E_AUD_MUX_I2S_CFG_00,
    E_AUD_MUX_I2S_1_CFG_00,
    E_AUD_MUX_SDM_DINL,
    E_AUD_MUX_SDM_DINR,
    E_AUD_MUX_AO_HDMI,
    E_AUD_MUX_TDM1_TX,
    E_AUD_MUX_TDM2_TX,
    E_AUD_MUX_NUM,
    E_AUD_MUX_NULL = 0xff
} AudMux_e;

typedef enum
{
    E_AUD_DPGA_MMC1,
    E_AUD_DPGA_MMC2,
    E_AUD_DPGA_ADC,
    E_AUD_DPGA_AEC1,
    E_AUD_DPGA_DEC1,
    E_AUD_DPGA_DEC2,
    E_AUD_DPGA_TDM,
    E_AUD_DPGA_NUM,
    E_AUD_DPGA_NULL = 0xff,
} AudDpga_e;

typedef enum
{
    E_AUD_PAD_MUX_DMIC,
    E_AUD_PAD_MUX_I2S_TX,
    E_AUD_PAD_MUX_I2S_RX,
    E_AUD_PAD_MUX_I2S_MCK,
    E_AUD_PAD_MUX_I2S_RXTX,
    E_AUD_PAD_MUX_NUM,
    E_AUD_PAD_MUX_NULL = 0xff,
} AudPadMux_e;

typedef enum
{
    E_AUD_MCH_SEL_DMIC01,
    E_AUD_MCH_SEL_DMIC23,
    E_AUD_MCH_SEL_AMIC01,
    E_AUD_MCH_SEL_AMIC23,
    E_AUD_MCH_SEL_I2S_A_RX01,
    E_AUD_MCH_SEL_I2S_A_RX23,
    E_AUD_MCH_SEL_I2S_A_RX45,
    E_AUD_MCH_SEL_I2S_A_RX67,
    E_AUD_MCH_SEL_I2S_B_RX01,
    E_AUD_MCH_SEL_I2S_B_RX23,
    E_AUD_MCH_SEL_I2S_B_RX45,
    E_AUD_MCH_SEL_I2S_B_RX67,
    E_AUD_MCH_SEL_I2S_C_RX01,
    E_AUD_MCH_SEL_I2S_C_RX23,
    E_AUD_MCH_SEL_I2S_C_RX45,
    E_AUD_MCH_SEL_I2S_C_RX67,
    E_AUD_MCH_SEL_I2S_D_RX01,
    E_AUD_MCH_SEL_I2S_D_RX23,
    E_AUD_MCH_SEL_I2S_D_RX45,
    E_AUD_MCH_SEL_I2S_D_RX67,
    E_AUD_MCH_SEL_SRC,
    E_AUD_MCH_SEL_NUM,
    E_AUD_MCH_SEL_NULL = 0xff,
} AudMchSel_e;

typedef enum
{
    E_AUD_MCH_CH_BIND_01,
    E_AUD_MCH_CH_BIND_23,
    E_AUD_MCH_CH_BIND_45,
    E_AUD_MCH_CH_BIND_67,
    E_AUD_MCH_CH_BIND_NUM,
} AudMchChBind_e;

typedef enum
{
    E_AUD_MCH_CLK_REF_DMIC,
    E_AUD_MCH_CLK_REF_ADC,
    E_AUD_MCH_CLK_REF_I2S_TDM_RX,
    E_AUD_MCH_CLK_REF_SRC,
    E_AUD_MCH_CLK_REF_NUM,
    E_AUD_MCH_CLK_REF_NONE,

} AudMchClkRef_e;

typedef enum
{
    E_AUD_AO_CLK_REF_A,
    E_AUD_AO_CLK_REF_I2S_TDM_TX_A,
    E_AUD_AO_CLK_REF_I2S_TDM_TX_B,
    E_AUD_AO_CLK_REF_NUM,
    E_AUD_AO_CLK_REF_NONE,

} AudAoClkRef_e;

typedef enum
{
    E_AUD_AO_CH_MODE_STEREO = 0,
    E_AUD_AO_CH_MODE_DOUBLE_MONO,
    E_AUD_AO_CH_MODE_DOUBLE_LEFT,
    E_AUD_AO_CH_MODE_DOUBLE_RIGHT,
    E_AUD_AO_CH_MODE_EXCHANGE,
    E_AUD_AO_CH_MODE_ONLY_LEFT,
    E_AUD_AO_CH_MODE_ONLY_RIGHT,

} AudAoChMode_e;

#endif //__HAL_AUD_TYPES_H__
