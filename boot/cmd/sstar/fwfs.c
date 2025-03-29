/*
 * fwfs.c - Sigmastar
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
#include <string.h>
#include <firmwarefs.h>

static int cmd_fwfs_read(char *file_path, char *dist_addr)
{
    int   file_len, ret;
    void *fd;

    fd = firmwarefs_open(file_path, O_RDONLY, 0);
    if (!fd)
    {
        printf("Fail open firmwarefs file:%s\n", file_path);
        return -ENOENT;
    }

    file_len = firmwarefs_lseek(fd, 0, SEEK_END);
    if (file_len <= 0)
    {
        printf("Fail get firmwarefs file len\n");
        goto close_exit;
    }

    ret = firmwarefs_read(fd, (void *)simple_strtoul(dist_addr, NULL, 16), file_len);
    if (ret <= 0)
    {
        printf("Fail read fwfs file %d\n", ret);
    }
    else
    {
        printf("Success read %d[%#x] bytes\n", ret, ret);
    }

close_exit:
    firmwarefs_close(fd);
    return 0;
}

static int cmd_fwfs_write(char *file_path, char *dist_addr)
{
    int   file_len, ret;
    void *fd;

    fd = firmwarefs_open(file_path, O_WRONLY, 0);
    if (!fd)
    {
        printf("Fail open firmwarefs file:%s\n", file_path);
        return -ENOENT;
    }

    file_len = firmwarefs_lseek(fd, 0, SEEK_SET);
    if (file_len < 0)
    {
        printf("Fail lseek firmwarefs file set\n");
        goto close_exit;
    }

    ret = firmwarefs_write(fd, dist_addr, strlen(dist_addr));
    if (ret <= 0)
    {
        printf("Fail write fwfs file %d\n", ret);
    }
    else
    {
        printf("Success write %d bytes\n", strlen(dist_addr));
    }

close_exit:
    firmwarefs_close(fd);
    return 0;
}

static int cmd_fwfs_create(char *file_path)
{
    void *fd;

    fd = firmwarefs_open(file_path, O_WRONLY | O_CREAT, 0);
    if (!fd)
    {
        printf("Fail create firmwarefs file:%s\n", file_path);
        return -ENOENT;
    }

close_exit:
    firmwarefs_close(fd);
    return 0;
}

static int cmd_fwfs_umount(void)
{
    firmwarefs_unmount();
    return 0;
}

static int cmd_fwfs_mount(char *part_name)
{
    int ret;
    ret = firmwarefs_mount(part_name, NULL);
    if (ret)
    {
        printf("Fail mount firmwarefs ret=%d\n", ret);
        return ret;
    }
    return 0;
}

static int do_fwfs_dispatch(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    if (argc < 2)
        return CMD_RET_USAGE;

    if (!strcmp(argv[1], "mount"))
    {
        return cmd_fwfs_mount(argv[2]);
    }

    if (!strcmp(argv[1], "umount"))
    {
        return cmd_fwfs_umount();
    }

    if (!strcmp(argv[1], "create"))
    {
        return cmd_fwfs_create(argv[2]);
    }

    if (!strcmp(argv[1], "read"))
    {
        return cmd_fwfs_read(argv[2], argv[3]);
    }

    if (!strcmp(argv[1], "write"))
    {
        return cmd_fwfs_write(argv[2], argv[3]);
    }

    return CMD_RET_USAGE;
}

U_BOOT_CMD(fwfs, 4, 0, do_fwfs_dispatch, "firmware filesystem support cmd",
           "mount MISC\n"
           "then:\n"
           "fwfs read /config.ini 0x21000000\n"
           "fwfs write /test hello,fwfs\n"
           "fwfs create /test\n"
           "fwfs umount\n");
