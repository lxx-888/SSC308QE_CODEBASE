/*
 * drv_audio_export.c - Sigmastar
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

#include <linux/kernel.h>
#include "mhal_audio.h"
#include <linux/module.h>

EXPORT_SYMBOL(MHAL_AUDIO_Init);
EXPORT_SYMBOL(MHAL_AUDIO_DeInit);
EXPORT_SYMBOL(MHAL_AUDIO_AI_If_Setting);
EXPORT_SYMBOL(MHAL_AUDIO_AO_If_Setting);
EXPORT_SYMBOL(MHAL_AUDIO_AI_If_Gain);
EXPORT_SYMBOL(MHAL_AUDIO_AO_If_Gain);
EXPORT_SYMBOL(MHAL_AUDIO_AI_If_Mute);
EXPORT_SYMBOL(MHAL_AUDIO_AO_If_Mute);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Dpga_Gain);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Dpga_Gain);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Dpga_Mute);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Dpga_Mute);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Config);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Config);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Open);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Open);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Attach);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Attach);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Detach);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Close);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Close);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Start);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Start);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Stop);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Stop);
EXPORT_SYMBOL(MHAL_AUDIO_AI_Read_Data);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Write_Data);
EXPORT_SYMBOL(MHAL_AUDIO_AI_IsXrun);
EXPORT_SYMBOL(MHAL_AUDIO_AO_IsXrun);
EXPORT_SYMBOL(MHAL_AUDIO_AI_PrepareToRestart);
EXPORT_SYMBOL(MHAL_AUDIO_AO_PrepareToRestart);
EXPORT_SYMBOL(MHAL_AUDIO_AI_GetCurrDataLen);
EXPORT_SYMBOL(MHAL_AUDIO_AO_GetCurrDataLen);
EXPORT_SYMBOL(MHAL_AUDIO_AI_IsDmaFree);
EXPORT_SYMBOL(MHAL_AUDIO_AO_IsDmaFree);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Pause);
EXPORT_SYMBOL(MHAL_AUDIO_AO_Resume);
EXPORT_SYMBOL(MHAL_AUDIO_SineGen_Start);
EXPORT_SYMBOL(MHAL_AUDIO_SineGen_Stop);
EXPORT_SYMBOL(MHAL_AUDIO_Module_Init);
EXPORT_SYMBOL(MHAL_AUDIO_Module_DeInit);
MODULE_LICENSE("PROPRIETARY");
