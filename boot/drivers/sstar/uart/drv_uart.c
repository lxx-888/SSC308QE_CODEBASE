/*
 * drv_uart.c- Sigmastar
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

#include <stdint.h>
#include <common.h>
#include <command.h>
#include <asm/arch/mach/sstar_types.h>
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/io.h>
#include <linux/compiler.h>
#include <serial.h>
#include <watchdog.h>
#ifdef CONFIG_DM_SERIAL
#include <dm.h>
#ifdef CONFIG_SSTAR_CLK
#include <clk.h>
#endif
#endif

#include "drv_uart.h"
#include "hal_uart.h"
#include "hal_uart_cfg.h"

#define UART_CLK CONFIG_UART_CLOCK
#if !CONFIG_UART_CLOCK
#error unknown UART_CLK
#endif

#ifdef CONFIG_DM_SERIAL

#define UART_REG16_READ(_reg) (*((volatile u16 *)((u32)_reg)))
#define UART_REG16_WRITE(_reg, _val, _mask) \
    *(volatile u16 *)((u32)_reg) = (_val & _mask) | (~_mask & UART_REG16_READ(_reg));

struct uart_control
{
    struct uart_hal    hal;
    struct uart_cfg_t *config_info;
    struct udevice *   udev;

    u32 sctp_en;
    u32 digmux_select;
    u32 tolerance;
    u32 baudrate;
    u32 rx_guard;
    u32 tx_silent;
    u32 tx_disable;
    u32 pad_disable;

    u8  is_console;
    u32 is_rx_blocked;
};

static int drv_uart_getc(struct udevice *udev)
{
    struct uart_control *plat = dev_get_priv(udev);
    u8                   ch;

    while (!hal_uart_read(&plat->hal, (u8 *)&ch, 1))
        ;

    return ch;
}

static int drv_uart_putc(struct udevice *udev, const char ch)
{
    struct uart_control *plat = dev_get_priv(udev);
    while (!hal_uart_write(&plat->hal, (u8 *)&ch, 1))
        ;

    return 0;
}

#define UART_DEFAULT_BAUDRATE  (115200)
#define UART_DEFAULT_CHAR_BITS (8)
#define UART_DEFAULT_STOP_BITS (1)
#define UART_DEFAULT_PARITY_EN (0) // 0-disable; 1-odd; 2-even.
static int drv_uart_of_to_plat(struct udevice *udev)
{
    struct uart_control *plat       = dev_get_priv(udev);
    struct uart_cfg_t *  config_tmp = NULL;
    fdt_addr_t           addr;
    u32                  group, i;
    u8                   cfg_array_size = 0;
    int                  ret;
    u32                  char_bits  = 0;
    u32                  stop_bits  = 0;
    u32                  parity_sel = 0;
    u32                  rts_cts    = 0;
    u32                  tolerance  = 0;

    addr = dev_read_addr(udev);
    if (addr == FDT_ADDR_T_NONE)
    {
        return -EINVAL;
    }
    plat->udev = udev;

    plat->hal.uart_base = (fdt_addr_t)addr;
    plat->hal.urdma_en  = 0;

    *(volatile u16 *)0x1f207980 = 0x0200;
    ret                         = dev_read_u32(udev, "group", &(group));
    if (ret)
    {
        group = 0;
    }

    cfg_array_size = sizeof(uart_config) / sizeof(struct uart_cfg_t);

    for (i = 0; i < cfg_array_size; i++)
    {
        config_tmp = &uart_config[i];
        if ((config_tmp->port_num == group))
        {
            plat->config_info = &uart_config[i];
            break;
        }
    }

    ret = dev_read_u32(udev, "uart-select", &(plat->digmux_select));
    if (ret)
    {
        plat->digmux_select = 0xFF;
    }

    ret = dev_read_u32(udev, "rate", &(plat->baudrate));
    if (ret)
    {
        plat->baudrate = UART_DEFAULT_BAUDRATE;
    }

    ret = dev_read_u32(udev, "char-bits", &(char_bits));
    if (ret)
    {
        plat->hal.char_bits = UART_DEFAULT_CHAR_BITS;
    }
    else
    {
        plat->hal.char_bits = (u8)char_bits;
    }

    ret = dev_read_u32(udev, "stop-bits", &(stop_bits));
    if (ret)
    {
        plat->hal.stop_bits = UART_DEFAULT_STOP_BITS;
    }
    else
    {
        plat->hal.stop_bits = (u8)stop_bits;
    }

    ret = dev_read_u32(udev, "parity-en", &(parity_sel));
    if (ret)
    {
        plat->hal.parity_en = UART_DEFAULT_PARITY_EN;
    }
    else
    {
        plat->hal.parity_en = (u8)parity_sel;
    }

    ret = dev_read_u32(udev, "auto-flow", &(rts_cts));
    if (ret)
    {
        plat->hal.rtscts_en = 0;
    }
    else
    {
        plat->hal.rtscts_en = (u8)rts_cts;
    }

    ret = dev_read_u32(udev, "tolerance", &(tolerance));
    if (ret)
    {
        plat->tolerance = 3;
    }
    else
    {
        plat->tolerance = tolerance;
    }

    return 0;
}

static int drv_uart_clk_calc(struct uart_control *ctrl, u32 rate)
{
    struct uart_cfg_t *config_tmp = NULL;
#ifdef CONFIG_SSTAR_CLK
    struct clk clock;
#endif
    u32             src_rtn, src_rate        = 0;
    u8              i, srcclk_array_size     = 0;
    u32             divisor, closest_divisor = 0xFF;
    u32             real_baud, baudtolerance, baudcalc, min_baudcalc = 0xFFFFFFFF;
    u8              clk_index = 0xFF;
    int             ret;
    struct udevice *udev = ctrl->udev;

    if (!ctrl->config_info)
    {
        src_rate = 172000000;
    }
    else
    {
        config_tmp = ctrl->config_info;
    }

    if (config_tmp)
    {
        srcclk_array_size = sizeof(fuart_src_clk) / sizeof(struct clk_info);
        for (i = 0; i < srcclk_array_size; i++)
        {
            src_rate = fuart_src_clk[i].rate;

            divisor   = (src_rate + (8 * rate)) / (16 * rate);
            real_baud = src_rate / (16 * divisor);

            baudcalc      = abs(real_baud - rate);
            baudtolerance = (rate * ctrl->tolerance) / 100;
            if ((baudcalc < baudtolerance) && (baudcalc < min_baudcalc))
            {
                clk_index       = i;
                min_baudcalc    = baudcalc;
                closest_divisor = divisor;
            }
        }
        if (0xFF == clk_index)
        {
            return -1;
        }
        src_rate = fuart_src_clk[clk_index].rate;
    }
    else
    {
        divisor   = (src_rate + (8 * rate)) / (16 * rate);
        real_baud = src_rate / (16 * divisor);

        baudcalc      = abs(real_baud - rate);
        baudtolerance = (rate * ctrl->tolerance) / 100;
        if (baudcalc < baudtolerance)
        {
            closest_divisor = divisor;
            goto out;
        }
        else
        {
            return -1;
        }
    }

#ifdef CONFIG_SSTAR_CLK
    ret = clk_get_by_index(udev, 0, &clock);

    ret |= clk_enable(&clock);
    if (ret)
    {
        goto clk_fail;
    }

    src_rtn = clk_set_rate(&clock, src_rate);
    if (src_rtn == src_rate)
    {
        goto out;
    }

clk_fail:
#endif

    // set source clock here if clk API fail,because when console init, dm_scan not active ever.
    if (config_tmp->reg_clkgen)
    {
        UART_REG16_WRITE((config_tmp->reg_clkgen->bank_base + (config_tmp->reg_clkgen->reg_offset << 2)),
                         (0 << config_tmp->reg_clkgen->bit_shift), config_tmp->reg_clkgen->bit_mask);
        UART_REG16_WRITE((config_tmp->reg_clkgen->bank_base + (config_tmp->reg_clkgen->reg_offset << 2)),
                         (config_tmp->src_clk_info[clk_index].value << (config_tmp->reg_clkgen->bit_shift + 2)),
                         config_tmp->reg_clkgen->bit_mask);
    }

out:
    return closest_divisor;
}
static int drv_uart_set_termios(struct uart_control *ctrl, struct uart_protocol *protocol)
{
    if (protocol)
    {
        ctrl->hal.char_bits = protocol->bit_length;
        ctrl->hal.stop_bits = protocol->stop;
        ctrl->hal.parity_en = protocol->parity;
        ctrl->hal.rtscts_en = protocol->rtscts;
        ctrl->baudrate      = protocol->rate;
    }
    ctrl->hal.divisor = drv_uart_clk_calc(ctrl, ctrl->baudrate);
    if (-1 == ctrl->hal.divisor)
    {
        return -1;
    }

    hal_uart_config(&ctrl->hal);
    return 0;
}
static int drv_uart_probe(struct udevice *udev)
{
    struct uart_control *plat = dev_get_priv(udev);

    drv_uart_set_termios(plat, NULL);

    hal_uart_init(&plat->hal);

    return 0;
}

static int drv_uart_setbrg(struct udevice *udev, int baudrate)
{
    struct uart_control *plat = dev_get_priv(udev);

    plat->baudrate = baudrate;
    drv_uart_set_termios(plat, NULL);
    return 0;
}

static int drv_uart_pending(struct udevice *udev, bool input)
{
    struct uart_control *plat       = dev_get_priv(udev);
    int                  data_ready = 0;

    if (!plat)
    {
        return 0;
    }

    data_ready = hal_uart_get_status(&(plat->hal));

    if (HAL_UART_FIFO_RX_READY & data_ready)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    return 0;
}

static const struct dm_serial_ops sstar_serial_ops = {
    .setbrg  = drv_uart_setbrg,
    .getc    = drv_uart_getc,
    .putc    = drv_uart_putc,
    .pending = drv_uart_pending,
};

static const struct udevice_id sstar_serial_ids[] = {{.compatible = "sstar,uart"}, {}};

U_BOOT_DRIVER(serial_sstar) = {
    .name       = "serial_sstar",
    .id         = UCLASS_SERIAL,
    .of_match   = sstar_serial_ids,
    .of_to_plat = drv_uart_of_to_plat,
    .priv_auto  = sizeof(struct uart_control),
    .probe      = drv_uart_probe,
    .ops        = &sstar_serial_ops,
    .flags      = DM_FLAG_PRE_RELOC,
};

#else // #ifdef CONFIG_DM_SERIAL

#define UART_CONSOLE_BASE_PA SSTAR_BASE_REG_UART0_PA
#define UART_REG8(_x_)       ((u8 volatile *)(UART_CONSOLE_BASE_PA))[((_x_)*4) - ((_x_)&1)]

#define UART_MULTI_BASE_PA   uart_multi_base
#define UART_MULTI_REG8(_x_) ((u8 volatile *)(UART_MULTI_BASE_PA))[((_x_)*4) - ((_x_)&1)]

#define UART_BAUDRATE_DEFAULT CONFIG_BAUDRATE

#if UART_DBG_MSG
#define uart_dbg_msg(fmt, arg...) printf("uart: " fmt, ##arg)
#else
#define uart_dbg_msg(fmt, arg...)
#endif

ulong uart_multi_base;

#ifndef CONFIG_SSTAR_CLK
U32   drv_uart_setclk(u8 port)
{
    switch (port)
    {
        case 0: /* UART0 */
            OUTREGMSK16(UART_PORT0_CLK_RIU_PA, 0x00, 0xF);
            break;
        case 1: /* UART1 */
            OUTREGMSK16(UART_PORT1_CLK_RIU_PA, 0x00 << 4, 0xF << 4);
            break;
        case 2: /* FUART */
            OUTREGMSK16(UART_FUART_CLK_RIU_PA, 0x00, 0xFF);
            break;
        default:
            return 0;
    }
    return 172800000;
}
#endif

U32 drv_uart_padset(u8 port)
{
    u32 src_clk_freq = 172800000;

#ifndef CONFIG_SSTAR_CLK
    src_clk_freq     = drv_uart_setclk(port);
#endif

    switch (port)
    {
        case 1:
            uart_multi_base = UART_PORT1_RIU_PA;
            OUTREGMSK16(UART_PORT1_CLK_RIU_PA, 0x00 << 4, 0xF << 4);
            /*padmux*/
            OUTREGMSK16(UART_PORT1_PADMUX_PA, UART_PORT1_PADMODE << 4, 0x7 << 4);
            OUTREGMSK16(UART_PAD_SEL0_RIU_PA, UART_PIU_PORT1 << 12, 0xF << 12);
            break;
        case 2:
            uart_multi_base = UART_FUART_RIU_PA;
            OUTREGMSK16(UART_FUART_CLK_RIU_PA, 0x00, 0xF);
            /*padmux*/
            OUTREGMSK16(UART_FUART_PADMUX_PA, UART_PORT4_PADMODE << 8, 0xF << 8);
            OUTREGMSK16(UART_PAD_SEL0_RIU_PA, UART_PIU_FUART << 4, 0xF << 4);
            break;
        default:
            uart_multi_base = SSTAR_BASE_REG_UART0_PA;
            /*padmux*/
            break;
    }
    return src_clk_freq;
}

static int drv_console_init(void)
{
    // i.   Set "reg_mcr_loopback";
    UART_REG8(UART_REG_LCR_MCR) |= 0x10;

    // ii.   Poll "reg_usr_busy" till 0;
    while (UART_REG8(UART_REG_USR) & 0x01)
    {
        UART_REG8(UART_REG_IIR_FCR) =
            (UART_REG8(UART_REG_IIR_FCR) | (UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT));
    }

    UART_REG8(UART_REG_IER_DLH) = 0x00;

    // Reset receiver and transmiter
    UART_REG8(UART_REG_IIR_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_1;

    // Set 8 bit char, 1 stop bit, no parity
    UART_REG8(UART_REG_LCR_MCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP2 | UART_LCR_PARITY);

    // i.   Set "reg_mcr_loopback back;
    UART_REG8(UART_REG_LCR_MCR) &= ~0x10;

    serial_setbrg();

    return (0);
}

static void drv_console_put_char(const char c)
{
    while (!(UART_REG8(UART_REG_LSR) & UART_LSR_THRE))
        ;
    UART_REG8(UART_REG_THR_RBR_DLL) = c;
}

static void drv_console_write_char(const char c)
{
    if (c == '\n')
        drv_console_put_char('\r');

    drv_console_put_char(c);
}

static int drv_console_read_char(void)
{
    char c;

    do
    {
        WATCHDOG_RESET();
    } while (!(UART_REG8(UART_REG_LSR) & UART_LSR_DR));

    c = (char)(UART_REG8(UART_REG_THR_RBR_DLL) & 0xff);

    return c;
}

static int drv_uart_tstc(void)
{
    return ((UART_REG8(UART_REG_LSR) & UART_LSR_DR));
}

static void drv_console_setbrg(void)
{
    // set baud_rate
    u16 divisor = ((UART_CLK + 8 * UART_BAUDRATE_DEFAULT) / (16 * UART_BAUDRATE_DEFAULT));

    // i.   Set "reg_mcr_loopback";
    UART_REG8(UART_REG_LCR_MCR) |= 0x10;

    //  Clear FIFO Buffer
    UART_REG8(UART_REG_IIR_FCR) |= 0x07;

    // ii.   Poll "reg_usr_busy" till 0;
    while (UART_REG8(UART_REG_USR) & 0x01)
    {
        UART_REG8(UART_REG_IIR_FCR) =
            (UART_REG8(UART_REG_IIR_FCR) | (UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT));
    }
    //
    UART_REG8(UART_REG_LCR_MCR) |= UART_LCR_DLAB;
    UART_REG8(UART_REG_THR_RBR_DLL) = (divisor & 0xFF);
    UART_REG8(UART_REG_IER_DLH)     = ((divisor >> 8) & 0xFF);
    UART_REG8(UART_REG_LCR_MCR) &= ~(UART_LCR_DLAB);
    //
    UART_REG8(UART_REG_LCR_MCR) &= ~0x10;
}

static struct serial_device sstar_console_drv = {
    .name   = "sstar_uart",
    .start  = drv_console_init,
    .stop   = NULL,
    .setbrg = drv_console_setbrg,
    .putc   = drv_console_write_char,
    .puts   = default_serial_puts,
    .getc   = drv_console_read_char,
    .tstc   = drv_uart_tstc,
};

void sstar_serial_initialize(void)
{
    serial_register(&sstar_console_drv);
}

__weak struct serial_device *default_serial_console(void)
{
    return &sstar_console_drv;
}

int drv_uart_read_char(void)
{
    char c;

    do
    {
        WATCHDOG_RESET();
    } while (!(UART_MULTI_REG8(UART_REG_LSR) & UART_LSR_DR));

    c = (char)(UART_MULTI_REG8(UART_REG_THR_RBR_DLL) & 0xff);

    return c;
}

void drv_uart_write_char(const char c)
{
    if (c == '\n')
    {
        while (!(UART_MULTI_REG8(UART_REG_LSR) & UART_LSR_THRE))
            ;
        UART_MULTI_REG8(UART_REG_THR_RBR_DLL) = '\r';
    }
    else
    {
        while (!(UART_MULTI_REG8(UART_REG_LSR) & UART_LSR_THRE))
            ;
        UART_MULTI_REG8(UART_REG_THR_RBR_DLL) = c;
    }
}

int drv_uart_init(u8 port, u32 baudrate)
{
    u16 divisor      = 0;
    u32 src_clk_freq = 0;

    // printf("drv_uart_init port %d baudrate %d!!!\n", port, baudrate);
    src_clk_freq = drv_uart_padset(port);

    if (baudrate == 0)
        baudrate = UART_BAUDRATE_DEFAULT;

    divisor = ((src_clk_freq + 8 * baudrate) / (16 * baudrate));
    // i.   Set "reg_mcr_loopback";
    // printf("baudRate: 0x%x\r\n", baudrate);
    // printf("uart base: 0x%lx\r\n", uart_multi_base);

    UART_MULTI_REG8(UART_REG_LCR_MCR) |= 0x10;

    // ii.   Poll "reg_usr_busy" till 0;
    while (UART_MULTI_REG8(UART_REG_USR) & 0x01)
    {
        UART_MULTI_REG8(UART_REG_IIR_FCR) =
            (UART_MULTI_REG8(UART_REG_IIR_FCR) | (UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT));
    }

    UART_MULTI_REG8(UART_REG_IER_DLH) = 0x00;
    // Reset receiver and transmiter
    UART_MULTI_REG8(UART_REG_IIR_FCR) =
        UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_1;
    // Set 8 bit char, 1 stop bit, no parity
    UART_MULTI_REG8(UART_REG_LCR_MCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP2 | UART_LCR_PARITY);
    // i.   Set "reg_mcr_loopback back;
    UART_MULTI_REG8(UART_REG_LCR_MCR) &= ~0x10;

    //    // set baud_rate
    //
    // i.   Set "reg_mcr_loopback";
    UART_MULTI_REG8(UART_REG_LCR_MCR) |= 0x10;
    //  Clear FIFO Buffer
    UART_MULTI_REG8(UART_REG_IIR_FCR) |= 0x07;

    // ii.   Poll "reg_usr_busy" till 0;
    while (UART_MULTI_REG8(UART_REG_USR) & 0x01)
    {
        UART_MULTI_REG8(UART_REG_IIR_FCR) =
            (UART_MULTI_REG8(UART_REG_IIR_FCR) | (UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT));
    }
    // Set divisor
    UART_MULTI_REG8(UART_REG_LCR_MCR) |= UART_LCR_DLAB;
    UART_MULTI_REG8(UART_REG_THR_RBR_DLL) = (divisor & 0xFF);
    UART_MULTI_REG8(UART_REG_IER_DLH)     = ((divisor >> 8) & 0xFF);
    UART_MULTI_REG8(UART_REG_LCR_MCR) &= ~(UART_LCR_DLAB);

    UART_MULTI_REG8(UART_REG_LCR_MCR) &= ~0x10;
    return (0);
}
#endif
