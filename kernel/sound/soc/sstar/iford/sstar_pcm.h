/*
 * sstar_pcm.h - Sigmastar
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

#ifndef __SOUND_SOC_PCM_H
#define __SOUND_SOC_PCM_H
#include "mhal_audio.h"

#define SSTAR_PCM_IRQ_EN    1
#define SSTAR_PCM_DUMP_FILE 1

#define SSTAR_DMA_EMPTY    0x0
#define SSTAR_DMA_UNDERRUN 0x1
#define SSTAR_DMA_OVERRUN  0x2
#define SSTAR_DMA_FULL     0x3
#define SSTAR_DMA_NORMAL   0x4
#define SSTAR_DMA_INIT     0x5

#define SSTAR_PCM_RUNTIME_STATE_INIT     (1 << 0) // 1
#define SSTAR_PCM_RUNTIME_STATE_SETUP    (1 << 1) // 2
#define SSTAR_PCM_RUNTIME_STATE_OPEN     (1 << 2) // 4
#define SSTAR_PCM_RUNTIME_STATE_PREPARED (1 << 3) // 8
#define SSTAR_PCM_RUNTIME_STATE_RUNNING  (1 << 4) // 16
#define SSTAR_PCM_RUNTIME_STATE_XRUN     (1 << 5) // 32
#define SSTAR_PCM_RUNTIME_STATE_PAUSED   (1 << 6) // 64

enum
{
    SSTAR_AUD_CH_CAPTURE,
    SSTAR_AUD_CH_PLAYBACK,
    SSTAR_AUD_CH_MAX,
};

struct sstar_dma_buffer
{
    unsigned char *area;  /* virtual pointer */
    u64            addr;  /* physical address */
    u64            bytes; /* buffer size in bytes */
};

struct sstar_pcm_stream
{
    struct device *           dev;
    struct snd_pcm_substream *substream;
#if SSTAR_PCM_IRQ_EN
    u8                           name[20]; /* stream identifier */
    spinlock_t                   irq_lock; /* spinlock for irq */
    int                          dma_state;
    int                          runtime_state;
    struct sstar_interupt_status irq_status;
    struct sstar_interupt_flag   irq_flag;
#else
    struct hrtimer hrt;
    u64            poll_time_ns;
    int            start_timer;
#endif
    snd_pcm_uframes_t         prev_frame;
    snd_pcm_uframes_t         prev_hw_ptr;
    snd_pcm_uframes_t         prev_appl_ptr;
    struct sstar_dma_buffer   buf;
    MHAL_AUDIO_I2sCfg_t       i2s_cfg;
    MHAL_AUDIO_PcmCfg_t       pcm;
    int                       mmap;
    int                       dma_id;
    int                       trig_frame;
    struct snd_soc_component *component;
    bool                      is_suspend;
#if SSTAR_PCM_DUMP_FILE
    struct file *dump_file[2];
#endif
};

struct sstar_pcm_engine
{
    struct device *         dev;
    int                     rdma;
    int                     wdma;
    unsigned int            irq_id;
    struct sstar_pcm_stream tx;
    struct sstar_pcm_stream rx;
};

int sstar_pcmengine_unregister(struct snd_soc_component_driver *sstar_component_driver);
int sstar_pcmengine_register(struct snd_soc_component_driver *sstar_component_driver);

#endif /* __SOUND_SOC_PCM_H */
