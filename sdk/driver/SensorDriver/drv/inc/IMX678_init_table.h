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

#ifndef _IMX678_INIT_TABLE_H_

#define _IMX678_PDWN_ON_ 1
#define _IMX678_RST_ON_ 1

//static CMDQ_CMDS gTable[] __attribute__((aligned(8))) =
static SENSOR_INIT_TABLE Sensor_init_table[] __attribute__((aligned(8)))=
{
#if defined(SENSOR_INIT_CMDQ_MODE)
    /*set cmdq0 busy*/
    CMDQ_REG_DUMMY(CMDQ_STATUS_BUSY),
    VIF_REG_DUMMY(CMDQ_STATUS_BUSY),

    /*sensor reset*/
    SNR_PDWN(~_IMX678_PDWN_ON_),     //power off
    CMDQ_DELAY_10US(10),             //T1, delay 100us
    SNR_RST(~_IMX678_RST_ON_),       //reset off
    CMDQ_DELAY_MS(1),                //T3, delay 1ms

    //SNR_RST(_IMX678_RST_ON_),       //reset off
    //SNR_MCLK_EN(0xB),               //0x7=MCLK 27MhZ, 0xB=37.125MHz
    SNR_MCLK_EN(0x7),               //0x7=MCLK 24MhZ
    CMDQ_DELAY_10US(2),             //T2, delay 20us

    SNR_PDWN(_IMX678_PDWN_ON_),    //power on
    CMDQ_DELAY_10US(2),             //T1, delay 20us
    SNR_RST(_IMX678_RST_ON_),      //reset on
    CMDQ_DELAY_10US(2),             //T1, delay 20us

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
    //I2CM_PARAM(14,7,10,10,0,11),
    I2CM_PARAM(30,15,20,20,0,20),
#endif

    CMDQ_DELAY_MS(2),
    I2CM_2A1D_W(0x34, 0x3000, 0x01),
    I2CM_2A1D_W(0x34, 0x3001, 0x00),
    I2CM_2A1D_W(0x34, 0x3002, 0x01),
    I2CM_2A1D_W(0x34, 0x3014, 0x04),
    I2CM_2A1D_W(0x34, 0x3015, 0x06),
    I2CM_2A1D_W(0x34, 0x3018, 0x00),
    I2CM_2A1D_W(0x34, 0x3019, 0x00),
    I2CM_2A1D_W(0x34, 0x301A, 0x00),
    I2CM_2A1D_W(0x34, 0x301B, 0x00),
    I2CM_2A1D_W(0x34, 0x301C, 0x00),
    I2CM_2A1D_W(0x34, 0x301E, 0x01),
    I2CM_2A1D_W(0x34, 0x3020, 0x00),
    I2CM_2A1D_W(0x34, 0x3021, 0x00),
    I2CM_2A1D_W(0x34, 0x3022, 0x00),
    I2CM_2A1D_W(0x34, 0x3023, 0x00),
    I2CM_2A1D_W(0x34, 0x3028, 0xA6),
    I2CM_2A1D_W(0x34, 0x3029, 0x0E),
    I2CM_2A1D_W(0x34, 0x302A, 0x00),
    I2CM_2A1D_W(0x34, 0x302C, 0x28),
    I2CM_2A1D_W(0x34, 0x302D, 0x05),
    I2CM_2A1D_W(0x34, 0x3030, 0x00),
    I2CM_2A1D_W(0x34, 0x3031, 0x00),
    I2CM_2A1D_W(0x34, 0x3032, 0x00),
    I2CM_2A1D_W(0x34, 0x303C, 0x00),
    I2CM_2A1D_W(0x34, 0x303D, 0x00),
    I2CM_2A1D_W(0x34, 0x303E, 0x10),
    I2CM_2A1D_W(0x34, 0x303F, 0x0F),
    I2CM_2A1D_W(0x34, 0x3040, 0x03),
    I2CM_2A1D_W(0x34, 0x3042, 0x00),
    I2CM_2A1D_W(0x34, 0x3043, 0x00),
    I2CM_2A1D_W(0x34, 0x3044, 0x00),
    I2CM_2A1D_W(0x34, 0x3045, 0x00),
    I2CM_2A1D_W(0x34, 0x3046, 0x84),
    I2CM_2A1D_W(0x34, 0x3047, 0x08),
    I2CM_2A1D_W(0x34, 0x3050, 0x03),
    I2CM_2A1D_W(0x34, 0x3051, 0x00),
    I2CM_2A1D_W(0x34, 0x3052, 0x00),
    I2CM_2A1D_W(0x34, 0x3054, 0x0E),
    I2CM_2A1D_W(0x34, 0x3055, 0x00),
    I2CM_2A1D_W(0x34, 0x3056, 0x00),
    I2CM_2A1D_W(0x34, 0x3058, 0x8A),
    I2CM_2A1D_W(0x34, 0x3059, 0x01),
    I2CM_2A1D_W(0x34, 0x305A, 0x00),
    I2CM_2A1D_W(0x34, 0x3060, 0x16),
    I2CM_2A1D_W(0x34, 0x3061, 0x01),
    I2CM_2A1D_W(0x34, 0x3062, 0x00),
    I2CM_2A1D_W(0x34, 0x3064, 0xC4),
    I2CM_2A1D_W(0x34, 0x3065, 0x0C),
    I2CM_2A1D_W(0x34, 0x3066, 0x00),
    I2CM_2A1D_W(0x34, 0x3069, 0x00),
    I2CM_2A1D_W(0x34, 0x306B, 0x00),
    I2CM_2A1D_W(0x34, 0x3070, 0x00),
    I2CM_2A1D_W(0x34, 0x3071, 0x00),
    I2CM_2A1D_W(0x34, 0x3072, 0x00),
    I2CM_2A1D_W(0x34, 0x3073, 0x00),
    I2CM_2A1D_W(0x34, 0x3074, 0x00),
    I2CM_2A1D_W(0x34, 0x3075, 0x00),
    I2CM_2A1D_W(0x34, 0x3081, 0x00),
    I2CM_2A1D_W(0x34, 0x308C, 0x00),
    I2CM_2A1D_W(0x34, 0x308D, 0x01),
    I2CM_2A1D_W(0x34, 0x3094, 0x00),
    I2CM_2A1D_W(0x34, 0x3095, 0x00),
    I2CM_2A1D_W(0x34, 0x309C, 0x00),
    I2CM_2A1D_W(0x34, 0x309D, 0x00),
    I2CM_2A1D_W(0x34, 0x30A4, 0xAA),
    I2CM_2A1D_W(0x34, 0x30A6, 0x00),
    I2CM_2A1D_W(0x34, 0x30CC, 0x00),
    I2CM_2A1D_W(0x34, 0x30CD, 0x00),
    I2CM_2A1D_W(0x34, 0x30DC, 0x32),
    I2CM_2A1D_W(0x34, 0x30DD, 0x40),
    I2CM_2A1D_W(0x34, 0x3400, 0x01),
    I2CM_2A1D_W(0x34, 0x3460, 0x22),
    I2CM_2A1D_W(0x34, 0x355A, 0x64),
    I2CM_2A1D_W(0x34, 0x3A02, 0x7A),
    I2CM_2A1D_W(0x34, 0x3A10, 0xEC),
    I2CM_2A1D_W(0x34, 0x3A12, 0x71),
    I2CM_2A1D_W(0x34, 0x3A14, 0xDE),
    I2CM_2A1D_W(0x34, 0x3A20, 0x2B),
    I2CM_2A1D_W(0x34, 0x3A24, 0x22),
    I2CM_2A1D_W(0x34, 0x3A25, 0x25),
    I2CM_2A1D_W(0x34, 0x3A26, 0x2A),
    I2CM_2A1D_W(0x34, 0x3A27, 0x2C),
    I2CM_2A1D_W(0x34, 0x3A28, 0x39),
    I2CM_2A1D_W(0x34, 0x3A29, 0x38),
    I2CM_2A1D_W(0x34, 0x3A30, 0x04),
    I2CM_2A1D_W(0x34, 0x3A31, 0x04),
    I2CM_2A1D_W(0x34, 0x3A32, 0x03),
    I2CM_2A1D_W(0x34, 0x3A33, 0x03),
    I2CM_2A1D_W(0x34, 0x3A34, 0x09),
    I2CM_2A1D_W(0x34, 0x3A35, 0x06),
    I2CM_2A1D_W(0x34, 0x3A38, 0xCD),
    I2CM_2A1D_W(0x34, 0x3A3A, 0x4C),
    I2CM_2A1D_W(0x34, 0x3A3C, 0xB9),
    I2CM_2A1D_W(0x34, 0x3A3E, 0x30),
    I2CM_2A1D_W(0x34, 0x3A40, 0x2C),
    I2CM_2A1D_W(0x34, 0x3A42, 0x39),
    I2CM_2A1D_W(0x34, 0x3A4E, 0x00),
    I2CM_2A1D_W(0x34, 0x3A52, 0x00),
    I2CM_2A1D_W(0x34, 0x3A56, 0x00),
    I2CM_2A1D_W(0x34, 0x3A5A, 0x00),
    I2CM_2A1D_W(0x34, 0x3A5E, 0x00),
    I2CM_2A1D_W(0x34, 0x3A62, 0x00),
    I2CM_2A1D_W(0x34, 0x3A64, 0x00),
    I2CM_2A1D_W(0x34, 0x3A6E, 0xA0),
    I2CM_2A1D_W(0x34, 0x3A70, 0x50),
    I2CM_2A1D_W(0x34, 0x3A8C, 0x04),
    I2CM_2A1D_W(0x34, 0x3A8D, 0x03),
    I2CM_2A1D_W(0x34, 0x3A8E, 0x09),
    I2CM_2A1D_W(0x34, 0x3A90, 0x38),
    I2CM_2A1D_W(0x34, 0x3A91, 0x42),
    I2CM_2A1D_W(0x34, 0x3A92, 0x3C),
    I2CM_2A1D_W(0x34, 0x3B0E, 0xF3),
    I2CM_2A1D_W(0x34, 0x3B12, 0xE5),
    I2CM_2A1D_W(0x34, 0x3B27, 0xC0),
    I2CM_2A1D_W(0x34, 0x3B2E, 0xEF),
    I2CM_2A1D_W(0x34, 0x3B30, 0x6A),
    I2CM_2A1D_W(0x34, 0x3B32, 0xF6),
    I2CM_2A1D_W(0x34, 0x3B36, 0xE1),
    I2CM_2A1D_W(0x34, 0x3B3A, 0xE8),
    I2CM_2A1D_W(0x34, 0x3B5A, 0x17),
    I2CM_2A1D_W(0x34, 0x3B5E, 0xEF),
    I2CM_2A1D_W(0x34, 0x3B60, 0x6A),
    I2CM_2A1D_W(0x34, 0x3B62, 0xF6),
    I2CM_2A1D_W(0x34, 0x3B66, 0xE1),
    I2CM_2A1D_W(0x34, 0x3B6A, 0xE8),
    I2CM_2A1D_W(0x34, 0x3B88, 0xEC),
    I2CM_2A1D_W(0x34, 0x3B8A, 0xED),
    I2CM_2A1D_W(0x34, 0x3B94, 0x71),
    I2CM_2A1D_W(0x34, 0x3B96, 0x72),
    I2CM_2A1D_W(0x34, 0x3B98, 0xDE),
    I2CM_2A1D_W(0x34, 0x3B9A, 0xDF),
    I2CM_2A1D_W(0x34, 0x3C0F, 0x06),
    I2CM_2A1D_W(0x34, 0x3C10, 0x06),
    I2CM_2A1D_W(0x34, 0x3C11, 0x06),
    I2CM_2A1D_W(0x34, 0x3C12, 0x06),
    I2CM_2A1D_W(0x34, 0x3C13, 0x06),
    I2CM_2A1D_W(0x34, 0x3C18, 0x20),
    I2CM_2A1D_W(0x34, 0x3C37, 0x10),
    I2CM_2A1D_W(0x34, 0x3C3A, 0x7A),
    I2CM_2A1D_W(0x34, 0x3C40, 0xF4),
    I2CM_2A1D_W(0x34, 0x3C48, 0xE6),
    I2CM_2A1D_W(0x34, 0x3C54, 0xCE),
    I2CM_2A1D_W(0x34, 0x3C56, 0xD0),
    I2CM_2A1D_W(0x34, 0x3C6C, 0x53),
    I2CM_2A1D_W(0x34, 0x3C6E, 0x55),
    I2CM_2A1D_W(0x34, 0x3C70, 0xC0),
    I2CM_2A1D_W(0x34, 0x3C72, 0xC2),
    I2CM_2A1D_W(0x34, 0x3C7E, 0xCE),
    I2CM_2A1D_W(0x34, 0x3C8C, 0xCF),
    I2CM_2A1D_W(0x34, 0x3C8E, 0xEB),
    I2CM_2A1D_W(0x34, 0x3C98, 0x54),
    I2CM_2A1D_W(0x34, 0x3C9A, 0x70),
    I2CM_2A1D_W(0x34, 0x3C9C, 0xC1),
    I2CM_2A1D_W(0x34, 0x3C9E, 0xDD),
    I2CM_2A1D_W(0x34, 0x3CB0, 0x7A),
    I2CM_2A1D_W(0x34, 0x3CB2, 0xBA),
    I2CM_2A1D_W(0x34, 0x3CC8, 0xBC),
    I2CM_2A1D_W(0x34, 0x3CCA, 0x7C),
    I2CM_2A1D_W(0x34, 0x3CD4, 0xEA),
    I2CM_2A1D_W(0x34, 0x3CD5, 0x01),
    I2CM_2A1D_W(0x34, 0x3CD6, 0x4A),
    I2CM_2A1D_W(0x34, 0x3CD8, 0x00),
    I2CM_2A1D_W(0x34, 0x3CD9, 0x00),
    I2CM_2A1D_W(0x34, 0x3CDA, 0xFF),
    I2CM_2A1D_W(0x34, 0x3CDB, 0x03),
    I2CM_2A1D_W(0x34, 0x3CDC, 0x00),
    I2CM_2A1D_W(0x34, 0x3CDD, 0x00),
    I2CM_2A1D_W(0x34, 0x3CDE, 0xFF),
    I2CM_2A1D_W(0x34, 0x3CDF, 0x03),
    I2CM_2A1D_W(0x34, 0x3CE4, 0x4C),
    I2CM_2A1D_W(0x34, 0x3CE6, 0xEC),
    I2CM_2A1D_W(0x34, 0x3CE7, 0x01),
    I2CM_2A1D_W(0x34, 0x3CE8, 0xFF),
    I2CM_2A1D_W(0x34, 0x3CE9, 0x03),
    I2CM_2A1D_W(0x34, 0x3CEA, 0x00),
    I2CM_2A1D_W(0x34, 0x3CEB, 0x00),
    I2CM_2A1D_W(0x34, 0x3CEC, 0xFF),
    I2CM_2A1D_W(0x34, 0x3CED, 0x03),
    I2CM_2A1D_W(0x34, 0x3CEE, 0x00),
    I2CM_2A1D_W(0x34, 0x3CEF, 0x00),
    I2CM_2A1D_W(0x34, 0x3CF2, 0xFF),
    I2CM_2A1D_W(0x34, 0x3CF3, 0x03),
    I2CM_2A1D_W(0x34, 0x3CF4, 0x00),
    I2CM_2A1D_W(0x34, 0x3E28, 0x82),
    I2CM_2A1D_W(0x34, 0x3E2A, 0x80),
    I2CM_2A1D_W(0x34, 0x3E30, 0x85),
    I2CM_2A1D_W(0x34, 0x3E32, 0x7D),
    I2CM_2A1D_W(0x34, 0x3E5C, 0xCE),
    I2CM_2A1D_W(0x34, 0x3E5E, 0xD3),
    I2CM_2A1D_W(0x34, 0x3E70, 0x53),
    I2CM_2A1D_W(0x34, 0x3E72, 0x58),
    I2CM_2A1D_W(0x34, 0x3E74, 0xC0),
    I2CM_2A1D_W(0x34, 0x3E76, 0xC5),
    I2CM_2A1D_W(0x34, 0x3E78, 0xC0),
    I2CM_2A1D_W(0x34, 0x3E79, 0x01),
    I2CM_2A1D_W(0x34, 0x3E7A, 0xD4),
    I2CM_2A1D_W(0x34, 0x3E7B, 0x01),
    I2CM_2A1D_W(0x34, 0x3EB4, 0x0B),
    I2CM_2A1D_W(0x34, 0x3EB5, 0x02),
    I2CM_2A1D_W(0x34, 0x3EB6, 0x4D),
    I2CM_2A1D_W(0x34, 0x3EB7, 0x42),
    I2CM_2A1D_W(0x34, 0x3EEC, 0xF3),
    I2CM_2A1D_W(0x34, 0x3EEE, 0xE7),
    I2CM_2A1D_W(0x34, 0x3F01, 0x01),
    I2CM_2A1D_W(0x34, 0x3F24, 0x10),
    I2CM_2A1D_W(0x34, 0x3F28, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F2A, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F2C, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F2E, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F30, 0x23),
    I2CM_2A1D_W(0x34, 0x3F38, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F3A, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F3C, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F3E, 0x28),
    I2CM_2A1D_W(0x34, 0x3F40, 0x1E),
    I2CM_2A1D_W(0x34, 0x3F48, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F4A, 0x2D),
    I2CM_2A1D_W(0x34, 0x3F4C, 0x00),
    I2CM_2A1D_W(0x34, 0x4004, 0xE4),
    I2CM_2A1D_W(0x34, 0x4006, 0xFF),
    I2CM_2A1D_W(0x34, 0x4018, 0x69),
    I2CM_2A1D_W(0x34, 0x401A, 0x84),
    I2CM_2A1D_W(0x34, 0x401C, 0xD6),
    I2CM_2A1D_W(0x34, 0x401E, 0xF1),
    I2CM_2A1D_W(0x34, 0x4038, 0xDE),
    I2CM_2A1D_W(0x34, 0x403A, 0x00),
    I2CM_2A1D_W(0x34, 0x403B, 0x01),
    I2CM_2A1D_W(0x34, 0x404C, 0x63),
    I2CM_2A1D_W(0x34, 0x404E, 0x85),
    I2CM_2A1D_W(0x34, 0x4050, 0xD0),
    I2CM_2A1D_W(0x34, 0x4052, 0xF2),
    I2CM_2A1D_W(0x34, 0x4108, 0xDD),
    I2CM_2A1D_W(0x34, 0x410A, 0xF7),
    I2CM_2A1D_W(0x34, 0x411C, 0x62),
    I2CM_2A1D_W(0x34, 0x411E, 0x7C),
    I2CM_2A1D_W(0x34, 0x4120, 0xCF),
    I2CM_2A1D_W(0x34, 0x4122, 0xE9),
    I2CM_2A1D_W(0x34, 0x4138, 0xE6),
    I2CM_2A1D_W(0x34, 0x413A, 0xF1),
    I2CM_2A1D_W(0x34, 0x414C, 0x6B),
    I2CM_2A1D_W(0x34, 0x414E, 0x76),
    I2CM_2A1D_W(0x34, 0x4150, 0xD8),
    I2CM_2A1D_W(0x34, 0x4152, 0xE3),
    I2CM_2A1D_W(0x34, 0x417E, 0x03),
    I2CM_2A1D_W(0x34, 0x417F, 0x01),
    I2CM_2A1D_W(0x34, 0x4186, 0xE0),
    I2CM_2A1D_W(0x34, 0x4190, 0xF3),
    I2CM_2A1D_W(0x34, 0x4192, 0xF7),
    I2CM_2A1D_W(0x34, 0x419C, 0x78),
    I2CM_2A1D_W(0x34, 0x419E, 0x7C),
    I2CM_2A1D_W(0x34, 0x41A0, 0xE5),
    I2CM_2A1D_W(0x34, 0x41A2, 0xE9),
    I2CM_2A1D_W(0x34, 0x41C8, 0xE2),
    I2CM_2A1D_W(0x34, 0x41CA, 0xFD),
    I2CM_2A1D_W(0x34, 0x41DC, 0x67),
    I2CM_2A1D_W(0x34, 0x41DE, 0x82),
    I2CM_2A1D_W(0x34, 0x41E0, 0xD4),
    I2CM_2A1D_W(0x34, 0x41E2, 0xEF),
    I2CM_2A1D_W(0x34, 0x4200, 0xDE),
    I2CM_2A1D_W(0x34, 0x4202, 0xDA),
    I2CM_2A1D_W(0x34, 0x4218, 0x63),
    I2CM_2A1D_W(0x34, 0x421A, 0x5F),
    I2CM_2A1D_W(0x34, 0x421C, 0xD0),
    I2CM_2A1D_W(0x34, 0x421E, 0xCC),
    I2CM_2A1D_W(0x34, 0x425A, 0x82),
    I2CM_2A1D_W(0x34, 0x425C, 0xEF),
    I2CM_2A1D_W(0x34, 0x4348, 0xFE),
    I2CM_2A1D_W(0x34, 0x4349, 0x06),
    I2CM_2A1D_W(0x34, 0x4352, 0xCE),
    I2CM_2A1D_W(0x34, 0x4420, 0x0B),
    I2CM_2A1D_W(0x34, 0x4421, 0x02),
    I2CM_2A1D_W(0x34, 0x4422, 0x4D),
    I2CM_2A1D_W(0x34, 0x4423, 0x0A),
    I2CM_2A1D_W(0x34, 0x4426, 0xF5),
    I2CM_2A1D_W(0x34, 0x442A, 0xE7),
    I2CM_2A1D_W(0x34, 0x4432, 0xF5),
    I2CM_2A1D_W(0x34, 0x4436, 0xE7),
    I2CM_2A1D_W(0x34, 0x4466, 0xB4),
    I2CM_2A1D_W(0x34, 0x446E, 0x32),
    I2CM_2A1D_W(0x34, 0x449F, 0x1C),
    I2CM_2A1D_W(0x34, 0x44A4, 0x2C),
    I2CM_2A1D_W(0x34, 0x44A6, 0x2C),
    I2CM_2A1D_W(0x34, 0x44A8, 0x2C),
    I2CM_2A1D_W(0x34, 0x44AA, 0x2C),
    I2CM_2A1D_W(0x34, 0x44B4, 0x2C),
    I2CM_2A1D_W(0x34, 0x44B6, 0x2C),
    I2CM_2A1D_W(0x34, 0x44B8, 0x2C),
    I2CM_2A1D_W(0x34, 0x44BA, 0x2C),
    I2CM_2A1D_W(0x34, 0x44C4, 0x2C),
    I2CM_2A1D_W(0x34, 0x44C6, 0x2C),
    I2CM_2A1D_W(0x34, 0x44C8, 0x2C),
    I2CM_2A1D_W(0x34, 0x4506, 0xF3),
    I2CM_2A1D_W(0x34, 0x450E, 0xE5),
    I2CM_2A1D_W(0x34, 0x4516, 0xF3),
    I2CM_2A1D_W(0x34, 0x4522, 0xE5),
    I2CM_2A1D_W(0x34, 0x4524, 0xF3),
    I2CM_2A1D_W(0x34, 0x452C, 0xE5),
    I2CM_2A1D_W(0x34, 0x453C, 0x22),
    I2CM_2A1D_W(0x34, 0x453D, 0x1B),
    I2CM_2A1D_W(0x34, 0x453E, 0x1B),
    I2CM_2A1D_W(0x34, 0x453F, 0x15),
    I2CM_2A1D_W(0x34, 0x4540, 0x15),
    I2CM_2A1D_W(0x34, 0x4541, 0x15),
    I2CM_2A1D_W(0x34, 0x4542, 0x15),
    I2CM_2A1D_W(0x34, 0x4543, 0x15),
    I2CM_2A1D_W(0x34, 0x4544, 0x15),
    I2CM_2A1D_W(0x34, 0x4548, 0x00),
    I2CM_2A1D_W(0x34, 0x4549, 0x01),
    I2CM_2A1D_W(0x34, 0x454A, 0x01),
    I2CM_2A1D_W(0x34, 0x454B, 0x06),
    I2CM_2A1D_W(0x34, 0x454C, 0x06),
    I2CM_2A1D_W(0x34, 0x454D, 0x06),
    I2CM_2A1D_W(0x34, 0x454E, 0x06),
    I2CM_2A1D_W(0x34, 0x454F, 0x06),
    I2CM_2A1D_W(0x34, 0x4550, 0x06),
    I2CM_2A1D_W(0x34, 0x4554, 0x55),
    I2CM_2A1D_W(0x34, 0x4555, 0x02),
    I2CM_2A1D_W(0x34, 0x4556, 0x42),
    I2CM_2A1D_W(0x34, 0x4557, 0x05),
    I2CM_2A1D_W(0x34, 0x4558, 0xFD),
    I2CM_2A1D_W(0x34, 0x4559, 0x05),
    I2CM_2A1D_W(0x34, 0x455A, 0x94),
    I2CM_2A1D_W(0x34, 0x455B, 0x06),
    I2CM_2A1D_W(0x34, 0x455D, 0x06),
    I2CM_2A1D_W(0x34, 0x455E, 0x49),
    I2CM_2A1D_W(0x34, 0x455F, 0x07),
    I2CM_2A1D_W(0x34, 0x4560, 0x7F),
    I2CM_2A1D_W(0x34, 0x4561, 0x07),
    I2CM_2A1D_W(0x34, 0x4562, 0xA5),
    I2CM_2A1D_W(0x34, 0x4564, 0x55),
    I2CM_2A1D_W(0x34, 0x4565, 0x02),
    I2CM_2A1D_W(0x34, 0x4566, 0x42),
    I2CM_2A1D_W(0x34, 0x4567, 0x05),
    I2CM_2A1D_W(0x34, 0x4568, 0xFD),
    I2CM_2A1D_W(0x34, 0x4569, 0x05),
    I2CM_2A1D_W(0x34, 0x456A, 0x94),
    I2CM_2A1D_W(0x34, 0x456B, 0x06),
    I2CM_2A1D_W(0x34, 0x456D, 0x06),
    I2CM_2A1D_W(0x34, 0x456E, 0x49),
    I2CM_2A1D_W(0x34, 0x456F, 0x07),
    I2CM_2A1D_W(0x34, 0x4572, 0xA5),
    I2CM_2A1D_W(0x34, 0x460C, 0x7D),
    I2CM_2A1D_W(0x34, 0x460E, 0xB1),
    I2CM_2A1D_W(0x34, 0x4614, 0xA8),
    I2CM_2A1D_W(0x34, 0x4616, 0xB2),
    I2CM_2A1D_W(0x34, 0x461C, 0x7E),
    I2CM_2A1D_W(0x34, 0x461E, 0xA7),
    I2CM_2A1D_W(0x34, 0x4624, 0xA8),
    I2CM_2A1D_W(0x34, 0x4626, 0xB2),
    I2CM_2A1D_W(0x34, 0x462C, 0x7E),
    I2CM_2A1D_W(0x34, 0x462E, 0x8A),
    I2CM_2A1D_W(0x34, 0x4630, 0x94),
    I2CM_2A1D_W(0x34, 0x4632, 0xA7),
    I2CM_2A1D_W(0x34, 0x4634, 0xFB),
    I2CM_2A1D_W(0x34, 0x4636, 0x2F),
    I2CM_2A1D_W(0x34, 0x4638, 0x81),
    I2CM_2A1D_W(0x34, 0x4639, 0x01),
    I2CM_2A1D_W(0x34, 0x463A, 0xB5),
    I2CM_2A1D_W(0x34, 0x463B, 0x01),
    I2CM_2A1D_W(0x34, 0x463C, 0x26),
    I2CM_2A1D_W(0x34, 0x463E, 0x30),
    I2CM_2A1D_W(0x34, 0x4640, 0xAC),
    I2CM_2A1D_W(0x34, 0x4641, 0x01),
    I2CM_2A1D_W(0x34, 0x4642, 0xB6),
    I2CM_2A1D_W(0x34, 0x4643, 0x01),
    I2CM_2A1D_W(0x34, 0x4644, 0xFC),
    I2CM_2A1D_W(0x34, 0x4646, 0x25),
    I2CM_2A1D_W(0x34, 0x4648, 0x82),
    I2CM_2A1D_W(0x34, 0x4649, 0x01),
    I2CM_2A1D_W(0x34, 0x464A, 0xAB),
    I2CM_2A1D_W(0x34, 0x464B, 0x01),
    I2CM_2A1D_W(0x34, 0x464C, 0x26),
    I2CM_2A1D_W(0x34, 0x464E, 0x30),
    I2CM_2A1D_W(0x34, 0x4654, 0xFC),
    I2CM_2A1D_W(0x34, 0x4656, 0x08),
    I2CM_2A1D_W(0x34, 0x4658, 0x12),
    I2CM_2A1D_W(0x34, 0x465A, 0x25),
    I2CM_2A1D_W(0x34, 0x4662, 0xFC),
    I2CM_2A1D_W(0x34, 0x46A2, 0xFB),
    I2CM_2A1D_W(0x34, 0x46D6, 0xF3),
    I2CM_2A1D_W(0x34, 0x46E6, 0x00),
    I2CM_2A1D_W(0x34, 0x46E8, 0xFF),
    I2CM_2A1D_W(0x34, 0x46E9, 0x03),
    I2CM_2A1D_W(0x34, 0x46EC, 0x7A),
    I2CM_2A1D_W(0x34, 0x46EE, 0xE5),
    I2CM_2A1D_W(0x34, 0x46F4, 0xEE),
    I2CM_2A1D_W(0x34, 0x46F6, 0xF2),
    I2CM_2A1D_W(0x34, 0x470C, 0xFF),
    I2CM_2A1D_W(0x34, 0x470D, 0x03),
    I2CM_2A1D_W(0x34, 0x470E, 0x00),
    I2CM_2A1D_W(0x34, 0x4714, 0xE0),
    I2CM_2A1D_W(0x34, 0x4716, 0xE4),
    I2CM_2A1D_W(0x34, 0x471E, 0xED),
    I2CM_2A1D_W(0x34, 0x472E, 0x00),
    I2CM_2A1D_W(0x34, 0x4730, 0xFF),
    I2CM_2A1D_W(0x34, 0x4731, 0x03),
    I2CM_2A1D_W(0x34, 0x4734, 0x7B),
    I2CM_2A1D_W(0x34, 0x4736, 0xDF),
    I2CM_2A1D_W(0x34, 0x4754, 0x7D),
    I2CM_2A1D_W(0x34, 0x4756, 0x8B),
    I2CM_2A1D_W(0x34, 0x4758, 0x93),
    I2CM_2A1D_W(0x34, 0x475A, 0xB1),
    I2CM_2A1D_W(0x34, 0x475C, 0xFB),
    I2CM_2A1D_W(0x34, 0x475E, 0x09),
    I2CM_2A1D_W(0x34, 0x4760, 0x11),
    I2CM_2A1D_W(0x34, 0x4762, 0x2F),
    I2CM_2A1D_W(0x34, 0x4766, 0xCC),
    I2CM_2A1D_W(0x34, 0x4776, 0xCB),
    I2CM_2A1D_W(0x34, 0x477E, 0x4A),
    I2CM_2A1D_W(0x34, 0x478E, 0x49),
    I2CM_2A1D_W(0x34, 0x4794, 0x7C),
    I2CM_2A1D_W(0x34, 0x4796, 0x8F),
    I2CM_2A1D_W(0x34, 0x4798, 0xB3),
    I2CM_2A1D_W(0x34, 0x4799, 0x00),
    I2CM_2A1D_W(0x34, 0x479A, 0xCC),
    I2CM_2A1D_W(0x34, 0x479C, 0xC1),
    I2CM_2A1D_W(0x34, 0x479E, 0xCB),
    I2CM_2A1D_W(0x34, 0x47A4, 0x7D),
    I2CM_2A1D_W(0x34, 0x47A6, 0x8E),
    I2CM_2A1D_W(0x34, 0x47A8, 0xB4),
    I2CM_2A1D_W(0x34, 0x47A9, 0x00),
    I2CM_2A1D_W(0x34, 0x47AA, 0xC0),
    I2CM_2A1D_W(0x34, 0x47AC, 0xFA),
    I2CM_2A1D_W(0x34, 0x47AE, 0x0D),
    I2CM_2A1D_W(0x34, 0x47B0, 0x31),
    I2CM_2A1D_W(0x34, 0x47B1, 0x01),
    I2CM_2A1D_W(0x34, 0x47B2, 0x4A),
    I2CM_2A1D_W(0x34, 0x47B3, 0x01),
    I2CM_2A1D_W(0x34, 0x47B4, 0x3F),
    I2CM_2A1D_W(0x34, 0x47B6, 0x49),
    I2CM_2A1D_W(0x34, 0x47BC, 0xFB),
    I2CM_2A1D_W(0x34, 0x47BE, 0x0C),
    I2CM_2A1D_W(0x34, 0x47C0, 0x32),
    I2CM_2A1D_W(0x34, 0x47C1, 0x01),
    I2CM_2A1D_W(0x34, 0x47C2, 0x3E),
    I2CM_2A1D_W(0x34, 0x47C3, 0x01),
    I2CM_2A1D_W(0x34, 0x4E3C, 0x07),
    I2CM_2A1D_W(0x34, 0x4E00, 0x11),  //uncontinue mode

    CMDQ_DELAY_MS(2),

#if defined(SENSOR_INIT_CMDQ_MODE)
    /*Sensor runtime parameter*/
    //I2CM_2A1D_W(0x34,0x3001, 0x01),//group hold begin
    SNR_SHUTTER_FPS_2A1D(0x34, 10000, 15000),   //shutter 10ms , fps 30
    SNR_GAIN_2A1D(0x34, 1024),
    //SNR_ORIENTATION_2A1D(0x34, 0 , 0),
    //I2CM_2A1D_W(0x34,0x3001, 0x00),//group hold end

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

#endif
