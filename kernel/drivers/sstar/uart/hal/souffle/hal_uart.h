/*
 * hal_uart.h- Sigmastar
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

#ifndef _HAL_UART_H_
#define _HAL_UART_H_

#include <ms_platform.h>
#include <asm/pgtable.h>
#include <cam_os_wrapper.h>
#include <drv_gpio.h>

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

#define HAL_URDMA_TXBUF_LENGTH  0x1000 // must be 8 byte aligned, linux should better be PAGE_ALIGN
#define HAL_URDMA_RXBUF_LENGTH  0x1000 // must be 8 byte aligned, linux should better be PAGE_ALIGN
#define HAL_URDMA_RX_INTR_LEVEL 0x500  // need to think
#define HAL_URDMA_RX_TIMEOUT    0x0F
#define HAL_URDMA_TX_TIMEOUT    0x0F

/* ===== os diff ======= */
#define HAL_UART_IMPL_USDELAY(x) CamOsUsDelay(x)

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

#define HAL_UART_MCR_DTR       0x01
#define HAL_UART_MCR_RTS       0x02
#define HAL_UART_MCR_OUT1      0x04
#define HAL_UART_MCR_OUT2      0x08
#define HAL_UART_MCR_LOOPBACK  0x10
#define HAL_UART_MCR_AFCE      0x20
#define HAL_UART_MCR_CTRL_MASK 0x1F

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

// uart prot number
#define HAL_UART_NR_PORTS 5
// uart bank
#define HAL_UART_FUART_BANK   (0x1102)
#define HAL_UART_UART0_BANK   (0x1108)
#define HAL_UART_UART1_BANK   (0x1109)
#define HAL_UART_UART2_BANK   (0x110A)
#define HAL_UART_UART3_BANK   (0x110B)
#define HAL_UART_UART4_BANK   (0x110C)
#define HAL_UART_PM_UART_BANK (0x0035)
// uart digmux select
#define HAL_UART_REG_SEL0   ((0x1F000000) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL1   ((0x1F000000) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL2   ((0x1F000000) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL3   ((0x1F000000) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL4   ((0x1F000000) + (0x101E << 9) + (0x54 << 2))
#define HAL_UART_REG_SEL5   ((0x1F000000) + (0x101E << 9) + (0x54 << 2))
#define HAL_UART_REG_SEL6   ((0x1F000000) + (0x101E << 9) + (0x59 << 2))
#define HAL_UART_SEL0_SHIFT (0)
#define HAL_UART_SEL1_SHIFT (4)
#define HAL_UART_SEL2_SHIFT (8)
#define HAL_UART_SEL3_SHIFT (12)
#define HAL_UART_SEL4_SHIFT (0)
#define HAL_UART_SEL5_SHIFT (4)
#define HAL_UART_SEL6_SHIFT (0)
#define HAL_UART_SEL0_MASK  (0xF << HAL_UART_SEL0_SHIFT)
#define HAL_UART_SEL1_MASK  (0xF << HAL_UART_SEL1_SHIFT)
#define HAL_UART_SEL2_MASK  (0xF << HAL_UART_SEL2_SHIFT)
#define HAL_UART_SEL3_MASK  (0xF << HAL_UART_SEL3_SHIFT)
#define HAL_UART_SEL4_MASK  (0xF << HAL_UART_SEL4_SHIFT)
#define HAL_UART_SEL5_MASK  (0xF << HAL_UART_SEL5_SHIFT)
#define HAL_UART_SEL6_MASK  (0xF << HAL_UART_SEL6_SHIFT)
// force_rx_disable
#define HAL_UART_REG_FORCE_RX_DISABLE ((0x1F000000) + (0x101E << 9) + (0x57 << 2))
// tx interrupt exception
#define HAL_UART_8250_BUG_THRE 1
// support uart tx empty interrupt
#define HAL_UART_TX_EMPTY_INTERRUPT 1

enum HAL_UART_IRQ_TYPE
{
    HAL_UART_IRQ_UNKNOWN            = 0x00000000,
    HAL_UART_IRQ_RX                 = 0x00000001,
    HAL_UART_IRQ_TX                 = 0x00000002,
    HAL_UART_IRQ_MODEM              = 0x00000004,
    HAL_UART_IRQ_BUSY               = 0x00000008,
    HAL_UART_IRQ_NO_INT             = 0x00000010,
    HAL_UART_IRQ                    = 0x000000FF,
    HAL_UART_IRQ_URDMA_RX           = 0x00000100,
    HAL_UART_IRQ_URDMA_TX           = 0x00000200,
    HAL_UART_IRQ_URDMA_RX_TIMEOUT   = 0x00000400,
    HAL_UART_IRQ_URDMA_RX_THRESHOLD = 0x00000800,
    HAL_UART_IRQ_URDMA              = 0x0000FF00,
    HAL_UART_IRQ_TX_EMPTY_INT       = 0x00010000,
    HAL_UART_IRQ_TX_EMPTY           = 0x00FF0000,
};

enum HAL_UART_STATUS
{
    HAL_UART_FIFO_RX_READY       = 0x00000001,
    HAL_UART_FIFO_RX_OE          = 0x00000002,
    HAL_UART_FIFO_RX_PE          = 0x00000004,
    HAL_UART_FIFO_RX_FE          = 0x00000008,
    HAL_UART_FIFO_BI             = 0x00000010,
    HAL_UART_FIFO_RX_ERROR       = 0x00000020,
    HAL_UART_FIFO_TX_SHIFT_EMPTY = 0x00000040,
    HAL_UART_FIFO_TX_NOT_FULL    = 0x00000080,
    HAL_UART_FIFO_TX_EMPTY       = 0x00000100,
    HAL_UART_FIFO_RX_NOT_EMPTY   = 0x00000200,
    HAL_UART_FIFO_RX_FULL        = 0x00000400,
    HAL_UART_URDMA_TX_EMPTY      = 0x00000800,
};

enum HAL_UART_ERR
{
    HAL_UART_ERR_SUCCESS = 0x00,
    HAL_UART_ERR_TIMEOUT,
    HAL_UART_ERR_FAILURE,
    HAL_UART_ERR_MCTRL
};

struct uart_hal
{
    /* software flag */
    u8 urdma_en;
    u8 tx_int_en;
    u8 tx_empty;

    /* gpio disable rx */
    u8 rx_pin;

    /* break ctl, used only by urdma mode */
    u8  break_ctl_pad;
    u32 padmux;

    /* hardware configuration */
    u8  digmux;
    u8  char_bits;
    u8  stop_bits;
    u8  parity_en;
    u8  even_parity_sel;
    u8  rtscts_en;
    u8  tx_fifo_level;
    u8  rx_fifo_level;
    u16 divisor;

    /* urdma */
    u16 tx_first;
    u16 rx_sw_rptr;
    u16 rx_buf_wptr;
    u16 rx_buf_rptr;
    u64 rx_urdma_base;
    u64 tx_urdma_base;
    u32 rx_urdma_size;
    u32 tx_urdma_size;
    u8 *rx_buf;
    u8 *tx_buf;

    /* register bank */
    unsigned long uart_base;
    unsigned long urdma_base;
};

u32 hal_uart_circ_empty(u16 wptr, u16 rptr);
u32 hal_uart_circ_cnt_to_end(u16 wptr, u16 rptr, u32 size);
u32 hal_uart_circ_space_end(u16 wptr, u16 rptr, u32 size);

void hal_uart_set_digmux(struct uart_hal *hal);
void hal_uart_init(struct uart_hal *hal);
void hal_uart_deinit(struct uart_hal *hal);
void hal_uart_config(struct uart_hal *hal);
void hal_uart_break(struct uart_hal *hal, u8 ctl);
void hal_uart_irq_enable(struct uart_hal *hal, enum HAL_UART_IRQ_TYPE type, u8 enable);
u32  hal_uart_get_irq_type(struct uart_hal *hal, enum HAL_UART_IRQ_TYPE type);
u32  hal_uart_get_status(struct uart_hal *hal);
u32  hal_uart_write(struct uart_hal *hal, u8 *buf, u32 size);
u32  hal_uart_read(struct uart_hal *hal, u8 *buf, u32 size);
u32  hal_uart_set_mctrl(struct uart_hal *hal, u8 mctrl_set);
u32  hal_uart_get_mctrl(struct uart_hal *hal);

#endif /* _HAL_UART_H_ */
