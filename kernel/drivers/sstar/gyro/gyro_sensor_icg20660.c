/*
 * gyro_sensor_icg20660.c- Sigmastar
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

#include "gyro.h"
#include "gyro_core.h"
#include "gyro_internal.h"
#include <linux/delay.h>

#define GYROSENSOR_ICG20660_INT_LEVEL_L       0x80
#define GYROSENSOR_ICG20660_INT_LEVEL_H       0x00
#define GYROSENSOR_ICG20660_INT_OPEN_DRAIN    0x40
#define GYROSENSOR_ICG20660_INT_PUSH_PULL     0x00
#define GYROSENSOR_ICG20660_LATCH_INT_EN      0x20
#define GYROSENSOR_ICG20660_INT_READ_CLEA     0x10
#define GYROSENSOR_ICG20660_FSYNC_INT_LEVEL_L 0x08
#define GYROSENSOR_ICG20660_FSYNC_INT_LEVEL_H 0x00
#define GYROSENSOR_ICG20660_FSYNC_INT_MODEN   0x04

#define GYROSENSOR_ICG20660_INT_NONE       0x00
#define GYROSENSOR_ICG20660_INT_FIFO_FULL  0x10
#define GYROSENSOR_ICG20660_INT_DATA_READY 0x01

#define GYROSENSOR_ICG20660_FIFO_RD_EN     0x40
#define GYROSENSOR_ICG20660_SPI_INTERFACEN 0x10
#define GYROSENSOR_ICG20660_RESET_FIFO     0x04

enum
{
    GSEN_ICG20660_SELF_TEST_X_GYRO = 0x00,
    GSEN_ICG20660_SELF_TEST_Y_GYRO = 0x01,
    GSEN_ICG20660_SELF_TEST_Z_GYRO = 0x02,

    GSEN_ICG20660_XG_OFFS_TC_H = 0x04,
    GSEN_ICG20660_XG_OFFS_TC_L = 0x05,
    GSEN_ICG20660_YG_OFFS_TC_H = 0x06,
    GSEN_ICG20660_YG_OFFS_TC_L = 0x07,
    GSEN_ICG20660_ZG_OFFS_TC_H = 0x08,
    GSEN_ICG20660_ZG_OFFS_TC_L = 0x09,
    GSEN_ICG20660_XG_OFFS_USRH = 0x13,
    GSEN_ICG20660_XG_OFFS_USRL = 0x14,
    GSEN_ICG20660_YG_OFFS_USRH = 0x15,
    GSEN_ICG20660_YG_OFFS_USRL = 0x16,
    GSEN_ICG20660_ZG_OFFS_USRH = 0x17,
    GSEN_ICG20660_ZG_OFFS_USRL = 0x18,
    GSEN_ICG20660_SMPLRT_DIV   = 0x19,
    GSEN_ICG20660_CONFIG       = 0x1A,
    GSEN_ICG20660_GYRO_CONFIG  = 0x1B,

    GSEN_ICG20660_ACCEL_CONFIG  = 0x1C,
    GSEN_ICG20660_ACCEL_CONFIG2 = 0x1D,
    GSEN_ICG20660_FIFO_EN       = 0x23,

    GSEN_ICG20660_FSYNC_INT_STATUS = 0x36,
    GSEN_ICG20660_INT_PIN_CFG      = 0x37,

    GSEN_ICG20660_INT_ENABLE = 0x38,
    GSEN_ICG20660_INT_STATUS = 0x3A,

    GSEN_ICG20660_ACCEL_XOUT_H      = 0x3B,
    GSEN_ICG20660_ACCEL_XOUT_L      = 0x3C,
    GSEN_ICG20660_ACCEL_YOUT_H      = 0x3D,
    GSEN_ICG20660_ACCEL_YOUT_L      = 0x3E,
    GSEN_ICG20660_ACCEL_ZOUT_H      = 0x3F,
    GSEN_ICG20660_ACCEL_ZOUT_L      = 0x40,
    GSEN_ICG20660_TEMP_OUT_H        = 0x41,
    GSEN_ICG20660_TEMP_OUT_L        = 0x42,
    GSEN_ICG20660_GYRO_XOUT_H       = 0x43,
    GSEN_ICG20660_GYRO_XOUT_L       = 0x44,
    GSEN_ICG20660_GYRO_YOUT_H       = 0x45,
    GSEN_ICG20660_GYRO_YOUT_L       = 0x46,
    GSEN_ICG20660_GYRO_ZOUT_H       = 0x47,
    GSEN_ICG20660_GYRO_ZOUT_L       = 0x48,
    GSEN_ICG20660_SIGNAL_PATH_RESET = 0x68,
    GSEN_ICG20660_ACCEL_INTEL_CTRL  = 0x69,
    GSEN_ICG20660_USER_CTRL         = 0x6A,

    GSEN_ICG20660_PWR_MGMT_1  = 0x6B,
    GSEN_ICG20660_PWR_MGMT_2  = 0x6C,
    GSEN_ICG20660_FIFO_COUNTH = 0x72,
    GSEN_ICG20660_FIFO_COUNTL = 0x73,
    GSEN_ICG20660_FIFO_R_W    = 0x74,
    GSEN_ICG20660_WHO_AM_I    = 0x75,
    GSEN_ICG20660_XA_OFFSET_H = 0x77,
    GSEN_ICG20660_XA_OFFSET_L = 0x78,
    GSEN_ICG20660_YA_OFFSET_H = 0x7A,
    GSEN_ICG20660_YA_OFFSET_L = 0x7B,
    GSEN_ICG20660_ZA_OFFSET_H = 0x7D,
    GSEN_ICG20660_ZA_OFFSET_L = 0x7E
};

static int _icg20660_reset_fifo(struct gyro_dev *dev);

static int _icg20660_init(struct gyro_dev *dev)
{
    int ret = 0;
    u8  val = 0;
    ret     = dev->reg_ops->write_reg(dev, GSEN_ICG20660_PWR_MGMT_1, 0x80);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_PWR_MGMT_1, ret:[%d]", ret);
        return ret;
    }

    /*!
     * Note: Need waiting for the reset operation done
     * When the reset is complete, it will automatically enter sleep mode
     */
    msleep(10);
    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_PWR_MGMT_1, 0x01);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_PWR_MGMT_1 fail, ret:[%d]", ret);
        return ret;
    }

    /*!
     * Note: Need to wait to wake up from sleep mode
     */
    usleep_range(1, 10);

    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_INT_ENABLE, GYROSENSOR_ICG20660_INT_DATA_READY);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_INT_ENABLE fail, ret:[%d]", ret);
        return ret;
    }

    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_CONFIG, 0x51);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_CONFIG fail, ret:[%d]", ret);
        return ret;
    }

    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_INT_PIN_CFG, GYROSENSOR_ICG20660_FSYNC_INT_MODEN);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_INT_PIN_CFG fail, ret:[%d]", ret);
        return ret;
    }

    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_SMPLRT_DIV, 0);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_SMPLRT_DIV fail, ret:[%d]", ret);
        return ret;
    }

    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_GYRO_CONFIG, 0x08);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_GYRO_CONFIG fail, ret:[%d]", ret);
        return ret;
    }

    dev->reg_ops->read_reg(dev, GSEN_ICG20660_INT_PIN_CFG, &val);
    GYRO_INFO("icg20660 init success");
    return 0;
}

static void _icg20660_deinit(struct gyro_dev *dev)
{
    int ret = 0;

    _icg20660_reset_fifo(dev);
    msleep(10);

    // reset the gyro device
    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_PWR_MGMT_1, 0x80);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_PWR_MGMT_1, ret:[%d]", ret);
        return;
    }

    /*!
     * Note: Need waiting for the reset operation done
     */
    msleep(10);

    // gyro sleep mode
    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_PWR_MGMT_1, 0x40);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICG20660_PWR_MGMT_1 fail, ret:[%d]", ret);
        return;
    }

    GYRO_INFO("icg20660 deinit success");
}

static int _icg20660_enable_fifo(struct gyro_dev *dev, struct gyro_arg_dev_mode mode,
                                 struct gyro_arg_fifo_info *fifo_info)
{
    int ret    = 0;
    u8  val    = 0x00;
    u8  offset = 0;
    int i      = 0;
    struct __icg20660_fifo_info
    {
        u8  fifo_type;
        u8 *axis_start;
        u8 *axis_end;
        u8  size;
        u8  reg_setting;
    } info[] = {
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->ax_start,
         &fifo_info->ax_end, 2, 0x08},
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->ay_start,
         &fifo_info->ay_end, 2, 0x08},
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->az_start,
         &fifo_info->az_end, 2, 0x08},
        {GYROSENSOR_TEMP_FIFO_EN, &fifo_info->temp_start, &fifo_info->temp_end, 2, 0x80},
        {GYROSENSOR_XG_FIFO_EN, &fifo_info->gx_start, &fifo_info->gx_end, 2, 0x40},
        {GYROSENSOR_YG_FIFO_EN, &fifo_info->gy_start, &fifo_info->gy_end, 2, 0x20},
        {GYROSENSOR_ZG_FIFO_EN, &fifo_info->gz_start, &fifo_info->gz_end, 2, 0x10},
    };

    if (mode.fifo_mode)
    {
        for (i = 0; i < sizeof(info) / sizeof(info[0]); ++i)
        {
            if (mode.fifo_type & (info[i].fifo_type))
            {
                *info[i].axis_start = offset;
                *info[i].axis_end   = offset + info[i].size - 1;
                val |= info[i].reg_setting;
                offset += 2;
            }
        }
        fifo_info->bytes_pre_data = offset;
        fifo_info->max_fifo_cnt   = 512;
        fifo_info->is_big_endian  = 1;
    }
    else
    {
        val = 0;
    }

    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_FIFO_EN, val);
    ret = dev->reg_ops->read_reg(dev, GSEN_ICG20660_USER_CTRL, &val);
    val |= GYROSENSOR_ICG20660_FIFO_RD_EN;
    ret = dev->reg_ops->write_reg(dev, GSEN_ICG20660_USER_CTRL, val);
    return ret;
}

static int _icg20660_set_sample_rate(struct gyro_dev *dev, struct gyro_arg_sample_rate rate)
{
    u8 div = 0;
    switch (rate.rate)
    {
        case 1000:
        {
            div = 0;
        }
        break;

        default:
        {
            div = 1;
        }
        break;
    }
    return dev->reg_ops->write_reg(dev, GSEN_ICG20660_SMPLRT_DIV, div);
}
static int _icg20660_get_sample_rate(struct gyro_dev *dev, struct gyro_arg_sample_rate *rate)
{
    int ret = 0;
    u8  div = 0;
    dev->reg_ops->read_reg(dev, GSEN_ICG20660_SMPLRT_DIV, &div);
    switch (div)
    {
        case 0:
            (*rate).rate = 1000;
            break;
        case 1:
            (*rate).rate = 500;
            break;
        default:
            GYRO_ERR("sample rate div is %d.", div);
            return -1;
    }
    return ret;
}
static int _icg20660_set_gyro_range(struct gyro_dev *dev, struct gyro_arg_gyro_range range)
{
    int ret = 0;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICG20660_GYRO_CONFIG, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= ~(0x18);

    switch (range.range)
    {
        case 125:
            val |= 0x00;
            break;
        case 250:
            val |= 0x08;
            break;
        case 500:
            val |= 0x10;
            break;

        default:
            GYRO_ERR("gyro range %u is not support.", range.range);
            return -1;
    }

    return dev->reg_ops->write_reg(dev, GSEN_ICG20660_GYRO_CONFIG, val);
}
static int _icg20660_set_accel_range(struct gyro_dev *dev, struct gyro_arg_accel_range range)
{
    int ret = 0;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICG20660_ACCEL_CONFIG, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= ~(0x18);

    switch (range.range)
    {
        case 2:
            val |= 0x00;
            break;
        case 4:
            val |= 0x08;
            break;
        case 8:
            val |= 0x10;
            break;
        case 16:
            val |= 0x18;
            break;

        default:
            GYRO_ERR("accel range %u is not support.", range.range);
            return -1;
    }

    return dev->reg_ops->write_reg(dev, GSEN_ICG20660_ACCEL_CONFIG, val);
}
static int _icg20660_get_gyro_range(struct gyro_dev *dev, struct gyro_arg_gyro_range *range)
{
    int ret = 0;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICG20660_GYRO_CONFIG, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0x18;

    switch (val)
    {
        case 0x00:
            (*range).range = 125;
            break;
        case 0x08:
            (*range).range = 250;
            break;
        case 0x10:
            (*range).range = 500;
            break;

        default:
            GYRO_ERR("gyro range is 0x%x.", val);
            return -1;
    }

    return ret;
}
static int _icg20660_get_gyro_sensitivity(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity)
{
    int ret = 0;
    u8  val = 0;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICG20660_GYRO_CONFIG, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0x18;

    switch (val)
    {
        case 0x00:
            (*sensitivity).num = 262;
            (*sensitivity).den = 1;
            break;
        case 0x08:
            (*sensitivity).num = 131;
            (*sensitivity).den = 1;
            break;
        case 0x10:
            (*sensitivity).num = 655;
            (*sensitivity).den = 10;
            break;
        default:
            GYRO_ERR("gyro sensitivity is 0x%x", val);
            return -1;
    }

    return ret;
}
static int _icg20660_get_accel_range(struct gyro_dev *dev, struct gyro_arg_accel_range *range)
{
    int ret = 0;
    u8  val;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICG20660_ACCEL_CONFIG, &val);
    if (ret != 0)
    {
        return ret;
    }

    val &= 0x18;

    switch (val)
    {
        case 0x00:
            (*range).range = 2;
            break;
        case 0x08:
            (*range).range = 4;
            break;
        case 0x10:
            (*range).range = 8;
            break;
        case 0x18:
            (*range).range = 16;
            break;

        default:
            GYRO_ERR("accel range is 0x%x", val);
            return -1;
    }

    return ret;
}
static int _icg20660_get_accel_sensitivity(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity)
{
    int ret = 0;
    u8  val = 0;

    ret = dev->reg_ops->read_reg(dev, GSEN_ICG20660_ACCEL_CONFIG, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0x18;

    switch (val)
    {
        case 0x00:
            (*sensitivity).num = 16384;
            (*sensitivity).den = 1;
            break;
        case 0x08:
            (*sensitivity).num = 8192;
            (*sensitivity).den = 1;
            break;
        case 0x10:
            (*sensitivity).num = 4096;
            (*sensitivity).den = 1;
            break;
        case 0x18:
            (*sensitivity).num = 2048;
            (*sensitivity).den = 1;
            break;

        default:
            GYRO_ERR("accel sensitivity is %d", val);
            return -1;
    }

    return ret;
}

static int _icg20660_read_fifo_cnt(struct gyro_dev *dev, u16 *fifo_cnt)
{
    u8 val[2];

    dev->reg_ops->read_regs(dev, GSEN_ICG20660_FIFO_COUNTH, val, 2);
    *fifo_cnt = (val[0] << 8) + val[1];
    return 0;
}

static int _icg20660_read_fifo_data(struct gyro_dev *dev, u8 *fifo_data, u16 fifo_cnt)
{
    dev->reg_ops->read_regs(dev, GSEN_ICG20660_FIFO_R_W, fifo_data, fifo_cnt);
    return 0;
}

static int _icg20660_reset_fifo(struct gyro_dev *dev)
{
    return dev->reg_ops->write_reg(dev, GSEN_ICG20660_USER_CTRL,
                                   GYROSENSOR_ICG20660_FIFO_RD_EN | GYROSENSOR_ICG20660_RESET_FIFO);
}

static int _icg20660_get_group_delay(struct gyro_dev *dev, struct gyro_arg_group_delay *arg)
{
#define _SHIFT_GYRO_FCHOICE_B (0)
#define _MASK_GYRO_FCHOICE_B  (u8)(0b00000011)

#define _SHIFT_GYRO_DLPF_CFG (0)
#define _MASK_GYRO_DLPF_CFG  (u8)(0b00000111)

    u8 fchoice_b = 0;
    u8 dlpf_cfg  = 0;

    dev->reg_ops->read_reg(dev, GSEN_ICG20660_GYRO_CONFIG, &fchoice_b);
    dev->reg_ops->read_reg(dev, GSEN_ICG20660_CONFIG, &dlpf_cfg);

    fchoice_b = ((fchoice_b & _MASK_GYRO_FCHOICE_B) >> _SHIFT_GYRO_FCHOICE_B);
    dlpf_cfg  = ((dlpf_cfg & _MASK_GYRO_DLPF_CFG) >> _SHIFT_GYRO_DLPF_CFG);

    if (fchoice_b == 0 && dlpf_cfg == 0)
    {
        arg->delay_us = 970;
    }
    else if (fchoice_b == 0 && dlpf_cfg == 1)
    {
        arg->delay_us = 2900;
    }
    else if (fchoice_b == 0 && dlpf_cfg == 2)
    {
        arg->delay_us = 3900;
    }
    else if (fchoice_b == 0 && dlpf_cfg == 7)
    {
        arg->delay_us = 170;
    }
    else if (fchoice_b == 2)
    {
        arg->delay_us = 110;
    }
    else
    {
        arg->delay_us = 64;
    }

    GYRO_INFO("fchoice_b[%u] dlpf_cfg[%u] delay[%u]\n", fchoice_b, dlpf_cfg, arg->delay_us);

    return GYRO_SUCCESS;
}

static int _icg20660_whoami_verify(struct gyro_dev *dev)
{
#define WHO_AM_I_VALUE 0x90
    u8  val = 0;
    int ret = 0;
    ret     = dev->reg_ops->read_reg(dev, GSEN_ICG20660_WHO_AM_I, &val);
    if (ret != 0)
    {
        return ret;
    }

    if (val != WHO_AM_I_VALUE)
    {
        GYRO_ERR("who am i ID err, err val[%u] and correct val[%u]", val, WHO_AM_I_VALUE);
        ret = -1;
    }

    return ret;
}

gyro_sensor_context_t icg20660_context = {
    .list_head =
        {
            .next = NULL,
            .prev = NULL,
        },
    .ops =
        {
            .early_init      = NULL,
            .final_deinit    = NULL,
            .init            = _icg20660_init,
            .deinit          = _icg20660_deinit,
            .enable_fifo     = _icg20660_enable_fifo,
            .set_sample_rate = _icg20660_set_sample_rate,
            .get_sample_rate = _icg20660_get_sample_rate,

            .set_gyro_range  = _icg20660_set_gyro_range,
            .set_accel_range = _icg20660_set_accel_range,

            .get_gyro_range       = _icg20660_get_gyro_range,
            .get_gyro_sensitivity = _icg20660_get_gyro_sensitivity,

            .get_accel_range       = _icg20660_get_accel_range,
            .get_accel_sensitivity = _icg20660_get_accel_sensitivity,

            .read_fifo_data      = _icg20660_read_fifo_data,
            .read_fifo_cnt       = _icg20660_read_fifo_cnt,
            .reset_fifo          = _icg20660_reset_fifo,
            .whoami_verify       = _icg20660_whoami_verify,
            .get_group_delay     = _icg20660_get_group_delay,
            .get_noise_bandwidth = NULL,
        },
};

ADD_SENSOR_CONTEXT(SENSOR_TYPE, icg20660_context);
