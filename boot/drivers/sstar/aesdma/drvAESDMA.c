/*
 * drvAESDMA.c- Sigmastar
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

#include "drvAESDMA.h"
#include <common.h>
#include "halAESDMA.h"
#include <cpu_func.h>

extern void invalidate_dcache_range(unsigned long start, unsigned long stop);
extern void Chip_Flush_Memory(void);
void        Reverse_Data(void* addr, int size)
{
    int   low, high;
    char* pdata = (char*)addr;
    char  temp;
    for (low = 0, high = size - 1; low < high; low++, high--)
    {
        temp        = pdata[low];
        pdata[low]  = pdata[high];
        pdata[high] = temp;
    }
}

static U32 _Hal_GetMsTime(U32 tPreTime, U32 u32Fac)
{
    U32 u32CurrTime = 0;
    u32CurrTime     = get_timer(0);

    if ((u32CurrTime - tPreTime) > u32Fac)
        return TRUE;
    return FALSE;
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void MDrv_AESDMA_Run(aesdmaConfig* pConfig)
#else
void MDrv_AESDMA_Run(aesdmaConfig* pConfig)
#endif
{
    U32 time1;

    HAL_AESDMA_Reset();

    if (!pConfig->u32Size)
    {
        printf("[%s#%d] ERROR!Size is zero!\n", __FUNCTION__, __LINE__);
        return;
    }
    if ((unsigned long)pConfig->pu16Key & 1)
    {
        printf("[%s#%d] ERROR!pu16Key(%p) have to align to half-word!\n", __FUNCTION__, __LINE__, pConfig->pu16Key);
        return;
    }
    Chip_Flush_Memory();
    flush_dcache_range(pConfig->u64SrcAddr & (~(64 - 1)),
                       ((pConfig->u64SrcAddr + pConfig->u32Size + 0x3f) & ~(0x00000040 - 1)) & (~(64 - 1)));

    HAL_AESDMA_SetFileinAddr(pConfig->u64SrcAddr);
    HAL_AESDMA_SetXIULength(pConfig->u32Size);
    HAL_AESDMA_SetFileoutAddr(pConfig->u64DstAddr, pConfig->u32Size);

#ifdef AESDMA_MULTIPLE_ROUND
    HAL_AESDMA_MultiRound(AESDMA_MULTIPLE_ROUND);
#endif
#ifdef AESDMA_ENABLE_RNG_INIT
    HAL_AESDMA_SCA(TRUE);
#endif

    if (HAL_AESDMA_CheckAesKey(pConfig->eKeyType) == FALSE)
    {
        printf("[%s#%d] ERROR!eKeyType(%d)\n", __FUNCTION__, __LINE__, pConfig->eKeyType);
        return;
    }
    if (pConfig->keylen == 0)
    {
        pConfig->keylen = AES_KEYSIZE_128;
        printf("[WARNING]AES,if keylen=0,it defautl use AES128.\r\n");
    }
    if (pConfig->keylen == AES_KEYSIZE_128 || pConfig->keylen == AES_KEYSIZE_192 || pConfig->keylen == AES_KEYSIZE_256)
    {
        HAL_AESDMA_SetKeylen(pConfig->keylen);
    }
    else
    {
        printf(
            "\033[0m"
            "[ERROR]AES,do not support AES_KEYSIZE_%d!\r\n"
            "\033[0m",
            8 * pConfig->keylen);
        return;
    }

    if (pConfig->eKeyType == E_AESDMA_KEY_CIPHER)
    {
        HAL_AESDMA_SetCipherKey(pConfig->pu16Key, pConfig->keylen);
    }
    else // OTP KEY
    {
        if (pConfig->keylen == AES_KEYSIZE_128)
        {
            if ((pConfig->eKeyType) > E_AESDMA_KEY_OTP_EFUSE_KEY8)
            {
                printf(
                    "\033[31m"
                    "[ERROR]AES,aeskey128 range must from[1,8]\r\n"
                    "\033[0m");
                return;
            }
            HAL_AESDMA_UseAesKey(pConfig->eKeyType);
        }
        else if (pConfig->keylen == AES_KEYSIZE_256)
        {
            if (pConfig->eKeyType < E_AESDMA_KEY_OTP_EFUSE_KEY256_1
                || pConfig->eKeyType > E_AESDMA_KEY_OTP_EFUSE_KEY256_4)
            {
                printf(
                    "\033[31m"
                    "[ERROR]AES,aeskey256 range must from[1,4]\r\n"
                    "\033[0m");
                return;
            }
            HAL_AESDMA_UseAesKey(pConfig->eKeyType); // key256_1->1,key256_2->3,key256_3->5,key256_4->7
        }
        else
        {
            printf(
                "\033[0m"
                "[ERROR]AES,do not support AES_KEYSIZE_%d!\r\n"
                "\033[0m",
                8 * pConfig->keylen);
            return;
        }
    }

    if (pConfig->bDecrypt)
    {
        HAL_AESDMA_CipherDecrypt();
    }
    else
    {
        HAL_AESDMA_CipherEncrypt();
    }

    if (pConfig->bSetIV)
    {
        HAL_AESDMA_SetIV(pConfig->pu16IV);
    }

    HAL_AESDMA_Enable();

    switch (pConfig->eChainMode)
    {
        case E_AESDMA_CHAINMODE_ECB:
            HAL_AESDMA_SetChainModeECB();
            HAL_AESDMA_SetXIULength(((pConfig->u32Size + 15) / 16) * 16); // ECB mode size should align 16byte
            break;
        case E_AESDMA_CHAINMODE_CTR:
            HAL_AESDMA_SetChainModeCTR();
            HAL_AESDMA_CipherEncrypt(); // CTR mode can't set cipher_decrypt bit
            break;
        case E_AESDMA_CHAINMODE_CBC:
            HAL_AESDMA_SetChainModeCBC();
            break;
        default:
            return;
    }

    HAL_AESDMA_FileOutEnable(1);

    invalidate_dcache_range(pConfig->u64DstAddr & (~(64 - 1)),
                            ((pConfig->u64DstAddr + pConfig->u32Size + 0x3f) & ~(0x00000040 - 1)) & (~(64 - 1)));

    HAL_AESDMA_Start(1);

    time1 = get_timer(0);
    // Wait for ready.
    while ((HAL_AESDMA_GetStatus() & AESDMA_CTRL_DMA_DONE) != AESDMA_CTRL_DMA_DONE)
    {
        if (_Hal_GetMsTime(time1, 1000))
        {
            printf("[%s#%d] timeout!\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    HAL_AESDMA_Reset();
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void MDrv_SHA_Run(MS_U64 u64SrcAddr, U32 u32Size, enumShaMode eMode, U16* pu16Output)
#else
void MDrv_SHA_Run(MS_U64 u64SrcAddr, U32 u32Size, enumShaMode eMode, U16* pu16Output)
#endif
{
    U32 time1;

    if (u64SrcAddr & 0xf)
    {
        printf("[%s#%d] ERROR!Must align to 16-bytes!\n", __FUNCTION__, __LINE__);
        return;
    }
    if (((unsigned long)pu16Output) & 0xf)
    {
        printf("[%s#%d] ERROR!Must align to 16-bytes!\n", __FUNCTION__, __LINE__);
        return;
    }

    HAL_SHA_Reset();
    flush_dcache_range(u64SrcAddr & (~(64 - 1)), ((u64SrcAddr + u32Size + 0x3f) & ~(0x00000040 - 1)));
    Chip_Flush_Memory();

    HAL_SHA_SetAddress(u64SrcAddr);
    HAL_SHA_SetLength(u32Size);

    switch (eMode)
    {
        case E_SHA_MODE_1:
            HAL_SHA_SelMode(0);
            break;
        case E_SHA_MODE_256:
            HAL_SHA_SelMode(1);
            break;
        default:
            return;
    }

    HAL_SHA_Start();

    // Wait for the SHA done.
    time1 = get_timer(0);
    while ((HAL_SHA_GetStatus() & SHARNG_CTRL_SHA_READY) != SHARNG_CTRL_SHA_READY)
    {
        if (_Hal_GetMsTime(time1, 1000))
        {
            printf("[%s#%d] timeout!\n", __FUNCTION__, __LINE__);
            goto sha_timeout;
        }
    }

    HAL_SHA_Out((U32)(unsigned long)pu16Output);

    Reverse_Data(pu16Output, 32);
sha_timeout:
    HAL_SHA_Clear();
    HAL_SHA_Reset();
}

#ifdef CONFIG_SRAM_DUMMY_ACCESS_RSA
__attribute__((optimize("O0"))) void MDrv_RSA_Run(rsaConfig* pConfig)
#else
void MDrv_RSA_Run(rsaConfig* pConfig)
#endif
{
    int nOutSize;
    int i;
    U32 time1;
    int operation;

    if (pConfig->pu32Sig && (U32)(unsigned long)pConfig->pu32Sig % 4)
    {
        printf("[%s#%d] ERROR!Must align to 4-bytes!\n", __FUNCTION__, __LINE__);
        return;
    }
    if (pConfig->pu32KeyN && (U32)(unsigned long)pConfig->pu32KeyN % 4)
    {
        printf("[%s#%d] ERROR!Must align to 4-bytes!\n", __FUNCTION__, __LINE__);
        return;
    }
    if (pConfig->pu32KeyE && (U32)(unsigned long)pConfig->pu32KeyE % 4)
    {
        printf("[%s#%d] ERROR!Must align to 4-bytes!\n", __FUNCTION__, __LINE__);
        return;
    }
    if (pConfig->pu32Output && (U32)(unsigned long)pConfig->pu32Output % 4)
    {
        printf("[%s#%d] ERROR!Must align to 4-bytes!\n", __FUNCTION__, __LINE__);
        return;
    }

#ifndef PKA_SUPPORT_RSA

    // HAL_RSA_Reset();
    HAL_RSA_SetKeyLength((pConfig->u32SigLen / 4 - 1) & 0x7F);
    HAL_RSA_SetKeyType(pConfig->bHwKey, pConfig->bPublicKey);

    HAL_RSA_Ind32Ctrl(1);
    HAL_RSA_LoadSignInverse_2byte(pConfig->pu32Sig, pConfig->u32SigLen / 4);

    if (!pConfig->bHwKey)
    {
        if (pConfig->pu32KeyN)
        {
            HAL_RSA_LoadKeyNInverse(pConfig->pu32KeyN, pConfig->u32SigLen / 4);
        }

        if (pConfig->pu32KeyE)
        {
            HAL_RSA_LoadKeyEInverse(pConfig->pu32KeyE);
        }
        else
        {
            HAL_RSA_LoadKeyE(); // use 65537
        }
    }

    HAL_RSA_ExponetialStart();
    time1 = get_timer(0);
    while ((HAL_RSA_GetStatus() & RSA_STATUS_RSA_DONE) != RSA_STATUS_RSA_DONE)
    {
        if (_Hal_GetMsTime(time1, 3000))
        {
            printf("[%s#%d] timeout!\n", __FUNCTION__, __LINE__);
            goto rsa_timeout;
        }
    }

    HAL_RSA_Ind32Ctrl(0);
    if (pConfig->u32OutputLen)
        nOutSize = pConfig->u32OutputLen / 4;
    else
        nOutSize = 256 / 4;

    for (i = 0; i < nOutSize; i++)
    {
        HAL_RSA_SetFileOutAddr(i);
        HAL_RSA_FileOutStart();
        //*(pConfig->pu32Output+i) = HAL_RSA_FileOut();
        *(pConfig->pu32Output + (nOutSize - 1) - i) = HAL_RSA_FileOut(); // big-endian
    }
rsa_timeout:
    HAL_RSA_FileOutEnd();
    HAL_RSA_Reset();

#else
    pConfig->u32KeyLen /= 4;
    pConfig->u32SigLen /= 4;

    if (pConfig->u32KeyLen == 0x80)
    {
        operation = PKA_RSA_OPERAND_4096; // Start CLUE operation for 4096-bits
    }
    else if (pConfig->u32KeyLen == 0x40)
    {
        operation = PKA_RSA_OPERAND_2048; // Start CLUE operation for 2048-bits
    }
    else if (pConfig->u32KeyLen == 0x20)
    {
        operation = PKA_RSA_OPERAND_1024; // Start CLUE operation for 1024-bits
    }
    else if (pConfig->u32KeyLen == 0x10)
    {
        operation = PKA_RSA_OPERAND_512; // Start CLUE operation for 512-bits
    }
    else
    {
        printf("[%s][%d]PKA_RSA_ERROR \n", __FUNCTION__, __LINE__);
        operation = PKA_RSA_OPERAND_NULL; // Start CLUE operation for 512-bits
        return;
    }

    HAL_RSA_LoadKeyNInverse(pConfig->pu32KeyN, (u8)pConfig->u32KeyLen);
    HAL_RSA_Init_Status(ELP_CLUE_ENTRY_CALC_R_INV, operation);
    HAL_RSA_WaitDone(FALSE);

    HAL_RSA_Init_Status(ELP_CLUE_ENTRY_CALC_MP, operation);
    HAL_RSA_WaitDone(TRUE);

    HAL_RSA_Init_Status(ELP_CLUE_ENTRY_CALC_R_SQR, operation);
    HAL_RSA_WaitDone(FALSE);

    if (pConfig->pu32KeyE)
    {
        HAL_RSA_LoadKeyEInverse(pConfig->pu32KeyE, (u8)pConfig->u32KeyLen, operation);
    }
    else
    {
        HAL_RSA_LoadKeyE((u8)pConfig->u32KeyLen, operation); // use 65537
    }
    HAL_RSA_LoadSignInverse_start();
    HAL_RSA_LoadSignInverse((pConfig->pu32Sig), (pConfig->u32SigLen), 0);

    nOutSize = pConfig->u32KeyLen;
    HAL_RSA_Init_Status(ELP_CLUE_ENTRY_MODEXP, operation);
    HAL_RSA_WaitDone(FALSE);

    HAL_RSA_FileOut_Start();
    for (i = 0; i < nOutSize; i++)
    {
        *(pConfig->pu32Output + (nOutSize - 1) - i) = HAL_RSA_FileOut(i); // big-endian
    }
    HAL_RSA_Reset((u8)pConfig->u32KeyLen, operation);
#endif
    time1     = time1;
    operation = operation;
}

void runDecrypt(MS_U64 u64ImageAddr, U32 u32ImageSize, U32 u32KeySel, U16* pu16Key, U32 keylen)
{
    aesdmaConfig config = {0};

    config.u64SrcAddr = u64ImageAddr;
    config.u64DstAddr = u64ImageAddr;
    config.u32Size    = u32ImageSize;

    if (u32KeySel == E_AESDMA_KEY_CIPHER)
    {
        config.pu16Key = pu16Key;
    }

    config.eKeyType   = u32KeySel;
    config.bDecrypt   = 1;
    config.eChainMode = E_AESDMA_CHAINMODE_ECB; // default use ECB mode
    config.keylen     = keylen;                 // aes128->16;aes256->32
    MDrv_AESDMA_Run(&config);
}

int Compare_data(char* dst, char* src, int size)
{
    int i;
    for (i = 0; i < size && *(dst + i) == *(src + i); i++)
        ;

    if (i == size)
        return 0;

    return 1;
}

void Dump_data(void* addr, int size)
{
    char* out_buf = (char*)addr;
    int   i;
    for (i = 0; i < size; i++)
    {
        if (i % 16 == 0)
            printf("%p: ", &out_buf[i]);

        printf("%02x ", out_buf[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n");
}

BOOL runAuthenticate2(MS_U64 u64ImageAddr, U32 u32ImageSize, MS_U64 u32Key, U32 u32KeySize, U32 u32Sig, U32 u32SigSize)
{
    U32 rsa_out[(RSA_SIG_LEN / 4)];
    U32 __attribute__((aligned(16))) sha_out[8] = {0};
    rsaConfig config                            = {0};

    memset(&sha_out, 0xff, sizeof(sha_out));
    memset(&rsa_out, 0x80, sizeof(rsa_out));

    MDrv_SHA_Run(u64ImageAddr, u32ImageSize, E_SHA_MODE_256, (U16*)sha_out); // image + SHA-256
    // printf("runAuthenticate: --> MDrv_SHA_Run done\n");
    config.u32SigLen  = u32SigSize;
    config.bPublicKey = 1;
    config.pu32Sig    = (U32*)(unsigned long)(u32Sig);
    if (u32Key == 0)
    {
        config.bHwKey = 1;
    }
    else
    {
        config.pu32KeyN  = (U32*)(unsigned long)u32Key;
        config.u32KeyLen = u32KeySize;
    }
    config.pu32Output   = rsa_out;
    config.u32OutputLen = sizeof(sha_out);

    // printf("runAuthenticate: compare RSA address is config.pu32Sig=0x%08X --> MDrv_RSA_Run\n", config.pu32Sig);
    MDrv_RSA_Run(&config);
#ifdef PKA_SUPPORT_RSA
    if (Compare_data((char*)sha_out, (char*)&rsa_out[(RSA_SIG_LEN / 4) - (sizeof(sha_out) / 4)], sizeof(sha_out)))
#else
    if (Compare_data((char*)sha_out, (char*)rsa_out, sizeof(sha_out)))
#endif
    {
        printf("Digest check failed.\n");
        Dump_data(sha_out, sizeof(sha_out));
        Dump_data(rsa_out, sizeof(rsa_out));
        return FALSE;
    }

    return TRUE;
}

BOOL runAuthenticate(MS_U64 u32ImageAddr, U32 u32ImageSize, U32* pu32Key)
{
    return runAuthenticate2(u32ImageAddr, u32ImageSize, (unsigned long)pu32Key, RSA_SIG_LEN,
                            u32ImageAddr + u32ImageSize, RSA_SIG_LEN);
}

U16 MDrv_RNG_Read(void)
{
    return HAL_RNG_Read();
}
