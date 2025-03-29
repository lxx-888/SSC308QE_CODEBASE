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
#ifndef __MI_AO_DATATYPE_H__
#define __MI_AO_DATATYPE_H__

#include "mi_common_datatype.h"
#include "mi_aio_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================
// Include files
//=============================================================================

//=============================================================================
// Extern definition
//=============================================================================

//=============================================================================
// Macro definition
//=============================================================================
#define MI_AO_ERR_INVALID_DEVID (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID))
#define MI_AO_ERR_ILLEGAL_PARAM (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM))
#define MI_AO_ERR_NOT_INIT      (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT))
#define MI_AO_ERR_NULL_PTR      (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR))
#define MI_AO_ERR_NOT_CONFIG    (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG))
#define MI_AO_ERR_NOT_SUPPORT   (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT))
#define MI_AO_ERR_NOT_PERM      (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_PERM))
#define MI_AO_ERR_NOMEM         (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM))
#define MI_AO_ERR_NOBUF         (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOBUF))
#define MI_AO_ERR_BUF_EMPTY     (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_EMPTY))
#define MI_AO_ERR_BUF_FULL      (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_FULL))
#define MI_AO_ERR_SYS_NOTREADY  (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_NOTREADY))
#define MI_AO_ERR_BUSY          (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY))
#define MI_AO_ERR_NOVASPACE     (MI_DEF_ERR(E_MI_MODULE_ID_AO, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOVASPACE))

#define MI_AO_MAX_CHN_NUM (2)

#define MI_AO_DEV_1 (0) /* DMA Reader 1 */
#define MI_AO_DEV_2 (1) /* DMA Reader 2 */

//=============================================================================
// Data type definition
//=============================================================================
typedef enum
{
    E_MI_AO_CHANNEL_MODE_STEREO = 0,   /* Stereo (stereo stream only) */
    E_MI_AO_CHANNEL_MODE_DOUBLE_MONO,  /* Stereo stream should be presented with the left and right audio channels
                                          mixed together and delivered to both channels.  Mono stream should be
                                          duplicated and delivered to both channels. */
    E_MI_AO_CHANNEL_MODE_DOUBLE_LEFT,  /* Stereo stream should be presented with the left audio channel replicated
                                          into the right audio channel. (stereo stream only) */
    E_MI_AO_CHANNEL_MODE_DOUBLE_RIGHT, /* Stereo stream should be  presented with the right audio channel replicated
                                          into the left audio channel.  (stereo stream only) */
    E_MI_AO_CHANNEL_MODE_EXCHANGE,  /* Stereo stream should be presented after being exchanged in the left and right
                                       audio channels. (stereo stream only) */
    E_MI_AO_CHANNEL_MODE_ONLY_LEFT, /* Stereo stream should only be presented with the left audio channel.
                                       Mono stream should be delivered to the left audio channel.  */
    E_MI_AO_CHANNEL_MODE_ONLY_RIGHT, /* Stereo stream should only be presented with the right audio channel.
                                        Mono stream should be delivered to the right audio channel. */
} MI_AO_ChannelMode_e;

typedef struct MI_AO_Attr_s
{
    MI_AUDIO_Format_e     enFormat;           /* The sample format of a PCM */
    MI_AUDIO_SoundMode_e  enSoundMode;        /* The number of channels in a frame */
    MI_AUDIO_SampleRate_e enSampleRate;       /* The number of frames per second */
    MI_U32                u32PeriodSize;      /* The number of frames in a period */
    MI_AO_ChannelMode_e   enChannelMode;      /* Channel mode */
    MI_AUDIO_SampleRate_e enMixerSampleRate;  /* The number of mixer frames per second */
} MI_AO_Attr_t;

typedef enum
{
    E_MI_AO_IF_NONE   = 0x0u,
    E_MI_AO_IF_DAC_AB = 0x1u,  /* DAC A 0 channel and B 0 channel */
    E_MI_AO_IF_DAC_CD = 0x2u,  /* DAC C 0 channel and D 0 channel */
    E_MI_AO_IF_I2S_A  = 0x4u,  /* I2S A any 2 channels */
    E_MI_AO_IF_I2S_B  = 0x8u,  /* I2S B any 2 channels */
    E_MI_AO_IF_ECHO_A = 0x10u, /* Echo left and right channel */
    E_MI_AO_IF_HDMI_A = 0x20u, /* HDMI A left and right channel */
    E_MI_AO_IF_MAX,
} MI_AO_If_e;

typedef enum
{
    E_MI_AO_GAIN_FADING_OFF = 0,
    E_MI_AO_GAIN_FADING_1_SAMPLE,
    E_MI_AO_GAIN_FADING_2_SAMPLE,
    E_MI_AO_GAIN_FADING_4_SAMPLE,
    E_MI_AO_GAIN_FADING_8_SAMPLE,
    E_MI_AO_GAIN_FADING_16_SAMPLE,
    E_MI_AO_GAIN_FADING_32_SAMPLE,
    E_MI_AO_GAIN_FADING_64_SAMPLE,
} MI_AO_GainFading_e;

typedef struct MI_AO_InitParam_s
{
    MI_U32 u32DevId;
    MI_U8 *u8Data;
} MI_AO_InitParam_t;

//=============================================================================
// Variable definition
//=============================================================================

//=============================================================================
// Global function definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif //__MI_AO_DATATYPE_H__
