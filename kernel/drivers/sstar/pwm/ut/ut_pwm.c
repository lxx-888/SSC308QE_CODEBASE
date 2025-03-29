/*
 * ut_pwm.c- Sigmastar
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

#include <autoconf.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <drv_pwm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
    int  ret    = 0;
    int  pwm_fd = -1;
    char path_name[24];

    struct pwm_ch_cfg pwm_channel;
    struct pwm_gp_cfg pwm_group;

    if (argc != 4)
    {
        printf("format: ut_pwm <channel> [channel_id] [enable]\n");
        printf("format: ut_pwm <group>   [group_id]   [enable]\n");
        printf("format: ut_pwm <stop>    [group_id]   [enable]\n");
        printf("format: ut_pwm <round>   [group_id]   [round_num]\n");
        printf("format: ut_pwm <ddt>     [channel_id] [dead_time]\n");
        return -1;
    }

    if (!strcmp(argv[1], "channel"))
    {
        pwm_channel.period   = 1000000;
        pwm_channel.duty     = 500000;
        pwm_channel.shift    = 200000;
        pwm_channel.polarity = 1;
        pwm_channel.channel  = atoi(argv[2]);
        pwm_channel.enable   = atoi(argv[3]);

        snprintf(path_name, sizeof(path_name), "/dev/pwm%u", pwm_channel.channel);
        pwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (pwm_fd < 0)
        {
            printf("open /dev/pwm%u fail errno:[%d]\n", pwm_channel.channel, pwm_fd);
            return -1;
        }

        ret = ioctl(pwm_fd, IOCTL_PWM_SET_CHAN_CFG, &pwm_channel);
        if (ret < 0)
        {
            printf("pwm channel[%u] set config fail\n", pwm_channel.channel);
            return ret;
        }

        ret = ioctl(pwm_fd, IOCTL_PWM_GET_CHAN_CFG, &pwm_channel);
        if (ret < 0)
        {
            printf("pwm channel[%u] get config fail\n", pwm_channel.channel);
            return ret;
        }

        usleep(500000);
        pwm_channel.duty = 700000;
        ret              = ioctl(pwm_fd, IOCTL_PWM_SET_CHAN_CFG, &pwm_channel);
        if (ret < 0)
        {
            printf("pwm channel[%u] set config again fail\n", pwm_channel.channel);
            return ret;
        }
    }
    else if (!strcmp(argv[1], "group"))
    {
        pwm_group.period   = 1000000;
        pwm_group.duty     = 500000;
        pwm_group.shift    = 200000;
        pwm_group.polarity = 0;
        pwm_group.group    = atoi(argv[2]);
        pwm_group.enable   = atoi(argv[3]);

        snprintf(path_name, sizeof(path_name), "/dev/pwm_group%u", pwm_group.group);
        pwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (pwm_fd < 0)
        {
            printf("open /dev/pwm-group%u fail errno:[%d]\n", pwm_group.group, pwm_fd);
            return -1;
        }

        ret = ioctl(pwm_fd, IOCTL_PWM_SET_GROUP_CFG, &pwm_group);
        if (ret < 0)
        {
            printf("pwm group[%u] set config fail\n", pwm_group.group);
            return ret;
        }

        ret = ioctl(pwm_fd, IOCTL_PWM_GET_GROUP_CFG, &pwm_group);
        if (ret < 0)
        {
            printf("pwm group[%u] get config fail\n", pwm_group.group);
            return ret;
        }

        pwm_group.duty = 800000;
        usleep(500000);
        ret = ioctl(pwm_fd, IOCTL_PWM_SET_GROUP_CFG, &pwm_group);
        if (ret < 0)
        {
            printf("pwm group[%u] set config again fail\n", pwm_group.group);
            return ret;
        }
    }
    else if (!strcmp(argv[1], "stop"))
    {
        pwm_group.group = atoi(argv[2]);
        snprintf(path_name, sizeof(path_name), "/dev/pwm_group%u", pwm_group.group);
        pwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (pwm_fd < 0)
        {
            printf("open /dev/pwm-group%u fail errno:[%d]\n", pwm_group.group, pwm_fd);
            return -1;
        }

        pwm_group.stop_en = atoi(argv[3]);
        ret               = ioctl(pwm_fd, IOCTL_PWM_GROUP_STOP, &pwm_group);
        if (ret < 0)
        {
            printf("pwm group[%u] stop config fail\n", pwm_group.group);
            return ret;
        }
    }
    else if (!strcmp(argv[1], "round"))
    {
        pwm_group.group = atoi(argv[2]);
        snprintf(path_name, sizeof(path_name), "/dev/pwm_group%u", pwm_group.group);
        pwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (pwm_fd < 0)
        {
            printf("open /dev/pwm-group%u fail errno:[%d]\n", pwm_group.group, pwm_fd);
            return -1;
        }
        pwm_group.round_num = atoi(argv[3]);
        ret                 = ioctl(pwm_fd, IOCTL_PWM_GROUP_ROUND, &pwm_group);
        if (ret < 0)
        {
            printf("pwm group[%u] round config fail\n", pwm_group.group);
            return ret;
        }
    }
#ifdef CONFIG_SSTAR_PWM_DDT
    else if (!strcmp(argv[1], "ddt"))
    {
        pwm_channel.channel = atoi(argv[2]);
        snprintf(path_name, sizeof(path_name), "/dev/pwm%u", pwm_channel.channel);
        pwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (pwm_fd < 0)
        {
            printf("open /dev/pwm%u fail errno:[%d]\n", pwm_channel.channel, pwm_fd);
            return -1;
        }

        ret = ioctl(pwm_fd, IOCTL_PWM_GET_CHAN_CFG, &pwm_channel);
        if (ret < 0)
        {
            printf("pwm channel[%u] get config fail\n", pwm_channel.channel);
            return ret;
        }

        pwm_channel.p_ddt  = atoi(argv[3]);
        pwm_channel.n_ddt  = atoi(argv[3]);
        pwm_channel.ddt_en = 1;

        ret = ioctl(pwm_fd, IOCTL_PWM_SET_CHAN_CFG, &pwm_channel);
        if (ret < 0)
        {
            printf("pwm channel[%u] set ddt config fail\n", pwm_channel.channel);
            return ret;
        }
    }
#endif
    else
    {
        printf("erro pwm command\n");
    }

    close(pwm_fd);

    return 0;
}
