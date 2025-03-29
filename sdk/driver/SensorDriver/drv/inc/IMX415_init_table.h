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

#ifndef _IMX415_INIT_TABLE_H_

#define _IMX415_PDWN_ON_ 0
#define _IMX415_RST_ON_ 0

//Linear
#define _IMX415_RES_4K30_ 1
#define _IMX415_RES_2M60_ 1
#define _IMX415_RES_2560x1440_30_ 1
#define _IMX415_RES_3840x2160_24_ 1

//HDR
#define _IMX415_RES_4K15_HDR 1
#define _IMX415_RES_4K30_HDR 1

#if _IMX415_RES_4K30_
static SENSOR_INIT_TABLE Sensor_init_table_4K30[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_IMX415_PDWN_ON_),     //power off
    SNR_RST(_IMX415_RST_ON_),       //reset  off
    SNR_PDWN(~_IMX415_PDWN_ON_),    //power on
    CMDQ_DELAY_10US(50),            // delay 500Us
    SNR_RST(~_IMX415_RST_ON_),      //reset on
    CMDQ_DELAY_10US(10),             //delay 10us
    SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0xB=37.125MHz


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
    //I2CM_PARAM(16,11,50,50,0,11),
    I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
#endif

#if _IMX415_RES_4K30_
    I2CM_2A1D_W(0x34,0x3000, 0x01),   // standby
    I2CM_2A1D_W(0x34,0x3002, 0x01),   //Master mode stop
    I2CM_2A1D_W(0x34,0x3008, 0x5D),   // BCWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x300A, 0x42),    // CPWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x3024, 0xCA),    // VMAX[19:0]
    I2CM_2A1D_W(0x34,0x3025, 0x08),
    I2CM_2A1D_W(0x34,0x3028, 0x4C),    //HMAX
    I2CM_2A1D_W(0x34,0x3029, 0x04),
    I2CM_2A1D_W(0x34,0x3031, 0x00),    // ADBIT[1:0]
    I2CM_2A1D_W(0x34,0x3032, 0x00),    // MDBIT
    I2CM_2A1D_W(0x34,0x3033, 0x05),    // SYS_MODE[3:0]
    I2CM_2A1D_W(0x34,0x3050, 0x08),    // SHR0[19:0]
    I2CM_2A1D_W(0x34,0x30C1, 0x00),    // XVS_DRV[1:0]
    I2CM_2A1D_W(0x34,0x3116, 0x23),    // INCKSEL2[7:0]
    I2CM_2A1D_W(0x34,0x3118, 0xC6),    // INCKSEL3[10:0]
    I2CM_2A1D_W(0x34,0x311A, 0xE7),    // INCKSEL4[10:0]
    I2CM_2A1D_W(0x34,0x311E, 0x23),    // INCKSEL5[7:0]
    I2CM_2A1D_W(0x34,0x32D4, 0x21),//
    I2CM_2A1D_W(0x34,0x32EC, 0xA1),//
    I2CM_2A1D_W(0x34,0x3452, 0x7F),//
    I2CM_2A1D_W(0x34,0x3453, 0x03),
    I2CM_2A1D_W(0x34,0x358A, 0x04),
    I2CM_2A1D_W(0x34,0x35A1, 0x02),
    I2CM_2A1D_W(0x34,0x36BC, 0x0C),
    I2CM_2A1D_W(0x34,0x36CC, 0x53),  // 2 lane for phy
    I2CM_2A1D_W(0x34,0x36CD, 0x00),//Y-out
    I2CM_2A1D_W(0x34,0x36CE, 0x3C),
    I2CM_2A1D_W(0x34,0x36D0, 0x8C),  // 2 lane
    I2CM_2A1D_W(0x34,0x36D1, 0x00),
    I2CM_2A1D_W(0x34,0x36D2, 0x71),
    I2CM_2A1D_W(0x34,0x36D4, 0x3C),
    I2CM_2A1D_W(0x34,0x36D6, 0x53),
    I2CM_2A1D_W(0x34,0x36D7, 0x00),
    I2CM_2A1D_W(0x34,0x36D8, 0x71),
    I2CM_2A1D_W(0x34,0x36DA, 0x8C),
    I2CM_2A1D_W(0x34,0x36DB, 0x00),
    I2CM_2A1D_W(0x34,0x3701, 0x00),
    I2CM_2A1D_W(0x34,0x3724, 0x02),
    I2CM_2A1D_W(0x34,0x3726, 0x02),//0x9c
    I2CM_2A1D_W(0x34,0x3732, 0x02),   // operating
    I2CM_2A1D_W(0x34,0x3734, 0x03),//
    I2CM_2A1D_W(0x34,0x3736, 0x03),//
    I2CM_2A1D_W(0x34,0x3742, 0x03),//
    I2CM_2A1D_W(0x34,0x3862, 0xE0),
    I2CM_2A1D_W(0x34,0x38CC, 0x30),
    I2CM_2A1D_W(0x34,0x38CD, 0x2F),
    I2CM_2A1D_W(0x34,0x395C, 0x0C),//
    I2CM_2A1D_W(0x34,0x3A42, 0xD1),//
    I2CM_2A1D_W(0x34,0x3A4C, 0x77),//
    I2CM_2A1D_W(0x34,0x3AE0, 0x02),
    I2CM_2A1D_W(0x34,0x3AEC, 0x0C),
    I2CM_2A1D_W(0x34,0x3B00, 0x2E),
    I2CM_2A1D_W(0x34,0x3B06, 0x29),//
    I2CM_2A1D_W(0x34,0x3B98, 0x25),//
    I2CM_2A1D_W(0x34,0x3B99, 0x21),//
    I2CM_2A1D_W(0x34,0x3B9B, 0x13),
    I2CM_2A1D_W(0x34,0x3B9C, 0x13),
    I2CM_2A1D_W(0x34,0x3B9D, 0x13),
    I2CM_2A1D_W(0x34,0x3B9E, 0x13),//
    I2CM_2A1D_W(0x34,0x3BA1, 0x00),//
    I2CM_2A1D_W(0x34,0x3BA2, 0x06),//
    I2CM_2A1D_W(0x34,0x3BA3, 0x0B),
    I2CM_2A1D_W(0x34,0x3BA4, 0x10),
    I2CM_2A1D_W(0x34,0x3BA5, 0x14),
    I2CM_2A1D_W(0x34,0x3BA6, 0x18),//
    I2CM_2A1D_W(0x34,0x3BA7, 0x1A),//
    I2CM_2A1D_W(0x34,0x3BA8, 0x1A),//
    I2CM_2A1D_W(0x34,0x3BA9, 0x1A),
    I2CM_2A1D_W(0x34,0x3BAC, 0xED),
    I2CM_2A1D_W(0x34,0x3BAD, 0x01),
    I2CM_2A1D_W(0x34,0x3BAE, 0xF6),//
    I2CM_2A1D_W(0x34,0x3BAF, 0x02),//
    I2CM_2A1D_W(0x34,0x3BB0, 0xA2),//
    I2CM_2A1D_W(0x34,0x3BB1, 0x03),
    I2CM_2A1D_W(0x34,0x3BB2, 0xE0),
    I2CM_2A1D_W(0x34,0x3BB3, 0x03),
    I2CM_2A1D_W(0x34,0x3BB4, 0xE0),//
    I2CM_2A1D_W(0x34,0x3BB5, 0x03),//
    I2CM_2A1D_W(0x34,0x3BB6, 0xE0),//
    I2CM_2A1D_W(0x34,0x3BB7, 0x03),
    I2CM_2A1D_W(0x34,0x3BB8, 0xE0),
    I2CM_2A1D_W(0x34,0x3BBA, 0xE0),
    I2CM_2A1D_W(0x34,0x3BBC, 0xDA),//
    I2CM_2A1D_W(0x34,0x3BBE, 0x88),//
    I2CM_2A1D_W(0x34,0x3BC0, 0x44),//
    I2CM_2A1D_W(0x34,0x3BC2, 0x7B),
    I2CM_2A1D_W(0x34,0x3BC4, 0xA2),
    I2CM_2A1D_W(0x34,0x3BC8, 0xBD),
    I2CM_2A1D_W(0x34,0x3BCA, 0xBD),//
    I2CM_2A1D_W(0x34,0x4000, 0x11),//mipi clock non-continuous mode enble
    I2CM_2A1D_W(0x34,0x4004, 0xC0),//
    I2CM_2A1D_W(0x34,0x4005, 0x06),//
    I2CM_2A1D_W(0x34,0x400C, 0x00),
    I2CM_2A1D_W(0x34,0x4018, 0x7F),
    I2CM_2A1D_W(0x34,0x401A, 0x37),
    I2CM_2A1D_W(0x34,0x401C, 0x37),//
    I2CM_2A1D_W(0x34,0x401E, 0xF7),//
    I2CM_2A1D_W(0x34,0x401F, 0x00),//
    I2CM_2A1D_W(0x34,0x4020, 0x3F),
    I2CM_2A1D_W(0x34,0x4022, 0x6F),
    I2CM_2A1D_W(0x34,0x4024, 0x3F),
    I2CM_2A1D_W(0x34,0x4026, 0x5F),//
    I2CM_2A1D_W(0x34,0x4028, 0x2F),//
    I2CM_2A1D_W(0x34,0x4074, 0x01),//

    //I2CM_2A1D_W(0x34,0x30E2, 0x00),  // PATGEN blk level 0
    //I2CM_2A1D_W(0x34,0x30E4, 0x01),  // PATGEN
    //I2CM_2A1D_W(0x34,0x30E6, 0x0B),  // PATGEN select color bar
    //I2CM_2A1D_W(0x34,0x3110, 0x20),  // PATGEN
    //I2CM_2A1D_W(0x34,0x32C8, 0x00),  // PATGEN
    //I2CM_2A1D_W(0x34,0x3390, 0x00),  // PATGEN

#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
    SNR_SHUTTER_FPS_2A1D(0x34, 10000, 30000),   //shutter 10ms , fps 30
    SNR_GAIN_2A1D(0x34, 1024),
    SNR_ORIENTATION_2A1D(0x34, 0 , 0),
    I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end

    SNR_STREAM_ONOFF_2A1D(0x34, 1),
#else
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3002, 0x00),
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3000, 0x00),//
#endif

#endif
    //CMDQ_DELAY_MS(10),
    //I2CM_2A1D_W(0x34,0x308c, 0x21),  //pattern mode config

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
#endif //end of 4K30

#if _IMX415_RES_2M60_
static SENSOR_INIT_TABLE Sensor_init_table_2M60[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_IMX415_PDWN_ON_),     //power off
    SNR_RST(_IMX415_RST_ON_),       //reset  off
    SNR_PDWN(~_IMX415_PDWN_ON_),    //power on
    CMDQ_DELAY_10US(50),            // delay 500Us
    SNR_RST(~_IMX415_RST_ON_),      //reset on
    CMDQ_DELAY_10US(10),             //delay 10us
    SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0xB=37.125MHz


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
    //I2CM_PARAM(16,11,50,50,0,11),
    I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
#endif

    I2CM_2A1D_W(0x34,0x3000, 0x01),    //Standby
    I2CM_2A1D_W(0x34,0x3002, 0x01),    //Master mode stop
    I2CM_2A1D_W(0x34,0x3008, 0x5D),  // BCWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x300A, 0x42),  // CPWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x3020, 0x01),  // HADD
    I2CM_2A1D_W(0x34,0x3021, 0x01),  // VADD
    I2CM_2A1D_W(0x34,0x3022, 0x01),  // ADDMODE[1:0]
    I2CM_2A1D_W(0x34,0x3024, 0xF8),  // VMAX[19:0]
    I2CM_2A1D_W(0x34,0x3028, 0x1B),  // HMAX[15:0]
    I2CM_2A1D_W(0x34,0x3031, 0x00),  // ADBIT[1:0]
    I2CM_2A1D_W(0x34,0x3033, 0x05),  // SYS_MODE[3:0]
    I2CM_2A1D_W(0x34,0x3050, 0x08),  // SHR0[19:0]
    I2CM_2A1D_W(0x34,0x30C1, 0x00),  // XVS_DRV[1:0]
    I2CM_2A1D_W(0x34,0x30D9, 0x02),  // DIG_CLP_VSTART[4:0]
    I2CM_2A1D_W(0x34,0x30DA, 0x01),  // DIG_CLP_VNUM[1:0]
    I2CM_2A1D_W(0x34,0x3116, 0x23),  // INCKSEL2[7:0]
    I2CM_2A1D_W(0x34,0x3118, 0xC6),  // INCKSEL3[10:0]
    I2CM_2A1D_W(0x34,0x311A, 0xE7),  // INCKSEL4[10:0]
    I2CM_2A1D_W(0x34,0x311E, 0x23),  // INCKSEL5[7:0]
    I2CM_2A1D_W(0x34,0x32D4, 0x21),  // -
    I2CM_2A1D_W(0x34,0x32EC, 0xA1),  // -
    I2CM_2A1D_W(0x34,0x3452, 0x7F),  // -
    I2CM_2A1D_W(0x34,0x3453, 0x03),  // -
    I2CM_2A1D_W(0x34,0x358A, 0x04),  // -
    I2CM_2A1D_W(0x34,0x35A1, 0x02),  // -
    I2CM_2A1D_W(0x34,0x36BC, 0x0C),  // -
    I2CM_2A1D_W(0x34,0x36CC, 0x53),  // -
    I2CM_2A1D_W(0x34,0x36CD, 0x00),  // -
    I2CM_2A1D_W(0x34,0x36CE, 0x3C),  // -
    I2CM_2A1D_W(0x34,0x36D0, 0x8C),  // -
    I2CM_2A1D_W(0x34,0x36D1, 0x00),  // -
    I2CM_2A1D_W(0x34,0x36D2, 0x71),  // -
    I2CM_2A1D_W(0x34,0x36D4, 0x3C),  // -
    I2CM_2A1D_W(0x34,0x36D6, 0x53),  // -
    I2CM_2A1D_W(0x34,0x36D7, 0x00),  // -
    I2CM_2A1D_W(0x34,0x36D8, 0x71),  // -
    I2CM_2A1D_W(0x34,0x36DA, 0x8C),  // -
    I2CM_2A1D_W(0x34,0x36DB, 0x00),  // -
    I2CM_2A1D_W(0x34,0x3701, 0x00),  // ADBIT1[7:0]
    I2CM_2A1D_W(0x34,0x3724, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3726, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3732, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3734, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3736, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3742, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3862, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x38CC, 0x30),  // -
    I2CM_2A1D_W(0x34,0x38CD, 0x2F),  // -
    I2CM_2A1D_W(0x34,0x395C, 0x0C),  // -
    I2CM_2A1D_W(0x34,0x3A42, 0xD1),  // -
    I2CM_2A1D_W(0x34,0x3A4C, 0x77),  // -
    I2CM_2A1D_W(0x34,0x3AE0, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3AEC, 0x0C),  // -
    I2CM_2A1D_W(0x34,0x3B00, 0x2E),  // -
    I2CM_2A1D_W(0x34,0x3B06, 0x29),  // -
    I2CM_2A1D_W(0x34,0x3B98, 0x25),  // -
    I2CM_2A1D_W(0x34,0x3B99, 0x21),  // -
    I2CM_2A1D_W(0x34,0x3B9B, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9C, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9D, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9E, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3BA1, 0x00),  // -
    I2CM_2A1D_W(0x34,0x3BA2, 0x06),  // -
    I2CM_2A1D_W(0x34,0x3BA3, 0x0B),  // -
    I2CM_2A1D_W(0x34,0x3BA4, 0x10),  // -
    I2CM_2A1D_W(0x34,0x3BA5, 0x14),  // -
    I2CM_2A1D_W(0x34,0x3BA6, 0x18),  // -
    I2CM_2A1D_W(0x34,0x3BA7, 0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BA8, 0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BA9, 0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BAC, 0xED),  // -
    I2CM_2A1D_W(0x34,0x3BAD, 0x01),  // -
    I2CM_2A1D_W(0x34,0x3BAE, 0xF6),  // -
    I2CM_2A1D_W(0x34,0x3BAF, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3BB0, 0xA2),  // -
    I2CM_2A1D_W(0x34,0x3BB1, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB2, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB3, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB4, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB5, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB6, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB7, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB8, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BBA, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BBC, 0xDA),  // -
    I2CM_2A1D_W(0x34,0x3BBE, 0x88),  // -
    I2CM_2A1D_W(0x34,0x3BC0, 0x44),  // -
    I2CM_2A1D_W(0x34,0x3BC2, 0x7B),  // -
    I2CM_2A1D_W(0x34,0x3BC4, 0xA2),  // -
    I2CM_2A1D_W(0x34,0x3BC8, 0xBD),  // -
    I2CM_2A1D_W(0x34,0x3BCA, 0xBD),  // -
    I2CM_2A1D_W(0x34,0x4000, 0x11),//mipi clock non-continuous mode enble
    I2CM_2A1D_W(0x34,0x4004, 0xC0),  // TXCLKESC_FREQ[15:0]
    I2CM_2A1D_W(0x34,0x4005, 0x06),  //
    I2CM_2A1D_W(0x34,0x400C, 0x00),  // INCKSEL6
    I2CM_2A1D_W(0x34,0x4018, 0x7F),  // TCLKPOST[15:0]
    I2CM_2A1D_W(0x34,0x401A, 0x37),  // TCLKPREPARE[15:0]
    I2CM_2A1D_W(0x34,0x401C, 0x37),  // TCLKTRAIL[15:0]
    I2CM_2A1D_W(0x34,0x401E, 0xF7),  // TCLKZERO[15:0]
    I2CM_2A1D_W(0x34,0x401F, 0x00),  //
    I2CM_2A1D_W(0x34,0x4020, 0x3F),  // THSPREPARE[15:0]
    I2CM_2A1D_W(0x34,0x4022, 0x6F),  // THSZERO[15:0]
    I2CM_2A1D_W(0x34,0x4024, 0x3F),  // THSTRAIL[15:0]
    I2CM_2A1D_W(0x34,0x4026, 0x5F),  // THSEXIT[15:0]
    I2CM_2A1D_W(0x34,0x4028, 0x2F),  // TLPX[15:0]
    I2CM_2A1D_W(0x34,0x4074, 0x01),  // INCKSEL7 [2:0]

#if 0 //enable sensor pattern mode
    I2CM_2A1D_W(0x34,0x30E2, 0x00),  // PATGEN blk level 0
    I2CM_2A1D_W(0x34,0x30E4, 0x01),  // PATGEN
    I2CM_2A1D_W(0x34,0x30E6, 0x0B),  // PATGEN select color bar
    I2CM_2A1D_W(0x34,0x3110, 0x20),  // PATGEN
    I2CM_2A1D_W(0x34,0x32C8, 0x00),  // PATGEN
    I2CM_2A1D_W(0x34,0x3390, 0x00),  // PATGEN
#endif

    //CMDQ_DELAY_MS(2),
    //I2CM_2A1D_W(0x34,0x3002, 0x00),    //Master mode start
    //CMDQ_DELAY_MS(2),
    //I2CM_2A1D_W(0x34,0x3000, 0x00),    //Operating

    //CMDQ_DELAY_MS(16),

#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
    SNR_HDR_SHUTTER_FPS_2A1D(0x34, 10000, 1000, 0, 60000),   //shutter 10ms , fps 30
    SNR_HDR_GAIN_2A1D(0x34, 1024, 1024, 0),
    I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end
    SNR_STREAM_ONOFF_2A1D(0x34, 1),
#else
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3002, 0x00),
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3000, 0x00),//
#endif

    //CMDQ_DELAY_MS(10),
    //I2CM_2A1D_W(0x34,0x308c, 0x21),  //pattern mode config

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
#endif //end of 2M60


#if _IMX415_RES_4K15_HDR
static SENSOR_INIT_TABLE Sensor_init_table_4K15_hdr[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_IMX415_PDWN_ON_),     //power off
    SNR_RST(_IMX415_RST_ON_),       //reset  off
    SNR_PDWN(~_IMX415_PDWN_ON_),    //power on
    CMDQ_DELAY_10US(50),            // delay 500Us
    SNR_RST(~_IMX415_RST_ON_),      //reset on
    CMDQ_DELAY_10US(10),             //delay 10us
    SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0xB=37.125MHz


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
    //I2CM_PARAM(16,11,50,50,0,11),
    I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
#endif

    /*HDR resolution id 0*/
    /*
    IMX415-AAQR All-pixel scan CSI-2_4lane 27MHz AD:12bit Output:12bit 891Mbps Master Mode DOL HDR 2frame VC 15fps Integration Time LEF:9.988ms SEF:0.121ms
    Tool ver : Ver6.0  vts:2250 hts:1100
    */
    I2CM_2A1D_W(0x34,0x3000, 0x01),    //Standby
    I2CM_2A1D_W(0x34,0x3002, 0x01),    //Master mode stop
    I2CM_2A1D_W(0x34,0x3008, 0x5D),  // BCWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x300A, 0x42),  // CPWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x3028, 0x4C),  // HMAX[15:0]
    I2CM_2A1D_W(0x34,0x3029, 0x04),  //
    I2CM_2A1D_W(0x34,0x302C, 0x01),  // WDMODE[1:0]
    I2CM_2A1D_W(0x34,0x302D, 0x01),  // WDSEL[1:0]
    I2CM_2A1D_W(0x34,0x3033, 0x05),  // SYS_MODE[3:0]
    I2CM_2A1D_W(0x34,0x3060, 0x11),  // RHS1[19:0]
    I2CM_2A1D_W(0x34,0x3050, 0xF2),  // SHR0[19:0]
    I2CM_2A1D_W(0x34,0x3051, 0x0E),  // SHR0
    I2CM_2A1D_W(0x34,0x3054, 0x09),  // SHR1[19:0]
    I2CM_2A1D_W(0x34,0x30C1, 0x00),  // XVS_DRV[1:0]
    I2CM_2A1D_W(0x34,0x30CF, 0x01),  // XVSMSKCNT_INT[1:0]
    I2CM_2A1D_W(0x34,0x3116, 0x23),  // INCKSEL2[7:0]
    I2CM_2A1D_W(0x34,0x3118, 0xC6),  // INCKSEL3[10:0]
    I2CM_2A1D_W(0x34,0x311A, 0xE7),  // INCKSEL4[10:0]
    I2CM_2A1D_W(0x34,0x311E, 0x23),  // INCKSEL5[7:0]
    I2CM_2A1D_W(0x34,0x32D4, 0x21),  // -
    I2CM_2A1D_W(0x34,0x32EC, 0xA1),  // -
    I2CM_2A1D_W(0x34,0x3452, 0x7F),  // -
    I2CM_2A1D_W(0x34,0x3453, 0x03),  // -
    I2CM_2A1D_W(0x34,0x358A, 0x04),  // -
    I2CM_2A1D_W(0x34,0x35A1, 0x02),  // -
    I2CM_2A1D_W(0x34,0x36BC, 0x0C),  // -
    I2CM_2A1D_W(0x34,0x36CC, 0x53),  // -
    I2CM_2A1D_W(0x34,0x36CD, 0x00),  // -
    I2CM_2A1D_W(0x34,0x36CE, 0x3C),  // -
    I2CM_2A1D_W(0x34,0x36D0, 0x8C),  // -
    I2CM_2A1D_W(0x34,0x36D1, 0x00),  // -
    I2CM_2A1D_W(0x34,0x36D2, 0x71),  // -
    I2CM_2A1D_W(0x34,0x36D4, 0x3C),  // -
    I2CM_2A1D_W(0x34,0x36D6, 0x53),  // -
    I2CM_2A1D_W(0x34,0x36D7, 0x00),  // -
    I2CM_2A1D_W(0x34,0x36D8, 0x71),  // -
    I2CM_2A1D_W(0x34,0x36DA, 0x8C),  // -
    I2CM_2A1D_W(0x34,0x36DB, 0x00),  // -
    I2CM_2A1D_W(0x34,0x3724, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3726, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3732, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3734, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3736, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3742, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3862, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x38CC, 0x30),  // -
    I2CM_2A1D_W(0x34,0x38CD, 0x2F),  // -
    I2CM_2A1D_W(0x34,0x395C, 0x0C),  // -
    I2CM_2A1D_W(0x34,0x3A42, 0xD1),  // -
    I2CM_2A1D_W(0x34,0x3A4C, 0x77),  // -
    I2CM_2A1D_W(0x34,0x3AE0, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3AEC, 0x0C),  // -
    I2CM_2A1D_W(0x34,0x3B00, 0x2E),  // -
    I2CM_2A1D_W(0x34,0x3B06, 0x29),  // -
    I2CM_2A1D_W(0x34,0x3B98, 0x25),  // -
    I2CM_2A1D_W(0x34,0x3B99, 0x21),  // -
    I2CM_2A1D_W(0x34,0x3B9B, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9C, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9D, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9E, 0x13),  // -
    I2CM_2A1D_W(0x34,0x3BA1, 0x00),  // -
    I2CM_2A1D_W(0x34,0x3BA2, 0x06),  // -
    I2CM_2A1D_W(0x34,0x3BA3, 0x0B),  // -
    I2CM_2A1D_W(0x34,0x3BA4, 0x10),  // -
    I2CM_2A1D_W(0x34,0x3BA5, 0x14),  // -
    I2CM_2A1D_W(0x34,0x3BA6, 0x18),  // -
    I2CM_2A1D_W(0x34,0x3BA7, 0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BA8, 0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BA9, 0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BAC, 0xED),  // -
    I2CM_2A1D_W(0x34,0x3BAD, 0x01),  // -
    I2CM_2A1D_W(0x34,0x3BAE, 0xF6),  // -
    I2CM_2A1D_W(0x34,0x3BAF, 0x02),  // -
    I2CM_2A1D_W(0x34,0x3BB0, 0xA2),  // -
    I2CM_2A1D_W(0x34,0x3BB1, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB2, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB3, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB4, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB5, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB6, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB7, 0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB8, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BBA, 0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BBC, 0xDA),  // -
    I2CM_2A1D_W(0x34,0x3BBE, 0x88),  // -
    I2CM_2A1D_W(0x34,0x3BC0, 0x44),  // -
    I2CM_2A1D_W(0x34,0x3BC2, 0x7B),  // -
    I2CM_2A1D_W(0x34,0x3BC4, 0xA2),  // -
    I2CM_2A1D_W(0x34,0x3BC8, 0xBD),  // -
    I2CM_2A1D_W(0x34,0x3BCA, 0xBD),  // -
    I2CM_2A1D_W(0x34,0x4000, 0x11),  //
    I2CM_2A1D_W(0x34,0x4004, 0xC0),  // TXCLKESC_FREQ[15:0]
    I2CM_2A1D_W(0x34,0x4005, 0x06),  //
    I2CM_2A1D_W(0x34,0x400C, 0x00),  // INCKSEL6
    I2CM_2A1D_W(0x34,0x4018, 0x7F),  // TCLKPOST[15:0]
    I2CM_2A1D_W(0x34,0x401A, 0x37),  // TCLKPREPARE[15:0]
    I2CM_2A1D_W(0x34,0x401C, 0x37),  // TCLKTRAIL[15:0]
    I2CM_2A1D_W(0x34,0x401E, 0xF7),  // TCLKZERO[15:0]
    I2CM_2A1D_W(0x34,0x401F, 0x00),  //
    I2CM_2A1D_W(0x34,0x4020, 0x3F),  // THSPREPARE[15:0]
    I2CM_2A1D_W(0x34,0x4022, 0x6F),  // THSZERO[15:0]
    I2CM_2A1D_W(0x34,0x4024, 0x3F),  // THSTRAIL[15:0]
    I2CM_2A1D_W(0x34,0x4026, 0x5F),  // THSEXIT[15:0]
    I2CM_2A1D_W(0x34,0x4028, 0x2F),  // TLPX[15:0]
    I2CM_2A1D_W(0x34,0x4074, 0x01),  // INCKSEL7 [2:0]
    CMDQ_DELAY_MS(10),            // delay 10ms
    I2CM_2A1D_W(0x34,0x3260, 0x00),    //Gain_Pgc_Fidmd

#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
    SNR_HDR_SHUTTER_FPS_2A1D(0x34, 10000, 1000, 0, 15000),   //shutter 10ms , fps 30
    SNR_HDR_GAIN_2A1D(0x34, 1024, 1024, 0),
    SNR_ORIENTATION_2A1D(0x34, 0, 0),
    I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end
    SNR_CSI_CONFIG(384000000, 0x1C000000,0x0000, 4, 11),
    SNR_STREAM_ONOFF_2A1D(0x34, 1),
#else
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3002, 0x00),
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3000, 0x00),//
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
#endif //endof _IMX415_RES_4K15_HDR

#if _IMX415_RES_4K30_HDR
static SENSOR_INIT_TABLE Sensor_init_table_4k30_hdr[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_IMX415_PDWN_ON_),     //power off
    SNR_RST(_IMX415_RST_ON_),       //reset  off
    SNR_PDWN(~_IMX415_PDWN_ON_),    //power on
    CMDQ_DELAY_10US(50),            // delay 500Us
    SNR_RST(~_IMX415_RST_ON_),      //reset on
    CMDQ_DELAY_10US(10),             //delay 10us
    SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0xB=37.125MHz


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
    //I2CM_PARAM(16,11,50,50,0,11),
    I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
#endif

    /*HDR resolution ID 2*/
    /*
        IMX415-AAQRAll-pixel scan
        CSI-2_4lane
        27MHz
        AD:10bit Output:10bit
        1485Mbps
        Master Mode DOL HDR 2frame VC
        30.002fps
        Integration Time LEF:1.002ms SEF:0.118ms
    */
    I2CM_2A1D_W(0x34,0x3000,0x01),    //Standby
    I2CM_2A1D_W(0x34,0x3002,0x01),    //Master mode stop
    I2CM_2A1D_W(0x34,0x3008,0x5D),  // BCWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x300A,0x42),  // CPWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34,0x3024,0xFC),  // VMAX[15:0]  //0xAC8 for 25fps
    I2CM_2A1D_W(0x34,0x3025,0x08),  //
    I2CM_2A1D_W(0x34,0x3028,0x1A),  // HMAX[15:0]
    I2CM_2A1D_W(0x34,0x3029,0x02),  //
    I2CM_2A1D_W(0x34,0x302C,0x01),  // WDMODE[1:0]
    I2CM_2A1D_W(0x34,0x302D,0x01),  // WDSEL[1:0]
    I2CM_2A1D_W(0x34,0x3031,0x00),  //
    I2CM_2A1D_W(0x34,0x3032,0x00),  //
    I2CM_2A1D_W(0x34,0x3033,0x08),  // SYS_MODE[3:0]
    I2CM_2A1D_W(0x34,0x3050,0x6E),  // SHR0[19:0]
    I2CM_2A1D_W(0x34,0x3051,0x11),  //
    I2CM_2A1D_W(0x34,0x3054,0x09),  // SHR1[19:0]
    I2CM_2A1D_W(0x34,0x3060,0x19),  // RHS1[19:0]
    I2CM_2A1D_W(0x34,0x30C1,0x00),  // XVS_DRV[1:0]
    I2CM_2A1D_W(0x34,0x30CF,0x01),  // XVSMSKCNT_INT[1:0]
    I2CM_2A1D_W(0x34,0x3116,0x23),  // INCKSEL2[7:0]
    I2CM_2A1D_W(0x34,0x3118,0xA5),  // INCKSEL3[10:0]
    I2CM_2A1D_W(0x34,0x311A,0xE7),  // INCKSEL4[10:0]
    I2CM_2A1D_W(0x34,0x311E,0x23),  // INCKSEL5[7:0]
    I2CM_2A1D_W(0x34,0x32D4,0x21),  // -
    I2CM_2A1D_W(0x34,0x32EC,0xA1),  // -
    I2CM_2A1D_W(0x34,0x3452,0x7F),  // -
    I2CM_2A1D_W(0x34,0x3453,0x03),  // -
    I2CM_2A1D_W(0x34,0x358A,0x04),  // -
    I2CM_2A1D_W(0x34,0x35A1,0x02),  // -
    I2CM_2A1D_W(0x34,0x36BC,0x0C),  // -
    I2CM_2A1D_W(0x34,0x36CC,0x53),  // -
    I2CM_2A1D_W(0x34,0x36CD,0x00),  // -
    I2CM_2A1D_W(0x34,0x36CE,0x3C),  // -
    I2CM_2A1D_W(0x34,0x36D0,0x8C),  // -
    I2CM_2A1D_W(0x34,0x36D1,0x00),  // -
    I2CM_2A1D_W(0x34,0x36D2,0x71),  // -
    I2CM_2A1D_W(0x34,0x36D4,0x3C),  // -
    I2CM_2A1D_W(0x34,0x36D6,0x53),  // -
    I2CM_2A1D_W(0x34,0x36D7,0x00),  // -
    I2CM_2A1D_W(0x34,0x36D8,0x71),  // -
    I2CM_2A1D_W(0x34,0x36DA,0x8C),  // -
    I2CM_2A1D_W(0x34,0x36DB,0x00),  // -
    I2CM_2A1D_W(0x34,0x3701,0x00),  // -
    I2CM_2A1D_W(0x34,0x3724,0x02),  // -
    I2CM_2A1D_W(0x34,0x3726,0x02),  // -
    I2CM_2A1D_W(0x34,0x3732,0x02),  // -
    I2CM_2A1D_W(0x34,0x3734,0x03),  // -
    I2CM_2A1D_W(0x34,0x3736,0x03),  // -
    I2CM_2A1D_W(0x34,0x3742,0x03),  // -
    I2CM_2A1D_W(0x34,0x3862,0xE0),  // -
    I2CM_2A1D_W(0x34,0x38CC,0x30),  // -
    I2CM_2A1D_W(0x34,0x38CD,0x2F),  // -
    I2CM_2A1D_W(0x34,0x395C,0x0C),  // -
    I2CM_2A1D_W(0x34,0x3A42,0xD1),  // -
    I2CM_2A1D_W(0x34,0x3A4C,0x77),  // -
    I2CM_2A1D_W(0x34,0x3AE0,0x02),  // -
    I2CM_2A1D_W(0x34,0x3AEC,0x0C),  // -
    I2CM_2A1D_W(0x34,0x3B00,0x2E),  // -
    I2CM_2A1D_W(0x34,0x3B06,0x29),  // -
    I2CM_2A1D_W(0x34,0x3B98,0x25),  // -
    I2CM_2A1D_W(0x34,0x3B99,0x21),  // -
    I2CM_2A1D_W(0x34,0x3B9B,0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9C,0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9D,0x13),  // -
    I2CM_2A1D_W(0x34,0x3B9E,0x13),  // -
    I2CM_2A1D_W(0x34,0x3BA1,0x00),  // -
    I2CM_2A1D_W(0x34,0x3BA2,0x06),  // -
    I2CM_2A1D_W(0x34,0x3BA3,0x0B),  // -
    I2CM_2A1D_W(0x34,0x3BA4,0x10),  // -
    I2CM_2A1D_W(0x34,0x3BA5,0x14),  // -
    I2CM_2A1D_W(0x34,0x3BA6,0x18),  // -
    I2CM_2A1D_W(0x34,0x3BA7,0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BA8,0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BA9,0x1A),  // -
    I2CM_2A1D_W(0x34,0x3BAC,0xED),  // -
    I2CM_2A1D_W(0x34,0x3BAD,0x01),  // -
    I2CM_2A1D_W(0x34,0x3BAE,0xF6),  // -
    I2CM_2A1D_W(0x34,0x3BAF,0x02),  // -
    I2CM_2A1D_W(0x34,0x3BB0,0xA2),  // -
    I2CM_2A1D_W(0x34,0x3BB1,0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB2,0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB3,0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB4,0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB5,0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB6,0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BB7,0x03),  // -
    I2CM_2A1D_W(0x34,0x3BB8,0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BBA,0xE0),  // -
    I2CM_2A1D_W(0x34,0x3BBC,0xDA),  // -
    I2CM_2A1D_W(0x34,0x3BBE,0x88),  // -
    I2CM_2A1D_W(0x34,0x3BC0,0x44),  // -
    I2CM_2A1D_W(0x34,0x3BC2,0x7B),  // -
    I2CM_2A1D_W(0x34,0x3BC4,0xA2),  // -
    I2CM_2A1D_W(0x34,0x3BC8,0xBD),  // -
    I2CM_2A1D_W(0x34,0x3BCA,0xBD),  // -
    I2CM_2A1D_W(0x34,0x4004,0xC0),  // TXCLKESC_FREQ[15:0]
    I2CM_2A1D_W(0x34,0x4000, 0x11),  //
    I2CM_2A1D_W(0x34,0x4005,0x06),  //
    I2CM_2A1D_W(0x34,0x4018,0xA7),  // TCLKPOST[15:0]
    I2CM_2A1D_W(0x34,0x401A,0x57),  // TCLKPREPARE[15:0]
    I2CM_2A1D_W(0x34,0x401C,0x5F),  // TCLKTRAIL[15:0]
    I2CM_2A1D_W(0x34,0x401E,0x97),  // TCLKZERO[15:0]
    I2CM_2A1D_W(0x34,0x4020,0x5F),  // THSPREPARE[15:0]
    I2CM_2A1D_W(0x34,0x4022,0xAF),  // THSZERO[15:0]
    I2CM_2A1D_W(0x34,0x4024,0x5F),  // THSTRAIL[15:0]
    I2CM_2A1D_W(0x34,0x4026,0x9F),  // THSEXIT[15:0]
    I2CM_2A1D_W(0x34,0x4028,0x4F),  // TLPX[15:0]
    CMDQ_DELAY_MS(10),            // delay 10ms
    I2CM_2A1D_W(0x34,0x3260,0x00),    //Gain_Pgc_Fidmd
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
    SNR_HDR_SHUTTER_FPS_2A1D(0x34, 10000, 1000, 0, 30000),   //shutter 10ms , fps 30
    SNR_HDR_GAIN_2A1D(0x34, 1024, 1024, 0),
    SNR_ORIENTATION_2A1D(0x34, 0, 0),
    I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end
    SNR_CSI_CONFIG(384000000, 0x1C000000,0x0000, 4, 11),
    SNR_STREAM_ONOFF_2A1D(0x34, 1),
#else
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3002, 0x00),
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3000, 0x00),//
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
#endif //end of _IMX415_RES_4K30_HDR

#if _IMX415_RES_2560x1440_30_
static SENSOR_INIT_TABLE Sensor_init_table_2560x1440_30_linear_[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_IMX415_PDWN_ON_),     //power off
    SNR_RST(_IMX415_RST_ON_),       //reset  off
    SNR_PDWN(~_IMX415_PDWN_ON_),    //power on
    CMDQ_DELAY_10US(50),            // delay 500Us
    SNR_RST(~_IMX415_RST_ON_),      //reset on
    CMDQ_DELAY_10US(10),             //delay 10us
    SNR_MCLK_EN(0x0),               //0x0=MCLK 27MhZ, 0xB=37.125MHz


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
    //I2CM_PARAM(16,11,50,50,0,11),
    I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
#endif

    /*HDR resolution ID 2*/
    /*
        IMX415-AAQRAll-pixel scan
        CSI-2_4lane
        27MHz
        AD:10bit Output:10bit
        1485Mbps
        Master Mode DOL HDR 2frame VC
        30.002fps
        Integration Time LEF:1.002ms SEF:0.118ms
    */
    I2CM_2A1D_W(0x34, 0x3000, 0x01),  //Standby
    I2CM_2A1D_W(0x34, 0x3002, 0x01),   //Master mode stop
    I2CM_2A1D_W(0x34, 0x3008, 0x5D),  // BCWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34, 0x300A, 0x42),  // CPWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34, 0x301C, 0x04),  // WINMODE[3:0]
    I2CM_2A1D_W(0x34, 0x3028, 0x4C),  // HMAX[15:0]
    I2CM_2A1D_W(0x34, 0x3029, 0x04),  //
    I2CM_2A1D_W(0x34, 0x3031, 0x00),  // ADBIT[1:0]
    I2CM_2A1D_W(0x34, 0x3032, 0x00),  // MDBIT
    I2CM_2A1D_W(0x34, 0x3033, 0x05),  // SYS_MODE[3:0]
    I2CM_2A1D_W(0x34, 0x3040, 0x7C),  // PIX_HST[12:0]
    I2CM_2A1D_W(0x34, 0x3041, 0x02),  //
    I2CM_2A1D_W(0x34, 0x3042, 0x20),  // PIX_HWIDTH[12:0]
    I2CM_2A1D_W(0x34, 0x3043, 0x0A),  //
    I2CM_2A1D_W(0x34, 0x3044, 0xD0),  // PIX_VST[12:0] //D2
    I2CM_2A1D_W(0x34, 0x3045, 0x02),  //
    I2CM_2A1D_W(0x34, 0x3046, 0x7C),  // PIX_VWIDTH[12:0]
    I2CM_2A1D_W(0x34, 0x3047, 0x0B),  //
    I2CM_2A1D_W(0x34, 0x3050, 0x08),  // SHR0[19:0]
    I2CM_2A1D_W(0x34, 0x30C1, 0x00),  // XVS_DRV[1:0]
    I2CM_2A1D_W(0x34, 0x3116, 0x23),  // INCKSEL2[7:0]
    I2CM_2A1D_W(0x34, 0x3118, 0xC6),  // INCKSEL3[10:0]
    I2CM_2A1D_W(0x34, 0x311A, 0xE7),  // INCKSEL4[10:0]
    I2CM_2A1D_W(0x34, 0x311E, 0x23),  // INCKSEL5[7:0]
    I2CM_2A1D_W(0x34, 0x32D4, 0x21),  // -
    I2CM_2A1D_W(0x34, 0x32EC, 0xA1),  // -
    I2CM_2A1D_W(0x34, 0x3452, 0x7F),  // -
    I2CM_2A1D_W(0x34, 0x3453, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x358A, 0x04),  // -
    I2CM_2A1D_W(0x34, 0x35A1, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x36BC, 0x0C),  // -
    I2CM_2A1D_W(0x34, 0x36CC, 0x53),  // -
    I2CM_2A1D_W(0x34, 0x36CD, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x36CE, 0x3C),  // -
    I2CM_2A1D_W(0x34, 0x36D0, 0x8C),  // -
    I2CM_2A1D_W(0x34, 0x36D1, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x36D2, 0x71),  // -
    I2CM_2A1D_W(0x34, 0x36D4, 0x3C),  // -
    I2CM_2A1D_W(0x34, 0x36D6, 0x53),  // -
    I2CM_2A1D_W(0x34, 0x36D7, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x36D8, 0x71),  // -
    I2CM_2A1D_W(0x34, 0x36DA, 0x8C),  // -
    I2CM_2A1D_W(0x34, 0x36DB, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x3701, 0x00),  // ADBIT1[7:0]
    I2CM_2A1D_W(0x34, 0x3724, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3726, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3732, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3734, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3736, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3742, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3862, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x38CC, 0x30),  // -
    I2CM_2A1D_W(0x34, 0x38CD, 0x2F),  // -
    I2CM_2A1D_W(0x34, 0x395C, 0x0C),  // -
    I2CM_2A1D_W(0x34, 0x3A42, 0xD1),  // -
    I2CM_2A1D_W(0x34, 0x3A4C, 0x77),  // -
    I2CM_2A1D_W(0x34, 0x3AE0, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3AEC, 0x0C),  // -
    I2CM_2A1D_W(0x34, 0x3B00, 0x2E),  // -
    I2CM_2A1D_W(0x34, 0x3B06, 0x29),  // -
    I2CM_2A1D_W(0x34, 0x3B98, 0x25),  // -
    I2CM_2A1D_W(0x34, 0x3B99, 0x21),  // -
    I2CM_2A1D_W(0x34, 0x3B9B, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3B9C, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3B9D, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3B9E, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3BA1, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x3BA2, 0x06),  // -
    I2CM_2A1D_W(0x34, 0x3BA3, 0x0B),  // -
    I2CM_2A1D_W(0x34, 0x3BA4, 0x10),  // -
    I2CM_2A1D_W(0x34, 0x3BA5, 0x14),  // -
    I2CM_2A1D_W(0x34, 0x3BA6, 0x18),  // -
    I2CM_2A1D_W(0x34, 0x3BA7, 0x1A),  // -
    I2CM_2A1D_W(0x34, 0x3BA8, 0x1A),  // -
    I2CM_2A1D_W(0x34, 0x3BA9, 0x1A),  // -
    I2CM_2A1D_W(0x34, 0x3BAC, 0xED),  // -
    I2CM_2A1D_W(0x34, 0x3BAD, 0x01),  // -
    I2CM_2A1D_W(0x34, 0x3BAE, 0xF6),  // -
    I2CM_2A1D_W(0x34, 0x3BAF, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3BB0, 0xA2),  // -
    I2CM_2A1D_W(0x34, 0x3BB1, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB2, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BB3, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB4, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BB5, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB6, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BB7, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB8, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BBA, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BBC, 0xDA),  // -
    I2CM_2A1D_W(0x34, 0x3BBE, 0x88),  // -
    I2CM_2A1D_W(0x34, 0x3BC0, 0x44),  // -
    I2CM_2A1D_W(0x34, 0x3BC2, 0x7B),  // -
    I2CM_2A1D_W(0x34, 0x3BC4, 0xA2),  // -
    I2CM_2A1D_W(0x34, 0x3BC8, 0xBD),  // -
    I2CM_2A1D_W(0x34, 0x3BCA, 0xBD),  // -
    I2CM_2A1D_W(0x34, 0x4000, 0x11),  //mipi clk non-continouns mode
    I2CM_2A1D_W(0x34, 0x4004, 0xC0),  // TXCLKESC_FREQ[15:0]
    I2CM_2A1D_W(0x34, 0x4005, 0x06),  //
    I2CM_2A1D_W(0x34, 0x400C, 0x00),  // INCKSEL6
    I2CM_2A1D_W(0x34, 0x4018, 0x7F),  // TCLKPOST[15:0]
    I2CM_2A1D_W(0x34, 0x401A, 0x37),  // TCLKPREPARE[15:0]
    I2CM_2A1D_W(0x34, 0x401C, 0x37),  // TCLKTRAIL[15:0]
    I2CM_2A1D_W(0x34, 0x401E, 0xF7),  // TCLKZERO[15:0]
    I2CM_2A1D_W(0x34, 0x401F, 0x00),  //
    I2CM_2A1D_W(0x34, 0x4020, 0x3F),  // THSPREPARE[15:0]
    I2CM_2A1D_W(0x34, 0x4022, 0x6F),  // THSZERO[15:0]
    I2CM_2A1D_W(0x34, 0x4024, 0x3F),  // THSTRAIL[15:0]
    I2CM_2A1D_W(0x34, 0x4026, 0x5F),  // THSEXIT[15:0]
    I2CM_2A1D_W(0x34, 0x4028, 0x2F),  // TLPX[15:0]
    I2CM_2A1D_W(0x34, 0x4074, 0x01),  // INCKSEL7 [2:0]

    CMDQ_DELAY_MS(10),                // delay 10ms
    I2CM_2A1D_W(0x34,0x3260,0x00),    //Gain_Pgc_Fidmd
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
    SNR_HDR_SHUTTER_FPS_2A1D(0x34, 10000, 1000, 0, 30000),   //shutter 10ms , fps 30
    SNR_HDR_GAIN_2A1D(0x34, 1024, 1024, 0),
    SNR_ORIENTATION_2A1D(0x34, 0, 0),
    I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end

    SNR_STREAM_ONOFF_2A1D(0x34, 1),
#else
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3002, 0x00),
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3000, 0x00),//
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
#endif //end of _IMX415_RES_2560x1440_30_

#if _IMX415_RES_3840x2160_24_
static SENSOR_INIT_TABLE Sensor_init_table_3840x2160_24_linear_[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(_IMX415_PDWN_ON_),     //power off
    SNR_RST(_IMX415_RST_ON_),       //reset  off
    SNR_PDWN(~_IMX415_PDWN_ON_),    //power on
    CMDQ_DELAY_10US(50),            // delay 500Us
    SNR_RST(~_IMX415_RST_ON_),      //reset on
    CMDQ_DELAY_10US(10),            //delay 10us
    SNR_MCLK_EN(0x7),               //0x7=MCLK 24MhZ, 0xB=37.125MHz


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
    //I2CM_PARAM(16,11,50,50,0,11),
    I2CM_PARAM(14,7,10,10,0,11), //lcnt, hcnt, start cnt, stop cnt, data late cnt, sda cnt
#endif

    /*HDR resolution ID 2*/
    /*
        IMX415-AAQRAll-pixel scan
        CSI-2_4lane
        27MHz
        AD:10bit Output:10bit
        1485Mbps
        Master Mode DOL HDR 2frame VC
        30.002fps
        Integration Time LEF:1.002ms SEF:0.118ms
    */
    I2CM_2A1D_W(0x34, 0x3000, 0x01),  // Standby
    I2CM_2A1D_W(0x34, 0x3002, 0x01),  // Master mode stop
    I2CM_2A1D_W(0x34, 0x3008, 0x54),  // BCWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34, 0x300A, 0x3B),  // CPWAIT_TIME[9:0]
    I2CM_2A1D_W(0x34, 0x3024, 0xFE),  // VMAX LSB
    I2CM_2A1D_W(0x34, 0x3025, 0x08),  // VMAX MSB    8FE:2302 9FE:2558 AFEh: 2814
    I2CM_2A1D_W(0x34, 0x3026, 0x00),  // VMAX MSB [0:3]
    I2CM_2A1D_W(0x34, 0x3028, 0x17),  // HMAX LSB
    I2CM_2A1D_W(0x34, 0x3029, 0x05),  // HMAX MSB    517:1303 494:1172 42Ah: 1066
    I2CM_2A1D_W(0x34, 0x3031, 0x00),  // ADBIT[1:0]
    I2CM_2A1D_W(0x34, 0x3032, 0x00),  // MDBIT[1:0]
    I2CM_2A1D_W(0x34, 0x3033, 0x09),  // SYS_MODE[3:0]
    I2CM_2A1D_W(0x34, 0x3050, 0x08),  // SHR0[19:0]
    I2CM_2A1D_W(0x34, 0x30C1, 0x00),  // XVS_DRV[1:0]
    I2CM_2A1D_W(0x34, 0x3116, 0x23),  // INCKSEL2[7:0]
    I2CM_2A1D_W(0x34, 0x3118, 0xB4),  // INCKSEL3[10:0]
    I2CM_2A1D_W(0x34, 0x311A, 0xFC),  // INCKSEL4[10:0]
    I2CM_2A1D_W(0x34, 0x311E, 0x23),  // INCKSEL5[7:0]
    I2CM_2A1D_W(0x34, 0x32D4, 0x21),  // -
    I2CM_2A1D_W(0x34, 0x32EC, 0xA1),  // -
    I2CM_2A1D_W(0x34, 0x344C, 0x2B),  // -
    I2CM_2A1D_W(0x34, 0x344D, 0x01),  // -
    I2CM_2A1D_W(0x34, 0x344E, 0xED),  // -
    I2CM_2A1D_W(0x34, 0x344F, 0x01),  // -
    I2CM_2A1D_W(0x34, 0x3450, 0xF6),  // -
    I2CM_2A1D_W(0x34, 0x3451, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3452, 0x7F),  // -
    I2CM_2A1D_W(0x34, 0x3453, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x358A, 0x04),  // -
    I2CM_2A1D_W(0x34, 0x35A1, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x35EC, 0x27),  // -
    I2CM_2A1D_W(0x34, 0x35EE, 0x8D),  // -
    I2CM_2A1D_W(0x34, 0x35F0, 0x8D),  // -
    I2CM_2A1D_W(0x34, 0x35F2, 0x29),  // -
    I2CM_2A1D_W(0x34, 0x36BC, 0x0C),  // -
    I2CM_2A1D_W(0x34, 0x36CC, 0x53),  // -
    I2CM_2A1D_W(0x34, 0x36CD, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x36CE, 0x3C),  // -
    I2CM_2A1D_W(0x34, 0x36D0, 0x8C),  // -
    I2CM_2A1D_W(0x34, 0x36D1, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x36D2, 0x71),  // -
    I2CM_2A1D_W(0x34, 0x36D4, 0x3C),  // -
    I2CM_2A1D_W(0x34, 0x36D6, 0x53),  // -
    I2CM_2A1D_W(0x34, 0x36D7, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x36D8, 0x71),  // -
    I2CM_2A1D_W(0x34, 0x36DA, 0x8C),  // -
    I2CM_2A1D_W(0x34, 0x36DB, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x3701, 0x00),  // ADBIT1[7:0]
    I2CM_2A1D_W(0x34, 0x3720, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x3724, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3726, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3732, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3734, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3736, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3742, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3862, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x38CC, 0x30),  // -
    I2CM_2A1D_W(0x34, 0x38CD, 0x2F),  // -
    I2CM_2A1D_W(0x34, 0x395C, 0x0C),  // -
    I2CM_2A1D_W(0x34, 0x39A4, 0x07),  // -
    I2CM_2A1D_W(0x34, 0x39A8, 0x32),  // -
    I2CM_2A1D_W(0x34, 0x39AA, 0x32),  // -
    I2CM_2A1D_W(0x34, 0x39AC, 0x32),  // -
    I2CM_2A1D_W(0x34, 0x39AE, 0x32),  // -
    I2CM_2A1D_W(0x34, 0x39B0, 0x32),  // -
    I2CM_2A1D_W(0x34, 0x39B2, 0x2F),  // -
    I2CM_2A1D_W(0x34, 0x39B4, 0x2D),  // -
    I2CM_2A1D_W(0x34, 0x39B6, 0x28),  // -
    I2CM_2A1D_W(0x34, 0x39B8, 0x30),  // -
    I2CM_2A1D_W(0x34, 0x39BA, 0x30),  // -
    I2CM_2A1D_W(0x34, 0x39BC, 0x30),  // -
    I2CM_2A1D_W(0x34, 0x39BE, 0x30),  // -
    I2CM_2A1D_W(0x34, 0x39C0, 0x30),  // -
    I2CM_2A1D_W(0x34, 0x39C2, 0x2E),  // -
    I2CM_2A1D_W(0x34, 0x39C4, 0x2B),  // -
    I2CM_2A1D_W(0x34, 0x39C6, 0x25),  // -
    I2CM_2A1D_W(0x34, 0x3A42, 0xD1),  // -
    I2CM_2A1D_W(0x34, 0x3A4C, 0x77),  // -
    I2CM_2A1D_W(0x34, 0x3AE0, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3AEC, 0x0C),  // -
    I2CM_2A1D_W(0x34, 0x3B00, 0x2E),  // -
    I2CM_2A1D_W(0x34, 0x3B06, 0x29),  // -
    I2CM_2A1D_W(0x34, 0x3B98, 0x25),  // -
    I2CM_2A1D_W(0x34, 0x3B99, 0x21),  // -
    I2CM_2A1D_W(0x34, 0x3B9B, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3B9C, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3B9D, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3B9E, 0x13),  // -
    I2CM_2A1D_W(0x34, 0x3BA1, 0x00),  // -
    I2CM_2A1D_W(0x34, 0x3BA2, 0x06),  // -
    I2CM_2A1D_W(0x34, 0x3BA3, 0x0B),  // -
    I2CM_2A1D_W(0x34, 0x3BA4, 0x10),  // -
    I2CM_2A1D_W(0x34, 0x3BA5, 0x14),  // -
    I2CM_2A1D_W(0x34, 0x3BA6, 0x18),  // -
    I2CM_2A1D_W(0x34, 0x3BA7, 0x1A),  // -
    I2CM_2A1D_W(0x34, 0x3BA8, 0x1A),  // -
    I2CM_2A1D_W(0x34, 0x3BA9, 0x1A),  // -
    I2CM_2A1D_W(0x34, 0x3BAC, 0xED),  // -
    I2CM_2A1D_W(0x34, 0x3BAD, 0x01),  // -
    I2CM_2A1D_W(0x34, 0x3BAE, 0xF6),  // -
    I2CM_2A1D_W(0x34, 0x3BAF, 0x02),  // -
    I2CM_2A1D_W(0x34, 0x3BB0, 0xA2),  // -
    I2CM_2A1D_W(0x34, 0x3BB1, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB2, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BB3, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB4, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BB5, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB6, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BB7, 0x03),  // -
    I2CM_2A1D_W(0x34, 0x3BB8, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BBA, 0xE0),  // -
    I2CM_2A1D_W(0x34, 0x3BBC, 0xDA),  // -
    I2CM_2A1D_W(0x34, 0x3BBE, 0x88),  // -
    I2CM_2A1D_W(0x34, 0x3BC0, 0x44),  // -
    I2CM_2A1D_W(0x34, 0x3BC2, 0x7B),  // -
    I2CM_2A1D_W(0x34, 0x3BC4, 0xA2),  // -
    I2CM_2A1D_W(0x34, 0x3BC8, 0xBD),  // -
    I2CM_2A1D_W(0x34, 0x3BCA, 0xBD),  // -
    I2CM_2A1D_W(0x34, 0x4000, 0x11),    //mipi clk non-continouns mode
    I2CM_2A1D_W(0x34, 0x4004, 0x00),  // TXCLKESC_FREQ[15:0]
    I2CM_2A1D_W(0x34, 0x4005, 0x06),  //
    I2CM_2A1D_W(0x34, 0x400C, 0x00),  // INCKSEL6
    I2CM_2A1D_W(0x34, 0x4018, 0x6F),  // TCLKPOST[15:0]
    I2CM_2A1D_W(0x34, 0x401A, 0x2F),  // TCLKPREPARE[15:0]
    I2CM_2A1D_W(0x34, 0x401C, 0x2F),  // TCLKTRAIL[15:0]
    I2CM_2A1D_W(0x34, 0x401E, 0xBF),  // TCLKZERO[15:0]
    I2CM_2A1D_W(0x34, 0x401F, 0x00),  //
    I2CM_2A1D_W(0x34, 0x4020, 0x2F),  // THSPREPARE[15:0]
    I2CM_2A1D_W(0x34, 0x4022, 0x57),  // THSZERO[15:0]
    I2CM_2A1D_W(0x34, 0x4024, 0x2F),  // THSTRAIL[15:0]
    I2CM_2A1D_W(0x34, 0x4026, 0x4F),  // THSEXIT[15:0]
    I2CM_2A1D_W(0x34, 0x4028, 0x27),  // TLPX[15:0]
    I2CM_2A1D_W(0x34, 0x4074, 0x01),  // INCKSEL7 [2:0]


    CMDQ_DELAY_MS(10),                // delay 10ms
    I2CM_2A1D_W(0x34,0x3260,0x00),    //Gain_Pgc_Fidmd
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
    SNR_HDR_SHUTTER_FPS_2A1D(0x34, 10000, 1000, 0, 30000),   //shutter 10ms , fps 30
    SNR_HDR_GAIN_2A1D(0x34, 1024, 1024, 0),
    SNR_ORIENTATION_2A1D(0x34, 0, 0),
    I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end

    SNR_STREAM_ONOFF_2A1D(0x34, 1),
#else
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3002, 0x00),
    //CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34,0x3000, 0x00),//
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
#endif //end of _IMX415_RES_2560x1440_30_


#endif
