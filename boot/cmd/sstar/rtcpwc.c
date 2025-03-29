/*
 * rtcpwc.c - Sigmastar
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
#include <log.h>
#include <command.h>
#include <sstar_sys_utility.h>
#include <drv_rtcpwc.h>

int do_rtcpwc(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32 val;

    if (argc < 2 || argc > 3)
        return CMD_RET_USAGE;
    if (0 == strncmp(argv[1], "save_in_sw3", 11))
    {
        if (argc == 2)
        {
            printf("value save in sw3 = %d\n", sstar_rtc_sw3_get());
        }
        else if (argc == 3)
        {
            val = simple_strtoul(argv[2], NULL, 16);
            sstar_rtc_sw3_set(val);
        }
        else
        {
            return CMD_RET_USAGE;
        }
    }
    else
    {
        return CMD_RET_USAGE;
    }
    return 0;
}

U_BOOT_CMD(rtcpwc, 3, 0, do_rtcpwc, "rtcpwc command",
           "save_in_sw3 - get value save in sw3\n"
           "rtcpwc save_in_sw3 [n] - save value to sw3\n");
