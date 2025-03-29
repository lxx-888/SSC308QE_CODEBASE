/*
 * riu_ut.c- Sigmastar
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

struct riu_irq_info
{
    unsigned char bridge;
    unsigned char capture;
};

#define RIU_GET_IRQ_INFO _IOW('R', 0, struct riu_irq_info)

#define DEVICE_NAME "/dev/riu"

static int riu_fd;

static void riu_signal(int sig)
{
    struct riu_irq_info irq_info;

    ioctl(riu_fd, RIU_GET_IRQ_INFO, &irq_info);

    printf("[%s] bridge %d capture %d\n", __FUNCTION__, irq_info.bridge, irq_info.capture);
}

void exit_riu_ut(int sign)
{
    printf("stop riu ut\n");
    close(riu_fd);
    exit(0);
}

int main(int argc, char *argv[])
{
    int flag;

    riu_fd = open(DEVICE_NAME, O_RDWR);
    if (riu_fd < 0)
    {
        perror(DEVICE_NAME " open fail\n");
    }

    signal(SIGINT, exit_riu_ut);

    signal(SIGIO, riu_signal);

    if (fcntl(riu_fd, F_SETOWN, getpid()))
    {
        perror("F_SETOWN fcntl");
    }
    flag = fcntl(riu_fd, F_GETFL);
    if (fcntl(riu_fd, F_SETFL, flag | FASYNC))
    {
        perror("F_SETFL fcntl");
    }

    while (1)
        sleep(100);

    return 0;
}
