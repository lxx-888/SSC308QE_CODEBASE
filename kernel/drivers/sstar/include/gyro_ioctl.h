/*
 * gyro_ioctl.h- Sigmastar
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

#ifndef _GYRO_IOCTL_H
#define _GYRO_IOCTL_H

#include "gyro.h"
#ifdef __KERNEL__
#else
#include <sys/ioctl.h>
#endif

#define GYRO_IOC_MAGIC     'G'
#define GYRO_IOC_DEVICNAME "gyro_ioc"
#define GYRO_IOC_DEV_COUNT 1

typedef enum
{
    /* set config or info */
    E_GYRO_IOC_CMD_SET_SAMPLE_RATE = 0,
    E_GYRO_IOC_CMD_SET_GYRO_RANGE,
    E_GYRO_IOC_CMD_SET_ACCEL_RANGE,
    E_GYRO_IOC_CMD_SET_DEV_MODE,

    /* get config or info */
    E_GYRO_IOC_CMD_GET_SAMPLE_RATE,
    E_GYRO_IOC_CMD_GET_GYRO_RANGE,
    E_GYRO_IOC_CMD_GET_GYRO_SENSITIVITY,
    E_GYRO_IOC_CMD_GET_ACCEL_RANGE,
    E_GYRO_IOC_CMD_GET_ACCEL_SENSITIVITY,
    E_GYRO_IOC_CMD_READ_FIFOCNT,
    E_GYRO_IOC_CMD_READ_FIFODATA,
    E_GYRO_IOC_CMD_READ_GYRO_XYZ,
    E_GYRO_IOC_CMD_READ_ACCEL_XYZ,
    E_GYRO_IOC_CMD_READ_TEMP,
    E_GYRO_IOC_CMD_WHOAMI_VERIFY,
    E_GYRO_IOC_CMD_GET_GROUP_DELAY,

    /* hw ops */
    E_GYRO_IOC_CMD_RESET_FIFO,

    /* ioctl num */
    E_GYRO_IOC_CMD_COUNT,
} GYRO_IOC_CMD_TYPE_e;

typedef struct gyro_arg_dev_mode_s
{
    struct gyro_arg_dev_mode  dev_mode;
    struct gyro_arg_fifo_info fifo_info;
} gyro_arg_dev_mode_info_t;

typedef struct gyro_fifo_data_info_s
{
    __u8 *pfifo_data;
    __u16 data_cnt;
} gyro_fifo_data_info_t;

#define GYRO_IOC_CMD_SET_SAMPLE_RATE _IOW(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_SET_SAMPLE_RATE, gyro_arg_sample_rate_t *)
#define GYRO_IOC_CMD_SET_GYRO_RANGE  _IOW(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_SET_GYRO_RANGE, gyro_arg_gyro_range_t *)
#define GYRO_IOC_CMD_SET_ACCEL_RANGE _IOW(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_SET_ACCEL_RANGE, gyro_arg_accel_range_t *)
#define GYRO_IOC_CMD_SET_DEV_MODE    _IOWR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_SET_DEV_MODE, gyro_arg_dev_mode_info_t *)

#define GYRO_IOC_CMD_GET_SAMPLE_RATE _IOWR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_GET_SAMPLE_RATE, gyro_arg_sample_rate_t *)
#define GYRO_IOC_CMD_GET_GYRO_RANGE  _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_GET_GYRO_RANGE, gyro_arg_gyro_range_t *)
#define GYRO_IOC_CMD_GET_GYRO_SENSITIVITY \
    _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_GET_GYRO_SENSITIVITY, gyro_arg_sensitivity_t *)
#define GYRO_IOC_CMD_GET_ACCEL_RANGE _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_GET_ACCEL_RANGE, gyro_arg_accel_range_t *)
#define GYRO_IOC_CMD_GET_ACCEL_SENSITIVITY \
    _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_GET_ACCEL_SENSITIVITY, gyro_arg_sensitivity_t *)

#define GYRO_IOC_CMD_READ_FIFOCNT   _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_READ_FIFOCNT, __u16 *)
#define GYRO_IOC_CMD_READ_FIFODATA  _IOWR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_READ_FIFODATA, gyro_fifo_data_info_t *)
#define GYRO_IOC_CMD_READ_GYRO_XYZ  _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_READ_GYRO_XYZ, gyro_arg_gyro_xyz_t *)
#define GYRO_IOC_CMD_READ_ACCEL_XYZ _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_READ_ACCEL_XYZ, gyro_arg_accel_xyz_t *)
#define GYRO_IOC_CMD_READ_TEMP      _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_READ_TEMP, gyro_arg_temp_t *)
#define GYRO_IOC_CMD_WHOAMI_VERIFY  _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_WHOAMI_VERIFY, void *)

#define GYRO_IOC_CMD_RESET_FIFO      _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_RESET_FIFO, void *) // not set input attr
#define GYRO_IOC_CMD_GET_GROUP_DELAY _IOR(GYRO_IOC_MAGIC, E_GYRO_IOC_CMD_GET_GROUP_DELAY, gyro_arg_group_delay_t *)

#endif /*_GYRO_IOCTL_H  */