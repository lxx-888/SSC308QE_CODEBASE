/*
 * cpu_wait.c - Sigmastar
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

#include <common.h>
#include <command.h>
#include <asm/mach-types.h>
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/io.h>

int do_cpu_wait(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u16                      wakeup_tag = CORE_WAKEUP_FLAG(0);
    volatile unsigned short *ptr        = (volatile unsigned short *)(CORE_WAKEUP_CONFIG_ADDR);

    *ptr &= ~(wakeup_tag);

    while ((*ptr & wakeup_tag) != wakeup_tag)
    {
        __asm__ __volatile__("wfe");
    }

    return 1;
}

U_BOOT_CMD(cpuwait, 1, 1, do_cpu_wait, "Do CPU wait", "");
