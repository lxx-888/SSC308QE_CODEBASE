/*
 * gyro.h- Sigmastar
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
#ifndef GYRO_H
#define GYRO_H

#include <asm-generic/int-ll64.h>

#define GYRO_SUCCESS (0)

typedef void *gyro_handle;

enum gyro_fifo_type
{
    GYROSENSOR_ZA_FIFO_EN   = 0x01,
    GYROSENSOR_YA_FIFO_EN   = 0x02,
    GYROSENSOR_XA_FIFO_EN   = 0x04,
    GYROSENSOR_ZG_FIFO_EN   = 0x10,
    GYROSENSOR_YG_FIFO_EN   = 0x20,
    GYROSENSOR_XG_FIFO_EN   = 0x40,
    GYROSENSOR_TEMP_FIFO_EN = 0x80,
    GYROSENSOR_FIFO_MAX_EN  = 0xFF,
};

typedef struct gyro_arg_sample_rate
{
    unsigned int rate;
} gyro_arg_sample_rate_t;

typedef struct gyro_arg_gyro_range
{
    unsigned int range;
} gyro_arg_gyro_range_t;

typedef struct gyro_arg_accel_range
{
    unsigned int range;
} gyro_arg_accel_range_t;

typedef struct gyro_arg_sensitivity
{
    unsigned short num;
    unsigned short den;
} gyro_arg_sensitivity_t;

typedef struct gyro_arg_gyro_xyz
{
    short x;
    short y;
    short z;
} gyro_arg_gyro_xyz_t;

typedef struct gyro_arg_accel_xyz
{
    short x;
    short y;
    short z;
} gyro_arg_accel_xyz_t;

typedef struct gyro_arg_temp
{
    short temp;
} gyro_arg_temp_t;

typedef struct gyro_arg_dev_mode
{
    char          fifo_mode; /* 1 or 0 */
    unsigned char fifo_type;
} gyro_arg_dev_mode_t;

typedef struct gyro_arg_fifo_info
{
    unsigned char  gx_start, gx_end;
    unsigned char  gy_start, gy_end;
    unsigned char  gz_start, gz_end;
    unsigned char  ax_start, ax_end;
    unsigned char  ay_start, ay_end;
    unsigned char  az_start, az_end;
    unsigned char  temp_start, temp_end;
    unsigned char  bytes_pre_data;
    unsigned char  is_big_endian;
    unsigned short max_fifo_cnt;
} gyro_arg_fifo_info_t;

typedef struct gyro_arg_group_delay
{
    unsigned int delay_us;
} gyro_arg_group_delay_t;

typedef struct gyro_arg_noise_bandwidth
{
    unsigned int nbw_hz;
} gyro_arg_noise_bandwidth_t;

#ifdef __KERNEL__
typedef struct MHAL_DIS_GyroRegisterHander_s
{
    int (*pGyroSetSampleRate)(gyro_handle phandle, struct gyro_arg_sample_rate arg);
    int (*pGyroGetSampleRate)(gyro_handle phandle, struct gyro_arg_sample_rate *arg);
    int (*pGyroSetGyroRange)(gyro_handle phandle, struct gyro_arg_gyro_range arg);
    int (*pGyroSetAccelRange)(gyro_handle phandle, struct gyro_arg_accel_range arg);
    int (*pGyroGetGyroRange)(gyro_handle phandle, struct gyro_arg_gyro_range *arg);
    int (*pGyroGetAccelRange)(gyro_handle phandle, struct gyro_arg_accel_range *arg);
    int (*pGyroGetAccelSensitivity)(gyro_handle phandle, struct gyro_arg_sensitivity *arg);
    int (*pGyroGetGyroSensitivity)(gyro_handle phandle, struct gyro_arg_sensitivity *arg);
    /* fifo ops */
    int (*pGyroReadFifodata)(gyro_handle phandle, u8 *fifo_data, u16 fifo_cnt);
    int (*pGyroReadFifocnt)(gyro_handle phandle, u16 *fifo_cnt);
    int (*pGyroResetFifo)(gyro_handle phandle);

    int (*pGyroReadGyroXyz)(gyro_handle phandle, struct gyro_arg_gyro_xyz *arg);
    int (*pGyroReadAccelXyz)(gyro_handle phandle, struct gyro_arg_accel_xyz *arg);
    int (*pGyroReadTemp)(gyro_handle phandle, struct gyro_arg_temp *arg);
    int (*pGyroSetDevMode)(gyro_handle phandle, struct gyro_arg_dev_mode dev_mode,
                           struct gyro_arg_fifo_info *fifo_info);
    int (*pGyroEnable)(gyro_handle phandle);
    int (*pGyroDisable)(gyro_handle phandle);
} MHAL_DIS_GyroRegisterHander_t;
#endif /* __KERNEL__ */

#endif /* ifndef GYRO_H */
