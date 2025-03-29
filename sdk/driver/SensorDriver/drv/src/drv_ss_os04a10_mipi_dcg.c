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
   Porting owner : ming-en.lu
   Date          : 2023/09/19
   Build on      : Master_V4 I6C
   Verified on : mixer preview linear ok , but AWB fail,
                    hdr fail, I6f can't work neither , P5 has a new version code but not complete. so M4 can't use this driver.
   Remark        : NA
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OS04A10);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

//#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
//#define SENSOR_CHANNEL_NUM (0)
//#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
//#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
//#define SENSOR_CHANNEL_NUM (0)
//#define SENSOR_CHANNEL_MODE CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
//#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
#define SENSOR_MIPI_LANE_NUM_HDR (2)
//#define SENSOR_MIPI_HDR_MODE (2) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

//#define R_GAIN_REG 1
//#define G_GAIN_REG 2
//#define B_GAIN_REG 3


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
//                                                                                                    ��//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

//#define SENSOR_ISP_TYPE     ISP_EXT                   //ISP_EXT, ISP_SOC
#define F_number  22                                  // CFG, demo module
//#define SENSOR_DATAFMT      CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC     CUS_DATAPRECISION_12    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR     CUS_DATAPRECISION_12
#define SENSOR_DATAPREC_DCG_HDR     CUS_DATAPRECISION_16
//#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR      CUS_BAYER_BG//CUS_BAYER_GR
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
/*
CUS_RGBIR_R0 = 1,
CUS_RGBIR_G0 = 2,
CUS_RGBIR_B0 = 3,
CUS_RGBIR_G1 = 4,
CUS_RGBIR_G2 = 5, ?
CUS_RGBIR_I0 = 6,
CUS_RGBIR_G3 = 7,
CUS_RGBIR_I1 = 8

*/
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_MAX_GAIN     248                 // max sensor again, a-gain

#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
//#define Preview_line_period 25960//30580                  //(36M/37.125M)*30fps=29.091fps(34.375msec), hts=34.375/1125=30556,
#define Preview_line_period_HDR 14723//26666//30580    //33333/2264=14723
//#define vts_30fps  1284//1266//1150//1090                              //for 29.091fps @ MCLK=36MHz
#define vts_30fps_HDR  2264                            //for 30fps @ MCLK=24MHz
u32 Preview_line_period = 26667;
u32 vts_30fps = 1144;
////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (248 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL       (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)

#define Preview_WIDTH       1920//2688                    //resolution Width when preview
#define Preview_HEIGHT      1080//1520                    //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MAX_FPS_HDR 30                     //fastest preview FPS
#define Preview_MIN_FPS     5                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR    0x6c                   //I2C slave address
#define SENSOR_I2C_SPEED   240000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

#define hcg_hcg 1

//static u32 HDR_LEF_exp=0;

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
CUS_MCLK_FREQ UseParaMclk(void);

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
        u32 sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 final_sef_gain;
        u32 final_vs_gain;
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
        u32 max_short;
        u32 line;
    } expo;
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    u32 skip_cnt;
    bool dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tGain_reg_HDR_SEF[5];
    I2C_ARRAY tGain_reg_HDR_VS[5];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tExpo_reg_HDR_VS[2];
    I2C_ARRAY tMirror_reg[2];
    I2C_ARRAY tMirror_reg_HDR[2];
    bool orien_dirty;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
} OS04A10_params;
// set sensor ID address and data,


static struct {  //LINEAR
    enum { LINEAR_RES_1 = 0, LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _senif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    // Show resolution string
    struct _senstr{
        const char* strResDesc;
    }senstr;
} OS04A10_mipi_linear[] = {
    //{LINEAR_RES_1, {2688, 1520, 3, 30}, {0, 0, 2688, 1520}, {"2688x1520@30fps"}},
    {LINEAR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},
};


static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_2, /*HDR_RES_3,*/ HDR_RES_END }mode;
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
}OS04A10_mipi_hdr[] = {
   {HDR_RES_1, {2688, 1520, 3, 25}, {0, 0, 2688, 1520}, {"2688x1520@25fps_DCG_16b"}},
   {HDR_RES_2, {1920, 1080, 3, 20}, {0, 0, 1920, 1080}, {"1920x1080@20fps_DCG_12b"}},
//   {HDR_RES_3, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_DCG_VS"}},
};


/* typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;
 */
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x300a, 0x58},      // {address of ID, ID },
    {0x300b, 0x03},      // {address of ID, ID },
    {0x300c, 0x41},      // {address of ID, ID },
    // max 8 sets in this table
};
const static I2C_ARRAY mirror_reg[] =
{
    {0x3820, 0x02},//[2]Flip [1]mirror
};


const static I2C_ARRAY Sensor_init_table[] =
{
//    Sensor revision: OS04A10
//    Input clock frequency:24MHZ
//    Image output size: 1920x1080
//    Image crop size: No special request
//    Frame timing and frame rate: 30fps
//    System clock frequency:108Mhz
//    Output interface and data rate: MIPI 2lane

    {0x0103, 0x01},
    {0x0109, 0x01},
    {0x0104, 0x02},
    {0x0102, 0x00},
    {0x0305, 0x3c},
    {0x0306, 0x00},
    {0x0307, 0x00},
    {0x0308, 0x04},
    {0x030a, 0x01},
    {0x0317, 0x09},
    {0x0322, 0x01},
    {0x0323, 0x02},
    {0x0324, 0x00},
    {0x0325, 0x90},
    {0x0327, 0x05},
    {0x0329, 0x02},
    {0x032c, 0x02},
    {0x032d, 0x02},
    {0x032e, 0x02},
    {0x300f, 0x11},
    {0x3012, 0x21},
    {0x3026, 0x10},
    {0x3027, 0x08},
    {0x302d, 0x24},
    {0x3104, 0x01},
    {0x3106, 0x11},
    {0x3400, 0x00},
    {0x3408, 0x05},
    {0x340c, 0x0c},
    {0x340d, 0xb0},
    {0x3425, 0x51},
    {0x3426, 0x10},
    {0x3427, 0x14},
    {0x3428, 0x10},
    {0x3429, 0x10},
    {0x342a, 0x10},
    {0x342b, 0x04},
    {0x3501, 0x02},
    {0x3504, 0x08},
    {0x3508, 0x01},
    {0x3509, 0x00},
    {0x350a, 0x01},
    {0x3544, 0x08},
    {0x3548, 0x01},
    {0x3549, 0x00},
    {0x3584, 0x08},
    {0x3588, 0x01},
    {0x3589, 0x00},
    {0x3601, 0x70},
    {0x3604, 0xe3},
    {0x3605, 0x7f},
    {0x3606, 0x80},
    {0x3608, 0xa8},
    {0x360a, 0xd0},
    {0x360b, 0x08},
    {0x360e, 0xc8},
    {0x360f, 0x66},
    {0x3610, 0x89},
    {0x3611, 0x8a},
    {0x3612, 0x4e},
    {0x3613, 0xbd},
    {0x3614, 0x9b},
    {0x362a, 0x0e},
    {0x362b, 0x0e},
    {0x362c, 0x0e},
    {0x362d, 0x0e},
    {0x362e, 0x1a},
    {0x362f, 0x34},
    {0x3630, 0x67},
    {0x3631, 0x7f},
    {0x3638, 0x00},
    {0x3643, 0x00},
    {0x3644, 0x00},
    {0x3645, 0x00},
    {0x3646, 0x00},
    {0x3647, 0x00},
    {0x3648, 0x00},
    {0x3649, 0x00},
    {0x364a, 0x04},
    {0x364c, 0x0e},
    {0x364d, 0x0e},
    {0x364e, 0x0e},
    {0x364f, 0x0e},
    {0x3650, 0xff},
    {0x3651, 0xff},
    {0x365a, 0x00},
    {0x365b, 0x00},
    {0x365c, 0x00},
    {0x365d, 0x00},
    {0x3661, 0x07},
    {0x3662, 0x02},
    {0x3663, 0x20},
    {0x3665, 0x12},
    {0x3667, 0xd4},
    {0x3668, 0x80},
    {0x366c, 0x00},
    {0x366d, 0x00},
    {0x366e, 0x00},
    {0x366f, 0x00},
    {0x3671, 0x08},
    {0x3673, 0x2a},
    {0x3681, 0x80},
    {0x3700, 0x2d},
    {0x3701, 0x22},
    {0x3702, 0x25},
    {0x3703, 0x20},
    {0x3705, 0x00},
    {0x3706, 0x72},
    {0x3707, 0x0a},
    {0x3708, 0x36},
    {0x3709, 0x57},
    {0x370a, 0x01},
    {0x370b, 0x14},
    {0x3714, 0x01},
    {0x3719, 0x1f},
    {0x371b, 0x16},
    {0x371c, 0x00},
    {0x371d, 0x08},
    {0x373f, 0x63},
    {0x3740, 0x63},
    {0x3741, 0x63},
    {0x3742, 0x63},
    {0x3756, 0x9d},
    {0x3757, 0x9d},
    {0x3762, 0x1c},
    {0x376c, 0x04},
    {0x3776, 0x05},
    {0x3777, 0x22},
    {0x3779, 0x60},
    {0x377c, 0x48},
    {0x3784, 0x06},
    {0x3785, 0x0a},
    {0x3790, 0x10},
    {0x3793, 0x04},
    {0x3794, 0x07},
    {0x3796, 0x00},
    {0x3797, 0x02},
    {0x379c, 0x4d},
    {0x37a1, 0x80},
    {0x37bb, 0x88},
    {0x37be, 0x48},
    {0x37bf, 0x01},
    {0x37c0, 0x01},
    {0x37c4, 0x72},
    {0x37c5, 0x72},
    {0x37c6, 0x72},
    {0x37ca, 0x21},
    {0x37cc, 0x13},
    {0x37cd, 0x90},
    {0x37cf, 0x02},
    {0x37d0, 0x00},
    {0x37d1, 0x72},
    {0x37d2, 0x01},
    {0x37d3, 0x14},
    {0x37d4, 0x00},
    {0x37d5, 0x6c},
    {0x37d6, 0x00},
    {0x37d7, 0xf7},
    {0x37d8, 0x01},
    {0x37dc, 0x00},
    {0x37dd, 0x00},
    {0x37da, 0x00},
    {0x37db, 0x00},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x8f},
    {0x3806, 0x05},
    {0x3807, 0xff},
    {0x3808, 0x0a},
    {0x3809, 0x80},
    {0x380a, 0x05},
    {0x380b, 0xf0},
    {0x380c, 0x05},
    {0x380d, 0xb8},
    {0x380e, 0x06},
    {0x380f, 0x66},
    {0x3811, 0x08},
    {0x3813, 0x08},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3816, 0x01},
    {0x3817, 0x01},
    {0x381c, 0x00},
    {0x3820, 0x02},
    {0x3821, 0x00},
    {0x3822, 0x14},
    {0x3823, 0x18},
    {0x3826, 0x00},
    {0x3827, 0x00},
    {0x3833, 0x40},
    {0x384c, 0x02},
    {0x384d, 0xdc},
    {0x3858, 0x3c},
    {0x3865, 0x02},
    {0x3866, 0x00},
    {0x3867, 0x00},
    {0x3868, 0x02},
    {0x3900, 0x13},
    {0x3940, 0x13},
    {0x3980, 0x13},
    {0x3c01, 0x11},
    {0x3c05, 0x00},
    {0x3c0f, 0x1c},
    {0x3c12, 0x0d},
    {0x3c19, 0x00},
    {0x3c21, 0x00},
    {0x3c3a, 0x10},
    {0x3c3b, 0x18},
    {0x3c3d, 0xc6},
    {0x3c55, 0xcb},
    {0x3c5a, 0x55},
    {0x3c5d, 0xcf},
    {0x3c5e, 0xcf},
    {0x3d8c, 0x70},
    {0x3d8d, 0x10},
    {0x4000, 0xf9},
    {0x4001, 0x2f},
    {0x4004, 0x00},
    {0x4005, 0x40},
    {0x4008, 0x02},
    {0x4009, 0x11},
    {0x400a, 0x06},
    {0x400b, 0x40},
    {0x400e, 0x40},
    {0x402e, 0x00},
    {0x402f, 0x40},
    {0x4030, 0x00},
    {0x4031, 0x40},
    {0x4032, 0x0f},
    {0x4033, 0x80},
    {0x4050, 0x00},
    {0x4051, 0x07},
    {0x4011, 0xbb},
    {0x410f, 0x01},
    {0x4288, 0xcf},
    {0x4289, 0x00},
    {0x428a, 0x46},
    {0x430b, 0x0f},
    {0x430c, 0xfc},
    {0x430d, 0x00},
    {0x430e, 0x00},
    {0x4314, 0x04},
    {0x4500, 0x18},
    {0x4501, 0x18},
    {0x4503, 0x10},
    {0x4504, 0x00},
    {0x4506, 0x32},
    {0x4507, 0x02},
    {0x4601, 0x30},
    {0x4603, 0x00},
    {0x460a, 0x50},
    {0x460c, 0x60},
    {0x4640, 0x62},
    {0x4646, 0xaa},
    {0x4647, 0x55},
    {0x4648, 0x99},
    {0x4649, 0x66},
    {0x464d, 0x00},
    {0x4654, 0x11},
    {0x4655, 0x22},
    {0x4800, 0x44},
    {0x480e, 0x00},
    {0x4810, 0xff},
    {0x4811, 0xff},
    {0x4813, 0x00},
    {0x481f, 0x30},
    {0x4837, 0x0e},
    {0x484b, 0x27},
    {0x4d00, 0x4d},
    {0x4d01, 0x9d},
    {0x4d02, 0xb9},
    {0x4d03, 0x2e},
    {0x4d04, 0x4a},
    {0x4d05, 0x3d},
    {0x4d09, 0x4f},
    {0x5000, 0x1f},
    {0x5001, 0x0d},
    {0x5080, 0x00},
    {0x50c0, 0x00},
    {0x5100, 0x00},
    {0x5200, 0x00},
    {0x5201, 0x00},
    {0x5202, 0x03},
    {0x5203, 0xff},
    {0x5780, 0x53},
    {0x5782, 0x18},
    {0x5783, 0x3c},
    {0x5786, 0x01},
    {0x5788, 0x18},
    {0x5789, 0x3c},
    {0x5792, 0x11},
    {0x5793, 0x33},
    {0x5857, 0xff},
    {0x5858, 0xff},
    {0x5859, 0xff},
    {0x58d7, 0xff},
    {0x58d8, 0xff},
    {0x58d9, 0xff},
    {0x0100, 0x01},
    {0x0100, 0x01},


};

const static I2C_ARRAY Sensor_init_table_DCG_VS_HDR[] =
{
    //Sensor revision: X3A R1E
    //Input clock frequency: 24MHz
    //Image output size: 1920x1080
    //Image crop size: No special request
    //Pixel data format: DCG12+VS12
    //Frame timing and frame rate: 60fps
    //System clock frequency: No special request
    //Output interface and data rate: MIPI1080, 4lane
    //Others:
    //Core Setting version info: 0406

    {0x0103 , 0x01},
    {0x4d07 , 0x21},
    {0x4d0e , 0x80},
    {0x4d11 , 0x7d},
    {0x0303 , 0x02},
    {0x0305 , 0x5a},
    {0x0306 , 0x03},
    {0x0307 , 0x00},
    {0x0316 , 0x00},
    {0x0317 , 0x42},
    {0x0323 , 0x02},
    {0x0325 , 0x6c},
    {0x0326 , 0x00},
    {0x032b , 0x00},
    {0x0400 , 0xe7},
    {0x0401 , 0xff},
    {0x0404 , 0x2b},
    {0x0405 , 0x32},
    {0x0406 , 0x33},
    {0x0407 , 0x8f},
    {0x0408 , 0x0c},
    {0x0410 , 0xe7},
    {0x0411 , 0xff},
    {0x0414 , 0x2b},
    {0x0415 , 0x32},
    {0x0416 , 0x33},
    {0x0417 , 0x8f},
    {0x0418 , 0x0c},
    {0x3002 , 0x03},
    {0x3012 , 0x41},
    {0x301e , 0xb0},
    {0x3706 , 0x39},
    {0x370a , 0x00},
    {0x370b , 0xa3},
    {0x370f , 0x40},
    {0x3711 , 0x22},
    {0x3712 , 0x12},
    {0x3713 , 0x00},
    {0x3718 , 0x04},
    {0x3719 , 0x32},
    {0x371a , 0x06},
    {0x371b , 0x14},
    {0x372c , 0x17},
    {0x3733 , 0x41},
    {0x3741 , 0x44},
    {0x3742 , 0x34},
    {0x3746 , 0x03},
    {0x374b , 0x03},
    {0x3755 , 0x09},
    {0x376c , 0x15},
    {0x376d , 0x08},
    {0x376f , 0x08},
    {0x3770 , 0x91},
    {0x3771 , 0x08},
    {0x3774 , 0x8a},
    {0x3777 , 0x00},
    {0x3779 , 0x22},
    {0x377a , 0x00},
    {0x377b , 0x00},
    {0x377c , 0x48},
    {0x3785 , 0x08},
    {0x3790 , 0x10},
    {0x3793 , 0x00},
    {0x379c , 0x01},
    {0x37a1 , 0x80},
    {0x37b3 , 0x0a},
    {0x37be , 0xe0},
    {0x37bf , 0x00},
    {0x37c6 , 0x48},
    {0x37c7 , 0x38},
    {0x37c9 , 0x00},
    {0x37ca , 0x39},
    {0x37cb , 0x00},
    {0x37cc , 0xa3},
    {0x37d1 , 0x39},
    {0x37d2 , 0x00},
    {0x37d3 , 0xa3},
    {0x37d5 , 0x39},
    {0x37d6 , 0x00},
    {0x37d7 , 0xa3},
    {0x3c06 , 0x29},
    {0x3c0b , 0xa8},
    {0x3c53 , 0x68},
    {0x3192 , 0x00},
    {0x3193 , 0x00},
    {0x3206 , 0xc0},
    {0x3216 , 0x01},
    {0x3400 , 0x08},
    {0x3409 , 0x02},
    {0x3501 , 0x00},
    {0x3502 , 0x24},
    {0x3508 , 0x01},
    {0x3509 , 0x00},
    {0x350a , 0x01},
    {0x350b , 0x00},
    {0x350c , 0x00},
    {0x3548 , 0x01},
    {0x3549 , 0x00},
    {0x354a , 0x01},
    {0x354b , 0x00},
    {0x354c , 0x00},
    {0x3581 , 0x00},
    {0x3582 , 0x24},
    {0x3588 , 0x01},
    {0x3589 , 0x00},
    {0x358a , 0x01},
    {0x358b , 0x00},
    {0x358c , 0x00},
    {0x3600 , 0x00},
    {0x3602 , 0x42},
    {0x3603 , 0xf3},
    {0x3604 , 0x93},
    {0x3605 , 0xff},
    {0x3606 , 0xc0},
    {0x3607 , 0x4a},
    {0x360a , 0xd0},
    {0x360b , 0x0b},
    {0x360e , 0x88},
    {0x3611 , 0x4b},
    {0x3612 , 0x4e},
    {0x3614 , 0x8a},
    {0x3615 , 0x98},
    {0x3619 , 0x00},
    {0x3620 , 0x02},
    {0x3626 , 0x0e},
    {0x362c , 0x0e},
    {0x362d , 0x12},
    {0x362e , 0x0b},
    {0x362f , 0x18},
    {0x3630 , 0x30},
    {0x3631 , 0x57},
    {0x3632 , 0x99},
    {0x3633 , 0x99},
    {0x3643 , 0x0c},
    {0x3644 , 0x00},
    {0x3645 , 0x0e},
    {0x3646 , 0x0f},
    {0x3647 , 0x0e},
    {0x3648 , 0x00},
    {0x3649 , 0x11},
    {0x364a , 0x12},
    {0x364c , 0x0e},
    {0x364d , 0x0e},
    {0x364e , 0x12},
    {0x364f , 0x0e},
    {0x3652 , 0xc5},
    {0x3657 , 0x88},
    {0x3658 , 0x08},
    {0x365a , 0x57},
    {0x365b , 0x30},
    {0x365c , 0x18},
    {0x365d , 0x0b},
    {0x3660 , 0x01},
    {0x3661 , 0x07},
    {0x3662 , 0x00},
    {0x3665 , 0x92},
    {0x3666 , 0x13},
    {0x3667 , 0x2c},
    {0x3668 , 0x95},
    {0x3669 , 0x2c},
    {0x366f , 0xc4},
    {0x3671 , 0x27},
    {0x3673 , 0x6a},
    {0x3678 , 0x88},
    {0x3800 , 0x00},
    {0x3801 , 0x00},
    {0x3802 , 0x00},
    {0x3803 , 0x68},
    {0x3804 , 0x07},
    {0x3805 , 0x8f},
    {0x3806 , 0x04},
    {0x3807 , 0xa7},
    {0x3808 , 0x07},
    {0x3809 , 0x80},
    {0x380a , 0x04},
    {0x380b , 0x38},
    {0x380c , 0x02},
    {0x380d , 0x14},
    {0x380e , 0x04},
    {0x380f , 0x78},
    {0x3810 , 0x00},
    {0x3811 , 0x08},
    {0x3813 , 0x04},
    {0x381c , 0x08},
    {0x3820 , 0x00},
    {0x3821 , 0x20},
    {0x3822 , 0x14},
    {0x3832 , 0x10},
    {0x3833 , 0x01},
    {0x3834 , 0xf0},
    {0x383d , 0x20},
    {0x384c , 0x02},
    {0x384d , 0x14},
    {0x384e , 0x00},
    {0x384f , 0x40},
    {0x3850 , 0x00},
    {0x3851 , 0x42},
    {0x3852 , 0x00},
    {0x3853 , 0x40},
    {0x3854 , 0x00},
    {0x3855 , 0x05},
    {0x3856 , 0x04},
    {0x3857 , 0x6b},
    {0x3858 , 0x3c},
    {0x3859 , 0x00},
    {0x385a , 0x03},
    {0x385b , 0x04},
    {0x385c , 0x6a},
    {0x385f , 0x00},
    {0x3860 , 0x10},
    {0x3861 , 0x00},
    {0x3862 , 0x40},
    {0x3863 , 0x00},
    {0x3864 , 0x40},
    {0x3865 , 0x00},
    {0x3866 , 0x40},
    {0x3b40 , 0x3e},
    {0x3b41 , 0x00},
    {0x3b42 , 0x02},
    {0x3b43 , 0x00},
    {0x3b44 , 0x00},
    {0x3b45 , 0x20},
    {0x3b46 , 0x00},
    {0x3b47 , 0x20},
    {0x3b84 , 0x05},
    {0x3b85 , 0x00},
    {0x3b86 , 0x00},
    {0x3b87 , 0x10},
    {0x3b88 , 0x00},
    {0x3b89 , 0x10},
    {0x3b8a , 0x00},
    {0x3b8b , 0x08},
    {0x3b8e , 0x03},
    {0x3b8f , 0xe8},
    {0x3d85 , 0x0b},
    {0x3d8c , 0x70},
    {0x3d8d , 0x26},
    {0x3d97 , 0x70},
    {0x3d98 , 0x24},
    {0x3d99 , 0x70},
    {0x3d9a , 0x6d},
    {0x3d9b , 0x70},
    {0x3d9c , 0x6e},
    {0x3d9d , 0x73},
    {0x3d9e , 0xff},
    {0x3f00 , 0x04},
    {0x4001 , 0x2b},
    {0x4004 , 0x00},
    {0x4005 , 0x80},
    {0x4008 , 0x02},
    {0x4009 , 0x0d},
    {0x400a , 0x08},
    {0x400b , 0x00},
    {0x400f , 0x80},
    {0x4010 , 0x10},
    {0x4011 , 0xbb},
    {0x4016 , 0x00},
    {0x4017 , 0x10},
    {0x402e , 0x00},
    {0x402f , 0x80},
    {0x4030 , 0x00},
    {0x4031 , 0x80},
    {0x4032 , 0x9f},
    {0x4033 , 0x00},
    {0x4308 , 0x00},
    {0x4502 , 0x00},
    {0x4507 , 0x16},
    {0x4580 , 0xf8},
    {0x4602 , 0x02},
    {0x4603 , 0x00},
    {0x460a , 0x36},
    {0x460c , 0x60},
    {0x4800 , 0x04},
    {0x480e , 0x04},
    {0x4813 , 0x12},
    {0x4815 , 0x2b},
    {0x4837 , 0x0e},
    {0x484b , 0x27},
    {0x484c , 0x02},
    {0x4886 , 0x00},
    {0x4903 , 0x80},
    {0x4f00 , 0xff},
    {0x4f01 , 0xff},
    {0x4f05 , 0x01},
    {0x5180 , 0x04},
    {0x5181 , 0x00},
    {0x5182 , 0x04},
    {0x5183 , 0x00},
    {0x5184 , 0x04},
    {0x5185 , 0x00},
    {0x5186 , 0x04},
    {0x5187 , 0x00},
    {0x51a0 , 0x04},
    {0x51a1 , 0x00},
    {0x51a2 , 0x04},
    {0x51a3 , 0x00},
    {0x51a4 , 0x04},
    {0x51a5 , 0x00},
    {0x51a6 , 0x04},
    {0x51a7 , 0x00},
    {0x51c0 , 0x04},
    {0x51c1 , 0x00},
    {0x51c2 , 0x04},
    {0x51c3 , 0x00},
    {0x51c4 , 0x04},
    {0x51c5 , 0x00},
    {0x51c6 , 0x04},
    {0x51c7 , 0x00},
    {0x5380 , 0x19},
    {0x5382 , 0x2e},
    {0x53a0 , 0x41},
    {0x53a2 , 0x04},
    {0x53a3 , 0x00},
    {0x53a4 , 0x04},
    {0x53a5 , 0x00},
    {0x53a7 , 0x00},
    {0x5400 , 0x19},
    {0x5402 , 0x2e},
    {0x5420 , 0x41},
    {0x5422 , 0x04},
    {0x5423 , 0x00},
    {0x5424 , 0x04},
    {0x5425 , 0x00},
    {0x5427 , 0x00},
    {0x5480 , 0x19},
    {0x5482 , 0x2e},
    {0x54a0 , 0x41},
    {0x54a2 , 0x04},
    {0x54a3 , 0x00},
    {0x54a4 , 0x04},
    {0x54a5 , 0x00},
    {0x54a7 , 0x00},
    {0x5800 , 0x38},
    {0x5801 , 0x03},
    {0x5802 , 0xc0},
    {0x5804 , 0x00},
    {0x5805 , 0x80},
    {0x5806 , 0x01},
    {0x5807 , 0x00},
    {0x580e , 0x10},
    {0x5812 , 0x34},
    {0x5000 , 0x89},
    {0x5001 , 0x40},//DPC disable
    {0x5002 , 0x39},
    {0x5003 , 0x16},
    {0x5004 , 0x00},
    {0x5005 , 0x40},
    {0x5006 , 0x00},
    {0x5007 , 0x40},
    {0x503e , 0x00},
    {0x503f , 0x00},
    {0x5602 , 0x02},
    {0x5603 , 0x58},
    {0x5604 , 0x03},
    {0x5605 , 0x20},
    {0x5606 , 0x02},
    {0x5607 , 0x58},
    {0x5608 , 0x03},
    {0x5609 , 0x20},
    {0x560a , 0x02},
    {0x560b , 0x58},
    {0x560c , 0x03},
    {0x560d , 0x20},
    {0x560e , 0x02},
    {0x560f , 0x58},
    {0x5610 , 0x03},
    {0x5611 , 0x20},
    {0x5612 , 0x02},
    {0x5613 , 0x58},
    {0x5614 , 0x03},
    {0x5615 , 0x20},
    {0x5616 , 0x02},
    {0x5617 , 0x58},
    {0x5618 , 0x03},
    {0x5619 , 0x20},
    {0x5642 , 0x02},
    {0x5643 , 0x58},
    {0x5644 , 0x03},
    {0x5645 , 0x20},
    {0x5646 , 0x02},
    {0x5647 , 0x58},
    {0x5648 , 0x03},
    {0x5649 , 0x20},
    {0x564a , 0x02},
    {0x564b , 0x58},
    {0x564c , 0x03},
    {0x564d , 0x20},
    {0x564e , 0x02},
    {0x564f , 0x58},
    {0x5650 , 0x03},
    {0x5651 , 0x20},
    {0x5652 , 0x02},
    {0x5653 , 0x58},
    {0x5654 , 0x03},
    {0x5655 , 0x20},
    {0x5656 , 0x02},
    {0x5657 , 0x58},
    {0x5658 , 0x03},
    {0x5659 , 0x20},
    {0x5682 , 0x02},
    {0x5683 , 0x58},
    {0x5684 , 0x03},
    {0x5685 , 0x20},
    {0x5686 , 0x02},
    {0x5687 , 0x58},
    {0x5688 , 0x03},
    {0x5689 , 0x20},
    {0x568a , 0x02},
    {0x568b , 0x58},
    {0x568c , 0x03},
    {0x568d , 0x20},
    {0x568e , 0x02},
    {0x568f , 0x58},
    {0x5690 , 0x03},
    {0x5691 , 0x20},
    {0x5692 , 0x02},
    {0x5693 , 0x58},
    {0x5694 , 0x03},
    {0x5695 , 0x20},
    {0x5696 , 0x02},
    {0x5697 , 0x58},
    {0x5698 , 0x03},
    {0x5699 , 0x20},
    {0x5709 , 0x0f},
    {0x5749 , 0x0f},
    {0x5789 , 0x0f},
    {0x5200 , 0x70},
    {0x5201 , 0x70},
    {0x5202 , 0x73},
    {0x5203 , 0xff},
    {0x5205 , 0x6f},
    {0x5209 , 0x18},
    {0x520b , 0x04},
    {0x5285 , 0x6f},
    {0x5289 , 0x18},
    {0x528b , 0x04},
    {0x5305 , 0x6f},
    {0x5309 , 0x18},
    {0x530b , 0x04},
    {0x380e , 0x08},//30fps ;04
    {0x380f , 0xd8},//6c
    {0x380c , 0x04},
    {0x380d , 0x13},
    {0x384c , 0x02},
    {0x384d , 0x14},
    {0x4603 , 0x01},
    {0x460a , 0x06},
    {0x460c , 0x10},
    {0x0100 , 0x01},

};

const static I2C_ARRAY Sensor_init_table_16bit_1520_DCG_HDR[] =
{
    //Sensor revision: OS04A10
    //Input clock frequency: 24MHz
    //Image output size: 2688x1520
    //Image crop size: No special request
    //Pixel data format: DCG16
    //Frame timing and frame rate: 25fps
    //System clock frequency: No special request
    //Output interface and data rate: MIPI1080, 2lane
    //Others:
    //Core Setting version info: 22/12/20

	{0x0103, 0x01},
	{0x0109, 0x01},
	{0x0104, 0x02},
	{0x0102, 0x00},
	{0x0305, 0x50},
	{0x0306, 0x00},
	{0x0307, 0x00},
	{0x0308, 0x04},
	{0x030a, 0x01},
	{0x0317, 0x0a},
	{0x0322, 0x01},
	{0x0323, 0x02},
	{0x0324, 0x00},
	{0x0325, 0xd8},
	{0x0327, 0x05},
	{0x0329, 0x02},
	{0x032c, 0x02},
	{0x032d, 0x02},
	{0x032e, 0x02},
	{0x300f, 0x11},
	{0x3012, 0x21},
	{0x3026, 0x10},
	{0x3027, 0x08},
	{0x302d, 0x24},
	{0x3104, 0x01},
	{0x3106, 0x11},
	{0x3400, 0x00},
	{0x3408, 0x05},
	{0x340c, 0x0c},
	{0x340d, 0xb0},
	{0x3425, 0x51},
	{0x3426, 0x10},
	{0x3427, 0x14},
	{0x3428, 0x10},
	{0x3429, 0x10},
	{0x342a, 0x10},
	{0x342b, 0x04},
	{0x3501, 0x02},
	{0x3504, 0x08},
	{0x3508, 0x03},
	{0x3509, 0x80},
	{0x350a, 0x01},
	{0x3544, 0x08},
	{0x3548, 0x01},
	{0x3549, 0x00},
	{0x3584, 0x08},
	{0x3588, 0x01},
	{0x3589, 0x00},
	{0x3601, 0x70},
	{0x3604, 0xe3},
	{0x3605, 0xff},
	{0x3606, 0x01},
	{0x3608, 0xa8},
	{0x360a, 0xd0},
	{0x360b, 0x08},
	{0x360e, 0xc8},
	{0x360f, 0x66},
	{0x3610, 0x89},
	{0x3611, 0x8a},
	{0x3612, 0x4e},
	{0x3613, 0xbd},
	{0x3614, 0x9b},
	{0x362a, 0x0e},
	{0x362b, 0x0e},
	{0x362c, 0x0e},
	{0x362d, 0x09},
	{0x362e, 0x1a},
	{0x362f, 0x34},
	{0x3630, 0x67},
	{0x3631, 0x7f},
	{0x3638, 0x00},
	{0x3643, 0x00},
	{0x3644, 0x00},
	{0x3645, 0x00},
	{0x3646, 0x00},
	{0x3647, 0x00},
	{0x3648, 0x00},
	{0x3649, 0x00},
	{0x364a, 0x04},
	{0x364c, 0x0e},
	{0x364d, 0x0e},
	{0x364e, 0x0e},
	{0x364f, 0x0e},
	{0x3650, 0xff},
	{0x3651, 0xff},
	{0x365a, 0x7f},
	{0x365b, 0x67},
	{0x365c, 0x34},
	{0x365d, 0x1a},
	{0x3661, 0x07},
	{0x3662, 0x00},
	{0x3663, 0x20},
	{0x3665, 0x12},
	{0x3667, 0x54},
	{0x3668, 0x80},
	{0x366c, 0x00},
	{0x366d, 0x00},
	{0x366e, 0x00},
	{0x366f, 0x00},
	{0x3671, 0x0d},
	{0x3673, 0x30},
	{0x3681, 0x80},
	{0x3700, 0x23},
	{0x3701, 0x1c},
	{0x3702, 0x25},
	{0x3703, 0x28},
	{0x3705, 0x01},
	{0x3706, 0x00},
	{0x3707, 0x0a},
	{0x3708, 0x35},
	{0x3709, 0x57},
	{0x370a, 0x03},
	{0x370b, 0x15},
	{0x3714, 0x01},
	{0x3719, 0x24},
	{0x371b, 0x1f},
	{0x371c, 0x00},
	{0x371d, 0x08},
	{0x373f, 0x63},
	{0x3740, 0x63},
	{0x3741, 0x63},
	{0x3742, 0x63},
	{0x3743, 0x01},
	{0x3756, 0xe7},
	{0x3757, 0xe7},
	{0x3762, 0x1c},
	{0x376c, 0x01},
	{0x3776, 0x05},
	{0x3777, 0x22},
	{0x3779, 0x60},
	{0x377c, 0x48},
	{0x3784, 0x06},
	{0x3785, 0x0a},
	{0x3790, 0x10},
	{0x3793, 0x04},
	{0x3794, 0x07},
	{0x3796, 0x00},
	{0x3797, 0x02},
	{0x379c, 0x4d},
	{0x37a1, 0x80},
	{0x37bb, 0x88},
	{0x37be, 0x48},
	{0x37bf, 0x01},
	{0x37c0, 0xc8},
	{0x37c4, 0x63},
	{0x37c5, 0x63},
	{0x37c6, 0x63},
	{0x37ca, 0x21},
	{0x37cc, 0x15},
	{0x37cd, 0x90},
	{0x37cf, 0x02},
	{0x37d0, 0x01},
	{0x37d1, 0x00},
	{0x37d2, 0x03},
	{0x37d3, 0x15},
	{0x37d4, 0x01},
	{0x37d5, 0x00},
	{0x37d6, 0x03},
	{0x37d7, 0x15},
	{0x37d8, 0x01},
	{0x37dc, 0x00},
	{0x37dd, 0x00},
	{0x37da, 0x00},
	{0x37db, 0x00},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x8f},
	{0x3806, 0x05},
	{0x3807, 0xff},
	{0x3808, 0x0a},
	{0x3809, 0x80},
	{0x380a, 0x05},
	{0x380b, 0xf0},
	{0x380c, 0x0b},
	{0x380d, 0x88},
	{0x380e, 0x07},
	{0x380f, 0x24},
	{0x3811, 0x08},
	{0x3813, 0x08},
	{0x3814, 0x01},
	{0x3815, 0x01},
	{0x3816, 0x01},
	{0x3817, 0x01},
	{0x381c, 0x00},
	{0x3820, 0x02},
	{0x3821, 0x00},
	{0x3822, 0x14},
	{0x3823, 0x18},
	{0x3826, 0x00},
	{0x3827, 0x00},
	{0x3833, 0x40},
	{0x384c, 0x05},
	{0x384d, 0xc4},
	{0x3858, 0x3c},
	{0x3865, 0x01},
	{0x3866, 0xa0},
	{0x3867, 0x00},
	{0x3868, 0x20},
	{0x3900, 0x13},
	{0x3940, 0x13},
	{0x3980, 0x13},
	{0x3c01, 0x11},
	{0x3c05, 0x00},
	{0x3c0f, 0x1c},
	{0x3c12, 0x0d},
	{0x3c19, 0x00},
	{0x3c21, 0x00},
	{0x3c3a, 0x10},
	{0x3c3b, 0x18},
	{0x3c3d, 0xc6},
	{0x3c55, 0xcb},
	{0x3c5a, 0xe5},
	{0x3c5d, 0xcf},
	{0x3c5e, 0xcf},
	{0x3d8c, 0x70},
	{0x3d8d, 0x10},
	{0x4000, 0xf9},
	{0x4001, 0xef},
	{0x4004, 0x00},
	{0x4005, 0x80},
	{0x4008, 0x02},
	{0x4009, 0x11},
	{0x400a, 0x03},
	{0x400b, 0x27},
	{0x400e, 0x40},
	{0x402e, 0x00},
	{0x402f, 0x80},
	{0x4030, 0x00},
	{0x4031, 0x80},
	{0x4032, 0x9f},
	{0x4033, 0x80},
	{0x4050, 0x00},
	{0x4051, 0x07},
	{0x4011, 0xbb},
	{0x410f, 0x01},
	{0x4288, 0xce},
	{0x4289, 0x00},
	{0x428a, 0x56},
	{0x430b, 0xff},
	{0x430c, 0xff},
	{0x430d, 0x00},
	{0x430e, 0x00},
	{0x4314, 0x04},
	{0x4500, 0x18},
	{0x4501, 0x18},
	{0x4503, 0x10},
	{0x4504, 0x00},
	{0x4506, 0x32},
	{0x4507, 0x03},
	{0x4601, 0x30},
	{0x4603, 0x00},
	{0x460a, 0x50},
	{0x460c, 0x60},
	{0x4640, 0x62},
	{0x4646, 0xaa},
	{0x4647, 0x55},
	{0x4648, 0x99},
	{0x4649, 0x66},
	{0x464d, 0x00},
	{0x4654, 0x11},
	{0x4655, 0x22},
	{0x4800, 0x44},
	{0x480e, 0x00},
	{0x4810, 0xff},
	{0x4811, 0xff},
	{0x4813, 0x00},
	{0x481f, 0x30},
	{0x4837, 0x10},
	{0x484b, 0x67},
	{0x4d00, 0x4d},
	{0x4d01, 0x9d},
	{0x4d02, 0xb9},
	{0x4d03, 0x2e},
	{0x4d04, 0x4a},
	{0x4d05, 0x3d},
	{0x4d09, 0x4f},
	{0x5000, 0x1f},
	{0x5001, 0x0c},
	{0x5080, 0x00},
	{0x50c0, 0x00},
	{0x5100, 0x00},
	{0x5200, 0x00},
	{0x5201, 0x00},
	{0x5202, 0x03},
	{0x5203, 0xff},
	{0x5780, 0x53},
	{0x5782, 0x60},
	{0x5783, 0xf0},
	{0x5786, 0x01},
	{0x5788, 0x60},
	{0x5789, 0xf0},
	{0x5792, 0x11},
	{0x5793, 0x33},
	{0x5857, 0xff},
	{0x5858, 0xff},
	{0x5859, 0xff},
	{0x58d7, 0xff},
	{0x58d8, 0xff},
	{0x58d9, 0xff},
	{0x0100, 0x01},


};
const static I2C_ARRAY Sensor_init_table_12bit_1520_DCG_HDR[] =
{
    //Sensor revision: OS04A10
    //Input clock frequency: 24MHz
    //Image output size: 1920x1080
    //Image crop size: No special request
    //Pixel data format: DCG16
    //Frame timing and frame rate: 20fps
    //System clock frequency: No special request
    //Output interface and data rate: MIPI1080, 4lane
    //Others:
    //Core Setting version info: 22/12/20

	{0x0103, 0x01},
	{0x0109, 0x01},
	{0x0104, 0x02},
	{0x0102, 0x00},
	{0x0305, 0x40},
	{0x0306, 0x00},
	{0x0307, 0x00},
	{0x0308, 0x05},
	{0x030a, 0x01},
	{0x0317, 0x0a},
	{0x0322, 0x01},
	{0x0323, 0x02},
	{0x0324, 0x00},
	{0x0325, 0xd8},
	{0x0327, 0x05},
	{0x0329, 0x02},
	{0x032c, 0x02},
	{0x032d, 0x02},
	{0x032e, 0x02},
	{0x300f, 0x11},
	{0x3012, 0x21},
	{0x3026, 0x10},
	{0x3027, 0x08},
	{0x302d, 0x24},
	{0x3104, 0x01},
	{0x3106, 0x11},
	{0x3400, 0x00},
	{0x3408, 0x05},
	{0x340c, 0x0c},
	{0x340d, 0xb0},
	{0x3425, 0x51},
	{0x3426, 0x10},
	{0x3427, 0x14},
	{0x3428, 0x10},
	{0x3429, 0x10},
	{0x342a, 0x10},
	{0x342b, 0x04},
	{0x3501, 0x06},
	{0x3502, 0x48},
	{0x3504, 0x08},
	{0x3508, 0x03},
	{0x3509, 0x00},
	{0x350a, 0x01},
	{0x3544, 0x08},
	{0x3548, 0x01},
	{0x3549, 0x00},
	{0x3584, 0x08},
	{0x3588, 0x01},
	{0x3589, 0x00},
	{0x3601, 0x70},
	{0x3604, 0xe3},
	{0x3605, 0x7f},
	{0x3606, 0x00},
	{0x3608, 0xa8},
	{0x360a, 0xd0},
	{0x360b, 0x08},
	{0x360e, 0xc8},
	{0x360f, 0x66},
	{0x3610, 0x89},
	{0x3611, 0x8a},
	{0x3612, 0x4e},
	{0x3613, 0xbd},
	{0x3614, 0x9b},
	{0x362a, 0x0e},
	{0x362b, 0x0e},
	{0x362c, 0x0e},
	{0x362d, 0x0e},
	{0x362e, 0x1a},
	{0x362f, 0x34},
	{0x3630, 0x67},
	{0x3631, 0x7f},
	{0x3638, 0x00},
	{0x3643, 0x00},
	{0x3644, 0x00},
	{0x3645, 0x00},
	{0x3646, 0x00},
	{0x3647, 0x00},
	{0x3648, 0x00},
	{0x3649, 0x00},
	{0x364a, 0x04},
	{0x364c, 0x0e},
	{0x364d, 0x0e},
	{0x364e, 0x0e},
	{0x364f, 0x0e},
	{0x3650, 0xff},
	{0x3651, 0xff},
	{0x365a, 0x7f},
	{0x365b, 0x67},
	{0x365c, 0x34},
	{0x365d, 0x1a},
	{0x3661, 0x07},
	{0x3662, 0x00},
	{0x3663, 0x20},
	{0x3665, 0x12},
	{0x3667, 0xd4},
	{0x3668, 0x80},
	{0x366c, 0x00},
	{0x366d, 0x00},
	{0x366e, 0x00},
	{0x366f, 0xc0},
	{0x3671, 0x2d},
	{0x3673, 0x6c},
	{0x3681, 0x80},
	{0x3700, 0x23},
	{0x3701, 0x1c},
	{0x3702, 0x24},
	{0x3703, 0x20},
	{0x3705, 0x00},
	{0x3706, 0x6c},
	{0x3707, 0x0a},
	{0x3708, 0x36},
	{0x3709, 0x57},
	{0x370a, 0x00},
	{0x370b, 0xf7},
	{0x3714, 0x01},
	{0x3719, 0x1f},
	{0x371b, 0x16},
	{0x371c, 0x00},
	{0x371d, 0x08},
	{0x373f, 0x63},
	{0x3740, 0x63},
	{0x3741, 0x63},
	{0x3742, 0x63},
	{0x3743, 0x01},
	{0x3756, 0x9d},
	{0x3757, 0x9d},
	{0x3762, 0x1c},
	{0x376c, 0x01},
	{0x3776, 0x05},
	{0x3777, 0x22},
	{0x3779, 0x60},
	{0x377c, 0x48},
	{0x3784, 0x06},
	{0x3785, 0x0a},
	{0x3790, 0x10},
	{0x3793, 0x04},
	{0x3794, 0x07},
	{0x3796, 0x00},
	{0x3797, 0x02},
	{0x379c, 0x4d},
	{0x37a1, 0x80},
	{0x37bb, 0x88},
	{0x37be, 0x48},
	{0x37bf, 0x01},
	{0x37c0, 0xc8},
	{0x37c4, 0x63},
	{0x37c5, 0x63},
	{0x37c6, 0x63},
	{0x37ca, 0x21},
	{0x37cc, 0x13},
	{0x37cd, 0x90},
	{0x37cf, 0x02},
	{0x37d0, 0x00},
	{0x37d1, 0x6c},
	{0x37d2, 0x00},
	{0x37d3, 0xf7},
	{0x37d4, 0x00},
	{0x37d5, 0x6c},
	{0x37d6, 0x00},
	{0x37d7, 0xf7},
	{0x37d8, 0x01},
	{0x37dc, 0x00},
	{0x37dd, 0x00},
	{0x37da, 0x00},
	{0x37db, 0x00},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x8f},
	{0x3806, 0x05},
	{0x3807, 0xff},
	{0x3808, 0x0a},
	{0x3809, 0x80},
	{0x380a, 0x05},
	{0x380b, 0xf0},
	{0x380c, 0x0c},
	{0x380d, 0xc4},
	{0x380e, 0x06},
	{0x380f, 0x74},
	{0x3811, 0x08},
	{0x3813, 0x08},
	{0x3814, 0x01},
	{0x3815, 0x01},
	{0x3816, 0x01},
	{0x3817, 0x01},
	{0x381c, 0x00},
	{0x3820, 0x02},
	{0x3821, 0x00},
	{0x3822, 0x14},
	{0x3823, 0x18},
	{0x3826, 0x00},
	{0x3827, 0x00},
	{0x3833, 0x40},
	{0x384c, 0x02},
	{0x384d, 0xdc},
	{0x3858, 0x3c},
	{0x3865, 0x01},
	{0x3866, 0xa0},
	{0x3867, 0x00},
	{0x3868, 0x20},
	{0x3900, 0x13},
	{0x3940, 0x13},
	{0x3980, 0x13},
	{0x3c01, 0x11},
	{0x3c05, 0x00},
	{0x3c0f, 0x1c},
	{0x3c12, 0x0d},
	{0x3c19, 0x00},
	{0x3c21, 0x00},
	{0x3c3a, 0x10},
	{0x3c3b, 0x18},
	{0x3c3d, 0xc6},
	{0x3c55, 0xcb},
	{0x3c5a, 0x55},
	{0x3c5d, 0xcf},
	{0x3c5e, 0xcf},
	{0x3d8c, 0x70},
	{0x3d8d, 0x10},
	{0x4000, 0xf9},
	{0x4001, 0x2f},
	{0x4004, 0x00},
	{0x4005, 0x40},
	{0x4008, 0x02},
	{0x4009, 0x11},
	{0x400a, 0x19},
	{0x400b, 0x00},
	{0x400e, 0x40},
	{0x402e, 0x00},
	{0x402f, 0x40},
	{0x4030, 0x00},
	{0x4031, 0x40},
	{0x4032, 0x9f},
	{0x4033, 0x80},
	{0x4050, 0x00},
	{0x4051, 0x07},
	{0x4011, 0xbb},
	{0x410f, 0x01},
	{0x4288, 0xcf},
	{0x4289, 0x00},
	{0x428a, 0x56},
	{0x430b, 0xff},
	{0x430c, 0xff},
	{0x430d, 0x00},
	{0x430e, 0x00},
	{0x4314, 0x04},
	{0x4500, 0x18},
	{0x4501, 0x18},
	{0x4503, 0x10},
	{0x4504, 0x00},
	{0x4506, 0x32},
	{0x4507, 0x02},
	{0x4601, 0x30},
	{0x4603, 0x00},
	{0x460a, 0x50},
	{0x460c, 0x60},
	{0x4640, 0x62},
	{0x4646, 0xaa},
	{0x4647, 0x55},
	{0x4648, 0x99},
	{0x4649, 0x66},
	{0x464d, 0x00},
	{0x4654, 0x11},
	{0x4655, 0x22},
	{0x4800, 0x44},
	{0x480e, 0x00},
	{0x4810, 0xff},
	{0x4811, 0xff},
	{0x4813, 0x00},
	{0x481f, 0x30},
	{0x4837, 0x14},
	{0x484b, 0x27},
	{0x4d00, 0x4d},
	{0x4d01, 0x9d},
	{0x4d02, 0xb9},
	{0x4d03, 0x2e},
	{0x4d04, 0x4a},
	{0x4d05, 0x3d},
	{0x4d09, 0x4f},
	{0x5000, 0x1f},
	{0x5001, 0x0d},
	{0x5080, 0x00},
	{0x50c0, 0x00},
	{0x5100, 0x00},
	{0x5200, 0x00},
	{0x5201, 0x00},
	{0x5202, 0x03},
	{0x5203, 0xff},
	{0x5780, 0x53},
	{0x5782, 0x60},
	{0x5783, 0xf0},
	{0x5786, 0x01},
	{0x5788, 0x60},
	{0x5789, 0xf0},
	{0x5792, 0x11},
	{0x5793, 0x33},
	{0x5857, 0xff},
	{0x5858, 0xff},
	{0x5859, 0xff},
	{0x58d7, 0xff},
	{0x58d8, 0xff},
	{0x58d9, 0xff},
	{0x0100, 0x01},
	{0x0100, 0x01},


};

/*const static I2C_ARRAY mirror_reg[] =
{
  {0x3820, 0x01}, //bit2 flip
  {0x3821, 0x00}, //bit2 mirror
};

const static I2C_ARRAY mirror_reg_HDR[] =
{
    {0x3820, 0x03},//M0F0 [3:2]flip [1]mirror [0] hdr
};*/

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

//static int g_sensor_ae_min_gain = 1024;//1280;

const static I2C_ARRAY gain_reg[] = {
    {0x3508, 0x0f},//long a-gain[13:8]
    {0x3509, 0x00},//long a-gain[7:0]
    {0x350A, 0x0f},// d-gain[13:8]
    {0x350B, 0x00},// d-gain[7:0]
    {0x350C, 0x00},// d-gain[7:0]
};

const static I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x3548, 0x01},//short a-gain[13:8]
    {0x3549, 0x00},//short a-gain[7:0]
    {0x354A, 0x01},// d-gain[13:8]
    {0x354B, 0x00},// d-gain[7:0]
    {0x354C, 0x00},// d-gain[7:0]
};

const static I2C_ARRAY gain_reg_HDR_VS[] = {
    {0x3588, 0x0f},//short a-gain[13:8]
    {0x3589, 0x00},//short a-gain[7:0]
    {0x358A, 0x0f},// d-gain[13:8]
    {0x358B, 0x00},// d-gain[7:0]
    {0x358C, 0x00},// d-gain[7:0]
};

const static I2C_ARRAY expo_reg[] = {
    {0x3501, 0x0A},//long exp[15,8]
    {0x3502, 0x00},//long exp[7,0]
};

const static I2C_ARRAY expo_reg_HDR_VS[] = {
    {0x3581, 0x00},//short
    {0x3582, 0x20},
};

const static I2C_ARRAY vts_reg[] = {
    {0x380E, 0x08},
    {0x380F, 0xdb},
};

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
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME OS04A10_DCG


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
#if 0
static int ISP_config_io(ss_cus_sensor *handle) {
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s]", __FUNCTION__);

    sensor_if->HsyncPol(handle, handle->HSYNC_POLARITY);
    sensor_if->VsyncPol(handle, handle->VSYNC_POLARITY);
    sensor_if->ClkPol(handle, handle->PCLK_POLARITY);
    sensor_if->BayerFmt(handle, handle->bayer_id);
    sensor_if->DataBus(handle, handle->sif_bus);

    sensor_if->DataPrecision(handle, handle->data_prec);
    sensor_if->FmtConv(handle,  handle->data_mode);
    return SUCCESS;
}
#endif
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period *2;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_min                   = Preview_line_period * 2;
        info->u32AEShutter_step                  = Preview_line_period;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_max                   = Preview_line_period * params->expo.max_short;
        }
        else
        {
            info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        }
    }
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    sensor_if->Reset(idx, params->reset_POLARITY);

    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, handle->mclk);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_288M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x7C00, 0);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_COMP) {
        if(handle->data_prec == CUS_DATAPRECISION_16){
//            sensor_if->SetCSI_LongPacketType(idx, 0, 0x7C00, 0);
            sensor_if->SetCSI_hdr_mode(idx, CUS_HDR_MODE_COMP, 1);

        }else{
//            sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
            sensor_if->SetCSI_hdr_mode(idx, CUS_HDR_MODE_COMP, 1);
        }
    }
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_COMP_VS) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 10);
    }

    //sensor_if->MCLK(idx, 1, handle->mclk);
    //SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    //sensor_if->Reset(idx, params->reset_POLARITY);
    //SENSOR_UDELAY(20);
    //SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    //sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    //SENSOR_UDELAY(20);
    ///////////////////

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_UDELAY(20);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !params->reset_POLARITY);

    SENSOR_UDELAY(30);
    //sensor_if->MCLK(idx, 1, handle->mclk);

    SENSOR_MSLEEP(50);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, params->reset_POLARITY);
    SENSOR_UDELAY(30);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_UDELAY(30);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC  ||
        handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_COMP  ||
        handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_COMP_VS)
    {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->MCLK(idx, 0, handle->mclk);

//    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int OS04A10_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    return SUCCESS;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_init(ss_cus_sensor *handle)
{
    int i,cnt=0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {
        if(Sensor_init_table[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table[i].reg,Sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                //SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    //pCus_AEStatusNotify(handle,CUS_FRAME_ACTIVE);

    //CamOsPrintf("pCus_init = %d us \n",timeGetTimeU()-TStart);
    return SUCCESS;
}


static int pCus_init_16bit_1520_DCG_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_16bit_1520_DCG_HDR);i++)
    {
        if(Sensor_init_table_16bit_1520_DCG_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_16bit_1520_DCG_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_16bit_1520_DCG_HDR[i].reg,Sensor_init_table_16bit_1520_DCG_HDR[i].data) != SUCCESS)
            {
                cnt++;
                SENSOR_DMSG("Sensor_init_table_16bit_1520_DCG_HDR -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    //pCus_AEStatusNotify(handle,CUS_FRAME_ACTIVE);

    //CamOsPrintf("pCus_init = %d us \n",timeGetTimeU()-TStart);
    return SUCCESS;
}

static int pCus_init_12bit_1520_DCG_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_12bit_1520_DCG_HDR);i++)
    {
        if(Sensor_init_table_12bit_1520_DCG_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_12bit_1520_DCG_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_12bit_1520_DCG_HDR[i].reg,Sensor_init_table_12bit_1520_DCG_HDR[i].data) != SUCCESS)
            {
                cnt++;
                SENSOR_DMSG("Sensor_init_table_16bit_1520_DCG_HDR -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    //pCus_AEStatusNotify(handle,CUS_FRAME_ACTIVE);

    //CamOsPrintf("pCus_init = %d us \n",timeGetTimeU()-TStart);
    return SUCCESS;
}
#if 0
static int pCus_init_DCG_VS_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_DCG_VS_HDR);i++)
    {
        if(Sensor_init_table_DCG_VS_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_DCG_VS_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_DCG_VS_HDR[i].reg,Sensor_init_table_DCG_VS_HDR[i].data) != SUCCESS)
            {
                cnt++;
                //SENSOR_DMSG("Sensor_init_table_DCG_VS_HDR -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    //pCus_AEStatusNotify(handle,CUS_FRAME_ACTIVE);

    //CamOsPrintf("pCus_init = %d us \n",timeGetTimeU()-TStart);
    return SUCCESS;
}
#endif
/*
int pCus_release(ss_cus_sensor *handle)
{
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    sensor_if->PCLK(NULL,CUS_PCLK_OFF);
    return SUCCESS;
}
*/

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
    u32 num_res = handle->video_res_supported.num_res;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case LINEAR_RES_1:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init;
            vts_30fps = 1144;
            Preview_line_period = 26667;  //33333/1250
            handle->mclk = CUS_CMU_CLK_24MHZ;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;
        /*case LINEAR_RES_2:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_16bit_1520_DCG_HDR;
            handle->data_prec   = SENSOR_DATAPREC_HDR;
            vts_30fps = 2267;
            Preview_line_period = 26667;
            handle->mclk = CUS_CMU_CLK_24MHZ;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_COMP;
            break;
        */
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case HDR_RES_1:  //DCG HDR
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_16bit_1520_DCG_HDR;
            handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_COMP;
            handle->interface_attr.attr_mipi.mipi_lane_num = 2;
            handle->bayer_id    = SENSOR_BAYERID_HDR;
            handle->RGBIR_id    = SENSOR_RGBIRID;
            handle->data_prec   = CUS_DATAPRECISION_16;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 25;
            params->expo.max_short=105;
            vts_30fps = 2267;
            Preview_line_period = 26667;
            break;
        case HDR_RES_2:  //DCG HDR
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_12bit_1520_DCG_HDR;
            handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_COMP;
            handle->interface_attr.attr_mipi.mipi_lane_num = 2;
            handle->bayer_id    = SENSOR_BAYERID_HDR;
            handle->RGBIR_id    = SENSOR_RGBIRID;
            handle->data_prec   = CUS_DATAPRECISION_12;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short=105;
            vts_30fps = 2057;
            Preview_line_period = 16204;
            break;
#if 0
        case HDR_RES_3:  //DCG_VS HDR
            handle->video_res_supported.ulcur_res = 2;
            handle->pCus_sensor_init = pCus_init_DCG_VS_HDR;
            //handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame
            handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_COMP_VS;
            handle->bayer_id    = SENSOR_BAYERID_HDR;
            handle->RGBIR_id    = SENSOR_RGBIRID;
            handle->data_prec   = SENSOR_DATAPREC_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short=vts_30fps_HDR-10;
            Preview_line_period = 14723;
            break;
#endif

        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    sen_data = params->tMirror_reg[0].data;
    SENSOR_DMSG("[%s] mirror:%x\r\n", __FUNCTION__, sen_data & 0x66);
    switch(sen_data)
    {
        case 0x02:
            *orit = CUS_ORIT_M0F0;
            break;
        case 0x00:
            *orit = CUS_ORIT_M1F0;
            break;
        case 0x06:
            *orit = CUS_ORIT_M0F1;
            break;
        case 0x04:
            *orit = CUS_ORIT_M1F1;
            break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    CamOsPrintf(KERN_DEBUG "[%s] mirror:%x now:%d\r\n", __FUNCTION__, params->tMirror_reg[0].data & 0x66,orit);
    switch(orit) {
    case CUS_ORIT_M0F0:
        params->tMirror_reg[0].data = 0x02;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F0:
        params->tMirror_reg[0].data = 0x00;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M0F1:
        params->tMirror_reg[0].data = 0x06;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F1:
        params->tMirror_reg[0].data = 0x04;
        params->orien_dirty = true;
        break;
    default :
        break;
    }
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_COMP) {
        params->tMirror_reg[0].data |= 0x1;
    }
    params->res.orit = orit;
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
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
    u32 vts=0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if ((params->expo.line) > (params->expo.vts)-8) {
        vts = params->expo.line + 8;
    }else
        vts = params->expo.vts;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetFPS_HDR_SEF(ss_cus_sensor *handle)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR(ss_cus_sensor *handle, u32 fps)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u16 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u16 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
        return FAIL;
    }

    params->expo.max_short = (((params->expo.vts)/17 - 1)>>1) << 1;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

#if 0
static int pCus_GetSensorCap(ss_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap) {
    if (cap)
        memcpy(cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP));
    else     return FAIL;
    return SUCCESS;
}
#endif

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if(params->orien_dirty){
                CamOsPrintf(KERN_DEBUG "[%s] mirror:%x now:%d\r\n", __FUNCTION__, params->tMirror_reg[0].data & 0x66, params->res.orit);
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orien_dirty = false;
            }
            break;
        case CUS_FRAME_ACTIVE:
            CamOsPrintf(KERN_DEBUG "[%s] set  exp:(0x%x%x) gain:(%x%x,%x%x%x)(%d) vts_reg value:(0x%x%x) drity:%d\n", __FUNCTION__,
                params->tExpo_reg[0].data,params->tExpo_reg[1].data, params->tGain_reg[0].data, params->tGain_reg[1].data,
                params->tGain_reg[2].data, params->tGain_reg[3].data,params->tGain_reg[4].data, params->expo.final_gain,
                params->tVts_reg[0].data,params->tVts_reg[1].data,params->dirty);
            if(params->dirty)
            {
                SensorReg_Write(0x3208,0x00);
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                //if(handle->pCus_sensor_init == pCus_init_16bit_1520_DCG_HDR)
                //{
                //    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
                //}
                SensorReg_Write(0x3208,0x10);
                SensorReg_Write(0x3208,0xa0);
                params->dirty = false;
            }
            break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_AEStatusNotify_HDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    CamOsPrintf(KERN_DEBUG "[%s] set  lsh:(0x%x%x) lgain:(%x,%x,%x)(%d) sgain:(%x,%x,%x)(%d) vts_reg value:(0x%x%x) reg_dirty:%d\n", __FUNCTION__,
        params->tExpo_reg[0].data, params->tExpo_reg[1].data,
        params->tGain_reg[0].data, params->tGain_reg[1].data,
        params->tGain_reg[2].data, params->expo.final_gain,
        params->tGain_reg_HDR_SEF[0].data, params->tGain_reg_HDR_SEF[1].data,
        params->tGain_reg_HDR_SEF[2].data, params->expo.final_sef_gain,
        params->tVts_reg[0].data, params->tVts_reg[1].data, params->dirty);
    return SUCCESS;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if(params->orien_dirty){
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orien_dirty = false;
            }
            break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty)
            {
                SensorReg_Write(0x3208,0x00);
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorReg_Write(0x3208,0x10);
                SensorReg_Write(0x3208,0xa0);
                params->dirty = false;
            }
            break;
        default :
        break;
    }

    return SUCCESS;

}
/*
static int pCus_AEStatusNotify_HDR_SEF(ss_cus_sensor *handle, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    //OS04A10_params *params = (OS04A10_params *)handle->private_data;
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
*/
#if 0
static int pCus_AEStatusNotify_HDR_VS(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->ori_dirty){
            //SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg_HDR, ARRAY_SIZE(mirror_reg_HDR));
            SensorReg_Write(mirror_flip_HDR[0].reg,mirror_flip_HDR[0].data);
            SensorReg_Write(mirror_flip_HDR[1].reg,mirror_flip_HDR[1].data);

            params->ori_dirty = false;
        }
       /* if(params->skip_cnt){
            sensor_if->SetSkipFrame(handle->snr_pad_group, params->expo.fps, params->skip_cnt);
            params->skip_cnt = 0;
        } */
        if(params->dirty)
        {
            SensorReg_Write(0x3208,0x00);
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_VS, ARRAY_SIZE(expo_reg_HDR_VS));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_VS, ARRAY_SIZE(gain_reg_HDR_VS));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            SensorReg_Write(0x3208,0x10);
            SensorReg_Write(0x3208,0xa0);
            params->dirty = false;
        }
        break;
        default :
        break;
    }

    return SUCCESS;
}
#endif

static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us) {
    u32 vts = 0, lines = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

#if 0
    lines=(1000*us)/Preview_line_period_HDR;

    if(lines<1) lines=1;
    if (lines > (params->expo.vts - params->expo.max_short - 2)) {
        lines = params->expo.vts - params->expo.max_short - 2;
    }else
        vts = params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );


    lines=20;

    HDR_LEF_exp = lines;

    params->tExpo_reg[0].data = (lines>>8) & 0xff;
    params->tExpo_reg[1].data = (lines>>0) & 0xff;
    CamOsPrintf("[%s] us %ld, lines %ld, vts %ld  Expo_reg[0]=%ld   Expo_reg[1]=%ld\n", __FUNCTION__,us,lines,params->expo.vts,params->tExpo_reg[0].data,params->tExpo_reg[1].data);

    //params->tVts_reg[0].data = (vts >> 8) & 0xff;
    //params->tVts_reg[1].data = (vts >> 0) & 0xff;
#endif
    lines=(1000*us)/Preview_line_period;

    if(lines<1) lines=1;
    if (lines > params->expo.vts-8) {
        vts = lines + 8;
    }else
        vts = params->expo.vts;

    params->expo.line = lines;
    CamOsPrintf("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    params->tExpo_reg[0].data = (lines>>8) & 0xff;
    params->tExpo_reg[1].data = (lines>>0) & 0xff;

    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;
    //CamOsPrintf("[%s] us %ld, lines %ld, vts %ld  Expo[0]=%ld   Expo[1]=%ld\n", __FUNCTION__,us,lines,params->expo.vts,params->tExpo_reg[0].data,params->tExpo_reg[1].data);



    params->dirty = true;
    return SUCCESS;
}
#if 0
static int pCus_GetAEUSecs_HDR_VS(ss_cus_sensor *handle, u32 *us) {
    u32 lines = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg_HDR_VS[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_VS[1].data&0xff)<<0;

    *us = (lines*Preview_line_period_HDR)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_VS(ss_cus_sensor *handle, u32 us) {
    u32 vts=0, lines = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_HDR;
    if(lines<1) lines=1;
    if (lines > params->expo.max_short - HDR_LEF_exp - 2) {
        lines = params->expo.max_short - HDR_LEF_exp - 2;
    }else
        vts = params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    params->tExpo_reg_HDR_VS[0].data = (lines>>8) & 0xff;
    params->tExpo_reg_HDR_VS[1].data = (lines>>0) & 0xff;
    //CamOsPrintf("[%s] us %ld, lines %ld, vts %ld  SEF[0]=%ld   SEF[1]=%ld\n", __FUNCTION__,us,lines,params->expo.vts,params->tExpo_reg_HDR_VS[0].data,params->tExpo_reg_HDR_VS[1].data);

    params->dirty = true;
    return SUCCESS;
}
#endif
static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period;

    if(lines<1) lines=1;
    if (lines > params->expo.vts-8) {
        vts = lines + 8;
    }else
        vts = params->expo.vts;

    params->expo.line = lines;
    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    params->tExpo_reg[0].data = (lines>>8) & 0xff;
    params->tExpo_reg[1].data = (lines>>0) & 0xff;

    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;
    //CamOsPrintf("[%s] us %ld, lines %ld, vts %ld  Expo[0]=%ld   Expo[1]=%ld\n", __FUNCTION__,us,lines,params->expo.vts,params->tExpo_reg[0].data,params->tExpo_reg[1].data);

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    *gain=params->expo.final_gain;
    return SUCCESS;
}
static int pCus_GetAEGain_HDR_SEF(ss_cus_sensor *handle, u32* gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    *gain=params->expo.final_sef_gain;
    return SUCCESS;
}
#if 0
static int pCus_GetAEGain_HDR_VS(ss_cus_sensor *handle, u32* gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    *gain=params->expo.final_vs_gain;
    return SUCCESS;
}
#endif


#define MAX_A_GAIN 15872//(15.5*1024)
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 gain16 = 0, Again = 0x01, gain1024 = 0x00;

    params->expo.final_gain = gain;

    if (gain<1024) {
        gain = 1024;
    } else if (gain>= MAX_A_GAIN) {
        gain = MAX_A_GAIN; //again max 15.5x,Without using digital gain
    }

    if(gain < 2048) {
        Again = gain>>10;  gain16 = (gain&0x3ff)>>6;
    }
    else if((gain>=2048 )&&(gain<4096))//X2~X4
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>7)<<1;
    }
    else if((gain>=4096 )&&(gain<8192))//X4~X8
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>8)<<2;
    }
    else if((gain>=8192 )&&(gain<=15872))//X8~X15.5
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>9)<<3;
    }

    gain1024 = (16*gain)/((Again<<4) + gain16);

    params->tGain_reg[0].data = Again;//high bit
    params->tGain_reg[1].data = (gain16<<4)&0xf0; //low byte
    params->tGain_reg[2].data = 0x01; //low byte
    params->tGain_reg[3].data = (gain1024&0x3fc)>>2;
    params->tGain_reg[4].data = (gain1024&0x03); //low byte

    SENSOR_DMSG("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
/*  CamOsPrintf("[%s] set gain =%d ,0x%x 0x%x 0x%x 0x%x 0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data,
                                                                params->tGain_reg[1].data,
                                                                params->tGain_reg[2].data,
                                                                params->tGain_reg[3].data,
                                                                params->tGain_reg[4].data);
*/
    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEGain_HDR_SEF(ss_cus_sensor *handle, u32 gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 gain16 = 0, Again = 0x01, gain1024 = 0x00;

    params->expo.final_sef_gain = gain;

    if (gain<1024) {
        gain = 1024;
    } else if (gain>= MAX_A_GAIN) {
        gain = MAX_A_GAIN; //again max 15.5x,Without using digital gain
    }

    if(gain < 2048) {
        Again = gain>>10;  gain16 = (gain&0x3ff)>>6;
    }
    else if((gain>=2048 )&&(gain<4096))//X2~X4
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>7)<<1;
    }
    else if((gain>=4096 )&&(gain<8192))//X4~X8
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>8)<<2;
    }
    else if((gain>=8192 )&&(gain<=15872))//X8~X15.5
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>9)<<3;
    }

    gain1024 = (16*gain)/((Again<<4) + gain16);

    params->tGain_reg_HDR_SEF[0].data = Again;//high bit
    params->tGain_reg_HDR_SEF[1].data = (gain16<<4)&0xf0; //low byte
    params->tGain_reg_HDR_SEF[2].data = 0x01; //low byte
    params->tGain_reg_HDR_SEF[3].data = (gain1024&0x3fc)>>2;
    params->tGain_reg_HDR_SEF[4].data = (gain1024&0x03); //low byte

    SENSOR_DMSG("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
    //CamOsPrintf("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}

#if 0
static int pCus_SetAEGain_HDR_VS(ss_cus_sensor *handle, u32 gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 gain16 = 0, Again = 0x01, gain1024 = 0x00;

    params->expo.final_vs_gain = gain;

    if (gain<1024) {
        gain = 1024;
    } else if (gain>= MAX_A_GAIN) {
        gain = MAX_A_GAIN; //again max 15.5x,Without using digital gain
    }

    if(gain < 2048) {
        Again = gain>>10;  gain16 = (gain&0x3ff)>>6;
    }
    else if((gain>=2048 )&&(gain<4096))//X2~X4
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>7)<<1;
    }
    else if((gain>=4096 )&&(gain<8192))//X4~X8
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>8)<<2;
    }
    else if((gain>=8192 )&&(gain<=15872))//X8~X15.5
    {
        Again = gain>>10;  gain16 = ((gain&0x3ff)>>9)<<3;
    }

    gain1024 = (16*gain)/((Again<<4) + gain16);

    params->tGain_reg_HDR_VS[0].data = Again;//high bit
    params->tGain_reg_HDR_VS[1].data = (gain16<<4)&0xf0; //low byte
    params->tGain_reg_HDR_VS[2].data = 0x01; //low byte
    params->tGain_reg_HDR_VS[3].data = (gain1024&0x3fc)>>2;
    params->tGain_reg_HDR_VS[4].data = (gain1024&0x03); //low byte

    SENSOR_DMSG("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
    //CamOsPrintf("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}
#endif
static int pCus_GetFPS_HDR_LEF(ss_cus_sensor *handle)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
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

#define AE_NOUSE 0
#define AE_1st_set 1
#define AE_2nd_set 2
#define AE_3rd_set 3
#define frame0_shutter 0
#define frame1_shutter 4
#define frame2_shutter 8
#define frame3_shutter 12
#define frame0_gain 16
#define frame1_gain 20
#define frame2_gain 24
#define frame3_gain 28

static u32 pCus_GetShutterGainShare(ss_cus_sensor *handle, int eHDRType)
{
    /*
    typedef enum
    {
        E_MHAL_HDR_TYPE_OFF,
        E_MHAL_HDR_TYPE_VC,                 //virtual channel mode HDR, vc0->long, vc1->short
        E_MHAL_HDR_TYPE_DOL,
        E_MHAL_HDR_TYPE_EMBEDDED,  //compressed HDR mode
        E_MHAL_HDR_TYPE_COMP = 3,
        E_MHAL_HDR_TYPE_LI,                //Line interlace HDR
        E_MHAL_HDR_TYPE_DCG,        // < Dual conversion gain
        //E_MHAL_HDR_TYPE_VC3,
        //E_MHAL_HDR_TYPE_DOL3,
        E_MHAL_HDR_TYPE_COMP_VS,
        E_MHAL_HDR_TYPE_MAX
    } MHalHDRType_e;

    */
    /*
    SG_share explain :
    bit0~16 descript shutter, bit16~31 descript gain
    bit0, bit16 ->frame 0
    bit4, bit20 ->frame 1
    bit8, bit24 ->frame 2
    bit12,bit28 ->frame 3
    */
    u32 SG_share=0;

    if(eHDRType == 0) //E_MHAL_HDR_TYPE_OFF  Linear
    {
        SG_share |= AE_1st_set << frame0_shutter;
        SG_share |= AE_1st_set << frame0_gain;
        SENSOR_DMSG("Linear mode  SG_share code is %d \n",SG_share);
    }
    else if(eHDRType == 1)//E_MHAL_HDR_TYPE_VC
    {
        SENSOR_DMSG("VC mode is not supported\n");
        return FAIL;
    }
    else if(eHDRType == 2)//E_MHAL_HDR_TYPE_DOL
    {
        SENSOR_DMSG("DOL mode is not supported\n");
        return FAIL;
    }
    else if(eHDRType == 3)//E_MHAL_HDR_TYPE_COMP
    {
        SG_share |= AE_1st_set << frame1_shutter;
        SG_share |= AE_1st_set << frame2_shutter;
        SG_share |= AE_1st_set << frame1_gain;
        SG_share |= AE_2nd_set << frame2_gain;
        SENSOR_DMSG("COMP mode  SG_share code is %d\n",SG_share);
    }
    else if(eHDRType == 4)//E_MHAL_HDR_TYPE_LI
    {
        SENSOR_DMSG("Line interlace mode is not supported\n");
        return FAIL;
    }
    else if(eHDRType == 5)//E_MHAL_HDR_TYPE_DCG
    {
        SENSOR_DMSG("DCG mode is not supported\n");
        return FAIL;
    }

    else if(eHDRType == 6)//E_MHAL_HDR_TYPE_COMP_VS
    {
        SG_share |= AE_1st_set << frame1_shutter;
        SG_share |= AE_1st_set << frame2_shutter;
        SG_share |= AE_3rd_set << frame3_shutter;
        SG_share |= AE_1st_set << frame1_gain;
        SG_share |= AE_2nd_set << frame2_gain;
        SG_share |= AE_3rd_set << frame3_gain;
        SENSOR_DMSG("COMP + VS mode  SG_share code is %d\n",SG_share);
    }
    else
    {
        SENSOR_DMSG("This mode is not supported\n");
        return FAIL;
    }

    return SG_share;

}

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OS04A10_params *params;
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

    params = (OS04A10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    //memcpy(params->tExpo_reg_HDR_VS, expo_reg_HDR_VS, sizeof(expo_reg_HDR_VS));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tMirror_reg_HDR, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"OS04A10_MIPI");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC;
    handle->bayer_id                                              = SENSOR_BAYERID;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;

    ////////////////////////////////////////////////////
    // Sensor i2c configuration
    ////////////////////////////////////////////////////
    handle->i2c_cfg.mode                                          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                                           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address                                       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed                                         = SENSOR_I2C_SPEED;

    ////////////////////////////////////////////////////
    // Sensor Power configuration
    ////////////////////////////////////////////////////
    handle->pCus_sensor_poweron                                   = pCus_poweron;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    handle->mclk                                                  = Preview_MCLK_SPEED;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = OS04A10_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = OS04A10_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = OS04A10_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = OS04A10_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = OS04A10_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = OS04A10_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = OS04A10_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = OS04A10_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, OS04A10_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = OS04A10_SetPatternMode;
    handle->pCus_sensor_Get_ShutterGainShare                      = pCus_GetShutterGainShare;
    handle->pCus_sensor_CustDefineFunction                        = pCus_sensor_CustDefineFunction;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    Preview_line_period = 26667;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/OS04A10_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps;
    params->expo.fps                                              = 30;
    params->expo.line                                             = 100;
    params->dirty                                                 = false;
    params->orien_dirty                                             = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_SEF(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OS04A10_params *params = NULL;
    s32 res;
    //int res;

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

    params = (OS04A10_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"OS04A10_MIPI_HDR_SEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DCG_HDR;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_COMP;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1;

    ////////////////////////////////////////////////////
    // Sensor i2c configuration
    ////////////////////////////////////////////////////
    handle->i2c_cfg.mode                                          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                                           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address                                       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed                                         = SENSOR_I2C_SPEED;

    ////////////////////////////////////////////////////
    // Sensor Power configuration
    ////////////////////////////////////////////////////
    //handle->pCus_sensor_poweron                                   = pCus_poweron;
    //handle->pCus_sensor_poweroff                                  = pCus_poweroff;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    //handle->mclk                                                  = Preview_MCLK_SPEED_HDR;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = OS04A10_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = OS04A10_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = OS04A10_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = OS04A10_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = OS04A10_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = OS04A10_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = OS04A10_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = OS04A10_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, OS04A10_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    //handle->pCus_sensor_init                                      = pCus_init_16bit_1520_DCG_HDR;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    //handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR;

    //handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    //handle->pCus_sensor_SetOrien                                  = pCus_SetOrien_HDR;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_SEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify_HDR_LEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs_HDR_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain_HDR_SEF;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain_HDR_SEF;
    handle->pCus_sensor_Get_ShutterGainShare                      = pCus_GetShutterGainShare;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    Preview_line_period = 26667;
    params->expo.max_short=105;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period * params->expo.max_short;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

static int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OS04A10_params *params;
    s32 res;
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }

    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (OS04A10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tGain_reg_HDR_VS, gain_reg_HDR_VS, sizeof(gain_reg_HDR_VS));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    //memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tMirror_reg_HDR, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"OS04A10_MIPI_HDR_LEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DCG_HDR;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_COMP;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;

    ////////////////////////////////////////////////////
    // Sensor i2c configuration
    ////////////////////////////////////////////////////
    handle->i2c_cfg.mode                                          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                                           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address                                       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed                                         = SENSOR_I2C_SPEED;

    ////////////////////////////////////////////////////
    // Sensor Power configuration
    ////////////////////////////////////////////////////
    handle->pCus_sensor_poweron                                   = pCus_poweron;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    handle->mclk                                                  = Preview_MCLK_SPEED_HDR;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = OS04A10_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = OS04A10_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = OS04A10_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = OS04A10_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = OS04A10_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = OS04A10_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = OS04A10_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = OS04A10_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, OS04A10_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_16bit_1520_DCG_HDR;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_LEF;
    handle->pCus_sensor_SetFPS                                    = NULL;
    handle->pCus_sensor_SetPatternMode                            = OS04A10_SetPatternMode;
    handle->pCus_sensor_Get_ShutterGainShare                      = pCus_GetShutterGainShare;
    handle->pCus_sensor_CustDefineFunction                        = pCus_sensor_CustDefineFunction;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = NULL;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs_HDR_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps_HDR;
    params->expo.fps                                              = 20;
    params->expo.max_short                                        = 105;
    params->dirty                                                 = false;
    params->orien_dirty                                             = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    Preview_line_period = 26667;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/OS04A10_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    return SUCCESS;
}

#if 0
int cus_camsensor_init_handle_HDR_VS(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    OS04A10_params *params;
    s32 res;

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
    params = (OS04A10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tGain_reg_HDR_VS, gain_reg_HDR_VS, sizeof(gain_reg_HDR_VS));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_VS, expo_reg_HDR_VS, sizeof(expo_reg_HDR_VS));
    memcpy(params->tMirror_reg, mirror_flip_reg, sizeof(mirror_flip_reg));
    memcpy(params->tMirror_reg_HDR, mirror_flip_HDR, sizeof(mirror_flip_HDR));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->model_id,"OS04A10_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;
    handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    params->res.orit      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_COMP_VS;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Long frame

    handle->pCus_sensor_init        = pCus_init_DCG_VS_HDR;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    //handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;

    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < 2; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].width         = OS04A10_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].height        = OS04A10_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].max_fps       = OS04A10_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].min_fps       = OS04A10_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].crop_start_x  = OS04A10_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].crop_start_y  = OS04A10_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].nOutputWidth  = OS04A10_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].nOutputHeight = OS04A10_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, "%s", OS04A10_mipi_hdr[res].senstr.strResDesc);
    }



    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //I2C_FMT_A16D8;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x6c;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //polarity
    /////////////////////////////////////////////////////
    params->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////


    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->ae_gain_delay       = 2;
    handle->ae_shutter_delay    = 2;

    handle->ae_gain_ctrl_num = 2;
    handle->ae_shutter_ctrl_num = 2;

    ///calibration
    handle->sat_mingain=g_sensor_ae_min_gain;


    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    //handle->pCus_sensor_init        = pCus_init_HDR_LEF;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;//pCus_poweroff_HDR_LEF;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID_HDR_LEF;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;//NULL;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;//NULL;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;//NULL;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien_HDR;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_LEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_LEF;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = OS04A10_SetPatternMode;
    handle->pCus_sensor_Get_ShutterGainShare = pCus_GetShutterGainShare;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_HDR_VS;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_VS;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_VS;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_HDR_VS;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_HDR_VS;

    handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;

     //sensor calibration
    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal_HDR_LEF;
    handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity_HDR_LEF;
    handle->pCus_sensor_GetShutterInfo = OS04A10_GetShutterInfo_HDR_LEF;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    params->expo.vts=vts_30fps_HDR;
    params->expo.fps = 30;//25;
    params->expo.max_short=105;
    params->dirty = false;
    params->ori_dirty = false;

    CamOsPrintf("\n[%s]\n",__FUNCTION__);
    return SUCCESS;
}
#endif

SENSOR_DRV_ENTRY_IMPL_END_EX2(  OS04A10_DCG,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_HDR_SEF,
                            cus_camsensor_init_handle_HDR_LEF,
                            NULL,
                            OS04A10_params
                         );
