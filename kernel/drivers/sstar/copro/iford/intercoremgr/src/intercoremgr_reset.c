/*
 * intercoremgr_reset.c - Sigmastar
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

#include "intercoremgr_reset.h"
#include <stdlib.h>

s32 SS_InterCoreMgr_Reset_ReLoad_Riscv(char *imgPathName, u8 isTcmMode)
{
    // todo
    // s32 SS_InterCoreMgr_IMPL_Reset_ReLoad_Riscv(char* imgPathName, u8
    // isTcmMode);  //steps：reset&reload
    // s32 _SS_InterCoreMgr_IMPL_Reset_Riscv(void)
    // s32 _SS_InterCoreMgr_IMPL_Reload_Riscv(char* imgPathName, u8 isTcmMode)
    // s32 _SS_InterCoreMgr_IMPL_UnReset_Riscv(void)

    return 0;
}

#if 0
s32 SS_InterCoreMgr_IMPL_Reset_ReLoad_CM4(void)
{
    //todo
    //  s32 SS_InterCoreMgr_IMPL_Reset_ReLoad_CM4(void);  //CM4 running @TCM
    return 0;
}
#endif

s32 SS_InterCoreMgr_Reset_ARM(void)
{
    // todo
    // s32 _SS_InterCoreMgr_Reset_IPU(void)
    return 0;
}

s32 SS_InterCoreMgr_Global_Reset(void)
{
    return system("reboot");
}

s32 SS_InterCoreMgr_PermInfo_Read(u8 offset, u8 readLength, void *rdDataPtr)
{
    // todo
    // SS_InterCoreMgr_IMPL_PermInfo_Read(u8 offset, u8 readLength, void
    // *rdDataPtr)
    return 0;
}

s32 SS_InterCoreMgr_PermInfo_Write(u8 offset, u8 writeLength, void *wrtDataPtr)
{
    // todo
    // S_InterCoreMgr_IMPL_PermInfo_Write(u8 offset, u8 writeLength, void
    // *wrtDataPtr)
    return 0;
}
