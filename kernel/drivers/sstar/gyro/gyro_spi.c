/*
 * gyro_spi.c - Sigmastar
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/spi/spi.h>

static int _gyro_spi_read_regs(struct gyro_dev *dev, u8 reg, void *val, int len)
{
    struct spi_device * spi = container_of(dev->transfer_dev, struct spi_device, dev);
    struct spi_message  msg;
    u8                  tx_reg = 0x80 | reg;
    struct spi_transfer t[]    = {{.rx_buf = NULL, .tx_buf = &tx_reg, .len = 1},
                               {.rx_buf = val, .tx_buf = NULL, .len = len}};
    int                 ret;

    if (len == 0)
    {
        GYRO_ERR("command format ill, len is zero");
        return -EINVAL;
    }

    spi_message_init(&msg);
    spi_message_add_tail(&t[0], &msg);
    spi_message_add_tail(&t[1], &msg);
    ret = spi_sync(spi, &msg);
    return ret;
}

static int _gyro_spi_read_reg(struct gyro_dev *dev, u8 reg, u8 *val)
{
    return _gyro_spi_read_regs(dev, reg, val, 1);
}

static int _gyro_spi_write_reg(struct gyro_dev *dev, u8 reg, u8 val)
{
    struct spi_device * spi = container_of(dev->transfer_dev, struct spi_device, dev);
    struct spi_message  msg;
    struct spi_transfer t[] = {{.rx_buf = NULL, .tx_buf = &reg, .len = 1}, {.rx_buf = NULL, .tx_buf = &val, .len = 1}};
    int                 ret;
    spi_message_init(&msg);
    spi_message_add_tail(&t[0], &msg);
    spi_message_add_tail(&t[1], &msg);
    ret = spi_sync(spi, &msg);
    return ret;
}

static gyro_transfer_context_t gyro_spi_context;

static int gyro_spi_probe(struct spi_device *spi)
{
    int ret = 0;

    ret = spi_setup(spi);
    if (ret != 0)
    {
        GYRO_ERR("gyro spi_setup failed.");
        return ret;
    }

    init_attach_pre_dev(&spi->dev, &gyro_spi_context);
    GYRO_DBG("gyro spi probe");
    GYRO_INFO("SPI Clk [%u]hz, Mode[%u], cs_select[%u]\n", spi->max_speed_hz, spi->mode, spi->chip_select);
    return 0;
}

static int gyro_spi_remove(struct spi_device *spi)
{
    detect_deinit_pre_dev_bydev(&spi->dev);
    GYRO_DBG("gyro spi remove");
    return 0;
}

static const struct spi_device_id gyro_spi_id[] = {
    {"gyro", 0},
    {},
};
MODULE_DEVICE_TABLE(spi, gyro_spi_id);

static const struct of_device_id gyro_spi_of_match[] = {
    {.compatible = "sstar,gyro_spi"},
    {},
};
MODULE_DEVICE_TABLE(of, gyro_spi_of_match);

static struct spi_driver gyro_spi_driver = {
    .probe  = gyro_spi_probe,
    .remove = gyro_spi_remove,
    .driver =
        {
            .owner          = THIS_MODULE,
            .name           = "gyro_spi",
            .of_match_table = of_match_ptr(gyro_spi_of_match),
        },
    .id_table = gyro_spi_id,
};

static int _gyro_transfer_init(void)
{
    int ret = 0;

    ret = spi_register_driver(&gyro_spi_driver);
    if (0 != ret)
    {
        GYRO_ERR("Spi Register driver error.");
        goto err_spi_register_driver;
    }

    GYRO_DBG("Gyro spi init");
    return 0;

err_spi_register_driver:
    return ret;
}

static void _gyro_transfer_deinit(void)
{
    spi_unregister_driver(&gyro_spi_driver);
    GYRO_DBG("Gyro spi deinit");
}

static int _gyro_spi_attach_dev(struct gyro_dev *dev)
{
    GYRO_INFO("gyro spi dev and spi transfer attach success");
    return 0;
}

static void _gyro_spi_detach_dev(struct gyro_dev *dev)
{
    GYRO_INFO("gyro spi dev and spi transfer detach success");
}

static gyro_transfer_context_t gyro_spi_context = {
    .u8Type = SPI_TRANSFER,
    .list_head =
        {
            .prev = NULL,
            .next = NULL,
        },
    .ops =
        {
            .read_regs = _gyro_spi_read_regs,
            .read_reg  = _gyro_spi_read_reg,
            .write_reg = _gyro_spi_write_reg,
        },

    .init       = _gyro_transfer_init,
    .deinit     = _gyro_transfer_deinit,
    .attach_dev = _gyro_spi_attach_dev,
    .detach_dev = _gyro_spi_detach_dev,
};

ADD_TRANSFER_CONTEXT(SPI_TRANSFER, gyro_spi_context);

MODULE_LICENSE("GPL");
