/*
 * hal_uart.c- Sigmastar
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
#include "hal_uart.h"

/* Interrupt Enable Register (IER)*/
#define HAL_UART_IER_RDI   0x01 /* Received Data Available Interrupt */
#define HAL_UART_IER_THRI  0x02 /* Transmitter Holding Register Empty Interrupt */
#define HAL_UART_IER_RLSI  0x04 /* Receiver Line Status Interrupt */
#define HAL_UART_IER_MSI   0x08 /* Modem Status Interrupt */
#define HAL_UART_IER_PTHRI 0x80 /* Programmable THRE Interrupt */

/* Interrupt Identification Register (IIR) */
#define HAL_UART_IIR_MSI        0x00 /* 0000: Modem Status */
#define HAL_UART_IIR_NO_INT     0x01 /* 0001: No pending interrupts */
#define HAL_UART_IIR_THRI       0x02 /* 0010: Transmitter Holding Register Empty */
#define HAL_UART_IIR_RDI        0x04 /* 0100: Receiver Data Available */
#define HAL_UART_IIR_RLSI       0x06 /* 0110: Receiver Line Status */
#define HAL_UART_IIR_BUSY       0x07 /* 0111: Busy detect indication (try to write LCR while UART is busy) */
#define HAL_UART_IIR_RX_TIMEOUT 0x0C /* 1100: Character timeout */
#define HAL_UART_IIR_ID_MASK    0x0F /* Mask Bit[3:0] for IIR */

/* FIFO Control Register (FCR) */
#define HAL_UART_FCR_FIFO_ENABLE   0x01 /* Clear & Reset Rx FIFO buffer */
#define HAL_UART_FCR_CLEAR_RCVR    0x02 /* Clear & Reset Rx FIFO buffer */
#define HAL_UART_FCR_CLEAR_XMIT    0x04 /* Clear & Reset Tx FIFO buffer */
#define HAL_UART_FCR_TRIGGER_TX_L0 0x00 /* Trigger Write when emtpy */
#define HAL_UART_FCR_TRIGGER_TX_L1 0x10 /* Trigger Write when 2 characters */
#define HAL_UART_FCR_TRIGGER_TX_L2 0x20 /* Trigger Write when 1/4 full */
#define HAL_UART_FCR_TRIGGER_TX_L3 0x30 /* Trigger Write when 1/2 full */
#define HAL_UART_FCR_TRIGGER_RX_L0 0x00 /* Trigger Read when there is 1 char*/
#define HAL_UART_FCR_TRIGGER_RX_L1 0x40 /* Trigger Read when 1/4 full */
#define HAL_UART_FCR_TRIGGER_RX_L2 0x80 /* Trigger Read when 1/2 full */
#define HAL_UART_FCR_TRIGGER_RX_L3 0xC0 /* Trigger Read when 2 less then full  */

/* Line Control Register (LCR) */
#define HAL_UART_LCR_WL_MASK    0x03 /* Word length mask */
#define HAL_UART_LCR_WLEN5      0x00 /* Word length is 5 bits */
#define HAL_UART_LCR_WLEN6      0x01 /* Word length is 6 bits */
#define HAL_UART_LCR_WLEN7      0x02 /* Word length is 7 bits */
#define HAL_UART_LCR_WLEN8      0x03 /* Word length is 8 bits */
#define HAL_UART_LCR_STOP_MASK  0x04 /* Stop bit mask */
#define HAL_UART_LCR_STOP1      0x00 /* Stop length is 1 bit */
#define HAL_UART_LCR_STOP2      0x04 /* Stop length is 1.5 bits (5-bit char), 2 bits (otherwise) */
#define HAL_UART_LCR_PARITY_EN  0x08 /* Parity Enable */
#define HAL_UART_LCR_PARITY_SEL 0x10 /* Even Parity Select */
#define HAL_UART_LCR_SBC        0x40 /* Set break control */
#define HAL_UART_LCR_DLAB       0x80 /* Divisor Latch Access bit, 1=Divisor Latch, 0=Normal Register */

#define HAL_UART_MCR_DTR      0x01
#define HAL_UART_MCR_RTS      0x02
#define HAL_UART_MCR_OUT1     0x04
#define HAL_UART_MCR_OUT2     0x08
#define HAL_UART_MCR_LOOPBACK 0x10
#define HAL_UART_MCR_AFCE     0x20

/* Line Status Register */
#define HAL_UART_LSR_DR    0x01 /* Data Ready, at least one char in FIFO buffer*/
#define HAL_UART_LSR_OE    0x02 /* Overrun Error, FIFO buffer is full */
#define HAL_UART_LSR_PE    0x04 /* Parity Error */
#define HAL_UART_LSR_FE    0x08 /* Framing Error, no valid stop bit */
#define HAL_UART_LSR_BI    0x10 /* Break Interrupt */
#define HAL_UART_LSR_THRE  0x20 /* Tx FIFO buffer is empty*/
#define HAL_UART_LSR_TEMT  0x40 /* Both TX FIFO buffer & shift register are empty */
#define HAL_UART_LSR_ERROR 0x80 /* Rx FIFO buffer is error */

#define HAL_UART_USR_BUSY            0x01
#define HAL_UART_USR_TXFIFO_NOT_FULL 0x02
#define HAL_UART_USR_TXFIFO_EMPTY    0x04

#define HAL_UART_REG_DLL_THR_RBR(hal) ((hal->uart_base) + ((0x00) << 2))
#define HAL_UART_REG_DLH_IER(hal)     ((hal->uart_base) + ((0x02) << 2))
#define HAL_UART_REG_IIR_FCR(hal)     ((hal->uart_base) + ((0x04) << 2))
#define HAL_UART_REG_LCR(hal)         ((hal->uart_base) + ((0x06) << 2))
#define HAL_UART_REG_MCR(hal)         ((hal->uart_base) + ((0x08) << 2))
#define HAL_UART_REG_LSR(hal)         ((hal->uart_base) + ((0x0A) << 2))
#define HAL_UART_REG_MSR(hal)         ((hal->uart_base) + ((0x0C) << 2))
#define HAL_UART_REG_USR(hal)         ((hal->uart_base) + ((0x0E) << 2))
#define HAL_UART_REG_TFL(hal)         ((hal->uart_base) + ((0x10) << 2))
#define HAL_UART_REG_RFL(hal)         ((hal->uart_base) + ((0x12) << 2))
#define HAL_UART_REG_RST(hal)         ((hal->uart_base) + ((0x14) << 2))

#ifdef SSTAR_URDMA_ENABLE

/* urdma ctrl */
#define HAL_URDMA_SW_RST     (1 << 0)
#define HAL_URDMA_DMA_MODE   (1 << 1)
#define HAL_URDMA_TX_DMA_EN  (1 << 2)
#define HAL_URDMA_RX_DMA_EN  (1 << 3)
#define HAL_URDMA_TX_ENDINA  (1 << 4)
#define HAL_URDMA_RX_ENDINA  (1 << 5)
#define HAL_URDMA_TX_SW_RST  (1 << 6)
#define HAL_URDMA_RX_SW_RST  (1 << 7)
#define HAL_URDMA_DMA2MIU_NS (1 << 8)
#define HAL_URDMA_RX_OP_MODE (1 << 11)
#define HAL_URDMA_TX_BUSY    (1 << 12)
#define HAL_URDMA_RX_BUSY    (1 << 13)

/* urdma int ctrl */
#define HAL_URDMA_RX_INTR_CLR      (1 << 0)
#define HAL_URDMA_RX_INTR1_EN      (1 << 1)
#define HAL_URDMA_RX_INTR2_EN      (1 << 2)
#define HAL_URDMA_RX_INTR1         (1 << 4)
#define HAL_URDMA_RX_INTR2         (1 << 5)
#define HAL_URDMA_RX_MCU_INTR      (1 << 7)
#define HAL_URDMA_TX_INTR_CLR      (1 << 8)
#define HAL_URDMA_TX_INTR_EN       (1 << 9)
#define HAL_URDMA_TX_EMPTY_INT_EN  (1 << 10)
#define HAL_URDMA_TX_EMPTY_INT_CLR (1 << 11)
#define HAL_URDMA_TX_EMPTY_INT     (1 << 14)
#define HAL_URDMA_TX_MCU_INTR      (1 << 15)

#define HAL_URDMA_REG_CTRL(hal)           ((hal->urdma_base) + ((0x00) << 2))
#define HAL_URDMA_REG_INTR_THRESHOLD(hal) ((hal->urdma_base) + ((0x01) << 2))
#define HAL_URDMA_REG_TXBUF_H(hal)        ((hal->urdma_base) + ((0x02) << 2))
#define HAL_URDMA_REG_TXBUF_L(hal)        ((hal->urdma_base) + ((0x03) << 2))
#define HAL_URDMA_REG_TXBUF_SIZE(hal)     ((hal->urdma_base) + ((0x04) << 2))
#define HAL_URDMA_REG_TXBUF_RPTR(hal)     ((hal->urdma_base) + ((0x05) << 2))
#define HAL_URDMA_REG_TXBUF_WPTR(hal)     ((hal->urdma_base) + ((0x06) << 2))
#define HAL_URDMA_REG_TX_TIMEOUT(hal)     ((hal->urdma_base) + ((0x07) << 2))
#define HAL_URDMA_REG_RXBUF_H(hal)        ((hal->urdma_base) + ((0x08) << 2))
#define HAL_URDMA_REG_RXBUF_L(hal)        ((hal->urdma_base) + ((0x09) << 2))
#define HAL_URDMA_REG_RXBUF_SIZE(hal)     ((hal->urdma_base) + ((0x0A) << 2))
#define HAL_URDMA_REG_RXBUF_WPTR(hal)     ((hal->urdma_base) + ((0x0B) << 2))
#define HAL_URDMA_REG_RX_TIMEOUT(hal)     ((hal->urdma_base) + ((0x0C) << 2))
#define HAL_URDMA_REG_INT_CTRL(hal)       ((hal->urdma_base) + ((0x0D) << 2))
#define HAL_URDMA_REG_DEBUG(hal)          ((hal->urdma_base) + ((0x0F) << 2))
#define HAL_URDMA_REG_2MIU_SEL(hal)       ((hal->urdma_base) + ((0x10) << 2))
#define HAL_URDMA_REG_TXBUF_MSB(hal)      ((hal->urdma_base) + ((0x14) << 2))
#define HAL_URDMA_REG_RXBUF_MSB(hal)      ((hal->urdma_base) + ((0x15) << 2))
#define HAL_URDMA_REG_MIU_ADDR0_SEL(hal)  ((hal->urdma_base) + ((0x16) << 2))
#endif // #ifdef SSTAR_URDMA_ENABLE

#define HAL_UART_READ_BYTE(_reg_)         (*(volatile unsigned char *)((_reg_)))
#define HAL_UART_WRITE_BYTE(_reg_, _val_) ((*(volatile unsigned char *)((_reg_))) = (unsigned char)(_val_))
#define HAL_UART_WRITE_BYTE_MASK(_reg_, _val_, mask) \
    ((*(volatile unsigned char *)((_reg_))) =        \
         ((*(volatile unsigned char *)((_reg_))) & ~(mask)) | ((unsigned char)(_val_) & (mask)))
#define HAL_UART_READ_WORD(_reg_)         (*(volatile unsigned short *)((_reg_)))
#define HAL_UART_WRITE_WORD(_reg_, _val_) ((*(volatile unsigned short *)((_reg_))) = (unsigned short)(_val_))
#define HAL_UART_WRITE_WORD_MASK(_reg_, _val_, mask) \
    ((*(volatile unsigned short *)((_reg_))) =       \
         ((*(volatile unsigned short *)((_reg_))) & ~(mask)) | ((unsigned short)(_val_) & (mask)))

/* chip diff */
static u16 hal_uart_bank[]                                            = HAL_UART_BANK;
static u32 hal_uart_digmux_reg[sizeof(hal_uart_bank) / sizeof(u16)]   = HAL_UART_DIGMUX_REG;
static u16 hal_uart_digmux_shift[sizeof(hal_uart_bank) / sizeof(u16)] = HAL_UART_DIGMUX_SHIFT;
static u16 hal_uart_digmux_mask[sizeof(hal_uart_bank) / sizeof(u16)]  = HAL_UART_DIGMUX_MASK;
static u16 hal_uart_digmux_sel[sizeof(hal_uart_bank) / sizeof(u16)]   = HAL_UART_DIGMUX_SEL;

static void hal_uart_force_rx_disable(struct uart_hal *hal, u8 status)
{
#ifdef CONFIG_GPIO_SUPPORT
    if (hal->rx_pin)
    {
        if (status)
        {
            drv_gpio_pad_oen(hal->rx_pin);
        }
        else
        {
            drv_gpio_pad_oen(hal->rx_pin);
        }
    }
    else if (hal->digmux != 0xFF)
#endif
    {
        HAL_UART_WRITE_WORD_MASK(HAL_UART_REG_FORCE_RX_DISABLE, (~status << hal->digmux), (1 << hal->digmux));
    }
}

static u8 hal_uart_clear_fifos(struct uart_hal *hal)
{
    unsigned int timeout = 0;

    if (!hal->urdma_en)
    {
        do
        {
            if (((HAL_UART_READ_BYTE(HAL_UART_REG_LSR(hal)) & HAL_UART_LSR_DR) == HAL_UART_LSR_DR))
                HAL_UART_READ_BYTE(HAL_UART_REG_DLL_THR_RBR(hal));
            else
                break;
        } while (1);
    }
    while (timeout < 2000)
    {
        if (!(HAL_UART_READ_BYTE(HAL_UART_REG_USR(hal)) & HAL_UART_USR_BUSY)
            && ((HAL_UART_READ_BYTE(HAL_UART_REG_IIR_FCR(hal)) & HAL_UART_IIR_ID_MASK) == HAL_UART_IIR_NO_INT))
        {
            break;
        }

        HAL_UART_WRITE_BYTE(HAL_UART_REG_IIR_FCR(hal), HAL_UART_FCR_FIFO_ENABLE | HAL_UART_FCR_CLEAR_RCVR
                                                           | HAL_UART_FCR_CLEAR_XMIT | ((hal->tx_fifo_level & 0x3) << 4)
                                                           | ((hal->rx_fifo_level & 0x3) << 6));
        HAL_UART_IMPL_USDELAY(2);
        timeout++;
    }

    if (timeout == 2000)
        return HAL_UART_ERR_TIMEOUT;

    return HAL_UART_ERR_SUCCESS;
}

static u8 hal_uart_reset(struct uart_hal *hal)
{
#ifdef SSTAR_URDMA_ENABLE
    u32 i;

    if (hal->urdma_en)
    {
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_DMA_MODE, HAL_URDMA_DMA_MODE);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_TX_DMA_EN);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_RX_DMA_EN);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_TX_INTR_CLR, HAL_URDMA_TX_INTR_CLR);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_RX_INTR_CLR, HAL_URDMA_RX_INTR_CLR);

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_SW_RST, HAL_URDMA_SW_RST);

        for (i = 0; ((HAL_URDMA_RX_BUSY | HAL_URDMA_TX_BUSY) & HAL_UART_READ_WORD(HAL_URDMA_REG_CTRL(hal))); i++)
        {
            if (0xFFFF == i)
            {
                return HAL_UART_ERR_FAILURE;
            }
        }

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_SW_RST);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_DMA_MODE);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_DMA_MODE, HAL_URDMA_DMA_MODE);

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_TX_DMA_EN);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_TX_SW_RST, HAL_URDMA_TX_SW_RST);

        for (i = 0; (HAL_URDMA_TX_BUSY & HAL_UART_READ_WORD(HAL_URDMA_REG_CTRL(hal))); i++)
        {
            if (0xFFFF == i)
            {
                return HAL_UART_ERR_FAILURE;
            }
        }

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_TX_SW_RST);

        hal->tx_first = 1;

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_RX_DMA_EN);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_RX_SW_RST, HAL_URDMA_RX_SW_RST);

        for (i = 0; (HAL_URDMA_RX_BUSY & HAL_UART_READ_WORD(HAL_URDMA_REG_CTRL(hal))); i++)
        {
            if (0xFFFF == i)
            {
                return HAL_UART_ERR_FAILURE;
            }
        }

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_RX_SW_RST);

        hal->rx_sw_rptr = 0;

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_TX_DMA_EN, HAL_URDMA_TX_DMA_EN);
        HAL_UART_WRITE_BYTE_MASK(HAL_URDMA_REG_CTRL(hal), HAL_URDMA_RX_DMA_EN, HAL_URDMA_RX_DMA_EN);

        hal->tx_empty = 1;
    }
#endif // #ifdef SSTAR_URDMA_ENABLE
    return HAL_UART_ERR_SUCCESS;
}

u32 hal_uart_circ_empty(u16 wptr, u16 rptr)
{
    return (wptr == rptr) ? 1 : 0;
}

u32 hal_uart_circ_cnt_to_end(u16 wptr, u16 rptr, u32 size)
{
    u32 end = size - rptr;
    u32 n   = (wptr + end) & (size - 1);

    return ((n < end) ? n : end);
}

u32 hal_uart_circ_space_end(u16 wptr, u16 rptr, u32 size)
{
    u32 end = size - 1 - wptr;
    u32 n   = (end + rptr) & (size - 1);

    return (n <= end ? n : end + 1);
}

void hal_uart_set_digmux(struct uart_hal *hal)
{
    u8 i;

    for (i = 0; i < (sizeof(hal_uart_bank) / sizeof(u16)); i++)
    {
        if (hal_uart_bank[i] == ((hal->uart_base & 0xFFFF00) >> 9)
            && (hal->digmux < (sizeof(hal_uart_bank) / sizeof(u16))))
        {
            break;
        }
        else if (hal_uart_bank[i] == ((hal->uart_base & 0xFFFF00) >> 9) && (hal->digmux == 0xFF))
        {
            hal->digmux = i;
            break;
        }
    }

    if (i != (sizeof(hal_uart_bank) / sizeof(u16)))
    {
        HAL_UART_WRITE_WORD_MASK(hal_uart_digmux_reg[hal->digmux],
                                 hal_uart_digmux_sel[i] << hal_uart_digmux_shift[hal->digmux],
                                 hal_uart_digmux_mask[hal->digmux]);
    }
}

void hal_uart_init(struct uart_hal *hal)
{
#ifdef SSTAR_URDMA_ENABLE
    if (hal->urdma_en)
    {
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_DMA_MODE);
    }
#endif // #ifdef SSTAR_URDMA_ENABLE

    hal_uart_force_rx_disable(hal, 0);
    hal_uart_clear_fifos(hal);

#ifdef SSTAR_URDMA_ENABLE
    if (hal->urdma_en)
    {
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_TXBUF_H(hal), (hal->tx_urdma_base >> 16) & 0xFFFF);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_TXBUF_L(hal), hal->tx_urdma_base & 0xFFFF);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_TXBUF_MSB(hal), (hal->tx_urdma_base >> 32) & 0xF);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_TXBUF_SIZE(hal), hal->tx_urdma_size >> 3);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_TX_TIMEOUT(hal), HAL_URDMA_TX_TIMEOUT);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_TXBUF_WPTR(hal), 0x0);

        HAL_UART_WRITE_WORD(HAL_URDMA_REG_RXBUF_H(hal), (hal->rx_urdma_base >> 16) & 0xFFFF);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_RXBUF_L(hal), hal->rx_urdma_base & 0xFFFF);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_RXBUF_MSB(hal), (hal->rx_urdma_base >> 32) & 0xF);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_RXBUF_SIZE(hal), hal->rx_urdma_size >> 3);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_RX_TIMEOUT(hal), HAL_URDMA_RX_TIMEOUT);
        HAL_UART_WRITE_WORD(HAL_URDMA_REG_INTR_THRESHOLD(hal), HAL_URDMA_RX_INTR_LEVEL);
    }
#endif // #ifdef SSTAR_URDMA_ENABLE

    hal_uart_reset(hal);
    hal_uart_force_rx_disable(hal, 1);
}

void hal_uart_deinit(struct uart_hal *hal)
{
#ifdef SSTAR_URDMA_ENABLE
    u32 i;
#endif
    hal_uart_force_rx_disable(hal, 0);

#ifdef SSTAR_URDMA_ENABLE
    if (hal->urdma_en)
    {
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_RX_DMA_EN);

        for (i = 0; (HAL_URDMA_RX_BUSY & HAL_UART_READ_WORD(HAL_URDMA_REG_CTRL(hal))); i++)
        {
            if (0xFFFF == i)
            {
                return;
            }
        }

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_TX_DMA_EN);

        for (i = 0; (HAL_URDMA_TX_BUSY & HAL_UART_READ_WORD(HAL_URDMA_REG_CTRL(hal))); i++)
        {
            if (0xFFFF == i)
            {
                return;
            }
        }

        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_DMA_MODE);
    }
#endif // #ifdef SSTAR_URDMA_ENABLE
}

void hal_uart_config(struct uart_hal *hal)
{
    u8 uart_flag = 0;

    // Configure Chararacter Size
    switch (hal->char_bits)
    {
        case 5: // Word length is 5 bits
            uart_flag |= HAL_UART_LCR_WLEN5;
            break;
        case 6: // Word length is 6 bits
            uart_flag |= HAL_UART_LCR_WLEN6;
            break;
        case 7: // Word length is 7 bits
            uart_flag |= HAL_UART_LCR_WLEN7;
            break;
        case 8: // Word length is 8 bits
            uart_flag |= HAL_UART_LCR_WLEN8;
            break;
        default:
            break;
    }

    // Configure Stop bit
    if (hal->stop_bits == 2)
    {
        // Stop length is 1.5 bits (5-bit char), 2 bits (otherwise)
        uart_flag |= HAL_UART_LCR_STOP2;
    }
    else
    {
        // Stop length is 1 bit
        uart_flag |= HAL_UART_LCR_STOP1;
    }

    // Configure Parity
    if (hal->parity_en)
    {
        // Parity Enable
        uart_flag |= HAL_UART_LCR_PARITY_EN;

        // Set Odd/Even Parity
        if (!hal->even_parity_sel)
        {
            // Odd Parity
            uart_flag &= (~HAL_UART_LCR_PARITY_SEL);
        }
        else
        {
            // Even Parity
            uart_flag |= HAL_UART_LCR_PARITY_SEL;
        }
    }
    else
    {
        // Parity Disable
        uart_flag &= (~HAL_UART_LCR_PARITY_EN);
    }

#ifdef SSTAR_URDMA_ENABLE
    if (hal->urdma_en)
    {
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_CTRL(hal), 0, HAL_URDMA_DMA_MODE);
    }
#endif

    hal_uart_force_rx_disable(hal, 0);
    // HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_MCR(hal), HAL_UART_MCR_LOOPBACK, HAL_UART_MCR_LOOPBACK);
    hal_uart_clear_fifos(hal);

    HAL_UART_WRITE_BYTE(HAL_UART_REG_LCR(hal), uart_flag);

    if (hal->divisor)
    {
        HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_LCR(hal), HAL_UART_LCR_DLAB, HAL_UART_LCR_DLAB);
        HAL_UART_WRITE_BYTE(HAL_UART_REG_DLH_IER(hal),
                            (u8)((hal->divisor >> 8) & 0xff)); // CAUTION: this causes IER being overwritten also
        HAL_UART_WRITE_BYTE(HAL_UART_REG_DLL_THR_RBR(hal), (u8)(hal->divisor & 0xff));
        HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_LCR(hal), 0, HAL_UART_LCR_DLAB);
    }

    HAL_UART_WRITE_BYTE(HAL_UART_REG_IIR_FCR(hal), HAL_UART_FCR_FIFO_ENABLE | ((hal->tx_fifo_level & 0x3) << 4)
                                                       | ((hal->rx_fifo_level & 0x3) << 6));

    if (hal->rtscts_en)
    {
        // rts cts enable
        HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_MCR(hal), HAL_UART_MCR_AFCE | HAL_UART_MCR_RTS,
                                 HAL_UART_MCR_AFCE | HAL_UART_MCR_RTS);
    }
    else
    {
        // rts cts disable
        HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_MCR(hal), 0, HAL_UART_MCR_AFCE | HAL_UART_MCR_RTS);
    }

    // HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_MCR(hal), 0, HAL_UART_MCR_LOOPBACK);
    hal_uart_reset(hal);
    hal_uart_force_rx_disable(hal, 1);
}

void hal_uart_irq_enable(struct uart_hal *hal, enum HAL_UART_IRQ_TYPE type, u8 u8_enable)
{
#ifdef SSTAR_URDMA_ENABLE
    if (hal->urdma_en && (type == HAL_UART_IRQ_URDMA_TX))
    {
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_INT_CTRL(hal), u8_enable ? HAL_URDMA_TX_INTR_EN : 0,
                                 HAL_URDMA_TX_INTR_EN);
    }
    else if (hal->urdma_en && (type == HAL_UART_IRQ_URDMA_RX))
    {
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_INT_CTRL(hal), u8_enable ? HAL_URDMA_RX_INTR1_EN : 0,
                                 HAL_URDMA_RX_INTR1_EN);
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_INT_CTRL(hal), u8_enable ? HAL_URDMA_RX_INTR2_EN : 0,
                                 HAL_URDMA_RX_INTR2_EN);
    }
#if HAL_UART_TX_EMPTY_INTERRUPT
    else if (hal->urdma_base && (type == HAL_UART_IRQ_TX_EMPTY))
    {
        HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_INT_CTRL(hal), u8_enable ? HAL_URDMA_TX_EMPTY_INT_EN : 0,
                                 HAL_URDMA_TX_EMPTY_INT_EN);
    }
#endif // #if HAL_UART_TX_EMPTY_INTERRUPT
    else if (!hal->urdma_en && (type == HAL_UART_IRQ_TX))
#else
    if (!hal->urdma_en && (type == HAL_UART_IRQ_TX))
#endif
    {
        if (u8_enable)
        {
            HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_DLH_IER(hal), HAL_UART_IER_THRI | HAL_UART_IER_PTHRI,
                                     HAL_UART_IER_THRI | HAL_UART_IER_PTHRI);
        }
        else
        {
            HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_DLH_IER(hal), 0, HAL_UART_IER_THRI | HAL_UART_IER_PTHRI);
        }

        hal->tx_int_en = u8_enable;
    }
    else if (!hal->urdma_en && (type == HAL_UART_IRQ_RX))
    {
        if (u8_enable)
        {
            HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_DLH_IER(hal), HAL_UART_IER_RDI | HAL_UART_IER_RLSI,
                                     HAL_UART_IER_RDI | HAL_UART_IER_RLSI);
        }
        else
        {
            HAL_UART_WRITE_BYTE_MASK(HAL_UART_REG_DLH_IER(hal), 0, HAL_UART_IER_RDI | HAL_UART_IER_RLSI);
        }
    }
}

u32 hal_uart_get_irq_type(struct uart_hal *hal, enum HAL_UART_IRQ_TYPE type)
{
    u8  iir_fcr  = 0; /* Interrupt Identification Register (IIR) */
    u8  count    = 0;
    u8  retry    = 100;
    u32 irq_type = 0;

#ifdef SSTAR_URDMA_ENABLE
    if (hal->urdma_en && (type == HAL_UART_IRQ_URDMA))
    {
        if (HAL_URDMA_RX_MCU_INTR & HAL_UART_READ_WORD(HAL_URDMA_REG_INT_CTRL(hal)))
        {
            if (HAL_URDMA_RX_INTR1 & HAL_UART_READ_WORD(HAL_URDMA_REG_INT_CTRL(hal)))
            {
                irq_type |= HAL_UART_IRQ_URDMA_RX_TIMEOUT;
            }

            if (HAL_URDMA_RX_INTR2 & HAL_UART_READ_WORD(HAL_URDMA_REG_INT_CTRL(hal)))
            {
                irq_type |= HAL_UART_IRQ_URDMA_RX_THRESHOLD;
            }

            HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_INT_CTRL(hal), HAL_URDMA_RX_INTR_CLR, HAL_URDMA_RX_INTR_CLR);
            irq_type |= HAL_UART_IRQ_URDMA_RX;
        }
        else if (HAL_URDMA_TX_MCU_INTR & HAL_UART_READ_WORD(HAL_URDMA_REG_INT_CTRL(hal)))
        {
            HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_INT_CTRL(hal), HAL_URDMA_TX_INTR_CLR, HAL_URDMA_TX_INTR_CLR);
            irq_type |= HAL_UART_IRQ_URDMA_TX;
        }
    }
#if HAL_UART_TX_EMPTY_INTERRUPT
    else if (hal->urdma_base && (type & HAL_UART_IRQ_TX_EMPTY))
    {
        if (HAL_URDMA_TX_EMPTY_INT & HAL_UART_READ_WORD(HAL_URDMA_REG_INT_CTRL(hal)))
        {
            HAL_UART_WRITE_WORD_MASK(HAL_URDMA_REG_INT_CTRL(hal), HAL_URDMA_TX_EMPTY_INT_CLR,
                                     HAL_URDMA_TX_EMPTY_INT_CLR);
            irq_type      = HAL_UART_IRQ_TX_EMPTY_INT;
            hal->tx_empty = 1;
        }
    }
#endif // #if HAL_UART_TX_EMPTY_INTERRUPT

    else if (!hal->urdma_en && (type == HAL_UART_IRQ))
#else  // #ifdef SSTAR_URDMA_ENABLE
    if (!hal->urdma_en && (type == HAL_UART_IRQ))
#endif // #ifdef SSTAR_URDMA_ENABLE
    {
        /* Read Interrupt Identification Register */
        iir_fcr = HAL_UART_READ_BYTE(HAL_UART_REG_IIR_FCR(hal)) & HAL_UART_IIR_ID_MASK;

        irq_type = HAL_UART_IRQ_UNKNOWN;

        if ((iir_fcr == HAL_UART_IIR_RDI || iir_fcr == HAL_UART_IIR_RX_TIMEOUT
             || iir_fcr == HAL_UART_IIR_RLSI)) /* Receive Data Available or Character timeout or Receiver line status */
        {
            irq_type = HAL_UART_IRQ_RX;
        }
        else if (iir_fcr == HAL_UART_IIR_THRI) /* Transmitter Holding Register Empty */
        {
            irq_type = HAL_UART_IRQ_TX;
        }
        else if (iir_fcr == HAL_UART_IIR_MSI) /* Modem Status */
        {
            // UART_ERR("UART Interrupt: Modem status\n");
            // Read MSR to clear
            HAL_UART_READ_BYTE(HAL_UART_REG_MSR(hal));
            irq_type = HAL_UART_IRQ_MODEM;
        }
        else if (iir_fcr == HAL_UART_IIR_BUSY) /* Busy detect indication */
        {
            // Read USR to clear
            HAL_UART_READ_BYTE(HAL_UART_REG_USR(hal));

            while (((HAL_UART_READ_BYTE(HAL_UART_REG_IIR_FCR(hal)) & HAL_UART_IIR_ID_MASK) == HAL_UART_IIR_BUSY)
                   && (count < retry))
            {
                // Read USR to clear
                HAL_UART_READ_BYTE(HAL_UART_REG_USR(hal));
                count++;
            }
            if (count == retry)
                irq_type = HAL_UART_IRQ_BUSY;
        }
        else if (iir_fcr == HAL_UART_IIR_NO_INT) /* No pending interrupts */
        {
            irq_type = HAL_UART_IRQ_NO_INT;
        }
    }

    return irq_type;
}

u32 hal_uart_get_status(struct uart_hal *hal)
{
    u8  lsr    = 0;
    u32 status = 0;

    if (!hal->urdma_en)
    {
        lsr = HAL_UART_READ_BYTE(HAL_UART_REG_LSR(hal));

        if (lsr & HAL_UART_LSR_DR)
        {
            status |= HAL_UART_FIFO_RX_READY;
        }

        if (lsr & HAL_UART_LSR_OE)
        {
            status |= HAL_UART_FIFO_RX_OE;
        }

        if (lsr & HAL_UART_LSR_PE)
        {
            status |= HAL_UART_FIFO_RX_PE;
        }

        if (lsr & HAL_UART_LSR_FE)
        {
            status |= HAL_UART_FIFO_RX_FE;
        }

        if (lsr & HAL_UART_LSR_BI)
        {
            status |= HAL_UART_FIFO_BI;
        }

        if (lsr & HAL_UART_LSR_TEMT)
        {
            status |= HAL_UART_FIFO_TX_SHIFT_EMPTY;
        }

        if (lsr & HAL_UART_LSR_THRE)
        {
            status |= HAL_UART_FIFO_TX_EMPTY;
        }

        if (lsr & HAL_UART_LSR_ERROR)
        {
            status |= HAL_UART_FIFO_RX_ERROR;
        }
    }

#ifdef SSTAR_URDMA_ENABLE
    else
    {
#if HAL_UART_TX_EMPTY_INTERRUPT
        if (hal->tx_empty
            && (HAL_UART_READ_WORD(HAL_URDMA_REG_TXBUF_RPTR(hal)) == HAL_UART_READ_WORD(HAL_URDMA_REG_TXBUF_WPTR(hal))))
        {
            status |= HAL_UART_URDMA_TX_EMPTY;
        }
#else
        if (HAL_UART_READ_WORD(HAL_URDMA_REG_TXBUF_RPTR(hal)) == HAL_UART_READ_WORD(HAL_URDMA_REG_TXBUF_WPTR(hal)))
        {
            status |= HAL_UART_URDMA_TX_EMPTY;
        }
#endif
    }
#endif
    return status;
}

u32 hal_uart_write(struct uart_hal *hal, u8 *buf, u32 size)
{
    u32 count = 0;

#ifdef SSTAR_URDMA_ENABLE
    u32 space      = 0;
    u16 sw_tx_wptr = 0;
    u16 sw_tx_rptr = 0;

    if (hal->urdma_en)
    {
        sw_tx_wptr = HAL_UART_READ_WORD(HAL_URDMA_REG_TXBUF_WPTR(hal));
        sw_tx_rptr = HAL_UART_READ_WORD(HAL_URDMA_REG_TXBUF_RPTR(hal));

        if (sw_tx_wptr == 0 && sw_tx_rptr == 0 && hal->tx_first)
        {
            hal->tx_first = 0;
        }
        else
        {
            sw_tx_wptr = (sw_tx_wptr + 1) & (hal->tx_urdma_size - 1);
        }

        do
        {
            space = hal_uart_circ_space_end(sw_tx_wptr, sw_tx_rptr, hal->tx_urdma_size);

            if (size < space)
                space = size;

            if (space <= 0)
                break;

            memcpy(hal->tx_buf + sw_tx_wptr, buf, space);
            sw_tx_wptr = (sw_tx_wptr + space) & (hal->tx_urdma_size - 1);
            buf += space;
            size -= space;
            count += space;
        } while (size);

        if (count)
        {
            uart_mem_flush((void *)hal->tx_buf, hal->tx_urdma_size);
            uart_miu_pipe_flush();
            HAL_UART_WRITE_WORD(HAL_URDMA_REG_TXBUF_WPTR(hal), (sw_tx_wptr - 1) & (hal->tx_urdma_size - 1));
#if HAL_UART_TX_EMPTY_INTERRUPT
            hal->tx_empty = 0;
#endif
        }
    }
    else
#endif
    {
        do
        {
            if ((HAL_UART_READ_BYTE(HAL_UART_REG_USR(hal)) & HAL_UART_USR_TXFIFO_NOT_FULL)
                != HAL_UART_USR_TXFIFO_NOT_FULL)
            {
                break;
            }

            HAL_UART_WRITE_BYTE(HAL_UART_REG_DLL_THR_RBR(hal), *buf);
            count++;
            buf++;
        } while (--size);
    }

    return count;
}

u32 hal_uart_read(struct uart_hal *hal, u8 *buf, u32 size)
{
    u32 count = 0;

#ifdef SSTAR_URDMA_ENABLE
    u32 cnt        = 0;
    u16 sw_rx_wptr = 0;
    u16 sw_rx_rptr = 0;
    if (hal->urdma_en)
    {
        hal->rx_buf_wptr = sw_rx_wptr =
            (HAL_UART_READ_WORD(HAL_URDMA_REG_RXBUF_WPTR(hal)) + 1) & (hal->tx_urdma_size - 1);
        hal->rx_buf_rptr = sw_rx_rptr = hal->rx_sw_rptr;
        hal->rx_sw_rptr               = sw_rx_wptr;

        uart_mem_invalidate((void *)hal->rx_buf, hal->tx_urdma_size);

        while (buf)
        {
            cnt = hal_uart_circ_cnt_to_end(sw_rx_wptr, sw_rx_rptr, hal->tx_urdma_size);

            if (size < cnt)
                cnt = size;

            if (cnt <= 0)
                break;

            memcpy(buf, hal->rx_buf + sw_rx_rptr, cnt);
            hal->rx_sw_rptr = sw_rx_rptr = (sw_rx_rptr + cnt) & (hal->tx_urdma_size - 1);
            buf += cnt;
            size -= cnt;
            count += cnt;
        }
    }
    else
#endif // #ifdef SSTAR_URDMA_ENABLE
    {
        do
        {
            // check if Receiver Data Ready
            if ((HAL_UART_READ_BYTE(HAL_UART_REG_LSR(hal)) & HAL_UART_LSR_DR) != HAL_UART_LSR_DR)
            {
                break;
            }

            *buf = HAL_UART_READ_BYTE(HAL_UART_REG_DLL_THR_RBR(hal));
            count++;
            buf++;
        } while (--size);
    }

    return count;
}
