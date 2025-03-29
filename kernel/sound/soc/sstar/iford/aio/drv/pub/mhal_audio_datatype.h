/*
 * mhal_audio_datatype.h - Sigmastar
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

#ifndef __MHAL_AUDIO_DATATYPE_H__

#define __MHAL_AUDIO_DATATYPE_H__
#include "mhal_common.h"
#include "cam_os_wrapper.h"

// -------------------------------------------------------------------------------
typedef enum
{
    E_MHAL_AUDIO_BITWIDTH_16,
    E_MHAL_AUDIO_BITWIDTH_24,
    E_MHAL_AUDIO_BITWIDTH_32,

} MHAL_AUDIO_BitWidth_e;

typedef enum
{
    E_MHAL_AUDIO_CHN_0  = 0,
    E_MHAL_AUDIO_CHN_1  = 1,
    E_MHAL_AUDIO_CHN_2  = 2,
    E_MHAL_AUDIO_CHN_4  = 4,
    E_MHAL_AUDIO_CHN_8  = 8,
    E_MHAL_AUDIO_CHN_16 = 16,
} MHAL_AUDIO_Channel_e;

typedef enum
{
    E_MHAL_AUDIO_I2S_FMT_I2S = 1,
    E_MHAL_AUDIO_I2S_FMT_LEFT_JUSTIFY,

} MHAL_AUDIO_I2sFmt_e;

typedef enum
{
    E_MHAL_AUDIO_MODE_TDM_MASTER = 1,
    E_MHAL_AUDIO_MODE_TDM_SLAVE  = 2,
} MHAL_AUDIO_I2sMode_e;

typedef enum
{
    E_MHAL_AUDIO_MCK_NULL,    // Disable
    E_MHAL_AUDIO_MCK_12_288M, // 12.288MHz
    E_MHAL_AUDIO_MCK_16_384M, // 16.384MHz
    E_MHAL_AUDIO_MCK_18_432M, // 18.432MHz
    E_MHAL_AUDIO_MCK_24_576M, // 24.576MHz
    E_MHAL_AUDIO_MCK_24M,     // 24MHz
    E_MHAL_AUDIO_MCK_48M,     // 48MHz

} MHAL_AUDIO_I2sMck_e;

typedef enum
{
    E_MHAL_AUDIO_4WIRE_ON = 1,
    E_MHAL_AUDIO_4WIRE_OFF,

} MHAL_AUDIO_I2s4WireMode_e;

typedef enum
{
    E_MHAL_AUDIO_GAIN_FADING_0, // OFF
    E_MHAL_AUDIO_GAIN_FADING_1, // 1 samples
    E_MHAL_AUDIO_GAIN_FADING_2, // 2 samples
    E_MHAL_AUDIO_GAIN_FADING_3, // 4 samples
    E_MHAL_AUDIO_GAIN_FADING_4, // 8 samples
    E_MHAL_AUDIO_GAIN_FADING_5, // 16 samples
    E_MHAL_AUDIO_GAIN_FADING_6, // 32 samples
    E_MHAL_AUDIO_GAIN_FADING_7, // 64 samples
    E_MHAL_AUDIO_GAIN_FADING_NUM,

} MHAL_AUDIO_GainFading_e;

typedef enum
{
    E_MHAL_AO_MIXER_A,
    E_MHAL_AO_MIXER_B,
    E_MHAL_AO_MIXER_TOTAL,
} MHAL_AO_Mixer_e;
// -------------------------------------------------------------------------------
typedef struct MHAL_AUDIO_CommonCfg_s
{
    MS_U32 u32Rate;
    MS_U32 u32DmaRate;
} MHAL_AUDIO_CommonCfg_t;

typedef struct MHAL_AUDIO_I2sCfg_s
{
    MHAL_AUDIO_CommonCfg_t    commonCfg;
    MHAL_AUDIO_I2sMode_e      enMode;
    MS_U16                    u16Width;
    MHAL_AUDIO_I2sFmt_e       enFmt;
    MHAL_AUDIO_I2sMck_e       enMck;
    MHAL_AUDIO_I2s4WireMode_e en4WireMode;
    MS_U16                    u16Channels;
} MHAL_AUDIO_I2sCfg_t;

typedef struct MHAL_AUDIO_PcmCfg_s
{
    MS_U32           u32Rate;
    MS_U16           u16Width;
    MS_U16           u16Channels;
    MS_BOOL          bInterleaved;
    void*            pu8DmaArea;    // DMA addr
    ss_miu_addr_t    phyDmaAddr;    // DMA phy addr
    MS_U32           u32BufferSize; // Buffer size should be nPeriodSize * nPeriodCount
    MS_U32           u32PeriodSize;
    MS_BOOL          u8IsOnlyEvenCh;
    MHAL_AO_ChMode_e enChMode;
    MS_U32           u32PeriodCnt;
} MHAL_AUDIO_PcmCfg_t;

struct sstar_interupt_status
{
    MS_U32 trigger_int;    // AI&AO period int
    MS_U32 boundary_int;   // AI&AO full&empty int
    MS_U32 local_data_int; // AI&AO local full & empty int
    MS_U32 hdmirx_headdone_int;
    MS_U32 frequency_offset_int;
};

struct sstar_interupt_flag
{
    MS_U32 trigger_flag;
    MS_U32 boundary_flag;
    MS_U32 transmit_flag;
    MS_U32 local_data_flag;
    MS_U32 hdmirx_headdone_flag;
    MS_U32 hdmirx_mute_flag;
    MS_U32 hdmirx_unmute_flag;
    MS_U32 frequency_offset_flag;
};

struct sstar_interupt_en
{
    MS_U32 trigger_en;  // AI&AO period int
    MS_U32 boundary_en; // AI&AO full&empty int
    MS_U32 transmit_en; // AI&AO transmit int
    MS_U32 hdmirx_headdone_en;
    MS_U32 frequency_offset_en;
};

typedef struct sstar_hdmi_audio_format_s
{
    MS_U32 islpcm;
    MS_U32 samplerates;
} hdmi_audio_format;

#endif //__MHAL_AUDIO_DATATYPE_H__
