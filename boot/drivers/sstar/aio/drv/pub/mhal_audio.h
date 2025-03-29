/*
 * mhal_audio.h - Sigmastar
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

#ifndef __MHAL_AUDIO_H__

#define __MHAL_AUDIO_H__
#include "mhal_audio_common.h"
#include "mhal_audio_datatype.h"

// -------------------------------------------------------------------------------
#define MHAL_FAILURE_NO_RESOURCE (-2)

// -------------------------------------------------------------------------------
typedef struct
{
    MHAL_AI_Dma_e enAiDma;
    MHAL_AI_IF_e  aenAiIf[E_MHAL_AI_DMA_CH_SLOT_TOTAL];

} MHAL_AI_Attach_t;

typedef struct
{
    MHAL_AO_Dma_e enAoDma;
    MS_U8         nAoDmaCh;
    MHAL_AO_IF_e  enAoIf;

} MHAL_AO_Attach_t;

typedef struct
{
    MHAL_SineGen_Freq_e
          enFreq; // For 48KHz sample rate, other sample rate must calulate by ratio, eg: 16KHz -> (freq * 16 / 48)
    MS_S8 s8Gain; // 0: 0dB(maximum), 1: -6dB, ... (-6dB per step)

} MHAL_SineGen_Cfg_t;

// -------------------------------------------------------------------------------
MS_S32 MHAL_AUDIO_Init(void *pdata);
MS_S32 MHAL_AUDIO_DeInit(void);

MS_S32 MHAL_AUDIO_AI_If_Setting(MHAL_AI_IF_e enAiIf, void *pAiSetting);
MS_S32 MHAL_AUDIO_AO_If_Setting(MHAL_AO_IF_e enAoIf, void *pAoSetting);

MS_S32 MHAL_AUDIO_AI_If_Gain(MHAL_AI_IF_e enAiIf, MS_S8 s8LeftIfGain, MS_S8 s8RightIfGain);
MS_S32 MHAL_AUDIO_AO_If_Gain(MHAL_AO_IF_e enAoIf, MS_S8 s8IfGain);

MS_S32 MHAL_AUDIO_AI_If_Mute(MHAL_AI_IF_e enAiIf, MS_BOOL bLeftEnable, MS_BOOL bRightEnable);
MS_S32 MHAL_AUDIO_AO_If_Mute(MHAL_AO_IF_e enAoIf, MS_BOOL bEnable);

MS_S32 MHAL_AUDIO_AI_Dpga_Gain(MHAL_AI_Dma_e enAiDma, MS_U8 u8ChId, MS_S8 s8Gain);
MS_S32 MHAL_AUDIO_AO_Dpga_Gain(MHAL_AO_Dma_e enAoDma, MS_U8 u8ChId, MS_S8 s8Gain, MHAL_AUDIO_GainFading_e enFading);

MS_S32 MHAL_AUDIO_AI_Dpga_Mute(MHAL_AI_Dma_e enAiDma, MS_U8 u8ChId, MS_BOOL bEnable);
MS_S32 MHAL_AUDIO_AO_Dpga_Mute(MHAL_AO_Dma_e enAoDma, MS_U8 u8ChId, MS_BOOL bEnable, MHAL_AUDIO_GainFading_e enFading);

MS_S32 MHAL_AUDIO_AI_Config(MHAL_AI_Dma_e enAiDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig);
MS_S32 MHAL_AUDIO_AO_Config(MHAL_AO_Dma_e enAoDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig);

MS_S32 MHAL_AUDIO_AI_Open(MHAL_AI_Dma_e enAiDma);
MS_S32 MHAL_AUDIO_AO_Open(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_AI_Attach(MHAL_AI_Attach_t *pAiAttach);
MS_S32 MHAL_AUDIO_AO_Attach(MHAL_AO_Attach_t *pAoAttach);

MS_S32 MHAL_AUDIO_AO_Detach(MHAL_AO_Attach_t *pAoAttach);

MS_S32 MHAL_AUDIO_AI_Close(MHAL_AI_Dma_e enAiDma);
MS_S32 MHAL_AUDIO_AO_Close(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_AI_Start(MHAL_AI_Dma_e enAiDma);
MS_S32 MHAL_AUDIO_AO_Start(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_AI_Stop(MHAL_AI_Dma_e enAiDma);
MS_S32 MHAL_AUDIO_AO_Stop(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_AI_Read_Data(MHAL_AI_Dma_e enAiDma, VOID *pRdBuffer, MS_U32 u32Size);
MS_S32 MHAL_AUDIO_AO_Write_Data(MHAL_AO_Dma_e enAoDma, VOID *pWrBuffer, MS_U32 u32Size);

MS_BOOL MHAL_AUDIO_AI_IsXrun(MHAL_AI_Dma_e enAiDma);
MS_BOOL MHAL_AUDIO_AO_IsXrun(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_AI_PrepareToRestart(MHAL_AI_Dma_e enAiDma);
MS_S32 MHAL_AUDIO_AO_PrepareToRestart(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_AI_GetCurrDataLen(MHAL_AI_Dma_e enAiDma, MS_U32 *len);
MS_S32 MHAL_AUDIO_AO_GetCurrDataLen(MHAL_AO_Dma_e enAoDma, MS_U32 *len);

MS_BOOL MHAL_AUDIO_AI_IsDmaFree(MHAL_AI_Dma_e enAiDma);
MS_BOOL MHAL_AUDIO_AO_IsDmaFree(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_AO_Pause(MHAL_AO_Dma_e enAoDma);
MS_S32 MHAL_AUDIO_AO_Resume(MHAL_AO_Dma_e enAoDma);

MS_S32 MHAL_AUDIO_SineGen_Start(MHAL_SineGen_e enSineGen, MHAL_SineGen_Cfg_t *pstSineGenConfig);
MS_S32 MHAL_AUDIO_SineGen_Stop(MHAL_SineGen_e enSineGen);

void MHAL_AUDIO_Module_Init(void);

void MHAL_AUDIO_Module_DeInit(void);

#endif //__MHAL_AUDIO_H__
