/*
 * hal_disp_chip.h- Sigmastar
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
#ifndef _HAL_DISP_CHIP_H_
#define _HAL_DISP_CHIP_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// Capability
#define DISP_SUPPORT_LCD 1

#define DISP_BANK_SIZE 512

#define HAL_DISP_IRQ_TTL_VSYNC_BIT (HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_BIT)
#define HAL_DISP_IRQ_TTL_VSYNC_MSK (HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_MSK)

#define HAL_SC_DISP_CMDQIRQ_DEV_RDMA_ERROR_BIT     (0x0200)
#define HAL_SC_DISP_CMDQIRQ_DEV_RDMA_ERROR_MSK     (0x0200)
#define HAL_SC_DISP_CMDQIRQ_DEV_OSDB_ERROR_BIT     (0x0100)
#define HAL_SC_DISP_CMDQIRQ_DEV_OSDB_ERROR_MSK     (0x0100)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVDE_OFF_BIT  (0x0080)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVDE_OFF_MSK  (0x0080)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVDE_ON_BIT   (0x0040)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVDE_ON_MSK   (0x0040)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_BIT (0x0020)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLORIVSYNC_ON_MSK (0x0020)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLVDE_OFF_BIT     (0x0010)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLVDE_OFF_MSK     (0x0010)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLVDE_ON_BIT      (0x0008) // BIT3
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLVDE_ON_MSK      (0x0008)
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLVSYNC_ON_BIT    (0x0004) // BIT2
#define HAL_SC_DISP_CMDQIRQ_DEV_TTLVSYNC_ON_MSK    (0x0004)
#define HAL_SC_DISP_CMDQIRQ_DEV_INT_BIT            (0x0002) // BIT1
#define HAL_SC_DISP_CMDQIRQ_DEV_INT_MSK            (0x0002)
#define HAL_SC_DISP_CMDQIRQ_DEV_RDMA1TRIG_BIT      (0x0001) // BIT0
#define HAL_SC_DISP_CMDQIRQ_DEV_RDMA1TRIG_MSK      (0x0001)

//------------------------------------------------------------------------------
// Fpll
#define HAL_DISP_FPLL_0_VGA_HDMI 0
#define HAL_DISP_FPLL_1_CVBS_DAC 1
#define HAL_DISP_FPLL_CNT        2

//------------------------------------------------------------------------------
// Ctx
#define HAL_DISP_CTX_MAX_INST 1
#define HAL_DISP_NOTSUPPORT_INTF                                                                           \
    (HAL_DISP_INTF_HDMI | HAL_DISP_INTF_CVBS | HAL_DISP_INTF_VGA | HAL_DISP_INTF_YPBPR | HAL_DISP_INTF_LCD \
     | HAL_DISP_INTF_MIPIDSI | HAL_DISP_INTF_BT601 | HAL_DISP_INTF_MCU | HAL_DISP_INTF_MCU_NOFLM           \
     | HAL_DISP_INTF_BT1120_DDR)

//------------------------------------------------------------------------------
// Device Ctx
#define HAL_DISP_DEVICE_ID_0 0
#define HAL_DISP_DEVICE_ID_1 1
#define HAL_DISP_DEVICE_ID_2 2
#define HAL_DISP_DEVICE_MAX  1

#define HAL_DISP_DEVICE_0_VID_CNT 1

#define HAL_DISP_DEVICE_0_SUPPORT_DMA 0

//------------------------------------------------------------------------------
// VideoLayer
#define HAL_DISP_VIDLAYER_ID_0 0
#define HAL_DISP_VIDLAYER_MAX  1 // (MOPG & MOPS) * 2 Device
#define HAL_DISP_VIDLAYER_CNT  1
//------------------------------------------------------------------------------
// InputPort
#define HAL_DISP_MOPG_GWIN_NUM 1   // MOPG 16 Gwins
#define HAL_DISP_MOPS_GWIN_NUM 0   // MOPS 1 Gwin
#define HAL_DISP_INPUTPORT_NUM (1) // MOP: 32Gwin_MOPG + 1Gwin_MOPS
#define HAL_DISP_INPUTPORT_MAX (HAL_DISP_DEVICE_MAX * HAL_DISP_INPUTPORT_NUM)
//------------------------------------------------------------------------------
// DMA  Ctx
#define HAL_DISP_DMA_ID_0 0
#define HAL_DISP_DMA_MAX  0
//------------------------------------------------------------------------------
// IRQ CTX
#define HAL_DISP_IRQ_ID_DEVICE_0 0
#define HAL_DISP_IRQ_ID_MAX      1

#define HAL_DISP_MAPPING_DEVICEID_FROM_MI(x) ((x == 0) ? HAL_DISP_DEVICE_ID_0 : HAL_DISP_DEVICE_MAX)

#define HAL_DISP_MAPPING_VIDLAYERID_FROM_MI(x) ((x == 0) ? HAL_DISP_VIDLAYER_ID_0 : HAL_DISP_VIDLAYER_ID_0)

#define HAL_DISP_IRQ_CFG \
    {                    \
        {0, 0},          \
    }
//------------------------------------------------------------------------------
// TimeZone
#define HAL_DISP_TIMEZONE_ISR_SUPPORT_LINUX 0
#define HAL_DISP_TIMEZONE_ISR_SUPPORT_RTK   0

#define E_HAL_DISP_IRQ_TYPE_TIMEZONE (E_HAL_DISP_IRQ_TYPE_TIMEZONE_VDE_NEGATIVE)

//------------------------------------------------------------------------------
// Vga HPD Isr
#define HAL_DISP_VGA_HPD_ISR_SUPPORT_LINUX 0
#define HAL_DISP_VGA_HPD_ISR_SUPPORT_RTK   0

#define E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF (E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON | E_HAL_DISP_IRQ_TYPE_VGA_HPD_OFF)

//------------------------------------------------------------------------------
// DMA Isr

#define E_HAL_DISP_IRQ_TYPE_DMA                                                                                \
    (E_HAL_DISP_IRQ_TYPE_DMA_ACTIVE_ON | E_HAL_DISP_IRQ_TYPE_DMA_REALDONE | E_HAL_DISP_IRQ_TYPE_DMA_REGSETFAIL \
     | E_HAL_DISP_IRQ_TYPE_DMA_FIFOFULL)

//------------------------------------------------------------------------------
// CLK
#define CLK_MHZ(m, k)                 (m * 1000000 + k * 1000)
#define HAL_DISP_CLK_FCLK_RATE        0
#define HAL_DISP_CLK_DISP_PIXEL0_RATE 2

#define HAL_DISP_CLK_
typedef enum
{
    E_HAL_DISP_CLK_FCLK,
    E_HAL_DISP_CLK_DISP_PIXEL_0,
    E_HAL_DISP_CLK_DISPPLL_CLK,
    E_HAL_DISP_CLK_NUM = 2,
} HAL_DISP_CLK_Type_e;

#define HAL_DISP_COLOR_CSCM_TO_Y2RM(matrix)                                                        \
    ((matrix == E_MI_DISP_CSC_MATRIX_BYPASS)            ? E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS \
     : (matrix == E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC) ? E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_SDTV   \
     : (matrix == E_MI_DISP_CSC_MATRIX_BT709_TO_RGB_PC) ? E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV   \
                                                        : E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_MAX)
#define HAL_DISP_COLOR_Y2RM_TO_CSCM(matrix)                                                      \
    ((matrix == E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS) ? E_MI_DISP_CSC_MATRIX_BYPASS          \
     : (matrix == E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_SDTV) ? E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC \
     : (matrix == E_HAL_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV) ? E_MI_DISP_CSC_MATRIX_BT709_TO_RGB_PC \
                                                          : E_MI_DISP_CSC_MATRIX_NUM)

#define HAL_DISP_CLK_GET_ON_SETTING(idx) (idx == E_HAL_DISP_CLK_DISP_PIXEL_0 ? 1 : 1)

#define HAL_DISP_CLK_GET_OFF_SETTING(idx) (idx == E_HAL_DISP_CLK_DISP_PIXEL_0 ? 0 : 0)

#define HAL_DISP_CLK_GET_RATE_ON_SETTING(idx) \
    (idx == E_HAL_DISP_CLK_DISP_PIXEL_0 ? HAL_DISP_CLK_DISP_PIXEL0_RATE : HAL_DISP_CLK_FCLK_RATE)

#define HAL_DISP_CLK_GET_RATE_OFF_SETTING(idx) \
    (idx == E_HAL_DISP_CLK_DISP_PIXEL_0 ? HAL_DISP_CLK_DISP_PIXEL0_RATE : HAL_DISP_CLK_FCLK_RATE)

// Mux_ATTR = 1 : clk idx
// Mux_ATTR = 0 : clk rate
#define HAL_DISP_CLK_GET_MUX_ATTR(idx) (idx == E_HAL_DISP_CLK_DISP_PIXEL_0 ? 1 : 1)

#define HAL_DISP_CLK_GET_NAME(idx) (idx == E_HAL_DISP_CLK_DISP_PIXEL_0 ? "disp_pixel_0" : "fclk")

#define HAL_DISP_CLK_ON_SETTING \
    {                           \
        1, 1,                   \
    }

#define HAL_DISP_CLK_OFF_SETTING \
    {                            \
        0, 0,                    \
    }

#define HAL_DISP_CLK_RATE_SETTING                              \
    {                                                          \
        HAL_DISP_CLK_FCLK_RATE, HAL_DISP_CLK_DISP_PIXEL0_RATE, \
    }

#define HAL_DISP_CLK_OFF_RATE_SETTING                          \
    {                                                          \
        HAL_DISP_CLK_FCLK_RATE, HAL_DISP_CLK_DISP_PIXEL0_RATE, \
    }

// Mux_ATTR = 1 : clk idx
// Mux_ATTR = 0 : clk rate
#define HAL_DISP_CLK_MUX_ATTR \
    {                         \
        1, 1,                 \
    }

#define HAL_DISP_CLK_NAME       \
    {                           \
        "fclk", "disp_pixel_0", \
    }

#define IsSgRdmaYUV422Pack(enPortFormat)                                                                   \
    ((enPortFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV || enPortFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU \
      || enPortFormat == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY || enPortFormat == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY))

#define IsArgb32bitPack(enPortFormat)                                                                \
    ((enPortFormat == E_MI_SYS_PIXEL_FRAME_ARGB8888 || enPortFormat == E_MI_SYS_PIXEL_FRAME_ABGR8888 \
      || enPortFormat == E_MI_SYS_PIXEL_FRAME_BGRA8888))

//------------------------------------------------------------------------------
// MIPIDSI

#define HAL_DISP_PNL_SIGNAL_CTRL_CH_MAX 5

//------------------------------------------------------------------------------
// LCD

#define DISP_SUPPORT_PNL                             (0x00050000)
#define HAL_DISP_PNL_IS_SUPPORTED_LINKTYPE(LinkType) (LinkType == HAL_DISP_INTF_TTL || LinkType == HAL_DISP_INTF_BT656)

//------------------------------------------------------------------------------
// DISPPLL

#define HAL_DISP_PNL_LPLL_REG_NUM      4
#define HAL_DISP_PNL_LPLL_REG_MAX      7
#define HAL_DISP_PNL_TXPLL_DIV_REG_MAX 1

#define DATA_LANE_2_8MHZ   (2800000)
#define DATA_LANE_3_25MHZ  (3250000)
#define DATA_LANE_6_5MHZ   (6500000)
#define DATA_LANE_9MHZ     (9000000)
#define DATA_LANE_9_5MHZ   (9500000)
#define DATA_LANE_12_5MHZ  (12500000)
#define DATA_LANE_25MHZ    (25000000)
#define DATA_LANE_50MHZ    (50000000)
#define DATA_LANE_100MHZ   (100000000)
#define DATA_LANE_187_5MHZ (187500000)
#define DATA_LANE_200MHZ   (200000000)
#define DATA_LANE_400MHZ   (400000000)
#define DATA_LANE_800MHZ   (800000000)
#define DATA_LANE_1500MHZ  (1500000000)

#define IS_DATA_LANE_LESS_100M(bps)          (bps <= DATA_LANE_100MHZ)
#define IS_DATA_LANE_BPS_100M_TO_200M(bps)   ((bps > DATA_LANE_100MHZ) && (bps <= DATA_LANE_200MHZ))
#define IS_DATA_LANE_BPS_200M_TO_400M(bps)   ((bps > DATA_LANE_200MHZ) && (bps <= DATA_LANE_400MHZ))
#define IS_DATA_LANE_BPS_400M_TO_800M(bps)   ((bps > DATA_LANE_400MHZ) && (bps <= DATA_LANE_800MHZ))
#define IS_DATA_LANE_BPS_800M_TO_15000M(bps) ((bps > DATA_LANE_800MHZ) && (bps <= DATA_LANE_1500MHZ))

#define IS_DATA_LANE_LESS_9M(bps)            (bps < DATA_LANE_9MHZ)
#define IS_DATA_LANE_BPS_9M_TO_9_5M(bps)     ((bps >= DATA_LANE_9MHZ) && (bps < DATA_LANE_9_5MHZ))
#define IS_DATA_LANE_BPS_2_8M_TO_3_25M(bps)  ((bps >= DATA_LANE_2_8MHZ) && (bps < DATA_LANE_3_25MHZ))
#define IS_DATA_LANE_BPS_3_25M_TO_6_5M(bps)  ((bps >= DATA_LANE_3_25MHZ) && (bps < DATA_LANE_6_5MHZ))
#define IS_DATA_LANE_BPS_6_5M_TO_12_5M(bps)  ((bps >= DATA_LANE_6_5MHZ) && (bps < DATA_LANE_12_5MHZ))
#define IS_DATA_LANE_BPS_12_5M_TO_25M(bps)   ((bps >= DATA_LANE_12_5MHZ) && (bps <= DATA_LANE_25MHZ))
#define IS_DATA_LANE_BPS_25M_TO_50M(bps)     ((bps > DATA_LANE_25MHZ) && (bps <= DATA_LANE_50MHZ))
#define IS_DATA_LANE_BPS_50M_TO_100M(bps)    ((bps > DATA_LANE_50MHZ) && (bps <= DATA_LANE_100MHZ))
#define IS_DATA_LANE_BPS_100M_TO_187_5M(bps) ((bps > DATA_LANE_100MHZ) && (bps <= DATA_LANE_187_5MHZ))

#define HAL_DISP_REG_ACCESS_MD_UBOOT E_MI_DISP_REG_ACCESS_CPU
#define HAL_DISP_REG_ACCESS_MD_LINUX E_MI_DISP_REG_ACCESS_CMDQ
#define HAL_DISP_REG_ACCESS_MD_RTK   E_MI_DISP_REG_ACCESS_CMDQ

//-------------------------------------------------------------------------------------------------
//  Maybe Diff in ASIC/FPGA
//-------------------------------------------------------------------------------------------------

// PLL cnt
#define HAL_DISP_PLL_DISPPLL (0)
#define HAL_DISP_PLL_CNT     (1)

#define HAL_DISP_1M   1000000
#define HAL_DISP_1K   1000
#define HAL_DISP_100K 100000
//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

#endif
