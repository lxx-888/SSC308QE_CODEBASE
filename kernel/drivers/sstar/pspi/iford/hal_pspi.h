/*
 * hal_pspi.h- Sigmastar
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

#ifndef __HAL_PSPI_H__
#define __HAL_PSPI_H__

#define MSPI_NUMS                 1
#define HAL_PSPI_MAX_SUPPORT_BITS 32
#define HAL_PSPI_NR_PORTS         1

enum hal_pspi_irq_type
{
    HAL_PSPI_IRQ_UNKNOWN                      = 0x00000000,
    HAL_PSPI_IRQ_RX_FIFO_LE                   = 0x00000001,
    HAL_PSPI_IRQ_RX_FIFO_GE                   = 0x00000002,
    HAL_PSPI_IRQ_RX_FIFO_EMPTY                = 0x00000004,
    HAL_PSPI_IRQ_RX_FIFO_FULL                 = 0x00000008,
    HAL_PSPI_IRQ_TX_FIFO_LE                   = 0x00000010,
    HAL_PSPI_IRQ_TX_FIFO_GE                   = 0x00000020,
    HAL_PSPI_IRQ_TX_FIFO_EMPTY                = 0x00000040,
    HAL_PSPI_IRQ_TX_FIFO_FULL                 = 0x00000080,
    HAL_PSPI_IRQ_RX_DMA_COMPLETED             = 0x00000100,
    HAL_PSPI_IRQ_TX_DMA_COMPLETED             = 0x00000200,
    HAL_PSPI_IRQ_TX_DONE                      = 0x00000400,
    HAL_PSPI_IRQ_TO_FIFO                      = 0x00000800,
    HAL_PSPI_IRQ_TRANSFER_COMPLETED           = 0x00001000,
    HAL_PSPI_IRQ_RX_FIFO_OVERFLOW             = 0x00100000,
    HAL_PSPI_IRQ_TX_FIFO_UNDERFLOW            = 0x00200000,
    HAL_PSPI_IRQ_RECEIVE_BIT_COUNTER_OVERFLOW = 0x00400000,
};

struct hal_pspi
{
    u32  group;
    u8   bits_per_word;
    u8   data_lane;
    u16  delay_cycle;
    u16  wait_cycle;
    u8   cpol;
    u8   cpha;
    u8   lsb_first;
    u8   chip_select_timing;
    u8   divisor;
    u8   te_mode;
    u8   te_polarity;
    u8   te_skip_number;
    u16  te_time_delay;
    u8   slave_mode;
    u8   rgb_swap;
    bool use_dma;
    u32  wire_3line;
    u8   mem_sel;
    u8   cs_level;
    u8   chip_select;
    u32  size;

    u8  pad_idx;
    u32 pad_mode;

    unsigned long pspi_base;
    u32           mem_rxaddr;
    u32           mem_txaddr;

    s32 (*wait_done)(struct hal_pspi *hal);
};

#define PSPI_LNX_BDMA_WRITE0 PSPI_BDMA_CM4_IMI_TO_MIU0
#define PSPI_LNX_BDMA_READ0  PSPI_BDMA_MIU0_TO_CM4_IMI
#define PSPI_BDMA_CH0        PSPI_BDMA2_CH1

int  hal_pspi_trigger(struct hal_pspi *hal);
void hal_pspi_controller_select(struct hal_pspi *hal);
void hal_pspi_init(struct hal_pspi *hal);
void hal_pspi_deinit(struct hal_pspi *hal);
int  hal_pspi_irq_enable(struct hal_pspi *hal, enum hal_pspi_irq_type type, u8 u8_enable);
u32  hal_pspi_get_irq_type(struct hal_pspi *hal);
void hal_pspi_config(struct hal_pspi *hal);
void hal_pspi_cs_assert(struct hal_pspi *hal, u8 cs, u8 cs_keep);
void hal_pspi_cs_deassert(struct hal_pspi *hal, u8 cs, u8 cs_keep);
int  hal_pspi_dma_write(struct hal_pspi *hal, u8 *data, u32 size);
int  hal_pspi_dma_read(struct hal_pspi *hal, u8 *data, u32 size);
int  hal_pspi_fifo_write(struct hal_pspi *hal, u8 *data, u32 size);
int  hal_pspi_fifo_read(struct hal_pspi *hal, u8 *data, u32 size);
void hal_pspi_transfer_en(struct hal_pspi *hal, bool is_tx);
int  hal_pspi_small_reset(struct hal_pspi *hal);

#endif
