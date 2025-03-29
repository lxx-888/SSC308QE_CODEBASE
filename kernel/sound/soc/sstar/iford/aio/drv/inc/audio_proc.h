/*
 * audio_proc.h- Sigmastar
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

#ifndef __AUDIO_PROC_H__
#define __AUDIO_PROC_H__

typedef struct
{
    int enable;
    int channels;
    int bit_width;

} AudProcInfoAi_t;

typedef struct
{
    int enable;
    int channels;
    int bit_width;

} AudProcInfoAo_t;

typedef struct
{
    const char *           name;
    const struct proc_ops *ops;
} AudProcDir_t;
#define PROC_CONFIG(_name, _ops)   \
    {                              \
        .name = _name, .ops = _ops \
    }
#define PROC_DIR_NAME "audio"

extern AudProcInfoAi_t g_audProInfoAiList[];
extern AudProcInfoAo_t g_audProInfoAoList[];

int AudioProcInit(void);
int AudioProcDeInit(void);

#endif
