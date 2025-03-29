/*
 * board.c - Sigmastar
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
#include <asm/mach-types.h>
#include <asm/global_data.h>
#include <asm/arch/mach/platform.h>
#include <asm/arch/mach/io.h>
#include <image.h>
#include <cpu_func.h>
#include <asm/system.h>
#ifdef CONFIG_SSTAR_RTC
#include <drv_rtcpwc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern void setup_serial_number(void);

void reset_misc(void)
{
    /* set SW reset type */
    SETREG16(BASE_REG_PMPOR_PA + REG_ID_01,
             BIT0); // assign bit to bank por, POR will be clear to 0 only when hw reset or power on
    SETREG16(BASE_REG_WDT_PA + REG_ID_02, BIT0); // clear wdt before sw reset to recognize reset type correctly
}
int board_init(void)
{
    /* arch number */
    gd->bd->bi_arch_number = MACH_TYPE_INFINITY6C;

    /* adress of boot parameters */
    gd->bd->bi_boot_params = BOOT_PARAMS;

    gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
    gd->bd->bi_dram[0].size  = CONFIG_SYS_SDRAM_SIZE;

#ifdef CONFIG_SSTAR_UBOOT_DTB_OVERLAY
    extern bool board_uboot_dtb_overlay(void);
    board_uboot_dtb_overlay();
#endif

#ifdef CONFIG_SSTAR_PADMUX
    extern int sstar_padmux_init(void);
    sstar_padmux_init();
#endif

    return 0;
}

void enable_caches(void)
{
    icache_enable();
    dcache_enable();
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
    setup_serial_number();
    return 0;
}
#endif

#if CONFIG_IS_ENABLED(DISPLAY_CPUINFO) && !CONFIG_IS_ENABLED(CPU)
int print_cpuinfo(void)
{
    printf("SoC: SigmaStar %s\n", CONFIG_SYS_SOC);

    return 0;
}
#endif
