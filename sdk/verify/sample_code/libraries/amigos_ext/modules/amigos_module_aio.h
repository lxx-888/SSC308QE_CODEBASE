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

#include "mi_aio_datatype.h"

SS_ENUM_CAST_STR(MI_AUDIO_Format_e,
{
    {E_MI_AUDIO_FORMAT_INVALID,               "invalid"},
    {E_MI_AUDIO_FORMAT_PCM_S16_LE,            "pcm_s16_le"},
});
SS_ENUM_CAST_STR(MI_AUDIO_SoundMode_e,
{
    {E_MI_AUDIO_SOUND_MODE_MONO,              "mono"},
    {E_MI_AUDIO_SOUND_MODE_STEREO,            "stereo"},
    {E_MI_AUDIO_SOUND_MODE_4CH,               "4ch"},
    {E_MI_AUDIO_SOUND_MODE_6CH,               "6ch"},
    {E_MI_AUDIO_SOUND_MODE_8CH,               "8ch"},
});
SS_ENUM_CAST_STR(MI_AUDIO_I2sMode_e,
{
    {E_MI_AUDIO_I2S_MODE_I2S_MASTER,          "I2S_MASTER"},
    {E_MI_AUDIO_I2S_MODE_I2S_SLAVE,           "I2S_SLAVE"},
    {E_MI_AUDIO_I2S_MODE_TDM_MASTER,          "TDM_MASTER"},
    {E_MI_AUDIO_I2S_MODE_TDM_SLAVE,           "TDM_SLAVE"},
});
SS_ENUM_CAST_STR(MI_AUDIO_I2sFormat_e,
{
    {E_MI_AUDIO_I2S_FMT_I2S_MSB,              "I2S_MSB"},
    {E_MI_AUDIO_I2S_FMT_LEFT_JUSTIFY_MSB,     "LEFT_JUSTIFY_MSB"},
});
SS_ENUM_CAST_STR(MI_AUDIO_I2sMclk_e,
{
    {E_MI_AUDIO_I2S_MCLK_0,                   "0"},
    {E_MI_AUDIO_I2S_MCLK_12_288M,             "12.288"},
    {E_MI_AUDIO_I2S_MCLK_16_384M,             "16.384"},
    {E_MI_AUDIO_I2S_MCLK_18_432M,             "18.432"},
    {E_MI_AUDIO_I2S_MCLK_24_576M,             "24.576"},
    {E_MI_AUDIO_I2S_MCLK_24M,                 "24"},
    {E_MI_AUDIO_I2S_MCLK_48M,                 "48"},
});
SS_ENUM_CAST_STR(MI_AUDIO_I2sBitWidth_e,
{
    {E_MI_AUDIO_BIT_WIDTH_16,                 "16"},
    {E_MI_AUDIO_BIT_WIDTH_32,                 "32"},
});

