/*
 * sstar_pcm_trace.h - Sigmastar
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

#undef TRACE_SYSTEM
#define TRACE_SYSTEM       sstar_pcm
#define TRACE_INCLUDE_FILE sstar_pcm_trace

#if !defined(_SSTAR_PCM_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _SSTAR_PCM_TRACE_H
#include <linux/tracepoint.h>
#include "sstar_pcm.h"
/*
 *  1.Make menuconfig:
 *      Kernel hacking --->
 *          [*]Tracers--->
 *          [*]Kernel Function Tracer
 *          [*]Trace syscalls
 *          [*]enable/disable function tracing dynamically
 *      Device Drivers --->
 *          Sound card support --->
 *          Advanced Linux Sound Architecture--->
 *          [*]Debug
 *          [*]Enable PCM ring buffer overun/underrun debugging
 *  2.Run on board:
 *      Init:
 *          echo 1 > /sys/kernel/debug/tracing/events/sstar_pcm/enable
 *          echo 0 > /sys/kernel/debug/tracing/trace
 *      Run tinyplay then:
 *          cat /sys/kernel/debug/tracing/trace
 */
TRACE_EVENT(ack, TP_PROTO(struct sstar_pcm_stream *sstar_stream, int level_cnt), TP_ARGS(sstar_stream, level_cnt),
            TP_STRUCT__entry(__field(unsigned int, card) __field(unsigned int, device) __field(unsigned int, number)
                                 __field(unsigned int, stream) __field(int, dma_id) __field(int, level_cnt)
                                     __field(int, trig_frame) __field(snd_pcm_uframes_t, appl_ptr)
                                         __field(snd_pcm_uframes_t, hw_ptr) __field(snd_pcm_uframes_t, prev_appl_ptr)
                                             __field(snd_pcm_uframes_t, hw_ptr_base)
                                                 __field(snd_pcm_uframes_t, hw_ptr_interrupt)),
            TP_fast_assign(__entry->card          = (sstar_stream)->substream->pcm->card->number;
                           __entry->device        = (sstar_stream)->substream->pcm->device;
                           __entry->number        = (sstar_stream)->substream->number;
                           __entry->dma_id        = (sstar_stream)->dma_id;
                           __entry->stream        = (sstar_stream)->substream->stream;
                           __entry->appl_ptr      = (sstar_stream)->substream->runtime->control->appl_ptr;
                           __entry->hw_ptr        = (sstar_stream)->substream->runtime->status->hw_ptr;
                           __entry->prev_appl_ptr = (sstar_stream)->prev_appl_ptr; __entry->level_cnt = level_cnt;
                           __entry->trig_frame       = (sstar_stream)->trig_frame;
                           __entry->hw_ptr_base      = (sstar_stream)->substream->runtime->hw_ptr_base;
                           __entry->hw_ptr_interrupt = (sstar_stream)->substream->runtime->hw_ptr_interrupt),
            TP_printk("\t\t[pcmC%dD%d%c/sub%d/dma%d]appl_ptr=%ld, hw_ptr=%ld, prev_appl_ptr=%ld, level_cnt=%d, "
                      "trig_frame=%d, total = %d, hw_ptr_base = %ld, hw_ptr_interrupt = %ld",
                      __entry->card, __entry->device, __entry->stream == SNDRV_PCM_STREAM_PLAYBACK ? 'p' : 'c',
                      __entry->number, __entry->dma_id, __entry->appl_ptr, __entry->hw_ptr, __entry->prev_appl_ptr,
                      __entry->level_cnt, __entry->trig_frame, __entry->level_cnt + __entry->trig_frame,
                      __entry->hw_ptr_base, __entry->hw_ptr_interrupt));

TRACE_EVENT(trigger, TP_PROTO(struct sstar_pcm_stream *sstar_stream, int cmd, int level_cnt),
            TP_ARGS(sstar_stream, cmd, level_cnt),
            TP_STRUCT__entry(__field(unsigned int, card) __field(unsigned int, device) __field(unsigned int, number)
                                 __field(unsigned int, stream) __field(int, dma_id) __field(int, cmd)
                                     __field(int, level_cnt) __field(snd_pcm_uframes_t, appl_ptr)
                                         __field(snd_pcm_uframes_t, hw_ptr) __field(snd_pcm_uframes_t, prev_frame)
                                             __field(snd_pcm_uframes_t, prev_appl_ptr)),
            TP_fast_assign(__entry->card   = (sstar_stream)->substream->pcm->card->number;
                           __entry->device = (sstar_stream)->substream->pcm->device;
                           __entry->number = (sstar_stream)->substream->number;
                           __entry->stream = (sstar_stream)->substream->stream;
                           __entry->dma_id = (sstar_stream)->dma_id; __entry->cmd = cmd; __entry->level_cnt = level_cnt;
                           __entry->appl_ptr      = (sstar_stream)->substream->runtime->control->appl_ptr;
                           __entry->hw_ptr        = (sstar_stream)->substream->runtime->status->hw_ptr;
                           __entry->prev_frame    = (sstar_stream)->prev_frame;
                           __entry->prev_appl_ptr = (sstar_stream)->prev_appl_ptr;),
            TP_printk("\t[pcmC%dD%d%c/sub%d/dma%d]cmd=%d, appl_ptr=%ld, hw_ptr=%ld, prev_appl_ptr=%ld, prev_frame = "
                      "%ld, level_cnt=%d",
                      __entry->card, __entry->device, __entry->stream == SNDRV_PCM_STREAM_PLAYBACK ? 'p' : 'c',
                      __entry->number, __entry->dma_id, __entry->cmd, __entry->appl_ptr, __entry->hw_ptr,
                      __entry->prev_appl_ptr, __entry->prev_frame, __entry->level_cnt));

TRACE_EVENT(pointer,
            TP_PROTO(struct sstar_pcm_stream *sstar_stream, snd_pcm_uframes_t pos, int level_cnt,
                     snd_pcm_uframes_t delta),
            TP_ARGS(sstar_stream, pos, level_cnt, delta),
            TP_STRUCT__entry(__field(unsigned int, card) __field(unsigned int, device) __field(unsigned int, number)
                                 __field(unsigned int, stream) __field(int, dma_id) __field(int, level_cnt)
                                     __field(snd_pcm_uframes_t, appl_ptr) __field(snd_pcm_uframes_t, hw_ptr)
                                         __field(snd_pcm_uframes_t, pos) __field(snd_pcm_uframes_t, delta)
                                             __field(snd_pcm_uframes_t, prev_frame)
                                                 __field(snd_pcm_uframes_t, prev_appl_ptr)
                                                     __field(snd_pcm_uframes_t, hw_ptr_base)
                                                         __field(snd_pcm_uframes_t, hw_ptr_interrupt)),
            TP_fast_assign(__entry->card   = (sstar_stream)->substream->pcm->card->number;
                           __entry->device = (sstar_stream)->substream->pcm->device;
                           __entry->number = (sstar_stream)->substream->number;
                           __entry->stream = (sstar_stream)->substream->stream;
                           __entry->dma_id = (sstar_stream)->dma_id; __entry->level_cnt = level_cnt;
                           __entry->appl_ptr = (sstar_stream)->substream->runtime->control->appl_ptr;
                           __entry->hw_ptr = (sstar_stream)->substream->runtime->status->hw_ptr; __entry->pos = pos;
                           __entry->delta = delta; __entry->prev_frame = (sstar_stream)->prev_frame;
                           __entry->prev_appl_ptr                      = (sstar_stream)->prev_appl_ptr;
                           __entry->hw_ptr_base      = (sstar_stream)->substream->runtime->hw_ptr_base;
                           __entry->hw_ptr_interrupt = (sstar_stream)->substream->runtime->hw_ptr_interrupt),
            TP_printk("\t[pcmC%dD%d%c/sub%d/dma%d]appl_ptr=%ld, hw_ptr=%ld, prev_appl_ptr=%ld, pos = %ld, prev_frame = "
                      "%ld, delta = %ld, level_cnt=%d, hw_ptr_base = %ld, hw_ptr_interrupt = %ld",
                      __entry->card, __entry->device, __entry->stream == SNDRV_PCM_STREAM_PLAYBACK ? 'p' : 'c',
                      __entry->number, __entry->dma_id, __entry->appl_ptr, __entry->hw_ptr, __entry->prev_appl_ptr,
                      __entry->pos, __entry->prev_frame, __entry->delta, __entry->level_cnt, __entry->hw_ptr_base,
                      __entry->hw_ptr_interrupt));
#endif /* _SSTAR_PCM_TRACE_H */
/* This part must be outside protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#include <trace/define_trace.h>
