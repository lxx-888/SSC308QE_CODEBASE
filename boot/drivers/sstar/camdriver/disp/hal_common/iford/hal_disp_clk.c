/*
 * hal_disp_clk.c- Sigmastar
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
#define _HAL_DISP_CLK_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mi_common_datatype.h"
#include "disp_debug.h"
#include "hal_disp_util.h"
#include "hal_disp_reg.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "hal_disp.h"
#include "drv_disp_ctx.h"
#include "hal_disp_clk.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    MI_U32 u32Reg;
    MI_U16 u16Mak;
    MI_U16 u16shift;
} HAL_DISP_CLK_SetReg_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MI_U32                      g_u32DispClkNum                           = E_HAL_DISP_CLK_NUM;
MI_U32                      g_u32DispClkDispPllId                     = E_HAL_DISP_CLK_DISPPLL_CLK;
HAL_DISP_CLK_GpCtrlConfig_t g_stDispClkGpCtrlCfg[HAL_DISP_DEVICE_MAX] = {{0, 0}};

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static void _HAL_DISP_CLK_FillRegStruct(HAL_DISP_CLK_SetReg_t *pCfg, MI_U32 u32Reg, MI_U16 u16Mak, MI_U16 u16shift)
{
    pCfg->u32Reg   = u32Reg;
    pCfg->u16shift = u16shift;
    pCfg->u16Mak   = u16Mak;
}
static void _HAL_DISP_CLK_GetVal(MI_U16 u16Val, MI_U8 *pbEn, MI_U32 *pu32ClkRate)
{
    *pbEn        = u16Val & (0x0001) ? 0 : 1;
    *pu32ClkRate = (u16Val >> (2));
}
static MI_U16 _HAL_DISP_CLK_SetVal(MI_U8 bEn, MI_U32 u32ClkRate, MI_U16 u16shift)
{
    MI_U16 u16Val;

    u16Val = bEn ? 0 : (1 << u16shift);
    u16Val |= (u32ClkRate) << (u16shift + 2);
    return u16Val;
}

static MI_U8 HalDispClkSetOdclk(HAL_DISP_CLK_GpCtrlType_e eDevTypeEn, HAL_DISP_CLK_GpCtrlType_e *eDevType, MI_U8 *u8bEn)
{
    MI_U8  u8Ret = 1;
    MI_U8  idx;
    MI_U8  u8Lcdcnt = 0;
    MI_U16 u16Val   = 0;

    for (idx = 0; idx < HAL_DISP_DEVICE_MAX; idx++)
    {
        if (u8bEn[idx] == 1 && eDevType[idx] & E_HAL_DISP_CLK_GP_CTRL_LCD)
        {
            u8Lcdcnt++;
        }
    }
    if (eDevTypeEn & E_HAL_DISP_CLK_GP_CTRL_LCD)
    {
        for (idx = 0; idx < HAL_DISP_DEVICE_MAX; idx++)
        {
            u16Val = u8bEn[idx] ? 0x0000 : 0x0001;
            u16Val |= (HAL_DISP_CLK_ODCLK_SEL_P & 0x0003) << 2;

            HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_SC_GP_CTRL_24_L, u16Val, 0x0F00);
            HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_SC_GP_CTRL_24_L, u16Val, 0xF000);
        }
    }

    return u8Ret;
}

//------------------------------------------------------------------------------
// Global Function
//------------------------------------------------------------------------------
MI_U8 HAL_DISP_CLK_GetClkMuxAttr(MI_U32 u32Idx)
{
    return HAL_DISP_CLK_GET_MUX_ATTR(u32Idx);
}

void HAL_DISP_CLK_GetClkDispPixel0(MI_U32 u32Dev, MI_U8 *pbEn, MI_U32 *pu32ClkRate)
{
    MI_U16 u16Val = 0;

    if (u32Dev == HAL_DISP_DEVICE_ID_0)
    {
        u16Val = HAL_DISP_UTILITY_R2BYTEDirect(REG_CLKGEN_49_L) & 0x3F;
    }

    _HAL_DISP_CLK_GetVal(u16Val, pbEn, pu32ClkRate);
}
void HAL_DISP_CLK_SetClkDispPixel0(MI_U32 u32Dev, MI_U8 bEn, MI_U32 u32ClkRate)
{
    MI_U16                u16Val = 0;
    HAL_DISP_CLK_SetReg_t stCfg;

    if (u32Dev == HAL_DISP_DEVICE_ID_0)
    {
        _HAL_DISP_CLK_FillRegStruct(&stCfg, REG_CLKGEN_49_L, 0x3F, 0);
    }
    else
    {
        DISP_ERR("%s %d, Dev Not Support:%d\n", __FUNCTION__, __LINE__, u32Dev);
        return;
    }
    u16Val = _HAL_DISP_CLK_SetVal(bEn, u32ClkRate, stCfg.u16shift);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(stCfg.u32Reg, u16Val, stCfg.u16Mak);
}

MI_U32 HAL_DISP_CLK_MapBasicClkgenToIdx(HAL_DISP_CLK_Type_e enClkType, MI_U32 u32ClkRate)
{
    MI_U32 u32ClkIdx;
    UNUSED(u32ClkRate);

    switch (enClkType)
    {
        case E_HAL_DISP_CLK_FCLK:
            u32ClkIdx = 0x00;
            break;
        case E_HAL_DISP_CLK_DISP_PIXEL_0:
            u32ClkIdx = 0x09;
            break;
        case E_HAL_DISP_CLK_DISPPLL_CLK:
            u32ClkIdx = 0x00;
            break;
        default:
            DISP_ERR("%s %d, Type Not Support:%d\n", __FUNCTION__, __LINE__, enClkType);
            return 0;
    }
    return u32ClkIdx;
}
MI_U32 HAL_DISP_CLK_MapBasicClkgenToRate(HAL_DISP_CLK_Type_e enClkType, MI_U32 u32ClkRateIdx)
{
    MI_U32 u32ClkRate;

    switch (enClkType)
    {
        case E_HAL_DISP_CLK_FCLK:
            u32ClkRate = u32ClkRateIdx == 0x00   ? CLK_MHZ(288, 0)
                         : u32ClkRateIdx == 0x01 ? CLK_MHZ(192, 0)
                         : u32ClkRateIdx == 0x02 ? CLK_MHZ(144, 0)
                         : u32ClkRateIdx == 0x03 ? CLK_MHZ(320, 0)
                         : u32ClkRateIdx == 0x04 ? CLK_MHZ(240, 0)
                                                 : CLK_MHZ(216, 0);
            break;
        case E_HAL_DISP_CLK_DISP_PIXEL_0:
            u32ClkRate = u32ClkRateIdx == 0x00   ? CLK_MHZ(72, 0)
                         : u32ClkRateIdx == 0x01 ? CLK_MHZ(54, 0)
                         : u32ClkRateIdx == 0x02 ? CLK_MHZ(100, 0)
                                                 : CLK_MHZ(50, 0);
            break;
        case E_HAL_DISP_CLK_DISPPLL_CLK:
            u32ClkRate = u32ClkRateIdx == 0x00 ? CLK_MHZ(148, 500) : CLK_MHZ(148, 500);
            break;
        default:
            DISP_ERR("%s %d, Type Not Support:%d\n", __FUNCTION__, __LINE__, enClkType);
            return 0;
    }
    return u32ClkRate;
}
void HAL_DISP_CLK_GetBasicClkgen(HAL_DISP_CLK_Type_e enClkType, MI_U8 *pbEn, MI_U32 *pu32ClkRate)
{
    MI_U16                u16Val = 0;
    HAL_DISP_CLK_SetReg_t stCfg;

    switch (enClkType)
    {
        case E_HAL_DISP_CLK_FCLK:
            _HAL_DISP_CLK_FillRegStruct(&stCfg, REG_CLKGEN_64_L, 0x001F, 0);
            break;
        case E_HAL_DISP_CLK_DISP_PIXEL_0:
            _HAL_DISP_CLK_FillRegStruct(&stCfg, REG_CLKGEN_49_L, 0x003F, 0);
            break;
        case E_HAL_DISP_CLK_DISPPLL_CLK:
            _HAL_DISP_CLK_FillRegStruct(&stCfg, REG_DISP_LPLL_40_L, 0x8000, 15);
            break;
        default:
            DISP_ERR("%s %d, Type Not Support:%d\n", __FUNCTION__, __LINE__, enClkType);
            return;
    }
    u16Val = HAL_DISP_UTILITY_R2BYTEDirect(stCfg.u32Reg) & stCfg.u16Mak;
    u16Val = u16Val >> stCfg.u16shift;
    _HAL_DISP_CLK_GetVal(u16Val, pbEn, pu32ClkRate);
}

void HAL_DISP_CLK_SetBasicClkgen(HAL_DISP_CLK_Type_e enClkType, MI_U8 bEn, MI_U32 u32ClkRate)
{
    HAL_DISP_CLK_SetReg_t stCfg;
    MI_U16                u16Val = 0;

    switch (enClkType)
    {
        case E_HAL_DISP_CLK_FCLK:
            _HAL_DISP_CLK_FillRegStruct(&stCfg, REG_CLKGEN_64_L, 0x001F, 0);
            break;
        case E_HAL_DISP_CLK_DISP_PIXEL_0:
            _HAL_DISP_CLK_FillRegStruct(&stCfg, REG_CLKGEN_49_L, 0x003F, 0);
            break;
        case E_HAL_DISP_CLK_DISPPLL_CLK:
            _HAL_DISP_CLK_FillRegStruct(&stCfg, REG_DISP_LPLL_40_L, 0x8000, 15);
            break;
        default:
            DISP_ERR("%s %d, Type Not Support:%d\n", __FUNCTION__, __LINE__, enClkType);
            return;
    }
    u16Val = _HAL_DISP_CLK_SetVal(bEn, u32ClkRate, stCfg.u16shift);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(stCfg.u32Reg, u16Val, stCfg.u16Mak);
}

void HAL_DISP_CLK_InitGpCtrlCfg(void *pCtx)
{
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg   = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain   = NULL;
    HAL_DISP_HwContain_t *        pstHalHwContain = NULL;
    MI_U8                         u8Idx = pstDevContain->u32DevId >= HAL_DISP_DEVICE_MAX ? 0 : pstDevContain->u32DevId;

    pstDispCtxCfg   = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain   = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstHalHwContain = pstDispCtxCfg->pstCtxContain->pstHalHwContain;

    if (pstHalHwContain->bClkGpCtrl[pstDevContain->u32DevId] == 0)
    {
        CamOsMemset(&g_stDispClkGpCtrlCfg[u8Idx], 0, sizeof(HAL_DISP_CLK_GpCtrlConfig_t));
        pstDispCtxCfg->pstCtxContain->pstHalHwContain->pvClkGpCtrl[pstDevContain->u32DevId] =
            (void *)&g_stDispClkGpCtrlCfg[pstDevContain->u32DevId];
        pstHalHwContain->bClkGpCtrl[pstDevContain->u32DevId] = 1;
    }
}

void HAL_DISP_CLK_DeInitGpCtrlCfg(void *pCtx)
{
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg   = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain   = NULL;
    HAL_DISP_HwContain_t *        pstHalHwContain = NULL;

    pstDispCtxCfg   = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain   = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstHalHwContain = pstDispCtxCfg->pstCtxContain->pstHalHwContain;

    if (pstHalHwContain->bClkGpCtrl[pstDevContain->u32DevId] == 1)
    {
        pstDispCtxCfg->pstCtxContain->pstHalHwContain->pvClkGpCtrl[pstDevContain->u32DevId] = NULL;
        pstHalHwContain->bClkGpCtrl[pstDevContain->u32DevId]                                = 0;
    }
}

void HAL_DISP_CLK_SetGpCtrlCfg(void *pCtx)
{
    DRV_DISP_CTX_Config_t *      pstDispCtxCfg = NULL;
    HAL_DISP_CLK_GpCtrlConfig_t *pstClkGpCtrlCfg[HAL_DISP_DEVICE_MAX];
    HAL_DISP_CLK_GpCtrlType_e    eDevType[HAL_DISP_DEVICE_MAX];
    HAL_DISP_CLK_GpCtrlType_e    eDevTypeEn = 0;
    MI_U8                        u8bEn[HAL_DISP_DEVICE_MAX];
    MI_U8                        idx;
    HAL_DISP_CLK_GpCtrlConfig_t  stFameClkGpCtrlCfg = {0, 0};

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    for (idx = 0; idx < HAL_DISP_DEVICE_MAX; idx++)
    {
        pstClkGpCtrlCfg[idx] =
            (HAL_DISP_CLK_GpCtrlConfig_t *)pstDispCtxCfg->pstCtxContain->pstHalHwContain->pvClkGpCtrl[idx];
        if (pstClkGpCtrlCfg[idx] == NULL)
        {
            pstClkGpCtrlCfg[idx] = &stFameClkGpCtrlCfg;
        }
        eDevType[idx] = pstClkGpCtrlCfg[idx]->eType;
        u8bEn[idx]    = pstClkGpCtrlCfg[idx]->bEn;
        if (pstClkGpCtrlCfg[idx]->bEn)
        {
            eDevTypeEn |= pstClkGpCtrlCfg[idx]->eType;
        }
    }

    DISP_DBG(DISP_DBG_LEVEL_CLK, "%s %d, Dev0(%d, %s)\n", __FUNCTION__, __LINE__, u8bEn[HAL_DISP_DEVICE_ID_0],
             PARSING_HAL_CLK_GP_CTRL_TYPE(eDevType[HAL_DISP_DEVICE_ID_0]));

    HalDispClkSetOdclk(eDevTypeEn, eDevType, u8bEn);
}

void HAL_DISP_CLK_Init(MI_U8 bEn) {}
