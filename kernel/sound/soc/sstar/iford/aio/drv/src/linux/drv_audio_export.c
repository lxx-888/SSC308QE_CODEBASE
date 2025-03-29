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

EXPORT_SYMBOL(mhal_audio_init);
EXPORT_SYMBOL(mhal_audio_deinit);
EXPORT_SYMBOL(mhal_audio_ai_if_setting);
EXPORT_SYMBOL(mhal_audio_ao_if_setting);
EXPORT_SYMBOL(mhal_audio_ai_config);
EXPORT_SYMBOL(mhal_audio_ao_config);
EXPORT_SYMBOL(mhal_audio_ai_if_gain);
EXPORT_SYMBOL(mhal_audio_ai_open);
EXPORT_SYMBOL(mhal_audio_ao_open);
EXPORT_SYMBOL(mhal_audio_ai_attach);
EXPORT_SYMBOL(mhal_audio_ai_detach);
EXPORT_SYMBOL(mhal_audio_ao_attach);
EXPORT_SYMBOL(mhal_audio_ao_detach);
EXPORT_SYMBOL(mhal_audio_ai_close);
EXPORT_SYMBOL(mhal_audio_ao_close);
EXPORT_SYMBOL(mhal_audio_ai_start);
EXPORT_SYMBOL(mhal_audio_ao_start);
EXPORT_SYMBOL(mhal_audio_ai_stop);
EXPORT_SYMBOL(mhal_audio_ao_stop);
EXPORT_SYMBOL(mhal_audio_ai_read_trig);
EXPORT_SYMBOL(mhal_audio_ao_write_trig);
EXPORT_SYMBOL(mhal_audio_ai_preparetorestart);
EXPORT_SYMBOL(mhal_audio_ao_preparetorestart);
EXPORT_SYMBOL(mhal_audio_ai_get_curr_datalen);
EXPORT_SYMBOL(mhal_audio_ao_get_curr_datalen);
EXPORT_SYMBOL(mhal_audio_ai_is_dmafree);
EXPORT_SYMBOL(mhal_audio_ao_is_dmafree);
EXPORT_SYMBOL(mhal_audio_ao_pause);
EXPORT_SYMBOL(mhal_audio_ao_resume);
EXPORT_SYMBOL(mhal_audio_ai_pause);
EXPORT_SYMBOL(mhal_audio_ai_resume);
EXPORT_SYMBOL(mhal_audio_ai_interrupt_get);
EXPORT_SYMBOL(mhal_audio_ao_interrupt_get);
EXPORT_SYMBOL(mhal_audio_ai_interrupt_set);
EXPORT_SYMBOL(mhal_audio_ao_interrupt_set);
EXPORT_SYMBOL(mhal_audio_ao_set_channel_mode);
EXPORT_SYMBOL(mhal_audio_set_audio_delay);
EXPORT_SYMBOL(mhal_audio_module_init);
EXPORT_SYMBOL(mhal_audio_mclk_setting);
EXPORT_SYMBOL(mhal_audio_module_deinit);
EXPORT_SYMBOL(mhal_audio_runtime_power_ctl);
EXPORT_SYMBOL(mhal_audio_ao_sidetone_regulate);
EXPORT_SYMBOL(mhal_audio_amp_state_get);
EXPORT_SYMBOL(mhal_audio_amp_state_set);
EXPORT_SYMBOL(mhal_audio_local_fifo_irq);
EXPORT_SYMBOL(mhal_audio_get_dts_value);
EXPORT_SYMBOL(mhal_audio_clear_int);

MODULE_LICENSE("GPL");
