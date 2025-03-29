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
   Date : 2023/10/31
   Build on : Master V4  I6C
   Verified on : mixer preview ok (linear/hdr),
               AE gain/shutter ok , IQ not verify.
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OV4689_HDR);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (4)
#define SENSOR_MIPI_LANE_NUM_HDR (4)
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

#define SENSOR_IFBUS_TYPE          CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC            CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR        CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_BAYERID             CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR         CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID             CUS_RGBIR_NONE
#define SENSOR_ORIT                CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define SENSOR_MAX_GAIN            128*1024//(16*15.99)    // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN            1024//(1*1024)          // min sensor again
#define SENSOR_GAIN_DELAY_FRAME_COUNT      (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (2)
#define SENSOR_SHUTTER_CTRL_NUM               (2)

#define Preview_MCLK_SPEED         CUS_CMU_CLK_24MHZ       //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR     CUS_CMU_CLK_24MHZ       //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
//#define Preview_line_period        19996//21177//17814     // MCLK=21.6 HTS/PCLK=3080 pixels/97.2MHZ=31.687us                              // 3126 for 25fps
#define Preview_line_period_DCG    21929//19817//17814     // MCLK=21.6 HTS/PCLK=3080 pixels/97.2MHZ=31.687us                              // 3126 for 25fps
//#define vts_30fps                  1667//1574//1770        // VTS for 30fps
#define vts_30fps_DCG              1520//1480//3000        // VTS for 25fps
u32 Preview_line_period = 19996;
u32 vts_30fps = 1667;

#define Preview_WIDTH              2560                    //resolution Width when preview
#define Preview_HEIGHT             1440                    //resolution Height when preview
//#define Preview_MAX_FPS            30                      //fastest preview FPS
//#define Preview_MAX_FPS_HDR        30                      //fastest preview FPS
#define Preview_MIN_FPS            3                       //slowest preview FPS

#define SENSOR_I2C_ADDR            0x6c                    //I2C slave address
#define SENSOR_I2C_SPEED           200000 //300000// 240000//I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY          I2C_NORMAL_MODE         //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT             I2C_FMT_A16D8           //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL            CUS_CLK_POL_NEG         // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL             CUS_CLK_POL_NEG         // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

//VSYNC/HSYNC POL can be found in data sheet timing diagram
//Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL           CUS_CLK_POL_NEG         // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL           CUS_CLK_POL_POS         // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL            CUS_CLK_POL_POS         // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
static int OV4689_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int OV4689_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int OV4689_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
static int OV4689_SetOrien_HDR(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
static int OV4689_SetAEUSecs_HDR_lef(ss_cus_sensor *handle, u32 us);
static int OV4689_SetAEUSecs_HDR_sef(ss_cus_sensor *handle, u32 us);
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
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tGain_vc1_reg[4];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tExpo_vc0_reg[3];
    I2C_ARRAY tExpo_vc1_reg[2];
    I2C_ARRAY tMirror_reg[2];
    I2C_ARRAY tMirror_reg_HDR[2];
    CUS_CAMSENSOR_ORIT cur_orien;
} ov4689_params;

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2 = 1, LINEAR_RES_END}mode;
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
}ov4689_mipi_linear[] = {
    {LINEAR_RES_1, {2560, 1440, 3, 30}, {0, 0, 2560, 1440}, {"2560x1440@30fps"}},
    {LINEAR_RES_2, {1920, 1080, 3, 60}, {0, 0, 1920, 1080}, {"1920x1080@60fps"}},};

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
}ov4689_mipi_hdr[] = {
   {HDR_RES_1, {2560, 1440, 3, 30}, {0, 0, 2560, 1440}, {"2560x1440@30fps_HDR"}}, // Modify it
};

const static I2C_ARRAY Sensor_init_table[] =
{
//@@ 4689 RES_2560x1440_4LANE_768Mbps(30fps)_24M
    {0x0103,0x01},
    {0x3638,0x00},
    {0x0300,0x00},
    {0x0302,0x20},
    {0x0303,0x00},
    {0x0304,0x03},
    {0x030b,0x00},
    {0x030d,0x1e},
    {0x030e,0x04},
    {0x030f,0x01},
    {0x0312,0x01},
    {0x031e,0x00},
    {0x3000,0x20},
    {0x3002,0x00},
    {0x3018,0x72},
    {0x3020,0x93},
    {0x3021,0x03},
    {0x3022,0x01},
    {0x3031,0x0a},
    {0x303f,0x0c},
    {0x3305,0xf1},
    {0x3307,0x04},
    {0x3309,0x29},
    {0x3500,0x00},
    {0x3501,0x60},
    {0x3502,0x00},
    {0x3503,0x04},
    {0x3504,0x00},
    {0x3505,0x00},
    {0x3506,0x00},
    {0x3507,0x00},
    {0x3508,0x00},
    {0x3509,0x80},
    {0x350a,0x00},
    {0x350b,0x00},
    {0x350c,0x00},
    {0x350d,0x00},
    {0x350e,0x00},
    {0x350f,0x80},
    {0x3510,0x00},
    {0x3511,0x00},
    {0x3512,0x00},
    {0x3513,0x00},
    {0x3514,0x00},
    {0x3515,0x80},
    {0x3516,0x00},
    {0x3517,0x00},
    {0x3518,0x00},
    {0x3519,0x00},
    {0x351a,0x00},
    {0x351b,0x80},
    {0x351c,0x00},
    {0x351d,0x00},
    {0x351e,0x00},
    {0x351f,0x00},
    {0x3520,0x00},
    {0x3521,0x80},
    {0x3522,0x08},
    {0x3524,0x08},
    {0x3526,0x08},
    {0x3528,0x08},
    {0x352a,0x08},
    {0x3602,0x00},
    {0x3603,0x40},
    {0x3604,0x02},
    {0x3605,0x00},
    {0x3606,0x00},
    {0x3607,0x00},
    {0x3609,0x12},
    {0x360a,0x40},
    {0x360c,0x08},
    {0x360f,0xe5},
    {0x3608,0x8f},
    {0x3611,0x00},
    {0x3613,0xf7},
    {0x3616,0x58},
    {0x3619,0x99},
    {0x361b,0x60},
    {0x361c,0x7a},
    {0x361e,0x79},
    {0x361f,0x02},
    {0x3632,0x00},
    {0x3633,0x10},
    {0x3634,0x10},
    {0x3635,0x10},
    {0x3636,0x15},
    {0x3646,0x86},
    {0x364a,0x0b},
    {0x3700,0x17},
    {0x3701,0x22},
    {0x3703,0x10},
    {0x370a,0x37},
    {0x3705,0x00},
    {0x3706,0x63},
    {0x3709,0x3c},
    {0x370b,0x01},
    {0x370c,0x30},
    {0x3710,0x24},
    {0x3711,0x0c},
    {0x3716,0x00},
    {0x3720,0x28},
    {0x3729,0x7b},
    {0x372a,0x84},
    {0x372b,0xbd},
    {0x372c,0xbc},
    {0x372e,0x52},
    {0x373c,0x0e},
    {0x373e,0x33},
    {0x3743,0x10},
    {0x3744,0x88},
    {0x3745,0xc0},
    {0x374a,0x43},
    {0x374c,0x00},
    {0x374e,0x23},
    {0x3751,0x7b},
    {0x3752,0x84},
    {0x3753,0xbd},
    {0x3754,0xbc},
    {0x3756,0x52},
    {0x375c,0x00},
    {0x3760,0x00},
    {0x3761,0x00},
    {0x3762,0x00},
    {0x3763,0x00},
    {0x3764,0x00},
    {0x3767,0x04},
    {0x3768,0x04},
    {0x3769,0x08},
    {0x376a,0x08},
    {0x376b,0x20},
    {0x376c,0x00},
    {0x376d,0x00},
    {0x376e,0x00},
    {0x3773,0x00},
    {0x3774,0x51},
    {0x3776,0xbd},
    {0x3777,0xbd},
    {0x3781,0x18},
    {0x3783,0x25},
    {0x3798,0x1b},
    {0x3800,0x00},
    {0x3801,0x08},
    {0x3802,0x00},
    {0x3803,0x04},
    {0x3804,0x0a},
    {0x3805,0x97},
    {0x3806,0x05},
    {0x3807,0xfb},
    {0x3808,0x0a},
    {0x3809,0x80},
    {0x380a,0x05},
    {0x380b,0xf0},
    {0x380c,0x09},
    {0x380d,0x5f},
    {0x380e,0x06},
    {0x380f,0x83},
    {0x3810,0x00},
    {0x3811,0x08},
    {0x3812,0x00},
    {0x3813,0x04},
    {0x3814,0x01},
    {0x3815,0x01},
    {0x3819,0x01},
    {0x3820,0x00},
    {0x3821,0x06},
    {0x3829,0x00},
    {0x382a,0x01},
    {0x382b,0x01},
    {0x382d,0x7f},
    {0x3830,0x04},
    {0x3836,0x01},
    {0x3837,0x00},
    {0x3841,0x02},
    {0x3846,0x08},
    {0x3847,0x07},
    {0x3d85,0x36},
    {0x3d8c,0x71},
    {0x3d8d,0xcb},
    {0x3f0a,0x00},
    {0x4000,0xf1},
    {0x4001,0x40},
    {0x4002,0x04},
    {0x4003,0x14},
    {0x400e,0x00},
    {0x4011,0x00},
    {0x401a,0x00},
    {0x401b,0x00},
    {0x401c,0x00},
    {0x401d,0x00},
    {0x401f,0x00},
    {0x4020,0x00},
    {0x4021,0x10},
    {0x4022,0x07},
    {0x4023,0xcf},
    {0x4024,0x09},
    {0x4025,0x60},
    {0x4026,0x09},
    {0x4027,0x6f},
    {0x4028,0x00},
    {0x4029,0x02},
    {0x402a,0x06},
    {0x402b,0x04},
    {0x402c,0x02},
    {0x402d,0x02},
    {0x402e,0x0e},
    {0x402f,0x04},
    {0x4302,0xff},
    {0x4303,0xff},
    {0x4304,0x00},
    {0x4305,0x00},
    {0x4306,0x00},
    {0x4308,0x02},
    {0x4500,0x6c},
    {0x4501,0xc4},
    {0x4502,0x40},
    {0x4503,0x01},
    {0x4601,0xA7},
    {0x4800,0x04},
    {0x4813,0x08},
    {0x481f,0x40},
    {0x4829,0x78},
    {0x4837,0x14},
    {0x4b00,0x2a},
    {0x4b0d,0x00},
    {0x4d00,0x04},
    {0x4d01,0x42},
    {0x4d02,0xd1},
    {0x4d03,0x93},
    {0x4d04,0xf5},
    {0x4d05,0xc1},
    {0x5000,0xf3},
    {0x5001,0x11},
    {0x5004,0x00},
    {0x500a,0x00},
    {0x500b,0x00},
    {0x5032,0x00},
    {0x5040,0x00},
    {0x5050,0x0c},
    {0x5500,0x00},
    {0x5501,0x10},
    {0x5502,0x01},
    {0x5503,0x0f},
    {0x8000,0x00},
    {0x8001,0x00},
    {0x8002,0x00},
    {0x8003,0x00},
    {0x8004,0x00},
    {0x8005,0x00},
    {0x8006,0x00},
    {0x8007,0x00},
    {0x8008,0x00},
    {0x3638,0x00},
    {0x3800,0x00},
    {0x3801,0x48},
    {0x3802,0x00},
    {0x3803,0x2C},
    {0x3804,0x0A},
    {0x3805,0x57},
    {0x3806,0x05},
    {0x3807,0xD3},
    {0x3808,0x0A},
    {0x3809,0x00},
    {0x380A,0x05},
    {0x380B,0xA0},
    {0x3810,0x00},
    {0x3811,0x08},
    {0x3812,0x00},
    {0x3813,0x04},
    {0x4020,0x00},
    {0x4021,0x10},
    {0x4022,0x08},
    {0x4023,0x93},
    {0x4024,0x09},
    {0x4025,0xC0},
    {0x4026,0x09},
    {0x4027,0xD0},
    {0x4600,0x00},
    {0x4601,0x9F},
    {0x0100,0x01},
};

const static I2C_ARRAY Sensor_init_table_2M60FPS[] =
{
    {0x0103, 0x01},
    {0x3638, 0x00},
    {0x0300, 0x00},
    {0x0302, 0x20},
    {0x0303, 0x00},
    {0x0304, 0x03},
    {0x030b, 0x00},
    {0x030d, 0x1e},
    {0x030e, 0x04},
    {0x030f, 0x01},
    {0x0312, 0x01},
    {0x031e, 0x00},
    {0x3000, 0x20},
    {0x3002, 0x00},
    {0x3018, 0x72},
    {0x3020, 0x93},
    {0x3021, 0x03},
    {0x3022, 0x01},
    {0x3031, 0x0a},
    {0x3305, 0xf1},
    {0x3307, 0x04},
    {0x3309, 0x29},
    {0x3500, 0x00},
    {0x3501, 0x48},
    {0x3502, 0x00},
    {0x3503, 0x04},
    {0x3504, 0x00},
    {0x3505, 0x00},
    {0x3506, 0x00},
    {0x3507, 0x00},
    {0x3508, 0x00},
    {0x3509, 0x80},
    {0x350a, 0x00},
    {0x350b, 0x00},
    {0x350c, 0x00},
    {0x350d, 0x00},
    {0x350e, 0x00},
    {0x350f, 0x80},
    {0x3510, 0x00},
    {0x3511, 0x00},
    {0x3512, 0x00},
    {0x3513, 0x00},
    {0x3514, 0x00},
    {0x3515, 0x80},
    {0x3516, 0x00},
    {0x3517, 0x00},
    {0x3518, 0x00},
    {0x3519, 0x00},
    {0x351a, 0x00},
    {0x351b, 0x80},
    {0x351c, 0x00},
    {0x351d, 0x00},
    {0x351e, 0x00},
    {0x351f, 0x00},
    {0x3520, 0x00},
    {0x3521, 0x80},
    {0x3522, 0x08},
    {0x3524, 0x08},
    {0x3526, 0x08},
    {0x3528, 0x08},
    {0x352a, 0x08},
    {0x3602, 0x00},
    {0x3604, 0x02},
    {0x3605, 0x00},
    {0x3606, 0x00},
    {0x3607, 0x00},
    {0x3609, 0x12},
    {0x360a, 0x40},
    {0x360c, 0x08},
    {0x360f, 0xe5},
    {0x3608, 0x8f},
    {0x3611, 0x00},
    {0x3613, 0xf7},
    {0x3616, 0x58},
    {0x3619, 0x99},
    {0x361b, 0x60},
    {0x361c, 0x7a},
    {0x361e, 0x79},
    {0x361f, 0x02},
    {0x3632, 0x00},
    {0x3633, 0x10},
    {0x3634, 0x10},
    {0x3635, 0x10},
    {0x3636, 0x15},
    {0x3646, 0x86},
    {0x364a, 0x0b},
    {0x3700, 0x17},
    {0x3701, 0x22},
    {0x3703, 0x10},
    {0x370a, 0x37},
    {0x3705, 0x00},
    {0x3706, 0x63},
    {0x3709, 0x3c},
    {0x370b, 0x01},
    {0x370c, 0x30},
    {0x3710, 0x24},
    {0x3711, 0x0c},
    {0x3716, 0x00},
    {0x3720, 0x28},
    {0x3729, 0x7b},
    {0x372a, 0x84},
    {0x372b, 0xbd},
    {0x372c, 0xbc},
    {0x372e, 0x52},
    {0x373c, 0x0e},
    {0x373e, 0x33},
    {0x3743, 0x10},
    {0x3744, 0x88},
    {0x374a, 0x43},
    {0x374c, 0x00},
    {0x374e, 0x23},
    {0x3751, 0x7b},
    {0x3752, 0x84},
    {0x3753, 0xbd},
    {0x3754, 0xbc},
    {0x3756, 0x52},
    {0x375c, 0x00},
    {0x3760, 0x00},
    {0x3761, 0x00},
    {0x3762, 0x00},
    {0x3763, 0x00},
    {0x3764, 0x00},
    {0x3767, 0x04},
    {0x3768, 0x04},
    {0x3769, 0x08},
    {0x376a, 0x08},
    {0x376b, 0x20},
    {0x376c, 0x00},
    {0x376d, 0x00},
    {0x376e, 0x00},
    {0x3773, 0x00},
    {0x3774, 0x51},
    {0x3776, 0xbd},
    {0x3777, 0xbd},
    {0x3781, 0x18},
    {0x3783, 0x25},
    {0x3800, 0x01},
    {0x3801, 0x88},
    {0x3802, 0x00},
    {0x3803, 0xe0},
    {0x3804, 0x09},
    {0x3805, 0x17},
    {0x3806, 0x05},
    {0x3807, 0x1f},
    {0x3808, 0x07},
    {0x3809, 0x80},
    {0x380a, 0x04},
    {0x380b, 0x38},
    {0x380c, 0x06},//05
    {0x380d, 0x3e},//0a
    {0x380e, 0x04},//06
    {0x380f, 0xe2},//0d
    {0x3810, 0x00},
    {0x3811, 0x08},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3819, 0x01},
    {0x3820, 0x00},
    {0x3821, 0x06},
    {0x3829, 0x00},
    {0x382a, 0x01},
    {0x382b, 0x01},
    {0x382d, 0x7f},
    {0x3830, 0x04},
    {0x3836, 0x01},
    {0x3841, 0x02},
    {0x3846, 0x08},
    {0x3847, 0x07},
    {0x3d85, 0x36},
    {0x3d8c, 0x71},
    {0x3d8d, 0xcb},
    {0x3f0a, 0x00},
    {0x4000, 0x71},
    {0x4001, 0x40},
    {0x4002, 0x04},
    {0x4003, 0x14},
    {0x400e, 0x00},
    {0x4011, 0x00},
    {0x401a, 0x00},
    {0x401b, 0x00},
    {0x401c, 0x00},
    {0x401d, 0x00},
    {0x401f, 0x00},
    {0x4020, 0x00},
    {0x4021, 0x10},
    {0x4022, 0x06},
    {0x4023, 0x13},
    {0x4024, 0x07},
    {0x4025, 0x40},
    {0x4026, 0x07},
    {0x4027, 0x50},
    {0x4028, 0x00},
    {0x4029, 0x02},
    {0x402a, 0x06},
    {0x402b, 0x04},
    {0x402c, 0x02},
    {0x402d, 0x02},
    {0x402e, 0x0e},
    {0x402f, 0x04},
    {0x4302, 0xff},
    {0x4303, 0xff},
    {0x4304, 0x00},
    {0x4305, 0x00},
    {0x4306, 0x00},
    {0x4308, 0x02},
    {0x4500, 0x6c},
    {0x4501, 0xc4},
    {0x4502, 0x40},
    {0x4503, 0x02},
    {0x4601, 0x77},
    {0x4800, 0x04},
    {0x4813, 0x08},
    {0x481f, 0x40},
    {0x4829, 0x78},
    {0x4837, 0x14},
    {0x4b00, 0x2a},
    {0x4b0d, 0x00},
    {0x4d00, 0x04},
    {0x4d01, 0x42},
    {0x4d02, 0xd1},
    {0x4d03, 0x93},
    {0x4d04, 0xf5},
    {0x4d05, 0xc1},
    {0x5000, 0xf3},
    {0x5001, 0x11},
    {0x5004, 0x00},
    {0x500a, 0x00},
    {0x500b, 0x00},
    {0x5032, 0x00},
    {0x5040, 0x00},
    {0x5050, 0x0c},
    {0x5500, 0x00},
    {0x5501, 0x10},
    {0x5502, 0x01},
    {0x5503, 0x0f},
    {0x8000, 0x00},
    {0x8001, 0x00},
    {0x8002, 0x00},
    {0x8003, 0x00},
    {0x8004, 0x00},
    {0x8005, 0x00},
    {0x8006, 0x00},
    {0x8007, 0x00},
    {0x8008, 0x00},
    {0x3638, 0x00},
    {0x3105, 0x31},
    {0x301a, 0xf9},
    {0x3508, 0x07},
    {0x484b, 0x05},
    {0x4805, 0x03},
    {0x3601, 0x01},
    {0x3745, 0xc0},
    {0x3798, 0x1b},
    {0x3105, 0x11},
    {0x301a, 0xf1},
    {0x4805, 0x00},
    {0x301a, 0xf0},
    {0x3208, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x3601, 0x00},
    {0x3638, 0x00},
    {0x3208, 0x10},
    {0x3208, 0xa0},
    {0x0100, 0x01},
};

const static I2C_ARRAY Sensor_init_table_DCG[] =
{
    {0x0103, 0x01},
    {0x3638, 0x00},
    {0x0300, 0x00},
    {0x0302, 0x26},//mipi rate 1a:684Mbps should change reg0x4837
    {0x0303, 0x00},
    {0x0304, 0x03},
    {0x030b, 0x00},
    {0x030d, 0x1f},//1f
    {0x030e, 0x04},
    {0x030f, 0x01},
    {0x0312, 0x01},
    {0x031e, 0x00},
    {0x3000, 0x20},
    {0x3002, 0x00},
    {0x3018, 0x72},
    {0x3020, 0x93},
    {0x3021, 0x03},
    {0x3022, 0x01},
    {0x3031, 0x0a},
    {0x303f, 0x0c},
    {0x3305, 0xf1},
    {0x3307, 0x04},
    {0x3309, 0x29},
    {0x3500, 0x00},
    {0x3501, 0x00},
    {0x3502, 0x10},
    {0x3503, 0x04},
    {0x3504, 0x00},
    {0x3505, 0x00},
    {0x3506, 0x00},
    {0x3507, 0x00},
    {0x3508, 0x07},
    {0x3509, 0x80},
    {0x350a, 0x00},
    {0x350b, 0x01},
    {0x350c, 0x00},
    {0x350d, 0x00},
    {0x350e, 0x00},
    {0x350f, 0x80},
    {0x3510, 0x00},
    {0x3511, 0x00},
    {0x3512, 0x80},
    {0x3513, 0x00},
    {0x3514, 0x00},
    {0x3515, 0x80},
    {0x3516, 0x00},
    {0x3517, 0x00},
    {0x3518, 0x00},
    {0x3519, 0x00},
    {0x351a, 0x00},
    {0x351b, 0x80},
    {0x351c, 0x00},
    {0x351d, 0x00},
    {0x351e, 0x00},
    {0x351f, 0x00},
    {0x3520, 0x00},
    {0x3521, 0x80},
    {0x3522, 0x08},
    {0x3524, 0x08},
    {0x3526, 0x08},
    {0x3528, 0x08},
    {0x352a, 0x08},
    {0x3602, 0x00},
    {0x3603, 0x40},
    {0x3604, 0x02},
    {0x3605, 0x00},
    {0x3606, 0x00},
    {0x3607, 0x00},
    {0x3609, 0x12},
    {0x360a, 0x40},
    {0x360c, 0x08},
    {0x360f, 0xe5},
    {0x3608, 0x8f},
    {0x3611, 0x00},
    {0x3613, 0xf7},
    {0x3616, 0x58},
    {0x3619, 0x99},
    {0x361b, 0x60},
    {0x361c, 0x7a},
    {0x361e, 0x79},
    {0x361f, 0x02},
    {0x3632, 0x00},
    {0x3633, 0x10},
    {0x3634, 0x10},
    {0x3635, 0x10},
    {0x3636, 0x15},
    {0x3646, 0x86},
    {0x364a, 0x0b},
    {0x3700, 0x17},
    {0x3701, 0x22},
    {0x3703, 0x10},
    {0x370a, 0x37},
    {0x3705, 0x00},
    {0x3706, 0x63},
    {0x3709, 0x3c},
    {0x370b, 0x01},
    {0x370c, 0x30},
    {0x3710, 0x24},
    {0x3711, 0x0c},
    {0x3716, 0x00},
    {0x3720, 0x28},
    {0x3729, 0x7b},
    {0x372a, 0x84},
    {0x372b, 0xbd},
    {0x372c, 0xbc},
    {0x372e, 0x52},
    {0x373c, 0x0e},
    {0x373e, 0x33},
    {0x3743, 0x10},
    {0x3744, 0x88},
    {0x3745, 0xc0},
    {0x374a, 0x43},
    {0x374c, 0x00},
    {0x374e, 0x23},
    {0x3751, 0x7b},
    {0x3752, 0x84},
    {0x3753, 0xbd},
    {0x3754, 0xbc},
    {0x3756, 0x52},
    {0x375c, 0x00},
    {0x3760, 0x00},
    {0x3761, 0x00},
    {0x3762, 0x00},
    {0x3763, 0x00},
    {0x3764, 0x00},
    {0x3767, 0x04},
    {0x3768, 0x04},
    {0x3769, 0x08},
    {0x376a, 0x08},
    {0x376b, 0x20},
    {0x376c, 0x00},
    {0x376d, 0x00},
    {0x376e, 0x00},
    {0x3773, 0x00},
    {0x3774, 0x51},
    {0x3776, 0xbd},
    {0x3777, 0xbd},
    {0x3781, 0x18},
    {0x3783, 0x25},
    {0x3798, 0x1b},
    {0x3800, 0x00},
    {0x3801, 0x48},
    {0x3802, 0x00},
    {0x3803, 0x2C},
    {0x3804, 0x0A},
    {0x3805, 0x57},
    {0x3806, 0x05},
    {0x3807, 0xD3},
    {0x3808, 0x0A},
    {0x3809, 0x00},
    {0x380A, 0x05},
    {0x380B, 0xA0},
    {0x380C, 0x05},//05
    {0x380D, 0x4f},//74
    {0x380E, 0x05},//05
    {0x380F, 0xf0},//c8
    {0x3810, 0x00},
    {0x3811, 0x08},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3819, 0x01},
    {0x3820, 0x00},
    {0x3821, 0x06},
    {0x3829, 0x00},
    {0x382a, 0x01},
    {0x382b, 0x01},
    {0x382d, 0x7f},
    {0x3830, 0x04},
    {0x3836, 0x01},
    {0x3837, 0x00},
    {0x3841, 0x02},
    {0x3846, 0x08},
    {0x3847, 0x07},
    {0x3d85, 0x36},
    {0x3d8c, 0x71},
    {0x3d8d, 0xcb},
    {0x3f0a, 0x00},
    {0x4000, 0xf1},
    {0x4001, 0x40},
    {0x4002, 0x04},
    {0x4003, 0x14},
    {0x400e, 0x00},
    {0x4011, 0x00},
    {0x401a, 0x00},
    {0x401b, 0x00},
    {0x401c, 0x00},
    {0x401d, 0x00},
    {0x401f, 0x00},
    {0x4020, 0x00},
    {0x4021, 0x10},
    {0x4022, 0x08},
    {0x4023, 0x93},
    {0x4024, 0x09},
    {0x4025, 0xC0},
    {0x4026, 0x09},
    {0x4027, 0xD0},
    {0x4028, 0x00},
    {0x4029, 0x02},
    {0x402a, 0x06},
    {0x402b, 0x04},
    {0x402c, 0x02},
    {0x402d, 0x02},
    {0x402e, 0x0e},
    {0x402f, 0x04},
    {0x4302, 0xff},
    {0x4303, 0xff},
    {0x4304, 0x00},
    {0x4305, 0x00},
    {0x4306, 0x00},
    {0x4308, 0x02},
    {0x4500, 0x6c},
    {0x4501, 0xc4},
    {0x4502, 0x40},
    {0x4503, 0x01},
    {0x4600, 0x00},
    {0x4601, 0x9F},
    {0x4800, 0x04},
    {0x4813, 0x08},
    {0x481f, 0x40},
    {0x4829, 0x78},
    {0x4837, 0x12}, //1a
    {0x4b00, 0x2a},
    {0x4b0d, 0x00},
    {0x4d00, 0x04},
    {0x4d01, 0x42},
    {0x4d02, 0xd1},
    {0x4d03, 0x93},
    {0x4d04, 0xf5},
    {0x4d05, 0xc1},
    {0x5000, 0xf3},
    {0x5001, 0x11},
    {0x5004, 0x00},
    {0x500a, 0x00},
    {0x500b, 0x00},
    {0x5032, 0x00},
    {0x5040, 0x00},
    {0x5050, 0x0c},
    {0x5500, 0x00},
    {0x5501, 0x10},
    {0x5502, 0x01},
    {0x5503, 0x0f},
    {0x8000, 0x00},
    {0x8001, 0x00},
    {0x8002, 0x00},
    {0x8003, 0x00},
    {0x8004, 0x00},
    {0x8005, 0x00},
    {0x8006, 0x00},
    {0x8007, 0x00},
    {0x8008, 0x00},
    {0x3638, 0x00},
    //{0x0100, 0x01},

// ov standard WDR setting-2X
    {0x3841, 0x03},
    {0x3846, 0x08},
    {0x3847, 0x07},
    {0x4800, 0x0C},
    {0x376e, 0x01},
    {0x350B, 0x01},
    {0x3511, 0x00},
    {0x3517, 0x00},
    {0x351d, 0x00},
    {0x3841, 0x03},//@@ 80 81 STG_HDR_2
    {0x3847, 0x06},//@@ 81 812 STG_HDR_2_ALL
    {0x0100,0x01},
};

I2C_ARRAY TriggerStartTbl[] = {
    //{0x0100,0x01},//normal mode
};

const I2C_ARRAY PatternTbl[] = {
    //{0x5081,0x00}, //colorbar pattern , bit 7 to enable
};

/////////////////////////////////////////////////////////////////
//       @@@@@@                                                //
//       @       @@                                            //
//         @@@@                                                //
//                                                             //
//      Step 3 --  complete camera features                    //
//                                                             //
//  camera set EV, MWB, orientation, contrast, sharpness       //
//  saturation, and Denoise can work correctly.                //
//                                                             //
/////////////////////////////////////////////////////////////////

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const I2C_ARRAY mirror_reg[] = {
    {0x3820, 0x00},//M0F0
    {0x3821, 0x06},
};

const I2C_ARRAY mirror_reg_HDR[] = {
    {0x3820, 0x00},//M0F0
    {0x3821, 0x06},
};

const I2C_ARRAY gain_reg[] = {
    //{0x3507, 0x00},//long a-gain [17:16] bit[1:0]
    {0x3508, 0x00},//long a-gain [15:8] bit[7:0]
    {0x3509, 0x80},//long a-gain [7:0] bit[7:0]
    {0x352A, 0x04},// d-gain[14:8]
    {0x352B, 0x00},// d-gain[7:0]
};

const I2C_ARRAY expo_reg[] = {
    {0x3500, 0x00},//long exp[19,16]
    {0x3501, 0x02},//long exp[15,8]
    {0x3502, 0x00},//long exp[7,0] [3:0]fraction bit
};

const I2C_ARRAY vts_reg[] = {
    {0x380e, 0x06},
    {0x380f, 0x66},
};

const static I2C_ARRAY gain_vc1_reg[] = {
    //{0x350d, 0x00},//short gain [17:16] bit[1:0]
    {0x350e, 0x00},//short gain [15:8] bit[7:0]
    {0x350f, 0x80},//short gain [7:0] bit[7:0]
    {0x3522, 0x04},// d-gain[14:8]
    {0x3523, 0x00},// d-gain[7:0]
};

const I2C_ARRAY expo_vc0_reg[] = {
    {0x3500, 0x00},//long
    {0x3501, 0x02},//long
    {0x3502, 0x00},
};

const I2C_ARRAY expo_vc1_reg[] = {
    {0x350b, 0x00},//short
    {0x350c, 0x80},//[3:0]fraction bit
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
#define SENSOR_NAME ov4689

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;

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

static int OV4689_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    sensor_if->MCLK(idx, 1, handle->mclk);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(1000);

    //sensor_if->Set3ATaskOrder(handle, def_order);
    // pure power on
    //ISP_config_io(handle);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(5000);
    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;
}

static int OV4689_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(2000);//mantis:1690203
    return SUCCESS;
}

static int OV4689_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    return SUCCESS;
}

static int OV4689_SetFPS(ss_cus_sensor *handle, u32 fps);
static int OV4689_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);

static int OV4689_init_DCG(ss_cus_sensor *handle)
{
    int i;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    ov4689_params *params = (ov4689_params *)handle->private_data;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_DCG);i++)
    {
        if(Sensor_init_table_DCG[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_DCG[i].data);
        }
        if(SensorReg_Write(Sensor_init_table_DCG[i].reg,Sensor_init_table_DCG[i].data) != SUCCESS)
        {
           CamOsPrintf("[%s] I2C write fail\n", __FUNCTION__);
            return FAIL;
        }
    }

    params->tVts_reg[0].data = ((params->expo.vts >> 8) & 0x00ff);
    params->tVts_reg[1].data = ((params->expo.vts >> 0) & 0x00ff);
    return SUCCESS;
}

static int OV4689_init(ss_cus_sensor *handle)
{
    int i;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    ov4689_params *params = (ov4689_params *)handle->private_data;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {
        if(Sensor_init_table[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        if(SensorReg_Write(Sensor_init_table[i].reg,Sensor_init_table[i].data) != SUCCESS)
        {
           CamOsPrintf("[%s] I2C write fail\n", __FUNCTION__);
            return FAIL;
        }
        // SensorReg_Read(Sensor_init_table[i].reg, &sen_data);
        // printf("[%s] i=0x%x,sen_data=0x%x\n", __FUNCTION__,i,sen_data);
    }

    params->tVts_reg[0].data = ((params->expo.vts >> 8) & 0x00ff);
    params->tVts_reg[1].data = ((params->expo.vts >> 0) & 0x00ff);
    return SUCCESS;
}

static int OV4689_init_2m60fps(ss_cus_sensor *handle)
{
    int i;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    ov4689_params *params = (ov4689_params *)handle->private_data;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2M60FPS);i++)
    {
        if(Sensor_init_table_2M60FPS[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2M60FPS[i].data);
        }
        if(SensorReg_Write(Sensor_init_table_2M60FPS[i].reg,Sensor_init_table_2M60FPS[i].data) != SUCCESS)
        {
           CamOsPrintf("[%s] I2C write fail\n", __FUNCTION__);
            return FAIL;
        }
    }

    params->tVts_reg[0].data = ((params->expo.vts >> 8) & 0x00ff);
    params->tVts_reg[1].data = ((params->expo.vts >> 0) & 0x00ff);
    return SUCCESS;
}

static int OV4689_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int OV4689_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int OV4689_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int OV4689_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0://2560x1440@30fps
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = OV4689_init;
            vts_30fps = 1667;
            Preview_line_period = 19996;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;
        case 1://1920x1080@60fps
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = OV4689_init_2m60fps;
            vts_30fps = 1250;
            Preview_line_period = 13333;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int OV4689_SetVideoRes_HDR_DCG(ss_cus_sensor *handle, u32 res_idx)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0://2688x1520@25fps_HDR
            handle->video_res_supported.ulcur_res = 0;
            if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1) {
                handle->pCus_sensor_init = OV4689_init_DCG;
            }
            params->expo.vts = vts_30fps_DCG;
            params->expo.fps = 20;
            params->expo.max_short=95;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int OV4689_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    ov4689_params *params = (ov4689_params *)handle->private_data;
    return params->cur_orien;
}

static int OV4689_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    SENSOR_DMSG(KERN_DEBUG "[%s] now:%d \r\n", __FUNCTION__,orit);
    switch(orit)
    {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[0].data = 0x00;
            params->tMirror_reg[1].data = 0x06;
            params->mirror_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[0].data = 0x00;
            params->tMirror_reg[1].data = 0x00;
            params->mirror_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[0].data = 0x06;
            params->tMirror_reg[1].data = 0x06;
            params->mirror_dirty = true;
            break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[0].data = 0x06;
            params->tMirror_reg[1].data = 0x00;
            params->mirror_dirty = true;
            break;
        default :
            break;
    }

    params->cur_orien = orit;
    return SUCCESS;
}

static int OV4689_SetOrien_HDR(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    switch(orit)
    {
        case CUS_ORIT_M0F0:
            params->tMirror_reg_HDR[0].data = 0x00;
            params->tMirror_reg_HDR[1].data = 0x06;
            params->mirror_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg_HDR[0].data = 0x00;
            params->tMirror_reg_HDR[1].data = 0x00;
            params->mirror_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg_HDR[0].data = 0x06;
            params->tMirror_reg_HDR[1].data = 0x06;
            params->mirror_dirty = true;
            break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg_HDR[0].data = 0x06;
            params->tMirror_reg_HDR[1].data = 0x00;
            params->mirror_dirty = true;
            break;
        default :
            break;
    }

    params->cur_orien = orit;
    return SUCCESS;
}

static int OV4689_GetFPS(ss_cus_sensor *handle)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int OV4689_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts = 0;
    ov4689_params *params = (ov4689_params *)handle->private_data;
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

    if ((params->expo.lines) > (params->expo.vts - 8))
        vts = params->expo.lines +8;
    else
        vts = params->expo.vts;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

static int OV4689_GetFPS_HDR_lef(ss_cus_sensor *handle)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_DCG*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_DCG*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int OV4689_SetFPS_HDR_lef(ss_cus_sensor *handle, u32 fps)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*max_fps)/fps;
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }
    //pr_info("[%s] %d  %d \n\n", __FUNCTION__, params->expo.vts, fps);
    params->expo.max_short = (((params->expo.vts)/17 - 1)>>1) << 1;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

static int OV4689_GetFPS_HDR_sef(ss_cus_sensor *handle)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_DCG*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_DCG*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int OV4689_SetFPS_HDR_sef(ss_cus_sensor *handle, u32 fps)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*max_fps)/fps;
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_DCG*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    //pr_info("[%s] %d  %d \n\n", __FUNCTION__, params->expo.vts, fps);
    params->expo.max_short = (((params->expo.vts)/17 - 1)>>1) << 1;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int OV4689_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
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
            SensorReg_Write(0x3208, 0x00);//Group 0 hold start
            SensorReg_Write(0x3209, 0x02);//Group 0 stay 2 frame
            SensorReg_Write(0x3601, 0x00);
            SensorReg_Write(0x3638, 0x00);
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
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

static int OV4689_AEStatusNotify_HDR_lef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    //ov4689_params *params = (ov4689_params *)handle->private_data;
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

static int OV4689_AEStatusNotify_HDR_sef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->mirror_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg_HDR, ARRAY_SIZE(mirror_reg_HDR));
            params->mirror_dirty = false;
        }
        if(params->dirty)
        {
            //SensorReg_Write(0x3208, 0x00);
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_vc0_reg, ARRAY_SIZE(expo_vc0_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_vc1_reg, ARRAY_SIZE(expo_vc1_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_vc1_reg, ARRAY_SIZE(gain_vc1_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            params->dirty = false;
        }

        break;
        default :
        break;
    }
    return SUCCESS;
}

static int OV4689_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    int rc = SUCCESS;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
    ov4689_params *params = (ov4689_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data&0x0f)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xf0)<<0;
    lines >>= 4;

    *us = (lines*Preview_line_period)/1000;

    return rc;
}

static int OV4689_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    ov4689_params *params = (ov4689_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period;
    if (lines < 2) lines = 2;
    if (lines >params->expo.vts-8)
        vts = lines +8;
    else
        vts=params->expo.vts;
    params->expo.lines = lines;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    lines <<= 4;
    params->tExpo_reg[0].data = (lines>>16) & 0x000f;
    params->tExpo_reg[1].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[2].data = (lines>>0) & 0x00f0;

    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;

    return SUCCESS;
}

static int OV4689_GetAEUSecs_HDR_lef(ss_cus_sensor *handle, u32 *us)
{
    int rc = SUCCESS;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_vc0_reg, ARRAY_SIZE(expo_vc0_reg));
    ov4689_params *params = (ov4689_params *)handle->private_data;

    lines |= (u32)(params->tExpo_vc0_reg[0].data&0x0f)<<16;
    lines |= (u32)(params->tExpo_vc0_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_vc0_reg[2].data&0xf0)<<0;
    lines >>= 4;

    *us = (lines*Preview_line_period_DCG)/1000;
    return rc;
}

static int OV4689_SetAEUSecs_HDR_lef(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    ov4689_params *params = (ov4689_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_DCG;
    if (lines < 2) lines = 2;
    if (lines > ((params->expo.vts) - (params->expo.max_short) - 8))
        lines = (params->expo.vts) - (params->expo.max_short) - 8;
    else
        vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    lines <<= 4;
    params->tExpo_vc0_reg[0].data = (lines>>16) & 0x000f;
    params->tExpo_vc0_reg[1].data = (lines>>8) & 0x00ff;
    params->tExpo_vc0_reg[2].data = (lines>>0) & 0x00f0;

    //pr_info("[%s] shutter %d  0x3e01 0x%x  0x3e02 0x%x\n", __FUNCTION__, us, params->tExpo_vc0_reg[0].data,params->tExpo_vc0_reg[1].data);
    params->dirty = true;
    return SUCCESS;
}

static int OV4689_GetAEUSecs_HDR_sef(ss_cus_sensor *handle, u32 *us)
{
    int rc = SUCCESS;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_vc1_reg, ARRAY_SIZE(expo_vc1_reg));
    ov4689_params *params = (ov4689_params *)handle->private_data;

    lines |= (u32)(params->tExpo_vc1_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_vc1_reg[1].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period_DCG)/1000;
    return rc;
}

static int OV4689_SetAEUSecs_HDR_sef(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    ov4689_params *params = (ov4689_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_DCG;
    if (lines < 2) lines = 2;
    if (lines > ((params->expo.max_short) - 2))
        lines = (params->expo.max_short) - 2;
    else
        vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    lines <<= 4;
    params->tExpo_vc1_reg[0].data = (lines>>8) & 0x00ff;
    params->tExpo_vc1_reg[1].data = (lines>>0) & 0x00f0;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int OV4689_GetAEGain(ss_cus_sensor *handle, u32* gain)
{

    //SENSOR_DMSG("[%s] get gain/reg0/reg1 (1024=1X)= %d/0x%x/0x%x\n", __FUNCTION__, *gain,params->tGain_reg[0].data,params->tGain_reg[1].data);
    return SUCCESS;
}

#define MAX_A_GAIN 16256//(15.875*1024)
static int OV4689_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 input_gain = 0;

    if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN) gain = SENSOR_MAX_GAIN;

    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    if (gain < 2*1024){
        params->tGain_reg[0].data = 0;
        params->tGain_reg[1].data = gain >> 3;                 // 1X ~ 2X
    }
    else if (gain < 4*1024){
        params->tGain_reg[0].data = 1;
        params->tGain_reg[1].data = (gain >> 4) - 8;    // 2X ~ 4X
    }
    else if (gain < 8*1024){
        params->tGain_reg[0].data = 3;
        params->tGain_reg[1].data = (gain >> 5) - 12;   // 4X ~ 8X
    }
    else if(gain < 16320){
        params->tGain_reg[0].data = 7;
        params->tGain_reg[1].data = (gain >> 6) - 8;    // 8X ~16X
    }else{
        params->tGain_reg[0].data = 7;
        params->tGain_reg[1].data = 0xf7;
    }

    if(input_gain > MAX_A_GAIN){
        params->tGain_reg[2].data=(u16)(input_gain / MAX_A_GAIN * 8) &0x7F;
        params->tGain_reg[3].data=(u16)(input_gain / MAX_A_GAIN * 2048) &0xFF;
    }
    else{
        params->tGain_reg[2].data=0x08;
        params->tGain_reg[3].data=0;
    }

    params->dirty = true;
    return SUCCESS;
}

static int OV4689_SetAEGain_HDR_sef(ss_cus_sensor *handle, u32 gain)
{
    ov4689_params *params = (ov4689_params *)handle->private_data;
    u32 input_gain = 0;

    if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN) gain = SENSOR_MAX_GAIN;

    //gain = 4*gain;
    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    if (gain < 2*1024){
        params->tGain_vc1_reg[0].data = 0;
        params->tGain_vc1_reg[1].data = gain >> 3;                 // 1X ~ 2X
    }
    else if (gain < 4*1024){
        params->tGain_vc1_reg[0].data = 1;
        params->tGain_vc1_reg[1].data = (gain >> 4) - 8;    // 2X ~ 4X
    }
    else if (gain < 8*1024){
        params->tGain_vc1_reg[0].data = 3;
        params->tGain_vc1_reg[1].data = (gain >> 5) - 12;   // 4X ~ 8X
    }
    else if(gain < 16320){
        params->tGain_vc1_reg[0].data = 7;
        params->tGain_vc1_reg[1].data = (gain >> 6) - 8;    // 8X ~16X
    }else{
        params->tGain_vc1_reg[0].data = 7;
        params->tGain_vc1_reg[1].data = 0xf7;
    }

    if(input_gain > MAX_A_GAIN){
        params->tGain_vc1_reg[2].data=(u16)(input_gain / MAX_A_GAIN * 8) &0x7F;
        params->tGain_vc1_reg[3].data=(u16)(input_gain / MAX_A_GAIN * 2048) &0xFF;
    }
    else{
        params->tGain_vc1_reg[2].data=0x08;
        params->tGain_vc1_reg[3].data=0;
    }

    params->dirty = true;
    return SUCCESS;
}


static int OV4689_SetPatternMode_hdr_lef(ss_cus_sensor *handle,u32 mode)
{
    return SUCCESS;
}

static int OV4689_poweron_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int OV4689_poweroff_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int OV4689_init_hdr_lef(ss_cus_sensor *handle)
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
    ov4689_params *params;
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
    params = (ov4689_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tMirror_reg_HDR, mirror_reg_HDR, sizeof(mirror_reg_HDR));
    memcpy(params->tGain_vc1_reg, gain_vc1_reg, sizeof(gain_vc1_reg));
    memcpy(params->tExpo_vc0_reg, expo_vc0_reg, sizeof(expo_vc0_reg));
    memcpy(params->tExpo_vc1_reg, expo_vc1_reg, sizeof(expo_vc1_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OV4688_MIPI");

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
        handle->video_res_supported.res[res].u16width         = ov4689_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = ov4689_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = ov4689_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = ov4689_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = ov4689_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = ov4689_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = ov4689_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = ov4689_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, ov4689_mipi_linear[res].senstr.strResDesc);
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
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = OV4689_init    ;
    handle->pCus_sensor_poweron     = OV4689_poweron ;
    handle->pCus_sensor_poweroff    = OV4689_poweroff;

    // Normal
    handle->pCus_sensor_GetVideoResNum = OV4689_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = OV4689_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = OV4689_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = OV4689_SetVideoRes;
    handle->pCus_sensor_GetOrien          = OV4689_GetOrien      ;
    handle->pCus_sensor_SetOrien          = OV4689_SetOrien      ;
    handle->pCus_sensor_GetFPS          = OV4689_GetFPS      ;
    handle->pCus_sensor_SetFPS          = OV4689_SetFPS      ;
    handle->pCus_sensor_SetPatternMode = OV4689_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = OV4689_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = OV4689_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = OV4689_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = OV4689_GetAEGain;
    handle->pCus_sensor_SetAEGain       = OV4689_SetAEGain;

    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    params->res.orit      = SENSOR_ORIT;          //CUS_ORIT_M0F0;
    params->cur_orien     = SENSOR_ORIT;
    params->expo.vts=vts_30fps;
    params->expo.fps = 20;
    params->expo.lines = 1000;
    params->mirror_dirty = false;
    params->dirty = false;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dcg_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    ov4689_params *params = NULL;
    int res;

    cus_camsensor_init_handle(drv_handle);
    params = (ov4689_params *)handle->private_data;

    sprintf(handle->strSensorStreamName,"OV4688_MIPI_HDR_SEF");

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
        handle->video_res_supported.res[res].u16width         = ov4689_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = ov4689_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = ov4689_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = ov4689_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = ov4689_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = ov4689_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = ov4689_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = ov4689_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, ov4689_mipi_hdr[res].senstr.strResDesc);
    }

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //Mirror / Flip
    params->cur_orien = SENSOR_ORIT;


    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = OV4689_init_DCG;

    // Normal
    handle->pCus_sensor_SetVideoRes       = OV4689_SetVideoRes_HDR_DCG;
    handle->pCus_sensor_GetFPS          = OV4689_GetFPS_HDR_sef;
    handle->pCus_sensor_SetFPS          = OV4689_SetFPS_HDR_sef;

    handle->pCus_sensor_AEStatusNotify = OV4689_AEStatusNotify_HDR_sef;
    handle->pCus_sensor_GetAEUSecs      = OV4689_GetAEUSecs_HDR_sef;
    handle->pCus_sensor_SetAEUSecs      = OV4689_SetAEUSecs_HDR_sef;
    handle->pCus_sensor_GetAEGain       = OV4689_GetAEGain;
    handle->pCus_sensor_SetAEGain       = OV4689_SetAEGain_HDR_sef;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    params->expo.vts=vts_30fps_DCG;
    params->cur_orien     = SENSOR_ORIT;
    params->res.orit      = SENSOR_ORIT;          //CUS_ORIT_M0F0;
    params->expo.fps = 20;
    params->expo.max_short=95;
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_DCG*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_DCG*params->expo.max_short;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_DCG;
    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dcg_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    ov4689_params *params;
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
    params = (ov4689_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tMirror_reg_HDR, mirror_reg_HDR, sizeof(mirror_reg_HDR));
    memcpy(params->tGain_vc1_reg, gain_vc1_reg, sizeof(gain_vc1_reg));
    memcpy(params->tExpo_vc0_reg, expo_vc0_reg, sizeof(expo_vc0_reg));
    memcpy(params->tExpo_vc1_reg, expo_vc1_reg, sizeof(expo_vc1_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OV4688_MIPI_HDR_LEF");

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
        handle->video_res_supported.res[res].u16width         = ov4689_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = ov4689_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = ov4689_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = ov4689_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = ov4689_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = ov4689_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = ov4689_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = ov4689_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, ov4689_mipi_hdr[res].senstr.strResDesc);
    }

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //Mirror / Flip
    params->cur_orien = SENSOR_ORIT;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_DCG*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_DCG;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = OV4689_init_hdr_lef    ;
    handle->pCus_sensor_poweron     = OV4689_poweron_hdr_lef ;
    handle->pCus_sensor_poweroff    = OV4689_poweroff_hdr_lef;

    // Normal
    //handle->pCus_sensor_GetVideoResNum = OV4689_GetVideoResNum;
    //handle->pCus_sensor_GetVideoRes       = OV4689_GetVideoRes;
    //handle->pCus_sensor_GetCurVideoRes  = OV4689_GetCurVideoRes;
    //handle->pCus_sensor_SetVideoRes       = OV4689_SetVideoRes_HDR_DCG_lef;
    handle->pCus_sensor_GetOrien          = OV4689_GetOrien;
    handle->pCus_sensor_SetOrien          = OV4689_SetOrien_HDR;
    handle->pCus_sensor_GetFPS          = OV4689_GetFPS_HDR_lef;
    handle->pCus_sensor_SetFPS          = OV4689_SetFPS_HDR_lef;
    handle->pCus_sensor_SetPatternMode = OV4689_SetPatternMode_hdr_lef;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = OV4689_AEStatusNotify_HDR_lef;
    handle->pCus_sensor_GetAEUSecs      = OV4689_GetAEUSecs_HDR_lef;
    handle->pCus_sensor_SetAEUSecs      = OV4689_SetAEUSecs_HDR_lef;
    handle->pCus_sensor_GetAEGain       = OV4689_GetAEGain;
    handle->pCus_sensor_SetAEGain       = OV4689_SetAEGain;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    params->res.orit      = SENSOR_ORIT;          //CUS_ORIT_M0F0;
    params->cur_orien     = SENSOR_ORIT;
    params->expo.vts=vts_30fps_DCG;
    params->expo.fps = 20;
    params->expo.lines = 1000;
    params->mirror_dirty = false;
    params->dirty = false;

    return SUCCESS;
}


SENSOR_DRV_ENTRY_IMPL_END_EX(  OV4689_HDR,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_hdr_dcg_sef,
                            cus_camsensor_init_handle_hdr_dcg_lef,
                            ov4689_params
                         );
