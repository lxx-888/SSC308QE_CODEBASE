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
#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>
#endif
#include <asm/system.h>
#ifdef CONFIG_SSTAR_RTC
#include <drv_rtcpwc.h>
#endif
#ifdef CONFIG_SSTAR_MCU
#include <drv_mcu.h>
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
    gd->bd->bi_arch_number = MACH_TYPE_PIONEER5;

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

#ifdef CONFIG_SSTAR_RTC
    sstar_rtc_init();
#endif

#ifdef CONFIG_SSTAR_MCU
    sstar_mcu_init();
#endif

#ifdef CONFIG_CMD_SSTAR_OTP
    if (!IS_ENABLED(CONFIG_SYS_DCACHE_OFF) && dcache_status())
        mmu_set_region_dcache_behaviour(IMI_START_ADDR, IMI_SIZE, DCACHE_WRITETHROUGH);
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

#if defined(CONFIG_SSTAR_GPIO)
#include "../drivers/sstar/gpio/drv_gpio.h"
#include "../drivers/sstar/include/pioneer5/gpio.h"
#define FORCE_UPGRADE_GPIO        PAD_SR_IO3 // pad_key12, default pull up,active low
#define FORCE_UPGRADE_ACTIVE_HIGH 1          // set to 1 when active high
bool sstar_force_enter_ufu(void)
{
    char level;
    sstar_gpio_pad_set(FORCE_UPGRADE_GPIO);
    sstar_gpio_pad_odn(FORCE_UPGRADE_GPIO);
    sstar_gpio_pad_read(FORCE_UPGRADE_GPIO, &level);

    printf("boot_key level=%d \n", level);
    return (FORCE_UPGRADE_ACTIVE_HIGH) ? level : !level;
}
#else
bool sstar_force_enter_ufu(void)
{
    return false
}
#endif

#ifdef CONFIG_ARM64
struct mm_region sstar_mem_map[] = {
    {
        .virt  = 0x0UL,
        .phys  = 0x0UL,
        .size  = 0x20000000UL,
        .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN,
    },
    {
        .virt  = 0x20000000UL,
        .phys  = 0x20000000UL,
        .size  = 0x80000000UL,
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE,
    },
    {
        .virt  = IMI_START_ADDR,
        .phys  = IMI_START_ADDR,
        .size  = IMI_SIZE,
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE,
    },
    /* never use addr base 0x1000000000 and 0x20000000 together
     * unless you already understand they are the same physical
     * memory
     */
    {
        .virt  = 0x1000000000UL,
        .phys  = 0x1000000000UL,
        .size  = CONFIG_SSTAR_RAM_SIZE,
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE,
    },
    {
        /* List terminator */
        0,
    }};

struct mm_region *mem_map = sstar_mem_map;
#endif
