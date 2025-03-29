/*
 * hal_uart_cfg.h- Sigmastar
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

#ifndef _HAL_UART_CFG_
#define _HAL_UART_CFG_

#define HAL_UART0_IO_ADDR     (0x1F221000)
#define HAL_UART1_IO_ADDR     (0x1F221200)
#define HAL_UART2_IO_ADDR     (0x1F221400)
#define HAL_UART3_IO_ADDR     (0x1F221600)
#define HAL_UART4_IO_ADDR     (0x1F221800)
#define HAL_UART5_IO_ADDR     (0x1F221A00)
#define HAL_UART6_IO_ADDR     (0x1F228000)
#define HAL_UART7_IO_ADDR     (0x1F228400)
#define HAL_UART8_IO_ADDR     (0x1F228800)
#define HAL_FUART_IO_ADDR     (0x1F220400)
#define HAL_PM_UART0_IO_ADDR  (0x1F006A00)
#define HAL_PM_FUART_IO_ADDR  (0x1F006C00)
#define HAL_PM_FUART1_IO_ADDR (0x1F00A400)

#define HAL_UART_CHIPTOP_SELECT_NUM (11)

#define HAL_PM_TOP_BANK            0x1e
#define HAL_PM_FUART_MIU_ADDR_SEL  (0x1F000000 + ((HAL_PM_TOP_BANK) << 9) + ((0x5c) << 2))
#define HAL_PM_FUART1_MIU_ADDR_SEL (0x1F000000 + ((HAL_PM_TOP_BANK) << 9) + ((0x5d) << 2))
struct reg_t
{
    unsigned int   bank_base;
    unsigned char  offset;
    unsigned char  bit_shift;
    unsigned short mask;
};

struct reg_t hal_uart_chiptop_select[HAL_UART_CHIPTOP_SELECT_NUM] = {
    // reg_uart_sel0
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x53,
        .bit_shift = 0,
        .mask      = 0x000F,
    },

    // reg_uart_sel1
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x53,
        .bit_shift = 4,
        .mask      = 0x00F0,
    },

    // reg_uart_sel2
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x53,
        .bit_shift = 8,
        .mask      = 0x0F00,
    },

    // ret_uart_sel3
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x53,
        .bit_shift = 12,
        .mask      = 0xF000,
    },

    // reg_uart_sel4
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x54,
        .bit_shift = 0,
        .mask      = 0x000F,
    },

    // reg_uart_sel5
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x54,
        .bit_shift = 4,
        .mask      = 0x00F0,
    },
    // reg_uart_sel6
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x54,
        .bit_shift = 8,
        .mask      = 0x0F00,
    },
    // reg_uart_sel7
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x54,
        .bit_shift = 12,
        .mask      = 0xF000,
    },
    // reg_uart_sel8
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x55,
        .bit_shift = 4,
        .mask      = 0x00F0,
    },
    // reg_uart_sel9
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x55,
        .bit_shift = 8,
        .mask      = 0x0F00,
    },
    // reg_uart_sel10
    {
        .bank_base = 0x1F203C00,
        .offset    = 0x55,
        .bit_shift = 12,
        .mask      = 0xF000,
    },

};

const unsigned int hal_uart_io2sel_val[HAL_UART_CHIPTOP_SELECT_NUM] = {
    ~(unsigned int)(0x0), HAL_FUART_IO_ADDR, HAL_UART0_IO_ADDR, HAL_UART1_IO_ADDR, HAL_UART2_IO_ADDR, HAL_UART3_IO_ADDR,
    HAL_UART4_IO_ADDR,    HAL_UART5_IO_ADDR, HAL_UART6_IO_ADDR, HAL_UART7_IO_ADDR, HAL_UART8_IO_ADDR};

#endif
