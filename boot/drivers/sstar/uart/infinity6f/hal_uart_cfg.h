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

#define PM_SLEEP_BANK_BASE  (0x0e)
#define PM_PADTOP_BANK_BASE (0x3f)

struct clk_info
{
    u32 rate;
    u16 value;
};

struct reg_t
{
    u64 bank_base;
    u8  reg_offset;
    u8  bit_shift;
    u32 bit_mask;
};

struct uart_cfg_t
{
    char uart_name[16];
    u8   port_num;
    u8   enable;

    struct clk_info *src_clk_info;
    struct reg_t *   reg_clkgen;
};

struct clk_info fuart_src_clk[] = {

    [0] =
        {
            .rate  = 172000000,
            .value = 0x00,
        },
    [1] =
        {
            .rate  = 144000000,
            .value = 0x01,
        },
    [2] =
        {
            .rate  = 24000000,
            .value = 0x02,
        },
    [3] =
        {
            .rate  = 216000000,
            .value = 0x03,
        },
};

#define PM_UART_SRC_CLK_NUM (sizeof(pm_uart_src) / sizeof(struct clk_info))

struct reg_t uart0_clkgen = {
    .bank_base  = 0x1f207000,
    .reg_offset = 0x31,
    .bit_shift  = 0x0,
    .bit_mask   = 0x000f,
};

struct reg_t uart1_clkgen = {
    .bank_base  = 0x1f207000,
    .reg_offset = 0x31,
    .bit_shift  = 0x4,
    .bit_mask   = 0x00f0,
};

struct reg_t uart2_clkgen = {
    .bank_base  = 0x1f207000,
    .reg_offset = 0x31,
    .bit_shift  = 0x8,
    .bit_mask   = 0x0f00,
};

struct uart_cfg_t uart_config[] = {
    // uart
    [0] =
        {
            .uart_name    = "uart0",
            .port_num     = 0,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = &uart0_clkgen,
        },

    // uart1
    [1] =
        {
            .uart_name    = "uart1",
            .port_num     = 1,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = &uart1_clkgen,
        },

    // uart2
    [2] =
        {
            .uart_name    = "uart2",
            .port_num     = 2,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = NULL,
        },

    // uart3
    [3] =
        {
            .uart_name    = "uart3",
            .port_num     = 3,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = &uart2_clkgen,
        },

    // uart4
    [4] =
        {
            .uart_name    = "uart4",
            .port_num     = 4,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = NULL,
        },

    // uart5
    [5] =
        {
            .uart_name    = "uart5",
            .port_num     = 5,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = NULL,
        },

    // uart6
    [6] =
        {
            .uart_name    = "uart6",
            .port_num     = 6,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = NULL,
        },

    // uart7
    [7] =
        {
            .uart_name    = "uart7",
            .port_num     = 7,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = NULL,
        },

    // fuart
    [8] =
        {
            .uart_name    = "fuart",
            .port_num     = 8,
            .enable       = 1,
            .src_clk_info = &fuart_src_clk[0],
            .reg_clkgen   = NULL,
        },

};
