/*
 * gyro_sensor_icm42607.c - Sigmastar
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

#include <linux/delay.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include "gyro_internal.h"
#include "gyro_core.h"
#include "gyro.h"

#define GYROSENSOR_REG_WHO_AM_I       0x75
#define GYROSENSOR_ICM40607_WHO_AM_I  0x38
#define GYROSENSOR_ICM40607E_WHO_AM_I 0x45
#define GYROSENSOR_ICM42607_WHO_AM_I  0x60
#define GYROSENSOR_ICM42627_WHO_AM_I  0x20

enum
{
    GSEN_ICM42607_MCLK_RDY          = 0x00,
    GSEN_ICM42607_DEVICE_CONFIG     = 0x01,
    GSEN_ICM42607_SIGNAL_PATH_RESET = 0x02,
    GSEN_ICM42607_INT_CONFIG        = 0x06,
    GSEN_ICM42607_ACCEL_DATA_X1     = 0x0B,
    GSEN_ICM42607_PWR_MGMT0         = 0x1F,
    GSEN_ICM42607_GYRO_CONFIG0      = 0x20,
    GSEN_ICM42607_ACCEL_CONFIG0     = 0x21,
    GSEN_ICM42607_GYRO_CONFIG1      = 0x23,
    GSEN_ICM42607_FIFO_CONFIG1      = 0x28,
    GSEN_ICM42607_FIFO_CONFIG2      = 0x29,
    GSEN_ICM42607_FIFO_CONFIG3      = 0x2A,
    GSEN_ICM42607_INT_SOURCE0       = 0x2B,
    GSEN_ICM42607_INTF_CONFIG0      = 0x35,
    GSEN_ICM42607_INTF_CONFIG1      = 0x36,
    GSEN_ICM42607_INT_STATUS        = 0x3A,
    GSEN_ICM42607_FIFO_COUNTH       = 0x3D,
    GSEN_ICM42607_FIFO_COUNTL       = 0x3E,
    GSEN_ICM42607_FIFO_DATA         = 0x3F,
    GSEN_ICM42607_BLK_SEL_W         = 0x79,
    GSEN_ICM42607_MADDR_W           = 0x7A,
    GSEN_ICM42607_M_W               = 0x7B,
    GSEN_ICM42607_BLK_SEL_R         = 0x7C,
    GSEN_ICM42607_MADDR_R           = 0x7D,
    GSEN_ICM42607_M_R               = 0x7E,

    GSEN_ICM42607_TMST_CONFIG1   = 0x00,
    GSEN_ICM42607_FIFO_CONFIG5   = 0x01,
    GSEN_ICM42607_INT_CONFIG1    = 0x05,
    GSEN_ICM42607_SENSOR_CONFIG3 = 0x06,
};

enum chip_type
{
    ICM40607 = 0,
    ICM42607,
    UNVALID_TYPE,
};

static enum chip_type product       = ICM42607;
static int            need_mclk_cnt = 0;

static int switch_on_mclk(struct gyro_dev *dev)
{
    int ret = 0;
    // u8 pwr_mgt;
    u8 data;
    if (need_mclk_cnt == 0)
    {
        ret |= dev->reg_ops->read_reg(dev, GSEN_ICM42607_PWR_MGMT0, &data);
        data |= 0x10;
        ret |= dev->reg_ops->write_reg(dev, GSEN_ICM42607_PWR_MGMT0, data);
        do
        {
            ret |= dev->reg_ops->read_reg(dev, GSEN_ICM42607_MCLK_RDY, &data);
            if (ret < 0)
            {
                return ret;
            }
            mdelay(1);
        } while (!(data & 0x08));
    }
    need_mclk_cnt++;
    return ret;
}

static int switch_off_mclk(struct gyro_dev *dev)
{
    int ret = 0;
    u8  data;
    if (need_mclk_cnt == 1)
    {
        ret |= dev->reg_ops->read_reg(dev, GSEN_ICM42607_PWR_MGMT0, &data);
        data &= ~0x10;
        ret |= dev->reg_ops->write_reg(dev, GSEN_ICM42607_PWR_MGMT0, data);
    }
    need_mclk_cnt--;
    return ret;
}

static int write_mclk_reg(struct gyro_dev *dev, u8 blk, u8 reg_addr, u32 len, u8 value)
{
    int ret = 0;
    int i;
    u8  blkoff = 0;

    for (i = 0; i < len; i++)
    {
        ret |= switch_on_mclk(dev);
        ret |= dev->reg_ops->write_reg(dev, GSEN_ICM42607_BLK_SEL_W, blk);
        ret |= dev->reg_ops->write_reg(dev, GSEN_ICM42607_MADDR_W, reg_addr);
        mdelay(1);
        ret |= dev->reg_ops->write_reg(dev, GSEN_ICM42607_M_W, value);
        mdelay(1);
        ret |= dev->reg_ops->write_reg(dev, GSEN_ICM42607_BLK_SEL_W, blkoff);
        ret |= switch_off_mclk(dev);
    }
    return ret;
}

static int icm42607_init(struct gyro_dev *dev)
{
    int i;
    int ret      = 0;
    u8  who_am_i = 0;

    for (i = 0; i < 10; i++)
    {
        ret = dev->reg_ops->read_reg(dev, GYROSENSOR_REG_WHO_AM_I, &who_am_i);
        GYRO_INFO("who_am_i = 0x%x", who_am_i);
        if (ret < 0)
        {
            GYRO_ERR("get whoami err %d", ret);
            mdelay(2);
        }
        else
        {
            break;
        }
    }
    if (ret < 0)
    {
        return ret;
    }
    switch (who_am_i)
    {
        case GYROSENSOR_ICM40607_WHO_AM_I:
        case GYROSENSOR_ICM40607E_WHO_AM_I:
        case GYROSENSOR_ICM42627_WHO_AM_I:
            product = ICM40607;
            GYRO_INFO("ICM40607 detected");
            break;
        case GYROSENSOR_ICM42607_WHO_AM_I:
            product = ICM42607;
            GYRO_INFO("ICM42607 detected");
            break;
    }

    if (product == ICM42607)
    {
        ret = dev->reg_ops->write_reg(dev, GSEN_ICM42607_SIGNAL_PATH_RESET, 0x10);
        if (ret < 0)
        {
            return ret;
        }
        mdelay(100);

        ret = dev->reg_ops->write_reg(dev, GSEN_ICM42607_FIFO_CONFIG1, 0x02);
        if (ret < 0)
        {
            return ret;
        }

        ret = dev->reg_ops->write_reg(dev, GSEN_ICM42607_GYRO_CONFIG0, 0x06);
        if (ret < 0)
        {
            return ret;
        }

        ret = dev->reg_ops->write_reg(dev, GSEN_ICM42607_ACCEL_CONFIG0, 0x66);
        if (ret < 0)
        {
            return ret;
        }

        ret = dev->reg_ops->write_reg(dev, GSEN_ICM42607_PWR_MGMT0, 0x0F);
        if (ret < 0)
        {
            return ret;
        }
    }
    else
    {
        GYRO_ERR("Gyro detected is not ICM42607\n");
    }
    mdelay(80);

    return 0;
}

static int icm42607_enable_fifo(struct gyro_dev *dev, struct gyro_arg_dev_mode mode,
                                struct gyro_arg_fifo_info *fifo_info)
{
    int ret    = 0;
    u8  val    = 0;
    u8  offset = 0;
    u8  tmp    = 0;
    int i      = 0;
    struct __icg42607_fifo_info
    {
        u8  fifo_type;
        u8 *axis_start;
        u8 *axis_end;
        u8  size;
        u8  reg_setting;
    } info[] = {
        {0xff, &tmp, &tmp, 1, 0x00},
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->ax_start,
         &fifo_info->ax_end, 2, 0x01},
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->ay_start,
         &fifo_info->ay_end, 2, 0x01},
        {GYROSENSOR_ZA_FIFO_EN | GYROSENSOR_YA_FIFO_EN | GYROSENSOR_XA_FIFO_EN, &fifo_info->az_start,
         &fifo_info->az_end, 2, 0x01},
        {GYROSENSOR_XG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_ZG_FIFO_EN, &fifo_info->gx_start,
         &fifo_info->gx_end, 2, 0x02},
        {GYROSENSOR_YG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_ZG_FIFO_EN, &fifo_info->gy_start,
         &fifo_info->gy_end, 2, 0x02},
        {GYROSENSOR_ZG_FIFO_EN | GYROSENSOR_YG_FIFO_EN | GYROSENSOR_ZG_FIFO_EN, &fifo_info->gz_start,
         &fifo_info->gz_end, 2, 0x02},
        {0xff, &fifo_info->temp_start, &fifo_info->temp_end, 1, 0x00},
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
                offset += info[i].size;
            }
        }
        fifo_info->bytes_pre_data = offset > 8 ? 16 : offset;
        fifo_info->max_fifo_cnt   = 2304;
        fifo_info->is_big_endian  = 1;
    }
    else
    {
        val = 0;
    }

    ret = write_mclk_reg(dev, 0x00, GSEN_ICM42607_FIFO_CONFIG5, 1, val);
    ret = write_mclk_reg(dev, 0x00, GSEN_ICM42607_SENSOR_CONFIG3, 1, 255);

    return ret;
}

static int icm42607_set_sample_rate(struct gyro_dev *dev, struct gyro_arg_sample_rate rate)
{
    u8 gyro_cfg_val  = 0;
    u8 accel_cfg_val = 0;
    u8 gyro_reg      = GSEN_ICM42607_GYRO_CONFIG0;
    u8 accel_reg     = GSEN_ICM42607_ACCEL_CONFIG0;

    dev->reg_ops->read_reg(dev, gyro_reg, &gyro_cfg_val);
    dev->reg_ops->read_reg(dev, accel_reg, &accel_cfg_val);

    gyro_cfg_val &= 0xf0;
    accel_cfg_val &= 0xf0;
    switch (rate.rate)
    {
        case 1600:
        {
            gyro_cfg_val |= 0x05;
            accel_cfg_val |= 0x05;
        }
        break;
        case 800:
        {
            gyro_cfg_val |= 0x06;
            accel_cfg_val |= 0x06;
        }
        break;
        case 400:
        {
            gyro_cfg_val |= 0x07;
            accel_cfg_val |= 0x07;
        }
        break;
        case 200:
        {
            gyro_cfg_val |= 0x08;
            accel_cfg_val |= 0x08;
        }
        break;
        case 100:
        {
            gyro_cfg_val |= 0x09;
            accel_cfg_val |= 0x09;
        }
        break;
        case 50:
        {
            gyro_cfg_val |= 0x0A;
            accel_cfg_val |= 0x0A;
        }
        break;
        case 25:
        {
            gyro_cfg_val |= 0x0B;
            accel_cfg_val |= 0x0B;
        }
        break;
        default:
        {
            GYRO_ERR("sample rate is not supported.");
            return -1;
        }
    }
    dev->reg_ops->write_reg(dev, gyro_reg, gyro_cfg_val);
    dev->reg_ops->write_reg(dev, accel_reg, accel_cfg_val);
    return 0;
}

static int icm42607_get_sample_rate(struct gyro_dev *dev, struct gyro_arg_sample_rate *rate)
{
    u8 gyro_cfg_val  = 0;
    u8 accel_cfg_val = 0;
    u8 gyro_reg      = GSEN_ICM42607_GYRO_CONFIG0;
    u8 accel_reg     = GSEN_ICM42607_ACCEL_CONFIG0;

    dev->reg_ops->read_reg(dev, gyro_reg, &gyro_cfg_val);
    dev->reg_ops->read_reg(dev, accel_reg, &accel_cfg_val);
    gyro_cfg_val &= 0x0f;
    accel_cfg_val &= 0x0f;
    if (gyro_cfg_val != accel_cfg_val)
    {
        GYRO_ERR("sample rate is different.");
        return -1;
    }
    switch (gyro_cfg_val)
    {
        case 0x05:
            (*rate).rate = 1600;
            break;
        case 0x06:
            (*rate).rate = 800;
            break;
        case 0x07:
            (*rate).rate = 400;
            break;
        case 0x08:
            (*rate).rate = 200;
            break;
        case 0x09:
            (*rate).rate = 100;
            break;
        case 0x0A:
            (*rate).rate = 50;
            break;
        case 0x0B:
            (*rate).rate = 25;
            break;
        default:
            GYRO_ERR("sample rate 0x%x", gyro_cfg_val);
            return -1;
    }
    return 0;
}

static int icm42607_get_group_delay(struct gyro_dev *dev, struct gyro_arg_group_delay *delay)
{
    int ret           = 0;
    u8  gyro_cfg0_val = 0;
    u8  gyro_cfg1_val = 0;
    u8  gyro_reg      = GSEN_ICM42607_GYRO_CONFIG0;
    u8  gyro_reg1     = GSEN_ICM42607_GYRO_CONFIG1;

    dev->reg_ops->read_reg(dev, gyro_reg1, &gyro_cfg1_val);
    gyro_cfg1_val &= 0x07;
    if (gyro_cfg1_val != 1)
    {
        GYRO_ERR("GYRO_UI_FILT_BW 0x%x\n ", gyro_cfg1_val);
        return -1;
    }

    ret = dev->reg_ops->read_reg(dev, gyro_reg, &gyro_cfg0_val);
    if (ret != 0)
    {
        return ret;
    }
    gyro_cfg0_val &= 0x0f;

    switch (gyro_cfg0_val)
    {
        case 0x05:
            (*delay).delay_us = 1000;
            break;
        case 0x06:
        case 0x07:
            (*delay).delay_us = 2000;
            break;
        case 0x08:
            (*delay).delay_us = 3000;
            break;
        case 0x09:
            (*delay).delay_us = 6000;
            break;
        case 0x0A:
            (*delay).delay_us = 11000;
            break;
        case 0x0B:
            (*delay).delay_us = 21000;
            break;
        case 0x0C:
            (*delay).delay_us = 41000;
            break;
        default:
            GYRO_ERR("group delay 0x%x", gyro_cfg0_val);
            return -1;
    }
    return 0;
}

static int icm42607_set_gyro_range(struct gyro_dev *dev, struct gyro_arg_gyro_range range)
{
    int ret = 0;
    u8  val;
    u8  gyro_reg = GSEN_ICM42607_GYRO_CONFIG0;

    ret = dev->reg_ops->read_reg(dev, gyro_reg, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= ~(0xe0);

    switch (range.range)
    {
        case 250:
            val |= 0x60;
            break;
        case 500:
            val |= 0x40;
            break;
        case 1000:
            val |= 0x20;
            break;
        case 2000:
            val |= 0x00;
            break;

        default:
            GYRO_ERR("gyro range is not supported.");
            return -1;
    }

    return dev->reg_ops->write_reg(dev, gyro_reg, val);
}

static int icm42607_set_accel_range(struct gyro_dev *dev, struct gyro_arg_accel_range range)
{
    int ret = 0;
    u8  val;
    u8  accel_reg = GSEN_ICM42607_ACCEL_CONFIG0;

    ret = dev->reg_ops->read_reg(dev, accel_reg, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= ~(0xe0);

    switch (range.range)
    {
        case 2:
            val |= 0x60;
            break;
        case 4:
            val |= 0x40;
            break;
        case 8:
            val |= 0x20;
            break;
        case 16:
            val |= 0x00;
            break;

        default:
            GYRO_ERR("accel range is not supported.");
            return -1;
    }

    return dev->reg_ops->write_reg(dev, accel_reg, val);
}

static int icm42607_get_gyro_range(struct gyro_dev *dev, struct gyro_arg_gyro_range *range)
{
    int ret = 0;
    u8  val;
    u8  gyro_reg = GSEN_ICM42607_GYRO_CONFIG0;

    ret = dev->reg_ops->read_reg(dev, gyro_reg, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*range).range = 2000;
            break;
        case 0x20:
            (*range).range = 1000;
            break;
        case 0x40:
            (*range).range = 500;
            break;
        case 0x60:
            (*range).range = 250;
            break;

        default:
            GYRO_ERR("gyro range 0x%x", val);
            return -1;
    }

    return ret;
}

static int icm42607_get_gyro_sensitivity(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity)
{
    int ret      = 0;
    u8  val      = 0;
    u8  gyro_reg = GSEN_ICM42607_GYRO_CONFIG0;

    ret = dev->reg_ops->read_reg(dev, gyro_reg, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*sensitivity).num = 164;
            (*sensitivity).den = 10;
            break;
        case 0x20:
            (*sensitivity).num = 328;
            (*sensitivity).den = 10;
            break;
        case 0x40:
            (*sensitivity).num = 655;
            (*sensitivity).den = 10;
            break;
        case 0x60:
            (*sensitivity).num = 131;
            (*sensitivity).den = 1;
            break;
        default:
            GYRO_ERR("gyro sensitivity 0x%x", val);
            return -1;
    }

    return ret;
}

static int icm42607_get_accel_range(struct gyro_dev *dev, struct gyro_arg_accel_range *range)
{
    int ret = 0;
    u8  val;
    u8  accel_reg = GSEN_ICM42607_ACCEL_CONFIG0;

    ret = dev->reg_ops->read_reg(dev, accel_reg, &val);
    if (ret != 0)
    {
        return ret;
    }

    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*range).range = 16;
            break;
        case 0x20:
            (*range).range = 8;
            break;
        case 0x40:
            (*range).range = 4;
            break;
        case 0x60:
            (*range).range = 2;
            break;

        default:
            GYRO_ERR("accel range 0x%x", val);
            return -1;
    }

    return ret;
}

static int icm42607_get_accel_sensitivity(struct gyro_dev *dev, struct gyro_arg_sensitivity *sensitivity)
{
    int ret       = 0;
    u8  val       = 0;
    u8  accel_reg = GSEN_ICM42607_ACCEL_CONFIG0;

    ret = dev->reg_ops->read_reg(dev, accel_reg, &val);
    if (ret != 0)
    {
        return ret;
    }
    val &= 0xe0;

    switch (val)
    {
        case 0x00:
            (*sensitivity).num = 2048;
            (*sensitivity).den = 1;
            break;
        case 0x20:
            (*sensitivity).num = 4096;
            (*sensitivity).den = 1;
            break;
        case 0x40:
            (*sensitivity).num = 8192;
            (*sensitivity).den = 1;
            break;
        case 0x60:
            (*sensitivity).num = 16384;
            (*sensitivity).den = 1;
            break;

        default:
            GYRO_ERR("accel sensitivity 0x%x", val);
            return -1;
    }

    return ret;
}

static int icm42607_read_fifo_cnt(struct gyro_dev *dev, u16 *cnt)
{
    u8 val[2];

    dev->reg_ops->read_reg(dev, GSEN_ICM42607_FIFO_COUNTH, &val[0]);
    dev->reg_ops->read_reg(dev, GSEN_ICM42607_FIFO_COUNTL, &val[1]);
    *cnt = (val[0] << 8) + val[1];
    pr_info("cnt[%u] l[%u] h[%u]\n", *cnt, val[1], val[0]);
    return 0;
}

static int icm42607_read_fifo_data(struct gyro_dev *dev, u8 *fifo_data, u16 fifo_cnt)
{
    u8 fifo_reg = GSEN_ICM42607_FIFO_DATA;

    return dev->reg_ops->read_regs(dev, fifo_reg, fifo_data, fifo_cnt);
}

static int icm42607_reset_fifo(struct gyro_dev *dev)
{
    u8 rst_reg = GSEN_ICM42607_SIGNAL_PATH_RESET;
    u8 rst_dat = 0x04;

    return dev->reg_ops->write_reg(dev, rst_reg, rst_dat);
}

static void icm42607_deinit(struct gyro_dev *dev)
{
    int ret = GYRO_SUCCESS;

    icm42607_reset_fifo(dev);
    msleep(10);

    // reset the gyro device
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM42607_SIGNAL_PATH_RESET, 0x10);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM42607_CONFIG fail, ret:[%d]", ret);
        return;
    }

    /*!
     * Note: Need waiting for the reset operation done
     */
    msleep(10);

    // turn off the gyro/accel/temperature sensor, make the gyro idle
    ret = dev->reg_ops->write_reg(dev, GSEN_ICM42607_PWR_MGMT0, 0x10);
    if (ret < 0)
    {
        GYRO_ERR("GSEN_ICM42607_PWR_MGMT_0 fail, ret:[%d]", ret);
        return;
    }
}

gyro_sensor_context_t icm42607_context = {
    .list_head =
        {
            .next = NULL,
            .prev = NULL,
        },
    .ops =
        {
            .early_init      = NULL,
            .final_deinit    = NULL,
            .init            = icm42607_init,
            .deinit          = icm42607_deinit,
            .enable_fifo     = icm42607_enable_fifo,
            .set_sample_rate = icm42607_set_sample_rate,
            .get_sample_rate = icm42607_get_sample_rate,

            .set_gyro_range  = icm42607_set_gyro_range,
            .set_accel_range = icm42607_set_accel_range,

            .get_gyro_range       = icm42607_get_gyro_range,
            .get_gyro_sensitivity = icm42607_get_gyro_sensitivity,

            .get_accel_range       = icm42607_get_accel_range,
            .get_accel_sensitivity = icm42607_get_accel_sensitivity,

            .read_fifo_data      = icm42607_read_fifo_data,
            .read_fifo_cnt       = icm42607_read_fifo_cnt,
            .reset_fifo          = icm42607_reset_fifo,
            .whoami_verify       = NULL,
            .get_group_delay     = icm42607_get_group_delay,
            .get_noise_bandwidth = NULL,
        },
};

ADD_SENSOR_CONTEXT(SENSOR_TYPE, icm42607_context);
