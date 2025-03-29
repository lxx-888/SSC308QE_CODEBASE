/*
 * ut_aes.c- Sigmastar
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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/file.h>
#include <ms_msys.h>
#include <drv_cipher.h>
#include <cam_os_wrapper.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include "ut_aes.h"

char strRreportBuf[128];

int g_Debug   = 1;
int g_RSA_MSG = 0;

// KERN_DEBUG KERN_ALERT KERN_WARNING
#define UT_CIPHER_DBG(fmt, arg...)                       \
    (                                                    \
        {                                                \
            if (g_Debug == 1)                            \
                printk(KERN_CONT KERN_ALERT fmt, ##arg); \
        })

#define UT_RSA_CIPHER_DBG(fmt, arg...)                   \
    (                                                    \
        {                                                \
            if (g_RSA_MSG == 1)                          \
                printk(KERN_CONT KERN_ALERT fmt, ##arg); \
        })

static u64 TimespecDiffNs(CamOsTimespec_t *pstStart, CamOsTimespec_t *pstStop)
{
    return (u64)(pstStop->nSec - pstStart->nSec) * 1000000000 + pstStop->nNanoSec - pstStart->nNanoSec;
}

void Dump_data(void *addr, int size)
{
    char *out_buf = (char *)addr;
    int   i;

    for (i = 0; i < size; i++)
    {
        UT_CIPHER_DBG("%02x ", out_buf[i]);
        if (i % 16 == 15)
            UT_CIPHER_DBG("\n");
    }

    UT_CIPHER_DBG("\n");
}

struct file *OpenFile(char *path, int flag, int mode)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    filp = filp_open(path, flag, mode);
    set_fs(oldfs);
    if (IS_ERR(filp))
    {
        return NULL;
    }
    return filp;
}

int ReadFile(struct file *fp, char *buf, int readlen)
{
    int ret = 0;
    ret     = kernel_read(fp, buf, readlen, &fp->f_pos);
    return ret;
}

int Get_FileSize(struct file *fp)
{
    int          len;
    struct file *filp = NULL;
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    len = fp->f_inode->i_size;
    set_fs(oldfs);

    return len;
}

int WriteFile(struct file *fp, char *buf, int writelen)
{
    int ret;

    ret = kernel_write(fp, buf, writelen, &fp->f_pos);
    return 0;
}

int CloseFile(struct file *fp)
{
    filp_close(fp, NULL);
    return 0;
}

static void *alloc_dmem(const char *name, unsigned int size, unsigned long long *addr)
{
    MSYS_DMEM_INFO dmem;

    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    if (0 != msys_request_dmem(&dmem))
    {
        return NULL;
    }
    *addr = dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}

static void free_dmem(const char *name, unsigned int size, void *virt, unsigned long long addr)
{
    MSYS_DMEM_INFO dmem;

    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    dmem.kvirt  = (void *)(unsigned long)((uintptr_t)virt);
    dmem.phys   = (unsigned long long)((uintptr_t)addr);
    msys_release_dmem(&dmem);
}

static void Aes_EncDec_Algo(aes_cipher_t *aes_cipher)
{
    MDRV_AES_HANDLE aes             = {0};
    u8              m_aes_hwkey[16] = {'S', 'S', 't', 'a', 'r', 'U'};

    // do encrypt
    cipher_aes_init();
#if (CRYPTO_ONCE)

    aes.in     = (u8 *)aes_cipher->p_inbuf_va;
    aes.out    = (u8 *)aes_cipher->p_outbuf_va;
    aes.in_pa  = aes_cipher->inbuf_pa;
    aes.out_pa = aes_cipher->outbuf_pa;
    if (aes_cipher->file_Virt == NULL)
        aes.len = AEC_ALGO_DATA_SZ - aes_cipher->cut_len;
    else
        aes.len = aes_cipher->filelen;

    aes.mode = aes_cipher->E_ALGO_MODE;

    if (aes_cipher->keylen == 16)
        memcpy(aes.key, m_aes_key, aes_cipher->keylen); // AES-128
    else if (aes_cipher->keylen == 24)
        memcpy(aes.key, m_aes_key_24, aes_cipher->keylen); // AES-192
    else if (aes_cipher->keylen == 32)
        memcpy(aes.key, m_aes_key_32, aes_cipher->keylen); // AES-256

    if (aes_cipher->aeskey_flag != 0)
    {
        memcpy(aes.key, aes_cipher->aeskey, aes_cipher->keylen); // AES-128
    }

    if (aes_cipher->hwkey_flag != 0)
    {
        if (aes_cipher->b_decrypt != 0)
        {
            m_aes_hwkey[7] = 0xff;
        }
        m_aes_hwkey[6] = aes_cipher->hwkey;
        memcpy(aes.key, m_aes_hwkey, aes_cipher->keylen); // AES-128
    }

    aes.keylen = aes_cipher->keylen;
    if (E_AES_ALGO_CBC == aes_cipher->E_ALGO_MODE)
        memcpy(aes.iv, m_aes_iv, 16); // AES-128
    if (aes_cipher->b_decrypt == 0)
    {
        cipher_aes_encrypt(&aes);
    }
    else
    {
        cipher_aes_decrypt(&aes);
    }
#else
    {
        u32 bytes = 0;
        u32 left;

        if (aes_cipher->file_Virt == NULL)
            left = AEC_ALGO_DATA_SZ;
        else
            left = aes_cipher->filelen;

        if (aes_cipher->keylen == 16)
            memcpy(aes.key, m_aes_key, aes_cipher->keylen); // AES-128
        else if (aes_cipher->keylen == 24)
            memcpy(aes.key, m_aes_key_24, aes_cipher->keylen); // AES-192
        else if (aes_cipher->keylen == 32)
            memcpy(aes.key, m_aes_key_32, aes_cipher->keylen); // AES-256

        if (aes_cipher->hwkey_flag != 0)
        {
            if (aes_cipher->b_decrypt != 0)
            {
                m_aes_hwkey[7] = 0xff;
            }
            m_aes_hwkey[6] = aes_cipher->hwkey;
            memcpy(aes.key, m_aes_hwkey, aes_cipher->keylen); // AES-128
        }

        aes.mode = aes_cipher->E_ALGO_MODE;
        if (E_AES_ALGO_CBC == aes_cipher->E_ALGO_MODE)
            memcpy(aes.iv, m_aes_iv, 16); // AES-128

        while (left >= AES_SEGMENT_UNIT)
        {
            aes.in     = (u8 *)aes_cipher->p_inbuf_va + bytes;
            aes.out    = (u8 *)aes_cipher->p_inbuf_va + bytes;
            aes.in_pa  = aes_cipher->inbuf_pa + bytes;
            aes.out_pa = aes_cipher->outbuf_pa + bytes;
            aes.len    = AES_SEGMENT_UNIT;

            if (aes_cipher->b_decrypt == 0)
            {
                cipher_aes_encrypt(&aes);
            }
            else
            {
                cipher_aes_decrypt(&aes);
            }
            bytes += AES_SEGMENT_UNIT;
            left -= AES_SEGMENT_UNIT;
        }
        if (left)
        {
            aes.in     = (u8 *)aes_cipher->p_inbuf_va + bytes;
            aes.out    = (u8 *)aes_cipher->p_inbuf_va + bytes;
            aes.in_pa  = aes_cipher->inbuf_pa + bytes;
            aes.out_pa = aes_cipher->outbuf_pa + bytes;
            aes.len    = left;
            if (aes_cipher->b_decrypt == 0)
            {
                cipher_aes_encrypt(&aes);
            }
            else
            {
                cipher_aes_decrypt(&aes);
            }
        }
    }
#endif
    cipher_aes_uninit();
}

#define KMALLOC_USE 0

static int TestAES_Algo(char *name, aes_cipher_t *aes_cipher)
{
    unsigned long long p_inbuf_pa; // unsigned long long
    void *             p_buf_va     = NULL;
    int                def_file_len = AEC_ALGO_DATA_SZ - aes_cipher->cut_len;
    int                i_cnt;
    u64                time = 0;
    CamOsTimespec_t    stTimer, endTimer;

    if (aes_cipher->file_Virt != NULL)
        def_file_len = aes_cipher->filelen;

    aes_cipher->b_decrypt = 0;
    memset(&stTimer, 0, sizeof(stTimer));
    memset(&endTimer, 0, sizeof(endTimer));

#if KMALLOC_USE
    p_buf_va = kmalloc(def_file_len * 2, GFP_KERNEL);
    if (p_buf_va == NULL)
    {
        printk("kmalloc faill . size (%x)\n", def_file_len * 2);
    }
    p_inbuf_pa = __pa(p_buf_va);
#else
    p_buf_va = alloc_dmem(name, (def_file_len * 2 + 1), &p_inbuf_pa);
#endif

    CamOsGetMonotonicTime(&stTimer);

    // for PZ1 EFFI & MCM and test encryption and decryption Frame rate in Asic
    // Aes encryption and decryption in a loop to ensure that there is enough data for test
    //
    for (i_cnt = 0; i_cnt <= aes_cipher->EFFI; i_cnt++)
    {
        // allocate input buffer, to test not align addr
        aes_cipher->p_inbuf_va  = p_buf_va + 1;
        aes_cipher->p_outbuf_va = p_buf_va + def_file_len + 1;
        aes_cipher->inbuf_pa    = p_inbuf_pa + 1;
        aes_cipher->outbuf_pa   = p_inbuf_pa + def_file_len + 1;

        if (aes_cipher->file_Virt == NULL)
        {
            memset(AesGoldenData, 0xa5, def_file_len);
            memcpy(aes_cipher->p_inbuf_va, AesGoldenData, def_file_len);
        }
        else
        {
            memcpy(aes_cipher->p_inbuf_va, aes_cipher->file_Virt, def_file_len);
        }
        memset(aes_cipher->p_outbuf_va, 0, def_file_len);

        UT_CIPHER_DBG("[%s ENC] InPA  [x%llX] InVA  [x%X]\r\n", name, aes_cipher->inbuf_pa,
                      (u32)aes_cipher->p_inbuf_va);

        UT_CIPHER_DBG("[%s ENC] OutPA  [x%llX] OutVA  [x%X]\r\n", name, aes_cipher->outbuf_pa,
                      (u32)aes_cipher->p_outbuf_va);
        aes_cipher->b_decrypt = 0;

        Aes_EncDec_Algo(aes_cipher); // AES ECB encrypt UT
        if (aes_cipher->file_Virt != NULL || aes_cipher->cut_len != 0)
        {
            printk("[result]:\n");
            UT_CIPHER_DBG("-encrypt(16byte) :\n");
            Dump_data((void *)(aes_cipher->p_inbuf_va + def_file_len), 32);
        }
        memset(aes_cipher->p_inbuf_va, 0, def_file_len);
        aes_cipher->b_decrypt = 1;

        aes_cipher->p_inbuf_va  = (void *)(p_buf_va + def_file_len + 1);
        aes_cipher->p_outbuf_va = p_buf_va + 1;
        aes_cipher->inbuf_pa    = p_inbuf_pa + def_file_len + 1;
        aes_cipher->outbuf_pa   = p_inbuf_pa + 1;

        UT_CIPHER_DBG("[%s DEC]InPA  [x%llX] InVA  [x%X]\r\n", name, aes_cipher->inbuf_pa, (u32)aes_cipher->p_inbuf_va);

        UT_CIPHER_DBG("[%s DEC] OutPA  [x%llX] OutVA  [x%X]\r\n", name, aes_cipher->outbuf_pa,
                      (u32)aes_cipher->p_outbuf_va);

        Aes_EncDec_Algo(aes_cipher); // AES ECB decrypt UT
        // UT_CIPHER_DBG("-decrypt :\n");
        // Dump_data((void *)(aes_cipher->p_outbuf_va ), def_file_len);
    }

    CamOsGetMonotonicTime(&endTimer);
    time = TimespecDiffNs(&stTimer, &endTimer);

    printk("ALL time: %lld ns\n", time);

    if (aes_cipher->cut_len != 0)
    {
        UT_CIPHER_DBG("-decrypt(16byte) :\n");
        Dump_data((void *)(aes_cipher->p_outbuf_va), 32);
    }

    if (aes_cipher->file_Virt == NULL)
    {
        if (memcmp(AesGoldenData, aes_cipher->p_outbuf_va, def_file_len))
        {
            sprintf(strRreportBuf, "[result] %s - FAIL\n", name);
            printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
        }
        else
        {
            sprintf(strRreportBuf, "[result] %s - PASS\n", name);
            printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
        }
    }
    else
    {
        UT_CIPHER_DBG("-decrypt(16byte) :\n");
        Dump_data((void *)(aes_cipher->p_outbuf_va), 16);

        if (memcmp(aes_cipher->file_Virt, aes_cipher->p_inbuf_va, def_file_len))
        {
            sprintf(strRreportBuf, "[result] %s - FAIL\n", name);
            printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
        }
        else
        {
            sprintf(strRreportBuf, "[result] %s - PASS\n", name);
            printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
        }
    }

#if KMALLOC_USE
    kfree(p_buf_va);
#else
    free_dmem(name, def_file_len * 2, p_buf_va, p_inbuf_pa);
#endif

    return 1;
}

static void Sha_Hash(MDRV_SHA_MODE E_SHA_Mode, MDRV_SHA_HANDLE *sha, char *buf, int size)
{
    u32                input_size = SHA_DATA_SZ;
    u32                in_bufsz;
    void *             p_inbuf_va = NULL;
    unsigned long long p_inbuf_pa;
    u32                i;

    if (size == 0 || buf == NULL)
    {
        // allocate input buffer, must be multiple of 64-byte + 64
        in_bufsz   = (((input_size + 63) >> 6) << 6) + 64;
        p_inbuf_va = alloc_dmem("SHA", in_bufsz, &p_inbuf_pa);
        memset(p_inbuf_va, 0, 6);
        input_size                = 6;
        *((char *)p_inbuf_va + 0) = '1';
        *((char *)p_inbuf_va + 1) = '2';
        *((char *)p_inbuf_va + 2) = '3';
        *((char *)p_inbuf_va + 3) = '4';
        *((char *)p_inbuf_va + 4) = '5';
        *((char *)p_inbuf_va + 5) = '6';
    }
    else
    {
        // allocate input buffer, must be multiple of 64-byte + 64
        in_bufsz   = (((size + 63) >> 6) << 6) + 64;
        p_inbuf_va = alloc_dmem("SHA", in_bufsz, &p_inbuf_pa);
        memset(p_inbuf_va, 0, in_bufsz);
        memcpy(p_inbuf_va, buf, size);
        input_size = size;
    }

    if (E_SHA_Mode == E_SHA_256)
    {
        UT_CIPHER_DBG("[SHA256] PA [x%llX] VA [x%X] \r\n", p_inbuf_pa, (u32)p_inbuf_va);
    }
    else if (E_SHA_Mode == E_SHA_1)
    {
        UT_CIPHER_DBG("[SHA1] PA [x%llX]  VA [x%X]\r\n", p_inbuf_pa, (u32)p_inbuf_va);
    }
    // Dump_data((char *)(p_inbuf_va), 8);
    // Dump_data((char *)(p_inbuf_va+size-8), 8);

    // do hash
    // giving mode at initial stage, ctx will be initialized
    sha->mode = E_SHA_Mode;
    cipher_sha_init(sha);

    {
        u32 bytes = 0;
        u32 left  = input_size;
        u8 *msg;
        u64 total_bits = 0;

        while (left >= AES_SEGMENT_UNIT)
        {
            sha->u32DataPhy = (p_inbuf_pa + bytes); // giving physical addr
            sha->u32DataLen = AES_SEGMENT_UNIT;     // must be multiple of 64-byte
            cipher_sha_update(sha);
            bytes += AES_SEGMENT_UNIT; // here is just an example unit
            left -= AES_SEGMENT_UNIT;
        }
        if (left >= 64) // do sha_update with length in multiple of 64-byte
        {
            u32 len = left & ~(0x3F);

            sha->u32DataPhy = (p_inbuf_pa + bytes);
            sha->u32DataLen = len;
            cipher_sha_update(sha);
            bytes += len;
            left -= len;
        }
        // Padding the last block with total length of message
        i   = left;
        msg = (u8 *)p_inbuf_va + bytes;

        if (left < 56)
        {
            msg[i++] = 0x80;
            while (i < 56)
            {
                msg[i++] = 0x00;
            }
        }
        else
        {
            msg[i++] = 0x80;
            while (i < 64)
            {
                msg[i++] = 0x00;
            }
            sha->u32DataPhy = (p_inbuf_pa + bytes);
            sha->u32DataLen = 64;
            cipher_sha_update(sha);
            msg += 64;
            bytes += 64;
            memset(msg, 0, 56);
        }

        // Append to the padding the total message's length in bits.
        total_bits      = (input_size << 3);
        msg[63]         = total_bits;
        msg[62]         = total_bits >> 8;
        msg[61]         = total_bits >> 16;
        msg[60]         = total_bits >> 24;
        msg[59]         = total_bits >> 32;
        msg[58]         = total_bits >> 40;
        msg[57]         = total_bits >> 48;
        msg[56]         = total_bits >> 56;
        sha->u32DataPhy = (p_inbuf_pa + bytes);
        sha->u32DataLen = 64;
        cipher_sha_update(sha);
    }

    cipher_sha_final(sha);

    free_dmem("SHA", in_bufsz, p_inbuf_va, p_inbuf_pa);
    if (size == 0 || buf == NULL)
    {
        if (E_SHA_Mode == E_SHA_256)
        {
            if (cpu_to_be32(sha->u32ShaVal[0]) != 0x8D969EEF || cpu_to_be32(sha->u32ShaVal[1]) != 0x6ECAD3C2
                || cpu_to_be32(sha->u32ShaVal[2]) != 0x9A3A6292 || cpu_to_be32(sha->u32ShaVal[3]) != 0x80E686CF
                || cpu_to_be32(sha->u32ShaVal[4]) != 0x0C3F5D5A || cpu_to_be32(sha->u32ShaVal[5]) != 0x86AFF3CA
                || cpu_to_be32(sha->u32ShaVal[6]) != 0x12020C92 || cpu_to_be32(sha->u32ShaVal[7]) != 0x3ADC6C92)
            {
                sprintf(strRreportBuf, "[result] Sha256_Hash - FAIL\n");
                printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
            }
            else
            {
                sprintf(strRreportBuf, "[result] Sha256_Hash - PASS\n");
                printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
            }
        }
        else if (E_SHA_Mode == E_SHA_1)
        {
            /*input:'123456' sha-1: 7c4a8d09ca3762af61e59520943dc26494f8941b*/
            if (cpu_to_be32(sha->u32ShaVal[0]) != 0x7C4A8D09 || cpu_to_be32(sha->u32ShaVal[1]) != 0xCA3762AF
                || cpu_to_be32(sha->u32ShaVal[2]) != 0x61E59520 || cpu_to_be32(sha->u32ShaVal[3]) != 0x943DC264
                || cpu_to_be32(sha->u32ShaVal[4]) != 0x94F8941B)
            {
                sprintf(strRreportBuf, "[result] Sha1_Hash - FAIL\n");
                printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
            }
            else
            {
                sprintf(strRreportBuf, "[result] Sha1_Hash - PASS\n");
                printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
            }
        }
    }
}

unsigned char PKCS_verify(unsigned char *digest, unsigned char *signature, u32 siglen)
{
    unsigned char count;
    int           i;
    signature += siglen - 31;
    for (count = 0; count != 32 && *(digest + count) == *(signature + count); count++) // compare 256 bits
        ;

    if (count != 32)
    {
        return 0;
    }

    return 1;
}

static void Rsa_EncDecrypt(MDRV_RSA_HANDLE *rsa, char *date, unsigned int datelen, char *Ndate, char *Edate,
                           unsigned int len)
{
    memcpy(rsa->in, date, datelen);
    memcpy(rsa->exp, Edate, len);
    memcpy(rsa->modulus, Ndate, len);

    rsa->len     = datelen;
    rsa->exp_len = len;
    rsa->mod_len = len;
#ifndef SUPPORT_PKA
    if (rsa->exp[0] == 0)
        rsa->pub_ekey = 1;
    else
        rsa->pub_ekey = 0;
#endif
    cipher_rsa_crypto(rsa);
}

void rsa2048_Crypto(void)
{
    int             i;
    MDRV_RSA_HANDLE rsa_en;

    memset(&rsa_en, 0, sizeof(rsa_en));
    Rsa_EncDecrypt(&rsa_en, g_date, sizeof(g_date), N_2048, E_2048, 256);
    Rsa_EncDecrypt(&rsa_en, (char *)rsa_en.out, 256, N_2048, Pexponent_2048, 256);

    if (!memcmp(&rsa_en.out[64 - (sizeof(g_date)) / 4], g_date, sizeof(g_date)))
    {
        sprintf(strRreportBuf, "[result] %s - PASS\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
    else
    {
        printk("decrypt:\n");
        Dump_data(&rsa_en.out[64 - (sizeof(g_date)) / 4], sizeof(g_date));

        printk("date:\n");
        Dump_data(&g_date[0], sizeof(g_date));

        sprintf(strRreportBuf, "[result] %s - FAIL\n", __FUNCTION__);
        printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
    }
}

void rsa_pendingdate(char *data, int size, char *sha)
{
    int i, nsize;

    memcpy(&data[size - 32], sha, 32);
    memcpy(&data[size - 32 - sizeof(pendingdate)], pendingdate, sizeof(pendingdate));

    nsize = size - 32 - sizeof(pendingdate);

    for (i = 0; i < nsize; i++)
    {
        if (i <= 1)
            *(data + i) = i;
        else
            *(data + i) = 0xff;
    }
}

//签章
void rsa2048_test_signature(void)
{
    int             i;
    MDRV_RSA_HANDLE rsa_en;
    MDRV_SHA_HANDLE sha;
    char            sha_result[256];

    memset(&rsa_en, 0, sizeof(rsa_en));

    Sha_Hash(E_SHA_256, &sha, g_date, sizeof(g_date));
    rsa_pendingdate(sha_result, 256, (char *)&sha.u32ShaVal[0]);

    Rsa_EncDecrypt(&rsa_en, sha_result, 256, N_2048, Pexponent_2048, 256);

    if (!memcmp(&sign_2048[0], &rsa_en.out[0], sizeof(sign_2048)))
    {
        sprintf(strRreportBuf, "[result] %s - PASS\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
    else
    {
        printk("sign_2048:\n");
        Dump_data(&sign_2048[0], 256);

        printk("rsa out:\n");
        Dump_data(&rsa_en.out[0], 256);

        sprintf(strRreportBuf, "[result] %s - FAIL\n", __FUNCTION__);
        printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
    }
}

//验签
void rsa2048_sigauth(void)
{
    int             size;
    int             ret, i;
    MDRV_RSA_HANDLE rsa;
    MDRV_SHA_HANDLE sha;

    memset(&rsa, 0, sizeof(rsa));
    memset(&sha, 0, sizeof(sha));
    size = sizeof(g_date) / sizeof(g_date[0]);

    Sha_Hash(E_SHA_256, &sha, g_date, size);
    Rsa_EncDecrypt(&rsa, sign_2048, sizeof(sign_2048), N_2048, E_2048, 256);

    ret = PKCS_verify((u8 *)sha.u32ShaVal, (u8 *)rsa.out, (256 - 1));
    if (ret && sha.u32ShaVal[0] != 0)
    {
        sprintf(strRreportBuf, "[result] %s - PASS\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
    else
    {
        printk("SHA256 out:\n");
        Dump_data(sha.u32ShaVal, 8);
        printk("\n");

        printk("RSA out:\n");
        Dump_data(rsa.out, 64 * 4);

        printk("\n");
        sprintf(strRreportBuf, "[result] %s - FAIL\n", __FUNCTION__);
        printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
    }
}

#ifdef SUPPORT_RSA4096

void rsa4096_Crypto(void)
{
    int             i;
    MDRV_RSA_HANDLE rsa_en;
    // MDRV_RSA_HANDLE rsa_de;

    memset(&rsa_en, 0, sizeof(rsa_en));
    // memset(&rsa_de,0,sizeof(rsa_de));

    Rsa_EncDecrypt(&rsa_en, g_date, sizeof(g_date), N_4096, E_4096, 512);
    Rsa_EncDecrypt(&rsa_en, (char *)rsa_en.out, 512, N_4096, Pexponent_4096, 512);

    if (!memcmp(&rsa_en.out[128 - (sizeof(g_date)) / 4], g_date, sizeof(g_date)))
    {
        sprintf(strRreportBuf, "[result] %s - PASS\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
    else
    {
        printk("decrypt:\n");
        Dump_data(&rsa_en.out[128 - (sizeof(g_date)) / 4], sizeof(g_date) / 4);

        printk("date:\n");
        Dump_data(&g_date[0], sizeof(g_date));

        sprintf(strRreportBuf, "[result] %s - FAIL\n", __FUNCTION__);
        printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
    }
}

void rsa4096_sigauth(void)
{
    int size;

    int             ret, i;
    MDRV_RSA_HANDLE rsa;
    MDRV_SHA_HANDLE sha;

    memset(&rsa, 0, sizeof(rsa));
    memset(&sha, 0, sizeof(sha));
    size = sizeof(g_date) / sizeof(g_date[0]);

    Sha_Hash(E_SHA_256, &sha, g_date, size);
    Rsa_EncDecrypt(&rsa, sign_4096, sizeof(sign_4096), N_4096, E_4096, 512);

    ret = PKCS_verify((u8 *)sha.u32ShaVal, (u8 *)rsa.out, (512 - 1));
    if (ret)
    {
        sprintf(strRreportBuf, "[result] %s - PASS\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
    else
    {
        sprintf(strRreportBuf, "[result] %s - Fail\n", __FUNCTION__);
        printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);

        printk("SHA256 out:\n");
        for (i = 0; i < 8; i++)
        {
            printk("%X\n", sha.u32ShaVal[i]);
        }
        printk("\n");
        printk("RSA out:\n");
        for (i = 0; i < 8; i++)
        {
            printk("%X", rsa.out[i]);
        }
    }
}

#ifdef SUPPORT_PKA
void pka_ecc512_test(void)
{
    int           i;
    unsigned long result_Qx[16];
    unsigned long result_Qy[16];

    /* setp1 get result */
    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x04; // D_ram selection
    for (i = 0; i <= 0xF; i++)
    {
        ECC0_REG32(i) = ecc_512_0[i];
    }

    ECC2_REG32(PKA_BANK_MAIN_CTL)    = 0x00;       // Control Register selection
    ECC0_REG32(PKA_BANK_FLAGS)       = 0x00;       // Write flags
    ECC0_REG32(PKA_BANK_ENTRY_PIONT) = 0x11;       // Entry Point Set to calc_r_inv = 11
    ECC0_REG32(PKA_BANK_PROBABILITY) = 0x00;       // Setup Dial-a-DTA for 0%
    ECC0_REG32(PKA_BANK_IRQ_EN)      = 0x40000000; // Setup interrupt enable register
    ECC0_REG32(PKA_BANK_MAIN_CTL)    = 0x80000300; // Start CLUE operation for 512-bits

    // Wait until r^(-1) precomputation is done
    while ((ECC0_REG32(PKA_BANK_STACK_PIONT) & 0x00000001) != 0x00000000)
        ; // Check stack point is 0
    while ((ECC0_REG32(PKA_BANK_ISR_STATUS) & 0x40ff0000) != 0x40000000)
        ;

    ECC0_REG32(PKA_BANK_STAT) = 0x40000000;
    while ((ECC0_REG32(PKA_BANK_STAT) & 0xf0ff0000) != 0x00000000)
        ; // Check cleared status

    // step2
    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x00; // Control Register selection
    ECC0_REG32(PKA_BANK_FLAGS)    = 0x00; // Write flags

    ECC0_REG32(PKA_BANK_ENTRY_PIONT) = 0x10;
    ECC0_REG32(PKA_BANK_PROBABILITY) = 0x00;

    ECC0_REG32(PKA_BANK_IRQ_EN)   = 0x40000000;
    ECC0_REG32(PKA_BANK_MAIN_CTL) = 0x80000300;

    while ((ECC0_REG32(PKA_BANK_STACK_PIONT) & 0x0F) != 0x00000000)
        ; // Check stack point is 0
    while ((ECC0_REG32(PKA_BANK_STAT) & 0x40000000) != 0x40000000)
        ; // Check expected status
    while ((ECC0_REG32(PKA_BANK_ISR_STATUS) & 0x50ff0000) != 0x50000000)
        ; // Check expected status: stop reason

    ECC0_REG32(PKA_BANK_STAT) = 0x40000000; // Clear Status
    // while( (ECC0_REG32(PKA_BANK_STAT) & 0xf0ff0000) != 0x0) ;  // Check cleared status
    while ((ECC0_REG32(PKA_BANK_STAT) & 0xffffffff) != 0x0)
        ; // Check cleared status

    //  step 3
    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x00; // Control Register selection
    ECC0_REG32(PKA_BANK_FLAGS)    = 0x00; // Write flags

    ECC0_REG32(PKA_BANK_ENTRY_PIONT) = 0x12;
    ECC0_REG32(PKA_BANK_PROBABILITY) = 0x00;
    ECC0_REG32(PKA_BANK_IRQ_EN)      = 0x40000000;
    ECC0_REG32(PKA_BANK_MAIN_CTL)    = 0x80000300;
    while ((ECC0_REG32(PKA_BANK_STACK_PIONT) & 0x0F) != 0x00000000)
        ; // Check stack point is 0
    while ((ECC0_REG32(PKA_BANK_STAT) & 0x40FF0000) != 0x40000000)
        ; // Check expected status

    ECC0_REG32(PKA_BANK_STAT) = 0x40000000; // Clear Status
    while ((ECC0_REG32(PKA_BANK_STAT) & 0xf0ff0000) != 0x0)
        ; // Check cleared status

    // step4
    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x00000001; // A RAM

    for (i = 0; i <= 0xF; i++)
    {
        ECC0_REG32(0x20 + i) = ecc_512_1[i];
    }

    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x00000002; // A RAM
    for (i = 0; i <= 0xF; i++)
    {
        ECC0_REG32(0x20 + i) = ecc_512_2[i];
    }

    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x00000001; // A RAM
    for (i = 0; i <= 0xF; i++)
    {
        ECC0_REG32(0x60 + i) = ecc_512_3[i];
    }

    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x00000004; // A RAM

    ECC0_REG32(0x70) = 0x11034401;
    for (i = 0; i < 0xF; i++)
    {
        ECC0_REG32(0x71 + i) = 0x0000;
    }

    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x00000000; // A RAM
    ECC0_REG32(0xf)               = 0x00000000;
    ECC0_REG32(0x9)               = 0x00000000;

    ECC0_REG32(0x01) = 0x19;
    ECC0_REG32(0x11) = 0x00;
    ECC0_REG32(0x10) = 0x40000000;
    ECC0_REG32(0x00) = 0x80000300;

    while ((ECC0_REG32(0x04) & 0xf) != 0x0)
        ; // Check stack point is 0
    while ((ECC0_REG32(0x02) & 0x40ff0000) != 0x40000000)
        ; // Check expected status: stop reason should be 0 (Normal)

    ECC0_REG32(0x08) = 0x40000000; // Clear Status
    while ((ECC0_REG32(0x08) & 0xf0ff0000) != 0x00000000)
        ; // Check cleared status

    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x01;

    for (i = 0x0; i <= 0xf; i++) // Read result value of c from register A0
    {
        result_Qx[i] = ECC0_REG32(0x20 + i);
    }

    ECC2_REG32(PKA_BANK_MAIN_CTL) = 0x02;

    for (i = 0x0; i <= 0xf; i++) // Read result value of c from register A0
    {
        result_Qy[i] = ECC0_REG32(0x20 + i);
    }
    if (!memcmp(result_Qx, ecc_512_Qx, sizeof(ecc_512_Qx)) && !memcmp(result_Qy, ecc_512_Qy, sizeof(ecc_512_Qy)))
    {
        sprintf(strRreportBuf, "[result] %s - PASS\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
    else
    {
        printk("result Qx:\n");
        Dump_data(result_Qx, sizeof(result_Qx));

        printk("right Qx:\n");
        Dump_data(ecc_512_Qx, sizeof(ecc_512_Qx));

        printk("result Qy:\n");
        Dump_data(result_Qy, sizeof(result_Qy));

        printk("right Qy:\n");
        Dump_data(ecc_512_Qy, sizeof(ecc_512_Qy));

        sprintf(strRreportBuf, "[result] %s - FAIL\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
}
#endif
#endif

int base64_decode(char *base64, unsigned char *bindata)
{
    int           i = 0, j = 0;
    unsigned int  k       = 0;
    unsigned int  temp[4] = {0};
    unsigned char jump    = 0;

    for (i = 0, j = 0; base64[i] != '\0'; i += 4)
    {
        memset(temp, 0xFF, sizeof(temp));
        jump = 0;

        for (k = 0; k < 3; k++)
        {
            if (base64[i + k] == 0x0a)
            {
                // printk("jump 0x0a local %x %x\n",i+k,k);
                if (k == 0)
                    i++;
                else
                    jump = 1;
            }
        }

        for (k = 0; k < 64; k++)
        {
            if (g_base64_char[k] == base64[i])
                temp[0] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (g_base64_char[k] == base64[i + 1 + jump])
                temp[1] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (g_base64_char[k] == base64[i + 2 + jump])
                temp[2] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (g_base64_char[k] == base64[i + 3 + jump])
                temp[3] = k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC))
                       | ((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03));
        if (base64[i + 2 + jump] == '=')
        {
            break;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0))
                       | ((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F));
        if (base64[i + 3 + jump] == '=')
        {
            break;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) | ((unsigned char)(temp[3] & 0x3F));
        if (jump != 0)
            i++;
    }

    return j;
}

char pem_GetChar(parseinfo_t *stparse)
{
    char c = 0x0;

    if (stparse->buf_left_bytes != 0)
    {
        c = *stparse->buf_offset++;
    }

    if (stparse->buf_left_bytes == 0)
        stparse->buf_left_bytes = 0;
    else
        stparse->buf_left_bytes--;

    return (c);
}

void pem_Movepoint(parseinfo_t *stparse, int num)
{
    if (stparse->buf_left_bytes >= num)
    {
        stparse->buf_offset += num;
        stparse->buf_left_bytes -= num;
    }
}

char pem_GetnextChar(parseinfo_t *stparse, int num)
{
    char c = 0x0;

    if ((stparse->buf_left_bytes - num) != 0)
    {
        c = *(stparse->buf_offset + num);
    }

    return (c);
}

char pem_GetMarker(parseinfo_t *stparse, int value)
{
    char c;
    int  cnt;
    do
    {
        c = pem_GetChar(stparse);

        if (stparse->buf_left_bytes == 0)
            break;

    } while (c != value);

    return pem_GetChar(stparse);
}

int pem_Parse(parseinfo_t *stparse)
{
    unsigned char c, c1, c2;
    unsigned char Nowchar;
    int           i;
    int           b_private = 0;

    c = pem_GetMarker(stparse, 0x30);
    if (c == E_KEY)
    {
        c1 = pem_GetChar(stparse);
        c2 = pem_GetChar(stparse);
        if (c1 == 0x04 && c2 == 0xa4)
        {
            stparse->rsa_size = 256;
            UT_CIPHER_DBG("rsa2048\n");
        }
        else if (c1 == 0x09 && c2 == 0x28)
        {
            stparse->rsa_size = 512;
            UT_CIPHER_DBG("rsa4096\n");
        }
    }

    for (;;)
    {
        c = pem_GetMarker(stparse, 0x02);
        // UT_CIPHER_DBG("getNextMarker: %x\n",c);
        if (c == E_KEY)
        {
            c1 = pem_GetChar(stparse);
            c2 = pem_GetChar(stparse);

            if (stparse->rsa_size == 512)
            {
                if (c1 == (0x02) && (c2 == 0x01) && b_private == 0)
                {
                    UT_RSA_CIPHER_DBG("--public key\n");
                    Nowchar = pem_GetnextChar(stparse, 0);
                    if (Nowchar == 0x00)
                    {
                        stparse->publickey = stparse->buf_offset + 1;
                        if (g_RSA_MSG)
                        {
                            Dump_data(stparse->buf_offset + 1, stparse->rsa_size);
                        }
                    }
                    else
                    {
                        stparse->publickey = stparse->buf_offset;
                        if (g_RSA_MSG)
                        {
                            Dump_data(stparse->buf_offset, stparse->rsa_size);
                        }
                    }
                    pem_Movepoint(stparse, stparse->rsa_size);
                    b_private = 1;
                }
                else if (c1 == (0x02) && (c2 == 0x01) && b_private == 1)
                {
                    Nowchar = pem_GetnextChar(stparse, 0);
                    UT_RSA_CIPHER_DBG("--private E key\n");
                    if (Nowchar == 0x00)
                    {
                        stparse->private_Ekey = stparse->buf_offset + 1;
                        if (g_RSA_MSG)
                            Dump_data(stparse->buf_offset + 1, stparse->rsa_size);
                    }
                    else
                    {
                        stparse->private_Ekey = stparse->buf_offset;
                        if (g_RSA_MSG)
                            Dump_data(stparse->buf_offset, stparse->rsa_size);
                    }
                    pem_Movepoint(stparse, stparse->rsa_size);
                }
            }
            else if (stparse->rsa_size == 256)
            {
                if (c1 == (0x01) && (c2 == 0x01))
                {
                    UT_RSA_CIPHER_DBG("--public key\n");
                    Nowchar = pem_GetnextChar(stparse, 0);
                    if (Nowchar == 0x00)
                    {
                        stparse->publickey = stparse->buf_offset + 1;
                        if (g_RSA_MSG)
                            Dump_data(stparse->buf_offset + 1, stparse->rsa_size);
                    }
                    else
                    {
                        stparse->publickey = stparse->buf_offset;
                        if (g_RSA_MSG)
                            Dump_data(stparse->buf_offset, stparse->rsa_size);
                    }
                    pem_Movepoint(stparse, stparse->rsa_size);
                }
                if (c1 == (0x01) && (c2 == 0x00))
                {
                    Nowchar = pem_GetnextChar(stparse, 0);
                    UT_RSA_CIPHER_DBG("--private E key\n");
                    if (Nowchar == 0x00)
                    {
                        stparse->private_Ekey = stparse->buf_offset + 1;
                        if (g_RSA_MSG)
                            Dump_data(stparse->buf_offset + 1, stparse->rsa_size);
                    }
                    else
                    {
                        stparse->private_Ekey = stparse->buf_offset;
                        if (g_RSA_MSG)
                            Dump_data(stparse->buf_offset, stparse->rsa_size);
                    }
                    pem_Movepoint(stparse, stparse->rsa_size);
                }
            }
        }
        /*
        if(c == E_PUBLIC_E)
        {
            UT_CIPHER_DBG("public E key\n");
            stparse->public_Ekey = stparse->buf_offset;
            Dump_data(stparse->buf_offset, 3);
        }
        */

        if (c == 0)
            break;
        if (stparse->buf_left_bytes == 0)
            break;
    }
    return 0;
}

static ssize_t aesdma_rsa(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    char        filename[32] = {0};
    UT_RSA_MODE run_op       = E_RSA_NULL_OP;
    parseinfo_t st_parse;

    int write = 0, out_binary = 0, out_pem = 0;
    int header_size = 0, pem_size = 0, binary_size = 0;

    void *             p_binary_va     = NULL;
    void *             p_pem_va        = NULL;
    void *             p_pem_decode_va = NULL;
    unsigned long long p_binary_pa;
    unsigned long long p_pem_pa;
    unsigned long long p_pem_decode_pa;

    if (NULL != buf)
    {
        char *pBuf  = (char *)buf;
        char *p_val = NULL;
        char *pName = NULL;
        while (1)
        {
            pName = strsep(&pBuf, "=");

            if (pName != NULL && strncmp(pName, "crypto", 6) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    run_op = E_RSA_TEST_ENCRYPT_DECRYPT;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "test_sig", 8) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    run_op = E_RSA_TEST_SIGN_VERIFY;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "all", 3) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    run_op = E_RSA_TEST_ALL;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "header", 6) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;
                header_size = CamOsStrtol(p_val, NULL, 10);

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "sign", 4) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;
                run_op = E_RSA_OUTBINARY_SIGN;

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "verify", 6) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;
                run_op = E_RSA_OUTBINARY_VERIFY;

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "write", 5) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;
                write = CamOsStrtol(p_val, NULL, 10);

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "binary", 6) == 0)
            {
                struct file *fp_binary = NULL;
                p_val                  = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;

                memset(filename, 0, sizeof(filename));
                memcpy(filename, p_val, (strlen(p_val)));
                if (filename[strlen(p_val) - 1] == '\n')
                    filename[strlen(p_val) - 1] = '\0';

                fp_binary = OpenFile(filename, O_RDWR, 0644); //| O_CREAT
                if (fp_binary)
                {
                    printk("Open  %s success:\n", filename);
                }
                else
                {
                    printk("Open  %s NG:\n", filename);
                    return n;
                }
                binary_size = Get_FileSize(fp_binary);

                p_binary_va = alloc_dmem(INPUT_FILE_RSA_BINARY, binary_size, &p_binary_pa);

                ReadFile(fp_binary, p_binary_va, binary_size);
                CloseFile(fp_binary);
                out_binary = 1;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "pem", 3) == 0)
            {
                char         pem_name[32] = {0};
                struct file *fp_pem       = NULL;
                p_val                     = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rsa_usage;

                memcpy(pem_name, p_val, (strlen(p_val)));
                if (pem_name[strlen(p_val) - 1] == '\n')
                    pem_name[strlen(p_val) - 1] = '\0';

                fp_pem = OpenFile(pem_name, O_RDONLY, 0666); //| O_CREAT O_RDONLY
                if (fp_pem)
                {
                    printk("Open  %s success:\n", pem_name);
                }
                else
                {
                    printk("Open  %s NG:\n", pem_name);
                    return n;
                }
                pem_size = Get_FileSize(fp_pem);

                p_pem_va = alloc_dmem(INPUT_FILE_RSA_PEM, pem_size * 2, &p_pem_pa);
                // p_decode_va = alloc_dmem(INPUT_FILE_RSA_D, filesize, &p_decode_pa);
                p_pem_decode_va = p_pem_va + pem_size;
                p_pem_decode_pa = p_pem_pa + pem_size;
                memset(p_pem_decode_va, 0, pem_size);
                ReadFile(fp_pem, p_pem_va, pem_size);
                CloseFile(fp_pem);

                st_parse.buf_left_bytes = base64_decode((void *)p_pem_va, (void *)p_pem_decode_va);
                st_parse.buf_offset     = p_pem_decode_va;

                if (g_RSA_MSG)
                {
                    UT_CIPHER_DBG("base64 decrypt date:\n");
                    Dump_data(p_pem_decode_va, st_parse.buf_left_bytes);
                }

                pem_Parse(&st_parse);

                out_pem = 1;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "help", 4) == 0)
            {
                goto rsa_usage;
            }

            if (pName == NULL)
            {
                break;
            }
        }
    }
    else
        goto rsa_usage;

    if (run_op == E_RSA_TEST_ENCRYPT_DECRYPT)
    {
        rsa2048_Crypto();
    }
    else if (run_op == E_RSA_TEST_SIGN_VERIFY)
    {
        rsa2048_test_signature();
        rsa2048_sigauth();
    }
    else if (run_op == E_RSA_TEST_ALL)
    {
        rsa2048_Crypto();
        rsa2048_test_signature();
        rsa2048_sigauth();
#ifdef SUPPORT_RSA4096
        rsa4096_sigauth();
        rsa4096_Crypto();
#ifdef SUPPORT_PKA
        pka_ecc512_test();
#endif
#endif
    }
    else if (run_op == E_RSA_OUTBINARY_SIGN)
    {
        if (out_binary == 1 && out_pem == 1)
        {
            MDRV_RSA_HANDLE rsa_en          = {0};
            MDRV_SHA_HANDLE sha             = {0};
            char            sha_result[512] = {0};

            Sha_Hash(E_SHA_256, &sha, (char *)(p_binary_va + header_size), (binary_size - header_size));
            rsa_pendingdate(sha_result, st_parse.rsa_size, (char *)&sha.u32ShaVal[0]);
            UT_CIPHER_DBG("sha out: \n");
            Dump_data(sha_result, st_parse.rsa_size);

            Rsa_EncDecrypt(&rsa_en, sha_result, st_parse.rsa_size, st_parse.publickey, st_parse.private_Ekey,
                           st_parse.rsa_size);
            UT_CIPHER_DBG("rsa out: \n");
            Dump_data(rsa_en.out, st_parse.rsa_size);
            if (write == 1)
            {
                char         writename[16] = {0};
                struct file *fp_wfile      = NULL;

                sprintf(writename, "%s.sig", filename);
                fp_wfile = OpenFile(writename, O_RDWR | O_CREAT, 666);
                WriteFile(fp_wfile, p_binary_va, binary_size);
                WriteFile(fp_wfile, (char *)rsa_en.out, st_parse.rsa_size);
                printk("write data to %s success\n", writename);
                CloseFile(fp_wfile);
            }
        }
    }
    else if (run_op == E_RSA_OUTBINARY_VERIFY)
    {
        if (out_binary == 1 && out_pem == 1)
        {
            MDRV_RSA_HANDLE rsa_en          = {0};
            MDRV_SHA_HANDLE sha             = {0};
            char            sha_result[512] = {0};
            Sha_Hash(E_SHA_256, &sha, (char *)(p_binary_va + header_size), (binary_size - st_parse.rsa_size));
            rsa_pendingdate(sha_result, st_parse.rsa_size, (char *)&sha.u32ShaVal[0]);
            if (st_parse.rsa_size == 512)
                Rsa_EncDecrypt(&rsa_en, (char *)(p_binary_va + binary_size - st_parse.rsa_size), st_parse.rsa_size,
                               st_parse.publickey, E_4096, st_parse.rsa_size);
            if (st_parse.rsa_size == 256)
                Rsa_EncDecrypt(&rsa_en, (char *)(p_binary_va + binary_size - st_parse.rsa_size), st_parse.rsa_size,
                               st_parse.publickey, E_2048, st_parse.rsa_size);

            if (!memcmp(rsa_en.out, sha_result, st_parse.rsa_size))
            {
                sprintf(strRreportBuf, "[result] Verify RSA - PASS\n");
                printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
            }
            else
            {
                UT_CIPHER_DBG("rsa out: \n");
                Dump_data(rsa_en.out, st_parse.rsa_size);
                UT_CIPHER_DBG("sha out: \n");
                Dump_data(sha_result, st_parse.rsa_size);
                sprintf(strRreportBuf, "[result] %s - FAIL\n", __FUNCTION__);
                printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
            }
        }
    }

    if (p_pem_va != NULL)
        free_dmem(INPUT_FILE_RSA_PEM, pem_size, p_pem_va, p_pem_pa);

    if (p_binary_va != NULL)
        free_dmem(INPUT_FILE_RSA_BINARY, binary_size * 2, p_binary_va, p_binary_pa);

    return n;
rsa_usage:
    printk("rsa auto usage:\n");
    printk("   echo crypto=1   > sys/class/sstar_ut/aesdma/rsa   <begin to rsa  encrypt/decrypt  defalut>\n");
    printk("   echo test_sig=1 > sys/class/sstar_ut/aesdma/rsa   <begin to rsa signature/sigauth defalut>\n");
    printk(
        "   echo all=1      > sys/class/sstar_ut/aesdma/rsa   <begin to rsa encrypt/decrypt signature/sigauth "
        "defalut>\n");
    printk(
        "   echo pem=private.pem binary=image.sig.bin verify=1> sys/class/sstar_ut/aesdma/rsa    <verify "
        "image.sig.bin> "
        "\n");
    printk(
        "   echo pem=private.pem binary=image.bin sign=1 write=1 > sys/class/sstar_ut/aesdma/rsa <sign image.bin,and "
        "write result>\n");
    return n;
}
DEVICE_ATTR(rsa, 0200, NULL, aesdma_rsa);

static ssize_t aesdma_sha(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    char            filename[32] = {0};
    MDRV_SHA_HANDLE sha;
    int             run_tset = 0, out_binary = 0, write = 0;
    int             header_size = 0, tailsize = 0, filesize = 0;

    void *             p_inbuf_va = NULL;
    unsigned long long p_inbuf_pa;

    if (NULL != buf)
    {
        char *pBuf  = (char *)buf;
        char *p_val = NULL;
        char *pName = NULL;
        while (1)
        {
            pName = strsep(&pBuf, "=");

            if (pName != NULL && strncmp(pName, "sha1", 4) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto sha_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    Sha_Hash(E_SHA_1, &sha, NULL, 0);
                }
                run_tset = 1;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "sha256", 6) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto sha_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    Sha_Hash(E_SHA_256, &sha, NULL, 0);
                }
                run_tset = 0;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "all", 3) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto sha_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    Sha_Hash(E_SHA_1, &sha, NULL, 0);
                    Sha_Hash(E_SHA_256, &sha, NULL, 0);
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "header", 6) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto sha_usage;
                header_size = CamOsStrtol(p_val, NULL, 10);

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "tail", 4) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto sha_usage;
                tailsize = CamOsStrtol(p_val, NULL, 10);

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "write", 5) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto sha_usage;
                write = CamOsStrtol(p_val, NULL, 10);

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "binary", 6) == 0)
            {
                struct file *fp_file = NULL;
                p_val                = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto sha_usage;

                memcpy(filename, p_val, (strlen(p_val)));
                if (filename[strlen(p_val) - 1] == '\n')
                    filename[strlen(p_val) - 1] = '\0';

                fp_file = OpenFile(filename, O_RDWR, 0644); //| O_CREAT
                if (fp_file)
                {
                    UT_CIPHER_DBG("Open  %s success:\n", filename);
                }
                else
                {
                    printk("Open  %s NG:\n", filename);
                    return n;
                }
                filesize = Get_FileSize(fp_file);

                p_inbuf_va = alloc_dmem(INPUT_FILE_SHA, filesize, &p_inbuf_pa);

                ReadFile(fp_file, p_inbuf_va, filesize);
                CloseFile(fp_file);
                out_binary = 1;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "help", 4) == 0)
            {
                goto sha_usage;
            }
            if (pName == NULL)
            {
                break;
            }
        }
    }
    else
        goto sha_usage;

    if (run_tset == 0 && out_binary == 1)
    {
        Sha_Hash(E_SHA_256, &sha, (char *)(p_inbuf_va + header_size), (filesize - header_size - tailsize));
        printk("<Jump Header(0x%x) tail(0x%x). sha256 calc date len(0x%x).> \n", header_size, tailsize,
               (filesize - header_size - tailsize));
        UT_CIPHER_DBG("SHA256 result:\n");
        Dump_data(sha.u32ShaVal, 32);
        if (write == 1)
        {
            char         writename[16] = {0};
            struct file *fp_wfile      = NULL;

            sprintf(writename, "%s.sha256.bin", filename);
            fp_wfile = OpenFile(writename, O_RDWR | O_CREAT, 666);
            WriteFile(fp_wfile, (char *)&sha.u32ShaVal[0], 32);
            printk("write data to %s success\n", writename);
            CloseFile(fp_wfile);
        }
        free_dmem(INPUT_FILE_SHA, filesize, p_inbuf_va, p_inbuf_pa);
    }
    else if (run_tset == 1 && out_binary == 1)
    {
        Sha_Hash(E_SHA_1, &sha, ((char *)(p_inbuf_va + header_size)), (filesize - header_size - tailsize));
        printk("<Jump Header(0x%x) tail(0x%x). sha1 calc date len(0x%x).> \n", header_size, tailsize,
               (filesize - header_size - tailsize));
        printk("SHA1 result:\n");
        Dump_data(sha.u32ShaVal, 20);
        if (write == 1)
        {
            char         writename[16] = {0};
            struct file *fp_wfile      = NULL;

            sprintf(writename, "%s.sha1.bin", filename);
            fp_wfile = OpenFile(writename, O_RDWR | O_CREAT, 666);
            WriteFile(fp_wfile, (char *)&sha.u32ShaVal[0], 20);
            printk("write data to %s success\n", writename);
            CloseFile(fp_wfile);
        }
        free_dmem(INPUT_FILE_SHA, filesize, p_inbuf_va, p_inbuf_pa);
    }

    return n;

sha_usage:
    printk("sha auto usage:\n");
    printk("   echo sha1=1   > /sys/class/sstar_ut/aesdma/sha              <start   sha1      defalut test>\n");
    printk("   echo sha256=1 > /sys/class/sstar_ut/aesdma/sha              <start   sha256    defalut test>\n");
    printk("   echo all=1    > /sys/class/sstar_ut/aesdma/sha              <start sha1/sha256 defalut test>\n");
    printk("   echo sha1=2   binary=xx > /sys/class/sstar_ut/aesdma/sha    <input binary to calc sha1>\n");
    printk("   echo sha256=2 binary=xx > /sys/class/sstar_ut/aesdma/sha    <input binary to calc sha256>\n");

    printk("   add < write=1 >              can write sha1/sha256 result file\n");
    printk("   add < header=xx tail=xx >    can jump binary header or tail\n");
    printk("\n");
    return n;
}
DEVICE_ATTR(sha, 0200, NULL, aesdma_sha);

static ssize_t aesdma_aes(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int  keylen   = 0;
    int  run_tset = 0;
    char tmp[16]  = {0};
    char name[16] = {0};

    int  header_size = 0, filesize = 0, keysize;
    char filename[32] = {0};

    unsigned long long p_inbuf_pa;
    void *             p_inbuf_va = NULL;

    aes_cipher_t st_aes_cipher = {0};

    if (NULL != buf)
    {
        char *pBuf  = (char *)buf;
        char *p_val = NULL;
        char *pName = NULL;
        while (1)
        {
            pName = strsep(&pBuf, "=");

            if (pName != NULL && strncmp(pName, "keylen", 6) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                keylen = CamOsStrtol(p_val, NULL, 10);
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "ecb", 3) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_ECB;
                    sprintf(tmp, "AES_ECB");
                    run_tset = 1;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "cbc", 3) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_CBC;
                    sprintf(tmp, "AES_CBC");
                    run_tset = 1;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "ctr", 3) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_CTR;
                    sprintf(tmp, "AES_CTR");
                    run_tset = 1;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "all", 3) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                if (CamOsStrtol(p_val, NULL, 10) == 1)
                {
                    run_tset = 2;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "header", 6) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                header_size = CamOsStrtol(p_val, NULL, 10);

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "hwkey", 5) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                st_aes_cipher.hwkey      = CamOsStrtol(p_val, NULL, 10);
                st_aes_cipher.hwkey_flag = 1;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "t_align", 7) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;
                st_aes_cipher.t_align = CamOsStrtol(p_val, NULL, 10);
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "EFFI", 4) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;

                st_aes_cipher.EFFI = CamOsStrtol(p_val, NULL, 10);
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "binary", 6) == 0)
            {
                struct file *fp_file = NULL;
                p_val                = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;

                memcpy(filename, p_val, (strlen(p_val)));
                if (filename[strlen(p_val) - 1] == '\n')
                    filename[strlen(p_val) - 1] = '\0';

                fp_file = OpenFile(filename, O_RDWR, 0644); //| O_CREAT
                if (fp_file)
                {
                    UT_CIPHER_DBG("Open  %s success:\n", filename);
                }
                else
                {
                    printk("Open  %s NG:\n", filename);
                    return n;
                }
                filesize = Get_FileSize(fp_file);

                p_inbuf_va = alloc_dmem(INPUT_FILE_AES, filesize, &p_inbuf_pa);

                ReadFile(fp_file, p_inbuf_va, filesize);
                CloseFile(fp_file);
                run_tset = 1;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "aeskey", 6) == 0)
            {
                char         key_filename[32] = {0};
                struct file *fp_file          = NULL;
                p_val                         = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto aes_usage;

                memcpy(key_filename, p_val, (strlen(p_val)));
                if (key_filename[strlen(p_val) - 1] == '\n')
                    key_filename[strlen(p_val) - 1] = '\0';

                fp_file = OpenFile(key_filename, O_RDWR, 0644); //| O_CREAT
                if (fp_file)
                {
                    UT_CIPHER_DBG("Open  %s success:\n", key_filename);
                }
                else
                {
                    printk("Open  %s NG:\n", key_filename);
                    return n;
                }
                keysize = Get_FileSize(fp_file);
                if (keysize > 32)
                {
                    printk("aeskey len(%d) error :\n", keysize);
                    return n;
                }
                ReadFile(fp_file, st_aes_cipher.aeskey, keysize);
                CloseFile(fp_file);
                st_aes_cipher.aeskey_flag = 1;

                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "help", 4) == 0)
            {
                goto aes_usage;
            }
            if (pName == NULL)
            {
                break;
            }
        }
    }
    else
        goto aes_usage;

    if (keylen == 0)
        keylen = 128; // default test aes128

    st_aes_cipher.file_Virt = p_inbuf_va;
    st_aes_cipher.filelen   = filesize;

    if (run_tset == 1)
    {
        if (st_aes_cipher.hwkey_flag != 0)
        {
            keylen = 128;
            sprintf(name, "%s_HWKEY%d", tmp, st_aes_cipher.hwkey);
        }
        else
        {
            sprintf(name, "%s_%d", tmp, keylen);
        }
        if (st_aes_cipher.aeskey_flag == 1)
        {
            st_aes_cipher.keylen = keysize;
            printk("input key len %d\n", keysize);
        }
        st_aes_cipher.keylen = keylen / 8;
        TestAES_Algo(name, &st_aes_cipher);
    }
    if (run_tset == 2)
    {
        int i;
        for (i = 0; i < 3; i++)
        {
            int j, round;
            st_aes_cipher.keylen  = (128 + i * 64) / 8;
            st_aes_cipher.cut_len = 0;

            round = (st_aes_cipher.t_align) ? (3) : (0);
            for (j = 0; j <= round; j++)
            {
                if (st_aes_cipher.t_align == 1)
                {
                    st_aes_cipher.cut_len = (AEC_ALGO_DATA_SZ - 16) - j;
                }

                st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_ECB;
                sprintf(name, "AES_ECB_%d", st_aes_cipher.keylen * 8);
                TestAES_Algo(name, &st_aes_cipher);

                sprintf(name, "AES_CBC_%d", st_aes_cipher.keylen * 8);
                st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_CBC;
                TestAES_Algo(name, &st_aes_cipher);

                sprintf(name, "AES_CTR_%d", st_aes_cipher.keylen * 8);
                st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_CTR;
                TestAES_Algo(name, &st_aes_cipher);
            }
        }
    }

    if (p_inbuf_va != NULL)
    {
        free_dmem(INPUT_FILE_AES, filesize, p_inbuf_va, p_inbuf_pa);
    }
    return n;

aes_usage:
    printk("aes auto usage:\n");
    printk("   echo all=1 > /sys/class/sstar_ut/aesdma/aes\n");
    printk("   echo keylen=xx ecb/cbc/ctr=1 > /sys/class/sstar_ut/aesdma/aes\n");
    printk("   echo ecb/cbc/ctr=1 binary=xx hwkey=xx > /sys/class/sstar_ut/aesdma/aes\n");
    printk("   ECB/CBC/CTR=1 is necessiarily\n");
    printk("   hwkey range <1~maxkey>\n");
    printk("\n");

    return n;
}
DEVICE_ATTR(aes, 0200, NULL, aesdma_aes);

int check_repeat(int *arr, int len)
{
    int i = 0;
    int j = 0;

    for (i = 0; i < len - 1; i++)
    {
        for (j = i + 1; j < len; j++)
        {
            if (arr[i] == arr[j])
                return 1;
        }
    }
    return 0;
}

void Random_Num_Generator(void)
{
    int          i;
    unsigned int u16rand[5];
    for (i = 0; i < sizeof(u16rand) / sizeof(u16rand[0]); i++)
    {
        u16rand[i] = cipher_random_num();
        printk("Random Value: %x\n", u16rand[i]);
    }

    if (check_repeat(u16rand, sizeof(u16rand) / sizeof(u16rand[0])))
    {
        sprintf(strRreportBuf, "[result] %s - FAIL\n", __FUNCTION__);
        printk(RED_FONT "%s" WHITE_FONT, strRreportBuf);
    }
    else
    {
        sprintf(strRreportBuf, "[result] %s - PASS\n", __FUNCTION__);
        printk(GREEN_FONT "%s" WHITE_FONT, strRreportBuf);
    }
}

static ssize_t ut_rng_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int          rng = 0, binary = 0;
    int          cnt          = 0;
    char         filename[32] = {0};
    struct file *fp_file      = NULL;

    unsigned long long p_inbuf_pa;
    void *             p_inbuf_va = NULL;

    if (NULL != buf)
    {
        char *pBuf  = (char *)buf;
        char *p_val = NULL;
        char *pName = NULL;
        while (1)
        {
            pName = strsep(&pBuf, "=");

            if (pName != NULL && strncmp(pName, "rng", 3) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rng_usage;
                rng = CamOsStrtol(p_val, NULL, 10);
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "savebin", 7) == 0)
            {
                struct file *fp_file = NULL;
                p_val                = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rng_usage;

                memcpy(filename, p_val, (strlen(p_val)));
                if (filename[strlen(p_val) - 1] == '\n')
                    filename[strlen(p_val) - 1] = '\0';

                binary = 1;
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "size", 4) == 0)
            {
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto rng_usage;
                cnt = CamOsStrtol(p_val, NULL, 10);
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName != NULL && strncmp(pName, "help", 4) == 0)
            {
                goto rng_usage;
            }
            if (pName == NULL)
            {
                break;
            }
        }
    }
    else
        goto rng_usage;

    if (binary == 1)
    {
        int          i;
        int          filesize = 0x100; // default
        unsigned int random;
        fp_file = OpenFile(filename, O_RDWR | O_CREAT, 0644); //| O_CREAT

        if (fp_file)
        {
            UT_CIPHER_DBG("Open  %s success:\n", filename);
        }
        else
        {
            printk("Open  %s NG:\n", filename);
            return n;
        }
        if (cnt != 0)
            filesize = cnt / 2;

        p_inbuf_va = alloc_dmem(OUT_FILE_RNG, filesize, &p_inbuf_pa);

        for (i = 0; i < filesize; i++)
        {
            random = cipher_random_num();
            WriteFile(fp_file, (char *)&random, 2);
        }
        printk("write data to %s success\n", filename);
        CloseFile(fp_file);
        free_dmem(OUT_FILE_RNG, filesize, p_inbuf_va, p_inbuf_pa);
    }
    else if (rng == 1)
    {
        Random_Num_Generator();
    }
    return n;

rng_usage:
    printk("rng auto usage:\n");
    printk("   echo rng=1 > /sys/class/sstar_ut/aesdma/rng\n");
    printk("   echo savebin=random.bin size=262144 > /sys/class/sstar_ut/aesdma/rng\n");
    printk("\n");

    return n;
}

DEVICE_ATTR(rng, 0200, NULL, ut_rng_test);

static ssize_t aesdma_test_all(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int run_flag = 0;

    if (NULL != buf)
    {
        char *pBuf  = (char *)buf;
        char *p_val = NULL;
        char *pName = NULL;
        while (1)
        {
            pName = strsep(&pBuf, "=");

            if (pName != NULL && strncmp(pName, "all", 3) == 0)
            {
                int value;
                p_val = strsep(&pBuf, " ");
                if (NULL == p_val)
                    goto test_all_usage;
                value = CamOsStrtol(p_val, NULL, 10);
                if (value)
                {
                    run_flag = 1;
                }
                if (NULL == pBuf)
                {
                    break;
                }
            }
            if (pName == NULL)
            {
                break;
            }
        }
    }
    else
        goto test_all_usage;

    if (run_flag == 1)
    {
        char            name[16] = {0};
        int             i, keylen = 0;
        MDRV_SHA_HANDLE sha;
        aes_cipher_t    st_aes_cipher = {0};
        st_aes_cipher.file_Virt       = NULL;
        for (i = 0; i < 3; i++)
        {
            st_aes_cipher.keylen      = (128 + i * 64) / 8;
            st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_ECB;
            sprintf(name, "AES_ECB_%d", st_aes_cipher.keylen * 8);
            TestAES_Algo(name, &st_aes_cipher);

            sprintf(name, "AES_CBC_%d", st_aes_cipher.keylen * 8);
            st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_CBC;
            TestAES_Algo(name, &st_aes_cipher);

            sprintf(name, "AES_CTR_%d", st_aes_cipher.keylen * 8);
            st_aes_cipher.E_ALGO_MODE = E_AES_ALGO_CTR;
            TestAES_Algo(name, &st_aes_cipher);
        }

        Sha_Hash(E_SHA_1, &sha, NULL, 0);
        Sha_Hash(E_SHA_256, &sha, NULL, 0);
        rsa2048_Crypto();

        rsa2048_test_signature();
        rsa2048_sigauth();
#ifdef SUPPORT_RSA4096
        rsa4096_sigauth();
        rsa4096_Crypto();
#ifdef SUPPORT_PKA
        pka_ecc512_test();
#endif
#endif
    }
    Random_Num_Generator();

    return n;
test_all_usage:
    printk("usage:\n");
    printk("   echo all=1 > /sys/class/sstar_ut/aesdma/test_all");
    printk("\n");
    return n;
}

DEVICE_ATTR(test_all, 0200, NULL, aesdma_test_all);

static ssize_t ut_debug_lev(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if (NULL != buf)
    {
        sscanf(buf, "%d %d", &g_Debug, &g_RSA_MSG);
    }
    printk("debug level : %d %d \n", g_Debug, g_RSA_MSG);
    return n;
}

DEVICE_ATTR(dbg_lev, 0200, NULL, ut_debug_lev);

static struct miscdevice aesdma_dev = {
    .name = "aes_auto",
    .mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
};

dev_t Dev_auto;

static int __init aes_init(void)
{
    int ret;
    Dev_auto = MKDEV(128, 0);

    aesdma_dev.this_device = device_create(msys_get_sysfs_ut_class(), NULL, Dev_auto, NULL, AESDMA_ATUONODE_NANE);
    device_create_file(aesdma_dev.this_device, &dev_attr_rsa);
    device_create_file(aesdma_dev.this_device, &dev_attr_aes);
    device_create_file(aesdma_dev.this_device, &dev_attr_sha);
    device_create_file(aesdma_dev.this_device, &dev_attr_rng);
    device_create_file(aesdma_dev.this_device, &dev_attr_test_all);
    device_create_file(aesdma_dev.this_device, &dev_attr_dbg_lev);

    printk("\n");
    printk("< pls run this cmd to test aesdma base func >\n");
    printk("    echo all=1 > /sys/class/sstar_ut/aesdma/test_all");
    printk("\n");

    return 0;
}

static void __exit aes_exit(void)
{
    device_remove_file(aesdma_dev.this_device, &dev_attr_rsa);
    device_remove_file(aesdma_dev.this_device, &dev_attr_aes);
    device_remove_file(aesdma_dev.this_device, &dev_attr_sha);
    device_remove_file(aesdma_dev.this_device, &dev_attr_rng);
    device_remove_file(aesdma_dev.this_device, &dev_attr_test_all);
    device_remove_file(aesdma_dev.this_device, &dev_attr_dbg_lev);

    device_destroy(msys_get_sysfs_ut_class(), Dev_auto);
    return;
}

module_init(aes_init);
module_exit(aes_exit);

// MODULE_LICENSE("PROPRIETARY");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIGMASTAR");
MODULE_DESCRIPTION("AES UT");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
