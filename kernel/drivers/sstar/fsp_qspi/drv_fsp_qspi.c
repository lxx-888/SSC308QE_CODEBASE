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

#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/types.h>
#include <linux/crc32.h>
#include <linux/mutex.h>
#include <ms_platform.h>
#include <ms_msys.h>
#include <generated/uapi/linux/version.h>
#include <cam_sysfs.h>
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
    struct clk *            fsp_qspi_clk;
    struct clk *            spi_pll_clk;
    struct clk *            spi_nonpm_clk;
    struct fsp_qspi_hal     hal;
    struct spiflash_control ctrl;
    CamOsMutex_t            lock;
    CamOsTsem_t             fsp_qspi_sem;
    CamOsTsem_t             fsp_qspi_bdma_sem;
};

void sstar_fsp_qspi_interrupt(u32 irq, void *dev_id)
{
    struct fsp_qspi_control *master = dev_id;

    CamOsTsemUp(&master->fsp_qspi_sem);

    return;
}

s32 sstar_fsp_qspi_wait_done(struct fsp_qspi_hal *fsp_qspi)
{
    struct fsp_qspi_control *master = container_of(fsp_qspi, struct fsp_qspi_control, hal);

    if (CAM_OS_TIMEOUT == CamOsTsemTimedDown(&master->fsp_qspi_sem, 5000))
        return -1;

    return 0;
}

static s32 sstar_fsp_qspi_wait_bdma_done(struct fsp_qspi_hal *fsp_qspi)
{
    struct fsp_qspi_control *master = container_of(fsp_qspi, struct fsp_qspi_control, hal);

    if (CAM_OS_TIMEOUT == CamOsTsemTimedDown(&master->fsp_qspi_bdma_sem, 5000))
        return -1;

    return 0;
}

static void sstar_fsp_qspi_bdma_callback(void *data)
{
    struct fsp_qspi_hal *    fsp_qspi = (struct fsp_qspi_hal *)data;
    struct fsp_qspi_control *master   = container_of(fsp_qspi, struct fsp_qspi_control, hal);

    CamOsTsemUp(&master->fsp_qspi_bdma_sem);
}

static void sstar_fsp_qspi_set_rate(struct fsp_qspi_hal *hal, u8 rate, u8 spi_pll_en)
{
    struct fsp_qspi_control *master = container_of(hal, struct fsp_qspi_control, hal);

    if (spi_pll_en)
    {
        if (master->spi_pll_clk)
        {
            clk_set_parent(master->fsp_qspi_clk, master->spi_pll_clk);
            clk_set_rate(master->spi_pll_clk, (rate * 1000000));
        }
        else
        {
            flash_impl_printf("spi pll no exist!!!\n");
        }
    }
    else
    {
        clk_set_parent(master->fsp_qspi_clk, master->spi_nonpm_clk);
        clk_set_rate(master->spi_nonpm_clk, (rate * 1000000));
    }
}

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
    u8                        ret      = 0;
    struct fsp_qspi_control * master   = (struct fsp_qspi_control *)ctrl->priv;
    struct fsp_qspi_ctrl_hal *hal_ctrl = master->hal.ctrl;

    if (!hal_ctrl[msg->cs_select].need_autok)
    {
        CamOsMutexLock(&master->lock);
    }

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

    if (!hal_ctrl[msg->cs_select].need_autok)
    {
        CamOsMutexUnlock(&master->lock);
    }

    return ret;
}

static s8 sstar_fsp_qspi_setup(struct spiflash_control *ctrl, struct spiflash_config *config)
{
    struct fsp_qspi_control *master = (struct fsp_qspi_control *)ctrl->priv;

    CamOsMutexLock(&master->lock);

    hal_fsp_qspi_set_rate(&master->hal, config->cs_select, config->rate, config->cmd);

    if (config->cs_select < (master->hal.cs_num + master->hal.cs_ext_num))
    {
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

    CamOsMutexUnlock(&master->lock);

    return 0;
}

static u8 sstar_fsp_qspi_need_autok(struct spiflash_control *ctrl, u8 cs_select)
{
    u8                       ret    = 0;
    struct fsp_qspi_control *master = (struct fsp_qspi_control *)ctrl->priv;

    CamOsMutexLock(&master->lock);
    ret = master->hal.ctrl[cs_select].need_autok;
    CamOsMutexUnlock(&master->lock);
    return ret;
}

static u8 sstar_fsp_qspi_try_phase(struct spiflash_control *ctrl, u8 (*parrtern_check)(void))
{
    u8                        ret      = 0;
    struct fsp_qspi_control * master   = (struct fsp_qspi_control *)ctrl->priv;
    struct fsp_qspi_ctrl_hal *hal_ctrl = master->hal.ctrl;

    flash_impl_printf("start try phase...\r\n");

    CamOsMutexLock(&master->lock);
    ret                                        = hal_fsp_qspi_try_phase(&master->hal, parrtern_check);
    hal_ctrl[master->hal.cs_select].need_autok = 0;
    CamOsMutexUnlock(&master->lock);

    return ret;
}

static int sstar_fsp_qspi_probe(struct platform_device *pdev)
{
    u8                       i;
    s32                      ret;
    s32                      num_parent;
    u32                      engine;
    u32                      cs_num;
    u32                      cs_mode;
    u32                      dma;
    struct fsp_qspi_control *master = NULL;
    struct resource *        resource_msg[2];
    struct clk_hw *          parent_clk_hw;

    resource_msg[0] = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!resource_msg[0])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    resource_msg[1] = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (!resource_msg[1])
    {
        ret = -ENOMEM;
        goto err_out;
    }

    if (NULL == (master = kmalloc(sizeof(struct fsp_qspi_control), GFP_KERNEL)))
    {
        ret = -ENOMEM;
        goto err_out;
    }

    ret = CamofPropertyReadU32(pdev->dev.of_node, "cs-num", &cs_num);
    if (ret)
    {
        ret = -ENOENT;
        goto err_out;
    }

    ret = CamofPropertyReadU32(pdev->dev.of_node, "cs-mode", &cs_mode);
    if (ret)
    {
        ret = -ENOENT;
        goto err_out;
    }

    ret = of_property_read_variable_u32_array(pdev->dev.of_node, "cs-ext", master->hal.cs_ext, 1, 16);
    if (ret > 0)
    {
        master->hal.cs_ext_num = ret;
        for (i = 0; i < master->hal.cs_ext_num; i++)
        {
#if (defined CONFIG_GPIO_SUPPORT) || (defined CONFIG_SSTAR_GPIO)
            sstar_gpio_pad_set(master->hal.cs_ext[i]);
#endif
        }
    }
    else
    {
        master->hal.cs_ext_num = 0;
    }

    if (NULL
        == (master->hal.ctrl = (struct fsp_qspi_ctrl_hal *)kmalloc(
                (cs_num + master->hal.cs_ext_num) * sizeof(struct fsp_qspi_ctrl_hal), GFP_KERNEL)))
    {
        ret = -ENOMEM;
        goto err_out;
    }

    memset(master->hal.ctrl, 0x0, (cs_num + master->hal.cs_ext_num) * sizeof(struct fsp_qspi_ctrl_hal));

    ret = CamofPropertyReadU32(pdev->dev.of_node, "engine", &engine);
    if (ret)
    {
        ret = -ENOENT;
        goto err_out;
    }

    ret = CamofPropertyReadU32(pdev->dev.of_node, "dma", &dma);
    if (ret)
    {
        dma = 0;
    }

    master->hal.fsp_base  = resource_msg[0]->start;
    master->hal.qspi_base = resource_msg[1]->start;

    master->use_sw_cs          = cs_mode;
    master->hal.cs_num         = cs_num;
    master->bdma_en            = dma;
    master->hal.interrupt_en   = 1;
    master->hal.rate           = 0;
    master->hal.wait_bdma_done = sstar_fsp_qspi_wait_bdma_done;
    master->hal.bdma_callback  = sstar_fsp_qspi_bdma_callback;
    master->hal.set_rate       = sstar_fsp_qspi_set_rate;

    master->ctrl.engine       = engine;
    master->ctrl.priv         = master;
    master->ctrl.boot_storage = hal_fsp_qspi_is_boot_storage(&master->hal);
    master->ctrl.transfer     = sstar_fsp_qspi_transfer;
    master->ctrl.setup        = sstar_fsp_qspi_setup;
    master->ctrl.need_autok   = sstar_fsp_qspi_need_autok;
    master->ctrl.try_phase    = sstar_fsp_qspi_try_phase;

    CamOsMutexInit(&master->lock);
    CamOsTsemInit(&master->fsp_qspi_sem, 0);
    CamOsTsemInit(&master->fsp_qspi_bdma_sem, 0);

    hal_fsp_qspi_init(&master->hal);

    master->fsp_qspi_clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR_OR_NULL(master->fsp_qspi_clk))
    {
        printk("fsp_qspi clk_get err!\n");
        ret = -1;
        goto err_out;
    }

    master->spi_nonpm_clk = of_clk_get(pdev->dev.of_node, 1);
    if (IS_ERR_OR_NULL(master->fsp_qspi_clk))
    {
        printk("fsp_qspi clk_get err!\n");
        ret = -1;
        goto err_out;
    }

    master->spi_pll_clk = of_clk_get(pdev->dev.of_node, 2);
    if (!IS_ERR_OR_NULL(master->spi_pll_clk))
    {
        num_parent = clk_hw_get_num_parents(__clk_get_hw(master->fsp_qspi_clk));
        for (i = 0; i < num_parent; i++)
        {
            parent_clk_hw = clk_hw_get_parent_by_index(__clk_get_hw(master->fsp_qspi_clk), i);
            if (parent_clk_hw == __clk_get_hw(master->spi_pll_clk))
            {
                clk_put(master->spi_pll_clk);
                master->spi_pll_clk = parent_clk_hw->clk;
                break;
            }
        }

        if (i == num_parent)
        {
            clk_put(master->spi_pll_clk);
            clk_put(master->fsp_qspi_clk);
            goto err_out;
        }
    }

    ret = clk_prepare_enable(master->fsp_qspi_clk);
    if (ret)
    {
        printk("failed to enable fsp_qspi clock\n");
        goto err_out;
    }

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

    platform_set_drvdata(pdev, master);

    return 0;

err_out:
    if (master)
    {
        kfree((void *)master);
    }

    return ret;
}

static int sstar_fsp_qspi_remove(struct platform_device *pdev)
{
    struct fsp_qspi_control *master = platform_get_drvdata(pdev);

    spiflash_unregister_master(&master->ctrl);

    if (!IS_ERR(master->fsp_qspi_clk))
    {
        clk_disable_unprepare(master->fsp_qspi_clk);
        clk_put(master->fsp_qspi_clk);
    }

    CamOsMutexDestroy(&master->lock);
    CamOsTsemDeinit(&master->fsp_qspi_sem);
    CamOsTsemDeinit(&master->fsp_qspi_bdma_sem);

    kfree(master);

    return 0;
}

static void sstar_fsp_qspi_shutdown(struct platform_device *pdev) {}

static int sstar_fsp_qspi_suspend(struct device *dev)
{
    struct fsp_qspi_control *master = dev_get_drvdata(dev);

    master->hal.rate = 0;

    return 0;
}

static int sstar_fsp_qspi_resume(struct device *dev)
{
    struct fsp_qspi_control *master = dev_get_drvdata(dev);

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

    return 0;
}

static const struct of_device_id fsp_qspi_of_dt_ids[] = {{.compatible = "sstar,fsp-qspi"}, {/* sentinel */}};

static const struct dev_pm_ops sstar_fsp_qspi_pm_ops = {
    .resume_early = sstar_fsp_qspi_resume,
    .suspend_late = sstar_fsp_qspi_suspend,
};

static struct platform_driver sstar_fsp_qspi_driver = {
    .probe    = sstar_fsp_qspi_probe,
    .remove   = sstar_fsp_qspi_remove,
    .shutdown = sstar_fsp_qspi_shutdown,
    .driver =
        {
            .name           = "sstar-fsp_qspi",
            .owner          = THIS_MODULE,
            .of_match_table = fsp_qspi_of_dt_ids,
            .pm             = &sstar_fsp_qspi_pm_ops,
        },
};

module_platform_driver(sstar_fsp_qspi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sstar FSP QSPI driver");
