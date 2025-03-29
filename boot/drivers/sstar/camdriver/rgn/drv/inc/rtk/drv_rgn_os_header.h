/*
 * drv_rgn_os_header.h - Sigmastar
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

#ifndef __DRV_RGN_OS_HEADER_H__
#define __DRV_RGN_OS_HEADER_H__

//------------------------------------------------------------------------------
//  Defines & Macro
//------------------------------------------------------------------------------
#define WRITE_RGN_REG(addr, type, data) ((*(volatile type *)(addr)) = (data))
#define READ_RGN_REG(addr, type)        ((*(volatile type *)(addr)))

#define READ_BYTE(x)     READ_RGN_REG(x, u8)
#define READ_WORD(x)     READ_RGN_REG(x, u16)
#define READ_LONG(x)     READ_RGN_REG(x, u32)
#define WRITE_BYTE(x, y) WRITE_RGN_REG(x, u8, y)
#define WRITE_WORD(x, y) WRITE_RGN_REG(x, u16, y)
#define WRITE_LONG(x, y) WRITE_RGN_REG(x, u32, y)

//------------------------------------------------------------------------------
//  Structure & Emu
//------------------------------------------------------------------------------
typedef struct
{
    CamOsTsem_t tSemCfg;
} DrvRgnOsTsemConfig_t;

#endif
