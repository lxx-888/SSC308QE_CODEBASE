/*
 * gyro_core.h- Sigmastar
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
#ifndef _GYRO_CORE_H
#define _GYRO_CORE_H

#include <linux/kernel.h>
#include <linux/mutex.h>
#include "gyro.h"

#define GYRO_DEVICNAME "gyro"
#define GYRO_DEV_COUNT 1

// Debug
#define PRINT_NONE   "\33[m"
#define PRINT_RED    "\33[1;31m"
#define PRINT_YELLOW "\33[1;33m"
#define PRINT_PURPLE "\33[1;35m"
#define PRINT_GREAN  "\33[1;32m"

#define GYRO_INFO(fmt, args...)                                           \
    do                                                                    \
    {                                                                     \
        printk(KERN_INFO "[%s]-info: " fmt "\n", GYRO_DEVICNAME, ##args); \
    } while (0)

#define GYRO_ERR(fmt, args...)                                                                               \
    do                                                                                                       \
    {                                                                                                        \
        printk(KERN_ERR "[%s]-error: " fmt " --> %s(%d)\n", GYRO_DEVICNAME, ##args, __FUNCTION__, __LINE__); \
    } while (0)

#define GYRO_WRN(fmt, args...)                                                                                 \
    do                                                                                                         \
    {                                                                                                          \
        printk(KERN_WARNING "[%s]-wrn: " fmt " --> %s(%d)\n", GYRO_DEVICNAME, ##args, __FUNCTION__, __LINE__); \
    } while (0)

#ifdef CONFIG_SS_GYRO_DEBUG_ON
#define GYRO_DBG(fmt, args...)                                                                                \
    do                                                                                                        \
    {                                                                                                         \
        printk(KERN_INFO "[%s]-debug: " fmt " --> %s(%d)\n", GYRO_DEVICNAME, ##args, __FUNCTION__, __LINE__); \
    } while (0)

#define GYRO_TRACE(fmt, args...)                                                                              \
    do                                                                                                        \
    {                                                                                                         \
        printk(KERN_INFO "[%s]-trace: " fmt " --> %s(%d)\n", GYRO_DEVICNAME, ##args, __FUNCTION__, __LINE__); \
    } while (0)
#endif
#ifndef GYRO_DBG
#define GYRO_DBG(fmt, args...) \
    {                          \
    }
#define GYRO_TRACE(fmt, args...) \
    {                            \
    }
#endif

#define MAX_FRAME_CNT (8)

struct gyro_info
{
    struct gyro_arg_gyro_xyz  gyro_xyz;
    struct gyro_arg_accel_xyz accel_xyz;
    struct gyro_arg_temp      temp;
    u8                        bytes_pre_data;
    bool                      en_fifo;
};

struct gyro_dev;

struct gyro_reg_ops
{
    int (*read_regs)(struct gyro_dev *, u8, void *, int);
    int (*read_reg)(struct gyro_dev *, u8, u8 *);
    int (*write_reg)(struct gyro_dev *, u8, u8);
};

struct gyro_sensor_ops
{
    /* early init - Init gyro on module_init
     *
     */
    int (*early_init)(struct gyro_dev *dev);

    /* finally deinit - Deinit gyro on module_exit
     *
     */
    int (*final_deinit)(struct gyro_dev *dev);

    /* init - Init gyro sensor
     *
     * Return 0 if success.
     */
    int (*init)(struct gyro_dev *dev);

    /* deinit - Deinit gyro sensor
     *
     */
    void (*deinit)(struct gyro_dev *dev);

    /* enable_fifo - enable gyro sensor fifo function
     *
     * @mode: device mode
     * @fifo_info: fifo info
     */
    int (*enable_fifo)(struct gyro_dev *dev, struct gyro_arg_dev_mode mode, struct gyro_arg_fifo_info *fifo_info);

    /* set_sample_rate - Set sample rate of gyro
     * get_sample_rate - Get sample rate of gyro
     *
     * @rate : rate
     */
    int (*set_sample_rate)(struct gyro_dev *dev, struct gyro_arg_sample_rate rate);
    int (*get_sample_rate)(struct gyro_dev *dev, struct gyro_arg_sample_rate *rate);

    /* set_gyro_range - Set the max range of gyro sensor
     * get_gyro_range - Get the max range of gyro sensor
     *
     * @range : enum of range
     *
     * get_gyro_sensitivity - Get sensitivity of gyro sensor that decided by gyro range
     *
     * @num :
     * @den :
     */
    int (*set_gyro_range)(struct gyro_dev *dev, struct gyro_arg_gyro_range range);
    int (*get_gyro_range)(struct gyro_dev *dev, struct gyro_arg_gyro_range *range);
    int (*get_gyro_sensitivity)(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity);

    /* set_accel_range - Set the max range of accel sensor
     * get_accel_range - Get the max range of accel sensor
     *
     * @range : enum of range
     *
     * get_gyro_sensitivity - Get sensitivity of accel sensor that decided by accel range
     *
     * @num :
     * @den :
     */
    int (*set_accel_range)(struct gyro_dev *dev, struct gyro_arg_accel_range range);
    int (*get_accel_range)(struct gyro_dev *dev, struct gyro_arg_accel_range *range);
    int (*get_accel_sensitivity)(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity);

    int (*read_fifo_cnt)(struct gyro_dev *dev, u16 *cnt);
    int (*reset_fifo)(struct gyro_dev *dev);
    int (*whoami_verify)(struct gyro_dev *dev);
    /* read_fifodata - Read fifo data
     *
     * @data : the pointer of data array
     * @cnt  : fifo count
     */
    int (*read_fifo_data)(struct gyro_dev *dev, u8 *data, u16 cnt);
    // int (*read_gyro_xyz)(struct gyro_arg_gyro_xyz *arg);
    // int (*read_accel_xyz)(struct gyro_arg_accel_xyz *arg);
    int (*read_temp)(struct gyro_arg_temp *arg);

    int (*get_group_delay)(struct gyro_dev *dev, struct gyro_arg_group_delay *delay);

    /*!
     * reserve
     */
    int (*get_noise_bandwidth)(struct gyro_dev *dev, struct gyro_arg_noise_bandwidth *nbw);
};

struct gyro_dev
{
    unsigned char     u8_devid;
    atomic_t          use_count;
    struct list_head  list_head;
    struct list_head *p_transfer_list_head;
    struct mutex      lock;

    struct gyro_info        gyro_info;
    struct device *         transfer_dev;
    struct gyro_reg_ops *   reg_ops;
    struct gyro_sensor_ops *sensor_ops;
};

int  gyro_transfer_init(struct gyro_dev *dev);
void gyro_transfer_deinit(struct gyro_dev *dev);

int  gyro_sysfs_init(void);
void gyro_sysfs_deinit(void);

int  gyro_ioctlfs_init(void);
void gyro_ioctlfs_deinit(void);

int  gyro_core_init(gyro_handle *ppDevHanle, unsigned char u8ValidDevNum);
void gyro_core_deinit(void);

int gyro_set_sample_rate(gyro_handle phandle, struct gyro_arg_sample_rate arg);
int gyro_get_sample_rate(gyro_handle phandle, struct gyro_arg_sample_rate *arg);
int gyro_set_gyro_range(gyro_handle phandle, struct gyro_arg_gyro_range arg);
int gyro_set_accel_range(gyro_handle phandle, struct gyro_arg_accel_range arg);
int gyro_get_gyro_range(gyro_handle phandle, struct gyro_arg_gyro_range *arg);
int gyro_get_gyro_sensitivity(gyro_handle phandle, struct gyro_arg_sensitivity *arg);
int gyro_get_accel_range(gyro_handle phandle, struct gyro_arg_accel_range *arg);
int gyro_get_accel_sensitivity(gyro_handle phandle, struct gyro_arg_sensitivity *arg);

int gyro_read_fifodata(gyro_handle phandle, u8 *fifo_data, u16 fifo_cnt);
int gyro_read_fifocnt(gyro_handle phandle, u16 *fifo_cnt);
int gyro_reset_fifo(gyro_handle phandle);
int gyro_whoami_verify(gyro_handle phandle);

int gyro_read_gyro_xyz(gyro_handle phandle, struct gyro_arg_gyro_xyz *arg);
int gyro_read_accel_xyz(gyro_handle phandle, struct gyro_arg_accel_xyz *arg);
int gyro_read_temp(gyro_handle phandle, struct gyro_arg_temp *arg);
int gyro_set_dev_mode(gyro_handle phandle, struct gyro_arg_dev_mode dev_mode, struct gyro_arg_fifo_info *fifo_info);
int gyro_enable(gyro_handle phandle);
int gyro_disable(gyro_handle phandle);
int gyro_get_group_delay(gyro_handle phandle, struct gyro_arg_group_delay *arg);
int gyro_get_noise_bandwidth(gyro_handle phandle, struct gyro_arg_noise_bandwidth *arg);

#endif
