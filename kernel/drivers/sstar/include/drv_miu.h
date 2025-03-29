/*
 * drv_miu.h - Sigmastar
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

#ifndef __DRV_MIU_H__
#define __DRV_MIU_H__

#include <cam_os_wrapper.h>
#include <drv_miu_datatype.h>

struct miu_dram_info
{
    u64 size;        // MB
    u32 dram_freq;   // MHz
    u32 miupll_freq; // MHz
    u8  type;        // 2:DDR2, 3:DDR3
    u8  data_rate;   // 4:4x mode, 8:8x mode,
    u8  bus_width;   // 16:16bit, 32:32bit, 64:64bit
    u8  ssc;         // 0:off, 1:on
};

// in order to be compatible with SDK
#ifndef Bool
#define Bool unsigned char
#endif

#if defined(CAM_OS_RTK)
#include <stdbool.h>
#ifndef bool
#define bool u8
#define true 1
#define false 0
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef CONFIG_MIU_HW_MMU
#define CONFIG_MIU_HW_MMU
#endif
#endif // defined(CAM_OS_RTK)

#ifdef CONFIG_MIU_HW_MMU
#ifndef CONFIG_MMU_INTERRUPT_ENABLE
#define CONFIG_MMU_INTERRUPT_ENABLE
#endif
#endif // CONFIG_MIU_HW_MMU

#define MIU_PROTECT_INVLID_BLOCK (~0)
#define MIU_PROTECT_UNIT         (1 << 13)

//=================================================================================================
//                                     MIU Client Function
//=================================================================================================
/**
 * sstar_miu_module_reset - do miu module reset flow
 * @ClientName: client name
 * @write: is write client
 *
 * return: 0: pass, -1: fail
 */
unsigned char sstar_miu_module_reset(char* ClientName, Bool write);

/**
 * sstar_miu_module_reset_byid - do miu module reset flow
 * @ClientID: client ID
 * @write: is write client
 *
 * return: 0: pass, -1: fail
 */
unsigned char sstar_miu_module_reset_byid(miu_client_id ClientID, Bool write);

/**
 * sstar_miu_client_id_to_name - Get ip name by id
 * @id: client ip
 * @write: is write client
 *
 * return: name if id invalid else NULL
 */
char* sstar_miu_client_id_to_name(u16 id, Bool write);

//=================================================================================================
//                                     MIU Protect Function
//=================================================================================================

/**
 * sstar_miu_protect_query - Returns an idle block index, otherwise returns MIU_PROTECT_INVLID_BLOCK
 * @mmu: is mmu block
 */
u8 sstar_miu_protect_query(Bool mmu);

/**
 * sstar_miu_protect_enable - Enable protect for block index
 * @index: block index, it is better to use the value obtained by sstar_miu_protect_query
 * @list: protect id list, must terminate with MIU_CLIENTW_NULL
 * @invert: false for whitelist, true for blacklist
 * @start: protect startd adress(unit: Bytes), must aligned with MIU_PROTECT_UNIT
 * @end: protect end adress(unit: Bytes), must aligned with MIU_PROTECT_UNIT
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_protect_enable(u8 index, u16* list, Bool invert, ss_phys_addr_t start, ss_phys_addr_t end);

/**
 * sstar_miu_protect_disable - Disable protect for block index
 * @index: block index
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_protect_disable(u8 index);

/**
 * sstar_miu_protect_append_id - Append a protect id to block index
 * @index: block index
 * @id: id to be appended, if already exists, do nothing
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_protect_append_id(u8 index, u16 id);

/**
 * sstar_miu_protect_append_id - Remove a protect id from block index
 * @index: block index
 * @id: id to be removed, if not exists, do nothing
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_protect_remove_id(u8 index, u16 id);

/**
 * sstar_miu_protect_default_kernel_whitelist - Return default kernel protect whitelist
 */
u16* sstar_miu_protect_default_kernel_whitelist(void);

/**
 * sstar_miu_protect_default_kernel_blacklist - Return default kernel protect blacklist
 * Must set with invert == true
 */
u16* sstar_miu_protect_default_kernel_blacklist(void);

/**
 * sstar_miu_protect_list - Return protect list for block index, NULL if block is disable
 * @invert: out, if the blokc list is invert
 */
u16* sstar_miu_protect_list(u8 index, Bool* invert);

/**
 * sstar_miu_protect_status - Return if the block is enable
 * @enable: out, if the block is enable
 *
 * return: 0 on success, < 1 on error
 */
Bool sstar_miu_protect_status(u8 index);

/**
 * sstar_miu_protect_address - Get protect address for block index
 * @start: out, protect start address
 * @end: out, protect end address
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_protect_address(u8 index, ss_phys_addr_t* start, ss_phys_addr_t* end);

/**
 * sstar_miu_protect_set_panic - Set if kernel panic when protect hit
 * @enable: if true, will kernel panic when protect hit
 */
void sstar_miu_protect_set_panic(Bool enable);

//=================================================================================================
//                                     MIU MMU Function
//=================================================================================================

#if defined(CONFIG_MIU_HW_MMU)
/**
 * The address of MIU has a total of 36 bits, which are distributed as follows:
 * bit35                                               bit0
 *   ┌───────────────┬──────────────────┬───────────────┐
 *   │   region      │     entry        │    page       │
 *   └───────────────┴──────────────────┴───────────────┘
 * Among them, entry of each chip is fixed, page size can be adjusted,
 * region size changes with page Size.
 */

#define MMU_PAGE_SIZE       (0x8000)
#define MMU_PAGE_SIZE_32K   (0x8000)
#define MMU_PAGE_SIZE_64K   (0x10000)
#define MMU_PAGE_SIZE_128K  (0x20000)
#define MMU_PAGE_SIZE_256K  (0x40000)
#define MMU_PAGE_SIZE_512K  (0x80000)
#define MMU_PAGE_SIZE_1024K (0x100000)

// #define MMU_PAGE_SIZE(K) (K << 10)

// enum miu_mmu_page_mode
// {
//     MIU_MMU_PGSZ_32  = 0,
//     MIU_MMU_PGSZ_64  = 1,
//     MIU_MMU_PGSZ_128 = 2,
// };

/**
 * sstar_miu_mmu_set_page_size - Set mmu page size
 * @page_mode: use MIU_MMU_PGSZ_XX to get a page size
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_mmu_set_page_size(u8 page_mode);

/**
 * sstar_miu_mmu_get_page_size - Return current mmu page size
 *
 * return: 0 on success, < 1 on error
 */
u32 sstar_miu_mmu_get_page_size(void);

/**
 * sstar_miu_mmu_set_region - map vpa_regiojn to pa_region
 * @vpa_region: virtual phyical address region
 * @pa_region: phyical address region
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_mmu_set_region(u16 vpa_region, u16 pa_region);

/**
 * sstar_miu_mmu_map_entry - map vpa_entry to pa_entry
 * @vpa_region: virtual phyical address entry
 * @pa_region: phyical address entry
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_mmu_map_entry(u16 vpa_entry, u16 pa_entry);

/**
 * sstar_miu_mmu_unmap_entry - unmap vpa_entry
 * @vpa_region: virtual phyical address entry
 * @pa_region: phyical address entry
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_mmu_unmap_entry(u16 vpa_entry);

/**
 * sstar_miu_mmu_query_entry - Query which pa_entry is mapped to the vpa_entry
 * @vpa_region: virtual phyical address entry
 *
 * return: pa_entry
 */
u16 sstar_miu_mmu_query_entry(u16 vpa_entry);

/**
 * sstar_miu_mmu_enable - Query which pa_entry is mapped to the vpa_entry
 * @enable: 1 for enable, 0 for disable
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_mmu_enable(Bool enable);

/**
 * sstar_miu_mmu_reset - Mmu SW reset
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_mmu_reset(void);

typedef void (*sstar_mmu_irq_callback)(u32, u16, u16, u8);

/**
 * sstar_mmu_callback_func - Register MMU interrupt callback function
 *
 */
void sstar_mmu_callback_func(sstar_mmu_irq_callback func);
#endif

//=================================================================================================
//                                     MIU Misc Function
//=================================================================================================

/**
 * sstar_miu_dram_info - Get miu dram info
 * @dram: fill this struct
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_dram_info(struct miu_dram_info* dram);

/**
 * sstar_miu_init - Init miu resources
 * must be called before any other func called
 *
 * return: 0 on success, < 1 on error
 */
int sstar_miu_init(void);

#if defined(CAM_OS_RTK)
/**
 * sstar_miu_phy_to_miu - Address transformation from physical domain to MIU one
 * @phy_addr: physical address
 *
 * return: transformed MIU address
 */
ss_miu_addr_t sstar_miu_phy_to_miu(ss_phys_addr_t phy_addr);

/**
 * sstar_miu_addr_to_phy - Address transformation from MIU domain to physical one
 * @miu_addr: MIU address
 *
 * return: transformed physical address
 */
ss_phys_addr_t sstar_miu_addr_to_phy(ss_miu_addr_t miu_addr);

#if defined(CONFIG_MIU_BWLA)
int sstar_miu_bw_show(char* buf, int offset, int n);
int sstar_miu_bw_set_rounds(int round);
#endif
#endif

#endif // #ifndef __MDRV_MIU_H__
