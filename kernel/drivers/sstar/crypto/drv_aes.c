/*
 * drv_aes.c- Sigmastar
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
#include <linux/clk-provider.h>
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
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/crypto.h>
#include <linux/spinlock.h>
#include <crypto/scatterwalk.h>
#include <crypto/algapi.h>
#include <crypto/aes.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/cipher.h>
#include <crypto/internal/skcipher.h>
#include <linux/miscdevice.h>

#include <ms_msys.h>
#include <ms_platform.h>
#include "halAESDMA.h"
#include "drv_aes.h"
#include "drv_cipher.h"
#include "drv_camclk_Api.h"
#include "drv_aes.h"

#ifdef CONFIG_SSTAR_CRYPTO_SUPPORT_AES256
int gbSupportAES256 = 1;
#else
int gbSupportAES256 = 0;
#endif

MODULE_IMPORT_NS(CRYPTO_INTERNAL);

#define AESDMA_DEBUG (0)
#if (AESDMA_DEBUG == 1)
#define AESDMA_DBG(fmt, arg...) printk(KERN_ALERT fmt, ##arg) // KERN_DEBUG KERN_ALERT KERN_WARNING
#else
#define AESDMA_DBG(fmt, arg...)
#endif
#if defined(CONFIG_SSTAR_AESDMA_INTR) && defined(CONFIG_SSTAR_AESDMA_INTR)
#define AESDMA_ISR
#endif

#define LOOP_CNT 100 // 100ms

#define CRYPT_BUF_SIZE 4096

DEFINE_SPINLOCK(crypto_lock);
struct platform_device * psg_mdrv_aesdma;
struct platform_device * psg_mdrv_rng;
struct aesdma_alloc_dmem ALLOC_DMEM = {0, 0, "AESDMA_ENG", "AESDMA_ENG1", 0, 0};
#ifdef AESDMA_ISR
static bool              _isr_requested = 0;
static struct completion _mdmadone;
#endif
#ifdef CONFIG_CAM_CLK
void **pvaesclkhandler;
#endif
extern int               sstar_sha_create(void);
extern int               sstar_sha_destroy(void);
extern struct miscdevice rsadev;

void *alloc_dmem(const char *name, unsigned int size, dma_addr_t *addr)
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
void free_dmem(const char *name, unsigned int size, void *virt, dma_addr_t addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name, name, strlen(name) + 1);
    dmem.length = size;
    dmem.kvirt  = virt;
    dmem.phys   = (unsigned long long)((uintptr_t)addr);
    msys_release_dmem(&dmem);
}

void ss_aes_mem_free(void)
{
    // aesdma_vir_SHABuf_addr
    if (ALLOC_DMEM.aesdma_vir_addr != 0)
    {
        // printk( "%s mem free \n",ALLOC_DMEM.DMEM_AES_ENG_INPUT);
        free_dmem(ALLOC_DMEM.DMEM_AES_ENG_INPUT, AESDMA_ALLOC_MEMSIZE, ALLOC_DMEM.aesdma_vir_addr,
                  ALLOC_DMEM.aesdma_phy_addr);
        ALLOC_DMEM.aesdma_vir_addr = 0;
        ALLOC_DMEM.aesdma_phy_addr = 0;
    }
    if (ALLOC_DMEM.aesdma_vir_SHABuf_addr != 0)
    {
        // printk( "%s mem free \n",ALLOC_DMEM.DMEM_AES_ENG_SHABUF);
        free_dmem(ALLOC_DMEM.DMEM_AES_ENG_SHABUF, AESDMA_ALLOC_MEMSIZE_TEMP, ALLOC_DMEM.aesdma_vir_SHABuf_addr,
                  ALLOC_DMEM.aesdma_phy_SHABuf_addr);
        ALLOC_DMEM.aesdma_vir_SHABuf_addr = 0;
        ALLOC_DMEM.aesdma_phy_SHABuf_addr = 0;
    }
}

void enableClock(void)
{
#ifdef CONFIG_CAM_CLK
    int  num_parents, i;
    int *aes_clks;

    if (of_find_property(psg_mdrv_aesdma->dev.of_node, "camclk", &num_parents))
    {
        num_parents /= sizeof(int);
        // printk( "[%s] Number : %d\n", __func__, num_parents);
        if (num_parents < 0)
        {
            printk("[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
            return;
        }
        aes_clks        = kzalloc((sizeof(int) * num_parents), GFP_KERNEL);
        pvaesclkhandler = kzalloc((sizeof(void *) * num_parents), GFP_KERNEL);
        if (!aes_clks)
        {
            return;
        }
        for (i = 0; i < num_parents; i++)
        {
            aes_clks[i] = 0;
            of_property_read_u32_index(psg_mdrv_aesdma->dev.of_node, "camclk", i, &aes_clks[i]);
            if (!aes_clks[i])
            {
                printk("[%s] Fail to get clk!\n", __func__);
            }
            else
            {
                CamClkRegister("aesdma", aes_clks[i], &pvaesclkhandler[i]);
                CamClkSetOnOff(pvaesclkhandler[i], 1);
            }
        }
        kfree(aes_clks);
    }
    else
    {
        printk("[%s] W/O Camclk \n", __func__);
    }
#else
    int          num_parents = 0, i = 0;
    struct clk **aesdma_clks;

    num_parents = of_clk_get_parent_count(psg_mdrv_aesdma->dev.of_node);

    if (num_parents > 0)
    {
        aesdma_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if (aesdma_clks == NULL)
        {
            printk("[AESDMA] -ENOMEM\n");
            return;
        }
        // enable all clk
        for (i = 0; i < num_parents; i++)
        {
            aesdma_clks[i] = of_clk_get(psg_mdrv_aesdma->dev.of_node, i);
            clk_set_rate(aesdma_clks[i], 288000000);
            if (IS_ERR(aesdma_clks[i]))
            {
                printk("[AESDMA] Fail to get clk!\n");
                kfree(aesdma_clks);
                return;
            }
            else
            {
                clk_prepare_enable(aesdma_clks[i]);
            }
            clk_put(aesdma_clks[i]);
        }
        kfree(aesdma_clks);
    }
#endif
}

void disableClock(void)
{
#ifdef CONFIG_CAM_CLK
    int  num_parents, i;
    int *aes_clks;
    if (of_find_property(psg_mdrv_aesdma->dev.of_node, "camclk", &num_parents))
    {
        num_parents /= sizeof(int);
        // printk( "[%s] Number : %d\n", __func__, num_parents);
        if (num_parents < 0)
        {
            printk("[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
            return;
        }
        aes_clks = kzalloc((sizeof(int) * num_parents), GFP_KERNEL);
        if (!aes_clks)
        {
            return;
        }
        for (i = 0; i < num_parents; i++)
        {
            of_property_read_u32_index(psg_mdrv_aesdma->dev.of_node, "camclk", i, &aes_clks[i]);
            if (!aes_clks[i])
            {
                printk("[%s] Fail to get clk!\n", __func__);
            }
            else
            {
                CamClkSetOnOff(pvaesclkhandler[i], 0);
                CamClkUnregister(pvaesclkhandler[i]);
            }
        }
        kfree(aes_clks);
        kfree(pvaesclkhandler);
        pvaesclkhandler = NULL;
    }
    else
    {
        printk("[%s] W/O Camclk \n", __func__);
    }
#else
    int num_parents = 0, i = 0;

    struct clk **aesdma_clks;

    num_parents = of_clk_get_parent_count(psg_mdrv_aesdma->dev.of_node);
    if (num_parents > 0)
    {
        aesdma_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

        if (aesdma_clks == NULL)
        {
            printk("[AESDMA] -ENOMEM\n");
            return;
        }

        // disable all clk
        for (i = 0; i < num_parents; i++)
        {
            aesdma_clks[i] = of_clk_get(psg_mdrv_aesdma->dev.of_node, i);
            if (IS_ERR(aesdma_clks[i]))
            {
                printk("[AESDMA] Fail to get clk!\n");
                kfree(aesdma_clks);
                return;
            }
            else
            {
                clk_disable_unprepare(aesdma_clks[i]);
            }
            clk_put(aesdma_clks[i]);
        }
        kfree(aesdma_clks);
    }
#endif
}

void enableRngClock(void)
{
#ifdef CONFIG_CAM_CLK
    int  num_parents, i;
    int *rng_clks;

    if (of_find_property(psg_mdrv_rng->dev.of_node, "camclk", &num_parents))
    {
        num_parents /= sizeof(int);
        // printk( "[%s] Number : %d\n", __func__, num_parents);
        if (num_parents < 0)
        {
            printk("[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
            return;
        }
        rng_clks        = kzalloc((sizeof(int) * num_parents), GFP_KERNEL);
        pvaesclkhandler = kzalloc((sizeof(void *) * num_parents), GFP_KERNEL);
        if (!rng_clks)
        {
            return;
        }
        for (i = 0; i < num_parents; i++)
        {
            rng_clks[i] = 0;
            of_property_read_u32_index(psg_mdrv_rng->dev.of_node, "camclk", i, &rng_clks[i]);
            if (!rng_clks[i])
            {
                printk("[%s] Fail to get clk!\n", __func__);
            }
            else
            {
                CamClkRegister("aesdma", rng_clks[i], &pvaesclkhandler[i]);
                CamClkSetOnOff(pvaesclkhandler[i], 1);
            }
        }
        kfree(rng_clks);
    }
    else
    {
        printk("[%s] W/O Camclk \n", __func__);
    }
#else
    int          num_parents = 0, i = 0;
    struct clk **random_clks;
    num_parents = of_clk_get_parent_count(psg_mdrv_rng->dev.of_node);
    if (num_parents > 0)
    {
        random_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if (random_clks == NULL)
        {
            printk("[RNG] -ENOMEM\n");
            return;
        }
        // enable all clk
        for (i = 0; i < num_parents; i++)
        {
            random_clks[i] = of_clk_get(psg_mdrv_rng->dev.of_node, i);

            clk_set_rate(random_clks[i], 480000000);
            if (IS_ERR(random_clks[i]))
            {
                printk("[AESDMA] Fail to get clk!\n");
                kfree(random_clks);
                return;
            }
            else
            {
                clk_prepare_enable(random_clks[i]);
            }
            clk_put(random_clks[i]);
        }
        kfree(random_clks);
    }
#endif
}

void disableRngClock(void)
{
#ifdef CONFIG_CAM_CLK
    int  num_parents, i;
    int *rng_clks;
    if (of_find_property(psg_mdrv_rng->dev.of_node, "camclk", &num_parents))
    {
        num_parents /= sizeof(int);
        // printk( "[%s] Number : %d\n", __func__, num_parents);
        if (num_parents < 0)
        {
            printk("[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
            return;
        }
        rng_clks = kzalloc((sizeof(int) * num_parents), GFP_KERNEL);
        if (!rng_clks)
        {
            return;
        }
        for (i = 0; i < num_parents; i++)
        {
            of_property_read_u32_index(psg_mdrv_rng->dev.of_node, "camclk", i, &rng_clks[i]);
            if (!rng_clks[i])
            {
                printk("[%s] Fail to get clk!\n", __func__);
            }
            else
            {
                CamClkSetOnOff(pvaesclkhandler[i], 0);
                CamClkUnregister(pvaesclkhandler[i]);
            }
        }
        kfree(rng_clks);
        kfree(pvaesclkhandler);
        pvaesclkhandler = NULL;
    }
    else
    {
        printk("[%s] W/O Camclk \n", __func__);
    }
#else
    int num_parents = 0, i = 0;

    struct clk **random_clks;

    num_parents = of_clk_get_parent_count(psg_mdrv_rng->dev.of_node);
    if (num_parents > 0)
    {
        random_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

        if (random_clks == NULL)
        {
            printk("[RNG] -ENOMEM\n");
            return;
        }

        // disable all clk
        for (i = 0; i < num_parents; i++)
        {
            random_clks[i] = of_clk_get(psg_mdrv_rng->dev.of_node, i);
            if (IS_ERR(random_clks[i]))
            {
                printk("[RNG] Fail to get clk!\n");
                kfree(random_clks);
                return;
            }
            else
            {
                clk_disable_unprepare(random_clks[i]);
            }
            clk_put(random_clks[i]);
        }
        kfree(random_clks);
    }
#endif
}

void allocMem(u32 len)
{
    if (!(ALLOC_DMEM.aesdma_vir_addr = alloc_dmem(ALLOC_DMEM.DMEM_AES_ENG_INPUT,
                                                  len, // AESDMA_ALLOC_MEMSIZE,
                                                  &ALLOC_DMEM.aesdma_phy_addr)))
    {
        printk("[input]unable to allocate aesdma memory\n");
    }
    memset(ALLOC_DMEM.aesdma_vir_addr, 0, len); // AESDMA_ALLOC_MEMSIZE);
}

#ifdef AESDMA_ISR
static irqreturn_t aes_dma_interrupt(int irq, void *argu)
{
    int status = 0;

    status = HAL_AESDMA_GetStatus();
    if (status & AESDMA_CTRL_DMA_DONE)
    {
        complete(&_mdmadone);
        HAL_AESDMA_INTDISABLE();
    }

    return IRQ_HANDLED;
}
#endif

int sstar_aes_crypt_pub(struct sstar_aes_op *op, u64 in_addr, u64 out_addr)
{
    unsigned long start = 0, err = 0;
    unsigned long timeout;
    unsigned int  wait_min = 0, wait_max = 0;
    int           bIn_atomic = 0;
    bIn_atomic               = in_atomic();

    AESDMA_DBG("%s %d\n", __FUNCTION__, __LINE__);

    Chip_Flush_MIU_Pipe();

    HAL_AESDMA_SetFileinAddr(Chip_Phys_to_MIU(in_addr));
    HAL_AESDMA_SetXIULength(op->len);
    HAL_AESDMA_SetFileoutAddr(Chip_Phys_to_MIU(out_addr), op->len);
#ifdef AESDMA_MULTIPLE_ROUND
    HAL_AESDMA_MultiRound(AESDMA_MULTIPLE_ROUND);
#endif
#if defined(CONFIG_SSTAR_CRYPTO_SUPPORT_SCA)
#ifdef CONFIG_SSTAR_RNG
    HAL_AESDMA_SCA(TRUE);
#else
    HAL_AESDMA_SCA(FALSE);
#endif
#endif

    if (op->key[0] == 'S' && op->key[1] == 'S' && op->key[2] == 't' && op->key[3] == 'a' && op->key[4] == 'r'
        && op->key[5] == 'U' && op->key[6] != 0) // SStarU
    {
        if (op->keylen == AES_KEYSIZE_128)
        {
            if (HAL_AESDMA_CheckAesKey(op->key[6]) == FALSE)
            {
                printk("[%s#%d] ERROR! eKeyType(%d)\n", __FUNCTION__, __LINE__, op->key[6]);
                return -1;
            }
            HAL_AESDMA_UseHwKey(op->key[6]);
            HAL_AESDMA_SetKeylen(op->keylen);
        }
        else if (op->keylen == AES_KEYSIZE_256)
        {
            if (HAL_AESDMA_CheckAesKey(op->key[6] * 2 - 1) == FALSE)
            {
                printk(
                    "\033[31m"
                    "[AES]ERROR!aeskey256 range must from[1,4]\r\n"
                    "\033[0m");
                return -1;
            }
            HAL_AESDMA_UseHwKey(op->key[6] * 2 - 1); // key256_1->1,key256_2->3,key256_3->5,key256_4->7
            HAL_AESDMA_SetKeylen(op->keylen);
        }
        else
        {
            printk(
                "\033[31m"
                "[AES]otpkey do not support AES_KEYSIZE_%d!"
                "\033[0m",
                8 * op->keylen);
            return -1;
        }
    }
    else // use cipher key
    {
        HAL_AESDMA_UseCipherKey();
        HAL_AESDMA_SetCipherKey((u16 *)op->key, op->keylen);
        HAL_AESDMA_SetKeylen(op->keylen);
    }

    if ((op->mode == AES_MODE_CBC) || (op->mode == AES_MODE_CTR))
    {
        HAL_AESDMA_SetIV((u16 *)op->iv);
    }

    HAL_AESDMA_Enable(AESDMA_CTRL_AES_EN);
    switch (op->mode)
    {
        case AES_MODE_ECB:
            HAL_AESDMA_SetChainModeECB();
            break;
        case AES_MODE_CBC:
            HAL_AESDMA_SetChainModeCBC();
            break;
        case AES_MODE_CTR:
            HAL_AESDMA_SetChainModeCTR();
            break;
        default:
            HAL_AESDMA_Reset();
            return -1;
    }

    if (op->dir == AES_DIR_DECRYPT)
    {
        HAL_AESDMA_CipherDecrypt();
    }

    HAL_AESDMA_FileOutEnable(1);
#ifdef AESDMA_ISR
    /* CAUTION: In tcrypt.ko test self, someone use atomic operation  that make BUG()*/
    if (_isr_requested && !bIn_atomic)
    {
        reinit_completion(&_mdmadone);
        HAL_AESDMA_INTMASK();
    }
#endif
    HAL_AESDMA_Start(1);

    start = jiffies;
#ifdef AESDMA_ISR
    if (_isr_requested && !bIn_atomic)
    {
        if (!wait_for_completion_timeout(&_mdmadone, msecs_to_jiffies(LOOP_CNT)))
        {
            err = -1;
        }
    }
    else
#endif
    {
        timeout = start + msecs_to_jiffies(LOOP_CNT);

        // Wait for ready.
        wait_min = (op->len * 8) >> 10;
        wait_max = (op->len * 10) >> 10;
        if (!bIn_atomic && wait_min >= 10)
        {
            usleep_range(wait_min, wait_max);
        }

        while ((HAL_AESDMA_GetStatus() & AESDMA_CTRL_DMA_DONE) != AESDMA_CTRL_DMA_DONE)
        {
            if (time_after_eq(jiffies, timeout))
            {
                err = -1;
                break;
            }
            /* CAUTION: In crypto test self task, crypto_ecb_crypt use atomic operation kmap_atomic that make BUG()*/
            if (!bIn_atomic)
            {
                schedule();
            }
        }
    }

    AESDMA_DBG("Elapsed time: %lu jiffies\n", jiffies - start);

    Chip_Flush_MIU_Pipe();
    HAL_AESDMA_Reset();
    if (err < 0)
    {
        printk("AES timeout\n");
        return err;
    }
    return op->len;
}

// return: op length
static unsigned int sstar_aes_crypt(struct sstar_aes_op *op)
{
    int ret = 0;

    memset(ALLOC_DMEM.aesdma_vir_addr, 0, op->len);
    memcpy(ALLOC_DMEM.aesdma_vir_addr, op->src, op->len);

    ret = sstar_aes_crypt_pub(op, ALLOC_DMEM.aesdma_phy_addr, ALLOC_DMEM.aesdma_phy_addr);
    if (ret < 0)
    {
        memset(ALLOC_DMEM.aesdma_vir_addr, 0, op->len);
    }
    memcpy(op->dst, ALLOC_DMEM.aesdma_vir_addr, op->len);

    return ret;
}

/* CRYPTO-API Functions */
static int sstar_setkey_cip(struct crypto_tfm *tfm, const u8 *key, unsigned int len)
{
    struct sstar_aes_op *op = crypto_tfm_ctx(tfm);
    int                  ret;
    ret = 0;
    AESDMA_DBG("%s %d\n", __FUNCTION__, __LINE__);

    if (gbSupportAES256 || len == AES_KEYSIZE_128)
    {
        op->keylen = len;
        memcpy(op->key, key, len);
        return 0;
    }

    /*
     * The requested key size is not supported by HW, do a fallback
     */
    op->keylen = len;
    op->cip->base.crt_flags &= ~CRYPTO_TFM_REQ_MASK;
    op->cip->base.crt_flags |= (tfm->crt_flags & CRYPTO_TFM_REQ_MASK);

    ret = crypto_cipher_setkey(op->cip, key, len);
    if (ret)
    {
        tfm->crt_flags &= ~CRYPTO_TFM_REQ_MASK;
        tfm->crt_flags |= (op->cip->base.crt_flags & CRYPTO_TFM_REQ_MASK);
    }
    return ret;
}

static int sstar_setkey_blk(struct crypto_skcipher *tfm, const u8 *key, unsigned int len)
{
    struct sstar_aes_op *op = crypto_tfm_ctx(&tfm->base);
    AESDMA_DBG("%s %d\n", __FUNCTION__, __LINE__);

    op->keylen = len;
    memcpy(op->key, key, len);
    return 0;
}

static void sstar_encrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
    struct sstar_aes_op *op  = crypto_tfm_ctx(tfm);
    int                  ret = 0;
    AESDMA_DBG("%s %d\n", __FUNCTION__, __LINE__);

    if (gbSupportAES256 || op->keylen == AES_KEYSIZE_128)
    {
        spin_lock(&crypto_lock);

        op->src   = (void *)in;
        op->dst   = (void *)out;
        op->mode  = AES_MODE_ECB;
        op->flags = 0;
        op->len   = AES_BLOCK_SIZE;
        op->dir   = AES_DIR_ENCRYPT;
        ret       = sstar_aes_crypt(op);
        if (ret < 0)
        {
            printk("[AESDMA][%s] sstar_aes_crypt return err:%d!\n", __FUNCTION__, ret);
        }

        spin_unlock(&crypto_lock);
        return;
    }
    /*
     * The requested key size is not supported by HW, do a fallback
     */
    crypto_cipher_encrypt_one(op->cip, out, in);
}

static void sstar_decrypt(struct crypto_tfm *tfm, u8 *out, const u8 *in)
{
    struct sstar_aes_op *op  = crypto_tfm_ctx(tfm);
    int                  ret = 0;
    AESDMA_DBG("%s %d\n", __FUNCTION__, __LINE__);

    if (gbSupportAES256 || op->keylen == AES_KEYSIZE_128)
    {
        spin_lock(&crypto_lock);

        op->src   = (void *)in;
        op->dst   = (void *)out;
        op->mode  = AES_MODE_ECB;
        op->flags = 0;
        op->len   = AES_BLOCK_SIZE;
        op->dir   = AES_DIR_DECRYPT;
        ret       = sstar_aes_crypt(op);
        if (ret < 0)
        {
            printk("[AESDMA][%s] sstar_aes_crypt return err:%d!\n", __FUNCTION__, ret);
        }

        spin_unlock(&crypto_lock);
        return;
    }
    /*
     * The requested key size is not supported by HW, do a fallback
     */
    crypto_cipher_decrypt_one(op->cip, out, in);
}

static int fallback_init_cip(struct crypto_tfm *tfm)
{
    // alloc a sw cipher alg as fallbck
    struct sstar_aes_op *op = crypto_tfm_ctx(tfm);
    op->cip                 = crypto_alloc_cipher("aes-generic", 0, 0);
    if (IS_ERR(op->cip))
    {
        return PTR_ERR(op->cip);
    }

    enableClock();
#ifdef CONFIG_SSTAR_RNG
    enableRngClock();
#endif
    return 0;
}
static void fallback_exit_cip(struct crypto_tfm *tfm)
{
    struct sstar_aes_op *op = crypto_tfm_ctx(tfm);

    crypto_free_cipher(op->cip);
    op->cip = NULL;

    disableClock();
#ifdef CONFIG_SSTAR_RNG
    disableRngClock();
#endif
}

static int sstar_cbc_decrypt(struct skcipher_request *req)
{
    u8           ivTemp[16] = {0};
    int          err = 0, ret = 0;
    unsigned int nbytes;

    struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
    struct skcipher_walk    walk;
    struct sstar_aes_op *   op = crypto_tfm_ctx(&skcipher->base);

    AESDMA_DBG("%s %d\n", __FUNCTION__, __LINE__);

    memset(&walk, 0, sizeof(walk));

    if (!gbSupportAES256 && op->keylen != AES_KEYSIZE_128)
    {
        // return fallback_blk_dec(desc, dst, src, nbytes);
        return 0;
    }
    memset(&walk, 0, sizeof(walk));
    spin_lock(&crypto_lock);

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = walk.nbytes))
    {
        op->iv = walk.iv;
        //          AESDMA_DBG(" 1%s %d\n",__FUNCTION__,nbytes);
        op->src = walk.src.virt.addr, op->dst = walk.dst.virt.addr;
        op->mode = AES_MODE_CBC;
        if (nbytes < AES_BLOCK_SIZE)
        {
            if (walk.total < AES_BLOCK_SIZE)
            {
                // last block which is not AES_BLOCK_SIZE aligned
                op->len = nbytes;
            }
            else
            {
                // should not go here, because CBC chunksize=AES_BLOCK_SIZE
                printk("[AESDMA][%s][%d] err: nbytes < AES_BLOCK_SIZE && walk.total >= AES_BLOCK_SIZE\n", __FUNCTION__,
                       __LINE__);
                goto finish;
            }
        }
        else
        {
            op->len = (nbytes - nbytes % AES_BLOCK_SIZE);
            memcpy(ivTemp, op->src + (op->len) - 16, 16);
        }
        op->dir = AES_DIR_DECRYPT;
        ret     = sstar_aes_crypt(op);
        if (ret < 0)
        {
            printk("[AESDMA][%s] sstar_aes_crypt return err:%d!\n", __FUNCTION__, ret);
            err = ret;
            goto finish;
        }

        nbytes -= ret;
        memcpy(walk.iv, ivTemp, 16);
        err = skcipher_walk_done(&walk, nbytes);
        if (err != 0)
        {
            printk("[AESDMA][%s][%d] skcipher_walk_done err:%d!\n", __FUNCTION__, __LINE__, err);
            goto finish;
        }
    }
finish:

    spin_unlock(&crypto_lock);
    return err;
}

static int sstar_cbc_encrypt(struct skcipher_request *req)
{
    int                     err, ret;
    unsigned int            nbytes;
    struct skcipher_walk    walk;
    struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
    struct sstar_aes_op *   op       = crypto_tfm_ctx(&skcipher->base);

    AESDMA_DBG("%s name %s\n", __FUNCTION__, req->base.tfm->__crt_alg->cra_name);

    if (!gbSupportAES256 && op->keylen != AES_KEYSIZE_128)
    {
        // return fallback_blk_dec(desc, dst, src, nbytes);
        return 0;
    }

    memset(&walk, 0, sizeof(walk));
    spin_lock(&crypto_lock);

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = walk.nbytes))
    {
        op->iv = walk.iv;
        //      AESDMA_DBG(" 1%s %d\n",__FUNCTION__,nbytes);
        op->src = walk.src.virt.addr, op->dst = walk.dst.virt.addr;
        op->mode = AES_MODE_CBC;
        if (nbytes < AES_BLOCK_SIZE)
        {
            if (walk.total < AES_BLOCK_SIZE)
            {
                // last block which is not AES_BLOCK_SIZE aligned
                op->len = nbytes;
            }
            else
            {
                // should not go here, because CBC chunksize=AES_BLOCK_SIZE
                printk("[AESDMA][%s][%d] err: nbytes < AES_BLOCK_SIZE && walk.total >= AES_BLOCK_SIZE\n", __FUNCTION__,
                       __LINE__);
                goto finish;
            }
        }
        else
        {
            op->len = (nbytes - nbytes % AES_BLOCK_SIZE);
        }
        op->dir = AES_DIR_ENCRYPT;
        ret     = sstar_aes_crypt(op);
        if (ret < 0)
        {
            printk("[AESDMA][%s] sstar_aes_crypt return err:%d!\n", __FUNCTION__, ret);
            err = ret;
            goto finish;
        }

        nbytes -= ret;
        if (walk.nbytes > 0)
            memcpy(walk.iv, (op->dst + (op->len) - 16), 16);

        err = skcipher_walk_done(&walk, nbytes);
        if (err != 0)
        {
            printk("[AESDMA][%s][%d] skcipher_walk_done err:%d!\n", __FUNCTION__, __LINE__, err);
            goto finish;
        }
    }

finish:
    spin_unlock(&crypto_lock);
    return err;
}

static int sstar_ecb_decrypt(struct skcipher_request *req)
{
    int                     err, ret;
    unsigned int            nbytes;
    struct skcipher_walk    walk;
    struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
    struct sstar_aes_op *   op       = crypto_tfm_ctx(&skcipher->base);

    AESDMA_DBG("%s name %s\n", __FUNCTION__, req->base.tfm->__crt_alg->cra_name);

    if (!gbSupportAES256 && op->keylen != AES_KEYSIZE_128)
    {
        // return fallback_blk_dec(desc, dst, src, nbytes);
        return 0;
    }

    memset(&walk, 0, sizeof(walk));
    spin_lock(&crypto_lock);

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = walk.nbytes))
    {
        op->src = walk.src.virt.addr, op->dst = walk.dst.virt.addr;
        op->mode = AES_MODE_ECB;
        if (nbytes < AES_BLOCK_SIZE)
        {
            // should not go here, because ECB cra_blocksize=AES_BLOCK_SIZE
            printk("[AESDMA][%s][%d] err: nbytes < AES_BLOCK_SIZE\n", __FUNCTION__, __LINE__);
            goto finish;
        }
        op->len = nbytes - (nbytes % AES_BLOCK_SIZE);
        op->dir = AES_DIR_DECRYPT;
        ret     = sstar_aes_crypt(op);
        if (ret < 0)
        {
            printk("[AESDMA][%s] sstar_aes_crypt return err:%d!\n", __FUNCTION__, ret);
            err = ret;
            goto finish;
        }

        nbytes -= ret;
        err = skcipher_walk_done(&walk, nbytes);
        if (err != 0)
        {
            printk("[AESDMA][%s][%d] skcipher_walk_done err:%d!\n", __FUNCTION__, __LINE__, err);
            goto finish;
        }
    }
finish:

    spin_unlock(&crypto_lock);
    return err;
}
static int sstar_ecb_encrypt(struct skcipher_request *req)
{
    int                     err, ret;
    unsigned int            nbytes;
    struct skcipher_walk    walk;
    struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
    struct sstar_aes_op *   op       = crypto_tfm_ctx(&skcipher->base);

    AESDMA_DBG("%s name %s\n", __FUNCTION__, req->base.tfm->__crt_alg->cra_name);

    if (!gbSupportAES256 && op->keylen != AES_KEYSIZE_128)
    {
        // return fallback_blk_dec(desc, dst, src, nbytes);
        return 0;
    }

    memset(&walk, 0, sizeof(walk));
    spin_lock(&crypto_lock);

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = walk.nbytes))
    {
        op->src = walk.src.virt.addr, op->dst = walk.dst.virt.addr;
        op->mode = AES_MODE_ECB;
        if (nbytes < AES_BLOCK_SIZE)
        {
            // should not go here, because ECB cra_blocksize=AES_BLOCK_SIZE
            printk("[AESDMA][%s][%d] err: nbytes < AES_BLOCK_SIZE\n", __FUNCTION__, __LINE__);
            goto finish;
        }
        op->len = nbytes - (nbytes % AES_BLOCK_SIZE);
        op->dir = AES_DIR_ENCRYPT;
        ret     = sstar_aes_crypt(op);
        if (ret < 0)
        {
            printk("[AESDMA][%s] sstar_aes_crypt return err:%d!\n", __FUNCTION__, ret);
            err = ret;
            goto finish;
        }

        nbytes -= ret;
        err = skcipher_walk_done(&walk, nbytes);
        if (err != 0)
        {
            printk("[AESDMA][%s][%d] skcipher_walk_done err:%d!\n", __FUNCTION__, __LINE__, err);
            goto finish;
        }
    }

finish:

    spin_unlock(&crypto_lock);
    return err;
}

/*ctr decrypt=encrypt*/
static int sstar_ctr_encrypt(struct skcipher_request *req)
{
    int                     err, ret;
    unsigned int            nbytes;
    int                     counter = 0, n = 0;
    struct skcipher_walk    walk;
    struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
    struct sstar_aes_op *   op       = crypto_tfm_ctx(&skcipher->base);

    AESDMA_DBG("%s name %s\n", __FUNCTION__, req->base.tfm->__crt_alg->cra_name);

    if (!gbSupportAES256 && op->keylen != AES_KEYSIZE_128)
    {
        // return fallback_blk_dec(desc, dst, src, nbytes);
        return 0;
    }
    memset(&walk, 0, sizeof(walk));
    spin_lock(&crypto_lock);

    err = skcipher_walk_virt(&walk, req, false);

    while (walk.nbytes >= AES_BLOCK_SIZE)
    {
        u8 *pdata;
        op->iv  = walk.iv;
        op->src = walk.src.virt.addr, op->dst = walk.dst.virt.addr;
        op->mode = AES_MODE_CTR;
        op->len  = walk.nbytes - (walk.nbytes % AES_BLOCK_SIZE);
        op->dir  = AES_DIR_ENCRYPT;

        pdata = (u8 *)walk.src.virt.addr;

        ret = sstar_aes_crypt(op);
        if (ret < 0)
        {
            err = ret;
            goto finish;
        }
        nbytes = walk.nbytes - ret; // the remain data
        if (err != 0)
        {
            printk("[AESDMA][%s][%d] skcipher_walk_done err:%d!\n", __FUNCTION__, __LINE__, err);
            goto finish;
        }

        counter = op->len >> 4;
        if (walk.nbytes > 0)
        {
            for (n = 0; n < counter; n++)
            {
                crypto_inc((u8 *)op->iv, AES_BLOCK_SIZE);
            }
        }
        err = skcipher_walk_done(&walk, nbytes);
    }

    if (walk.nbytes)
    {
        if (walk.total >= AES_BLOCK_SIZE)
        {
            // should not go here, because CTR chunksize=AES_BLOCK_SIZE
            printk("[AESDMA][%s][%d] err: nbytes < AES_BLOCK_SIZE && walk.total >= AES_BLOCK_SIZE\n", __FUNCTION__,
                   __LINE__);
            goto finish;
        }
        op->iv  = walk.iv;
        op->src = walk.src.virt.addr, op->dst = walk.dst.virt.addr;
        op->mode = AES_MODE_CTR;
        op->len  = walk.nbytes;
        op->dir  = AES_DIR_ENCRYPT;
        ret      = sstar_aes_crypt(op);
        if (ret < 0)
        {
            err = ret;
            goto finish;
        }
        nbytes = walk.nbytes - ret;
        if (err != 0)
        {
            printk("[AESDMA][%s][%d] skcipher_walk_done err:%d!\n", __FUNCTION__, __LINE__, err);
            goto finish;
        }
        crypto_inc((u8 *)op->iv, AES_BLOCK_SIZE);
        err = skcipher_walk_done(&walk, nbytes);
    }

finish:

    spin_unlock(&crypto_lock);
    return err;
}

u32 sstar_random(void)
{
    u32 data = 0;
    enableRngClock();
    data = HAL_RNG_Read();
    disableRngClock();
    return data;
}

static struct crypto_alg sstar_alg = {.cra_name        = "aes",
                                      .cra_driver_name = "sstar-aes",
                                      .cra_priority    = 300,
                                      .cra_alignmask   = 0,
                                      .cra_flags       = CRYPTO_ALG_TYPE_CIPHER | CRYPTO_ALG_NEED_FALLBACK
                                                   | CRYPTO_ALG_KERN_DRIVER_ONLY,
                                      .cra_init      = fallback_init_cip,
                                      .cra_exit      = fallback_exit_cip,
                                      .cra_blocksize = AES_BLOCK_SIZE,
                                      .cra_ctxsize   = sizeof(struct sstar_aes_op),
                                      .cra_module    = THIS_MODULE,
                                      .cra_u         = {.cipher = {.cia_min_keysize = AES_MIN_KEY_SIZE,
                                                           .cia_max_keysize = AES_MAX_KEY_SIZE,
                                                           .cia_setkey      = sstar_setkey_cip,
                                                           .cia_encrypt     = sstar_encrypt,
                                                           .cia_decrypt     = sstar_decrypt}}};

static struct skcipher_alg sstar_cbc_alg = {
    .base.cra_name        = "cbc(aes)",
    .base.cra_driver_name = "cbc-aes-sstar",
    .base.cra_priority    = 400,
    .base.cra_blocksize   = 1,
    .base.cra_ctxsize     = sizeof(struct sstar_aes_op),
    .base.cra_module      = THIS_MODULE,
    .base.cra_alignmask   = 0,
    .base.cra_flags       = CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_NEED_FALLBACK | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_init        = fallback_init_cip,
    .base.cra_exit        = fallback_exit_cip,
    .base.cra_u           = {.cipher = {.cia_min_keysize = AES_MIN_KEY_SIZE,
                              .cia_max_keysize = AES_MAX_KEY_SIZE,
                              .cia_setkey      = sstar_setkey_cip,
                              .cia_encrypt     = sstar_encrypt,
                              .cia_decrypt     = sstar_decrypt}},
    .min_keysize          = AES_MIN_KEY_SIZE,
    .max_keysize          = AES_MAX_KEY_SIZE,
    .ivsize               = AES_BLOCK_SIZE,
    .setkey               = sstar_setkey_blk,
    .encrypt              = sstar_cbc_encrypt,
    .decrypt              = sstar_cbc_decrypt,
    .chunksize            = AES_BLOCK_SIZE,
};

static struct skcipher_alg sstar_ecb_alg = {
    .base.cra_name        = "ecb(aes)",
    .base.cra_driver_name = "ecb-aes-sstar",
    .base.cra_priority    = 400,
    .base.cra_blocksize   = AES_BLOCK_SIZE,
    .base.cra_ctxsize     = sizeof(struct sstar_aes_op),
    .base.cra_module      = THIS_MODULE,
    .base.cra_alignmask   = 0,
    .base.cra_flags       = CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_NEED_FALLBACK | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_init        = fallback_init_cip,
    .base.cra_exit        = fallback_exit_cip,
    .base.cra_u           = {.cipher = {.cia_min_keysize = AES_MIN_KEY_SIZE,
                              .cia_max_keysize = AES_MAX_KEY_SIZE,
                              .cia_setkey      = sstar_setkey_cip,
                              .cia_encrypt     = sstar_encrypt,
                              .cia_decrypt     = sstar_decrypt}},
    .min_keysize          = AES_MIN_KEY_SIZE,
    .max_keysize          = AES_MAX_KEY_SIZE,
    .ivsize               = AES_BLOCK_SIZE,
    .setkey               = sstar_setkey_blk,
    .encrypt              = sstar_ecb_encrypt,
    .decrypt              = sstar_ecb_decrypt,
    .chunksize            = AES_BLOCK_SIZE,
};

static struct skcipher_alg sstar_ctr_alg = {
    .base.cra_name        = "ctr(aes)",
    .base.cra_driver_name = "ctr-aes-sstar",
    .base.cra_priority    = 400,
    .base.cra_blocksize   = 1,
    .base.cra_ctxsize     = sizeof(struct sstar_aes_op),
    .base.cra_module      = THIS_MODULE,
    .base.cra_alignmask   = 0,
    .base.cra_flags       = CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_NEED_FALLBACK | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_init        = fallback_init_cip,
    .base.cra_exit        = fallback_exit_cip,
    .base.cra_u           = {.cipher = {.cia_min_keysize = AES_MIN_KEY_SIZE,
                              .cia_max_keysize = AES_MAX_KEY_SIZE,
                              .cia_setkey      = sstar_setkey_cip,
                              .cia_encrypt     = sstar_encrypt,
                              .cia_decrypt     = sstar_decrypt}},
    .min_keysize          = AES_MIN_KEY_SIZE,
    .max_keysize          = AES_MAX_KEY_SIZE,
    .ivsize               = AES_BLOCK_SIZE,
    .setkey               = sstar_setkey_blk,
    .encrypt              = sstar_ctr_encrypt,
    .decrypt              = sstar_ctr_encrypt,
    .chunksize            = AES_BLOCK_SIZE,
};

#ifdef CONFIG_PM_SLEEP
static int sstar_aes_resume(struct platform_device *pdev)
{
    return 0;
}

static int sstar_aes_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}
#endif

static int sstar_aes_remove(struct platform_device *pdev)
{
    crypto_unregister_alg(&sstar_alg);
    crypto_unregister_skcipher(&sstar_ecb_alg);
    crypto_unregister_skcipher(&sstar_cbc_alg);
    crypto_unregister_skcipher(&sstar_ctr_alg);
    sstar_sha_destroy();
    misc_deregister(&rsadev);
    ss_aes_mem_free();
    return 0;
}

static int sstar_aes_probe(struct platform_device *pdev)
{
    int ret = 0;
#ifdef AESDMA_ISR
    int irq;
#endif

    if (!(ALLOC_DMEM.aesdma_vir_addr =
              alloc_dmem(ALLOC_DMEM.DMEM_AES_ENG_INPUT, CRYPT_BUF_SIZE, &ALLOC_DMEM.aesdma_phy_addr)))
    {
        printk("Allocate aesdma InputBuf fail\n");
        goto initial_fail;
    }
    else
    {
        memset(ALLOC_DMEM.aesdma_vir_addr, 0, CRYPT_BUF_SIZE);
    }
    psg_mdrv_aesdma = pdev;

#if 0
    ret = crypto_register_alg(&sstar_des_alg);
    if (ret)
        goto eiomap;
    ret = crypto_register_alg(&sstar_tdes_alg);
    if (ret)
        goto eiomap;
#endif
    /*
    #if defined(CONFIG_SSTAR_RNG)
        {
            extern int sstar_rng_probe(struct platform_device * pdev);
            sstar_rng_probe(pdev);
        }
    #endif
    */

    ret = crypto_register_alg(&sstar_alg);
    if (ret)
        goto eiomap;

    ret = crypto_register_skcipher(&sstar_ecb_alg);
    if (ret)
        goto eecb;

    ret = crypto_register_skcipher(&sstar_cbc_alg);
    if (ret)
        goto ecbc;

    ret = crypto_register_skcipher(&sstar_ctr_alg);
    if (ret)
        goto ectr;

#if 0
    ret = crypto_register_alg(&sstar_des_ecb_alg);
    if (ret)
        goto edesecb;

    ret = crypto_register_alg(&sstar_des_cbc_alg);
    if (ret)
        goto edescbc;

    ret = crypto_register_alg(&sstar_des_ctr_alg);
    if (ret)
        goto edesctr;

    ret = crypto_register_alg(&sstar_tdes_ecb_alg);
    if (ret)
        goto etdesecb;

    ret = crypto_register_alg(&sstar_tdes_cbc_alg);
    if (ret)
        goto etdescbc;

    ret = crypto_register_alg(&sstar_tdes_ctr_alg);
    if (ret)
        goto etdesctr;
#endif

    sstar_sha_create();     // inital SHA part
    misc_register(&rsadev); // inital RSA part
    dev_dbg(&pdev->dev, "SSTAR AES engine enabled.\n");
#ifdef AESDMA_ISR
    irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (request_irq(irq, aes_dma_interrupt, 0, "aes interrupt", NULL) == 0)
    {
        init_completion(&_mdmadone);
        AESDMA_DBG("sstar AES interrupt registered\n");
        _isr_requested = 1;
    }
    else
    {
        pr_err("sstar AES interrupt failed\n");
        _isr_requested = 0;
    }
#endif
    /*
    #if defined(CONFIG_SSTAR_RNG)
        {
            extern int sstar_rng_probe(struct platform_device * pdev);
            sstar_rng_probe(pdev);
        }
    #endif
    */
    return 0;

ectr:
    crypto_unregister_skcipher(&sstar_ctr_alg);
ecbc:
    crypto_unregister_skcipher(&sstar_cbc_alg);
eecb:
    crypto_unregister_skcipher(&sstar_ecb_alg);

#if 0
 edesecb:
    crypto_unregister_alg(&sstar_des_ecb_alg);
 edescbc:
    crypto_unregister_alg(&sstar_des_cbc_alg);
 edesctr:
    crypto_unregister_alg(&sstar_des_ctr_alg);
 etdesecb:
    printk("!!!sstar_tdes_ecb_alg initialization failed.\n");
    crypto_unregister_alg(&sstar_tdes_ecb_alg);
 etdescbc:
    printk("!!!sstar_tdes_cbc_alg initialization failed.\n");
    crypto_unregister_alg(&sstar_tdes_cbc_alg);
 etdesctr:
    printk("!!!sstar_tdes_ctr_alg initialization failed.\n");
    crypto_unregister_alg(&sstar_tdes_ctr_alg);
#endif
eiomap:
    crypto_unregister_alg(&sstar_alg);
initial_fail:
    pr_err("SSTAR AES initialization failed.\n");
    return ret;
}

static const struct of_device_id sstar_aes_dt_ids[] = {{.compatible = "sstar,aesdma"}, {}};
MODULE_DEVICE_TABLE(of, sstar_aes_dt_ids);

static struct platform_driver sstar_aes_driver = {
    .probe  = sstar_aes_probe,
    .remove = sstar_aes_remove,
#ifdef CONFIG_PM_SLEEP
    .suspend = sstar_aes_suspend,
    .resume  = sstar_aes_resume,
#endif
    .driver =
        {
            .name           = "sstar_aesdma",
            .owner          = THIS_MODULE,
            .of_match_table = of_match_ptr(sstar_aes_dt_ids),
        },
};

module_platform_driver(sstar_aes_driver);

MODULE_DESCRIPTION("sstar AES hw acceleration support.");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
