/*
 * sstar_sys_utility.c - Sigmastar
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
#include <log.h>
#include <sstar_sys_utility.h>
#include <asm/arch/mach/io.h>
#include <asm/arch/mach/platform.h>

#ifndef RIUBASEADDR
#define RIUBASEADDR (0x1F000000UL)
#endif

static const void *uboot_ld_addr __attribute__((used)) = (void *)CONFIG_SYS_TEXT_BASE;
static const void *uboot_ep_addr __attribute__((used)) = (void *)CONFIG_SYS_TEXT_BASE;

MS_U8 ReadByte(MS_U32 u32RegAddr)
{
    if (u32RegAddr & 0x01)
        return INREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFF00) << 1) + (((u32RegAddr - 1) & 0xFF) << 1) + 1);
    else
        return INREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFFFF) << 1));
}

MS_U16 Read2Byte(MS_U32 u32RegAddr)
{
    MS_U16 u16Value = 0;

    if (u32RegAddr & 0x01)
    {
        u16Value = INREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFF00) << 1) + (((u32RegAddr - 1) & 0xFF) << 1) + 1);
        u16Value = (u16Value & 0xFF) << 8;
        u16Value += INREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFF00) << 1) + (((u32RegAddr - 1) & 0xFF) << 1));
    }
    else
    {
        u16Value = INREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFFFF) << 1) + 1);
        u16Value = (u16Value & 0xFF) << 8;
        u16Value += INREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFFFF) << 1));
    }

    return u16Value;
}

MS_BOOL WriteByte(MS_U32 u32RegAddr, MS_U8 u8Val)
{
    if (u32RegAddr & 0x01)
        OUTREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFF00) << 1) + (((u32RegAddr - 1) & 0xFF) << 1) + 1, u8Val);
    else
        OUTREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFFFF) << 1), u8Val);

    return TRUE;
}

MS_BOOL Write2Byte(MS_U32 u32RegAddr, MS_U16 u16Val)
{
    if (u32RegAddr & 0x01)
    {
        OUTREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFF00) << 1) + (((u32RegAddr - 1) & 0xFF) << 1) + 1,
                (u16Val >> 8) & 0xFF);
        OUTREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFF00) << 1) + (((u32RegAddr - 1) & 0xFF) << 1), u16Val & 0xFF);
    }
    else
    {
        OUTREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFFFF) << 1) + 1, (u16Val >> 8) & 0xFF);
        OUTREG8(RIUBASEADDR + ((u32RegAddr & 0xFFFFFF) << 1), u16Val & 0xFF);
    }

    return TRUE;
}

MS_BOOL WriteRegBit(MS_U32 u32RegAddr, MS_U16 u16Bit, MS_BOOL bEnable)
{
    MS_U16 u16Value = 0;

    u16Value = Read2Byte(u32RegAddr);
    u16Value = (bEnable) ? (u16Value | u16Bit) : (u16Value & ~u16Bit);
    Write2Byte(u32RegAddr, u16Value);

    return TRUE;
}

MS_BOOL WriteRegBitPos(MS_U32 u32RegAddr, MS_U8 u8Bit, MS_BOOL bEnable)
{
    MS_U16 u16Bit;

    if (u8Bit > 15)
    {
        log_err("Over the bank boundary!\n");
        return FALSE;
    }

    u16Bit = (1 << u8Bit);
    WriteRegBit(u32RegAddr, u16Bit, bEnable);

    return TRUE;
}
