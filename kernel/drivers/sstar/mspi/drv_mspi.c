/*
 * drv_mspi.c- Sigmastar
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

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/spi/spi.h>
#include <linux/clk-provider.h>

#include <hal_mspi.h>
#include <hal_mspireg.h>
#include <ms_platform.h>
#include <drv_camclk_Api.h>
#include <drv_gpio.h>
#include <gpio.h>
#include <padmux.h>
#include <drv_padmux.h>
#include <drv_puse.h>

#define MSPI_TIMEOUT_MS     30000
#define MSPI_DMA_ALLOC_SIZE 4096
#define MSPI_DEBUG_MSG      0
#if MSPI_DEBUG_MSG == 1
#define mspi_dbgmsg(args...) printk("[MSPI] : " args)
#else
#define mspi_dbgmsg(args...) \
    do                       \
    {                        \
    } while (0)
#endif
#define mspi_errmsg(fmt, ...)  printk(KERN_ERR "err: " fmt, ##__VA_ARGS__)
#define mspi_warnmsg(fmt, ...) printk(KERN_ERR "warning: " fmt, ##__VA_ARGS__)

#define SSTAR_SPI_MODE_BITS (SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_3WIRE | SPI_LSB_FIRST)

struct mspi_clk
{
    u8  clk_src;
    u8  clk_div;
    u32 clk_rate;
};

struct mspi_clk_tbl
{
    u32 *            clk_src_tbl;
    u32              clk_src_tbl_sz;
    u32 *            clk_div_tbl;
    u32              clk_div_tbl_sz;
    u32              clk_cfg_tbl_sz;
    struct mspi_clk *clk_cfg_tbl;
};

struct mspi_control
{
    int       irq_num;
    char      irq_name[20];
    int       len;
    const u8 *tx_buf;
    u8 *      rx_buf;

    u16 mode;
    s16 bus_num;
    u32 use_dma;
    u32 xfer_dma;
    u32 mosi_pad;
    u32 mosi_mode;
    u32 pin_4to3;
    u32 bits_per_word;
    u32 max_speed_hz;

    u32               word_size;
    struct completion done;

#ifdef CONFIG_CAM_CLK
    void **camclk;
#endif

    struct hal_mspi         hal;
    struct platform_device *pdev;
    struct mspi_clk_tbl     mspi_clk_tbl;
};

static const u32 sstar_mspi_clk_div_tbl[] = HAL_MSPI_CLK_DIV_VAL;

static irqreturn_t sstar_mspi_interrupt(int irq, void *dev_id)
{
    struct spi_master *  spi_mst   = dev_id;
    struct mspi_control *mspi_ctrl = spi_master_get_devdata(spi_mst);
    int                  done_flag = 0;
    done_flag                      = hal_mspi_check_done(&mspi_ctrl->hal);

    if (done_flag == 1)
    {
        hal_mspi_clear_done(&mspi_ctrl->hal);
        complete(&mspi_ctrl->done);
    }
    return IRQ_HANDLED;
}

s32 sstar_mspi_waitdone(struct hal_mspi *mspi)
{
    unsigned int         timeout;
    struct mspi_control *mspi_ctrl = container_of(mspi, struct mspi_control, hal);

    reinit_completion(&mspi_ctrl->done);

    hal_mspi_trigger(&mspi_ctrl->hal);
    timeout = wait_for_completion_timeout(&mspi_ctrl->done, msecs_to_jiffies(MSPI_TIMEOUT_MS));

    if (!timeout)
    {
        mspi_errmsg("wait done timeout\n");
        return -HAL_MSPI_TIMEOUT;
    }
    return HAL_MSPI_OK;
}

static void sstar_mspi_clk_tbl_init(struct mspi_clk_tbl *clk_tbl)
{
    u8              i   = 0;
    u8              j   = 0;
    u32             clk = 0;
    struct mspi_clk mspi_clk_temp;

    memset(&mspi_clk_temp, 0, sizeof(struct mspi_clk));
    memset(clk_tbl->clk_cfg_tbl, 0, sizeof(struct mspi_clk) * clk_tbl->clk_cfg_tbl_sz);

    for (i = 0; i < clk_tbl->clk_src_tbl_sz; i++)
    {
        for (j = 0; j < clk_tbl->clk_div_tbl_sz; j++)
        {
            clk                                                            = clk_tbl->clk_src_tbl[i];
            clk_tbl->clk_cfg_tbl[j + clk_tbl->clk_div_tbl_sz * i].clk_src  = i;
            clk_tbl->clk_cfg_tbl[j + clk_tbl->clk_div_tbl_sz * i].clk_div  = j;
            clk_tbl->clk_cfg_tbl[j + clk_tbl->clk_div_tbl_sz * i].clk_rate = clk / clk_tbl->clk_div_tbl[j];
        }
    }

    for (i = 0; i < clk_tbl->clk_cfg_tbl_sz; i++)
    {
        for (j = i; j < clk_tbl->clk_cfg_tbl_sz; j++)
        {
            if (clk_tbl->clk_cfg_tbl[i].clk_rate > clk_tbl->clk_cfg_tbl[j].clk_rate)
            {
                memcpy(&mspi_clk_temp, &clk_tbl->clk_cfg_tbl[i], sizeof(struct mspi_clk));

                memcpy(&clk_tbl->clk_cfg_tbl[i], &clk_tbl->clk_cfg_tbl[j], sizeof(struct mspi_clk));

                memcpy(&clk_tbl->clk_cfg_tbl[j], &mspi_clk_temp, sizeof(struct mspi_clk));
            }
        }
    }

    for (i = 0; i < clk_tbl->clk_cfg_tbl_sz; i++)
    {
        mspi_dbgmsg("clk_cfg_tbl[%d].clk_cfg  = %d\n", i, clk_tbl->clk_cfg_tbl[i].clk_src);
        mspi_dbgmsg("clk_cfg_tbl[%d].clk_div  = %d\n", i, clk_tbl->clk_cfg_tbl[i].clk_div);
        mspi_dbgmsg("clk_cfg_tbl[%d].clk_rate = %d\n", i, clk_tbl->clk_cfg_tbl[i].clk_rate);
    }
}

/*
 * return: u32
 *         return set speed
 *         if fail,return 0 means set fail.
 */
static u32 sstar_mspi_set_clock(struct mspi_control *mspi, u32 speed)
{
    u8 i = 0;
#ifdef CONFIG_CAM_CLK
    u8                   cam_on;
    CAMCLK_Set_Attribute camclk_attr_set;
#else
    struct clk *clock;
#endif
    struct mspi_clk_tbl *clk_tbl = &mspi->mspi_clk_tbl;

    for (i = 0; i < clk_tbl->clk_cfg_tbl_sz; i++)
    {
        if (speed < clk_tbl->clk_cfg_tbl[i].clk_rate)
        {
            break;
        }
    }

    if (i >= 1)
    {
        i--;
    }
    else
    {
        mspi_errmsg("clk table size 0\n");
        return 0;
    }

#ifdef CONFIG_CAM_CLK
    if (CamClkGetOnOff(mspi->camclk[0], &cam_on))
    {
        mspi_errmsg("cam clk 0 on get fail\n");
        return 0;
    }
    if (!cam_on)
    {
        if (CamClkSetOnOff(mspi->camclk[0], 1))
        {
            mspi_errmsg("cam clk 0 on set fail\n");
            return 0;
        }
    }
    CAMCLK_SETRATE_ROUNDUP(camclk_attr_set, clk_tbl->clk_src_tbl[clk_tbl->clk_cfg_tbl[i].clk_src]);
    if (CamClkAttrSet(mspi->camclk[0], &camclk_attr_set))
    {
        mspi_errmsg("cam clk 0 rate set fail\n");
        return 0;
    }
#else
    clock = of_clk_get(mspi->pdev->dev.of_node, 0);
    if (IS_ERR(clock))
    {
        mspi_errmsg("get clock fail 0\n");
        return 0;
    }
    clk_set_rate(clock, clk_tbl->clk_src_tbl[clk_tbl->clk_cfg_tbl[i].clk_src]);
    clk_put(clock);
#endif

    hal_mspi_set_div_clk(&mspi->hal, clk_tbl->clk_cfg_tbl[i].clk_div);

    mspi_dbgmsg("calc config  : %04d\n", clk_tbl->clk_cfg_tbl[i].clk_src);
    mspi_dbgmsg("calc div     : %04d\n", clk_tbl->clk_cfg_tbl[i].clk_div);
    mspi_dbgmsg("calc rate    : %d\n", clk_tbl->clk_cfg_tbl[i].clk_rate);

    return clk_tbl->clk_cfg_tbl[i].clk_rate;
}
#ifdef CONFIG_MS_MOVE_DMA
static int sstar_mspi_select_dma_clk(u8 u8Channel, struct mspi_control *mspi)
{
#ifdef CONFIG_CAM_CLK
    u8                   cam_on;
    CAMCLK_Get_Attribute camclk_attr_get;
    CAMCLK_Set_Attribute camclk_attr_set;
#else
    u32            num_parents;
    struct clk *   movdma_clock;
    struct clk_hw *movdma_hw;
    struct clk_hw *parent_hw;
#endif

#ifdef CONFIG_CAM_CLK
    if (CamClkGetOnOff(mspi->camclk[1], &cam_on))
    {
        mspi_errmsg("camclk get clk 1 fail\n");
        return -HAL_MSPI_CLKCONFIG_ERROR;
    }
    if (!cam_on)
    {
        if (CamClkSetOnOff(mspi->camclk[1], 1))
        {
            mspi_errmsg("camclk set on clk 1 fail\n");
            return -HAL_MSPI_CLKCONFIG_ERROR;
        }
    }
    if (CamClkAttrGet(mspi->camclk[1], &camclk_attr_get))
    {
        mspi_errmsg("camclk get clk 1 attr fail\n");
        return -HAL_MSPI_CLKCONFIG_ERROR;
    }
    CAMCLK_SETPARENT(camclk_attr_set, camclk_attr_get.u32Parent[u8Channel]);
#else
    num_parents = of_clk_get_parent_count(mspi->pdev->dev.of_node);
    if (num_parents < 2)
    {
        mspi_errmsg("can't find mspi clocks property %d\n", num_parents);
        return -HAL_MSPI_CLKCONFIG_ERROR;
    }
    movdma_clock = of_clk_get(mspi->pdev->dev.of_node, 1);
    if (IS_ERR(movdma_clock))
    {
        mspi_errmsg("get clock fail\n");
        return -HAL_MSPI_CLKCONFIG_ERROR;
    }
    movdma_hw = __clk_get_hw(movdma_clock);
    parent_hw = clk_hw_get_parent_by_index(movdma_hw, u8Channel);
    clk_set_parent(movdma_clock, parent_hw->clk);
    clk_prepare_enable(movdma_clock);
    clk_put(movdma_clock);
#endif

    return HAL_MSPI_OK;
}
#else
static int sstar_mspi_select_dma_clk(u8 u8Channel, struct mspi_control *mspi)
{
    return 0;
}
#endif

static int sstar_mspi_setup(struct spi_device *spi)
{
    int                  err       = 0;
    u32                  new_clock = 0;
    struct mspi_control *mspi_ctrl = spi_master_get_devdata(spi->master);

    if (mspi_ctrl->mode != spi->mode)
    {
        mspi_ctrl->mode = spi->mode;

        err = hal_mspi_set_mode(&mspi_ctrl->hal, mspi_ctrl->mode & (SPI_CPHA | SPI_CPOL));
        if (err)
        {
            return -EIO;
        }

        err = hal_mspi_set_lsb(&mspi_ctrl->hal, (mspi_ctrl->mode & SPI_LSB_FIRST) >> 3);
        if (err)
        {
            return -EIO;
        }

        hal_mspi_chip_select(&mspi_ctrl->hal, ((spi->mode & SPI_CS_HIGH) == SPI_CS_HIGH), spi->chip_select);

        err = hal_mspi_set_3wire_mode(&mspi_ctrl->hal, (mspi_ctrl->mode & SPI_3WIRE));
        if (err)
        {
            return -EIO;
        }
        mspi_dbgmsg("mspi-%d setup mode:%d\n", mspi_ctrl->bus_num, mspi_ctrl->mode);
    }

    if (mspi_ctrl->max_speed_hz != spi->max_speed_hz)
    {
        new_clock = sstar_mspi_set_clock(mspi_ctrl, spi->max_speed_hz);
        if (new_clock > 0)
        {
            spi->max_speed_hz       = new_clock;
            mspi_ctrl->max_speed_hz = spi->max_speed_hz;
        }
        else
        {
            return -EIO;
        }

        mspi_dbgmsg("mspi-%d setup speed: %d\n", mspi_ctrl->bus_num, mspi_ctrl->max_speed_hz);
    }

    if (mspi_ctrl->bits_per_word != spi->bits_per_word)
    {
        mspi_ctrl->xfer_dma          = (spi->bits_per_word % 8 == 0) ? mspi_ctrl->use_dma : false;
        mspi_ctrl->bits_per_word     = spi->bits_per_word;
        mspi_ctrl->hal.bits_per_word = spi->bits_per_word;
        if (spi->bits_per_word > HAL_MSPI_MAX_SUPPORT_BITS)
        {
            return -EINVAL;
        }
        else if (spi->bits_per_word > 8)
        {
            mspi_ctrl->word_size = 2;
        }
        else
        {
            mspi_ctrl->word_size = 1;
        }
        hal_mspi_set_frame_cfg(&mspi_ctrl->hal, spi->bits_per_word);

        mspi_dbgmsg("mspi-%d setup bits: %d\n", mspi_ctrl->bus_num, mspi_ctrl->bits_per_word);
    }
    return 0;
}

static int sstar_mspi_start_transfer(struct spi_device *spi, struct spi_transfer *tfr)
{
    int                  err       = 0;
    struct mspi_control *mspi_ctrl = spi_master_get_devdata(spi->master);

    mspi_ctrl->tx_buf = tfr->tx_buf;
    mspi_ctrl->rx_buf = tfr->rx_buf;
    mspi_ctrl->len    = tfr->len;

    hal_mspi_chip_select(&mspi_ctrl->hal, !((spi->mode & SPI_CS_HIGH) == SPI_CS_HIGH), spi->chip_select);

    if (mspi_ctrl->use_dma)
    {
        err = sstar_mspi_select_dma_clk(spi->master->bus_num, mspi_ctrl);
        if (err)
        {
            return err;
        }
    }

    /*
     * Document\spi\spi-summary:
     * which I/O buffers are used ... each spi_transfer wraps a
     * buffer for each transfer direction, supporting full duplex
     * (two pointers, maybe the same one in both cases) and half
     * duplex (one pointer is NULL) transfers;
     */

    if (mspi_ctrl->tx_buf != NULL && mspi_ctrl->rx_buf != NULL)
    {
        hal_mspi_full_duplex(spi->master->bus_num, &mspi_ctrl->hal, (u8 *)mspi_ctrl->rx_buf, (u8 *)mspi_ctrl->tx_buf,
                             (u16)mspi_ctrl->len);
    }
    else if (mspi_ctrl->tx_buf != NULL)
    {
        if (mspi_ctrl->xfer_dma)
        {
            err =
                hal_mspi_dma_write(spi->master->bus_num, &mspi_ctrl->hal, (u8 *)mspi_ctrl->tx_buf, (u32)mspi_ctrl->len);
            if (err)
            {
                goto out;
            }
        }
        else
        {
            err = hal_mspi_write(spi->master->bus_num, &mspi_ctrl->hal, (u8 *)mspi_ctrl->tx_buf, (u32)mspi_ctrl->len);
            if (err)
            {
                goto out;
            }
        }
    }
    else if (mspi_ctrl->rx_buf != NULL)
    {
        if (mspi_ctrl->pin_4to3)
        {
#if defined(CONFIG_SSTAR_GPIO)
            sstar_gpio_pad_val_set(mspi_ctrl->mosi_pad, PINMUX_FOR_GPIO_MODE);
            sstar_gpio_pad_odn(mspi_ctrl->mosi_pad);
#else
            mspi_errmsg("4to3 mode not support.\n");
#endif
        }
        if (mspi_ctrl->xfer_dma)
        {
            err =
                hal_mspi_dma_read(spi->master->bus_num, &mspi_ctrl->hal, (u8 *)mspi_ctrl->rx_buf, (u32)mspi_ctrl->len);
            if (err)
            {
                goto out;
            }
        }
        else
        {
            err = hal_mspi_read(spi->master->bus_num, &mspi_ctrl->hal, (u8 *)mspi_ctrl->rx_buf, (u32)mspi_ctrl->len);
            if (err)
            {
                goto out;
            }
        }
        if (mspi_ctrl->pin_4to3)
        {
#if defined(CONFIG_SSTAR_GPIO)
            sstar_gpio_pad_val_set(mspi_ctrl->mosi_pad, mspi_ctrl->mosi_mode);
#else
            mspi_errmsg("4to3 mode not support.\n");
#endif
        }
    }

out:
    if (err)
    {
        mspi_errmsg("mspi transfer fail, err %d\n", err);
    }
    return err;
}

static int sstar_mspi_finish_transfer(struct spi_device *spi, struct spi_transfer *tfr, bool cs_change)
{
    struct mspi_control *mspi_ctrl = spi_master_get_devdata(spi->master);

    if (tfr->delay_usecs)
        udelay(tfr->delay_usecs);

    if (cs_change)
    {
        /* Clear TA flag */
        hal_mspi_chip_select(&mspi_ctrl->hal, ((spi->mode & SPI_CS_HIGH) == SPI_CS_HIGH), spi->chip_select);
    }
    return 0;
}

static int sstar_mspi_transfer_one(struct spi_master *master, struct spi_message *mesg)
{
    int                  err = 0;
    bool                 cs_change;
    struct spi_transfer *tfr;
    struct spi_device *  spi       = mesg->spi;
    struct mspi_control *mspi_ctrl = spi_master_get_devdata(master);

    // mspi_dbgmsg("[sstar_mspi_transfer_one]\n");
    list_for_each_entry(tfr, &mesg->transfers, transfer_list)
    {
        if (mspi_ctrl->bits_per_word != tfr->bits_per_word)
        {
            mspi_ctrl->xfer_dma          = (tfr->bits_per_word % 8 == 0) ? mspi_ctrl->use_dma : false;
            mspi_ctrl->bits_per_word     = tfr->bits_per_word;
            mspi_ctrl->hal.bits_per_word = tfr->bits_per_word;
            if (tfr->bits_per_word > HAL_MSPI_MAX_SUPPORT_BITS)
            {
                goto out;
            }
            else if (tfr->bits_per_word > 8)
            {
                mspi_ctrl->word_size = 2;
            }
            else
            {
                mspi_ctrl->word_size = 1;
            }
            hal_mspi_set_frame_cfg(&mspi_ctrl->hal, tfr->bits_per_word);

            mspi_dbgmsg("setup bits : %d\n", mspi_ctrl->bits_per_word);
        }

        if (tfr->len % mspi_ctrl->word_size != 0)
        {
            mspi_dbgmsg("invalid transfer len\n");
            goto out;
        }

        err = sstar_mspi_start_transfer(spi, tfr);
        if (err)
        {
            goto out;
        }

        cs_change = tfr->cs_change || list_is_last(&tfr->transfer_list, &mesg->transfers);

        err = sstar_mspi_finish_transfer(spi, tfr, cs_change);
        if (err)
        {
            goto out;
        }
        mesg->actual_length += mspi_ctrl->len;
        mspi_dbgmsg("transfered:%d\n", mesg->actual_length);
    }

out:
    /* Clear FIFOs, and disable the HW block */
    mesg->status = err;
    spi_finalize_current_message(master);

    return HAL_MSPI_OK;
}

static int sstar_mspi_probe(struct platform_device *pdev)
{
    int i   = 0;
    int err = 0;
#ifdef CONFIG_SSTAR_PADMUX
    int puse = 0;
#endif
    u32  use_dma       = 0;
    u32  pad_ctrl      = 0;
    u32  cs_num        = 0;
    u32  irq_num       = 0;
    u32  mosi_pad      = 0;
    u32  mosi_mode     = 0;
    bool _4to3mode     = 0;
    u32  mspi_group    = 0;
    u8   num_parents   = 0;
    u32  clk_out_mode  = 0;
    u32  poll_waitdone = 0;
#ifdef CONFIG_CAM_CLK
    char                 cam_name[20];
    u32                  mspi_clkid;
    u32                  movedma_clkid;
    CAMCLK_Get_Attribute stGetCfg;
#else
    struct clk *mspi_clock;
    struct clk_hw *parent_hw;
#endif
    u64                  io_addr;
    struct mspi_control *mspi_ctrl;
    struct spi_master *  spi_mst;
    struct resource *    resource_msg;

    mspi_dbgmsg("MSPI Probe Start\n");

    err = of_property_read_u32(pdev->dev.of_node, "mspi-group", &mspi_group);
    if (err)
    {
        mspi_errmsg("read mspi-group property fail\n");
        return err;
    }
    mspi_dbgmsg("mspi-grounp = %d\n", mspi_group);

    irq_num = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (irq_num == 0)
    {
        mspi_errmsg("mspi-%d parse irq number fail\n", mspi_group);
        return -ENOENT;
    }
    mspi_dbgmsg("irq_num = %d\n", irq_num);

    use_dma = of_property_read_bool(pdev->dev.of_node, "dma-enable");
    if (!use_dma)
    {
        mspi_dbgmsg("use fifo mode config, group:%d!\n", mspi_group);
        use_dma = 0;
    }

    if (use_dma && hal_mspi_check_dma_mode(mspi_group))
    {
        mspi_warnmsg("mspi-%d no support dma mode, change to normal mode default\n", mspi_group);
        use_dma = 0;
    }
    else
    {
        printk(KERN_DEBUG "mspi-%d use dma mode\n", mspi_group);
    }

    err = of_property_read_u32(pdev->dev.of_node, "pad-ctrl", &pad_ctrl);
    if (err)
    {
        mspi_dbgmsg("mspi-%d read pad-ctrl fail \n", mspi_group);
    }

    err = of_property_read_u32(pdev->dev.of_node, "clk-out-mode", &clk_out_mode);
    if (err)
    {
        clk_out_mode = 0;
        mspi_dbgmsg("mspi-%d read clk-out-mode fail \n", mspi_group);
    }
    else
    {
        mspi_dbgmsg("mspi-%d clk_out_mode : %d\n", mspi_group, clk_out_mode);
    }

    err = of_property_read_u32(pdev->dev.of_node, "cs-num", &cs_num);
    if (err)
    {
        cs_num = 1;
        mspi_dbgmsg("mspi-%d read cs_num fail \n", mspi_group);
    }

    _4to3mode = of_property_read_bool(pdev->dev.of_node, "4to3-mode");
    if (_4to3mode)
    {
#ifdef CONFIG_SSTAR_PADMUX
        puse     = drv_padmux_getpuse(PUSE_SPI, mspi_group, SPI_DI);
        mosi_pad = drv_padmux_getpad(puse);
        if (mosi_pad == PAD_UNKNOWN)
        {
            _4to3mode = 0;
        }
        else
        {
            mosi_mode = drv_padmux_getmode(puse);
            if (mosi_mode == PINMUX_FOR_UNKNOWN_MODE)
            {
                _4to3mode = 0;
            }
        }
#else
        mspi_errmsg("4to3 mode not support.\n");
#endif
    }
    resource_msg = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!resource_msg)
    {
        return -ENOENT;
    }
    io_addr = (IO_ADDRESS(resource_msg->start));
    mspi_dbgmsg("mspi-%d io_addr = %#llx\n", mspi_group, io_addr);

    spi_mst = spi_alloc_master(&pdev->dev, sizeof(struct mspi_control));
    if (!spi_mst)
    {
        mspi_errmsg("alloc spi master fail\n");
        dev_err(&pdev->dev, "alloc spi master fail\n");
        return -ENOMEM;
    }

    spi_mst->mode_bits            = SSTAR_SPI_MODE_BITS;
    spi_mst->bits_per_word_mask   = SPI_BPW_RANGE_MASK(1, HAL_MSPI_MAX_SUPPORT_BITS);
    spi_mst->num_chipselect       = cs_num;
    spi_mst->transfer_one_message = sstar_mspi_transfer_one;
    spi_mst->dev.of_node          = pdev->dev.of_node;
    spi_mst->setup                = sstar_mspi_setup;
    // spi_mst->max_speed_hz = 72000000;
    // spi_mst->min_speed_hz = 46875;
    spi_mst->bus_num = mspi_group;
    platform_set_drvdata(pdev, spi_mst);

    mspi_ctrl                   = spi_master_get_devdata(spi_mst);
    mspi_ctrl->hal.base         = (u64)io_addr;
    mspi_ctrl->use_dma          = use_dma;
    mspi_ctrl->bus_num          = mspi_group;
    mspi_ctrl->mosi_pad         = mosi_pad;
    mspi_ctrl->mosi_mode        = mosi_mode;
    mspi_ctrl->pin_4to3         = _4to3mode;
    mspi_ctrl->pdev             = pdev;
    mspi_ctrl->hal.pad_ctrl     = pad_ctrl;
    mspi_ctrl->hal.clk_out_mode = clk_out_mode;
    mspi_ctrl->hal.cs_num       = cs_num;
    mspi_ctrl->hal.wait_done    = sstar_mspi_waitdone;
    poll_waitdone               = of_property_read_bool(pdev->dev.of_node, "use-polling");
    if (poll_waitdone)
    {
        mspi_ctrl->hal.wait_done = NULL;
    }

    init_completion(&mspi_ctrl->done);
    if (!snprintf(mspi_ctrl->irq_name, sizeof(mspi_ctrl->irq_name), "mspi%d_Isr", mspi_group))
    {
        mspi_errmsg("set irq name fail\n");
        return -ENOENT;
    }
    mspi_dbgmsg("irq name : %s\n", mspi_ctrl->irq_name);
    err = of_property_read_variable_u32_array(pdev->dev.of_node, "cs-ext", mspi_ctrl->hal.cs_ext, 1, 16);
    if (err > 0)
    {
        spi_mst->num_chipselect += err;
    }

    err = request_irq(irq_num, sstar_mspi_interrupt, 0, mspi_ctrl->irq_name, (void *)spi_mst);
    if (err == 0)
    {
        mspi_dbgmsg("%s registered\n", mspi_ctrl->irq_name);
    }
    else
    {
        mspi_errmsg("%s register fail", mspi_ctrl->irq_name);
        goto err_irq;
    }
    mspi_ctrl->irq_num = irq_num;
    if (use_dma)
    {
        mspi_ctrl->hal.virt_addr = kmalloc(MSPI_DMA_ALLOC_SIZE, GFP_DMA);
        if (!mspi_ctrl->hal.virt_addr)
        {
            err = -ENOMEM;
            mspi_errmsg("mspi-%d malloc dma virt_addr fail\n", mspi_group);
            goto err_dma;
        }
    }
#ifdef CONFIG_CAM_CLK
    mspi_ctrl->camclk = kmalloc((sizeof(void *) * 2), GFP_KERNEL);
    if (!mspi_ctrl->camclk)
    {
        err = -ENOMEM;
        mspi_errmsg("mspi-%d malloc camclk fail\n", mspi_group);
        goto err_cam;
    }
    err = of_property_read_u32_index(pdev->dev.of_node, "camclk", 0, &mspi_clkid);
    if (err)
    {
        err = -EINVAL;
        mspi_errmsg("mspi-%d read camclk 0 property fail\n", mspi_group);
        goto err_cam;
    }
    mspi_dbgmsg("mspi %d camclk property : %d\n", mspi_group, mspi_clkid);

    err = of_property_read_u32_index(pdev->dev.of_node, "camclk", 1, &movedma_clkid);
    if (err)
    {
        err = -EINVAL;
        mspi_errmsg("mspi-%d read camclk 1 property fail\n", mspi_group);
        goto err_cam;
    }
    mspi_dbgmsg("movedma camclk property : %d\n", movedma_clkid);

    if (!snprintf(cam_name, sizeof(cam_name), "mspi%d camclk", mspi_group))
    {
        mspi_errmsg("mspi-%d camclk name reformat fail\n", mspi_group);
        err = -ENOENT;
        goto err_cam;
    }
    mspi_dbgmsg("camlck name : %s\n", cam_name);

    if (CamClkRegister(cam_name, mspi_clkid, &mspi_ctrl->camclk[0]))
    {
        err = -ENOENT;
        mspi_errmsg("register mspi-%d camclk fail\n", mspi_group);
        goto err_cam;
    }

    if (CamClkRegister("movedma camclk", movedma_clkid, &mspi_ctrl->camclk[1]))
    {
        err = -ENOENT;
        mspi_errmsg("mspi-%d register movedma camclk fail\n", mspi_group);
        goto err_cam;
    }

    if (CamClkAttrGet(mspi_ctrl->camclk[0], &stGetCfg))
    {
        err = -ENOENT;
        mspi_errmsg("mspi-%d get camclk att fail\n", mspi_group);
        goto err_cam;
    }
    num_parents = stGetCfg.u32NodeCount;
    mspi_dbgmsg("parents n = %d\n", num_parents);
    mspi_ctrl->mspi_clk_tbl.clk_src_tbl_sz = num_parents;
    mspi_ctrl->mspi_clk_tbl.clk_src_tbl    = kmalloc(sizeof(u32) * mspi_ctrl->mspi_clk_tbl.clk_src_tbl_sz, GFP_KERNEL);
    if (!mspi_ctrl->mspi_clk_tbl.clk_src_tbl)
    {
        err = -ENOMEM;
        mspi_errmsg("malloc clk_src_tbl fail\n");
        goto err_src;
    }
    for (i = 0; i < num_parents; i++)
    {
        mspi_ctrl->mspi_clk_tbl.clk_src_tbl[i] = CamClkRateGet(stGetCfg.u32Parent[i]);
        mspi_dbgmsg("clk src %d = %d\n", i, mspi_ctrl->mspi_clk_tbl.clk_src_tbl[i]);
    }
#else
    // clock table source table cfg
    mspi_clock = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR(mspi_clock))
    {
        mspi_errmsg("get mspi-%d clock fail\n", mspi_group);
        err = -ENXIO;
        goto err_irq;
    }
    num_parents = clk_hw_get_num_parents(__clk_get_hw(mspi_clock));
    mspi_dbgmsg("parents n = %d\n", num_parents);
    mspi_ctrl->mspi_clk_tbl.clk_src_tbl_sz = num_parents;
    mspi_ctrl->mspi_clk_tbl.clk_src_tbl = kmalloc(sizeof(u32) * mspi_ctrl->mspi_clk_tbl.clk_src_tbl_sz, GFP_KERNEL);
    if (!mspi_ctrl->mspi_clk_tbl.clk_src_tbl)
    {
        err = -ENOMEM;
        mspi_errmsg("mspi-%d malloc clk_src_tbl fail\n", mspi_group);
        clk_put(mspi_clock);
        goto err_src;
    }
    for (i = 0; i < num_parents; i++)
    {
        parent_hw = clk_hw_get_parent_by_index(__clk_get_hw(mspi_clock), i);
        mspi_ctrl->mspi_clk_tbl.clk_src_tbl[i] = clk_get_rate(parent_hw->clk);
        mspi_dbgmsg("mspi-%d clk src %d = %d\n", mspi_group, i, mspi_ctrl->mspi_clk_tbl.clk_src_tbl[i]);
    }
    err = clk_prepare_enable(mspi_clock);
    if (err)
    {
        mspi_errmsg("failed to enable mspi clock\n");
        clk_put(mspi_clock);
        goto err_src;
    }
    clk_put(mspi_clock);
#endif

    // clock table divide table cfg
    mspi_ctrl->mspi_clk_tbl.clk_div_tbl_sz = ARRAY_SIZE(sstar_mspi_clk_div_tbl);
    mspi_ctrl->mspi_clk_tbl.clk_div_tbl    = kmalloc(sizeof(u32) * mspi_ctrl->mspi_clk_tbl.clk_div_tbl_sz, GFP_KERNEL);
    if (!mspi_ctrl->mspi_clk_tbl.clk_div_tbl)
    {
        err = -ENOMEM;
        mspi_errmsg("malloc clk_div_tbl fail\n");
        goto err_div;
    }
    memcpy(mspi_ctrl->mspi_clk_tbl.clk_div_tbl, sstar_mspi_clk_div_tbl, sizeof(sstar_mspi_clk_div_tbl));
    for (i = 0; i < ARRAY_SIZE(sstar_mspi_clk_div_tbl); i++)
    {
        mspi_dbgmsg("clk div %d = %d\n", i, mspi_ctrl->mspi_clk_tbl.clk_div_tbl[i]);
    }

    // clock table config table malloc
    mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl_sz =
        mspi_ctrl->mspi_clk_tbl.clk_div_tbl_sz * mspi_ctrl->mspi_clk_tbl.clk_src_tbl_sz;
    mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl =
        kmalloc(sizeof(struct mspi_clk) * mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl_sz, GFP_KERNEL);
    if (!mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl)
    {
        err = -ENOMEM;
        mspi_errmsg("mspi-%d malloc clk_cfg_tbl fail\n", mspi_group);
        goto err_cfg;
    }

    /* initialise the clock calc table for calc closest clock */
    sstar_mspi_clk_tbl_init(&mspi_ctrl->mspi_clk_tbl);

    spi_mst->max_speed_hz   = mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl[mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl_sz - 1].clk_rate;
    spi_mst->min_speed_hz   = mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl[0].clk_rate;
    mspi_ctrl->max_speed_hz = spi_mst->max_speed_hz;
    mspi_dbgmsg("max_speed_hz = %d\n", spi_mst->max_speed_hz);
    mspi_dbgmsg("min_speed_hz = %d\n", spi_mst->min_speed_hz);

    if (clk_out_mode)
    {
        if (!sstar_mspi_set_clock(mspi_ctrl, clk_out_mode))
        {
            goto err_cfg;
        }
    }

    /* initialise the hardware */
    err = hal_mspi_config(&mspi_ctrl->hal);
    if (err)
    {
        err = -EIO;
        mspi_errmsg("config mspi-%d master: %d\n", mspi_group, err);
        dev_err(&pdev->dev, "config mspi-%d master: %d\n", mspi_group, err);
        goto err_cfg;
    }

    // err = devm_spi_register_master(spi_mst);
    err = spi_register_master(spi_mst);
    if (err)
    {
        mspi_errmsg("could not register mspi-%d master: %d\n", mspi_group, err);
        dev_err(&pdev->dev, "could not register mspi-%d master: %d\n", mspi_group, err);
        goto err_out;
    }

    mspi_dbgmsg("MSPI Probe End\n");

    return 0;

err_out:
    spi_unregister_master(spi_mst);
err_cfg:
    kfree(mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl);
err_div:
    kfree(mspi_ctrl->mspi_clk_tbl.clk_div_tbl);
err_src:
    kfree(mspi_ctrl->mspi_clk_tbl.clk_src_tbl);
#ifdef CONFIG_CAM_CLK
err_cam:
    kfree(mspi_ctrl->camclk);
#endif
err_dma:
    kfree(mspi_ctrl->hal.virt_addr);
err_irq:
    free_irq(irq_num, (void *)spi_mst);
    return err;
}

static int sstar_mspi_remove(struct platform_device *pdev)
{
#ifdef CONFIG_CAM_CLK
    u8 cam_on;
#else
    struct clk *mspi_clock;
#ifdef CONFIG_MS_MOVE_DMA
    struct clk *movdma_clock;
    struct clk_hw *movdma_hw;
#endif
    struct clk_hw *mspi_hw;
    struct clk_hw *parent_hw;
#endif

    struct spi_master *  spi_mst   = platform_get_drvdata(pdev);
    struct mspi_control *mspi_ctrl = spi_master_get_devdata(spi_mst);

    if (spi_mst)
    {
#ifdef CONFIG_CAM_CLK
        if (CamClkGetOnOff(mspi_ctrl->camclk[0], &cam_on))
        {
            mspi_errmsg("mspi-%d cam clk 0 get on fail\n", mspi_ctrl->bus_num);
            return -ENOENT;
        }
        if (cam_on)
        {
            if (CamClkSetOnOff(mspi_ctrl->camclk[0], 0))
            {
                mspi_errmsg("mspi-%d cam clk 0 set on fail\n", mspi_ctrl->bus_num);
                return -ENOENT;
            }
        }
        if (CamClkUnregister(mspi_ctrl->camclk[0]))
        {
            mspi_errmsg("mspi-%d cam clk 0 unregister fail\n", mspi_ctrl->bus_num);
            return -ENOENT;
        }

        if (CamClkGetOnOff(mspi_ctrl->camclk[1], &cam_on))
        {
            mspi_errmsg("mspi-%d cam clk 1 get on fail\n", mspi_ctrl->bus_num);
            return -ENOENT;
        }
        if (cam_on)
        {
            if (CamClkSetOnOff(mspi_ctrl->camclk[1], 0))
            {
                mspi_errmsg("mspi-%d cam clk 1 set on fail\n", mspi_ctrl->bus_num);
                return -ENOENT;
            }
        }
        if (CamClkUnregister(mspi_ctrl->camclk[1]))
        {
            mspi_errmsg("mspi-%d cam clk 1 unregister fail\n", mspi_ctrl->bus_num);
            return -ENOENT;
        }
        kfree(mspi_ctrl->camclk);
        mspi_ctrl->camclk = NULL;
#else
        mspi_clock = of_clk_get(mspi_ctrl->pdev->dev.of_node, 0);
        if (IS_ERR(mspi_clock))
        {
            mspi_errmsg("mspi-%d get clock fail 0\n", mspi_ctrl->bus_num);
            return -ENOENT;
        }
        mspi_hw = __clk_get_hw(mspi_clock);
        parent_hw = clk_hw_get_parent_by_index(mspi_hw, 0);
        clk_set_parent(mspi_clock, parent_hw->clk);
        clk_disable_unprepare(mspi_clock);
        clk_put(mspi_clock);

#ifdef CONFIG_MS_MOVE_DMA
        movdma_clock = of_clk_get(mspi_ctrl->pdev->dev.of_node, 1);
        if (IS_ERR(movdma_clock))
        {
            mspi_errmsg("mspi-%d get clock fail 1\n", mspi_ctrl->bus_num);
            return -ENOENT;
        }
        movdma_hw = __clk_get_hw(movdma_clock);
        parent_hw = clk_hw_get_parent_by_index(movdma_hw, 0);
        clk_set_parent(movdma_clock, parent_hw->clk);
        clk_disable_unprepare(movdma_clock);
        clk_put(movdma_clock);
#endif
#endif

        if (mspi_ctrl->hal.virt_addr)
        {
            kfree(mspi_ctrl->hal.virt_addr);
        }

        free_irq(mspi_ctrl->irq_num, (void *)spi_mst);

        if (mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl)
        {
            kfree(mspi_ctrl->mspi_clk_tbl.clk_cfg_tbl);
        }
        if (mspi_ctrl->mspi_clk_tbl.clk_div_tbl)
        {
            kfree(mspi_ctrl->mspi_clk_tbl.clk_div_tbl);
        }
        if (mspi_ctrl->mspi_clk_tbl.clk_src_tbl)
        {
            kfree(mspi_ctrl->mspi_clk_tbl.clk_src_tbl);
        }

        spi_unregister_master(spi_mst);
    }
    else
    {
        return -EINVAL;
    }
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_mspi_suspend(struct device *dev)
{
    return 0;
}

static int sstar_mspi_resume(struct device *dev)
{
    int                     err       = 0;
    struct platform_device *plat_dev  = to_platform_device(dev);
    struct spi_master *     spi_mst   = platform_get_drvdata(plat_dev);
    struct mspi_control *   mspi_ctrl = spi_master_get_devdata(spi_mst);

    err = hal_mspi_config(&mspi_ctrl->hal);
    if (err)
    {
        return -HAL_MSPI_INIT_FLOW_ERROR;
    }

    err = hal_mspi_set_mode(&mspi_ctrl->hal, mspi_ctrl->mode & (SPI_CPHA | SPI_CPOL));
    if (err)
    {
        return -HAL_MSPI_INIT_FLOW_ERROR;
    }

    err = hal_mspi_set_lsb(&mspi_ctrl->hal, (mspi_ctrl->mode & SPI_LSB_FIRST) >> 3);
    if (err)
    {
        return -HAL_MSPI_INIT_FLOW_ERROR;
    }

    err = hal_mspi_set_3wire_mode(&mspi_ctrl->hal, (mspi_ctrl->mode & SPI_3WIRE));
    if (err)
    {
        return -HAL_MSPI_INIT_FLOW_ERROR;
    }

    mspi_dbgmsg("resume mode:0x%x speed:%d channel:%d\n", mspi_ctrl->mode, mspi_ctrl->max_speed_hz, spi_mst->bus_num);
    return err;
}

#else
#define sstar_mspi_suspend NULL
#define sstar_mspi_resume  NULL
#endif

static const struct of_device_id mspi_of_device_id[] = {{
                                                            .compatible = "sstar,mspi",
                                                        },
                                                        {}};
MODULE_DEVICE_TABLE(of, mspi_of_device_id);

static const struct dev_pm_ops mspi_dev_pm_ops = {
    .suspend = sstar_mspi_suspend,
    .resume  = sstar_mspi_resume,
};

static struct platform_driver mspi_platform_driver = {
    .driver =
        {
            .name           = "spi",
            .owner          = THIS_MODULE,
            .pm             = &mspi_dev_pm_ops,
            .of_match_table = mspi_of_device_id,
        },
    .probe  = sstar_mspi_probe,
    .remove = sstar_mspi_remove,
};
module_platform_driver(mspi_platform_driver);

MODULE_DESCRIPTION("SPI controller driver for SigmaStar");
MODULE_AUTHOR("Gavin Xu <gavin.xu@sigmastar.com.cn>");
MODULE_LICENSE("GPL v2");
