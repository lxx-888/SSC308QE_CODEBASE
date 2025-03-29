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
#include <linux/delay.h>
#include "ms_platform.h"
#include "gpio.h"
#include "registers.h"
#include "mdrv_msys_io_st.h"
#include "../platform_msys.h"

#define MSYS_ERROR(fmt, args...) printk(KERN_ERR "MSYS: " fmt, ##args)
#define MSYS_WARN(fmt, args...)  printk(KERN_WARNING "MSYS: " fmt, ##args)

#define BIST_READ_ONLY  BIT5
#define BIST_WRITE_ONLY BIT6

#define BIST_SINGLE_MODE 0x0000
#define BIST_LOOP_MODE   0x0010

#define BIST_CMDLEN_1  0x0000
#define BIST_CMDLEN_8  0x0400
#define BIST_CMDLEN_16 0x0800
#define BIST_CMDLEN_64 0x0C00

#define BIST_BURSTLEN_16  0x0000
#define BIST_BURSTLEN_32  0x0100
#define BIST_BURSTLEN_64  0x0200
#define BIST_BURSTLEN_128 0x0300

U16 burst_len = BIST_CMDLEN_64 | BIST_BURSTLEN_128;

int msys_miu_check_cmd(const char *buf)
{
    char name[32];

    sscanf(buf, "%s", name);

    if (strcmp(name, "MIU_BIST_OFF") == 0)
        return 0;
    else if (strcmp(name, "MIU_BIST256_OCCUPY") == 0)
        return 0;
    else if (strcmp(name, "MIU_BIST_BW_RESTRICT_ALL") == 0)
        return 0;
    else if (strcmp(name, "MIU_BIST256_STOP") == 0)
        return 0;
    else if (strcmp(name, "MIU_PA_SET_MAX_SERVICE") == 0)
        return 0;
    else if (strcmp(name, "MIU_PREARB_SET_RCMD_WIN") == 0)
        return 0;
    else if (strcmp(name, "MIU_PREARB_SET_WCMD_WIN") == 0)
        return 0;

    return 1;
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

void msys_miu_ioctl(MSYS_DMEM_INFO mem_info, const char *buf)
{
    printk("Not support this command\r\n");
}
