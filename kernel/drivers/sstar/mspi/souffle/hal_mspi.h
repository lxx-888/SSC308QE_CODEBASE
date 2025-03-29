/*
 * hal_mspi.h- Sigmastar
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

#ifndef __HAL_MSPI_H__
#define __HAL_MSPI_H__

#define HAL_MSPI_MAX_SUPPORT_BITS 16

struct hal_mspi
{
    u64 base;
    u32 pad_ctrl;
    u32 clk_out_mode;
    u32 cs_num;
    u32 cs_ext[16];
    u32 bits_per_word;
    s32 (*wait_done)(struct hal_mspi *hal);
};

enum hal_mspi_ch
{
    HAL_MSPI_CHL0,
    HAL_MSPI_CHL1,
    HAL_MSPI_CHL2,
    HAL_MSPI_CHL3,
    HAL_MSPI_MAX,
};

typedef enum
{
    HAL_MSPI_MODE0, // CPOL = 0,CPHA =0
    HAL_MSPI_MODE1, // CPOL = 0,CPHA =1
    HAL_MSPI_MODE2, // CPOL = 1,CPHA =0
    HAL_MSPI_MODE3, // CPOL = 1,CPHA =1
    HAL_MSPI_MODE_MAX,
} hal_mspi_mode;

typedef enum
{
    HAL_MSPI_BIT_MSB_FIRST,
    HAL_MSPI_BIT_LSB_FIRST,
} hal_mspi_bit_seq;

enum hal_mspi_return
{
    HAL_MSPI_OK = 0,
    HAL_MSPI_INIT_FLOW_ERROR,
    HAL_MSPI_DCCONFIG_ERROR,
    HAL_MSPI_CLKCONFIG_ERROR,
    HAL_MSPI_FRAMECONFIG_ERROR,
    HAL_MSPI_OPERATION_ERROR,
    HAL_MSPI_PARAM_OVERFLOW,
    HAL_MSPI_MMIO_ERROR,
    HAL_MSPI_TIMEOUT,
    HAL_MSPI_HW_NOT_SUPPORT,
    HAL_MSPI_NOMEM,
    HAL_MSPI_NULL,
    HAL_MSPI_ERR,
    HAL_MSPI_BDMA_CH_INVAILD,
    HAL_MSPI_BDMA_PATH_INVAILD,
};
int  hal_mspi_trigger(struct hal_mspi *hal);
int  hal_mspi_check_dma_mode(u8 group);
int  hal_mspi_config(struct hal_mspi *hal);
u16  hal_mspi_check_done(struct hal_mspi *hal);
void hal_mspi_clear_done(struct hal_mspi *hal);
int  hal_mspi_set_lsb(struct hal_mspi *hal, u8 enable);
int  hal_mspi_set_3wire_mode(struct hal_mspi *hal, u8 enable);
void hal_mspi_set_div_clk(struct hal_mspi *hal, u8 div);
int  hal_mspi_set_mode(struct hal_mspi *hal, hal_mspi_mode mode);
void hal_mspi_chip_select(struct hal_mspi *hal, u8 enable, u8 select);
int  hal_mspi_read(u8 group, struct hal_mspi *hal, u8 *data, u16 size);
int  hal_mspi_write(u8 group, struct hal_mspi *hal, u8 *data, u16 size);
int  hal_mspi_dma_write(u8 group, struct hal_mspi *hal, u8 *data, u32 size);
int  hal_mspi_dma_read(u8 group, struct hal_mspi *hal, u8 *data, u32 size);
int  hal_mspi_full_duplex(u8 group, struct hal_mspi *hal, u8 *rx_buff, u8 *tx_buff, u16 size);
int  hal_mspi_set_frame_cfg(struct hal_mspi *hal, int bits_per_word);

#endif
