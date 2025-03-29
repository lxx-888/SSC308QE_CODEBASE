/*
 * hal_rgn_if.h - Sigmastar
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

#ifndef __HAL_RGN_IF_H__
#define __HAL_RGN_IF_H__

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifndef __HAL_RGN_IF_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE MS_S32          HAL_RGN_IF_GetChipCapOps(MHAL_RGN_ChipCapOps_t *pstChipCapOps);
INTERFACE MHAL_RGN_Dev_t *HAL_RGN_IF_Create(const MHAL_RGN_DevAttr_t *pstRgnDevAttr);
INTERFACE void            HAL_RGN_IF_Destory(const MHAL_RGN_Dev_t *pstRgnDev);
INTERFACE MS_S32          HAL_RGN_IF_Active(const MHAL_RGN_Dev_t *pstRgnDev);
INTERFACE MS_S32          HAL_RGN_IF_Deactive(const MHAL_RGN_Dev_t *pstRgnDev);

#undef INTERFACE
#endif /* __HAL_RGN_IF_H__ */
