/*
 * loados.c- Sigmastar
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
#include <image.h>
#include <asm/cache.h>
#include <mapmem.h>
#include <hal_xzdec.h>
#include <drv_xzdec.h>
#include <drv_part.h>
#include <mtd.h>

static int loados_get_part(const char *partname, loff_t *off, loff_t *size)
{
#ifdef CONFIG_CMD_MTDPARTS
    struct mtd_device *dev;
    struct part_info * part;
    u8                 pnum;
    int                ret;

    ret = mtdparts_init();
    if (ret)
        return ret;

    ret = find_dev_and_part(partname, &dev, &pnum, &part);
    if (ret)
        return ret;

    *off  = part->offset;
    *size = part->size;

    return 0;
#else
    puts("mtdparts support missing.\n");
    return -1;
#endif
}

static int do_loados(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    char *                endp;
    ulong                 image_head_size = image_get_header_size();
    const image_header_t *os_hdr          = NULL;
    u8 *                  head_buf        = NULL;
    u8 *                  load_buf        = NULL;
    u8 *                  comp_buf        = NULL;
    u32                   part_offset     = 0;
    ulong                 load_addr       = 0;
    ulong                 miu_addr        = 0;
    ulong                 image_size      = 0;
    struct sstar_part     part;
    xzdec_handle          handle = NULL;
    xzdec_buf             buf;
    u8                    loados_env_buf[64];

    if (argc < 5)
        return CMD_RET_FAILURE;

    if (strcmp(argv[1], "nand") == 0)
    {
        loff_t off;
        loff_t size;
        if (loados_get_part(argv[3], &off, &size))
        {
            printf("Failed to get %s partition!!!\n", argv[3]);
            goto ret;
        }

        if (!sstar_part_get_dev("nand0", &part))
        {
            printf("Failed to get nand0 device!!!\n");
            goto ret;
        }

        part.offset = off;
        part.size   = size;
    }
    else if (strcmp(argv[1], "nor") == 0)
    {
        loff_t off;
        loff_t size;
        if (loados_get_part(argv[3], &off, &size))
            goto ret;

        if (!sstar_part_get_dev("nor0", &part))
        {
            printf("Failed to get nor0 device!!!\n");
            goto ret;
        }

        part.offset = off;
        part.size   = size;
    }
    else
    {
        printf("no support storage type\n");
        return CMD_RET_FAILURE;
    }

    buf.out_size = 0;

    if ((strcmp(argv[1], "nand") == 0) || (strcmp(argv[1], "nor") == 0))
    {
        if (argc > 5)
        {
            part_offset = (u32)hextoul(argv[5], &endp);
            if (*argv[3] == 0 || *endp != 0)
                return CMD_RET_USAGE;
        }

        if (argc > 6)
        {
            miu_addr = (u32)hextoul(argv[6], &endp);
            if (*argv[6] == 0 || *endp != 0)
                return CMD_RET_USAGE;
        }

        head_buf = (u8 *)memalign(ARCH_DMA_MINALIGN, image_head_size);

        memset(head_buf, 0, image_head_size);

        if (!head_buf)
        {
            printk("os_hdr alloc memory fail!\n");
            goto ret;
        }

        if (image_head_size != sstar_part_load(&part, part_offset, head_buf, image_head_size))
        {
            printf("Error load header data!!!\n");
            goto ret;
        }

        os_hdr = (const image_header_t *)head_buf;

        if (image_check_magic(os_hdr))
        {
            load_addr  = image_get_load(os_hdr);
            image_size = image_get_data_size(os_hdr);
        }

        if (strcmp(argv[2], "by_header") != 0)
        {
            load_addr = (u32)hextoul(argv[2], &endp);
            if (*argv[2] == 0 || *endp != 0)
                return CMD_RET_USAGE;
        }

        if (strcmp(argv[4], "by_header") != 0)
        {
            image_size = (u32)hextoul(argv[4], &endp);
            if (*argv[4] == 0 || *endp != 0)
                return CMD_RET_USAGE;
        }

        if (!image_size)
        {
            printf("Image size is 0!!!\n");
            goto ret;
        }

        load_buf = map_sysmem(load_addr, 0);

        if (image_check_magic(os_hdr) && IH_COMP_LZMA2 == image_get_comp(os_hdr))
        {
            buf.select = sstar_part_is_support_xzdec(&part) ? XZDEC_BDMA_SPI_TO_DEC : XZDEC_BDMA_MIU_TO_DEC;

            if (miu_addr)
            {
                buf.select = XZDEC_BDMA_MIU_TO_DEC;
            }

            if (buf.select == XZDEC_BDMA_SPI_TO_DEC)
            {
                buf.in_part = &part;
                buf.in      = image_head_size + part_offset;
                buf.in_size = image_size;
                buf.out     = (u8 *)load_buf;
                buf.head    = 0;
            }
            else
            {
                buf.in_part = NULL;
                buf.in      = miu_addr + image_head_size;
                buf.in_size = image_size;
                buf.out     = (u8 *)load_buf;
                buf.head    = (u8 *)(unsigned long)(buf.in);

                if ((image_size + image_head_size)
                    != sstar_part_load(&part, part_offset, (u8 *)(unsigned long)(buf.in - image_head_size),
                                       image_size + image_head_size))
                    printf("loados fail!!!\n");
            }

            comp_buf = (u8 *)memalign(ARCH_DMA_MINALIGN, image_size);

            if (!comp_buf)
            {
                printk("comp_buf alloc memory fail!\n");
                goto ret;
            }

            if (XZDEC_SUCCESS != drv_xzdec_init(0, comp_buf))
            {
                printf("xzdec init port0 fail!\n");
                goto ret;
            }

            handle = drv_xzdec_get(0);

            if (XZDEC_SUCCESS != drv_xzdec_decode(handle, &buf))
                printf("sz decompression fail!!!\n");
        }
        else
        {
            if ((image_size + image_head_size)
                != sstar_part_load(&part, part_offset, load_buf, image_size + image_head_size))
                printf("loados fail!!!\n");

            buf.out_size = image_size + image_head_size;
        }
    }

    memset(loados_env_buf, 0, sizeof(loados_env_buf));
    sprintf(loados_env_buf, "0x%lx", load_addr);
    env_set("loados_addr", (const char *)loados_env_buf);

    memset(loados_env_buf, 0, sizeof(loados_env_buf));
    sprintf(loados_env_buf, "0x%x", buf.out_size);
    env_set("loados_size", (const char *)loados_env_buf);

ret:
    if (comp_buf)
        free((void *)comp_buf);

    if (head_buf)
        free((void *)head_buf);

    return CMD_RET_SUCCESS;
}

static char loados_help_text[] =
    "loados storage loados_addr/by_header partition size/by_header [partition_offset]\n    - \n";

U_BOOT_CMD(loados, CONFIG_SYS_MAXARGS, 1, do_loados, "Get images from various types of storage", loados_help_text);
