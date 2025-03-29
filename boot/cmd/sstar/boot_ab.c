/*
 * boot_ab.c - Sigmastar
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
#include <env.h>

const static char *SLOT_SUFFIX[] = {"_a", "_b"};

static int ab_get_current_slot_select(void)
{
    char *slot_select_str = NULL;
    char *slot_number_str = NULL;

    slot_select_str = env_get("slot_select");
    slot_number_str = env_get("slot_number");

    if (!slot_select_str || !slot_number_str || *slot_select_str >= *slot_number_str)
    {
        return -EINVAL;
    }

    return (*slot_select_str - '0');
}

static int do_bootab(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    int   slot_select = 0;
    char  bootcmd[32];
    char *arg;

    slot_select = ab_get_current_slot_select();

    if (slot_select < 0 || slot_select > 1)
    {
        printf("No slot is available!!\n");
        return CMD_RET_FAILURE;
    }

    memset(bootcmd, 0, sizeof(bootcmd));
    sprintf(bootcmd, "bootcmd%s", SLOT_SUFFIX[slot_select]);

    arg = env_get(bootcmd);

    if (arg == NULL)
    {
        printf("## Error: \"%s\" not defined\n", bootcmd);
        return CMD_RET_FAILURE;
    }

    if (run_command(arg, flag | CMD_FLAG_ENV) != 0)
        return CMD_RET_FAILURE;

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(bootab, 5, 0, do_bootab, "Execute the Bootloader flow.", "");
