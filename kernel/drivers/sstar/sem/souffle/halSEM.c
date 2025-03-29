/*
 * halSEM.c- Sigmastar
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
#include "ms_platform.h"
#include "registers.h"
#include "ms_types.h"
#include "halSEM.h"
#include "drvSEM.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
const eSemId SemIdTbl[SEM_MAX_CLIENT] = {SEM_ID};

S16 HAL_SEM_GetSemId(eSemId eSemId)
{
    U8 idx;

    for (idx = 0; idx < SEM_MAX_CLIENT; idx++)
        if (eSemId == SemIdTbl[idx])
            return idx;
    return (-1);
}

U32 HAL_SEM_Get_Num(void)
{
    return SEM_MAX_NUM;
}

BOOL HAL_SEM_Get_Resource(U8 u8SemID, U16 u16ResId)
{
    if (u8SemID > SEM_MAX_NUM)
        return FALSE;

    OUTREG16(GET_REG_ADDR(BASE_REG_SEM_PA, u8SemID), u16ResId);
    return (u16ResId == INREG16(GET_REG_ADDR(BASE_REG_SEM_PA, u8SemID))) ? TRUE : FALSE;
}

BOOL HAL_SEM_Free_Resource(U8 u8SemID, U16 u16ResId)
{
    if (u8SemID > SEM_MAX_NUM)
        return FALSE;

    if (u16ResId != INREG16(GET_REG_ADDR(BASE_REG_SEM_PA, u8SemID)))
    {
        return FALSE;
    }

    OUTREG16(GET_REG_ADDR(BASE_REG_SEM_PA, u8SemID), 0x00);
    return TRUE;
}

BOOL HAL_SEM_Reset_Resource(U8 u8SemID)
{
    if (u8SemID > SEM_MAX_NUM)
        return FALSE;

    OUTREG16(GET_REG_ADDR(BASE_REG_SEM_PA, u8SemID), 0x00);
    return TRUE;
}

BOOL HAL_SEM_Get_ResourceID(U8 u8SemID, U16* pu16ResId)
{
    if (u8SemID > SEM_MAX_NUM)
        return FALSE;

    *pu16ResId = INREG16(GET_REG_ADDR(BASE_REG_SEM_PA, u8SemID));
    return TRUE;
}
