/*
 * debug.c - Sigmastar
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
#include <mapmem.h>
#include <asm/arch/mach/sstar_types.h>

int do_debug(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u16 *buf = map_sysmem(0x1F001C24, 2);

    *buf = *buf & ~BIT11;
    printf("\ndebug mode on, cmdline is disabled\n\n");

    return 0;
}

U_BOOT_CMD(debug, CONFIG_SYS_MAXARGS, 1, do_debug, "Disable uart rx via PAD_DDCA to use debug tool",
           "debug - disable uart rx for using debug tool");
