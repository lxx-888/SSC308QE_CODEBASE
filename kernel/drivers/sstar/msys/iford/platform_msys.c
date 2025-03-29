/*
 * platform_msys.c- Sigmastar
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

#include <asm/uaccess.h>
#include <linux/errno.h>
#include "ms_platform.h"
#include <asm/pgtable.h>
#include "gpio.h"
#include "registers.h"
#include "mdrv_msys_io_st.h"
#include "../platform_msys.h"

#define MSYS_ERROR(fmt, args...) printk(KERN_ERR "MSYS: " fmt, ##args)
#define MSYS_WARN(fmt, args...)  printk(KERN_WARNING "MSYS: " fmt, ##args)

int msys_miu_check_cmd(const char *buf)
{
    return 0;
}

int msys_miu_set_bist(U8 enable)
{
    MSYS_WARN("Not support this command\r\n");
    return 0;
}

int msys_miu_client_switch(msys_miu_client_sw_e sw)
{
    switch (sw)
    {
        case MSYS_MIU_CLIENT_SW_BIST:
            break;
        case MSYS_MIU_CLIENT_SW_TVTOOL:
            break;
        default:
            return -1;
    }

    return 0;
}

int msys_request_freq(MSYS_FREQGEN_INFO *freq_info)
{
    if (freq_info->padnum != PAD_CLOCK_OUT)
    {
        MSYS_ERROR("Not implement PAD: %d for this platform\n", freq_info->padnum);
        return -EINVAL;
    }

    if (freq_info->bEnable)
    {
        if (freq_info->freq != FREQ_24MHZ)
        {
            MSYS_ERROR("Not implement FREQ: %d for this platform\n", freq_info->freq);
            return -EINVAL;
        }

        if (freq_info->bInvert != false)
            MSYS_WARN("Not support invert clock in this platform");

        switch (freq_info->padnum)
        {
            case PAD_CLOCK_OUT:
                // reg_ext_xtali_se_enb[1]=0
                CLRREG16(BASE_REG_XTAL_ATOP_PA + REG_ID_06, BIT0); // enable clock
                break;
            default:
                MSYS_ERROR("Not implement PAD: %d for this platform\n", freq_info->padnum);
                break;
        }
    }
    else // disable clk
    {
        switch (freq_info->padnum)
        {
            case PAD_CLOCK_OUT:
                // reg_ext_xtali_se_enb[1]=1
                SETREG16(BASE_REG_XTAL_ATOP_PA + REG_ID_06, BIT1); // disable clk
                break;
            default:
                MSYS_ERROR("Not implement PAD: %d for this platform\n", freq_info->padnum);
                break;
        }
    }
    return 0;
}

extern unsigned long long notrace sched_clock(void);

// mode: 0-> r/w, 1-> r
void miu_bist(u64 start_addr, u64 length, u16 mode, u32 grp_base, u32 effi_base)
{
    unsigned int timer0, timer1;
    printk("Set bist start %llx\r\n", start_addr);
    OUTREG16(effi_base + (0x50 << 2), 0x00); // clear bist
    // read mode loop
    if (mode == 1)
    {
        // set 1 to access BIST
        OUTREGMSK16(grp_base + (0x59 << 2), BIT15, BIT15 | BIT14);
        OUTREG16(effi_base + (0x50 << 2), 0x0C30);
        //        OUTREG16(effi_base + (0x50 << 2), 0x0430);
    }
    else if (mode == 2)
    {
        printk("one shot bist\r\n");
        OUTREGMSK16(grp_base + (0x59 << 2), BIT15 | BIT14, BIT15 | BIT14); // 0xF Bist mux
        OUTREG16(effi_base + (0x50 << 2), 0x0400);                         // one shot
    }
    else
    {
        // set 1 to access BIST
        OUTREGMSK16(grp_base + (0x59 << 2), BIT15 | BIT14, BIT15 | BIT14);
        OUTREG16(effi_base + (0x50 << 2), 0x0410);
    }
    // test data byte
    OUTREG16(effi_base + (0x51 << 2), 0xffff);

    // test start address
    OUTREG16(effi_base + (0x57 << 2), (start_addr >> 10) & 0xFFFF);
    OUTREG16(effi_base + (0x58 << 2), (start_addr >> 26) & 0xFFFF);

    // test start length
    OUTREG16(effi_base + (0x54 << 2), (length >> 4) & 0xFFFF);
    OUTREG16(effi_base + (0x55 << 2), (length >> 20) & 0xFFFF);

    // trigger bist
    SETREG16(effi_base + (0x50 << 2), BIT0);

    if (mode == 2)
    {
        timer0 = sched_clock();
        while (1)
        {
            if ((INREG16(effi_base + (0x59 << 2)) & 0xFF) == 0x14)
            {
                timer1 = sched_clock();
                break;
            }
        }

        timer1 = timer1 - timer0;
        printk("MIU_Bist time: %d(ns)\r\n", timer1);
    }
}

void msys_miu_ioctl(MSYS_DMEM_INFO mem_info, const char *buf)
{
    return;
}
