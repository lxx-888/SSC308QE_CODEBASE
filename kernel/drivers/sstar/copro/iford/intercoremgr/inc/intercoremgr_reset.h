/*
 * intercoremgr_reset.h - Sigmastar
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

#ifndef __INTERCOREMGR_RESET_H__
#define __INTERCOREMGR_RESET_H__

#include "intercoremgr.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 SS_InterCoreMgr_Reset_ReLoad_Riscv(char *imgPathName, u8 isTcmMode);
s32 SS_InterCoreMgr_Reset_ARM(void);

s32 SS_InterCoreMgr_Global_Reset(void);
s32 SS_InterCoreMgr_PermInfo_Read(u8 offset, u8 readLength, void *rdDataPtr);
s32 SS_InterCoreMgr_PermInfo_Write(u8 offset, u8 writeLength, void *wrtDataPtr);

#ifdef __cplusplus
}
#endif

#endif
