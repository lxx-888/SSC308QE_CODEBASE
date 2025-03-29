/*
 * rpmb.c - Sigmastar
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
#include <console.h>
#include <sstar_rpmb.h>

static int confirm_key_prog(void)
{
    puts(
        "Warning: Programming authentication key can be done only once !\n"
        "         Use this command only if you are sure of what you are doing,\n"
        "Really perform the key programming? <y/N> ");
    if (confirm_yesno())
        return 1;

    puts("Authentication key programming aborted\n");
    return 0;
}

static int do_rpmb_provision(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    if (!confirm_key_prog())
        return CMD_RET_FAILURE;
    if (sstar_rpmb_set_key())
    {
        printf("ERROR - Key already programmed ?\n");
        return CMD_RET_FAILURE;
    }
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(rpmb_provision, 1, 0, do_rpmb_provision, "Execute the SimaStar rpmb provision flow.", "rpmb_provisioned\n");
