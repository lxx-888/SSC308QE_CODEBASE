/*
 * drv_flash_os_impl.c- Sigmastar
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

#include <drv_flash_os_impl.h>

extern DEVINFO_BOOT_TYPE sstar_devinfo_boot_type(void);
extern void              Chip_Flush_Memory(void);
extern void              flush_dcache_range(unsigned long start, unsigned long stop);
extern void              invalidate_dcache_range(unsigned long start, unsigned long stop);

u32 flash_impl_size_to_align_cache_size(u32 size)
{
    return ((size + (63)) & ~(63));
}

u8 flash_impl_count_bits(u32 x)
{
    u8 i = 0;

    while (x)
    {
        i++;
        x >>= 1;
    }

    return (i - 1);
}

u32 flash_impl_checksum(u8 *data, u32 size)
{
    u32 checksum = 0;

    while (0 < size)
    {
        checksum += *data;
        data++;
        size--;
    }

    return checksum;
}

void flash_impl_printf(const char *data)
{
    printf("%s", data);
}

void flash_impl_printf_hex(const char *data, u32 value, const char *data2)
{
    if (data && data2)
        printf("%s%x%s", data, value, data2);
    else if (data)
        printf("%s%x", data, value);
    else if (data2)
        printf("%x%s", value, data2);
    else
        printf("%x", value);
}

void flash_impl_show_id(u8 *data, u8 cnt)
{
    printf("[FLASH] Device id is 0x%02x 0x%02x 0x%02x\n", data[0], data[1], data[2]);
}

void flash_impl_get_time(void) {}

void flash_impl_printf_time_diff(void) {}

void flash_impl_printf_load_info(void *partInfo, u32 offset, u32 address, u32 size) {}

#define FLASH_IMPL_VirtToPhys(_virt_) (_virt_)

u64 flash_impl_virt_to_phys(void *virt)
{
    return (u64)(unsigned long)(virt);
}

u64 flash_impl_phys_to_miu(u64 phys)
{
    return (phys - MIU0_START_ADDR);
}

void flash_impl_miupipe_flush(void)
{
    Chip_Flush_Memory();
}

void flash_impl_mem_flush(void *data, u32 size)
{
    unsigned long stop = (unsigned long)data + size;

    stop = (stop + (CONFIG_SYS_CACHELINE_SIZE - 1)) & (~(CONFIG_SYS_CACHELINE_SIZE - 1));

    flush_dcache_range((unsigned long)data, stop);
}

void flash_impl_mem_invalidate(void *data, u32 size)
{
    unsigned long stop = (unsigned long)data + size;

    stop = (stop + (CONFIG_SYS_CACHELINE_SIZE - 1)) & (~(CONFIG_SYS_CACHELINE_SIZE - 1));

    invalidate_dcache_range((unsigned long)data, stop);
}

u8 flash_impl_memcmp(u8 *cs, const u8 *ct, u8 size)
{
    return !memcmp((const void *)cs, (const void *)ct, size);
}

void *flash_impl_memcpy(u8 *dst, const u8 *src, u32 size)
{
    return memcpy((void *)dst, (const void *)src, size);
}

void *flash_impl_memset(u8 *s, u32 c, u32 size)
{
    return memset((void *)s, c, size);
}

u32 flash_impl_strcmp(const u8 *cs, const u8 *ct)
{
    return strcmp((const char *)(cs), (const char *)(ct));
}

u32 flash_impl_get_ipl_size(u8 *buf)
{
    if (0x5F4C5049 == (*(volatile u32 *)(buf + 4)))
    {
        if ((*((volatile u8 *)(buf + 14))) >= 0x05)
        {
            return ((*((volatile u16 *)(buf + 8))) << 4);
        }
        else
        {
            return (*((volatile u16 *)(buf + 8)));
        }
    }

    return 0;
}

u8 flash_impl_get_boot_storage(void)
{
    u16 storage_type = sstar_devinfo_boot_type();

    if (storage_type == DEVINFO_BOOT_TYPE_SPINAND_INT_ECC)
        return FLASH_BOOT_STORAGE_NAND;
    else if (storage_type == DEVINFO_BOOT_TYPE_SPI)
        return FLASH_BOOT_STORAGE_NOR;
    else
        return FLASH_BOOT_STORAGE_OTHER;
}

u8 flash_impl_bdma_transfer(struct flash_bdma_param *param)
{
    HalBdmaParam_t *bdma_param = &param->bdma_param;

    memset(bdma_param, 0, sizeof(HalBdmaParam_t));

    switch (param->path)
    {
        case FLASH_BDMA_SPI2MIU:
            bdma_param->ePathSel      = HAL_BDMA_SPI_TO_MIU0;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->pSrcAddr      = (phys_addr_t)param->src;
            bdma_param->pDstAddr      = (phys_addr_t)param->dst - MIU0_START_ADDR;
            break;
        case FLASH_BDMA_MIU2SPI:
            bdma_param->ePathSel      = HAL_BDMA_MIU0_TO_SPI;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->pSrcAddr      = (phys_addr_t)param->src - MIU0_START_ADDR;
            bdma_param->pDstAddr      = 0;
            break;
        case FLASH_BDMA_SPI2XZDEC:
            bdma_param->ePathSel      = HAL_BDMA_SPI_TO_XZDEC;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->pSrcAddr      = (phys_addr_t)param->src;
            bdma_param->pDstAddr      = 0;
            break;
        case FLASH_BDMA_PMSPI2MIU:
            bdma_param->ePathSel      = HAL_BDMA_PM_SPI_TO_MIU0;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->pSrcAddr      = (phys_addr_t)param->src;
            bdma_param->pDstAddr      = (phys_addr_t)param->dst - MIU0_START_ADDR;
            break;
        case FLASH_BDMA_MIU2PMSPI:
            bdma_param->ePathSel      = HAL_BDMA_MIU0_TO_PM_SPI;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->pSrcAddr      = (phys_addr_t)param->src - MIU0_START_ADDR;
            bdma_param->pDstAddr      = 0;
            break;
#if defined(CONFIG_FLASH_XZDEC)
        case FLASH_BDMA_PMSPI2XZDEC:
            bdma_param->ePathSel      = HAL_BDMA_SPI_TO_XZDEC;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->pSrcAddr      = (phys_addr_t)param->src;
            bdma_param->pDstAddr      = 0;
            break;
        case FLASH_BDMA_MIU2XZDEC:
            bdma_param->ePathSel      = HAL_BDMA_MIU_TO_XZDEC;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->pSrcAddr      = (phys_addr_t)param->src - MIU0_START_ADDR;
            bdma_param->pDstAddr      = 0;
            break;
#endif
        default:
            return FLASH_BDMA_NOSUPPORT;
    }

    bdma_param->bIntMode     = !!param->interrupt_en;
    bdma_param->eDstAddrMode = HAL_BDMA_ADDR_INC; // address increase
    bdma_param->u32TxCount   = param->size;
    bdma_param->u32Pattern   = 0;
    bdma_param->pfTxCbFunc   = NULL;

    if (HAL_BDMA_PROC_DONE != HalBdma_DoTransfer(HAL_BDMA_CH0, bdma_param))
    {
        printf("[FLASH] bdma fail\r\n");
        return FLASH_BDMA_TIMEOUT;
    }

    return FLASH_BDMA_SUCCESS;
}
