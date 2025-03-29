/*
 * hal_ddrfreq.c - Sigmastar
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
#include <linux/devfreq.h>
#include <linux/delay.h>
#include "registers.h"
#include "ms_platform.h"
#include "hal_ddrfreq.h"

typedef struct st_hal_ddrfreq
{
    /* sdram type */
    SSTAR_SDRAM_TYPE type;
    /* frequency parameters */
    unsigned long cur_freq;
    unsigned int  cur_ddfset;
    /* 4X/8X mode */
    unsigned int xmode;
    /* timing parameters */
    unsigned int t_refi;
    /* initial flag */
    bool initialized;
} hal_ddrfreq;

static hal_ddrfreq m_ddrfreq;
static DEFINE_SPINLOCK(ddrfreq_lock);

#if IS_ENABLED(CONFIG_MSTAR_MIU)
extern void miu_bw_get_max_load(ulong *busy, ulong *total);
#endif

static inline void hal_ddrfreq_ddfset(int from, int to, int step)
{
    if ((to >> 16) != (from >> 16))
    {
        /* NOTE: MSB word needs updating!!
         * due to ddfset registers can not be programmed by a single instruction,
         * we need to take the risk of unstable rates during transition.
         *
         * for example, when the rate goes DOWN and ddfset changes from 0x1af9dc to 0x1b01dc
         *     ddfset 0x1af9dc -> 2048M
         *     ddfset 0x1b01dc -> 2047M
         * if lsb word is updated first then msb word, the rates in transition might be over high
         *     ddfset 0x1af9dc -> 0x1a01dc -> 0x1b01dc
         *     rate      2048M ->    2126M ->    2047M
         * to avoid this situation, we need to update msb word first
         *     ddfset 0x1af9dc -> 0x1bf9dc -> 0x1b01dc
         *     rate      2048M ->    1976M ->    2047M
         */
        spin_lock_irq(&ddrfreq_lock);
        if (step > 0)
        {
            // update MSB word first to avoid a unexpected over-high frequency during tuning
            OUTREGMSK16(BASE_REG_ATOP_PA + REG_ID_19, (to >> 16) & 0xFF, 0x00FF);
            OUTREGMSK16(BASE_REG_ATOP_PA + REG_ID_18, to & 0xFFFF, 0xFFFF);
        }
        else
        {
            // update LSB word first
            OUTREGMSK16(BASE_REG_ATOP_PA + REG_ID_18, to & 0xFFFF, 0xFFFF);
            OUTREGMSK16(BASE_REG_ATOP_PA + REG_ID_19, (to >> 16) & 0xFF, 0x00FF);
        }
        spin_unlock_irq(&ddrfreq_lock);
    }
    else
    {
        OUTREGMSK16(BASE_REG_ATOP_PA + REG_ID_18, to & 0xFFFF, 0xFFFF);
    }
    udelay(1);
}

static int hal_ddrfreq_set_rate(unsigned int mhz)
{
    int          curr   = 0, temp;
    int          target = 0;
    int          step   = 0x800;
    int          diff;
    unsigned int refi = 0;

    if (m_ddrfreq.initialized && m_ddrfreq.cur_ddfset)
        curr = m_ddrfreq.cur_ddfset;
    else
        curr = (INREGMSK16(BASE_REG_ATOP_PA + REG_ID_19, 0x00FF) << 16) + INREG16(BASE_REG_ATOP_PA + REG_ID_18);
    target = ((432 * 4 * 4) << 19) / mhz;

    printk(KERN_DEBUG "ddfset x%x -> x%x, (tREFI %d)\r\n", curr, target, m_ddrfreq.t_refi);
    refi = (((m_ddrfreq.t_refi * mhz) / m_ddrfreq.xmode) >> 4) / 1000;

    if (target > curr)
    {
        diff = target - curr;
        /* target rate is slower, reduce the ticks of refresh interval first */
        OUTREGMSK16(BASE_REG_MIU_DIG + REG_ID_03, refi, 0x00FF);
    }
    else
    {
        diff = curr - target;
        step = -0x800;
    }

    temp = curr;
    do
    {
        curr = (diff >= 0x800) ? curr + step : target;
        hal_ddrfreq_ddfset(temp, curr, step);
        temp = curr;
        diff = (diff >= 0x800) ? diff - 0x800 : 0;
    } while (diff);

    if (step < 0)
    {
        /* target rate is faster, increase the ticks of refresh interval after transition completed */
        OUTREGMSK16(BASE_REG_MIU_DIG + REG_ID_03, refi, 0x00FF);
    }

    m_ddrfreq.cur_ddfset = target;
    m_ddrfreq.cur_freq   = mhz * 1000000;

    return 0;
}

int hal_ddrfreq_target(unsigned long freq)
{
    if (freq == m_ddrfreq.cur_freq)
        return 0;

    return hal_ddrfreq_set_rate((unsigned int)(freq / 1000000));
}

int hal_ddrfreq_get_load(struct devfreq_dev_status *stat)
{
    stat->current_frequency = m_ddrfreq.cur_freq;

#if IS_ENABLED(CONFIG_MSTAR_MIU)
    miu_bw_get_max_load(&stat->busy_time, &stat->total_time);
#else
    OUTREG16((BASE_REG_MIU_DIG + REG_ID_15), 0);                // reg_256_deb_sel: 0
    OUTREG16((BASE_REG_MIU_DIG + REG_ID_0D), 0);                // reset measure function, set client ID as overall
    OUTREG16((BASE_REG_MIU_DIG + REG_ID_29), 0);                // period: 1M cycle
    OUTREG16((BASE_REG_MIU_DIG + REG_ID_0D), (0x6 << 4) | 0x1); // max bandwidth/effi
    msleep_interruptible(50);
    stat->busy_time  = INREG16(BASE_REG_MIU_DIG + REG_ID_0E);
    stat->total_time = 1024;
    OUTREG16((BASE_REG_MIU_DIG + REG_ID_0D), 0); // reset measure function
#endif

    return 0;
}

unsigned long hal_ddrfreq_get_freq(void)
{
    unsigned int freq_mhz;

    if (m_ddrfreq.initialized && m_ddrfreq.cur_freq)
        return m_ddrfreq.cur_freq;

    m_ddrfreq.cur_ddfset =
        (INREGMSK16(BASE_REG_ATOP_PA + REG_ID_19, 0x00FF) << 16) + INREG16(BASE_REG_ATOP_PA + REG_ID_18);
    freq_mhz           = ((432 * 4 * 4) << 19) / m_ddrfreq.cur_ddfset;
    m_ddrfreq.cur_freq = freq_mhz * 1000000;

    m_ddrfreq.xmode  = 1 << (INREGMSK16(BASE_REG_MIU_DIG + REG_ID_01, 0x0300) >> 8);
    m_ddrfreq.t_refi = ((INREGMSK16(BASE_REG_MIU_DIG + REG_ID_03, 0xFF)) * 16 * m_ddrfreq.xmode * 1000) / freq_mhz;

    return m_ddrfreq.cur_freq;
}

SSTAR_SDRAM_TYPE hal_ddrfreq_get_sdram_type(void)
{
    unsigned short type;

    if (m_ddrfreq.initialized && m_ddrfreq.type)
        return m_ddrfreq.type;

    type = INREGMSK16(BASE_REG_MIU_DIG + REG_ID_01, 0x0003);
    if (type == 2)
        return SSTAR_DDR2_SDRAM;
    else if (type == 3)
        return SSTAR_DDR3_SDRAM;

    return SSTAR_SDRAM_TYPE_UNKNOWN;
}

void hal_ddrfreq_init(void)
{
    memset(&m_ddrfreq, 0, sizeof(hal_ddrfreq));
    m_ddrfreq.type        = hal_ddrfreq_get_sdram_type();
    m_ddrfreq.initialized = 1;
}
