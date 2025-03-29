/*
 * drv_miu.c - Sigmastar
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

#include <irqs.h>
#include <drv_miu.h>
#include "drv_miu_defs.h"

#if defined(CAM_OS_LINUX_KERNEL)
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#elif defined(CAM_OS_RTK)
#include <string.h>
#include <initcall.h>
#include <cpu_mem_map.hc>
#endif

//=================================================================================================
//                                     MIU Device Function
//=================================================================================================

static int miuSelId = 1;
#if defined(CONFIG_MIU_HW_MMU)
static int mmuSelId = 2;
#endif

#ifdef CONFIG_PM_SLEEP
static unsigned int miu_irq = 0;
#if defined(CONFIG_MIU_HW_MMU)
static unsigned int mmu_irq = 0;
#endif
#endif

static struct miu_device* __miu_dev__;
// Do not allow direct access to __miu_dev__
// __miu_dev__ must be obtained through the following APIs:
struct miu_device* sstar_get_miu_device(void)
{
    return __miu_dev__;
}

#if defined(CAM_OS_LINUX_KERNEL)
struct miu_device* sstar_to_miu_device(struct device* dev)
{
    return dev_get_drvdata(dev);
}
#endif

void sstar_set_miu_device(struct miu_device* mdev)
{
    __miu_dev__ = mdev;
}
//=================================================================================================
//                                     MIU Client Function
//=================================================================================================
unsigned char sstar_miu_module_reset(char* ClientName, Bool write)
{
    struct miu_device*   mdev = sstar_get_miu_device();
    enum hal_miu_rw_mode mode = write ? HAL_MIU_WO : HAL_MIU_RO;
    int                  ret;

    ret = mdev->client.callback.name_to_id(&mdev->client, ClientName, mode);

    if (ret != -1)
    {
        ret = mdev->client.callback.module_reset(&mdev->client, ret, mode);
        if (ret == -1)
        {
            sstar_miu_err("Client %s is not support!!!\n", ClientName);
            return ret;
        }
    }
    else
    {
        sstar_miu_err("Client %s is wrong naming!!!\n", ClientName);
        return ret;
    }

    return 0;
}
MIU_EXPORT_SYMBOL(sstar_miu_module_reset);

unsigned char sstar_miu_module_reset_byid(miu_client_id ClientID, Bool write)
{
    struct miu_device*   mdev = sstar_get_miu_device();
    enum hal_miu_rw_mode mode = write ? HAL_MIU_WO : HAL_MIU_RO;
    int                  ret  = 0;

    ret = mdev->client.callback.module_reset(&mdev->client, ClientID, mode);
    if (ret == -1)
    {
        sstar_miu_err("Client %d is not support!!!\n", ClientID);
        return ret;
    }

    return 0;
}
MIU_EXPORT_SYMBOL(sstar_miu_module_reset_byid);

char* sstar_miu_client_id_to_name(u16 id, Bool write)
{
    struct miu_device*   mdev = sstar_get_miu_device();
    enum hal_miu_rw_mode mode = write ? HAL_MIU_WO : HAL_MIU_RO;

    return mdev->client.callback.id_to_name(&mdev->client, id, mode);
}
MIU_EXPORT_SYMBOL(sstar_miu_client_id_to_name);

//=================================================================================================
//                                     MIU Client Function
//================================================================================================

int sstar_miu_dram_info(struct miu_dram_info* dram)
{
    hal_miu_dram_info(dram);

    return 0;
}
MIU_EXPORT_SYMBOL(sstar_miu_dram_info);

//=================================================================================================
//                                     MIU Protect Function
//=================================================================================================

void sstar_miu_protect_interrupt(u32 irq, void* dev_id)
{
    struct miu_device*              mdev = sstar_get_miu_device();
    struct hal_miu_protect_hit_info info = {0};

    mdev->protect.callback.get_hit_info(&mdev->protect, &info);

    if (info.hit)
    {
        mdev->protect.callback.show_hit_info(&mdev->protect, &info);

        if (mdev->protect_panic_on)
        {
            CamOsPanic("MIU Protect Hit!!!");
        }
    }
}

u8 sstar_miu_protect_query(Bool mmu)
{
    struct miu_device* mdev = sstar_get_miu_device();

    return mdev->protect.callback.query(&mdev->protect, mmu);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_query);

int sstar_miu_protect_enable(u8 index, u16* whitelist, Bool invert, ss_phys_addr_t start, ss_phys_addr_t end)
{
    struct miu_device*           mdev  = sstar_get_miu_device();
    struct hal_miu_protect_block block = {
        .index      = index,
        .enable     = true,
        .invert     = invert,
        .white_num  = 0,
        .whitelist  = whitelist,
        .start_addr = start,
        .end_addr   = end,
    };

    while (whitelist && whitelist[block.white_num++] != HAL_MIU_PROTECT_INVLID_ID)
        ;

    return mdev->protect.callback.enable(&mdev->protect, &block);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_enable);

int sstar_miu_protect_disable(u8 index)
{
    struct miu_device* mdev = sstar_get_miu_device();

    return mdev->protect.callback.disable(&mdev->protect, index);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_disable);

int sstar_miu_protect_append_id(u8 index, u16 id)
{
    struct miu_device* mdev = sstar_get_miu_device();

    return mdev->protect.callback.append_id(&mdev->protect, index, id);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_append_id);

int sstar_miu_protect_remove_id(u8 index, u16 id)
{
    struct miu_device* mdev = sstar_get_miu_device();

    return mdev->protect.callback.remove_id(&mdev->protect, index, id);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_remove_id);

void sstar_miu_protect_set_panic(Bool enable)
{
    struct miu_device* mdev = sstar_get_miu_device();
    mdev->protect_panic_on  = enable;
    sstar_miu_info("protect_panic_on: %d\n", mdev->protect_panic_on);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_set_panic);

u16* sstar_miu_protect_list(u8 index, Bool* invert)
{
    struct miu_device*            mdev  = sstar_get_miu_device();
    struct hal_miu_protect_block* block = mdev->protect.callback.get_block(&mdev->protect, index);

    if (!block)
    {
        return NULL;
    }

    if (invert)
    {
        *invert = block->invert;
    }

    return block->whitelist;
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_list);

Bool sstar_miu_protect_status(u8 index)
{
    struct miu_device*            mdev  = sstar_get_miu_device();
    struct hal_miu_protect_block* block = mdev->protect.callback.get_block(&mdev->protect, index);

    if (!block)
    {
        return false;
    }

    return block->enable;
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_status);

int sstar_miu_protect_address(u8 index, ss_phys_addr_t* start, ss_phys_addr_t* end)
{
    struct miu_device*            mdev  = sstar_get_miu_device();
    struct hal_miu_protect_block* block = mdev->protect.callback.get_block(&mdev->protect, index);

    if (!block || !start || !end)
    {
        return -1;
    }

    *start = block->start_addr;
    *end   = block->end_addr;

    return 0;
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_address);

u16* sstar_miu_protect_default_kernel_whitelist(void)
{
    struct miu_device* mdev = sstar_get_miu_device();
    return mdev->protect.callback.get_kernel_whitelist(&mdev->protect);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_default_kernel_whitelist);

u16* sstar_miu_protect_default_kernel_blacklist(void)
{
    struct miu_device* mdev = sstar_get_miu_device();
    return mdev->protect.callback.get_kernel_blacklist(&mdev->protect);
}
MIU_EXPORT_SYMBOL(sstar_miu_protect_default_kernel_blacklist);
//=================================================================================================
//                                     MIU MMU Function
//=================================================================================================
#if defined(CONFIG_MIU_HW_MMU)
void sstar_miu_mmu_interrupt(u32 irq, void* dev_id)

{
    u32                status;
    u16                entry, id;
    u8                 is_write;
    struct miu_device* mdev = sstar_get_miu_device();

    status = mdev->mmu.callback.get_irq_status(&mdev->mmu, &entry, &id, &is_write);
    if (mdev->mmu_irq_callback)
    {
        mdev->mmu_irq_callback(status, entry, id, is_write);
    }
    else
    {
        sstar_miu_err("Status=0x%x, PhyAddrEntry=0x%x, ClientId=0x%x, IsWrite=%d\n", status, entry, id, is_write);
    }
}

int sstar_miu_mmu_set_page_size(u8 page_mode)
{
    int                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.set_page_size(&mdev->mmu, page_mode);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_set_page_size);

u32 sstar_miu_mmu_get_page_size(void)
{
    u32                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.get_page_size(&mdev->mmu);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_get_page_size);

int sstar_miu_mmu_set_region(u16 vpa_region, u16 pa_region)
{
    int                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.set_region(&mdev->mmu, vpa_region, pa_region);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_set_region);

int sstar_miu_mmu_map_entry(u16 vpa_entry, u16 pa_entry)
{
    int                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.map_entry(&mdev->mmu, vpa_entry, pa_entry);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_map_entry);

int sstar_miu_mmu_unmap_entry(u16 vpa_entry)
{
    int                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.unmap_entry(&mdev->mmu, vpa_entry);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_unmap_entry);

u16 sstar_miu_mmu_query_entry(u16 vpa_entry)
{
    u16                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.query_entry(&mdev->mmu, vpa_entry);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_query_entry);

int sstar_miu_mmu_enable(Bool enable)
{
    int                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.enable(&mdev->mmu, enable);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_enable);

int sstar_miu_mmu_reset(void)
{
    int                ret;
    struct miu_device* mdev = sstar_get_miu_device();

    CamOsSpinLockIrqSave(&mdev->mmu_lock);
    ret = mdev->mmu.callback.reset(&mdev->mmu);
    CamOsSpinUnlockIrqRestore(&mdev->mmu_lock);

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_mmu_reset);

void sstar_mmu_callback_func(sstar_mmu_irq_callback func)
{
    struct miu_device* mdev = sstar_get_miu_device();

    mdev->mmu_irq_callback = func;
}
MIU_EXPORT_SYMBOL(sstar_mmu_callback_func);
#endif

//=================================================================================================
//                                     MIU Init Function
//=================================================================================================

static void sstar_miu_device_object_free(struct miu_device* mdev, int end)
{
    int i;

    for (i = end - 1; i >= 0; i--)
    {
        if (mdev->obj_init[i].free)
        {
            mdev->obj_init[i].free(mdev->obj_init[i].obj);
        }
    }
}

static int sstar_miu_device_object_init(void)
{
    int                    i, err;
    struct miu_device*     mdev = sstar_get_miu_device();
    struct miu_object_init dev_init[] =
    {
        {"client", &mdev->client, hal_miu_client_init, hal_miu_client_free},
        {"protect", &mdev->protect, hal_miu_protect_init, hal_miu_protect_free},
        {"dram", &mdev->dram, hal_miu_dram_init, hal_miu_dram_free},
#if defined(CAM_OS_LINUX_KERNEL)
        {"bist", &mdev->bist, hal_miu_bist_init, hal_miu_bist_free},
#endif
#if defined(CONFIG_MIU_BWLA)
        {"bw", &mdev->bw, hal_miu_bw_init, hal_miu_bw_free},
#endif
#if defined(CONFIG_MIU_HW_MMU)
        {"mmu", &mdev->mmu, hal_miu_mmu_init, hal_miu_mmu_free},
#endif
    };

    mdev->obj_num  = sizeof(dev_init) / sizeof(dev_init[0]);
    mdev->obj_init = CamOsMemAlloc(mdev->obj_num * sizeof(struct miu_object_init));

    if (!mdev->obj_init)
    {
        sstar_miu_err("[INIT] alloc memory for obj_init failed!\n");
        return -1;
    }

    CamOsMemcpy(mdev->obj_init, dev_init, mdev->obj_num * sizeof(struct miu_object_init));
    for (i = 0; i < mdev->obj_num; i++)
    {
        if (mdev->obj_init[i].init)
        {
            err = mdev->obj_init[i].init(mdev->obj_init[i].obj, &mdev->client);
            if (err < 0)
            {
                sstar_miu_err("[INIT] miu %s init failed! error = %d\n", mdev->obj_init[i].name, err);
                sstar_miu_device_object_free(mdev, i);
                return -1;
            }
        }
    }
#if defined(CAM_OS_LINUX_KERNEL)
    mdev->client.ip_func[mdev->client.func_num - 1].call(NULL, 0, NULL, 0);
#endif

    return 0;
}

static int sstar_miu_device_init(void)
{
    struct miu_device* mdev = CamOsMemAlloc(sizeof(struct miu_device));
    if (!mdev)
    {
        sstar_miu_err("[INIT] Alloc memory for miu_device failed!\n");
        return -1;
    }

    CamOsMemset(mdev, 0, sizeof(struct miu_device));

    mdev->index            = 0;
    mdev->is_irq_init      = false;
    mdev->protect_panic_on = true;
    mdev->protect_irq_num  = INT_IRQ_MIU;
    CamOsSpinInit(&mdev->mdev_lock);
#if defined(CONFIG_MIU_HW_MMU)
    mdev->mmu_irq_num = INT_IRQ_MMU;
    CamOsSpinInit(&mdev->mmu_lock);
#endif
    sstar_set_miu_device(mdev);

    return 0;
}

static int sstar_miu_device_free(struct miu_device* mdev)
{
    if (!mdev)
    {
        return 0;
    }

#if defined(CAM_OS_LINUX_KERNEL)
    lockdep_unregister_key(&mdev->sub_sysfs.lock_key);
#endif

    sstar_miu_device_object_free(mdev, mdev->obj_num);

    if (mdev->obj_init)
    {
        CamOsMemRelease(mdev->obj_init);
    }

    CamOsMemRelease(mdev);

    return 0;
}

int sstar_miu_irq_real_init(u32* irq_num, char* node, char* irq_name, void* irq_handle, void* dev_id)
{
    int ret = CAM_OS_OK;

#if defined(CAM_OS_LINUX_KERNEL)
    struct device_node* dev_node = of_find_compatible_node(NULL, NULL, node);
    if (!dev_node)
    {
        sstar_miu_err("[INIT] of_find_compatible_node failed\n");
        return -EINVAL;
    }
    *irq_num = irq_of_parse_and_map(dev_node, 0);
#endif

    ret = CamOsIrqRequest(*irq_num, (CamOsIrqHandler)irq_handle, irq_name, dev_id);
    if (ret != CAM_OS_OK)
    {
        sstar_miu_err("[INIT] request miu irq failed\n");
        return ret;
    }

    return ret;
}

static int sstar_miu_irq_init(void)
{
    int ret = CAM_OS_OK;

    struct miu_device* mdev = sstar_get_miu_device();

#if 1 // detect and clear protect interrupts
    mdev->protect.callback.clear_interrupt(&mdev->protect);
#endif

    sstar_miu_info("\n");
    if (mdev->is_irq_init)
    {
        return 0;
    }

    ret = sstar_miu_irq_real_init(&mdev->protect_irq_num, "sstar,miu", "sstar_miu_protect", sstar_miu_protect_interrupt,
                                  &miuSelId);
    if (ret != CAM_OS_OK)
    {
        return ret;
    }
#ifdef CONFIG_PM_SLEEP
    miu_irq = mdev->protect_irq_num;
#endif
#if defined(CONFIG_MIU_HW_MMU)

    ret = sstar_miu_irq_real_init(&mdev->mmu_irq_num, "sstar,mmu", "sstar_mmu", sstar_miu_mmu_interrupt, &mmuSelId);
    if (ret != CAM_OS_OK)
    {
        return ret;
    }
#ifdef CONFIG_PM_SLEEP
    mmu_irq = mdev->mmu_irq_num;
#endif
#endif

    mdev->is_irq_init = true;
    sstar_miu_info("\n");
    return ret;
}

// for MI SYS call
int sstar_miu_init(void)
{
#if defined(CAM_OS_LINUX_KERNEL)
    struct miu_device* mdev = sstar_get_miu_device();
#endif
    int ret = 0;

    sstar_miu_info("\n");
#if defined(CAM_OS_LINUX_KERNEL)
    sstar_miu_run_init(sstar_miu_irq_init, ret);
#endif

    sstar_miu_run_init(hal_miu_init, ret);

    if (ret)
        sstar_miu_err("ret=%d\n", ret);
#if defined(CAM_OS_LINUX_KERNEL)
    // qos rule initialize
    mdev->client.ip_func[mdev->client.func_num - 1].call(NULL, 0, NULL, 0);
#elif defined(CAM_OS_RTK)
    hal_miu_qos_init();
#endif

    return ret;
}
MIU_EXPORT_SYMBOL(sstar_miu_init);

// #define MIU_PROTECT_TEST
#ifdef MIU_PROTECT_TEST // simple test only, dot not remove

#include <hal_miu.h>
#include <ms_msys.h>

void miu_protect_dump_bank(u32 bank)
{
    int i;

    printk(KERN_CONT "BANK:0x%x\n00: ", bank);
    bank = GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, bank << 8);
    for (i = 0; i <= 0x7F; i++)
    {
        if (i && (i & 0x7) == 0)
        {
            printk(KERN_CONT "\n%02x: ", i);
        }
        printk(KERN_CONT "%#06x ", reg_miu_read(bank, i));
    }
}

void miu_protect_test(void)
{
    // u32   i;
    u32*          virt_addr;
    MSYS_DMA_FILL stDmaCfg;
    // struct miu_device* mdev  = sstar_get_miu_device();
    u16 pp1[] = {MIU_CLIENTW_BWLA, MIU_CLIENTW_NULL};
    u16 pp2[] = {MIU_CLIENTW_CA55, MIU_CLIENTW_BDMA, MIU_CLIENTW_NULL};

    sstar_miu_init();
    miu_protect_dump_bank(0x1659);
    sstar_miu_mmu_reset();
    miu_protect_dump_bank(0x1659);
    CamOsMsDelay(20);
    sstar_miu_mmu_set_page_size(2);
    sstar_miu_mmu_set_region(16, 0);
    sstar_miu_mmu_enable(1);

    sstar_miu_mmu_unmap_entry(0);

    printk("protect test 1\r\n");
    sstar_miu_protect_enable(32, pp2, 0, 0x1400000000, 0x1410000000);
    sstar_miu_protect_set_panic(0);

    sstar_miu_err("BDMA begin\n");
    stDmaCfg.phyaddr = (unsigned long long)0x400000000;
    stDmaCfg.length  = 0x10;
    stDmaCfg.pattern = 0xABCD1234;

    // msys_dma_fill(&stDmaCfg);
    // msleep(1);
    // sstar_miu_err("BDMA done\n");

    virt_addr = (unsigned int*)CamOsMemMap(0x1400000000, 0x200, 0);
    if (!virt_addr)
    {
        printk("CamOsMemMap failed\r\n");
    }

    sstar_miu_err("*(u32*)virt_addr: %#x\n", *virt_addr);
    *virt_addr = 0x1000;
    sstar_miu_err("*(u32*)virt_addr + 1: %#x\n", *(virt_addr + 1));
    *(virt_addr + 1) = 0x2000;

    msys_dma_fill(&stDmaCfg);

    // for (; virt_addr < virt_addr + 0x100; virt_addr += 4)
    // {
    //     i = *(u32*)virt_addr;
    //     sstar_miu_err("*(u32*)virt_addr: %#x\n", *(u32*)virt_addr);
    //     // *(u32*)virt_addr = 0x1000;
    // }
    miu_protect_dump_bank(0x1659);

    reg_miu_bist_init(BASE_REG_MIU_GRP_SC0, BASE_REG_MIU_EFFI_SC0);
    reg_miu_bist_set_base(BASE_REG_MIU_EFFI_SC0, 0x00000000);
    reg_miu_bist_set_length(BASE_REG_MIU_EFFI_SC0, 0x1000);
    reg_miu_bist_start(BASE_REG_MIU_EFFI_SC0);

    miu_protect_dump_bank(0x1668);
    msleep(10);

    miu_protect_dump_bank(0x1659);

    printk("protect test 2\r\n");
    sstar_miu_protect_enable(33, pp1, 0, 0x1400001000, 0x1400002000);
    virt_addr = (unsigned int*)CamOsMemMap(0x1400001000, 0x200, 0);
    if (!virt_addr)
    {
        printk("CamOsMemMap failed\r\n");
    }
    *(u32*)virt_addr = 0x1000;
}
#endif

int sstar_miu_real_probe(void)
{
    int ret = 0;

    sstar_miu_run_init(sstar_miu_device_init, ret);
    sstar_miu_run_init(sstar_miu_device_object_init, ret);
#if defined(CAM_OS_LINUX_KERNEL)
    sstar_miu_run_init(sstar_miu_sysfs_init, ret);
#elif defined(CAM_OS_RTK)
    sstar_miu_run_init(sstar_miu_irq_init, ret);
#endif

#ifdef MIU_PROTECT_TEST
    miu_protect_test();
#endif

    return ret;
}

#if defined(CAM_OS_LINUX_KERNEL)
static int sstar_miu_driver_probe(struct platform_device* pdev)
{
    sstar_miu_info("\n");
    return sstar_miu_real_probe();
}

static int sstar_miu_driver_remove(struct platform_device* pdev)
{
    sstar_miu_device_free(sstar_get_miu_device());

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_miu_driver_suspend(struct platform_device* dev, pm_message_t state)
{
    struct miu_device* mdev = sstar_get_miu_device();

    if (miu_irq)
    {
        disable_irq(miu_irq);
        CamOsIrqFree(miu_irq, (void*)&miuSelId);
        miu_irq = 0;
    }

#ifdef CONFIG_MMU_INTERRUPT_ENABLE
    if (mmu_irq)
    {
        disable_irq(mmu_irq);
        CamOsIrqFree(mmu_irq, (void*)&mmuSelId);
        mmu_irq = 0;
    }
#endif

    mdev->is_irq_init = false;
    return 0;
}

static int sstar_miu_driver_resume(struct platform_device* dev)
{
    return 0;
}
#endif

static struct of_device_id sstar_miu_of_device_ids[] = {
    {.compatible = "sstar,miu"},
    {},
};

static struct platform_driver sstar_miu_driver = {
    .probe  = sstar_miu_driver_probe,
    .remove = sstar_miu_driver_remove,
#ifdef CONFIG_PM_SLEEP
    .suspend = sstar_miu_driver_suspend,
    .resume  = sstar_miu_driver_resume,
#endif
    .driver =
        {
            .name           = "SStar-miu",
            .of_match_table = sstar_miu_of_device_ids,
            .owner          = THIS_MODULE,
        },
};

// module_platform_driver(sstar_miu_driver);
static int __init __sstar_miu_driver_init(void)
{
    return platform_driver_register(&sstar_miu_driver);
}
subsys_initcall(__sstar_miu_driver_init);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("MIU driver");
MODULE_LICENSE("GPL");

#elif defined(CAM_OS_RTK)

#define MAX_32BITS_ADDR_MASK 0xFFFFFFFF
#define INVALID_MIU_ADDR     0xFFFFFFFF
#define MIU_MAX_PHY_ADDR     0xFFFFFFFF

ss_miu_addr_t sstar_miu_phy_to_miu(ss_phys_addr_t phy_addr)
{
    ss_miu_addr_t miu_addr = 0;

#if defined(CONFIG_LPAE_SUPPORT) || defined(CONFIG_RTOS_RUN_ENV_64BIT)
    if (phy_addr > MIU_MAX_PHY_ADDR)
        miu_addr = INVALID_MIU_ADDR;
    else if (phy_addr >= MIU_BASE_HIGH_PHYADDR && phy_addr <= MIU_MAX_PHY_ADDR)
        miu_addr = (phy_addr - MIU_BASE_HIGH_PHYADDR) & MAX_32BITS_ADDR_MASK;
    else if (phy_addr > MIU_MAX_PHY_ADDR && phy_addr < MIU_BASE_HIGH_PHYADDR)
        miu_addr = INVALID_MIU_ADDR;
    else if (phy_addr >= MIU_BASE_PHYADDR && phy_addr <= MIU_MAX_PHY_ADDR)
        miu_addr = (phy_addr - MIU_BASE_PHYADDR) & MAX_32BITS_ADDR_MASK;
    else
        miu_addr = INVALID_MIU_ADDR;
#else
    if ((u32)phy_addr > MIU_MAX_PHY_ADDR || (u32)phy_addr < MIU_BASE_PHYADDR)
        miu_addr = INVALID_MIU_ADDR;
    else
        miu_addr = ((u32)phy_addr - MIU_BASE_PHYADDR) & MAX_32BITS_ADDR_MASK;
#endif
    return miu_addr;
}

ss_phys_addr_t sstar_miu_addr_to_phy(ss_miu_addr_t miu_addr)
{
    ss_phys_addr_t phy_addr = 0;

    if (miu_addr & ~MIU_ADDR_MASK)
    {
        phy_addr = INVALID_MIU_ADDR;
    }
    else
    {
        phy_addr = miu_addr + MIU_BASE_PHYADDR;
        if (phy_addr > MIU_MAX_PHY_ADDR)
            phy_addr = INVALID_MIU_ADDR;

        // TBD for MIU_BASE_HIGH_PHYADDR, need return u64PhyAddr;
    }
    return phy_addr;
}

#if defined(CONFIG_MIU_BWLA)
int sstar_miu_bw_show(char* buf, int offset, int n)
{
    struct miu_device* mdev = sstar_get_miu_device();

    // CamOsPrintf(KERN_EMERG "[%s %d] offset=%x, size=%X\r\n", __FUNCTION__, __LINE__, offset, n);
    return mdev->bw.callback.show(&mdev->bw, offset, buf, buf + n);
}

int sstar_miu_bw_set_rounds(int round)
{
    struct miu_device* mdev = sstar_get_miu_device();

    return mdev->bw.callback.store(&mdev->bw, round);
}

#endif

void sstar_miu_driver_probe(void)
{
    if (sstar_miu_real_probe() < 0)
    {
        sstar_miu_device_free(sstar_get_miu_device());
        CamOsPanic("sstar miu driver init failed!\n");
    }
    sstar_miu_info("sstar miu driver init success\n");
}

rtos_subsys_initcall(sstar_miu_driver_probe);
#endif
