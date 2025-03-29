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
#include <asm/cache.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_SSTAR_PADMUX
extern int sstar_padmux_init(void);
#endif

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
    gd->bd->bi_arch_number = MACH_TYPE_MERCURY6P;

    /* adress of boot parameters */
    gd->bd->bi_boot_params = BOOT_PARAMS;

    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;

#ifdef CONFIG_SSTAR_PADMUX
    sstar_padmux_init();
#endif

#ifdef CONFIG_CMD_SSTAR_OTP
    if (dcache_status())
        mmu_set_region_dcache_behaviour(IMI_START_ADDR, MMU_SECTION_SIZE, DCACHE_WRITETHROUGH);
#endif

    return 0;
}

void enable_caches(void)
{
    icache_enable();
    dcache_enable();
}

void custom_mmu_setup(void)
{
#ifdef CONFIG_OPTEE
    int i;

    for (i = (TF_A_START_ADDR >> MMU_SECTION_SHIFT); i <= (TF_A_END_ADDR >> MMU_SECTION_SHIFT); i++)
        set_section_dcache(i, DCACHE_OFF);

    for (i = (OPTEE_START_ADDR >> MMU_SECTION_SHIFT); i <= (OPTEE_END_ADDR >> MMU_SECTION_SHIFT); i++)
        set_section_dcache(i, DCACHE_OFF);
#endif
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
    return 0;
}
#endif

int sstar_prep_linux(bootm_headers_t *images)
{
    return 0;
}

#if CONFIG_IS_ENABLED(DISPLAY_CPUINFO) && !CONFIG_IS_ENABLED(CPU)
int print_cpuinfo(void)
{
    printf("SoC: SigmaStar %s\n", CONFIG_SYS_SOC);

    return 0;
}
#endif
