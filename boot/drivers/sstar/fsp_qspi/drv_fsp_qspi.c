/*
 * drv_fsp_qspi.c- Sigmastar
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

#include <string.h>
#include <fdtdec.h>
#include <dm/device.h>
#include <dm/read.h>
#include <stdlib.h>
#include <drv_flash_os_impl.h>
#include <spi_flash_controller.h>
#include <hal_fsp_qspi.h>
#if (defined CONFIG_GPIO_SUPPORT) || (defined CONFIG_SSTAR_GPIO)
#include <drv_gpio.h>
#endif

struct fsp_qspi_control
{
    u8                      use_sw_cs;
    u8                      bdma_en;
    struct fsp_qspi_hal     hal;
    struct spiflash_control ctrl;
};

static u32 sstar_fsp_qspi_read_data(struct spiflash_control *ctrl, struct spiflash_msg *msg)
{
    u32                      read_size        = 0;
    u32                      read_left        = msg->size;
    struct fsp_qspi_control *master           = (struct fsp_qspi_control *)ctrl->priv;
    u32                      cs_timeout_value = master->hal.cs_timeout_value;
    u32                      cs_timeout_en    = master->hal.cs_timeout_en;

    if (master->use_sw_cs)
    {
        hal_fsp_qspi_pull_cs(&master->hal, 0);
    }

    if (!master->bdma_en || !msg->bdma_en)
    {
        hal_fsp_qspi_write(&master->hal, (struct fsp_qspi_command *)&msg->cmd, NULL, 0);
    }

    do
    {
        if (master->bdma_en && msg->bdma_en)
        {
            if (cs_timeout_en)
            {
                hal_fsp_qspi_set_timeout(&master->hal, 0, 0);
            }
            if (SPIFLASH_COMMAND_READ == msg->command)
                read_size =
                    hal_fsp_qspi_bdma_read(&master->hal, (struct fsp_qspi_command *)&msg->cmd, msg->buffer, read_left);
#if defined(CONFIG_FLASH_XZDEC)
            else if (SPIFLASH_COMMAND_READ_TO_XZDEC == msg->command)
                read_size = hal_fsp_qspi_bdma_read_to_xzdec(&master->hal, (struct fsp_qspi_command *)&msg->cmd,
                                                            msg->buffer, read_left);
#endif
            if (cs_timeout_en)
            {
                hal_fsp_qspi_set_timeout(&master->hal, 1, cs_timeout_value);
            }
        }
        else
        {
#if defined(CONFIG_FLASH_XZDEC)
            if (SPIFLASH_COMMAND_READ_TO_XZDEC == msg->command)
                break;
#endif

            read_size = hal_fsp_qspi_read(&master->hal, (struct fsp_qspi_command *)&msg->cmd, msg->buffer, read_left);
        }

        if (!read_size)
            break;

        msg->buffer += read_size;
        msg->cmd.address += read_size;
        read_left -= read_size;
    } while (read_left);

    if (master->use_sw_cs)
    {
        hal_fsp_qspi_pull_cs(&master->hal, 1);
    }

    return (msg->size - read_left);
}

static u32 sstar_fsp_qspi_program_data(struct spiflash_control *ctrl, struct spiflash_msg *msg)
{
    u32                      write_size       = 0;
    u32                      write_left       = msg->size;
    struct fsp_qspi_control *master           = (struct fsp_qspi_control *)ctrl->priv;
    u32                      cs_timeout_value = master->hal.cs_timeout_value;
    u32                      cs_timeout_en    = master->hal.cs_timeout_en;

    if (master->use_sw_cs)
    {
        hal_fsp_qspi_pull_cs(&master->hal, 0);
        hal_fsp_qspi_write(&master->hal, (struct fsp_qspi_command *)&msg->cmd, NULL, 0);
    }

    do
    {
        if (master->bdma_en && msg->bdma_en)
        {
            if (cs_timeout_en)
            {
                hal_fsp_qspi_set_timeout(&master->hal, 0, 0);
            }
            write_size =
                hal_fsp_qspi_bdma_write(&master->hal, (struct fsp_qspi_command *)&msg->cmd, msg->buffer, write_left);
            if (cs_timeout_en)
            {
                hal_fsp_qspi_set_timeout(&master->hal, 1, cs_timeout_value);
            }
        }
        else
        {
            write_size =
                hal_fsp_qspi_write(&master->hal, (struct fsp_qspi_command *)&msg->cmd, msg->buffer, write_left);
        }

        if (!write_size)
            break;

        msg->buffer += write_size;
        msg->cmd.address += write_size;
        write_left -= write_size;
    } while (write_left);

    if (master->use_sw_cs)
    {
        hal_fsp_qspi_pull_cs(&master->hal, 1);
    }

    return (msg->size - write_left);
}

static s8 sstar_fsp_qspi_transfer(struct spiflash_control *ctrl, struct spiflash_msg *msg)
{
    u8                       ret    = 0;
    struct fsp_qspi_control *master = (struct fsp_qspi_control *)ctrl->priv;

    hal_fsp_qspi_chip_select(&master->hal, msg->cs_select, msg->cmd.cmd);

    switch (msg->command)
    {
        case SPIFLASH_COMMAND_WRITE_STATUS:
            if (master->use_sw_cs)
            {
                hal_fsp_qspi_pull_cs(&master->hal, 0);
            }

            if (msg->size
                != hal_fsp_qspi_write(&master->hal, (struct fsp_qspi_command *)&msg->cmd, msg->buffer, msg->size))
            {
                ret = -1;
            }

            if (master->use_sw_cs)
            {
                hal_fsp_qspi_pull_cs(&master->hal, 1);
            }
            break;
        case SPIFLASH_COMMAND_READ_STATUS:
            if (master->use_sw_cs)
            {
                hal_fsp_qspi_pull_cs(&master->hal, 0);
            }

            if (msg->size
                != hal_fsp_qspi_read(&master->hal, (struct fsp_qspi_command *)&msg->cmd, msg->buffer, msg->size))
            {
                ret = -1;
            }

            if (master->use_sw_cs)
            {
                hal_fsp_qspi_pull_cs(&master->hal, 1);
            }
            break;
        case SPIFLASH_COMMAND_PROGRAM:
            if (msg->size != sstar_fsp_qspi_program_data(ctrl, msg))
            {
                ret = -1;
            }
            break;
        case SPIFLASH_COMMAND_READ_TO_XZDEC:
        case SPIFLASH_COMMAND_READ:
            if (msg->size != sstar_fsp_qspi_read_data(ctrl, msg))
            {
                ret = -1;
            }
            break;
        default:
            flash_impl_printf("no support spiflash command!!!\n");
            ret = -1;
    }

    return ret;
}

static s8 sstar_fsp_qspi_setup(struct spiflash_control *ctrl, struct spiflash_config *config)
{
    struct fsp_qspi_control *master = (struct fsp_qspi_control *)ctrl->priv;

    if (config->cs_select < (master->hal.cs_num + master->hal.cs_ext_num))
    {
        hal_fsp_qspi_set_rate(&master->hal, config->cs_select, config->rate, config->cmd);

        if (master->hal.ctrl[config->cs_select].need_autok && (config->have_phase == 1))
        {
            hal_fsp_qspi_set_phase(&master->hal, config->phase);
            master->hal.ctrl[config->cs_select].need_autok = 0;
        }
    }
    else
    {
        flash_impl_printf("use wrong cs num!!!\n");
    }

    return 0;
}

static u8 sstar_fsp_qspi_need_autok(struct spiflash_control *ctrl, u8 cs_select)
{
    u8                       ret    = 0;
    struct fsp_qspi_control *master = (struct fsp_qspi_control *)ctrl->priv;

    ret = master->hal.ctrl[cs_select].need_autok;

    return ret;
}

static u8 sstar_fsp_qspi_try_phase(struct spiflash_control *ctrl, u8 (*parrtern_check)(void))
{
    u8                        ret      = 0;
    struct fsp_qspi_control * master   = (struct fsp_qspi_control *)ctrl->priv;
    struct fsp_qspi_ctrl_hal *hal_ctrl = master->hal.ctrl;

    flash_impl_printf("start try phase...\r\n");

    ret                                        = hal_fsp_qspi_try_phase(&master->hal, parrtern_check);
    hal_ctrl[master->hal.cs_select].need_autok = 0;

    return ret;
}

static int sstar_fsp_qspi_probe(struct udevice *dev)
{
    return 0;
}

static int sstar_fsp_qspi_bind(struct udevice *dev)
{
    u8                       i          = 0;
    s32                      ret        = 0;
    s32                      engine     = 0;
    s32                      cs_num     = 0;
    s32                      dma        = 0;
    s32                      cs_mode    = 0;
    s32                      ext_cs_num = 0;
    struct fsp_qspi_control *master     = NULL;
    fdt_addr_t               addr[2]    = {0};

    addr[0] = dev_read_addr_index(dev, 0);
    if (!addr[0])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    addr[1] = dev_read_addr_index(dev, 1);
    if (!addr[1])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    if (NULL == (master = malloc(sizeof(struct fsp_qspi_control))))
    {
        ret = -ENOENT;
        goto err_out;
    }

    cs_num = dev_read_u32_default(dev, "cs-num", 0);
    if (cs_num <= 0)
    {
        cs_num = 1;
    }

    cs_mode = dev_read_u32_default(dev, "cs-mode", 0);
    if (cs_mode < 0)
    {
        cs_mode = 1;
    }

    /* already have 2 cs in one fsp_qspi engine */
    ext_cs_num = dev_read_u32_default(dev, "ext-num", 0);
    if (ext_cs_num < 0)
    {
        ext_cs_num = 0;
    }
    else
    {
        master->hal.cs_ext_num = ext_cs_num;
    }

    if (master->hal.cs_ext_num > 0)
    {
        ret = dev_read_u32_array(dev, "cs-ext", master->hal.cs_ext, master->hal.cs_ext_num);
        if (ret)
        {
            master->hal.cs_ext_num = 0;
        }
        else
        {
            for (i = 0; i < master->hal.cs_ext_num; i++)
            {
#if (defined CONFIG_GPIO_SUPPORT) || (defined CONFIG_SSTAR_GPIO)
                sstar_gpio_pad_set(master->hal.cs_ext[i]);
#endif
            }
        }
    }

    if (NULL
        == (master->hal.ctrl =
                (struct fsp_qspi_ctrl_hal *)malloc((cs_num + ext_cs_num) * sizeof(struct fsp_qspi_ctrl_hal))))
    {
        if (master)
        {
            free(master);
        }
        ret = -ENOMEM;
        goto err_out;
    }

    memset(master->hal.ctrl, 0x0, (cs_num + ext_cs_num) * sizeof(struct fsp_qspi_ctrl_hal));
    engine = dev_read_u32_default(dev, "engine", 0);
    if (engine < 0)
    {
        ret = -ENOENT;
        goto err_out;
    }

    dma = dev_read_u32_default(dev, "dma", 0);
    if (dma < 0)
    {
        dma = 0;
    }

    master->hal.fsp_base  = (unsigned long)addr[0];
    master->hal.qspi_base = (unsigned long)addr[1];

    master->use_sw_cs          = cs_mode;
    master->hal.cs_num         = cs_num;
    master->bdma_en            = !!dma;
    master->hal.interrupt_en   = 0;
    master->hal.rate           = 0;
    master->hal.wait_bdma_done = NULL;
    master->hal.bdma_callback  = NULL;
    master->hal.set_rate       = NULL;

    master->ctrl.engine       = engine;
    master->ctrl.priv         = master;
    master->ctrl.boot_storage = hal_fsp_qspi_is_boot_storage(&master->hal);
    master->ctrl.transfer     = sstar_fsp_qspi_transfer;
    master->ctrl.setup        = sstar_fsp_qspi_setup;
    master->ctrl.need_autok   = sstar_fsp_qspi_need_autok;
    master->ctrl.try_phase    = sstar_fsp_qspi_try_phase;

    hal_fsp_qspi_init(&master->hal);

    if (master->use_sw_cs)
    {
        hal_fsp_qspi_set_timeout(&master->hal, 0, 0);
        hal_fsp_qspi_use_sw_cs(&master->hal, 1);
        hal_fsp_qspi_pull_cs(&master->hal, 1);
    }
    else
    {
        hal_fsp_qspi_use_sw_cs(&master->hal, 0);
        hal_fsp_qspi_set_timeout(&master->hal, 1, 0x0030);
    }

    spiflash_register_master(&master->ctrl);

err_out:

    return ret;
}

UCLASS_DRIVER(fsp_qspi) = {
    .id    = UCLASS_SPI_FLASH,
    .name  = "fsp_qspi",
    .flags = DM_UC_FLAG_SEQ_ALIAS,
};

static const struct udevice_id sstar_fsp_qspi_ids[] = {{.compatible = "sstar,fsp-qspi"}, {}};

U_BOOT_DRIVER(sstar_fsp_qspi) = {.name     = "ssatr_fsp_qspi",
                                 .id       = UCLASS_SPI_FLASH,
                                 .of_match = sstar_fsp_qspi_ids,
                                 .bind     = sstar_fsp_qspi_bind,
                                 .probe    = sstar_fsp_qspi_probe};
