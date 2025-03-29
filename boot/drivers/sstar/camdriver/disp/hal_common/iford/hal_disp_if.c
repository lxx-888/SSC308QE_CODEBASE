/*
 * hal_disp_if.c- Sigmastar
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
#define _HAL_DISP_IF_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mi_disp.h"
#include "mi_disp_impl_datatype.h"
#include "drv_disp_os.h"
#include "disp_debug.h"
#include "hal_disp_include.h"
#include "drv_disp_ctx.h"
#include "drv_disp_if.h"
#include "hal_disp_clk.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MI_U8                           g_bDispHwIfInit = 0;
HAL_DISP_QueryCallBackFunc_t    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_MAX];
MI_DISP_IMPL_MhalDeviceConfig_t g_stInterCfg[HAL_DISP_DEVICE_MAX] = {
    {0, 0, HAL_DISP_COLOR_CSC_ID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

// color & picture
MI_S16 g_stVideoColorCorrectionMatrix[] = {
    0x0400,  0x0000, 0x0000,  0x0000, 0x0400,  0x0000,  0x0000,  0x0000, 0x0400,  -0x034B, 0x0196,
    -0x068B, 0x03C9, -0x0439, 0x0032, -0x0004, -0x07EE, 0x04E7,  0x07CB, -0x04C3, 0x0404,  0x023B,
    -0x023E, 0x01D5, -0x0831, 0x0100, -0x0102, 0x0101,  -0x0101, 0x0000, 0x0000,  0x0000,
};

// lpll settings
MI_U16 g_u16LpllLoopGainDsi[E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX] = {
    16, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ   NO.0
    8,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ      NO.1
    4,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ       NO.2
    2,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ     NO.3
    1,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ    NO.4
    1,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ    NO.5
    1,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ    NO.6
};

MI_U16 g_u16LpllLoopDivDsi[E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX] = {
    1, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ   NO.0
    1, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ      NO.1
    1, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ       NO.2
    1, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ     NO.3
    1, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ    NO.4
    1, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ    NO.5
    1, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ    NO.6
};

MI_U16 g_u16LpllLoopGain[E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX] = {
    16, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ   NO.0
    8,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ      NO.1
    4,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ       NO.2
    2,  // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ     NO.3
    16, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ    NO.4
    16, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ    NO.5
    16, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ    NO.6
};

MI_U16 g_u16LpllLoopDiv[E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX] = {
    8,   // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ   NO.0
    8,   // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ      NO.1
    8,   // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ       NO.2
    8,   // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ     NO.3
    130, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ    NO.4
    260, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ    NO.5
    300, // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ    NO.6
};

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static MI_U8 _HAL_DISP_IF_GetPnlLpllIdx(MI_U32 u32Dclk, MI_U16 *pu16Idx, MI_BOOL bDsi)
{
    MI_U8 bRet = 1;

    if (bDsi)
    {
        if (IS_DATA_LANE_LESS_100M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX;
            bRet     = 0;
        }
        else if (IS_DATA_LANE_BPS_100M_TO_200M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ;
        }
        else if (IS_DATA_LANE_BPS_200M_TO_400M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ;
        }
        else if (IS_DATA_LANE_BPS_400M_TO_800M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ;
        }
        else if (IS_DATA_LANE_BPS_800M_TO_15000M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ;
        }
        else
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX;
            bRet     = 0;
        }
    }
    else
    {
        if (IS_DATA_LANE_BPS_2_8M_TO_3_25M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ;
        }
        else if (IS_DATA_LANE_BPS_3_25M_TO_6_5M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ;
        }
        else if (IS_DATA_LANE_BPS_6_5M_TO_12_5M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ;
        }
        else if (IS_DATA_LANE_BPS_12_5M_TO_25M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ;
        }
        else if (IS_DATA_LANE_BPS_25M_TO_50M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ;
        }
        else if (IS_DATA_LANE_BPS_50M_TO_100M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ;
        }
        else if (IS_DATA_LANE_BPS_100M_TO_187_5M(u32Dclk))
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ;
        }
        else
        {
            *pu16Idx = E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX;
            bRet     = 0;
        }
    }

    return bRet;
    ;
}

static MI_U16 _HAL_DISP_IF_GetPnlLpllGain(MI_U16 u16Idx, MI_BOOL bDsi)
{
    MI_U16 *pu16Tbl = NULL;

    pu16Tbl = bDsi ? g_u16LpllLoopGainDsi : g_u16LpllLoopGain;

    return (u16Idx < E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX) ? pu16Tbl[u16Idx] : 1;
}

static MI_U16 _HAL_DISP_IF_GetPnlLpllDiv(MI_U16 u16Idx, MI_BOOL bDsi)
{
    MI_U16 *pu16Tbl = NULL;

    pu16Tbl = bDsi ? g_u16LpllLoopDivDsi : g_u16LpllLoopDiv;

    return (u16Idx < E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX) ? pu16Tbl[u16Idx] : 1;
}

void HAL_DISP_IF_SetUnifiedLpllConfig(void *pCtx)
{
    DRV_DISP_CTX_Config_t *                   pstDispCtxCfg  = NULL;
    DRV_DISP_CTX_DeviceContain_t *            pstDevContain  = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstUfdParamCfg = NULL;

    MI_U16 u16LpllIdx = 0;
    MI_U16 u16Htotal = 0, u16Vtotal = 0;
    MI_U16 u16LoopGain = 0, u16LoopDiv = 0;
    MI_U32 u32Fps    = 0;
    MI_U32 u32HttVtt = 0, u32Mod = 0;
    MI_U32 u32Dividen = 0, u32Divisor = 0, u32LplLSet = 0;
    MI_U32 u32Dclk, u32DclkHz;
    MI_U16 u16LaneNum, u16BitPerPixel;
    MI_U8  bDsi = 0;

    pstDispCtxCfg  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain  = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstUfdParamCfg = &pstDevContain->stPnlUdParamCfg;

    if (pstDevContain->u32Interface == HAL_DISP_INTF_MIPIDSI)
    {
        u16LaneNum = pstUfdParamCfg->stMpdInfo.enLaneNum == E_MI_DISP_MHALPNL_MIPI_DSI_LANE_4   ? 4
                     : pstUfdParamCfg->stMpdInfo.enLaneNum == E_MI_DISP_MHALPNL_MIPI_DSI_LANE_3 ? 3
                     : pstUfdParamCfg->stMpdInfo.enLaneNum == E_MI_DISP_MHALPNL_MIPI_DSI_LANE_2 ? 2
                                                                                                : 1;

        u16BitPerPixel = pstUfdParamCfg->stMpdInfo.enFormat == E_MI_DISP_MHALPNL_MIPI_DSI_RGB888   ? 24
                         : pstUfdParamCfg->stMpdInfo.enFormat == E_MI_DISP_MHALPNL_MIPI_DSI_RGB565 ? 16
                                                                                                   : 18;

        u16Htotal = pstUfdParamCfg->stMpdInfo.u16Hpw + pstUfdParamCfg->stMpdInfo.u16Hbp
                    + pstUfdParamCfg->stMpdInfo.u16Hfp + pstUfdParamCfg->stMpdInfo.u16Hactive;
        u16Vtotal = pstUfdParamCfg->stMpdInfo.u16Vpw + pstUfdParamCfg->stMpdInfo.u16Vbp
                    + pstUfdParamCfg->stMpdInfo.u16Vfp + pstUfdParamCfg->stMpdInfo.u16Vactive;
        ;

        DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, H(%d %d %d %d %d) V(%d %d %d %d %d) FPS:%d,\n", __FUNCTION__, __LINE__,
                 u16Htotal, pstUfdParamCfg->stMpdInfo.u16Hfp, pstUfdParamCfg->stMpdInfo.u16Hpw,
                 pstUfdParamCfg->stMpdInfo.u16Hbp, pstUfdParamCfg->stMpdInfo.u16Hactive, u16Vtotal,
                 pstUfdParamCfg->stMpdInfo.u16Vfp, pstUfdParamCfg->stMpdInfo.u16Vpw, pstUfdParamCfg->stMpdInfo.u16Vbp,
                 pstUfdParamCfg->stMpdInfo.u16Vactive, pstUfdParamCfg->stMpdInfo.u16Fps);

        u32DclkHz = ((MI_U32)u16Htotal * (MI_U32)u16Vtotal * (MI_U32)pstUfdParamCfg->stMpdInfo.u16Fps);
        u32Fps    = pstUfdParamCfg->stMpdInfo.u16Fps;
        u32HttVtt = (MI_U32)(u16Vtotal * u16Htotal);
    }
    else
    {
        u16LaneNum     = 1;
        u16BitPerPixel = 1;
        u16Htotal      = pstUfdParamCfg->stTgnTimingInfo.u16HTotal;
        u16Vtotal      = pstUfdParamCfg->stTgnTimingInfo.u16VTotal;
        u32DclkHz      = pstUfdParamCfg->stTgnTimingInfo.u32Dclk;

        u32HttVtt = (MI_U32)(u16Vtotal * u16Htotal);
        u32Mod    = u32DclkHz % u32HttVtt;

        if (u32Mod > (u32HttVtt / 2))
        {
            u32Fps = (u32DclkHz + u32HttVtt - 1) / u32HttVtt;
        }
        else
        {
            u32Fps = u32DclkHz / u32HttVtt;
        }
    }

    DISP_DBG_VAL(u32Fps);
    bDsi = (pstDevContain->u32Interface == HAL_DISP_INTF_MIPIDSI) ? 1 : 0;

    u32Dclk = u32DclkHz * (MI_U32)u16BitPerPixel / (MI_U32)u16LaneNum;

    DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, DclkHz:%ld, Lane:%d, BPS:%d\n", __FUNCTION__, __LINE__, u32DclkHz,
             u16LaneNum, u16BitPerPixel);

    if (_HAL_DISP_IF_GetPnlLpllIdx(u32Dclk, &u16LpllIdx, bDsi))
    {
        if (u16LpllIdx != E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX)
        {
            u16LoopDiv  = _HAL_DISP_IF_GetPnlLpllDiv(u16LpllIdx, bDsi);
            u16LoopGain = _HAL_DISP_IF_GetPnlLpllGain(u16LpllIdx, bDsi);

            u32Dividen = ((MI_U32)432 * (MI_U32)524288 * (MI_U32)u16LoopGain);
            u32Divisor = u32Dclk * (MI_U32)u16LoopDiv / 1000000;
            u32LplLSet = u32Dividen / u32Divisor;

            DISP_DBG(DISP_DBG_LEVEL_HAL_LCD,
                     "%s %d, Idx:%d, LoopGain:%d, LoopDiv:%d, fps=%ld, dclk=%ld, Divden:0x%lx, Divisor:0x%lx, "
                     "LpllSe:0x%lx\n",
                     __FUNCTION__, __LINE__, u16LpllIdx, u16LoopGain, u16LoopDiv, u32Fps, u32Dclk, u32Dividen,
                     u32Divisor, u32LplLSet);

            DISP_EXEC_LCD(HAL_DISP_LpllSetTbl(u16LpllIdx, u32LplLSet, bDsi));
        }
    }
    else
    {
        DISP_ERR("%s %d, DCLK out of range: %ld\n", __FUNCTION__, __LINE__, u32Dclk);
    }
}

static void _HAL_DISP_IF_SetTgenResetSt(DRV_DISP_CTX_Config_t *         pstDispCtxCfg,
                                        HAL_DISP_ST_DeviceTimingInfo_t *pstDeviceTimingCfg)
{
    DRV_DISP_CTX_DeviceContain_t *pstDevContain   = NULL;
    MI_U16                        u16TimeGenStart = 0;
    UNUSED(pstDeviceTimingCfg);

    pstDevContain   = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    u16TimeGenStart = HAL_DISP_GetDeviceTimeGenStart((void *)pstDispCtxCfg, pstDevContain->u32DevId);

    if (u16TimeGenStart)
    {
        HAL_DISP_SetSwReset(1, (void *)pstDispCtxCfg, pstDevContain->u32DevId);
    }
}
static void _HAL_DISP_IF_SetTgenResetEnd(DRV_DISP_CTX_Config_t *         pstDispCtxCfg,
                                         HAL_DISP_ST_DeviceTimingInfo_t *pstDeviceTimingCfg)
{
    DRV_DISP_CTX_DeviceContain_t *  pstDevContain   = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *pCmdqCtx        = NULL;
    MI_U16                          u16TimeGenStart = 0;
    UNUSED(pstDeviceTimingCfg);

    pstDevContain   = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    u16TimeGenStart = HAL_DISP_GetDeviceTimeGenStart((void *)pstDispCtxCfg, pstDevContain->u32DevId);
    HAL_DISP_GetCmdqByCtx((void *)pstDispCtxCfg, (void *)&pCmdqCtx);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ContainId:%d, DevId:%d, Interface:%s(%x)\n", __FUNCTION__, __LINE__,
             pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId, PARSING_HAL_INTERFACE(pstDevContain->u32Interface),
             pstDevContain->u32Interface);

    DISP_STATISTIC_VAL(g_stDispIrqHist.stWorkingHist.stDevHist[pstDevContain->u32DevId].u32ChangeTimingCnt++);
    DISP_STATISTIC_VAL(g_stDispIrqHist.stWorkingHist.stDevHist[pstDevContain->u32DevId].u64ChangeTimingTimeStamp =
                           DRV_DISP_OS_GetSystemTimeStamp());
    if (u16TimeGenStart)
    {
        HAL_DISP_SetSwReset(0, (void *)pstDispCtxCfg, pstDevContain->u32DevId);
    }
}

static void _HAL_DISP_IF_SetTgenFwVtt(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    HAL_DISP_SetTgenFwVtt(1, pstDispCtxCfg);
}

static void _HAL_DISP_IF_SetRegFlipPreAct(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    DRV_DISP_CTX_DeviceContain_t *  pstDevContain = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx    = NULL;
    MI_U16                          u16CmdqInProcess;
    MI_U16                          u16TimeGenStart = 0;

    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    HAL_DISP_GetCmdqByCtx((void *)pstDispCtxCfg, (void *)&pstCmdqCtx);

    u16CmdqInProcess = HAL_DISP_UTILITY_R2BYTEMaskDirect(
        REG_HAL_DISP_UTILIYT_CMDQDEV_IN_PROCESS(pstCmdqCtx->s32UtilityId), REG_CMDQ_IN_PROCESS_MSK);
    u16TimeGenStart = HAL_DISP_GetDeviceTimeGenStart(pstDispCtxCfg, pstDevContain->u32DevId);
    // Add Wait Event if cmdq is idle
    HAL_DISP_UTILITY_CNT_ADD(pstCmdqCtx->u16RegFlipCnt, 1);
    HAL_DISP_UTILITY_W2BYTEMSK(REG_HAL_DISP_UTILIYT_CMDQDEV_IN_PROCESS(pstCmdqCtx->s32UtilityId),
                               REG_CMDQ_IN_PROCESS_ON | ((pstCmdqCtx->u16RegFlipCnt << 1) & REG_CMDQ_PROCESS_FENCE_MSK),
                               REG_CMDQ_IN_PROCESS_MSK | REG_CMDQ_PROCESS_FENCE_MSK, (void *)pstCmdqCtx);

    if (u16CmdqInProcess == 0 && u16TimeGenStart)
    {
        // Polling De Active in first cmd.
        // HAL_DISP_UTILITY_PollWait(pstCmdqCtx->pvCmdqInf, HAL_DISP_UTILITY_SC_TO_CMDQDEV_FLAG,
        // HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVDE_ON_BIT, HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVDE_ON_MSK, 2000000000, 1);
    }
}

static void _HAL_DISP_IF_SetRegFlipPostAct(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = NULL;

    HAL_DISP_GetCmdqByCtx((void *)pstDispCtxCfg, (void *)&pstCmdqCtx);
    HAL_DISP_UTILITY_W2BYTEMSKDirectCmdqWrite((void *)pstCmdqCtx);
}

static void _HAL_DISP_IF_SetWaitDonePreAct(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = NULL;

    HAL_DISP_GetCmdqByCtx(pstDispCtxCfg, (void *)&pstCmdqCtx);
    HAL_DISP_SetCmdqIntClear(HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_MSK, pstDispCtxCfg);
    HAL_DISP_SetCmdqIntMask(~HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_MSK, pstDispCtxCfg);
    HAL_DISP_UTILITY_W2BYTEMSKDirectCmdqWrite((void *)pstCmdqCtx);
}

static void _HAL_DISP_IF_SetWaitDonePostAct(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = NULL;

    HAL_DISP_GetCmdqByCtx(pstDispCtxCfg, (void *)&pstCmdqCtx);
    HAL_DISP_SetCmdqIntClear(HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_MSK, pstDispCtxCfg);
    HAL_DISP_UTILITY_CNT_ADD(pstCmdqCtx->u16WaitDoneCnt, 1);
    HAL_DISP_UTILITY_W2BYTEMSK(REG_HAL_DISP_UTILITY_CMDQ_WAIT_CNT(pstCmdqCtx->s32UtilityId),
                               (pstCmdqCtx->u16WaitDoneCnt << REG_CMDQ_WAIT_CNT_SHIFT), REG_CMDQ_WAIT_CNT_MSK,
                               (void *)pstCmdqCtx);
    HAL_DISP_UTILITY_W2BYTEMSK(REG_HAL_DISP_UTILIYT_CMDQDEV_IN_PROCESS(pstCmdqCtx->s32UtilityId),
                               REG_CMDQ_IN_PROCESS_OFF, REG_CMDQ_IN_PROCESS_MSK, (void *)pstCmdqCtx);
    HAL_DISP_UTILITY_W2BYTEMSKDirectCmdqWrite((void *)pstCmdqCtx);
}

static void _HAL_DISP_IF_SetTgenConfig(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain      = NULL;
    HAL_DISP_ST_DeviceTimingConfig_t *pstDeviceTimingCfg = NULL;
    MI_U16                            u16HsyncSt, u16VsyncSt;
    MI_U16                            u16HdeSt, u16VdeSt;

    pstDevContain      = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstDeviceTimingCfg = &pstDevContain->stDevTimingCfg;

    u16HdeSt = pstDeviceTimingCfg->u16Hstart;
    u16VdeSt = pstDeviceTimingCfg->u16Vstart;

    u16HsyncSt =
        (u16HdeSt)
            ? pstDeviceTimingCfg->u16Hstart - pstDeviceTimingCfg->u16HsyncWidth - pstDeviceTimingCfg->u16HsyncBackPorch
            : pstDeviceTimingCfg->u16Htotal - pstDeviceTimingCfg->u16HsyncWidth - pstDeviceTimingCfg->u16HsyncBackPorch;

    u16VsyncSt =
        (u16VdeSt)
            ? pstDeviceTimingCfg->u16Vstart - pstDeviceTimingCfg->u16VsyncWidth - pstDeviceTimingCfg->u16VsyncBackPorch
            : pstDeviceTimingCfg->u16Vtotal - pstDeviceTimingCfg->u16VsyncWidth - pstDeviceTimingCfg->u16VsyncBackPorch;

    DISP_DBG(DISP_DBG_LEVEL_HAL,
             "%s %d, Hsync(%d %d), Vsync(%d %d) SyncStart(%d %d), DeStart(%d, %d) Size(%d %d), Total(%d %d)\n",
             __FUNCTION__, __LINE__, pstDeviceTimingCfg->u16HsyncWidth, pstDeviceTimingCfg->u16HsyncBackPorch,
             pstDeviceTimingCfg->u16VsyncWidth, pstDeviceTimingCfg->u16VsyncBackPorch, u16HsyncSt, u16VsyncSt, u16HdeSt,
             u16VdeSt, pstDeviceTimingCfg->u16Hactive, pstDeviceTimingCfg->u16Vactive, pstDeviceTimingCfg->u16Htotal,
             pstDeviceTimingCfg->u16Vtotal);

    // Vertical
    HAL_DISP_SetTgenVsyncSt(u16VsyncSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenVsyncEnd(u16VsyncSt + pstDeviceTimingCfg->u16VsyncWidth - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenVfdeSt(u16VdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenVfdeEnd(u16VdeSt + pstDeviceTimingCfg->u16Vactive - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenVdeSt(u16VdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenVdeEnd(u16VdeSt + pstDeviceTimingCfg->u16Vactive - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenVtt(pstDeviceTimingCfg->u16Vtotal - 1, (void *)pstDispCtxCfg);

    // horizontal
    if (pstDevContain->u32Interface == HAL_DISP_INTF_BT656)
    {
        // BT656,BT1120 Timing:
        //  | hfp | hsync_width | hbp | h_acitve |
        //  | -->    Blanking     <-- |  active  |
        //  | -->    De invalid   <-- | De valid |
        //  H_Blank = hsync_width + hbp + hfp = htt - h_acitve;
        HAL_DISP_SetTgenHsyncSt(0, (void *)pstDispCtxCfg); // Blanking start
        HAL_DISP_SetTgenHsyncEnd(pstDeviceTimingCfg->u16Htotal - pstDeviceTimingCfg->u16Hactive - 1,
                                 (void *)pstDispCtxCfg); // Blanking end
        HAL_DISP_SetTgenHfdeSt(pstDeviceTimingCfg->u16Htotal - pstDeviceTimingCfg->u16Hactive, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHfdeEnd(pstDeviceTimingCfg->u16Htotal - 1, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHdeSt(pstDeviceTimingCfg->u16Htotal - pstDeviceTimingCfg->u16Hactive, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHdeEnd(pstDeviceTimingCfg->u16Htotal - 1, (void *)pstDispCtxCfg);
    }
    else
    {
        HAL_DISP_SetTgenHsyncSt(u16HsyncSt, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHsyncEnd(u16HsyncSt + pstDeviceTimingCfg->u16HsyncWidth - 1, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHfdeSt(u16HdeSt, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHfdeEnd(u16HdeSt + pstDeviceTimingCfg->u16Hactive - 1, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHdeSt(u16HdeSt, (void *)pstDispCtxCfg);
        HAL_DISP_SetTgenHdeEnd(u16HdeSt + pstDeviceTimingCfg->u16Hactive - 1, (void *)pstDispCtxCfg);
    }

    HAL_DISP_SetTgenHtt(pstDeviceTimingCfg->u16Htotal - 1, (void *)pstDispCtxCfg);
}

static void _HAL_DISP_IF_SetRdmaPatTg(DRV_DISP_CTX_Config_t *pstDispCtxCfg)
{
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain      = NULL;
    HAL_DISP_ST_DeviceTimingConfig_t *pstDeviceTimingCfg = NULL;
    MI_U16                            u16HsyncSt, u16VsyncSt;
    MI_U16                            u16HdeSt, u16VdeSt;

    pstDevContain      = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstDeviceTimingCfg = &pstDevContain->stDevTimingCfg;

    u16HdeSt = pstDeviceTimingCfg->u16Hstart;
    u16VdeSt = pstDeviceTimingCfg->u16Vstart - 1;

    u16HsyncSt =
        (u16HdeSt)
            ? pstDeviceTimingCfg->u16Hstart - pstDeviceTimingCfg->u16HsyncWidth - pstDeviceTimingCfg->u16HsyncBackPorch
            : pstDeviceTimingCfg->u16Htotal - pstDeviceTimingCfg->u16HsyncWidth - pstDeviceTimingCfg->u16HsyncBackPorch;

    u16VsyncSt =
        (u16VdeSt)
            ? pstDeviceTimingCfg->u16Vstart - pstDeviceTimingCfg->u16VsyncWidth - pstDeviceTimingCfg->u16VsyncBackPorch
            : pstDeviceTimingCfg->u16Vtotal - pstDeviceTimingCfg->u16VsyncWidth - pstDeviceTimingCfg->u16VsyncBackPorch;

    // Vertical
    HAL_DISP_SetRdmaPatTgVsyncSt(u16VsyncSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgVsyncEnd(u16VsyncSt + pstDeviceTimingCfg->u16VsyncWidth - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgVfdeSt(u16VdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgVfdeEnd(u16VdeSt + pstDeviceTimingCfg->u16Vactive - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgVdeSt(u16VdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgVdeEnd(u16VdeSt + pstDeviceTimingCfg->u16Vactive - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgVtt(pstDeviceTimingCfg->u16Vtotal - 1, (void *)pstDispCtxCfg);

    // horizontal
    HAL_DISP_SetRdmaPatTgHsyncSt(u16HsyncSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgHsyncEnd(u16HsyncSt + pstDeviceTimingCfg->u16HsyncWidth - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgHfdeSt(u16HdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgHfdeEnd(u16HdeSt + pstDeviceTimingCfg->u16Hactive - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgHdeSt(u16HdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgHdeEnd(u16HdeSt + pstDeviceTimingCfg->u16Hactive - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetRdmaPatTgHtt(pstDeviceTimingCfg->u16Htotal * 2 - 1, (void *)pstDispCtxCfg);
}

static void _HAL_DISP_IF_SetInitWithoutCmdq(DRV_DISP_CTX_Config_t *pstDispCtx)
{
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    MI_U32                        u32OrigAccessMode;
    MI_S32                        s32UtilityId;

    pstDevContain = pstDispCtx->pstCtxContain->pstDevContain[pstDispCtx->u32ContainIdx];
    if (HAL_DISP_GetUtilityIdByDevId(pstDevContain->u32DevId, &s32UtilityId)
        && ((s32UtilityId >= 0) && (s32UtilityId < HAL_DISP_UTILITY_CMDQ_NUM)))
    {
        u32OrigAccessMode = HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)s32UtilityId);
        HAL_DISP_UTILITY_SetRegAccessMode(s32UtilityId, E_MI_DISP_REG_ACCESS_CPU);
        HAL_DISP_SetCmdqIntMask(~HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_MSK, (void *)pstDispCtx); //
        //        HAL_DISP_SetSwReset(1, NULL, pstDevContain->u32DevId);
        HAL_DISP_IRQ_SetDevIntClr(pstDevContain->u32DevId, 0xFC00);
        HAL_DISP_SetTimeGenStartFlag(0, NULL, pstDevContain->u32DevId);
        HAL_DISP_UTILITY_SetRegAccessMode((MI_U32)s32UtilityId, u32OrigAccessMode);
        g_stInterCfg[pstDevContain->u32DevId].bDispPat                                    = 0;
        g_stInterCfg[pstDevContain->u32DevId].bRstDisp                                    = 1;
        g_stDispIrqHist.stWorkingStatus.stDevStatus[pstDevContain->u32DevId].u8Deviceused = 0;
    }
    else
    {
        DISP_ERR("%s %d, GetCmdqIdByDevId Fail\n", __FUNCTION__, __LINE__);
    }
}

static void _HAL_DISP_IF_SetCscToDefualt(MI_U32 u32ColorId, void *pCtx)
{
    HAL_DISP_COLOR_SeletYuvToRgbMatrix(u32ColorId, E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS, NULL, pCtx);
    HAL_DISP_COLOR_AdjustVideoRGB(u32ColorId, 0x80, 0x80, 0x80, pCtx);
    HAL_DISP_COLOR_AdjustHCS(u32ColorId, 50, 0x80, 0x80, pCtx);
    HAL_DISP_COLOR_AdjustBrightness(u32ColorId, 0x80, 0x80, 0x80, pCtx);
}
static void _HAL_DISP_IF_SetCscToInit(MI_U32 u32ColorId, MI_DISP_Csc_t *pstCsc, void *pCtx)
{
    HAL_DISP_PICTURE_Config_t stPictureCfg;

    HAL_DISP_COLOR_SeletYuvToRgbMatrix(u32ColorId, E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV, NULL, pCtx);
    HAL_DISP_PICTURE_TransNonLinear(pCtx, pstCsc, NULL, &stPictureCfg);

    HAL_DISP_COLOR_AdjustHCS(u32ColorId, stPictureCfg.u16Hue, stPictureCfg.u16Saturation, stPictureCfg.u16Contrast,
                             pCtx);

    HAL_DISP_COLOR_AdjustBrightness(u32ColorId, stPictureCfg.u16BrightnessR, stPictureCfg.u16BrightnessG,
                                    stPictureCfg.u16BrightnessB, pCtx);
}
static void _HAL_DISP_IF_SetInterfaceCsc(void *pCtx, MI_U32 u32DevId, MI_DISP_CscMatrix_e enMatrix)
{
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = pCtx;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s,Dev:%d pCtx:%px Matrix:%d ", __FUNCTION__, u32DevId, pCtx, enMatrix);
    if (!(pstDispCtxCfg) || !(pstDispCtxCfg->pstCtxContain))
    {
        DRV_DISP_CTX_GetDeviceCurCtx(u32DevId, &pstDispCtxCfg);
    }
    if (pstDispCtxCfg && pstDispCtxCfg->pstCtxContain)
    {
        if (pstDispCtxCfg->pstCtxContain->bDevContainUsed[pstDispCtxCfg->u32ContainIdx])
        {
            pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
        }
        if (pstDevContain)
        {
            DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s,Dev:%d Interface:%d Matrix:%d ", __FUNCTION__, u32DevId,
                     pstDevContain->u32Interface, enMatrix);
            if (DISP_OUTDEV_IS_PNL(pstDevContain->u32Interface))
            {
                pstDevContain->stDeviceParam.stCsc.eCscMatrix = enMatrix;
            }
        }
    }
}

static void _HAL_DISP_IF_ClkGet(HAL_DISP_CLK_Type_e enType, HAL_DISP_ST_ClkConfig_t *pstClkCfg)
{
    MI_U8 * p8En                             = NULL;
    MI_U32 *p32Rate                          = NULL;
    MI_U8   u8ClkMuxAttr[E_HAL_DISP_CLK_NUM] = HAL_DISP_CLK_MUX_ATTR;

    if (pstClkCfg)
    {
        p8En    = &pstClkCfg->bEn[enType];
        p32Rate = &pstClkCfg->u32Rate[enType];
    }
    else
    {
        DISP_ERR("%s %d, pstClkCfg is NULL ,enType:%d\n", __FUNCTION__, __LINE__, enType);
        return;
    }
    HAL_DISP_CLK_GetBasicClkgen(enType, p8En, p32Rate);
    if (u8ClkMuxAttr[enType] == 0)
    {
        *p32Rate = HAL_DISP_CLK_MapBasicClkgenToRate(enType, *p32Rate);
    }
}
static void _HAL_DISP_IF_ClkSet(HAL_DISP_CLK_Type_e enType, HAL_DISP_ST_ClkConfig_t *pstClkCfg)
{
    MI_U8  u8ClkMuxAttr[E_HAL_DISP_CLK_NUM] = HAL_DISP_CLK_MUX_ATTR;
    MI_U32 u32ClkIdx                        = 0;
    MI_U8  u8En                             = 0;

    if (pstClkCfg)
    {
        u8En      = pstClkCfg->bEn[enType];
        u32ClkIdx = u8ClkMuxAttr[enType] ? pstClkCfg->u32Rate[enType]
                                         : HAL_DISP_CLK_MapBasicClkgenToIdx(enType, pstClkCfg->u32Rate[enType]);
    }
    HAL_DISP_CLK_SetBasicClkgen(enType, u8En, u32ClkIdx);
}
static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceInit(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e        enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    void *                        pvCmdqCtx     = NULL;
    MI_S32                        u32UtilityId;
    UNUSED(pCfg);

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];

    if (HAL_DISP_GetUtilityIdByDevId(pstDevContain->u32DevId, &u32UtilityId) == 0)
    {
        DISP_ERR("%s %d, UnSupport Cmdq Id\n", __FUNCTION__, __LINE__);
    }
    else
    {
        if (HAL_DISP_UTILITY_Init((MI_U32)u32UtilityId) == 0)
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, ContainId:%d, DevId:%d, Utility Init Fail\n", __FUNCTION__, __LINE__,
                     pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId);
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ContainId:%d, DevId:%d\n", __FUNCTION__, __LINE__,
                     pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId);

            if (HAL_DISP_UTILITY_GetCmdqContext(&pvCmdqCtx, u32UtilityId) == 0)
            {
                enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
                DISP_ERR("%s %d, Get CmdqContext Fail\n", __FUNCTION__, __LINE__);
            }
            else
            {
                if (pvCmdqCtx)
                {
                    pstDispCtxCfg->pstCtxContain->pstHalHwContain->pvCmdqCtx[u32UtilityId] = pvCmdqCtx;

                    pstDevContain->stDeviceParam.u32Gain             = 50;
                    pstDevContain->stDeviceParam.stCsc.u32Contrast   = 50;
                    pstDevContain->stDeviceParam.stCsc.u32Hue        = 50;
                    pstDevContain->stDeviceParam.stCsc.u32Saturation = 50;
                    pstDevContain->stDeviceParam.stCsc.u32Luma       = 50;
                    pstDevContain->stDeviceParam.stCsc.eCscMatrix    = E_MI_DISP_CSC_MATRIX_BT709_TO_RGB_PC;
                    pstDevContain->stDeviceParam.u32Sharpness        = 50;

                    pstDevContain->u32BgColor = 0;
                    HAL_DISP_CLK_InitGpCtrlCfg(pCtx);
                }
                else
                {
                    enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
                    DISP_ERR("%s %d, Get CmdqContext Fail\n", __FUNCTION__, __LINE__);
                }
            }
        }
    }

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceInit(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *         pstDispCtx    = NULL;
    DRV_DISP_CTX_DeviceContain_t *  pstDevContain = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx    = NULL;
    UNUSED(pCfg);

    pstDispCtx    = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtx->pstCtxContain->pstDevContain[pstDispCtx->u32ContainIdx];
    HAL_DISP_GetCmdqByCtx(pCtx, (void *)&pstCmdqCtx);

    if (pstCmdqCtx)
    {
        // Init Setting
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Dev:%d, ConId:%d\n", __FUNCTION__, __LINE__, pstDevContain->u32DevId,
                 pstDispCtx->u32ContainIdx);
        _HAL_DISP_IF_SetInitWithoutCmdq(pstDispCtx);
        HAL_DISP_SetDispMux(pCtx);
        HAL_DISP_SetFrameColor(0x00, 0x00, 0x00, pCtx);

        HAL_DISP_COLOR_InitVar(pCtx);
        HAL_DISP_COLOR_SetColorCorrectMatrix(E_HAL_DISP_COLOR_CSC_ID_0, g_stVideoColorCorrectionMatrix, pCtx);
        _HAL_DISP_IF_SetCscToDefualt(E_HAL_DISP_COLOR_CSC_ID_0, pCtx);
        g_stInterCfg[pstDevContain->u32DevId].bCscEn  = 1;
        g_stInterCfg[pstDevContain->u32DevId].u8CscMd = (MI_U8)E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV;
        if (HAL_DISP_PICTURE_IsPqUpdate(pstDevContain->u32DevId))
        {
            HAL_DISP_PICTURE_ApplyPqConfig(pCtx);
        }
        else
        {
            _HAL_DISP_IF_SetCscToInit(g_stInterCfg[pstDevContain->u32DevId].u8ColorId,
                                      &pstDevContain->stDeviceParam.stCsc, pCtx);
        }
        HAL_DISP_SetFixedCsc(pCtx);
    }
    else
    {
        DISP_ERR("%s %d, Null Cmdq Ctx\n", __FUNCTION__, __LINE__);
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceDeInit(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e        enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    MI_S32                        s32UtilityId;
    UNUSED(pCfg);

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];

    HAL_DISP_CLK_DeInitGpCtrlCfg(pCtx);

    if (HAL_DISP_GetUtilityIdByDevId(pstDevContain->u32DevId, &s32UtilityId))
    {
        if (HAL_DISP_UTILITY_DeInit(s32UtilityId) == 0)
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, ContainId:%d, DevId:%d, Utility DeInit Fail\n", __FUNCTION__, __LINE__,
                     pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId);
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ContainId:%d, DevId:%d\n", __FUNCTION__, __LINE__,
                     pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId);

            pstDispCtxCfg->pstCtxContain->pstHalHwContain->pvCmdqCtx[s32UtilityId] = NULL;
        }
    }
    else
    {
        DISP_ERR("%s %d, Get Cmdq Id Fail\n", __FUNCTION__, __LINE__);
    }
    return enRet;
}

static void _HAL_DISP_IF_SetDeviceDeInit(void *pCtx, void *pCfg)
{
    MI_U32 u32DevId = 0;
    UNUSED(pCtx);
    UNUSED(pCfg);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d\n", __FUNCTION__, __LINE__);
    for (u32DevId = 0; u32DevId < HAL_DISP_DEVICE_MAX; u32DevId++)
    {
        if (g_stDispIrqHist.stWorkingStatus.stDevStatus[u32DevId].u8Deviceused)
        {
            HAL_DISP_SetSwReset(1, NULL, u32DevId);
            g_stDispIrqHist.stWorkingStatus.stDevStatus[u32DevId].u8Deviceused = 0;
        }
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceEnable(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e        enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = NULL;
    MI_U8 *                       pbEn;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pbEn          = (MI_U8 *)pCfg;

    pstDevContain->bEnable = *pbEn;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ContainId:%d, DevId:%d, Enable:%x\n", __FUNCTION__, __LINE__,
             pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId, pstDevContain->bEnable);

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceEnable(void *pCtx, void *pCfg)
{
    MI_U8                         bEn;
    DRV_DISP_CTX_Config_t *       pstDispCtx    = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    pstDispCtx    = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtx->pstCtxContain->pstDevContain[pstDispCtx->u32ContainIdx];
    bEn           = *(MI_U8 *)pCfg;

    g_stInterCfg[pstDevContain->u32DevId].bRstDisp                                    = bEn ? 0 : 1;
    g_stDispIrqHist.stWorkingStatus.stDevStatus[pstDevContain->u32DevId].u8Deviceused = bEn;
    // if (bEn == 0)
    //{
    // HAL_DISP_SetTimeGenStartFlag(0, pCtx, pstDevContain->u32DevId);
    //}

    HAL_DISP_SetTgenModeEn();
    HAL_DISP_SetTimeGenStartFlag(bEn, pCtx, pstDevContain->u32DevId);
    HAL_DISP_SetSwReset(1, pCtx, pstDevContain->u32DevId);

    HAL_DISP_SetSwReset(bEn ? 0 : 1, pCtx, pstDevContain->u32DevId);
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceBackGroundColor(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e        enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = NULL;
    MI_U32 *                      pu32BgColor;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pu32BgColor   = (MI_U32 *)pCfg;

    pstDevContain->u32BgColor = *pu32BgColor;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ContainId:%d, DevId:%d, BgColor:%x\n", __FUNCTION__, __LINE__,
             pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId, pstDevContain->u32BgColor);

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceBackGroundColor(void *pCtx, void *pCfg)
{
    MI_U32 *pu32BgColor;
    MI_U8   u8R, u8G, u8B;

    pu32BgColor = (MI_U32 *)pCfg;

    u8R = (*pu32BgColor & 0x000000FF);
    u8G = (*pu32BgColor & 0x0000FF00) >> (8);
    u8B = (*pu32BgColor & 0x00FF0000) >> (16);
    HAL_DISP_SetFrameColor(u8R, u8G, u8B, pCtx);
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceInterface(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e        enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = NULL;
    MI_U32 *                      pu32Interface;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pu32Interface = (MI_U32 *)pCfg;

    if (*pu32Interface & HAL_DISP_NOTSUPPORT_INTF)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, ContainId:%d, DevId:%d, In Intf:%s(%x) Not Supported\n", __FUNCTION__, __LINE__,
                 pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId, PARSING_HAL_INTERFACE(*pu32Interface),
                 *pu32Interface);
    }
    else
    {
        MI_U8 i, u8Cnt;

        u8Cnt = 0;
        for (i = 0; i < 32; i++)
        {
            u8Cnt = (*pu32Interface & (1 << i)) ? u8Cnt + 1 : u8Cnt;
        }

        if (u8Cnt > 1)
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, ContainId:%d, DevId:%d, In Intf:%s(%x) Not Supported\n", __FUNCTION__, __LINE__,
                     pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId, PARSING_HAL_INTERFACE(*pu32Interface),
                     *pu32Interface);
        }
        else
        {
            pstDevContain->u32Interface = *pu32Interface;
            DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ContainId:%d, DevId:%d, In Intf:%s(%x), Contain Intf:%s(%x)\n",
                     __FUNCTION__, __LINE__, pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId,
                     PARSING_HAL_INTERFACE(*pu32Interface), *pu32Interface,
                     PARSING_HAL_INTERFACE(pstDevContain->u32Interface), pstDevContain->u32Interface);
        }
    }

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceInterface(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg   = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain   = NULL;
    HAL_DISP_CLK_GpCtrlConfig_t * pstClkGpCtrlCfg = NULL;
    UNUSED(pCfg);

    pstDispCtxCfg   = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain   = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstClkGpCtrlCfg = (HAL_DISP_CLK_GpCtrlConfig_t *)
                          pstDispCtxCfg->pstCtxContain->pstHalHwContain->pvClkGpCtrl[pstDevContain->u32DevId];

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Dev:%d, ConId:%d If:%s\n", __FUNCTION__, __LINE__, pstDevContain->u32DevId,
             pstDispCtxCfg->u32ContainIdx, PARSING_HAL_INTERFACE(pstDevContain->u32Interface));

    if (pstDevContain->u32Interface == HAL_DISP_INTF_TTL || pstDevContain->u32Interface == HAL_DISP_INTF_BT656)
    {
        pstClkGpCtrlCfg->bEn   = 1;
        pstClkGpCtrlCfg->eType = E_HAL_DISP_CLK_GP_CTRL_LCD;
    }
    else
    {
        pstClkGpCtrlCfg->bEn   = 0;
        pstClkGpCtrlCfg->eType = E_HAL_DISP_CLK_GP_CTRL_NONE;
    }
    HAL_DISP_CLK_SetGpCtrlCfg(pCtx);
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceOutputTiming(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e          enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *         pstDispCtxCfg = NULL;
    HAL_DISP_ST_DeviceTimingInfo_t *pstDeviceTimingCfg;
    DRV_DISP_CTX_DeviceContain_t *  pstDevContain = NULL;

    pstDispCtxCfg      = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain      = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstDeviceTimingCfg = (HAL_DISP_ST_DeviceTimingInfo_t *)pCfg;

    pstDevContain->eTimeType = pstDeviceTimingCfg->eTimeType;
    CamOsMemcpy(&pstDevContain->stDevTimingCfg, &pstDeviceTimingCfg->stDeviceTimingCfg,
                sizeof(HAL_DISP_ST_DeviceTimingConfig_t));

    DISP_DBG(DISP_DBG_LEVEL_HAL,
             "%s %d, ContainId:%d, DevId:%d, H(%d %d %d %d %d) V(%d %d %d %d %d) Fps:%d, Ssc(%x %x)\n", __FUNCTION__,
             __LINE__, pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId,
             pstDevContain->stDevTimingCfg.u16HsyncWidth, pstDevContain->stDevTimingCfg.u16HsyncBackPorch,
             pstDevContain->stDevTimingCfg.u16Hstart, pstDevContain->stDevTimingCfg.u16Hactive,
             pstDevContain->stDevTimingCfg.u16Htotal, pstDevContain->stDevTimingCfg.u16VsyncWidth,
             pstDevContain->stDevTimingCfg.u16VsyncBackPorch, pstDevContain->stDevTimingCfg.u16Vstart,
             pstDevContain->stDevTimingCfg.u16Vactive, pstDevContain->stDevTimingCfg.u16Vtotal,
             pstDevContain->stDevTimingCfg.u16Fps, pstDevContain->stDevTimingCfg.u16SscStep,
             pstDevContain->stDevTimingCfg.u16SscSpan);

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceOutputTiming(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *         pstDispCtxCfg = NULL;
    HAL_DISP_ST_DeviceTimingInfo_t *pstDeviceTimingCfg;
    DRV_DISP_CTX_DeviceContain_t *  pstDevContain = NULL;

    pstDispCtxCfg      = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain      = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstDeviceTimingCfg = (HAL_DISP_ST_DeviceTimingInfo_t *)pCfg;

    DISP_DBG_VAL(pstDevContain);
    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ConId:%d DevId:%d, Interface:%s, TimingId:%s\n", __FUNCTION__, __LINE__,
             pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId, PARSING_HAL_INTERFACE(pstDevContain->u32Interface),
             PARSING_HAL_TMING_ID(pstDeviceTimingCfg->eTimeType));

    _HAL_DISP_IF_SetTgenResetSt(pstDispCtxCfg, pstDeviceTimingCfg);

    _HAL_DISP_IF_SetTgenConfig(pstDispCtxCfg);
    _HAL_DISP_IF_SetRdmaPatTg(pstDispCtxCfg);

    _HAL_DISP_IF_SetTgenResetEnd(pstDispCtxCfg, pstDeviceTimingCfg);
    _HAL_DISP_IF_SetTgenFwVtt(pstDispCtxCfg);
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceParam(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e        enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    HAL_DISP_ST_DeviceParam_t *   pstDevParm    = NULL;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstDevParm    = (HAL_DISP_ST_DeviceParam_t *)pCfg;

    if (pstDevParm->stCsc.eCscMatrix == E_MI_DISP_CSC_MATRIX_BYPASS
        || pstDevParm->stCsc.eCscMatrix == E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC
        || pstDevParm->stCsc.eCscMatrix == E_MI_DISP_CSC_MATRIX_BT709_TO_RGB_PC)
    {
        CamOsMemcpy(&pstDevContain->stDeviceParam.stCsc, &pstDevParm->stCsc, sizeof(MI_DISP_Csc_t));
        pstDevContain->stDeviceParam.u32Sharpness = pstDevParm->u32Sharpness;

        DISP_DBG(DISP_DBG_LEVEL_HAL,
                 "%s %d, ContainId:%d, DevId:%d, Matrix:%s, Luma:%d, Contrast:%d, Hue:%d, Sat:%d, Gain:%d Sharp:%d\n",
                 __FUNCTION__, __LINE__, pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId,
                 (pstDevParm->stCsc.eCscMatrix), pstDevParm->stCsc.u32Luma, pstDevParm->stCsc.u32Contrast,
                 pstDevParm->stCsc.u32Hue, pstDevParm->stCsc.u32Saturation, pstDevParm->u32Gain,
                 pstDevParm->u32Sharpness);
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Not Support %s\n", __FUNCTION__, __LINE__, (pstDevParm->stCsc.eCscMatrix));
    }

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceParam(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_DeviceParam_t *pstDevParm = NULL;
    HAL_DISP_PICTURE_Config_t  stPictureCfg;

    pstDevParm = (HAL_DISP_ST_DeviceParam_t *)pCfg;
    if (HAL_DISP_PICTURE_TransNonLinear(pCtx, &pstDevParm->stCsc, &pstDevParm->u32Sharpness, &stPictureCfg))
    {
        HAL_DISP_PICTURE_SetConfig(pstDevParm->stCsc.eCscMatrix, &stPictureCfg, pCtx);
    }
    else
    {
        DISP_ERR("%s %d, Trans NonLiear Fail\n", __FUNCTION__, __LINE__);
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDevicDisplayInfo(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e            enRet           = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *           pstDispCtxCfg   = NULL;
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain   = NULL;
    HAL_DISP_ST_DeviceTimingConfig_t *pstDevTimingCfg = NULL;
    MHAL_DISP_DisplayInfo_t *         pstDisplayInfo  = NULL;

    pstDispCtxCfg   = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain   = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstDevTimingCfg = &pstDevContain->stDevTimingCfg;
    pstDisplayInfo  = (MHAL_DISP_DisplayInfo_t *)pCfg;

    pstDisplayInfo->u16Htotal      = pstDevTimingCfg->u16Htotal;
    pstDisplayInfo->u16Vtotal      = pstDevTimingCfg->u16Vtotal;
    pstDisplayInfo->u16HdeStart    = pstDevTimingCfg->u16Hstart;
    pstDisplayInfo->u16VdeStart    = pstDevTimingCfg->u16Vstart;
    pstDisplayInfo->u16Width       = pstDevTimingCfg->u16Hactive;
    pstDisplayInfo->u16Height      = pstDevTimingCfg->u16Vactive;
    pstDisplayInfo->bInterlaceMode = 0;
    pstDisplayInfo->bYuvOutput     = 0;

    DISP_DBG(DISP_DBG_LEVEL_HAL,
             "%s %d, ContainId:%d, DevId:%d, Total(%d %d) DeSt(%d %d), Size(%d %d), Interlace:%d, Yuv:%d\n",
             __FUNCTION__, __LINE__, pstDispCtxCfg->u32ContainIdx, pstDevContain->u32DevId, pstDisplayInfo->u16Htotal,
             pstDisplayInfo->u16Vtotal, pstDisplayInfo->u16HdeStart, pstDisplayInfo->u16VdeStart,
             pstDisplayInfo->u16Width, pstDisplayInfo->u16Height, pstDisplayInfo->bInterlaceMode,
             pstDisplayInfo->bYuvOutput);

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceDisplayInfo(void *pCtx, void *pCfg) {}

//-------------------------------------------------------------------------------
// VidLayer
//-------------------------------------------------------------------------------
static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoVidLayerInit(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_OK;
    UNUSED(pCtx);
    UNUSED(pCfg);

    return enRet;
}

static void _HAL_DISP_IF_SetVidLayerInit(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoVidLayerEnable(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_NONEED;
    UNUSED(pCtx);
    UNUSED(pCfg);

    return enRet;
}

static void _HAL_DISP_IF_SetVidLayerEnable(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoVidLayerBind(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e            enRet                = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *           pstDispVideoLayerCtx = NULL;
    DRV_DISP_CTX_Config_t *           pstDispDevCtx        = NULL;
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain        = NULL;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain   = NULL;

    pstDispVideoLayerCtx = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDispDevCtx        = (DRV_DISP_CTX_Config_t *)pCfg;
    pstDevContain        = pstDispDevCtx->pstCtxContain->pstDevContain[pstDispDevCtx->u32ContainIdx];
    pstVidLayerContain   = pstDispVideoLayerCtx->pstCtxContain->pstVidLayerContain[pstDispVideoLayerCtx->u32ContainIdx];

    if (pstDispDevCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE
        && pstDispVideoLayerCtx->enCtxType == E_DRV_DISP_CTX_TYPE_VIDLAYER)
    {
        if (pstVidLayerContain->pstDevCtx)
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, VidLayerContainId:%d, VidLayerType:%d,  Arelady Bind The Device\n", __FUNCTION__, __LINE__,
                     pstDispVideoLayerCtx->u32ContainIdx, pstVidLayerContain->eVidLayerType);
        }
        else
        {
            if (pstDevContain->u32DevId == HAL_DISP_DEVICE_ID_0
                && pstVidLayerContain->eVidLayerType != HAL_DISP_VIDLAYER_ID_0)
            {
                enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
                DISP_ERR("%s %d, VidLayerContainId:%d, VidLayerType:%d,  HW Not Supoorted\n", __FUNCTION__, __LINE__,
                         pstDispVideoLayerCtx->u32ContainIdx, pstVidLayerContain->eVidLayerType);
            }
            else
            {
                DISP_DBG(DISP_DBG_LEVEL_HAL,
                         "%s %d, DevContainId:%d, DevId:%d, VidLayerContainId:%d, VidLayerType:%d\n", __FUNCTION__,
                         __LINE__, pstDispDevCtx->u32ContainIdx, pstDevContain->u32DevId,
                         pstDispVideoLayerCtx->u32ContainIdx, pstVidLayerContain->eVidLayerType);

                pstVidLayerContain->pstDevCtx                                   = (void *)pstDevContain;
                pstDevContain->pstVidLayeCtx[pstVidLayerContain->eVidLayerType] = (void *)pstVidLayerContain;
                pstDevContain->eBindVideoLayer |= (0x1 << pstVidLayerContain->eVidLayerType);
            }
        }
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, DevContainId:%d, DevId:%d(%s), VidLayerContainId:%d, VidLayerType:%d(%s)\n", __FUNCTION__,
                 __LINE__, pstDispDevCtx->u32ContainIdx, pstDevContain->u32DevId,
                 PARSING_CTX_TYPE(pstDispDevCtx->enCtxType), pstDispVideoLayerCtx->u32ContainIdx,
                 pstVidLayerContain->eVidLayerType, PARSING_CTX_TYPE(pstDispVideoLayerCtx->enCtxType));
    }

    return enRet;
}

static void _HAL_DISP_IF_SetVidLayerBind(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoVidLayerUnBind(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e            enRet                = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *           pstDispVideoLayerCtx = NULL;
    DRV_DISP_CTX_Config_t *           pstDispDevCtx        = NULL;
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain        = NULL;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain   = NULL;

    pstDispVideoLayerCtx = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDispDevCtx        = (DRV_DISP_CTX_Config_t *)pCfg;
    pstDevContain        = pstDispDevCtx->pstCtxContain->pstDevContain[pstDispDevCtx->u32ContainIdx];
    pstVidLayerContain   = pstDispVideoLayerCtx->pstCtxContain->pstVidLayerContain[pstDispVideoLayerCtx->u32ContainIdx];

    if (pstDispDevCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE
        && pstDispVideoLayerCtx->enCtxType == E_DRV_DISP_CTX_TYPE_VIDLAYER)
    {
        if (pstVidLayerContain->pstDevCtx == pstDevContain)
        {
            pstDevContain->eBindVideoLayer &= ~(0x1 << pstVidLayerContain->eVidLayerType);

            DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevContainId:%d, DevId:%d, VidLayerContainId:%d, VidLayrId:%d\n",
                     __FUNCTION__, __LINE__, pstDispDevCtx->u32ContainIdx, pstDevContain->u32DevId,
                     pstDispVideoLayerCtx->u32ContainIdx, pstDevContain->eBindVideoLayer);
        }
        else
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, DevContainId:%d, DevId:%d,(%px) != VidLayerContainId:%d, VidLayerType:%d(%px)\n",
                     __FUNCTION__, __LINE__, pstDispDevCtx->u32ContainIdx, pstDevContain->u32DevId, pstDevContain,
                     pstDispVideoLayerCtx->u32ContainIdx, pstVidLayerContain->eVidLayerType,
                     pstVidLayerContain->pstDevCtx);
        }
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, DevContainId:%d, DevId:%d(%s), VidLayerContainId:%d, VidLayerType:%d(%s)\n", __FUNCTION__,
                 __LINE__, pstDispDevCtx->u32ContainIdx, pstDevContain->u32DevId,
                 PARSING_CTX_TYPE(pstDispDevCtx->enCtxType), pstDispVideoLayerCtx->u32ContainIdx,
                 pstVidLayerContain->eVidLayerType, PARSING_CTX_TYPE(pstDispVideoLayerCtx->enCtxType));
    }

    return enRet;
}

static void _HAL_DISP_IF_SetVidLayerUnBind(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *           pstDispVideoLayerCtx = NULL;
    DRV_DISP_CTX_Config_t *           pstDispDevCtx        = NULL;
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain        = NULL;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain   = NULL;

    pstDispVideoLayerCtx = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDispDevCtx        = (DRV_DISP_CTX_Config_t *)pCfg;
    pstDevContain        = pstDispDevCtx->pstCtxContain->pstDevContain[pstDispDevCtx->u32ContainIdx];
    pstVidLayerContain   = pstDispVideoLayerCtx->pstCtxContain->pstVidLayerContain[pstDispVideoLayerCtx->u32ContainIdx];

    pstVidLayerContain->pstDevCtx                                   = NULL;
    pstDevContain->pstVidLayeCtx[pstVidLayerContain->eVidLayerType] = NULL;
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoVidLayerAttr(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_NONEED;
    UNUSED(pCtx);
    UNUSED(pCfg);

    return enRet;
}

static void _HAL_DISP_IF_SetVidLayerAttr(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoVidLayerPriority(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_NONEED;

    return enRet;
}

static void _HAL_DISP_IF_SetVidLayerPriority(void *pCtx, void *pCfg) {}

//-------------------------------------------------------------------------------
// InputPort
//-------------------------------------------------------------------------------
static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoInputPortInit(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_OK;
    UNUSED(pCtx);
    UNUSED(pCfg);

    return enRet;
}

static void _HAL_DISP_IF_SetInputPortInit(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoInputPortEnable(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e            enRet               = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *           pstDispCtxCfg       = NULL;
    MI_U8 *                           pbEnable            = NULL;
    DRV_DISP_CTX_InputPortContain_t * pstInputPortContain = NULL;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain;
    MI_U32                            u32VidLayerId;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];
    pstVidLayerContain  = (DRV_DISP_CTX_VideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    u32VidLayerId       = pstVidLayerContain->eVidLayerType;
    pbEnable            = (MI_U8 *)pCfg;

    if (pstInputPortContain->bEnInPort == 0 && *pbEnable == 1)
    {
        pstInputPortContain->u16FlipFrontPorchCnt = 0;
    }

    pstInputPortContain->bEnInPort = *pbEnable;

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, video_id=%d, Port_id:%d En:%d\n", __FUNCTION__, __LINE__, u32VidLayerId,
             pstInputPortContain->u32PortId, pstInputPortContain->bEnInPort);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, En:%d\n", __FUNCTION__, __LINE__, *pbEnable);

    if (pstInputPortContain->u32PortId > HAL_DISP_INPUTPORT_NUM)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%d is not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }

    DISP_DBG_VAL(u32VidLayerId);

    return enRet;
}

static void _HAL_DISP_IF_SetInputPortEnable(void *pCtx, void *pCfg)
{
    // MI_U8 *pbEnable = NULL;

    // pbEnable = (MI_U8 *)pCfg;
    HAL_DISP_SetSgRdmaEn(pCtx, pCfg);
    /*if (*pbEnable == 0)
    {
        HAL_DISP_SetFrameColorForce(*pbEnable ? 0 : 1, pCtx);
    }*/
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoInputPortAttr(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e            enRet               = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *           pstDispCtxCfg       = NULL;
    MI_DISP_InputPortAttr_t *         pstHalInputPortCfg  = NULL;
    DRV_DISP_CTX_InputPortContain_t * pstInputPortContain = NULL;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain;
    MI_U32                            u32VidLayerId;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];
    pstVidLayerContain  = (DRV_DISP_CTX_VideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    u32VidLayerId       = pstVidLayerContain->eVidLayerType;
    pstHalInputPortCfg  = (MI_DISP_InputPortAttr_t *)pCfg;

    CamOsMemcpy(&pstInputPortContain->stAttr, pstHalInputPortCfg, sizeof(MI_DISP_InputPortAttr_t));

    DISP_DBG_VAL(pstHalInputPortCfg);
    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, video_id=%d, The settings of Port_id:%d\n", __FUNCTION__, __LINE__,
             u32VidLayerId, pstInputPortContain->u32PortId);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Disp(%d %d %d %d) Src(%d %d)\n", __FUNCTION__, __LINE__,
             pstHalInputPortCfg->stDispWin.u16X, pstHalInputPortCfg->stDispWin.u16Y,
             pstHalInputPortCfg->stDispWin.u16Width, pstHalInputPortCfg->stDispWin.u16Height,
             pstHalInputPortCfg->u16SrcWidth, pstHalInputPortCfg->u16SrcHeight);

    if (pstInputPortContain->u32PortId > HAL_DISP_MOPG_GWIN_NUM)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, Port_id:%d does not support\n", __FUNCTION__, __LINE__, pstInputPortContain->u32PortId);
    }
    DISP_DBG_VAL(u32VidLayerId);
    return enRet;
}

static void _HAL_DISP_IF_SetInputPortAttr(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *  pstDispCtxCfg      = NULL;
    MI_DISP_InputPortAttr_t *pstHalInputPortCfg = NULL;
    // DRV_DISP_CTX_InputPortContain_t * pstInputPortContain = NULL;
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain      = NULL;
    HAL_DISP_ST_DeviceTimingConfig_t *pstDeviceTimingCfg = NULL;
    MI_U16                            u16HdeSt, u16VdeSt;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    // pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];
    pstHalInputPortCfg = (MI_DISP_InputPortAttr_t *)pCfg;
    pstDeviceTimingCfg = &pstDevContain->stDevTimingCfg;

    if (pstDevContain->u32Interface == HAL_DISP_INTF_BT656)
    {
        u16HdeSt = pstDeviceTimingCfg->u16Htotal - pstDeviceTimingCfg->u16Hactive + pstHalInputPortCfg->stDispWin.u16X;
    }
    else
    {
        u16HdeSt = pstDeviceTimingCfg->u16Hstart + pstHalInputPortCfg->stDispWin.u16X;
    }
    u16VdeSt = pstDeviceTimingCfg->u16Vstart + pstHalInputPortCfg->stDispWin.u16Y;

    HAL_DISP_SetTgenVdeSt(u16VdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenVdeEnd(u16VdeSt + pstHalInputPortCfg->stDispWin.u16Height - 1, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenHdeSt(u16HdeSt, (void *)pstDispCtxCfg);
    HAL_DISP_SetTgenHdeEnd(u16HdeSt + pstHalInputPortCfg->stDispWin.u16Width - 1, (void *)pstDispCtxCfg);
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoInputPortFlip(void *pCtx, void *pCfg)
{
    MI_U16                             i;
    HAL_DISP_ST_QueryRet_e             enRet               = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *            pstDispCtxCfg       = NULL;
    MI_DISP_IMPL_MhalVideoFrameData_t *pstFramedata        = NULL;
    DRV_DISP_CTX_InputPortContain_t *  pstInputPortContain = NULL;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];
    pstFramedata        = (MI_DISP_IMPL_MhalVideoFrameData_t *)pCfg;

    if (enRet == E_HAL_DISP_ST_QUERY_RET_OK)
    {
        CamOsMemcpy(&pstInputPortContain->stFrameData, pstFramedata, sizeof(MI_DISP_IMPL_MhalVideoFrameData_t));

        for (i = 0; i < 3; i++)
        {
            if (pstInputPortContain->stFrameData.aPhyAddr[i] & 0xF) // physical address should be 16 align
            {
                enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
                DISP_ERR("%s %d, PHY_ADDR should be 16 align, addr[%d](%08llx)\n", __FUNCTION__, __LINE__, i,
                         pstInputPortContain->stFrameData.aPhyAddr[i]);
            }
        }

        for (i = 0; i < 3; i++)
        {
            if (i == 0)
            {
                if (IsSgRdmaYUV422Pack(pstFramedata->ePixelFormat))
                {
                    pstInputPortContain->stFrameData.au32Stride[i] = pstInputPortContain->stFrameData.au32Stride[i] / 2;
                }
                else if (IsArgb32bitPack(pstFramedata->ePixelFormat))
                {
                    pstInputPortContain->stFrameData.au32Stride[i] = pstInputPortContain->stFrameData.au32Stride[i] / 4;
                }
            }
            if ((pstFramedata->ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR
                 || pstFramedata->ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR))
            {
                if (i != 0)
                {
                    pstInputPortContain->stFrameData.au32Stride[i] = pstInputPortContain->stFrameData.au32Stride[i] * 2;
                }
            }
        }

        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Fmt:%s, PhyAddr(%08llx, %08llx, %08llx), Stride(%d %d %d)\n", __FUNCTION__,
                 __LINE__, PARSING_HAL_PIXEL_FMT(pstInputPortContain->stFrameData.ePixelFormat),
                 pstInputPortContain->stFrameData.aPhyAddr[0], pstInputPortContain->stFrameData.aPhyAddr[1],
                 pstInputPortContain->stFrameData.aPhyAddr[2], pstInputPortContain->stFrameData.au32Stride[0],
                 pstInputPortContain->stFrameData.au32Stride[1], pstInputPortContain->stFrameData.au32Stride[2]);
    }

    return enRet;
}

static void _HAL_DISP_IF_SetInputPortFlip(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *            pstDispCtxCfg = NULL;
    MI_DISP_IMPL_MhalVideoFrameData_t *pstFramedata  = NULL;
    // DRV_DISP_CTX_InputPortContain_t *  pstInputPortContain = NULL;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    // pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];
    pstFramedata = (MI_DISP_IMPL_MhalVideoFrameData_t *)pCfg;

    HAL_DISP_SetInputPortCrop(pCtx);
    HAL_DISP_SetSgRdmaCfg(pstFramedata->ePixelFormat, (void *)pstDispCtxCfg);
    HAL_DISP_SetSgRdmaSize(pstFramedata->ePixelFormat, (void *)pstDispCtxCfg);
    HAL_DISP_SetSgRdmaBufConfig((void *)pstDispCtxCfg);
    HAL_DISP_SetSgRdmaFifoPush((void *)pstDispCtxCfg);
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoInputPortCrop(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e           enRet               = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *          pstDispCtxCfg       = NULL;
    MI_DISP_VidWinRect_t *           pstHalInputPortCfg  = NULL;
    DRV_DISP_CTX_InputPortContain_t *pstInputPortContain = NULL;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];
    pstHalInputPortCfg  = (MI_DISP_VidWinRect_t *)pCfg;

    if (pstHalInputPortCfg->u16Width != pstInputPortContain->stAttr.stDispWin.u16Width
        || pstHalInputPortCfg->u16Height != pstInputPortContain->stAttr.stDispWin.u16Height)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Crop size(%d, %d) need to be equal to disp win size(%d, %d)\n", __FUNCTION__, __LINE__,
                 pstHalInputPortCfg->u16Width, pstHalInputPortCfg->u16Height,
                 pstInputPortContain->stAttr.stDispWin.u16Width, pstInputPortContain->stAttr.stDispWin.u16Height);
    }
    else
    {
        CamOsMemcpy(&pstInputPortContain->stCrop, pstHalInputPortCfg, sizeof(MI_DISP_VidWinRect_t));

        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Crop(%d %d %d %d)\n", __FUNCTION__, __LINE__, pstHalInputPortCfg->u16X,
                 pstHalInputPortCfg->u16Y, pstHalInputPortCfg->u16Width, pstHalInputPortCfg->u16Height);
    }

    return enRet;
}
static void _HAL_DISP_IF_SetInputPortCrop(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoClkGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e   enRet                             = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_ST_ClkConfig_t *pstClkCfg                         = NULL;
    MI_U8                    u8ClkName[E_HAL_DISP_CLK_NUM][20] = HAL_DISP_CLK_NAME;
    MI_U32                   i;
    UNUSED(pCtx);

    pstClkCfg = (HAL_DISP_ST_ClkConfig_t *)pCfg;

    pstClkCfg->u32Num = E_HAL_DISP_CLK_NUM;

    for (i = 0; i < E_HAL_DISP_CLK_NUM; i++)
    {
        _HAL_DISP_IF_ClkGet(i, pstClkCfg);
        DISP_DBG(DISP_DBG_LEVEL_CLK, "%s %d, CLK_%-10s: En:%d, Rate:%d\n", __FUNCTION__, __LINE__, u8ClkName[i],
                 pstClkCfg->bEn[i], pstClkCfg->u32Rate[i]);
    }
    DISP_DBG_VAL(u8ClkName[0][0]);
    return enRet;
}

static void _HAL_DISP_IF_SetClkGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDrvTurningSet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;

    return enRet;
}

static void _HAL_DISP_IF_SetDrvTurningSet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDrvTurningGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;

    return enRet;
}

static void _HAL_DISP_IF_SetDrvTurningGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDbgmgGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e          enRet       = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx  = NULL;
    HAL_DISP_ST_DbgmgConfig_t *     pstDbgmgCfg = NULL;

    pstDbgmgCfg = (HAL_DISP_ST_DbgmgConfig_t *)pCfg;

    HAL_DISP_GetCmdqByCtx(pCtx, (void *)&pstCmdqCtx);
    CamProcSeqPrintf((CamProcSeqFile_t *)pstDbgmgCfg->pData, "Cmdq_%d:: WaitCnt:%d, FlipCnt:%d\n",
                     pstCmdqCtx->s32UtilityId, pstCmdqCtx->u16WaitDoneCnt, pstCmdqCtx->u16RegFlipCnt);
    return enRet;
}

static void _HAL_DISP_IF_SetDbgmgGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoClkSet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e   enRet     = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_ST_ClkConfig_t *pstClkCfg = NULL;
    UNUSED(pCtx);

    pstClkCfg = (HAL_DISP_ST_ClkConfig_t *)pCfg;

    if (pstClkCfg->u32Num != E_HAL_DISP_CLK_NUM)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Clk Num (%d) is not match %d\n", __FUNCTION__, __LINE__, pstClkCfg->u32Num,
                 E_HAL_DISP_CLK_NUM);
    }
    else
    {
        MI_U32 i;
        MI_U8  au8ClkName[E_HAL_DISP_CLK_NUM][20] = HAL_DISP_CLK_NAME;

        for (i = 0; i < E_HAL_DISP_CLK_NUM; i++)
        {
            DISP_DBG(DISP_DBG_LEVEL_CLK, "%s %d, CLK_%s: En:%d, Rate:%d\n", __FUNCTION__, __LINE__, au8ClkName[i],
                     pstClkCfg->bEn[i], pstClkCfg->u32Rate[i]);
        }
        DISP_DBG_VAL(au8ClkName[0][0]);
    }

    return enRet;
}

static void _HAL_DISP_IF_SetClkSet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_ClkConfig_t *pstClkCfg = NULL;
    MI_U32                   i;
    UNUSED(pCtx);

    pstClkCfg = (HAL_DISP_ST_ClkConfig_t *)pCfg;
    for (i = 0; i < E_HAL_DISP_CLK_NUM; i++)
    {
        _HAL_DISP_IF_ClkSet(i, pstClkCfg);
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoRegAccess(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *pstDispCtxCfg = NULL;
    void *                 pstCmdqCtx    = NULL;
    UNUSED(pCfg);

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    DISP_DBG_VAL(pstDispCtxCfg);

    if (HAL_DISP_GetCmdqByCtx(pCtx, &pstCmdqCtx))
    {
        DISP_DBG(DISP_DBG_LEVEL_UTILITY_CMDQ, "%s %d, AccessMode:%s, CmdqHandler:%px\n", __FUNCTION__, __LINE__,
                 PARSING_HAL_REG_ACCESS_TYPE(((HAL_DISP_UTILITY_CmdqContext_t *)pstCmdqCtx)->s32UtilityId), pstCmdqCtx);
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Unknow CTX Type:%d\n", __FUNCTION__, __LINE__, pstDispCtxCfg->enCtxType);
    }
    return enRet;
}

static void _HAL_DISP_IF_SetRegAccess(void *pCtx, void *pCfg)
{
    HAL_DISP_UTILITY_CmdqContext_t *pstCmdqCtx = NULL;
    UNUSED(pCfg);

    HAL_DISP_GetCmdqByCtx(pCtx, (void *)&pstCmdqCtx);

    if (pstCmdqCtx)
    {
        DISP_DBG(DISP_DBG_LEVEL_UTILITY_CMDQ, "%s %d, Id:%s Change AccessMode Not support:, CmdqHandler:%px\n",
                 __FUNCTION__, __LINE__,
                 PARSING_HAL_REG_ACCESS_TYPE(((HAL_DISP_UTILITY_CmdqContext_t *)pstCmdqCtx)->s32UtilityId), pstCmdqCtx);
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoRegFlip(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e            enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *           pstDispCtxCfg = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *  pstCmdqCtx    = NULL;
    MI_DISP_IMPL_MhalRegFlipConfig_t *pstRegFlipCfg = NULL;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstRegFlipCfg = (MI_DISP_IMPL_MhalRegFlipConfig_t *)pCfg;

    DISP_DBG_VAL(pstDispCtxCfg);
    if (HAL_DISP_GetCmdqByCtx(pCtx, (void *)&pstCmdqCtx))
    {
        if (pstCmdqCtx)
        {
            HAL_DISP_UTILITY_SetCmdqInf(pstRegFlipCfg->pCmdqInf, (MI_U32)pstCmdqCtx->s32UtilityId);

            if ((HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)pstCmdqCtx->s32UtilityId) == E_MI_DISP_REG_ACCESS_CPU)
                || pstRegFlipCfg->pCmdqInf == NULL)
            {
                DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, CtxType:%s, CmdqId:%d, RIU MODE\n", __FUNCTION__,
                         __LINE__, PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType), pstCmdqCtx->s32UtilityId);
            }
            else
            {
                DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, CtxType:%s, CmdqId:%d, CMDQ MODE\n", __FUNCTION__,
                         __LINE__, PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType), pstCmdqCtx->s32UtilityId);
            }
        }
        else
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, CtxType:%s, CmdqId:%d, CmdqCtx:%px, CmdqHander:%px \n", __FUNCTION__, __LINE__,
                     PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType), pstCmdqCtx->s32UtilityId, pstCmdqCtx,
                     pstRegFlipCfg->pCmdqInf);
        }
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Get Cmdq Ctx Fail, CtxType:%s\n", __FUNCTION__, __LINE__,
                 PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType));
    }

    return enRet;
}

static void _HAL_DISP_IF_SetRegFlip(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *           pstDispCtxCfg = NULL;
    void *                            pstCmdqCtx    = NULL;
    MI_DISP_IMPL_MhalRegFlipConfig_t *pstRegFlipCfg = (MI_DISP_IMPL_MhalRegFlipConfig_t *)pCfg;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;

    if (pstDispCtxCfg->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        HAL_DISP_GetCmdqByCtx(pCtx, &pstCmdqCtx);
        DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, CmdqCtx:%px, bEnFlip:%hhd, Dev MODE\n", __FUNCTION__, __LINE__,
                 pstCmdqCtx, pstRegFlipCfg->bEnable);
        if (pstRegFlipCfg->bEnable)
        {
            if (pstCmdqCtx)
            {
                _HAL_DISP_IF_SetRegFlipPreAct(pstDispCtxCfg);

                HAL_DISP_UTILITY_W2BYTEMSKDirectCmdqWrite(pstCmdqCtx);
                _HAL_DISP_IF_SetRegFlipPostAct(pstDispCtxCfg);
            }
        }
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoRegWaitDone(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e                enRet             = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *               pstDispCtxCfg     = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *      pstCmdqCtx        = NULL;
    MI_DISP_IMPL_MhalRegWaitDoneConfig_t *pstRegWaitDoneCfg = NULL;

    pstDispCtxCfg     = (DRV_DISP_CTX_Config_t *)pCtx;
    pstRegWaitDoneCfg = (MI_DISP_IMPL_MhalRegWaitDoneConfig_t *)pCfg;

    DISP_DBG_VAL(pstDispCtxCfg);

    if (HAL_DISP_GetCmdqByCtx(pCtx, (void *)&pstCmdqCtx))
    {
        if (pstCmdqCtx)
        {
            HAL_DISP_UTILITY_SetCmdqInf(pstRegWaitDoneCfg->pCmdqInf, pstCmdqCtx->s32UtilityId);

            if ((HAL_DISP_UTILITY_GetRegAccessMode((MI_U32)pstCmdqCtx->s32UtilityId) == E_MI_DISP_REG_ACCESS_CPU)
                || pstRegWaitDoneCfg->pCmdqInf == NULL)
            {
                DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, CtxType:%s, CmdqId:%d, RIU MODE\n", __FUNCTION__,
                         __LINE__, PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType), pstCmdqCtx->s32UtilityId);
            }
            else
            {
                DISP_DBG(DISP_DBG_LEVEL_UTILITY_FLIP, "%s %d, CtxType:%s, CmdqId:%d, CMDQ MODE\n", __FUNCTION__,
                         __LINE__, PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType), pstCmdqCtx->s32UtilityId);
            }
        }
        else
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, CtxType:%s, CmdqId:%d, CmdqCtx:%px, CmdqHander:%px \n", __FUNCTION__, __LINE__,
                     PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType), pstCmdqCtx->s32UtilityId, pstCmdqCtx,
                     pstRegWaitDoneCfg->pCmdqInf);
        }
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Get Cmdq Ctx Fail, CtxType%s\n", __FUNCTION__, __LINE__,
                 PARSING_CTX_TYPE(pstDispCtxCfg->enCtxType));
    }

    return enRet;
}

static void _HAL_DISP_IF_SetRegWaitDone(void *pCtx, void *pCfg)
{
    DRV_DISP_CTX_Config_t *               pstDispCtxCfg     = NULL;
    MI_DISP_IMPL_MhalRegWaitDoneConfig_t *pstRegWaitDoneCfg = NULL;
    HAL_DISP_UTILITY_CmdqContext_t *      pstCmdqCtx        = NULL;

    pstDispCtxCfg     = (DRV_DISP_CTX_Config_t *)pCtx;
    pstRegWaitDoneCfg = (MI_DISP_IMPL_MhalRegWaitDoneConfig_t *)pCfg;

    if (pstDispCtxCfg->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        HAL_DISP_GetCmdqByCtx(pCtx, (void *)&pstCmdqCtx);

        if (pstRegWaitDoneCfg->pCmdqInf)
        {
            HAL_DISP_UTILITY_SetCmdqInf(pstRegWaitDoneCfg->pCmdqInf, pstCmdqCtx->s32UtilityId);
        }

        _HAL_DISP_IF_SetWaitDonePreAct(pstDispCtxCfg);
        HAL_DISP_UTILITY_AddWaitCmd(pstCmdqCtx);        // add wait even
        _HAL_DISP_IF_SetWaitDonePostAct(pstDispCtxCfg); // Clear disp2cmdq intterupt
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoHwInfoConfig(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e      enRet        = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_ST_HwInfoConfig_t *pstHwInfoCfg = (HAL_DISP_ST_HwInfoConfig_t *)pCfg;
    UNUSED(pCtx);

    if (pstHwInfoCfg->eType == E_HAL_DISP_ST_HW_INFO_DEVICE)
    {
        pstHwInfoCfg->u32Count = HAL_DISP_DEVICE_MAX;
    }
    else if (pstHwInfoCfg->eType == E_HAL_DISP_ST_HW_INFO_VIDEOLAYER)
    {
        pstHwInfoCfg->u32Count = HAL_DISP_VIDLAYER_MAX;
    }
    else if (pstHwInfoCfg->eType == E_HAL_DISP_ST_HW_INFO_INPUTPORT)
    {
        if (pstHwInfoCfg->u32Id >= HAL_DISP_VIDLAYER_MAX)
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, VidLayerId:%d, is out of range\n", __FUNCTION__, __LINE__, pstHwInfoCfg->u32Id);
        }
        else
        {
            if (pstHwInfoCfg->u32Id == HAL_DISP_VIDLAYER_ID_0)
            {
                pstHwInfoCfg->u32Count = HAL_DISP_MOPG_GWIN_NUM;
            }
            else
            {
                pstHwInfoCfg->u32Count = HAL_DISP_MOPS_GWIN_NUM;
            }
        }
    }
    else if (pstHwInfoCfg->eType == E_HAL_DISP_ST_HW_INFO_DMA)
    {
        pstHwInfoCfg->u32Count = HAL_DISP_DMA_MAX;
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Unknown Type:%d\n", __FUNCTION__, __LINE__, pstHwInfoCfg->eType);
    }
    return enRet;
}

static void _HAL_DISP_IF_SetHwInfoConfig(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoClkInit(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e       enRet         = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_ST_ClkInitConfig_t *pstClkInitCfg = (HAL_DISP_ST_ClkInitConfig_t *)pCfg;
    UNUSED(pCtx);

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, ClkInit:%d\n", __FUNCTION__, __LINE__, pstClkInitCfg->bEn);
    DISP_DBG_VAL(pstClkInitCfg);
    return enRet;
}

static void _HAL_DISP_IF_SetClkInit(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_ClkInitConfig_t *pstClkInitCfg = (HAL_DISP_ST_ClkInitConfig_t *)pCfg;
    UNUSED(pCtx);

    HAL_DISP_CLK_Init(pstClkInitCfg->bEn);
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoInterCfgSet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e           enRet       = E_HAL_DISP_ST_QUERY_RET_OK;
    MI_DISP_IMPL_MhalDeviceConfig_t *pstInterCfg = NULL;
    DRV_DISP_CTX_Config_t *          pstDispCtx  = NULL;
    MI_U32                           u32Dev;

    pstDispCtx  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInterCfg = (MI_DISP_IMPL_MhalDeviceConfig_t *)pCfg;
    if (pstDispCtx->pstCtxContain && pstDispCtx->pstCtxContain->bDevContainUsed[pstDispCtx->u32ContainIdx])
    {
        u32Dev = HAL_DISP_GetDeviceId(pstDispCtx, 0);
    }
    else
    {
        u32Dev = pstDispCtx->u32ContainIdx;
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_BOOTLOGO)
    {
        g_stInterCfg[u32Dev].bBootlogoEn = pstInterCfg->bBootlogoEn;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d BootLogo En:%d\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].bBootlogoEn);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_CSC_EN)
    {
        g_stInterCfg[u32Dev].bCscEn = pstInterCfg->bCscEn;
        pstInterCfg->u8CscMd        = (pstInterCfg->bCscEn) ? g_stInterCfg[u32Dev].u8CscMd : 0;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, ColorCscEn:%hhd\n", __FUNCTION__, __LINE__, u32Dev,
                 pstInterCfg->bCscEn);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_COLORID)
    {
        if (pstInterCfg->u8ColorId < E_HAL_DISP_COLOR_CSC_ID_NUM)
        {
            g_stInterCfg[u32Dev].u8ColorId = pstInterCfg->u8ColorId;
            DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, ColorID:%d\n", __FUNCTION__, __LINE__, u32Dev,
                     g_stInterCfg[u32Dev].u8ColorId);
        }
        else
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, Not Support Color ID_%d\n", __FUNCTION__, __LINE__, pstInterCfg->u8ColorId);
        }
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_GOPBLENDID)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Not Support GOP Blend ID\n", __FUNCTION__, __LINE__);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_DISP_PAT)
    {
        g_stInterCfg[u32Dev].bDispPat = pstInterCfg->bDispPat;
        g_stInterCfg[u32Dev].u8CscMd  = pstInterCfg->u8PatMd;
        g_stInterCfg[u32Dev].bMute    = pstInterCfg->bMute;
        pstInterCfg->bCscEn           = (pstInterCfg->bDispPat) ? 0 : g_stInterCfg[u32Dev].bCscEn;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, bDispPat:%hhu u8PatMd:%hhu bMute:%hhu\n", __FUNCTION__, __LINE__,
                 u32Dev, g_stInterCfg[u32Dev].bDispPat, pstInterCfg->u8PatMd, pstInterCfg->bMute);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_MOP)
    {
        g_stInterCfg[u32Dev].bRstMop = pstInterCfg->bRstMop;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, bRstMop:%d\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].bRstMop);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_DISP)
    {
        g_stInterCfg[u32Dev].bRstDisp = pstInterCfg->bRstDisp;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, bRstDisp:%d\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].bRstDisp);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_DAC)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, DevId:%d, bRstDac:%d\n", __FUNCTION__, __LINE__, u32Dev, g_stInterCfg[u32Dev].bRstDac);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_HDMITX)
    {
        g_stInterCfg[u32Dev].bRstHdmitx = pstInterCfg->bRstHdmitx;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, bRstHdmitx:%d\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].bRstHdmitx);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_LCD)
    {
        g_stInterCfg[u32Dev].bRstLcd = pstInterCfg->bRstLcd;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, bRstLcd:%d\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].bRstLcd);
    }
    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_DACAFF)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, DevId:%d, bRstDacAff:%d\n", __FUNCTION__, __LINE__, u32Dev, g_stInterCfg[u32Dev].bRstDacAff);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_DACSYN)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;
        DISP_ERR("%s %d, DevId:%d, bRstDacSyn:%d\n", __FUNCTION__, __LINE__, u32Dev, g_stInterCfg[u32Dev].bRstDacSyn);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_FPLL)
    {
        g_stInterCfg[u32Dev].bRstFpll = pstInterCfg->bRstFpll;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, bRstFpll:%d\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].bRstFpll);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_MOP_Y_THRD)
    {
        g_stInterCfg[u32Dev].u8MopYThrd = pstInterCfg->u8MopYThrd;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, MopYThrd:%x\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].u8MopYThrd);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_MOP_C_THRD)
    {
        g_stInterCfg[u32Dev].u8MopCThrd = pstInterCfg->u8MopCThrd;
        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, MopCThrd:%x\n", __FUNCTION__, __LINE__, u32Dev,
                 g_stInterCfg[u32Dev].u8MopCThrd);
    }

    if (enRet == E_HAL_DISP_ST_QUERY_RET_OK)
    {
        g_stInterCfg[u32Dev].eType |= pstInterCfg->eType;
    }

    return enRet;
}

static void _HAL_DISP_IF_SetInterCfgSet(void *pCtx, void *pCfg)
{
    MI_DISP_IMPL_MhalDeviceConfig_t *pstInterCfg = NULL;
    DRV_DISP_CTX_Config_t *          pstDispCtx  = NULL;
    MI_U32                           u32Dev;
    HAL_DISP_UTILITY_CmdqContext_t * pstCmdqCtx = NULL;

    pstDispCtx  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInterCfg = (MI_DISP_IMPL_MhalDeviceConfig_t *)pCfg;

    if (pstDispCtx->pstCtxContain && pstDispCtx->pstCtxContain->bDevContainUsed[pstDispCtx->u32ContainIdx])
    {
        u32Dev = HAL_DISP_GetDeviceId(pstDispCtx, 0);
        HAL_DISP_GetCmdqByCtx(pCtx, (void *)&pstCmdqCtx);
    }
    else
    {
        u32Dev = pstDispCtx->u32ContainIdx;
    }

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Dev:%d, eType:%x\n", __FUNCTION__, __LINE__, u32Dev, pstInterCfg->eType);

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_DISP_PAT)
    {
        HAL_DISP_SetInterCfgDispPat(pstInterCfg->bDispPat, pstInterCfg->u8PatMd, pstDispCtx);
        if (pstCmdqCtx)
        {
            HAL_DISP_UTILITY_W2BYTEMSKDirectCmdqWrite(pstCmdqCtx);
            HAL_DISP_UTILITY_KickoffCmdq(pstCmdqCtx);
        }
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_CSC_EN)
    {
        HAL_DISP_COLOR_SetColorMatrixEn(g_stInterCfg[u32Dev].u8ColorId, pstInterCfg->bCscEn, u32Dev, pstDispCtx);
        HAL_DISP_COLOR_SetColorMatrixMd(g_stInterCfg[u32Dev].u8ColorId, pstInterCfg->u8CscMd, u32Dev, pstDispCtx);
        _HAL_DISP_IF_SetInterfaceCsc(pCtx, u32Dev, HAL_DISP_COLOR_Y2RM_TO_CSCM(pstInterCfg->u8CscMd));
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_MOP)
    {
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_DISP)
    {
        HAL_DISP_SetSwReset(pstInterCfg->bRstDisp, NULL, u32Dev);
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_HDMITX)
    {
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_LCD)
    {
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_RST_FPLL)
    {
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_MOP_Y_THRD)
    {
    }

    if (pstInterCfg->eType & E_MI_DISP_DEV_CFG_MOP_C_THRD)
    {
    }
}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoInterCfgGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e           enRet       = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *          pstDispCtx  = NULL;
    MI_DISP_IMPL_MhalDeviceConfig_t *pstInterCfg = NULL;

    pstDispCtx  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInterCfg = (MI_DISP_IMPL_MhalDeviceConfig_t *)pCfg;

    if (pstDispCtx->u32ContainIdx >= HAL_DISP_DEVICE_MAX)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, DevId is not supported.(%d)\n", __FUNCTION__, __LINE__, pstDispCtx->u32ContainIdx);
    }
    else
    {
        CamOsMemcpy(pstInterCfg, &g_stInterCfg[pstDispCtx->u32ContainIdx], sizeof(MI_DISP_IMPL_MhalDeviceConfig_t));
    }

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, Type:%x, BootLogo:%d, ColorID:%d, bDispPat:%d u8PatMd:%d bMute:%d\n",
             __FUNCTION__, __LINE__, pstDispCtx->u32ContainIdx, pstInterCfg->eType, pstInterCfg->bBootlogoEn,
             pstInterCfg->u8ColorId, pstInterCfg->bDispPat, pstInterCfg->u8PatMd, pstInterCfg->bMute);

    return enRet;
}

static void _HAL_DISP_IF_SetInterCfgGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceParamGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e        enRet            = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg    = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDeviceContain = NULL;
    HAL_DISP_ST_DeviceParam_t *   pstDevParm       = NULL;

    pstDispCtxCfg    = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDeviceContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstDevParm       = (HAL_DISP_ST_DeviceParam_t *)pCfg;

    CamOsMemcpy(pstDevParm, &pstDeviceContain->stDeviceParam, sizeof(HAL_DISP_ST_DeviceParam_t));

    DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, Matrix:%s, Luma:%d, Contrast:%d, Hue:%d, Sat:%d, Sharp:%d\n", __FUNCTION__,
             __LINE__, (pstDevParm->stCsc.eCscMatrix), pstDevParm->stCsc.u32Luma, pstDevParm->stCsc.u32Contrast,
             pstDevParm->stCsc.u32Hue, pstDevParm->stCsc.u32Saturation, pstDevParm->u32Gain, pstDevParm->u32Sharpness);

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceParamGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceCapabilityGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e                enRet        = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_ST_DeviceCapabilityConfig_t *pstDevCapCfg = NULL;
    UNUSED(pCtx);

    pstDevCapCfg = (HAL_DISP_ST_DeviceCapabilityConfig_t *)pCfg;

    if (pstDevCapCfg->u32DevId < HAL_DISP_DEVICE_MAX)
    {
        if (pstDevCapCfg->u32DevId == HAL_DISP_DEVICE_ID_0)
        {
            pstDevCapCfg->stDevCapCfg.ePqType      = E_MI_DISP_DEV_PQ_MACE;
            pstDevCapCfg->stDevCapCfg.bWdmaSupport = HAL_DISP_DEVICE_0_SUPPORT_DMA;
            pstDevCapCfg->stDevCapCfg.u32VideoLayerStartOffset =
                HAL_DISP_MAPPING_VIDLAYERID_FROM_MI(HAL_DISP_VIDLAYER_ID_0);
            pstDevCapCfg->stDevCapCfg.u32VideoLayerHwCnt = HAL_DISP_DEVICE_0_VID_CNT;
        }
        else
        {
            enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
            DISP_ERR("%s %d, DevId is not support(%d)\n", __FUNCTION__, __LINE__, pstDevCapCfg->u32DevId);
        }

        DISP_DBG(DISP_DBG_LEVEL_HAL, "%s %d, DevId:%d, Pq:%s, WdmaSupport:%d, VideLayerCnt:%x\n", __FUNCTION__,
                 __LINE__, pstDevCapCfg->u32DevId, PARSING_HAL_DEV_PQ_TYPE(pstDevCapCfg->stDevCapCfg.ePqType),
                 pstDevCapCfg->stDevCapCfg.bWdmaSupport, pstDevCapCfg->stDevCapCfg.u32VideoLayerHwCnt);
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, DevId is not support(%d)\n", __FUNCTION__, __LINE__, pstDevCapCfg->u32DevId);
    }

    return enRet;
}

static void _HAL_DISP_IF_SetDeviceCapabilityGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoVidLayerCapabilityGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e                    enRet             = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_ST_VideoLayerCapabilityConfig_t *pstVidLayerCapCfg = NULL;
    UNUSED(pCtx);

    pstVidLayerCapCfg = (HAL_DISP_ST_VideoLayerCapabilityConfig_t *)pCfg;

    if (pstVidLayerCapCfg->eVidLayerType == E_MI_DISP_VIDEOLAYER_SINGLEWIN)
    {
        pstVidLayerCapCfg->stVidLayerCapCfg.bRotateSupport             = 0;
        pstVidLayerCapCfg->stVidLayerCapCfg.bCompressFmtSupport        = 0;
        pstVidLayerCapCfg->stVidLayerCapCfg.u32InputPortHwCnt          = 1;
        pstVidLayerCapCfg->stVidLayerCapCfg.u32InputPortPitchAlignment = 16;
        pstVidLayerCapCfg->stVidLayerCapCfg.u32RotateHeightAlighment   = 32;
        pstVidLayerCapCfg->stVidLayerCapCfg.u32LayerTypeHwCnt          = 1;
    }

    if (pstVidLayerCapCfg->eVidLayerType >= E_MI_DISP_VIDEOLAYER_TYPE)
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, UnSupport VideoLayerId:%x \n", __FUNCTION__, __LINE__, pstVidLayerCapCfg->eVidLayerType);
    }

    if (enRet == E_HAL_DISP_ST_QUERY_RET_OK)
    {
        DISP_DBG(DISP_DBG_LEVEL_HAL,
                 "%s %d, VidLayerType:%x, Rotate:%d, InputPorHwCnt:%d, LayerTypeHwCnt:%d, PitchAlight:%d, "
                 "RotateHeightAlign:%d\n",
                 __FUNCTION__, __LINE__, pstVidLayerCapCfg->eVidLayerType,
                 pstVidLayerCapCfg->stVidLayerCapCfg.bRotateSupport,
                 pstVidLayerCapCfg->stVidLayerCapCfg.u32InputPortHwCnt,
                 pstVidLayerCapCfg->stVidLayerCapCfg.u32LayerTypeHwCnt,
                 pstVidLayerCapCfg->stVidLayerCapCfg.u32InputPortPitchAlignment,
                 pstVidLayerCapCfg->stVidLayerCapCfg.u32RotateHeightAlighment);
    }

    return enRet;
}

static void _HAL_DISP_IF_SetVidLayerCapabilityGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDeviceInterfaceCapabilityGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e                   enRet              = E_HAL_DISP_ST_QUERY_RET_OK;
    HAL_DISP_ST_InterfaceCapabilityConfig_t *pstInterfaceCapCfg = NULL;
    UNUSED(pCtx);

    pstInterfaceCapCfg = (HAL_DISP_ST_InterfaceCapabilityConfig_t *)pCfg;

    if (pstInterfaceCapCfg->u32Interface & HAL_DISP_NOTSUPPORT_INTF)
    {
        pstInterfaceCapCfg->stInterfaceCapCfg.u32HwCount = 0;
        CamOsMemset(pstInterfaceCapCfg->stInterfaceCapCfg.bSupportTiming, 0, sizeof(MI_U8) * E_MI_DISP_OUTPUT_MAX);
    }
    else
    {
        pstInterfaceCapCfg->stInterfaceCapCfg.u32HwCount = 1;
        CamOsMemset(pstInterfaceCapCfg->stInterfaceCapCfg.bSupportTiming, 0, sizeof(MI_U8) * E_MI_DISP_OUTPUT_MAX);
        pstInterfaceCapCfg->stInterfaceCapCfg.bSupportTiming[E_MI_DISP_OUTPUT_USER] = 1;
    }
    return enRet;
}

static void _HAL_DISP_IF_SetDeviceInterfaceCapabilityGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoDmaCapabilityGet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e enRet = E_HAL_DISP_ST_QUERY_RET_OK;

    return enRet;
}

static void _HAL_DISP_IF_SetDmaCapabilityGet(void *pCtx, void *pCfg) {}

static HAL_DISP_ST_QueryRet_e _HAL_DISP_IF_GetInfoPnlUnifiedParamSet(void *pCtx, void *pCfg)
{
    HAL_DISP_ST_QueryRet_e                    enRet            = E_HAL_DISP_ST_QUERY_RET_OK;
    DRV_DISP_CTX_Config_t *                   pstDispCtxCfg    = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstPnlUdParamCfg = NULL;
    DRV_DISP_CTX_DeviceContain_t *            pstDevContain    = NULL;

    pstDispCtxCfg    = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain    = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstPnlUdParamCfg = (MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *)pCfg;

    if (HAL_DISP_PNL_IS_SUPPORTED_LINKTYPE(pstDevContain->u32Interface))
    {
        CamOsMemcpy(&pstDevContain->stPnlUdParamCfg, pstPnlUdParamCfg,
                    sizeof(MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t));
    }
    else
    {
        enRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Not support LinkType: %s\n", __FUNCTION__, __LINE__,
                 PARSING_HAL_INTERFACE(pstDevContain->u32Interface));
    }

    return enRet;
}

static void _HAL_DISP_IF_SetPnlUnifiedParamSet(void *pCtx, void *pCfg)
{
    // DRV_DISP_CTX_Config_t *                   pstDispCtxCfg = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstUfdParamCfg = NULL;
    // DRV_DISP_CTX_DeviceContain_t *            pstDevContain = NULL;

    // pstDispCtxCfg  = (DRV_DISP_CTX_Config_t *)pCtx;
    // pstDevContain  = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstUfdParamCfg = (MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *)pCfg;

    DISP_EXEC_LCD(HAL_DISP_LcdSetUnifiedPrototypeConfig(pCtx));

    // Set timing
    if (pstUfdParamCfg->u8TgnTimingFlag)
    {
        HAL_DISP_IF_SetUnifiedLpllConfig(pCtx);
    }

    // Set polarity
    if (pstUfdParamCfg->u8TgnPolarityFlag)
    {
        DISP_EXEC_LCD(HAL_DISP_LcdSetUnifiedPolarityConfig(pCtx));
    }

    // Set swap channel
    if (pstUfdParamCfg->u8TgnRgbSwapFlag)
    {
        DISP_EXEC_LCD(HAL_DISP_LcdSetUnifiedSwapChnConfig(pCtx));
    }

    // Set SSC
    if (pstUfdParamCfg->u8TgnSscFlag)
    {
        DISP_EXEC_LCD(HAL_DISP_LpllSetSscEn(1));
        DISP_EXEC_LCD(HAL_DISP_LpllSetSscConfigEx(&pstUfdParamCfg->stTgnSscInfo));
    }
    else
    {
        DISP_EXEC_LCD(HAL_DISP_LpllSetSscEn(0));
    }

    // RgbDeltaMode
    if (pstUfdParamCfg->u8RgbDeltaMdFlag && HAL_DISP_PNL_IS_SUPPORTED_LINKTYPE(HAL_DISP_INTF_SRGB))
    {
        DISP_EXEC_LCD(HAL_DISP_LcdSetUnifiedRgbDeltaMode(pCtx));
    }

    // Set PADMUX
    if (pstUfdParamCfg->u8TgnPadMuxFlag)
    {
        DISP_EXEC_LCD(HAL_DISP_LcdSetPadMux(pCtx));
    }
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------

static MI_U8 _HAL_DISP_IF_GetCallBack(DRV_DISP_CTX_Config_t *pstDispCfg, HAL_DISP_ST_QueryConfig_t *pstQueryCfg)
{
    CamOsMemset(&pstQueryCfg->stOutCfg, 0, sizeof(HAL_DISP_ST_QueryOutConfig_t));

    if (pstQueryCfg->stInCfg.u32CfgSize != g_pDispCbTbl[pstQueryCfg->stInCfg.enQueryType].u16CfgSize)
    {
        pstQueryCfg->stOutCfg.enQueryRet = E_HAL_DISP_ST_QUERY_RET_CFGERR;
        DISP_ERR("%s %d, Query:%s, Size %d != %d\n", __FUNCTION__, __LINE__,
                 PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType), pstQueryCfg->stInCfg.u32CfgSize,
                 g_pDispCbTbl[pstQueryCfg->stInCfg.enQueryType].u16CfgSize);
    }
    else
    {
        pstQueryCfg->stOutCfg.pSetFunc = g_pDispCbTbl[pstQueryCfg->stInCfg.enQueryType].pSetFunc;

        if (pstQueryCfg->stOutCfg.pSetFunc == NULL)
        {
            pstQueryCfg->stOutCfg.enQueryRet = E_HAL_DISP_ST_QUERY_RET_NOTSUPPORT;
            DISP_ERR("%s %d, Query:%s, SetFunc Empty\n", __FUNCTION__, __LINE__,
                     PARSING_HAL_QUERY_TYPE(pstQueryCfg->stInCfg.enQueryType));
        }
        else
        {
            pstQueryCfg->stOutCfg.enQueryRet =
                g_pDispCbTbl[pstQueryCfg->stInCfg.enQueryType].pGetInfoFunc(pstDispCfg, pstQueryCfg->stInCfg.pInCfg);
        }
    }

    return 1;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

MI_U8 HAL_DISP_IF_Init(void)
{
    void *pNull;

    if (g_bDispHwIfInit)
    {
        return 1;
    }

    pNull = NULL;

    CamOsMemset(g_pDispCbTbl, 0, sizeof(HAL_DISP_QueryCallBackFunc_t) * E_HAL_DISP_ST_QUERY_MAX);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_INIT].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_INIT].pSetFunc     = _HAL_DISP_IF_SetDeviceInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_INIT].u16CfgSize   = 0;

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_DEINIT].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceDeInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_DEINIT].pSetFunc     = _HAL_DISP_IF_SetDeviceDeInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_DEINIT].u16CfgSize   = 0;

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_ENABLE].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceEnable;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_ENABLE].pSetFunc     = _HAL_DISP_IF_SetDeviceEnable;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_ENABLE].u16CfgSize   = sizeof(MI_U8);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_BACKGROUND_COLOR].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceBackGroundColor;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_BACKGROUND_COLOR].pSetFunc     = _HAL_DISP_IF_SetDeviceBackGroundColor;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_BACKGROUND_COLOR].u16CfgSize   = sizeof(MI_U32);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_INTERFACE].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceInterface;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_INTERFACE].pSetFunc     = _HAL_DISP_IF_SetDeviceInterface;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_INTERFACE].u16CfgSize   = sizeof(MI_U32);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_OUTPUTTIMING].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceOutputTiming;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_OUTPUTTIMING].pSetFunc     = _HAL_DISP_IF_SetDeviceOutputTiming;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_OUTPUTTIMING].u16CfgSize   = sizeof(HAL_DISP_ST_DeviceTimingInfo_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_PARAM].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceParam;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_PARAM].pSetFunc     = _HAL_DISP_IF_SetDeviceParam;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_PARAM].u16CfgSize   = sizeof(HAL_DISP_ST_DeviceParam_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_DISPLAY_INFO].pGetInfoFunc = _HAL_DISP_IF_GetInfoDevicDisplayInfo;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_DISPLAY_INFO].pSetFunc     = _HAL_DISP_IF_SetDeviceDisplayInfo;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_DISPLAY_INFO].u16CfgSize   = sizeof(MHAL_DISP_DisplayInfo_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_INIT].pGetInfoFunc = _HAL_DISP_IF_GetInfoVidLayerInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_INIT].pSetFunc     = _HAL_DISP_IF_SetVidLayerInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_INIT].u16CfgSize   = 0;

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_ENABLE].pGetInfoFunc = _HAL_DISP_IF_GetInfoVidLayerEnable;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_ENABLE].pSetFunc     = _HAL_DISP_IF_SetVidLayerEnable;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_ENABLE].u16CfgSize   = sizeof(MI_U8);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_BIND].pGetInfoFunc = _HAL_DISP_IF_GetInfoVidLayerBind;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_BIND].pSetFunc     = _HAL_DISP_IF_SetVidLayerBind;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_BIND].u16CfgSize   = sizeof(pNull);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_UNBIND].pGetInfoFunc = _HAL_DISP_IF_GetInfoVidLayerUnBind;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_UNBIND].pSetFunc     = _HAL_DISP_IF_SetVidLayerUnBind;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_UNBIND].u16CfgSize   = sizeof(pNull);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_ATTR].pGetInfoFunc = _HAL_DISP_IF_GetInfoVidLayerAttr;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_ATTR].pSetFunc     = _HAL_DISP_IF_SetVidLayerAttr;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_ATTR].u16CfgSize   = sizeof(MI_DISP_VideoLayerAttr_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_PRIORITY].pGetInfoFunc = _HAL_DISP_IF_GetInfoVidLayerPriority;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_PRIORITY].pSetFunc     = _HAL_DISP_IF_SetVidLayerPriority;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDEOLAYER_PRIORITY].u16CfgSize   = sizeof(MI_U32);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_INIT].pGetInfoFunc = _HAL_DISP_IF_GetInfoInputPortInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_INIT].pSetFunc     = _HAL_DISP_IF_SetInputPortInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_INIT].u16CfgSize   = 0;

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_ENABLE].pGetInfoFunc = _HAL_DISP_IF_GetInfoInputPortEnable;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_ENABLE].pSetFunc     = _HAL_DISP_IF_SetInputPortEnable;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_ENABLE].u16CfgSize   = sizeof(MI_U8);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_ATTR].pGetInfoFunc = _HAL_DISP_IF_GetInfoInputPortAttr;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_ATTR].pSetFunc     = _HAL_DISP_IF_SetInputPortAttr;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_ATTR].u16CfgSize   = sizeof(MI_DISP_InputPortAttr_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_FLIP].pGetInfoFunc = _HAL_DISP_IF_GetInfoInputPortFlip;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_FLIP].pSetFunc     = _HAL_DISP_IF_SetInputPortFlip;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_FLIP].u16CfgSize   = sizeof(MI_DISP_IMPL_MhalVideoFrameData_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_CROP].pGetInfoFunc = _HAL_DISP_IF_GetInfoInputPortCrop;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_CROP].pSetFunc     = _HAL_DISP_IF_SetInputPortCrop;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INPUTPORT_CROP].u16CfgSize   = sizeof(MI_DISP_VidWinRect_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_SET].pGetInfoFunc = _HAL_DISP_IF_GetInfoClkSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_SET].pSetFunc     = _HAL_DISP_IF_SetClkSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_SET].u16CfgSize   = sizeof(HAL_DISP_ST_ClkConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoClkGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_GET].pSetFunc     = _HAL_DISP_IF_SetClkGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_GET].u16CfgSize   = sizeof(HAL_DISP_ST_ClkConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DRVTURNING_SET].pGetInfoFunc = _HAL_DISP_IF_GetInfoDrvTurningSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DRVTURNING_SET].pSetFunc     = _HAL_DISP_IF_SetDrvTurningSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DRVTURNING_SET].u16CfgSize   = sizeof(HAL_DISP_ST_DrvTurningConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DRVTURNING_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoDrvTurningGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DRVTURNING_GET].pSetFunc     = _HAL_DISP_IF_SetDrvTurningGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DRVTURNING_GET].u16CfgSize   = sizeof(HAL_DISP_ST_DrvTurningConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DBGMG_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoDbgmgGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DBGMG_GET].pSetFunc     = _HAL_DISP_IF_SetDbgmgGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DBGMG_GET].u16CfgSize   = sizeof(HAL_DISP_ST_DbgmgConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_ACCESS].pGetInfoFunc = _HAL_DISP_IF_GetInfoRegAccess;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_ACCESS].pSetFunc     = _HAL_DISP_IF_SetRegAccess;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_ACCESS].u16CfgSize   = sizeof(MI_DISP_IMPL_MhalRegAccessConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_FLIP].pGetInfoFunc = _HAL_DISP_IF_GetInfoRegFlip;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_FLIP].pSetFunc     = _HAL_DISP_IF_SetRegFlip;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_FLIP].u16CfgSize   = sizeof(MI_DISP_IMPL_MhalRegFlipConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_WAITDONE].pGetInfoFunc = _HAL_DISP_IF_GetInfoRegWaitDone;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_WAITDONE].pSetFunc     = _HAL_DISP_IF_SetRegWaitDone;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_REG_WAITDONE].u16CfgSize   = sizeof(MI_DISP_IMPL_MhalRegWaitDoneConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_HW_INFO].pGetInfoFunc = _HAL_DISP_IF_GetInfoHwInfoConfig;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_HW_INFO].pSetFunc     = _HAL_DISP_IF_SetHwInfoConfig;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_HW_INFO].u16CfgSize   = sizeof(HAL_DISP_ST_HwInfoConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_INIT].pGetInfoFunc = _HAL_DISP_IF_GetInfoClkInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_INIT].pSetFunc     = _HAL_DISP_IF_SetClkInit;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_CLK_INIT].u16CfgSize   = sizeof(HAL_DISP_ST_ClkInitConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERCFG_SET].pGetInfoFunc = _HAL_DISP_IF_GetInfoInterCfgSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERCFG_SET].pSetFunc     = _HAL_DISP_IF_SetInterCfgSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERCFG_SET].u16CfgSize   = sizeof(MI_DISP_IMPL_MhalDeviceConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERCFG_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoInterCfgGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERCFG_GET].pSetFunc     = _HAL_DISP_IF_SetInterCfgGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERCFG_GET].u16CfgSize   = sizeof(MI_DISP_IMPL_MhalDeviceConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceParamGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET].pSetFunc     = _HAL_DISP_IF_SetDeviceParamGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_PARAM_GET].u16CfgSize   = sizeof(HAL_DISP_ST_DeviceParam_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_CAPABILITY_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoDeviceCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_CAPABILITY_GET].pSetFunc     = _HAL_DISP_IF_SetDeviceCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DEVICE_CAPABILITY_GET].u16CfgSize   = sizeof(HAL_DISP_ST_DeviceCapabilityConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDLAYER_CAPABILITY_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoVidLayerCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDLAYER_CAPABILITY_GET].pSetFunc     = _HAL_DISP_IF_SetVidLayerCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_VIDLAYER_CAPABILITY_GET].u16CfgSize =
        sizeof(HAL_DISP_ST_VideoLayerCapabilityConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERFACE_CAPABILITY_GET].pGetInfoFunc =
        _HAL_DISP_IF_GetInfoDeviceInterfaceCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERFACE_CAPABILITY_GET].pSetFunc = _HAL_DISP_IF_SetDeviceInterfaceCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_INTERFACE_CAPABILITY_GET].u16CfgSize =
        sizeof(HAL_DISP_ST_InterfaceCapabilityConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DMA_CAPABILITY_GET].pGetInfoFunc = _HAL_DISP_IF_GetInfoDmaCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DMA_CAPABILITY_GET].pSetFunc     = _HAL_DISP_IF_SetDmaCapabilityGet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_DMA_CAPABILITY_GET].u16CfgSize   = sizeof(HAL_DISP_ST_DmaCapabilitytConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_SET].pGetInfoFunc = _HAL_DISP_IF_GetInfoPnlUnifiedParamSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_SET].pSetFunc     = _HAL_DISP_IF_SetPnlUnifiedParamSet;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_SET].u16CfgSize =
        sizeof(MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t);

    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_GET].pGetInfoFunc = NULL;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_GET].pSetFunc     = NULL;
    g_pDispCbTbl[E_HAL_DISP_ST_QUERY_PNL_UNIFIED_PARAM_GET].u16CfgSize =
        sizeof(MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t);

    g_bDispHwIfInit = 1;

    return 1;
}

MI_U8 HAL_DISP_IF_DeInit(void)
{
    if (g_bDispHwIfInit == 0)
    {
        DISP_ERR("%s %d, HalIf not init\n", __FUNCTION__, __LINE__);
        return 0;
    }
    g_bDispHwIfInit = 0;
    CamOsMemset(g_pDispCbTbl, 0, sizeof(HAL_DISP_QueryCallBackFunc_t) * E_HAL_DISP_ST_QUERY_MAX);

    return 1;
}

MI_U8 HAL_DISP_IF_Query(void *pCtx, void *pCfg)
{
    MI_U8 bRet = 1;

    if (pCtx == NULL)
    {
        DISP_ERR("%s %d, Input Ctx is Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else if (pCfg == NULL)
    {
        DISP_ERR("%s %d, Input Cfg is Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = _HAL_DISP_IF_GetCallBack((DRV_DISP_CTX_Config_t *)pCtx, (HAL_DISP_ST_QueryConfig_t *)pCfg);
    }

    return bRet;
}

MI_U8 HAL_DISP_IF_ParseFunc(MI_U8 *ps8FuncType, MI_DISP_IMPL_MhalDeviceConfig_t *pstCfg, MI_U8 u8Val)
{
    MI_U8 u8Ret = 0;

    if (DRV_DISP_OS_StrCmp(ps8FuncType, "bootlogo") == 0)
    {
        pstCfg->eType       = E_MI_DISP_DEV_CFG_BOOTLOGO;
        pstCfg->bBootlogoEn = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "colorid") == 0)
    {
        pstCfg->eType     = E_MI_DISP_DEV_CFG_COLORID;
        pstCfg->u8ColorId = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "gopblendid") == 0)
    {
        pstCfg->eType        = E_MI_DISP_DEV_CFG_GOPBLENDID;
        pstCfg->u8GopBlendId = u8Val;
    }
#if 0
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "disptestcb") == 0)
    {
        pstCfg->eType       = E_MI_DISP_DEV_CFG_DISP_PAT;
        pstCfg->bDispTestCb = (u8Val == 1 || u8Val == 2) ? 1 : 0;
        pstCfg->bDispGray   = (u8Val == 2) ? 1 : 0;
        pstCfg->bFrameColor = (u8Val == 3) ? 1 : 0;
    }
#endif
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "disppatctx") == 0)
    {
        pstCfg->eType    = E_MI_DISP_DEV_CFG_DISP_PAT;
        pstCfg->bDispPat = u8Val ? 1 : 0;
        pstCfg->u8PatMd  = (u8Val == 1) ? 0x7 : u8Val;
        pstCfg->bCtx     = 1;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstmop") == 0)
    {
        pstCfg->eType   = E_MI_DISP_DEV_CFG_RST_MOP;
        pstCfg->bRstMop = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstdisp") == 0)
    {
        pstCfg->eType    = E_MI_DISP_DEV_CFG_RST_DISP;
        pstCfg->bRstDisp = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstdac") == 0)
    {
        pstCfg->eType   = E_MI_DISP_DEV_CFG_RST_DAC;
        pstCfg->bRstDac = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rsthdmitx") == 0)
    {
        pstCfg->eType      = E_MI_DISP_DEV_CFG_RST_HDMITX;
        pstCfg->bRstHdmitx = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstlcd") == 0)
    {
        pstCfg->eType   = E_MI_DISP_DEV_CFG_RST_LCD;
        pstCfg->bRstLcd = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstdacaff") == 0)
    {
        pstCfg->eType      = E_MI_DISP_DEV_CFG_RST_DACAFF;
        pstCfg->bRstDacAff = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstdacsyn") == 0)
    {
        pstCfg->eType      = E_MI_DISP_DEV_CFG_RST_DACSYN;
        pstCfg->bRstDacSyn = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstfpll") == 0)
    {
        pstCfg->eType    = E_MI_DISP_DEV_CFG_RST_FPLL;
        pstCfg->bRstFpll = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "rstdma") == 0)
    {
        pstCfg->eType = E_MI_DISP_DEV_CFG_NONE;
        u8Ret         = 1;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "crc16md") == 0)
    {
        pstCfg->eType = E_MI_DISP_DEV_CFG_NONE;
        u8Ret         = 1;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "cscen") == 0)
    {
        pstCfg->eType  = E_MI_DISP_DEV_CFG_CSC_EN;
        pstCfg->bCscEn = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "mopythrd") == 0)
    {
        pstCfg->eType      = E_MI_DISP_DEV_CFG_MOP_Y_THRD;
        pstCfg->u8MopYThrd = u8Val;
    }
    else if (DRV_DISP_OS_StrCmp(ps8FuncType, "mopcthrd") == 0)
    {
        pstCfg->eType      = E_MI_DISP_DEV_CFG_MOP_C_THRD;
        pstCfg->u8MopCThrd = u8Val;
    }
    else
    {
        u8Ret = 1;
    }

    return u8Ret;
}
