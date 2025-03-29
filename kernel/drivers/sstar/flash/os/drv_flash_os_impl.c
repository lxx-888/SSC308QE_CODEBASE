/*
 * drv_flash_os_impl.c- Sigmastar
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

#include <drv_flash_os_impl.h>

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
    printk("%s", data);
}

void flash_impl_printf_hex(const char *data, u32 value, const char *data2)
{
    if (data && data2)
        printk("%s%x%s", data, value, data2);
    else if (data)
        printk("%s%x", data, value);
    else if (data2)
        printk("%x%s", value, data2);
    else
        printk("%x", value);
}

void flash_impl_show_id(u8 *data, u8 cnt)
{
    printk("[FLASH] Device id is 0x%02x 0x%02x 0x%02x\n", data[0], data[1], data[2]);
}

u64 flash_impl_virt_to_phys(void *virt)
{
    return (u64)CamOsMemVirtToPhys(virt);
}

u64 flash_impl_phys_to_miu(u64 phys)
{
    return (u64)Chip_Phys_to_MIU((ss_phys_addr_t)phys);
}

void flash_impl_miupipe_flush(void)
{
    Chip_Flush_MIU_Pipe();
}

void flash_impl_mem_flush(void *data, u32 size)
{
    CamOsMemFlush(data, size);
}

void flash_impl_mem_invalidate(void *data, u32 size)
{
    CamOsMemInvalidate(data, size);
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
    int storage_type = Chip_Get_Storage_Type();

    if (storage_type == MS_STORAGE_SPINAND_ECC)
        return FLASH_BOOT_STORAGE_NAND;
    else if (storage_type == MS_STORAGE_NOR)
        return FLASH_BOOT_STORAGE_NOR;
    else
        return FLASH_BOOT_STORAGE_OTHER;
}

u8 flash_impl_bdma_transfer(struct flash_bdma_param *param)
{
    u64             phy_src_addr;
    u64             phy_dst_addr;
    hal_bdma_param *bdma_param = &param->bdma_param;

    memset(bdma_param, 0, sizeof(hal_bdma_param));

    switch (param->path)
    {
        case FLASH_BDMA_SPI2MIU:
            phy_dst_addr              = (u64)CamOsMemVirtToPhys((void *)param->dst);
            bdma_param->ePathSel      = HAL_BDMA_SPI_TO_MIU0;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->pSrcAddr      = (phys_addr_t)(unsigned long)param->src;
            bdma_param->pDstAddr      = (phys_addr_t)Chip_Phys_to_MIU((ss_phys_addr_t)phy_dst_addr);
            break;
        case FLASH_BDMA_MIU2SPI:
            phy_src_addr              = (u64)CamOsMemVirtToPhys((void *)param->src);
            bdma_param->ePathSel      = HAL_BDMA_MIU0_TO_SPI;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->pSrcAddr      = (phys_addr_t)Chip_Phys_to_MIU((ss_phys_addr_t)phy_src_addr);
            bdma_param->pDstAddr      = 0;
            break;
        case FLASH_BDMA_SPI2XZDEC:
        case FLASH_BDMA_PMSPI2XZDEC:
            return FLASH_BDMA_NOSUPPORT;
        case FLASH_BDMA_PMSPI2MIU:
            phy_dst_addr              = (u64)CamOsMemVirtToPhys((void *)param->dst);
            bdma_param->ePathSel      = HAL_BDMA_PM_SPI_TO_MIU0;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->pSrcAddr      = (phys_addr_t)(unsigned long)param->src;
            bdma_param->pDstAddr      = (phys_addr_t)Chip_Phys_to_MIU((ss_phys_addr_t)phy_dst_addr);
            break;
            break;
        case FLASH_BDMA_MIU2PMSPI:
            phy_src_addr              = (u64)CamOsMemVirtToPhys((void *)param->src);
            bdma_param->ePathSel      = HAL_BDMA_MIU0_TO_PM_SPI;
            bdma_param->eSrcDataWidth = HAL_BDMA_DATA_BYTE_16;
            bdma_param->eDstDataWidth = HAL_BDMA_DATA_BYTE_8;
            bdma_param->pSrcAddr      = (phys_addr_t)Chip_Phys_to_MIU((ss_phys_addr_t)phy_src_addr);
            bdma_param->pDstAddr      = 0;
            break;
        default:
            return FLASH_BDMA_NOSUPPORT;
    }

    bdma_param->bIntMode     = !!param->interrupt_en;
    bdma_param->eDstAddrMode = HAL_BDMA_ADDR_INC; // address increase
    bdma_param->u32TxCount   = param->size;
    bdma_param->pTxCbParm    = param->callback_parm;
    bdma_param->pfTxCbFunc   = param->callback;

    if (HAL_BDMA_PROC_DONE != hal_bdma_transfer(HAL_BDMA_CH0, bdma_param))
    {
        flash_impl_printf("[FLASH] bdma fail\r\n");
        return FLASH_BDMA_TIMEOUT;
    }

    return FLASH_BDMA_SUCCESS;
}

u8 flash_impl_bit_change(u8 data)
{
    data = (data << 4) | (data >> 4);
    data = ((data << 2) & 0xcc) | ((data >> 2) & 0x33);
    data = ((data << 2) & 0xaa) | ((data >> 2) & 0x55);

    return data;
}

u32 flash_impl_bdma_sw_crc32(u8 flag, u32 crc, u32 poly, u8 *buf, u32 size)
{
    u8  data;
    u32 crc_out;
    u32 crc_poly;
    u32 i, j;

    crc_out  = crc;
    crc_poly = poly;
    for (j = 0; j < size; j = j + 1)
    {
        data = buf[j];
        data = flag ? flash_impl_bit_change(data) : data;
        for (i = 0; i <= 7; i = i + 1)
        {
            if (((crc_out >> 31) ^ ((data >> i) & 0x1)) & 0x1)
            {
                crc_out = ((crc_out << 1) >> 1) ^ (crc_poly >> 1);
                crc_out = (crc_out << 1) | 0x1;
            }
            else
            {
                crc_out = (crc_out << 1);
            }
        }
    }

    return crc_out;
}

EXPORT_SYMBOL_GPL(flash_impl_size_to_align_cache_size);
EXPORT_SYMBOL_GPL(flash_impl_count_bits);
EXPORT_SYMBOL_GPL(flash_impl_checksum);
EXPORT_SYMBOL_GPL(flash_impl_printf);
EXPORT_SYMBOL_GPL(flash_impl_printf_hex);
EXPORT_SYMBOL_GPL(flash_impl_show_id);
EXPORT_SYMBOL_GPL(flash_impl_virt_to_phys);
EXPORT_SYMBOL_GPL(flash_impl_phys_to_miu);
EXPORT_SYMBOL_GPL(flash_impl_miupipe_flush);
EXPORT_SYMBOL_GPL(flash_impl_mem_flush);
EXPORT_SYMBOL_GPL(flash_impl_mem_invalidate);
EXPORT_SYMBOL_GPL(flash_impl_memcmp);
EXPORT_SYMBOL_GPL(flash_impl_memcpy);
EXPORT_SYMBOL_GPL(flash_impl_memset);
EXPORT_SYMBOL_GPL(flash_impl_strcmp);
EXPORT_SYMBOL_GPL(flash_impl_get_ipl_size);
EXPORT_SYMBOL_GPL(flash_impl_get_boot_storage);
EXPORT_SYMBOL_GPL(flash_impl_bdma_transfer);
EXPORT_SYMBOL_GPL(flash_impl_bit_change);
EXPORT_SYMBOL_GPL(flash_impl_bdma_sw_crc32);
