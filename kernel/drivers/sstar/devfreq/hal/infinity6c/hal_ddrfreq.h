/*
 * hal_ddrfreq.h - Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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
#ifndef HAL_DDRFREQ_H
#define HAL_DDRFREQ_H

#define SSTAR_DDRFREQ_PARAMS 0xc400001dUL
#define SSTAR_DDRFREQ_ADJUST 0xc400001eUL

#define DDRFREQ_TIMING_NUM      2
#define DDRFREQ_TIMING_IMI_BASE 0x39000UL

typedef enum
{
    SSTAR_SDRAM_TYPE_UNKNOWN = 0,
    SSTAR_DDR2_SDRAM,
    SSTAR_DDR3_SDRAM,
    SSTAR_DDR4_SDRAM,
    SSTAR_LPDDR4_SDRAM,
    SSTAR_SDRAM_TYPE_MAX,
} SSTAR_SDRAM_TYPE;

typedef struct sstar_dram_timing
{
    unsigned int ddr_freq;
    unsigned int ddr_synth;
} sstar_dram_timing_t;

int              hal_ddrfreq_target(unsigned long freq);
int              hal_ddrfreq_get_load(struct devfreq_dev_status *stat);
unsigned long    hal_ddrfreq_get_freq(void);
SSTAR_SDRAM_TYPE hal_ddrfreq_get_sdram_type(void);
void             hal_ddrfreq_init(void);

#endif
