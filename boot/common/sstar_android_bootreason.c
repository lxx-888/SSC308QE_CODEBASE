/*
 * sstar_android_bootreason.c - Sigmastar
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
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/io.h>
#include <sstar_android_bootreason.h>


static char *known_boot_reason[ANDR_BOOT_REASON_TYPES] = {
	[ANDR_BOOT_REASON_COLD] =               "cold",
	[ANDR_BOOT_REASON_HARD] =               "hard",
	[ANDR_BOOT_REASON_WARM] =               "warm",
	[ANDR_BOOT_REASON_WATCHDOG] =           "watchdog",
	[ANDR_BOOT_REASON_REBOOT] =             "reboot",
};

char *sstar_get_reboot_reason(void)
{
	uint16_t reg_val = 0;

	reg_val = INREG16(REG_ADDR_BASE_WDT + REG_ID_02);
	if (reg_val & BIT_1) {
		return known_boot_reason[ANDR_BOOT_REASON_WATCHDOG];
	}

	reg_val = INREG16(REG_ADDR_BASE_PMPOR + REG_ID_01);

	if (reg_val >= ANDR_BOOT_REASON_TYPES) {
		return NULL;
	}

	return known_boot_reason[reg_val];
}

