/*
 * uart.c - Sigmastar
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

#include <command.h>
#include <common.h>
#include <malloc.h>

#include <asm/arch/mach/io.h>
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/sstar_types.h>

#ifdef CONFIG_DM
#include <dm.h>
#ifdef CONFIG_DM_SERIAL
#include <serial.h>
#endif
#endif

#ifdef CONFIG_DM_SERIAL
extern U32 drv_uart_padset(u8 port);
#else
extern int  drv_uart_init(u8 port, u32 baudrate);
extern void drv_uart_write_char(const char c);
extern int  drv_uart_read_char(void);
#endif

int do_uart(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    int        dev;
    int        baud;
    static int uart_probe = 0;
    char       c;
    char *     cmd;
#ifdef CONFIG_DM_SERIAL
    int                          ret  = 0;
    static struct udevice *      udev = NULL;
    static struct dm_serial_ops *ops  = NULL;
#endif

    if (argc < 2)
    {
        return CMD_RET_USAGE;
    }
    cmd = argv[1];

    if (strcmp(cmd, "init") == 0)
    {
        if (argc < 3)
        {
            printf("uart init ERROR!\r\n");
            printf("uart init [port] [baudrate]\r\n");
            printf("eg: uart init 3 115200 \r\n");
            return CMD_RET_USAGE;
        }

        dev  = (int)simple_strtoul(argv[2], NULL, 10);
        baud = (int)simple_strtoul(argv[3], NULL, 10);

#ifdef CONFIG_DM_SERIAL

        ret = uclass_get_device_by_seq(UCLASS_SERIAL, dev, &udev);
        if (ret)
        {
            printf("failed to find uart device %d\r\n", dev);
            return CMD_RET_FAILURE;
        }
        ops = serial_get_ops(udev);
        if (ops && ops->setbrg)
            ops->setbrg(udev, baud);
#else
        drv_uart_init(dev, baud);
#endif

        uart_probe += 1;
    }
    else if (strcmp(cmd, "putchar") == 0)
    {
        if (uart_probe)
        {
            c = (int)simple_strtoul(argv[2], NULL, 10);
#ifdef CONFIG_DM_SERIAL
            if (udev && ops && ops->putc)
                ops->putc(udev, c);
#else
            drv_uart_write_char(c);
#endif
        }
        else
            printf("please run uart init first!\r\n");
    }
    else if (strcmp(cmd, "getchar") == 0)
    {
        if (uart_probe)
        {
#ifdef CONFIG_DM_SERIAL
            if (udev && ops && ops->getc)
                printf("%x\r\n", ops->getc(udev));
#else
            printf("%x\r\n", drv_uart_read_char());
#endif
        }
        else
            printf("please run uart init first!\r\n");
    }
    else
        return CMD_RET_USAGE;
    return 0;
}

static char uart_help_text[] =
    "uart init [port] [baudrate] - initial uart with the specified port & "
    "baudrate\n"
    "uart init 1 buad\n"
    "uart init 2 buad\n"
    "...\n"
    "uart putchar [char] - output character in Decimal base\n"
    "uart getchar - get character, will show in Hex base\n";

U_BOOT_CMD(uart, CONFIG_SYS_MAXARGS, 1, do_uart, "UART sub-system", uart_help_text);
