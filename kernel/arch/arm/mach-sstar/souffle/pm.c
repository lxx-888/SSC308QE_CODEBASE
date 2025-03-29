/*
* pm.c- Sigmastar
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

#include <linux/suspend.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <asm/suspend.h>
#include <asm/fncpy.h>
#include <asm/cacheflush.h>
#include <asm/secure_cntvoff.h>

#include "ms_platform.h"
#include "registers.h"

#ifdef CONFIG_SSTAR_MCU
#include <drv_mcu.h>
#endif

#define SUSPEND_WAKEUP           0
#define SUSPEND_SLEEP            1

#ifdef CONFIG_ARM64
extern void sstar_flush_cache_all(void);
#endif
extern int psci_system_suspend(unsigned long unused);

int suspend_status = SUSPEND_WAKEUP;

typedef struct {
    char magic[8];
    uint64_t resume_entry;
    uint32_t count;
    uint32_t status;
    uint32_t password;
    uint32_t dram_crc;
    uint64_t dram_crc_start;
    uint64_t dram_crc_size;
} suspend_keep_info;

static uint64_t dram_crc_end;
static uint64_t dram_crc_start;
static suspend_keep_info *suspend_info;

static int __init suspend_crc_setup(char *param)
{
    if ((NULL == param) || ('\0' == *param))
    {
        return 0;
    }

    sscanf(param, "%llx,%llx", &dram_crc_start, &dram_crc_end);

    return 0;
}
early_param("suspend_crc", suspend_crc_setup);

static int sstar_suspend_ready(unsigned long ret)
{
    suspend_status = SUSPEND_SLEEP;

    //flush cache to ensure memory is updated before self-refresh
#ifdef CONFIG_ARM64
    sstar_flush_cache_all();
#else
    __cpuc_flush_kern_all();
#endif
    //flush tlb to ensure following translation is all in tlb
    local_flush_tlb_all();
    psci_system_suspend(0);
    return 0;
}

static int sstar_suspend_enter(suspend_state_t state)
{
    switch (state)
    {
        case PM_SUSPEND_MEM:
            printk(KERN_INFO "%s PM_SUSPEND_MEM\n", __func__);
            cpu_suspend(0, sstar_suspend_ready);
#ifdef CONFIG_SMP
            //secure_cntvoff_init();
#endif
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static void sstar_suspend_wake(void)
{
    suspend_status = SUSPEND_WAKEUP;
}

struct platform_suspend_ops sstar_suspend_ops = {
    .enter    = sstar_suspend_enter,
    .wake     = sstar_suspend_wake,
    .valid    = suspend_valid_only_mem,
};

int __init sstar_pm_init(void)
{
    printk(KERN_INFO "%s init ...\n", __func__);

    suspend_info = (suspend_keep_info *)__va(MIU0_BASE);

    if ((dram_crc_start >= MIU0_BASE) && (dram_crc_end > dram_crc_start))
    {
        suspend_info->dram_crc_start = dram_crc_start - MIU0_BASE;
        suspend_info->dram_crc_size = dram_crc_end - dram_crc_start;
    }
    else
    {
        suspend_info->dram_crc_size = 0;
    }

    suspend_set_ops(&sstar_suspend_ops);
    return 0;
}
