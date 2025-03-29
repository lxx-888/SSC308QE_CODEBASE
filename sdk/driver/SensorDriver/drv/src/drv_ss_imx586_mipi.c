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
/*
Sensor Porting on Master V4
Porting owner :Jilly
Date          :23/09/22
Build on      :Master_V4 i6c
Verified on   :not yet
Remark        :base on master_v3 imx586
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(imx586);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

//#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
//#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE            CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL   CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
#define ENABLE            1
#define DISABLE           0
#undef SENSOR_DBG
#define SENSOR_DBG        0

#define DEBUG_INFO        0

#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
//#define SENSOR_DMSG(args...) printf(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
///////////////////////////////////////////////////////////////
//          @@@                                              //
//         @ @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//          @@@@                                             //
//                                                           //
//      Start Step 1 --  show preview on LCM                 //
//                                                           //
//  Fill these #define value and table with correct settings //
//      camera can work and show preview on LCM              //
//                                                           //
///////////////////////////////////////////////////////////////

////////////////////////////////////
// Sensor-If Info                 //
////////////////////////////////////
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM        4//(lane_num)          //imx586 Linear mode supports MIPI 4-Lane
//#define SENSOR_MIPI_LANE_NUM_DOL    (hdr_lane_num)      //imx586 DOL mode supports MIPI 4-Lane
//#define SENSOR_MIPI_HDR_MODE        (0) //0: Non-HDR mode. 1:Sony DOL mode

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
//#define SENSOR_DATAFMT             CUS_DATAFMT_BAYER    //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9098  //CFG
#define SENSOR_BAYERID              CUS_BAYER_RG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_YCORDER              CUS_SEN_YCODR_YC     //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define long_packet_type_enable     0x00 //UD1~UD8 (user define)

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ//CUS_CMU_CLK_12MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x20                //I2C slave address
//#define SENSOR_I2C_ADDR              0x34                //I2C slave address
#define SENSOR_I2C_SPEED             300000              //200000 //300000 //240000                  //I2C speed, 60000~320000
//#define SENSOR_I2C_CHANNEL           1                 //I2C Channel
//#define SENSOR_I2C_PAD_MODE          2                 //Pad/Mode Number
#define SENSOR_I2C_LEGACY            I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT               I2C_FMT_A16D8       //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

////////////////////////////////////
// Sensor Signal                  //
////////////////////////////////////
#define SENSOR_PWDN_POL              CUS_CLK_POL_NEG     // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL               CUS_CLK_POL_NEG     // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG
                                                         // VSYNC/HSYNC POL can be found in data sheet timing diagram,
                                                         // Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL             CUS_CLK_POL_NEG     // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL             CUS_CLK_POL_NEG     // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL              CUS_CLK_POL_NEG     // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

////////////////////////////////////
// Sensor ID                      //
////////////////////////////////////
//define SENSOR_ID

#undef SENSOR_NAME
#define SENSOR_NAME                   imx586

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    /*enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_END}mode;*/
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, /*LINEAR_RES_3,*/ LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _sensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
   // u32 nMinFrameLengthLine;
    // Show resolution string
    struct _senstr{
        const char* strResDesc;
    }senstr;
}imx586_mipi_linear[] = {
    {LINEAR_RES_1, {4000,3000, 3, 30}, {0, 0, 4000, 3000}, {"4000x3000@30fps_ms"}},
    {LINEAR_RES_2, {4000,3000, 3, 30}, {0, 0, 4000, 3000}, {"4000x3000@30fps_sl"}},
    //{LINEAR_RES_3, {4000,3000, 3, 30}, {0, 364, 4000, 2272}, {"4000x2272@30fps"}},
    //{LINEAR_RES_2, {3280, 2456, 3, 15}, {0, 0, 3200, 2400}, {"3200x2400@30fps"}},
};

#if 0
static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_2, HDR_RES_3, HDR_RES_END}mode;
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
}imx586_mipi_hdr[] = {
    {HDR_RES_1, {2592, 1944, 3, 25}, {4, 0, 2592, 1944}, {"2592x1944@25fps_HDR"}}, // Modify it
    {HDR_RES_2, {2592, 1944, 3, 20}, {4, 0, 2592, 1944}, {"2592x1944@20fps_HDR"}}, // Modify it
    {HDR_RES_3, {2592, 1944, 3, 30}, {4, 0, 2592, 1944}, {"2592x1944@30fps_HDR"}}, // Modify it
};
#endif


u32 Preview_line_period;
u32 vts_30fps;
u32 Preview_MAX_FPS;


//#define SENSOR_YCORDER      CUS_SEN_YCODR_YC       //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
//#define long_packet_type_enable 0x00 //UD1~UD8 (user define)
////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (22 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_CTRL_NUM                        (1)
#define SENSOR_SHUTTER_CTRL_NUM                     (1)

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR_FLIP_REG                             0x0101

#define SENSOR_NOR                                  0x0
#define MIRROR_EN                                   0x1 << (0)
#define FLIP_EN                                     0x1 << (1)

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif

//static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);

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
        u32 expo_lines;
        u32 expo_lef_us;
        u32 expo_sef_us;
    } expo;
    u32 max_rhs1;
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool dirty;
    bool orien_dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tExpo_reg[2];
    u8 uSnrPad;
    u8 bSlaveMode;
} imx586_params;
// set sensor ID address and data,

const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3003, 0x00},      // {address of ID, ID },
    {0x3033, 0x00}
};

#if 0 /*4000x3000@30fps, official setting*/
const static I2C_ARRAY Sensor_init_table_4lane_12m30fps[] =
{
#if 1
/*IMX586-AAJH5-C setting*/
/**** 25fps ****/
{0x0136,0x18},
{0x0137,0x00},
{0x3C7E,0x01},
{0x3C7F,0x03},
{0x0111,0x02},
{0x3702,0x1F},
{0x3706,0x17},
{0x3707,0x6F},
{0x3C00,0x10},
{0x3C01,0x10},
{0x3C02,0x10},
{0x3C03,0x10},
{0x3C04,0x10},
{0x3C05,0x01},
{0x3C06,0x00},
{0x3C07,0x00},
{0x3C08,0x03},
{0x3C09,0xFF},
{0x3C0A,0x01},
{0x3C0B,0x00},
{0x3C0C,0x00},
{0x3C0D,0x03},
{0x3C0E,0xFF},
{0x3C0F,0x20},
{0x4D14,0xA6},
{0x4D29,0xB0},
{0x4D45,0x56},
{0x4D49,0x00},
{0x4D53,0xCF},
{0x4D55,0x00},
{0x4D5C,0xA6},
{0x4D71,0xB0},
{0x4D8D,0x86},
{0x4D91,0x00},
{0x4D99,0x3C},
{0x4D9B,0x17},
{0x4D9D,0x00},
{0x4DA4,0xA6},
{0x4DB9,0xB0},
{0x4DD5,0x7A},
{0x4DD9,0x00},
{0x4DE3,0xED},
{0x4DE5,0x00},
{0x4DEC,0xA6},
{0x4E01,0xB0},
{0x4E1D,0x4A},
{0x4E2B,0x69},
{0x4E2D,0x00},
{0x4E34,0xA6},
{0x4E49,0xB0},
{0x4E65,0x49},
{0x4E69,0x00},
{0x4E73,0x7C},
{0x4E75,0x00},
{0x4E81,0x0D},
{0x4E87,0x82},
{0x4E95,0xBF},
{0x4E97,0x00},
{0x5715,0x09},
{0x5717,0x93},
{0x5729,0x93},
{0x572B,0x0A},
{0x572D,0x0C},
{0x572F,0x93},
{0x57BF,0x5C},
{0x5855,0xA0},
{0x5863,0x9E},
{0x586F,0xA4},
{0x5C49,0x59},
{0x5C4A,0x01},
{0x5C4B,0x9C},
{0x5D0A,0x01},
{0x5D0B,0x93},
{0x5D28,0x01},
{0x5D29,0x93},
{0x5D4A,0x01},
{0x5D4B,0x93},
{0x5D68,0x01},
{0x5D69,0x93},
{0x646F,0x1E},
{0x6607,0x1E},
{0x6630,0x1E},
{0x6659,0x1E},
{0x6682,0x1E},
{0x66AB,0x1E},
{0x66D4,0x1E},
{0x66FD,0x1E},
{0x6726,0x1E},
{0x674F,0x1E},
{0x6778,0x1E},
{0x6C1C,0x00},
{0x6C1D,0x01},
{0x6C1E,0x03},
{0x6C1F,0x04},
{0x6C60,0x95},
{0x6C61,0x3B},
{0x6C66,0x01},
{0x6C67,0x85},
{0x6C68,0x52},
{0x6C6E,0x99},
{0x6C6F,0x14},
{0x6C76,0x75},
{0x6C77,0x79},
{0x6C7D,0x95},
{0x6C7E,0x8B},
{0x6C83,0x01},
{0x6C84,0x45},
{0x6C85,0x62},
{0x6C8B,0x89},
{0x6C8C,0x19},
{0x6C92,0x1C},
{0x6C94,0xB9},
{0x6C99,0x05},
{0x6C9A,0x89},
{0x6C9B,0x4B},
{0x6CA1,0xE5},
{0x6CA2,0x56},
{0x6CA8,0x81},
{0x6CA9,0x15},
{0x6CAF,0x22},
{0x6CB0,0xA5},
{0x6CB1,0xF9},
{0x6CB6,0x05},
{0x6CB7,0x09},
{0x6CB8,0x4B},
{0x6CBD,0x01},
{0x6CBE,0x4A},
{0x6CBF,0x56},
{0x6CC5,0x82},
{0x6CC6,0x96},
{0x6CCC,0x22},
{0x6CCD,0x75},
{0x6CCE,0xF9},
{0x6CD3,0x05},
{0x6CD4,0x89},
{0x6CD5,0x3B},
{0x6CDB,0xE5},
{0x6CDC,0x52},
{0x6CE2,0x81},
{0x6CE3,0x15},
{0x6CE9,0x22},
{0x6CEA,0xA5},
{0x6CEB,0x79},
{0x6CF0,0x05},
{0x6CF1,0x09},
{0x6CF2,0x4B},
{0x6CF7,0x01},
{0x6CF8,0x6A},
{0x6CF9,0x52},
{0x6F29,0x07},
{0x6F2A,0x08},
{0x7103,0x92},
{0x7104,0x95},
{0x710A,0x22},
{0x710B,0x75},
{0x710C,0xB9},
{0x7111,0x05},
{0x7112,0x89},
{0x7113,0x0B},
{0x7119,0xE5},
{0x711A,0x46},
{0x7120,0x61},
{0x7121,0x11},
{0x7127,0x28},
{0x7128,0x64},
{0x7129,0xB9},
{0x712E,0x04},
{0x712F,0x89},
{0x7130,0x0B},
{0x7135,0x01},
{0x7136,0x65},
{0x7137,0x46},
{0x713D,0xA1},
{0x713E,0x12},
{0x7144,0x28},
{0x7145,0x64},
{0x7146,0xB9},
{0x714B,0x05},
{0x714C,0x89},
{0x714D,0x2B},
{0x7153,0xE5},
{0x7154,0x4E},
{0x715A,0x61},
{0x715B,0x14},
{0x7161,0x1C},
{0x7162,0xA5},
{0x7163,0x79},
{0x7168,0x07},
{0x7169,0x09},
{0x716A,0x3B},
{0x716F,0x01},
{0x7170,0xA5},
{0x7171,0x52},
{0x7177,0x72},
{0x7178,0x14},
{0x717E,0x1C},
{0x7180,0x79},
{0x71C4,0xE4},
{0x71C9,0xC9},
{0x71CD,0x9D},
{0x71D0,0xC9},
{0x71D4,0x9D},
{0x71D7,0xC9},
{0x71D9,0xE2},
{0x71DA,0x75},
{0x9004,0x10},
{0x9200,0xF4},
{0x9201,0xA7},
{0x9202,0xF4},
{0x9203,0xAA},
{0x9204,0xF4},
{0x9205,0xAD},
{0x9206,0xF4},
{0x9207,0xB0},
{0x9208,0xF4},
{0x9209,0xB3},
{0x920A,0xB7},
{0x920B,0x34},
{0x920C,0xB7},
{0x920D,0x36},
{0x920E,0xB7},
{0x920F,0x37},
{0x9210,0xB7},
{0x9211,0x38},
{0x9212,0xB7},
{0x9213,0x39},
{0x9214,0xB7},
{0x9215,0x3A},
{0x9216,0xB7},
{0x9217,0x3C},
{0x9218,0xB7},
{0x9219,0x3D},
{0x921A,0xB7},
{0x921B,0x3E},
{0x921C,0xB7},
{0x921D,0x3F},
{0x921E,0x77},
{0x921F,0x77},
{0x9385,0x3C},
{0x9387,0x3C},
{0x9389,0x3C},
{0x938B,0x26},
{0x938D,0x26},
{0x938F,0x3C},
{0x9391,0x3C},
{0x9393,0x32},
{0x9395,0x26},
{0x9397,0x26},
{0x9399,0x78},
{0x939B,0x78},
{0x939D,0x5A},
{0x939F,0x26},
{0x93A1,0x26},
{0x93A3,0x8C},
{0x93A5,0x8C},
{0x93A7,0x78},
{0x93A9,0x26},
{0x93AB,0x26},
{0x93AD,0x78},
{0x93AF,0x78},
{0x93B1,0x5A},
{0x93B3,0x26},
{0x93B5,0x26},
{0x93B7,0x50},
{0x93B9,0x50},
{0x93BB,0x50},
{0x93BD,0x26},
{0x93BF,0x26},
{0x93C1,0x78},
{0x93C3,0x78},
{0x93C5,0x5A},
{0x93C7,0x26},
{0x93C9,0x26},
{0x93CB,0x8C},
{0x93CD,0x8C},
{0x93CF,0x78},
{0x93D1,0x26},
{0x93D3,0x26},
{0x93D5,0x78},
{0x93D7,0x78},
{0x93D9,0x5A},
{0x93DB,0x26},
{0x93DD,0x26},
{0x93DF,0x50},
{0x93E1,0x50},
{0x93E3,0x50},
{0x93E5,0x26},
{0x93E7,0x26},
{0x9810,0x14},
{0x9814,0x14},
{0x99B2,0x20},
{0x99B3,0x0F},
{0x99B4,0x0F},
{0x99B5,0x0F},
{0x99B6,0x0F},
{0x99E4,0x0F},
{0x99E5,0x0F},
{0x99E6,0x0F},
{0x99E7,0x0F},
{0x99E8,0x0F},
{0x99E9,0x0F},
{0x99EA,0x0F},
{0x99EB,0x0F},
{0x99EC,0x0F},
{0x99ED,0x0F},
{0xBC76,0x0E},
{0xBC77,0x98},
{0xBC79,0xA0},
{0xBC7B,0xB8},
{0xBC7C,0x10},
{0xBC7D,0xA0},
{0xBC7F,0x30},
{0xC61D,0x00},
{0xC625,0x00},
{0xC638,0x03},
{0xC63B,0x01},
{0x0112,0x0A},
{0x0113,0x0A},
{0x0114,0x03},
{0x0342,0x1E},
{0x0343,0xC0},
{0x0340,0x0B},
{0x0341,0xF4},
{0x0344,0x00},
{0x0345,0x00},
{0x0346,0x00},
{0x0347,0x00},
{0x0348,0x1F},
{0x0349,0x3F},
{0x034A,0x17},
{0x034B,0x6F},
{0x0220,0x62},
{0x0222,0x01},
{0x0900,0x01},
{0x0901,0x22},
{0x0902,0x08},
{0x3140,0x00},
{0x3246,0x81},
{0x3247,0x81},
{0x3F15,0x00},
{0x0401,0x00},
{0x0404,0x00},
{0x0405,0x10},
{0x0408,0x00},
{0x0409,0x00},
{0x040A,0x00},
{0x040B,0x00},
{0x040C,0x0F},
{0x040D,0xA0},
{0x040E,0x0B},
{0x040F,0xB8},
{0x034C,0x0F},
{0x034D,0xA0},
{0x034E,0x0B},
{0x034F,0xB8},
{0x0301,0x05},
{0x0303,0x04},
{0x0305,0x04},
{0x0306,0x00},
{0x0307,0xF9},
{0x030B,0x02},
{0x030D,0x06},
{0x030E,0x01},
{0x030F,0x90},
{0x0310,0x01},
{0x3620,0x00},
{0x3621,0x00},
{0x3F0C,0x01},
{0x3F14,0x00},
{0x3F80,0x00},
{0x3F81,0x00},
{0x3FFC,0x00},
{0x3FFD,0x00},
{0x0202,0x0B},
{0x0203,0xC4},
{0x0224,0x01},
{0x0225,0xF4},
{0x3FE0,0x01},
{0x3FE1,0xF4},
{0x0204,0x00},
{0x0205,0x70},
{0x0216,0x00},
{0x0217,0x00},
{0x0218,0x01},
{0x0219,0x00},
{0x020E,0x01},
{0x020F,0x00},
{0x0210,0x01},
{0x0211,0x00},
{0x0212,0x01},
{0x0213,0x00},
{0x0214,0x01},
{0x0215,0x00},
{0x3FE2,0x00},
{0x3FE3,0x00},
{0x3FE4,0x01},
{0x3FE5,0x00},
{0x3E20,0x01},
{0x3E37,0x01},
{0xBCF1,0x00},
{0x0100,0x01},
{0x3130,0x01},

#else
    /**** 30fps ****/
	{0x0136,0x18},
	{0x0137,0x00},
	{0x3c7e,0x01},
	{0x3c7f,0x08},
	{0x0111,0x02},
	{0x380C,0x00},
	{0x3C00,0x10},
	{0x3C01,0x10},
	{0x3C02,0x10},
	{0x3C03,0x10},
	{0x3C04,0x10},
	{0x3C05,0x01},
	{0x3C06,0x00},
	{0x3C07,0x00},
	{0x3C08,0x03},
	{0x3C09,0xFF},
	{0x3C0A,0x01},
	{0x3C0B,0x00},
	{0x3C0C,0x00},
	{0x3C0D,0x03},
	{0x3C0E,0xFF},
	{0x3C0F,0x20},
	{0x3F88,0x00},
	{0x3F8E,0x00},
	{0x5282,0x01},
	{0x9004,0x14},
	{0x9200,0xF4},
	{0x9201,0xA7},
	{0x9202,0xF4},
	{0x9203,0xAA},
	{0x9204,0xF4},
	{0x9205,0xAD},
	{0x9206,0xF4},
	{0x9207,0xB0},
	{0x9208,0xF4},
	{0x9209,0xB3},
	{0x920A,0xB7},
	{0x920B,0x34},
	{0x920C,0xB7},
	{0x920D,0x36},
	{0x920E,0xB7},
	{0x920F,0x37},
	{0x9210,0xB7},
	{0x9211,0x38},
	{0x9212,0xB7},
	{0x9213,0x39},
	{0x9214,0xB7},
	{0x9215,0x3A},
	{0x9216,0xB7},
	{0x9217,0x3C},
	{0x9218,0xB7},
	{0x9219,0x3D},
	{0x921A,0xB7},
	{0x921B,0x3E},
	{0x921C,0xB7},
	{0x921D,0x3F},
	{0x921E,0x77},
	{0x921F,0x77},
	{0x9222,0xC4},
	{0x9223,0x4B},
	{0x9224,0xC4},
	{0x9225,0x4C},
	{0x9226,0xC4},
	{0x9227,0x4D},
	{0x9810,0x14},
	{0x9814,0x14},
	{0x99B2,0x20},
	{0x99B3,0x0F},
	{0x99B4,0x0F},
	{0x99B5,0x0F},
	{0x99B6,0x0F},
	{0x99E4,0x0F},
	{0x99E5,0x0F},
	{0x99E6,0x0F},
	{0x99E7,0x0F},
	{0x99E8,0x0F},
	{0x99E9,0x0F},
	{0x99EA,0x0F},
	{0x99EB,0x0F},
	{0x99EC,0x0F},
	{0x99ED,0x0F},
	{0xA569,0x06},
	{0xA679,0x20},
	{0xC020,0x01},
	{0xC61D,0x00},
	{0xC625,0x00},
	{0xC638,0x03},
	{0xC63B,0x01},
	{0xE286,0x31},
	{0xE2A6,0x32},
	{0xE2C6,0x33},
	{0xBCF1,0x00},
	//MIPI output setting
	{0x0112,0x0A},
	{0x0113,0x0A},
	{0x0114,0x03},
			//Line Length PCK Setting
	{0x0342,0x1E},
	{0x0343,0xC0},
		//Frame Length Lines Setting
	{0x0340,0x0B},
	{0x0341,0xF4},
		//ROI Setting
	{0x0344,0x00},
	{0x0345,0x00},
	{0x0346,0x00},
	{0x0347,0x00},
	{0x0348,0x1F},
	{0x0349,0x3F},
	{0x034A,0x17},
	{0x034B,0x6F},
		//Mode Setting
	{0x0220,0x62},
	{0x0222,0x01},
	{0x0900,0x01},
	{0x0901,0x22},
	{0x0902,0x08},
	{0x3140,0x00},
	{0x3246,0x81},
	{0x3247,0x81},
	{0x3F15,0x00},
		//Digital Crop & Scaling
	{0x0401,0x00},
	{0x0404,0x00},
	{0x0405,0x10},
	{0x0408,0x00},
	{0x0409,0x00},
	{0x040A,0x00},
	{0x040B,0x00},
	{0x040C,0x0F},
	{0x040D,0xA0},
	{0x040E,0x0B},
	{0x040F,0xB8},
	//Output Size Setting
	{0x034C,0x0F},
	{0x034D,0xA0},
	{0x034E,0x0B},
	{0x034F,0xB8},
		//Clock Setting
	{0x0301,0x05},
	{0x0303,0x04},
	{0x0305,0x04},
	{0x0306,0x01},
	{0x0307,0x2D},
	{0x030B,0x02},
	{0x030D,0x06},
	{0x030E,0x01},
	{0x030F,0xE3},
	{0x0310,0x01},
		//Other Setting
	{0x3620,0x00},
	{0x3621,0x00},
	{0x3C11,0x04},
	{0x3C12,0x03},
	{0x3C13,0x2D},
	{0x3F0C,0x01},
	{0x3F14,0x00},
	{0x3F80,0x01},
	{0x3F81,0x90},
	{0x3F8C,0x00},
	{0x3F8D,0x14},
	{0x3FF8,0x01},
	{0x3FF9,0x2A},
	{0x3FFE,0x00},
	{0x3FFF,0x6C},
	//Integration Setting
	{0x0202,0x0B},
	{0x0203,0xC4},
	{0x0224,0x01},
	{0x0225,0xF4},
	{0x3FE0,0x01},
	{0x3FE1,0xF4},
		//Gain Setting
	{0x0204,0x00},
	{0x0205,0x70},
	{0x0216,0x00},
	{0x0217,0x70},
	{0x0218,0x01},
	{0x0219,0x00},
	{0x020E,0x01},
	{0x020F,0x00},
	{0x0210,0x01},
	{0x0211,0x00},
	{0x0212,0x01},
	{0x0213,0x00},
	{0x0214,0x01},
	{0x0215,0x00},
	{0x3FE2,0x00},
	{0x3FE3,0x70},
	{0x3FE4,0x01},
	{0x3FE5,0x00},
	//Image Quality adjustment setting
	{0x9852,0x00},
	{0x9954,0x0F},
	{0xA7AD,0x01},
	{0xA7CB,0x01},
	{0xAE09,0xFF},
	{0xAE0A,0xFF},
	{0xAE12,0x58},
	{0xAE13,0x58},
	{0xAE15,0x10},
	{0xAE16,0x10},
	{0xAF05,0x48},
	{0xB07C,0x02},

		//PDAF TYPE1 Setting
	{0x3E20,0x01},
	{0x3E37,0x01},
		//Streaming setting
	{0x0100,0x01},

#endif

};
#endif

/* 4000x3000@30fps master mode , ResID 0 */
const static I2C_ARRAY Sensor_init_table_4lane_12m30fps_master[] =
{
    /**** 30fps ****/
	{0x0136,0x18},
	{0x0137,0x00},
	{0x3c7e,0x01},
	{0x3c7f,0x08},
	{0x0111,0x02},
	{0x380C,0x00},
	{0x3C00,0x10},
	{0x3C01,0x10},
	{0x3C02,0x10},
	{0x3C03,0x10},
	{0x3C04,0x10},
	{0x3C05,0x01},
	{0x3C06,0x00},
	{0x3C07,0x00},
	{0x3C08,0x03},
	{0x3C09,0xFF},
	{0x3C0A,0x01},
	{0x3C0B,0x00},
	{0x3C0C,0x00},
	{0x3C0D,0x03},
	{0x3C0E,0xFF},
	{0x3C0F,0x20},
	{0x3F88,0x00},
	{0x3F8E,0x00},
	{0x5282,0x01},
	{0x9004,0x14},
	{0x9200,0xF4},
	{0x9201,0xA7},
	{0x9202,0xF4},
	{0x9203,0xAA},
	{0x9204,0xF4},
	{0x9205,0xAD},
	{0x9206,0xF4},
	{0x9207,0xB0},
	{0x9208,0xF4},
	{0x9209,0xB3},
	{0x920A,0xB7},
	{0x920B,0x34},
	{0x920C,0xB7},
	{0x920D,0x36},
	{0x920E,0xB7},
	{0x920F,0x37},
	{0x9210,0xB7},
	{0x9211,0x38},
	{0x9212,0xB7},
	{0x9213,0x39},
	{0x9214,0xB7},
	{0x9215,0x3A},
	{0x9216,0xB7},
	{0x9217,0x3C},
	{0x9218,0xB7},
	{0x9219,0x3D},
	{0x921A,0xB7},
	{0x921B,0x3E},
	{0x921C,0xB7},
	{0x921D,0x3F},
	{0x921E,0x77},
	{0x921F,0x77},
	{0x9222,0xC4},
	{0x9223,0x4B},
	{0x9224,0xC4},
	{0x9225,0x4C},
	{0x9226,0xC4},
	{0x9227,0x4D},
	{0x9810,0x14},
	{0x9814,0x14},
	{0x99B2,0x20},
	{0x99B3,0x0F},
	{0x99B4,0x0F},
	{0x99B5,0x0F},
	{0x99B6,0x0F},
	{0x99E4,0x0F},
	{0x99E5,0x0F},
	{0x99E6,0x0F},
	{0x99E7,0x0F},
	{0x99E8,0x0F},
	{0x99E9,0x0F},
	{0x99EA,0x0F},
	{0x99EB,0x0F},
	{0x99EC,0x0F},
	{0x99ED,0x0F},
	{0xA569,0x06},
	{0xA679,0x20},
	{0xC020,0x01},
	{0xC61D,0x00},
	{0xC625,0x00},
	{0xC638,0x03},
	{0xC63B,0x01},
	{0xE286,0x31},
	{0xE2A6,0x32},
	{0xE2C6,0x33},
	{0xBCF1,0x00},
	//MIPI output setting
	{0x0112,0x0A},
	{0x0113,0x0A},
	{0x0114,0x03},
    //Line Length PCK Setting
	{0x0342,0x1E},
	{0x0343,0xC0},
    //Frame Length Lines Setting
	{0x0340,0x0B},
	{0x0341,0xF4},
    //ROI Setting
	{0x0344,0x00},
	{0x0345,0x00},
	{0x0346,0x00},
	{0x0347,0x00},
	{0x0348,0x1F},
	{0x0349,0x3F},
	{0x034A,0x17},
	{0x034B,0x6F},
    //Mode Setting
	{0x0220,0x62},
	{0x0222,0x01},
	{0x0900,0x01},
	{0x0901,0x22},
	{0x0902,0x08},
	{0x3140,0x00},
	{0x3246,0x81},
	{0x3247,0x81},
	{0x3F15,0x00},
    //Digital Crop & Scaling
	{0x0401,0x00},
	{0x0404,0x00},
	{0x0405,0x10},
	{0x0408,0x00},
	{0x0409,0x00},
	{0x040A,0x00},
	{0x040B,0x00},
	{0x040C,0x0F},
	{0x040D,0xA0},
	{0x040E,0x0B},
	{0x040F,0xB8},
    //Output Size Setting
	{0x034C,0x0F},
	{0x034D,0xA0},
	{0x034E,0x0B},
	{0x034F,0xB8},
    //Clock Setting
	{0x0301,0x05},
	{0x0303,0x04},
	{0x0305,0x04},
	{0x0306,0x01},
	{0x0307,0x2D},
	{0x030B,0x02},
	{0x030D,0x06},
	{0x030E,0x01},
	{0x030F,0xE3},
	{0x0310,0x01},
    //Other Setting
	{0x3620,0x00},
	{0x3621,0x00},
	{0x3C11,0x04},
	{0x3C12,0x03},
	{0x3C13,0x2D},
	{0x3F0C,0x01},
	{0x3F14,0x00},
	{0x3F80,0x01},
	{0x3F81,0x90},
	{0x3F8C,0x00},
	{0x3F8D,0x14},
	{0x3FF8,0x01},
	{0x3FF9,0x2A},
	{0x3FFE,0x00},
	{0x3FFF,0x6C},
    //Integration Setting
	{0x0202,0x0B},
	{0x0203,0xC4},
	{0x0224,0x01},
	{0x0225,0xF4},
	{0x3FE0,0x01},
	{0x3FE1,0xF4},
    //Gain Setting
	{0x0204,0x00},
	{0x0205,0x70},
	{0x0216,0x00},
	{0x0217,0x70},
	{0x0218,0x01},
	{0x0219,0x00},
	{0x020E,0x01},
	{0x020F,0x00},
	{0x0210,0x01},
	{0x0211,0x00},
	{0x0212,0x01},
	{0x0213,0x00},
	{0x0214,0x01},
	{0x0215,0x00},
	{0x3FE2,0x00},
	{0x3FE3,0x70},
	{0x3FE4,0x01},
	{0x3FE5,0x00},
    //Image Quality adjustment setting
	{0x9852,0x00},
	{0x9954,0x0F},
	{0xA7AD,0x01},
	{0xA7CB,0x01},
	{0xAE09,0xFF},
	{0xAE0A,0xFF},
	{0xAE12,0x58},
	{0xAE13,0x58},
	{0xAE15,0x10},
	{0xAE16,0x10},
	{0xAF05,0x48},
	{0xB07C,0x02},

    //PDAF TYPE1 Setting
	{0x3E20,0x01},
	{0x3E37,0x01},

    /*master mode setting*/
//    {0x3F70,0x01}, //[MC_MODE] fixed time mode, pre shutter period is defined by PRSH_LENGTH_LINES
    {0x3041,0x01}, //[MASTER_SLAVE_SEL] 1:master, 0:slave
    //0x3F79~0x3F7B //[PRSH_LENGTH_LINES]
    //0x0350 //[FRM_LENGTH_CTL] frame length automatic tracking. default 1
    {0x3040,0x01}, //[XVS_IO_CTRL] 1: XVS PIN output enable, 0:XVS pin output disable
    {0x3F71,0x01}, //[EXTOUT_EN] 1: XVS output enable, 0:XVS output disable
    {0x4B82,0x03}, //[EXTOUT_XVS_WID] pulse width setting of XVS pin signal
#if 0
    {0x4B83,0x01}, //[EXTOUT_XVS_POL] XVS output polarity 0:high active, 1:low active
    {0x4B85,0x01}, //[EXTIN_XVS_POL] XVS input polarity 0:high active, 1:low active
    {0x424A,0x00}, //[MONSEL[10:8]]
    {0x424B,0x00}, //[MONSEL[7:0]]
    {0x423D,0xFF}, //[MON_XVS]
    {0x4BD7,0x16}, //[MNTTEST1_SEL]
    {0x4225,0x00}, //[IO_CTRL_XVS] disabled
#endif

    /*Streaming setting*/
    {0x0100,0x01},
    {0x3130,0x01},
}; //end of master mode


/* 4000x3000@30fps slave mode , ResID 1 */
const static I2C_ARRAY Sensor_init_table_4lane_12m30fps_slave[] =
{
    /**** 30fps ****/
	{0x0136,0x18},
	{0x0137,0x00},
	{0x3c7e,0x01},
	{0x3c7f,0x08},
	{0x0111,0x02},
	{0x380C,0x00},
	{0x3C00,0x10},
	{0x3C01,0x10},
	{0x3C02,0x10},
	{0x3C03,0x10},
	{0x3C04,0x10},
	{0x3C05,0x01},
	{0x3C06,0x00},
	{0x3C07,0x00},
	{0x3C08,0x03},
	{0x3C09,0xFF},
	{0x3C0A,0x01},
	{0x3C0B,0x00},
	{0x3C0C,0x00},
	{0x3C0D,0x03},
	{0x3C0E,0xFF},
	{0x3C0F,0x20},
	{0x3F88,0x00},
	{0x3F8E,0x00},
	{0x5282,0x01},
	{0x9004,0x14},
	{0x9200,0xF4},
	{0x9201,0xA7},
	{0x9202,0xF4},
	{0x9203,0xAA},
	{0x9204,0xF4},
	{0x9205,0xAD},
	{0x9206,0xF4},
	{0x9207,0xB0},
	{0x9208,0xF4},
	{0x9209,0xB3},
	{0x920A,0xB7},
	{0x920B,0x34},
	{0x920C,0xB7},
	{0x920D,0x36},
	{0x920E,0xB7},
	{0x920F,0x37},
	{0x9210,0xB7},
	{0x9211,0x38},
	{0x9212,0xB7},
	{0x9213,0x39},
	{0x9214,0xB7},
	{0x9215,0x3A},
	{0x9216,0xB7},
	{0x9217,0x3C},
	{0x9218,0xB7},
	{0x9219,0x3D},
	{0x921A,0xB7},
	{0x921B,0x3E},
	{0x921C,0xB7},
	{0x921D,0x3F},
	{0x921E,0x77},
	{0x921F,0x77},
	{0x9222,0xC4},
	{0x9223,0x4B},
	{0x9224,0xC4},
	{0x9225,0x4C},
	{0x9226,0xC4},
	{0x9227,0x4D},
	{0x9810,0x14},
	{0x9814,0x14},
	{0x99B2,0x20},
	{0x99B3,0x0F},
	{0x99B4,0x0F},
	{0x99B5,0x0F},
	{0x99B6,0x0F},
	{0x99E4,0x0F},
	{0x99E5,0x0F},
	{0x99E6,0x0F},
	{0x99E7,0x0F},
	{0x99E8,0x0F},
	{0x99E9,0x0F},
	{0x99EA,0x0F},
	{0x99EB,0x0F},
	{0x99EC,0x0F},
	{0x99ED,0x0F},
	{0xA569,0x06},
	{0xA679,0x20},
	{0xC020,0x01},
	{0xC61D,0x00},
	{0xC625,0x00},
	{0xC638,0x03},
	{0xC63B,0x01},
	{0xE286,0x31},
	{0xE2A6,0x32},
	{0xE2C6,0x33},
	{0xBCF1,0x00},
    //MIPI output setting
	{0x0112,0x0A},
	{0x0113,0x0A},
	{0x0114,0x03},
    //Line Length PCK Setting
	{0x0342,0x1E},
	{0x0343,0xC0},
    //Frame Length Lines Setting
	{0x0340,0x0B},
	{0x0341,0xF4},
    //ROI Setting
	{0x0344,0x00},
	{0x0345,0x00},
	{0x0346,0x00},
	{0x0347,0x00},
	{0x0348,0x1F},
	{0x0349,0x3F},
	{0x034A,0x17},
	{0x034B,0x6F},
    //Mode Setting
	{0x0220,0x62},
	{0x0222,0x01},
	{0x0900,0x01},
	{0x0901,0x22},
	{0x0902,0x08},
	{0x3140,0x00},
	{0x3246,0x81},
	{0x3247,0x81},
	{0x3F15,0x00},
    //Digital Crop & Scaling
	{0x0401,0x00},
	{0x0404,0x00},
	{0x0405,0x10},
	{0x0408,0x00},
	{0x0409,0x00},
	{0x040A,0x00},
	{0x040B,0x00},
	{0x040C,0x0F},
	{0x040D,0xA0},
	{0x040E,0x0B},
	{0x040F,0xB8},
    //Output Size Setting
	{0x034C,0x0F},
	{0x034D,0xA0},
	{0x034E,0x0B},
	{0x034F,0xB8},
    //Clock Setting
	{0x0301,0x05},
	{0x0303,0x04},
	{0x0305,0x04},
	{0x0306,0x01},
	{0x0307,0x2D},
	{0x030B,0x02},
	{0x030D,0x06},
	{0x030E,0x01},
	{0x030F,0xE3},
	{0x0310,0x01},
    //Other Setting
	{0x3620,0x00},
	{0x3621,0x00},
	{0x3C11,0x04},
	{0x3C12,0x03},
	{0x3C13,0x2D},
	{0x3F0C,0x01},
	{0x3F14,0x00},
	{0x3F80,0x01},
	{0x3F81,0x90},
	{0x3F8C,0x00},
	{0x3F8D,0x14},
	{0x3FF8,0x01},
	{0x3FF9,0x2A},
	{0x3FFE,0x00},
	{0x3FFF,0x6C},
    //Integration Setting
	{0x0202,0x0B},
	{0x0203,0xC4},
	{0x0224,0x01},
	{0x0225,0xF4},
	{0x3FE0,0x01},
	{0x3FE1,0xF4},
    //Gain Setting
	{0x0204,0x00},
	{0x0205,0x70},
	{0x0216,0x00},
	{0x0217,0x70},
	{0x0218,0x01},
	{0x0219,0x00},
	{0x020E,0x01},
	{0x020F,0x00},
	{0x0210,0x01},
	{0x0211,0x00},
	{0x0212,0x01},
	{0x0213,0x00},
	{0x0214,0x01},
	{0x0215,0x00},
	{0x3FE2,0x00},
	{0x3FE3,0x70},
	{0x3FE4,0x01},
	{0x3FE5,0x00},
    //Image Quality adjustment setting
	{0x9852,0x00},
	{0x9954,0x0F},
	{0xA7AD,0x01},
	{0xA7CB,0x01},
	{0xAE09,0xFF},
	{0xAE0A,0xFF},
	{0xAE12,0x58},
	{0xAE13,0x58},
	{0xAE15,0x10},
	{0xAE16,0x10},
	{0xAF05,0x48},
	{0xB07C,0x02},

    //PDAF TYPE1 Setting
	{0x3E20,0x01},
	{0x3E37,0x01},

    /*slave mode setting*/
//    {0x3F70,0x01}, //[MC_MODE] fixed time mode, pre shutter period is defined by PRSH_LENGTH_LINES
    {0x3041,0x00}, //[MASTER_SLAVE_SEL] 1:master, 0:slave
    {0x3040,0x00}, //[XVS_IO_CTRL] 1: XVS PIN output enable, 0:XVS pin output disable

#if 0
    //0x3F79~0x3F7B //[PRSH_LENGTH_LINES]
    {0x0350,0x01}, //[FRM_LENGTH_CTL] frame length automatic tracking. default 1

//    {0x3F71,0x00}, //[EXTOUT_EN] 1: XVS output enable, 0:XVS output disable
    //{0x4B82,0x03}, //[EXTOUT_XVS_WID] pulse width setting of XVS pin signal
    //{0x4B83,0x01}, //[EXTOUT_XVS_POL] XVS output polarity 0:high active, 1:low active
    //{0x4B85,0x01}, //[EXTIN_XVS_POL] XVS input polarity 0:high active, 1:low active
    //{0x424A,0x00}, //[MONSEL[10:8]]
    //{0x424B,0x00}, //[MONSEL[7:0]]
    //{0x423D,0xFF}, //[MON_XVS]
    //{0x4BD7,0x16}, //[MNTTEST1_SEL]
    //{0x4225,0x00}, //[IO_CTRL_XVS] disabled

#endif

    //Streaming setting
    {0x0100,0x01},
    {0x3130,0x01},
}; //end of master mode

static I2C_ARRAY PatternTbl[] = {
    {0x0000, 0x00}, //colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY expo_reg[] =
{ //SEL
    {0x0202, 0x09},  // bit8-15
    {0x0203, 0xb1},  // bit0-7
};

const static I2C_ARRAY gain_reg[] = {
    {0x0204, 0x03}, // bit0-1(8-10)
    {0x0205, 0x9c}, // bit0-7 low
};

const static I2C_ARRAY vts_reg[] = {
    {0x0340, 0x09},  // bit0-7(8-15)
    {0x0341, 0xc7},  // bit0-7
};

//static int g_sensor_ae_min_gain = 1024;
/*static CUS_GAIN_GAP_ARRAY gain_gap_compensate[16] = {  //compensate  gain gap
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}
};*/

/////////////////////////////////////////////////////////////////
//       @@@@@@@                                               //
//           @@                                                //
//          @@                                                 //
//          @@@                                                //
//       @   @@                                                //
//        @@@@                                                 //
//                                                             //
//      Step 3 --  complete camera features                    //
//                                                             //
//  camera set EV, MWB, orientation, contrast, sharpness       //
//   , saturation, and Denoise can work correctly.             //
//                                                             //
/////////////////////////////////////////////////////////////////
#if 0
static CUS_INT_TASK_ORDER def_order = {
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
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
#if 0
static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

static CUS_MCLK_FREQ UseParaMclk(const char *mclk)
{
    if (strcmp(mclk, "27M") == 0) {
        return CUS_CMU_CLK_27MHZ;
    } else if (strcmp(mclk, "12M") == 0) {
        return CUS_CMU_CLK_12MHZ;
    } else if (strcmp(mclk, "36M") == 0) {
        return CUS_CMU_CLK_36MHZ;
    } else if (strcmp(mclk, "48M") == 0) {
        return CUS_CMU_CLK_48MHZ;
    } else if (strcmp(mclk, "54M") == 0) {
        return CUS_CMU_CLK_54MHZ;
    } else if (strcmp(mclk, "24M") == 0) {
        return CUS_CMU_CLK_24MHZ;
    } else if (strcmp(mclk, "37.125M") == 0) {
        return CUS_CMU_CLK_37P125MHZ;
    }
    return Preview_MCLK_SPEED;
}
#endif
static void pCus_PowerOn_InitChipRX(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }
}
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period * 9;
    info->u32AEShutter_step                  = Preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx586_params *param = (imx586_params*) handle->private_data;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    param->uSnrPad = idx;

    //Sensor power on sequence
    //Sensor board PWDN/RST Pull Low
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);

    //Configuration Chip RX
    pCus_PowerOn_InitChipRX(handle, idx);

    SENSOR_UDELAY(100);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    //Sensor board RST Pull High after PWDN Enable
    SENSOR_UDELAY(1);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_UDELAY(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_MDELAY(1);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    imx586_params *params = (imx586_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    params->res.orit = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// Check Sensor Product ID /////////////////////////
#if 0
static int pCus_CheckSensorProductID(ss_cus_sensor *handle)
{
    u16 sen_id_msb, sen_id_lsb, sen_data;

    /* Read Product ID */
    SensorReg_Read(0x3f12, &sen_id_lsb);
    SensorReg_Read(0x3f13, &sen_id_msb);//CHIP_ID_r3F13
    sen_data = ((sen_id_lsb & 0x0F) << 8) | (sen_id_lsb & 0xF0) | (sen_id_msb & 0x0F);
#if 0
    if (sen_data != CHIP_ID) {
        printk("[***ERROR***]Check Product ID Fail: 0x%x\n", sen_data);
        return FAIL;
    }
#endif
    return SUCCESS;
}

//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
static int pCus_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
    int i,n;
    int table_length= ARRAY_SIZE(Sensor_id_table);
    I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

    for(n=0;n<table_length;++n)
    {
      id_from_sensor[n].reg = Sensor_id_table[n].reg;
      id_from_sensor[n].data = 0;
    }

    *id =0;
    if(table_length>8) table_length=8;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(n=0;n<4;++n) //retry , until I2C success
    {
      if(n>2) return FAIL;

      if(/* SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == */SUCCESS) //read sensor ID from I2C
          break;
      else
          SENSOR_MSLEEP(1);
    }

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
      if( id_from_sensor[i].data != Sensor_id_table[i].data )
        return FAIL;
      *id = id_from_sensor[i].data;
    }

    SENSOR_DMSG("[%s]imx586 sensor ,Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}
#endif

static int imx586_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    int i;
    switch(mode)
    {
    case 1:
        PatternTbl[0].data = 0x21; //enable
        break;
    case 0:
        PatternTbl[0].data &= 0xFE; //disable
        break;
    default:
        PatternTbl[0].data &= 0xFE; //disable
        break;
      }

    for(i=0;i< ARRAY_SIZE(PatternTbl);i++)
    {
        if(SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
        {
            //MSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }
    return SUCCESS;
}

static int pCus_init_mipi4lane_8m30fps_linear(ss_cus_sensor *handle)
{
    int i,cnt=0;
    int nArraySize = 0;
    const I2C_ARRAY *pInitTable = NULL;

    switch(handle->video_res_supported.ulcur_res)
    {
    case 0: /*4000x3000@30fps master mode*/
        pInitTable = Sensor_init_table_4lane_12m30fps_master;
        nArraySize = ARRAY_SIZE(Sensor_init_table_4lane_12m30fps_master);
        break;
    case 1: /*4000x3000@30fps slave mode*/
        pInitTable = Sensor_init_table_4lane_12m30fps_slave;
        nArraySize = ARRAY_SIZE(Sensor_init_table_4lane_12m30fps_slave);
        break;
    default:
        SENSOR_EMSG("IMX586 res id %d out of range.\n", handle->video_res_supported.ulcur_res);
        break;
    }

    for(i=0;i< nArraySize;i++)
    {
        if(pInitTable[i].reg==0xffff)
        {
            SENSOR_MSLEEP(pInitTable[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(pInitTable[i].reg,pInitTable[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    return FAIL;
                }
            }
        }
    }

    return SUCCESS;
}

static int pCus_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int pCus_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int pCus_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    imx586_params *params = (imx586_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: /*4000x3000 30fps master mode*/
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi4lane_8m30fps_linear;
            vts_30fps = 3060;
            Preview_MAX_FPS = imx586_mipi_linear[handle->video_res_supported.ulcur_res].senout.max_fps;;
            Preview_line_period  = 10893;//16236;
            params->bSlaveMode = 0;
            break;
        case 1: /*4000x3000 30fps slave mode*/
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_mipi4lane_8m30fps_linear;
            vts_30fps = 3060;
            Preview_MAX_FPS = imx586_mipi_linear[handle->video_res_supported.ulcur_res].senout.max_fps;;
            Preview_line_period  = 10893;//16236;
            params->bSlaveMode = 1;
            break;
    }
    params->expo.vts = vts_30fps;
    params->expo.fps = Preview_MAX_FPS;

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    s16 sen_data;
    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP_REG, &sen_data);

    switch(sen_data)
    {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
        break;
        case 0x01:
            *orit = CUS_ORIT_M1F0;
        break;
        case 0x02:
            *orit = CUS_ORIT_M0F1;
        break;
        case 0x03:
            *orit = CUS_ORIT_M1F1;
        break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx586_params *params = (imx586_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    s16 sen_data;
    imx586_params *params = (imx586_params *)handle->private_data;
    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP_REG, (void*)&sen_data);
    sen_data &= ~(MIRROR_EN | FLIP_EN);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            //sen_data |= SENSOR_NOR;
            params->res.orit = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M1F0:
            sen_data |= MIRROR_EN;
            params->res.orit = CUS_ORIT_M1F0;
            break;
        case CUS_ORIT_M0F1:
            sen_data |= FLIP_EN;
            params->res.orit = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            sen_data |= MIRROR_EN;
            sen_data |= FLIP_EN;
            params->res.orit = CUS_ORIT_M1F1;
            break;
        default :
            params->res.orit = CUS_ORIT_M0F0;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(MIRROR_FLIP_REG, sen_data);

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx586_params *params = (imx586_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts = 0;
    imx586_params *params = (imx586_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);
    }else if((fps>=(min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts = (vts_30fps*(max_fps*1000) + (fps>>1))/fps;
    }else{
        //params->expo.vts=vts_30fps;
        //params->expo.fps=25;
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.expo_lines > params->expo.vts -2){
        vts = params->expo.expo_lines + 8;
    }else{
        vts = params->expo.vts;
    }
    params->expo.vts = vts;
    pCus_SetAEUSecs(handle, params->expo.expo_lef_us);

    params->dirty = true;
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx586_params *params = (imx586_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             //SensorReg_Write(0x0104,1);
             break;
        case CUS_FRAME_ACTIVE:

            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x0104,1);
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));

                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x0104,0);
                params->dirty = false;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    u32 lines = 0;
    imx586_params *params = (imx586_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;
    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0,activeline = 0;
    imx586_params *params = (imx586_params *)handle->private_data;

    params->expo.expo_lef_us = us;

    lines = (1000 * us) / Preview_line_period;
    if(lines < 9) lines = 9;
    params->expo.expo_lines = lines;

    if (lines >params->expo.vts-1)
        vts = lines +1;
    else
        vts = params->expo.vts;

    SENSOR_DMSG("[%s] us %u, lines %u, vts %u\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    activeline = lines;
    if(activeline < 9) activeline = 9;

    params->tExpo_reg[0].data = (activeline>>8) & 0xff;
    params->tExpo_reg[1].data = (activeline>>0) & 0xff;

    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    //int rc = SensorRegArrayR((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
    unsigned short temp_gain;
  //  *gain=params->expo.final_gain;
    temp_gain = gain_reg[0].data << 8 | gain_reg[1].data << 0 ;

    *gain=(u32)(10^((temp_gain*3)/200))*1024;
    //if (gain_reg[1].data & 0x10)
    //   *gain = (*gain) * 2;

    SENSOR_DMSG("[%s] get gain/reg (1024=1X)= %u/0x%x\n", __FUNCTION__, *gain,gain_reg[0].data);
    //return rc;
    return SUCCESS;
}



static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    //extern DBG_ITEM Dbg_Items[DBG_TAG_MAX];
    imx586_params *params = (imx586_params *)handle->private_data;

    u32 gain_cal;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
	{
        gain = SENSOR_MIN_GAIN;
	}
    else if(gain >= SENSOR_MAX_GAIN)
	{
        gain = SENSOR_MAX_GAIN;
	}


	gain_cal = (u32)(1024 - ((1024*1024)/gain));

    params->tGain_reg[1].data = gain_cal & 0xff;
    params->tGain_reg[0].data = (gain_cal >> 8) & 0xff;

	//printk("\ngain[%d] gain_cal[%d] gain0[0x%x]  gain1[0x%x]\n",gain,gain_cal,params->tGain_reg[0].data,params->tGain_reg[1].data);
    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[1].data);
    params->dirty = true;
    return SUCCESS;
}

#if 0
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/imx586_mipi_linear[0].senout.min_fps;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = SENSOR_MIN_GAIN;//handle->sat_mingain;
    *max = SENSOR_MAX_GAIN;//3980*1024;
    return SUCCESS;
}

static int imx586_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/imx586_mipi_linear[0].senout.min_fps;
    info->min = (Preview_line_period * 9);
    info->step = Preview_line_period;
    return SUCCESS;
}

/*static int pCus_setCaliData_gain_linearity(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num) {
    u32 i, j;

    for(i=0,j=0;i< num ;i++,j+=2){
        gain_gap_compensate[i].gain=pArray[i].gain;
        gain_gap_compensate[i].offset=pArray[i].offset;
    }
    return SUCCESS;
}*/
#endif
int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx586_params *params;
    int res;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    ////////////////////////////////////
    // private data allocation & init //
    ////////////////////////////////////
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }

    params = (imx586_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"imx586_MIPI_Linear");

    ////////////////////////////////////
    //    i2c config                  //
    ////////////////////////////////////
    handle->i2c_cfg.mode          = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt           = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D8;
    handle->i2c_cfg.address       = SENSOR_I2C_ADDR;      //0x34;
    handle->i2c_cfg.speed         = SENSOR_I2C_SPEED;     //300000;

    ////////////////////////////////////
    //    mclk                        //
    ////////////////////////////////////
    handle->mclk                  = Preview_MCLK_SPEED;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //handle->isp_type              = SENSOR_ISP_TYPE;
    //handle->data_fmt              = SENSOR_DATAFMT;
    handle->sif_bus               = SENSOR_IFBUS_TYPE;
    handle->data_prec             = SENSOR_DATAPREC;
    //handle->data_mode             = SENSOR_DATAMODE;
    handle->bayer_id              = SENSOR_BAYERID;
    handle->RGBIR_id              = SENSOR_RGBIRID;
    params->res.orit                = SENSOR_ORIT;
    //handle->YC_ODER               = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num    = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order   = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode  = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode    = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
   //handle->video_res_supported.num_res = LINEAR_RES_END;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx586_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx586_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx586_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx586_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx586_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx586_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx586_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx586_mipi_linear[res].senout.height;
        //handle->video_res_supported.res[res].nMinFrameLengthLine = imx586_mipi_linear[res].nMinFrameLengthLine;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx586_mipi_linear[res].senstr.strResDesc);
    }
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period * 9);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx586_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ////////////////////////////////////
    //    Sensor polarity             //
    ////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    //handle->pCus_sensor_release         = cus_camsensor_release_handle;
    handle->pCus_sensor_init            = pCus_init_mipi4lane_8m30fps_linear;
    //handle->pCus_sensor_powerupseq      = pCus_powerupseq;
    handle->pCus_sensor_poweron         = pCus_poweron;
    handle->pCus_sensor_poweroff        = pCus_poweroff;
    //handle->pCus_sensor_GetSensorID     = pCus_GetSensorID;
    handle->pCus_sensor_GetVideoResNum  = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes     = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes     = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode  = imx586_SetPatternMode;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    //handle->ae_gain_delay              = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay           = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    //handle->ae_gain_ctrl_num           = 1;
    //handle->ae_shutter_ctrl_num        = 1;
    //handle->sat_mingain                = SENSOR_MIN_GAIN;  //calibration
    //handle->dgain_remainder = 0;
    //nPixelSize
   // handle->nPixelSize                  = 1450;
    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    //handle->pCus_sensor_GetDGainRemainder = pCus_GetDGainRemainder;

    //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    //handle->pCus_sensor_GetShutterInfo = imx586_GetShutterInfo;

    params->expo.vts        = vts_30fps;
    params->expo.expo_lines = 5000;
    params->dirty           = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(imx586_HDR,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            imx586_params
                         );
