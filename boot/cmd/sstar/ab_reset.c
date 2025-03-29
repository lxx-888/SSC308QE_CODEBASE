/*
 * ab_reset.c - Sigmastar
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

#include <stdlib.h>
#include <common.h>
#include <android_ab.h>
#include <command.h>
#include <env.h>
#include <part.h>
#include <android_bootloader_message.h>

extern int ab_control_default(struct bootloader_control *abc);
extern int ab_control_create_from_disk(struct blk_desc *dev_desc, const struct disk_partition *part_info,
                                       struct bootloader_control **abc);
extern int ab_control_store(struct blk_desc *dev_desc, const struct disk_partition *part_info,
                            struct bootloader_control *abc);

static int do_ab_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    int                        ret;
    struct blk_desc *          dev_desc;
    struct disk_partition      part_info;
    struct bootloader_control *abc = NULL;

    if (argc != 3)
        return CMD_RET_USAGE;

    /* Lookup the "misc" partition from argv[1] and argv[2] */
    if (part_get_info_by_dev_and_name_or_num(argv[1], argv[2], &dev_desc, &part_info, false) < 0)
    {
        return CMD_RET_FAILURE;
    }

    ret = ab_control_create_from_disk(dev_desc, &part_info, &abc);
    if (ret < 0)
    {
        return CMD_RET_FAILURE;
    }

    ret = ab_control_default(abc);
    if (ret < 0)
    {
        goto end;
    }

    ab_control_store(dev_desc, &part_info, abc);
    if (ret < 0)
    {
        goto end;
    }

end:
    free(abc);
    return ret;
}

U_BOOT_CMD(ab_reset, 3, 0, do_ab_reset, "Execute the SimaStar init android control block.",
           "example:\n"
           "ab_reset mmc 0#misc\n");
