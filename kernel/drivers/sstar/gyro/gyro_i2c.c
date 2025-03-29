/*
 * gyro_i2c.c - Sigmastar
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
#include "gyro_core.h"
#include "gyro_internal.h"
#include <linux/i2c.h>
#include <linux/module.h>

static int _gyro_i2c_read_regs(struct gyro_dev *dev, u8 reg, void *val, int len)
{
    struct i2c_client *client = container_of(dev->transfer_dev, struct i2c_client, dev);
    struct i2c_msg     msgs[2];
    int                ret;

    msgs[0].addr  = client->addr;
    msgs[0].flags = 0;
    msgs[0].buf   = &reg;
    msgs[0].len   = 1;

    msgs[1].addr  = client->addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].buf   = val;
    msgs[1].len   = len;

    if (len == 0)
    {
        GYRO_ERR("command format ill, len is zero\n");
        return -EINVAL;
    }

    ret = i2c_transfer(client->adapter, msgs, 2);

    return ret < 0 ? ret : (ret != ARRAY_SIZE(msgs) ? -EIO : 0);
}

static int _gyro_i2c_read_reg(struct gyro_dev *dev, u8 reg, u8 *val)
{
    return _gyro_i2c_read_regs(dev, reg, val, 1);
}

static int _gyro_i2c_write_reg(struct gyro_dev *dev, u8 reg, u8 val)
{
    struct i2c_client *client = container_of(dev->transfer_dev, struct i2c_client, dev);
    struct i2c_msg     msg;
    int                ret;

    u8 buf[2] = {0};
    buf[0]    = reg;
    buf[1]    = val;

    msg.addr  = client->addr;
    msg.flags = 0;
    msg.buf   = buf;
    msg.len   = 2;
    ret       = i2c_transfer(client->adapter, &msg, 1);

    return ret < 0 ? ret : (ret != 1 ? -EIO : 0);
}

static gyro_transfer_context_t gyro_i2c_context;
static int                     _gyro_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    init_attach_pre_dev(&client->dev, &gyro_i2c_context);
    GYRO_DBG("gyro i2c probe, slave addr:[%u]\n", client->addr);
    return 0;
}

static int _gyro_i2c_remove(struct i2c_client *client)
{
    detect_deinit_pre_dev_bydev(&client->dev);
    GYRO_DBG("gyro i2c remove");
    return 0;
}

static const struct i2c_device_id gyro_i2c_id[] = {
    {"gyro", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, gyro_i2c_id);

static const struct of_device_id gyro_i2c_of_match[] = {
    {.compatible = "sstar,gyro"},
    {},
};
MODULE_DEVICE_TABLE(of, gyro_i2c_of_match);

static struct i2c_driver gyro_i2c_driver = {
    .probe  = _gyro_i2c_probe,
    .remove = _gyro_i2c_remove,
    .driver =
        {
            .owner          = THIS_MODULE,
            .name           = "gyro",
            .of_match_table = of_match_ptr(gyro_i2c_of_match),
        },
    .id_table = gyro_i2c_id,
};

static int _gyro_transfer_init(void)
{
    int ret = 0;

    ret = i2c_add_driver(&gyro_i2c_driver);
    if (0 != ret)
    {
        GYRO_ERR("Add i2c driver error.");
        goto err_i2c_add_driver;
    }

    GYRO_DBG("Gyro i2c init");
    return 0;

err_i2c_add_driver:
    return ret;
}

static void _gyro_transfer_deinit(void)
{
    i2c_del_driver(&gyro_i2c_driver);
    GYRO_DBG("Gyro i2c deinit");
}

static int _gyro_i2c_attach_dev(struct gyro_dev *dev)
{
    GYRO_INFO("gyro dev and i2c transfer attach success");
    return 0;
}

static void _gyro_i2c_detach_dev(struct gyro_dev *dev)
{
    GYRO_INFO("gyro dev and i2c transfer detach success");
}

static gyro_transfer_context_t gyro_i2c_context = {
    .u8Type = I2C_TRANSFER,
    .list_head =
        {
            .prev = NULL,
            .next = NULL,
        },
    .ops =
        {
            .read_regs = _gyro_i2c_read_regs,
            .read_reg  = _gyro_i2c_read_reg,
            .write_reg = _gyro_i2c_write_reg,
        },
    .init       = _gyro_transfer_init,
    .deinit     = _gyro_transfer_deinit,
    .attach_dev = _gyro_i2c_attach_dev,
    .detach_dev = _gyro_i2c_detach_dev,
};

ADD_TRANSFER_CONTEXT(I2C_TRANSFER, gyro_i2c_context);

MODULE_LICENSE("GPL");
