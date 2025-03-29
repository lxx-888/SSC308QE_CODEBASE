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

#include "ptree_enum.h"
#include "ptree_sur_aio.h"
#include "mi_ai_datatype.h"
#include "mi_ao_datatype.h"

PTREE_ENUM_DEFINE(
    MI_AI_If_e, {E_MI_AI_IF_NONE, "NONE"}, {E_MI_AI_IF_ADC_AB, "ADC_AB"}, {E_MI_AI_IF_ADC_CD, "ADC_CD"},
    {E_MI_AI_IF_DMIC_A_01, "DMIC_A_01"}, {E_MI_AI_IF_DMIC_A_23, "DMIC_A_23"}, {E_MI_AI_IF_I2S_A_01, "I2S_A_01"},
    {E_MI_AI_IF_I2S_A_23, "I2S_A_23"}, {E_MI_AI_IF_I2S_A_45, "I2S_A_45"}, {E_MI_AI_IF_I2S_A_67, "I2S_A_67"},
    {E_MI_AI_IF_I2S_A_89, "I2S_A_89"}, {E_MI_AI_IF_I2S_A_ab, "I2S_A_ab"}, {E_MI_AI_IF_I2S_A_cd, "I2S_A_cd"},
    {E_MI_AI_IF_I2S_A_ef, "I2S_A_ef"}, {E_MI_AI_IF_I2S_B_01, "I2S_B_01"}, {E_MI_AI_IF_I2S_B_23, "I2S_B_23"},
    {E_MI_AI_IF_I2S_B_45, "I2S_B_45"}, {E_MI_AI_IF_I2S_B_67, "I2S_B_67"}, {E_MI_AI_IF_I2S_B_89, "I2S_B_89"},
    {E_MI_AI_IF_I2S_B_ab, "I2S_B_ab"}, {E_MI_AI_IF_I2S_B_cd, "I2S_B_cd"}, {E_MI_AI_IF_I2S_B_ef, "I2S_B_ef"},
    {E_MI_AI_IF_I2S_C_01, "I2S_C_01"}, {E_MI_AI_IF_I2S_C_23, "I2S_C_23"}, {E_MI_AI_IF_I2S_C_45, "I2S_C_45"},
    {E_MI_AI_IF_I2S_C_67, "I2S_C_67"}, {E_MI_AI_IF_I2S_C_89, "I2S_C_89"}, {E_MI_AI_IF_I2S_C_ab, "I2S_C_ab"},
    {E_MI_AI_IF_I2S_C_cd, "I2S_C_cd"}, {E_MI_AI_IF_I2S_C_ef, "I2S_C_ef"}, {E_MI_AI_IF_I2S_D_01, "I2S_D_01"},
    {E_MI_AI_IF_I2S_D_23, "I2S_D_23"}, {E_MI_AI_IF_I2S_D_45, "I2S_D_45"}, {E_MI_AI_IF_I2S_D_67, "I2S_D_67"},
    {E_MI_AI_IF_I2S_D_89, "I2S_D_89"}, {E_MI_AI_IF_I2S_D_ab, "I2S_D_ab"}, {E_MI_AI_IF_I2S_D_cd, "I2S_D_cd"},
    {E_MI_AI_IF_I2S_D_ef, "I2S_D_ef"}, {E_MI_AI_IF_ECHO_A, "ECHO_A"}, {E_MI_AI_IF_HDMI_A, "HDMI_A"},
    {E_MI_AI_IF_DMIC_A_45, "DMIC_A_45"})

PTREE_ENUM_DEFINE(MI_AO_If_e, {E_MI_AO_IF_NONE, "NONE"}, {E_MI_AO_IF_DAC_AB, "DAC_AB"}, {E_MI_AO_IF_DAC_CD, "DAC_CD"},
                  {E_MI_AO_IF_I2S_A, "I2S_A"}, {E_MI_AO_IF_I2S_B, "I2S_B"}, {E_MI_AO_IF_ECHO_A, "ECHO_A"},
                  {E_MI_AO_IF_HDMI_A, "HDMI_A"}, )

PTREE_ENUM_DEFINE(MI_AO_ChannelMode_e, {E_MI_AO_CHANNEL_MODE_STEREO, "STEREO"},
                  {E_MI_AO_CHANNEL_MODE_DOUBLE_MONO, "DOUBLE_MONO"}, {E_MI_AO_CHANNEL_MODE_DOUBLE_LEFT, "DOUBLE_LEFT"},
                  {E_MI_AO_CHANNEL_MODE_DOUBLE_RIGHT, "DOUBLE_RIGHT"}, {E_MI_AO_CHANNEL_MODE_EXCHANGE, "EXCHANGE"},
                  {E_MI_AO_CHANNEL_MODE_ONLY_LEFT, "ONLY_LEFT"}, {E_MI_AO_CHANNEL_MODE_ONLY_RIGHT, "ONLY_RIGHT"}, )

PTREE_ENUM_DEFINE(MI_AUDIO_Format_e, {E_MI_AUDIO_FORMAT_INVALID, "invalid"},
                  {E_MI_AUDIO_FORMAT_PCM_S16_LE, "pcm_s16_le"}, )

PTREE_ENUM_DEFINE(MI_AUDIO_SoundMode_e, {E_MI_AUDIO_SOUND_MODE_MONO, "mono"}, {E_MI_AUDIO_SOUND_MODE_STEREO, "stereo"},
                  {E_MI_AUDIO_SOUND_MODE_4CH, "4ch"}, {E_MI_AUDIO_SOUND_MODE_6CH, "6ch"},
                  {E_MI_AUDIO_SOUND_MODE_8CH, "8ch"}, )

PTREE_ENUM_DEFINE(MI_AUDIO_I2sMode_e, {E_MI_AUDIO_I2S_MODE_I2S_MASTER, "I2S_MASTER"},
                  {E_MI_AUDIO_I2S_MODE_I2S_SLAVE, "I2S_SLAVE"}, {E_MI_AUDIO_I2S_MODE_TDM_MASTER, "TDM_MASTER"},
                  {E_MI_AUDIO_I2S_MODE_TDM_SLAVE, "TDM_SLAVE"}, )

PTREE_ENUM_DEFINE(MI_AUDIO_I2sFormat_e, {E_MI_AUDIO_I2S_FMT_I2S_MSB, "I2S_MSB"},
                  {E_MI_AUDIO_I2S_FMT_LEFT_JUSTIFY_MSB, "LEFT_JUSTIFY_MSB"}, )

PTREE_ENUM_DEFINE(MI_AUDIO_I2sMclk_e, {E_MI_AUDIO_I2S_MCLK_0, "0"}, {E_MI_AUDIO_I2S_MCLK_12_288M, "12.288"},
                  {E_MI_AUDIO_I2S_MCLK_16_384M, "16.384"}, {E_MI_AUDIO_I2S_MCLK_18_432M, "18.432"},
                  {E_MI_AUDIO_I2S_MCLK_24_576M, "24.576"}, {E_MI_AUDIO_I2S_MCLK_24M, "24"},
                  {E_MI_AUDIO_I2S_MCLK_48M, "48"}, )

PTREE_ENUM_DEFINE(MI_AUDIO_I2sBitWidth_e, {E_MI_AUDIO_BIT_WIDTH_16, "16"}, {E_MI_AUDIO_BIT_WIDTH_32, "32"}, )
