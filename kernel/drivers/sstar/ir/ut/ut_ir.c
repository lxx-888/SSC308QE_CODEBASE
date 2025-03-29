/*
 * ut_ir.c- Sigmastar
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

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/input.h>
#include <stdio.h>

static struct input_event data;

int main(int argc, char **argv)
{
    int   ret;
    int   count;
    int   ev_misc;
    int   fd_event;
    int   fd_group;
    void *buf;
    char  buf_event[64];
    char  buf_group[64];

    if (argc != 5)
    {
        printf("format: ut_ir [group] [event] [decode_mode] [count]\n");
        return -1;
    }

    snprintf(buf_group, sizeof(buf_group), "/sys/class/sstar/ir%d/decode_mode", atoi(argv[1]));
    fd_group = open(buf_group, O_RDWR);
    if (fd_group == -1)
    {
        printf("failed to open the ir file\n");
        return -1;
    }

    snprintf(buf_event, sizeof(buf_event), "/dev/input/%s", argv[2]);
    fd_event = open(buf_event, O_RDWR);
    if (fd_event == -1)
    {
        printf("failed to open the envent file\n");
        return -1;
    }

    buf = argv[3];
    ret = write(fd_group, buf, strlen(buf));
    if (ret < 0)
    {
        printf("failed to change ir decode mode\n");
        return ret;
    }

    count   = 0;
    ev_misc = 0;
    if (atoi(argv[4]) == 0)
    {
        printf("count can not be equal to 0\n");
        return -1;
    }

    while (1)
    {
        memset(&data, 0, sizeof(data));
        read(fd_event, &data, sizeof(data));
        switch (data.type)
        {
            case EV_SYN:
                break;
            case EV_KEY:
                if (ev_misc && data.value)
                {
                    printf("EV_KEY_DOWN:0x%04x\n", data.code);
                }
                else if (ev_misc && !data.value)
                {
                    printf("EV_KEY_UP:0x%04x\n", data.code);
                    count++;
                    ev_misc = 0;
                    if (count == atoi(argv[4]))
                        return 0;
                }
                break;
            case EV_MSC:
                printf("EV_MSC:0x%04x\n", data.value);
                ev_misc = 1;
            default:
                break;
        }
    }
    return 0;
}
