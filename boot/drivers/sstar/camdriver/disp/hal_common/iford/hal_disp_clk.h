/*
 * hal_disp_clk.h- Sigmastar
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
#ifndef _HAL_DISP_CLK_H_
#define _HAL_DISP_CLK_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
// reg_ckg_disp_pixel_0
#define HAL_DISP_CLK_DISP_PIXEL_0_SEL_72M       0x00
#define HAL_DISP_CLK_DISP_PIXEL_0_SEL_54M       0x01
#define HAL_DISP_CLK_DISP_PIXEL_0_SEL_LPLL      0x02
#define HAL_DISP_CLK_DISP_PIXEL_0_SEL_LPLL_DIV2 0x03

// reg_clk_mipi_tx_dsi_apb
#define HAL_DISP_CLK_ODCLK_SEL_P              0x00
#define HAL_DISP_CLK_ODCLK_SEL_MIPI1_TX_CSI_P 0x01

// reg_clk_mipi_tx_dsi_apb
#define HAL_DISP_CLK_MIPI_TX_DSI_APB_SEL_0 0x00
#define HAL_DISP_CLK_MIPI_TX_DSI_APB_SEL_1 0x01

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_CLK_GP_CTRL_NONE       = 0x0000,
    E_HAL_DISP_CLK_GP_CTRL_HDMI       = 0x0001,
    E_HAL_DISP_CLK_GP_CTRL_LCD        = 0x0002,
    E_HAL_DISP_CLK_GP_CTRL_DAC        = 0x0004,
    E_HAL_DISP_CLK_GP_CTRL_MIPIDSI    = 0x0008,
    E_HAL_DISP_CLK_GP_CTRL_CVBS       = 0x0010,
    E_HAL_DISP_CLK_GP_CTRL_BT1120_SDR = 0x0020,
    E_HAL_DISP_CLK_GP_CTRL_BT1120_DDR = 0x0040,
    E_HAL_DISP_CLK_GP_CTRL_MIPIDSI_1  = 0x0080,
    E_HAL_DISP_CLK_GP_CTRL_HDMI_DAC   = 0x0005,
    E_HAL_DISP_CLK_GP_CTRL_HVP        = 0x0100,
    E_HAL_DISP_CLK_GP_CTRL_LVDS       = 0x0200,
    E_HAL_DISP_CLK_GP_CTRL_LVDS_1     = 0x0400,
    E_HAL_DISP_CLK_GP_CTRL_DUAL_LVDS  = 0x0800,
    E_HAL_DISP_CLK_GP_CTRL_INTERNAL   = 0x8000,
} HAL_DISP_CLK_GpCtrlType_e;

typedef struct
{
    MI_U8                     bEn;
    HAL_DISP_CLK_GpCtrlType_e eType;
} HAL_DISP_CLK_GpCtrlConfig_t;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _HAL_DISP_CLK_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void   HAL_DISP_CLK_SetClkDispPixel0(MI_U32 u32Dev, MI_U8 bEn, MI_U32 u32ClkRate);
INTERFACE void   HAL_DISP_CLK_GetClkDispPixel0(MI_U32 u32Dev, MI_U8 *pbEn, MI_U32 *pu32ClkRate);
INTERFACE void   HAL_DISP_CLK_SetBasicClkgen(HAL_DISP_CLK_Type_e enClkType, MI_U8 bEn, MI_U32 u32ClkRate);
INTERFACE MI_U32 HAL_DISP_CLK_MapBasicClkgenToIdx(HAL_DISP_CLK_Type_e enClkType, MI_U32 u32ClkRate);
INTERFACE void   HAL_DISP_CLK_GetBasicClkgen(HAL_DISP_CLK_Type_e enClkType, MI_U8 *pbEn, MI_U32 *pu32ClkRate);
INTERFACE MI_U32 HAL_DISP_CLK_MapBasicClkgenToRate(HAL_DISP_CLK_Type_e enClkType, MI_U32 u32ClkRateIdx);

INTERFACE void HAL_DISP_CLK_InitGpCtrlCfg(void *pCtx);
INTERFACE void HAL_DISP_CLK_DeInitGpCtrlCfg(void *pCtx);
INTERFACE void HAL_DISP_CLK_SetGpCtrlCfg(void *pCtx);
INTERFACE void HAL_DISP_CLK_Init(MI_U8 bEn);
#undef INTERFACE
#endif
