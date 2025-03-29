/*
 * drv_spinor.h- Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

#ifndef _DRV_SPINOR_H_
#define _DRV_SPINOR_H_

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

struct spinor_init
{
    u8          bus;
    u8          cs_select;
    const char *dev_name;
    u8 *        cis_map;
    u8          cis_cnt;
    u8          bypass_io;
    u8 (*autok_parrtern_check)(void);
};

u8 sstar_spinor_init(struct spinor_init *init);

#endif /* _DRV_SPINOR_H_ */
