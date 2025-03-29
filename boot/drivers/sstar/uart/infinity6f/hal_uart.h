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

#include <asm/arch/mach/platform.h>
#if defined(CONFIG_PADMUX_SUPPORT) && defined(CONFIG_GPIO_SUPPORT)
#include <drv_gpio.h>
#endif

#include "os_uart.h"

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

/* ===== chip diff ======= */
#define HAL_UART_BANK                                                                                               \
    {                                                                                                               \
        0, HAL_UART_FUART_BANK, HAL_UART_UART0_BANK, HAL_UART_UART1_BANK, HAL_UART_UART2_BANK, HAL_UART_UART3_BANK, \
            HAL_UART_UART4_BANK, HAL_UART_UART5_BANK, HAL_UART_UART6_BANK, HAL_UART_UART7_BANK,                     \
    }
#define HAL_UART_DIGMUX_REG                                                                            \
    {                                                                                                  \
        HAL_UART_REG_SEL0, HAL_UART_REG_SEL1, HAL_UART_REG_SEL2, HAL_UART_REG_SEL3, HAL_UART_REG_SEL4, \
            HAL_UART_REG_SEL5, HAL_UART_REG_SEL6, HAL_UART_REG_SEL7, HAL_UART_REG_SEL8                 \
    }
#define HAL_UART_DIGMUX_SHIFT                                                                                    \
    {                                                                                                            \
        HAL_UART_SEL0_SHIFT, HAL_UART_SEL1_SHIFT, HAL_UART_SEL2_SHIFT, HAL_UART_SEL3_SHIFT, HAL_UART_SEL4_SHIFT, \
            HAL_UART_SEL5_SHIFT, HAL_UART_SEL6_SHIFT, HAL_UART_SEL7_SHIFT, HAL_UART_SEL8_SHIFT                   \
    }
#define HAL_UART_DIGMUX_MASK                                                                                \
    {                                                                                                       \
        HAL_UART_SEL0_MASK, HAL_UART_SEL1_MASK, HAL_UART_SEL2_MASK, HAL_UART_SEL3_MASK, HAL_UART_SEL4_MASK, \
            HAL_UART_SEL5_MASK, HAL_UART_SEL6_MASK, HAL_UART_SEL7_MASK, HAL_UART_SEL8_MASK                  \
    }
#define HAL_UART_SELECT_NUM 9
#define HAL_UART_DIGMUX_SEL \
    {                       \
        0, 1, 2, 3, 4, 5, 6 \
    }

// uart prot number
#define HAL_UART_NR_PORTS 11
// uart bank
#define HAL_UART_FUART_BANK   (0x1102)
#define HAL_UART_UART0_BANK   (0x1108)
#define HAL_UART_UART1_BANK   (0x1109)
#define HAL_UART_UART2_BANK   (0x110A)
#define HAL_UART_UART3_BANK   (0x110B)
#define HAL_UART_UART4_BANK   (0x110C)
#define HAL_UART_UART5_BANK   (0x110D)
#define HAL_UART_UART6_BANK   (0x111C)
#define HAL_UART_UART7_BANK   (0x111D)
#define HAL_UART_PM_UART_BANK (0x0035)
// uart digmux select
#define HAL_UART_REG_SEL0 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL1 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL2 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL3 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x53 << 2))
#define HAL_UART_REG_SEL4 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x54 << 2))
#define HAL_UART_REG_SEL5 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x54 << 2))
#define HAL_UART_REG_SEL6 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x59 << 2))
#define HAL_UART_REG_SEL7 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x59 << 2))
#define HAL_UART_REG_SEL8 ((UART_BASE_RIU_PA) + (0x101E << 9) + (0x59 << 2))

#define HAL_UART_SEL0_SHIFT (0)
#define HAL_UART_SEL1_SHIFT (4)
#define HAL_UART_SEL2_SHIFT (8)
#define HAL_UART_SEL3_SHIFT (12)
#define HAL_UART_SEL4_SHIFT (0)
#define HAL_UART_SEL5_SHIFT (4)
#define HAL_UART_SEL6_SHIFT (0)
#define HAL_UART_SEL7_SHIFT (4)
#define HAL_UART_SEL8_SHIFT (8)

#define HAL_UART_SEL0_MASK (0xF << HAL_UART_SEL0_SHIFT)
#define HAL_UART_SEL1_MASK (0xF << HAL_UART_SEL1_SHIFT)
#define HAL_UART_SEL2_MASK (0xF << HAL_UART_SEL2_SHIFT)
#define HAL_UART_SEL3_MASK (0xF << HAL_UART_SEL3_SHIFT)
#define HAL_UART_SEL4_MASK (0xF << HAL_UART_SEL4_SHIFT)
#define HAL_UART_SEL5_MASK (0xF << HAL_UART_SEL5_SHIFT)
#define HAL_UART_SEL6_MASK (0xF << HAL_UART_SEL6_SHIFT)
#define HAL_UART_SEL7_MASK (0xF << HAL_UART_SEL7_SHIFT)
#define HAL_UART_SEL8_MASK (0xF << HAL_UART_SEL8_SHIFT)
// force_rx_disable
#define HAL_UART_REG_FORCE_RX_DISABLE (UART_BASE_RIU_PA + (0x101E << 9) + (0x57 << 2))
// tx interrupt exception
#define HAL_UART_8250_BUG_THRE 1
// support uart tx empty interrupt
#define HAL_UART_TX_EMPTY_INTERRUPT 0

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
    HAL_UART_ERR_FAILURE
};

struct uart_hal
{
    /* software flag */
    u8 urdma_en;
    u8 tx_int_en;
    u8 tx_empty;

    /* gpio disable rx */
    u8 rx_pin;

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

#ifdef SSTAR_URDMA_ENABLE
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
    uart_addr_t urdma_base;
#endif
    uart_addr_t uart_base;
};

u32 hal_uart_circ_empty(u16 wptr, u16 rptr);
u32 hal_uart_circ_cnt_to_end(u16 wptr, u16 rptr, u32 size);
u32 hal_uart_circ_space_end(u16 wptr, u16 rptr, u32 size);

void hal_uart_set_digmux(struct uart_hal *hal);
void hal_uart_init(struct uart_hal *hal);
void hal_uart_deinit(struct uart_hal *hal);
void hal_uart_config(struct uart_hal *hal);
void hal_uart_irq_enable(struct uart_hal *hal, enum HAL_UART_IRQ_TYPE type, u8 enable);
u32  hal_uart_get_irq_type(struct uart_hal *hal, enum HAL_UART_IRQ_TYPE type);
u32  hal_uart_get_status(struct uart_hal *hal);
u32  hal_uart_write(struct uart_hal *hal, u8 *buf, u32 size);
u32  hal_uart_read(struct uart_hal *hal, u8 *buf, u32 size);

#endif /* _HAL_UART_H_ */
