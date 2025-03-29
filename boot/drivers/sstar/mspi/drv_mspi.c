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

#include "drv_mspi.h"
#include "hal_mspireg.h"
#include <spi.h>
#include <string.h>
#include <fdtdec.h>
#include <dm/device.h>
#include <dm/read.h>
#include <stdlib.h>
#ifdef CONFIG_SSTAR_CLK
#include <clk.h>
#endif

/*******************************************************************/
/*                    marco                                        */
/*******************************************************************/
#define MSPI_CS_NUM_DEFAULT (1)

#define MSPI_MESG_ERR(_msg_, ...)                  \
    do                                             \
    {                                              \
        printf("[mspi err]" _msg_, ##__VA_ARGS__); \
    } while (0)

#define MSPI_MESG_INFO(_msg_, ...)                  \
    do                                              \
    {                                               \
        printf("[mspi info]" _msg_, ##__VA_ARGS__); \
    } while (0)

#ifdef DEBUG_MSG_ON
#define MSPI_MESG_DEBUG(_msg_, ...)                 \
    do                                              \
    {                                               \
        printf("[mspi info]" _msg_, ##__VA_ARGS__); \
    } while (0)
#else
#define MSPI_MESG_DEBUG(_msg_, ...)
#endif

#define MSPI_SPEED_DEFAULT        (27000000)
#define MSPI_MODE_DEFAULT         (HAL_MSPI_MODE0)
#define MSPI_BITSPERWORLD_DEFAULT (8)

/*******************************************************************/
/*                    function definition                          */
/*******************************************************************/
typedef enum mspi_err_num
{
    MSPI_RET_OK,
    MSPI_RET_OFPLAT,
    MSPI_RET_CLKSET,
    MSPI_RET_CLKEN,
    MSPI_RET_TRANS,
    MSPI_RET_MAX
} MSPI_ERRNUM;

typedef enum mspi_bus_num
{
    MSPI_0,
    MSPI_1,
    MSPI_2,
    MSPI_3,
    MSPI_MAX
} MSPI_BUSNUM;

/*******************************************************************/
/*                    const value                                  */
/*******************************************************************/
const u32 clk_div_array[MSPI_CLK_DIV_TBL_SIZE] = {2, 4, 8, 16, 32, 64, 128, 256};
const u32 clk_src_array[MSPI_CLK_SRC_TBL_SEZE] = {108000000, 54000000, 12000000, 144000000};

/*******************************************************************/
/*                    function definition                          */
/*******************************************************************/
static int sstar_mspi_clktbl_init(struct mspi_control *mspi_bus)
{
    unsigned int         tmp_clk;
    unsigned char        i, j;
    struct mspi_clk_tbl *mspi_clk_tbl = mspi_bus->clock_table;
    struct mspi_clk      tmp_mspi_clk;
    struct dm_spi_bus *  dm_spi_bus = (struct dm_spi_bus *)dev_get_uclass_priv(mspi_bus->dev);

    mspi_clk_tbl->clk_src_tbl_sz = MSPI_CLK_SRC_TBL_SEZE;
    memcpy(mspi_clk_tbl->clk_src_tbl, clk_src_array, (sizeof(u32) * MSPI_CLK_SRC_TBL_SEZE));

    mspi_clk_tbl->clk_div_tbl_sz = MSPI_CLK_DIV_TBL_SIZE;
    memcpy(mspi_clk_tbl->clk_div_tbl, clk_div_array, (sizeof(u32) * MSPI_CLK_DIV_TBL_SIZE));

    mspi_clk_tbl->clk_cfg_tbl_sz = (MSPI_CLK_SRC_TBL_SEZE * MSPI_CLK_DIV_TBL_SIZE);

    for (i = 0; i < mspi_clk_tbl->clk_src_tbl_sz; i++)
    {
        tmp_clk = mspi_clk_tbl->clk_src_tbl[i];
        for (j = 0; j < mspi_clk_tbl->clk_div_tbl_sz; j++)
        {
            mspi_clk_tbl->clk_cfg_tbl[j + mspi_clk_tbl->clk_div_tbl_sz * i].clk_div = j;
            mspi_clk_tbl->clk_cfg_tbl[j + mspi_clk_tbl->clk_div_tbl_sz * i].clk_src = i;
            mspi_clk_tbl->clk_cfg_tbl[j + mspi_clk_tbl->clk_div_tbl_sz * i].clk_rate =
                tmp_clk / mspi_clk_tbl->clk_div_tbl[j];
        }
    }

    for (i = 0; i < mspi_clk_tbl->clk_cfg_tbl_sz; i++)
    {
        for (j = i; j < mspi_clk_tbl->clk_cfg_tbl_sz; j++)
        {
            if (mspi_clk_tbl->clk_cfg_tbl[i].clk_rate > mspi_clk_tbl->clk_cfg_tbl[j].clk_rate)
            {
                memcpy(&tmp_mspi_clk, &mspi_clk_tbl->clk_cfg_tbl[i], sizeof(struct mspi_clk));

                memcpy(&mspi_clk_tbl->clk_cfg_tbl[i], &mspi_clk_tbl->clk_cfg_tbl[j], sizeof(struct mspi_clk));

                memcpy(&mspi_clk_tbl->clk_cfg_tbl[j], &tmp_mspi_clk, sizeof(struct mspi_clk));
            }
        }
    }
    for (i = 0; i < mspi_clk_tbl->clk_cfg_tbl_sz; i++)
    {
        MSPI_MESG_DEBUG("i = %d src clk = %d\n", i, mspi_clk_tbl->clk_cfg_tbl[i].clk_src);
        MSPI_MESG_DEBUG("i = %d src div = %d\n", i, mspi_clk_tbl->clk_cfg_tbl[i].clk_div);
        MSPI_MESG_DEBUG("i = %d  rate = %d\n", i, mspi_clk_tbl->clk_cfg_tbl[i].clk_rate);
    }
    dm_spi_bus->max_hz = mspi_clk_tbl->clk_cfg_tbl[mspi_clk_tbl->clk_cfg_tbl_sz - 1].clk_rate;

    return 0;
}

static int sstar_mspi_clk_set(struct mspi_control *mspi_bus, u32 speed)
{
    s32        ret = 0;
    u32        i;
    ulong      clk_rate_set = 0;
    struct clk clk          = {0};

    struct mspi_clk_tbl *pst_mspi_clk_tbl = mspi_bus->clock_table;

    for (i = 0; i < pst_mspi_clk_tbl->clk_cfg_tbl_sz; i++)
    {
        if (speed <= pst_mspi_clk_tbl->clk_cfg_tbl[i].clk_rate)
        {
            MSPI_MESG_DEBUG("Count i = %d, clk_rate = %d, speed = %d\n", i, pst_mspi_clk_tbl->clk_cfg_tbl[i].clk_rate,
                            speed);
            break;
        }
    }
    if (pst_mspi_clk_tbl->clk_cfg_tbl_sz == i)
    {
        i--;
    }
    // match Closer clk
    if ((i < (pst_mspi_clk_tbl->clk_cfg_tbl_sz - 1))
        && ((speed - pst_mspi_clk_tbl->clk_cfg_tbl[i - 1].clk_rate)
            < (pst_mspi_clk_tbl->clk_cfg_tbl[i].clk_rate - speed)))
    {
        i -= 1;
    }
    MSPI_MESG_DEBUG("set |mspi%d| i = %d, src clk rate: %d, index: %d, speed: %d\n", mspi_bus->bus_num, i,
                    pst_mspi_clk_tbl->clk_cfg_tbl[i].clk_rate, pst_mspi_clk_tbl->clk_cfg_tbl[i].clk_src, speed);

    ret = clk_get_by_index(mspi_bus->dev, 0, &clk);
    if (ret < 0)
    {
        MSPI_MESG_ERR("get mspi%d clk failed\n", mspi_bus->bus_num);
    }
    else
    {
        ret = clk_enable(&clk);
        if (ret < 0)
        {
            MSPI_MESG_ERR("enable mspi%d clk failed\n", mspi_bus->bus_num);
        }
    }
    clk_rate_set = clk_set_rate(&clk, pst_mspi_clk_tbl->clk_src_tbl[pst_mspi_clk_tbl->clk_cfg_tbl[i].clk_src]);
    MSPI_MESG_DEBUG("set rate return : %d\n", clk_rate_set);
    hal_mspi_set_div_clk(&mspi_bus->hal_ctrl, pst_mspi_clk_tbl->clk_cfg_tbl[i].clk_div);

    return ret;
}

static int sstar_mspi_start_transfer(struct mspi_control *mspi_bus, struct spi_transfer *tfr)
{
    s32                  ret       = 0;
    struct mspi_control *mspi_ctrl = mspi_bus;

    MSPI_MESG_DEBUG("All = %x\n", mspi_ctrl->mode);
    MSPI_MESG_DEBUG("SPI mode = %d\n", mspi_ctrl->mode & 0x03);
    MSPI_MESG_DEBUG("LSB first = %d\n", mspi_ctrl->mode & 0x08);

    hal_mspi_chip_select(&mspi_ctrl->hal_ctrl, !((mspi_ctrl->mode & SPI_CS_HIGH) == SPI_CS_HIGH), 0);
    mspi_ctrl->tx_buf = tfr->tx_buf;
    mspi_ctrl->rx_buf = tfr->rx_buf;
    mspi_ctrl->len    = tfr->len;

#if 0
    if (pst_mspi_bus->use_dma)
    {
        err = sstar_mspi_dma_clk_set(spi->master->bus_num, sstar_spimst);
        if (err)
        {
            return -EIO;
        }
    }
#endif

    if (mspi_ctrl->tx_buf != NULL && mspi_ctrl->rx_buf != NULL)
    {
        hal_mspi_full_duplex(mspi_ctrl->bus_num, &mspi_ctrl->hal_ctrl, (u8 *)mspi_ctrl->rx_buf, (u8 *)mspi_ctrl->tx_buf,
                             (u16)mspi_bus->len);
    }
    else if (mspi_ctrl->tx_buf != NULL)
    {
        if (mspi_ctrl->xfer_dma)
        {
            ret = hal_mspi_dma_write(mspi_ctrl->bus_num, &mspi_ctrl->hal_ctrl, (u8 *)mspi_ctrl->tx_buf,
                                     (u32)mspi_ctrl->len);
            if (ret)
            {
                return -EIO;
            }
        }
        else
        {
            ret =
                hal_mspi_write(mspi_ctrl->bus_num, &mspi_ctrl->hal_ctrl, (u8 *)mspi_ctrl->tx_buf, (u32)mspi_ctrl->len);
            if (ret)
            {
                return -EIO;
            }
        }
    }
    else if (mspi_ctrl->rx_buf != NULL)
    {
        /* //MSPI_4TO3MODE
        if (mspi_ctrl->_4to3mode)
        {
            sstar_gpio_pad_val_set(mspi_ctrl->mosi_pad, PINMUX_FOR_GPIO_MODE);
            sstar_gpio_pad_odn(mspi_ctrl->mosi_pad);
        }
        *///end of MSPI_4TO3MODE
        if (mspi_ctrl->xfer_dma)
        {
            ret = hal_mspi_dma_read(mspi_ctrl->bus_num, &mspi_ctrl->hal_ctrl, (u8 *)mspi_ctrl->rx_buf,
                                    (u32)mspi_ctrl->len);
            if (ret)
            {
                return -EIO;
            }
        }
        else
        {
            ret = hal_mspi_read(mspi_ctrl->bus_num, &mspi_ctrl->hal_ctrl, (u8 *)mspi_ctrl->rx_buf, (u32)mspi_ctrl->len);
            if (ret)
            {
                return -EIO;
            }
        }
        /* //MSPI_4TO3MODE
        if (mspi_ctrl->_4to3mode)
        {
            sstar_gpio_pad_val_set(mspi_ctrl->mosi_pad, mspi_ctrl->mosi_mode);
        }
        *///end of MSPI_4TO3MODE
    }

    return ret;
}

static int sstar_mspi_finish_transfer(struct mspi_control *mspi_bus, struct spi_transfer *tfr, u8 cs_change)
{
    struct mspi_control *mspi_ctrl = mspi_bus;

    if (cs_change)
    {
        /* Clear TA flag */
        hal_mspi_chip_select(&mspi_ctrl->hal_ctrl, ((mspi_ctrl->mode & SPI_CS_HIGH) == SPI_CS_HIGH), 0);
    }
    return 0;
}

static int sstar_mspi_transfer_one(struct mspi_control *mspi_bus, struct spi_transfer *tfr, unsigned long flags)
{
    s32                  ret       = 0;
    u8                   cs_change = 1;
    struct mspi_control *mspi_ctrl = mspi_bus;

    mspi_ctrl->xfer_dma = ((mspi_ctrl->hal_ctrl.bits_per_word % 8) == 0) ? (mspi_ctrl->use_dma) : 0;

    ret = sstar_mspi_start_transfer(mspi_ctrl, tfr);
    if (ret)
    {
        MSPI_MESG_ERR("start_transfer err\n");
        return -MSPI_RET_TRANS;
    }

    if (flags & SPI_XFER_END)
    {
        ret = sstar_mspi_finish_transfer(mspi_ctrl, tfr, cs_change);
        if (ret)
        {
            MSPI_MESG_ERR("finish transfer err\n");
            return -MSPI_RET_TRANS;
        }
    }

    return ret;
}

static int sstar_mspi_claim_bus(struct udevice *dev)
{
    int                  ret       = 0;
    struct mspi_control *mspi_ctrl = NULL;
    struct udevice *     udev      = (struct udevice *)dev_get_parent(dev);

    mspi_ctrl = (struct mspi_control *)dev_get_priv(udev);

    mspi_ctrl->hal_ctrl.mode = mspi_ctrl->mode;
    ret |= sstar_mspi_clktbl_init(mspi_ctrl);
    ret |= sstar_mspi_clk_set(mspi_ctrl, mspi_ctrl->max_speed_hz);
    hal_mspi_config(&mspi_ctrl->hal_ctrl);

    return ret;
}

static int sstar_mspi_release_bus(struct udevice *dev)
{
    int                  ret       = 0;
    struct mspi_control *mspi_ctrl = NULL;
    struct udevice *     udev      = (struct udevice *)dev_get_parent(dev);
    struct clk           clk       = {0};

    mspi_ctrl = (struct mspi_control *)dev_get_priv(udev);
    ret       = clk_get_by_index(mspi_ctrl->dev, 0, &clk);

    ret = clk_disable(&clk);

    return ret;
}

static int sstar_mspi_set_wordlen(struct udevice *dev, unsigned int wordlen)
{
    int                  ret       = 0;
    struct mspi_control *mspi_ctrl = NULL;

    mspi_ctrl                         = (struct mspi_control *)dev_get_priv(dev);
    mspi_ctrl->hal_ctrl.bits_per_word = wordlen;
    return ret;
}

static int sstar_mspi_xfer(struct udevice *dev, unsigned int bitlen, const void *dout, void *din, unsigned long flags)
{
    int                  ret = 0;
    struct spi_transfer  tfr;
    struct mspi_control *mspi_ctrl = NULL;
    struct udevice *     udev      = (struct udevice *)dev_get_parent(dev);

    mspi_ctrl = (struct mspi_control *)dev_get_priv(udev);
    hal_mspi_set_frame_cfg(&mspi_ctrl->hal_ctrl, mspi_ctrl->hal_ctrl.bits_per_word);

    mspi_ctrl->xfer_dma = (bitlen % 8 == 0) ? mspi_ctrl->use_dma : false;

    tfr.tx_buf = dout;
    tfr.rx_buf = din;
    tfr.len    = bitlen;
    ret |= sstar_mspi_transfer_one(mspi_ctrl, &tfr, flags);

    return ret;
}

static int sstar_mspi_set_speed(struct udevice *dev, uint hz)
{
    int                  ret       = 0;
    struct mspi_control *mspi_ctrl = NULL;

    mspi_ctrl = (struct mspi_control *)dev_get_priv(dev);

    mspi_ctrl->max_speed_hz = hz;

    return ret;
}

static int sstar_mspi_set_mode(struct udevice *dev, uint mode)
{
    int                  ret       = 0;
    struct mspi_control *mspi_ctrl = NULL;

    mspi_ctrl = (struct mspi_control *)dev_get_priv(dev);

    mspi_ctrl->mode = mode;
    return ret;
}

// check whether cs is aviliable
static int sstar_mspi_cs_info(struct udevice *dev, uint cs, struct spi_cs_info *info)
{
    int ret = 0;

    // info->dev = dev;
    return ret;
}

static int sstar_mspi_get_mmap(struct udevice *dev, ulong *map_basep, uint *map_sizep, uint *offsetp)
{
    int ret = 0;

    return ret;
}

static int sstar_mspi_of_to_plat(struct udevice *dev)
{
    int                  ret = 0;
    int                  group, use_dma, cs_num;
    int                  cs_ext, _4to3_mode, clkout_mode;
    fdt_addr_t           io_addr;
    struct mspi_control *mspi_ctrl = dev_get_priv(dev);

    memset(mspi_ctrl, 0, sizeof(struct mspi_control));

    mspi_ctrl->clock_table = (struct mspi_clk_tbl *)malloc(sizeof(struct mspi_clk_tbl));
    memset(mspi_ctrl->clock_table, 0, sizeof(struct mspi_clk_tbl));

    mspi_ctrl->clock_table->clk_src_tbl = (u32 *)malloc(sizeof(unsigned int) * MSPI_CLK_SRC_TBL_SEZE);
    memset(mspi_ctrl->clock_table->clk_src_tbl, 0, (sizeof(u32) * MSPI_CLK_SRC_TBL_SEZE));

    mspi_ctrl->clock_table->clk_div_tbl = (u32 *)malloc(sizeof(u32) * MSPI_CLK_DIV_TBL_SIZE);
    memset(mspi_ctrl->clock_table->clk_div_tbl, 0, (sizeof(u32) * MSPI_CLK_DIV_TBL_SIZE));

    mspi_ctrl->clock_table->clk_cfg_tbl =
        (struct mspi_clk *)malloc(sizeof(struct mspi_clk) * (MSPI_CLK_DIV_TBL_SIZE * MSPI_CLK_SRC_TBL_SEZE));
    memset(mspi_ctrl->clock_table->clk_cfg_tbl, 0,
           (sizeof(struct mspi_clk) * (MSPI_CLK_DIV_TBL_SIZE * MSPI_CLK_SRC_TBL_SEZE)));

    io_addr = dev_read_addr(dev);
    if (io_addr == FDT_ADDR_T_NONE)
    {
        return -EINVAL;
    }
    mspi_ctrl->hal_ctrl.base = (u64)(phys_addr_t)(void __iomem *)io_addr;

    group = dev_read_u32_default(dev, "mspi-group", -1);
    if (-1 == group)
    {
        MSPI_MESG_ERR("get mspi_group failed\n");
        return -MSPI_RET_OFPLAT;
    }

    use_dma = dev_read_u32_default(dev, "use-dma", 0);
    cs_num  = dev_read_u32_default(dev, "cs-num", 0);
    if (!cs_num)
    {
        cs_num = 1;
    }

    cs_ext      = dev_read_u32_default(dev, "cs-ext", 0);
    _4to3_mode  = dev_read_u32_default(dev, "4to3-mode", 0);
    clkout_mode = dev_read_u32_default(dev, "clk-out-mode", 0);

    mspi_ctrl->bus_num         = (u32)group;
    mspi_ctrl->use_dma         = (u32)use_dma;
    mspi_ctrl->_4to3mode       = (u32)_4to3_mode;
    mspi_ctrl->hal_ctrl.cs_num = (u32)cs_num;
    // mspi_ctrl->st_hal_ctrl.cs_ext       = 0;
    mspi_ctrl->hal_ctrl.bits_per_word = MSPI_BITSPERWORLD_DEFAULT;
    mspi_ctrl->hal_ctrl.clk_out_mode  = (u32)_4to3_mode;
    mspi_ctrl->mode                   = MSPI_MODE_DEFAULT;

    mspi_ctrl->dev = dev;

    return ret;
}

/*******************************************************************/
/*                    uboot struct                                 */
/*******************************************************************/
static const struct dm_spi_ops mspi_dm_ops = {
    .claim_bus   = sstar_mspi_claim_bus,
    .release_bus = sstar_mspi_release_bus,
    .set_wordlen = sstar_mspi_set_wordlen,
    .xfer        = sstar_mspi_xfer,
    // const struct spi_controller_mem_ops *mem_ops,
    .set_speed = sstar_mspi_set_speed,
    .set_mode  = sstar_mspi_set_mode,
    .cs_info   = sstar_mspi_cs_info,
    .get_mmap  = sstar_mspi_get_mmap,
};

static const struct udevice_id mspi_udev_id[] = {{.compatible = "sstar,mspi"}, {}};

U_BOOT_DRIVER(mspi_control) = {.name       = "ssatr_mspi",
                               .id         = UCLASS_SPI,
                               .of_match   = mspi_udev_id,
                               .of_to_plat = sstar_mspi_of_to_plat,
                               .priv_auto  = sizeof(struct mspi_control),
                               .ops        = &mspi_dm_ops};
