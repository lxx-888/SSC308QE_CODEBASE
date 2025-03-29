/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _GC2053_INIT_TABLE_H_

#define _GC2053_PDWN_ON_ 0
#define _GC2053_RST_ON_ 0

//static CMDQ_CMDS gTable[] __attribute__((aligned(8))) =
static SENSOR_INIT_TABLE Sensor_init_table[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_GC2053_PDWN_ON_),     //power off
    CMDQ_DELAY_MS(1),             //T1 delay 1ms
    SNR_RST(_GC2053_RST_ON_),       //reset  off
    CMDQ_DELAY_MS(1),             //T1 delay 1ms
    SNR_PDWN(~_GC2053_PDWN_ON_),    //power on
    CMDQ_DELAY_MS(1),   //T2 delay 2ms
    SNR_RST(~_GC2053_RST_ON_),       //reset  on
    CMDQ_DELAY_MS(1),  //T3 delay 1ms

    SNR_MCLK_EN(0x0), //MCLK //0x0=MCLK 27MhZ, 0xB=37.125MHz
    CMDQ_DELAY_MS(1),     //T4 delay 2ms

    /*I2C timing*/
    I2CM_CLK_EN(0x2),  //12MHz

    I2CM_PARAM(16,11,50,50,0,11),
#endif

    I2CM_1A1D_W(0x6E,0xfe,0x80),
    I2CM_1A1D_W(0x6E,0xf2,0x00),    //[1]I2C_open_ena [0]pwd_dn
    I2CM_1A1D_W(0x6E,0xf3,0x00),    //0f//00[3]Sdata_pad_io [2:0]Ssync_pad_io
    I2CM_1A1D_W(0x6E,0xf4,0x36),    //[6:4]pll_ldo_set/ BCWAIT_TIME[9:0]
    I2CM_1A1D_W(0x6E,0xf5,0xc0),    //[7]soc_mclk_enable [6]pll_ldo_en [5:4]cp_clk_sel [3:0]cp_clk_div
    I2CM_1A1D_W(0x6E,0xf6,0x44),    //[7:3]wpllclk_div [2:0]refmp_div
    I2CM_1A1D_W(0x6E,0xf7,0x01),    //[7]refdiv2d5_en [6]refdiv1d5_en [5:4]scaler_mode [3]refmp_enb [1]div2en [0]pllmp_en
    I2CM_1A1D_W(0x6E,0xf8,0x2c),    //[7:0]pllmp_div
    I2CM_1A1D_W(0x6E,0xf9,0x42),    //[7:3]rpllclk_div [2:1]pllmp_prediv [0]analog_pwc
    I2CM_1A1D_W(0x6E,0xfc,0x8e),    // /****CISCTL & ANALOG****/
    I2CM_1A1D_W(0x6E,0xfe,0x00),
    I2CM_1A1D_W(0x6E,0x87,0x18),    //[6]aec_delay_mode
    I2CM_1A1D_W(0x6E,0xee,0x30),    //[5:4]dwen_sramen
    I2CM_1A1D_W(0x6E,0xd0,0xb7),    //ramp_en
    I2CM_1A1D_W(0x6E,0x03,0x04),
    I2CM_1A1D_W(0x6E,0x04,0x60),
    I2CM_1A1D_W(0x6E,0x05,0x04),
    I2CM_1A1D_W(0x6E,0x06,0x4c),    //60//[11:0]hb
    I2CM_1A1D_W(0x6E,0x07,0x00),//
    I2CM_1A1D_W(0x6E,0x08,0x19),//
    I2CM_1A1D_W(0x6E,0x09,0x00),//
    I2CM_1A1D_W(0x6E,0x0a,0x02),   //cisctl row start
    I2CM_1A1D_W(0x6E,0x0b,0x00),
    I2CM_1A1D_W(0x6E,0x0c,0x02),   //cisctl col start
    I2CM_1A1D_W(0x6E,0x0d,0x04),
    I2CM_1A1D_W(0x6E,0x0e,0x40),
    I2CM_1A1D_W(0x6E,0x12,0xe2),   //vsync_ahead_mode
    I2CM_1A1D_W(0x6E,0x13,0x16),
    I2CM_1A1D_W(0x6E,0x19,0x0a),  //ad_pipe_num
    I2CM_1A1D_W(0x6E,0x21,0x1c),   //eqc1fc_eqc2fc_sw
    I2CM_1A1D_W(0x6E,0x28,0x0a),   //16//eqc2_c2clpen_sw
    I2CM_1A1D_W(0x6E,0x29,0x24),   //eq_post_width
    I2CM_1A1D_W(0x6E,0x2b,0x04),   //c2clpen --eqc2
    I2CM_1A1D_W(0x6E,0x32,0xf8),   //[5]txh_en ->avdd28
    I2CM_1A1D_W(0x6E,0x37,0x03),   //[3:2]eqc2sel=0
    I2CM_1A1D_W(0x6E,0x39,0x15),    //17 //[3:0]rsgl
    I2CM_1A1D_W(0x6E,0x43,0x07),    //vclamp
    I2CM_1A1D_W(0x6E,0x44,0x40),    //0e//post_tx_width
    I2CM_1A1D_W(0x6E,0x46,0x0b),
    I2CM_1A1D_W(0x6E,0x4b,0x20),   //rst_tx_width
    I2CM_1A1D_W(0x6E,0x4e,0x08),   //12//ramp_t1_width
    I2CM_1A1D_W(0x6E,0x55,0x20), //read_tx_width_pp
    I2CM_1A1D_W(0x6E,0x66,0x05), //18//stspd_width_r1
    I2CM_1A1D_W(0x6E,0x67,0x05),  //40//5//stspd_width_r
    I2CM_1A1D_W(0x6E,0x77,0x01),  //dacin offset x31
    I2CM_1A1D_W(0x6E,0x78,0x00),  //dacin offset
    I2CM_1A1D_W(0x6E,0x7c,0x93),  //[1:0] co1comp
    I2CM_1A1D_W(0x6E,0x8c,0x12),  //12 ramp_t1_ref
    I2CM_1A1D_W(0x6E,0x8d,0x92),  //90
    I2CM_1A1D_W(0x6E,0x90,0x00),
    I2CM_1A1D_W(0x6E,0x9d,0x10),
    I2CM_1A1D_W(0x6E,0xce,0x7c),//70//78//[4:2]c1isel
    I2CM_1A1D_W(0x6E,0xd2,0x41),//[5:3]c2clamp
    I2CM_1A1D_W(0x6E,0xd3,0xdc),//ec//0x39[7]=0,0xd3[3]=1 rsgh=vref
    I2CM_1A1D_W(0x6E,0xe6,0x50),//ramps offset                 /*gain*/
    I2CM_1A1D_W(0x6E,0xb6,0xc0),
    I2CM_1A1D_W(0x6E,0xb0,0x70),
    I2CM_1A1D_W(0x6E,0xb1,0x01),
    I2CM_1A1D_W(0x6E,0xb2,0x00),
    I2CM_1A1D_W(0x6E,0xb3,0x00),
    I2CM_1A1D_W(0x6E,0xb4,0x00),
    I2CM_1A1D_W(0x6E,0xb8,0x01),
    I2CM_1A1D_W(0x6E,0xb9,0x00),
    /*blk*/
    I2CM_1A1D_W(0x6E,0x26,0x30),//23
    I2CM_1A1D_W(0x6E,0xfe,0x01),
    I2CM_1A1D_W(0x6E,0x40,0x23),
    I2CM_1A1D_W(0x6E,0x55,0x07),
    //I2CM_1A1D_W(0x6E,0x58,0x00),//random noise disable

    I2CM_1A1D_W(0x6E,0x60,0x40), //[7:0]WB_offset
    I2CM_1A1D_W(0x6E,0xfe,0x04),
    I2CM_1A1D_W(0x6E,0x14,0x78), //g1 ratio
    I2CM_1A1D_W(0x6E,0x15,0x78), //r ratio
    I2CM_1A1D_W(0x6E,0x16,0x78), //b ratio
    I2CM_1A1D_W(0x6E,0x17,0x78), //g2 ratio
    /*window*/
    I2CM_1A1D_W(0x6E,0xfe,0x01),
    I2CM_1A1D_W(0x6E,0x92,0x00), //win y1
    I2CM_1A1D_W(0x6E,0x94,0x03), //win x1
    I2CM_1A1D_W(0x6E,0x95,0x04),
    I2CM_1A1D_W(0x6E,0x96,0x38), //[10:0]out_height
    I2CM_1A1D_W(0x6E,0x97,0x07),
    I2CM_1A1D_W(0x6E,0x98,0x80), //[11:0]out_width
    /*ISP*/
    I2CM_1A1D_W(0x6E,0xfe,0x01),
    I2CM_1A1D_W(0x6E,0x58,0x00),
    I2CM_1A1D_W(0x6E,0x83,0x01),
    I2CM_1A1D_W(0x6E,0x87,0x50),
    I2CM_1A1D_W(0x6E,0x01,0x05),//03//[3]dpc blending mode [2]noise_mode [1:0]center_choose 2b'11:median 2b'10:avg 2'b00:near
    I2CM_1A1D_W(0x6E,0x02,0x89), //[7:0]BFF_sram_mode
    I2CM_1A1D_W(0x6E,0x04,0x01), //[0]DD_en
    I2CM_1A1D_W(0x6E,0x07,0xa6),
    I2CM_1A1D_W(0x6E,0x08,0xa9),
    I2CM_1A1D_W(0x6E,0x09,0xa8),
    I2CM_1A1D_W(0x6E,0x0a,0xa7),
    I2CM_1A1D_W(0x6E,0x0b,0xff),
    I2CM_1A1D_W(0x6E,0x0c,0xff),
    I2CM_1A1D_W(0x6E,0x0f,0x00),
    I2CM_1A1D_W(0x6E,0x50,0x1c),
    I2CM_1A1D_W(0x6E,0x89,0x03),
    I2CM_1A1D_W(0x6E,0xfe,0x04),
    I2CM_1A1D_W(0x6E,0x28,0x86),//84
    I2CM_1A1D_W(0x6E,0x29,0x86),//84
    I2CM_1A1D_W(0x6E,0x2a,0x86),//84
    I2CM_1A1D_W(0x6E,0x2b,0x68),//84
    I2CM_1A1D_W(0x6E,0x2c,0x68),//84
    I2CM_1A1D_W(0x6E,0x2d,0x68),//84
    I2CM_1A1D_W(0x6E,0x2e,0x68),//83
    I2CM_1A1D_W(0x6E,0x2f,0x68),//82
    I2CM_1A1D_W(0x6E,0x30,0x4f),//82
    I2CM_1A1D_W(0x6E,0x31,0x68),//82
    I2CM_1A1D_W(0x6E,0x32,0x67),//82
    I2CM_1A1D_W(0x6E,0x33,0x66),//82
    I2CM_1A1D_W(0x6E,0x34,0x66),//82
    I2CM_1A1D_W(0x6E,0x35,0x66),//82
    I2CM_1A1D_W(0x6E,0x36,0x66),//64
    I2CM_1A1D_W(0x6E,0x37,0x66),//68
    I2CM_1A1D_W(0x6E,0x38,0x62),
    I2CM_1A1D_W(0x6E,0x39,0x62),
    I2CM_1A1D_W(0x6E,0x3a,0x62),
    I2CM_1A1D_W(0x6E,0x3b,0x62),
    I2CM_1A1D_W(0x6E,0x3c,0x62),
    I2CM_1A1D_W(0x6E,0x3d,0x62),
    I2CM_1A1D_W(0x6E,0x3e,0x62),
    I2CM_1A1D_W(0x6E,0x3f,0x62),
    /****DVP & MIPI****/
    I2CM_1A1D_W(0x6E,0xfe,0x01),
    I2CM_1A1D_W(0x6E,0x9a,0x06),//[5]OUT_gate_mode [4]hsync_delay_half_pclk [3]data_delay_half_pclk [2]vsync_polarity [1]hsync_polarity [0]pclk_out_polarity
    I2CM_1A1D_W(0x6E,0xfe,0x00),
    I2CM_1A1D_W(0x6E,0x7b,0x2a),//[7:6]updn [5:4]drv_high_data [3:2]drv_low_data [1:0]drv_pclk
    I2CM_1A1D_W(0x6E,0x23,0x2d),//[3]rst_rc [2:1]drv_sync [0]pwd_rc
    I2CM_1A1D_W(0x6E,0xfe,0x03),
    I2CM_1A1D_W(0x6E,0x01,0x27),//20//27[6:5]clkctr [2]phy-lane1_en [1]phy-lane0_en [0]phy_clk_en
    I2CM_1A1D_W(0x6E,0x02,0x56),//[7:6]data1ctr [5:4]data0ctr [3:0]mipi_diff
    I2CM_1A1D_W(0x6E,0x03,0xb6),//b2//b6[7]clklane_p2s_sel [6:5]data0hs_ph [4]data0_delay1s [3]clkdelay1s [2]mipi_en [1:0]clkhs_ph
    I2CM_1A1D_W(0x6E,0x12,0x80),
    I2CM_1A1D_W(0x6E,0x13,0x07),//LWC
    I2CM_1A1D_W(0x6E,0x15,0x10),//[1:0]clk_lane_mode //0x10 no continue mode; 0x12 continue mode
    I2CM_1A1D_W(0x6E,0xfe,0x00),
    I2CM_1A1D_W(0x6E,0x3e,0x91),//40//91[7]lane_ena [6]DVPBUF_ena [5]ULPEna [4]MIPI_ena [3]mipi_set_auto_disable [2]RAW8_mode [1]ine_sync_mode [0]double_lane_en

#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    SNR_SHUTTER_FPS_1A1D(0x6E, 10000, 30000),   //shutter 10ms , fps 30
    SNR_GAIN_1A1D(0x6E, 1024),
#endif

#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 idle*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_READY),
    VIF_REG_DUMMY(CMDQ_STATUS_READY),

    /*append dummy*/
    //CMDQ_TAG('EOT'),    //end of table
    CMDQ_NULL(),
    CMDQ_NULL(),
    CMDQ_NULL(),
    CMDQ_NULL(),
#endif
};

#endif
