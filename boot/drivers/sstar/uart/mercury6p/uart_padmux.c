/*
 * uart_padmux.c - Sigmastar
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

/*-----------------------------------------------------------------------------
    Include Files
------------------------------------------------------------------------------*/
#include <common.h>
#include <command.h>
#include "asm/arch/mach/sstar_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include "sstar_serial.h"
#include <linux/compiler.h>
#include <serial.h>

/* Constants of RIU banks */
#define MS_BASE_REG_UART1_PA GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x110900)
#define MS_BASE_REG_UART2_PA GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x110A00)
#define MS_BASE_REG_UART3_PA GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x110B00)
#define MS_BASE_REG_FUART_PA GET_BASE_ADDR_BY_BANK(SSTAR_BASE_REG_RIU_PA, 0x110200)

#define REG_UART0_CLK 0x1F2070C4 /*0x1038, 0x31 BIT[3:0]*/
#define REG_UART1_CLK 0x1F2070C4 /*0x1038, 0x31 BIT[7:4]*/
#define REG_UART2_CLK 0x1F2070C4 /*0x1038, 0x31 BIT[11:8]*/
#define REG_UART3_CLK 0x1F2070C4 /*0x1038, 0x31 BIT[15:12]*/
#define REG_UART4_CLK 0x1F207E04 /*0x103F, 0x01 BIT[3:0]*/
#define REG_UART5_CLK 0x1F207E08 /*0x103F, 0x02 BIT[3:0]*/
#define REG_FUART_CLK 0x1F2070D0 /*0x1038, 0x34 BIT[3:0]*/

#define REG_FUART_SEL 0x1F203D50 /*0x101E, h54*/
#define REG_UART_SEL  0x1F203D4C /*0x101E, h53*/

#define UART_PIU_UART1 3
#define UART_PIU_UART2 4
#define UART_PIU_UART3 5
#define UART_PIU_FUART 1

#define REG_UART1_PADMUX 0x1F2079B4 /*0x103C, 0x6D BIT[6:4]*/
#define REG_UART2_PADMUX 0x1F2079B4 /*0x103C, 0x6D BIT[10:8]*/
#define REG_UART3_PADMUX 0x1F2079B4 /*0x103C, 0x6D BIT[14:12]*/
#define REG_FUART_PADMUX 0x1F2079B8 /*0x103C, 0x6E BIT[11:8]*/

#define CONFIG_UART1_PAD_MODE 6
#define CONFIG_UART2_PAD_MODE 0
#define CONFIG_UART3_PAD_MODE 0
#define CONFIG_FUART_PAD_MODE 3

extern U32 uart_multi_base;

/*
 * port mapping is specified by dts
 *
 * serial0 = &uart0;
 * serial1 = &uart1;
 * serial2 = &fuart;
 * serial3 = &uart2;
 * serial4 = &uart3;
 * serial5 = &uart4;
 * serial6 = &uart5;
 */
#ifndef CONFIG_SSTAR_CLK
U32 sstar_uart_setclk(U8 u8_Port)
{
    switch (u8_Port)
    {
        case 0: /* UART0 */
            OUTREGMSK16(REG_UART0_CLK, 0x00, 0xF);
            break;
        case 1: /* UART1 */
            OUTREGMSK16(REG_UART1_CLK, 0x00 << 4, 0xF << 4);
            break;
        case 2: /* FUART */
            OUTREGMSK16(REG_FUART_CLK, 0x00, 0xFF);
            break;
        case 3: /* UART2 */
            OUTREGMSK16(REG_UART2_CLK, 0x00 << 8, 0xF << 8);
            break;
        case 4: /* UART3 */
            OUTREGMSK16(REG_UART3_CLK, 0x00 << 12, 0xF << 12);
            break;
        case 5: /* UART4 */
            OUTREGMSK16(REG_UART4_CLK, 0x00, 0xF);
            break;
        case 6: /* UART5 */
            OUTREGMSK16(REG_UART5_CLK, 0x00, 0xF);
            break;
        default:
            return 0;
    }
    return 172800000;
}
#endif

U32 sstar_uart_padmux(U8 u8_Port)
{
    U32 uartClk = 172800000;

#ifndef CONFIG_SSTAR_CLK
    uartClk = sstar_uart_setclk(u8_Port);
#endif

    switch (u8_Port)
    {
        case 1:
            uart_multi_base = MS_BASE_REG_UART1_PA;
            /*padmux*/
            OUTREGMSK16(REG_UART1_PADMUX, CONFIG_UART1_PAD_MODE << 4, 0x7 << 4);
            OUTREGMSK16(REG_UART_SEL, UART_PIU_UART1 << 12, 0xF << 12);
            break;
        case 2:
            uart_multi_base = MS_BASE_REG_FUART_PA;
            OUTREGMSK16(REG_FUART_CLK, 0x00 << 4, 0xF << 4);
            /*padmux*/
            OUTREGMSK16(REG_FUART_PADMUX, CONFIG_FUART_PAD_MODE << 8, 0xF << 8);
            OUTREGMSK16(REG_UART_SEL, UART_PIU_FUART << 4, 0xF << 4);
            break;
        case 3:
            uart_multi_base = MS_BASE_REG_UART2_PA;
            /*padmux*/
            OUTREGMSK16(REG_UART2_PADMUX, CONFIG_UART2_PAD_MODE << 8, 0x7 << 8);
            OUTREGMSK16(REG_FUART_SEL, UART_PIU_UART2 << 0, 0xF << 0);
            break;
        case 4:
            uart_multi_base = MS_BASE_REG_UART3_PA;
            OUTREGMSK16(REG_UART3_CLK, 0x00 << 12, 0xF << 12);
            /*padmux*/
            OUTREGMSK16(REG_UART3_PADMUX, CONFIG_UART3_PAD_MODE << 12, 0x7 << 12);
            OUTREGMSK16(REG_FUART_SEL, UART_PIU_UART3 << 4, 0xF << 4);
            break;
        default:
            uart_multi_base = SSTAR_BASE_REG_UART0_PA;
            /*padmux*/
            break;
    }
    return uartClk;
}
