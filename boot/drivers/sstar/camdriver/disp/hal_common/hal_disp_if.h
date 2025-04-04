/*
 * hal_disp_if.h- Sigmastar
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

#ifndef _HAL_DISP_IF_H_
#define _HAL_DISP_IF_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifdef _HAL_DISP_IF_C_
#define INTERFACE

#else
#define INTERFACE extern
#endif

INTERFACE MI_U8  HAL_DISP_IF_Init(void);
INTERFACE MI_U8  HAL_DISP_IF_DeInit(void);
INTERFACE MI_U8  HAL_DISP_IF_Query(void *pCtx, void *pCfg);
INTERFACE MI_U8  HAL_DISP_IF_ParseFunc(MI_U8 *ps8FuncType, MI_DISP_IMPL_MhalDeviceConfig_t *pstCfg, MI_U8 val);
INTERFACE MI_S32 HAL_DISP_IF_IntsShow(MI_U8 *p8DstBuf);

#undef INTERFACE

#endif
