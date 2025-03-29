/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _MI_AIO_DEP_DATATYPE_H_
#define _MI_AIO_DEP_DATATYPE_H_

#include "mi_sys_datatype.h"

//=============================================================================
// Include files
//=============================================================================

//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#define MI_AUDIO_PASSTHROUGH_DEV_1 (8) /* Passthrough(AI -> AO)  */
#define MI_AUDIO_PASSTHROUGH_DEV_2 (9) /* Passthrough(AI -> AO)  */

//=============================================================================
// Data type definition
//=============================================================================
typedef enum
{
    E_MI_AUDIO_FORMAT_INVALID    = -1,
    E_MI_AUDIO_FORMAT_PCM_S16_LE = 0, /* Signed 16-bit, little endian */
    // E_MI_AUDIO_FORMAT_PCM_S24_LE,             /* Signed, 24-bit (32-bit in memory), little endian */
    // E_MI_AUDIO_FORMAT_PCM_S24_3LE,            /* Signed, 24-bit, little endian */
    // E_MI_AUDIO_FORMAT_PCM_S32_LE,             /* Signed, 32-bit, little endian */
    // E_MI_AUDIO_FORMAT_PCM_S8,                 /* Signed, 8-bit */
    // E_MI_AUDIO_FORMAT_PCM_S16_BE,             /* Signed, 16-bit, big endian */
    // E_MI_AUDIO_FORMAT_PCM_S24_BE,             /* Signed, 24-bit (32-bit in memory), big endian */
    // E_MI_AUDIO_FORMAT_PCM_S24_3BE,            /* Signed, 24-bit, big endian */
    // E_MI_AUDIO_FORMAT_PCM_S32_BE,             /* Signed, 32-bit, big endian */
    // E_MI_AUDIO_FORMAT_PCM_FLOAT_LE,           /* 32-bit float, little endian */
    // E_MI_AUDIO_FORMAT_PCM_FLOAT_BE,           /* 32-bit float, big endian */
    E_MI_AUDIO_FORMAT_MAX,
} MI_AUDIO_Format_e;

typedef enum
{
    E_MI_AUDIO_SAMPLE_RATE_8000   = 8000,   /* 8kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_11025  = 11025,  /* 11.025kHz sampling rate, AO only */
    E_MI_AUDIO_SAMPLE_RATE_12000  = 12000,  /* 12kHz sampling rate, AO only */
    E_MI_AUDIO_SAMPLE_RATE_16000  = 16000,  /* 16kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_22050  = 22050,  /* 22.05kHz sampling rate, AO only */
    E_MI_AUDIO_SAMPLE_RATE_24000  = 24000,  /* 24kHz sampling rate, AO only */
    E_MI_AUDIO_SAMPLE_RATE_32000  = 32000,  /* 32kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_44100  = 44100,  /* 44.1kHz sampling rate, AO only */
    E_MI_AUDIO_SAMPLE_RATE_48000  = 48000,  /* 48kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_96000  = 96000,  /* 96kHz sampling rate */
    E_MI_AUDIO_SAMPLE_RATE_192000 = 192000, /* 192kHz sampling rate, AO only */
} MI_AUDIO_SampleRate_e;

typedef enum
{
    E_MI_AUDIO_SOUND_MODE_MONO   = 1, /* mono */
    E_MI_AUDIO_SOUND_MODE_STEREO = 2, /* stereo */
    E_MI_AUDIO_SOUND_MODE_4CH    = 4, /* 4 channels data, AI only */
    E_MI_AUDIO_SOUND_MODE_6CH    = 6, /* 6 channels data, AI only */
    E_MI_AUDIO_SOUND_MODE_8CH    = 8, /* 8 channels data, AI only */
    E_MI_AUDIO_SOUND_MODE_10CH   = 10, /* 10 channels data, AI only */
    E_MI_AUDIO_SOUND_MODE_12CH   = 12, /* 12 channels data, AI only */
    E_MI_AUDIO_SOUND_MODE_14CH   = 14, /* 14 channels data, AI only */
    E_MI_AUDIO_SOUND_MODE_16CH   = 16, /* 16 channels data, AI only */
} MI_AUDIO_SoundMode_e;

typedef enum
{
    E_MI_AUDIO_I2S_MODE_I2S_MASTER, /* I2S master mode */
    E_MI_AUDIO_I2S_MODE_I2S_SLAVE,  /* I2S slave mode */
    E_MI_AUDIO_I2S_MODE_TDM_MASTER, /* TDM master mode */
    E_MI_AUDIO_I2S_MODE_TDM_SLAVE,  /* TDM slave mode */
} MI_AUDIO_I2sMode_e;

typedef enum
{
    E_MI_AUDIO_I2S_FMT_I2S_MSB,
    E_MI_AUDIO_I2S_FMT_LEFT_JUSTIFY_MSB,
} MI_AUDIO_I2sFormat_e;

typedef enum
{
    E_MI_AUDIO_I2S_MCLK_0,       /* not enable */
    E_MI_AUDIO_I2S_MCLK_12_288M, /* 12.288M */
    E_MI_AUDIO_I2S_MCLK_16_384M, /* 16.384M */
    E_MI_AUDIO_I2S_MCLK_18_432M, /* 18.432M */
    E_MI_AUDIO_I2S_MCLK_24_576M, /* 24.576M */
    E_MI_AUDIO_I2S_MCLK_24M,     /* 24MHz */
    E_MI_AUDIO_I2S_MCLK_48M,     /* 48MHz */
    E_MI_AUDIO_I2S_MCLK_22_5792M,/* 22.5792MHz */
    E_MI_AUDIO_I2S_MCLK_32_768M, /* 32.768MHz */
    E_MI_AUDIO_I2S_MCLK_36_864M, /* 36.864MHz */
    E_MI_AUDIO_I2S_MCLK_49_152M, /* 49.152MHz */
    E_MI_AUDIO_I2S_MCLK_76_8M,   /* 76.8MHz */
} MI_AUDIO_I2sMclk_e;

typedef enum
{
    E_MI_AUDIO_BIT_WIDTH_16, /* 16bit width */
    E_MI_AUDIO_BIT_WIDTH_32, /* 32bit width */
} MI_AUDIO_I2sBitWidth_e;

typedef struct MI_AUDIO_I2sConfig_s
{
    MI_AUDIO_I2sMode_e     enMode;
    MI_AUDIO_I2sBitWidth_e enBitWidth;
    MI_AUDIO_I2sFormat_e   enFormat;
    MI_AUDIO_SampleRate_e  enSampleRate;
    MI_AUDIO_I2sMclk_e     enMclk;
    MI_BOOL                bSyncClock; /* I2s Tx BCLK, Rx BCLK use the same clock source */
    MI_U32                 u32TdmSlots;
} MI_AUDIO_I2sConfig_t;

//=============================================================================
// Variable definition
//=============================================================================

//=============================================================================
// Global function definition
//=============================================================================

#endif // _MI_AIO_DEP_DATATYPE_H_
