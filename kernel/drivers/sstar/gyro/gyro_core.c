/*
 * gyro_core.c - Sigmastar
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
#include <linux/export.h>
#include <linux/module.h>

#include "gyro_core.h"
#include "gyro_internal.h"

int gyro_set_sample_rate(gyro_handle phandle, struct gyro_arg_sample_rate arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->set_sample_rate, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->set_sample_rate(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_get_sample_rate(gyro_handle phandle, struct gyro_arg_sample_rate *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(arg, NULL);
    PARAM_SAFE(p_dev->sensor_ops->get_sample_rate, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->get_sample_rate(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_set_gyro_range(gyro_handle phandle, struct gyro_arg_gyro_range arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->set_gyro_range, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->set_gyro_range(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_set_accel_range(gyro_handle phandle, struct gyro_arg_accel_range arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->set_accel_range, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->set_accel_range(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_get_gyro_range(gyro_handle phandle, struct gyro_arg_gyro_range *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->get_gyro_range, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->get_gyro_range(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_get_gyro_sensitivity(gyro_handle phandle, struct gyro_arg_sensitivity *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->get_gyro_sensitivity, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->get_gyro_sensitivity(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_get_accel_range(gyro_handle phandle, struct gyro_arg_accel_range *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->get_accel_range, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->get_accel_range(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_get_accel_sensitivity(gyro_handle phandle, struct gyro_arg_sensitivity *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->get_accel_sensitivity, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->get_accel_sensitivity(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_read_fifodata(gyro_handle phandle, u8 *fifo_data, u16 fifo_cnt)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(fifo_data, NULL);
    PARAM_SAFE(p_dev->sensor_ops->read_fifo_data, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->read_fifo_data(p_dev, fifo_data, fifo_cnt);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_read_fifocnt(gyro_handle phandle, u16 *fifo_cnt)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(fifo_cnt, NULL);
    PARAM_SAFE(p_dev->sensor_ops->read_fifo_cnt, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->read_fifo_cnt(p_dev, fifo_cnt);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_reset_fifo(gyro_handle phandle)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->reset_fifo, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->reset_fifo(p_dev);
    UNLOCK_DEV(p_dev);

    return ret;
}

__always_unused int gyro_read_gyro_xyz(gyro_handle phandle, struct gyro_arg_gyro_xyz *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;
    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(arg, NULL);
    return ret;
}

__always_unused int gyro_read_accel_xyz(gyro_handle phandle, struct gyro_arg_accel_xyz *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;
    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(arg, NULL);
    return ret;
}

__always_unused int gyro_read_temp(gyro_handle phandle, struct gyro_arg_temp *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;
    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(arg, NULL);

    return ret;
}

int gyro_set_dev_mode(gyro_handle phandle, struct gyro_arg_dev_mode dev_mode, struct gyro_arg_fifo_info *fifo_info)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(fifo_info, NULL);
    PARAM_SAFE(p_dev->sensor_ops->enable_fifo, NULL);

    memset(fifo_info, 0xff, sizeof(struct gyro_arg_fifo_info));
    fifo_info->is_big_endian = 1;

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->enable_fifo(p_dev, dev_mode, fifo_info);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_get_group_delay(gyro_handle phandle, struct gyro_arg_group_delay *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->get_group_delay, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->get_group_delay(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

__always_unused int gyro_get_noise_bandwidth(gyro_handle phandle, struct gyro_arg_noise_bandwidth *arg)
{
    int              ret   = GYRO_SUCCESS;
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->get_noise_bandwidth, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->get_noise_bandwidth(p_dev, arg);
    UNLOCK_DEV(p_dev);

    return ret;
}

int gyro_whoami_verify(gyro_handle phandle)
{
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;
    int              ret   = 0;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->whoami_verify, NULL);

    LOCK_DEV(p_dev);
    ret = p_dev->sensor_ops->whoami_verify(p_dev);
    UNLOCK_DEV(p_dev);
    return GYRO_SUCCESS;
}

int gyro_enable(gyro_handle phandle)
{
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;
    int              ret   = 0;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->init, NULL);

    if (atomic_read(&p_dev->use_count) > 0)
    {
        atomic_inc(&p_dev->use_count);
        pr_info("gyro dev[%d] is already inited\n", p_dev->u8_devid);
        return -EBUSY;
    }

    // don't need lock dev
    ret = p_dev->sensor_ops->init(p_dev);
    if (ret != GYRO_SUCCESS)
    {
        GYRO_ERR("gyro init fail, ret[%d]", ret);
        return ret;
    }

    atomic_inc(&p_dev->use_count);
    return GYRO_SUCCESS;
}

int gyro_disable(gyro_handle phandle)
{
    struct gyro_dev *p_dev = (struct gyro_dev *)phandle;

    PARAM_SAFE(p_dev, NULL);
    PARAM_SAFE(p_dev->sensor_ops->deinit, NULL);

    // don't need lock dev
    if (atomic_read(&p_dev->use_count) > 1)
    {
        atomic_dec(&p_dev->use_count);
        pr_info("gyro dev[%d] is used\n", p_dev->u8_devid);
        return -EBUSY;
    }

    p_dev->sensor_ops->deinit(p_dev);
    atomic_dec(&p_dev->use_count);

    return GYRO_SUCCESS;
}

#ifndef SS_GYRO_UT
static MHAL_DIS_GyroRegisterHander_t dis_register_gyro_hander = {
    .pGyroSetSampleRate       = gyro_set_sample_rate,
    .pGyroGetSampleRate       = gyro_get_sample_rate,
    .pGyroSetGyroRange        = gyro_set_gyro_range,
    .pGyroSetAccelRange       = gyro_set_accel_range,
    .pGyroGetGyroRange        = gyro_get_gyro_range,
    .pGyroGetAccelRange       = gyro_get_accel_range,
    .pGyroGetGyroSensitivity  = gyro_get_gyro_sensitivity,
    .pGyroGetAccelSensitivity = gyro_get_gyro_sensitivity,
    .pGyroReadFifodata        = gyro_read_fifodata,
    .pGyroReadFifocnt         = gyro_read_fifocnt,
    .pGyroResetFifo           = gyro_reset_fifo,
    .pGyroReadGyroXyz         = gyro_read_gyro_xyz,
    .pGyroReadAccelXyz        = gyro_read_accel_xyz,
    .pGyroReadTemp            = gyro_read_temp,
    .pGyroSetDevMode          = gyro_set_dev_mode,
    .pGyroEnable              = gyro_enable,
    .pGyroDisable             = gyro_disable,
};

#define GYRO_CALL_SYMBOL(name, type, ...)      \
    do                                         \
    {                                          \
        type pSymFunc = __symbol_get(#name);   \
        if (pSymFunc)                          \
        {                                      \
            pSymFunc(__VA_ARGS__);             \
            GYRO_INFO("%s Done.", #name);      \
            __symbol_put(#name);               \
        }                                      \
        else                                   \
        {                                      \
            GYRO_INFO("%s not found.", #name); \
        }                                      \
    } while (0)

__attribute__((weak)) int MHal_DIS_RegisterGyroHanderCallback(MHAL_DIS_GyroRegisterHander_t *pHander);
__attribute__((weak)) int MHal_DIS_UnRegisterGyroHanderCallback(void);
typedef int (*MHAL_DIS_Register_t)(MHAL_DIS_GyroRegisterHander_t *);
typedef int (*MHAL_DIS_UnRegister_t)(void);
#endif

int gyro_core_init(gyro_handle *ppDevHanle, unsigned char u8ValidDevNum)
{
    GYRO_DBG("gyro_core init");

    if (u8ValidDevNum == 0)
    {
        GYRO_WRN("gyro valid dev number is zero");
    }
    else
    {
        GYRO_INFO("gyro valid dev number is [%u]", u8ValidDevNum);
    }

    GYRO_CALL_SYMBOL(MHal_DIS_RegisterGyroHanderCallback, MHAL_DIS_Register_t, &dis_register_gyro_hander);

    return 0;
}

void gyro_core_deinit(void)
{
    GYRO_CALL_SYMBOL(MHal_DIS_UnRegisterGyroHanderCallback, MHAL_DIS_UnRegister_t);

    GYRO_DBG("gyro_core deinit");
}

MODULE_LICENSE("GPL");
