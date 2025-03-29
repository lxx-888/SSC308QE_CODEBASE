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

#ifndef _OS02N10_INIT_TABLE_H_
#define _OS02N10_INIT_TABLE_H_

#define _OS02N10_PDWN_ON_ 0
#define _OS02N10_RST_ON_ 0
//#define SENSOR_I2C_ADDR    0x78                   //I2C slave address

//static CMDQ_CMDS gTable[] __attribute__((aligned(8))) =
static SENSOR_INIT_TABLE Sensor_init_table[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_OS02N10_PDWN_ON_),     //power on
    SNR_RST(_OS02N10_RST_ON_),       //reset off
    CMDQ_DELAY_MS(1),
    SNR_MCLK_EN(0x7),               //0x7=MCLK 27MhZ, 0xB=37.125MHz
    CMDQ_DELAY_MS(2),
    SNR_RST(~_OS02N10_RST_ON_),       //reset off
    CMDQ_DELAY_MS(1),                //T1, delay 1ms
    SNR_PDWN(~_OS02N10_PDWN_ON_),     //power on
    CMDQ_DELAY_MS(2),                //T4, delay 5ms
    /*I2C timing*/
    I2CM_CLK_EN(0x2),  //12MHz
    /*
    I2CM_REGW(reg_lcnt,16),
    I2CM_REGW(reg_hcnt,11),
    I2CM_REGW(reg_start_cnt,50),
    I2CM_REGW(reg_stop_cnt,50),
    I2CM_REGW(reg_data_lat_cnt,0),
    I2CM_REGW(reg_sda_cnt,11),
    */
    I2CM_PARAM(16,11,50,50,0,11),
	//I2CM_PARAM(0x39,0x38,0x71,0x71,0x1C,0x1C),
#endif

#if 1
	I2CM_1A1D_W(0X78,0xfc, 0x01),//
	I2CM_1A1D_W(0X78,0xfd, 0x00),//
	I2CM_1A1D_W(0X78,0xba, 0x02),//
	I2CM_1A1D_W(0X78,0xfd, 0x00),//
	I2CM_1A1D_W(0X78,0xb1, 0x14),// ;dpll 480m
	I2CM_1A1D_W(0X78,0xba, 0x00),//
	I2CM_1A1D_W(0X78,0x1a, 0x00),// ;disable dvp
	I2CM_1A1D_W(0X78,0x1b, 0x13),//
	I2CM_1A1D_W(0X78,0xfd, 0x01),//
	I2CM_1A1D_W(0X78,0x0e, 0x00),// ;exp_h
	I2CM_1A1D_W(0X78,0x0f, 0x02),// ;exp_l
	I2CM_1A1D_W(0X78,0x24, 0xff),// ;again
	I2CM_1A1D_W(0X78,0x2f, 0x30),//
	I2CM_1A1D_W(0X78,0xfe, 0x02),//
	I2CM_1A1D_W(0X78,0x2b, 0xff),// ;col_cap
	I2CM_1A1D_W(0X78,0x30, 0x00),//
	I2CM_1A1D_W(0X78,0x31, 0x16),//;rowsg_toggle
	I2CM_1A1D_W(0X78,0x32, 0x25),//
	I2CM_1A1D_W(0X78,0x33, 0xfb),// ;[5]cms
	I2CM_1A1D_W(0X78,0xfd, 0x01),// ;timing
	I2CM_1A1D_W(0X78,0x50, 0x03),//
	I2CM_1A1D_W(0X78,0x51, 0x07),//
	I2CM_1A1D_W(0X78,0x52, 0x04),//
	I2CM_1A1D_W(0X78,0x53, 0x05),//
	I2CM_1A1D_W(0X78,0x57, 0x40),//
	I2CM_1A1D_W(0X78,0x66, 0x04),//;0c
	I2CM_1A1D_W(0X78,0x6d, 0x58),// ;sig_clamp start
	I2CM_1A1D_W(0X78,0x77, 0x01),//
	I2CM_1A1D_W(0X78,0x79, 0x32),//
	I2CM_1A1D_W(0X78,0x7c, 0x01),//;04
	I2CM_1A1D_W(0X78,0x90, 0x3b),//
	I2CM_1A1D_W(0X78,0x91, 0x0b),//;08
	I2CM_1A1D_W(0X78,0x92, 0x18),//
	I2CM_1A1D_W(0X78,0x95, 0x40),//
	I2CM_1A1D_W(0X78,0x99, 0x05),//;dac-samp
	I2CM_1A1D_W(0X78,0xaa, 0x0e),//;0c
	I2CM_1A1D_W(0X78,0xab, 0x0c),//
	I2CM_1A1D_W(0X78,0xac, 0x10),//;0e
	I2CM_1A1D_W(0X78,0xad, 0x10),//
	I2CM_1A1D_W(0X78,0xae, 0x20),//;24
	I2CM_1A1D_W(0X78,0xb0, 0x0e),//;0d
	I2CM_1A1D_W(0X78,0xb1, 0x0f),//
	I2CM_1A1D_W(0X78,0xb2, 0x1a),//;1a
	I2CM_1A1D_W(0X78,0xb3, 0x1c),//
	I2CM_1A1D_W(0X78,0xfd, 0x00),// ;fix reg bug
	I2CM_1A1D_W(0X78,0xb0, 0x00),//
	I2CM_1A1D_W(0X78,0xb1, 0x14),//
	I2CM_1A1D_W(0X78,0xb2, 0x00),//
	I2CM_1A1D_W(0X78,0xb3, 0x10),//
	I2CM_1A1D_W(0X78,0xfd, 0x03),//
	I2CM_1A1D_W(0X78,0x08, 0x00),//;01
	I2CM_1A1D_W(0X78,0x09, 0x20),//;cc ob select col
	I2CM_1A1D_W(0X78,0x0a, 0x02),//
	I2CM_1A1D_W(0X78,0x0b, 0x80),// ;n2p
	I2CM_1A1D_W(0X78,0x11, 0x41),// ;target
	I2CM_1A1D_W(0X78,0x12, 0x41),//
	I2CM_1A1D_W(0X78,0x13, 0x41),//
	I2CM_1A1D_W(0X78,0x14, 0x41),//
	I2CM_1A1D_W(0X78,0x17, 0x72),// ;68;60 ;blc exp coe
	I2CM_1A1D_W(0X78,0x18, 0x6f),// ;68;60
	I2CM_1A1D_W(0X78,0x19, 0x70),// ;68;60
	I2CM_1A1D_W(0X78,0x1a, 0x6f),// ;68;60
	I2CM_1A1D_W(0X78,0x1b, 0xc0),//;blc dc limit lsb
	I2CM_1A1D_W(0X78,0x1d, 0x01),//;blc_dc_limit en
	I2CM_1A1D_W(0X78,0x1f, 0x80),//
	I2CM_1A1D_W(0X78,0x20, 0x40),//
	I2CM_1A1D_W(0X78,0x21, 0x80),//
	I2CM_1A1D_W(0X78,0x22, 0x40),//
	I2CM_1A1D_W(0X78,0x23, 0x88),//
	I2CM_1A1D_W(0X78,0x4b, 0x06),//
	I2CM_1A1D_W(0X78,0x0e, 0x03),//
	I2CM_1A1D_W(0X78,0x58, 0x7b),// ;79;59 ;blc en
	I2CM_1A1D_W(0X78,0x59, 0x17),// ;08 ;auto
	I2CM_1A1D_W(0X78,0x5a, 0x32),// ;blc dc limit msb
	I2CM_1A1D_W(0X78,0xfd, 0x03),//
	I2CM_1A1D_W(0X78,0x4c, 0x01),// ;03
	I2CM_1A1D_W(0X78,0x4d, 0x01),//
	I2CM_1A1D_W(0X78,0x4e, 0x01),// ;03
	I2CM_1A1D_W(0X78,0x4f, 0x02),//
	I2CM_1A1D_W(0X78,0xfd, 0x00),//
	I2CM_1A1D_W(0X78,0x13, 0xbe),// ;icomp1 5.64u
	I2CM_1A1D_W(0X78,0x14, 0x02),//;bitline current
	I2CM_1A1D_W(0X78,0x4c, 0x24),// ;bit3 osc off
	I2CM_1A1D_W(0X78,0xb6, 0x00),// ;reg psnc on
	I2CM_1A1D_W(0X78,0xb7, 0x08),// ;ncp lp op en
	I2CM_1A1D_W(0X78,0xb9, 0xd6),//;d5 ;[7]dac 8bit, [6]dac 2x current
	I2CM_1A1D_W(0X78,0xc6, 0x95),//;a5;c5 ;vlow 1.3
	I2CM_1A1D_W(0X78,0xc7, 0x77),//;aa;bsun sig 2x 1x
	I2CM_1A1D_W(0X78,0xc9, 0x22),//;66 ;bsun rst 2x, 1x 1.92V
	I2CM_1A1D_W(0X78,0xca, 0x32),//;22;66 ;bsun rst 2x, 1x 1.92V
	I2CM_1A1D_W(0X78,0xd7, 0xaa),//;6a
	I2CM_1A1D_W(0X78,0xbc, 0x1f),// ;bc~d1 psnc
	I2CM_1A1D_W(0X78,0xbd, 0x60),//
	I2CM_1A1D_W(0X78,0xbe, 0x78),//
	I2CM_1A1D_W(0X78,0xbf, 0xa5),//
	I2CM_1A1D_W(0X78,0xcb, 0x00),//
	I2CM_1A1D_W(0X78,0xcc, 0x00),//
	I2CM_1A1D_W(0X78,0xce, 0x20),//
	I2CM_1A1D_W(0X78,0xcf, 0x3f),//
	I2CM_1A1D_W(0X78,0xd0, 0x76),//
	I2CM_1A1D_W(0X78,0xd1, 0xec),//
	I2CM_1A1D_W(0X78,0xfd, 0x04),//
	I2CM_1A1D_W(0X78,0x1b, 0x01),// ;00 ;[0]=0 dpc off, =1 dpc on
	I2CM_1A1D_W(0X78,0xfd, 0x03),//
	I2CM_1A1D_W(0X78,0x01, 0x04),//
	I2CM_1A1D_W(0X78,0x02, 0x07),//
	I2CM_1A1D_W(0X78,0x03, 0x80),//
	I2CM_1A1D_W(0X78,0x05, 0x04),//
	I2CM_1A1D_W(0X78,0x06, 0x04),//
	I2CM_1A1D_W(0X78,0x07, 0x38),//
	I2CM_1A1D_W(0X78,0xfd, 0x00),//
	I2CM_1A1D_W(0X78,0x1e, 0x0f),//
	I2CM_1A1D_W(0X78,0x1d, 0xa1),// ;[5]mipi pwdn sel
	I2CM_1A1D_W(0X78,0x21, 0x04),//
	I2CM_1A1D_W(0X78,0x24, 0x02),//
	I2CM_1A1D_W(0X78,0x27, 0x07),// ;mipi output size
	I2CM_1A1D_W(0X78,0x28, 0x80),// //88
	I2CM_1A1D_W(0X78,0x29, 0x04),//
	I2CM_1A1D_W(0X78,0x2a, 0x38),// //40
	I2CM_1A1D_W(0X78,0x2d, 0x04),//
	I2CM_1A1D_W(0X78,0x2e, 0x03),//
	I2CM_1A1D_W(0X78,0x2f, 0x0c),//
	I2CM_1A1D_W(0X78,0x31, 0x04),//
	I2CM_1A1D_W(0X78,0x32, 0x1a),//
	I2CM_1A1D_W(0X78,0x33, 0x04),//
	I2CM_1A1D_W(0X78,0x34, 0x05),//;02
	I2CM_1A1D_W(0X78,0x3f, 0x40),//
	I2CM_1A1D_W(0X78,0x40, 0x94),//
	I2CM_1A1D_W(0X78,0x23, 0x01),// ;mipi en
	I2CM_1A1D_W(0X78,0xfd, 0x03),//
	I2CM_1A1D_W(0X78,0x26, 0x00),// ;01
	I2CM_1A1D_W(0X78,0x28, 0x0a),// ;0c ;L
	I2CM_1A1D_W(0X78,0x29, 0x0a),// ;0d ;0c ;R
	I2CM_1A1D_W(0X78,0x2a, 0x52),// ;37 ;U
	I2CM_1A1D_W(0X78,0x2b, 0x5a),// ;52 ;37 ;D
	I2CM_1A1D_W(0X78,0x2c, 0x0a),// ;0c
	I2CM_1A1D_W(0X78,0x2d, 0x0a),//; 0d ;0c
	I2CM_1A1D_W(0X78,0x2e, 0x52),// ;37
	I2CM_1A1D_W(0X78,0x2f, 0x5a),// ;52 ;37
	I2CM_1A1D_W(0X78,0x31, 0x0a),//; 0c
	I2CM_1A1D_W(0X78,0x32, 0x0a),// ;0d ;0c
	I2CM_1A1D_W(0X78,0x33, 0x52),// ;37
	I2CM_1A1D_W(0X78,0x34, 0x5a),// ;52 ;37
	I2CM_1A1D_W(0X78,0x35, 0x0c),// ;0a ;LU
	I2CM_1A1D_W(0X78,0x36, 0x10),// ;17 ;15 ;LD
	I2CM_1A1D_W(0X78,0x37, 0x07),// ;05 ;RU
	I2CM_1A1D_W(0X78,0x38, 0x0a),// ;10 ;RD
	I2CM_1A1D_W(0X78,0x39, 0x0c),// ;0a
	I2CM_1A1D_W(0X78,0x3a, 0x10),// ;17 ;15
	I2CM_1A1D_W(0X78,0x3b, 0x07),// ;05
	I2CM_1A1D_W(0X78,0x3c, 0x0a),// ;10
	I2CM_1A1D_W(0X78,0x3d, 0x0c),// ;0a
	I2CM_1A1D_W(0X78,0x3e, 0x10),// ;17 ;15
	I2CM_1A1D_W(0X78,0x3f, 0x07),// ;05
	I2CM_1A1D_W(0X78,0x40, 0x0a),// ;10
	I2CM_1A1D_W(0X78,0x41, 0x07),//
	I2CM_1A1D_W(0X78,0x42, 0x07),//
	I2CM_1A1D_W(0X78,0x43, 0x07),//
	I2CM_1A1D_W(0X78,0x44, 0x07),// ;00
	I2CM_1A1D_W(0X78,0x46, 0x10),//
	I2CM_1A1D_W(0X78,0x47, 0xe2),//
	I2CM_1A1D_W(0X78,0x45, 0x50),//
	I2CM_1A1D_W(0X78,0xfb, 0x03),// ;fast exp mode



#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
	//I2CM_2A1D_W(0x60,0x3001, 0x01),//group hold begin
    SNR_SHUTTER_FPS_1A1D(0x60, 10000, 30000),   //shutter 10ms , fps 30
    SNR_GAIN_1A1D(0x78, 1024),
	//I2CM_2A1D_W(0x60,0x3001, 0x00),//group hold end
#endif

	I2CM_1A1D_W(0x78,0xfd,0x00),
    I2CM_1A1D_W(0x78,0x23,0x01),
    CMDQ_DELAY_MS(10),
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
