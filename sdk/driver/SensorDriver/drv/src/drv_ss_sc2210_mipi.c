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
   Date          : 2023/09/26
   Build on      : Master_V4 I6C
   Verified on   : Not yet
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC2210);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
#define SENSOR_MIPI_LANE_NUM_HDR (2)
#define SENSOR_MIPI_HDR_MODE (2) //0: Non-HDR mode. 1:Sony DOL mode 2:DCG
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3

//#undef SENSOR_DBG
#define SENSOR_DBG 0

//#define SENSOR_ISP_TYPE     ISP_EXT                   //ISP_EXT, ISP_SOC
#define F_number  22                                  // CFG, demo module
//#define SENSOR_DATAFMT       CUS_DATAFMT_BAYER      //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE      CUS_SENIF_BUS_MIPI     //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC        CUS_DATAPRECISION_12   //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR    CUS_DATAPRECISION_12
//#define SENSOR_DATAMODE        CUS_SEN_10TO12_9000    //CFG
//#define SENSOR_MAXGAIN       (15875*315)/10000   /////sensor again 15.875 dgain=31.5
//#define SENSOR_MAXGAIN         (53975*3175 / 100000)
#define SENSOR_MAX_GAIN        ((53975*3175/100000)*1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN        (1 * 1024)
#define SENSOR_BAYERID         CUS_BAYER_BG           //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR     CUS_BAYER_BG
#define SENSOR_RGBIRID         CUS_RGBIR_NONE
#define SENSOR_ORIT            CUS_ORIT_M0F0          //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_MAX_GAIN        80                     // max sensor again, a-gain

#define Preview_MCLK_SPEED     CUS_CMU_CLK_27MHZ      //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR CUS_CMU_CLK_27MHZ

#define Preview_line_period     14988
#define vts_30fps               1112
#define Preview_line_period_HDR 14988
#define vts_30fps_HDR           2224
#define Preview_WIDTH           1920                  //resolution Width when preview
#define Preview_HEIGHT          1080                  //resolution Height when preview
#define Preview_MAX_FPS         60                    //fastest preview FPS
#define Preview_MAX_FPS_HDR     30                    //fastest preview FPS
#define Preview_MIN_FPS         3                     //slowest preview FPS
#define Preview_CROP_START_X    0                     //CROP_START_X
#define Preview_CROP_START_Y    0                     //CROP_START_Y

#define SENSOR_I2C_ADDR         0x60                  //I2C slave address
#define SENSOR_I2C_SPEED        200000                //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY       I2C_NORMAL_MODE        //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT          I2C_FMT_A16D8          //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL         CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL          CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
//static volatile int mirror_delay=30;
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
}sc2210_mipi_linear[] = {
    {LINEAR_RES_1, {1920, 1080, 3, 60}, {0, 0, 1920, 1080}, {"1920x1080@60fps"}},
};
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
}sc2210_mipi_hdr[] = {
    {HDR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
};

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
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
//static int g_sensor_ae_min_gain = 1024;
#define ENABLE_NR 1

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
        u32 sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 back_pv_us;
        u32 preview_fps;
        u32 fps;
        u32 max_short_exp;
        u32 line;
    } expo;
    struct {
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tGain_reg_HDR_SEF[4];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tExpo_reg_HDR_SEF[2];
    I2C_ARRAY tMax_short_exp_reg[2];
    I2C_ARRAY tMirror_reg[1];
#if ENABLE_NR
    I2C_ARRAY tTemperature_reg[1];
#endif
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool orient_dirty;
    bool reg_dirty;
    bool temperature_reg_dirty;
    CUS_CAMSENSOR_ORIT cur_orien;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
} sc2210_params;
// set sensor ID address and data,

typedef struct {
    u64 gain;
    u8 fine_gain_reg;
} FINE_GAIN;

I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0x22},
    {0x3108, 0x10},
};

const I2C_ARRAY Sensor_init_table_2M30fps[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x3001,0x07},
    {0x3002,0xc0},
    {0x300a,0x2c},
    {0x300f,0x00},
    {0x3018,0x33},
    {0x3019,0x0c},
    {0x301f,0x01},
    {0x3031,0x0c},
    {0x3033,0x20},
    {0x3038,0x22},
    {0x3106,0x81},
    {0x3201,0x04},
    {0x3203,0x04},
    {0x3204,0x07},
    {0x3205,0x8b},
    {0x3206,0x04},
    {0x3207,0x43},
    {0x320c,0x04},
    {0x320d,0x37},
    {0x320e,0x04},
    {0x320f,0x58},
    {0x3211,0x04},
    {0x3213,0x04},
    {0x3231,0x02},
    {0x3253,0x04},
    {0x3301,0x0a},
    {0x3302,0x10},
    {0x3304,0x58},
    {0x3305,0x00},
    {0x3306,0xb0},
    {0x3308,0x20},
    {0x3309,0x98},
    {0x330a,0x01},
    {0x330b,0x68},
    {0x330e,0x48},
    {0x3314,0x92},
    {0x331e,0x49},
    {0x3000,0xc0},
    {0x331f,0x89},
    {0x334c,0x10},
    {0x335d,0x60},
    {0x335e,0x02},
    {0x335f,0x06},
    {0x3364,0x16},
    {0x3366,0x92},
    {0x3367,0x10},
    {0x3368,0x04},
    {0x3369,0x00},
    {0x336a,0x00},
    {0x336b,0x00},
    {0x336d,0x03},
    {0x337c,0x08},
    {0x337d,0x0e},
    {0x337f,0x33},
    {0x3390,0x10},
    {0x3391,0x30},
    {0x3392,0x40},
    {0x3393,0x0a},
    {0x3394,0x0a},
    {0x3395,0x0a},
    {0x3396,0x08},
    {0x3397,0x30},
    {0x3398,0x3f},
    {0x3399,0x30},
    {0x339a,0x30},
    {0x339b,0x30},
    {0x339c,0x30},
    {0x33a2,0x0a},
    {0x33b9,0x0e},
    {0x33e1,0x08},
    {0x33e2,0x18},
    {0x33e3,0x18},
    {0x33e4,0x18},
    {0x33e5,0x10},
    {0x33e6,0x06},
    {0x33e7,0x02},
    {0x33e8,0x18},
    {0x33e9,0x10},
    {0x33ea,0x0c},
    {0x33eb,0x10},
    {0x33ec,0x04},
    {0x33ed,0x02},
    {0x33ee,0xa0},
    {0x33ef,0x08},
    {0x33f4,0x18},
    {0x33f5,0x10},
    {0x33f6,0x0c},
    {0x33f7,0x10},
    {0x33f8,0x06},
    {0x33f9,0x02},
    {0x33fa,0x18},
    {0x33fb,0x10},
    {0x33fc,0x0c},
    {0x33fd,0x10},
    {0x33fe,0x04},
    {0x33ff,0x02},
    {0x360f,0x01},
    {0x3622,0xf7},
    {0x3625,0x0a},
    {0x3627,0x02},
    {0x3630,0xa2},
    {0x3631,0x00},
    {0x3632,0xd8},
    {0x3633,0x43},
    {0x3635,0x20},
    {0x3638,0x24},
    {0x363a,0x80},
    {0x363b,0x02},
    {0x363e,0x22},
    {0x3670,0x48},
    {0x3671,0xf7},
    {0x3672,0xf7},
    {0x3673,0x07},
    {0x367a,0x40},
    {0x367b,0x7f},
    {0x3690,0x42},
    {0x3691,0x43},
    {0x3692,0x54},
    {0x369c,0x40},
    {0x369d,0x7f},
    {0x36b5,0x40},
    {0x36b6,0x7f},
    {0x36c0,0x80},
    {0x36c1,0x9f},
    {0x36c2,0x9f},
    {0x36cc,0x20},
    {0x36cd,0x20},
    {0x36ce,0x30},
    {0x36d0,0x20},
    {0x36d1,0x40},
    {0x36d2,0x7f},
    {0x36ea,0x38},
    {0x36eb,0x0e},
    {0x36ec,0x03},
    {0x36ed,0x14},
    {0x36fa,0x3a},
    {0x36fb,0x15},
    {0x36fc,0x01},
    {0x36fd,0x14},
    {0x3905,0xd8},
    {0x3907,0x01},
    {0x3908,0x11},
    {0x391b,0x83},
    {0x391f,0x00},
    {0x3933,0x28},
    {0x3934,0xa6},
    {0x3940,0x70},
    {0x3942,0x08},
    {0x3943,0xbc},
    {0x3958,0x02},
    {0x3959,0x04},
    {0x3980,0x61},
    {0x3987,0x0b},
    {0x3990,0x00},
    {0x3991,0x00},
    {0x3992,0x00},
    {0x3993,0x00},
    {0x3994,0x00},
    {0x3995,0x00},
    {0x3996,0x00},
    {0x3997,0x00},
    {0x3998,0x00},
    {0x3999,0x00},
    {0x399a,0x00},
    {0x399b,0x00},
    {0x399c,0x00},
    {0x399d,0x00},
    {0x399e,0x00},
    {0x399f,0x00},
    {0x39a0,0x00},
    {0x39a1,0x00},
    {0x39a2,0x03},
    {0x39a3,0x30},
    {0x39a4,0x03},
    {0x39a5,0x60},
    {0x39a6,0x03},
    {0x39a7,0xa0},
    {0x39a8,0x03},
    {0x39a9,0xb0},
    {0x39aa,0x00},
    {0x39ab,0x00},
    {0x39ac,0x00},
    {0x39ad,0x20},
    {0x39ae,0x00},
    {0x39af,0x40},
    {0x39b0,0x00},
    {0x39b1,0x60},
    {0x39b2,0x00},
    {0x39b3,0x00},
    {0x39b4,0x08},
    {0x39b5,0x14},
    {0x39b6,0x20},
    {0x39b7,0x38},
    {0x39b8,0x38},
    {0x39b9,0x20},
    {0x39ba,0x14},
    {0x39bb,0x08},
    {0x39bc,0x08},
    {0x39bd,0x10},
    {0x39be,0x20},
    {0x39bf,0x30},
    {0x39c0,0x30},
    {0x39c1,0x20},
    {0x39c2,0x10},
    {0x39c3,0x08},
    {0x39c4,0x00},
    {0x39c5,0x80},
    {0x39c6,0x00},
    {0x39c7,0x80},
    {0x39c8,0x00},
    {0x39c9,0x00},
    {0x39ca,0x80},
    {0x39cb,0x00},
    {0x39cc,0x00},
    {0x39cd,0x00},
    {0x39ce,0x00},
    {0x39cf,0x00},
    {0x39d0,0x00},
    {0x39d1,0x00},
    {0x39e2,0x05},
    {0x39e3,0xeb},
    {0x39e4,0x07},
    {0x39e5,0xb6},
    {0x39e6,0x00},
    {0x39e7,0x3a},
    {0x39e8,0x3f},
    {0x39e9,0xb7},
    {0x39ea,0x02},
    {0x39eb,0x4f},
    {0x39ec,0x08},
    {0x39ed,0x00},
    {0x3e00,0x00},
    {0x3e01,0x45},
    {0x3e02,0x40},
    {0x3e03,0x0b},
    {0x3e06,0x00},
    {0x3e07,0x80},
    {0x3e08,0x03},
    {0x3e09,0x40},
    {0x3e14,0x31},
    {0x3e1b,0x3a},
    {0x3e26,0x40},
    {0x3f08,0x08},
    {0x4401,0x1a},
    {0x4407,0xc0},
    {0x4418,0x34},
    {0x4500,0x18},
    {0x4501,0xb4},
    {0x4509,0x20},
    {0x4603,0x00},
    {0x4800,0x24},
    {0x4837,0x13},
    {0x5000,0x0e},
    {0x550f,0x20},
    {0x36e9,0x24},
    {0x36f9,0x14},
    {0x0100,0x01},
    {0xffff,0x0a},/////delay 10ms
};

const I2C_ARRAY Sensor_init_table_HDR[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x3001,0x07},
    {0x3002,0xc0},
    {0x300a,0x2c},
    {0x300f,0x00},
    {0x3018,0x33},
    {0x3019,0x0c},
    {0x301f,0x43},
    {0x3031,0x0c},
    {0x3033,0x20},
    {0x3038,0x22},
    {0x3106,0x81},
    {0x3201,0x04},
    {0x3203,0x04},
    {0x3204,0x07},
    {0x3205,0x8b},
    {0x3206,0x04},
    {0x3207,0x43},
    {0x320c,0x04},
    {0x320d,0x37},
    {0x320e,0x08},
    {0x320f,0xb0},
    {0x3211,0x04},
    {0x3213,0x04},
    {0x3220,0x53},
    {0x3231,0x02},
    {0x3235,0x11},
    {0x3236,0x5e},
    {0x3250,0x3f},
    {0x3253,0x04},
    {0x3301,0x0a},
    {0x3302,0x10},
    {0x3304,0x58},
    {0x3305,0x00},
    {0x3306,0xb0},
    {0x3308,0x20},
    {0x3309,0x98},
    {0x330a,0x01},
    {0x3000,0xc0},
    {0x330b,0x68},
    {0x330e,0x48},
    {0x3314,0x94},
    {0x331e,0x49},
    {0x331f,0x89},
    {0x334c,0x10},
    {0x335d,0x60},
    {0x335e,0x02},
    {0x335f,0x06},
    {0x3364,0x16},
    {0x3366,0x92},
    {0x3367,0x10},
    {0x3368,0x04},
    {0x3369,0x00},
    {0x336a,0x00},
    {0x336b,0x00},
    {0x336d,0x03},
    {0x337c,0x08},
    {0x337d,0x0e},
    {0x337f,0x33},
    {0x3390,0x10},
    {0x3391,0x30},
    {0x3392,0x40},
    {0x3393,0x0a},
    {0x3394,0x0a},
    {0x3395,0x0a},
    {0x3396,0x08},
    {0x3397,0x30},
    {0x3398,0x3f},
    {0x3399,0x30},
    {0x339a,0x30},
    {0x339b,0x30},
    {0x339c,0x30},
    {0x33a2,0x0a},
    {0x33b9,0x0e},
    {0x33e1,0x08},
    {0x33e2,0x18},
    {0x33e3,0x18},
    {0x33e4,0x18},
    {0x33e5,0x10},
    {0x33e6,0x06},
    {0x33e7,0x02},
    {0x33e8,0x18},
    {0x33e9,0x10},
    {0x33ea,0x0c},
    {0x33eb,0x10},
    {0x33ec,0x04},
    {0x33ed,0x02},
    {0x33ee,0xa0},
    {0x33ef,0x08},
    {0x33f4,0x18},
    {0x33f5,0x10},
    {0x33f6,0x0c},
    {0x33f7,0x10},
    {0x33f8,0x06},
    {0x33f9,0x02},
    {0x33fa,0x18},
    {0x33fb,0x10},
    {0x33fc,0x0c},
    {0x33fd,0x10},
    {0x33fe,0x04},
    {0x33ff,0x02},
    {0x360f,0x01},
    {0x3622,0xf7},
    {0x3625,0x0a},
    {0x3627,0x02},
    {0x3630,0xa2},
    {0x3631,0x00},
    {0x3632,0xd8},
    {0x3633,0x43},
    {0x3635,0x20},
    {0x3638,0x24},
    {0x363a,0x80},
    {0x363b,0x02},
    {0x363e,0x22},
    {0x3670,0x48},
    {0x3671,0xf7},
    {0x3672,0xf7},
    {0x3673,0x07},
    {0x367a,0x40},
    {0x367b,0x7f},
    {0x3690,0x42},
    {0x3691,0x43},
    {0x3692,0x54},
    {0x369c,0x40},
    {0x369d,0x7f},
    {0x36b5,0x40},
    {0x36b6,0x7f},
    {0x36c0,0x9f},
    {0x36c1,0x9f},
    {0x36c2,0x9f},
    {0x36cc,0x20},
    {0x36cd,0x20},
    {0x36ce,0x30},
    {0x36d0,0x20},
    {0x36d1,0x40},
    {0x36d2,0x7f},
    {0x36ea,0x38},
    {0x36eb,0x0e},
    {0x36ec,0x03},
    {0x36ed,0x14},
    {0x36fa,0x3a},
    {0x36fb,0x15},
    {0x36fc,0x01},
    {0x36fd,0x14},
    {0x3905,0xd8},
    {0x3907,0x01},
    {0x3908,0x11},
    {0x391b,0x83},
    {0x391f,0x00},
    {0x3933,0x28},
    {0x3934,0xa6},
    {0x3940,0x70},
    {0x3942,0x08},
    {0x3943,0xbc},
    {0x3958,0x02},
    {0x3959,0x04},
    {0x3980,0x61},
    {0x3987,0x0b},
    {0x3990,0x00},
    {0x3991,0x00},
    {0x3992,0x00},
    {0x3993,0x00},
    {0x3994,0x00},
    {0x3995,0x00},
    {0x3996,0x00},
    {0x3997,0x00},
    {0x3998,0x00},
    {0x3999,0x00},
    {0x399a,0x00},
    {0x399b,0x00},
    {0x399c,0x00},
    {0x399d,0x00},
    {0x399e,0x00},
    {0x399f,0x00},
    {0x39a0,0x00},
    {0x39a1,0x00},
    {0x39a2,0x03},
    {0x39a3,0x30},
    {0x39a4,0x03},
    {0x39a5,0x60},
    {0x39a6,0x03},
    {0x39a7,0xa0},
    {0x39a8,0x03},
    {0x39a9,0xb0},
    {0x39aa,0x00},
    {0x39ab,0x00},
    {0x39ac,0x00},
    {0x39ad,0x20},
    {0x39ae,0x00},
    {0x39af,0x40},
    {0x39b0,0x00},
    {0x39b1,0x60},
    {0x39b2,0x00},
    {0x39b3,0x00},
    {0x39b4,0x08},
    {0x39b5,0x14},
    {0x39b6,0x20},
    {0x39b7,0x38},
    {0x39b8,0x38},
    {0x39b9,0x20},
    {0x39ba,0x14},
    {0x39bb,0x08},
    {0x39bc,0x08},
    {0x39bd,0x10},
    {0x39be,0x20},
    {0x39bf,0x30},
    {0x39c0,0x30},
    {0x39c1,0x20},
    {0x39c2,0x10},
    {0x39c3,0x08},
    {0x39c4,0x00},
    {0x39c5,0x80},
    {0x39c6,0x00},
    {0x39c7,0x80},
    {0x39c8,0x00},
    {0x39c9,0x00},
    {0x39ca,0x80},
    {0x39cb,0x00},
    {0x39cc,0x00},
    {0x39cd,0x00},
    {0x39ce,0x00},
    {0x39cf,0x00},
    {0x39d0,0x00},
    {0x39d1,0x00},
    {0x39e2,0x05},
    {0x39e3,0xeb},
    {0x39e4,0x07},
    {0x39e5,0xb6},
    {0x39e6,0x00},
    {0x39e7,0x3a},
    {0x39e8,0x3f},
    {0x39e9,0xb7},
    {0x39ea,0x02},
    {0x39eb,0x4f},
    {0x39ec,0x08},
    {0x39ed,0x00},
    {0x3c10,0x03},
    {0x3c11,0xd0},
    {0x3c14,0x14},
    {0x3e00,0x00},
    {0x3e01,0x82},
    {0x3e02,0x00},
    {0x3e03,0x0b},
    {0x3e04,0x08},
    {0x3e05,0x20},
    {0x3e06,0x00},
    {0x3e07,0x80},
    {0x3e08,0x03},
    {0x3e09,0x40},
    {0x3e12,0x03},
    {0x3e13,0x40},
    {0x3e14,0x31},
    {0x3e1b,0x3a},
    {0x3e23,0x00},
    {0x3e24,0x88},
    {0x3e26,0x40},
    {0x3e53,0x00},
    {0x3e54,0x00},
    {0x3f05,0x18},
    {0x3f08,0x08},
    {0x4401,0x1a},
    {0x4407,0xc0},
    {0x4418,0x34},
    {0x4500,0x18},
    {0x4501,0xb4},
    {0x4503,0xc0},
    {0x4505,0x12},
    {0x4509,0x20},
    {0x4603,0x00},
    {0x4800,0x24},
    {0x4837,0x13},
    {0x4850,0xab},
    {0x4851,0x5b},
    {0x4853,0xfd},
    {0x5000,0x0e},
    {0x550f,0x20},
    {0x36e9,0x24},
    {0x36f9,0x14},
    {0x0100,0x01},
    {0xffff,0x0a},/////delay 10ms
};


I2C_ARRAY TriggerStartTbl[] = {
//{0x30f4,0x00},//Master mode start
};

I2C_ARRAY PatternTbl[] = {
    //pattern mode
};

I2C_ARRAY mirror_reg[] =
{
    {0x3221, 0x00}, // mirror[2:1], flip[6:5]
};

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

static I2C_ARRAY gain_reg[] = {
    {0x3e06, 0x00},
    {0x3e07, 0x80},
    {0x3e08, 0x03},
    {0x3e09, 0x40}, //low bit, 0x10 - 0x3e0, step 1/64
};

static I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x3e10, 0x00},
    {0x3e11, 0x80},
    {0x3e12, 0x03},
    {0x3e13, 0x40}, //low bit, 0x10 - 0x3e0, step 1/16
    };

I2C_ARRAY expo_reg[] = {  // max expo line vts*2-6!
    {0x3e00, 0x00}, //expo [20:17]
    {0x3e01, 0x8c}, // expo[16:8]
    {0x3e02, 0x40}, // expo[7:0], [3:0] fraction of line
};

I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x3e04, 0x21}, // expo[7:0]
    {0x3e05, 0x00}, // expo[7:4]
};

I2C_ARRAY vts_reg[] = {
    {0x320e, 0x04},
    {0x320f, 0x58}
};

I2C_ARRAY max_short_exp_reg[] = {
    {0x3e23, 0x00},
    {0x3e24, 0x88}
};

#if ENABLE_NR
const I2C_ARRAY temperature_reg[] = {
    {0x5799, 0x00},
};
#endif

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
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME sc2210

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
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
    sc2210_params *params = (sc2210_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_min                   = Preview_line_period_HDR * 2;
        info->u32AEShutter_step                  = Preview_line_period_HDR;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
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
    sc2210_params *params = (sc2210_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    //ISP_config_io(handle);
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, params->reset_POLARITY);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_USLEEP(1000);

    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, handle->mclk);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_288M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    SENSOR_USLEEP(3000);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sc2210_params *params = (sc2210_params *)handle->private_data;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_USLEEP(1000);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->MCLK(idx, 0, handle->mclk);

    params->cur_orien = CUS_ORIT_M0F0;
    return SUCCESS;
}

static int sc2210_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    return SUCCESS;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init_mipi2lane_linear_2M30fps(ss_cus_sensor *handle)
{
    sc2210_params *params = (sc2210_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;
    //ISensorIfAPI *sensor_if = (handle->sensor_if_api);
    //sensor_if->PCLK(NULL,CUS_PCLK_MIPI_TOP);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2M30fps);i++)
    {
        if(Sensor_init_table_2M30fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2M30fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2M30fps[i].reg, Sensor_init_table_2M30fps[i].data) != SUCCESS)
            {
                cnt++;
                SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }

    pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

static int pCus_init_mipi2lane_HDR(ss_cus_sensor *handle)
{
    sc2210_params *params = (sc2210_params *)handle->private_data;
    int i,cnt=0;
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR);i++)
    {
        if(Sensor_init_table_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR[i].data);
        }
        else
        {
            cnt = 0;
            SENSOR_DMSG("reg =  %x, data = %x\n", Sensor_init_table_HDR[i].reg, Sensor_init_table_HDR[i].data);
            while(SensorReg_Write(Sensor_init_table_HDR[i].reg,Sensor_init_table_HDR[i].data) != SUCCESS)
            {
                cnt++;
                 SENSOR_DMSG("Sensor_init_table_HDR -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }

    pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

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
    sc2210_params *params = (sc2210_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"1920x1080@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_linear_2M30fps;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    sc2210_params *params = (sc2210_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short_exp=130;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    sc2210_params *params = (sc2210_params *)handle->private_data;
    sen_data = params->tMirror_reg[0].data;
    SENSOR_DMSG("mirror:%x\r\n", sen_data);
    switch(sen_data) {
      case 0x00:
        *orit = CUS_ORIT_M0F0;
        break;
      case 0x06:
        *orit = CUS_ORIT_M1F0;
        break;
      case 0x60:
        *orit = CUS_ORIT_M0F1;
        break;
      case 0x66:
        *orit = CUS_ORIT_M1F1;
        break;
      }
      return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
   sc2210_params *params = (sc2210_params *)handle->private_data;

    switch(orit) {
    case CUS_ORIT_M0F0:
        params->tMirror_reg[0].data = 0;
        params->orient_dirty = true;
        break;
    case CUS_ORIT_M1F0:
        params->tMirror_reg[0].data = 6;
        params->orient_dirty = true;
        break;
    case CUS_ORIT_M0F1:
        params->tMirror_reg[0].data = 0x60;
        params->orient_dirty = true;
        break;
    case CUS_ORIT_M1F1:
        params->tMirror_reg[0].data = 0x66;
        params->orient_dirty = true;
        break;
    default :
        break;
    }
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    sc2210_params *params = (sc2210_params *)handle->private_data;
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
    sc2210_params *params = (sc2210_params *)handle->private_data;
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

    if(params->expo.line > (params->expo.vts - 4)){
        vts = params->expo.line + 4;
    }else{
        vts = params->expo.vts;
    }
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_SetFPS_HDR_SEF(ss_cus_sensor *handle, u32 fps)
{
    sc2210_params *params = (sc2210_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.max_short_exp = (((params->expo.vts)/17 - 1)>>1) << 1;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
    params->tMax_short_exp_reg[0].data = (params->expo.max_short_exp >> 8) & 0x00ff;
    params->tMax_short_exp_reg[1].data = (params->expo.max_short_exp >> 0) & 0x00ff;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_SetFPS_hdr_lef(ss_cus_sensor *handle, u32 fps)
{
    sc2210_params *params = (sc2210_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.max_short_exp = (((params->expo.vts)/17 - 1)>>1) << 1;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
    params->tMax_short_exp_reg[0].data = (params->expo.max_short_exp >> 8) & 0x00ff;
    params->tMax_short_exp_reg[1].data = (params->expo.max_short_exp >> 0) & 0x00ff;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->reg_dirty = true;
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

   sc2210_params *params = (sc2210_params *)handle->private_data;

   switch(status)
   {
        case CUS_FRAME_INACTIVE:
            break;
        case CUS_FRAME_ACTIVE:
            if(params->orient_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
                params->orient_dirty = false;
            }
            if(params->reg_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, sizeof(expo_reg)/sizeof(I2C_ARRAY));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));
                params->reg_dirty = false;
            }
            if(params->temperature_reg_dirty) {
                //SensorRegArrayW((I2C_ARRAY*)params->tTemperature_reg, sizeof(temperature_reg)/sizeof(I2C_ARRAY));
                params->temperature_reg_dirty = false;
            }
            break;
        default :
            break;
   }
   return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    //sc2210_params *params = (sc2210_params *)handle->private_data;
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

static int pCus_AEStatusNotifyHDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    sc2210_params *params = (sc2210_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            break;
        case CUS_FRAME_ACTIVE:
            if(params->orient_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orient_dirty = false;
            }
            if(params->reg_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tMax_short_exp_reg, ARRAY_SIZE(max_short_exp_reg));
                params->reg_dirty = false;
            }
            if(params->temperature_reg_dirty) {
                //SensorRegArrayW((I2C_ARRAY*)params->tTemperature_reg, sizeof(temperature_reg)/sizeof(I2C_ARRAY));
                params->temperature_reg_dirty = false;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_LEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 lines = 0,dou_lines = 0,vts = 0;
    sc2210_params *params = (sc2210_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));

    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    lines = 2*dou_lines;
    if(lines<=2) lines=2;
    if (lines >  (params->expo.vts - params->expo.max_short_exp - 8)) {
        lines = params->expo.vts-params->expo.max_short_exp - 8;
    }
    else
        vts=params->expo.vts;

    lines = lines<<4;
    params->tExpo_reg[0].data = (lines>>16) & 0x07;
    params->tExpo_reg[1].data = (lines>>8) & 0xff;
    params->tExpo_reg[2].data = (lines>>0) & 0xf0;

    for (i = 0; i < sizeof(expo_reg)/sizeof(I2C_ARRAY); i++)
    {
        if (params->tExpo_reg[i].data != expo_reg_temp[i].data)
        {
            params->reg_dirty = true;
            break;
        }
    }
    return SUCCESS;
}

static int pCus_GetAEUSecsHDR_SEF(ss_cus_sensor *handle, u32 *us) {
    int rc=0;
    u32 lines = 0;
    sc2210_params *params = (sc2210_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period_HDR)/1000; //return us

    SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
    return rc;
}

static int pCus_SetAEUSecsHDR_SEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 lines = 0,dou_lines = 0,vts = 0;
    sc2210_params *params = (sc2210_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {
        {0x3e22, 0x00}, // expo[3:0]
        {0x3e04, 0x21}, // expo[7:0]
        {0x3e05, 0x00}, // expo[7:4]
    };
    memcpy(expo_reg_temp, params->tExpo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));

    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    lines = 2*dou_lines;
    if(lines<=2) lines=2;
    if (lines > (params->expo.max_short_exp - 6)) {
        lines = params->expo.max_short_exp - 6;
    }
    else
        vts=params->expo.vts;

    lines = lines<<4;
    params->tExpo_reg_HDR_SEF[0].data =  (lines>>8) & 0xff;
    params->tExpo_reg_HDR_SEF[1].data = (lines>>0) & 0xf0;

    for (i = 0; i < sizeof(expo_reg_HDR_SEF)/sizeof(I2C_ARRAY); i++)
    {
        if (params->tExpo_reg_HDR_SEF[i].data != expo_reg_temp[i].data)
        {
            params->reg_dirty = true;
        break;
        }
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    int rc=0;
    u32 lines = 0;
    sc2210_params *params = (sc2210_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0x0f)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period)/1000; //return us

    SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
    return rc;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    int i;
    u32 lines = 0,vts = 0;
    sc2210_params *params = (sc2210_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));

    lines = (1000*us)/Preview_line_period; // Preview_line_period in ns
    if(lines <= 1) lines=1;
    if (lines > (params->expo.vts-4)) {
        vts = lines + 4;
    }
    else
        vts=params->expo.vts;

    params->expo.line = lines;
    SENSOR_DMSG("[%s] us %d, lines %d, vts %d\n", __FUNCTION__, us, lines, params->expo.vts);

    lines = lines<<4;
    params->tExpo_reg[0].data = (lines>>16) & 0x0f;
    params->tExpo_reg[1].data = (lines>>8) & 0xff;
    params->tExpo_reg[2].data = (lines>>0) & 0xf0;
    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;

    for (i = 0; i < sizeof(expo_reg)/sizeof(I2C_ARRAY); i++)
    {
       if (params->tExpo_reg[i].data != expo_reg_temp[i].data)
       {
            params->reg_dirty = true;
            break;
       }
    }
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    int rc = 0;

    return rc;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    sc2210_params *params = (sc2210_params *)handle->private_data;
    u8 i=0 , Coarse_gain = 1,DIG_gain=1;
    u32 Dcg_gainx1024 = 1, ANA_Fine_gainx64 = 1,DIG_Fine_gainx1024 =1;
    u8 Dcg_gain_reg = 0, Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x20,DIG_Fine_gain_reg=0x80;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e08, 0x00|0x03},
        {0x3e09, 0x40},
    };
    I2C_ARRAY temperature_reg_temp[] ={
        {0x5799, 0x00},
    };
    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg));
    memcpy(temperature_reg_temp, params->tTemperature_reg, sizeof(temperature_reg_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN) {
        gain = SENSOR_MAX_GAIN;
    }

    if (gain < 2 * 1024) // start again
    {
        Dcg_gainx1024 = 1024; Coarse_gain = 1;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 0;     Coarse_gain_reg = 0x03; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 3488)//3.40625*1024
    {
        Dcg_gainx1024 = 1024; Coarse_gain = 2;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 0;     Coarse_gain_reg = 0x07; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 6976)//6.8125*1024
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 1;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x23; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 13952)//13.625*1024
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 2;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x27; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 27904)//27.25*1024
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 4;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x2f; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain <= 55372) // end again
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
#if 1
    else if (gain < 55372 * 2) // start dgain
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 1;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x0; ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain < 55372 * 4)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 2;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x1; ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain < 55372 * 8)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 4;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x3; ANA_Fine_gain_reg = 0x7f;
    }
   else if (gain < 55372 * 16)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 8;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x7; ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain <= 1754822)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 16;      ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0xf; ANA_Fine_gain_reg = 0x7f;
    }
#endif

    if(gain < 55270)
    {
        ANA_Fine_gain_reg = abs(64 * gain / (Dcg_gainx1024 * Coarse_gain));
    }else{
        DIG_Fine_gain_reg = abs(8192 * gain /(Dcg_gainx1024 * Coarse_gain * DIG_gain) / ANA_Fine_gainx64);
    }

    params->tGain_reg[3].data = ANA_Fine_gain_reg;
    params->tGain_reg[2].data = Coarse_gain_reg;
    params->tGain_reg[1].data = DIG_Fine_gain_reg;
    params->tGain_reg[0].data = DIG_gain_reg & 0xF;

    for (i = 0; i < sizeof(params->tGain_reg)/sizeof(I2C_ARRAY); i++)
    {
      if (params->tGain_reg[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

    // highTemp dpc
    if (gain >= 30 * 1024) {
        params->tTemperature_reg[0].data = 0x07;
    } else if (gain <= 20 * 1024) {
        params->tTemperature_reg[0].data = 0x00;
    }

    for (i = 0; i < sizeof(temperature_reg_temp)/sizeof(I2C_ARRAY); i++) {
        if (params->tTemperature_reg[i].data != temperature_reg_temp[i].data) {
            params->temperature_reg_dirty = true;
        break;
        }
    }
    return SUCCESS;
}

static int pCus_SetAEGainHDR_SEF(ss_cus_sensor *handle, u32 gain) {
    sc2210_params *params = (sc2210_params *)handle->private_data;
    u8 i=0 , Coarse_gain = 1,DIG_gain=1;
    u32 Dcg_gainx1024 = 1, ANA_Fine_gainx64 = 1,DIG_Fine_gainx1024 =1;
    u8 Dcg_gain_reg = 0, Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x20,DIG_Fine_gain_reg=0x80;
    I2C_ARRAY gain_reg_temp[] = {
        {0x3e10, 0x00},
        {0x3e11, 0x80},
        {0x3e12, 0x00|0x03},
        {0x3e13, 0x40},
    };
    I2C_ARRAY temperature_reg_temp[] ={
        {0x5799, 0x00},
    };
    memcpy(gain_reg_temp, params->tGain_reg_HDR_SEF, sizeof(gain_reg_temp));
    memcpy(temperature_reg_temp, params->tTemperature_reg, sizeof(temperature_reg_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN) {
        gain = SENSOR_MAX_GAIN;
    }

    if (gain < 2 * 1024) // start again
    {
        Dcg_gainx1024 = 1024; Coarse_gain = 1;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 0;     Coarse_gain_reg = 0x03; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 3488)//3.40625*1024
    {
        Dcg_gainx1024 = 1024; Coarse_gain = 2;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 0;     Coarse_gain_reg = 0x07; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 6976)//6.8125*1024
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 1;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x23; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 13952)//13.625*1024
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 2;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x27; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 27904)//27.25*1024
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 4;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x2f; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
    else if (gain <= 55372) // end again
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 1;       DIG_Fine_gainx1024 = 1024;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x0; DIG_Fine_gain_reg = 0x80;
    }
#if 1
    else if (gain < 55372 * 2) // start dgain
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 1;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x0; ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain < 55372 * 4)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 2;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x1; ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain < 55372 * 8)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 4;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x3; ANA_Fine_gain_reg = 0x7f;
    }
   else if (gain < 55372 * 16)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 8;       ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0x7; ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain <= 1754822)
    {
        Dcg_gainx1024 = 3488; Coarse_gain = 8;        DIG_gain = 16;      ANA_Fine_gainx64 = 127;
        Dcg_gain_reg = 1;     Coarse_gain_reg = 0x3f; DIG_gain_reg = 0xf; ANA_Fine_gain_reg = 0x7f;
    }
#endif

    if(gain < 55270)
    {
        ANA_Fine_gain_reg = abs(64 * gain / (Dcg_gainx1024 * Coarse_gain));
    }else{
        DIG_Fine_gain_reg = abs(8192 * gain /(Dcg_gainx1024 * Coarse_gain * DIG_gain) / ANA_Fine_gainx64);
    }

    params->tGain_reg_HDR_SEF[3].data = ANA_Fine_gain_reg;
    params->tGain_reg_HDR_SEF[2].data = Coarse_gain_reg;
    params->tGain_reg_HDR_SEF[1].data = DIG_Fine_gain_reg;
    params->tGain_reg_HDR_SEF[0].data = DIG_gain_reg & 0xF;

    for (i = 0; i < sizeof(params->tGain_reg_HDR_SEF)/sizeof(I2C_ARRAY); i++)
    {
      if (params->tGain_reg_HDR_SEF[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

    // highTemp dpc
    if (gain >= 30 * 1024) {
        params->tTemperature_reg[0].data = 0x07;
    } else if (gain <= 20 * 1024) {
        params->tTemperature_reg[0].data = 0x00;
    }

    for (i = 0; i < sizeof(temperature_reg_temp)/sizeof(I2C_ARRAY); i++) {
        if (params->tTemperature_reg[i].data != temperature_reg_temp[i].data)
        {
            params->temperature_reg_dirty = true;
            break;
        }
    }
    return SUCCESS;
}

static int pCus_poweron_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}
static int pCus_init_hdr_lef(ss_cus_sensor *handle)
{
    return SUCCESS;
}
#if 0
static int pCus_GetVideoRes_hdr_lef( ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res )
{
    *res = &handle->video_res_supported.res[res_idx];
    return SUCCESS;
}

static int pCus_SetVideoRes_hdr_lef( ss_cus_sensor *handle, u32 res )
{
    handle->video_res_supported.ulcur_res = 0; //TBD
    return SUCCESS;
}
#endif

static int pCus_GetFPS_hdr_lef(ss_cus_sensor *handle)
{
    sc2210_params *params = (sc2210_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (vts_reg[0].data << 8) | (vts_reg[1].data << 0);

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

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sc2210_params *params;
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

    params = (sc2210_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMax_short_exp_reg, max_short_exp_reg, sizeof(max_short_exp_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
#if ENABLE_NR
    memcpy(params->tTemperature_reg, temperature_reg, sizeof(temperature_reg));
#endif

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"sc2210_MIPI");

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
        handle->video_res_supported.res[res].u16width             = sc2210_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = sc2210_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = sc2210_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = sc2210_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = sc2210_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = sc2210_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = sc2210_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = sc2210_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc2210_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi2lane_linear_2M30fps;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = sc2210_SetPatternMode;
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
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/sc2210_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps;
    params->expo.fps                                              = 30;
    params->reg_dirty                                             = false;
    params->orient_dirty                                          = false;
    params->temperature_reg_dirty                                 = false;
    params->expo.line                                             = 1000;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sc2210_params *params = NULL;
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
    cus_camsensor_init_handle_linear(drv_handle);
    params = (sc2210_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"sc2210_MIPI_HDR_SEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_HDR;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;
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
        handle->video_res_supported.res[res].u16width             = sc2210_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = sc2210_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = sc2210_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = sc2210_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = sc2210_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = sc2210_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = sc2210_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = sc2210_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc2210_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi2lane_HDR;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    //handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_SEF1;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_SEF;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_SEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecsHDR_SEF;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_SEF;
    //handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_SEF;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps_HDR;
    params->expo.fps                                              = 30;
    params->expo.max_short_exp                                    = 130;
    //params->expo.expo_lines                                       = 4250;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR * 2;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sc2210_params *params;
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
    params = (sc2210_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
#if ENABLE_NR
    memcpy(params->tTemperature_reg, temperature_reg, sizeof(temperature_reg));
#endif

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"SC2210_MIPI_HDR_LEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_HDR;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;
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
    handle->pCus_sensor_poweron                                   = pCus_poweron_hdr_lef;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff_hdr_lef;

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
        handle->video_res_supported.res[res].u16width             = sc2210_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = sc2210_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = sc2210_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = sc2210_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = sc2210_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = sc2210_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = sc2210_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = sc2210_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc2210_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_hdr_lef;
    handle->pCus_sensor_GetVideoResNum                            = NULL;
    handle->pCus_sensor_GetVideoRes                               = NULL;
    handle->pCus_sensor_GetCurVideoRes                            = NULL;
    handle->pCus_sensor_SetVideoRes                               = NULL;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_hdr_lef;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_hdr_lef;
    handle->pCus_sensor_SetPatternMode                            = sc2210_SetPatternMode;
    handle->pCus_sensor_CustDefineFunction                        = pCus_sensor_CustDefineFunction;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_LEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/sc2210_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR * 2;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps_HDR;
    params->expo.fps                                              = 30;
    params->reg_dirty                                             = false;
    params->temperature_reg_dirty                                 = false;
    params->orient_dirty                                          = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX( SC2210,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            sc2210_params
                         );
