/*
 * ddr_ott.c - Sigmastar
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
#include <stdlib.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <blk.h>
#include <part.h>
#include <u-boot/crc.h>

#include <asm/arch/mach/io.h>
#include <asm/arch/mach/platform.h>

#include <sstar_sys_utility.h>
#include <ddr_ott.h>

ddrtrain_info_t *read_ddrtrain_info(void)
{
    struct blk_desc *dev_desc;
#ifdef CONFIG_SSTAR_DDRTRAIN_INFO_STORE_IN_PARTITION
    struct disk_partition part_info;
#endif
    lbaint_t         blk_start;
    lbaint_t         blk_cnt;
    size_t           size_bytes;
    ddrtrain_info_t *train_info;

    if (blk_get_device_by_str(CONFIG_SSTAR_BOOT_DEV, simple_itoa(CONFIG_SSTAR_BOOT_DEV_ID), &dev_desc) < 0)
    {
        log_err("No such block device: '%s#%d' fail\n", CONFIG_SSTAR_BOOT_DEV, CONFIG_SSTAR_BOOT_DEV_ID);
        return NULL;
    }

#if defined(CONFIG_SSTAR_DDRTRAIN_INFO_STORE_IN_PARTITION)
    if (part_get_info_by_name(dev_desc, CONFIG_SSTAR_DDRTRAIN_INFO_PARTITION, &part_info) < 0)
    {
        log_err("No such partition: '%s'\n", CONFIG_SSTAR_DDRTRAIN_INFO_PARTITION);
        return NULL;
    }
    blk_start = part_info.start;
#else // defined(CONFIG_SSTAR_DDRTRAIN_INFO_STORE_IN_FIXED_BLOCK)
    blk_start = CONFIG_SSTAR_DDRTRAIN_INFO_BLOCK_OFFSET;
#endif
    blk_cnt    = DIV_ROUND_UP(sizeof(ddrtrain_info_t), dev_desc->blksz);
    size_bytes = blk_cnt * (dev_desc->blksz);

    train_info = (ddrtrain_info_t *)memalign(CONFIG_SYS_CACHELINE_SIZE, size_bytes);
    if (train_info == NULL)
    {
        log_err("Fail to alloc %#lx\n", size_bytes);
        return NULL;
    }

    log_debug("read %#lx blocks from offset %#lx ...\n", blk_cnt, blk_start);
    if (blk_cnt != blk_dread(dev_desc, blk_start, blk_cnt, (void *)train_info))
    {
        log_err("Fail to read block %#lx\n", blk_start);
        free((void *)train_info);
        return NULL;
    }

    return train_info;
}

int store_ddrtrain_info(ddrtrain_info_t *train_info)
{
    struct blk_desc *dev_desc;
#ifdef CONFIG_SSTAR_DDRTRAIN_INFO_STORE_IN_PARTITION
    struct disk_partition part_info;
#endif
    lbaint_t blk_start;
    lbaint_t blk_cnt;
    size_t   size_bytes;
    void *   io_data;

    if (NULL == train_info)
    {
        return -EINVAL;
    }

    if (blk_get_device_by_str(CONFIG_SSTAR_BOOT_DEV, simple_itoa(CONFIG_SSTAR_BOOT_DEV_ID), &dev_desc) < 0)
    {
        log_err("No such block device: '%s#%d' fail\n", CONFIG_SSTAR_BOOT_DEV, CONFIG_SSTAR_BOOT_DEV_ID);
        return -ENXIO;
    }

#if defined(CONFIG_SSTAR_DDRTRAIN_INFO_STORE_IN_PARTITION)
    if (part_get_info_by_name(dev_desc, CONFIG_SSTAR_DDRTRAIN_INFO_PARTITION, &part_info) < 0)
    {
        log_err("No such partition: '%s'\n", CONFIG_SSTAR_DDRTRAIN_INFO_PARTITION);
        return -EINVAL;
    }
    blk_start = part_info.start;
#else // defined(CONFIG_SSTAR_DDRTRAIN_INFO_STORE_IN_FIXED_BLOCK)
    blk_start = CONFIG_SSTAR_DDRTRAIN_INFO_BLOCK_OFFSET;
#endif
    blk_cnt    = DIV_ROUND_UP(sizeof(ddrtrain_info_t), dev_desc->blksz);
    size_bytes = blk_cnt * (dev_desc->blksz);

    log_info("blk start %d, cnt:%d, blksz:%d, cache line:0x%x\n", blk_start, blk_cnt, dev_desc->blksz,
             CONFIG_SYS_CACHELINE_SIZE);
    io_data = memalign(CONFIG_SYS_CACHELINE_SIZE, size_bytes);
    if (io_data == NULL)
    {
        log_err("Fail to alloc %#lx\n", size_bytes);
        return -ENOMEM;
    }
    memset(io_data, 0x00, size_bytes);

    memcpy(io_data, (void *)train_info, sizeof(ddrtrain_info_t));
    log_info("write %#lx blocks to offset %#lx ...\n", blk_cnt, blk_start);
    if (blk_cnt != blk_dwrite(dev_desc, blk_start, blk_cnt, io_data))
    {
        log_err("Fail to write block %#lx\n", blk_start);
        free(io_data);
        return -EIO;
    }

    free(io_data);
    return 0;
}

static int do_ddr_ott_erase(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    ddrtrain_info_t *train_info;

    train_info = (ddrtrain_info_t *)malloc(sizeof(ddrtrain_info_t));
    if (train_info == NULL)
    {
        log_err("Fail to alloc %#lx\n", sizeof(ddrtrain_info_t));
        return CMD_RET_FAILURE;
    }

    /* We recognize that zeroing out all fields is equivalent to erasing */
    memset((void *)train_info, 0x00, sizeof(ddrtrain_info_t));
    if (store_ddrtrain_info(train_info) < 0)
    {
        printf("Fail to store ddrtrain info\n");
        free((void *)train_info);
        return CMD_RET_FAILURE;
    }

    free((void *)train_info);
    return CMD_RET_SUCCESS;
}

static int do_ddr_ott_dump(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    ddrtrain_info_t *train_info;
    void *           target_address;

    if (argc != 2)
        return CMD_RET_USAGE;

    target_address = (ddrtrain_info_t *)simple_strtoul(argv[1], NULL, 16);

    train_info = read_ddrtrain_info();
    if (train_info == NULL)
    {
        log_err("Fail to read ddrtrain info\n");
        return CMD_RET_FAILURE;
    }

    memcpy(target_address, (void *)train_info, sizeof(ddrtrain_info_t));

    printf("Dump training info %#lx bytes to %#lx\n", sizeof(ddrtrain_info_t), (unsigned long)target_address);
    printf("flag: %x, chksum %x\n, version %x\n", train_info->flag.flag.enable, train_info->flag.chksum,
           train_info->flag.flag.version);
    for (int i = 0; i < MIU_CH_COUNT; i++)
    {
        printf("data: hdr %x %x, chksum %x\n", train_info->data[i].data.header, train_info->data[i].data.tail,
               train_info->data[i].chksum);
    }

    free((void *)train_info);
    return CMD_RET_SUCCESS;
}

static int do_ddr_ott_force(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    uint32_t         version = 0;
    ddrtrain_info_t *train_info;

    if (argc != 2)
        return CMD_RET_USAGE;

    train_info = read_ddrtrain_info();
    if (train_info == NULL)
    {
        log_err("Fail to read ddrtrain info\n");
        return CMD_RET_FAILURE;
    }
    version = train_info->flag.flag.version;
    memset((void *)(&train_info->flag), 0, sizeof(ott_flag_t));
    train_info->flag.flag.version = version;

    /* Set auto-K flag and update crc */
    if (simple_strtoul(argv[1], NULL, 10) == 1)
    {
        train_info->flag.flag.enable = FLAG_FORCE_AUTOK_ON;
    }
    else
    {
        train_info->flag.flag.enable = FLAG_FORCE_AUTOK_OFF;
    }
    train_info->flag.chksum = crc32(0, (void *)&train_info->flag.flag, sizeof(train_flag_t));

    /* Write-back ddrtrain info */
    if (store_ddrtrain_info(train_info) < 0)
    {
        printf("Fail to store ddrtrain info\n");
        free((void *)train_info);
        return CMD_RET_FAILURE;
    }

    free((void *)train_info);
    return CMD_RET_SUCCESS;
}

static int do_ddr_ott_mode(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    unsigned int mode = GET_DDR_INIT_MODE();

    switch (mode)
    {
        case FLAG_DDR_USE_DEFULT:
            printf("ddr initail with default settings\n");
            break;
        case FLAG_DDR_USE_TRAIN_DATA:
            printf("ddr initail with training data applied\n");
            break;
        case FLAG_DDR_RUN_AUTOK:
            printf("ddr initail with force training\n");
            break;
        default:
            printf("Invalid ddr training mode:%x\n", mode);
            break;
    }

    return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_ddr_ott_sub[] = {
    U_BOOT_CMD_MKENT(erase, 1, 1, do_ddr_ott_erase, "", ""),
    U_BOOT_CMD_MKENT(dump, 2, 1, do_ddr_ott_dump, "", ""),
    U_BOOT_CMD_MKENT(force, 2, 1, do_ddr_ott_force, "", ""),
    U_BOOT_CMD_MKENT(mode, 1, 1, do_ddr_ott_mode, "", ""),
};

static int do_ddr_ott(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    struct cmd_tbl *c;

    argc--;
    argv++;

    c = find_cmd_tbl(argv[0], cmd_ddr_ott_sub, ARRAY_SIZE(cmd_ddr_ott_sub));
    if (!c)
        return CMD_RET_USAGE;

    return c->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(ddr_ott, CONFIG_SYS_MAXARGS, 3, do_ddr_ott, "SigmaStar one-time DDR training function",
           "erase - erase DDR traning info\n"
           "ddr_ott dump <addr> - dump DDR traning info to addr\n"
           "ddr_ott force <0|1> - set DDR training flag to 0 or 1\n"
           "ddr_ott mode - show ddr initial mode\n");
