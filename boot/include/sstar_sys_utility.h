/*
 * sstar_sys_utility.h - Sigmastar
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

#ifndef _SSTAR_SYS_UTILITY_H_
#define _SSTAR_SYS_UTILITY_H_

#include <sstar_types.h>
#include <command.h>

MS_U8 ReadByte(MS_U32 u32RegAddr);
MS_U16 Read2Byte(MS_U32 u32RegAddr);
MS_BOOL WriteByte(MS_U32 u32RegAddr, MS_U8 u16Val);
MS_BOOL Write2Byte(MS_U32 u32RegAddr, MS_U16 u16Val);
MS_BOOL WriteRegBit(MS_U32 u32RegAddr, MS_U16 u8Bit, MS_BOOL bEnable);
MS_BOOL WriteRegBitPos(MS_U32 u32RegAddr, MS_U8 u8Bit, MS_BOOL bEnable);

int do_riu(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);
int do_gpio(struct cmd_tbl * cmdtp, int flag, int argc, char * const argv[]);
int do_sar(struct cmd_tbl * cmdtp, int flag, int argc, char * const argv[]);

#endif /* _SSTAR_SYS_UTILITY_H_ */

