/*
 * drv_disp_irq.h- Sigmastar
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

#ifndef _DRV_DISP_IRQ_H_
#define _DRV_DISP_IRQ_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _DRV_DISP_IRQ_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
INTERFACE void   DRV_DISP_IRQ_Init(void);
INTERFACE void   DRV_DISP_IRQ_SetIsrNum(MI_U32 u32DevId, MI_U32 u32IsrNum);
INTERFACE MI_U8  DRV_DISP_IRQ_GetIsrNum(void *pDevCtx, MI_U32 *pu32IsrNum);
INTERFACE MI_U8  DRV_DISP_IRQ_GetIsrNumByDevId(MI_U32 u32DevId, MI_U32 *pu32IsrNum);
INTERFACE MI_U8  DRV_DISP_IRQ_Enable(void *pDevCtx, MI_U32 u32DevIrq, MI_U8 bEnable);
INTERFACE MI_U8  DRV_DISP_IRQ_GetFlag(void *pDevCtx, MI_DISP_IMPL_MhalIRQFlag_t *pstIrqFlag);
INTERFACE MI_U8  DRV_DISP_IRQ_Clear(void *pDevCtx, void *pData);
INTERFACE MI_U8  DRV_DISP_IRQ_GetLcdIsrNum(void *pDevCtx, MI_U32 *pu32IsrNum);
INTERFACE MI_U8  DRV_DISP_IRQ_EnableLcd(void *pDevCtx, MI_U32 u32DevIrq, MI_U8 bEnable);
INTERFACE MI_U8  DRV_DISP_IRQ_GetLcdFlag(void *pDevCtx, MI_DISP_IMPL_MhalIRQFlag_t *pstIrqFlag);
INTERFACE MI_U8  DRV_DISP_IRQ_ClearLcd(void *pDevCtx, void *pData);
INTERFACE MI_U8  DRV_DISP_IRQ_Str(void *pCtx, MI_U8 bResume);
INTERFACE MI_U8  DRV_DISP_IRQ_CreateInternalIsr(void *pDispCtx);
INTERFACE MI_U8  DRV_DISP_IRQ_DestroyInternalIsr(void *pDispCtx);
INTERFACE MI_U16 DRV_DISP_IRQ_GetIrqCount(void);
#undef INTERFACE
#endif
