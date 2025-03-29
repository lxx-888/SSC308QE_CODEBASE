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
    E_CHIP_AI_SPDIF_A,
    E_CHIP_AI_SPDIF_TOTAL,
    E_CHIP_AI_SPDIF_NONE,

} CHIP_AI_SPDIF_e;

typedef enum
{
    E_CHIP_AI_SRC_A,
    E_CHIP_AI_SRC_TOTAL,
    E_CHIP_AI_SRC_NONE,

} CHIP_AI_SRC_e;

// -------------------------------------------------------------------------------

// AO
#define CHIP_AO_DAC_IDX_BY_DEV(x)    (AO_DEV_DAC_IDX(x))
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
    E_CHIP_AI_HDMI_A,
    E_CHIP_AI_HDMI_TOTAL,
    E_CHIP_AI_HDMI_NONE,

} CHIP_AI_HDMI_e;

typedef enum
{
    E_CHIP_AO_SRC_A,
    E_CHIP_AO_SRC_TOTAL,
    E_CHIP_AO_SRC_NONE,

} CHIP_AO_SRC_e;

// -------------------------------------------------------------------------------

// AIO
#define CHIP_AIO_DMA_IDX_BY_AI_DMA(x) ((x) + 0)
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

typedef enum
{
    E_CHIP_WDMA1_TRANSLATE = 1,
    E_CHIP_WDMA2_TRANSLATE,
    E_CHIP_RDMA1_TRANSLATE,
    E_CHIP_RDMA2_TRANSLATE,
    E_CHIP_RDMA3_TRANSLATE,
} CHIP_IRQ_OBJECT_e;

typedef enum
{
    E_CHIP_BACKUP_READ,
    E_CHIP_BACKUP_WRITE,
} CHIP_AIO_BACKUP_e;

typedef enum
{
    E_CHIPL_SWITCH_READ,
    E_CHIP_SWITCH_WRITE,
    E_CHIP_SWITCH_SPDIF_TX,
    E_CHIP_SWITCH_HDMI_ARC,
    E_CHIP_SWITCH_BOTH,
} CHIP_SWITCH_e;

typedef enum
{
    E_CHIP_AO_DMA_START,
    E_CHIP_AO_DMA_AO_A_L,
    E_CHIP_AO_DMA_AO_A_R,
    E_CHIP_AO_DMA_AO_B_L,
    E_CHIP_AO_DMA_AO_B_R,
    E_CHIP_AO_DMA_AO_C_L,
    E_CHIP_AO_DMA_AO_C_R,
    E_CHIP_AO_DMA_DIRECT_A_L,
    E_CHIP_AO_DMA_DIRECT_A_R,
    E_CHIP_AO_DMA_DIRECT_B_L,
    E_CHIP_AO_DMA_DIRECT_B_R,
    E_CHIP_AO_DMA_END = E_CHIP_AO_DMA_DIRECT_B_R,
    E_CHIP_AO_DMA_TOTAL,
} CHIP_AO_DMA_e;

typedef enum
{
    E_CLK_SRC_48M = 0,
    E_CLK_SRC_384M,
    E_CLK_SRC_432M,
} CLK_SRC_e;

#define CHIP_AIO_I2S_IDX_BY_AI_DEV_I2S_RX(x) (AI_DEV_I2S_RX_IDX(x))
#define CHIP_AIO_I2S_IDX_BY_AO_DEV_I2S_TX(x) \
    (AO_DEV_I2S_TX_IDX(x) + (E_CHIP_AIO_I2S_RX_END - E_CHIP_AIO_I2S_RX_START + 1))
#define CHIP_AIO_I2S_IDX_VALID(x) ((unsigned int)x < E_CHIP_AIO_I2S_TOTAL)

typedef enum
{
    E_CHIP_AIO_I2S_RX_START,
    E_CHIP_AIO_I2S_RX_A   = E_CHIP_AIO_I2S_RX_START,
    E_CHIP_AIO_I2S_RX_END = E_CHIP_AIO_I2S_RX_A,
    E_CHIP_AIO_I2S_TX_START,
    E_CHIP_AIO_I2S_TX_A   = E_CHIP_AIO_I2S_TX_START,
    E_CHIP_AIO_I2S_TX_END = E_CHIP_AIO_I2S_TX_A,
    E_CHIP_AIO_I2S_TOTAL,
    E_CHIP_AIO_I2S_NONE,

} CHIP_AIO_I2S_e;

typedef enum
{
    E_CHIP_AIO_SPDIF_TX_START,
    E_CHIP_AIO_SPDIF_TX_A   = E_CHIP_AIO_SPDIF_TX_START,
    E_CHIP_AIO_SPDIF_TX_END = E_CHIP_AIO_SPDIF_TX_A,
    E_CHIP_AIO_SPDIF_TOTAL,
    E_CHIP_AIO_SPDIF_NONE,

} CHIP_AIO_SPDIF_e;

typedef enum
{
    E_AUD_HPF_ADC1 = 0,
    E_AUD_HPF_DMIC,
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
    E_CHIP_DPGA_A = 1, // E_AUD_DPGA_MMC1 RDMA1
    E_CHIP_DPGA_B,     // E_AUD_DPGA_MMC2 RDMA2
    E_CHIP_DPGA_C,     // E_AUD_DPGA_ADC0 ADC_01
    E_CHIP_DPGA_D,     // E_AUD_DPGA_ADC1 ADC_23
    E_CHIP_DPGA_E,     // E_AUD_DPGA_DEC1 SRC1
    E_CHIP_DPGA_F,     // E_AUD_DPGA_DEC2 SRC2
    E_CHIP_DPGA_G,     // E_AUD_DPGA_TDM0_TX
    E_CHIP_DPGA_H,     // E_AUD_DPGA_TDM1_TX
    E_CHIP_DPGA_I,     // E_AUD_DPGA_HDMI_RX
    E_CHIP_DPGA_J,     // E_AUD_DPGA_DMIC_01
    E_CHIP_DPGA_K,     // E_AUD_DPGA_DMIC_23
    E_CHIP_DPGA_L,     // E_AUD_DPGA_DMIC_45
    E_CHIP_DPGA_M,     // E_AUD_DPGA_DMIC_67
    E_CHIP_DPGA_N,     // E_AUD_DPGA_TDMO_RX_01
    E_CHIP_DPGA_O,     // E_AUD_DPGA_TDMO_RX_23
    E_CHIP_DPGA_P,     // E_AUD_DPGA_TDMO_RX_45
    E_CHIP_DPGA_Q,     // E_AUD_DPGA_TDMO_RX_67
    E_CHIP_DPGA_R,     // E_AUD_DPGA_TDM0_RX_89
    E_CHIP_DPGA_S,     // E_AUD_DPGA_TDM0_RX_AB
    E_CHIP_DPGA_T,     // E_AUD_DPGA_TDM0_RX_CD
    E_CHIP_DPGA_U,     // E_AUD_DPGA_TDM0_RX_EF
    E_CHIP_DPGA_V,     // E_AUD_DPGA_SPDIF_TX
    E_CHIP_DPGA_W,     // E_AUD_DPGA_TDM1_RX_01
    E_CHIP_DPGA_X,     // E_AUD_DPGA_1_MMC1_DAC
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
    E_AUD_RATE_88K,
    E_AUD_RATE_96K,
    E_AUD_RATE_192K,
    E_AUD_RATE_NUM,
    E_AUD_RATE_NULL = 0xff,

} AudRate_e;

typedef enum
{
    E_AUD_NONE,
    E_AUD_BYPASS_TO_SPDIF,
    E_AUD_16BIT_DMA,
    E_AUD_16BIT_DMA_BYPASS_DPGA,
} AudHdmiRxSel_e;

typedef enum
{
    E_AUD_BITWIDTH_16,
    E_AUD_BITWIDTH_24,
    E_AUD_BITWIDTH_32,
    E_AUD_BITWIDTH_NULL = 0xff
} AudBitWidth_e;

typedef enum
{
    E_AUD_CHANNEL_NUM_1  = 1,
    E_AUD_CHANNEL_NUM_2  = 2,
    E_AUD_CHANNEL_NUM_4  = 4,
    E_AUD_CHANNEL_NUM_6  = 6,
    E_AUD_CHANNEL_NUM_8  = 8,
    E_AUD_CHANNEL_NUM_16 = 16,
} AudChannelNum_e;

typedef enum
{
    E_AUD_SRC_NULL,
    E_AUD_SRC1,
    E_AUD_SRC_NUM,
} SrcFrom_e;

typedef enum
{
    E_AUD_MIXER_NULL,
    E_AUD_MIXER_1,
    E_AUD_MIXER_2,
    E_AUD_MIXER_NUM = E_AUD_MIXER_2,
} SrcTo_e;

typedef struct
{
    int       nDmaSampleRate;
    bool      nUseSrc;
    bool      nConnected;
    SrcFrom_e nSrcFro;
    SrcTo_e   nSrcTo;
} PathCfg_t;

typedef struct
{
    int       nIfSampleRate;
    PathCfg_t nConnnectDmaA;
    PathCfg_t nConnnectDmaB;
    PathCfg_t nConnnectDmaC;
    PathCfg_t nConnnectVirDma;
} AudAoIfCfg_t;

typedef struct
{
    volatile bool used;
    AudRate_e     nSrcInputSampleRate;
} SrcStaTab_t;

typedef struct
{
    volatile bool used;
    CHIP_AO_DMA_e from;
    U8            nDmaCh;
} SrcMuxTab_t;

typedef struct
{
    volatile bool used;
    U32           from;
    AO_CH_e       to;
} SrcMixTab_t;

typedef struct chip_hdmi_audio_format_s
{
    U32 islpcm;
    U32 samplerates;
} HdmiAudFmt;

typedef enum
{
    AUD_AO_SRC1_MUX_L = 0x1,
    AUD_AO_SRC1_MUX_R = 0x2,
    AUD_AO_SRC2_MUX_L = 0x4,
    AUD_AO_SRC2_MUX_R = 0x8,
} AoSrcMux_e;

typedef enum
{
    AUD_AO_SRC1_START_INDEX = 1,
    AUD_AO_SRC1_MUX_INDEX_L = AUD_AO_SRC1_START_INDEX,
    AUD_AO_SRC1_MUX_INDEX_R,
    AUD_AO_SRC1_END_INDEX = AUD_AO_SRC1_MUX_INDEX_R,
    AUD_AO_SRC2_START_INDEX,
    AUD_AO_SRC2_MUX_INDEX_L = AUD_AO_SRC2_START_INDEX,
    AUD_AO_SRC2_MUX_INDEX_R,
    AUD_AO_SRC2_END_INDEX = AUD_AO_SRC2_MUX_INDEX_R,
    AUD_AO_SRC_MAX_MUX,
} AudAoSrcMux_e;

typedef enum
{
    MMC1_L                 = 0,
    MMC1_R                 = 1,
    MMC2_L                 = 2,
    MMC2_R                 = 3,
    MMC1_L_ADD_MMC1_R_DIV2 = 8,
    MMC2_L_ADD_MMC2_R_DIV2 = 0xB,
    MMC1_L_ADD_MMC2_L_DIV2 = 0xC,
    MMC1_R_ADD_MMC2_R_DIV2 = 0xD,
    MMC1_L_ADD_MMC2_R_DIV2 = 0xE,
    MMC1_R_ADD_MMC2_L_DIV2 = 0xF,
    MMC1_L_ADD_MMC1_R      = 0x18,
    MMC2_L_ADD_MMC2_R      = 0x1A,
    MMC1_L_ADD_MMC2_L      = 0x1C,
    MMC1_R_ADD_MMC2_R      = 0x1D,
    MMC1_L_ADD_MMC2_R      = 0x1E,
    MMC1_R_ADD_MMC2_L      = 0x1F,
    MMC_MIXER_MUTE         = 0x10,
} AudSrcMix_e;

typedef enum
{
    AUD_AO_SRC1_START_ID     = 1,
    AUD_AO_SRC1_MIXER_1_ID_L = AUD_AO_SRC1_START_ID,
    AUD_AO_SRC1_MIXER_1_ID_R,
    AUD_AO_SRC1_MIXER_2_ID_L,
    AUD_AO_SRC1_MIXER_2_ID_R,
    AUD_AO_SRC1_END_ID = AUD_AO_SRC1_MIXER_2_ID_R,
    AUD_AO_SRC2_START_ID,
    AUD_AO_SRC2_MIXER_1_ID_L = AUD_AO_SRC2_START_ID,
    AUD_AO_SRC2_MIXER_1_ID_R,
    AUD_AO_SRC2_END_ID = AUD_AO_SRC2_MIXER_1_ID_R,
    AUD_AO_SRC_MAX_MIXER,
} AudAoSrcMix_e;

typedef enum
{
    AUD_AO_SRC_NULL = 0,
    AUD_AO_SRC1_MIXER,
    AUD_AO_SRC2_MIXER,
} AudAoSrc_e;

typedef enum
{
    MUX_SEL_DMA1_L,
    MUX_SEL_DMA1_R,
    MUX_SEL_DMA2_L,
    MUX_SEL_DMA2_R,
    MUX_SEL_DMA3_L,
    MUX_SEL_DMA3_R,
    MUX_SEL_ADC_0 = 0x10,
    MUX_SEL_ADC_1,
    MUX_SEL_ADC_2,
    MUX_SEL_ADC_3,
    MUX_SEL_DMIC_0 = 0x20,
    MUX_SEL_DMIC_1,
    MUX_SEL_DMIC_2,
    MUX_SEL_DMIC_3,
    MUX_SEL_DMIC_4,
    MUX_SEL_DMIC_5,
    MUX_SEL_DMIC_6,
    MUX_SEL_DMIC_7,
    MUX_SEL_I2S0_RX0 = 0x30,
    MUX_SEL_I2S0_RX1,
    MUX_SEL_I2S0_RX2,
    MUX_SEL_I2S0_RX3,
    MUX_SEL_I2S0_RX4,
    MUX_SEL_I2S0_RX5,
    MUX_SEL_I2S0_RX6,
    MUX_SEL_I2S0_RX7,
    MUX_SEL_I2S1_RX0,
    MUX_SEL_I2S1_RX1,
    MUX_SEL_I2S2_RX0,
    MUX_SEL_I2S2_RX1,
    MUX_SEL_ZERO,
} Mmc2DinSel_e;

typedef enum
{
    E_AUD_I2S_MODE_I2S,
    E_AUD_I2S_MODE_TDM,
} AudI2sMode_e;

typedef enum
{
    E_AUD_I2S_MSMODE_MASTER = 1,
    E_AUD_I2S_MSMODE_SLAVE,
} AudI2sMsMode_e;

typedef enum
{
    E_AUD_I2S_FMT_I2S = 1,
    E_AUD_I2S_FMT_LEFT_JUSTIFY
} AudI2sFmt_e;

typedef enum
{
    E_AUD_I2S_MCK_NULL,     // Disable
    E_AUD_I2S_MCK_12_288M,  // 12.288MHz
    E_AUD_I2S_MCK_16_384M,  // 16.384MHz
    E_AUD_I2S_MCK_18_432M,  // 18.432MHz
    E_AUD_I2S_MCK_22_5792M, // 22.5792MHz
    E_AUD_I2S_MCK_24_576M,  // 24.576MHz
    E_AUD_I2S_MCK_32_768M,  // 32.768MHz
    E_AUD_I2S_MCK_36_864M,  // 36.864MHz
    E_AUD_I2S_MCK_49_152M,  // 49.152MHz
    E_AUD_I2S_MCK_76_8M,    // 76.8MHz
    E_AUD_I2S_MCK_24M,      // 24MHz
    E_AUD_I2S_MCK_48M,      // 48MHz
} AudI2sMck_e;

typedef enum
{
    E_AUD_I2S_WIRE_4 = 1,
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
    E_AUD_ADC_LINEIN_A,
    E_AUD_ADC_LINEIN_B,
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
    BOOL           bMckActive;
    int            nChannelNum;
    AudRate_e      eRate;
    AudWireMode_e  eWireMode;
    BOOL           bActive;
    BOOL           bBckActive;
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
    E_AUD_MUX_DMARD2_TDM_SEL,
    E_AUD_MUX_I2STDM_RX,
    E_AUD_MUX_I2STDM1_RX,
    E_AUD_MUX_I2STDM2_RX,
    E_AUD_MUX_I2S_TX,
    E_AUD_MUX_I2S_1_CFG_00,
    E_AUD_MUX_SDM_DINL,
    E_AUD_MUX_SDM_DINR,
    E_AUD_MUX_AO_HDMI,
    E_AUD_MUX_AO_SPDIF_TX,
    E_AUD_MUX_I2S_TDM_TX0_DATA_SEL,
    E_AUD_MUX_I2S_TDM_TX1_DATA_SEL,
    E_AUD_MUX_I2S_TDM_TX2_DATA_SEL,
    E_AUD_MUX_TDM0_TX,
    E_AUD_MUX_TDM1_TX,
    E_AUD_MUX_TDM2_TX,
    E_AUD_MUX_MMC1_DIN_L_SEL,
    E_AUD_MUX_MMC1_DIN_R_SEL,
    E_AUD_MUX_MMC2_DIN_L_SEL,
    E_AUD_MUX_MMC2_DIN_R_SEL,
    E_AUD_MUX_I2S_CFG_00,
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
    E_AUD_MCH_SEL_DMIC45,
    E_AUD_MCH_SEL_DMIC67,
    E_AUD_MCH_SEL_AMIC01,
    E_AUD_MCH_SEL_AMIC23,
    E_AUD_MCH_SEL_I2S_A_RX01,
    E_AUD_MCH_SEL_I2S_A_RX23,
    E_AUD_MCH_SEL_I2S_A_RX45,
    E_AUD_MCH_SEL_I2S_A_RX67,
    E_AUD_MCH_SEL_I2S_B_RX01,
    E_AUD_MCH_SEL_I2S_C_RX01,
    E_AUD_MCH_SEL_SPDIF_RX01,
    E_AUD_MCH_SEL_SRC01,
    E_AUD_MCH_SEL_SRC23,
    E_AUD_MCH_SEL_FIXED_ZERO,
    E_AUD_MCH_SEL_NUM,
    E_AUD_MCH_SEL_NULL = 0xff,
} AudMchSel_e;

typedef enum
{
    E_AUD_MCH_CH_BIND_01,
    E_AUD_MCH_CH_BIND_23,
    E_AUD_MCH_CH_BIND_45,
    E_AUD_MCH_CH_BIND_67,
    E_AUD_MCH_CH_BIND_89,
    E_AUD_MCH_CH_BIND_AB,
    E_AUD_MCH_CH_BIND_CD,
    E_AUD_MCH_CH_BIND_EF,
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
    E_AUD_AO_CLK_REF_B,
    E_AUD_AO_CLK_REF_I2S_TDM_TX_A,
    E_AUD_AO_CLK_REF_I2S_TDM_TX_B,
    E_AUD_AO_CLK_REF_I2S_TDM_TX_C,
    E_AUD_AO_CLK_REF_HDMI_RX,
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
    E_AUD_AO_CH_MODE_DOUBLE_NONE,

} AudAoChMode_e;

typedef enum
{
    E_AUD_AO_CH_SEL_L = 0,
    E_AUD_AO_CH_SEL_R,
    E_AUD_AO_CH_SEL_RL_DIV2,
    E_AUD_AO_CH_SEL_RL,
    E_AUD_AO_CH_MASK_TO_ZERO,

} AudAoDmaSel_e;

/**
 * \brief Audio HDMIRX
 */
typedef struct
{
    U8 nConsumer : 1;
    U8 nLinearPcm : 1;
    U8 nCopyRight : 1;
    U8 nState : 3;
    U8 nChannelStatusMode : 2;
    U8 nCategoryCode : 8;
    U8 nSourceNumber : 4;
    U8 nChannelNumber : 4;
    U8 nSamplingFrequency : 4;
    U8 nClockAccuracy : 2;
    U8 nReserved0 : 2;
    U8 nWordLength : 4;
    U8 nOriginalSamplingFrequency : 4;
    U8 nCgmsA : 2;
    U8 nReserved1 : 6;
} Aud60958ChannelStatus_t;

typedef struct
{
    unsigned int* reg_back_up[6];
    unsigned int  reg_back_up_list_num[6];
} AudBachRegBackUp_t;

typedef struct
{
    U8 nChannelStatusMode : 2;
    U8 nState : 3;
    U8 nCopyRight : 1;
    U8 nLinearPcm : 1;
    U8 nConsumer : 1;
    U8 nCategoryCode : 8;
    U8 nChannelNumber : 4;
    U8 nSourceNumber : 4;
    U8 nReserved0 : 2;
    U8 nClockAccuracy : 2;
    U8 nSamplingFrequency : 4;
    U8 nOriginalSamplingFrequency : 4;
    U8 nWordLength : 4;
    U8 nReserved1 : 6;
    U8 nCgmsA : 2;
} AudSStarChannelStatus_t;

typedef enum
{
    DTS_CONFIG_KEY_START = 0,
    AMP_GPIO             = DTS_CONFIG_KEY_START,
    I2S_DRIVING,
    I2S_MCK0,
    I2SA_RX0_CHANNEL,
    I2SA_RX0_TDM_MODE,
    I2SA_RX0_TDM_FMT,
    I2SA_RX0_TDM_WIREMODE,
    I2SA_RX0_TDM_WS_PGM,
    I2SA_RX0_TDM_WS_WIDTH,
    I2SA_RX0_TDM_WS_INV,
    I2SA_RX0_TDM_BCK_INV,
    I2SA_RX0_TDM_CH_SWAP,
    I2SA_RX0_SHORT_FF_MODE,
    I2SA_TX0_CHANNEL,
    I2SA_TX0_TDM_MODE,
    I2SA_TX0_TDM_FMT,
    I2SA_TX0_TDM_WIREMODE,
    I2SA_TX0_TDM_WS_PGM,
    I2SA_TX0_TDM_WS_WIDTH,
    I2SA_TX0_TDM_WS_INV,
    I2SA_TX0_TDM_BCK_INV,
    I2SA_TX0_TDM_CH_SWAP,
    I2SA_TX0_TDM_ACTIVE_SLOT,
    I2SA_TX0_SHORT_FF_MODE,
    I2SA_TX0_SOUNDBAR_MODE,
    HPF_ADC1_LEVEL,
    HPF_DMIC_LEVEL,
    DMIC_BCK_MODE,
    DMIC_BCK_EXT_MODE,
    ADC_OUT_SEL,
    DEBUG_LEVEL,
    KEEP_DAC_POWER_ON,
    KEEP_ADC_POWER_ON,
    DTS_CONFIG_KEY_TOTAL
} DTS_CONFIG_KEY_e;

typedef struct DTS_CONFIG
{
    const char* name;
    int         arraySize;
    int         defValue;
} DTS_CONFIG_st;

#define DTS_CONFIG(index, key, size, def_value) [index] = {.name = key, .arraySize = size, .defValue = def_value},

#define FOREACH_DTS_CONFIG                                                                              \
    /*         ENUM_ID                    DTS_KEY                       PARAM_COUNT        DEF_VALUE */ \
    DTS_CONFIG(AMP_GPIO, "amp-gpio", 4, 0)                                                              \
    DTS_CONFIG(I2S_DRIVING, "i2s-driving", 1, 0)                                                        \
    DTS_CONFIG(I2S_MCK0, "i2s-mck0", 1, 0)                                                              \
    DTS_CONFIG(I2SA_TX0_TDM_MODE, "i2s-tx0-tdm-mode", 1, 0)                                             \
    DTS_CONFIG(I2SA_RX0_TDM_MODE, "i2s-rx0-tdm-mode", 1, 0)                                             \
    DTS_CONFIG(I2SA_TX0_TDM_WIREMODE, "i2s-tx0-tdm-wiremode", 1, 0)                                     \
    DTS_CONFIG(I2SA_RX0_TDM_WIREMODE, "i2s-rx0-tdm-wiremode", 1, 0)                                     \
    DTS_CONFIG(I2SA_TX0_TDM_FMT, "i2s-tx0-tdm-fmt", 1, 0)                                               \
    DTS_CONFIG(I2SA_RX0_TDM_FMT, "i2s-rx0-tdm-fmt", 1, 0)                                               \
    DTS_CONFIG(I2SA_TX0_TDM_BCK_INV, "i2s-tx0-tdm-bck-inv", 1, 0)                                       \
    DTS_CONFIG(I2SA_RX0_TDM_BCK_INV, "i2s-rx0-tdm-bck-inv", 1, 0)                                       \
    DTS_CONFIG(I2SA_TX0_CHANNEL, "i2s-tx0-channel", 1, 0)                                               \
    DTS_CONFIG(I2SA_RX0_CHANNEL, "i2s-rx0-channel", 1, 0)                                               \
    DTS_CONFIG(I2SA_TX0_TDM_WS_PGM, "i2s-tx0-tdm-ws-pgm", 1, 0)                                         \
    DTS_CONFIG(I2SA_RX0_TDM_WS_PGM, "i2s-rx0-tdm-ws-pgm", 1, 0)                                         \
    DTS_CONFIG(I2SA_TX0_TDM_WS_WIDTH, "i2s-tx0-tdm-ws-width", 1, 0)                                     \
    DTS_CONFIG(I2SA_RX0_TDM_WS_WIDTH, "i2s-rx0-tdm-ws-width", 1, 0)                                     \
    DTS_CONFIG(I2SA_RX0_TDM_WS_INV, "i2s-rx0-tdm-ws-inv", 1, 0)                                         \
    DTS_CONFIG(I2SA_TX0_TDM_CH_SWAP, "i2s-tx0-tdm-ch-swap", 4, 0)                                       \
    DTS_CONFIG(I2SA_RX0_TDM_CH_SWAP, "i2s-rx0-tdm-ch-swap", 4, 0)                                       \
    DTS_CONFIG(I2SA_TX0_SHORT_FF_MODE, "i2s-tx0-short-ff-mode", 1, 0)                                   \
    DTS_CONFIG(I2SA_RX0_SHORT_FF_MODE, "i2s-rx0-short-ff-mode", 1, 0)                                   \
    DTS_CONFIG(I2SA_TX0_TDM_ACTIVE_SLOT, "i2s-tx0-tdm-active-slot", 1, 0)                               \
    DTS_CONFIG(HPF_ADC1_LEVEL, "hpf-adc1-level", 2, 0)                                                  \
    DTS_CONFIG(HPF_DMIC_LEVEL, "hpf-dmic-level", 2, 0)                                                  \
    DTS_CONFIG(DMIC_BCK_MODE, "dmic-bck-mode", 4, 0)                                                    \
    DTS_CONFIG(DMIC_BCK_EXT_MODE, "dmic-bck-ext-mode", 1, 0)                                            \
    DTS_CONFIG(ADC_OUT_SEL, "adc-out-sel", 2, 0)                                                        \
    DTS_CONFIG(KEEP_ADC_POWER_ON, "keep-adc-power-on", 1, 0)                                            \
    DTS_CONFIG(KEEP_DAC_POWER_ON, "keep-dac-power-on", 1, 0)                                            \
    DTS_CONFIG(DEBUG_LEVEL, "debug-level", 1, 0)

typedef struct
{
    U16              bank;
    U16              addr;
    U16              mask;
    U16              shift;
    U16              val;
    DTS_CONFIG_KEY_e key;
} AudReg_t;

#endif //__HAL_AUD_TYPES_H__
