/*
 * sigauth.c- Sigmastar
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
#include <image.h>
#include <common.h>
#include <command.h>
#include <malloc.h>
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
//#include <rtk.h>
#include <ipl.h>
#include <image.h>
#include <u-boot/crc.h>
#include "../drivers/sstar/flash/drv_part.h"
#include <../drivers/sstar/aesdma/drvAESDMA.h>

#ifdef CONFIG_SSTAR_NAND
#include <nand.h>
#elif CONFIG_SSTAR_NOR
#include <spi.h>
#include <spi_flash.h>
#elif CONFIG_ENV_IS_IN_MMC
#include <mmc.h>
#include <memalign.h>
#include <fs.h>

#define MMC_DEV_BOOT 0
#endif
#include <asm/cache.h>

#define PARTS_IPL_CUST  "IPL_CUST"
#define ENV_ROOTFS_SIZE "rootfs_size"
#define SBOOT_PASS      0
#define SBOOT_FAIL      1
#define SBOOT_MAGIC_1   0x41545353 // SSTA
#define SBOOT_MAGIC_2   0x54425352 // RSBT
#define SBOOT_VERSION   8
#define SBOOT_SIZE      9
#define SBOOT_AESKEYNUM 10
#define SBOOT_AES_MSG   11
#define SBOOT_ABK       12
#define SBOOT_IV        16
#define AES_FUNC        0x6 // bit1,2
#define AES_ENCRYPT     0x1 // bit0

#define SBOOT_PASS       0
#define SBOOT_FAIL       1
#define SBOOT_MAGIC_FAIL 2
#define SBOOT_KEYN_FAIL  3

#define SBOOT_GET_DATA_U8(addr, oft)  (*(volatile unsigned char *)((u32)(addr + oft)))
#define SBOOT_GET_DATA_U32(addr, oft) (*(volatile unsigned int *)((u32)(addr + oft)))

// sbot
#define SECURE_BOOT_IMAGE_NAME 0x746f6273

extern void halt(void);

typedef struct
{
    u8 aes_enable;
    u8 chain_mode;
    u8 aeskeysel;

    u32 head_size;

    /* size of the image after decompression,  include header */
    u32 image_size;

    /* store the compressed image, include header. padding and sig*/
    u32 image_addr;

    /* image padding data addr*/
    u32 padding_addr;

    u32 iv_addr;

} sboot_op;

int do_test_sig2(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32    img_sz, key_len, sig_addr, sig_len;
    MS_U64 key_addr, img_addr;

    if (argc < 6)
    {
        return CMD_RET_USAGE;
    }

    img_addr = (u32)simple_strtoul(argv[1], NULL, 16);
    img_sz   = (u32)simple_strtoul(argv[2], NULL, 16);
    key_addr = (u32)simple_strtoul(argv[3], NULL, 16);
    key_len  = (u32)simple_strtoul(argv[4], NULL, 16);
    sig_addr = (u32)simple_strtoul(argv[5], NULL, 16);
    sig_len  = (u32)simple_strtoul(argv[6], NULL, 16);

    if (runAuthenticate2(img_addr, img_sz, key_addr, key_len, sig_addr, sig_len))
        printf("runAuthenticate2 passed!\n");
    else
        printf("runAuthenticate2 failed!\n");

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(test_sig, CONFIG_SYS_MAXARGS, 1, do_test_sig2, "Test runAuthenticate2\n",
           "test_sig img_addr img_sz key_addr key_len sig_addr sig_len(bytes)\n");

int image_check_dcrc_variable(ulong hdr, ulong data, ulong len)
{
    ulong dcrc = crc32_wd(0, (unsigned char *)data, len, CHUNKSZ_CRC32);
    return (dcrc == swab32(image_get_uboot_dataCRC(hdr)));
}

int read_cust_part(char *part_name, u32 dst_addr)
{
#if defined(CONFIG_SSTAR_NAND)
    struct mtd_info * nand_flash;
    struct sstar_part part;
    loff_t            u64_env_part_offset   = 0;
    loff_t            u64_env_part_size     = 0;
    size_t            u64_env_part_tmp_size = 0;

    nand_flash = get_nand_dev_by_index(nand_curr_device);
    if (!nand_flash)
    {
        return 1;
    }

    sstar_part_get_active((u8 *)part_name, &part);

    u64_env_part_offset = (loff_t)(part.offset);
    u64_env_part_size   = (loff_t)(part.size);

    if (u64_env_part_size == 0)
    {
        return 1;
    }
    u64_env_part_tmp_size = (size_t)u64_env_part_size;
    if (nand_read_skip_bad(nand_flash, u64_env_part_offset, &u64_env_part_tmp_size, NULL, nand_flash->size,
                           (unsigned char *)(unsigned long)dst_addr))
    {
        return 1;
    }

#elif defined(CONFIG_SSTAR_NOR)
    static struct spi_flash *nor_flash;
    struct sstar_part        part;
    loff_t                   u64_env_part_offset   = 0;
    loff_t                   u64_env_part_size     = 0;
    size_t                   u64_env_part_tmp_size = 0;

    nor_flash = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
    if (!nor_flash)
    {
        return 1;
    }

    sstar_part_get_active((u8 *)part_name, &part);

    u64_env_part_offset = (loff_t)(part.offset);
    u64_env_part_size   = (loff_t)(part.size);

    u64_env_part_tmp_size = (size_t)u64_env_part_size;

    spi_flash_read(nor_flash, u64_env_part_offset, u64_env_part_tmp_size, (void *)(unsigned long)dst_addr);

#elif defined(CONFIG_ENV_IS_IN_MMC)

    struct mmc *mmc;
    char        str_part[4], tmp_buf[32];

    char * p_part = str_part;
    int    part, ret;
    u64    cust_addr;
    int    cust_size;
    loff_t u64_env_part_size = 0;

    mmc = find_mmc_device(MMC_DEV_BOOT);
    ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, 102400);

    // sd op
    if (IS_MMC(mmc))
    {
        run_command("mmc partconf 0", 0);
        p_part = env_get("partconf");
        part   = simple_strtoul(p_part, NULL, 10);

        ret = blk_select_hwpart_devnum(IF_TYPE_MMC, MMC_DEV_BOOT, part);
        printf("switch to partitions #%d, %s\n", part, (!ret) ? "OK" : "ERROR");

        ret = blk_dread(mmc_get_blk_desc(mmc), 0, 200, buffer);
        sprintf(tmp_buf, "emmc_get_cust %#lx", (unsigned long)buffer);
        run_command(tmp_buf, 0);

        p_part    = env_get("cust_address");
        cust_addr = simple_strtoul(p_part, NULL, 16);
        cust_size = image_get_ipl_size(cust_addr);

        memcpy((void *)(unsigned long)dst_addr, (void *)(unsigned long)cust_addr, cust_size + 256);
    }
    else if (IS_SD(mmc))
    {
        if (fs_set_blk_dev("mmc", "0:1", FS_TYPE_FAT))
        {
            return 1;
        }
        if (fs_read(part_name, dst_addr, 0, 0, &u64_env_part_size) < 0)
        {
            return 1;
        }
    }
#endif

    return 0;

#ifdef CONFIG_MS_NOR_ONEBIN
out:
    spi_flash_free(nor_flash);
    nor_flash = NULL;
    return 1;
#endif
}

static u8 sboot_aeschainMap(u8 chainnum)
{
    u8 mapping[] = {
        E_AESDMA_CHAINMODE_ECB,
        E_AESDMA_CHAINMODE_CTR,
        E_AESDMA_CHAINMODE_CBC,
    };

    printf("[SECURE] AES %s \n", ((mapping[chainnum] == E_AESDMA_CHAINMODE_ECB)
                                      ? "ECB"
                                      : ((mapping[chainnum] == E_AESDMA_CHAINMODE_CBC)
                                             ? "CBC"
                                             : ((mapping[chainnum] == E_AESDMA_CHAINMODE_CTR) ? "CTR" : "ECB"))));

    if (chainnum < E_AESDMA_CHAINMODE_NUM)
        return mapping[chainnum];

    return E_AESDMA_CHAINMODE_ECB;
}

static u8 sboot_aeskeyMap(u8 keynum)
{
    u8 mapping[] = {
        E_AESDMA_KEY_OTP_EFUSE_KEY256_1, E_AESDMA_KEY_OTP_EFUSE_KEY256_2, E_AESDMA_KEY_OTP_EFUSE_KEY256_3,
        E_AESDMA_KEY_OTP_EFUSE_KEY256_4, E_AESDMA_KEY_OTP_EFUSE_KEY1,     E_AESDMA_KEY_OTP_EFUSE_KEY2,
        E_AESDMA_KEY_OTP_EFUSE_KEY3,     E_AESDMA_KEY_OTP_EFUSE_KEY4,     E_AESDMA_KEY_OTP_EFUSE_KEY5,
        E_AESDMA_KEY_OTP_EFUSE_KEY6,     E_AESDMA_KEY_OTP_EFUSE_KEY7,     E_AESDMA_KEY_OTP_EFUSE_KEY8,
    };

    if (keynum >= 4 && keynum <= 15)
        return mapping[keynum - 4];
    else
        return E_AESDMA_KEY_NUM;
}

static u32 sboot_check_magic(u32 addr)
{
    u32 align_addr = (addr + 0xfUL) & (~(0xfUL));

    if (SBOOT_GET_DATA_U32(addr, 0) == SBOOT_MAGIC_1 && SBOOT_GET_DATA_U32(addr, 4) == SBOOT_MAGIC_2)
        return addr;
    else if (SBOOT_GET_DATA_U32(align_addr, 0) == SBOOT_MAGIC_1 && SBOOT_GET_DATA_U32(align_addr, 4) == SBOOT_MAGIC_2)
        return align_addr;

    printf("[SECURE] chk sboot magic fail \n");
    return 0;
}

static u8 sboot_parse_data(sboot_op *sboot)
{
    u8  ret        = 0;
    u32 start_addr = 0;

    start_addr = sboot_check_magic(sboot->padding_addr);
    if (start_addr == 0)
        ret = SBOOT_FAIL;

    sboot->padding_addr = start_addr;
    if (ret == SBOOT_PASS)
    {
        u8 aes_msg;
        // parse append data
        aes_msg = SBOOT_GET_DATA_U8(start_addr, SBOOT_AES_MSG);

        sboot->aes_enable = aes_msg & AES_ENCRYPT;
        if (sboot->aes_enable == AES_ENCRYPT)
        {
            sboot->chain_mode = sboot_aeschainMap((aes_msg & AES_FUNC) >> 1);
            sboot->aeskeysel  = sboot_aeskeyMap(SBOOT_GET_DATA_U8(start_addr, SBOOT_AESKEYNUM));
            sboot->iv_addr    = sboot->padding_addr + SBOOT_IV;
        }
    }

    // printf("[SECURE] aes_enable = 0x%x, chain_mode = 0x%x, iv_addr = 0x%x, aeskeysel = 0x%x \n", sboot->aes_enable,
    //            sboot->chain_mode, sboot->iv_addr, sboot->aeskeysel);

    return ret;
}

int do_sig_auth(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32      addr, size = 0;
    u32      key = 0, KeyN = 0;
    sboot_op st_sboot = {0};

    if (argc < 3)
    {
        return CMD_RET_USAGE;
    }

    addr = (u64)simple_strtoul(argv[1], NULL, 16);
    key  = (u64)simple_strtoul(argv[2], NULL, 16);

    if (IPLK_HEADER_CHAR != image_get_ipl_magic(key))
    {
        if (read_cust_part(PARTS_IPL_CUST, key))
        {
            printf("[U-Boot] Read IPL_CUST fail\n\r");
            return CMD_RET_FAILURE;
        }
    }

    if (IPLK_HEADER_CHAR == image_get_ipl_magic(key))
    {
        KeyN = image_get_ipl_cust_keyn_data(key);
    }
    else
    {
        printf("[U-Boot] CUST RSA Key fail\n\r");
        halt();
    }

    if (image_check_magic((const image_header_t *)addr))
    {
        size               = image_get_data_size((const image_header_t *)addr);
        st_sboot.head_size = image_get_header_size();
        if (*(u32 *)image_get_name((const image_header_t *)addr) == SECURE_BOOT_IMAGE_NAME)
        {
            st_sboot.image_addr   = addr + st_sboot.head_size;
            st_sboot.image_size   = size - SIGNATURE_APEEND_SIZE - RSA_SIG_LEN;
            st_sboot.padding_addr = st_sboot.image_addr + st_sboot.image_size;
        }
        else
        {
            st_sboot.image_addr   = addr;
            st_sboot.image_size   = size + st_sboot.head_size;
            st_sboot.padding_addr = st_sboot.image_addr + st_sboot.image_size;
        }
    }
    else if (env_get(ENV_ROOTFS_SIZE))
    {
        size                  = env_get_ulong(ENV_ROOTFS_SIZE, 16, 0);
        st_sboot.head_size    = 0;
        st_sboot.image_addr   = addr;
        st_sboot.image_size   = size - SIGNATURE_APEEND_SIZE - RSA_SIG_LEN;
        st_sboot.padding_addr = st_sboot.image_addr + st_sboot.image_size;
    }
    else
    {
        printf("[U-Boot] chk image magic fail.\n\r");
        halt();
    }

    printf("[U-Boot] Image addr = %x \n\r", st_sboot.image_addr);
    printf("[U-Boot] Image size = %d bytes\n\r", st_sboot.image_size);
    printf("[U-Boot] RSA%d \n\r", RSA_SIG_LEN * 8);

    if ((sboot_parse_data(&st_sboot) != SBOOT_PASS) || size == 0)
        halt();

    if (st_sboot.aes_enable == 1)
    {
        st_sboot.image_size = (st_sboot.image_size + 0xfUL) & (~(0xfUL));
    }
    st_sboot.image_size += SIGNATURE_APEEND_SIZE;

    if (runAuthenticate(st_sboot.image_addr, st_sboot.image_size, (U32 *)(unsigned long)KeyN))
    {
        printf("[U-Boot] Authenticate pass!\n\r");
    }
    else
    {
        printf("[U-Boot] Authenticate failed!\n\r");
        halt();
    }

    // enter decrypt flow
    if (st_sboot.aes_enable == 1)
    {
        u32 addr_chk, size_chk;

        aesdmaConfig aes_cfg = {0};

        if (st_sboot.aeskeysel >= E_AESDMA_KEY_OTP_EFUSE_KEY256_1
            && st_sboot.aeskeysel <= E_AESDMA_KEY_OTP_EFUSE_KEY256_4)
            aes_cfg.keylen = AES_KEYSIZE_256;
        else
            aes_cfg.keylen = AES_KEYSIZE_128;

        aes_cfg.u64SrcAddr = st_sboot.image_addr + image_get_header_size();
        aes_cfg.u32Size    = st_sboot.image_size - image_get_header_size() - SIGNATURE_APEEND_SIZE;
        aes_cfg.u64DstAddr = st_sboot.image_addr + image_get_header_size();
        aes_cfg.eKeyType   = st_sboot.aeskeysel;
        aes_cfg.eChainMode = st_sboot.chain_mode;
        aes_cfg.pu16Key    = 0;
        if (st_sboot.chain_mode == E_AESDMA_CHAINMODE_CTR || st_sboot.chain_mode == E_AESDMA_CHAINMODE_CBC)
        {
            aes_cfg.pu16IV = (U16 *)st_sboot.iv_addr;
            aes_cfg.bSetIV = 1;
        }
        aes_cfg.bDecrypt = 1;

        MDrv_AESDMA_Run(&aes_cfg);

        addr_chk = st_sboot.image_addr;
        size_chk = image_get_data_size((const image_header_t *)addr_chk);
        if (image_check_magic((const image_header_t *)addr_chk))
        {
            if (!image_check_dcrc_variable(addr_chk, addr_chk + st_sboot.head_size, size_chk))
            {
                printf("[U-Boot] Image CRC32 check error!\n\r");
                halt();
            }
        }
        else
        {
            printf("[U-Boot] Checksum error!\n\r");
            halt();
        }
        printf("[U-Boot] Checksum ok!\n\r");
    }

    if (*(u32 *)image_get_name((const image_header_t *)addr) == SECURE_BOOT_IMAGE_NAME)
    {
        memcpy((void *)addr, (void *)st_sboot.image_addr, st_sboot.image_size);
    }
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(sigauth, CONFIG_SYS_MAXARGS, 1, do_sig_auth, "Only verify digital signature and aes",
           "Usage: sigauth [Image Load Address] [Key Load Address] [--aes]\n\r"
           "if [Key Load Address] is zero, it means using sigmastar's key.\n\r"
           "command with [--aes] means using AES decryption, but don't use AES decrption without [--aes].\n\r");
