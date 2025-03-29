/*
 * gmac.c - Sigmastar
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
#include <log.h>
#include <command.h>
#include <asm/arch/mach/io.h>
#include <asm/arch/mach/platform.h>
#include <sstar_sys_utility.h>
#include "mhal_gmac.h"

#define CMD_SSTAR_GMAC_MIN_ARGS 2
#define CMD_SSTAR_GMAC_MAX_ARGS 5
static char *gmac_drv_help_text =
    "gmac drv <gmac number> <io index> <gear>\n"
    "<gmac number> = 0: gmac0 , 1: gmac1\n"
    "<io index> = from 0 to 7  mapping to\n"
    "GMAC_MDC,\n"
    "GMAC_MDIO,\n"
    "GMAC_RGMII_TX_CLK,\n"
    "GMAC_RGMII_TX_D0,\n"
    "GMAC_RGMII_TX_D1,\n"
    "GMAC_RGMII_TX_D2,\n"
    "GMAC_RGMII_TX_D3,\n"
    "GMAC_RGMII_TX_CTL,\n"
    "GMAC_RGMII_MCLK,\n"
    "<gear> = driving gear, from 0 to (4 or 15)\n";
static char *gmac_epf_help_text =
    "gmac epf <enable>\n"
    "<enable> = 0: disable , 1: enable\n";
static char *gmac_dump_help_text =
    "gmac dump <direction>\n"
    "<direction> = 0: disable , 1: Tx, 2: RX, 3 :TX and RX\n";
static char *gmac_loopback_help_text =
    "gmac loopback <type> <length> <speed>\n"
    "<type> = 0: disable , 1: PHY loopback, 2: MAC loopback\n"
    "<length> the length in bytes for loopback test packet\n"
    "<speed>  speed in MHz for loopback test\n";

char gGmacEPF           = 0;
char gGmacDumpTx        = 0;
char gGmacDumpRx        = 0;
char gGmacLoopback      = 0;
int  gGmacLoopbackSpeed = 0;
int  gGmacLoopbackLen   = 0;

int adjust_gmac(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    if (argc < CMD_SSTAR_GMAC_MIN_ARGS)
    {
        cmd_usage(cmdtp);
        return 0;
    }

    if (strcmp(argv[1], "drv") == 0)
    {
        int gmacid, io, gear;

        if (argc < 5)
        {
            printf("%s", gmac_drv_help_text);
        }

        gmacid = simple_strtoul(argv[2], NULL, 10);
        io     = simple_strtoul(argv[3], NULL, 10);
        gear   = simple_strtoul(argv[4], NULL, 10);

        if (sstar_gamc_adjust_driving(gmacid, io, gear))
        {
            printf("%s", gmac_drv_help_text);
        }
    }
    else if (strcmp(argv[1], "epf") == 0)
    {
        if (argc < 3)
        {
            printf("%s", gmac_epf_help_text);
        }

        gGmacEPF = simple_strtoul(argv[2], NULL, 10);

        printf("%s packet forwarding!\n", gGmacEPF ? "enable" : "disable");
    }
    else if (strcmp(argv[1], "dump") == 0)
    {
        char dir;

        if (argc < 3)
        {
            printf("%s", gmac_dump_help_text);
        }

        dir = simple_strtoul(argv[2], NULL, 10);

        if (dir == 0)
        {
            gGmacDumpTx = gGmacDumpRx = 0;
            printf("disable dump function!\n");
        }
        else if (dir == 1)
        {
            gGmacDumpTx = 1;
            gGmacDumpRx = 0;
            printf("enable tx dump function!\n");
        }
        else if (dir == 2)
        {
            gGmacDumpTx = 0;
            gGmacDumpRx = 1;
            printf("enable rx dump function!\n");
        }
        else if (dir == 3)
        {
            gGmacDumpTx = 1;
            gGmacDumpRx = 1;
            printf("enable tx&rx dump function!\n");
        }
        else
        {
            printf("%s", gmac_dump_help_text);
        }
    }
    else if (strcmp(argv[1], "loopback") == 0)
    {
        char loopback;
        int  speed;
        int  length;

        if (argc < 5)
        {
            printf("%s", gmac_loopback_help_text);
        }

        loopback = simple_strtoul(argv[2], NULL, 10);
        length   = simple_strtoul(argv[3], NULL, 10);
        speed    = simple_strtoul(argv[4], NULL, 10);

        if (loopback == 0)
        {
            gGmacLoopback = 0;
            printf("disable loopback!\n");
        }
        else if (loopback == 1)
        {
            gGmacLoopback      = 1;
            gGmacLoopbackLen   = length;
            gGmacLoopbackSpeed = speed;

            printf("enable PHY loopback, len=%u speed=%u\n", gGmacLoopbackLen, gGmacLoopbackSpeed);
        }
        else if (loopback == 2)
        {
            gGmacLoopback      = 2;
            gGmacLoopbackLen   = length;
            gGmacLoopbackSpeed = speed;

            printf("enable MAC loopback, len=%u speed=%u\n", gGmacLoopbackLen, gGmacLoopbackSpeed);
        }
        else
        {
            printf("%s", gmac_loopback_help_text);
        }
    }
    else
    {
        cmd_usage(cmdtp);
    }

    return 0;
}

U_BOOT_CMD(gmac, CMD_SSTAR_GMAC_MAX_ARGS, 0, adjust_gmac, "gmac command",
           "drv : adjust driving \n"
           "epf : enable or disable packet forwarding \n"
           "dump : dump(show) tx/rx packets content\n"
           "loopback : enable loopback test\n");
