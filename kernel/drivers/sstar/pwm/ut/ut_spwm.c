/*
 * ut_spwm.c- Sigmastar
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
    u8   i       = 0;
    int  ret     = 0;
    u32  duty    = 0;
    int  spwm_fd = -1;
    char path_name[24];

    struct spwm_gp_cfg gp_cfg;

    if (argc != 4 && argc != 5)
    {
        printf("format: ut_spwm <group>  [group_id]  [mode]   [duty_len]\n");
        printf("format: ut_spwm <duty>   [group_id]  [index]  [duty_len]\n");
        printf("format: ut_spwm <stop>   [group_id]  [enable]\n");
        printf("format: ut_spwm <round>  [group_id]  [round_num]\n");
        return -1;
    }

    if (!strcmp(argv[1], "group"))
    {
        gp_cfg.group = atoi(argv[2]);

        snprintf(path_name, sizeof(path_name), "/dev/spwm_group%u", gp_cfg.group);
        spwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (spwm_fd < 0)
        {
            printf("open /dev/spwm_group%u fail errno:[%d]\n", gp_cfg.group, spwm_fd);
            return -1;
        }

        gp_cfg.step      = 0;
        gp_cfg.scale     = 0;
        gp_cfg.clamp_en  = 0;
        gp_cfg.clamp_max = 12000;
        gp_cfg.clamp_min = 10000;

        gp_cfg.enable   = 1;
        gp_cfg.mode     = atoi(argv[3]);
        gp_cfg.duty_len = atoi(argv[4]);
        gp_cfg.period   = 20000;

        if (gp_cfg.duty_len > 64)
        {
            printf("spwm group[%u] max duty len is 64\n", gp_cfg.group);
            close(spwm_fd);
            return -1;
        }

        duty = 250;
        for (i = 0; i < gp_cfg.duty_len; i++)
        {
            gp_cfg.duty[i] = duty * (i + 1);
        }

        ret = ioctl(spwm_fd, IOCTL_SPWM_SET_GROUP_CFG, &gp_cfg);
        if (ret < 0)
        {
            printf("spwm group[%u] set config fail， err[%d]\n", gp_cfg.group, ret);
            close(spwm_fd);
            return ret;
        }

        /*spwm period change from 50Khz to 25Khz */
        gp_cfg.period = 40000;
        ret           = ioctl(spwm_fd, IOCTL_SPWM_UPDATE_PERIOD, &gp_cfg);
        if (ret < 0)
        {
            printf("spwm group[%u] update period fail\n", gp_cfg.group);
            close(spwm_fd);
            return ret;
        }

        usleep(6000);
        /*spwm period change from 25Khz to 40Khz */
        gp_cfg.period = 25000;
        ret           = ioctl(spwm_fd, IOCTL_SPWM_UPDATE_PERIOD, &gp_cfg);
        if (ret < 0)
        {
            printf("spwm group[%u] update period fail\n", gp_cfg.group);
            close(spwm_fd);
            return ret;
        }
    }
    else if (!strcmp(argv[1], "duty"))
    {
        gp_cfg.group    = atoi(argv[2]);
        gp_cfg.index    = atoi(argv[3]);
        gp_cfg.duty_len = atoi(argv[4]);

        snprintf(path_name, sizeof(path_name), "/dev/spwm_group%u", gp_cfg.group);
        spwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (spwm_fd < 0)
        {
            printf("open /dev/spwm_group%u fail errno:[%d]\n", gp_cfg.group, spwm_fd);
            return -1;
        }

        for (i = 0; i < gp_cfg.duty_len; i++)
        {
            gp_cfg.duty[i] = 16000;
        }

        ret = ioctl(spwm_fd, IOCTL_SPWM_UPDATE_DUTY, &gp_cfg);
        if (ret < 0)
        {
            printf("spwm group[%u] update duty fail\n", gp_cfg.group);
            close(spwm_fd);
            return ret;
        }
    }
    else if (!strcmp(argv[1], "stop"))
    {
        gp_cfg.group   = atoi(argv[2]);
        gp_cfg.stop_en = atoi(argv[3]);
        snprintf(path_name, sizeof(path_name), "/dev/spwm_group%u", gp_cfg.group);
        spwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (spwm_fd < 0)
        {
            printf("open /dev/spwm-group%u fail errno:[%d]\n", gp_cfg.group, spwm_fd);
            return -1;
        }

        ret = ioctl(spwm_fd, IOCTL_SPWM_GROUP_STOP, &gp_cfg);
        if (ret < 0)
        {
            printf("spwm group[%u] stop config fail\n", gp_cfg.group);
            close(spwm_fd);
            return ret;
        }
    }
    else if (!strcmp(argv[1], "round"))
    {
        gp_cfg.group     = atoi(argv[2]);
        gp_cfg.round_num = atoi(argv[3]);
        snprintf(path_name, sizeof(path_name), "/dev/spwm_group%u", gp_cfg.group);
        spwm_fd = open((const char *)(char *)path_name, O_RDWR);
        if (spwm_fd < 0)
        {
            printf("open /dev/spwm-group%u fail errno:[%d]\n", gp_cfg.group, spwm_fd);
            return -1;
        }
        ret = ioctl(spwm_fd, IOCTL_SPWM_GROUP_ROUND, &gp_cfg);
        if (ret < 0)
        {
            printf("spwm group[%u] round config fail\n", gp_cfg.group);
            close(spwm_fd);
            return ret;
        }
    }
    else
    {
        printf("erro command\n");
    }

    close(spwm_fd);

    return 0;
}
