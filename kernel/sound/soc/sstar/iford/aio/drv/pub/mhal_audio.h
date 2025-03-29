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
#include <sound/pcm.h>
#include "mhal_audio_common.h"
#include "mhal_audio_datatype.h"

#include "hal_audio_common.h"
#include "hal_audio_types.h"

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
    bool          nUseSrc;

} MHAL_AO_Attach_t;

typedef struct
{
    MHAL_SineGen_Freq_e
          enFreq; // For 48KHz sample rate, other sample rate must calulate by ratio, eg: 16KHz -> (freq * 16 / 48)
    MS_S8 s8Gain; // 0: 0dB(maximum), 1: -6dB, ... (-6dB per step)

} MHAL_SineGen_Cfg_t;

// -------------------------------------------------------------------------------
MS_S32 mhal_audio_init(void *pdata);
MS_S32 mhal_audio_deinit(void);

MS_S32 mhal_audio_ai_if_setting(MHAL_AI_IF_e enAiIf, void *pAiSetting);
MS_S32 mhal_audio_ao_if_setting(MHAL_AO_IF_e enAoIf, void *pAoSetting, MHAL_AO_PATH_RATE_t *aoAttribute);

MS_S32 mhal_audio_ai_if_gain(MHAL_AI_IF_e enAiIf, MS_S8 s8LeftIfGain, MS_S8 s8RightIfGain);
MS_S32 mhal_audio_ao_if_gain(MHAL_AO_IF_e enAoIf, MS_S8 s8IfGain);

MS_S32 mhal_audio_ai_config(MHAL_AI_Dma_e enAiDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig);
MS_S32 mhal_audio_ao_config(MHAL_AO_Dma_e enAoDma, MHAL_AUDIO_PcmCfg_t *pstDmaConfig);

MS_S32 mhal_audio_ai_open(MHAL_AI_Dma_e enAiDma);
MS_S32 mhal_audio_ao_open(MHAL_AO_Dma_e enAoDma);

MS_S32 mhal_audio_ai_attach(MHAL_AI_Attach_t *pAiAttach);
MS_S32 mhal_audio_ao_attach(MHAL_AO_Attach_t *pAoAttach);

MS_S32 mhal_audio_ai_detach(MHAL_AI_Attach_t *pAiAttach);
MS_S32 mhal_audio_ao_detach(MHAL_AO_Attach_t *pAoAttach);

MS_S32 mhal_audio_ai_close(MHAL_AI_Dma_e enAiDma);
MS_S32 mhal_audio_ao_close(MHAL_AO_Dma_e enAoDma);

MS_S32 mhal_audio_ai_start(MHAL_AI_Dma_e enAiDma);
MS_S32 mhal_audio_ao_start(MHAL_AO_Dma_e enAoDma);

MS_S32 mhal_audio_ai_stop(MHAL_AI_Dma_e enAiDma);
MS_S32 mhal_audio_ao_stop(MHAL_AO_Dma_e enAoDma);

MS_S32 mhal_audio_ai_read_trig(MHAL_AI_Dma_e enAiDma, MS_U32 u32Frames);
MS_S32 mhal_audio_ao_write_trig(MHAL_AO_Dma_e enAoDma, MS_U32 u32Frames);

MS_S32 mhal_audio_ai_preparetorestart(MHAL_AI_Dma_e enAiDma);
MS_S32 mhal_audio_ao_preparetorestart(MHAL_AO_Dma_e enAoDma);

MS_S32 mhal_audio_ai_get_curr_datalen(MHAL_AI_Dma_e enAiDma, MS_U32 *len);
MS_S32 mhal_audio_ao_get_curr_datalen(MHAL_AO_Dma_e enAoDma, MS_U32 *len);

MS_BOOL mhal_audio_ai_is_dmafree(MHAL_AI_Dma_e enAiDma);
MS_BOOL mhal_audio_ao_is_dmafree(MHAL_AO_Dma_e enAoDma);

MS_S32 mhal_audio_ao_pause(MHAL_AO_Dma_e enAoDma);
MS_S32 mhal_audio_ao_resume(MHAL_AO_Dma_e enAoDma);

MS_S32 mhal_audio_ai_pause(MHAL_AI_Dma_e enAiDma);
MS_S32 mhal_audio_ai_resume(MHAL_AI_Dma_e enAiDma);

MS_S32 mhal_audio_ai_interrupt_get(MHAL_AI_Dma_e enAiDma, struct sstar_interupt_status *status,
                                   struct sstar_interupt_flag *flag);
MS_S32 mhal_audio_ao_interrupt_get(MHAL_AO_Dma_e enAoDma, struct sstar_interupt_status *status,
                                   struct sstar_interupt_flag *flag);
MS_S32 mhal_audio_ai_interrupt_set(MHAL_AI_Dma_e enAiDma, struct sstar_interupt_en *irq_en);
MS_S32 mhal_audio_ao_interrupt_set(MHAL_AO_Dma_e enAoDma, struct sstar_interupt_en *irq_en);

MS_S32 mhal_audio_ao_set_channel_mode(MHAL_AO_Dma_e enAoDma, MHAL_AO_ChMode_e enChMode);
MS_S32 mhal_audio_set_audio_delay(MHAL_AO_Dma_e enAoDma, MS_BOOL bCtl);
MS_S32 mhal_audio_set_hdmi_rx_sel(MHAL_AO_Dma_e enAoDma, int nSel);
MS_S32 mhal_audio_save_cache(MHAL_AO_Dma_e enAoDma, int nMs);

void mhal_audio_module_init(void);
void mhal_audio_module_deinit(void);

MS_S32 mhal_audio_runtime_power_ctl(MS_BOOL bEnable);

void   mhal_audio_get_hdmi_audio_format(hdmi_audio_format *pstFmt);
void   mhal_audio_renew_hdmi_audio_format(void);
MS_S32 mhal_audio_ao_sidetone_regulate(MHAL_AO_Dma_e enAoDma, MS_U8 u8ChId, int nVolume,
                                       MHAL_AUDIO_GainFading_e enFading);
MS_S32 mhal_audio_mclk_setting(MHAL_MCLK_ID_e mclk_id, int mclk_freq, MS_BOOL bEnable);

MS_S32 mhal_audio_amp_state_get(int *nEnable, MS_U8 u8ChId);
MS_S32 mhal_audio_amp_state_set(int nEnable, MS_U8 u8ChId);

MS_S32 mhal_audio_spdif_switch_ctl(MHAL_SWITCH_e enCtl, MHAL_SWITCH_e *enState);

MS_BOOL mhal_audio_set_mute(int nSel, MS_BOOL bEnable);
MS_BOOL mhal_audio_get_mute(int nSel);

MS_S32 mhal_audio_local_fifo_irq(struct snd_pcm_substream *substream, int dma_id);
MS_U32 mhal_audio_get_dts_value(DTS_CONFIG_KEY_e key);

MS_U32 mhal_audio_clear_int(struct snd_pcm_substream *substream, int dma_id);

#endif //__MHAL_AUDIO_H__
