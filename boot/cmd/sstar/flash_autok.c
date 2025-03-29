/*
 * flash_autok.c- Sigmastar
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
#include <malloc.h>
#include <asm/cache.h>
#include <mapmem.h>
#include <mtd.h>
#include <spi_flash_controller.h>

static char     autok_dev_name[16]      = {0};
static u8 *     autok_bdma_buf          = NULL;
static const u8 autok_parrtern_data[16] = {0x4d, 0x53, 0x54, 0x41, 0x52, 0x53, 0x45, 0x4d,
                                           0x49, 0x55, 0x53, 0x46, 0x44, 0x43, 0x49, 0x53};

static u8 flash_autok_parrtern_check(void)
{
    size_t           retlen = 0;
    struct mtd_info *mtd    = NULL;

    mtd = get_mtd_device_nm(autok_dev_name);

    if (IS_ERR(mtd))
    {
        return 0;
    }

    mtd_read(mtd, 0, 64, &retlen, autok_bdma_buf);

    if (!memcmp(autok_parrtern_data, autok_bdma_buf, 16))
    {
        return 1;
    }

    return 0;
}

static int do_flash_autok(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u8 *              name = NULL;
    struct mtd_info * mtd  = NULL;
    struct erase_info instr;
    size_t            retlen = 0;
    u8                bus    = 0;
    u8                cs     = 0;

    if (argc < 4)
        return CMD_RET_FAILURE;

    if (NULL == (name = (u8 *)kzalloc(8, GFP_KERNEL)))
    {
        printf("flash autok malloc fail!!\n");
        goto ret;
    }

    if (strcmp(argv[1], "nand") == 0)
    {
        strcpy((char *)name, "nand");
        name[4] = '0' + (bus << 1) + cs;
        mtd     = get_mtd_device_nm(name);

        if (IS_ERR(mtd))
        {
            printf("Failed to get nand%x device!!!\n", (bus << 1) + cs);
            goto ret;
        }

        strncpy(autok_dev_name, name, 8);
    }
    else if (strcmp(argv[1], "nor") == 0)
    {
        strcpy((char *)name, "nor");
        name[3] = '0' + (bus << 1) + cs;
        mtd     = get_mtd_device_nm(name);

        if (IS_ERR(mtd))
        {
            printf("Failed to get nor%x device!!!\n", (bus << 1) + cs);
            goto ret;
        }

        strncpy(autok_dev_name, name, 8);
    }
    else
    {
        printf("no support device %s\n", argv[1]);
        if (name)
        {
            kfree(name);
        }
        return CMD_RET_FAILURE;
    }

    bus = (u8)simple_strtoul(argv[2], NULL, 0);
    cs  = (u8)simple_strtoul(argv[3], NULL, 0);

    if (!autok_bdma_buf && NULL == (autok_bdma_buf = (u8 *)memalign(ARCH_DMA_MINALIGN, mtd->writesize)))
    {
        printf("flash autok malloc fail!!\n");
        goto ret;
    }

    memcpy(autok_bdma_buf, autok_parrtern_data, 16);

    instr.callback = NULL;
    instr.len      = mtd->erasesize;
    instr.addr     = 0;
    instr.mtd      = mtd;

    mtd_erase(mtd, &instr);
    mtd_write(mtd, 0, mtd->writesize, &retlen, autok_bdma_buf);

    if (spiflash_need_autok(spiflash_get_master(bus), cs))
    {
        spiflash_try_phase(spiflash_get_master(bus), flash_autok_parrtern_check);
    }

ret:
    if (mtd)
    {
        if (!IS_ERR(mtd))
        {
            put_mtd_device(mtd);
        }
    }

    if (name)
    {
        kfree(name);
    }

    return CMD_RET_SUCCESS;
}

static char flash_autok_help_text[] = "flash_autok nor/nand bus cs \n    - \n";

U_BOOT_CMD(flash_autok, CONFIG_SYS_MAXARGS, 1, do_flash_autok, "flash autok", flash_autok_help_text);
