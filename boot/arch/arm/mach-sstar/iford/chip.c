/*
 * chip.c - Sigmastar
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
#include <env_internal.h>
#include <asm/global_data.h>
#include "env.h"
#include "asm/arch/mach/sstar_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include "sstar_version.h"
#include <sstar_android_bootreason.h>

#define STORAGE_SPI_NONE         (0x00)
#define STORAGE_SPI_NAND_SKIP_SD (BIT2)
#define STORAGE_SPI_NAND         (BIT4)
#define STORAGE_SPI_NOR          (BIT5)
#define STORAGE_SPI_NOR_SKIP_SD  (BIT1)
#define STORAGE_USB              (BIT12)
#define STORAGE_EMMC_8           (BIT7 | BIT3)
#define STORAGE_EMMC_4           (BIT3)
#define STORAGE_BOOT_TYPES       (BIT12 | BIT7 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1)

typedef struct
{
    unsigned char tagPrefix[3];
    unsigned char headerVersion[1];
    unsigned char libType[2];
    unsigned char chip[4];
    unsigned char changelist[8];
    unsigned char component[10];
    unsigned char reserved[1];
    unsigned char tagSuffix[3];
} SSTAR_VERSION;

SSTAR_VERSION UBT_VERSION = {
    {'M', 'V', 'X'}, MVXV_HEAD_VER,
    MVXV_LIB_TYPE, // R = general release version
    MVXV_CHIP_ID,    MVXV_CHANGELIST, MVXV_COMP_ID, {'#'}, {'X', 'V', 'M'},
};
DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
    gd->xtal_clk = CONFIG_PIUTIMER_CLOCK;

    return 0;
}

// we borrow the DRAN init to do the devinfo setting...
int dram_init(void)
{
    gd->ram_size = CONFIG_UBOOT_RAM_SIZE;

    return 0;
}

DEVINFO_CHIP_TYPE sstar_check_chip(void)
{
    return DEVINFO_NON;
}

int checkboard(void)
{
    int i = 0;

    printf("Version: ");
    for (i = 0; i < 4; i++)
    {
        printf("%c", UBT_VERSION.chip[i]);
    }
    for (i = 0; i < 8; i++)
    {
        printf("%c", UBT_VERSION.changelist[i]);
    }
    printf("\n");

#ifdef CONFIG_CMD_BDI
    printf("***********************************************************\r\n");
    printf("* MEMORY LAYOUT                                            \r\n");
    printf("* PHYS_SDRAM_1:           0X%08x                           \r\n", PHYS_SDRAM_1);
    printf("* PHYS_SDRAM_1_SIZE:      0X%08x                           \r\n", PHYS_SDRAM_1_SIZE);
    printf("* CONFIG_SYS_TEXT_BASE:   0X%08x                           \r\n", CONFIG_SYS_TEXT_BASE);
    printf("* CONFIG_SYS_SDRAM_BASE:  0X%08x                           \r\n", CONFIG_SYS_SDRAM_BASE);
    printf("* CONFIG_SYS_INIT_SP_ADDR:0X%08x  (gd_t *)pointer          \r\n", CONFIG_SYS_INIT_SP_ADDR);
    printf("* SCFG_MEMP_START:        0X%08x                           \r\n", SCFG_MEMP_START);
    printf("* SCFG_PNLP_START:        0X%08x                           \r\n", SCFG_PNLP_START);
    printf("* BOOT_PARAMS:            0X%08x                           \r\n", BOOT_PARAMS);
    printf("* CONFIG_SYS_LOAD_ADDR:   0X%08x                           \r\n", CONFIG_SYS_LOAD_ADDR);
    // printf("* KERNEL_RAM_BASE:0X%08x                                   \r\n",KERNEL_RAM_BASE);
    printf("* CONFIG_UNLZO_DST_ADDR:  0X%08x                           \r\n", CONFIG_UNLZO_DST_ADDR);
    printf("\r\n");
    printf("* CONFIG_ENV_SIZE:        0X%08x                           \r\n", CONFIG_ENV_SIZE);
    printf("* CONFIG_SYS_MALLOC_LEN:  0X%08x                           \r\n", CONFIG_SYS_MALLOC_LEN);
    printf("* CONFIG_STACKSIZE:       0X%08x                           \r\n", CONFIG_STACKSIZE);
    printf("* KERNEL_IMAGE_SIZE:      0X%08x                           \r\n", KERNEL_IMAGE_SIZE);
    printf("***********************************************************\r\n");
#endif
    return 0;
}

DEVINFO_BOOT_TYPE sstar_devinfo_boot_type(void)
{
    U16 u16Storage;
    u16Storage = (INREG16(GET_REG_ADDR(SSTAR_BASE_REG_DID_KEY_PA, 0x70))) & STORAGE_BOOT_TYPES;

    if (u16Storage == STORAGE_SPI_NOR || u16Storage == STORAGE_SPI_NOR_SKIP_SD)
    {
        return DEVINFO_BOOT_TYPE_SPI;
    }
    else if (u16Storage == STORAGE_SPI_NAND || u16Storage == STORAGE_SPI_NAND_SKIP_SD)
    {
        return DEVINFO_BOOT_TYPE_SPINAND_INT_ECC;
    }
    else if (u16Storage == STORAGE_EMMC_4 || u16Storage == STORAGE_EMMC_8)
    {
        return DEVINFO_BOOT_TYPE_EMMC;
    }

    return DEVINFO_BOOT_TYPE_NONE;
}

#ifndef CONFIG_SSTAR_SAVE_ENV_IN_ISP_FLASH
#ifdef CONFIG_ENV_IS_IN_NAND
extern int  nand_env_init(void);
extern int  nand_saveenv(void);
extern void nand_env_relocate_spec(void);
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
extern int  mmc_env_init(void);
extern int  mmc_saveenv(void);
extern void mmc_env_relocate_spec(void);
#endif

env_t *env_ptr;

void env_relocate_spec(void)
{
#ifdef CONFIG_ENV_IS_IN_MMC
    mmc_env_relocate_spec();
#elif defined(CONFIG_ENV_IS_IN_NAND)
    nand_env_relocate_spec();
#endif
}

int saveenv(void)
{
#ifdef CONFIG_ENV_IS_IN_MMC
    return mmc_saveenv();
#elif defined(CONFIG_ENV_IS_IN_NAND)
    return nand_saveenv();
#endif
    return 0;
}
#endif

inline void _chip_flush_miu_pipe(void)
{
    // M6P don't need to flush the miu pipe
}

inline void Chip_Flush_Memory(void)
{
    _chip_flush_miu_pipe();
}

inline void Chip_Read_Memory(void)
{
    _chip_flush_miu_pipe();
}

void Chip_Set_RebootType(uint16_t arg)
{
    SETREG16(REG_ADDR_BASE_PMPOR + REG_ID_01, ANDR_BOOT_REASON_REBOOT);
    // remove later
    SETREG16(REG_ADDR_BASE_WDT + REG_ID_02, BIT0);
}

uint16_t Chip_Get_RebootType(void)
{
    uint16_t reg_val;

    reg_val = INREG16(REG_ADDR_BASE_WDT + REG_ID_02);
    if (reg_val & 0x01)
        return ANDR_BOOT_REASON_WATCHDOG;

    reg_val = INREG16(REG_ADDR_BASE_PMPOR + REG_ID_01);
    return (reg_val > ANDR_BOOT_REASON_TYPES) ? ANDR_BOOT_REASON_REBOOT : reg_val;
}

int Chip_Get_Revision(void)
{
    u16 tmp = 0;
    tmp     = INREG16((unsigned int)(BASE_REG_PMTOP_PA + REG_ID_01));
    tmp     = ((tmp >> 8) & 0x00FF);

    return (tmp + 1);
}
