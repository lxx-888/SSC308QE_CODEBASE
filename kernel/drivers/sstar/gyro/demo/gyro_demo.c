/*
 * gyro_demo.c- Sigmastar
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

//#include <asm-generic/int-ll64.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include "gyro_ioctl.h"
#include "gyro.h"
#include "gyro_demo.h"
#include "stddef.h"

const gyro_ioc_desc_t g_gyro_ioc_desc[E_GYRO_IOC_CMD_COUNT] = {
    /*set config or info */
    {
        .ioc_num = GYRO_IOC_CMD_SET_SAMPLE_RATE,
        .name    = "{GYRO_IOC_CMD_SET_SAMPLE_RATE}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_SET_GYRO_RANGE,
        .name    = "{GYRO_IOC_CMD_SET_GYRO_RANGE}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_SET_ACCEL_RANGE,
        .name    = "{GYRO_IOC_CMD_SET_ACCEL_RANGE}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_SET_DEV_MODE,
        .name    = "{GYRO_IOC_CMD_SET_DEV_MODE}",
    },

    /* get config or info */
    {
        .ioc_num = GYRO_IOC_CMD_GET_SAMPLE_RATE,
        .name    = "{GYRO_IOC_CMD_GET_SAMPLE_RATE}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_GET_GYRO_RANGE,
        .name    = "{GYRO_IOC_CMD_GET_GYRO_RANGE}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_GET_GYRO_SENSITIVITY,
        .name    = "{GYRO_IOC_CMD_GET_GYRO_SENSITIVITY}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_GET_ACCEL_RANGE,
        .name    = "{GYRO_IOC_CMD_GET_ACCEL_RANGE}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_GET_ACCEL_SENSITIVITY,
        .name    = "{GYRO_IOC_CMD_GET_ACCEL_SENSITIVITY}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_READ_FIFOCNT,
        .name    = "{GYRO_IOC_CMD_READ_FIFOCNT}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_READ_FIFODATA,
        .name    = "{GYRO_IOC_CMD_READ_FIFODATA}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_READ_GYRO_XYZ,
        .name    = "{GYRO_IOC_CMD_READ_GYRO_XYZ}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_READ_ACCEL_XYZ,
        .name    = "{GYRO_IOC_CMD_READ_ACCEL_XYZ}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_READ_TEMP,
        .name    = "{GYRO_IOC_CMD_READ_TEMP}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_WHOAMI_VERIFY,
        .name    = "{GYRO_IOC_CMD_WHOAMI_VERIFY}",
    },
    {
        .ioc_num = GYRO_IOC_CMD_GET_GROUP_DELAY,
        .name    = "{GYRO_IOC_CMD_GET_GROUP_DELAY}",
    },

    /* hw ops */
    {
        .ioc_num = GYRO_IOC_CMD_RESET_FIFO,
        .name    = "{GYRO_IOC_CMD_RESET_FIFO}",
    },
};

static gyro_arg_fifo_info_t g_fifo_info = {};
static bool                 g_bExit     = false;
static unsigned char        u8DevID     = 0;

static int show_one_frame_fifo_data(__u8 *data, struct gyro_arg_fifo_info fifo_info, u16 u16Fifo_cnt)
{
    typedef struct fifo_parse_s
    {
        char *name;
        __u8  start;
        __u8  end;
    } fifo_parse_t;

    fifo_parse_t fp[] = {
        {"gx", fifo_info.gx_start, fifo_info.gx_end},     {"gy", fifo_info.gy_start, fifo_info.gy_end},
        {"gz", fifo_info.gz_start, fifo_info.gz_end},     {"ax", fifo_info.ax_start, fifo_info.ax_end},
        {"ay", fifo_info.ay_start, fifo_info.ay_end},     {"az", fifo_info.az_start, fifo_info.az_end},
        {"te", fifo_info.temp_start, fifo_info.temp_end},
    };
    unsigned int  i     = 0;
    unsigned char j     = 0;
    int           num   = 0;
    int           shift = 0;

    printf("|");
    for (i = 0; i < sizeof(fp) / sizeof(fp[0]); ++i)
    {
        if (fp[i].start > fifo_info.bytes_pre_data || fp[i].end > fifo_info.bytes_pre_data)
        {
            continue;
        }
        num   = 0;
        shift = fifo_info.is_big_endian ? (fp[i].end - fp[i].start) * 8 : 0;

        for (j = fp[i].start; j <= fp[i].end; ++j)
        {
            if ((fifo_info.is_big_endian == 1 && j == fp[i].start) || (fifo_info.is_big_endian == 0 && j == fp[i].end))
            {
                num |= ((signed char)data[j] << shift);
            }
            else
            {
                num |= ((unsigned char)data[j] << shift);
            }

            shift += fifo_info.is_big_endian ? -8 : 8;
        }

        printf("%4s: %6d| ", fp[i].name, num);
    }
    printf("\n");

    return 0;
}

static int _gyro_range_test(gyro_ioc_handle_t *p_handle)
{
    int                        i          = 0;
    bool                       result     = true;
    struct gyro_arg_gyro_range gyro_range = {0};
#ifdef CONFIG_SS_GYRO_CHIP_ICM42607
    unsigned int test_gyro_range[] = {
        250,
        500,
        1000,
        2000,
    };
#else
    unsigned int test_gyro_range[] = {
        125,
        250,
        2000,
    };
#endif
    PARAM_SAFE(p_handle, NULL);
    for (i = 0; i < sizeof(test_gyro_range) / sizeof(test_gyro_range[0]); ++i)
    {
        gyro_range.range = test_gyro_range[i];
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_SET_GYRO_RANGE], &gyro_range);
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_GET_GYRO_RANGE], &gyro_range);
        GYRO_LOG_DBG("gyro range set [%u] and get [%u]\n", test_gyro_range[i], gyro_range.range);
        CHECK_SUCCESS_OPS(gyro_range.range == test_gyro_range[i], continue;
                          , "check gyro range fail: set [%u] and get [%u]\n", test_gyro_range[i], gyro_range.range);
    }
    return 0;
}

static int _gyro_accel_range_test(gyro_ioc_handle_t *p_handle)
{
    bool                        result = true;
    int                         i      = 0;
    struct gyro_arg_accel_range accel_range;
    unsigned int                test_accel_range[] = {2, 4, 8, 16};
    PARAM_SAFE(p_handle, NULL);

    printf("----accel range test --------------------------------------\n");
    for (i = 0; i < sizeof(test_accel_range) / sizeof(test_accel_range[0]); ++i)
    {
        accel_range.range = test_accel_range[i];
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_SET_ACCEL_RANGE], &accel_range);
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_GET_ACCEL_RANGE], &accel_range);
        GYRO_LOG_DBG("accel range set [%u] and get [%u]\n", test_accel_range[i], accel_range.range);
        CHECK_SUCCESS_OPS(accel_range.range == test_accel_range[i], continue;
                          , "check accel range fail: set [%u] and get [%u]\n", test_accel_range[i], accel_range.range);
    }
    return 0;
}

static int _gyro_sample_rate_test(gyro_ioc_handle_t *p_handle)
{
    int  i      = 0;
    bool result = true;

#ifdef CONFIG_SS_GYRO_CHIP_ICM42607
    unsigned int test_sample_rate[] = {1600, 800, 400, 200, 100, 50, 25};
#else
    unsigned int test_sample_rate[] = {500, 1000};
#endif
    struct gyro_arg_sample_rate sample_rate;

    PARAM_SAFE(p_handle, NULL);
    printf("----sample rate test --------------------------------------\n");
    for (i = 0; i < sizeof(test_sample_rate) / sizeof(test_sample_rate[0]); ++i)
    {
        sample_rate.rate = test_sample_rate[i];
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_SET_SAMPLE_RATE], &sample_rate);
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_GET_SAMPLE_RATE], &sample_rate);
        GYRO_LOG_DBG("sample range set [%u] and get [%u]\n", test_sample_rate[i], sample_rate.rate);
        CHECK_SUCCESS_OPS(sample_rate.rate == test_sample_rate[i], continue;
                          , "check sample range fail: set [%u] and get [%u]\n", test_sample_rate[i], sample_rate.rate);
    }

    return 0;
}

static int _gyro_dev_mode_test(gyro_ioc_handle_t *p_handle)
{
    int                      i             = 0;
    bool                     result        = true;
    gyro_arg_dev_mode_info_t dev_mode_info = {0};
    gyro_arg_group_delay_t   group_delay   = {0};

    struct gyro_arg_dev_mode test_dev_mode[] = {
        {0, 0},
        {1, GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN | GYROSENSOR_ZG_FIFO_EN
                | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_XG_FIFO_EN | GYROSENSOR_TEMP_FIFO_EN},
        {1, GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN},
        {1, GYROSENSOR_ZG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_XG_FIFO_EN},
        {1, GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN | GYROSENSOR_ZG_FIFO_EN
                | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_XG_FIFO_EN},
    };

    PARAM_SAFE(p_handle, NULL);

    printf("----gyro_set_dev_mode -------------------------------------\n");
    for (i = 0; i < sizeof(test_dev_mode) / sizeof(test_dev_mode[0]); ++i)
    {
        dev_mode_info.dev_mode = test_dev_mode[i];
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_SET_DEV_MODE], &dev_mode_info);
        printf("\tgx_start, gx_end      %d, %d\n", dev_mode_info.fifo_info.gx_start, dev_mode_info.fifo_info.gx_end);
        printf("\tgy_start, gy_end      %d, %d\n", dev_mode_info.fifo_info.gy_start, dev_mode_info.fifo_info.gy_end);
        printf("\tgz_start, gz_end      %d, %d\n", dev_mode_info.fifo_info.gz_start, dev_mode_info.fifo_info.gz_end);
        printf("\tax_start, ax_end      %d, %d\n", dev_mode_info.fifo_info.ax_start, dev_mode_info.fifo_info.ax_end);
        printf("\tay_start, ay_end      %d, %d\n", dev_mode_info.fifo_info.ay_start, dev_mode_info.fifo_info.ay_end);
        printf("\taz_start, az_end      %d, %d\n", dev_mode_info.fifo_info.az_start, dev_mode_info.fifo_info.az_end);
        printf("\ttemp_start, temp_end  %d, %d\n", dev_mode_info.fifo_info.temp_start,
               dev_mode_info.fifo_info.temp_end);
        printf("\tbytes_pre_data        %d\n", dev_mode_info.fifo_info.bytes_pre_data);
        printf("\tis_big_endian         %d\n", dev_mode_info.fifo_info.is_big_endian);
        printf("\tmax_fifo_cnt          %d\n", dev_mode_info.fifo_info.max_fifo_cnt);
    }

    printf("----gyro group_delay -------------------------------------\n");
    GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_GET_GROUP_DELAY], &group_delay);
    printf("\tgroup_delay [%u] us\n", group_delay.delay_us);

    g_fifo_info = dev_mode_info.fifo_info;
    return 0;
}

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (((val) / (alignment)) * (alignment))
#endif
static int _gyro_fifo_test(gyro_ioc_handle_t *p_handle)
{
    bool                        result           = true;
    struct gyro_arg_fifo_info   fifo_info        = g_fifo_info;
    unsigned long long          sum_fifo_cnt     = 0;
    unsigned long long          average_fifo_cnt = 0;
    unsigned long long          max_fifo_cnt     = 0;
    unsigned long long          loop_cnt         = 0;
    unsigned long long          tmp_fifo_cnt     = 0;
    gyro_fifo_data_info_t       fifo_data_info   = {0};
    struct gyro_arg_sample_rate sample_rate;

    PARAM_SAFE(p_handle, NULL);

#ifdef CONFIG_SS_GYRO_CHIP_ICM42607
#define FIFO_CNT 2304
    sample_rate.rate = 800;
#else
#define FIFO_CNT 2048
    sample_rate.rate                = 1000;
#endif
    printf("----gyro fifo test bytes_pre_data:%u---------------------------------------\n", fifo_info.bytes_pre_data);

    // To avoid FIFO overflow due to excessively high sampling rates,
    // we'll default to a sampling rate of 1kHz before reading from the FIFO.

    GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_SET_SAMPLE_RATE], &sample_rate);
    GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_GET_SAMPLE_RATE], &sample_rate);
    GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_RESET_FIFO], NULL);

    fifo_data_info.pfifo_data = (__u8 *)malloc(FIFO_CNT);
    if (fifo_data_info.pfifo_data == NULL)
    {
        printf("no mem\n");
        return -1;
    }

    while (g_bExit == false)
    {
        __u16 cnt = 0;
        __u16 j   = 0;

        bool bNeedReset = false;
        usleep(33000);
        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_READ_FIFOCNT], &cnt);
        if (cnt > FIFO_CNT)
        {
            cnt = FIFO_CNT / fifo_info.bytes_pre_data * fifo_info.bytes_pre_data;
        }

        memset(fifo_data_info.pfifo_data, 0, cnt);

        if (cnt % fifo_info.bytes_pre_data != 0)
        {
            // fifo_data_info.data_cnt = ALIGN_DOWN(cnt, fifo_info.bytes_pre_data);
            fifo_data_info.data_cnt = cnt;
            GYRO_LOG_WRN("fifocnt[%u] not align [%u], will force align as[%u]\n", cnt, fifo_info.bytes_pre_data,
                         fifo_data_info.data_cnt);
            bNeedReset = true;
        }
        else
        {
            fifo_data_info.data_cnt = cnt;
        }

        tmp_fifo_cnt = cnt;
        ++loop_cnt;
        sum_fifo_cnt += tmp_fifo_cnt;
        average_fifo_cnt = sum_fifo_cnt / loop_cnt;
        if (tmp_fifo_cnt > max_fifo_cnt)
        {
            max_fifo_cnt = tmp_fifo_cnt;
        }

        GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_READ_FIFODATA], &fifo_data_info);
        if (bNeedReset == true)
        {
            GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_RESET_FIFO], NULL);
        }

        printf("loop[%08llx] curcnt[%03llu] avgcnt[%llu] maxcnt[%03llu]\n", loop_cnt, tmp_fifo_cnt, average_fifo_cnt,
               max_fifo_cnt);
        for (j = 0; j <= cnt - fifo_info.bytes_pre_data; j += fifo_info.bytes_pre_data)
        {
            show_one_frame_fifo_data(&fifo_data_info.pfifo_data[j], fifo_info, tmp_fifo_cnt);
        }
    }

    free(fifo_data_info.pfifo_data);
    return 0;
}

int gyro_ioc_init(gyro_ioc_handle_t *p_handle)
{
    char str[17] = {};
    PARAM_SAFE(p_handle, NULL);

    snprintf(str, 16, GYRO_IOC_DIR "%u", u8DevID);
    p_handle->ioc_fd = open((const char *)str, O_WRONLY);
    if (p_handle->ioc_fd == -1)
    {
        GYRO_LOG_ERR("fail to open %s device\n", str);
        return -1;
    }

    p_handle->p_ioc_desc = (gyro_ioc_desc_t *)g_gyro_ioc_desc;

    // GYRO_CALL_IOCTL(result, p_handle->ioc_fd, p_handle->p_ioc_desc[E_GYRO_IOC_CMD_WHOAMI_VERIFY]);
    // CHECK_SUCCESS(result != false, "init fail, gyro sernor whoami check fail\n");

    GYRO_LOG_INFO("init success\n");
    return 0;
}

int gyro_ioc_run(gyro_ioc_handle_t *p_handle)
{
    _gyro_range_test(p_handle);
    _gyro_accel_range_test(p_handle);
    _gyro_sample_rate_test(p_handle);
    _gyro_dev_mode_test(p_handle);
    _gyro_fifo_test(p_handle);

    return 0;
}

int gyro_ioc_deinit(gyro_ioc_handle_t *p_handle)
{
    PARAM_SAFE(p_handle, NULL);

    if (p_handle->ioc_fd)
    {
        close(p_handle->ioc_fd);
    }

    memset(p_handle, 0, sizeof(*p_handle));

    GYRO_LOG_INFO("gyro ioctl device deinit success\n");

    return 0;
}

void gyro_handle_sig(int signo)
{
    if (signo == SIGINT)
    {
        GYRO_LOG_INFO("catch Ctrl + C, exit normally\n");
        g_bExit = true;
    }
}

void gyro_siginit(void)
{
    struct sigaction sigAction = {0};

    sigAction.sa_handler = gyro_handle_sig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);
}

int help_message(char **argv)
{
    printf("Usage: %s -options arguments\n\n", argv[0]);
#define print_cmd(arg, desc) printf("  %-20s,  %-30s\n", arg, desc);
#define print_cmd_sub(desc)  printf("", "  " desc);

    printf("-d devid Specify the devid of the test");

#undef print_cmd_sub
#undef print_cmd
    printf("\n");
    return 0;
}

int main(int argc, char **argv)
{
    int               choice          = 0;
    gyro_ioc_handle_t gyro_ioc_handle = {0};
    gyro_siginit();

    if (argc <= 1)
    {
        help_message(argv);
        exit(-1);
    }

    while (-1 != (choice = getopt(argc, (char **)argv, "d:h:")))
    {
        switch (choice)
        {
            case 'd':
                u8DevID = atoi(optarg);
                break;
            case 'h':
                help_message(argv);
                exit(-1);
            default:
                GYRO_LOG_ERR("Not support this [%d]option\n", choice);
                exit(-1);
        }
    }

    if (gyro_ioc_init(&gyro_ioc_handle) != 0)
    {
        goto err_init;
    }

    gyro_ioc_run(&gyro_ioc_handle);

err_init:
    gyro_ioc_deinit(&gyro_ioc_handle);
    return 0;
}
