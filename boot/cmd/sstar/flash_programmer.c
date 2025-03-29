/*
 * flash_programmer.c- Sigmastar
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
#include <nand.h>

#define FLASH_PROGRAM_MAX_PART        (64)
#define FLASH_PROGRAM_MAX_IMG         (16)
#define FLASH_PROGRAM_IMG_BUFFER_SIZE (0x20000)

struct flash_program_part_info
{
    u32 part_number;
    u32 part_offset[FLASH_PROGRAM_MAX_PART];
    u32 part_size[FLASH_PROGRAM_MAX_PART];
    u32 image_offset[FLASH_PROGRAM_MAX_PART];
    u32 image_size[FLASH_PROGRAM_MAX_PART];
};

struct part_pase_tbl
{
    char *name;
    int (*pull_def)(ulong def_addr);
    int (*part_pase)(char *def, struct flash_program_part_info *info);
};

static bool flash_program_img_inited = false;
static char flash_program_img[FLASH_PROGRAM_MAX_IMG][64];

static int leap_pull_def(ulong def_addr)
{
    char pull_def_cmd[128] = {0};

    memset(pull_def_cmd, 0, sizeof(pull_def_cmd));
    sprintf(pull_def_cmd, "tftp 0x%lx %s", def_addr, "leap.def");
    printf(">>>>>>>> %s\n", pull_def_cmd);
    if (CMD_RET_SUCCESS != run_command(pull_def_cmd, 0))
        return 1;

    return 0;
}

static int part_pase_def_leap(char *def, struct flash_program_part_info *info)
{
    u8     i              = 0;
    u_char def_header[16] = {0x47, 0x52, 0x4f, 0x55, 0x50, 0x20, 0x44, 0x45,
                             0x46, 0x49, 0x4e, 0x45, 0x32, 0x0,  0x0,  0x0};
    u_char def_tail[16]   = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (0 != memcmp(def, def_header, 16))
    {
        printf("Failed to identify leap header!!!\n");
        return 1;
    }

    while (i < FLASH_PROGRAM_MAX_PART)
    {
        def += 16;

        if (!memcmp(def, def_tail, 16) || (*((ulong *)def) != 0x00000001))
            break;

        info->part_offset[i] = *((ulong *)(def + 0x4));
        info->part_size[i]   = *((ulong *)(def + 0x8)) - info->part_offset[i] + 1;
        info->image_size[i]  = *((ulong *)(def + 0xc));
        if (i == 0)
            info->image_offset[i] = 0;
        else
            info->image_offset[i] = info->image_offset[i - 1] + info->image_size[i - 1];

        i++;
    }

    info->part_number = i;

    if (0 != memcmp(def, def_tail, 16))
    {
        printf("Failed to identify leap tail!!!\n");
        return 1;
    }

    return 0;
}

static int snchk_pull_def(ulong def_addr)
{
    char pull_def_cmd[128] = {0};

    memset(pull_def_cmd, 0, sizeof(pull_def_cmd));
    sprintf(pull_def_cmd, "tftp 0x%lx %s", def_addr, "snchk.def");
    printf(">>>>>>>> %s\n", pull_def_cmd);
    if (CMD_RET_SUCCESS != run_command(pull_def_cmd, 0))
        return 1;

    return 0;
}

static int part_pase_def_snchk(char *def, struct flash_program_part_info *info)
{
    u8     i            = 0;
    u_char def_tail[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    while (i < FLASH_PROGRAM_MAX_PART)
    {
        if (!memcmp(def, def_tail, 16))
            break;

        info->part_offset[i] = *((ulong *)(def + 0x0));
        info->part_size[i]   = *((ulong *)(def + 0x4)) - info->part_offset[i] + 1;
        info->image_size[i]  = *((ulong *)(def + 0x8));
        if (i == 0)
            info->image_offset[i] = 0;
        else
            info->image_offset[i] = info->image_offset[i - 1] + info->image_size[i - 1];

        i++;
        def += 16;
    }

    info->part_number = i;

    if (0 != memcmp(def, def_tail, 16))
    {
        printf("Failed to identify snchk tail!!!\n");
        return 1;
    }

    return 0;
}

static struct part_pase_tbl flash_program_part_pase_tbl[] = {
    {"leap", leap_pull_def, part_pase_def_leap},
    {"snchk", snchk_pull_def, part_pase_def_snchk},
    {NULL, NULL, NULL},
};

static void flash_program_show_def_type(void)
{
    struct part_pase_tbl *def_pase = flash_program_part_pase_tbl;

    printf("flash_program support def type:\n");

    while (def_pase->name)
    {
        printf("\t\%s\n", def_pase->name);
        def_pase++;
    }
}

static int flash_program_parse_partition(const char *def_type, ulong def_addr, struct flash_program_part_info *info)
{
    char *                def_buf  = (char *)def_addr;
    struct part_pase_tbl *part_tbl = flash_program_part_pase_tbl;

    while (part_tbl->name)
    {
        if (strcmp(def_type, part_tbl->name) == 0)
            break;
        part_tbl++;
    }

    if (!part_tbl->name)
    {
        printf("no support def type : %s!!!\n", def_type);
        return 1;
    }

    if (0 != part_tbl->pull_def(def_addr))
    {
        printf("pull def fail!!!\n");
        return 1;
    }

    return part_tbl->part_pase(def_buf, info);
}

static int flash_program_pull_file(ulong image_addr, u8 file_node)
{
    char * file_size_str      = NULL;
    char   pull_file_cmd[128] = {0};
    loff_t file_size          = 0;

    if (!flash_program_img[file_node][0])
        return 0;

    memset(pull_file_cmd, 0, sizeof(pull_file_cmd));
    sprintf(pull_file_cmd, "tftp 0x%lx %s", image_addr, flash_program_img[file_node]);
    printf(">>>>>>>> %s\n", pull_file_cmd);
    if (CMD_RET_SUCCESS != run_command(pull_file_cmd, 0))
        return 0;

    file_size_str = env_get("filesize");

    if (!str2off(file_size_str, &file_size))
    {
        return 0;
    }

    return file_size;
}

static void flash_program_burn(ulong image_addr, struct flash_program_part_info *info)
{
    char             file_node = 0;
    u32              i;
    struct mtd_info *mtd;
    char             burn_cmd[128]   = {0};
    u32              left_file_size  = 0;
    u32              file_lseek      = 0;
    u32              write_offset    = 0;
    u32              left_write_size = 0;
    u32              write_size      = 0;

    mtd = get_nand_dev_by_index(nand_curr_device);

    if (!mtd)
        return;

    for (i = 0; i < info->part_number; i++)
    {
        info->part_offset[i] *= mtd->erasesize;
        info->part_size[i] *= mtd->erasesize;
        info->image_offset[i] *= mtd->erasesize;
        info->image_size[i] *= mtd->erasesize;
        left_write_size = info->image_size[i];

        memset(burn_cmd, 0, sizeof(burn_cmd));
        sprintf(burn_cmd, "nand erase 0x%x 0x%x", info->part_offset[i], info->part_size[i]);
        printf(">>>>>>>> %s\n", burn_cmd);
        if (CMD_RET_SUCCESS != run_command(burn_cmd, 0))
            return;

        write_offset = 0;

        while (left_write_size)
        {
            if (!left_file_size)
            {
                file_lseek     = 0;
                left_file_size = flash_program_pull_file(image_addr, file_node++);
                if (!left_file_size)
                    break;
            }

            write_size = (left_write_size > left_file_size) ? left_file_size : left_write_size;

            memset(burn_cmd, 0, sizeof(burn_cmd));
            sprintf(burn_cmd, "nand write 0x%lx 0x%x 0x%x", image_addr + file_lseek,
                    info->part_offset[i] + write_offset, write_size);
            printf(">>>>>>>> %s\n", burn_cmd);
            if (CMD_RET_SUCCESS != run_command(burn_cmd, 0))
                return;

            left_file_size -= write_size;
            file_lseek += write_size;
            left_write_size -= write_size;
            write_offset += write_size;
        }

        if (left_write_size)
        {
            printk("program fail!!!\n");
            break;
        }
    }
}

static int do_flash_program(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u8                             i = 0;
    const char *                   cmd;
    const char *                   def_type;
    ulong                          def_addr;
    ulong                          image_addr;
    struct flash_program_part_info info;
    char *                         endptr;

    /* need at least two arguments */
    if (argc < 2)
        return CMD_RET_USAGE;

    cmd = argv[1];
    argc -= 2;

    if (!strcmp(cmd, "info"))
    {
        flash_program_show_def_type();
    }
    else if (!strcmp(cmd, "img") && (argc < FLASH_PROGRAM_MAX_IMG))
    {
        memset(flash_program_img, 0, sizeof(flash_program_img));
        argv += 2;
        while ((argc > 0) && (i < FLASH_PROGRAM_MAX_IMG))
        {
            strcpy(flash_program_img[i++], *argv);
            --argc;
            ++argv;
        }
        flash_program_img_inited = true;
    }
    else if (flash_program_img_inited && !strcmp(cmd, "burn"))
    {
        if (argc < 3)
            return CMD_RET_FAILURE;

        def_type   = argv[2];
        def_addr   = simple_strtoul(argv[3], &endptr, 16);
        image_addr = simple_strtoul(argv[4], &endptr, 16);

        if (!flash_program_parse_partition(def_type, def_addr, &info))
            flash_program_burn(image_addr, &info);
        else
        {
            printf("flash_program_parse_partition fail!!!\n");
            return CMD_RET_FAILURE;
        }
    }

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(flash_program, CONFIG_SYS_MAXARGS, 1, do_flash_program, "flash_program sub-system",
           "info - show supporte def type\n"
           "img image_name [...] - Specifies the image, There can be multiple images\n"
           "flash_program burn def_type def_addr image_addr\n");
