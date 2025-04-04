/*
 * Copyright 2003-2004, Instant802 Networks, Inc.
 * Copyright 2006, Devicescape Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef AES_CCM_H
#define AES_CCM_H
#ifdef CONFIG_SSTAR_USE_SW_ENC
#include <linux/crypto.h>

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 60))
#include <crypto/algapi.h>
#include <crypto/internal/cipher.h>
#include <crypto/internal/skcipher.h>
#endif

struct crypto_cipher *ieee80211_aes_key_setup_encrypt(const u8 key[]);
void ieee80211_aes_ccm_encrypt(struct crypto_cipher *tfm, u8 *scratch, u8 *data,
			       size_t data_len, u8 *cdata, u8 *mic);
int ieee80211_aes_ccm_decrypt(struct crypto_cipher *tfm, u8 *scratch, u8 *cdata,
			      size_t data_len, u8 *mic, u8 *data);
void ieee80211_aes_key_free(struct crypto_cipher *tfm);
#endif
#endif /* AES_CCM_H */
