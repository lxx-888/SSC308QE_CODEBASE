/*
 * rndis.c - Sigmastar
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
#include <net.h>
#include <env.h>

int do_rndis(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    /* set env ethact to usb_ether,
     * and give a very long connect timeout for configuring ethernet card in host side
     */
    env_set("ethact", "usb_ether");
    env_set("cdc_connect_timeout", "5000000");
    /* initial usb ethernet */
    usb_ether_init();
    /* net commands now can use... e.g. ping */

    return 0;
}

U_BOOT_CMD(rndis, 1, 1, do_rndis, "initial usb ethernet", "rndis - initial usb ethernet");
