/*
 * ufu.c - Sigmastar
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

#include <common.h>
#include <command.h>
#include <console.h>
#include <errno.h>
#include "g_sstar_dnl.h"
#include <usb.h>
#include <usb_mass_storage.h>
#include <watchdog.h>
#include <linux/delay.h>

static int do_ufu(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    const char * usb_controller;
    unsigned int controller_index = 0;
    int          rc;
    int          cable_ready_timeout;

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

    rc = fsg_init(NULL, 1, controller_index);
    if (rc)
    {
        pr_err("fsg_init failed\n");
        rc = CMD_RET_FAILURE;
        goto cleanup_board;
    }

    rc = g_sstar_dnl_register("usb_dnl_ufu");
    if (rc)
    {
        pr_err("g_sstar_dnl_register failed\n");
        rc = CMD_RET_FAILURE;
        goto cleanup_board;
    }

    /* Timeout unit: seconds */
    cable_ready_timeout = UMS_CABLE_READY_TIMEOUT;

    if (!g_sstar_dnl_board_usb_cable_connected())
    {
        /*
         * Won't execute if we don't know whether the cable is
         * connected.
         */
        puts("Please connect USB cable.\n");

        while (!g_sstar_dnl_board_usb_cable_connected())
        {
            if (ctrlc())
            {
                puts("\rCTRL+C - Operation aborted.\n");
                rc = CMD_RET_SUCCESS;
                goto cleanup_register;
            }
            if (!cable_ready_timeout)
            {
                puts(
                    "\rUSB cable not detected.\n"
                    "Command exit.\n");
                rc = CMD_RET_SUCCESS;
                goto cleanup_register;
            }

            printf("\rAuto exit in: %.2d s.", cable_ready_timeout);
            mdelay(1000);
            cable_ready_timeout--;
        }
        puts("\r\n");
    }

    while (1)
    {
        usb_gadget_handle_interrupts(controller_index);

        rc = fsg_main_thread(NULL);
        if (rc)
        {
            printf("fsg_main_thread err: %d\n", rc);

            /* Check I/O error */
            if (rc == -EIO)
                printf("\rCheck USB cable connection\n");

            /* Check CTRL+C */
            if (rc == -EPIPE)
                printf("\rCTRL+C - Operation aborted\n");

            /* Check invalud command */
            if (rc == -EINVAL)
            {
                printf("\rInvalid command\n");
            }

            rc = CMD_RET_SUCCESS;
            goto cleanup_register;
        }

        WATCHDOG_RESET();
    }

cleanup_register:
    g_sstar_dnl_unregister();
cleanup_board:
    usb_gadget_release(controller_index);

    return rc;
}

U_BOOT_CMD(ufu, 2, 1, do_ufu, "Use the UFU [USB Firmware Update]",
           "ufu <USB_controller> e.g. ufu 0\n"
           "    USB_controller defaults to 0");
