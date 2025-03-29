/*
 * hal_fcie.h- Sigmastar
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

#ifndef _HAL_FCIE_H_
#define _HAL_FCIE_H_

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

typedef enum
{
    HAL_FCIE_SUCCESS = 0x00,
    HAL_FCIE_TIMEOUT,
    HAL_FCIE_INVALID,
    HAL_FCIE_DEVICE_FAILURE
} hal_fcie_error_number;

typedef enum
{
    FCIE_ECC_PATH_IMI = 0x0,
    FCIE_ECC_PATH_MIU
} hal_fcie_ecc_path;

struct hal_fcie_ecc_config
{
    u16 page_size;
    u16 oob_size;
    u16 sector_size;
    u8  max_correct_bits;
    u8  ecc_step;
    u16 ecc_bytes;
};

struct hal_fcie
{
    /* register bank */
    unsigned long fcie0_base;
    unsigned long fcie3_base;

    /* software flag */
    u8 interrupt_en;

    /* state machine */
    u16 page_size;
    u16 oob_size;
};

u8 hal_fcie_is_soc_ecc(void);
u8 hal_fcie_ecc_reset(struct hal_fcie *hal);
u8 hal_fcie_ecc_get_config(u8 mode, const struct hal_fcie_ecc_config **config);
u8 hal_fcie_ecc_setup(struct hal_fcie *hal, u8 mode);
u8 hal_fcie_ecc_encode(struct hal_fcie *hal, u8 path, u8 sector_cnt, u8 *fcie_buf);
u8 hal_fcie_ecc_decode(struct hal_fcie *hal, u8 path, u8 sector_cnt, u8 *fcie_buf);
u8 hal_fcie_ecc_get_status(struct hal_fcie *hal, u8 *ecc_status, u8 *ecc_bitflip_cnt);

#endif /* _HAL_FSP_QSPI_H_ */
