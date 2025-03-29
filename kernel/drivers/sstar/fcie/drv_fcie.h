/*
 * drv_fcie.h- Sigmastar
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

#ifndef _DRV_FCIE_H_
#define _DRV_FCIE_H_

#define SSTAR_FCIE_ECC_BUFFER_SIZE (0x1400)
#define SSTAR_FCIE_ECC_DATA_SIZE   (0x1000)

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
    FCIE_SUCCESS = 0x00,
    FCIE_TIMEOUT,
    FCIE_INVALID,
    FCIE_DEVICE_FAILURE
} fcie_error_number;

typedef enum
{
    FCIE_IF_0 = 0,

    FCIE_IF_NUM
} fcie_interface;

struct sstar_fcie_ecc
{
    /* input */
    u8  path;
    u8  mode;
    u8  sector_cnt;
    u8 *fcie_buffer;

    /* config */
    u16 page_size;
    u16 oob_size;
    u16 sector_size;

    /* output */
    u8 ecc_status;
    u8 ecc_bitflip_cnt;
};

void sstar_fcie_get(fcie_interface fcie_id);
void sstar_fcie_release(fcie_interface fcie_id);
u8   sstar_fcie_ecc_set_config(fcie_interface fcie_id, struct sstar_fcie_ecc *ecc);
u8   sstar_fcie_ecc_encode(fcie_interface fcie_id, struct sstar_fcie_ecc *ecc);
u8   sstar_fcie_ecc_decode(fcie_interface fcie_id, struct sstar_fcie_ecc *ecc);

#endif /* _DRV_FCIE_H_ */
