/*
 * hal_disp.c- Sigmastar
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
#define _HAL_DISP_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mi_disp.h"
#include "mi_disp_impl_datatype.h"
#include "hal_disp_color.c"
#include "hal_disp_utility.c"
#include "hal_disp_clk.c"
#include "hal_disp_picture.c"

#include "drv_disp_if.h"
#include "drv_disp_ctx.h"
#include "disp_debug.h"
#include "hal_disp_chip.h"
#include "hal_disp.h"
#include "hal_disp_st.h"
#include "hal_disp_irq.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define HAL_DISP_SGRDMA_C_OFFSET (0x40)

#define HAL_DISP_HW_SUPPORT_SGRDMA_STRIDE_ALIGN(fmt)            \
    (fmt == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV               ? 8  \
     : fmt == E_MI_SYS_PIXEL_FRAME_RGB565                  ? 8  \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422      ? 16 \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420      ? 16 \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21 ? 16 \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU             ? 8  \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY             ? 8  \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY             ? 8  \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR           ? 32 \
     : fmt == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR           ? 32 \
                                                           : 4)
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_COLOR_RGB565 = 0,
    E_HAL_DISP_COLOR_RGB888,
} HAL_DISP_ColorFmt_e;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
HAL_DISP_ST_PnlLpllTbl_t g_lpllSettingTbl[E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX][HAL_DISP_PNL_LPLL_REG_MAX] = {
    {
        // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ   NO.0
        // Address,Value
        {0x103784, 0x0041},
        {0x103780, 0x2201},
        {0x103782, 0x0420},
        {0x103786, 0x0000},
        {0x103784, 0x0080}, // div8
    },

    {
        // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ      NO.1
        // Address,Value
        {0x103784, 0x0042},
        {0x103780, 0x2201},
        {0x103782, 0x0420},
        {0x103786, 0x0001},
        {0x103784, 0x0081}, // div16
    },

    {
        // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ       NO.2
        // Address,Value
        {0x103784, 0x0043},
        {0x103780, 0x2201},
        {0x103782, 0x0420},
        {0x103786, 0x0002},
        {0x103784, 0x0082}, // div32
    },

    {
        // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ     NO.3
        // Address,Value
        {0x103784, 0x0083},
        {0x103780, 0x2201},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103784, 0x0083}, // div64
    },

    {
        // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ    NO.4
        // Address,Value
        {0x103784, 0x00D2},
        {0x103780, 0x2301},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103784, 0x00D2}, // div130
    },

    {
        // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ    NO.5
        // Address,Value
        {0x103784, 0x00D3},
        {0x103780, 0x2301},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103784, 0x00D3}, // div260
    },

    {
        // E_HAL_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ    NO.6
        // Address,Value
        {0x103784, 0x00F3},
        {0x103780, 0x2301},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103784, 0x00F3}, // div300
    },
};

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static MI_U32 _HAL_DISP_GetPlaneNum(MI_SYS_PixelFormat_e eFmt)
{
    MI_U32 u32PlaneNum = 1;

    u32PlaneNum = (eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)               ? 1
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU)             ? 1
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY)             ? 1
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY)             ? 1
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)      ? 2
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21) ? 2
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)      ? 2
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR)           ? 3
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR)           ? 3
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_ARGB8888)                ? 1
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_ABGR8888)                ? 1
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_BGRA8888)                ? 1
                  : (eFmt == E_MI_SYS_PIXEL_FRAME_RGB565)                  ? 1
                                                                           : 0;
    if (u32PlaneNum == 0)
    {
        DISP_ERR("%s %d, Not Support This Color Format: %d\n", __FUNCTION__, __LINE__, eFmt);
    }

    return u32PlaneNum;
}

static void _HAL_DISP_SetTgenModeEn(void)
{
    W2BYTEMSK(REG_DISP_TTL_00_L, DISP_BIT8, DISP_BIT8);
}

static void _HAL_DISP_GetSrRdma128BitsDiv(MI_SYS_PixelFormat_e eFmt, MI_U8 u8PlaneId, MI_U32 *pu32DivW,
                                          MI_U32 *pu32DivH)
{
    if (eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV || eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU
        || eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY || eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY
        || eFmt == E_MI_SYS_PIXEL_FRAME_RGB565)
    {
        *pu32DivW = (u8PlaneId == 0) ? 8 : 0;
        *pu32DivH = (u8PlaneId == 0) ? 1 : 0;
    }
    else if (eFmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        *pu32DivW = (u8PlaneId == 0) ? 16 : (u8PlaneId == 1) ? 16 : 0;
        *pu32DivH = (u8PlaneId == 0) ? 1 : (u8PlaneId == 1) ? 1 : 0;
    }
    else if (eFmt == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR)
    {
        *pu32DivW = (u8PlaneId == 0) ? 16 : (u8PlaneId == 1) ? 32 : (u8PlaneId == 2) ? 32 : 0;
        *pu32DivH = (u8PlaneId == 0) ? 1 : (u8PlaneId == 1) ? 1 : (u8PlaneId == 2) ? 1 : 0;
    }
    else if (eFmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 || eFmt == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21)
    {
        *pu32DivW = (u8PlaneId == 0) ? 16 : (u8PlaneId == 1) ? 16 : 0;
        *pu32DivH = (u8PlaneId == 0) ? 1 : (u8PlaneId == 1) ? 2 : 0;
    }
    else if (eFmt == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR)
    {
        *pu32DivW = (u8PlaneId == 0) ? 16 : (u8PlaneId == 1) ? 32 : (u8PlaneId == 2) ? 32 : 0;
        *pu32DivH = (u8PlaneId == 0) ? 1 : (u8PlaneId == 1) ? 2 : (u8PlaneId == 2) ? 2 : 0;
    }
    else if (eFmt == E_MI_SYS_PIXEL_FRAME_ARGB8888 || eFmt == E_MI_SYS_PIXEL_FRAME_ABGR8888
             || eFmt == E_MI_SYS_PIXEL_FRAME_BGRA8888)
    {
        *pu32DivW = (u8PlaneId == 0) ? 4 : 0;
        *pu32DivH = (u8PlaneId == 0) ? 1 : 0;
    }
    else
    {
        DISP_ERR("%s %d, fmt %d, not support\n", __FUNCTION__, __LINE__, eFmt);

        *pu32DivW = 0;
        *pu32DivH = 0;
    }
}

static void _HAL_DISP_GetCmdqCtxByDevId(DRV_DISP_CTX_Config_t *pstDispCtx, MI_U32 u32DevId, void **pCmdqCtx)
{
    MI_S32 s32UtilityId = -1;

    ;
    if (HAL_DISP_GetUtilityIdByDevId(u32DevId, &s32UtilityId))
    {
        *pCmdqCtx = pstDispCtx->pstCtxContain->pstHalHwContain->pvCmdqCtx[s32UtilityId];
    }
    else
    {
        *pCmdqCtx = NULL;
        DISP_ERR("%s %d, UnSupport DevId %d \n", __FUNCTION__, __LINE__, u32DevId);
    }
}

static MI_U8 _HAL_DISP_GetCmdqByDevCtx(DRV_DISP_CTX_Config_t *pstDevCtx, void **pCmdqCtx)
{
    MI_U8                         bRet          = 1;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;

    pstDevContain = pstDevCtx->pstCtxContain->pstDevContain[pstDevCtx->u32ContainIdx];
    _HAL_DISP_GetCmdqCtxByDevId(pstDevCtx, pstDevContain->u32DevId, pCmdqCtx);
    if (*pCmdqCtx == NULL)
    {
        bRet = 0;
        DISP_ERR("%s %d, ContainId%d, DevId:%d No Cmdq Handler Init\n", __FUNCTION__, __LINE__,
                 pstDevCtx->u32ContainIdx, pstDevContain->u32DevId);
    }

    return bRet;
}

static MI_U8 _HAL_DISP_GetCmdqByVideoLayerCtx(DRV_DISP_CTX_Config_t *pstVidlayerCtx, void **pCmdqCtx)
{
    MI_U8                             bRet                 = 1;
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain        = NULL;
    DRV_DISP_CTX_VideoLayerContain_t *pstVideoLayerContain = NULL;

    pstVideoLayerContain = pstVidlayerCtx->pstCtxContain->pstVidLayerContain[pstVidlayerCtx->u32ContainIdx];
    pstDevContain        = (DRV_DISP_CTX_DeviceContain_t *)pstVideoLayerContain->pstDevCtx;

    if (pstDevContain)
    {
        _HAL_DISP_GetCmdqCtxByDevId(pstVidlayerCtx, pstDevContain->u32DevId, pCmdqCtx);

        if (*pCmdqCtx == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, ContainId:%d, VideoLayerId:%d, DevId:%d No Cmdq Handler Init\n", __FUNCTION__, __LINE__,
                     pstVidlayerCtx->u32ContainIdx, pstVideoLayerContain->eVidLayerType, pstDevContain->u32DevId);
        }
    }
    else
    {
        *pCmdqCtx = NULL;
        bRet      = 0;
        DISP_ERR("%s %d, VideoLayer(%d) No Bind Device \n", __FUNCTION__, __LINE__, pstVidlayerCtx->u32ContainIdx);
    }

    return bRet;
}

static void _HAL_DISP_LpllDumpSetting(MI_U16 u16Idx, MI_U8 bDsi)
{
    MI_U16 u16StartIdx;
    MI_U16 u16EndIdx;
    MI_U16 u16RegIdx;

    u16StartIdx = bDsi ? 1 : 0;
    u16EndIdx   = u16StartIdx + HAL_DISP_PNL_LPLL_REG_NUM;

    DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, Idx:%d, Num(%d %d), bDsi:%d\n", __FUNCTION__, __LINE__, u16Idx,
             u16StartIdx, u16EndIdx, bDsi);

    if (u16Idx < E_HAL_DISP_PNL_SUPPORTED_LPLL_MAX)
    {
        for (u16RegIdx = u16StartIdx; u16RegIdx < u16EndIdx; u16RegIdx++)
        {
            if (g_lpllSettingTbl[u16Idx][u16RegIdx].address == 0xFFFFFFF)
            {
                // DrvSclOsDelayTask(g_lpllSettingTbl[u16Idx][u16RegIdx].value);
                continue;
            }

            HAL_DISP_UTILITY_W2BYTEDirect(g_lpllSettingTbl[u16Idx][u16RegIdx].address,
                                          g_lpllSettingTbl[u16Idx][u16RegIdx].value);
        }
    }
}

static void _HAL_DISP_LpllSetLpllSet(MI_U32 u32LpllSet)
{
    MI_U16 u16LpllSetLo, u16LpllSetHi;

    u16LpllSetLo = (MI_U16)(u32LpllSet & 0x0000FFFF);
    u16LpllSetHi = (MI_U16)((u32LpllSet & 0x00FF0000) >> 16);

    HAL_DISP_UTILITY_W2BYTEDirect(REG_DISP_LPLL_48_L, u16LpllSetLo);
    HAL_DISP_UTILITY_W2BYTEDirect(REG_DISP_LPLL_49_L, u16LpllSetHi);
}

/*static void _HAL_DISP_LcdSetRgbDtype(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_LCD_19_L, u16Val, 0x003F);
}*/

static void _HAL_DISP_LcdSetRgbR2YMode(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, DISP_BIT0, DISP_BIT0);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, u16Val ? 0x0000 : DISP_BIT0, DISP_BIT0);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, DISP_BIT1, DISP_BIT1);
}

static void _HAL_DISP_LcdSetBtEncMode(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_5E_L, 0x0000, DISP_BIT0);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_5E_L, u16Val ? DISP_BIT0 : 0x0000, DISP_BIT0);
}

static void _HAL_DISP_LcdSetBtEncModeEn(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, 0x0000, DISP_BIT11);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, u16Val ? DISP_BIT11 : 0x0000, DISP_BIT11);
}

static void _HAL_DISP_SetYuv444To422(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, 0x0000, DISP_BIT4);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, u16Val ? DISP_BIT4 : 0x0000, DISP_BIT4);
}

static void _HAL_DISP_SetBtModeSel(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, ((u16Val & 0x3) << 8), 0x300);
}

static void _HAL_DISP_SetTtlFmt(HAL_DISP_ColorFmt_e enFmt)
{
    switch (enFmt)
    {
        case E_HAL_DISP_COLOR_RGB565:
            HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_24_L, 0x3 << 14, DISP_BIT14 | DISP_BIT15);
            HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_24_L, 0x2 << 7,
                                             DISP_BIT8 | DISP_BIT7); // b00：RGB888 b01: RGB565
            break;
        case E_HAL_DISP_COLOR_RGB888:
            HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_24_L, 0, DISP_BIT14 | DISP_BIT15);
            HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_24_L, 0, DISP_BIT8 | DISP_BIT7);
            break;
        default:
            break;
    }
}

static void _HAL_DISP_SetBtModeAuto(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_57_L, u16Val ? DISP_BIT10 : 0x0000, DISP_BIT10);
}

static void _HAL_DISP_SetTtlMux(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_4A_L, u16Val ? DISP_BIT1 : 0x0000, DISP_BIT1);
}

static void _HAL_DISP_SetTgenFwVtt(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_08_L, u16Val, DISP_BIT0);
}

static void _HAL_DISP_SetBtMux(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_4A_L, u16Val ? DISP_BIT0 : 0x0000, DISP_BIT0);
}

static void _HAL_DISP_SetCkgPipeTtl(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_SC_GP_CTRL_26_L, u16Val, 0x000F);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_SC_GP_CTRL_26_L, u16Val ? 0x0000 : DISP_BIT4, 0x00F0);
}

static void _HAL_DISP_SetCkgPipeBt(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_SC_GP_CTRL_26_L, u16Val, 0x00FF);
}

/*static void _HAL_DISP_LcdPnlSetOdClkRate(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_49_L, u16Val, 0x000F);
}*/

static void _HAL_DISP_LcdSetRgbVsyncPol(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_48_L, 0x0000, DISP_BIT1);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_48_L, u16Val ? DISP_BIT1 : 0x0000, DISP_BIT1);
}

static void _HAL_DISP_LcdSetRgbHsyncPol(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_48_L, 0x0000, DISP_BIT2);
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_48_L, u16Val ? DISP_BIT2 : 0x0000, DISP_BIT2);
}

static void _HAL_DISP_LcdSetRgbClkPol(MI_U32 u32Interface, MI_U16 u16Val)
{
    if (u32Interface == HAL_DISP_INTF_TTL)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_48_L, 0x0000, DISP_BIT0);
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_48_L, u16Val ? DISP_BIT0 : 0x0000, DISP_BIT0);
    }
    else if (u32Interface == HAL_DISP_INTF_BT656)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_5E_L, 0x0000, DISP_BIT2);
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_5E_L, u16Val ? DISP_BIT2 : 0x0000, DISP_BIT2);
    }
}

static void _HAL_DISP_LcdPnlSetRgbSwap(MI_U8 enChR, MI_U8 enChG, MI_U8 enChB)
{
    MI_U16 u16R, u16G, u16B;

    u16B = enChB << 0;
    u16G = enChG << 2;
    u16R = enChR << 4;

    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_24_L, (u16R | u16G | u16B), 0x003F);
}

static void _HAL_DISP_LcdPnlSetRgbMlSwap(MI_U8 u8Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_TTL_24_L, u8Val ? DISP_BIT6 : 0x0000, DISP_BIT6);
}

static void _HAL_DISP_LpllSetSscEn(MI_U8 u8Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_LPLL_4E_L, u8Val ? 0x8000 : 0x0000, 0x8000);
}

static void _HAL_DISP_LpllSetSscSpan(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_LPLL_4F_L, u16Val, 0x3FFF);
}

static void _HAL_DISP_LpllSetSscStep(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_DISP_LPLL_4E_L, u16Val, 0x0FFF);
}

static void _HAL_DISP_LcdSetRgbDeltaMode(MI_U8 u8DummyMode, MI_DISP_IMPL_MhalPnlRgbDeltaMode_e eOddLine,
                                         MI_DISP_IMPL_MhalPnlRgbDeltaMode_e eEvenLine)
{
}

static void _HAL_DISP_LcdSetTTLPadMux(MI_U16 u16Val)
{
    HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_PADTOP_78_L, u16Val, 0x0003);
}

static void _HAL_DISP_LcdSetBTPadMux(MI_U32 u32Interface, MI_U16 u16Val)
{
    if (u32Interface == HAL_DISP_INTF_BT656)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_PADTOP_60_L, (u16Val & 0x03) << 4, 0x0030);
    }
    else if (u32Interface == HAL_DISP_INTF_BT601)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_PADTOP_72_L, (u16Val & 0x03) << 4, 0x0030);
    }
    else if (u32Interface == HAL_DISP_INTF_BT1120)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_PADTOP_72_L, (u16Val & 0x03), 0x0003);
    }
}

static void _HAL_DISP_LcdSetRGBPadMux(MI_DISP_IMPL_MhalPnlOutputFormatBitMode_e enOpFmtBitMode, MI_U16 u16Val)
{
    if (enOpFmtBitMode == E_MI_DISP_MHALPNL_OUTPUT_8BIT_MODE)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_PADTOP_74_L, (u16Val & 0x03) << 8, 0x0300);
    }
    else if (enOpFmtBitMode == E_MI_DISP_MHALPNL_OUTPUT_565BIT_MODE)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(REG_PADTOP_74_L, (u16Val & 0x01) << 12, 0x1000);
    }
}

static MI_U8 _HAL_DISP_GetCmdqByInputPortCtx(DRV_DISP_CTX_Config_t *pstInputPortCtx, void **pCmdqCtx)
{
    MI_U8                             bRet                = 1;
    DRV_DISP_CTX_DeviceContain_t *    pstDevContain       = NULL;
    DRV_DISP_CTX_VideoLayerContain_t *pstVidLayerContain  = NULL;
    DRV_DISP_CTX_InputPortContain_t * pstInputPortContain = NULL;

    pstInputPortContain = pstInputPortCtx->pstCtxContain->pstInputPortContain[pstInputPortCtx->u32ContainIdx];
    pstVidLayerContain  = (DRV_DISP_CTX_VideoLayerContain_t *)pstInputPortContain->pstVidLayerContain;
    if (pstVidLayerContain)
    {
        pstDevContain = (DRV_DISP_CTX_DeviceContain_t *)pstVidLayerContain->pstDevCtx;
        if (pstDevContain)
        {
            _HAL_DISP_GetCmdqCtxByDevId(pstInputPortCtx, pstDevContain->u32DevId, pCmdqCtx);

            if (*pCmdqCtx == NULL)
            {
                bRet = 0;
                DISP_ERR("%s %d, ContainId:%d, InputportId:%d, VideoayerId:%d, DevId:%d, No Cmdq Handler Init\n",
                         __FUNCTION__, __LINE__, pstInputPortCtx->u32ContainIdx, pstInputPortContain->u32PortId,
                         pstVidLayerContain->eVidLayerType, pstDevContain->u32DevId);
            }
        }
        else
        {
            bRet      = 0;
            *pCmdqCtx = NULL;
            DISP_ERR("%s %d, VideoLayer(%d) No Bind Device \n", __FUNCTION__, __LINE__,
                     pstVidLayerContain->eVidLayerType);
        }
    }
    else
    {
        bRet      = 0;
        *pCmdqCtx = NULL;
        DISP_ERR("%s %d, Inputport(%d), VideoLayerContain is NULL\n", __FUNCTION__, __LINE__,
                 pstInputPortCtx->u32ContainIdx);
    }

    return bRet;
}
//-------------------------------------------------------------------------------
// Global Functions
//-------------------------------------------------------------------------------

MI_U8 HAL_DISP_GetCmdqByCtx(void *pCtx, void **pCmdqCtx)
{
    DRV_DISP_CTX_Config_t *pstDispCtx = (DRV_DISP_CTX_Config_t *)pCtx;
    MI_U8                  bRet       = 1;

    if (pCtx == NULL || pstDispCtx->pstCtxContain == NULL)
    {
        *pCmdqCtx = NULL;
        return 0;
    }
    if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_DEVICE)
    {
        bRet = _HAL_DISP_GetCmdqByDevCtx(pstDispCtx, pCmdqCtx);
    }
    else if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_VIDLAYER)
    {
        bRet = _HAL_DISP_GetCmdqByVideoLayerCtx(pstDispCtx, pCmdqCtx);
    }
    else if (pstDispCtx->enCtxType == E_DRV_DISP_CTX_TYPE_INPUTPORT)
    {
        bRet = _HAL_DISP_GetCmdqByInputPortCtx(pstDispCtx, pCmdqCtx);
    }
    else
    {
        bRet      = 0;
        *pCmdqCtx = NULL;
        DISP_ERR("%s %d, Unknown Ctx Type %d\n", __FUNCTION__, __LINE__, pstDispCtx->enCtxType);
    }
    return bRet;
}

MI_U8 HAL_DISP_GetUtilityIdByDevId(MI_U32 u32DevId, MI_S32 *ps32UtilityId)
{
    MI_U8 bRet = 1;

    *ps32UtilityId = u32DevId == HAL_DISP_DEVICE_ID_0 ? HAL_DISP_UTILITY_CMDQ_ID_DEVICE0 : -1;
    if (*ps32UtilityId < 0)
    {
        DISP_ERR("%s %d, UnSupport DevId: %d\n", __FUNCTION__, __LINE__, u32DevId);
        bRet = 0;
    }

    return bRet;
}

MI_U32 HAL_DISP_GetDeviceId(void *pstCtx, MI_U8 u8bDevContain)
{
    DRV_DISP_CTX_Config_t *       pstDispCtx    = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    MI_U32                        u32Dev;

    if (u8bDevContain)
    {
        pstDevContain = pstCtx;
        u32Dev        = GET_DISP_ID_BYDEVCONTAIN(pstDevContain);
    }
    else
    {
        pstDispCtx = pstCtx;
        u32Dev     = GET_DISP_ID(pstDispCtx);
    }
    return u32Dev;
}

MI_U16 HAL_DISP_GetDeviceTimeGenStart(void *pstCtx, MI_U32 u32DevId)
{
    MI_U16 u16TimeGenStart = 0;
    MI_S32 u32UtilId;
    UNUSED(pstCtx);

    HAL_DISP_GetUtilityIdByDevId(u32DevId, &u32UtilId);
    if (g_stDispIrqHist.stWorkingStatus.stDevStatus[u32DevId].u8Deviceused
        && g_stDispIrqHist.stWorkingStatus.stDevStatus[u32DevId].u8bStartTimegen)
    {
        u16TimeGenStart = HAL_DISP_UTILITY_R2BYTEMaskDirect(REG_HAL_DISP_UTILIYT_CMDQDEV_TIMEGEN_START(u32UtilId),
                                                            REG_CMDQ_DEV_TIMEGEN_START_MSK);
    }
    return u16TimeGenStart;
}

void HAL_DISP_SetSwReset(MI_U8 u8Val, void *pCtx, MI_U32 u32DevId)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;
    UNUSED(u32DevId);

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_00_L;

    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, (MI_U16)u8Val, 0x00FF, pCmdqCtx);
    }
    else if (u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(u32Reg, (MI_U16)u8Val, 0x00FF);
    }
}

void HAL_DISP_SetInterCfgDispPat(MI_U8 u8DispPat, MI_U8 u8PatMd, void *pCtx)
{
    void *pCmdqCtx = NULL;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    // Maybe RDMA pattern gen need set CSC y2r reg(0x19 b0).
    // HalDispUtilityW2BYTEMSK(REG_DISP_TTL_19_L, u8DispTestCb ? DISP_BIT0: 0, DISP_BIT0, pCmdqCtx);
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_27_L, u8DispPat ? DISP_BIT4 : 0, DISP_BIT4, pCmdqCtx);
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_27_L, u8PatMd, DISP_BIT0 | DISP_BIT1 | DISP_BIT2 | DISP_BIT3, pCmdqCtx);
}

void HAL_DISP_SetTgenFwVtt(MI_U16 u16Val, void *pCtx)
{
    void *pCmdqCtx = NULL;
    u32   u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_08_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, DISP_BIT0, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenHtt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_0F_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenVtt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_07_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenHsyncSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_09_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenHsyncEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_0A_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenVsyncSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_01_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenVsyncEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_02_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenHfdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_0B_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenHfdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_0C_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenVfdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_03_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenVfdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_04_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenHdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_0D_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenHdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_0E_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenVdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_05_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenVdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_06_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetFrameColor(MI_U8 u8R, MI_U8 u8G, MI_U8 u8B, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;
    MI_U16 u16Val;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    u16Val = u8B;
    u32Reg = REG_DISP_TTL_11_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x00FF, pCmdqCtx);
    }
    u16Val = (MI_U16)(u8G) << 8;
    u32Reg = REG_DISP_TTL_11_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0xFF00, pCmdqCtx);
    }
    u16Val = u8R;
    u32Reg = REG_DISP_TTL_12_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x00FF, pCmdqCtx);
    }
}

void HAL_DISP_SetFrameColorForce(MI_U8 u8Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_10_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u8Val ? DISP_BIT8 : 0x0000, DISP_BIT8, pCmdqCtx);
    }
    else if (u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(u32Reg, u8Val ? DISP_BIT8 : 0x0000, DISP_BIT8);
    }
}

void HAL_DISP_SetFixedCsc(void *pCtx)
{
    void *pCmdqCtx = NULL;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_19_L, 0x1000, 0xFF00, pCmdqCtx); //
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_1A_L, 0x8080, 0xFFFF, pCmdqCtx); //
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_1B_L, 0x0cc4, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a11
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_1C_L, 0x0950, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a12
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_1D_L, 0x3ffc, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a1
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_1E_L, 0x397e, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a21
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_1F_L, 0x0950, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a22
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_20_L, 0x3cde, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a23
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_21_L, 0x3ffe, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a31
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_22_L, 0x0950, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a32
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_23_L, 0x1024, 0x3FFF, pCmdqCtx); // reg_csc_y2r_a33
}

void HAL_DISP_SetInputPortCrop(void *pCtx)
{
    void *                           pCmdqCtx            = NULL;
    DRV_DISP_CTX_Config_t *          pstDispCtxCfg       = NULL;
    DRV_DISP_CTX_InputPortContain_t *pstInputPortContain = NULL;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_50_L, DISP_BIT0, DISP_BIT0, pCmdqCtx); // reg_crop_en
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_51_L, pstInputPortContain->stFrameData.au32Stride[0], 0xFFFF,
                               pCmdqCtx); // reg_crop_in_hsize
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_52_L, pstInputPortContain->stAttr.u16SrcHeight, 0xFFFF,
                               pCmdqCtx); // reg_crop_in_vsize
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_53_L, pstInputPortContain->stCrop.u16X, 0xFFFF, pCmdqCtx); // reg_crop_h_st
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_54_L, pstInputPortContain->stCrop.u16Width, 0xFFFF,
                               pCmdqCtx);                                                              // reg_crop_hsize
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_55_L, pstInputPortContain->stCrop.u16Y, 0xFFFF, pCmdqCtx); // reg_crop_v_st
    HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_56_L, pstInputPortContain->stCrop.u16Height, 0xFFFF,
                               pCmdqCtx); // reg_crop_vsize
}

void HAL_DISP_SetDispMux(void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_TOP_0_7A_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_TOP_0_0A_L, DISP_BIT9, 0xFF00, pCmdqCtx);
        HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_TOP_0_50_L, 0, 0xFF00, pCmdqCtx);
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, DISP_BIT8, 0xFF00, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgHtt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_3F_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgVtt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_37_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgHsyncSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_39_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgHsyncEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_3A_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgVsyncSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_31_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgVsyncEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_32_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgHfdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_3B_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgHfdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_3C_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgVfdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_33_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgVfdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_34_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgHdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_3D_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgHdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_3E_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetRdmaPatTgVdeSt(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_35_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetTgenModeEn(void)
{
    _HAL_DISP_SetTgenModeEn();
}

void HAL_DISP_SetRdmaPatTgVdeEnd(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_SC_RDMA1_36_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x0FFF, pCmdqCtx);
    }
}

void HAL_DISP_SetSgRdmaCfg(MI_SYS_PixelFormat_e ePixelFormat, void *pCtx)
{
    void *pCmdqCtx = NULL;
    // DRV_DISP_CTX_Config_t *          pstDispCtxCfg       = NULL;
    // DRV_DISP_CTX_InputPortContain_t *pstInputPortContain = NULL;
    MI_U8  u8b420;
    MI_U8  u8b444;
    MI_U8  u8buvswap;
    MI_U8  u8bycswap;
    MI_U16 u16RLocation = 0, u16GLocation = 0, u16BLocation = 0;

    // pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    // pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    u8buvswap = (ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU || ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY
                 || ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21)
                    ? 1
                    : 0; // TBD

    u8bycswap =
        (ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY || ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY) ? 1 : 0;

    u8b420 = (ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420
              || ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21)
                 ? 1
                 : 0;
    u8b444 = IsArgb32bitPack(ePixelFormat);

    if (u8b444)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_19_L, 0, DISP_BIT0, pCmdqCtx); //[0]reg_csc_y2r_en
        if (ePixelFormat == E_MI_SYS_PIXEL_FRAME_BGRA8888)
        {
            u16RLocation = 1 << 12; // reg_sc_frm_r_444_v_location
            u16GLocation = 2 << 8;  // reg_sc_frm_r_444_y_location
            u16BLocation = 3 << 10; // reg_sc_frm_r_444_u_location
        }
        else if (ePixelFormat == E_MI_SYS_PIXEL_FRAME_ARGB8888)
        {
            u16RLocation = 2 << 12;
            u16GLocation = 1 << 8;
            u16BLocation = 0 << 10;
        }
        else if (ePixelFormat == E_MI_SYS_PIXEL_FRAME_ABGR8888)
        {
            u16RLocation = 0 << 12;
            u16GLocation = 1 << 8;
            u16BLocation = 2 << 10;
        }
        else // if (ePixelFormat == E_MI_SYS_PIXEL_FRAME_RGBA8888)
        {
            u16RLocation = 3 << 12;
            u16GLocation = 2 << 8;
            u16BLocation = 1 << 10;
        }
    }
    else
    {
        HAL_DISP_UTILITY_W2BYTEMSK(REG_DISP_TTL_19_L, DISP_BIT0, DISP_BIT0, pCmdqCtx); //[0]reg_csc_y2r_en
        u16RLocation = 0;
        u16GLocation = 0;
        u16BLocation = 0;
    }

    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_01_L, u8buvswap ? DISP_BIT10 : 0, DISP_BIT10,
                               pCmdqCtx);                                   //[12]reg_sc_frm_r_yc_swap
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, 0, DISP_BIT11, pCmdqCtx); //[11]reg_sc_frm_r_420to422_eco_option
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, u8bycswap ? DISP_BIT12 : 0, DISP_BIT12,
                               pCmdqCtx); //[12]reg_sc_frm_r_yc_swap
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_03_L, u16GLocation, DISP_BIT8 | DISP_BIT9,
                               pCmdqCtx); // reg_sc_frm_r_444_y_location
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_03_L, u16BLocation, DISP_BIT10 | DISP_BIT11,
                               pCmdqCtx); // reg_sc_frm_r_444_u_location
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_03_L, u16RLocation, DISP_BIT12 | DISP_BIT13,
                               pCmdqCtx); // reg_sc_frm_r_444_v_location
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_03_L, u8b444 ? DISP_BIT15 : 0, DISP_BIT15,
                               pCmdqCtx); //[15]reg_sc_frm_r_444_pack
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, u8b444 ? 0 : 0x3 << 8, DISP_BIT8 | DISP_BIT9,
                               pCmdqCtx); //[9:8]reg_sc_frm_r_422to444_md
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, u8b420 ? DISP_BIT4 : 0, DISP_BIT4,
                               pCmdqCtx); //[4]reg_sc_frm_r_420to422_md_avg
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, u8b420 ? DISP_BIT2 : 0, DISP_BIT2,
                               pCmdqCtx); //[2]reg_sc_frm_r_420to422_md
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, (IsSgRdmaYUV422Pack(ePixelFormat)) ? DISP_BIT7 : 0, DISP_BIT7,
                               pCmdqCtx); //[7]reg_sc_frm_r_422_bypass
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L,
                               ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422 ? DISP_BIT5 : 0, DISP_BIT5,
                               pCmdqCtx); //[5]reg_sc_frm_r_422_sep
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR ? DISP_BIT6 : 0,
                               DISP_BIT6, pCmdqCtx); //[6]reg_sc_frm_r_422_planar
    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_02_L, ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR ? DISP_BIT3 : 0,
                               DISP_BIT3, pCmdqCtx); //[3]reg_sc_frm_r_420_plnar
}

void HAL_DISP_SetSgRdmaSize(MI_SYS_PixelFormat_e ePixelFormat, void *pCtx)
{
    void *                           pCmdqCtx            = NULL;
    DRV_DISP_CTX_Config_t *          pstDispCtxCfg       = NULL;
    DRV_DISP_CTX_InputPortContain_t *pstInputPortContain = NULL;
    MI_U16                           u16PlaneIdx         = 0;
    MI_U32                           u32PlaneNum         = 0;
    MI_U32                           u32RegRdmaW         = 0;
    MI_U32                           u32RegRdmaH         = 0;
    MI_U32                           u32RegRdmaWPix      = 0;
    MI_U32                           u32W, u32H, u32DivW, u32DivH, u32Align, u32FinalW, u32FinalH, u32FinalPix;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32RegRdmaW    = REG_SC_SGRDMA1_02_L;
    u32RegRdmaH    = REG_SC_SGRDMA1_03_L;
    u32RegRdmaWPix = REG_SC_SGRDMA1_05_L;

    u32PlaneNum = _HAL_DISP_GetPlaneNum(ePixelFormat);
    u32W        = pstInputPortContain->stAttr.u16SrcWidth;
    u32H        = pstInputPortContain->stAttr.u16SrcHeight;

    for (u16PlaneIdx = 0; u16PlaneIdx < 3; u16PlaneIdx++)
    {
        if (u16PlaneIdx < u32PlaneNum)
        {
            _HAL_DISP_GetSrRdma128BitsDiv(ePixelFormat, u16PlaneIdx, &u32DivW, &u32DivH);
            if (u32DivW == 0 || u32DivH == 0)
            {
                DISP_ERR("%s %d, u32DivW or u32DivH = 0\n", __FUNCTION__, __LINE__);
                return;
            }
            u32Align  = HAL_DISP_HW_SUPPORT_SGRDMA_STRIDE_ALIGN(ePixelFormat);
            u32FinalW = (((u32W + (u32Align - 1)) / u32Align) * u32Align) / u32DivW;
            u32FinalH = (u32H + (u32DivH - 1)) / (u32DivH);

            if ((ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR
                 || ePixelFormat == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR)
                && (u16PlaneIdx == 1 || u16PlaneIdx == 2))
            {
                u32FinalPix = u32FinalW * u32DivW / 2;
            }
            else
            {
                u32FinalPix = u32FinalW * u32DivW;
            }

            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaW + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, u32FinalW, 0x1FFF,
                                       pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaH + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, u32FinalH, 0xFFFF,
                                       pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaWPix + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, u32FinalPix, 0x7FFF,
                                       pCmdqCtx);
        }
        else
        {
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaW + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, 0, 0x1FFF, pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaH + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, 0, 0xFFFF, pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaWPix + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, 0, 0x7FFF, pCmdqCtx);
        }
    }
}

void HAL_DISP_SetSgRdmaBufConfig(void *pCtx)
{
    void *                           pCmdqCtx            = NULL;
    DRV_DISP_CTX_Config_t *          pstDispCtxCfg       = NULL;
    DRV_DISP_CTX_InputPortContain_t *pstInputPortContain = NULL;
    MI_U16                           u16PlaneIdx         = 0;
    MI_U32                           u32PlaneNum         = 0;
    MI_U32                           u32RegBaseL         = 0;
    MI_U32                           u32RegBaseH         = 0;
    MI_U32                           u32RegRdmaPitch     = 0;
    MI_U32                           u32DivW, u32DivH;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    u32RegRdmaPitch = REG_SC_SGRDMA1_04_L;
    u32RegBaseL     = REG_SC_SGRDMA1_06_L;
    u32RegBaseH     = REG_SC_SGRDMA1_07_L;

    u32PlaneNum = _HAL_DISP_GetPlaneNum(pstInputPortContain->stFrameData.ePixelFormat);
    for (u16PlaneIdx = 0; u16PlaneIdx < 3; u16PlaneIdx++)
    {
        if (u16PlaneIdx < u32PlaneNum)
        {
            HAL_DISP_UTILITY_W2BYTE(u32RegBaseL + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET,
                                    (MI_U16)((pstInputPortContain->stFrameData.aPhyAddr[u16PlaneIdx] >> 4) & 0xFFFF),
                                    pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTE(u32RegBaseH + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET,
                                    (MI_U16)((pstInputPortContain->stFrameData.aPhyAddr[u16PlaneIdx] >> 4) >> 16),
                                    pCmdqCtx);

            _HAL_DISP_GetSrRdma128BitsDiv(pstInputPortContain->stFrameData.ePixelFormat, u16PlaneIdx, &u32DivW,
                                          &u32DivH);
            if (u32DivW == 0 || u32DivH == 0)
            {
                DISP_ERR("%s %d, u32DivW or u32DivH = 0\n", __FUNCTION__, __LINE__);
                return;
            }
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaPitch + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET,
                                       pstInputPortContain->stFrameData.au32Stride[u16PlaneIdx] / u32DivW, 0x7FFF,
                                       pCmdqCtx);
        }
        else
        {
            HAL_DISP_UTILITY_W2BYTE(u32RegBaseL + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, 0, pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTE(u32RegBaseH + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, 0, pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaPitch + u16PlaneIdx * HAL_DISP_SGRDMA_C_OFFSET, 0, 0x7FFF, pCmdqCtx);
        }
    }
}

void HAL_DISP_SetSgRdmaEn(void *pCtx, void *pCfg)
{
    void *                             pCmdqCtx            = NULL;
    DRV_DISP_CTX_Config_t *            pstDispCtxCfg       = NULL;
    DRV_DISP_CTX_InputPortContain_t *  pstInputPortContain = NULL;
    MI_DISP_IMPL_MhalVideoFrameData_t *pstFramedata        = NULL;
    MI_U8 *                            pbEnable            = NULL;
    MI_U32                             u32RegRdmaEn        = 0;
    MI_U32                             u32RegRdmaFifoSize  = 0;
    MI_U32                             u32RegRdmaFifoStart = 0;
    MI_U32                             u32PlaneId          = 0;
    MI_U32                             u32PlaneNum         = 0;
    MI_U32                             u32FifoSize[3];
    MI_U32                             u32FifoStart[3];
    MI_U16                             u16RegVal = 0;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];
    pstFramedata        = &pstInputPortContain->stFrameData;
    pbEnable            = (MI_U8 *)pCfg;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    u32RegRdmaEn        = REG_SC_SGRDMA1_00_L;
    u32RegRdmaFifoSize  = REG_SC_SGRDMA1_01_L;
    u32RegRdmaFifoStart = REG_SC_SGRDMA1_01_L;

    u32PlaneNum = _HAL_DISP_GetPlaneNum(pstFramedata->ePixelFormat);
    if (u32PlaneNum == 1)
    {
        u32FifoSize[0]  = 2;
        u32FifoStart[0] = 0;
    }
    else if (u32PlaneNum == 2)
    {
        u32FifoSize[0]  = 1;
        u32FifoSize[1]  = 1;
        u32FifoStart[0] = 0;
        u32FifoStart[1] = 1;
    }
    else
    {
        u32FifoSize[0]  = 1;
        u32FifoSize[1]  = 0;
        u32FifoSize[2]  = 0;
        u32FifoStart[0] = 0;
        u32FifoStart[1] = 2;
        u32FifoStart[2] = 3;
    }
    for (u32PlaneId = 0; u32PlaneId < 3; u32PlaneId++)
    {
        if (u32PlaneId < u32PlaneNum)
        {
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaEn + u32PlaneId * HAL_DISP_SGRDMA_C_OFFSET, *pbEnable ? DISP_BIT0 : 0,
                                       DISP_BIT0, pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaFifoSize + u32PlaneId * HAL_DISP_SGRDMA_C_OFFSET,
                                       u32FifoSize[u32PlaneId] << 8, DISP_BIT8 | DISP_BIT9 | DISP_BIT10, pCmdqCtx);
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaFifoStart + u32PlaneId * HAL_DISP_SGRDMA_C_OFFSET,
                                       u32FifoStart[u32PlaneId] << 12, DISP_BIT12 | DISP_BIT13, pCmdqCtx);
        }
        else
        {
            // disable no use plane
            HAL_DISP_UTILITY_W2BYTEMSK(u32RegRdmaEn + u32PlaneId * HAL_DISP_SGRDMA_C_OFFSET, 0, DISP_BIT0, pCmdqCtx);
        }
    }

    HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_RDMA1_01_L, *pbEnable ? DISP_BIT15 : 0, DISP_BIT15, pCmdqCtx);
    u16RegVal = HAL_DISP_UTILITY_R2BYTEMaskDirect(REG_SC_TOP_0_7A_L, DISP_BIT1);
    if (*pbEnable)
    {
        if (!u16RegVal)
        {
            HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_TOP_0_7A_L, DISP_BIT2, DISP_BIT2, pCmdqCtx); // clr rdma hub
            HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_TOP_0_7A_L, DISP_BIT1, DISP_BIT1, pCmdqCtx); // sel disp path
        }
    }
    else
    {
        if (u16RegVal)
        {
            HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_TOP_0_7A_L, DISP_BIT2, DISP_BIT2, pCmdqCtx); // clr rdma hub
        }
    }
}

void HAL_DISP_SetSgRdmaFifoPush(void *pCtx)
{
    void *                           pCmdqCtx            = NULL;
    DRV_DISP_CTX_Config_t *          pstDispCtxCfg       = NULL;
    DRV_DISP_CTX_InputPortContain_t *pstInputPortContain = NULL;
    MI_U32                           u32PlaneId          = 0;
    MI_U32                           u32PlaneNum         = 0;

    pstDispCtxCfg       = (DRV_DISP_CTX_Config_t *)pCtx;
    pstInputPortContain = pstDispCtxCfg->pstCtxContain->pstInputPortContain[pstDispCtxCfg->u32ContainIdx];

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);

    u32PlaneNum = _HAL_DISP_GetPlaneNum(pstInputPortContain->stFrameData.ePixelFormat);

    for (u32PlaneId = 0; u32PlaneId < u32PlaneNum; u32PlaneId++)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(REG_SC_SGRDMA1_00_L + u32PlaneId * HAL_DISP_SGRDMA_C_OFFSET, DISP_BIT4, DISP_BIT4,
                                   pCmdqCtx);
    }
}

void HAL_DISP_SetCmdqIntMask(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_63_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x00FF, pCmdqCtx);
    }
}
void HAL_DISP_SetCmdqIntClear(MI_U16 u16Val, void *pCtx)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg   = 0;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_DISP_TTL_65_L;
    if (pCmdqCtx && u32Reg)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val, 0x00FF, pCmdqCtx);
    }
}

void HAL_DISP_LpllSetTbl(MI_U16 u16LpllIdx, MI_U32 u32LpllSet, MI_U8 bDsi)
{
    _HAL_DISP_LpllDumpSetting(u16LpllIdx, bDsi);
    _HAL_DISP_LpllSetLpllSet(u32LpllSet);
}

void HAL_DISP_LcdSetUnifiedPrototypeConfig(void *pCtx)
{
    DRV_DISP_CTX_Config_t *       pstDispCtxCfg = NULL;
    DRV_DISP_CTX_DeviceContain_t *pstDevContain = NULL;
    // MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstUfdParamCfg = NULL;
    // MI_U16                                    u16RgbDtype;
    // MI_U8                                     u8RgbDswap;
    // MI_U8                                     u8RgbDummy;

    pstDispCtxCfg = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    // pstUfdParamCfg = &pstDevContain->stPnlUdParamCfg;

    // u16RgbDtype = (MI_U16)pstUfdParamCfg->stRgbDataInfo.eRgbDtype;
    // u8RgbDswap  = pstUfdParamCfg->stRgbDataInfo.u8RgbDswap;
    // u8RgbDummy  = pstUfdParamCfg->stRgbDeltaInfo.u8DummyMode;

    if (pstDevContain->u32Interface == HAL_DISP_INTF_TTL)
    {
        _HAL_DISP_SetTtlFmt(E_HAL_DISP_COLOR_RGB565);
        _HAL_DISP_LcdSetBtEncMode(0);
        _HAL_DISP_LcdSetBtEncModeEn(0);
        _HAL_DISP_SetYuv444To422(0);
        _HAL_DISP_SetBtModeSel(0);
        _HAL_DISP_SetBtModeAuto(0);
        _HAL_DISP_SetTtlMux(0);
        _HAL_DISP_SetTgenFwVtt(1);
        _HAL_DISP_SetCkgPipeTtl(0);
    }
    else if (pstDevContain->u32Interface == HAL_DISP_INTF_BT656)
    {
        _HAL_DISP_LcdSetBtEncMode(0);
        _HAL_DISP_LcdSetRgbR2YMode(1);
        _HAL_DISP_LcdSetBtEncModeEn(1);
        _HAL_DISP_SetYuv444To422(1);
        _HAL_DISP_SetTgenFwVtt(1);
        _HAL_DISP_SetBtModeAuto(1);
        _HAL_DISP_SetBtMux(0);
        _HAL_DISP_SetBtModeSel(0);
        _HAL_DISP_SetCkgPipeBt(0);
    }
}

void HAL_DISP_LcdSetUnifiedPolarityConfig(void *pCtx)
{
    DRV_DISP_CTX_Config_t *                   pstDispCtxCfg  = NULL;
    DRV_DISP_CTX_DeviceContain_t *            pstDevContain  = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstUfdParamCfg = NULL;

    pstDispCtxCfg  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain  = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstUfdParamCfg = &pstDevContain->stPnlUdParamCfg;

    DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, ClkInv:%d, DeInv:%d, HsyncInv:%d, VsyncInv:%d\n", __FUNCTION__, __LINE__,
             pstUfdParamCfg->stTgnPolarityInfo.u8InvDCLK, pstUfdParamCfg->stTgnPolarityInfo.u8InvDE,
             pstUfdParamCfg->stTgnPolarityInfo.u8InvHSync, pstUfdParamCfg->stTgnPolarityInfo.u8InvVSync);

    //_HAL_DISP_LcdSetDeInv(pstUfdParamCfg->stTgnPolarityInfo.u8InvDE);
    _HAL_DISP_LcdSetRgbVsyncPol(pstUfdParamCfg->stTgnPolarityInfo.u8InvVSync);
    _HAL_DISP_LcdSetRgbHsyncPol(pstUfdParamCfg->stTgnPolarityInfo.u8InvHSync);
    _HAL_DISP_LcdSetRgbClkPol(pstDevContain->u32Interface, pstUfdParamCfg->stTgnPolarityInfo.u8InvDCLK);
}

void HAL_DISP_LcdSetUnifiedSwapChnConfig(void *pCtx)
{
    DRV_DISP_CTX_Config_t *                   pstDispCtxCfg  = NULL;
    DRV_DISP_CTX_DeviceContain_t *            pstDevContain  = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstUfdParamCfg = NULL;

    pstDispCtxCfg  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain  = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstUfdParamCfg = &pstDevContain->stPnlUdParamCfg;

    DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, (%s %s %s) MlSwap=%d\n", __FUNCTION__, __LINE__,
             pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapChnR, pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapChnG,
             pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapChnB, pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapRgbML);

    _HAL_DISP_LcdPnlSetRgbSwap(pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapChnR, pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapChnG,
                               pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapChnB);
    _HAL_DISP_LcdPnlSetRgbMlSwap(pstUfdParamCfg->stTgnRgbSwapInfo.u8SwapRgbML);
}

void HAL_DISP_LpllSetSscEn(MI_U8 u8Val)
{
    _HAL_DISP_LpllSetSscEn(u8Val);
}

void HAL_DISP_LpllSetSscConfigEx(MI_DISP_IMPL_MhalPnlUnifiedTgnSpreadSpectrumConfig_t *pstSscCfg)
{
    DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, Span:0x%x, Step:0x%x\n", __FUNCTION__, __LINE__,
             pstSscCfg->u16SpreadSpectrumSpan, pstSscCfg->u16SpreadSpectrumStep);

    _HAL_DISP_LpllSetSscEn(1);
    _HAL_DISP_LpllSetSscStep(pstSscCfg->u16SpreadSpectrumStep);
    _HAL_DISP_LpllSetSscSpan(pstSscCfg->u16SpreadSpectrumSpan);
}

void HAL_DISP_LcdSetUnifiedRgbDeltaMode(void *pCtx)
{
    DRV_DISP_CTX_Config_t *                   pstDispCtxCfg  = NULL;
    DRV_DISP_CTX_DeviceContain_t *            pstDevContain  = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstUfdParamCfg = NULL;

    pstDispCtxCfg  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain  = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstUfdParamCfg = &pstDevContain->stPnlUdParamCfg;

    DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, LinkType:%s, RgbDeltaMdFlag:%d RgbDelta:(%s, %s) RgbDummy:%d\n",
             __FUNCTION__, __LINE__, PARSING_HAL_INTERFACE(pstDevContain->u32Interface),
             pstUfdParamCfg->u8RgbDeltaMdFlag, (pstUfdParamCfg->stRgbDeltaInfo.eOddLine),
             (pstUfdParamCfg->stRgbDeltaInfo.eEvenLine), pstUfdParamCfg->stRgbDeltaInfo.u8DummyMode);

    _HAL_DISP_LcdSetRgbDeltaMode(pstUfdParamCfg->stRgbDeltaInfo.u8DummyMode, pstUfdParamCfg->stRgbDeltaInfo.eOddLine,
                                 pstUfdParamCfg->stRgbDeltaInfo.eEvenLine);
}

void HAL_DISP_LcdSetPadMux(void *pCtx)
{
    DRV_DISP_CTX_Config_t *                   pstDispCtxCfg  = NULL;
    DRV_DISP_CTX_DeviceContain_t *            pstDevContain  = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pstUfdParamCfg = NULL;

    pstDispCtxCfg  = (DRV_DISP_CTX_Config_t *)pCtx;
    pstDevContain  = pstDispCtxCfg->pstCtxContain->pstDevContain[pstDispCtxCfg->u32ContainIdx];
    pstUfdParamCfg = &pstDevContain->stPnlUdParamCfg;

    DISP_DBG(DISP_DBG_LEVEL_HAL_LCD, "%s %d, LinkType:%s, PadMuxFlag:%d PadMux:%d\n", __FUNCTION__, __LINE__,
             PARSING_HAL_INTERFACE(pstDevContain->u32Interface), pstUfdParamCfg->u8TgnPadMuxFlag,
             pstUfdParamCfg->u16PadMux);

    switch (pstDevContain->u32Interface)
    {
        case HAL_DISP_INTF_TTL:
            _HAL_DISP_LcdSetTTLPadMux(pstUfdParamCfg->u16PadMux);
            break;
        case HAL_DISP_INTF_BT656:
        case HAL_DISP_INTF_BT601:
        case HAL_DISP_INTF_BT1120:
            _HAL_DISP_LcdSetBTPadMux(pstDevContain->u32Interface, pstUfdParamCfg->u16PadMux);
            break;
        case HAL_DISP_INTF_SRGB:
            _HAL_DISP_LcdSetRGBPadMux(pstUfdParamCfg->eOutputFormatBitMode, pstUfdParamCfg->u16PadMux);
            break;
        default:
            break;
    }
}

void HAL_DISP_SetTimeGenStartFlag(MI_U16 u16Val, void *pCtx, MI_U32 u32DevId)
{
    void * pCmdqCtx = NULL;
    MI_U32 u32Reg;

    HAL_DISP_GetCmdqByCtx(pCtx, &pCmdqCtx);
    u32Reg = REG_HAL_DISP_UTILIYT_CMDQDEV_TIMEGEN_START(u32DevId);
    if (u32Reg == 0)
    {
        return;
    }
    g_stDispIrqHist.stWorkingStatus.stDevStatus[u32DevId].u8bStartTimegen = u16Val;
    if (pCmdqCtx)
    {
        HAL_DISP_UTILITY_W2BYTEMSK(u32Reg, u16Val ? REG_CMDQ_DEV_TIMEGEN_START_MSK : 0, REG_CMDQ_DEV_TIMEGEN_START_MSK,
                                   pCmdqCtx);
    }
    else
    {
        HAL_DISP_UTILITY_W2BYTEMSKDirect(u32Reg, u16Val ? REG_CMDQ_DEV_TIMEGEN_START_MSK : 0,
                                         REG_CMDQ_DEV_TIMEGEN_START_MSK);
    }
}
