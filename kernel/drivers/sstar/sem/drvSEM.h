/*
 * drvSEM.h- Sigmastar
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
#ifndef __DRV_SEM_H__
#define __DRV_SEM_H__

#include "ms_types.h"

#define SEM_ARM_S_ID  (0x01)
#define SEM_ARM_NS_ID (0x02)

#define SEM_WAIT_FOREVER  (0xffffff00)
#define TICK_PER_ONE_MS   (1) // Note: confirm Kernel fisrt
#define MSOS_WAIT_FOREVER (0xffffff00 / TICK_PER_ONE_MS)

//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_SEM_AESDMA,
    E_SEM_MAX_NUM,
} eSemId;

BOOL MDrv_SEM_Init(void);
BOOL MDrv_SEM_Get_Resource(U8 u8SemID, U16 u16ResId);
BOOL MDrv_SEM_Free_Resource(U8 u8SemID, U16 u16ResId);
BOOL MDrv_SEM_Reset_Resource(U8 u8SemID);
BOOL MDrv_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId);
U32  MDrv_SEM_Get_Num(void);
BOOL MDrv_SEM_Lock(eSemId SemId, U32 u32WaitMs);
BOOL MDrv_SEM_Unlock(eSemId SemId);
BOOL MDrv_SEM_Delete(eSemId SemId);

#endif // #ifndef __DRV_SEM_H__
