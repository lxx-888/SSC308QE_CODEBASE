/*
 * drv_mspi.h- Sigmastar
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

#include "hal_mspi.h"

#define MSPI_CLK_DIV_TBL_SIZE (8)
#define MSPI_CLK_DIV_VAL_TBL  ({2, 4, 8, 16, 32, 64, 128, 256})

#define MSPI_CLK_SRC_TBL_SEZE (4)
#define MSPI_CLK_SRC_VAL_TBL  ({108000000, 54000000, 12000000, 144000000})

struct spi_transfer
{
    const u8 *tx_buf;
    u8 *      rx_buf;
    u16       len;
};

struct mspi_clk
{
    u32 clk_src;
    u32 clk_div;
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
    u16       len;
    const u8 *tx_buf;
    u8 *      rx_buf;

    u32                  mode;
    u32                  bus_num;
    u32                  use_dma;
    u32                  xfer_dma;
    u32                  mosi_pad;
    u32                  mosi_mode;
    u32                  _4to3mode;
    u32                  _3wiremode;
    u32                  max_speed_hz;
    struct hal_mspi      hal_ctrl;
    struct mspi_clk_tbl *clock_table;

    struct udevice *dev;
};
