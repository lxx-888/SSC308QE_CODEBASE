/*
 * pm.c - Sigmastar
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

#ifdef CONFIG_SSTAR_MCU
#include <drv_mcu.h>
#endif
#ifdef CONFIG_SSTAR_RTC
#include <drv_rtcpwc.h>
#endif
#ifdef CONFIG_SSTAR_FLASH
#include <drvFSP_QSPI.h>
#endif

#define MCU_AC_ON_MAGIC 0x5AA5

int do_pm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u16 magic;

    magic = INREG16(REG_ADDR_BASE_PM_MAILBOX + REG_ID_2F);
    if (magic != MCU_AC_ON_MAGIC)
    {
        OUTREG16((REG_ADDR_BASE_PM_MAILBOX + REG_ID_2F), MCU_AC_ON_MAGIC);
        printf("\nRequesting system poweroff\n");
#ifdef CONFIG_SSTAR_FLASH
        DRV_QSPI_use_sw_cs(0);
#endif
#ifdef CONFIG_SSTAR_MCU
        sstar_mcu_write_config();
#endif
#ifdef CONFIG_SSTAR_RTC
        sstar_rtc_poweroff();
#endif
        asm volatile("b .");
    }
    return 0;
}

U_BOOT_CMD(pm, 1, 0, do_pm, "power management command", "pm\n");
