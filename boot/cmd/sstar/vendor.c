/*
 * vendor.c- Sigmastar
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
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include <nand.h>
#include <jffs2/load_kernel.h>

#define VENDOR_MAGIC_MAX_SIZE 16
#define VENDOR_DATA_MAX_SIZE  64
#define VENDOR_DATA_MAX_ID    64

static u64 vendor_part_addr;
static u64 vendor_part_size;

typedef struct
{
    char magic[VENDOR_MAGIC_MAX_SIZE];
    int  id;
    int  len;
    char data[VENDOR_DATA_MAX_SIZE];
    char reserve[40];
} vendor_info;

vendor_info g_info[VENDOR_DATA_MAX_ID];
int         g_info_size;

static int get_vendor_partition_info(void)
{
#ifdef CONFIG_SSTAR_VENDOR_EMMC
    vendor_part_addr = 0x50;
    vendor_part_size = VENDOR_DATA_MAX_ID;
#else
    struct mtd_device *dev;
    struct part_info * part;
    unsigned char      pnum;

    if (mtdparts_init())
        return 1;

    if (find_dev_and_part("vendor_storage", &dev, &pnum, &part) != 0)
        return 1;

    vendor_part_addr = part->offset;
    vendor_part_size = sizeof(vendor_info) * VENDOR_DATA_MAX_ID;
#endif

#ifdef CONFIG_SSTAR_VENDOR_SPINOR
    run_command("sf probe", 0);
#endif
    return 0;
}

#ifdef CONFIG_SSTAR_VENDOR_EMMC
static int read_vendor_block(vendor_info *p_info, int id)
{
    char cmd[100];

    memset(cmd, 0, 100);

    sprintf(cmd, "mmc read %p 0x%llx 0x%x", p_info, vendor_part_addr + id, 0x1);
    return run_command(cmd, 0);
}

static int write_vendor_block(vendor_info *p_info, int id)
{
    char cmd[100];

    memset(cmd, 0, 100);
    sprintf(cmd, "mmc write %p 0x%llx 0x%x", p_info, vendor_part_addr + id, 0x1);
    return run_command(cmd, 0);
}
#else
static int erase_vendor_part(void)
{
    char cmd[100];

    memset(cmd, 0, 100);
#ifdef CONFIG_VENDOR_NAND
    sprintf(cmd, "nand erase 0x%llx 0x%llx", vendor_part_addr, vendor_part_size);
#else
    sprintf(cmd, "sf erase 0x%llx 0x%llx", vendor_part_addr, vendor_part_size);
#endif

    return run_command(cmd, 0);
}
#endif

static int read_vendor_part(vendor_info *p_info)
{
    char cmd[100];

    memset(cmd, 0, 100);
#ifdef CONFIG_SSTAR_VENDOR_EMMC
    sprintf(cmd, "mmc read %p 0x%llx 0x%llx", p_info, vendor_part_addr, vendor_part_size);
#elif CONFIG_VENDOR_NAND
    sprintf(cmd, "nand read %p 0x%llx 0x%llx", p_info, vendor_part_addr, vendor_part_size);
#else
    sprintf(cmd, "sf read %p 0x%llx 0x%llx", p_info, vendor_part_addr, vendor_part_size);
#endif
    return run_command(cmd, 0);
}

static int write_vendor_part(vendor_info *p_info)
{
    char cmd[100];

    memset(cmd, 0, 100);
#ifdef CONFIG_SSTAR_VENDOR_EMMC
    sprintf(cmd, "mmc write %p 0x%llx 0x%llx", p_info, vendor_part_addr, vendor_part_size);
#elif CONFIG_VENDOR_NAND
    sprintf(cmd, "nand write %p 0x%llx 0x%llx", p_info, vendor_part_addr, vendor_part_size);
#else
    sprintf(cmd, "sf write %p 0x%llx 0x%llx", p_info, vendor_part_addr, vendor_part_size);
#endif
    return run_command(cmd, 0);
}

void dump_info(char *szBuf, u32 nSize)
{
    int  i, j;
    int  cx         = 0;
    char szLine[80] = {0};

    printf(
        "\nOffset(h)  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F\n"
        "-----------------------------------------------------------\n");

    if ((u32)szBuf % 16)
    {
        cx = 0;
        cx += snprintf(szLine + cx, sizeof(szLine) - cx, "%08X  ", (u32)szBuf & 0xFFFFFFF0);

        for (i = 0; i < (u32)szBuf % 16; i++)
        {
            cx += snprintf(szLine + cx, sizeof(szLine) - cx, "   ");
            szLine[i + 62] = ' ';
            if (i % 8 == 0)
            {
                cx += snprintf(szLine + cx, sizeof(szLine) - cx, " ");
            }
        }
    }

    for (i = 0; i < nSize; i++)
    {
        if ((i + (u32)szBuf) % 16 == 0)
        {
            cx = 0;
            cx += snprintf(szLine + cx, sizeof(szLine) - cx, "%08X  ", (u32)szBuf + i);
        }
        if ((i + (u32)szBuf) % 8 == 0)
        {
            cx += snprintf(szLine + cx, sizeof(szLine) - cx, " ");
        }

        cx += snprintf(szLine + cx, sizeof(szLine) - cx, "%02X ", szBuf[i]);

        if (((unsigned char *)szBuf)[i] >= ' ' && ((unsigned char *)szBuf)[i] <= '~')
        {
            szLine[(i + (u32)szBuf) % 16 + 62] = ((unsigned char *)szBuf)[i];
        }
        else
        {
            szLine[(i + (u32)szBuf) % 16 + 62] = '.';
        }

        if ((i + (u32)szBuf) % 16 == 15)
        {
            szLine[59] = ' ';
            szLine[60] = ' ';
            szLine[61] = '|';
            szLine[78] = '|';
            szLine[79] = 0;
            printf("%s\n", szLine);
        }
        else if (i == nSize - 1)
        {
            for (j = ((i + (u32)szBuf) + 1) % 16; j < 16; j++)
            {
                cx += snprintf(szLine + cx, sizeof(szLine) - cx, "   ");
                szLine[j + 62] = ' ';
            }
            if (((i + (u32)szBuf) + 1) % 16 <= 8)
            {
                cx += snprintf(szLine + cx, sizeof(szLine) - cx, " ");
            }
            szLine[59] = ' ';
            szLine[60] = ' ';
            szLine[61] = '|';
            szLine[78] = '|';
            szLine[79] = 0;
            printf("%s\n", szLine);
        }
    }
    printf("\n");
}

int usb_read_vendor_info(u8 *buf)
{
    memcpy(buf, g_info, g_info_size);

    printf("[UFU read vendor info] magic:%s, id:%d, len:%d, data:%s\n", g_info[0].magic, g_info[0].id, g_info[0].len,
           g_info[0].data);

    return g_info_size;
}

static int do_vendor(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    unsigned char id;
    unsigned char len;
    vendor_info * info;
    int           ret = CMD_RET_SUCCESS;

    if ((info = (vendor_info *)memalign(ARCH_DMA_MINALIGN, sizeof(vendor_info) * VENDOR_DATA_MAX_ID)) == NULL)
    {
        printf("fail info mem fail\n");
        return CMD_RET_FAILURE;
    }
    memset(info, 0x0, sizeof(vendor_info) * VENDOR_DATA_MAX_ID);

    if (get_vendor_partition_info())
    {
        ret = CMD_RET_FAILURE;
        goto end;
    }

    if (strcmp(argv[1], "readall") == 0 && argc == 2)
    {
        if ((ret = read_vendor_part(info)) != CMD_RET_SUCCESS)
        {
            goto end;
        }

        memcpy(g_info, info, sizeof(vendor_info) * VENDOR_DATA_MAX_ID);
        g_info_size = sizeof(vendor_info) * VENDOR_DATA_MAX_ID;
    }
    else if (strcmp(argv[1], "read") == 0 && argc == 3)
    {
        id = (int)simple_strtoul(argv[2], NULL, 10);
        if (id >= VENDOR_DATA_MAX_ID)
        {
            ret = CMD_RET_USAGE;
            goto end;
        }
#ifdef CONFIG_SSTAR_VENDOR_EMMC
        vendor_info *temp = info + id;

        if ((ret = read_vendor_block(temp, id)) != CMD_RET_SUCCESS)
        {
            goto end;
        }
        memcpy(g_info, temp, sizeof(vendor_info));
#else
        if ((ret = read_vendor_part(info)) != CMD_RET_SUCCESS)
        {
            goto end;
        }
        memcpy(g_info, info + id, sizeof(vendor_info));
#endif
        g_info_size = sizeof(vendor_info);
    }
    else if (strcmp(argv[1], "write") == 0 && argc == 6)
    {
        id = (int)simple_strtoul(argv[3], NULL, 10);
        if (id >= VENDOR_DATA_MAX_ID)
        {
            ret = CMD_RET_USAGE;
            goto end;
        }

        len = (int)simple_strtoul(argv[4], NULL, 10);
        if (len > VENDOR_DATA_MAX_SIZE)
        {
            ret = CMD_RET_USAGE;
            goto end;
        }

#ifdef CONFIG_SSTAR_VENDOR_EMMC
        memset(&info[0].magic, 0x0, 16);
        memcpy(&info[0].magic, argv[2], strlen(argv[2]));
        info[0].id  = id;
        info[0].len = len;
        memset(&info[0].data, 0x0, VENDOR_DATA_MAX_SIZE);
        memcpy(&info[0].data, argv[5], len);
        if ((ret = write_vendor_block(&info[0], id)) != CMD_RET_SUCCESS)
        {
            goto end;
        }
#else
        if ((ret = read_vendor_part(info)) != CMD_RET_SUCCESS)
        {
            goto end;
        }

        if ((ret = erase_vendor_part()) != CMD_RET_SUCCESS)
        {
            goto end;
        }

        memset(&info[id].magic, 0x0, 16);
        memcpy(&info[id].magic, argv[2], strlen(argv[2]));
        info[id].id = id;
        info[id].len = len;
        memset(&info[id].data, 0x0, VENDOR_DATA_MAX_SIZE);
        memcpy(&info[id].data, argv[5], len);

        if ((ret = write_vendor_part(info)) != CMD_RET_SUCCESS)
        {
            goto end;
        }
#endif
    }
    else if (strcmp(argv[1], "erase") == 0 && argc <= 3)
    {
        if (argc == 3)
        {
            id = (int)simple_strtoul(argv[2], NULL, 10);
            if (id >= VENDOR_DATA_MAX_ID)
            {
                ret = CMD_RET_USAGE;
                goto end;
            }
#ifdef CONFIG_SSTAR_VENDOR_EMMC
            if (read_vendor_block(info, id))
            {
                ret = CMD_RET_FAILURE;
                goto end;
            }
            memset(info, 0x0, sizeof(vendor_info));
            if (write_vendor_block(info, id))
            {
                ret = CMD_RET_FAILURE;
                goto end;
            }
#else
            if (read_vendor_part(info))
            {
                ret = CMD_RET_FAILURE;
                goto end;
            }
            if ((ret = erase_vendor_part()) != CMD_RET_SUCCESS)
            {
                goto end;
            }
            memset(info + id, 0x0, sizeof(vendor_info));
            if (write_vendor_part(info))
            {
                ret = CMD_RET_FAILURE;
                goto end;
            }
#endif
        }
        else
        {
        erase_all:
#ifdef CONFIG_SSTAR_VENDOR_EMMC
            if (read_vendor_part(info))
            {
                ret = CMD_RET_FAILURE;
                goto end;
            }
            memset(info, 0x0, sizeof(vendor_info) * VENDOR_DATA_MAX_ID);
            if (write_vendor_part(info))
            {
                ret = CMD_RET_FAILURE;
                goto end;
            }
#else
            if ((ret = erase_vendor_part()) != CMD_RET_SUCCESS)
            {
                goto end;
            }
#endif
        }
    }
    else if (strcmp(argv[1], "eraseall") == 0)
    {
        goto erase_all;
    }
    else
    {
        ret = CMD_RET_USAGE;
    }

end:
    free(info);
    return ret;
}

U_BOOT_CMD(vendor, CONFIG_SYS_MAXARGS, 1, do_vendor, "vendor partition",
           "read [id]  - read info\n"
           "write magic id len data  - write info\n"
           "vendor erase [id]  - erase info\n");
