/*
 * drvAESDMA.h- Sigmastar
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

#ifndef _DRV_AESDMA_H_
#define _DRV_AESDMA_H_

#include "sstar_types.h"

#ifndef BOOL
#define BOOL unsigned int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define AES_KEYSIZE_128 16
#define AES_KEYSIZE_192 24
#define AES_KEYSIZE_256 32

typedef enum
{
    E_AESDMA_KEY_CIPHER,
    E_AESDMA_KEY_OTP_EFUSE_KEY1,
    E_AESDMA_KEY_OTP_EFUSE_KEY2,
    E_AESDMA_KEY_OTP_EFUSE_KEY3,
    E_AESDMA_KEY_OTP_EFUSE_KEY4,
    E_AESDMA_KEY_OTP_EFUSE_KEY5,
    E_AESDMA_KEY_OTP_EFUSE_KEY6,
    E_AESDMA_KEY_OTP_EFUSE_KEY7,
    E_AESDMA_KEY_OTP_EFUSE_KEY8,
    E_AESDMA_KEY_OTP_EFUSE_KEY256_1,
    E_AESDMA_KEY_OTP_EFUSE_KEY256_2,
    E_AESDMA_KEY_OTP_EFUSE_KEY256_3,
    E_AESDMA_KEY_OTP_EFUSE_KEY256_4,
    E_AESDMA_KEY_NUM
} enumAESDMA_KeyType;

typedef enum
{
    E_AESDMA_CHAINMODE_ECB,
    E_AESDMA_CHAINMODE_CTR,
    E_AESDMA_CHAINMODE_CBC,
    E_AESDMA_CHAINMODE_NUM
} enumAESDMA_ChainMode;

typedef struct
{
    MS_U64               u64SrcAddr;
    U32                  u32Size;
    MS_U64               u64DstAddr;
    enumAESDMA_KeyType   eKeyType;
    U16 *                pu16Key;
    BOOL                 bSetIV;
    BOOL                 bDecrypt;
    U16 *                pu16IV;
    enumAESDMA_ChainMode eChainMode;
    U32                  keylen; // aes128->16,aes256->32
} __attribute__((aligned(16))) aesdmaConfig;

typedef enum
{
    E_SHA_MODE_1,
    E_SHA_MODE_256,
    E_SHA_MODE_NUM
} enumShaMode;

typedef struct
{
    U32 *pu32Sig;
    U32 *pu32KeyN;
    U32 *pu32KeyE;
    U32 *pu32Output;
    BOOL bHwKey;
    BOOL bPublicKey;
    U32  u32KeyLen;
    U32  u32SigLen;
    U32  u32OutputLen;
} __attribute__((aligned(16))) rsaConfig;

void MDrv_AESDMA_Run(aesdmaConfig *pConfig);
void MDrv_SHA_Run(MS_U64 u64SrcAddr, U32 u32Size, enumShaMode eMode, U16 *pu16Output);
void MDrv_RSA_Run(rsaConfig *pConfig);
void runDecrypt(MS_U64 u64ImageAddr, U32 u32ImageSize, U32 u32KeySel, U16 *pu16Key, U32 keylen);
BOOL runAuthenticate(MS_U64 u64ImageAddr, U32 u32ImageSize, U32 *pu32Key);
BOOL runAuthenticate2(MS_U64 u64ImageAddr, U32 u32ImageSize, MS_U64 u32Key, U32 u32KeySize, U32 u32Sig, U32 u32SigSize);
U16  MDrv_RNG_Read(void);

#endif
