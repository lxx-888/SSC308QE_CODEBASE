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

#ifndef _H66_INIT_TABLE_H_

#define _H66_PDWN_ON_ 0
#define _H66_RST_ON_ 0

//static CMDQ_CMDS gTable[] __attribute__((aligned(8))) =
static SENSOR_INIT_TABLE Sensor_init_table[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(~_H66_PDWN_ON_),            //power high
    SNR_RST(_H66_RST_ON_),             //reset low
    CMDQ_DELAY_MS(2),       //T1, delay 2ms

    SNR_RST(~_H66_RST_ON_),            //rst high
    CMDQ_DELAY_MS(2),             //T2, delay 5ms
    SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0xB=37.125MHz, 0x7=27Mhz
    CMDQ_DELAY_MS(2),             //T2, delay 2ms

    SNR_RST(_H66_RST_ON_),       //reset low
    CMDQ_DELAY_MS(12),             //T3, delay > 10ms
    SNR_RST(~_H66_RST_ON_),       //reset high
    CMDQ_DELAY_MS(4),             //T4, delay 4ms
    SNR_PDWN(_H66_PDWN_ON_),            //power low
    CMDQ_DELAY_MS(5),             //T5, delay 5ms
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
#endif

    /*Linear, 2 lane */
    I2CM_1A1D_W(0x80, 0x12,0x40),
    I2CM_1A1D_W(0x80, 0x48,0x85),
    I2CM_1A1D_W(0x80, 0x48,0x05),
    I2CM_1A1D_W(0x80, 0x0E,0x11),
    I2CM_1A1D_W(0x80, 0x0F,0x14),
    I2CM_1A1D_W(0x80, 0x10,0x20),
    I2CM_1A1D_W(0x80, 0x11,0x80),
    I2CM_1A1D_W(0x80, 0x0D,0xF0),
    I2CM_1A1D_W(0x80, 0x5F,0x42),
    I2CM_1A1D_W(0x80, 0x60,0x2B),
    I2CM_1A1D_W(0x80, 0x58,0x18),
    I2CM_1A1D_W(0x80, 0x57,0x60),
    I2CM_1A1D_W(0x80, 0x20,0xD0),
    I2CM_1A1D_W(0x80, 0x21,0x02),
    I2CM_1A1D_W(0x80, 0x22,0xE8),
    I2CM_1A1D_W(0x80, 0x23,0x03),
    I2CM_1A1D_W(0x80, 0x24,0x80),
    I2CM_1A1D_W(0x80, 0x25,0xC0),
    I2CM_1A1D_W(0x80, 0x26,0x32),
    I2CM_1A1D_W(0x80, 0x27,0xCA),
    I2CM_1A1D_W(0x80, 0x28,0x15),
    I2CM_1A1D_W(0x80, 0x29,0x02),
    I2CM_1A1D_W(0x80, 0x2A,0xBF),
    I2CM_1A1D_W(0x80, 0x2B,0x12),
    I2CM_1A1D_W(0x80, 0x2C,0x00),
    I2CM_1A1D_W(0x80, 0x2D,0x00),
    I2CM_1A1D_W(0x80, 0x2E,0xF6),
    I2CM_1A1D_W(0x80, 0x2F,0x40),
    I2CM_1A1D_W(0x80, 0x41,0x84),
    I2CM_1A1D_W(0x80, 0x42,0x12),
    I2CM_1A1D_W(0x80, 0x46,0x10),
    I2CM_1A1D_W(0x80, 0x47,0x42),
    I2CM_1A1D_W(0x80, 0x76,0x40),
    I2CM_1A1D_W(0x80, 0x77,0x06),
    I2CM_1A1D_W(0x80, 0x80,0x01),
    I2CM_1A1D_W(0x80, 0xAF,0x22),
    I2CM_1A1D_W(0x80, 0x1D,0x00),
    I2CM_1A1D_W(0x80, 0x1E,0x04),
    I2CM_1A1D_W(0x80, 0x6C,0x50),
    I2CM_1A1D_W(0x80, 0x6E,0x2C),
    I2CM_1A1D_W(0x80, 0x70,0x90),
    I2CM_1A1D_W(0x80, 0x71,0x8D),
    I2CM_1A1D_W(0x80, 0x72,0xAA),
    I2CM_1A1D_W(0x80, 0x73,0x56),
    I2CM_1A1D_W(0x80, 0x74,0x00),
    I2CM_1A1D_W(0x80, 0x78,0x8F),
    I2CM_1A1D_W(0x80, 0x89,0x01),
    I2CM_1A1D_W(0x80, 0x6B,0x20),
    I2CM_1A1D_W(0x80, 0x86,0x40),
    I2CM_1A1D_W(0x80, 0x2F,0x60),
    I2CM_1A1D_W(0x80, 0x30,0x86),
    I2CM_1A1D_W(0x80, 0x31,0x08),
    I2CM_1A1D_W(0x80, 0x32,0x18),
    I2CM_1A1D_W(0x80, 0x33,0x52),
    I2CM_1A1D_W(0x80, 0x34,0x24),
    I2CM_1A1D_W(0x80, 0x35,0x22),
    I2CM_1A1D_W(0x80, 0x3A,0xA0),
    I2CM_1A1D_W(0x80, 0x3B,0x00),
    I2CM_1A1D_W(0x80, 0x3C,0x42),
    I2CM_1A1D_W(0x80, 0x3D,0x4C),
    I2CM_1A1D_W(0x80, 0x3E,0xD0),
    I2CM_1A1D_W(0x80, 0x56,0x1A),
    I2CM_1A1D_W(0x80, 0x59,0x3C),
    I2CM_1A1D_W(0x80, 0x5A,0x04),
    I2CM_1A1D_W(0x80, 0x84,0x28),
    I2CM_1A1D_W(0x80, 0x85,0x26),
    I2CM_1A1D_W(0x80, 0x8A,0x04),
    I2CM_1A1D_W(0x80, 0x9C,0xE1),
    I2CM_1A1D_W(0x80, 0x5B,0xA0),
    I2CM_1A1D_W(0x80, 0x5C,0x28),
    I2CM_1A1D_W(0x80, 0x5D,0xE4),
    I2CM_1A1D_W(0x80, 0x5E,0x04),
    I2CM_1A1D_W(0x80, 0x64,0xE0),
    I2CM_1A1D_W(0x80, 0x66,0x40),
    I2CM_1A1D_W(0x80, 0x67,0x74),
    I2CM_1A1D_W(0x80, 0x68,0x00),
    I2CM_1A1D_W(0x80, 0x69,0x70),
    I2CM_1A1D_W(0x80, 0x7A,0x62),
    I2CM_1A1D_W(0x80, 0xAE,0x30),
    I2CM_1A1D_W(0x80, 0x13,0x81),
    I2CM_1A1D_W(0x80, 0x96,0x04),
    I2CM_1A1D_W(0x80, 0x4A,0x05),
    I2CM_1A1D_W(0x80, 0x7E,0xCC),
    I2CM_1A1D_W(0x80, 0x50,0x02),
    I2CM_1A1D_W(0x80, 0x49,0x10),
    I2CM_1A1D_W(0x80, 0x7B,0x4A),
    I2CM_1A1D_W(0x80, 0x7C,0x09),
    I2CM_1A1D_W(0x80, 0x7F,0x56),
    I2CM_1A1D_W(0x80, 0x62,0x21),
    I2CM_1A1D_W(0x80, 0x8F,0x80),
    I2CM_1A1D_W(0x80, 0x90,0x00),
    I2CM_1A1D_W(0x80, 0x8C,0xFF),
    I2CM_1A1D_W(0x80, 0x8D,0xC7),
    I2CM_1A1D_W(0x80, 0x8E,0x00),
    I2CM_1A1D_W(0x80, 0x8B,0x01),
    I2CM_1A1D_W(0x80, 0x0C,0x40),
    I2CM_1A1D_W(0x80, 0xA3,0x20),
    I2CM_1A1D_W(0x80, 0xA0,0x01),
    I2CM_1A1D_W(0x80, 0x81,0x74),
    I2CM_1A1D_W(0x80, 0xA2,0x78),
    I2CM_1A1D_W(0x80, 0x82,0x01),
    I2CM_1A1D_W(0x80, 0x19,0x20),
    I2CM_1A1D_W(0x80, 0x12,0x00),
    I2CM_1A1D_W(0x80, 0x48,0x85),
    I2CM_1A1D_W(0x80, 0x48,0x05),

#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    SNR_SHUTTER_FPS_1A1D(0x80, 10000, 30000),   //shutter 10ms , fps 30
    SNR_GAIN_1A1D(0x80, 1024),
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
