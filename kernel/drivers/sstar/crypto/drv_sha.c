/*
 * drv_sha.c- Sigmastar
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/hw_random.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/spinlock.h>
#include <crypto/scatterwalk.h>
#include <crypto/algapi.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>
#include <linux/mm.h>

#include "ms_platform.h"
#include "ms_msys.h"
#include "halAESDMA.h"

#define LOOP_CNT       10000
#define SHA_MEMOEY_MAX 4096

#define SHA_DEBUG_FLAG (0)

extern spinlock_t crypto_lock;
static DEFINE_SPINLOCK(sha_lock);

typedef enum
{
    E_SHA_1_MODE        = 0,
    E_SHA_256_MODE      = 1,
    E_SHA_1_ONCE_MODE   = 2,
    E_SHA_256_ONCE_MODE = 3
} E_MDRV_SHA_MODE;

#if (SHA_DEBUG_FLAG == 1)
#define SHA_DBG(fmt, arg...) printk(KERN_ERR fmt, ##arg) // KERN_DEBUG KERN_ALERT KERN_WARNING
#else
#define SHA_DBG(fmt, arg...)
#endif

// memory probe from aes.c
extern struct aesdma_alloc_dmem
{
    dma_addr_t  aesdma_phy_addr;
    dma_addr_t  aesdma_phy_SHABuf_addr;
    const char *DMEM_AES_ENG_INPUT;
    const char *DMEM_AES_ENG_SHABUF;
    u8 *        aesdma_vir_addr;
    u8 *        aesdma_vir_SHABuf_addr;
} ALLOC_DMEM;

extern struct platform_device *psg_mdrv_aesdma;
extern void                    enableClock(void);
extern void                    disableClock(void);
extern void                    allocMem(u32 len);
extern void                    ss_aes_mem_free(void);

extern void *alloc_dmem(const char *name, unsigned int size, dma_addr_t *addr);
extern void  free_dmem(const char *name, unsigned int size, void *virt, dma_addr_t addr);

struct sstar_sha256_ctx
{
    u8  digest[SHA256_DIGEST_SIZE];
    u32 u32digest_len;
    u32 u32Bufcnt;
};

int sstar_sha_init(u8 sha256_mode)
{
    return 0;
}

int sstar_sha_update(u64 in, u32 len, u32 *state, u32 count, u8 sha_mode)
{
    u32 loop    = 0;
    u32 msg_cnt = 0;

    SHA_DBG(" %s len %d count %d\n", __FUNCTION__, len, count);

    if (sha_mode > E_SHA_256_ONCE_MODE)
    {
        printk(KERN_ERR "Error.sha_mode(%d max:%d) value is undefined.\n", sha_mode, E_SHA_256_ONCE_MODE);
        return 1;
    }
    spin_lock(&crypto_lock);

    HAL_SHA_Clear();
    HAL_SHA_Reset();

    if (sha_mode == E_SHA_256_MODE || sha_mode == E_SHA_256_ONCE_MODE)
    {
        HAL_SHA_SelMode(1);
    }
    else
    {
        HAL_SHA_SelMode(0);
    }

    if (count)
    {
        HAL_SHA_Write_InitValue(state);
        HAL_SHA_WriteWordCnt(count >> 2); // in unit of 4-bytes
        HAL_SHA_SetInitHashMode(1);
    }
    else
    {
        HAL_SHA_SetInitHashMode(0);
    }

    Chip_Flush_MIU_Pipe();

    HAL_SHA_SetAddress(in);
    HAL_SHA_SetLength(len);

    if (sha_mode == E_SHA_1_MODE || sha_mode == E_SHA_256_MODE)
    {
        HAL_SHA_ManualMode(1);
    }
    else
    {
        HAL_SHA_ManualMode(0);
    }

    HAL_SHA_Start();
    udelay(1); // sha256 cost about 1~1.4us

    while (((HAL_SHA_GetStatus() & SHARNG_CTRL_SHA_READY) != SHARNG_CTRL_SHA_READY) && (loop < LOOP_CNT))
    {
        loop++;
        // usleep_range(20, 80);
    }
    HAL_SHA_ReadOut(state);
    msg_cnt = HAL_SHA_ReadWordCnt() << 2;
    SHA_DBG("msg calculated %d -> %d\n", count, (u32)msg_cnt);
    SHA_DBG("x%x, x%x, x%x, x%x, x%x, x%x, x%x, x%x\n", state[0], state[1], state[2], state[3], state[4], state[5],
            state[6], state[7]);
    HAL_SHA_Clear();
    HAL_SHA_Reset();
    spin_unlock(&crypto_lock);

    return 0;
}

int sstar_sha_final(void)
{
    return 0;
}

static int sstar_sha256_init(struct shash_desc *desc)
{
    struct sha256_state *sctx = shash_desc_ctx(desc);

    SHA_DBG("%s %d\n", __FUNCTION__, __LINE__);

    memset(sctx, 0, sizeof(struct sha256_state));

    return 0;
}

static int sstar_sha256_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
    struct sha256_state *sctx        = shash_desc_ctx(desc);
    u32                  partial     = sctx->count % SHA256_BLOCK_SIZE;
    u64                  sstar_count = sctx->count - partial; // hardware calculate multiples of SHA256_BLOCK_SIZE once

    SHA_DBG("\t %s,#%d sctx->count:%llu len:%d u32Bufcnt %d\n", __FUNCTION__, __LINE__, sctx->count, len,
            sstar_ctx->u32Bufcnt);

    sctx->count += len;

    if ((partial + len) >= SHA256_BLOCK_SIZE)
    {
        int blocks;

        if (partial)
        {
            int p = SHA256_BLOCK_SIZE - partial;

            memcpy(sctx->buf + partial, data, p);
            data += p;
            len -= p;

            spin_lock(&sha_lock);
            memcpy(ALLOC_DMEM.aesdma_vir_SHABuf_addr, sctx->buf, SHA256_BLOCK_SIZE);
            sstar_sha_update(Chip_Phys_to_MIU(ALLOC_DMEM.aesdma_phy_SHABuf_addr), SHA256_BLOCK_SIZE, sctx->state,
                             sstar_count, E_SHA_256_MODE);
            spin_unlock(&sha_lock);

            sstar_count += SHA256_BLOCK_SIZE;
        }

        blocks = len / SHA256_BLOCK_SIZE;
        len %= SHA256_BLOCK_SIZE;

        if (blocks)
        {
            do
            {
                int opblocks =
                    (blocks < (SHA_MEMOEY_MAX / SHA256_BLOCK_SIZE)) ? (blocks) : (SHA_MEMOEY_MAX / SHA256_BLOCK_SIZE);

                spin_lock(&sha_lock);
                memcpy(ALLOC_DMEM.aesdma_vir_SHABuf_addr, data, SHA256_BLOCK_SIZE * opblocks);
                sstar_sha_update(Chip_Phys_to_MIU(ALLOC_DMEM.aesdma_phy_SHABuf_addr), SHA256_BLOCK_SIZE * opblocks,
                                 sctx->state, sstar_count, E_SHA_256_MODE);
                spin_unlock(&sha_lock);

                data += opblocks * SHA256_BLOCK_SIZE;
                sstar_count += (opblocks * SHA256_BLOCK_SIZE);
                blocks -= opblocks;
            } while (blocks);
        }
        partial = 0;
    }
    if (len)
    {
        memcpy(sctx->buf + partial, data, len);
    }

    return 0;
}

static int sstar_sha256_final(struct shash_desc *desc, u8 *out)
{
    struct sha256_state *sctx = shash_desc_ctx(desc);
    __be64               bits;
    int                  i;
    u8 *                 result           = (u8 *)sctx->state;
    unsigned int         index            = sctx->count % SHA256_BLOCK_SIZE;
    static const u8      padding[64 + 56] = {
        0x80,
    };

    SHA_DBG("\t %s,#%d %llu \n", __FUNCTION__, __LINE__, sctx->count);
    bits = cpu_to_be64((sctx->count) << 3);

    /* Pad out to 56 mod 64 */
    sstar_sha256_update(desc, padding, (index < 56) ? (56 - index) : ((64 + 56) - index));

    /* Append length field bytes */
    sstar_sha256_update(desc, (const u8 *)&bits, sizeof(bits));

    // memcpy(out, sctx->state, SHA256_DIGEST_SIZE);
    for (i = 0; i < SHA256_DIGEST_SIZE; i++)
    {
        out[i] = result[31 - i];
    }

    sctx->count = 0;
    return 0;
}

static int sstar_sha256_export(struct shash_desc *desc, void *out)
{
    SHA_DBG(" %s %d \n", __FUNCTION__, __LINE__);
    memcpy(out, shash_desc_ctx(desc), crypto_shash_descsize(desc->tfm));

    return 0;
}
static int sstar_sha256_import(struct shash_desc *desc, const void *in)
{
    memcpy(shash_desc_ctx(desc), in, crypto_shash_descsize(desc->tfm));
    return 0;
}

static int sstar_sha256_cra_init(struct crypto_tfm *tfm)
{
    enableClock();
    return 0;
}
void sstar_sha256_cra_exit(struct crypto_tfm *tfm)
{
    disableClock();
}

struct shash_alg sstar_shash_sha256_alg = {
    .digestsize = SHA256_DIGEST_SIZE,
    .init       = sstar_sha256_init,
    .update     = sstar_sha256_update,
    .final      = sstar_sha256_final,
    .export     = sstar_sha256_export,
    .import     = sstar_sha256_import,
    .descsize   = sizeof(struct sha256_state),
    .statesize  = sizeof(struct sha256_state),
    .base       = {
        .cra_name        = "sha256",
        .cra_driver_name = "sha256-sstar",
        .cra_priority    = 400,
        .cra_flags       = CRYPTO_ALG_TYPE_SHASH | CRYPTO_ALG_NEED_FALLBACK | CRYPTO_ALG_KERN_DRIVER_ONLY,
        .cra_blocksize   = SHA256_BLOCK_SIZE,
        .cra_module      = THIS_MODULE,
        .cra_ctxsize     = sizeof(struct sstar_sha256_ctx),
        .cra_init        = sstar_sha256_cra_init,
        .cra_exit        = sstar_sha256_cra_exit,
    }};

int sstar_sha_create(void)
{
    int ret = -1;

    SHA_DBG(" %s %d \n", __FUNCTION__, __LINE__);

    if (!(ALLOC_DMEM.aesdma_vir_SHABuf_addr =
              alloc_dmem(ALLOC_DMEM.DMEM_AES_ENG_SHABUF, SHA_MEMOEY_MAX, &ALLOC_DMEM.aesdma_phy_SHABuf_addr)))
    {
        printk("Allocate aesdma SHABuf fail\n");
        return ret;
    }
    else
        memset(ALLOC_DMEM.aesdma_vir_SHABuf_addr, 0, SHA_MEMOEY_MAX);

    ret = crypto_register_shash(&sstar_shash_sha256_alg);

    return ret;
}

int sstar_sha_destroy(void)
{
    ss_aes_mem_free();

    //    crypto_unregister_alg(&sstar_shash_sha256_alg);
    return 0;
}
