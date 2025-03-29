/*
 * aes.c- Sigmastar
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
#include <command.h>
//#include <environment.h>
#include <malloc.h>
#include "sstar_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include <../drivers/sstar/aesdma/drvAESDMA.h>
#include "image.h"
#include <asm/arch/mach/platform.h>

#define BUF_SIZE 4096

extern void Dump_data(void *addr, int size);
extern int  Compare_data(char *dst, char *src, int size);

char out_buf[128] = {0};

/***********************************************************/

char aes_ecb_plaintext[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
                            0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                            0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
char aes_ecb_key[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
char aes_ecb_result[]    = {0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7,
                         0x80, 0x70, 0xb4, 0xc5, 0x5a, 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b,
                         0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a};
char aes256_ecb_key[]    = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                         0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
                         0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
char aes256_ecb_result[] = {0x04, 0xa1, 0x21, 0xe9, 0x20, 0x33, 0xc9, 0x21, 0x04, 0x89, 0x17,
                            0x75, 0x4f, 0x96, 0x1b, 0x0d, 0x04, 0xa1, 0x21, 0xe9, 0x20, 0x33,
                            0xc9, 0x21, 0x04, 0x89, 0x17, 0x75, 0x4f, 0x96, 0x1b, 0x0d};

/***********************************************************/

char aes_cbc_plaintext[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                            0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                            0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
char aes_cbc_key[] = {0xc2, 0x86, 0x69, 0x6d, 0x88, 0x7c, 0x9a, 0xa0, 0x61, 0x1b, 0xbb, 0x3e, 0x20, 0x25, 0xa4, 0x5a};
char aes_cbc_iv[]  = {0x56, 0x2e, 0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58};
char aes_cbc_result[] = {0xd2, 0x96, 0xcd, 0x94, 0xc2, 0xcc, 0xcf, 0x8a, 0x3a, 0x86, 0x30,
                         0x28, 0xb5, 0xe1, 0xdc, 0x0a, 0x75, 0x86, 0x60, 0x2d, 0x25, 0x3c,
                         0xff, 0xf9, 0x1b, 0x82, 0x66, 0xbe, 0xa6, 0xd6, 0x1a, 0xb1};

char aes256_cbc_key[]    = {0xc2, 0x86, 0x69, 0x6d, 0x88, 0x7c, 0x9a, 0xa0, 0x61, 0x1b, 0xbb,
                         0x3e, 0x20, 0x25, 0xa4, 0x5a, 0xc2, 0x86, 0x69, 0x6d, 0x88, 0x7c,
                         0x9a, 0xa0, 0x61, 0x1b, 0xbb, 0x3e, 0x20, 0x25, 0xa4, 0x5a};
char aes256_cbc_result[] = {0x7d, 0xc4, 0x7b, 0x16, 0x9c, 0xd2, 0xfb, 0x23, 0x31, 0xd8, 0x26,
                            0x35, 0x57, 0x39, 0xf9, 0x01, 0x75, 0xa2, 0x9a, 0x90, 0x6e, 0xf0,
                            0x53, 0x37, 0x05, 0xab, 0x72, 0x65, 0x95, 0x5f, 0xbd, 0xa5};

/***********************************************************/

char aes_ctr_plaintext[] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73,
                            0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7,
                            0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51, 0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4,
                            0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f, 0x24, 0x45,
                            0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};
char aes_ctr_key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
char aes_ctr_iv[]  = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
char aes_ctr_result[] = {0x87, 0x4d, 0x61, 0x91, 0xb6, 0x20, 0xe3, 0x26, 0x1b, 0xef, 0x68, 0x64, 0x99,
                         0x0d, 0xb6, 0xce, 0x98, 0x06, 0xf6, 0x6b, 0x79, 0x70, 0xfd, 0xff, 0x86, 0x17,
                         0x18, 0x7b, 0xb9, 0xff, 0xfd, 0xff, 0x5a, 0xe4, 0xdf, 0x3e, 0xdb, 0xd5, 0xd3,
                         0x5e, 0x5b, 0x4f, 0x09, 0x02, 0x0d, 0xb0, 0x3e, 0xab, 0x1e, 0x03, 0x1d, 0xda,
                         0x2f, 0xbe, 0x03, 0xd1, 0x79, 0x21, 0x70, 0xa0, 0xf3, 0x00, 0x9c, 0xee};

char aes256_ctr_key[]    = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15,
                         0x88, 0x09, 0xcf, 0x4f, 0x3c, 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae,
                         0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
char aes256_ctr_result[] = {0x9e, 0xd0, 0xb8, 0xb9, 0xc9, 0xbe, 0x50, 0xe1, 0x75, 0xdc, 0x80, 0x62, 0x82,
                            0x43, 0x37, 0x72, 0x72, 0xb5, 0xa4, 0x36, 0x37, 0xb1, 0x8f, 0x26, 0x9b, 0x55,
                            0x7a, 0x63, 0x56, 0x76, 0x82, 0xc8, 0x11, 0x9f, 0xbf, 0xb3, 0x32, 0x2e, 0xcc,
                            0x41, 0x57, 0x03, 0x7c, 0x76, 0x52, 0x77, 0x2f, 0xf8, 0x46, 0x02, 0xfe, 0xaf,
                            0xf0, 0x0d, 0xa9, 0x10, 0x00, 0xe1, 0xc2, 0x11, 0xad, 0x01, 0x7c, 0x56};

/***********************************************************/

char __attribute__((aligned(16))) RSA_plaintext[] = {
    0x31, 0x5d, 0xfa, 0x52, 0xa4, 0x93, 0x52, 0xf8, 0xf5, 0xed, 0x39, 0xf4, 0xf8, 0x23, 0x4b, 0x30, 0x11, 0xa2, 0x2c,
    0x5b, 0xa9, 0x8c, 0xcf, 0xdf, 0x19, 0x66, 0xf5, 0xf5, 0x1a, 0x6d, 0xf6, 0x25, 0x89, 0xaf, 0x06, 0x13, 0xdc, 0xa4,
    0xd4, 0x0b, 0x3c, 0x1c, 0x4f, 0xb9, 0xd3, 0xd0, 0x63, 0x29, 0x2a, 0x5d, 0xfe, 0xb6, 0x99, 0x20, 0x58, 0x36, 0x2b,
    0x1d, 0x57, 0xf4, 0x71, 0x38, 0xa7, 0x8b, 0xad, 0x8c, 0xef, 0x1f, 0x2f, 0xea, 0x4c, 0x87, 0x2b, 0xd7, 0xb8, 0xc8,
    0xb8, 0x09, 0xcb, 0xb9, 0x05, 0xab, 0x43, 0x41, 0xd9, 0x75, 0x36, 0x4d, 0xb6, 0x8a, 0xd3, 0x45, 0x96, 0xfd, 0x9c,
    0xe8, 0x6e, 0xc8, 0x37, 0x5e, 0x4f, 0x63, 0xf4, 0x1c, 0x18, 0x2c, 0x38, 0x79, 0xe2, 0x5a, 0xe5, 0x1d, 0x48, 0xf6,
    0xb2, 0x79, 0x57, 0x12, 0xab, 0xae, 0xc1, 0xb1, 0x9d, 0x11, 0x4f, 0xa1, 0x4d, 0x1b, 0x4c, 0x8c, 0x3a, 0x2d, 0x7b,
    0x98, 0xb9, 0x89, 0x7b, 0x38, 0x84, 0x13, 0x8e, 0x3f, 0x3c, 0xe8, 0x59, 0x26, 0x90, 0x77, 0xe7, 0xca, 0x52, 0xbf,
    0x3a, 0x5e, 0xe2, 0x58, 0x54, 0xd5, 0x9b, 0x2a, 0x0d, 0x33, 0x31, 0xf4, 0x4d, 0x68, 0x68, 0xf3, 0xe9, 0xb2, 0xbe,
    0x28, 0xeb, 0xce, 0xdb, 0x36, 0x1e, 0xae, 0xb7, 0x37, 0xca, 0xaa, 0xf0, 0x9c, 0x6e, 0x27, 0x93, 0xc9, 0x61, 0x76,
    0x99, 0x1a, 0x0a, 0x99, 0x57, 0xa8, 0xea, 0x71, 0x96, 0x63, 0xbc, 0x76, 0x11, 0x5c, 0x0c, 0xd4, 0x70, 0x0b, 0xd8,
    0x1c, 0x4e, 0x95, 0x89, 0x5b, 0x09, 0x17, 0x08, 0x44, 0x70, 0xec, 0x60, 0x7c, 0xc9, 0x8a, 0xa0, 0xe8, 0x98, 0x64,
    0xfa, 0xe7, 0x52, 0x73, 0xb0, 0x04, 0x9d, 0x78, 0xee, 0x09, 0xa1, 0xb9, 0x79, 0xd5, 0x52, 0x4f, 0xf2, 0x39, 0x1c,
    0xf7, 0xb9, 0x73, 0xe0, 0x3d, 0x6b, 0x54, 0x64, 0x86};

char __attribute__((aligned(16))) RSA_KEYN[] = {
    0x82, 0x78, 0xA0, 0xC5, 0x39, 0xE6, 0xF6, 0xA1, 0x5E, 0xD1, 0xC6, 0x8B, 0x9C, 0xF9, 0xC4, 0x3F, 0xEA, 0x19, 0x16,
    0xB0, 0x96, 0x3A, 0xB0, 0x5A, 0x94, 0xED, 0x6A, 0xD3, 0x83, 0xE8, 0xA0, 0xFD, 0x01, 0x5E, 0x92, 0x2A, 0x7D, 0x0D,
    0xF9, 0x72, 0x1E, 0x03, 0x8A, 0x68, 0x8B, 0x4D, 0x57, 0x55, 0xF5, 0x2F, 0x9A, 0xC9, 0x45, 0xCF, 0x9B, 0xB7, 0xF5,
    0x11, 0x94, 0x7A, 0x16, 0x0B, 0xED, 0xD9, 0xA3, 0xF0, 0x63, 0x8A, 0xEC, 0xD3, 0x21, 0xAB, 0xCF, 0x74, 0xFC, 0x6B,
    0xCE, 0x06, 0x4A, 0x51, 0xC9, 0x7C, 0x7C, 0xA3, 0xC4, 0x10, 0x63, 0x7B, 0x00, 0xEC, 0x2D, 0x02, 0x18, 0xD5, 0xF1,
    0x8E, 0x19, 0x7F, 0xBE, 0xE2, 0x45, 0x5E, 0xD7, 0xA8, 0x95, 0x90, 0x88, 0xB0, 0x73, 0x35, 0x89, 0x66, 0x1C, 0x23,
    0xB9, 0x6E, 0x88, 0xE0, 0x7A, 0x57, 0xB0, 0x55, 0x8B, 0x81, 0x9B, 0x9C, 0x34, 0x9F, 0x86, 0x0E, 0x15, 0x94, 0x2C,
    0x6B, 0x12, 0xC3, 0xB9, 0x56, 0x60, 0x25, 0x59, 0x3E, 0x50, 0x7B, 0x62, 0x4A, 0xD0, 0xF0, 0xB6, 0xB1, 0x94, 0x83,
    0x51, 0x66, 0x6F, 0x60, 0x4D, 0xEF, 0x8F, 0x94, 0xA6, 0xD1, 0xA2, 0x80, 0x06, 0x24, 0xF2, 0x6E, 0xD2, 0xC7, 0x01,
    0x34, 0x8D, 0x2B, 0x6B, 0x03, 0xF7, 0x05, 0xA3, 0x99, 0xCC, 0xC5, 0x16, 0x75, 0x1A, 0x81, 0xC1, 0x67, 0xA0, 0x88,
    0xE6, 0xE9, 0x00, 0xFA, 0x62, 0xAF, 0x2D, 0xA9, 0xFA, 0xC3, 0x30, 0x34, 0x98, 0x05, 0x4C, 0x1A, 0x81, 0x0C, 0x52,
    0xCE, 0xBA, 0xD6, 0xEB, 0x9C, 0x1E, 0x76, 0x01, 0x41, 0x6C, 0x34, 0xFB, 0xC0, 0x83, 0xC5, 0x4E, 0xB3, 0xF2, 0x5B,
    0x4F, 0x94, 0x08, 0x33, 0x87, 0x5E, 0xF8, 0x39, 0xEF, 0x7F, 0x72, 0x94, 0xFF, 0xD7, 0x51, 0xE8, 0xA2, 0x5E, 0x26,
    0x25, 0x5F, 0xE9, 0xCC, 0x2A, 0x7D, 0xAC, 0x5B, 0x35};

char __attribute__((aligned(16))) RSA_KEY_PrivateE[] = {
    0x49, 0x7E, 0x93, 0xE9, 0xA5, 0x7D, 0x42, 0x0E, 0x92, 0xB0, 0x0E, 0x6C, 0x94, 0xC7, 0x69, 0x52, 0x2B, 0x97, 0x68,
    0x5D, 0x9E, 0xB2, 0x7E, 0xA6, 0xF7, 0xDF, 0x69, 0x5E, 0xAE, 0x9E, 0x7B, 0x19, 0x2A, 0x0D, 0x50, 0xBE, 0xD8, 0x64,
    0xE7, 0xCF, 0xED, 0xB2, 0x46, 0xE4, 0x2F, 0x1C, 0x29, 0x07, 0x45, 0xAF, 0x44, 0x3C, 0xFE, 0xB3, 0x3C, 0xDF, 0x7A,
    0x10, 0x26, 0x18, 0x43, 0x95, 0x02, 0xAD, 0xA7, 0x98, 0x81, 0x2A, 0x3F, 0xCF, 0x8A, 0xD7, 0x12, 0x6C, 0xAE, 0xC8,
    0x37, 0x6C, 0xF9, 0xAE, 0x6A, 0x96, 0x52, 0x4B, 0x99, 0xE5, 0x35, 0x74, 0x93, 0x87, 0x76, 0xAF, 0x08, 0xB8, 0x73,
    0x72, 0x7D, 0x50, 0xA5, 0x81, 0x26, 0x5C, 0x8F, 0x94, 0xEA, 0x73, 0x59, 0x5C, 0x33, 0xF9, 0xC3, 0x65, 0x1E, 0x92,
    0xCD, 0x20, 0xC3, 0xBF, 0xD7, 0x8A, 0xCF, 0xCC, 0xD0, 0x61, 0xF8, 0xFB, 0x1B, 0xF4, 0xB6, 0x0F, 0xD4, 0xCF, 0x3E,
    0x55, 0x48, 0x4C, 0x99, 0x2D, 0x40, 0x44, 0x7C, 0xBA, 0x7B, 0x6F, 0xDB, 0x5D, 0x71, 0x91, 0x2D, 0x93, 0x80, 0x19,
    0xE3, 0x26, 0x5D, 0x59, 0xBE, 0x46, 0x6D, 0x90, 0x4B, 0xDF, 0x72, 0xCE, 0x6C, 0x69, 0x72, 0x8F, 0x5B, 0xA4, 0x74,
    0x50, 0x2A, 0x42, 0x95, 0xB2, 0x19, 0x04, 0x88, 0xD7, 0xDA, 0xBB, 0x17, 0x23, 0x69, 0xF4, 0x52, 0xEB, 0xC8, 0x55,
    0xBE, 0xBC, 0x2E, 0xA9, 0xD0, 0x57, 0x7D, 0xC6, 0xC8, 0x8B, 0x86, 0x7B, 0x73, 0xCD, 0xE4, 0x32, 0x79, 0xC0, 0x75,
    0x53, 0x53, 0xE7, 0x59, 0x38, 0x0A, 0x8C, 0xEC, 0x06, 0xA9, 0xFC, 0xA5, 0x15, 0x81, 0x61, 0x3E, 0x44, 0xCD, 0x05,
    0xF8, 0x54, 0x04, 0x00, 0x79, 0xB2, 0x0D, 0x69, 0x2A, 0x47, 0x60, 0x1A, 0x2B, 0x79, 0x3D, 0x4B, 0x50, 0x8A, 0x31,
    0x72, 0x48, 0xBB, 0x75, 0x78, 0xD6, 0x35, 0x90, 0xE1,
};

char __attribute__((aligned(16))) RSA_KEY_PublicE[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};

void halt(void)
{
    printf("[HALT]\r\n");
    while (1)
        ;
}

void verify_rsa(void)
{
    U8 __attribute__((aligned(16))) rsa_encrypt_out[256];
    U8 __attribute__((aligned(16))) rsa_decrypt_out[256];

    memset(rsa_encrypt_out, 0, sizeof(rsa_encrypt_out));
    memset(rsa_decrypt_out, 0, sizeof(rsa_decrypt_out));

    printf("\nTest %s encrypt\n", __FUNCTION__);
    {
        rsaConfig config  = {0};
        config.pu32KeyN   = (U32 *)RSA_KEYN;
        config.pu32KeyE   = (U32 *)RSA_KEY_PrivateE;
        config.u32KeyLen  = 256;
        config.pu32Sig    = (U32 *)(RSA_plaintext);
        config.u32SigLen  = sizeof(RSA_plaintext);
        config.bPublicKey = 0;
        config.pu32Output = (U32 *)rsa_encrypt_out;
        MDrv_RSA_Run(&config);
    }
    printf("Test %s decrypt\n", __FUNCTION__);
    {
        rsaConfig config  = {0};
        config.pu32KeyN   = (U32 *)RSA_KEYN;
        config.pu32KeyE   = (U32 *)RSA_KEY_PublicE;
        config.u32KeyLen  = 256;
        config.pu32Sig    = (U32 *)(rsa_encrypt_out);
        config.u32SigLen  = sizeof(rsa_encrypt_out);
        config.pu32Output = (U32 *)rsa_decrypt_out;

        config.bPublicKey = 1;
        MDrv_RSA_Run(&config);
    }

    if (Compare_data((char *)RSA_plaintext, (char *)rsa_decrypt_out, 256))
    {
        printf("Failed\n");
        printf("RSA_plaintext:\n");
        Dump_data(RSA_plaintext, 256);
        printf("rsa_encrypt_out:\n");
        Dump_data(rsa_encrypt_out, 256);
        printf("rsa_decrypt_out:\n");
        Dump_data(rsa_decrypt_out, 256);
    }
    else
    {
        printf("passed!!\n");
    }
}

/***********************************************************/

char __attribute__((aligned(16))) sha_plaintext[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
int  sha_psize                                    = 56;
char sha_digest[] = {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                     0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};

void verify_sha(void)
{
    char out_buf[128] = {0};

    printf("\nTest %s ", __FUNCTION__);

    MDrv_SHA_Run((unsigned long)sha_plaintext, sha_psize, E_SHA_MODE_256, (U16 *)out_buf);

    if (Compare_data(out_buf, sha_digest, 32))
    {
        printf("Failed\n");
        Dump_data(out_buf, 32);
    }
    else
    {
        printf("passed!!\n");
    }
}

int check_repeat(unsigned int *arr, int len)
{
    int i      = 0;
    int j      = 0;
    int repeat = 0;

    for (i = 0; i < len - 1; i++)
    {
        for (j = i + 1; j < len; j++)
        {
            if (arr[i] == arr[j])
                repeat++;
        }
    }
    if (repeat >= 2)
        return 1;
    return 0;
}

void verify_rng()
{
    int          i;
    unsigned int u16rand[5];
    for (i = 0; i < sizeof(u16rand) / sizeof(u16rand[0]); i++)
    {
        u16rand[i] = MDrv_RNG_Read();
        printf("Random Value: %x\n", u16rand[i]);
    }

    printf("\nTest %s ", __FUNCTION__);

    if (check_repeat(u16rand, sizeof(u16rand) / sizeof(u16rand[0])))
    {
        printf("Failed\n");
    }
    else
    {
        printf("passed!!\n");
    }
}

typedef struct
{
    char          name[20];
    aesdmaConfig *cfg;
    char *        gold;
} aesVerify;

int do_verifyaes(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    aesdmaConfig ecb_aes_128_enc = {.u64SrcAddr = (unsigned long)aes_ecb_plaintext,
                                    .u32Size    = sizeof(aes_ecb_plaintext),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes_ecb_key,
                                    .bSetIV     = 0,
                                    .bDecrypt   = 0,
                                    .pu16IV     = NULL,
                                    .eChainMode = E_AESDMA_CHAINMODE_ECB,
                                    .keylen     = sizeof(aes_ecb_key)};

    aesdmaConfig ecb_aes_128_dec = {.u64SrcAddr = (unsigned long)aes_ecb_result,
                                    .u32Size    = sizeof(aes_ecb_result),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes_ecb_key,
                                    .bSetIV     = 0,
                                    .bDecrypt   = 1,
                                    .pu16IV     = NULL,
                                    .eChainMode = E_AESDMA_CHAINMODE_ECB,
                                    .keylen     = sizeof(aes_ecb_key)};

    aesdmaConfig cbc_aes_128_enc = {.u64SrcAddr = (unsigned long)aes_cbc_plaintext,
                                    .u32Size    = sizeof(aes_cbc_plaintext),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes_cbc_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 0,
                                    .pu16IV     = (U16 *)aes_cbc_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CBC,
                                    .keylen     = sizeof(aes_cbc_key)};

    aesdmaConfig cbc_aes_128_dec = {.u64SrcAddr = (unsigned long)aes_cbc_result,
                                    .u32Size    = sizeof(aes_cbc_result),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes_cbc_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 1,
                                    .pu16IV     = (U16 *)aes_cbc_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CBC,
                                    .keylen     = sizeof(aes_cbc_key)};

    aesdmaConfig ctr_aes_128_enc = {.u64SrcAddr = (unsigned long)aes_ctr_plaintext,
                                    .u32Size    = sizeof(aes_ctr_plaintext),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes_ctr_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 0,
                                    .pu16IV     = (U16 *)aes_ctr_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CTR,
                                    .keylen     = sizeof(aes_ctr_key)};

    aesdmaConfig ctr_aes_128_dec = {.u64SrcAddr = (unsigned long)aes_ctr_result,
                                    .u32Size    = sizeof(aes_ctr_result),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes_ctr_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 1,
                                    .pu16IV     = (U16 *)aes_ctr_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CTR,
                                    .keylen     = sizeof(aes_ctr_key)};

    aesdmaConfig ecb_aes_256_enc = {.u64SrcAddr = (unsigned long)aes_ecb_plaintext,
                                    .u32Size    = sizeof(aes_ecb_plaintext),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes256_ecb_key,
                                    .bSetIV     = 0,
                                    .bDecrypt   = 0,
                                    .pu16IV     = NULL,
                                    .eChainMode = E_AESDMA_CHAINMODE_ECB,
                                    .keylen     = sizeof(aes256_ecb_key)};

    aesdmaConfig ecb_aes_256_dec = {.u64SrcAddr = (unsigned long)aes256_ecb_result,
                                    .u32Size    = sizeof(aes256_ecb_result),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes256_ecb_key,
                                    .bSetIV     = 0,
                                    .bDecrypt   = 1,
                                    .pu16IV     = NULL,
                                    .eChainMode = E_AESDMA_CHAINMODE_ECB,
                                    .keylen     = sizeof(aes256_ecb_key)};

    aesdmaConfig cbc_aes_256_enc = {.u64SrcAddr = (unsigned long)aes_cbc_plaintext,
                                    .u32Size    = sizeof(aes_cbc_plaintext),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes256_cbc_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 0,
                                    .pu16IV     = (U16 *)aes_cbc_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CBC,
                                    .keylen     = sizeof(aes256_cbc_key)};

    aesdmaConfig cbc_aes_256_dec = {.u64SrcAddr = (unsigned long)aes256_cbc_result,
                                    .u32Size    = sizeof(aes256_cbc_result),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes256_cbc_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 1,
                                    .pu16IV     = (U16 *)aes_cbc_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CBC,
                                    .keylen     = sizeof(aes256_cbc_key)};

    aesdmaConfig ctr_aes_256_enc = {.u64SrcAddr = (unsigned long)aes_ctr_plaintext,
                                    .u32Size    = sizeof(aes_ctr_plaintext),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes256_ctr_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 0,
                                    .pu16IV     = (U16 *)aes_ctr_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CTR,
                                    .keylen     = sizeof(aes256_ctr_key)};

    aesdmaConfig ctr_aes_256_dec = {.u64SrcAddr = (unsigned long)aes256_ctr_result,
                                    .u32Size    = sizeof(aes256_ctr_result),
                                    .u64DstAddr = (unsigned long)out_buf,
                                    .eKeyType   = E_AESDMA_KEY_CIPHER,
                                    .pu16Key    = (U16 *)aes256_ctr_key,
                                    .bSetIV     = 1,
                                    .bDecrypt   = 1,
                                    .pu16IV     = (U16 *)aes_ctr_iv,
                                    .eChainMode = E_AESDMA_CHAINMODE_CTR,
                                    .keylen     = sizeof(aes256_ctr_key)};

    aesVerify aesVerify[] = {
        {"ecb_aes_128_enc", &ecb_aes_128_enc, aes_ecb_result},
        {"ecb_aes_128_dec", &ecb_aes_128_dec, aes_ecb_plaintext},
        {"cbc_aes_128_enc", &cbc_aes_128_enc, aes_cbc_result},
        {"cbc_aes_128_dec", &cbc_aes_128_dec, aes_cbc_plaintext},
        {"ctr_aes_128_enc", &ctr_aes_128_enc, aes_ctr_result},
        {"ctr_aes_128_dec", &ctr_aes_128_dec, aes_ctr_plaintext},
        {"ecb_aes_256_enc", &ecb_aes_256_enc, aes256_ecb_result},
        {"ecb_aes_256_dec", &ecb_aes_256_dec, aes_ecb_plaintext},
        {"cbc_aes_256_enc", &cbc_aes_256_enc, aes256_cbc_result},
        {"cbc_aes_256_dec", &cbc_aes_256_dec, aes_cbc_plaintext},
        {"ctr_aes_256_enc", &ctr_aes_256_enc, aes256_ctr_result},
        {"ctr_aes_256_dec", &ctr_aes_256_dec, aes_ctr_plaintext},
    };

    for (int i = 0; i < sizeof(aesVerify) / sizeof(aesVerify[0]); i++)
    {
        MDrv_AESDMA_Run(aesVerify[i].cfg);
        if (Compare_data((void *)(unsigned long)aesVerify[i].cfg->u64DstAddr, aesVerify[i].gold,
                         aesVerify[i].cfg->u32Size))
        {
            printf("%s Failed\n", aesVerify[i].name);
            printf("gold:\r\n");
            Dump_data(aesVerify[i].gold, sizeof(aesVerify[i].gold));
            printf("out:\r\n");
            Dump_data((void *)(unsigned long)aesVerify[i].cfg->u64DstAddr, aesVerify[i].cfg->u32Size);
        }
        else
        {
            printf("%s passed!!\n", aesVerify[i].name);
        }
    }

    verify_rsa(); // RSA2048 Encrypt/decrypt

    verify_sha();

    verify_rng();
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(verifyaes, CONFIG_SYS_MAXARGS, 1, do_verifyaes, "verifyaes - Verify AES/RSA/SHA/RNG function\n", "");

#if defined(CONFIG_SSTAR_AESDMA)

void image_set_encrypt_flag(U8 *hdr)
{
    hdr[0x3F] = 'E';
}

int do_aesdma(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    aesdmaConfig  config      = {0};
    unsigned long image_start = 0;
    int           ret         = 0;

    if (argc < 5)
    {
        printf("missing parameters\n");
        return CMD_RET_USAGE;
    }

    if (strncmp(argv[1], "dec", 3) == 0)
    {
        config.bDecrypt = 1;
    }
    else if (strncmp(argv[1], "enc", 3) == 0)
    {
        config.bDecrypt = 0;
    }
    else
    {
        return CMD_RET_USAGE;
    }

    image_start = (u32)simple_strtoul(argv[2], NULL, 16);

    printf("image_addr = 0x%08lX\n", image_start);

    if (!image_check_magic((void *)image_start))
    {
        printf("image header check failed, can't get data size\n");
        return CMD_RET_USAGE;
    }
    else
    {
        config.u32Size    = image_get_data_size((void *)image_start);
        config.u64SrcAddr = config.u64DstAddr = image_start + image_get_header_size();
        printf("image header check ok, data size=0x%08X\n", config.u32Size);
        printf("data start addr=0x%08llX\n", config.u64SrcAddr);
    }

    if (strncmp(argv[3], "ECB", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_ECB;
    }
    else if (strncmp(argv[3], "CTR", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_CTR;
    }
    else if (strncmp(argv[3], "CBC", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_CBC;
    }
    else
    {
        printf("use default chainmode - CBC\n");
        config.eChainMode = E_AESDMA_CHAINMODE_CBC;
    }

    if (strncmp(argv[4], "CIPHER", 6) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_CIPHER;
        config.pu16Key  = (U16 *)(KEY_CUST_LOAD_ADDRESS + image_get_header_size());
    }
    else if (strncmp(argv[4], "EFUSE", 5) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_OTP_EFUSE_KEY1;
    }
    else if (strncmp(argv[4], "HW", 2) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_OTP_EFUSE_KEY1;
    }
    else
    {
        printf("use default keytype - EFUSE\n");
        config.eKeyType = E_AESDMA_KEY_OTP_EFUSE_KEY1;
    }

    MDrv_AESDMA_Run(&config);

    if (config.bDecrypt)
    {
        printf("Decrypt done!\n");
    }
    else
    {
        printf("Encrypt done!\n");
        image_set_encrypt_flag((U8 *)image_start);
    }

    if (6 == argc)
    {
        char *buffer = (char *)malloc(BUF_SIZE);
        if ((buffer == NULL))
        {
            printf("no memory for command string!!\n");
            return -1;
        }
        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "mxp r.info %s", argv[5]);
        ret |= run_command(buffer, 0);

        ret |= run_command("sf probe", 0);
        ret |= run_command("sf erase $(sf_part_start) $(sf_part_size)", 0);

        memset(buffer, 0, BUF_SIZE);
        sprintf(buffer, "sf write %lx $(sf_part_start) %x", image_start, config.u32Size + image_get_header_size());
        ret |= run_command(buffer, 0);

        free(buffer);

        if (!ret)
            printf("Secure writeback \033[1;36m%s\033[m done\n\n", argv[5]);
        // run_command("reset", 0);
    }
    return ret;
}

U_BOOT_CMD(aes, CONFIG_SYS_MAXARGS, 1, do_aesdma, "Control SigmaStar AES engine",
           "direction image_addr chainmode keytype partition\n\n"
           "\tdirection - enc, dec\n"
           "\timage_addr - image location\n"
           "\tchainmode - ECB, CTR, CBC\n"
           "\tkeytype - CIPHER, EFUSE, HW\n"
           "\tpartition - partition name to program into\n");

void *memcpy_4byte(void *dst, const void *src, size_t n)
{
    U32 *      pdst = (U32 *)dst;
    const U32 *psrc = (const U32 *)src;
    n               = (n % 4) ? (n / 4 + 1) : (n / 4);

    for (; n > 0; ++pdst, ++psrc, --n)
    {
        *pdst = *psrc;
    }

    return (dst);
}
void chip_flush_miu_pipe(void)
{
    unsigned short dwReadData = 0;
    // toggle the flush miu pipe fire bit
    *(volatile unsigned short *)(0x1F204414) = 0x0;
    *(volatile unsigned short *)(0x1F204414) = 0x1;
    do
    {
        dwReadData = *(volatile unsigned short *)(0x1F204440);
        dwReadData &= BIT12; // Check Status of Flush Pipe Finish
    } while (dwReadData == 0);
}

#if 0
U8 image_check_encryption(void *hdr)
{
	return (image_get_encryption((image_header_t *)hdr) == 'E' ? 1 : 0);
}
#endif

int do_auth(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    aesdmaConfig config      = {0};
    u8           uAuthON     = 0;
    u8           uAesON      = 0;
    u64          image_start = 0;
    int          ret         = 0;

    if (argc < 8)
    {
        printf("missing parameters\n");
        return CMD_RET_USAGE;
    }

    if (strncmp(argv[1], "dec", 3) == 0)
    {
        config.bDecrypt = 1;
    }
    else if (strncmp(argv[1], "enc", 3) == 0)
    {
        config.bDecrypt = 0;
    }
    else
    {
        return CMD_RET_USAGE;
    }

    image_start = (u32)simple_strtoul(argv[2], NULL, 16);

    printf("image_addr = 0x%08llX\n", image_start);

    // Check kernel.xz.img magic is 27 05 19 56, first 4 bytes
    if (!image_check_magic((void *)(unsigned long)image_start))
    {
        printf("image header check failed, can't get data size\n");
        return CMD_RET_USAGE;
    }
    else
    { // real image address
        config.u32Size    = image_get_data_size((void *)(unsigned long)image_start);
        config.u64SrcAddr = config.u64DstAddr = image_start;
        printf("image header check ok, data size=0x%08X\n", config.u32Size); // Data Siz or Data Siz with padding
        printf("data start addr=0x%08llX\n", config.u64SrcAddr);
    }

    if (strncmp(argv[3], "ECB", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_ECB;
        printf("[U-Boot] ChainMode = ECB\n");
    }
    else if (strncmp(argv[3], "CTR", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_CTR;
        printf("[U-Boot] ChainMode = CTR\n");
    }
    else if (strncmp(argv[3], "CBC", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_CBC;
        printf("[U-Boot] ChainMode = CBC\n");
    }
    else
    {
        printf("use default chainmode - CBC\n");
        config.eChainMode = E_AESDMA_CHAINMODE_CBC;
    }

    if (strncmp(argv[4], "CIPHER", 6) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_CIPHER;
        config.pu16Key  = (U16 *)(KEY_CUST_LOAD_ADDRESS + 0x210);
        printf("[U-Boot] KeyType = CIPHER\n");
    }
    else if (strncmp(argv[4], "EFUSE", 5) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_OTP_EFUSE_KEY1;
        printf("[U-Boot] KeyType = EFUSE\n");
    }
    else if (strncmp(argv[4], "HW", 2) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_OTP_EFUSE_KEY1;
        printf("[U-Boot] KeyType = HW\n");
    }
    else
    {
        printf("use default keytype - EFUSE\n");
        config.eKeyType = E_AESDMA_KEY_OTP_EFUSE_KEY1;
    }

    if (config.bDecrypt)
    {
        printf("Decrypt setting done!\n");
    }
    else
    {
        printf("Encrypt setting done!\n");
        // image_set_encrypt_flag((U8*)image_start);
    }

    if (strncmp(argv[6], "AUTHON", 6) == 0)
    {
        uAuthON = 1;
    }
    else if (strncmp(argv[6], "AUTHOFF", 7) == 0)
    {
        uAuthON = 0;
    }
    else
    {
        return CMD_RET_USAGE;
    }

    if (strncmp(argv[7], "AESON", 5) == 0)
    {
        uAesON = 1;
    }
    else if (strncmp(argv[7], "AESOFF", 6) == 0)
    {
        uAesON = 0;
    }
    else
    {
        return CMD_RET_USAGE;
    }

    if (8 == argc)
    {
        /*Proceed RSA authentication first , then do AES decryption*/
        if (uAuthON)
        {
            if (runAuthenticate(config.u64SrcAddr, config.u32Size + image_get_header_size(),
                                (U32 *)(KEY_CUST_LOAD_ADDRESS)))
            {
                printf("[U-Boot] Authenticate KERNEL pass!\n\r");
            }
            else
            {
                printf("[U-Boot] Authenticate KERNEL failed!\n\r");
                uAesON = 0;
                halt();
            }
        }
        if (uAesON)
        {
            printf("[U-Boot] Decrypt Kernel\n\r");
            config.u32Size    = config.u32Size;
            config.u64SrcAddr = config.u64DstAddr = image_start + image_get_header_size();
            printf("SrcAddr 0x%08llX DstAddr 0x%08llX AES size=0x%08X --> MDrv_AESDMA_Run\n", config.u64SrcAddr,
                   config.u64DstAddr, config.u32Size);
            MDrv_AESDMA_Run(&config);
            printf("[U-Boot] Decrypt AES done!\n\r");
        }
    }

    return ret;
}

U_BOOT_CMD(secauth, CONFIG_SYS_MAXARGS, 1, do_auth, "Control Sstar security authenticate sequence",
           "direction image_addr chainmode keytype partition\n\n"
           "\tdirection - enc, dec\n"
           "\timage_addr - image location\n"
           "\tchainmode - ECB, CTR, CBC\n"
           "\tkeytype - CIPHER, EFUSE, HW\n"
           "\tpartition - partition name to program into\n");
#endif
