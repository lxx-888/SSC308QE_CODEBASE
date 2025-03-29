/*
 * sstar_pcm.c - Sigmastar
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/extcon-provider.h>
#include <linux/extcon.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/dmaengine_pcm.h>
#include "pcm_local.h"

#include "sstar_pcm.h"
#include "sstar_common.h"
#include "cam_os_wrapper.h"
#include "cam_sysfs.h"
#include "ms_msys.h"

#include "hal_audio_common.h"
#include "hal_audio_config.h"
#include "hal_audio_sys.h"
#include "hal_audio_types.h"
#include "hal_audio_api.h"
#include "hal_audio_os_api.h"

#ifdef CONFIG_SND_PCM_XRUN_DEBUG
#define CREATE_TRACE_POINTS
#include "sstar_pcm_trace.h"
#else
#define trace_ack(sstar_stream, level_cnt)
#define trace_trigger(sstar_stream, cmd, level_cnt)
#define trace_pointer(sstar_stream, pos, level_cnt, delta)
#endif

struct _sstar_dev
{
    struct device *         dev;
    struct sstar_pcm_engine pcm_engine;
};

struct dma_buffer
{
    unsigned char *area;  /* virtual pointer */
    u64            addr;  /* physical address */
    size_t         bytes; /* buffer size in bytes */
};

static int sstar_pcm_ack(struct snd_soc_component *component, struct snd_pcm_substream *substream);

static void *alloc_dmem(const char *name, unsigned int size, dma_addr_t *addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    if (0 != msys_request_dmem(&dmem))
    {
        return NULL;
    }
    *addr = dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}

static void free_dmem(const char *name, unsigned int size, void *virt, dma_addr_t addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    dmem.kvirt  = (void *)((uintptr_t)virt);
    dmem.phys   = (unsigned long long)((uintptr_t)addr);
    msys_release_dmem(&dmem);
}

/* pcm ops */
/*
static const struct snd_pcm_hardware sstar_hardware = {
    .info           = SNDRV_PCM_INFO_MMAP |
                  SNDRV_PCM_INFO_MMAP_VALID |
                  SNDRV_PCM_INFO_PAUSE |
                  SNDRV_PCM_INFO_RESUME |
                  SNDRV_PCM_INFO_INTERLEAVED,
    .period_bytes_min   = 32,
    .period_bytes_max   = 8192,
    .periods_min        = 1,
    .periods_max        = 52,
    .buffer_bytes_max   = 64 * 1024,
    .fifo_size      = 32,
};
*/
static const struct snd_pcm_hardware sstar_hardware = {
    .info             = SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME | SNDRV_PCM_INFO_INTERLEAVED,
    .period_bytes_min = 32,
    .period_bytes_max = 512 * 1024,
    .periods_min      = 2,
    .periods_max      = 4096 * 10,
    .buffer_bytes_max = 1024 * 1024,
    .fifo_size        = 32,
};

#if SSTAR_PCM_IRQ_EN
static void sstar_process_irq(struct sstar_pcm_stream *ss_pcm_stream, struct snd_pcm_substream *substream)
{
    struct sstar_interupt_en     irq_en = {0};
    struct sstar_interupt_status irq_status;
    struct sstar_interupt_flag   irq_flag;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        mhal_audio_ao_interrupt_get(ss_pcm_stream->dma_id, &irq_status, &irq_flag);
    }
    else
    {
        mhal_audio_ai_interrupt_get(ss_pcm_stream->dma_id, &irq_status, &irq_flag);
    }
#if 0
    {
        int len = 0;
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        {
            mhal_audio_ao_get_curr_datalen(ss_pcm_stream->dma_id, &len);
        }
        else
        {
            mhal_audio_ai_get_curr_datalen(ss_pcm_stream->dma_id, &len);
        }
        printk("[%s]len[%d]\n", substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture", len);
    }
    // DBGMSG(AUDIO_DBG_LEVEL_IRQ, "len[%d]", len);
#endif

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        if (irq_flag.boundary_flag == TRUE && irq_status.boundary_int == TRUE)
        {
            irq_en.trigger_en  = FALSE;
            irq_en.boundary_en = FALSE;
            irq_en.transmit_en = TRUE;
            mhal_audio_ao_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
            mhal_audio_ao_stop(ss_pcm_stream->dma_id);
            ss_pcm_stream->dma_state = SSTAR_DMA_EMPTY;
            snd_pcm_period_elapsed(substream);
        }
        else if (irq_flag.transmit_flag == TRUE)
        {
            irq_en.trigger_en  = FALSE;
            irq_en.boundary_en = TRUE;
            irq_en.transmit_en = TRUE;
            mhal_audio_ao_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
        }
    }
    else
    {
        if (irq_flag.boundary_flag == TRUE && irq_status.boundary_int == TRUE)
        // if (irq_flag.trigger_flag == TRUE && irq_status.trigger_int == TRUE)
        {
            irq_en.trigger_en  = FALSE;
            irq_en.boundary_en = FALSE;
            irq_en.transmit_en = TRUE;
            mhal_audio_ai_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
            mhal_audio_ai_stop(ss_pcm_stream->dma_id);
            ss_pcm_stream->dma_state = SSTAR_DMA_FULL;
        }
        else if (irq_flag.transmit_flag == TRUE)
        {
            irq_en.trigger_en  = FALSE;
            irq_en.boundary_en = TRUE;
            irq_en.transmit_en = TRUE;
            mhal_audio_ai_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
        }
    }
}

static irqreturn_t sstar_snd_irq_handler(int irq_id, void *dev)
{
    struct sstar_pcm_stream *    ss_pcm_stream = (struct sstar_pcm_stream *)dev;
    struct snd_pcm_substream *   substream     = ss_pcm_stream->substream;
    struct sstar_interupt_status irq_status;
    struct sstar_interupt_flag   irq_flag;
    unsigned long                flags;
    // static unsigned long         i = 0;

    spin_lock_irqsave(&ss_pcm_stream->irq_lock, flags);

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        mhal_audio_ao_interrupt_get(ss_pcm_stream->dma_id, &irq_status, &irq_flag);
    }
    else
    {
        mhal_audio_ai_interrupt_get(ss_pcm_stream->dma_id, &irq_status, &irq_flag);
    }

    if (irq_flag.transmit_flag == FALSE && irq_flag.boundary_flag == FALSE && irq_flag.local_data_flag == FALSE)
    {
        goto OUT;
    }

    // For dmix
    if (irq_flag.transmit_flag == TRUE && ss_pcm_stream->component && substream->stream == SNDRV_PCM_STREAM_PLAYBACK
        && ss_pcm_stream->mmap
        && substream->runtime->status->hw_ptr
               >= substream->runtime->control->appl_ptr - substream->runtime->buffer_size / 2)
    {
        substream->runtime->control->appl_ptr += substream->runtime->buffer_size / 2;
        sstar_pcm_ack(ss_pcm_stream->component, substream);
    }

    if (ss_pcm_stream->component && substream->stream == SNDRV_PCM_STREAM_CAPTURE && ss_pcm_stream->mmap
        && substream->runtime->status->hw_ptr - substream->runtime->buffer_size / 2
               >= substream->runtime->control->appl_ptr)
    {
        substream->runtime->control->appl_ptr += substream->runtime->buffer_size / 2;
        sstar_pcm_ack(ss_pcm_stream->component, substream);
    }

    // Local empty will make the alsa state error, so if ouccer local data empty, don't refresh the alsa asoc pointer.
    if (irq_flag.local_data_flag == TRUE)
    {
        if (irq_status.local_data_int == TRUE)
        {
            mhal_audio_local_fifo_irq(substream, ss_pcm_stream->dma_id);
        }
        // mhal_audio_clear_int(substream, ss_pcm_stream->dma_id);
        trace_trigger(ss_pcm_stream, 10086, 0);
        // goto OUT;
    }
    // Alsa asoc update hw ptr.
    snd_pcm_period_elapsed(substream);

    // Process sstar irq, update irq status.
    sstar_process_irq(ss_pcm_stream, substream);
OUT:
    spin_unlock_irqrestore(&ss_pcm_stream->irq_lock, flags);

    return IRQ_HANDLED;
}

#else
static enum hrtimer_restart snd_hrtimer_callback(struct hrtimer *hrt)
{
    struct sstar_pcm_stream * ss_pcm_stream = container_of(hrt, struct sstar_pcm_stream, hrt);
    struct snd_pcm_substream *substream     = ss_pcm_stream->substream;

    hrtimer_forward_now(hrt, ns_to_ktime(ss_pcm_stream->poll_time_ns));

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK && ss_pcm_stream->mmap
        && substream->runtime->status->hw_ptr
               >= substream->runtime->control->appl_ptr - substream->runtime->period_size)
    {
        substream->runtime->control->appl_ptr += substream->runtime->buffer_size;
        sstar_pcm_ack(ss_pcm_stream->component, substream);
    }

    snd_pcm_period_elapsed(substream);

    return HRTIMER_RESTART;
}

#endif

static int sstar_pcm_probe(struct snd_soc_component *component)
{
    int                      ret    = 0;
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    char                     name[20];
    struct sstar_dma_buffer *buf;
    int                      size_max = 512 * 512; // max=65535;

    // alloc memory
    if (ss_pcm->wdma >= 0)
    {
        // AI buffer allocate
        buf = &ss_pcm->rx.buf;
        snprintf(name, 16, "pcmC%d", ss_pcm->wdma);
        memset(buf, 0, sizeof(struct dma_buffer));

        buf->area = alloc_dmem(name, PAGE_ALIGN(size_max), (dma_addr_t *)(&buf->addr));
        if (!buf->area)
            return -ENOMEM;
        buf->bytes = PAGE_ALIGN(size_max);

        dev_info(ss_pcm->dev, "%s dma buffer size 0x%llx\n", name, buf->bytes);
        dev_info(ss_pcm->dev, "physical dma address 0x%08llx\n", buf->addr);
        dev_info(ss_pcm->dev, "miu address 0x%08llx\n", CamOsMemPhysToMiu(buf->addr));
        dev_info(ss_pcm->dev, "virtual dma address %p\n", buf->area);

        ss_pcm->rx.dma_id = ss_pcm->wdma;
#if SSTAR_PCM_IRQ_EN
        CamOsSnprintf((char *)ss_pcm->rx.name, sizeof(name), "ai_dma%d", (s16)ss_pcm->wdma);
        spin_lock_init(&ss_pcm->rx.irq_lock);
#endif
    }

    if (ss_pcm->rdma >= 0)
    {
        // AO buffer allocate
        buf = &ss_pcm->tx.buf;
        snprintf(name, 16, "pcmP%d", ss_pcm->rdma);
        memset(buf, 0, sizeof(struct dma_buffer));

        // RDMA0 use 1M memory for sound-bar.
        if (ss_pcm->rdma == 0)
        {
            size_max = 1024 * 1024;
        }

        buf->area = alloc_dmem(name, PAGE_ALIGN(size_max), (dma_addr_t *)(&buf->addr));
        if (!buf->area)
            return -ENOMEM;
        buf->bytes = PAGE_ALIGN(size_max);

        dev_info(ss_pcm->dev, "%s dma buffer size 0x%llx\n", name, buf->bytes);
        dev_info(ss_pcm->dev, "physical dma address 0x%08llx\n", buf->addr);
        dev_info(ss_pcm->dev, "miu address 0x%08llx\n", CamOsMemPhysToMiu(buf->addr));
        dev_info(ss_pcm->dev, "virtual dma address %p\n", buf->area);

        ss_pcm->tx.dma_id = ss_pcm->rdma;
#if SSTAR_PCM_IRQ_EN
        CamOsSnprintf((char *)ss_pcm->tx.name, sizeof(name), "ao_dma%d", (s16)ss_pcm->rdma);
        spin_lock_init(&ss_pcm->tx.irq_lock);
#endif
    }
    return ret;
}

static void sstar_pcm_remove(struct snd_soc_component *component)
{
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    char                     name[16];
    struct sstar_dma_buffer *buf;

    if (ss_pcm->wdma > 0)
    {
        buf = &ss_pcm->rx.buf;
        snprintf(name, 16, "pcmC%d", ss_pcm->wdma);
        free_dmem(name, buf->bytes, buf->area, buf->addr);
    }

    if (ss_pcm->rdma > 0)
    {
        buf = &ss_pcm->tx.buf;
        snprintf(name, 16, "pcmC%d", ss_pcm->rdma);
        free_dmem(name, buf->bytes, buf->area, buf->addr);
    }
}

static int sstar_pcm_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
    int                      ret    = 0;
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    struct sstar_pcm_stream *ss_pcm_stream;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
    }
    else
    {
        ss_pcm_stream = &ss_pcm->rx;
    }
    snd_soc_set_runtime_hwparams(substream, &sstar_hardware);

    ss_pcm_stream->dev           = ss_pcm->dev;
    ss_pcm_stream->substream     = substream;
    ss_pcm_stream->prev_frame    = 0;
    ss_pcm_stream->prev_hw_ptr   = 0;
    ss_pcm_stream->mmap          = 0;
    ss_pcm_stream->prev_appl_ptr = 0;
    ss_pcm_stream->trig_frame    = 0;

#if SSTAR_PCM_IRQ_EN
    ret = request_irq(ss_pcm->irq_id, sstar_snd_irq_handler, IRQF_SHARED, ss_pcm_stream->name, (void *)ss_pcm_stream);
    if (ret)
    {
        dev_err(ss_pcm->dev, "%s request irq[%d] err=%d\n", __FUNCTION__, ss_pcm->irq_id, ret);
        return ret;
    }
    ss_pcm_stream->dma_state     = SSTAR_DMA_INIT;
    ss_pcm_stream->runtime_state = SSTAR_PCM_RUNTIME_STATE_INIT;
#else
    hrtimer_init(&ss_pcm_stream->hrt, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    ss_pcm_stream->hrt.function = snd_hrtimer_callback;
    ss_pcm_stream->start_timer  = 0;
#endif

    return ret;
}

static int sstar_pcm_close(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
    int                      ret    = 0;
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    struct sstar_pcm_stream *ss_pcm_stream;
    dev_dbg(ss_pcm->dev, "[%s][%d]\n", __func__, __LINE__);

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
    }
    else
    {
        ss_pcm_stream = &ss_pcm->rx;
    }

#if SSTAR_PCM_DUMP_FILE
    if (mhal_audio_get_dts_value(DEBUG_LEVEL) & AUDIO_DBG_LEVEL_DUMP_PCM)
    {
        filp_close(substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? ss_pcm_stream->dump_file[0]
                                                                  : ss_pcm_stream->dump_file[1],
                   NULL);
    }
#endif

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        if (mhal_audio_ao_close(ss_pcm_stream->dma_id))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_close fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        dev_info(ss_pcm->dev, "[%s][%d] rdma [%d]\n", __func__, __LINE__, ss_pcm_stream->dma_id);
    }
    else
    {
        if (mhal_audio_ai_close(ss_pcm_stream->dma_id))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_close fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        dev_info(ss_pcm->dev, "[%s][%d] wdma [%d]\n", __func__, __LINE__, ss_pcm_stream->dma_id);
    }
#if SSTAR_PCM_IRQ_EN
    free_irq(ss_pcm->irq_id, (void *)ss_pcm_stream);
#else
    if (ss_pcm_stream->start_timer == 1)
    {
        ss_pcm_stream->start_timer = 0;
        hrtimer_cancel(&ss_pcm_stream->hrt);
    }
#endif
    return ret;
}

static int sstar_pcm_hw_params(struct snd_soc_component *component, struct snd_pcm_substream *substream,
                               struct snd_pcm_hw_params *hw_params)
{
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    struct sstar_pcm_stream *ss_pcm_stream;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
    }
    else
    {
        ss_pcm_stream = &ss_pcm->rx;
    }

    switch (sstar_params_dma_rate(hw_params))
    {
        case 8000:
        case 16000:
        case 32000:
        case 44100:
        case 48000:
            ss_pcm_stream->pcm.u32Rate = sstar_params_dma_rate(hw_params);
            break;
        default:
            ss_pcm_stream->pcm.u32Rate = params_rate(hw_params);
            break;
    }

    substream->runtime->buffer_size = params_buffer_size(hw_params);
    substream->runtime->format      = params_format(hw_params);
    substream->runtime->channels    = params_channels(hw_params);
    substream->runtime->frame_bits =
        snd_pcm_format_physical_width(params_format(hw_params)) * params_channels(hw_params);
    ss_pcm_stream->pcm.u16Width      = params_width(hw_params); // 16bit
    ss_pcm_stream->pcm.u16Channels   = params_channels(hw_params);
    ss_pcm_stream->pcm.bInterleaved  = 1;
    ss_pcm_stream->pcm.u32BufferSize = params_buffer_bytes(hw_params);
    ss_pcm_stream->pcm.u32PeriodSize = params_period_bytes(hw_params);
    if (ss_pcm_stream->pcm.u16Channels == 1)
    {
        ss_pcm_stream->pcm.u8IsOnlyEvenCh = 1;
    }
    else
    {
        ss_pcm_stream->pcm.u8IsOnlyEvenCh = 0;
    }

    ss_pcm_stream->pcm.u32PeriodCnt =
        (ss_pcm_stream->pcm.u32PeriodSize / (ss_pcm_stream->pcm.u16Width / 8)) / ss_pcm_stream->pcm.u16Channels;
    ss_pcm_stream->pcm.phyDmaAddr = CamOsMemPhysToMiu(ss_pcm_stream->buf.addr);
    ss_pcm_stream->pcm.pu8DmaArea = ss_pcm_stream->buf.area;

    // update dma info
    substream->runtime->dma_bytes = ss_pcm_stream->pcm.u32BufferSize;
    substream->runtime->dma_addr  = ss_pcm_stream->buf.addr;
    substream->runtime->dma_area  = ss_pcm_stream->buf.area;
    substream->dma_buffer.area    = ss_pcm_stream->buf.area;
    substream->dma_buffer.addr    = ss_pcm_stream->buf.addr;
    substream->dma_buffer.bytes   = params_buffer_bytes(hw_params);
    if (sstar_params_time_out(hw_params))
    {
        substream->wait_time = msecs_to_jiffies(sstar_params_time_out(hw_params));
    }

    dev_info(ss_pcm->dev,
             "rate[%d] chanl[%d], bufferbyte[%d], buffersize[%ld], periodbyte[%d], periodsize[%d], period[%d] %s[%d] "
             "bitwidth[%d]\n",
             ss_pcm_stream->pcm.u32Rate, ss_pcm_stream->pcm.u16Channels, ss_pcm_stream->pcm.u32BufferSize,
             substream->runtime->buffer_size, params_period_bytes(hw_params), params_period_size(hw_params),
             params_periods(hw_params), substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "rdma" : "wdma",
             ss_pcm_stream->dma_id, params_width(hw_params));

#if !SSTAR_PCM_IRQ_EN
    ss_pcm_stream->poll_time_ns = 1000000 * params_period_size(hw_params) / (ss_pcm_stream->pcm.u32Rate / 1000);
    dev_info(ss_pcm->dev, "poll_time_ns is =[%lld]= ns params_rate(hw_params) is %d params_period_size(hw_params) %d\n",
             ss_pcm_stream->poll_time_ns, ss_pcm_stream->pcm.u32Rate, params_period_size(hw_params));
#endif

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        if (mhal_audio_ao_config(ss_pcm_stream->dma_id, &ss_pcm_stream->pcm))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_config fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        if (mhal_audio_ao_open(ss_pcm_stream->dma_id))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_open fail\n", __func__, __LINE__);
            return -EFAULT;
        }
    }
    else
    {
        if (mhal_audio_ai_config(ss_pcm_stream->dma_id, &ss_pcm_stream->pcm))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_config fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        if (mhal_audio_ai_open(ss_pcm_stream->dma_id))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_open fail\n", __func__, __LINE__);
            return -EFAULT;
        }
    }

#if SSTAR_PCM_DUMP_FILE
    if (mhal_audio_get_dts_value(DEBUG_LEVEL) & AUDIO_DBG_LEVEL_DUMP_PCM)
    {
        char          file_name[64] = {0};
        struct file **fp            = substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? &(ss_pcm_stream->dump_file[0])
                                                                                     : &(ss_pcm_stream->dump_file[1]);
        sprintf(file_name, "/tmp/dump_%s_%d_%dch_%dbit.pcm",
                substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "ao" : "ai", ss_pcm_stream->pcm.u32Rate,
                substream->runtime->channels, ss_pcm_stream->pcm.u16Width);
        *fp = filp_open(file_name, O_RDWR | O_CREAT, 0644);
        if (IS_ERR(*fp))
        {
            dev_err(ss_pcm->dev, "[%s][%d] filp_open %s fail %ld\n", __func__, __LINE__, file_name, PTR_ERR(*fp));
        }
    }
#endif

    return 0;
}

static int sstar_pcm_trigger(struct snd_soc_component *component, struct snd_pcm_substream *substream, int cmd)
{
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    struct sstar_pcm_stream *ss_pcm_stream;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
    }
    else
    {
        ss_pcm_stream = &ss_pcm->rx;
    }

    dev_err(ss_pcm->dev, "==[%s][%d] cmd is ==[%s:%d] dma[%d]==\n", __func__, __LINE__,
            (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) ? ("playback") : ("capture"), cmd, ss_pcm_stream->dma_id);

    trace_trigger(ss_pcm_stream, cmd, 0);

    switch (cmd)
    {
        case SNDRV_PCM_TRIGGER_START:
            if (ss_pcm_stream->is_suspend && ss_pcm_stream->prev_hw_ptr != 0)
            {
                mhal_audio_ao_write_trig(ss_pcm_stream->dma_id, ss_pcm_stream->prev_hw_ptr);
                ss_pcm_stream->prev_hw_ptr = 0;
            }
            ss_pcm_stream->is_suspend = false;

            if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
            {
                dev_dbg(ss_pcm->dev, "appl_ptr = %ld\n", substream->runtime->control->appl_ptr);
                if (ss_pcm_stream->mmap && substream->runtime->control->appl_ptr == 0)
                {
                    ss_pcm_stream->component              = component;
                    substream->runtime->control->appl_ptr = substream->runtime->buffer_size / 2;
                    sstar_pcm_ack(component, substream);
                }
                if (mhal_audio_ao_start(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            else
            {
                if (ss_pcm_stream->mmap)
                {
                    ss_pcm_stream->component = component;
                }
                if (mhal_audio_ai_start(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
#if SSTAR_PCM_IRQ_EN
            {
                struct sstar_interupt_en irq_en = {0};
                irq_en.transmit_en              = TRUE;
                irq_en.boundary_en              = FALSE;
                irq_en.trigger_en               = FALSE;
                ss_pcm_stream->runtime_state    = SSTAR_PCM_RUNTIME_STATE_RUNNING;
                if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
                {
                    mhal_audio_ao_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
                }
                else
                {
                    mhal_audio_ai_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
                }
                ss_pcm_stream->dma_state = SSTAR_DMA_NORMAL;
            }
#else
            if (ss_pcm_stream->start_timer == 0)
            {
                ss_pcm_stream->start_timer = 1;
                hrtimer_start(&ss_pcm_stream->hrt, ns_to_ktime(ss_pcm_stream->poll_time_ns), HRTIMER_MODE_REL);
            }
#endif
            break;
        case SNDRV_PCM_TRIGGER_RESUME:
            if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
            {
                mhal_audio_ao_write_trig(ss_pcm_stream->dma_id, ss_pcm_stream->prev_hw_ptr);
                ss_pcm_stream->prev_hw_ptr = 0;
                if (mhal_audio_ao_resume(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
                mhal_audio_clear_int(substream, ss_pcm_stream->dma_id);
            }
            else
            {
                if (mhal_audio_ai_resume(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_resume fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
#if SSTAR_PCM_IRQ_EN
            {
                struct sstar_interupt_en irq_en = {0};
                irq_en.transmit_en              = TRUE;
                irq_en.boundary_en              = FALSE;
                irq_en.trigger_en               = FALSE;
                ss_pcm_stream->runtime_state    = SSTAR_PCM_RUNTIME_STATE_RUNNING;
                if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
                {
                    mhal_audio_ao_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
                }
                else
                {
                    mhal_audio_ai_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
                }
                ss_pcm_stream->dma_state = SSTAR_DMA_NORMAL;
            }
#else
            if (ss_pcm_stream->start_timer == 0)
            {
                ss_pcm_stream->start_timer = 1;
                hrtimer_start(&ss_pcm_stream->hrt, ns_to_ktime(ss_pcm_stream->poll_time_ns), HRTIMER_MODE_REL);
            }
#endif
            break;

        case SNDRV_PCM_TRIGGER_STOP:
            ss_pcm_stream->is_suspend = true;
            if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
            {
                if (mhal_audio_ao_preparetorestart(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_stop fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            else
            {
                if (mhal_audio_ai_preparetorestart(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            ss_pcm_stream->prev_appl_ptr = 0;
            ss_pcm_stream->prev_frame    = 0;
            ss_pcm_stream->prev_hw_ptr   = 0;
#if SSTAR_PCM_IRQ_EN
            {
                struct sstar_interupt_en irq_en = {0};
                irq_en.transmit_en              = FALSE;
                irq_en.boundary_en              = FALSE;
                irq_en.trigger_en               = FALSE;
                ss_pcm_stream->runtime_state    = SSTAR_PCM_RUNTIME_STATE_PAUSED;
                if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
                {
                    mhal_audio_ao_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
                }
                else
                {
                    mhal_audio_ai_interrupt_set(ss_pcm_stream->dma_id, &irq_en);
                }
                ss_pcm_stream->dma_state = SSTAR_DMA_NORMAL;
            }
#endif
            break;
        case SNDRV_PCM_TRIGGER_SUSPEND:
            ss_pcm_stream->is_suspend = true;
            if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
            {
                if (mhal_audio_ao_pause(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }

                // Copy data to the 0 position.
                {
                    char *buffer = (char *)kzalloc(frames_to_bytes(substream->runtime, substream->runtime->buffer_size),
                                                   GFP_KERNEL);
                    if (buffer)
                    {
                        ss_pcm_stream->prev_hw_ptr =
                            substream->runtime->control->appl_ptr - substream->runtime->status->hw_ptr;
                        if (ss_pcm_stream->prev_hw_ptr + ss_pcm_stream->prev_frame > substream->runtime->buffer_size)
                        {
                            memcpy(buffer,
                                   substream->runtime->dma_area
                                       + frames_to_bytes(substream->runtime, ss_pcm_stream->prev_frame),
                                   frames_to_bytes(substream->runtime,
                                                   substream->runtime->buffer_size - ss_pcm_stream->prev_frame));
                            memcpy(buffer
                                       + frames_to_bytes(substream->runtime,
                                                         substream->runtime->buffer_size - ss_pcm_stream->prev_frame),
                                   substream->runtime->dma_area,
                                   frames_to_bytes(substream->runtime, ss_pcm_stream->prev_hw_ptr
                                                                           - substream->runtime->buffer_size
                                                                           + ss_pcm_stream->prev_frame));
                        }
                        else
                        {
                            memcpy(buffer,
                                   substream->runtime->dma_area
                                       + frames_to_bytes(substream->runtime, ss_pcm_stream->prev_frame),
                                   frames_to_bytes(substream->runtime, ss_pcm_stream->prev_hw_ptr));
                        }

                        memcpy(substream->runtime->dma_area, buffer,
                               frames_to_bytes(substream->runtime, ss_pcm_stream->prev_hw_ptr));
                    }
                    kfree(buffer);
                }

                ss_pcm_stream->prev_frame             = 0;
                substream->runtime->status->hw_ptr    = 0;
                substream->runtime->control->appl_ptr = ss_pcm_stream->prev_hw_ptr;
                ss_pcm_stream->prev_appl_ptr          = substream->runtime->control->appl_ptr;
                substream->runtime->hw_ptr_base       = 0;
                substream->runtime->hw_ptr_interrupt  = 0;
            }
            else
            {
                substream->runtime->hw_ptr_interrupt = substream->runtime->status->hw_ptr;
                ss_pcm_stream->prev_frame            = 0;
                substream->runtime->status->hw_ptr   = substream->runtime->control->appl_ptr;
                if (substream->runtime->hw_ptr_base > substream->runtime->control->appl_ptr)
                {
                    substream->runtime->hw_ptr_base -= substream->runtime->buffer_size;
                }
                else if (substream->runtime->hw_ptr_base == 0
                         && substream->runtime->control->appl_ptr
                                > substream->runtime->boundary - substream->runtime->buffer_size)
                {
                    dev_info(ss_pcm->dev, "beyond appl_ptr[%ld], hw_ptr_base[%ld], boundary[%ld]\n",
                             substream->runtime->control->appl_ptr, substream->runtime->hw_ptr_base,
                             substream->runtime->boundary);
                    substream->runtime->hw_ptr_base = substream->runtime->boundary - substream->runtime->buffer_size;
                }
                if (mhal_audio_ai_pause(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] MHAL_AUDIO_AI_Resume Not support\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            break;
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            ss_pcm_stream->is_suspend = true;
            if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
            {
                if (mhal_audio_ao_pause(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            else
            {
                if (mhal_audio_ai_pause(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            break;
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            ss_pcm_stream->is_suspend = false;
            if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
            {
                if (mhal_audio_ao_resume(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            else
            {
                if (mhal_audio_ai_resume(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
            }
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static snd_pcm_uframes_t sstar_pcm_pointer(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
    int                      len;
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    struct sstar_pcm_stream *ss_pcm_stream;
    snd_pcm_uframes_t        frame = 0;
    snd_pcm_sframes_t        delta = 0;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
        if (mhal_audio_ao_get_curr_datalen(ss_pcm_stream->dma_id, &len))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_get_curr_datalen fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        if (ss_pcm_stream->dma_state == SSTAR_DMA_EMPTY)
        {
            len = 0;
        }
        else
        {
            // round up if level cnt not aligned by 16.
            len = roundup(len, (substream->runtime->frame_bits / 8));
            len = bytes_to_frames(substream->runtime, len);
        }
    }
    else
    {
        ss_pcm_stream = &ss_pcm->rx;
        if (mhal_audio_ai_get_curr_datalen(ss_pcm_stream->dma_id, &len))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_get_curr_datalen fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        len = bytes_to_frames(substream->runtime, len);
    }

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        // 1.for: playback
        //     delta = new_hw_ptr - hw_ptr
        //     hw_ptr               new_hw_ptr           appl_ptr
        //     ^                    ^                    ^
        //     |____________________|____________________|___________________|
        //     |<---    delta   --->|
        //                                               |<---   avail   --->|
        //                          |<---  dma_len   --->|
        //     |<---                     buffer_size                     --->|

        //     delta = buffer_size - dma_len - avail
        delta = substream->runtime->control->appl_ptr - substream->runtime->status->hw_ptr - len;
    }
    else
    {
        // 2.for: capture
        //     delta = new_hw_ptr - hw_ptr
        //     appl_ptr             hw_ptr               new_hw_ptr
        //     ^                    ^                    ^
        //     |____________________|____________________|___________________|
        //                          |<---    delta   --->|
        //     |<---    avail   --->|
        //     |<---              dma_len            --->|
        //     |<---                     buffer_size                     --->|

        //     delta = dma_len - avail
        delta = len - (substream->runtime->status->hw_ptr - substream->runtime->control->appl_ptr);
    }

    if (delta < 0 && delta + ss_pcm_stream->pcm.u32BufferSize > 0)
    {
        delta = 0;
    }

#if SSTAR_PCM_DUMP_FILE
    if ((mhal_audio_get_dts_value(DEBUG_LEVEL) & AUDIO_DBG_LEVEL_DUMP_PCM))
    {
        struct file *fp = NULL;
        mm_segment_t fs;
        fs = get_fs();
        set_fs(KERNEL_DS);
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        {
            fp = ss_pcm_stream->dump_file[0];
        }
        else
        {
            fp = ss_pcm_stream->dump_file[1];
        }
        if (!IS_ERR(fp) && delta > 0)
        {
            snd_pcm_uframes_t new_hw_ptr  = substream->runtime->status->hw_ptr + delta;
            snd_pcm_uframes_t old_hw_ptr  = substream->runtime->status->hw_ptr;
            snd_pcm_uframes_t buffer_size = substream->runtime->buffer_size;
            MS_U8 *           dma_area =
                ss_pcm_stream->pcm.pu8DmaArea + frames_to_bytes(substream->runtime, old_hw_ptr % buffer_size);

            if (new_hw_ptr / buffer_size > old_hw_ptr / buffer_size)
            {
                // Ring-buffer write beyond buffer_size.
                vfs_write(fp, dma_area, frames_to_bytes(substream->runtime, delta - new_hw_ptr % buffer_size),
                          &(fp->f_pos));
                vfs_write(fp, ss_pcm_stream->pcm.pu8DmaArea,
                          frames_to_bytes(substream->runtime, new_hw_ptr % buffer_size), &(fp->f_pos));
            }
            else
            {
                vfs_write(fp, dma_area, frames_to_bytes(substream->runtime, delta), &(fp->f_pos));
            }
        }
        set_fs(fs);
    }
#endif

    frame = ss_pcm_stream->prev_frame + delta;
    frame %= substream->runtime->buffer_size;
#if 1
    dev_dbg(ss_pcm->dev,
            "[%s][%d] prev_frame(%06ld), delta(%06ld), frame(%06ld), len(%06d), ptr(app:%06ld, hw:%06ld)\n", __func__,
            __LINE__, ss_pcm_stream->prev_frame, delta, frame, len, substream->runtime->control->appl_ptr,
            substream->runtime->status->hw_ptr);
    trace_pointer(ss_pcm_stream, frame, len, delta);
#endif
    ss_pcm_stream->prev_frame = frame;
    return frame;
}

/*
 * ALSA PCM mmap callback
 */
static int sstar_pcm_mmap(struct snd_soc_component *component, struct snd_pcm_substream *substream,
                          struct vm_area_struct *vma)
{
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    struct sstar_pcm_stream *ss_pcm_stream;

    dev_dbg(ss_pcm->dev, "[%s][%d]=\n", __func__, __LINE__);

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
    }
    else
    {
        ss_pcm_stream = &ss_pcm->rx;
    }
    ss_pcm_stream->mmap = 1;

    return remap_pfn_range(vma, vma->vm_start, ss_pcm_stream->buf.addr >> PAGE_SHIFT, vma->vm_end - vma->vm_start,
                           vma->vm_page_prot);
}

static int sstar_pcm_ack(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
    struct _sstar_dev *      p      = snd_soc_component_get_drvdata(component);
    struct sstar_pcm_engine *ss_pcm = &p->pcm_engine;
    struct sstar_pcm_stream *ss_pcm_stream;
    int                      len;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        ss_pcm_stream = &ss_pcm->tx;
    }
    else
    {
        ss_pcm_stream = &ss_pcm->rx;
    }

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        if (mhal_audio_ao_get_curr_datalen(ss_pcm_stream->dma_id, &len))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_get_curr_datalen fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        ss_pcm_stream->trig_frame = substream->runtime->control->appl_ptr - ss_pcm_stream->prev_appl_ptr;
        if (ss_pcm_stream->trig_frame < 0)
        {
            ss_pcm_stream->trig_frame = ss_pcm_stream->trig_frame % substream->runtime->buffer_size;
        }
        mhal_audio_ao_write_trig(ss_pcm_stream->dma_id, ss_pcm_stream->trig_frame);

        dev_dbg(ss_pcm->dev, "%s len:%d, app:%ld, hw:%ld\n", __func__, len, substream->runtime->control->appl_ptr,
                substream->runtime->status->hw_ptr);

#if SSTAR_PCM_IRQ_EN
        if (ss_pcm_stream->mmap)
        {
            if (bytes_to_frames(substream->runtime, len) + substream->runtime->control->appl_ptr
                    - ss_pcm_stream->prev_appl_ptr
                >= (bytes_to_frames(substream->runtime, (int)ss_pcm_stream->pcm.u32BufferSize)
                    - bytes_to_frames(substream->runtime, (int)ss_pcm_stream->pcm.u32PeriodSize)))
            {
                if (ss_pcm_stream->dma_state == SSTAR_DMA_EMPTY)
                {
                    if (mhal_audio_ao_start(ss_pcm_stream->dma_id))
                    {
                        dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_start fail\n", __func__, __LINE__);
                        return -EFAULT;
                    }
                    dev_dbg(ss_pcm->dev, "AO DMA %d restart\n", ss_pcm_stream->dma_id);
                }
                ss_pcm_stream->runtime_state = SSTAR_PCM_RUNTIME_STATE_RUNNING;
                ss_pcm_stream->dma_state     = SSTAR_DMA_NORMAL;
            }
            else
            {
                dev_dbg(ss_pcm->dev, "[%s][%d] [%ld]\n", __func__, __LINE__,
                        substream->runtime->control->appl_ptr - ss_pcm_stream->prev_appl_ptr);
            }
        }
        else if (len >= ss_pcm_stream->pcm.u32BufferSize - ss_pcm_stream->pcm.u32PeriodSize)
        {
            if (ss_pcm_stream->dma_state == SSTAR_DMA_EMPTY)
            {
                if (mhal_audio_ao_start(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_start Fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
                dev_dbg(ss_pcm->dev, "AO DMA id %d restart\n", ss_pcm_stream->dma_id);
            }

            ss_pcm_stream->runtime_state = SSTAR_PCM_RUNTIME_STATE_RUNNING;
            ss_pcm_stream->dma_state     = SSTAR_DMA_NORMAL;
        }
#endif
    }
    else
    {
        if (mhal_audio_ai_get_curr_datalen(ss_pcm_stream->dma_id, &len))
        {
            dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ao_get_curr_datalen fail\n", __func__, __LINE__);
            return -EFAULT;
        }
        ss_pcm_stream->trig_frame = substream->runtime->control->appl_ptr - ss_pcm_stream->prev_appl_ptr;
        if (ss_pcm_stream->trig_frame < 0)
        {
            ss_pcm_stream->trig_frame = ss_pcm_stream->trig_frame % substream->runtime->buffer_size;
        }
        if (ss_pcm_stream->trig_frame > bytes_to_frames(substream->runtime, len))
        {
            ss_pcm_stream->trig_frame = bytes_to_frames(substream->runtime, len);
        }
        mhal_audio_ai_read_trig(ss_pcm_stream->dma_id, ss_pcm_stream->trig_frame);
        dev_dbg(ss_pcm->dev, "appl_prt[%ld] pre%ld len[%ld] trig[%d]\n", substream->runtime->control->appl_ptr,
                ss_pcm_stream->prev_appl_ptr, bytes_to_frames(substream->runtime, len), ss_pcm_stream->trig_frame);
#if SSTAR_PCM_IRQ_EN
        if (ss_pcm_stream->mmap)
        {
            if (bytes_to_frames(substream->runtime, len)
                    - (substream->runtime->control->appl_ptr - ss_pcm_stream->prev_appl_ptr)
                <= bytes_to_frames(substream->runtime, (int)ss_pcm_stream->pcm.u32PeriodSize))
            {
                if (ss_pcm_stream->dma_state == SSTAR_DMA_FULL)
                {
                    if (mhal_audio_ai_start(ss_pcm_stream->dma_id))
                    {
                        dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_start fail\n", __func__, __LINE__);
                        return -EFAULT;
                    }
                    dev_dbg(ss_pcm->dev, "AI DMA %d restart\n", ss_pcm_stream->dma_id);
                }
                ss_pcm_stream->runtime_state = SSTAR_PCM_RUNTIME_STATE_RUNNING;
                ss_pcm_stream->dma_state     = SSTAR_DMA_NORMAL;
            }
        }
        else if (len - frames_to_bytes(substream->runtime, ss_pcm_stream->trig_frame)
                 <= ss_pcm_stream->pcm.u32PeriodSize)
        {
            if (ss_pcm_stream->dma_state == SSTAR_DMA_FULL)
            {
                if (mhal_audio_ai_start(ss_pcm_stream->dma_id))
                {
                    dev_err(ss_pcm->dev, "[%s][%d] mhal_audio_ai_start fail\n", __func__, __LINE__);
                    return -EFAULT;
                }
                dev_dbg(ss_pcm->dev, "AI DMA %d restart\n", ss_pcm_stream->dma_id);
            }
            ss_pcm_stream->runtime_state = SSTAR_PCM_RUNTIME_STATE_RUNNING;
            ss_pcm_stream->dma_state     = SSTAR_DMA_NORMAL;
        }
#endif
    }
    trace_ack(ss_pcm_stream, bytes_to_frames(substream->runtime, len));
    ss_pcm_stream->prev_appl_ptr = substream->runtime->control->appl_ptr;

    return 0;
}

int sstar_pcmengine_register(struct snd_soc_component_driver *sstar_component_driver)
{
    int ret = 0;

    sstar_component_driver->probe     = sstar_pcm_probe;
    sstar_component_driver->remove    = sstar_pcm_remove;
    sstar_component_driver->open      = sstar_pcm_open;
    sstar_component_driver->close     = sstar_pcm_close;
    sstar_component_driver->hw_params = sstar_pcm_hw_params;
    sstar_component_driver->trigger   = sstar_pcm_trigger;
    sstar_component_driver->pointer   = sstar_pcm_pointer;
    sstar_component_driver->mmap      = sstar_pcm_mmap;
    sstar_component_driver->ack       = sstar_pcm_ack;

    return ret;
}

int sstar_pcmengine_unregister(struct snd_soc_component_driver *sstar_component_driver)
{
    return 0;
}

// EXPORT_SYMBOL_GPL(sstar_pcmengine_register);
// EXPORT_SYMBOL_GPL(sstar_pcmengine_unregister);

MODULE_LICENSE("GPL v2");
