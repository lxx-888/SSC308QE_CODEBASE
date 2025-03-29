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
/* Sensor Porting on Master V4
   Porting owner : paul-pc.wang
   Date : 2023/9/25
   Build on : Master V4  I6C
   Verified on : not yet。
   Remark : NA
*/
#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>

#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SP2329_HDR);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
#define SENSOR_MIPI_LANE_NUM_HDR (2)
#define SENSOR_MIPI_HDR_MODE (2) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3

//#undef SENSOR_DBG
#define SENSOR_DBG 0

///////////////////////////////////////////////////////////////
//          @@@                                                                                       //
//       @   @@      ==  S t a r t * H e r e ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                           //
//         @@@@                                                                                  //
//                                                                                                     //
//      Start Step 1 --  show preview on LCM                                         //
//                                                                                                    �@//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR  CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR  CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define SENSOR_MAX_GAIN     32256//(31.5)                  // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN     1024//                     // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_CTRL_NUM                  (2)
#define SENSOR_SHUTTER_CTRL_NUM               (2)

#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period 30057                  //
#define Preview_line_period_DCG 27593//17814                     // MCLK=21.6 HTS/PCLK=3080 pixels/97.2MHZ=31.687us                              // 3126 for 25fps
#define vts_30fps  1109//1090                              //for 29.091fps @ MCLK=36MHz
#define vts_30fps_DCG       1206//1178                               // VTS for 20fps
#define Preview_WIDTH       1920                   //resolution Width when preview
#define Preview_HEIGHT      1080                   //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     3                      //slowest preview FPS

#define SENSOR_I2C_ADDR     0x78                    //I2C slave address
#define SENSOR_I2C_SPEED    200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY   I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT      I2C_FMT_A8D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
static int SP2329_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int SP2329_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int SP2329_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
static int SP2329_SetAEUSecs_HDR_lef(ss_cus_sensor *handle, u32 us);
static int SP2329_SetAEUSecs_HDR_sef(ss_cus_sensor *handle, u32 us);
#define HDR_HCG   0

typedef struct {
    struct {
        u16 pre_div0;
        u16 div124;
        u16 div_cnt7b;
        u16 sdiv0;
        u16 mipi_div0;
        u16 r_divp;
        u16 sdiv1;
        u16 r_seld5;
        u16 r_sclk_dac;
        u16 sys_sel;
        u16 pdac_sel;
        u16 adac_sel;
        u16 pre_div_sp;
        u16 r_div_sp;
        u16 div_cnt5b;
        u16 sdiv_sp;
        u16 div12_sp;
        u16 mipi_lane_sel;
        u16 div_dac;
    } clk_tree;
    struct {
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    struct {
        float sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
        u32 lines;
        u32 max_short;
    } expo;
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool mirror_dirty;
    bool dirty;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tMirror_reg[4];
    I2C_ARRAY tGain_vc1_reg[3];
    I2C_ARRAY tExpo_vc0_reg[2];
    I2C_ARRAY tExpo_vc1_reg[2];
    CUS_CAMSENSOR_ORIT cur_orien;
} sp2329_params;

// set sensor ID address and data,

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _sensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    // Show resolution string
    struct _senstr{
        const char* strResDesc;
    }senstr;
}sp2329_mipi_linear[] = {
    {LINEAR_RES_1, {1928, 1088, 3, 30}, {4, 4, 1920, 1080}, {"1920x1080@30fps"}},};

static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_END}mode;
    // Sensor Output Image info
    struct _hsenout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _hsensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    // Show resolution string
    struct _hsenstr{
        const char* strResDesc;
    }senstr;
}sp2329_mipi_hdr[] = {
   {HDR_RES_1, {1928, 1088, 3, 30}, {4, 4, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
};

const static I2C_ARRAY Sensor_init_table[] =
{
//SP2329_mipi_raw10_1928_1088_mclk24M_Pclk67.2M_2lane_30fps_V15_20200430
    {0xfd,0x00},
    {0x2e,0x19},
    {0x31,0x12},
    {0x34,0x30},
    {0x41,0x0a},
    {0xa1,0x02},
    {0xfd,0x01},
    {0x03,0x01},
    {0x04,0x54},
    {0x06,0x00},
    {0x09,0x02},
    {0x0a,0x00},
    {0x24,0x10},
    {0x01,0x01},
    {0x0d,0x00},
    {0x11,0x0e},
    {0x12,0x04},
    {0x13,0x22},
    {0x14,0x00},
    {0x16,0x78},
    {0x19,0x42},
    {0x1a,0xdf},
    {0x1c,0x02},
    {0x1e,0x16},
    {0x1f,0x23},
    {0x21,0x5f},
    {0x22,0x00},
    {0x25,0x00},
    {0x27,0x42},
    {0x28,0x00},
    {0x2c,0x00},
    {0x31,0x01},
    {0x01,0x01},
    {0x34,0x00},
    {0x35,0x02},
    {0x36,0x03},
    {0x37,0xc4},
    {0x38,0x30},
    {0x4a,0x00},
    {0x4b,0x04},
    {0x4c,0x04},
    {0x4d,0x40},
    {0x50,0x00},
    {0x52,0x1e},
    {0x54,0x00},
    {0x55,0x1a},
    {0x56,0x00},
    {0x57,0x1a},
    {0x59,0x18},
    {0x5a,0xff},
    {0x5b,0x02},
    {0x5c,0xd0},
    {0x5f,0x04},
    {0x62,0x00},
    {0x65,0xff},
    {0x67,0xff},
    {0x6e,0xf0},
    {0x70,0xff},
    {0x71,0x0b},
    {0x72,0x35},
    {0x73,0x0b},
    {0x76,0xd8},
    {0x79,0x00},
    {0x7a,0xf7},
    {0x86,0x2a},
    {0x8a,0x00},
    {0x8b,0x00},
    {0x8e,0xfd},
    {0x8f,0xfd},
    {0x90,0xfd},
    {0x91,0xfd},
    {0x92,0xfb},
    {0x93,0xfb},
    {0x94,0xfe},
    {0x95,0xfc},
    {0x96,0xa0},
    {0x97,0xa1},
    {0x98,0xa1},
    {0x99,0xa1},
    {0xd3,0x0e},
    {0xd5,0x44},
    {0xd6,0x01},
    {0xd7,0x34},
    {0xfd,0x00},
    {0x8e,0x07},
    {0x8f,0x88},
    {0x90,0x04},
    {0x91,0x40},
    {0xb1,0x01},
    {0xfd,0x02},
    {0x6c,0x01},
    {0x6d,0x3b},
    {0x6e,0x20},
    {0x72,0x40},
    {0x73,0x40},
    {0x74,0x40},
    {0x75,0x40},
    {0xfd,0x00},
    {0xb1,0x03},
};

const static I2C_ARRAY Sensor_init_table_DCG[] =
{
#if 1
//SP2329_1928X1088_MIPI2LANE_PCLK88P8M_30FPS_HDR_V18_20200825
    {0xfd,0x00},
    {0x2e,0x23},
    {0x33,0x01},
    {0x41,0x1a},
    {0x42,0x3d},
    {0xfd,0x01},
    {0x03,0x02},
    {0x04,0xd6},
    {0x06,0x41},
    {0x09,0x01},
    {0x0a,0x10},
    {0x24,0x22},
    {0x42,0x22},
    {0x01,0x01},
    {0x0d,0x00},
    {0x11,0x0e},
    {0x12,0x04},
    {0x13,0x62},
    {0x14,0x00},
    {0x16,0x78},
    {0x19,0x62},
    {0x1a,0xdf},
    {0x1c,0xa2},
    {0x1e,0x14},
    {0x1f,0x21},
    {0x21,0x5f},
    {0x22,0x82},
    {0x25,0x00},
    {0x27,0x42},
    {0x28,0x01},
    {0x2c,0x00},
    {0x31,0x21},
    {0x01,0x01},
    {0x34,0x00},
    {0x35,0x04},
    {0x36,0x03},
    {0x37,0xc4},
    {0x38,0x10},
    {0x4a,0x00},
    {0x4b,0x04},
    {0x4c,0x04},
    {0x4d,0x40},
    {0x50,0x00},
    {0x52,0x1e},
    {0x54,0x00},
    {0x55,0x55},
    {0x56,0x00},
    {0x57,0x20},
    {0x59,0x02},
    {0x5a,0x58},
    {0x5b,0x00},
    {0x5c,0x00},
    {0x5d,0x30},
    {0x5f,0x04},
    {0x62,0x00},
    {0x65,0x45},
    {0x67,0x45},
    {0x6c,0x0f},
    {0x6d,0x2e},
    {0x6e,0xf0},
    {0x70,0xf0},
    {0x71,0x0b},
    {0x72,0x2a},
    {0x73,0x0b},
    {0x76,0xd8},
    {0x79,0x00},
    {0x7a,0xf7},
    {0x7b,0x30},
    {0x86,0x29},
    {0x8a,0x55},
    {0x8b,0x55},
    {0x8e,0xf0},
    {0x8f,0xf0},
    {0x90,0xf0},
    {0x91,0xf0},
    {0x92,0xee},
    {0x93,0xf2},
    {0x94,0xf2},
    {0x95,0xee},
    {0x96,0xa0},
    {0x97,0xa1},
    {0x98,0xa1},
    {0x99,0xa1},
    {0xd3,0x0e},
    {0xd5,0x27},
    {0xd7,0xb2},
    {0xfd,0x00},
    {0x8e,0x07},
    {0x8f,0x88},
    {0x90,0x04},
    {0x91,0x40},
    {0xb1,0x01},
    {0xfd,0x02},
    {0x34,0xff},
    {0x6c,0x01},
    {0x6d,0x3b},
    {0x6e,0x1c},
    {0x72,0x40},
    {0x73,0x40},
    {0x74,0x40},
    {0x75,0x40},
    {0x82,0x18},
    {0x9f,0x18},
    {0xfd,0x00},
    {0xb1,0x03},
#else

    {0xfd, 0x00},
    {0x36, 0x01},
    {0xfd, 0x00},
    {0x36, 0x00},
    {0xfd, 0x00},
    {0x20, 0x00},
    {0xff, 0x05},
    {0x2e, 0x22},
    {0x33, 0x01},
    {0x41, 0x1a},//19
    {0x42, 0x3d},
    {0xfd, 0x01},
    {0x03, 0x02},
    {0x04, 0x98},
    {0x06, 0x00},
    {0x09, 0x01},
    {0x0a, 0x60},
    {0x24, 0x22},
    {0x42, 0x22},
    {0x01, 0x01},
    {0x0d, 0x00},
    {0x11, 0x0e},
    {0x12, 0x04},
    {0x13, 0x62},
    {0x14, 0x00},
    {0x16, 0x78},
    {0x19, 0x62},
    {0x1a, 0xdf},
    {0x1c, 0xa2},
    {0x1e, 0x14},
    {0x1f, 0x23},
    {0x21, 0x5f},
    {0x22, 0x82},
    {0x25, 0x00},
    {0x27, 0x42},
    {0x28, 0x01},
    {0x2c, 0x00},
    {0x31, 0x21},
    {0x01, 0x01},
    {0x34, 0x00},
    {0x35, 0x04},
    {0x36, 0x03},
    {0x37, 0xc0},
    {0x38, 0x10},
    {0x4a, 0x00},
    {0x4b, 0x08},
    {0x4c, 0x04},
    {0x4d, 0x38},
    {0x50, 0x00},
    {0x52, 0x1e},
    {0x54, 0x00},
    {0x55, 0x55},
    {0x56, 0x00},
    {0x57, 0x2a},
    {0x59, 0x02},
    {0x5a, 0x60},
    {0x5b, 0x00},
    {0x5c, 0x00},
    {0x5d, 0x30},
    {0x5f, 0x04},
    {0x62, 0x00},
    {0x65, 0x60},
    {0x67, 0x60},
    {0x6c, 0x0f},
    {0x6d, 0x2e},
    {0x6e, 0xf0},
    {0x70, 0xff},
    {0x71, 0x0b},
    {0x72, 0x3b},
    {0x73, 0x0b},
    {0x76, 0xd8},
    {0x79, 0x00},
    {0x7a, 0xf7},//00
    {0x7b, 0x30},
    {0x86, 0x3a},
    {0x8a, 0x55},
    {0x8b, 0x55},
    {0x8e, 0xf0},
    {0x8f, 0xf0},
    {0x90, 0xf0},
    {0x91, 0xf0},
    {0x92, 0xf4},//ee
    {0x93, 0xf2},
    {0x94, 0xf2},
    {0x95, 0xee},
    {0x96, 0xa0},
    {0x97, 0xa1},
    {0x98, 0xa1},
    {0x99, 0xa1},
    {0xd3, 0x0e},
    {0xd5, 0x27},
    {0xd7, 0xb2},
    {0xfd, 0x00},
    {0x8e, 0x07},
    {0x8f, 0x80},
    {0x90, 0x04},
    {0x91, 0x38},
    {0xb1, 0x01},
    {0xfd, 0x02},
    {0x6c, 0x01},
    {0x6d, 0x3b},
    {0x6e, 0x1c},//20
    {0x72, 0x40},
    {0x73, 0x40},
    {0x74, 0x40},
    {0x75, 0x40},
    {0x82, 0x18},
    {0x9f, 0x18},
    {0xfd, 0x00},
    {0xb1, 0x03},//;mipi en
#endif
};

I2C_ARRAY TriggerStartTbl[] = {
    //{0x0100,0x01},//normal mode
};

const I2C_ARRAY PatternTbl[] = {
//    {0xb2,0x40}, //colorbar pattern , bit 0 to enable
};

/////////////////////////////////////////////////////////////////
//       @@@@@@                                                                                    //
//                 @@                                                                                    //
//             @@@                                                                                      //
//       @       @@                                                                                    //
//         @@@@                                                                                        //
//                                                                                                          //
//      Step 3 --  complete camera features                                              //
//                                                                                                         //
//                                                                                                         //
//  camera set EV, MWB, orientation, contrast, sharpness                          //
//   , saturation, and Denoise can work correctly.                                     //
//                                                                                                          //
/////////////////////////////////////////////////////////////////

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const I2C_ARRAY mirror_reg[] = {
    {0xfd, 0x01}, //0
    {0x01, 0x00}, //1
    {0x3f, 0x00}, //mirror/flip
    {0x01, 0x01}, //3
};

const I2C_ARRAY gain_reg[] = {
    {0x38, 0x00},//long a-gain[8]
    {0x24, 0x20},//long a-gain[7:0]
    {0x39, 0x08},// d-gain[7:0] 1x:0x08 32x:0xff
};


const I2C_ARRAY expo_reg[] = {
    {0x03, 0x01},
    {0x04, 0x00},
};

const I2C_ARRAY vts_reg[] = {
    {0x0d,0x10},//0x10 enable
    {0x0E,0x04},//MSB
    {0x0F,0x7f},//LSB
};

const static I2C_ARRAY gain_vc1_reg[] = {
    {0x41, 0x00},//long a-gain[8]
    {0x42, 0x20},//long a-gain[7:0]
    {0x40, 0x08},// d-gain[7:0] 1x:0x08 32x:0xff
};

const I2C_ARRAY expo_vc0_reg[] = {
    {0x03, 0x01},
    {0x04, 0x00},
};

const I2C_ARRAY expo_vc1_reg[] = {
    {0x2f, 0x00},
    {0x30, 0x10},
};


#if 0
CUS_INT_TASK_ORDER def_order = {
        .RunLength = 9,
        .Orders = {
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
        },
};
#endif

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME sp2329

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period*2;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_min                   = Preview_line_period_DCG*2;
        info->u32AEShutter_step                  = Preview_line_period_DCG;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_max                   = Preview_line_period_DCG*params->expo.max_short;
        }
        else
        {
            info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        }
    }
    return SUCCESS;
}

static int SP2329_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sp2329_params *params = (sp2329_params *)handle->private_data;

    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(5000);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }
    SENSOR_USLEEP(2000);
    // power -> high, reset -> high
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(2000);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_USLEEP(5000);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(5000);

    params->cur_orien = SENSOR_ORIT;
    return SUCCESS;
}

static int SP2329_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    //handle->i2c_bus->i2c_close(handle->i2c_bus);
    SENSOR_USLEEP(1000);
    //Set_csi_if(0, 0);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->MCLK(idx, 0, handle->mclk);

    return SUCCESS;
}

/////////////////// image function /////////////////////////
//Get and check sensor ID

static int SP2329_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    return SUCCESS;
}

static int SP2329_SetFPS(ss_cus_sensor *handle, u32 fps);
static int SP2329_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);

static int SP2329_init_DCG(ss_cus_sensor *handle)
{
    int i;
    sp2329_params *params = (sp2329_params *)handle->private_data;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_DCG);i++)
    {
        if(Sensor_init_table_DCG[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_DCG[i].data);
        }
        if(SensorReg_Write(Sensor_init_table_DCG[i].reg,Sensor_init_table_DCG[i].data) != SUCCESS)
        {
           SENSOR_DMSG("[%s] I2C write fail\n", __FUNCTION__);
            return FAIL;
        }
     // SensorReg_Read(Sensor_init_table_DCG[i].reg, &sen_data);
      // printf("[%s] i=0x%x,sen_data=0x%x\n", __FUNCTION__,i,sen_data);
    }

    for(i=0;i< ARRAY_SIZE(mirror_reg); i++)
    {
        if(SensorReg_Write(params->tMirror_reg[i].reg,params->tMirror_reg[i].data) != SUCCESS)
        {
            return FAIL;
        }
    }

    SP2329_SetOrien(handle, params->cur_orien);
    params->tVts_reg[1].data = ((params->expo.vts >> 8) & 0x00ff);
    params->tVts_reg[2].data = ((params->expo.vts >> 0) & 0x00ff);

    return SUCCESS;
}

static int SP2329_init(ss_cus_sensor *handle)
{
    int i;
    sp2329_params *params = (sp2329_params *)handle->private_data;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {
        if(Sensor_init_table[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        if(SensorReg_Write(Sensor_init_table[i].reg,Sensor_init_table[i].data) != SUCCESS)
        {
           SENSOR_DMSG("[%s] I2C write fail\n", __FUNCTION__);
            return FAIL;
        }
        // SensorReg_Read(Sensor_init_table[i].reg, &sen_data);
        // printf("[%s] i=0x%x,sen_data=0x%x\n", __FUNCTION__,i,sen_data);
    }

    for(i=0;i< ARRAY_SIZE(mirror_reg); i++)
    {
        if(SensorReg_Write(params->tMirror_reg[i].reg,params->tMirror_reg[i].data) != SUCCESS)
        {
            return FAIL;
        }
    }

    SP2329_SetOrien(handle, params->cur_orien);
    params->tVts_reg[1].data = ((params->expo.vts >> 8) & 0x00ff);
    params->tVts_reg[2].data = ((params->expo.vts >> 0) & 0x00ff);

    return SUCCESS;
}

static int SP2329_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int SP2329_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int SP2329_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int SP2329_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0://3840x2160@30fps
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = SP2329_init;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int SP2329_SetVideoRes_HDR_DCG(ss_cus_sensor *handle, u32 res_idx)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0://3840x2160@30fps
            handle->video_res_supported.ulcur_res = 0;
            params->expo.vts = vts_30fps_DCG;
            params->expo.fps = 20;
            params->expo.max_short=100;
            if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1) {
                handle->pCus_sensor_init = SP2329_init_DCG;
            }
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int SP2329_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    sp2329_params *params = (sp2329_params *)handle->private_data;
    return params->cur_orien;
}

static int SP2329_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    switch(orit)
    {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[2].data = 0x00;
            params->mirror_dirty = true;
            break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[2].data = 0x01;
            params->mirror_dirty = true;
            break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[2].data = 0x02;
            params->mirror_dirty = true;
            break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[2].data = 0x03;
            params->mirror_dirty = true;
            break;
        default :
            break;
    }
    params->cur_orien = orit;
    return SUCCESS;
}

static int SP2329_GetFPS(ss_cus_sensor *handle)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int SP2329_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts = 0;
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*max_fps)/fps;
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if ((params->expo.lines) > (params->expo.vts - 4))
        vts = params->expo.lines + 4;
    else
        vts = params->expo.vts;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

static int SP2329_GetFPS_HDR_lef(ss_cus_sensor *handle)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 3000)
        params->expo.preview_fps = (vts_30fps_DCG*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_DCG*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int SP2329_SetFPS_HDR_lef(ss_cus_sensor *handle, u32 fps)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.max_short = (((params->expo.vts)/17 - 1)>>1) << 1;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

static int SP2329_GetFPS_HDR_sef(ss_cus_sensor *handle)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_DCG*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_DCG*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int SP2329_SetFPS_HDR_sef(ss_cus_sensor *handle, u32 fps)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    //pr_info("[%s] %d  %d \n\n", __FUNCTION__, params->expo.vts, fps);
    params->expo.max_short = (((params->expo.vts)/17 - 1)>>1) << 1;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int SP2329_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->mirror_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->mirror_dirty = false;
        }
        if(params->dirty)
        {
            SensorReg_Write(0xfd,0x01);//page 1
            SensorReg_Write(0x01,0x00);//frame sync disable
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            SensorReg_Write(0x01,0x01);//frame sync enable
            params->dirty = false;
        }

        break;
        default :
        break;
    }
    return SUCCESS;
}

static int SP2329_AEStatusNotify_HDR_lef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    //sp2329_params *params = (sp2329_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:

        break;
        default :
        break;
    }
    return SUCCESS;
}

static int SP2329_AEStatusNotify_HDR_sef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->mirror_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->mirror_dirty = false;
        }
        if(params->dirty)
        {
            SensorReg_Write(0xfd,0x01);//page 1
            SensorReg_Write(0x01,0x00);//frame sync disable
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_vc0_reg, ARRAY_SIZE(expo_vc0_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_vc1_reg, ARRAY_SIZE(expo_vc1_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_vc1_reg, ARRAY_SIZE(gain_vc1_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            SensorReg_Write(0x01,0x01);//frame sync enable
            params->dirty = false;
        }

        break;
        default :
        break;
    }
    return SUCCESS;
}

static int SP2329_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    int rc = SUCCESS;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
    sp2329_params *params = (sp2329_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[1].data&0xff);

    *us = (lines*Preview_line_period)/1000;

    return rc;
}

static int SP2329_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    sp2329_params *params = (sp2329_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period;
    if (lines < 2) lines = 2;
    if (lines >params->expo.vts - 4)
        vts = lines + 4;
    else
        vts=params->expo.vts;
    params->expo.lines = lines;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

   // lines <<= 4;
    params->tExpo_reg[0].data =(lines>>8) & 0x00ff;
    params->tExpo_reg[1].data =(lines>>0) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;

    return SUCCESS;
}

static int SP2329_GetAEUSecs_HDR_lef(ss_cus_sensor *handle, u32 *us)
{
    int rc = SUCCESS;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_vc0_reg, ARRAY_SIZE(expo_vc0_reg));
    sp2329_params *params = (sp2329_params *)handle->private_data;

    lines |= (u32)(params->tExpo_vc0_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_vc0_reg[1].data&0xff)<<0;

    *us = (lines*Preview_line_period_DCG)/1000;

    return rc;
}

static int SP2329_SetAEUSecs_HDR_lef(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    sp2329_params *params = (sp2329_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_DCG;
    if (lines < 2) lines = 2;
    if (lines > ((params->expo.vts) - (params->expo.max_short) - 4))
        lines = (params->expo.vts) - (params->expo.max_short) - 4;
    else
        vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    // lines <<= 4;
    params->tExpo_vc0_reg[0].data = (lines>>8) & 0x00ff;
    params->tExpo_vc0_reg[1].data = (lines>>0) & 0x00ff;

    //pr_info("[%s] shutter %d  0x3e01 0x%x  0x3e02 0x%x\n", __FUNCTION__, us, params->tExpo_vc0_reg[0].data,params->tExpo_vc0_reg[1].data);
    params->dirty = true;

    return SUCCESS;
}

static int SP2329_GetAEUSecs_HDR_sef(ss_cus_sensor *handle, u32 *us)
{
    int rc = SUCCESS;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_vc1_reg, ARRAY_SIZE(expo_vc1_reg));
    sp2329_params *params = (sp2329_params *)handle->private_data;

    lines |= (u32)(params->tExpo_vc1_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_vc1_reg[1].data&0xff)<<0;

    *us = (lines*Preview_line_period_DCG)/1000;

    return rc;
}

static int SP2329_SetAEUSecs_HDR_sef(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    sp2329_params *params = (sp2329_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_DCG;
    if (lines < 2) lines = 2;
    if (lines > ((params->expo.max_short) - 4))
        lines = (params->expo.max_short) - 4;
    else
        vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    // lines <<= 4;
    params->tExpo_vc1_reg[0].data = (lines>>8) & 0x00ff;
    params->tExpo_vc1_reg[1].data = (lines>>0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int SP2329_GetAEGain(ss_cus_sensor *handle, u32* gain)
{

    //SENSOR_DMSG("[%s] get gain/reg0/reg1 (1024=1X)= %d/0x%x/0x%x\n", __FUNCTION__, *gain,params->tGain_reg[0].data,params->tGain_reg[1].data);
    return SUCCESS;
}

#define MAX_A_GAIN 31744//(31*1024)
static int SP2329_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 input_gain = 0;
    u16 gain16 = 0;

    gain = (gain * SENSOR_MIN_GAIN + 512)>>10; // need to add min sat gain

    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    /* A Gain */
    if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < 16384)) {
        gain = (gain>>9)<<9;
    } else if ((gain >= 16384) && (gain < MAX_A_GAIN)) {
        gain = (gain>>10)<<10;
    } else {
        gain = MAX_A_GAIN;
    }

    gain16=(u16)(gain>>6);
    params->tGain_reg[0].data = (gain16>>8)&0x01;//high bit
    params->tGain_reg[1].data = gain16&0xff; //low byte

    //printk("[%s] gain_input %d, gain_real %d, reg3798 0x%x\n",__FUNCTION__, input_gain_rel, input_gain, params->tConvgain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}

static int SP2329_SetAEGain_HDR_sef(ss_cus_sensor *handle, u32 gain)
{
    sp2329_params *params = (sp2329_params *)handle->private_data;
    u32 input_gain = 0;
    u16 gain16 = 0;

    gain = (gain * SENSOR_MIN_GAIN + 512)>>10; // need to add min sat gain

    //gain = 4*gain;
    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    /* A Gain */
    if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < 16384)) {
        gain = (gain>>9)<<9;
    } else if ((gain >= 16384) && (gain < MAX_A_GAIN)) {
        gain = (gain>>10)<<10;
    } else {
        gain = MAX_A_GAIN;
    }

    gain16=(u16)(gain>>6);
    params->tGain_vc1_reg[0].data = (gain16>>8)&0x01;//high bit
    params->tGain_vc1_reg[1].data = gain16&0xff; //low byte

    params->dirty = true;
    //pr_info("[%s] set input gain/gain/AregH/AregL/DregH/DregL=%d/%d/0x%x/0x%x/0x%x/0x%x\n", __FUNCTION__, input_gain,gain,params->tGain_reg[0].data,params->tGain_reg[1].data,params->tGain_reg[2].data,params->tGain_reg[3].data);

    return SUCCESS;
}

static int SP2329_SetPatternMode_hdr_lef(ss_cus_sensor *handle,u32 mode)
{
    return SUCCESS;
}

static int SP2329_poweron_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int SP2329_poweroff_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int SP2329_init_hdr_lef(ss_cus_sensor *handle)
{
    return SUCCESS;
}

#define CMDID_I2C_READ   (0x01)
#define CMDID_I2C_WRITE  (0x02)

static int pCus_sensor_CustDefineFunction(ss_cus_sensor* handle,u32 cmd_id, void *param) {

    if(param == NULL || handle == NULL)
    {
        SENSOR_EMSG("param/handle data NULL \n");
        return FAIL;
    }

    switch(cmd_id)
    {
        case CMDID_I2C_READ:
        {
            I2C_ARRAY *reg = (I2C_ARRAY *)param;
            SensorReg_Read(reg->reg, &reg->data);
            SENSOR_EMSG("reg %x, read data %x \n", reg->reg, reg->data);
            break;
        }
        case CMDID_I2C_WRITE:
        {
            I2C_ARRAY *reg = (I2C_ARRAY *)param;
            SENSOR_EMSG("reg %x, write data %x \n", reg->reg, reg->data);
            SensorReg_Write(reg->reg, reg->data);
            break;
        }
        default:
            SENSOR_EMSG("cmd id %d err \n", cmd_id);
            break;
    }

    return SUCCESS;
}

int cus_camsensor_init_handle(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sp2329_params *params;
    int res;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (sp2329_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tGain_vc1_reg, gain_vc1_reg, sizeof(gain_vc1_reg));
    memcpy(params->tExpo_vc0_reg, expo_vc0_reg, sizeof(expo_vc0_reg));
    memcpy(params->tExpo_vc1_reg, expo_vc1_reg, sizeof(expo_vc1_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SP2329_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = sp2329_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sp2329_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sp2329_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sp2329_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sp2329_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sp2329_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sp2329_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sp2329_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sp2329_mipi_linear[res].senstr.strResDesc);
    }

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    /////////////////////////////////////////////////////

    //Mirror / Flip
    params->cur_orien = SENSOR_ORIT;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = SP2329_init    ;
    handle->pCus_sensor_poweron     = SP2329_poweron ;
    handle->pCus_sensor_poweroff    = SP2329_poweroff;

    // Normal
    handle->pCus_sensor_GetVideoResNum = SP2329_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = SP2329_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = SP2329_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = SP2329_SetVideoRes;
    handle->pCus_sensor_GetOrien          = SP2329_GetOrien      ;
    handle->pCus_sensor_SetOrien          = SP2329_SetOrien      ;
    handle->pCus_sensor_GetFPS          = SP2329_GetFPS      ;
    handle->pCus_sensor_SetFPS          = SP2329_SetFPS      ;
    handle->pCus_sensor_SetPatternMode = SP2329_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = SP2329_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = SP2329_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = SP2329_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = SP2329_GetAEGain;
    handle->pCus_sensor_SetAEGain       = SP2329_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    params->expo.vts=vts_30fps;
    params->expo.fps = 30;
    params->expo.lines = 1000;
    params->mirror_dirty = false;
    params->dirty = false;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dcg_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sp2329_params *params = NULL;
    int res;

    cus_camsensor_init_handle(drv_handle);
    params = (sp2329_params *)handle->private_data;

    sprintf(handle->strSensorStreamName,"SP2329_MIPI_HDR_SEF");

    handle->data_prec   = SENSOR_DATAPREC_HDR;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = sp2329_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sp2329_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sp2329_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sp2329_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sp2329_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sp2329_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sp2329_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sp2329_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sp2329_mipi_hdr[res].senstr.strResDesc);
    }

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //Mirror / Flip
    params->cur_orien = SENSOR_ORIT;


    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = SP2329_init_DCG;

    // Normal
    handle->pCus_sensor_SetVideoRes       = SP2329_SetVideoRes_HDR_DCG;
    handle->pCus_sensor_GetFPS          = SP2329_GetFPS_HDR_sef;
    handle->pCus_sensor_SetFPS          = SP2329_SetFPS_HDR_sef;

    handle->pCus_sensor_AEStatusNotify = SP2329_AEStatusNotify_HDR_sef;
    handle->pCus_sensor_GetAEUSecs      = SP2329_GetAEUSecs_HDR_sef;
    handle->pCus_sensor_SetAEUSecs      = SP2329_SetAEUSecs_HDR_sef;
    handle->pCus_sensor_GetAEGain       = SP2329_GetAEGain;
    handle->pCus_sensor_SetAEGain       = SP2329_SetAEGain_HDR_sef;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    params->expo.vts=vts_30fps_DCG;
    params->expo.fps = 20;
    params->expo.max_short=90;
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_DCG*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_DCG*params->expo.max_short;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_DCG;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dcg_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sp2329_params *params;
    int res;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (sp2329_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tGain_vc1_reg, gain_vc1_reg, sizeof(gain_vc1_reg));
    memcpy(params->tExpo_vc0_reg, expo_vc0_reg, sizeof(expo_vc0_reg));
    memcpy(params->tExpo_vc1_reg, expo_vc1_reg, sizeof(expo_vc1_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SP2329_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_HDR;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = sp2329_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sp2329_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sp2329_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sp2329_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sp2329_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sp2329_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sp2329_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sp2329_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sp2329_mipi_hdr[res].senstr.strResDesc);
    }

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    /////////////////////////////////////////////////////

    //Mirror / Flip
    params->cur_orien = SENSOR_ORIT;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_DCG*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_DCG;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = SP2329_init_hdr_lef    ;
    handle->pCus_sensor_poweron     = SP2329_poweron_hdr_lef ;
    handle->pCus_sensor_poweroff    = SP2329_poweroff_hdr_lef;

    // Normal
    handle->pCus_sensor_GetOrien          = SP2329_GetOrien;
    handle->pCus_sensor_SetOrien          = SP2329_SetOrien;
    handle->pCus_sensor_GetFPS          = SP2329_GetFPS_HDR_lef;
    handle->pCus_sensor_SetFPS          = SP2329_SetFPS_HDR_lef;
    handle->pCus_sensor_SetPatternMode = SP2329_SetPatternMode_hdr_lef;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = SP2329_AEStatusNotify_HDR_lef;
    handle->pCus_sensor_GetAEUSecs      = SP2329_GetAEUSecs_HDR_lef;
    handle->pCus_sensor_SetAEUSecs      = SP2329_SetAEUSecs_HDR_lef;
    handle->pCus_sensor_GetAEGain       = SP2329_GetAEGain;
    handle->pCus_sensor_SetAEGain       = SP2329_SetAEGain;

    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    params->expo.vts=vts_30fps_DCG;
    params->expo.fps = 30;
    params->expo.lines = 1000;
    params->mirror_dirty = false;
    params->dirty = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  SP2329_HDR,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_hdr_dcg_sef,
                            cus_camsensor_init_handle_hdr_dcg_lef,
                            sp2329_params
                         );
