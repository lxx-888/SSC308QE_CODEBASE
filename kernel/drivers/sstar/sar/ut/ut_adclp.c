/*
 * ut_adclp.c- Sigmastar
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

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <drv_adclp.h>

void sample_warn(int num)
{
    printf("adclp data exceeding the threshold\n");
}

int main(int argc, char **argv)
{
    int                fd;
    char               cmd;
    int                flags;
    unsigned short     value;
    unsigned int       channel;
    char               path[64];
    struct adclp_bound adclp_bd;

    if (argc == 2)
    {
        channel = atoi(argv[1]);
    }
    else if (argc == 4)
    {
        channel              = atoi(argv[1]);
        adclp_bd.upper_bound = atoi(argv[2]);
        adclp_bd.lower_bound = atoi(argv[3]);
    }
    else
    {
        printf("format: ut_adclp [channel] <upper> <lower>\n");
        return -1;
    }

    snprintf(path, sizeof(path), "/dev/adclp%u", channel);
    fd = open((const char *)(char *)path, O_RDWR);
    if (fd < 0)
    {
        printf("open device fail\n");
        return -1;
    }

    if (argc == 4)
    {
        ioctl(fd, IOCTL_ADCLP_SET_BOUND, &adclp_bd);
    }

    signal(SIGIO, sample_warn);

    fcntl(fd, F_SETOWN, getpid());
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | FASYNC);

    while (1)
    {
        cmd = getchar();
        if (cmd == 'q' || cmd == 'Q')
        {
            break;
        }

        ioctl(fd, IOCTL_ADCLP_READ_VALUE, &value);
        printf("adclp%u data[%hu]\n", channel, value);
    }

    close(fd);

    return 0;
}
