/*
* pm_mcu.c- Sigmastar
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
#include "ms_platform.h"
#include <linux/pm.h>
#include <linux/platform_device.h>

//#define UART_TX_BY_DRIVER

#ifdef UART_TX_BY_DRIVER
#include "hal_uart.h"
#else
#define UART_READ_BYTE(_reg_)         (*(volatile unsigned char *)(IO_ADDRESS(_reg_)))
#define UART_WRITE_BYTE(_reg_, _val_) ((*(volatile unsigned char *)(IO_ADDRESS(_reg_))) = (unsigned char)(_val_))
#define UART_REG_USR(base)            ((base) + ((0x0E) << 2))
#define UART_REG_DLL_THR_RBR(base)    ((base) + ((0x00) << 2))
#define UART_USR_TXFIFO_NOT_FULL      0x02
#endif

struct pm_mcu_cmd {
    u32 header;     /* 0x5A5A5A5A */
    u8  is_ack;     /* 0: is a request to mcu */
    u8  cmd;        /* E_TASK_POWEROFF_OK: 7 for power off */
    u8  response;   /* for ACK only, not use for power off */
    u8  status;     /* E_SUSPENDED:6 for power off */
    u16 usr_dat;    /* not use for power off */
    u32 tail;       /* 0xA5A5A5A5 */
} __attribute__((packed));

static struct pm_mcu_cmd mcu_cmd = {
    .header   = 0x5A5A5A5A,
    .is_ack   = 0,
    .cmd      = 7,
    .response = 0,
    .status   = 6,
    .usr_dat  = 0,
    .tail     = 0xA5A5A5A5,
};

static void sstar_pm_mcu_pwroff(void)
{
#if defined(CONFIG_PM_MCU_USE_UART1)
    u32 reg_base = 0x1F221200;
#elif defined(CONFIG_PM_MCU_USE_FUART)
    u32 reg_base = 0x1F220400;
#endif

#ifdef UART_TX_BY_DRIVER
    struct uart_hal hal;

    hal.uart_base = reg_base;
    hal.urdma_en  = 0;
    hal_uart_write(&hal, (u8 *)&mcu_cmd, sizeof(mcu_cmd));
#else
    u32 size = sizeof(mcu_cmd);
    u8 *buf  = (u8 *)&mcu_cmd;
    do
    {
        //while(!(UART_READ_BYTE(UART_REG_USR(reg_base)) & UART_USR_TXFIFO_NOT_FULL));
        UART_WRITE_BYTE(UART_REG_DLL_THR_RBR(reg_base), *buf++);
    } while (--size);
#endif
    printk(KERN_ERR "pm_mcu_pwroff\r\n");
}

void __init sstar_pm_mcu_init(void)
{
    pm_power_off = sstar_pm_mcu_pwroff;
}

