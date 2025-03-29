/*
 * miu_ut.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifndef SSTAR_MIU_UT_H
#define SSTAR_MIU_UT_H

#include "cam_os_wrapper.h"

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "mdrv_types.h"
#include "drv_miu.h"
// #include "mhal_common.h"
#include "hal_miu_if.h"
#include "ms_msys.h"
#include "mdrv_msys_io_st.h"
#include "sstar_ut.h"
#include "registers.h"

#define MIU_PASS true
#define MIU_FAIL false

#define MIU_UT_DEV "miu_ut"

typedef struct miu_ut_memory
{
    unsigned int   size;
    ss_phys_addr_t phy_addr;
    ss_phys_addr_t virt_phy_addr;
    ss_miu_addr_t  miu_addr;
    ss_miu_addr_t  miu_entry;
    unsigned int   cover_entry;
    unsigned int   region;
    unsigned int*  virt_addr;
} miu_ut_memory;

typedef struct miu_mmu_info
{
    unsigned int page_size;
    unsigned int page_bits;

    unsigned int entry_size;
    unsigned int entry_bits;
    unsigned int max_entry;

    unsigned int region_size;
    unsigned int region_bits;
    unsigned int max_region;
} miu_mmu_info;

typedef enum
{
    E_MMU_PGSZ_32 = 0,
    E_MMU_PGSZ_64,
    E_MMU_PGSZ_128,
} MMU_PGSZ_MODE;
//==============================================================================
//                          MMU UT Test
//==============================================================================

#define MIU_MMU_ADDR_BIT         (36)
#define MMU_TEST_MEM_SIZE        (1 * 1024 * 1024)                // 1MB
#define MIU_PROTECT_ALIGNED_MASK (~((HAL_MIU_PROTECT_SHIFT - 1))) // for aligned

#define PAGE_SIZE_NUM 3

// chip
#if defined(CONFIG_ARCH_PIONEER5) || defined(CONFIG_ARCH_SOUFFLE)
#define CPU_CLIENT_ID MIU_CLIENTW_CA55
#elif defined(CONFIG_ARCH_IFORD)
#define CPU_CLIENT_ID MIU_CLIENTW_CPU_CA32
#endif

bool alloc_miu_ut_memory(miu_ut_memory* mem, unsigned int size);
void free_miu_ut_memory(miu_ut_memory* mem);

#endif /* SSTAR_MIU_UT_H */
