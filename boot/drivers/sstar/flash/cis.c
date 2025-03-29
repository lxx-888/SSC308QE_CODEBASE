/*
 * cis.c- Sigmastar
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
#include <mdrv_spinor.h>
#include <mdrv_spinand.h>

static const char *cis_nri_magic_data = "NRIF";
static const char *cis_sni_magic_data = "SNIF";
static const char *cis_pni_magic_data = "SSTARSEMICIS0001";

struct sstar_cis
{
    u8  inited;
    u8  store_cnt;
    u8  boot_cis;
    u8  need_pni_adjust;
    u8 *map[CIS_MAPPING_CNT];
    u8 *fcie_buf;
    u8  fcie_buf_path;
    u8  ecc_mode;
    u8  storage_type;
};

static struct sstar_cis cis = {
    .inited          = 0,
    .store_cnt       = 0,
    .boot_cis        = 0,
    .need_pni_adjust = 0,
};

u8 sstar_cis_is_sni_header(u8 *sni_buf)
{
    return flash_impl_memcmp(sni_buf, (const u8 *)cis_sni_magic_data, CIS_HEADER_SIZE);
}

u8 sstar_cis_is_nri_header(u8 *sni_buf)
{
    return flash_impl_memcmp(sni_buf, (const u8 *)cis_nri_magic_data, CIS_HEADER_SIZE);
}

static u8 sstar_cis_is_sni(void *cis_buf)
{
    spinand_sni_t * sni;
    spinand_info_t *spinand_info;

    sni          = (spinand_sni_t *)cis_buf;
    spinand_info = &sni->spinand_info;

    do
    {
        if (!sstar_cis_is_sni_header((u8 *)sni))
        {
            break;
        }

        if (sni->crc32 != FLASH_IMPL_CRC32(0, (void *)spinand_info, (CIS_SNI_SIZE - CIS_HEADER_SIZE - 0x4)))
        {
            break;
        }

        return 1;
    } while (0);

    return 0;
}

static u8 sstar_cis_is_nri(void *cis_buf)
{
    spinor_nri_t * spinor_sni;
    spinor_info_t *spinor_info;

    spinor_sni  = (spinor_nri_t *)cis_buf;
    spinor_info = &spinor_sni->spinor_info;

    do
    {
        if (!sstar_cis_is_nri_header((u8 *)spinor_sni))
        {
            break;
        }

        if (spinor_sni->crc32 != FLASH_IMPL_CRC32(0, (void *)spinor_info, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4)))
        {
            break;
        }

        return 1;
    } while (0);

    return 0;
}

static u8 sstar_cis_is_pni(void *cis_buf)
{
    struct parts_tbl *tbl = NULL;

    tbl = (struct parts_tbl *)cis_buf;

    if (flash_impl_memcmp(tbl->magic, (const u8 *)cis_pni_magic_data, sizeof(cis_pni_magic_data)))
    {
        if (tbl->checksum == flash_impl_checksum((u8 *)&(tbl->size), tbl->size + sizeof(tbl->size)))
        {
            return 1;
        }
    }

    return 0;
}

#if defined(CONFIG_SSTAR_NOR) && defined(CONFIG_SSTAR_NAND)
static u8 sstar_cis_write_back_to_nand(struct spinand_handle *nand_storage);
static u8 sstar_cis_write_back_to_nor(struct spinor_handle *nor_storage);

static u8 sstar_cis_sni_match_from_nor(struct spinor_handle *nor_storage, struct spinand_handle *handle, u8 *sni_buf)
{
    u16             bytes            = 0;
    u32             offset           = 0;
    u32             sni_list_address = 0;
    spinand_sni_t * sni;
    spinand_info_t *sni_info;

    sni      = (spinand_sni_t *)(sni_buf + CIS_ROM_SIZE);
    sni_info = &sni->spinand_info;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_read(nor_storage, CIS_IPL_OFFSET, (u8 *)&sni_list_address, 2))
        return 0;

    sni_list_address = (sni_list_address + 0xfff) & ~0xfff;

    for (; CIS_SEARCH_END > sni_list_address; sni_list_address += CIS_PAGE_SIZE)
    {
        if (ERR_SPINOR_SUCCESS != mdrv_spinor_read(nor_storage, sni_list_address, sni_buf, CIS_PAGE_SIZE))
            break;

        if (sni->crc32 == FLASH_IMPL_CRC32(0, (void *)&sni_info->id_byte_cnt, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4))
            && sstar_cis_is_sni_header(sni_buf + CIS_ROM_SIZE))
            break;
    }

    if (sni->crc32 != FLASH_IMPL_CRC32(0, (void *)&sni_info->id_byte_cnt, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4))
        || !sstar_cis_is_sni_header(sni_buf + CIS_ROM_SIZE))
    {
        flash_impl_printf("[FLASH_ERR] no find sni flash_list!\r\n");
        return 0;
    }

    offset = CIS_ROM_SIZE;

    do
    {
        for (bytes = 0;; bytes += (CIS_SNI_SIZE + CIS_ROM_SIZE))
        {
            sni      = (spinand_sni_t *)(&sni_buf[offset]);
            sni_info = &sni->spinand_info;

            if (ERR_SPINOR_SUCCESS
                != mdrv_spinor_read(nor_storage, sni_list_address + bytes, &sni_buf[0], (CIS_SNI_SIZE + CIS_ROM_SIZE)))
                break;

            if (sni->crc32
                != FLASH_IMPL_CRC32(0, (void *)&sni_info->id_byte_cnt, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4)))
                break;

            if (!sstar_cis_is_sni_header(&sni_buf[offset]))
            {
                break;
            }

            if (!mdrv_spinand_is_id_match(handle, sni_info->id, sni_info->id_byte_cnt))
            {
                handle->sni = (spinand_sni_t *)(&sni_buf[offset]);
                cis.store_cnt++;
                return 1;
            }
        }
    } while (0);

    return 0;
}

static u8 sstar_cis_nri_match_from_nand(struct spinand_handle *nand_storage, struct spinor_handle *handle, u8 *nri_buf)
{
    u16            bytes       = 0;
    u32            page        = 0;
    u32            search_page = 0;
    u32            offset      = 0;
    spinor_nri_t * nri;
    spinor_info_t *nri_info;

    page   = 10 * FLASH_PAGES_PER_BLOCK_DEFAULT;
    offset = CIS_ROM_SIZE;

    do
    {
        page -= FLASH_PAGES_PER_BLOCK_DEFAULT;

        if (page & FLASH_PAGES_PER_BLOCK_DEFAULT)
        {
            for (search_page = 2; search_page < FLASH_PAGES_PER_BLOCK_DEFAULT; search_page++)
            {
                for (bytes = 0;; bytes += (CIS_NRI_SIZE + CIS_ROM_SIZE)) // the sni has 512bytes
                {
                    nri      = (spinor_nri_t *)(&nri_buf[offset]);
                    nri_info = &nri->spinor_info;

                    if (ERR_SPINAND_TIMEOUT <= mdrv_spinand_page_read(nand_storage, page + search_page, bytes,
                                                                      &nri_buf[0], (CIS_NRI_SIZE + CIS_ROM_SIZE)))
                        break;

                    if (nri->crc32
                        != FLASH_IMPL_CRC32(0, (void *)(&nri_info->id_byte_cnt),
                                            (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4)))
                    {
                        break;
                    }

                    if (!sstar_cis_is_sni_header(&nri_buf[offset]))
                    {
                        break;
                    }

                    if (mdrv_spinor_is_nri_match(handle, nri_info))
                    {
                        handle->nri = (spinor_nri_t *)(&nri_buf[offset]);
                        cis.store_cnt++;
                        return 1;
                    }
                }
            }
        }
    } while (0 != page);

    return 0;
}
#endif

#if defined(CONFIG_SSTAR_NAND)
#if defined(CONFIG_FLASH_WRITE_BACK)
static u8 sstar_cis_pni_adjust_onebin(struct spinand_handle *handle, u8 *pni_buf, u8 trunk)
{
    u8                 change       = 0;
    u32                i            = 0;
    u32                count        = 0;
    u32                ipl_end      = 0;
    u32                ipl_cust_end = 0;
    u32                uboot_start  = 0;
    u32                size         = 0;
    u32                check_start  = 0;
    u32                check_end    = 0;
    u32                bad_offset   = 0;
    u32                end          = 0;
    struct parts_tbl * tbl          = NULL;
    struct parts_info *part_info    = NULL;
    struct parts_info  ipl_info;
    struct parts_info  ipl_cust_info;
    struct parts_info  uboot_info;
    flash_nand_info_t  nand_info;

    tbl       = (struct parts_tbl *)pni_buf;
    count     = tbl->size / sizeof(struct parts_info);
    part_info = (struct parts_info *)(pni_buf + sizeof(struct parts_tbl));

    mdrv_spinand_info(handle, &nand_info);

    memset((void *)&ipl_info, 0, sizeof(struct parts_info));
    memset((void *)&ipl_cust_info, 0, sizeof(struct parts_info));
    memset((void *)&uboot_info, 0, sizeof(struct parts_info));

    for (i = 0; i < count; i++)
    {
        if ((part_info->trunk == trunk) && !flash_impl_strcmp(part_info->part_name, (u8 *)"IPL")
            && (0 == part_info->group))
        {
            memcpy(&ipl_info, part_info, sizeof(struct parts_info));
            size += part_info->size;
            ipl_end = part_info->offset + part_info->size;
        }
        if ((part_info->trunk == trunk) && !flash_impl_strcmp(part_info->part_name, (u8 *)"IPL_CUST")
            && (0 == part_info->group))
        {
            memcpy(&ipl_cust_info, part_info, sizeof(struct parts_info));
            size += part_info->size;
            ipl_cust_end = part_info->offset + part_info->size;
        }
        if ((part_info->trunk == trunk) && !flash_impl_strcmp(part_info->part_name, (u8 *)"UBOOT")
            && (0 == part_info->group))
        {
            memcpy(&uboot_info, part_info, sizeof(struct parts_info));
            size += part_info->size;
            uboot_start = part_info->offset;
        }
        part_info++;
    }
    if ((ipl_end != ipl_cust_info.offset) || (ipl_cust_end != uboot_info.offset) || 0 == ipl_end || 0 == ipl_cust_end
        || 0 == uboot_start)
    {
        // It isn't onebin,needn't to adjust partition table
        return 0;
    }
    end = ipl_info.offset + size;
    // find ipl_cust start address
    check_start = ipl_info.offset >> flash_impl_count_bits(nand_info.page_size);
    check_end   = (ipl_info.offset + ipl_info.size) >> flash_impl_count_bits(nand_info.page_size); // good block size
    while (check_start < check_end)
    {
        if (mdrv_spinand_block_isbad(handle, check_start))
        {
            if (bad_offset == ipl_info.offset) // The first block of IPL_CUST is bad block
                ipl_info.offset += nand_info.block_size;
            else
                ipl_info.size += nand_info.block_size;
        }

        check_start += nand_info.blk_page_cnt;
    }

    if (ipl_info.offset >= end)
    {
        ipl_info.offset = end;
        ipl_info.size   = 0;
    }

    ipl_cust_info.offset = ipl_info.offset + ipl_info.size;
    if (ipl_cust_info.offset >= end) // beyond the onebine range
    {
        ipl_cust_info.offset = end;
        ipl_cust_info.size   = 0;
    }

    // find uboot start address
    check_start = ipl_cust_info.offset >> flash_impl_count_bits(nand_info.page_size);
    check_end   = (ipl_cust_info.offset + ipl_cust_info.size) >> flash_impl_count_bits(nand_info.page_size);
    while (check_start < check_end)
    {
        if (mdrv_spinand_block_isbad(handle, check_start))
        {
            if (bad_offset == ipl_info.offset) // The first block of IPL_CUST is bad block
                ipl_info.offset += nand_info.block_size;
            else
                ipl_info.size += nand_info.block_size;
        }

        check_start += nand_info.blk_page_cnt;
    }

    if (ipl_cust_info.offset >= end)
    {
        ipl_cust_info.offset = end;
        ipl_cust_info.size   = 0;
    }

    uboot_info.offset = ipl_cust_info.offset + ipl_cust_info.size;
    if (uboot_info.offset >= end) // beyond the onebine range
    {
        uboot_info.offset = end;
        uboot_info.size   = 0;
    }
    else
    {
        uboot_info.size = end - uboot_info.offset;
    }

    // check bad block in uboot partition
    check_start = uboot_info.offset >> flash_impl_count_bits(nand_info.page_size);
    check_end   = (uboot_info.offset + uboot_info.size) >> flash_impl_count_bits(nand_info.page_size);
    while (check_start < check_end)
    {
        if (mdrv_spinand_block_isbad(handle, check_start))
        {
            if (bad_offset == ipl_info.offset) // The first block of IPL_CUST is bad block
                ipl_info.offset += nand_info.block_size;
            else
                ipl_info.size += nand_info.block_size;
        }

        check_start += nand_info.blk_page_cnt;
    }

    if (uboot_info.offset >= end) // beyond the onebine range
    {
        uboot_info.offset = end;
        uboot_info.size   = 0;
    }

    if ((uboot_start != uboot_info.offset) && (uboot_start != 0))
    {
        part_info = (struct parts_info *)(pni_buf + sizeof(struct parts_tbl));
        for (i = 0; i < count; i++)
        {
            if ((part_info->trunk == trunk) && !flash_impl_strcmp(part_info->part_name, (u8 *)"IPL"))
            {
                memcpy(part_info, &ipl_info, sizeof(struct parts_info));
            }
            if ((part_info->trunk == trunk) && !flash_impl_strcmp(part_info->part_name, (u8 *)"IPL_CUST"))
            {
                memcpy(part_info, &ipl_cust_info, sizeof(struct parts_info));
            }
            if ((part_info->trunk == trunk) && !flash_impl_strcmp(part_info->part_name, (u8 *)"UBOOT"))
            {
                memcpy(part_info, &uboot_info, sizeof(struct parts_info));
            }
            part_info++;
        }
        change = 1;
        flash_impl_printf_hex("Change the table of onebin", trunk, "\r\n");
    }
    return change;
}

static u8 sstar_cis_pni_adjust_group(struct spinand_handle *handle, u8 *pni_buf)
{
    u8                 change         = 0;
    u8                 group          = 0;
    u32                i              = 0;
    u32                count          = 0;
    u32                bad_block_size = 0;
    u32                check_start    = 0;
    u32                check_end      = 0;
    struct parts_tbl * tbl            = NULL;
    struct parts_info *part_info      = NULL;
    struct parts_info *part_info_tmp  = NULL;
    flash_nand_info_t  nand_info;

    tbl       = (struct parts_tbl *)pni_buf;
    count     = tbl->size / sizeof(struct parts_info);
    part_info = (struct parts_info *)(pni_buf + sizeof(struct parts_tbl));

    mdrv_spinand_info(handle, &nand_info);

    for (i = 0; i < count; i++)
    {
        if ((0 != group) && (group != part_info->group) && bad_block_size)
        {
            do
            {
                if (part_info_tmp->size >= bad_block_size)
                {
                    part_info_tmp->size -= bad_block_size;
                    bad_block_size = 0;
                }
                else
                {
                    bad_block_size -= part_info_tmp->size;
                    part_info_tmp->size = 0;
                }

                part_info_tmp--;
            } while (bad_block_size && (group == part_info_tmp->group));

            group          = 0;
            bad_block_size = 0;
            part_info_tmp  = NULL;
            change         = 1;
        }

        if (0 != part_info->group)
        {
            if (group != part_info->group)
                group = part_info->group;
            else
                part_info->offset += bad_block_size;

            part_info_tmp = part_info;
            check_start   = part_info->offset >> flash_impl_count_bits(nand_info.page_size);
            check_end     = (part_info->offset + part_info->size) >> flash_impl_count_bits(nand_info.page_size);

            while (check_start < check_end)
            {
                if (mdrv_spinand_block_isbad(handle, check_start))
                {
                    if (check_start
                        == (part_info->offset
                            >> flash_impl_count_bits(nand_info.page_size))) // The first block of IPL_CUST is bad block
                        part_info->offset += nand_info.block_size;
                    else
                        part_info->size += nand_info.block_size;

                    bad_block_size += nand_info.block_size;
                }

                check_start += nand_info.blk_page_cnt;
            }

            if (bad_block_size)
            {
                flash_impl_printf("adjust part: ");
                flash_impl_printf_hex((const char *)part_info->part_name, part_info->trunk, NULL);
                flash_impl_printf_hex(", part_offset 0x", part_info->offset, NULL);
                flash_impl_printf_hex(", part_size 0x", part_info->size, "\r\n");
            }
        }

        part_info++;
    }

    if (part_info_tmp && bad_block_size)
    {
        part_info_tmp->size -= bad_block_size;
        change = 1;
    }

    return change;
}

static void sstar_cis_pni_adjust_parts_tbl(struct spinand_handle *handle, u8 *pni_buf)
{
    u8                u8_ret      = 0;
    struct parts_tbl *pstPartsTbl = (struct parts_tbl *)pni_buf;

    u8_ret |= sstar_cis_pni_adjust_onebin(handle, pni_buf, 0);
    u8_ret |= sstar_cis_pni_adjust_onebin(handle, pni_buf, 1);
    u8_ret |= sstar_cis_pni_adjust_group(handle, pni_buf);

    if (u8_ret)
    {
        // when the pni is changed,the checksum should be recaculated
        pstPartsTbl->checksum =
            flash_impl_checksum((u8 *)(&(pstPartsTbl->size)), pstPartsTbl->size + sizeof(pstPartsTbl->size));
    }
}

static u8 sstar_cis_write_back_to_nand(struct spinand_handle *nand_storage)
{
    u8                 i;
    u8                 mark_bad = 0;
    u32                count;
    u32                search_page = 0;
    struct parts_tbl * tbl         = NULL;
    struct parts_info *part_info   = NULL;
    flash_nand_info_t  info;

    if (cis.boot_cis == 0xFF)
        return 0;

    nand_storage->sni = (spinand_sni_t *)(cis.map[cis.boot_cis] + CIS_ROM_SIZE);

    mdrv_spinand_hardware_init(nand_storage);
    mdrv_spinand_info(nand_storage, &info);

    tbl = sstar_cis_get_pni();

    if (!tbl)
        return 0;

    part_info = (struct parts_info *)(((u8 *)tbl) + sizeof(struct parts_tbl));
    count     = tbl->size / sizeof(struct parts_info);

    for (i = 0; i < count; i++)
    {
        if (part_info->active && !flash_impl_strcmp(part_info->part_name, (const u8 *)"CIS"))
            break;

        part_info++;
    }

    if (i == count)
        return 0;

    search_page = (part_info->offset + part_info->size) >> flash_impl_count_bits(info.page_size);

    do
    {
        search_page -= info.blk_page_cnt;

        if (search_page & info.blk_page_cnt)
            continue;

        flash_impl_printf_hex("write back sni to page 0x", search_page, "\r\n");

        if (mdrv_spinand_block_isbad(nand_storage, search_page))
            continue;

        if (ERR_SPINAND_SUCCESS != mdrv_spinand_block_erase(nand_storage, search_page))
        {
            mdrv_spinand_page_program(nand_storage, search_page, info.page_size, &mark_bad, 1);
            continue;
        }

        if (ERR_SPINAND_SUCCESS
            != mdrv_spinand_page_program(nand_storage, search_page, 0, cis.map[cis.boot_cis], CIS_PAGE_SIZE))
        {
            mdrv_spinand_block_erase(nand_storage, search_page);
            mdrv_spinand_page_program(nand_storage, search_page, info.page_size, &mark_bad, 1);
            continue;
        }
    } while (search_page >= info.blk_page_cnt);

    for (i = 0; i < cis.store_cnt; i++)
    {
        if (sstar_cis_is_pni(cis.map[i]))
        {
            break;
        }
    }

    if (i == cis.store_cnt)
        return 0;

    if (cis.need_pni_adjust)
        sstar_cis_pni_adjust_parts_tbl(nand_storage, cis.map[i]);

    search_page = (part_info->offset + part_info->size) >> flash_impl_count_bits(info.page_size);

    do
    {
        search_page -= info.blk_page_cnt;

        if (search_page & info.blk_page_cnt)
            continue;

        flash_impl_printf_hex("write back pni to page 0x", search_page + 1, "\r\n");

        if (mdrv_spinand_block_isbad(nand_storage, search_page))
            continue;

        if (ERR_SPINAND_SUCCESS
            != mdrv_spinand_page_program(nand_storage, search_page + 1, 0, cis.map[i], CIS_PAGE_SIZE))
        {
            mdrv_spinand_block_erase(nand_storage, search_page);
            mdrv_spinand_page_program(nand_storage, search_page, info.page_size, &mark_bad, 1);
            continue;
        }
    } while (search_page >= info.blk_page_cnt);

    return 1;
}
#else
static u8 sstar_cis_write_back_to_nand(struct spinand_handle *nand_storage)
{
    FLASH_IMPL_UNUSED_VAR(nand_storage);
    return 1;
}
#endif

static u8 sstar_cis_sni_match_from_nand(struct spinand_handle *nand_storage, struct spinand_handle *handle, u8 *sni_buf)
{
    u16             bytes       = 0;
    u32             page        = 0;
    u32             search_page = 0;
    u32             offset      = 0;
    spinand_sni_t * sni;
    spinand_info_t *sni_info;

    page   = 10 * FLASH_PAGES_PER_BLOCK_DEFAULT;
    offset = CIS_ROM_SIZE;

    do
    {
        page -= FLASH_PAGES_PER_BLOCK_DEFAULT;

        for (search_page = 2; search_page < FLASH_PAGES_PER_BLOCK_DEFAULT; search_page++)
        {
            for (bytes = 0;; bytes += (CIS_SNI_SIZE + CIS_ROM_SIZE))
            {
                sni      = (spinand_sni_t *)(&sni_buf[offset]);
                sni_info = &sni->spinand_info;

                if (ERR_SPINAND_TIMEOUT <= mdrv_spinand_page_read(nand_storage, page + search_page, bytes, &sni_buf[0],
                                                                  (CIS_SNI_SIZE + CIS_ROM_SIZE)))
                    break;

                if (sni->crc32 != FLASH_IMPL_CRC32(0, (void *)sni_info, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4)))
                    break;

                if (!sstar_cis_is_sni_header(&sni_buf[offset]))
                {
                    break;
                }

                if (mdrv_spinand_is_id_match(handle, sni_info->id, sni_info->id_byte_cnt))
                {
                    if ((cis.boot_cis == 0xFF) && mdrv_spinand_is_id_match(handle, sni_info->id, sni_info->id_byte_cnt))
                    {
                        cis.boot_cis = cis.store_cnt;
                    }

                    handle->sni = (spinand_sni_t *)(&sni_buf[offset]);
                    cis.store_cnt++;
                    return 1;
                }
            }
        }
    } while (0 != page);

    return 0;
}

static u8 sstar_cis_sni_write_back(struct spinand_handle *handle, u8 *sni_buf)
{
    if (cis.storage_type == FLASH_BOOT_STORAGE_NAND)
    {
        struct spinand_handle nand_storage;
        nand_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nand_storage.msg.cs_select = 0;
        nand_storage.sni           = NULL;

        mdrv_spinand_setup_by_default(&nand_storage);

        mdrv_spinand_soc_ecc_init(&nand_storage, cis.fcie_buf_path, cis.ecc_mode, cis.fcie_buf);

        if (!sstar_cis_sni_match_from_nand(&nand_storage, handle, sni_buf))
            return 0;

        return sstar_cis_write_back_to_nand(&nand_storage);
    }
#if defined(CONFIG_SSTAR_NOR)
    else if (cis.storage_type == FLASH_BOOT_STORAGE_NOR)
    {
        struct spinor_handle nor_storage;
        nor_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nor_storage.msg.cs_select = 0;
        nor_storage.nri           = NULL;

        mdrv_spinor_setup_by_default(&nor_storage);

        if (!sstar_cis_sni_match_from_nor(&nor_storage, handle, sni_buf))
            return 0;

        return sstar_cis_write_back_to_nor(&nor_storage);
    }
#endif
    return 0;
}

static u8 sstar_cis_load_from_nand(struct spinand_handle *boot_storage)
{
    u8              find_sni_page = 0xFF;
    u32             page          = 0;
    u32             search_page   = 0;
    spinand_sni_t * sni;
    spinand_info_t *info;

#if defined(CONFIG_FLASH_SOC_ECC)
    if (boot_storage->soc_ecc_en)
    {
        if (!cis.fcie_buf)
            return 0;

        for (search_page = 0; search_page < (10 * FLASH_PAGES_PER_BLOCK_DEFAULT);
             search_page += (FLASH_PAGES_PER_BLOCK_DEFAULT << 1))
        {
            cis.ecc_mode = 0;

            while (ERR_SPINAND_SUCCESS
                   == mdrv_spinand_soc_ecc_init(boot_storage, cis.fcie_buf_path, cis.ecc_mode++, cis.fcie_buf))
            {
                if (ERR_SPINAND_ECC_NOT_CORRECTED
                    <= mdrv_spinand_page_read(boot_storage, search_page, 0, cis.map[0], CIS_PAGE_SIZE))
                    continue;

                if (!sstar_cis_is_sni_header(cis.map[0]))
                    continue;

                sni  = (spinand_sni_t *)(cis.map[0]);
                info = &sni->spinand_info;

                if ((boot_storage->ecc.page_size != info->page_byte_cnt)
                    || (boot_storage->ecc.oob_size != info->spare_byte_cnt))
                    continue;

                cis.ecc_mode -= 1;
                goto load_cis;
            }
        }
    }

    return 0;

load_cis:
#endif

    do
    {
        for (search_page = cis.store_cnt; search_page < CIS_MAPPING_CNT; search_page++)
        {
            if (ERR_SPINAND_TIMEOUT
                <= mdrv_spinand_page_read(boot_storage, page + search_page, 0, cis.map[cis.store_cnt], CIS_PAGE_SIZE))
                break;

            if (!(page & FLASH_PAGES_PER_BLOCK_DEFAULT) && sstar_cis_is_sni(cis.map[cis.store_cnt] + CIS_ROM_SIZE))
            {
                sni  = (spinand_sni_t *)(cis.map[cis.store_cnt] + CIS_ROM_SIZE);
                info = &sni->spinand_info;

                if ((cis.boot_cis == 0xFF) && mdrv_spinand_is_id_match(boot_storage, info->id, info->id_byte_cnt))
                {
                    find_sni_page = page;
                    cis.boot_cis  = cis.store_cnt;
                }

                cis.store_cnt++;
                continue;
            }

            if (!(page & FLASH_PAGES_PER_BLOCK_DEFAULT) && sstar_cis_is_nri(cis.map[cis.store_cnt] + CIS_ROM_SIZE))
            {
                cis.store_cnt++;
                continue;
            }

            if (sstar_cis_is_pni(cis.map[cis.store_cnt]))
            {
                if ((find_sni_page != 0xFF) && (find_sni_page != page))
                {
                    flash_impl_memcpy(cis.map[0], cis.map[cis.store_cnt], CIS_PAGE_SIZE);
                    cis.boot_cis  = 0xFF;
                    cis.store_cnt = 0;
                }

                if ((page & FLASH_PAGES_PER_BLOCK_DEFAULT) || (find_sni_page == 0xFF))
                    cis.need_pni_adjust = 1;

                cis.store_cnt++;
                return 1;
            }
        }

        page += FLASH_PAGES_PER_BLOCK_DEFAULT;
    } while (page < (10 * FLASH_PAGES_PER_BLOCK_DEFAULT));

    return 0;
}

u8 sstar_cis_get_sni_from_dram(void *handle, u8 *sni_list)
{
    u8                     i           = 0;
    spinand_sni_t *        sni         = NULL;
    spinand_info_t *       info        = NULL;
    struct spinand_handle *nand_handle = handle;

    if (!sni_list)
        return 0;

    do
    {
        if (sstar_cis_is_sni(cis.map[i] + CIS_ROM_SIZE))
        {
            sni  = (spinand_sni_t *)(cis.map[i] + CIS_ROM_SIZE);
            info = &sni->spinand_info;

            if (mdrv_spinand_is_id_match(nand_handle, info->id, info->id_byte_cnt))
            {
                break;
            }
        }

        i++;
    } while (cis.map[i] && (i < cis.store_cnt));

    if (cis.store_cnt >= CIS_MAPPING_CNT)
        return 0;

    while (sstar_cis_is_sni_header((sni_list + CIS_ROM_SIZE)))
    {
        sni  = (spinand_sni_t *)(sni_list + CIS_ROM_SIZE);
        info = &sni->spinand_info;

        if (mdrv_spinand_is_id_match(nand_handle, info->id, info->id_byte_cnt)
            && sni->crc32 == FLASH_IMPL_CRC32(0, (void *)info, (CIS_SNI_SIZE - CIS_HEADER_SIZE - 0x4)))
        {
            memcpy((void *)cis.map[i], (void *)sni_list, (CIS_SNI_SIZE + CIS_ROM_SIZE));
            break;
        }

        sni_list += (CIS_SNI_SIZE + CIS_ROM_SIZE);
    }

    if (!sstar_cis_is_sni_header((sni_list + CIS_ROM_SIZE)))
        return 0;

    nand_handle->sni = (spinand_sni_t *)(cis.map[i] + CIS_ROM_SIZE);

    if (i == cis.store_cnt)
        cis.store_cnt++;

    return 1;
}

u8 sstar_cis_get_sni(void *handle)
{
    u8                     i = 0;
    spinand_sni_t *        sni;
    spinand_info_t *       info;
    struct spinand_handle *nand_handle = handle;

    if (!cis.inited)
        return 0;

    do
    {
        if (sstar_cis_is_sni(cis.map[i] + CIS_ROM_SIZE))
        {
            sni  = (spinand_sni_t *)(cis.map[i] + CIS_ROM_SIZE);
            info = &sni->spinand_info;

            if (mdrv_spinand_is_id_match(nand_handle, info->id, info->id_byte_cnt))
            {
                nand_handle->sni = sni;
                break;
            }
        }

        i++;
    } while (cis.map[i] && (i < cis.store_cnt));

    if ((i == cis.store_cnt) && (cis.store_cnt < CIS_MAPPING_CNT))
    {
        return sstar_cis_sni_write_back(nand_handle, cis.map[cis.store_cnt]);
    }

    return 1;
}
#endif

#if defined(CONFIG_SSTAR_NOR)
#if defined(CONFIG_FLASH_WRITE_BACK)
static u8 sstar_cis_write_back_to_nor(struct spinor_handle *nor_storage)
{
    u8                 i;
    u32                count;
    struct parts_tbl * tbl       = NULL;
    struct parts_info *part_info = NULL;
    flash_nor_info_t   info;

    if (cis.boot_cis == 0xFF)
        return 0;

    nor_storage->nri = (spinor_nri_t *)(cis.map[cis.boot_cis] + CIS_ROM_SIZE);

    mdrv_spinor_hardware_init(nor_storage);
    mdrv_spinor_info(nor_storage, &info);

    tbl = sstar_cis_get_pni();

    if (!tbl)
        return 0;

    part_info = (struct parts_info *)(((u8 *)tbl) + sizeof(struct parts_tbl));
    count     = tbl->size / sizeof(struct parts_info);

    for (i = 0; i < count; i++)
    {
        if (part_info->active && !flash_impl_strcmp(part_info->part_name, (const u8 *)"CIS"))
            break;

        part_info++;
    }

    if (i == count)
        return 0;

    // write back nri
    mdrv_spinor_erase(nor_storage, part_info->offset, info.sector_size);
    mdrv_spinor_program(nor_storage, part_info->offset, cis.map[cis.boot_cis], CIS_PAGE_SIZE);
    flash_impl_printf_hex("write back nri to 0x", part_info->offset, "\r\n");

    return 1;
}
#else
static u8 sstar_cis_write_back_to_nor(struct spinor_handle *nor_storage)
{
    FLASH_IMPL_UNUSED_VAR(nor_storage);
    return 1;
}
#endif

static u8 sstar_cis_nri_match_from_nor(struct spinor_handle *nor_storage, struct spinor_handle *handle, u8 *nri_buf)
{
    u32 nri_list_address;

    u16            bytes  = 0;
    u32            offset = 0;
    spinor_nri_t * nri;
    spinor_info_t *nri_info;

    nri      = (spinor_nri_t *)nri_buf;
    nri_info = &nri->spinor_info;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_read(nor_storage, CIS_IPL_OFFSET, nri_buf, 16))
        return 0;

    nri_list_address = flash_impl_get_ipl_size(nri_buf);
    nri_list_address = (nri_list_address + 0xfff) & ~0xfff;

    for (; CIS_SEARCH_END > nri_list_address; nri_list_address += CIS_PAGE_SIZE)
    {
        if (ERR_SPINOR_SUCCESS != mdrv_spinor_read(nor_storage, nri_list_address, nri_buf, CIS_PAGE_SIZE))
            break;

        if (sstar_cis_is_pni((void *)nri_buf))
        {
            break;
        }
    }

    offset = CIS_ROM_SIZE;
    nri_list_address += 0x1000;

    do
    {
        /* NRI become 1k after iford, contain old nri of 0x300 bytes and rom nri of 0x100 bytes*/
        for (bytes = 0;; bytes += (CIS_NRI_SIZE + CIS_ROM_SIZE))
        {
            nri      = (spinor_nri_t *)(&nri_buf[offset]);
            nri_info = &nri->spinor_info;

            if (ERR_SPINOR_SUCCESS
                != mdrv_spinor_read(nor_storage, nri_list_address + bytes, (u8 *)nri_buf,
                                    (CIS_NRI_SIZE + CIS_ROM_SIZE)))
                break;

            /*crc32 no contain magic(16 bytes) and crc32(4bytes)*/
            if (nri->crc32 != FLASH_IMPL_CRC32(0, (void *)nri_info, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4)))
            {
                break;
            }

            if (!sstar_cis_is_nri_header(&nri_buf[offset]))
            {
                break;
            }

            if (mdrv_spinor_is_nri_match(handle, nri_info))
            {
                if ((cis.boot_cis == 0xFF) && mdrv_spinor_is_nri_match(nor_storage, nri_info))
                {
                    cis.boot_cis = cis.store_cnt;
                }

                handle->nri = (spinor_nri_t *)&nri_buf[offset];
                cis.store_cnt++;
                return 1;
            }
        }
    } while (0);

    return 0;
}

static u8 sstar_cis_nri_write_back(struct spinor_handle *handle, u8 *nri_buf)
{
    if (cis.storage_type == FLASH_BOOT_STORAGE_NOR)
    {
        struct spinor_handle nor_storage;
        nor_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nor_storage.msg.cs_select = 0;
        nor_storage.nri           = NULL;

        mdrv_spinor_setup_by_default(&nor_storage);

        if (!sstar_cis_nri_match_from_nor(&nor_storage, handle, nri_buf))
            return 0;

        return sstar_cis_write_back_to_nor(&nor_storage);
    }
#if defined(CONFIG_SSTAR_NAND)
    else if (cis.storage_type == FLASH_BOOT_STORAGE_NAND)
    {
        struct spinand_handle nand_storage;
        nand_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nand_storage.msg.cs_select = 0;
        nand_storage.sni           = NULL;

        mdrv_spinand_setup_by_default(&nand_storage);

        if (!sstar_cis_nri_match_from_nand(&nand_storage, handle, nri_buf))
            return 0;

        return sstar_cis_write_back_to_nand(&nand_storage);
    }
#endif

    return 0;
}

static u8 sstar_cis_load_from_nor(struct spinor_handle *boot_storage)
{
    u32            cis_address;
    spinor_nri_t * spinor_nri;
    spinor_info_t *spinor_info;

    if (ERR_SPINOR_SUCCESS != mdrv_spinor_read(boot_storage, CIS_IPL_OFFSET, cis.map[0], 16))
        return 0;

    cis_address = flash_impl_get_ipl_size(cis.map[0]);
    cis_address = (cis_address + 0xfff) & ~0xfff;

    for (; (CIS_SEARCH_END > cis_address) && (cis.store_cnt < CIS_MAPPING_CNT); cis_address += CIS_PAGE_SIZE)
    {
        if (ERR_SPINOR_SUCCESS != mdrv_spinor_read(boot_storage, cis_address, cis.map[cis.store_cnt], CIS_PAGE_SIZE))
            break;

        /* NRI cis become 1k after iford, contain old nri of 0x300 bytes and rom nri of 0x100 bytes*/
        if (sstar_cis_is_sni(cis.map[cis.store_cnt] + CIS_ROM_SIZE))
        {
            cis.store_cnt++;
            continue;
        }

        if (sstar_cis_is_nri(cis.map[cis.store_cnt] + CIS_ROM_SIZE))
        {
            spinor_nri  = (spinor_nri_t *)(cis.map[cis.store_cnt] + CIS_ROM_SIZE);
            spinor_info = &spinor_nri->spinor_info;

            if ((cis.boot_cis == 0xFF) && mdrv_spinor_is_nri_match(boot_storage, spinor_info))
            {
                cis.boot_cis = cis.store_cnt;
            }

            cis.store_cnt++;
            continue;
        }

        if (sstar_cis_is_pni(cis.map[cis.store_cnt]))
        {
            cis.store_cnt++;
            return 1;
        }
    }

    return 0;
}

u8 sstar_cis_get_nri_from_dram(void *handle, u8 *nri_list)
{
    u8                    i           = 0;
    spinor_nri_t *        spinor_nri  = NULL;
    spinor_info_t *       spinor_info = NULL;
    struct spinor_handle *nor_handle  = handle;

    if (!nri_list)
        return 0;

    do
    {
        if (sstar_cis_is_nri((cis.map[i] + CIS_ROM_SIZE)))
        {
            spinor_nri  = (spinor_nri_t *)(cis.map[i] + CIS_ROM_SIZE);
            spinor_info = &spinor_nri->spinor_info;

            if (mdrv_spinor_is_nri_match(nor_handle, spinor_info))
            {
                break;
            }
        }

        i++;
    } while (cis.map[i] && (i < cis.store_cnt));

    if (cis.store_cnt >= CIS_MAPPING_CNT)
        return 0;

    while (sstar_cis_is_nri_header((nri_list + CIS_ROM_SIZE)))
    {
        spinor_nri  = (spinor_nri_t *)(nri_list + CIS_ROM_SIZE);
        spinor_info = &spinor_nri->spinor_info;

        if (mdrv_spinor_is_nri_match(nor_handle, spinor_info))
        {
            memcpy((void *)cis.map[i], (void *)nri_list, (CIS_NRI_SIZE + CIS_ROM_SIZE));
            break;
        }

        nri_list += (CIS_NRI_SIZE + CIS_ROM_SIZE);
    }

    if (!sstar_cis_is_nri_header((nri_list + CIS_ROM_SIZE)))
        return 0;

    nor_handle->nri = (spinor_nri_t *)(cis.map[i] + CIS_ROM_SIZE);

    if (i == cis.store_cnt)
        cis.store_cnt++;

    return 1;
}

u8 sstar_cis_get_nri(void *handle)
{
    u8                    i = 0;
    spinor_nri_t *        spinor_nri;
    spinor_info_t *       spinor_info;
    struct spinor_handle *nor_handle = handle;

    if (!cis.inited)
        return 0;

    do
    {
        if (sstar_cis_is_nri(cis.map[i] + CIS_ROM_SIZE))
        {
            spinor_nri  = (spinor_nri_t *)(cis.map[i] + CIS_ROM_SIZE);
            spinor_info = &spinor_nri->spinor_info;

            if (mdrv_spinor_is_nri_match(nor_handle, spinor_info))
            {
                nor_handle->nri = spinor_nri;
                break;
            }
        }

        i++;
    } while (cis.map[i] && (i < cis.store_cnt));

    if ((i == cis.store_cnt) && (cis.store_cnt < CIS_MAPPING_CNT))
    {
        return sstar_cis_nri_write_back(nor_handle, cis.map[cis.store_cnt]);
    }

    return 1;
}
#endif

static u8 sstar_cis_update(void)
{
#if defined(CONFIG_SSTAR_NOR)
    if (cis.storage_type == FLASH_BOOT_STORAGE_NOR)
    {
        struct spinor_handle nor_storage;
        nor_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nor_storage.msg.cs_select = 0;
        nor_storage.nri           = NULL;

        mdrv_spinor_setup_by_default(&nor_storage);

        return sstar_cis_write_back_to_nor(&nor_storage);
    }
#endif
#if defined(CONFIG_SSTAR_NAND)
    if (cis.storage_type == FLASH_BOOT_STORAGE_NAND)
    {
        struct spinand_handle nand_storage;
        nand_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nand_storage.msg.cs_select = 0;
        nand_storage.sni           = NULL;

        mdrv_spinand_setup_by_default(&nand_storage);

        return sstar_cis_write_back_to_nand(&nand_storage);
    }
#endif

    return 1;
}

struct parts_tbl *sstar_cis_get_pni(void)
{
    u8                i    = 0;
    struct parts_tbl *info = NULL;

    do
    {
        if (sstar_cis_is_pni(cis.map[i]))
        {
            info = (struct parts_tbl *)(cis.map[i]);
            break;
        }

        i++;
    } while (cis.map[i] && (i < cis.store_cnt));

    return info;
}
u8 sstar_cis_update_sni(u8 *sni_buf)
{
    spinand_sni_t * sni;
    spinand_info_t *spinand_info;

    sni          = (spinand_sni_t *)(sni_buf);
    spinand_info = &sni->spinand_info;

    sni->crc32 = FLASH_IMPL_CRC32(0, (void *)spinand_info, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4));

    return sstar_cis_update();
}
u8 sstar_cis_update_nri(u8 *nri_buf)
{
    spinor_nri_t * spinor_sni;
    spinor_info_t *spinor_info;

    spinor_sni  = (spinor_nri_t *)(nri_buf);
    spinor_info = &spinor_sni->spinor_info;

    spinor_sni->crc32 = FLASH_IMPL_CRC32(0, (void *)spinor_info, (CIS_NRI_SIZE - CIS_HEADER_SIZE - 0x4));

    return sstar_cis_update();
}
u8 sstar_cis_update_pni(u8 *pni_buf)
{
    struct parts_tbl *tbl = NULL;

    tbl = (struct parts_tbl *)pni_buf;

    tbl->checksum = flash_impl_checksum((u8 *)&(tbl->size), tbl->size + sizeof(tbl->size));

    return sstar_cis_update();
}

u8 sstar_cis_init(u8 *cis_buffer, u8 cnt, u8 *fcie_buf, u8 fcie_buf_path, u8 load_cis)
{
    u8 i;

    if (cis.inited)
        return 0;

    cis.boot_cis     = 0xFF;
    cis.store_cnt    = 0;
    cis.storage_type = flash_impl_get_boot_storage();

    for (i = 0; i < cnt; i++)
    {
        cis.map[i] = cis_buffer + (CIS_PAGE_SIZE * i);
    }

    cis.fcie_buf      = fcie_buf;
    cis.fcie_buf_path = fcie_buf_path;

    if (!load_cis)
    {
        cis.store_cnt = cnt;
        goto bypass_load_cis;
    }

#if defined(CONFIG_SSTAR_NAND)
    if (cis.storage_type == FLASH_BOOT_STORAGE_NAND)
    {
        struct spinand_handle nand_storage;
        nand_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nand_storage.msg.cs_select = 0;
        nand_storage.sni           = NULL;

        mdrv_spinand_setup_by_default(&nand_storage);

        sstar_cis_load_from_nand(&nand_storage);
    }
#endif

#if defined(CONFIG_SSTAR_NOR)
    if (cis.storage_type == FLASH_BOOT_STORAGE_NOR)
    {
        struct spinor_handle nor_storage;
        nor_storage.ctrl_id       = spiflash_get_boot_storage_master();
        nor_storage.msg.cs_select = 0;
        nor_storage.nri           = NULL;

        mdrv_spinor_setup_by_default(&nor_storage);

        sstar_cis_load_from_nor(&nor_storage);
    }
#endif

bypass_load_cis:

    cis.inited = !!cis.store_cnt;

    return cis.store_cnt;
}

void sstar_cis_deinit(void)
{
    flash_impl_memset((u8 *)cis.map, 0, sizeof(cis.map));
    cis.store_cnt = 0;
    cis.inited    = 0;
}
