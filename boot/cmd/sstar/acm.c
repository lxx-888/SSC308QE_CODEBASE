/*
 * acm.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#include <common.h>
#include <command.h>
#include <console.h>
#include <errno.h>
#include "g_dnl.h"
#include <usb.h>
#include <watchdog.h>
#include <linux/delay.h>

int sstar_usbacm_tstc(void)
{
    struct stdio_dev *dev;

    dev = stdio_get_by_name("usbacm");
    return dev->tstc(dev);
}

void sstar_usbacm_start(void)
{
    struct stdio_dev *dev;

    dev = stdio_get_by_name("usbacm");
    dev->start(dev);
}

void sstar_usbacm_stop(void)
{
    struct stdio_dev *dev;

    dev = stdio_get_by_name("usbacm");
    dev->stop(dev);
}

char sstar_usbacm_getc(void)
{
    struct stdio_dev *dev;

    dev = stdio_get_by_name("usbacm");

    return dev->getc(dev);
}

void sstar_usbacm_putc(char c)
{
    struct stdio_dev *dev;

    dev = stdio_get_by_name("usbacm");

    dev->putc(dev, c);
}

void sstar_usbacm_puts(char *str)
{
    struct stdio_dev *dev;

    dev = stdio_get_by_name("usbacm");

    dev->puts(dev, str);
}

static int do_acm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    const char * usb_controller;
    unsigned int controller_index = 0;
    char         receive;
    unsigned int i = 0;

    if (argc == 2)
    {
        usb_controller   = argv[1];
        controller_index = (unsigned int)(simple_strtoul(usb_controller, NULL, 0));
    }

    if (usb_gadget_initialize(controller_index))
    {
        pr_err("Couldn't init USB controller.\n");
        return CMD_RET_FAILURE;
    }

    sstar_usbacm_start();

    while (1)
    {
        if (sstar_usbacm_tstc())
        {
            receive = sstar_usbacm_getc();
            printf("%c", receive);
            i++;
            if (i % 64 == 0)
            {
                printf("\n");
            }

            // tar_usbacm_putc(receive);
            // sstar_usbacm_puts(str);
        }
        if (ctrlc())
            break;

        WATCHDOG_RESET();
    }

    sstar_usbacm_stop();
    usb_gadget_release(controller_index);

    return 0;
}

U_BOOT_CMD(acm, 2, 1, do_acm, "Use the CDC ACM [USB Serial]",
           "acm <USB_controller> e.g. acm 0\n"
           "    USB_controller defaults to 0");
