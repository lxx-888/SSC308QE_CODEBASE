/*
 * hal_complex_clk.h- Sigmastar
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

#ifndef _HAL_COMPLEX_CLK_H_
#define _HAL_COMPLEX_CLK_H_

#define CLK_DEBUG 0

#if CLK_DEBUG
#define clk_dbg(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define clk_dbg(fmt, arg...)
#endif
#define clk_err(fmt, arg...) printk(KERN_ERR fmt, ##arg)

#ifdef CONFIG_PM_SLEEP
typedef int (*pll_init)(struct clk_hw *hw);
struct hal_clk_pll_info
{
    struct list_head list;
    pll_init         init;
    const char *     name;
};
#endif

typedef struct
{
    u32 address;
    u16 value;
} hal_clk_disp_pnl_lpll_tbl_t;

typedef struct
{
    U32 frequency;
    U32 val;
} hal_clk_fuart_tbl;

typedef enum
{
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO312D5MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX,
} hal_clk_pnl_lpll_type_e;

/*
 * the config is defined to apply control PLL by framework or not
 */

#define HAL_CLK_DISP_PNL_LPLL_REG_NUM (7)
#define HAL_CLK_DISP_PNL_LPLL_REG_MAX (8)

#define HAL_CLK_REG_DISP_LPLL_0_BASE (0x103700UL)
#define HAL_CLK_REG_DISP_LPLL_1_BASE (0x103B00UL)
#define HAL_CLK_REG_DISP_LPLL_1_48_L (HAL_CLK_REG_DISP_LPLL_1_BASE + 0x90)
#define HAL_CLK_REG_DISP_LPLL_1_49_L (HAL_CLK_REG_DISP_LPLL_1_BASE + 0x92)
#define HAL_CLK_REG_DISP_LPLL_0_48_L (HAL_CLK_REG_DISP_LPLL_0_BASE + 0x90)
#define HAL_CLK_REG_DISP_LPLL_0_49_L (HAL_CLK_REG_DISP_LPLL_0_BASE + 0x92)

#define HAL_CLK_DATA_LANE_9MHZ     (9000000)
#define HAL_CLK_DATA_LANE_9_5MHZ   (9500000)
#define HAL_CLK_DATA_LANE_12_5MHZ  (12500000)
#define HAL_CLK_DATA_LANE_25MHZ    (25000000)
#define HAL_CLK_DATA_LANE_50MHZ    (50000000)
#define HAL_CLK_DATA_LANE_100MHZ   (100000000)
#define HAL_CLK_DATA_LANE_187_5MHZ (187500000)
#define HAL_CLK_DATA_LANE_200MHZ   (200000000)
#define HAL_CLK_DATA_LANE_312_5MHZ (312500000)
#define HAL_CLK_DATA_LANE_400MHZ   (400000000)
#define HAL_CLK_DATA_LANE_800MHZ   (800000000)
#define HAL_CLK_DATA_LANE_1500MHZ  (1500000000)

#define IS_DATA_LANE_LESS_9M(bps)            (bps < HAL_CLK_DATA_LANE_9MHZ)
#define IS_DATA_LANE_BPS_9M_TO_9_5M(bps)     ((bps >= HAL_CLK_DATA_LANE_9MHZ) && (bps < HAL_CLK_DATA_LANE_9_5MHZ))
#define IS_DATA_LANE_BPS_12_5M_TO_25M(bps)   ((bps > HAL_CLK_DATA_LANE_12_5MHZ) && (bps <= HAL_CLK_DATA_LANE_25MHZ))
#define IS_DATA_LANE_BPS_25M_TO_50M(bps)     ((bps > HAL_CLK_DATA_LANE_25MHZ) && (bps <= HAL_CLK_DATA_LANE_50MHZ))
#define IS_DATA_LANE_BPS_50M_TO_100M(bps)    ((bps > HAL_CLK_DATA_LANE_50MHZ) && (bps <= HAL_CLK_DATA_LANE_100MHZ))
#define IS_DATA_LANE_BPS_100M_TO_312_5M(bps) ((bps > HAL_CLK_DATA_LANE_100MHZ) && (bps <= HAL_CLK_DATA_LANE_312_5MHZ))

u16 lpll_loop_gain[HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX] = {

    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO187D5MHZ NO.0
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ    NO.1
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ     NO.2
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ   NO.3
};

u16 lpll_loop_div[HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX] = {
    8,  // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO187D5MHZ NO.0
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ    NO.1
    32, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ     NO.2
    64, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ   NO.3
};

hal_clk_disp_pnl_lpll_tbl_t lpll_setting_tbl[HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX][HAL_CLK_DISP_PNL_LPLL_REG_MAX] = {
    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO187D5MHZ    NO.0
         *
         * {Address,Value}
         */
        {0x103784, 0x0041},
        {0x103780, 0x0a81},
        {0x103782, 0x0420},
        {0x103786, 0x0000},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x0080}, // div8
    },

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ    NO.1
         *
         * {Address,Value}
         */
        {0x103784, 0x0042},
        {0x103780, 0x0a81},
        {0x103782, 0x0420},
        {0x103786, 0x0001},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x0081}, // div16
    },

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ    NO.2
         *
         * {Address,Value}
         */
        {0x103784, 0x0043},
        {0x103780, 0x0a81},
        {0x103782, 0x0420},
        {0x103786, 0x0002},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x0082}, // div32
    },

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ    NO.3
         *
         * {Address,Value}
         */
        {0x103784, 0x0083},
        {0x103780, 0x0a81},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x0083}, // div64
    },
};

/*
 * the less frequency ones should be placed in front of the table
 */
hal_clk_fuart_tbl fuart_synth_out_tbl[]  = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                           {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart0_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart1_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart2_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart3_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart4_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart5_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart6_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};
hal_clk_fuart_tbl fuart7_synth_out_tbl[] = {{164571000, 0x1500}, {172800000, 0x1400}, {181895000, 0x1300},
                                            {192000000, 0x1200}, {203294000, 0x1100}, {216000000, 0x1000}};

#endif
