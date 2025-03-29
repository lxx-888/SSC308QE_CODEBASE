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
#include <init.h>
#include <asm/mach-types.h>
#include <asm/global_data.h>
#include <asm/arch/mach/platform.h>
#include <asm/armv8/mmu.h>
#include <image.h>
#include <cpu_func.h>
#if defined(CONFIG_SSTAR_AUTOBOOT_RUN_UFU_ALWAYS) || defined(CONFIG_SSTAR_AUTOBOOT_RUN_UFU_BY_ENV)
#include <env.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_SSTAR_PADMUX
extern int sstar_padmux_init(void);
#endif

#if defined(CONFIG_SSTAR_PCIE) || defined(CONFIG_SSTAR_PCIE_ENDPOINT)
extern int sstar_dm_pcie_init(void);
#endif

#if defined(CONFIG_SSTAR_AUTOBOOT_RUN_UFU_ALWAYS) || defined(CONFIG_SSTAR_AUTOBOOT_RUN_UFU_BY_ENV)
extern int ufu_overwrite_bootcmd(void);
#endif

#if defined(CONFIG_CMD_SSTAR_MMC)
extern int recovery_check(int forceRecovery);
#endif
extern void setup_serial_number(void);

int board_init(void)
{
    /* arch number*/
    gd->bd->bi_arch_number = MACH_TYPE_INFINITY7;

    /* adress of boot parameters */
    gd->bd->bi_boot_params = BOOT_PARAMS;

    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;

#if defined(CONFIG_SSTAR_UBOOT_DTB_OVERLAY)
    extern bool board_uboot_dtb_overlay(void);
    board_uboot_dtb_overlay();
#endif

#if defined(CONFIG_SSTAR_PADMUX)
    sstar_padmux_init();
#endif

    return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#if defined(CONFIG_CMD_SSTAR_MMC)
    recovery_check(0);
#endif

#if defined(CONFIG_SSTAR_PCIE) || defined(CONFIG_SSTAR_PCIE_ENDPOINT)
    sstar_dm_pcie_init();
#endif

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

/* size: IO + NR_DRAM_BANKS + terminator */
struct mm_region sstar_mem_map[] = {
    {
        .virt  = 0x0UL,
        .phys  = 0x0UL,
        .size  = 0x20000000UL,
        .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN,
    },
    {
        .virt  = PHYS_SDRAM_1,
        .phys  = PHYS_SDRAM_1,
        .size  = PHYS_SDRAM_1_SIZE,
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE,
    },
    /* current max ram size is 256M(0x20000000~0x30000000)
     * map 0x30000000 for avb/fastboot
     */
    {
        .virt  = 0x30000000UL,
        .phys  = 0x30000000UL,
        .size  = 0x10000000UL,
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
