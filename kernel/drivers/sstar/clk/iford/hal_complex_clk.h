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
struct hal_clk_pll_info
{
    struct list_head      list;
    const char *          name;
    u8                    enable;
    unsigned long         rate_save;
    const struct clk_ops *pll_ops;
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
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_148D5MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_74D25MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ,
    HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX,
} hal_clk_pnl_lpll_type_e;

typedef struct
{
    unsigned char high;
    unsigned char mid;
    unsigned char low;
    unsigned char outDiv;
    unsigned char postDiv;
    unsigned long freq;
} IPU_FreqTable_t;

typedef struct
{
    u32 frequency;
    u32 synthesizer;
    u8  input_div_first;
    u8  loop_div_first;
    u8  loop_div_second;
    u8  post_div;
} hal_spipll_table;

/*
 * the config is defined to apply control PLL by framework or not
 */

#define HAL_CLK_DISP_PNL_LPLL_REG_NUM (7)
#define HAL_CLK_DISP_PNL_LPLL_REG_MAX (8)

#define HAL_CLK_REG_DISP_LPLL_0_BASE (0x103700UL)
#define HAL_CLK_REG_DISP_LPLL_0_40_L (HAL_CLK_REG_DISP_LPLL_0_BASE + 0x80)
#define HAL_CLK_REG_DISP_LPLL_0_43_L (HAL_CLK_REG_DISP_LPLL_0_BASE + 0x86)
#define HAL_CLK_REG_DISP_LPLL_0_48_L (HAL_CLK_REG_DISP_LPLL_0_BASE + 0x90)
#define HAL_CLK_REG_DISP_LPLL_0_49_L (HAL_CLK_REG_DISP_LPLL_0_BASE + 0x92)

#define HAL_CLK_DATA_LANE_2_8MHZ   (2800000)
#define HAL_CLK_DATA_LANE_3_25MHZ  (3250000)
#define HAL_CLK_DATA_LANE_6_5MHZ   (6500000)
#define HAL_CLK_DATA_LANE_9MHZ     (9000000)
#define HAL_CLK_DATA_LANE_9_5MHZ   (9500000)
#define HAL_CLK_DATA_LANE_12_5MHZ  (12500000)
#define HAL_CLK_DATA_LANE_25MHZ    (25000000)
#define HAL_CLK_DATA_LANE_50MHZ    (50000000)
#define HAL_CLK_DATA_LANE_74_25MHZ (74250000)
#define HAL_CLK_DATA_LANE_100MHZ   (100000000)
#define HAL_CLK_DATA_LANE_148_5MHZ (148500000)
#define HAL_CLK_DATA_LANE_187_5MHZ (187500000)
#define HAL_CLK_DATA_LANE_200MHZ   (200000000)
#define HAL_CLK_DATA_LANE_312_5MHZ (312500000)
#define HAL_CLK_DATA_LANE_400MHZ   (400000000)
#define HAL_CLK_DATA_LANE_800MHZ   (800000000)
#define HAL_CLK_DATA_LANE_1500MHZ  (1500000000)

#define IS_DATA_LANE_LESS_9M(bps)            (bps < HAL_CLK_DATA_LANE_9MHZ)
#define IS_DATA_LANE_LESS_2_8M(bps)          (bps < HAL_CLK_DATA_LANE_2_8MHZ)
#define IS_DATA_LANE_BPS_2_8M_TO_3_25M(bps)  ((bps >= HAL_CLK_DATA_LANE_2_8MHZ) && (bps < HAL_CLK_DATA_LANE_3_25MHZ))
#define IS_DATA_LANE_BPS_3_25M_TO_6_5M(bps)  ((bps >= HAL_CLK_DATA_LANE_3_25MHZ) && (bps < HAL_CLK_DATA_LANE_6_5MHZ))
#define IS_DATA_LANE_BPS_6_5M_TO_12_5M(bps)  ((bps >= HAL_CLK_DATA_LANE_6_5MHZ) && (bps < HAL_CLK_DATA_LANE_12_5MHZ))
#define IS_DATA_LANE_BPS_9M_TO_9_5M(bps)     ((bps >= HAL_CLK_DATA_LANE_9MHZ) && (bps < HAL_CLK_DATA_LANE_9_5MHZ))
#define IS_DATA_LANE_BPS_12_5M_TO_25M(bps)   ((bps > HAL_CLK_DATA_LANE_12_5MHZ) && (bps <= HAL_CLK_DATA_LANE_25MHZ))
#define IS_DATA_LANE_BPS_25M_TO_50M(bps)     ((bps > HAL_CLK_DATA_LANE_25MHZ) && (bps <= HAL_CLK_DATA_LANE_50MHZ))
#define IS_DATA_LANE_BPS_74_25M(bps)         ((bps == HAL_CLK_DATA_LANE_74_25MHZ))
#define IS_DATA_LANE_BPS_50M_TO_100M(bps)    ((bps > HAL_CLK_DATA_LANE_50MHZ) && (bps <= HAL_CLK_DATA_LANE_100MHZ))
#define IS_DATA_LANE_BPS_148_5M(bps)         ((bps == HAL_CLK_DATA_LANE_148_5MHZ))
#define IS_DATA_LANE_BPS_100M_TO_312_5M(bps) ((bps > HAL_CLK_DATA_LANE_100MHZ) && (bps <= HAL_CLK_DATA_LANE_312_5MHZ))

u16 lpll_loop_gain[HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX] = {

    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO187D5MHZ NO.0
    22, // E_HAL_PNL_SUPPORTED_LPLL_HS_LVDS_CH_148D5MHZ             NO.1
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ    NO.2
    22, // E_HAL_PNL_SUPPORTED_LPLL_HS_LVDS_CH_74D25MHZ             NO.3
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ     NO.4
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ   NO.5
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ  NO.6
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ  NO.7
    16, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ  NO.8
};

u16 lpll_loop_div[HAL_CLK_DISP_PNL_SUPPORTED_LPLL_MAX] = {
    8,   // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_100TO187D5MHZ NO.0
    8,   // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_148D5MHZ      NO.1
    16,  // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ    NO.2
    16,  // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_74D25MHZ      NO.3
    32,  // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ     NO.4
    64,  // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ   NO.5
    130, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ  NO.6
    260, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ  NO.7
    300, // HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ  NO.8

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
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_148D5MHZ        NO.1
         *
         * Address,Value
         */
        {0x103780, 0x0a81},
        {0x103782, 0x0B10},
        {0x103784, 0x0041},
        {0x103786, 0x0000},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x0041},
    },

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_50TO100MHZ    NO.2
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
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_74D25MHZ        NO.3
         *
         * Address,Value
         */
        {0x103780, 0x0a81},
        {0x103782, 0x0B10},
        {0x103784, 0x0042},
        {0x103786, 0x0001},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x0042},
    },

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_25TO50MHZ    NO.4
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
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_12D5TO25MHZ    NO.5
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

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_6D5TO12D5MHZ    NO.6
         *
         * {Address,Value}
         */
        {0x103784, 0x00D2},
        {0x103780, 0x2301},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x00D2}, // div130
    },

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_3D25TO6D5MHZ    NO.7
         *
         * {Address,Value}
         */
        {0x103784, 0x00D3},
        {0x103780, 0x2301},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x00D3}, // div260
    },

    {
        /*
         * HAL_CLK_DISP_PNL_SUPPORTED_LPLL_HS_LVDS_CH_2D8TO3D25MHZ    NO.8
         *
         * {Address,Value}
         */
        {0x103784, 0x00F3},
        {0x103780, 0x2301},
        {0x103782, 0x0420},
        {0x103786, 0x0003},
        {0x103788, 0x0500},
        {0x103794, 0x0003},
        {0x103796, 0x0020},
        {0x103784, 0x00F3}, // div300
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

IPU_FreqTable_t stIpuFreqTable[] = {
    /* 600MHz ~ 300MHz */
    {0x2E, 0x14, 0x7B, 0x10, 0x01, 600000000},
    {0x37, 0x4b, 0xc6, 0x10, 0x01, 500000088},
    {0x45, 0x1e, 0xb8, 0x10, 0x01, 400000024},
    {0x2E, 0x14, 0x7B, 0x00, 0x07, 300000000},
};

hal_spipll_table spipll_synth_tbl[] = {
    /* 104MHz ~ 216MHz */
    {104000000, 0x427617, 0x0, 0x2, 0x0, 0x0}, // spipll_104m
    {133000000, 0x19fc27, 0x0, 0x0, 0x4, 0x2}, // spipll_133m
    {160000000, 0x159999, 0x0, 0x0, 0x2, 0x0}, // spipll_160m
    {172000000, 0x1417D0, 0x0, 0x0, 0x2, 0x0}, // spipll_172m
    {208000000, 0x213b13, 0x0, 0x0, 0x4, 0x0}, // spipll_208m
    {216000000, 0x200000, 0x0, 0x0, 0x4, 0x0}, // spipll_216m
};

#endif
