/*
 * halSEM.h- Sigmastar
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
#ifndef __HAL_SEM_H__
#define __HAL_SEM_H__

#include "ms_types.h"
#include "drvSEM.h"

//-------------------------------------------------------------------------------------------------
//   Macro and Define
//-------------------------------------------------------------------------------------------------
#define SEM_MAX_NUM    (16)
#define SEM_MAX_CLIENT (E_SEM_MAX_NUM)
#define SEM_ID         E_SEM_AESDMA

BOOL HAL_SEM_Get_Resource(U8 u8SemID, U16 u16ResId);
BOOL HAL_SEM_Free_Resource(U8 u8SemID, U16 u16ResId);
BOOL HAL_SEM_Reset_Resource(U8 u8SemID);
BOOL HAL_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId);
U32  HAL_SEM_Get_Num(void);
S16  HAL_SEM_GetSemId(eSemId SemId);

#endif // #ifndef __HAL_SEM_H__
