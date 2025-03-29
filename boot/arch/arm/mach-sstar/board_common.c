/*
 * board_common.c - Sigmastar
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
#include <stdlib.h>
#include <string.h>
#include <part.h>
#include <blk.h>
#include <image.h>
#include <image-android-dt.h>
#include <linux/sizes.h>
#include <mmc.h>
#include <part.h>
#include <cpu_func.h>
#include <dm/root.h>
#include <memalign.h>
#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>
#endif
#include <env.h>
#include <search.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * at nor/nand flash boot, we define "CONFIG_ENV_OFFSET" as
 * global var 'ss_sf_env_offset' of 'ss_nand_env_offset' to
 * support dynamic env offset feature, declare it before use.
 */
#if defined(CONFIG_SSTAR_NOR) && defined(CONFIG_ENV_IS_IN_SPI_FLASH)
extern loff_t ss_sf_env_offset;
#endif
#if defined(CONFIG_SSTAR_NAND) && defined(CONFIG_ENV_IS_IN_NAND)
extern loff_t ss_nand_env_offset;
#endif

#if defined(CONFIG_SSTAR_DTB_OVERLAY_SUPPORT)
char *board_get_dtbo_list(char *list_name)
{
    struct blk_desc *     dev_desc = NULL;
    struct disk_partition part_info;
    lbaint_t              offset;
    char *                env_buf   = NULL;
    char *                dtbo_list = NULL;
    int                   ret       = -1;

    if (!(gd->flags & GD_FLG_ENV_READY))
    {
        if (blk_get_device_by_str(CONFIG_SSTAR_BOOT_DEV, simple_itoa(CONFIG_SSTAR_BOOT_DEV_ID), &dev_desc) < 0)
        {
            printf("Error: Get dev_desc of '%s#%d' fail\n", CONFIG_SSTAR_BOOT_DEV, CONFIG_SSTAR_BOOT_DEV_ID);
            return NULL;
        }

        /*
         * CONFIG_SSTAR_ENV_PART_NAME == "" means cunrrent env is store
         * in bytes offset "CONFIG_ENV_OFFSET".
         */
        if (!strcmp(CONFIG_SSTAR_ENV_PART_NAME, ""))
        {
            // round "CONFIG_ENV_OFFSET" up to block size
            offset = DIV_ROUND_UP((u32)CONFIG_ENV_OFFSET, dev_desc->blksz);
            printf("Info: Load env from raw offset %#x\n", CONFIG_ENV_OFFSET);
        }
        else
        {
            ret = part_get_info_by_name(dev_desc, CONFIG_SSTAR_ENV_PART_NAME, &part_info);
            if (ret < 0)
            {
                printf("Error: Could not find %s partition\n", CONFIG_SSTAR_ENV_PART_NAME);
                return NULL;
            }
            offset = part_info.start;
            printf("Info: Load env from raw partition %s\n", CONFIG_SSTAR_ENV_PART_NAME);
        }

        // minimum granularity of the "blk_dread" read device is block size,
        // align to block size to prevent out-of-bounds access durning read.
        env_buf = malloc_cache_aligned(ALIGN(CONFIG_ENV_SIZE, dev_desc->blksz));
        if (!env_buf)
        {
            printf("Error: Malloc %d bytes fail\n", ALIGN(CONFIG_ENV_SIZE, dev_desc->blksz));
            return NULL;
        }

        ret = blk_dread(dev_desc, offset, DIV_ROUND_UP(CONFIG_ENV_SIZE, dev_desc->blksz), env_buf);
        if (ret != DIV_ROUND_UP(CONFIG_ENV_SIZE, dev_desc->blksz))
        {
            printf("Error: Load env from %s fail\n", CONFIG_SSTAR_BOOT_DEV);
            free(env_buf);
            return NULL;
        }

        ret = env_import(env_buf, 1, H_EXTERNAL);
        if (ret)
        {
            printf("Error:Import env fail");
            free(env_buf);
            return NULL;
        }
    }

    dtbo_list = env_get(list_name);

    return dtbo_list;
}
#endif

#if defined(CONFIG_SSTAR_UBOOT_DTB_OVERLAY)
bool board_uboot_dtb_overlay(void)
{
    struct blk_desc *dev_desc  = NULL;
    char *           dtbo_list = NULL;
    void *           new_fdt   = NULL;

    if (blk_get_device_by_str(CONFIG_SSTAR_BOOT_DEV, simple_itoa(CONFIG_SSTAR_BOOT_DEV_ID), &dev_desc) < 0)
    {
        printf("Error: Get dev_desc of '%s#%d' fail\n", CONFIG_SSTAR_BOOT_DEV, CONFIG_SSTAR_BOOT_DEV_ID);
        return false;
    }

    dtbo_list = board_get_dtbo_list("uboot_dtbo_list");
    if (!dtbo_list || !strcmp(dtbo_list, ""))
    {
        printf("Info: Dtbo list is not set or empty, not thining to do\n");
        return true;
    }

    // dtb overlay will make fdt size increase, alloc a new buffer
    // to place new fdt instead overlay to current fdt directly.
    new_fdt = malloc(SZ_64K);
    if (!new_fdt)
    {
        printf("Error: Malloc %d fail\n", SZ_64K);
        return false;
    }

    memcpy(new_fdt, gd->fdt_blob, fdt_totalsize(gd->fdt_blob));

    if (android_dt_overlay_apply_verbose(new_fdt, dtbo_list, dev_desc) < 0)
    {
        printf("Error: Uboot dtb overlay fail\n");
        free(new_fdt);
        return false;
    }

    printf("Info: Uboot dtb overlay success\n");
    gd->fdt_blob = new_fdt;

    // dm reinit, make new fdt effect
    dm_uninit();
    assert(!dm_init_and_scan(false));

    return true;
}
#endif

#ifdef CONFIG_SSTAR_KERNEL_DTB_OVERLAY
bool board_kernel_dtb_overlay(void *dtb)
{
    struct blk_desc *dev_desc  = NULL;
    char *           dtbo_list = NULL;

    if (blk_get_device_by_str(CONFIG_SSTAR_BOOT_DEV, simple_itoa(CONFIG_SSTAR_BOOT_DEV_ID), &dev_desc) < 0)
    {
        printf("Error: Get dev_desc of '%s#%d' fail\n", CONFIG_SSTAR_BOOT_DEV, CONFIG_SSTAR_BOOT_DEV_ID);
        return false;
    }

    dtbo_list = board_get_dtbo_list("kernel_dtbo_list");
    if (!dtbo_list || !strcmp(dtbo_list, ""))
    {
        printf("Info: Dtbo list is not set or empty, not thining to do\n");
        return true;
    }

    if (android_dt_overlay_apply_verbose(dtb, dtbo_list, dev_desc) < 0)
    {
        printf("Error: Kernel dtb overlay fail\n");
        return false;
    }

    printf("Info: Kernel dtb overlay success\n");
    return true;
}
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
    struct blk_desc *     desc;
    struct disk_partition info;
    int                   ret;

    /* CONFIG_SSTAR_ENV_PART_NAME == "" means cunrrent env is store
     * in offset CONFIG_ENV_OFFSET*/
    if (!strcmp(CONFIG_SSTAR_ENV_PART_NAME, ""))
    {
        *env_addr = CONFIG_ENV_OFFSET;
    }
    else
    {
        desc = mmc_get_blk_desc(mmc);
        if (!desc)
            return 1;

        ret = part_get_info_by_name(desc, CONFIG_SSTAR_ENV_PART_NAME, &info);
        if (ret < 0)
            return 1;

        *env_addr = info.start * info.blksz;
    }

    return 0;
}
#endif

__weak int sstar_prep_linux(bootm_headers_t *images)
{
#if CONFIG_SSTAR_KERNEL_ADDR_OFFSET
    printf("Info: kernel address offset %#lx\n", CONFIG_SSTAR_KERNEL_ADDR_OFFSET);
    images->ep += CONFIG_SSTAR_KERNEL_ADDR_OFFSET;

    if (images->ft_addr)
        images->ft_addr += CONFIG_SSTAR_KERNEL_ADDR_OFFSET;

    if (images->initrd_start)
    {
        images->initrd_start += CONFIG_SSTAR_KERNEL_ADDR_OFFSET;
        images->initrd_end += CONFIG_SSTAR_KERNEL_ADDR_OFFSET;
    }

    /* images->*_addr change, flush dcache. */
    if (!IS_ENABLED(CONFIG_SYS_DCACHE_OFF) && dcache_status())
    {
        flush_dcache_all();
    }
#endif

#ifdef CONFIG_SSTAR_KERNEL_DTB_OVERLAY
    if (!board_kernel_dtb_overlay(images->ft_addr))
        return -1;
#endif

    return 0;
}

void get_uuid_from_otp(u64 *uuid)
{
    *uuid = (u64)INREG16(BASE_REG_OTP2_PA + REG_ID_7C) | ((u64)INREG16(BASE_REG_OTP2_PA + REG_ID_7D) << 16)
            | ((u64)INREG16(BASE_REG_OTP2_PA + REG_ID_7E) << 32);

    return;
}

void setup_serial_number(void)
{
    char serialno[128] = "";
    u64  uuid;

    if (env_get("serial#"))
        return;

    get_uuid_from_otp(&uuid);
    if (uuid)
    {
        printf("Generating serial number by uuid: %016llx\n", uuid);
        snprintf(serialno, sizeof(serialno), "SigmaStar#%016llx", uuid);
    }
#if defined(CONFIG_RANDOM_UUID)
    else
    {
        u64 r[2] = {0};

        gen_rand_uuid((unsigned char *)r);
        printf("Generating random serial number: %016llx\n", r[0]);
        snprintf(serialno, sizeof(serialno), "SigmaStar#%016llx", r[0]);
    }
#endif
    env_set("serial#", serialno);
    env_save();
    return;
}
