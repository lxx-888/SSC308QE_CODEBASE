/*
 * hal_disp.h- Sigmastar
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
#ifndef _HAL_DISP_H_
#define _HAL_DISP_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define GET_DISP_ID(pstDispCtx)                                                                     \
    ((pstDispCtx) ? (pstDispCtx)->pstCtxContain->pstDevContain[pstDispCtx->u32ContainIdx]->u32DevId \
                  : HAL_DISP_DEVICE_MAX)
#define GET_DISP_ID_BYDEVCONTAIN(pstDevContain) ((pstDevContain) ? pstDevContain->u32DevId : HAL_DISP_DEVICE_MAX)
#define GET_DISP_RED(ID, BANKNAME, REGNAME) \
    ((ID == 0) ? BANKNAME##0_##REGNAME : (ID == 1) ? BANKNAME##1_##REGNAME : (ID == 2) ? BANKNAME##2_##REGNAME : 0)
//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ, // 0
    E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ,    // 1
    E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ,     // 2
    E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ,   // 3
    E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ,  // 4
    E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ,  // 5
    E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ,  // 6
    E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX,                      // 7
} HAL_DISP_PnlLpllType_e;

typedef struct
{
    HAL_DISP_ST_QueryRet_e (*pGetInfoFunc)(void *, void *);
    void (*pSetFunc)(void *, void *);
    MI_U16 u16CfgSize;
} HAL_DISP_QueryCallBackFunc_t;

typedef struct
{
    MI_U8 bClkGpCtrl[HAL_DISP_DEVICE_MAX];
    void *pvCmdqCtx[HAL_DISP_UTILITY_CMDQ_NUM]; // HAL_DISP_UTILITY_CmdqContext_t
    void *pvClkGpCtrl[HAL_DISP_DEVICE_MAX];
} HAL_DISP_HwContain_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef __HAL_DISP_C__
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE MI_U8  HAL_DISP_GetCmdqByCtx(void *pCtx, void **pCmdqCtx);
INTERFACE MI_U8  HAL_DISP_GetUtilityIdByDevId(MI_U32 u32DevId, MI_S32 *ps32UtilityId);
INTERFACE MI_U32 HAL_DISP_GetDeviceId(void *pstCtx, MI_U8 u8bDevContain);
INTERFACE MI_U16 HAL_DISP_GetDeviceTimeGenStart(void *pstCtx, MI_U32 u32DevId);

INTERFACE void HAL_DISP_SetSwReset(MI_U8 u8Val, void *pCtx, MI_U32 u32DevId);
INTERFACE void HAL_DISP_SetInterCfgDispPat(MI_U8 u8DispPat, MI_U8 u8PatMd, void *pCtx);
INTERFACE void HAL_DISP_SetTgenFwVtt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenHtt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenVtt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenHsyncSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenHsyncEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenVsyncSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenVsyncEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenHfdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenHfdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenVfdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenVfdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenHdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenHdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenVdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenVdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetFrameColor(MI_U8 u8R, MI_U8 u8G, MI_U8 u8B, void *pCtx);
INTERFACE void HAL_DISP_SetFrameColorForce(MI_U8 u8Val, void *pCtx);
INTERFACE void HAL_DISP_SetFixedCsc(void *pCtx);
INTERFACE void HAL_DISP_SetInputPortCrop(void *pCtx);

INTERFACE void HAL_DISP_SetDispMux(void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgHtt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgVtt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgHsyncSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgHsyncEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgVsyncSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgVsyncEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgHfdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgHfdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgVfdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgVfdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgHdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgHdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgVdeSt(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetRdmaPatTgVdeEnd(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetTgenModeEn(void);
INTERFACE void HAL_DISP_SetSgRdmaCfg(MI_SYS_PixelFormat_e ePixelFormat, void *pCtx);
INTERFACE void HAL_DISP_SetSgRdmaSize(MI_SYS_PixelFormat_e ePixelFormatvoid, void *pCtx);
INTERFACE void HAL_DISP_SetSgRdmaBufConfig(void *pCtx);
INTERFACE void HAL_DISP_SetSgRdmaEn(void *pCtx, void *pCfg);
INTERFACE void HAL_DISP_SetSgRdmaFifoPush(void *pCtx);

// LCD
INTERFACE void HAL_DISP_SetCmdqIntMask(MI_U16 u16Val, void *pCtx);
INTERFACE void HAL_DISP_SetCmdqIntClear(MI_U16 u16Val, void *pCtx);

INTERFACE void HAL_DISP_SetTimeGenStartFlag(MI_U16 u16Val, void *pCtx, MI_U32 u32DevId);

// PNL
INTERFACE void HAL_DISP_LpllSetTbl(MI_U16 u16LpllIdx, MI_U32 u32LpllSet, MI_U8 bDsi);
INTERFACE void HAL_DISP_LcdSetUnifiedPrototypeConfig(void *pCtx);
INTERFACE void HAL_DISP_LcdSetUnifiedPolarityConfig(void *pCtx);
INTERFACE void HAL_DISP_LcdSetUnifiedSwapChnConfig(void *pCtx);
INTERFACE void HAL_DISP_LpllSetSscEn(MI_U8 u8Val);
INTERFACE void HAL_DISP_LpllSetSscConfigEx(MI_DISP_IMPL_MhalPnlUnifiedTgnSpreadSpectrumConfig_t *pstSscCfg);
INTERFACE void HAL_DISP_LcdSetUnifiedRgbDeltaMode(void *pCtx);
INTERFACE void HAL_DISP_LcdSetPadMux(void *pCtx);

#undef INTERFACE

#endif
