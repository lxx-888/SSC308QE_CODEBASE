/*
 * drv_pspi.c- Sigmastar
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

#include <hal_pspi.h>
#include <hal_pspireg.h>
#include <pspi_os.h>
#include <ms_platform.h>
#include <drv_camclk_Api.h>
#include <drv_gpio.h>
#include <drv_padmux.h>
#include <drv_puse.h>
#include <gpio.h>
#if (defined CONFIG_BDMA_SUPPORT) || (defined CONFIG_SSTAR_BDMA)
#include <hal_bdma.h>
#endif

#define IMI_ADDR            0x0
#define PSPI_DMA_ALLOC_SIZE 4096
#define PSPI_TIMEOUT_MS     2000
#define PSPI_GROUP(x)       (x - MSPI_NUMS)

#define pspi_cache_flush(_ptr_, _size_)      CamOsMemFlush((void *)_ptr_, (u32)_size_)
#define pspi_cache_invalidate(_ptr_, _size_) CamOsMemInvalidate((void *)_ptr_, (u32)_size_)

#define PSPI_MODE_BITS (SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_3WIRE | SPI_LSB_FIRST)

struct pspi_clk_tbl
{
    u32 *src_tbl;
    u32  src_tbl_sz;
};

struct pspi_ctrl
{
    struct device *      dev;
    struct clk *         clk;
    struct completion    xfer_done;
    struct hal_pspi      hal;
    bool                 slave_aborted;
    u32                  max_speed_hz;
    u32                  mode;
    char                 irq_name[20];
    struct spi_transfer *cur_transfer;
    u32                  group;
    bool                 use_dma;

    u32 len;
    u32 align_size;
    u8 *buffer;
    u32 miu_addr;

    struct pspi_clk_tbl clk_tbl;
};

s32 drv_pspi_waitdone(struct hal_pspi *pspi)
{
    u32               timeout;
    struct pspi_ctrl *pspi_ctrl = container_of(pspi, struct pspi_ctrl, hal);

    if (pspi->slave_mode)
    {
        if (wait_for_completion_interruptible(&pspi_ctrl->xfer_done) || pspi_ctrl->slave_aborted)
        {
            dev_err(pspi_ctrl->dev, "wait interrupted\n");
            return -EINTR;
        }
    }
    else
    {
        timeout = wait_for_completion_timeout(&pspi_ctrl->xfer_done, msecs_to_jiffies(PSPI_TIMEOUT_MS));

        if (!timeout)
        {
            dev_err(pspi_ctrl->dev, "wait interrupted\n");
            return -EINTR;
        }
    }

    return 0;
}

/*  stop pspi slave */
static int drv_pspi_slave_abort(struct spi_controller *ctlr)
{
    struct pspi_ctrl *pspi_ctrl = spi_controller_get_devdata(ctlr);

    pspi_ctrl->slave_aborted = true;
    complete(&pspi_ctrl->xfer_done);

    return 0;
}

static irqreturn_t drv_pspi_interrupt(int irq, void *dev_id)
{
    struct spi_controller *ctlr      = dev_id;
    struct pspi_ctrl *     pspi_ctrl = spi_controller_get_devdata(ctlr);

    u32 irq_type = 0;

    irq_type = hal_pspi_get_irq_type(&pspi_ctrl->hal);

    if (irq_type)
    {
        complete(&pspi_ctrl->xfer_done);
    }

    return IRQ_HANDLED;
}

static int drv_pspi_prepare_message(struct spi_controller *ctlr, struct spi_message *message)
{
    return 0;
}

static int drv_pspi_set_clock(struct pspi_ctrl *pspi_ctrl, u32 speed)
{
    u8  i               = 0;
    u32 pspi_src        = 0;
    u32 difference      = 0xffffffff;
    int difference_temp = 0;
    u32 clk_src         = 0;
    int div_temp        = 0;
    u32 div             = 0;

    struct clk *clock;

    if (pspi_ctrl->hal.slave_mode)
    {
        /* The set clk should be greater than the clk of the spi slave device */
        for (i = 0; i < pspi_ctrl->clk_tbl.src_tbl_sz; i++)
        {
            clk_src = pspi_ctrl->clk_tbl.src_tbl[i];

            if (clk_src >= speed * 4)
            {
                pspi_src = clk_src;
                break;
            }
        }
    }
    else
    {
        /* The set clk should be smaller than the device clk */
        for (i = 0; i < pspi_ctrl->clk_tbl.src_tbl_sz; i++)
        {
            clk_src = pspi_ctrl->clk_tbl.src_tbl[i];

            div_temp = clk_src / (speed * 2) - 1;
            if (div_temp == 0)
            {
                if ((speed * 2) != (clk_src / (div_temp + 1)))
                {
                    if (!pspi_src)
                    {
                        div      = div_temp + 1;
                        pspi_src = clk_src;
                    }
                    continue;
                }
                else
                {
                    div      = div_temp;
                    pspi_src = clk_src;
                    break;
                }
            }

            if (div_temp > 255 || (div_temp == -1))
            {
                continue;
            }

            div_temp += 1;

            difference_temp = (speed * 2) - (clk_src / (div_temp + 1));

            if (!pspi_src)
            {
                div      = div_temp;
                pspi_src = clk_src;
            }
            else if ((difference_temp >= 0) && (difference > difference_temp))
            {
                difference = difference_temp;
                div        = div_temp;
                pspi_src   = clk_src;
            }

            if (difference_temp == 0)
                break;
        }
    }

    if (!pspi_src)
    {
        dev_err(pspi_ctrl->dev, "get src clock fail\n");
        return -EIO;
    }

    clock = of_clk_get(pspi_ctrl->dev->of_node, 0);
    if (IS_ERR(clock))
    {
        dev_err(pspi_ctrl->dev, "get clock fail\n");
        return -EIO;
    }
    clk_set_rate(clock, pspi_src);
    clk_put(clock);

    if (pspi_ctrl->hal.slave_mode)
    {
        pspi_ctrl->max_speed_hz = pspi_src / 4;
    }
    else
    {
        pspi_ctrl->hal.divisor  = div;
        pspi_ctrl->max_speed_hz = pspi_src / (2 * (div + 1));
    }

    return 0;
}

static int drv_pspi_setup(struct spi_device *spi)
{
    int               ret;
    struct pspi_ctrl *pspi_ctrl = spi_controller_get_devdata(spi->master);

    pspi_ctrl->hal.chip_select   = spi->chip_select;
    pspi_ctrl->hal.bits_per_word = spi->bits_per_word;

    pspi_ctrl->hal.cpol = (spi->mode & SPI_CPOL) ? 1 : 0;
    pspi_ctrl->hal.cpha = (spi->mode & SPI_CPHA) ? 1 : 0;

    pspi_ctrl->hal.lsb_first  = (spi->mode & SPI_LSB_FIRST) ? 1 : 0;
    pspi_ctrl->hal.wire_3line = spi->mode & SPI_3WIRE;
    pspi_ctrl->hal.cs_level   = ((spi->mode & SPI_CS_HIGH) == SPI_CS_HIGH);
    pspi_ctrl->hal.mem_sel    = 0; // pm imi

    if (spi->max_speed_hz && (pspi_ctrl->max_speed_hz != spi->max_speed_hz))
    {
        pspi_ctrl->max_speed_hz = spi->max_speed_hz;
        ret                     = drv_pspi_set_clock(pspi_ctrl, pspi_ctrl->max_speed_hz);
        if (ret)
        {
            dev_err(pspi_ctrl->dev, "set clock fail\n");
            return -EIO;
        }
    }

    hal_pspi_config(&pspi_ctrl->hal);

    return 0;
}

#if (defined CONFIG_SSTAR_BDMA)
static int drv_pspi_get_bdma_ch(u8 channel)
{
    switch (channel)
    {
        case 0:
            return PSPI_BDMA_CH0;
        default:
            return -1;
    }
}
#define PSPI_BDMA_SEL_WRITE (0x0)
#define PSPI_BDMA_SEL_READ  (0x1)
static int drv_pspi_get_bdma_path(u8 ch, u8 mode)
{
    // mode 0:write;  1:read
    if (mode)
    {
        switch (ch)
        {
            case 0:
                return PSPI_LNX_BDMA_WRITE0;
            default:
                return -1;
        }
    }
    else
    {
        switch (ch)
        {
            case 0:
                return PSPI_LNX_BDMA_READ0;
            default:
                return -1;
        }
    }

    return -1;
}

static void drv_pspi_bdma_done(void *Parm)
{
    CamOsTsem_t *pstBdmaDoneSem = (CamOsTsem_t *)Parm;
    CamOsTsemUp(pstBdmaDoneSem);
}

int drv_pspi_dma_write(struct spi_master *master, u8 *data, u32 size)
{
    int               ret    = 0;
    s8                dma_ch = 0;
    pspi_bdma_param   param  = {0};
    CamOsTsem_t       bdmadone;
    struct pspi_ctrl *pspi_ctrl = spi_master_get_devdata(master);

    memset(&bdmadone, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&bdmadone, 0);

    dma_ch = drv_pspi_get_bdma_ch(PSPI_GROUP(pspi_ctrl->group));
    if (dma_ch == -1)
    {
        dev_err(pspi_ctrl->dev, "get dma ch fail\n");
        return -EIO;
    }
    memset(&param, 0, sizeof(pspi_bdma_param));

    param.bIntMode     = 1; // 0:use polling mode
    param.ePathSel     = drv_pspi_get_bdma_path(PSPI_GROUP(pspi_ctrl->group), PSPI_BDMA_SEL_WRITE);
    param.eDstAddrMode = HAL_BDMA_ADDR_INC; // address increase
    param.u32TxCount   = size;
    param.u32Pattern   = 0;
    param.pSrcAddr     = (phys_addr_t)pspi_ctrl->miu_addr;

    param.pDstAddr   = pspi_ctrl->hal.mem_txaddr;
    param.pfTxCbFunc = drv_pspi_bdma_done;
    param.pTxCbParm  = (void *)&bdmadone;

    pspi_cache_flush(data, pspi_ctrl->align_size);

    hal_bdma_initialize((u8)dma_ch);
    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer((u8)dma_ch, &param))
    {
        dev_err(pspi_ctrl->dev, "dma transfer fail\n");
        return -EIO;
    }

    if (param.bIntMode)
    {
        CamOsTsemDown(&bdmadone);
    }
    CamOsTsemDeinit(&bdmadone);

    ret = hal_pspi_dma_write(&pspi_ctrl->hal, (u8 *)pspi_ctrl->buffer, (u32)pspi_ctrl->cur_transfer->len);
    if (ret < 0)
    {
        dev_err(pspi_ctrl->dev, "hal pspi dma write fail\n");
        return -EIO;
    }

    return ret;
}

int drv_pspi_dma_read(struct spi_master *master, u8 *data, u32 size)
{
    int               ret    = 0;
    s8                dma_ch = 0;
    pspi_bdma_param   param  = {0};
    CamOsTsem_t       bdmadone;
    struct pspi_ctrl *pspi_ctrl = spi_master_get_devdata(master);

    memset(&bdmadone, 0, sizeof(CamOsTsem_t));
    CamOsTsemInit(&bdmadone, 0);

    dma_ch = drv_pspi_get_bdma_ch(PSPI_GROUP(pspi_ctrl->group));
    if (dma_ch == -1)
    {
        dev_err(pspi_ctrl->dev, "get dma ch fail\n");
        return -EIO;
    }
    memset(&param, 0, sizeof(pspi_bdma_param));

    param.bIntMode     = 1;
    param.ePathSel     = drv_pspi_get_bdma_path(PSPI_GROUP(pspi_ctrl->group), PSPI_BDMA_SEL_READ);
    param.eDstAddrMode = HAL_BDMA_ADDR_INC;
    param.u32TxCount   = size;
    param.u32Pattern   = 0;
    param.pDstAddr     = (phys_addr_t)pspi_ctrl->miu_addr;

    param.pSrcAddr   = pspi_ctrl->hal.mem_rxaddr;
    param.pfTxCbFunc = drv_pspi_bdma_done;
    param.pTxCbParm  = (void *)&bdmadone;

    ret = hal_pspi_dma_read(&pspi_ctrl->hal, (u8 *)pspi_ctrl->buffer, (u32)pspi_ctrl->cur_transfer->len);
    if (ret < 0)
    {
        dev_err(pspi_ctrl->dev, "hal pspi dma read fail\n");
        return -EIO;
    }

    pspi_cache_invalidate(data, pspi_ctrl->align_size);

    hal_bdma_initialize((u8)dma_ch);
    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer((u8)dma_ch, &param))
    {
        dev_err(pspi_ctrl->dev, "dma transfer fail\n");
        return -EIO;
    }

    if (param.bIntMode)
    {
        CamOsTsemDown(&bdmadone);
    }
    CamOsTsemDeinit(&bdmadone);

    pspi_cache_invalidate(data, pspi_ctrl->align_size);

    return ret;
}
#else
int drv_pspi_dma_write(struct spi_master *master, u8 *data, u32 size)
{
    return 0;
}

int drv_pspi_dma_read(struct spi_master *master, u8 *data, u32 size)
{
    return 0;
}
#endif

static int drv_pspi_transfer_one(struct spi_device *spi, struct spi_transfer *xfer, u8 cs_keep)
{
    int ret = 0;

    struct pspi_ctrl *pspi_ctrl = spi_master_get_devdata(spi->master);

    pspi_ctrl->cur_transfer = xfer;

    if (pspi_ctrl->cur_transfer->tx_buf != NULL)
    {
        if (pspi_ctrl->use_dma)
        {
            if (pspi_ctrl->cur_transfer->len > PSPI_DMA_ALLOC_SIZE)
            {
                dev_err(&spi->dev, "sent data is greater than %d\n", PSPI_DMA_ALLOC_SIZE);
                return -EIO;
            }

            memcpy(pspi_ctrl->buffer, pspi_ctrl->cur_transfer->tx_buf, pspi_ctrl->cur_transfer->len);

            ret = hal_pspi_irq_enable(&pspi_ctrl->hal, HAL_PSPI_IRQ_TX_DONE, 1);
            if (ret < 0)
            {
                dev_err(&spi->dev, "irq en err\n");
                return -EIO;
            }

            ret = drv_pspi_dma_write(spi->master, (u8 *)pspi_ctrl->buffer, (u32)pspi_ctrl->cur_transfer->len);
            hal_pspi_irq_enable(&pspi_ctrl->hal, HAL_PSPI_IRQ_TX_DONE, 0);
            if (ret < 0)
            {
                dev_err(&spi->dev, "dma send err\n");
                return -EIO;
            }
        }
        else
        {
            ret = hal_pspi_fifo_write(&pspi_ctrl->hal, (u8 *)pspi_ctrl->cur_transfer->tx_buf,
                                      (u32)pspi_ctrl->cur_transfer->len);
            if (ret < 0)
            {
                dev_err(&spi->dev, "fifo send err\n");
                return -EIO;
            }
        }
    }
    else if (pspi_ctrl->cur_transfer->rx_buf != NULL)
    {
        if (pspi_ctrl->use_dma)
        {
            if (pspi_ctrl->cur_transfer->len > PSPI_DMA_ALLOC_SIZE)
            {
                dev_err(&spi->dev, "recevied data is greater than %d\n", PSPI_DMA_ALLOC_SIZE);
                return -EIO;
            }

            memset(pspi_ctrl->buffer, 0, PSPI_DMA_ALLOC_SIZE);
            ret = hal_pspi_irq_enable(&pspi_ctrl->hal, HAL_PSPI_IRQ_RX_DMA_COMPLETED, 1);
            if (ret < 0)
            {
                dev_err(&spi->dev, "irq en err\n");
                return -EIO;
            }

            ret = drv_pspi_dma_read(spi->master, (u8 *)pspi_ctrl->buffer, (u32)pspi_ctrl->cur_transfer->len);

            hal_pspi_irq_enable(&pspi_ctrl->hal, HAL_PSPI_IRQ_RX_DMA_COMPLETED, 0);

            if (ret < 0)
            {
                dev_err(&spi->dev, "dma recevied err\n");
                return -EIO;
            }
            memcpy(pspi_ctrl->cur_transfer->rx_buf, pspi_ctrl->buffer, pspi_ctrl->cur_transfer->len);
        }
        else
        {
            ret = hal_pspi_fifo_read(&pspi_ctrl->hal, (u8 *)pspi_ctrl->cur_transfer->rx_buf,
                                     (u32)pspi_ctrl->cur_transfer->len);
            if (ret < 0)
            {
                dev_err(&spi->dev, "fifo recevied err\n");
                return -EIO;
            }
        }
    }

    pspi_ctrl->len          = ret;
    pspi_ctrl->cur_transfer = NULL;

    return 0;
}

static int drv_pspi_transfer_one_message(struct spi_master *master, struct spi_message *mesg)
{
    int                  ret = 0;
    bool                 cs_keep;
    struct spi_transfer *tfr;
    struct spi_device *  spi = mesg->spi;

    struct pspi_ctrl *pspi_ctrl = spi_master_get_devdata(master);

    list_for_each_entry(tfr, &mesg->transfers, transfer_list)
    {
        if ((!tfr->tx_buf) && (!tfr->rx_buf))
        {
            dev_err(pspi_ctrl->dev, "buffer is NULL\n");
            return -EIO;
        }
        else if (tfr->tx_buf && tfr->rx_buf)
        {
            dev_err(pspi_ctrl->dev, "Does not support full duplex\n");
            return -EIO;
        }

        if (pspi_ctrl->use_dma)
        {
            pspi_ctrl->align_size = (u32)ALIGN(tfr->len, cache_line_size());
        }

        reinit_completion(&pspi_ctrl->xfer_done);

        ret = hal_pspi_small_reset(&pspi_ctrl->hal);
        if (ret)
        {
            return -EIO;
        }

        hal_pspi_transfer_en(&pspi_ctrl->hal, tfr->tx_buf ? 1 : 0);

        if (pspi_ctrl->hal.slave_mode)
        {
            pspi_ctrl->slave_aborted = false;
        }
        else if (!cs_keep)
        {
            hal_pspi_cs_assert(&pspi_ctrl->hal, spi->chip_select, cs_keep);
        }

        cs_keep = !(tfr->cs_change || list_is_last(&tfr->transfer_list, &mesg->transfers));
        ret     = drv_pspi_transfer_one(spi, tfr, cs_keep);
        if (ret)
        {
            goto out;
        }
        mesg->actual_length += pspi_ctrl->len;

        if (!pspi_ctrl->hal.slave_mode)
        {
            hal_pspi_cs_deassert(&pspi_ctrl->hal, spi->chip_select, cs_keep);
        }
    }

out:
    /* Clear FIFOs, and disable the HW block */
    mesg->status = ret;
    spi_finalize_current_message(master);

    return 0;
}

static int drv_pspi_hw_init(struct pspi_ctrl *pspi_ctrl)
{
    hal_pspi_init(&pspi_ctrl->hal);
    hal_pspi_config(&pspi_ctrl->hal);
    hal_pspi_controller_select(&pspi_ctrl->hal);
    return 0;
}

static int drv_of_clk_init(struct platform_device *pdev)
{
    int i           = 0;
    int ret         = 0;
    u8  num_parents = 0;
    u32 src_speed   = 0;

    struct clk *   clock;
    struct clk_hw *parent_hw;

    struct spi_controller *master  = (struct spi_master *)platform_get_drvdata(pdev);
    struct pspi_ctrl *     control = spi_master_get_devdata(master);

    // clock table source table cfg
    clock = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR(clock))
    {
        dev_err_probe(&pdev->dev, PTR_ERR(clock), "get clock failed\n");
        return PTR_ERR(clock);
    }

    num_parents = clk_hw_get_num_parents(__clk_get_hw(clock));

    control->clk_tbl.src_tbl_sz = num_parents;
    control->clk_tbl.src_tbl    = devm_kmalloc(&pdev->dev, sizeof(u32) * control->clk_tbl.src_tbl_sz, GFP_KERNEL);
    if (!control->clk_tbl.src_tbl)
    {
        dev_err(&pdev->dev, "allocate clock source table failed\n");
        return -ENOMEM;
    }

    for (i = 0; i < num_parents; i++)
    {
        parent_hw                   = clk_hw_get_parent_by_index(__clk_get_hw(clock), i);
        control->clk_tbl.src_tbl[i] = clk_get_rate(parent_hw->clk);
    }

    ret = clk_prepare_enable(clock);
    if (ret)
    {
        dev_err_probe(&pdev->dev, ret, "enable clock failed\n");
        return ret;
    }
    clk_put(clock);

    for (i = 0; i < num_parents; i++)
    {
        if (src_speed < control->clk_tbl.src_tbl[i])
            src_speed = control->clk_tbl.src_tbl[i];
    }
    if (control->hal.slave_mode)
    {
        control->max_speed_hz = src_speed >> 2;
    }
    else
    {
        control->max_speed_hz = src_speed >> 1;
        control->hal.divisor  = 0;
    }

    return 0;
}

static int drv_pspi_probe(struct platform_device *pdev)
{
    int  ret       = 0;
    u32  cs_num    = 0;
    u32  group     = 0;
    int  irq       = 0;
    u64  io_addr   = 0;
    bool slave     = 0;
    bool use_dma   = 0;
    u64  phys_addr = 0;
    int  pad_idx   = 0;
    int  pad_mode  = 0;
    int  puse      = 0;

    struct pspi_ctrl *     pspi_ctrl = NULL;
    struct spi_controller *ctlr      = NULL;
    struct resource *      resource  = NULL;

    slave = of_property_read_bool(pdev->dev.of_node, "pspi-slave");
    if (slave)
    {
        ctlr = devm_spi_alloc_slave(&pdev->dev, sizeof(*pspi_ctrl));
        if (!ctlr)
        {
            dev_err(&pdev->dev, "failed to alloc pspi slave\n");
            return -ENOMEM;
        }
    }
    else
    {
        ctlr = devm_spi_alloc_master(&pdev->dev, sizeof(*pspi_ctrl));
        if (!ctlr)
        {
            dev_err(&pdev->dev, "failed to alloc pspi master\n");
            return -ENOMEM;
        }

        puse     = drv_padmux_getpuse(PUSE_PSPI, 0, PSPI_CLK);
        pad_idx  = drv_padmux_getpad(puse);
        pad_mode = drv_padmux_getmode(puse);
        if (pad_idx == PAD_UNKNOWN)
        {
            dev_info(&pdev->dev, "pspi not support POL high\n");
            pad_mode = 0;
        }
    }
    dev_info(&pdev->dev, "pspi controller is %s\n", slave ? "slave" : "master");

    pspi_ctrl = spi_master_get_devdata(ctlr);

    use_dma = of_property_read_bool(pdev->dev.of_node, "dma-enable");
    if (use_dma)
    {
        pspi_ctrl->buffer = devm_kmalloc(&pdev->dev, PSPI_DMA_ALLOC_SIZE, GFP_KERNEL);
        if (!pspi_ctrl->buffer)
        {
            dev_err(&pdev->dev, "allocate dma buffer failed\n");
            return -ENOMEM;
        }
        else
        {
            phys_addr           = CamOsMemVirtToPhys(pspi_ctrl->buffer);
            pspi_ctrl->miu_addr = CamOsMemPhysToMiu(phys_addr);
        }
    }

    ret = of_property_read_u32(pdev->dev.of_node, "group", &group);
    if (ret)
    {
        group = 0;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "cs-num", &cs_num);
    if (ret || slave)
    {
        cs_num = 1;
    }

    resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!resource)
    {
        dev_err(&pdev->dev, "failed to request resource\n");
        return -ENOENT;
    }
    io_addr = (IO_ADDRESS(resource->start));

    irq = platform_get_irq(pdev, 0);
    if (irq < 0)
    {
        dev_err(&pdev->dev, "failed to request irq_num\n");
        return -EIO;
    }

    ctlr->mode_bits            = PSPI_MODE_BITS;
    ctlr->bits_per_word_mask   = SPI_BPW_RANGE_MASK(3, HAL_PSPI_MAX_SUPPORT_BITS);
    ctlr->num_chipselect       = cs_num;
    ctlr->dev.of_node          = pdev->dev.of_node;
    ctlr->bus_num              = group;
    ctlr->slave                = slave;
    ctlr->prepare_message      = drv_pspi_prepare_message;
    ctlr->transfer_one_message = drv_pspi_transfer_one_message;
    ctlr->setup                = drv_pspi_setup;

    platform_set_drvdata(pdev, ctlr);

    pspi_ctrl->dev            = &pdev->dev;
    pspi_ctrl->group          = group;
    pspi_ctrl->use_dma        = use_dma;
    pspi_ctrl->hal.use_dma    = use_dma;
    pspi_ctrl->hal.slave_mode = slave;
    pspi_ctrl->hal.pspi_base  = (unsigned long)io_addr;
    pspi_ctrl->hal.wait_done  = drv_pspi_waitdone;
    pspi_ctrl->hal.pad_idx    = (u8)(pad_idx & 0xff);
    pspi_ctrl->hal.pad_mode   = pad_mode;
    pspi_ctrl->hal.mem_rxaddr = IMI_ADDR;
    pspi_ctrl->hal.mem_txaddr = IMI_ADDR;

    if (ctlr->slave)
    {
        ctlr->slave_abort = drv_pspi_slave_abort;
    }

    // clock table source table cfg
    ret = drv_of_clk_init(pdev);
    if (ret)
    {
        dev_err(&pdev->dev, "get pspi-%d clock fail\n", group);
        return -EIO;
    }

    if (!snprintf(pspi_ctrl->irq_name, sizeof(pspi_ctrl->irq_name), "pspi%d_Isr", group))
    {
        return -ENOENT;
    }
    ret = devm_request_irq(&pdev->dev, irq, drv_pspi_interrupt, IRQF_TRIGGER_NONE, dev_name(&pdev->dev), ctlr);
    if (ret)
    {
        dev_err(&pdev->dev, "failed to register irq (%d)\n", ret);
        return -ENOENT;
    }

    init_completion(&pspi_ctrl->xfer_done);

    /* initialise the hardware */
    ret = drv_pspi_hw_init(pspi_ctrl);
    if (ret)
    {
        dev_err(&pdev->dev, "config pspi-%d master: %d\n", group, ret);
        return -EIO;
    }

    ret = spi_register_master(ctlr);
    if (ret)
    {
        dev_err(&pdev->dev, "could not register pspi-%d master: %d\n", group, ret);
        return -EIO;
    }

    return 0;
}

static int drv_pspi_remove(struct platform_device *pdev)
{
    struct clk_hw *        spi_hw;
    struct clk_hw *        parent_hw;
    struct clk *           spi_clock;
    struct spi_controller *ctlr      = spi_controller_get(platform_get_drvdata(pdev));
    struct pspi_ctrl *     pspi_ctrl = spi_master_get_devdata(ctlr);

    if (ctlr)
    {
        hal_pspi_deinit(&pspi_ctrl->hal);

        spi_clock = of_clk_get(pspi_ctrl->dev->of_node, 0);
        if (IS_ERR(spi_clock))
        {
            dev_err(pspi_ctrl->dev, "pspi-%d get clock fail\n", ctlr->bus_num);
            spi_unregister_master(ctlr);
            return -ENOENT;
        }
        spi_hw    = __clk_get_hw(spi_clock);
        parent_hw = clk_hw_get_parent_by_index(spi_hw, 0);
        clk_set_parent(spi_clock, parent_hw->clk);
        clk_disable_unprepare(spi_clock);
        clk_put(spi_clock);

        spi_unregister_master(ctlr);
    }
    else
    {
        return -ENOENT;
    }

    return 0;
}

static const struct of_device_id pspi_of_device_id[] = {{
                                                            .compatible = "sstar,pspi",
                                                        },
                                                        {}};
MODULE_DEVICE_TABLE(of, pspi_of_device_id);

static struct platform_driver pspi_platform_driver = {
    .driver =
        {
            .name           = "pspi",
            .owner          = THIS_MODULE,
            .of_match_table = pspi_of_device_id,
        },
    .probe  = drv_pspi_probe,
    .remove = drv_pspi_remove,
};
module_platform_driver(pspi_platform_driver);

MODULE_DESCRIPTION("PSPI controller driver for SigmaStar");
MODULE_LICENSE("GPL v2");
