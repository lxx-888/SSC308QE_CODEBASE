/*
 * miu_ut.c - Sigmastar
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

#include <linux/device.h>
#include "MsTypes.h"
#include "mdrv_types.h"

#include "miu_ut.h"

// static miu_ut_memory ut_memory;
static miu_mmu_info mmu_info;
static unsigned int mmu_irq_status;
static u32          page_modes[PAGE_SIZE_NUM] = {E_MMU_PGSZ_32, E_MMU_PGSZ_64, E_MMU_PGSZ_128};
//==============================================================================
//                          help function
//==============================================================================
bool mmu_entry_query_and_check(unsigned int entry, unsigned int expect_result)
{
    unsigned int map_entry = sstar_miu_mmu_query_entry(entry);
    if (map_entry == -1)
    {
        SSTAR_UT_ERR("entry %d sstar_miu_mmu_query_entry failed.\n", entry);
        return UT_FAIL;
    }
    if (map_entry != expect_result)
    {
        SSTAR_UT_ERR("entry %d value %d (expected %d)\n", entry, map_entry, expect_result);
        return UT_FAIL;
    }
    return UT_PASS;
}

ss_miu_addr_t miu_addr_to_region(ss_miu_addr_t addr)
{
    return addr >> (HAL_MIU_MMU_ENTRY_BIT + mmu_info.page_bits);
}

ss_miu_addr_t miu_addr_to_entry(ss_miu_addr_t addr)
{
    return (addr >> mmu_info.page_bits) & (mmu_info.max_entry - 1);
}

unsigned int* virt_addr_shift(unsigned int* virt, unsigned long long shift)
{
    unsigned long long int_virt = (unsigned long long)virt;
    return (unsigned int*)(int_virt + shift);
}

void set_miu_mmu_info(int page_bits)
{
    mmu_info.page_bits = page_bits;
    mmu_info.page_size = 1 << page_bits;

    mmu_info.entry_bits = HAL_MIU_MMU_ENTRY_BIT;
    mmu_info.entry_size = mmu_info.page_size;
    mmu_info.max_entry  = 1 << HAL_MIU_MMU_ENTRY_BIT;

    mmu_info.region_bits = MIU_MMU_ADDR_BIT - HAL_MIU_MMU_ENTRY_BIT - page_bits;
    mmu_info.region_size = 1 << (page_bits + HAL_MIU_MMU_ENTRY_BIT);
    mmu_info.max_region  = 1 << mmu_info.region_bits;
}

void set_miu_mmu_page_mode(int page_mode)
{
    switch (page_mode)
    {
        case E_MMU_PGSZ_32:
            set_miu_mmu_info(15);
            break;
        case E_MMU_PGSZ_64:
            set_miu_mmu_info(16);
            break;
        case E_MMU_PGSZ_128:
            set_miu_mmu_info(17);
            break;
        default:
            SSTAR_UT_ERR("Error page_mode = %d \n", page_mode);
            break;
    }
    sstar_miu_mmu_set_page_size(page_mode);
}

unsigned int get_mmu_map_start_region(void)
{
    unsigned int         region;
    struct miu_dram_info dram_info;
    sstar_miu_dram_info(&dram_info);
    region = dram_info.size / mmu_info.region_size + 1;
    SSTAR_UT_INFO("dram_info.size: 0x%llX, mmu_info.region_size: %u\n", dram_info.size, mmu_info.region_size);
    return region;
}

bool map_miu_ut_memory(miu_ut_memory* mem, unsigned int region)
{
    // unsigned long long offset;
    mem->virt_phy_addr = mem->phy_addr;

    if (region > mem->region)
        mem->virt_phy_addr += (ss_phys_addr_t)(region - mem->region) * mmu_info.region_size;
    else if (region < mem->region)
        mem->virt_phy_addr -= (ss_phys_addr_t)(mem->region - region) * mmu_info.region_size;

    SSTAR_UT_INFO("phy addr: 0x%llX, virt phy addr: 0x%llX\n", mem->phy_addr, mem->virt_phy_addr);

    if (mem->virt_addr)
        CamOsMemUnmap(mem->virt_addr, mem->size);

    mem->virt_addr = (unsigned int*)CamOsMemMap(mem->virt_phy_addr, mem->size, 0);
    if (mem->virt_addr == 0)
    {
        SSTAR_UT_ERR("CamOsMemMap fail!\n");
        return UT_FAIL;
    }

    // mem->virt_addr = mem->virt_addr;
    // if (mem->miu_addr % mmu_info.page_size != 0)
    // {
    //     SSTAR_UT_ERR("align: %u, offset: %llu  %llu\n", (unsigned int)mem->virt_addr, mem->miu_addr %
    //     mmu_info.page_size, offset); offset = mmu_info.page_size - (mem->miu_addr % mmu_info.page_size);
    //     mem->virt_addr = virt_addr_shift(mem->virt_addr, offset);
    // }

    return UT_PASS;
}

// The allocated memory may not be aligned with the page, like,
//  [mem]:    |----------------------|
// [page]: |---|---|---|---|---|---|---|
// so, we only use the memory covering the complete page for testing, eg.
//  [mem]:     |-------------------|
// [page]: |---|---|---|---|---|---|---|
bool alloc_miu_ut_memory(miu_ut_memory* mem, unsigned int size)
{
    memset(mem, 0, sizeof(miu_ut_memory));
    mem->size     = size;
    mem->phy_addr = CamOsContiguousMemAlloc(size);
    if (mem->phy_addr == 0)
    {
        SSTAR_UT_ERR("CamOsContiguousMemAlloc fail!\n");
        return UT_FAIL;
    }

    mem->miu_addr = CamOsMemPhysToMiu(mem->phy_addr);
    if (mem->miu_addr == 0)
    {
        SSTAR_UT_ERR("CamOsMemPhysToMiu fail!\n");
        goto ALLOC_FAIL;
    }

    mem->region = miu_addr_to_region(mem->miu_addr);
    if (mem->region != miu_addr_to_region(mem->miu_addr + size))
    {
        SSTAR_UT_ERR("The allocated memory is not in the same region!\n");
        goto ALLOC_FAIL;
    }

    if (map_miu_ut_memory(mem, mem->region) == UT_FAIL)
    {
        SSTAR_UT_ERR("map_miu_ut_memory() failed!\n");
        goto ALLOC_FAIL;
    }

    mem->miu_entry   = miu_addr_to_entry(mem->miu_addr);
    mem->cover_entry = MMU_TEST_MEM_SIZE / mmu_info.page_size;
    // if (mem->miu_addr % mmu_info.page_size != 0)
    // {
    //     mem->miu_entry += 1;
    //     mem->cover_entry -= 2;
    // }
    mem->cover_entry -= mem->cover_entry & 1; // make sure entry num is even

    memset(mem->virt_addr, 0, size);
    SSTAR_UT_INFO("size: %u, phy: 0x%llX, miu: 0x%llX, virtual: 0x%X, region: %d, aligned: 0x%X, entry: %llu\n",
                  mem->size, mem->phy_addr, mem->miu_addr, (unsigned int)mem->virt_addr, mem->region,
                  (unsigned int)mem->virt_addr, mem->miu_entry);

    return UT_PASS;

ALLOC_FAIL:
    free_miu_ut_memory(mem);
    return UT_FAIL;
}

void free_miu_ut_memory(miu_ut_memory* mem)
{
    if (mem->virt_addr)
        CamOsMemUnmap(mem->virt_addr, mem->size);
    CamOsContiguousMemRelease(mem->phy_addr);
}

//==============================================================================
//                          mmu entry suite
//==============================================================================
bool mmu_entry_access(sstar_ut_test* test)
{
    unsigned int i;

    for (i = 0; i < mmu_info.max_entry; i++)
    {
        if (mmu_entry_query_and_check(i, HAL_MIU_MMU_INVALID_ENTRY) == UT_FAIL)
            return UT_FAIL;
    }

    return UT_PASS;
}

bool mmu_entry_access_with_map(sstar_ut_test* test)
{
    unsigned int i, j;

    for (i = 0; i < mmu_info.max_entry; i++)
    {
        j = mmu_info.max_entry - i - 1;
        sstar_miu_mmu_map_entry(i, j);
        if (mmu_entry_query_and_check(i, j) == UT_FAIL)
            return UT_FAIL;
    }

    return UT_PASS;
}

//==============================================================================
//                          mmu map suite
//==============================================================================
static bool mmu_map_check(miu_ut_memory* mem, int mmu_enable)
{
    int           i;
    unsigned int* virt_addr;
    unsigned int  miu_entry, map_entry;

    for (i = 0; i < mem->cover_entry; ++i)
    {
        miu_entry = mem->miu_entry + i;
        map_entry = mem->miu_entry + (i ^ mmu_enable);

        if (mmu_entry_query_and_check(miu_entry, map_entry) == UT_FAIL)
        {
            SSTAR_UT_ERR("mmu_entry_query_and_check() fialed.\n");
            return UT_FAIL;
        }
        virt_addr = virt_addr_shift(mem->virt_addr, mmu_info.page_size * i);
        if (*virt_addr != map_entry)
        {
            SSTAR_UT_ERR("virtual address 0x%X value %u (expected %u), mmu enable: %d\n", (unsigned int)virt_addr,
                         *virt_addr, map_entry, mmu_enable);
            return UT_FAIL;
        }
        SSTAR_UT_INFO("virtual address 0x%X value %u (expected %u), mmu enable: %d\n", (unsigned int)virt_addr,
                      *virt_addr, map_entry, mmu_enable);
    }

    return UT_PASS;
}

static bool mmu_map_and_assign_value(miu_ut_memory* mem)
{
    int           i;
    unsigned int  miu_entry, map_entry;
    unsigned int* virt_addr;

    // map all entry in place
    sstar_miu_mmu_set_region(0, 0);
    for (i = 0; i < mmu_info.max_entry; ++i)
    {
        sstar_miu_mmu_map_entry(i, i);
    }

    // (i ^ 1) invert the lowest bit of i,
    // eg. (0^1=1, 1^1=0), (2^1=3, 3^1=2), (4^1=5, 5^1=4), ...
    for (i = 0; i < mem->cover_entry; ++i)
    {
        miu_entry  = mem->miu_entry + i;
        map_entry  = mem->miu_entry + (i ^ 1);
        virt_addr  = virt_addr_shift(mem->virt_addr, mmu_info.page_size * i);
        *virt_addr = miu_entry;

        sstar_miu_mmu_map_entry(miu_entry, map_entry);
        if (mmu_entry_query_and_check(miu_entry, map_entry) == UT_FAIL)
        {
            SSTAR_UT_ERR("mmu_entry_query_and_check() fialed.\n");
            return UT_FAIL;
        }
        SSTAR_UT_INFO("virtual 0x%X, val=%d, map entry %u to entry %u\n", (unsigned int)virt_addr, *virt_addr,
                      miu_entry, map_entry);
    }
    return UT_PASS;
}

bool mmu_map_region_with_check(sstar_ut_test* test)
{
    int           i;
    bool          result = UT_FAIL;
    unsigned int  start_region;
    miu_ut_memory mem;

    u32* page_mode = (u32*)(test->user_data);

    set_miu_mmu_page_mode(*page_mode);
    // set_miu_mmu_info(16);
    // sstar_miu_mmu_set_page_size(1);

    start_region = get_mmu_map_start_region();
    SSTAR_UT_INFO("start_region=%X, pagesize=%X\r\n", start_region, *page_mode);

    if (alloc_miu_ut_memory(&mem, MMU_TEST_MEM_SIZE) == UT_FAIL)
    {
        SSTAR_UT_ERR("Failed to alloc memory for mmu map test!\n");
        return UT_FAIL;
    }

    if (mmu_map_and_assign_value(&mem) == UT_FAIL)
    {
        SSTAR_UT_ERR("mmu_map_and_assign_value() failed!\n");
        return UT_FAIL;
    }

    SSTAR_UT_INFO("max_region: %u , start_region %u, virtual=0x%X\n", mmu_info.max_region, start_region,
                  (unsigned int)mem.virt_addr);
    // for (i = 2; i < 6; i++)
    {
        i = mem.region + 1;

        sstar_miu_mmu_enable(0);
        sstar_miu_mmu_set_region(i, mem.region);
        SSTAR_UT_ERR("MMU set region for region %u to %u\n", mem.region, i);
        // printk("[%s %d] virtual=0x%X\r\n", __FUNCTION__, __LINE__, (unsigned int)mem.virt_addr);
        map_miu_ut_memory(&mem, i);
        // printk("[%s %d] virtual=0x%X\r\n", __FUNCTION__, __LINE__, (unsigned int)mem.virt_addr);
        sstar_miu_mmu_enable(1);

        SSTAR_UT_INFO("MMU enable\n");
        if (mmu_map_check(&mem, 1) == UT_FAIL)
        {
            SSTAR_UT_ERR("mmu_map_check() failed with mmu_enable=1.\n");
            goto MMU_MAP_AND_CHECK_FAIL;
        }
    }

    result = UT_PASS;

MMU_MAP_AND_CHECK_FAIL:
    free_miu_ut_memory(&mem);
    sstar_miu_mmu_enable(0);

    return result;
}

// todo: check all page size
bool mmu_map_all_region_no_check(sstar_ut_test* test)
{
    int          i;
    unsigned int start_region;
    u32*         page_mode = (u32*)(test->user_data);

    set_miu_mmu_page_mode(*page_mode);

    start_region = get_mmu_map_start_region();

    sstar_miu_mmu_set_region(0, 0);
    for (i = 0; i < mmu_info.max_entry; ++i)
    {
        sstar_miu_mmu_map_entry(i, i);
    }

    SSTAR_UT_INFO("max_region: %u , start_region %u\n", mmu_info.max_region, start_region);

    for (i = mmu_info.max_region - 1; i >= start_region; --i)
    {
        sstar_miu_mmu_enable(0);
        sstar_miu_mmu_set_region(i, 0);
        sstar_miu_mmu_enable(1);
    }

    return UT_PASS;
}

//==============================================================================
//                          mmu interrupt suite
//==============================================================================

void mmu_interrupt_callback(unsigned int status, unsigned short phy_entry, unsigned short client_id,
                            unsigned char is_write_cmd)
{
    mmu_irq_status = status;
    SSTAR_UT_INFO("mmu_irq_status=0x%x, phy_entry=0x%x, client_id=0x%x, is_write_cmd=%d\n", status, phy_entry,
                  client_id, is_write_cmd);
}

bool mmu_interrupt_rw_invalid(sstar_ut_test* test)
{
    int           i;
    bool          result = UT_FAIL;
    miu_ut_memory mem;
    // unsigned int   status;
    // unsigned short phy_entry, client_id;
    // unsigned char  is_write_cmd = 0; // I6C no need
    u32* page_mode = (u32*)(test->user_data);

    set_miu_mmu_page_mode(*page_mode);

    sstar_mmu_callback_func(mmu_interrupt_callback);

    if (alloc_miu_ut_memory(&mem, MMU_TEST_MEM_SIZE) == UT_FAIL)
    {
        SSTAR_UT_ERR("Failed to alloc memory for mmu map test!\n");
        return UT_FAIL;
    }

    if (mmu_map_and_assign_value(&mem) == UT_FAIL)
    {
        SSTAR_UT_ERR("mmu_map_and_assign_value() failed!\n");
        return UT_FAIL;
    }

    sstar_miu_mmu_set_region(mem.region, mem.region);
    sstar_miu_mmu_enable(1);

    sstar_miu_mmu_unmap_entry(mem.miu_entry);
    SSTAR_UT_INFO("Unmap entry: 0x%llu\r\n", mem.miu_entry);

    i = *(mem.virt_addr);
    msleep(10);
    if (mmu_irq_status != 2)
    {
        SSTAR_UT_ERR("mmu interrupt read invalid test failed!, vir_addr(0x%X)=%d\n", (unsigned int)mem.virt_addr,
                     *(mem.virt_addr));
        goto INTERRUPT_RW_INVALID_FAIL;
    }

    *(mem.virt_addr) = i + 1;
    msleep(10);
    if (mmu_irq_status != 4)
    {
        SSTAR_UT_ERR("mmu interrupt write invalid test failed!\n");
        goto INTERRUPT_RW_INVALID_FAIL;
    }

    result = UT_PASS;

INTERRUPT_RW_INVALID_FAIL:
    sstar_miu_mmu_enable(0);
    free_miu_ut_memory(&mem);
    return result;
}

bool mmu_interrupt_rw_collision(sstar_ut_test* test)
{
    int           i, j;
    bool          result = UT_FAIL;
    miu_ut_memory mem;
    unsigned int  miu_entry, map_entry;
    // CamOsThread    bdma_fill_thread;
    u32* page_mode = (u32*)(test->user_data);

    set_miu_mmu_page_mode(*page_mode);

    sstar_mmu_callback_func(mmu_interrupt_callback);

    if (alloc_miu_ut_memory(&mem, MMU_TEST_MEM_SIZE) == UT_FAIL)
    {
        SSTAR_UT_ERR("Failed to alloc memory for mmu map test!\n");
        return UT_FAIL;
    }

    if (mmu_map_and_assign_value(&mem) == UT_FAIL)
    {
        SSTAR_UT_ERR("mmu_map_and_assign_value() failed!\n");
        return UT_FAIL;
    }

    sstar_miu_mmu_set_region(mem.region, mem.region);
    sstar_miu_mmu_enable(1);

    msys_set_miubist("MIU_BIST_ALL", mem.phy_addr, mem.size);

    for (j = 0; j < 50000; ++j)
    {
        for (i = 0; i < mem.cover_entry; ++i)
        {
            miu_entry = mem.miu_entry + i;
            map_entry = mem.miu_entry + (i ^ (j % 2));
            sstar_miu_mmu_map_entry(miu_entry, map_entry);
        }

        if (mmu_irq_status == 1)
        {
            // SSTAR_UT_ERR("mmu interrupt read/write collision test failed!\n");
            result = UT_PASS;
            break;
        }
    }

    msys_set_miubist("MIU_BIST_OFF", mem.phy_addr, mem.size);

    free_miu_ut_memory(&mem);

    return result;
}

//==============================================================================
//                          miu/mmu protect suite
//==============================================================================

bool miu_mmu_protect_enable(U16* protect_id, U8 mmu, int page_mode)
{
    int            i;
    miu_ut_memory  mem;
    U8             block, start_block, end_block;
    U32            block_size;
    ss_phys_addr_t bus_start;
    ss_phys_addr_t bus_end;
    // BOOL           set_flag    = TRUE;
    // BOOL           id_flag     = TRUE;
    // BOOL           invert_flag = FALSE;
    bool result = UT_FAIL;
    U32* virt_addr;

    set_miu_mmu_page_mode(page_mode);

    if (alloc_miu_ut_memory(&mem, MMU_TEST_MEM_SIZE) == UT_FAIL)
    {
        SSTAR_UT_ERR("Failed to alloc memory for miu protect whitelist!\n");
        return UT_FAIL;
    }

    if (mmu)
    {
        start_block = HAL_MIU_MMU_PROTECT_MIN;
        end_block   = HAL_MIU_PROTECT_MAX_BLOCK - 1;
        block_size  = (mem.size / (end_block - start_block + 1)) & MIU_PROTECT_ALIGNED_MASK;
        sstar_miu_mmu_set_region(mem.region, mem.region);
        for (i = 0; i < mmu_info.max_entry; ++i)
        {
            sstar_miu_mmu_map_entry(i, i);
        }
        map_miu_ut_memory(&mem, mem.region);
        sstar_miu_mmu_enable(1);
    }
    else
    {
        start_block = 0;
        end_block   = HAL_MIU_MMU_PROTECT_MIN - 1;
        block_size  = (mem.size / (end_block - start_block + 1)) & MIU_PROTECT_ALIGNED_MASK;
    }

    virt_addr = mem.virt_addr;
    end_block = start_block + 2;
    for (block = start_block; block < end_block; ++block)
    {
        bus_start = mem.virt_phy_addr + (block - start_block) * block_size;
        bus_end   = bus_start + block_size;

        if (sstar_miu_protect_enable(block, protect_id, 0, bus_start, bus_end) == -1)
        {
            SSTAR_UT_ERR("sstar_miu_protect_enable() for block %d failed, bus_start: 0x%llX, bus_end: 0x%llX\n", block,
                         bus_start, bus_end);
            goto MIU_PROTECT_FAIL;
        }

        // if (!MDrv_MIU_Protect_Add_Ext_Feature(mmu, protect_id, block, set_flag, id_flag, invert_flag))
        // {
        //     SSTAR_UT_ERR("MDrv_MIU_Protect_Add_Ext_Feature() for block %d failed, bus_start: 0x%llX, bus_end:
        //     0x%llX\n",
        //                  block, bus_start, bus_end);
        //     goto MIU_PROTECT_FAIL;
        // }
        SSTAR_UT_INFO("virt_addr(0x%X) write to %X\r\n", (unsigned int)virt_addr, mem.size);
        *virt_addr = mem.size;
        if (*virt_addr != mem.size)
        {
            SSTAR_UT_ERR("Failed to write and read protect block %d, bus_start: 0x%llX, bus_end: 0x%llX\n", block,
                         bus_start, bus_end);
            goto MIU_PROTECT_FAIL;
        }
        virt_addr = virt_addr_shift(mem.virt_addr, block_size);
    }

    for (block = start_block; block < end_block; ++block)
    {
        bus_start = mem.virt_phy_addr + (block - start_block) * block_size;
        bus_end   = bus_start + block_size;
        if (sstar_miu_protect_disable(block) == -1)
        {
            SSTAR_UT_ERR("sstar_miu_protect_disable() for block %d failed, bus_start: 0x%llX, bus_end: 0x%llX\n", block,
                         bus_start, bus_end);
            goto MIU_PROTECT_FAIL;
        }
    }

    result = UT_PASS;

MIU_PROTECT_FAIL:
    free_miu_ut_memory(&mem);
    sstar_miu_mmu_enable(0);

    return result;
}

bool miu_protect_whitelist(sstar_ut_test* test)
{
    U16  protect_id[] = {CPU_CLIENT_ID, 0};
    u32* page_mode    = (u32*)(test->user_data);
    return miu_mmu_protect_enable(protect_id, 0, *page_mode);
}

bool miu_protect_blacklist(sstar_ut_test* test)
{
    U16  protect_id[] = {0};
    u32* page_mode    = (u32*)(test->user_data);
    return miu_mmu_protect_enable(protect_id, 0, *page_mode);
}

bool mmu_protect_whitelist(sstar_ut_test* test)
{
    U16  protect_id[] = {CPU_CLIENT_ID, 0};
    u32* page_mode    = (u32*)(test->user_data);
    return miu_mmu_protect_enable(protect_id, 1, *page_mode);
}

bool mmu_protect_blacklist(sstar_ut_test* test)
{
    U16  protect_id[] = {0};
    u32* page_mode    = (u32*)(test->user_data);
    return miu_mmu_protect_enable(protect_id, 1, *page_mode);
}

//==============================================================================
//                          main flow
//==============================================================================
static sstar_ut* miu_ut;

void ut_init(sstar_ut* ut)
{
    sstar_miu_mmu_reset();
    msleep(10);
    sstar_miu_protect_set_panic(0);
}

void ut_exit(sstar_ut* ut)
{
    sstar_miu_mmu_enable(0);
    sstar_miu_protect_set_panic(1);
}

// Setup test, the newly test items are uniformly added in this function
void miu_ut_setup(void)
{
    sstar_ut_suite* suite;

    miu_ut = sstar_ut_create("MIU", ut_init, ut_exit, NULL);

    suite = sstar_ut_add_suite_lite(miu_ut, "mmu_entry");
    sstar_ut_add_test_lite(suite, mmu_entry_access);
    sstar_ut_add_test_lite(suite, mmu_entry_access_with_map);

    suite = sstar_ut_add_suite_lite(miu_ut, "mmu_map");
    sstar_ut_add_test(suite, "mmu_map_all_region_no_check_32KB", mmu_map_all_region_no_check, false, NULL, page_modes);
    sstar_ut_add_test(suite, "mmu_map_all_region_no_check_64KB", mmu_map_all_region_no_check, false, NULL,
                      page_modes + 1);
    sstar_ut_add_test(suite, "mmu_map_all_region_no_check_128KB", mmu_map_all_region_no_check, false, NULL,
                      page_modes + 2);

    sstar_ut_add_test(suite, "mmu_map_region_with_check_32KB", mmu_map_region_with_check, false, NULL, page_modes);
    sstar_ut_add_test(suite, "mmu_map_region_with_check_64KB", mmu_map_region_with_check, false, NULL, page_modes + 1);
    sstar_ut_add_test(suite, "mmu_map_region_with_check_128KB", mmu_map_region_with_check, false, NULL, page_modes + 2);

    suite = sstar_ut_add_suite_lite(miu_ut, "mmu_interrupt");
    sstar_ut_add_test(suite, "mmu_interrupt_rw_invalid_32KB", mmu_interrupt_rw_invalid, false, NULL, page_modes);
    sstar_ut_add_test(suite, "mmu_interrupt_rw_invalid_64KB", mmu_interrupt_rw_invalid, false, NULL, page_modes + 1);
    sstar_ut_add_test(suite, "mmu_interrupt_rw_invalid_128KB", mmu_interrupt_rw_invalid, false, NULL, page_modes + 2);

    sstar_ut_add_test(suite, "mmu_interrupt_rw_collision_32KB", mmu_interrupt_rw_collision, false, NULL, page_modes);
    sstar_ut_add_test(suite, "mmu_interrupt_rw_collision_64KB", mmu_interrupt_rw_collision, false, NULL,
                      page_modes + 1);
    sstar_ut_add_test(suite, "mmu_interrupt_rw_collision_128KB", mmu_interrupt_rw_collision, false, NULL,
                      page_modes + 2);

    suite = sstar_ut_add_suite_lite(miu_ut, "miu_mmu_protect");
    sstar_ut_add_test(suite, "miu_protect_whitelist_32KB", miu_protect_whitelist, false, NULL, page_modes);
    sstar_ut_add_test(suite, "miu_protect_whitelist_64KB", miu_protect_whitelist, false, NULL, page_modes + 1);
    sstar_ut_add_test(suite, "miu_protect_whitelist_128KB", miu_protect_whitelist, false, NULL, page_modes + 2);

    sstar_ut_add_test(suite, "miu_protect_blacklist_32KB", miu_protect_blacklist, false, NULL, page_modes);
    sstar_ut_add_test(suite, "miu_protect_blacklist_64KB", miu_protect_blacklist, false, NULL, page_modes + 1);
    sstar_ut_add_test(suite, "miu_protect_blacklist_128KB", miu_protect_blacklist, false, NULL, page_modes + 2);

    sstar_ut_add_test(suite, "mmu_protect_whitelist_32KB", mmu_protect_whitelist, false, NULL, page_modes);
    sstar_ut_add_test(suite, "mmu_protect_whitelist_64KB", mmu_protect_whitelist, false, NULL, page_modes + 1);
    sstar_ut_add_test(suite, "mmu_protect_whitelist_128KB", mmu_protect_whitelist, false, NULL, page_modes + 2);

    sstar_ut_add_test(suite, "mmu_protect_blacklist_32KB", mmu_protect_blacklist, false, NULL, page_modes);
    sstar_ut_add_test(suite, "mmu_protect_blacklist_64KB", mmu_protect_blacklist, false, NULL, page_modes + 1);
    sstar_ut_add_test(suite, "mmu_protect_blacklist_128KB", mmu_protect_blacklist, false, NULL, page_modes + 2);
}

// static int miu_ut_read(struct device* dev, struct device_attribute* attr, char* buf)
// {
//     sstar_ut_read(ut);

//     return 0;
// }

// static int miu_ut_write(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
// {
//     sstar_ut_write(ut, buf, count);

//     return count;
// }

SSTAR_UT_ATTR(miu_ut);
DEVICE_ATTR(miu_ut, 0644, sstar_show_miu_ut, sstar_store_miu_ut);

struct file_operations miu_ut_ops = {
    .owner = THIS_MODULE,
};

static int            miu_major;
static struct class*  miu_class;
static struct device* miu_ut_dev;

static int __init miu_ut_init(void)
{
    miu_ut_setup();
    miu_major = register_chrdev(0, "miu", &miu_ut_ops);
    if (miu_major < 0)
    {
        printk("[MIU UT] cannot register miu_ut (err=%d)\n", miu_major);
        return -1;
    }
    miu_class  = class_create(THIS_MODULE, "miu");
    miu_ut_dev = device_create(miu_class, 0, MKDEV(miu_major, 0), NULL, "ut");
    device_create_file(miu_ut_dev, &dev_attr_miu_ut);
    printk("[MIU UT] init.\n");

    return 0;
}

static void __exit miu_ut_exit(void)
{
    device_destroy(miu_class, MKDEV(miu_major, 0));
    class_destroy(miu_class);
    unregister_chrdev(miu_major, "ut");
    sstar_ut_free(miu_ut);
    printk("[MIU UT] exit.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIGMASTAR");
MODULE_DESCRIPTION("MIU UT driver");
module_init(miu_ut_init);
module_exit(miu_ut_exit);
