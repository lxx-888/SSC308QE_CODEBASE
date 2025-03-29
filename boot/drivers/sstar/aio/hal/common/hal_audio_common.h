/*
 * hal_audio_common.h - Sigmastar
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

#ifndef __HAL_AUD_COMMON_H__
#define __HAL_AUD_COMMON_H__

//=============================================================================
// Include files
//=============================================================================
#include "cam_os_wrapper.h"
#include "mhal_audio_common.h"

// errno
#define EPERM     1
#define EIO       5
#define EAGAIN    11
#define ENOMEM    12
#define EFAULT    14
#define EINVAL    22
#define ENOTTY    25
#define EPIPE     32
#define ETIME     62 // Timer expired
#define EOVERFLOW 75
#define EBADFD    77

//
#if !defined(U8)
typedef unsigned char      U8;
typedef signed char        S8;
typedef unsigned short     U16;
typedef short              S16;
typedef unsigned int       U32;
typedef int                S32;
typedef unsigned long long U64;
#endif

//
#if !defined(BOOL)
typedef unsigned int BOOL;
#endif

//
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define HAL_DEBUG_ON 0

#define ERRMSG(fmt, args...) printf(fmt, ##args)

#if HAL_DEBUG_ON
#define DBGMSG(fmt, args...) printf(fmt, ##args)
#else
#define DBGMSG(fmt, args...)
#endif

// -------------------------------------------------------------------------------
#define AIO_OK (0)
#define AIO_NG (-1)

// -------------------------------------------------------------------------------
typedef enum
{
    E_AI_CH_ADC_A_0,
    E_AI_CH_ADC_B_0,
    E_AI_CH_ADC_C_0,
    E_AI_CH_ADC_D_0,
    E_AI_CH_DMIC_A_0,
    E_AI_CH_DMIC_A_1,
    E_AI_CH_DMIC_A_2,
    E_AI_CH_DMIC_A_3,
    E_AI_CH_HDMI_A_0,
    E_AI_CH_HDMI_A_1,
    E_AI_CH_ECHO_A_0,
    E_AI_CH_ECHO_A_1,
    E_AI_CH_I2S_RX_A_0,
    E_AI_CH_I2S_RX_A_1,
    E_AI_CH_I2S_RX_A_2,
    E_AI_CH_I2S_RX_A_3,
    E_AI_CH_I2S_RX_A_4,
    E_AI_CH_I2S_RX_A_5,
    E_AI_CH_I2S_RX_A_6,
    E_AI_CH_I2S_RX_A_7,
    E_AI_CH_I2S_RX_A_8,
    E_AI_CH_I2S_RX_A_9,
    E_AI_CH_I2S_RX_A_10,
    E_AI_CH_I2S_RX_A_11,
    E_AI_CH_I2S_RX_A_12,
    E_AI_CH_I2S_RX_A_13,
    E_AI_CH_I2S_RX_A_14,
    E_AI_CH_I2S_RX_A_15,
    E_AI_CH_I2S_RX_B_0,
    E_AI_CH_I2S_RX_B_1,
    E_AI_CH_I2S_RX_B_2,
    E_AI_CH_I2S_RX_B_3,
    E_AI_CH_I2S_RX_B_4,
    E_AI_CH_I2S_RX_B_5,
    E_AI_CH_I2S_RX_B_6,
    E_AI_CH_I2S_RX_B_7,
    E_AI_CH_I2S_RX_B_8,
    E_AI_CH_I2S_RX_B_9,
    E_AI_CH_I2S_RX_B_10,
    E_AI_CH_I2S_RX_B_11,
    E_AI_CH_I2S_RX_B_12,
    E_AI_CH_I2S_RX_B_13,
    E_AI_CH_I2S_RX_B_14,
    E_AI_CH_I2S_RX_B_15,
    E_AI_CH_I2S_RX_C_0,
    E_AI_CH_I2S_RX_C_1,
    E_AI_CH_I2S_RX_C_2,
    E_AI_CH_I2S_RX_C_3,
    E_AI_CH_I2S_RX_C_4,
    E_AI_CH_I2S_RX_C_5,
    E_AI_CH_I2S_RX_C_6,
    E_AI_CH_I2S_RX_C_7,
    E_AI_CH_I2S_RX_C_8,
    E_AI_CH_I2S_RX_C_9,
    E_AI_CH_I2S_RX_C_10,
    E_AI_CH_I2S_RX_C_11,
    E_AI_CH_I2S_RX_C_12,
    E_AI_CH_I2S_RX_C_13,
    E_AI_CH_I2S_RX_C_14,
    E_AI_CH_I2S_RX_C_15,
    E_AI_CH_I2S_RX_D_0,
    E_AI_CH_I2S_RX_D_1,
    E_AI_CH_I2S_RX_D_2,
    E_AI_CH_I2S_RX_D_3,
    E_AI_CH_I2S_RX_D_4,
    E_AI_CH_I2S_RX_D_5,
    E_AI_CH_I2S_RX_D_6,
    E_AI_CH_I2S_RX_D_7,
    E_AI_CH_I2S_RX_D_8,
    E_AI_CH_I2S_RX_D_9,
    E_AI_CH_I2S_RX_D_10,
    E_AI_CH_I2S_RX_D_11,
    E_AI_CH_I2S_RX_D_12,
    E_AI_CH_I2S_RX_D_13,
    E_AI_CH_I2S_RX_D_14,
    E_AI_CH_I2S_RX_D_15,
    E_AI_CH_TOTAL,

} AI_CH_e;

typedef enum
{
    E_AO_CH_DAC_A_0,
    E_AO_CH_DAC_B_0,
    E_AO_CH_DAC_C_0,
    E_AO_CH_DAC_D_0,
    E_AO_CH_HDMI_A_0,
    E_AO_CH_HDMI_A_1,
    E_AO_CH_ECHO_A_0,
    E_AO_CH_ECHO_A_1,
    E_AO_CH_I2S_TX_A_0,
    E_AO_CH_I2S_TX_A_1,
    E_AO_CH_I2S_TX_B_0,
    E_AO_CH_I2S_TX_B_1,
    E_AO_CH_TOTAL,

} AO_CH_e;

typedef enum
{
    E_AI_DEV_ADC_START,
    E_AI_DEV_ADC_A = E_AI_DEV_ADC_START,
    E_AI_DEV_ADC_B,
    E_AI_DEV_ADC_C,
    E_AI_DEV_ADC_D,
    E_AI_DEV_ADC_END = E_AI_DEV_ADC_D,
    E_AI_DEV_HDMI_START,
    E_AI_DEV_HDMI_A   = E_AI_DEV_HDMI_START,
    E_AI_DEV_HDMI_END = E_AI_DEV_HDMI_A,
    E_AI_DEV_DMIC_START,
    E_AI_DEV_DMIC_A   = E_AI_DEV_DMIC_START,
    E_AI_DEV_DMIC_END = E_AI_DEV_DMIC_A,
    E_AI_DEV_ECHO_START,
    E_AI_DEV_ECHO_A   = E_AI_DEV_ECHO_START,
    E_AI_DEV_ECHO_END = E_AI_DEV_ECHO_A,
    E_AI_DEV_I2S_RX_START,
    E_AI_DEV_I2S_RX_A = E_AI_DEV_I2S_RX_START,
    E_AI_DEV_I2S_RX_B,
    E_AI_DEV_I2S_RX_C,
    E_AI_DEV_I2S_RX_D,
    E_AI_DEV_I2S_RX_END = E_AI_DEV_I2S_RX_D,
    E_AI_DEV_TOTAL,

} AI_DEV_e;

typedef enum
{
    E_AO_DEV_DAC_START,
    E_AO_DEV_DAC_A = E_AO_DEV_DAC_START,
    E_AO_DEV_DAC_B,
    E_AO_DEV_DAC_C,
    E_AO_DEV_DAC_D,
    E_AO_DEV_DAC_END = E_AO_DEV_DAC_D,
    E_AO_DEV_HDMI_START,
    E_AO_DEV_HDMI_A   = E_AO_DEV_HDMI_START,
    E_AO_DEV_HDMI_END = E_AO_DEV_HDMI_A,
    E_AO_DEV_ECHO_START,
    E_AO_DEV_ECHO_A   = E_AO_DEV_ECHO_START,
    E_AO_DEV_ECHO_END = E_AO_DEV_ECHO_A,
    E_AO_DEV_I2S_TX_START,
    E_AO_DEV_I2S_TX_A = E_AO_DEV_I2S_TX_START,
    E_AO_DEV_I2S_TX_B,
    E_AO_DEV_I2S_TX_END = E_AO_DEV_I2S_TX_B,
    E_AO_DEV_TOTAL,

} AO_DEV_e;

typedef enum
{
    E_AUDIO_TDM_START = 0,
    E_AUDIO_TDM_RXA   = E_AUDIO_TDM_START,
    E_AUDIO_TDM_RXB,
    E_AUDIO_TDM_RXC,
    E_AUDIO_TDM_RXD,
    E_AUDIO_TDM_RX_END = E_AUDIO_TDM_RXD,
    E_AUDIO_TDM_TXA,
    E_AUDIO_TDM_TXB,
    E_AUDIO_TDM_END = E_AUDIO_TDM_TXB,
    E_AUDIO_TDM_TOTAL,

} AUDIO_TDM_e;

// -------------------------------------------------------------------------------
#define AI_DMA_e   MHAL_AI_Dma_e
#define AO_DMA_e   MHAL_AO_Dma_e
#define AI_IF_e    MHAL_AI_IF_e
#define AO_IF_e    MHAL_AO_IF_e
#define SINE_GEN_e MHAL_SineGen_e

// -------------------------------------------------------------------------------
#define AI_DMA_CH_NUM_PER_SLOT (2)

#define AO_DMA_CH_NUM_PER_SLOT (1)

// AI DEV
#define AI_DEV_ADC_IDX(x)    (x - E_AI_DEV_ADC_START)
#define AI_DEV_HDMI_IDX(x)   (x - E_AI_DEV_HDMI_START)
#define AI_DEV_DMIC_IDX(x)   (x - E_AI_DEV_DMIC_START)
#define AI_DEV_ECHO_IDX(x)   (x - E_AI_DEV_ECHO_START)
#define AI_DEV_I2S_RX_IDX(x) (x - E_AI_DEV_I2S_RX_START)

#define AI_DEV_IS_ADC(x)    ((E_AI_DEV_ADC_START <= x) && (x <= E_AI_DEV_ADC_END))
#define AI_DEV_IS_HDMI(x)   ((E_AI_DEV_HDMI_START <= x) && (x <= E_AI_DEV_HDMI_END))
#define AI_DEV_IS_DMIC(x)   ((E_AI_DEV_DMIC_START <= x) && (x <= E_AI_DEV_DMIC_END))
#define AI_DEV_IS_ECHO(x)   ((E_AI_DEV_ECHO_START <= x) && (x <= E_AI_DEV_ECHO_END))
#define AI_DEV_IS_I2S_RX(x) ((E_AI_DEV_I2S_RX_START <= x) && (x <= E_AI_DEV_I2S_RX_END))

#define AI_DEV_ADC_TOTAL    (E_AI_DEV_ADC_END - E_AI_DEV_ADC_START + 1)
#define AI_DEV_HDMI_TOTAL   (E_AI_DEV_HDMI_END - E_AI_DEV_HDMI_START + 1)
#define AI_DEV_DMIC_TOTAL   (E_AI_DEV_DMIC_END - E_AI_DEV_DMIC_START + 1)
#define AI_DEV_ECHO_TOTAL   (E_AI_DEV_ECHO_END - E_AI_DEV_ECHO_START + 1)
#define AI_DEV_I2S_RX_TOTAL (E_AI_DEV_I2S_RX_END - E_AI_DEV_I2S_RX_START + 1)

// AO DEV
#define AO_DEV_DAC_IDX(x)    (x - E_AO_DEV_DAC_START)
#define AO_DEV_HDMI_IDX(x)   (x - E_AO_DEV_HDMI_START)
#define AO_DEV_ECHO_IDX(x)   (x - E_AO_DEV_ECHO_START)
#define AO_DEV_I2S_TX_IDX(x) (x - E_AO_DEV_I2S_TX_START)

#define AO_DEV_IS_DAC(x)    ((E_AO_DEV_DAC_START <= x) && (x <= E_AO_DEV_DAC_END))
#define AO_DEV_IS_HDMI(x)   ((E_AO_DEV_HDMI_START <= x) && (x <= E_AO_DEV_HDMI_END))
#define AO_DEV_IS_ECHO(x)   ((E_AO_DEV_ECHO_START <= x) && (x <= E_AO_DEV_ECHO_END))
#define AO_DEV_IS_I2S_TX(x) ((E_AO_DEV_I2S_TX_START <= x) && (x <= E_AO_DEV_I2S_TX_END))

#define AO_DEV_DAC_TOTAL    (E_AO_DEV_DAC_END - E_AO_DEV_DAC_START + 1)
#define AO_DEV_HDMI_TOTAL   (E_AO_DEV_HDMI_END - E_AO_DEV_HDMI_START + 1)
#define AO_DEV_ECHO_TOTAL   (E_AO_DEV_ECHO_END - E_AO_DEV_ECHO_START + 1)
#define AO_DEV_I2S_TX_TOTAL (E_AO_DEV_I2S_TX_END - E_AO_DEV_I2S_TX_START + 1)

typedef struct
{
    AI_DMA_e enAiDma;
    AI_IF_e  eAiIf[E_MHAL_AI_DMA_CH_SLOT_TOTAL];

} AI_ATTACH_t;

typedef struct
{
    AO_DMA_e enAoDma;
    AO_IF_e  eAoIf[E_MHAL_AO_DMA_CH_SLOT_TOTAL];

} AO_ATTACH_t;

#endif //__HAL_AUD_COMMON_H__
