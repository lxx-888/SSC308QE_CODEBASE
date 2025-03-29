/*
 * aio.c - Sigmastar
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

#include <asm/arch/mach/sstar_types.h>
#include <command.h>
#include <common.h>
#include <malloc.h>

#include "include/cmd_sstar_common.h"

#define ARG_SCRIPT_FILE_INDEX 1

#define IS_ARG_AVAILABLE_SCRIPT_FILE(x) ((x) > ARG_SCRIPT_FILE_INDEX)
#define ARG_SCRIPT_FILE(x)              (x)[ARG_SCRIPT_FILE_INDEX]

extern void lineout_start(void);
// extern void lineout_gain(int nGain);

int do_lineout(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
#if 1
{
    int gain = 0;

    gain = simple_strtoul(argv[1], NULL, 0);
    // lineout_gain(gain);

    printf("audio line-out start\n");
    lineout_start();
    printf("audio line-out end\n");

    return 0;
}
#endif

U_BOOT_CMD(lineout, CONFIG_SYS_MAXARGS, 1, do_lineout, "audio line-out start\n", "");
