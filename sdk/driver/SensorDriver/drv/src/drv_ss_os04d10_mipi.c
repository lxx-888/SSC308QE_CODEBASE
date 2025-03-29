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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OS04D10);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET       CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM         (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode
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
//                                                                                                    嚙瑾//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define MAX_A_GAIN          (49725)                 //255/16 * 3120/1024 * 1024
#define SENSOR_MAX_GAIN     (MAX_A_GAIN * 32) // max sensor again, a-gain * conversion-gain*d-gain
#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ       //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M

#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)

#define Preview_WIDTH       2560                    //resolution Width when preview
#define Preview_HEIGHT      1440                    //resolution Height when preview

#define SENSOR_I2C_ADDR     0x78                    //I2C slave address
#define SENSOR_I2C_SPEED    240000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY   I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT      I2C_FMT_A8D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)

static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif

//#define vts_120fps           369
//#define Preview_line_period_FPS_120    22583                   // 1s/(vts*vts_maxfps) ns

#define vts_15fps                  1552
#define Preview_line_period_FPS_15 43802 // 1s/(vts*vts_maxfps) ns

//#define vts_15fps           2946                    //init table fps=15 VTS for 15fps  1s/(vts*fps) ns
#define vts_30fps                  1473  // VTS for 30fps
#define Preview_line_period_FPS_30 22629 // 1s/(vts*vts_maxfps) ns

static int OS04D10_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int OS04D10_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int OS04D10_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx);
static int pCus_init(ss_cus_sensor *handle);
//static int g_sensor_ae_min_gain = 1024;
static unsigned short linear_last_dcg_data = 0;

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_3, LINEAR_RES_4, LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout
    {
        s32 width, height, min_fps, max_fps;
    } senout;
    // VIF Get Image Info
    struct _sensif
    {
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    } senif;
    // Show resolution string
    struct _senstr
    {
        const char *strResDesc;
    } senstr;
} os04d10_mipi_linear[] = {
    {LINEAR_RES_1, {2560, 1440, 3, 30}, {0, 0, 2560, 1440}, {"2560x1440@30fps"}},
    {LINEAR_RES_2, {2560, 1440, 3, 15}, {0, 0, 2560, 1440}, {"2560x1440@15fps"}},
    {LINEAR_RES_3, {2560, 1440, 3, 30}, {0, 0, 2560, 1440}, {"fastAe2560x1440@30fps"}},//FastAE little picture
    {LINEAR_RES_4, {2560, 1440, 3, 30}, {0, 0, 2560, 1440}, {"fastAe2560x1440@30fps"}},//earlyinit fast ae final use index 4, app get info.
};

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
        u32 fps;
        u32 preview_fps;
        u32 lines;
    } expo;
    I2C_ARRAY tVts_reg[4];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[6];
    I2C_ARRAY tLcg_Hcg_reg[2];
    I2C_ARRAY tAETarget_reg[2];
    u32 otp_data;
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool reg_dirty;
    bool orien_dirty;
    u32 Init_Vts;
    u32 Line_Period;
    const I2C_ARRAY *pTable_linear;
    u32 Init_Array_Size;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
    CUS_UserDefSnrInfo_t stUserDefInfo;
    bool bFastAE_en;
    u32 PreSetGain;
    u32 PreSetShutter;
    u32 PreSetFPS;
} OS04D10_params;
// set sensor ID address and data


const I2C_ARRAY Sensor_id_table[] =
{
    {0xfd, 0x00},
    {0x02, 0x53},      // {address of ID, ID },
    {0x03, 0x04},      // {address of ID, ID },
    {0x04, 0x44},      // {address of ID, ID },
    {0x05, 0x10},      // {address of ID, ID },
};

const I2C_ARRAY Sensor_init_table_fps_30[] = {
    // 100 99 2560 1440
    //;mpll_clk=720M, timer_clk=72M, dpll_clk=864M, cnt_clk=216M, row_clk=36M, hts=782, vts=1473, fps=30
    {0xfd ,0x00},
    {0x20 ,0x00},
    {0x20 ,0x01},
    {0x20 ,0x01},
    {0x20 ,0x01},
    {0x20 ,0x01},
    {0x41 ,0xa8},
    {0x45 ,0x24},
    {0x31 ,0x20},
    {0x38 ,0x15},
    {0xfd ,0x01},
    {0x03 ,0x00},
    {0x04 ,0x04},
    {0x05 ,0x00},
    {0x06 ,0x01},
    {0x24 ,0xff},
    {0x02 ,0x01},
    {0x42 ,0x5a},
    {0x47 ,0x0c},
    {0x45 ,0x02},
    {0x48 ,0x0c},
    {0x4b ,0x88},
    {0xd4 ,0x05},
    {0xd5 ,0xd2},
    {0xd7 ,0x05},
    {0xd8 ,0xd2},
    {0x50 ,0x01},
    {0x51 ,0x11},
    {0x52 ,0x18},
    {0x53 ,0x01},
    {0x54 ,0x01},
    {0x55 ,0x01},
    {0x57 ,0x08},
    {0x5c ,0x40},
    {0x7c ,0x06},
    {0x7d ,0x05},
    {0x7e ,0x05},
    {0x7f ,0x05},
    {0x90 ,0x60},
    {0x91 ,0x0f},
    {0x92 ,0x35},
    {0x93 ,0x36},
    {0x94 ,0x0f},
    {0x95 ,0x7e},
    {0x98 ,0x5d},
    {0xa8 ,0x50},
    {0xaa ,0x14},
    {0xab ,0x05},
    {0xac ,0x14},
    {0xad ,0x05},
    {0xae ,0x4a},
    {0xaf ,0x0e},
    {0xb2 ,0x07},
    {0xb3 ,0x0c},
    {0xc9 ,0x28},
    {0xca ,0x5e},
    {0xcb ,0x5e},
    {0xcc ,0x5e},
    {0xcd ,0x5e},
    {0xce ,0x5c},
    {0xcf ,0x5c},
    {0xd0 ,0x5c},
    {0xd1 ,0x5c},
    {0xd2 ,0x7c},
    {0xd3 ,0x7c},
    {0xdb ,0x0f},

    {0xfd ,0x01},
    {0x46 ,0x77},
    {0xdd ,0x00},
    {0xde ,0x3f},
    {0xfd ,0x03},
    {0x2b ,0x0a},
    {0x01 ,0x22},
    {0x02 ,0x03},
    {0x00 ,0x06},
    {0x2a ,0x22},
    {0x29 ,0x0b},
    {0x1e ,0x10},
    {0x1f ,0x02},
    {0x1a ,0x24},
    {0x1b ,0x62},
    {0x1c ,0xce},
    {0x1d ,0xd3},
    {0x04 ,0x0f},
    {0x36 ,0x00},
    {0x37 ,0x05},
    {0x38 ,0x09},
    {0x39 ,0x19},
    {0x3a ,0x38},
    {0x3b ,0x22},
    {0x3c ,0x22},
    {0x3d ,0x22},
    {0x3e ,0x03},

    {0xfd ,0x02},
    {0xce ,0x65},
    {0xfd ,0x03},
    {0x03 ,0x30},
    {0x05 ,0x00},
    {0x12 ,0x70},
    {0x13 ,0x70},
    {0x16 ,0x13},
    {0x21 ,0xca},
    {0x27 ,0x95},
    {0x2c ,0x55},
    {0x2d ,0x08},
    {0x2e ,0xca},
    {0x3f ,0xe7},
    {0xfd ,0x00},
    {0x8b ,0x01},
    {0x8d ,0x00},
    {0xfd ,0x01},
    {0x01 ,0x02},
    {0xfd ,0x05},
    {0xc4 ,0x62},
    {0xc5 ,0x62},
    {0xc6 ,0x62},
    {0xc7 ,0x62},
#if 1
    {0xf0, 0x41}, // ;blc Gb offset
    {0xf1, 0x41}, // ;blc B offset
    {0xf2, 0x41}, // ;blc R offset
    {0xf3, 0x41}, // ;blc Gr offset
#else
    {0xf0, 0x00}, // ;blc Gb offset
    {0xf1, 0x00}, // ;blc B offset
    {0xf2, 0x00}, // ;blc R offset
    {0xf3, 0x00}, // ;blc Gr offset
#endif
    {0xf4 ,0x00},
    {0xf9 ,0x03},
    {0xfa ,0x5d},
    {0xfb ,0x6b},
    {0xfd ,0x02},
    {0x5e ,0x32},

    {0x92, 0x03},//add non continue mode

    {0xfd ,0x02},
    {0xa0 ,0x00},
    {0xa1 ,0x04},
    {0xa2 ,0x05},
    {0xa3 ,0xa0},
    {0xa4 ,0x00},
    {0xa5 ,0x04},
    {0xa6 ,0x0a},
    {0xa7 ,0x00},
    {0x8e ,0x0a},
    {0x8f ,0x00},
    {0x90 ,0x05},
    {0x91 ,0xb0},
    {0xfd ,0x05},
    {0xb1 ,0x01},
    {0xfd ,0x00},
    {0x20 ,0x03},

};
const I2C_ARRAY Sensor_init_table_fps_15[] = {
//100 99 2560 1440
//;mpll_clk=720M, timer_clk=72M, dpll_clk=864M, cnt_clk=216M, row_clk=36M, hts=782, vts=1473, fps=30
/*  {0xfd, 0x00},
    {0x20, 0x00},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x31, 0x20},  //;row clk from dpll
    {0x38, 0x15},  //;enable clk
    {0xfd, 0x01},  //
    {0x03, 0x01},  //;exp 8msb
    {0x04, 0x04},  //;exp 8lsb
    {0x06, 0x01},  //;vb 8lsb
    {0x24, 0xff},  // ;analog gain
    {0x45, 0x02},  //;scg en
    {0x48, 0x0c},  //;vb_psv_exp_en,vb_psv_fl_en
    {0x4b, 0x88},  //;dummy row1,dummy row2
    {0xd4, 0x05},  //;ulp_fl_limit[15:8]
    {0xd5, 0xd2},  //;ulp_fl_limit[7:0], ulp_fl=1490
    {0xd7, 0x05},  //;ulp_exp_limit[15:8]
    {0xd8, 0xd2},  //;ulp_exp_limit[7:0], ulp_exp=1490
    {0x50, 0x01},  //;p50
    {0x51, 0x11},  //;p51
    {0x52, 0x18},  //;p52
    {0x53, 0x01},  //;p53
    {0x54, 0x01},  //;p54
    {0x55, 0x01},  //;p55
    {0x57, 0x08},  //;p57
    {0x5c, 0x40},  //;p5c
    {0x7c, 0x1b},  //;p7c
    {0x90, 0x60},  //;p90
    {0x91, 0x0f},  // ;p91
    {0x92, 0x30},  //;p92
    {0x93, 0x3a},  // ;p93
    {0x94, 0x0f},  // ;p94
    {0x95, 0x84},  //;p95
    {0x98, 0x5d},  //;p98
    {0xa8, 0x50},  //;pa8
    {0xaa, 0x14},  //;paa
    {0xab, 0x05},  //;pab
    {0xac, 0x14},  //;pac
    {0xad, 0x05},  //;pad
    {0xae, 0x44},  //;pae
    {0xaf, 0x10},  // ;paf
    {0xc9, 0x28},  //;cap_s_pd_rst_en_hcg,cap_r_fd_rst_en_hcg
    {0xca, 0x5e},  //;p86_x1 sc1
    {0xcb, 0x5e},  //;p86_x2 sc1
    {0xcc, 0x5e},  //;p86_x3 sc1
    {0xcd, 0x5e},  //;p86_x4 sc1
    {0xce, 0x5c},  //;p88_x1 sc2
    {0xcf, 0x5c},  //;p88_x2 sc2
    {0xd0, 0x5c},  //;p88_x3 sc2
    {0xd1, 0x5c},  //;p88_x4 sc2
    {0xd2, 0x7c},  //;col cap10
    {0xd3, 0x7c},  //;col cap32
    {0xdb, 0x2f},  //;bitline bias, comp1 bias
    {0xfd, 0x01},
    {0x46, 0x77},
    {0xdd, 0x00},
    {0xde, 0x3f},
    {0xfd, 0x03},
    {0x2b, 0x0a},
    {0x01, 0x24},
    {0x02, 0x03},
    {0x00, 0x06},
    {0x2a, 0x22},
    {0x29, 0x13},
    {0x1e, 0x10},
    {0x1f, 0x02},
    {0x1a, 0x20},
    {0x1b, 0x5f},
    {0x1c, 0xc8},
    {0x1d, 0xca},
    {0x04, 0x0f},
    {0x36, 0x00},
    {0x37, 0x03},
    {0x38, 0x09},
    {0x39, 0x17},
    {0x3a, 0x34},
    {0x3b, 0x22},
    {0x3c, 0x22},
    {0x3d, 0x22},
    {0x3e, 0x13},
    {0xfd, 0x02},
    {0xce, 0x65},  //;frame end dly
    {0xfd, 0x03},  //
    {0x05, 0x02},  //;adc range 590mv
    {0x12, 0x20},  //;rcnt_num[7:0]
    {0x13, 0x40},  //;scnt_num[7:0]
    {0x21, 0xca},  //;vref_bsun_rst_hcg
    {0x27, 0x85},  //;bsun_rst_dn_en,bsun_sig_dn_en
    {0x2c, 0x55},  // ;vn1=vn2=-1.3v
    {0x2d, 0x05},  //;vh=3.25v
    {0x2e, 0xca},  //;vref_bsun_rst_lcg
    {0x3f, 0xf7},  // ;vcap=avdd
    {0xfd, 0x00},  //
    {0x8b, 0x01},  //;mipi p2s rst=0
    {0x8d, 0x00},  //;mipi pwd sel=0
    {0xfd, 0x01},  //
    {0x01, 0x02},  //
    {0xfd, 0x05},  //
    {0xf0, 0x40},  // ;blc Gb offset
    {0xf1, 0x40},  // ;blc B offset
    {0xf2, 0x40},  // ;blc R offset
    {0xf3, 0x40},  // ;blc Gr offset
    {0xf4, 0x00},  // ;random gain limit
    {0xfa, 0x5d},  // ;blc trig en
    {0xfb, 0x69},  // ;blc en, blc_bpc_en, blc_filter_en, random_en
    {0xb1, 0x01},  //;mipi en
    {0xfd, 0x00},  //
    {0x20, 0x03},  //;logic start
    {0xfd, 0x01},  //
    {0x03, 0x02},  //;exp 8msb
    {0x04, 0x04},  //;exp 8lsb */

    {0xfd, 0x00},
    {0x20, 0x00},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x2e, 0x35},
    {0x31, 0x20},  //;row clk from dpll
    {0x38, 0x15},  //;enable clk
    {0xfd, 0x01},  //
    {0x03, 0x00},  //;exp 8msb
    {0x04, 0x04},  //;exp 8lsb
    {0x06, 0x4f},  //;vb 8lsb
    {0x24, 0xff},  // ;analog gain
    {0x42, 0x59},  //;rowsg/rowsel timing sel
    {0x45, 0x02},  //;scg en
    {0x48, 0x0c},  //;vb_psv_exp_en,vb_psv_fl_en
    {0x4b, 0x88},  //;dummy row1,dummy row2
    {0xd4, 0x05},  //;ulp_fl_limit[15:8]
    {0xd5, 0xd2},  //;ulp_fl_limit[7:0], ulp_fl=1490
    {0xd7, 0x05},  //;ulp_exp_limit[15:8]
    {0xd8, 0xd2},  //;ulp_exp_limit[7:0], ulp_exp=1490
    {0x50, 0x01},  //;p50
    {0x51, 0x11},  //;p51
    {0x52, 0x18},  //;p52
    {0x53, 0x01},  //;p53
    {0x54, 0x01},  //;p54
    {0x55, 0x01},  //;p55
    {0x57, 0x08},  //;p57
    {0x5c, 0x40},  //;p5c
    {0x7c, 0x1b},  //;p7c
    {0x90, 0x60},  //;p90
    {0x91, 0x0f},  // ;p91
    {0x92, 0x30},  //;p92
    {0x93, 0x3a},  // ;p93
    {0x94, 0x0f},  // ;p94
    {0x95, 0x84},  //;p95
    {0x98, 0x5d},  //;p98
    {0xa8, 0x50},  //;pa8
    {0xaa, 0x14},  //;paa
    {0xab, 0x05},  //;pab
    {0xac, 0x14},  //;pac
    {0xad, 0x05},  //;pad
    {0xae, 0x47},  //;pae
    {0xaf, 0x10},  // ;paf
    {0xc9, 0x28},  //;cap_s_pd_rst_en_hcg,cap_r_fd_rst_en_hcg
    {0xca, 0x5e},  //;p86_x1 sc1
    {0xcb, 0x5e},  //;p86_x2 sc1
    {0xcc, 0x5e},  //;p86_x3 sc1
    {0xcd, 0x5e},  //;p86_x4 sc1
    {0xce, 0x5c},  //;p88_x1 sc2
    {0xcf, 0x5c},  //;p88_x2 sc2
    {0xd0, 0x5c},  //;p88_x3 sc2
    {0xd1, 0x5c},  //;p88_x4 sc2
    {0xd2, 0x7c},  //;col cap10
    {0xd3, 0x7c},  //;col cap32
    {0xdb, 0x2f},  //;bitline bias, comp1 bias
//;;;psrr;;;;;;
    {0xfd, 0x01},
    {0x46, 0x77},
    {0xdd, 0x00},
    {0xde, 0x3f},
    {0xfd, 0x03},
    {0x2b, 0x0a},
    {0x01, 0x24},
    {0x02, 0x03},
    {0x00, 0x06},
    {0x2a, 0x22},
    {0x29, 0x13},
    {0x1e, 0x10},
    {0x1f, 0x02},
    {0x1a, 0x20},
    {0x1b, 0x5f},
    {0x1c, 0xc8},
    {0x1d, 0xca},
    {0x04, 0x0f},
    {0x36, 0x00},
    {0x37, 0x03},
    {0x38, 0x09},
    {0x39, 0x17},
    {0x3a, 0x34},
    {0x3b, 0x22},
    {0x3c, 0x22},
    {0x3d, 0x22},
    {0x3e, 0x13},
//;;;;;;;;;;;;;;;;
    {0xfd, 0x02},
    {0xce, 0x65},  //;frame end dly
    {0xfd, 0x03},  //
    {0x03, 0x30},  //;vcap bias
    {0x05, 0x00},  //;adc range 551mv
    {0x12, 0x20},  //;rcnt_num[7:0]
    {0x13, 0x40},  //;scnt_num[7:0]
    {0x21, 0xca},  //;vref_bsun_rst_hcg
    {0x27, 0x85},  //;bsun_rst_dn_en,bsun_sig_dn_en
    {0x2c, 0x55},  // ;vn1=vn2=-1.3v
    {0x2d, 0x08},  //;vh=3.4v
    {0x2e, 0xca},  //;vref_bsun_rst_lcg
    {0x3f, 0xe7},  // ;vcap=2.2v
    {0xfd, 0x00},  //
    {0x8b, 0x01},  //;mipi p2s rst=0
    {0x8d, 0x00},  //;mipi pwd sel=0
    {0xfd, 0x01},
    {0x01, 0x02},
    {0xfd, 0x05},
#if 1
    {0xf0, 0x41}, // ;blc Gb offset
    {0xf1, 0x41}, // ;blc B offset
    {0xf2, 0x41}, // ;blc R offset
    {0xf3, 0x41}, // ;blc Gr offset
#else
    {0xf0, 0x00}, // ;blc Gb offset
    {0xf1, 0x00}, // ;blc B offset
    {0xf2, 0x00}, // ;blc R offset
    {0xf3, 0x00}, // ;blc Gr offset
#endif
    {0xf4, 0x00},  //;random gain limit
    {0xf9, 0x03},  //;trig frame count
    {0xfa, 0x5d},  //;blc trig en
    {0xfb, 0x6b},  //;blc en, blc_bpc_en, blc_filter_en, random_en, 4 frame average en

//psrr
    {0xfd, 0x03},
    {0x1d, 0xd3},
    {0x29, 0x0b},
    {0x1a, 0x24},
    {0x1b, 0x62},
    {0x1c, 0xce},
    {0x37, 0x05},
    {0x3e, 0x03},
    {0x01, 0x22},
    {0x3a, 0x38},
    {0x38, 0x0b},
    {0x39, 0x19},

     //digital crop to 2560x1440
    {0xfd, 0x02},
    {0x5e, 0x32}, //digital crop enable
    {0xfd, 0x02}, //

    {0x92, 0x03},//add non continue mode

    {0xa0, 0x00}, //
    {0xa1, 0x04}, //row_start
    {0xa2, 0x05}, //
    {0xa3, 0xa0}, //row_size
    {0xa4, 0x00}, //
    {0xa5, 0x04}, //col_start
    {0xa6, 0x0a}, //
    {0xa7, 0x00}, //;col_size
    {0x8e, 0x0a}, //
    {0x8f, 0x00}, //mipi col_size
    {0x90, 0x05}, //
    {0x91, 0xb0}, //mipi row_size

    {0xfd, 0x05},
    {0xb1, 0x01}, // ;mipi en
    {0xfd, 0x00},
    {0x20, 0x03}, // ;logic start
};

I2C_ARRAY Sensor_init_table_360p_fps_120[] = {
    {0xfd, 0x00},
    {0x20, 0x00},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x20, 0x01},
    {0x41, 0xa8}, //;dpll_nc
    {0x45, 0x24}, //;row_clk_div7
    {0x30, 0x02}, //;mpll clk div(mpll_predivp_sel)
    {0x31, 0x24}, //;row clk from dpll，dclk=pclk_in/4
    {0x35, 0xc9}, //;phy clk div(mpll_phy_clk_div_sel)
    {0x38, 0x15}, //;enable clk
    {0xfd, 0x01},
    //{0x03, 0x00}, //;exp 8msb
    //{0x04, 0xa5}, //;exp 8lsb
    //{0x24, 0x10}, //;again
    {0x12, 0x20}, //default is 0x20,sync with OV
    {0x06, 0x01}, //;vb 8lsb
    {0x31, 0x26}, //;binning4_en, v_binning_en,v_binning4_en
    {0x02, 0x01}, //;cms en
    {0x42, 0x5a},
    {0x47, 0x0c},
    //{0x45, 0x02}, //;scg en
    {0x48, 0x0c}, //;vb_psv_exp_en,vb_psv_fl_en
    {0x4b, 0x88}, //;dummy row1,dummy row2
    {0xd4, 0x05}, //;ulp_fl_limit[15:8]
    {0xd5, 0xd2}, //;ulp_fl_limit[7:0], ulp_fl=1490
    {0xd7, 0x05}, //;ulp_exp_limit[15:8]
    {0xd8, 0xd2}, //;ulp_exp_limit[7:0], ulp_exp=1490
    {0x50, 0x01}, //;p50
    {0x51, 0x11}, //;p51
    {0x52, 0x18}, //;p52
    {0x53, 0x01}, //;p53
    {0x54, 0x01}, //;p54
    {0x55, 0x01}, //;p55
    {0x57, 0x08}, //;p57
    {0x5c, 0x40}, //;p5c
    {0x7c, 0x06},
    {0x7d, 0x05},
    {0x7e, 0x05},
    {0x7f, 0x05},
    {0x90, 0x60}, //;p90
    {0x91, 0x0f}, // ;p91
    {0x92, 0x35}, //;p92
    {0x93, 0x36},
    {0x94, 0x0f}, // ;p94
    {0x95, 0x7e}, //;p95
    {0x98, 0x5d}, //;p98
    {0xa8, 0x50}, //;pa8
    {0xaa, 0x14}, //;paa
    {0xab, 0x05}, //;pab
    {0xac, 0x14}, //;pac
    {0xad, 0x05}, //;pad
    {0xae, 0x4a}, //;pae
    {0xaf, 0x0e},
    {0xb2, 0x07},
    {0xb3, 0x0c},
    {0xc9, 0x28}, //;cap_s_pd_rst_en_hcg,cap_r_fd_rst_en_hcg
    {0xca, 0x5e}, //;p86_x1 sc1
    {0xcb, 0x5e}, //;p86_x2 sc1
    {0xcc, 0x5e}, //;p86_x3 sc1
    {0xcd, 0x5e}, //;p86_x4 sc1
    {0xce, 0x5c}, //;p88_x1 sc2
    {0xcf, 0x5c}, //;p88_x2 sc2
    {0xd0, 0x5c}, //;p88_x3 sc2
    {0xd1, 0x5c}, //;p88_x4 sc2
    {0xd2, 0x7c}, //;col cap10
    {0xd3, 0x7c}, //;col cap32
    {0xdb, 0x0f},

    {0xfd, 0x01},
    {0x46, 0x77},
    {0xdd, 0x00},
    {0xde, 0x3f},
    {0xfd, 0x03},
    {0x2b, 0x0a},
    {0x01, 0x22}, //  ;24
    {0x02, 0x03},
    {0x00, 0x06},
    {0x2a, 0x22},
    {0x29, 0x0b}, // ;13
    {0x1e, 0x10},
    {0x1f, 0x02},
    {0x1a, 0x24}, // ;20
    {0x1b, 0x62}, // ;5f
    {0x1c, 0xce}, //  ;c8
    {0x1d, 0xd3}, // ;ca
    {0x04, 0x0f},
    {0x36, 0x00},
    {0x37, 0x05}, //  ;03
    {0x38, 0x09},
    {0x39, 0x19}, //  ;17
    {0x3a, 0x38}, //  ;34
    {0x3b, 0x22},
    {0x3c, 0x22},
    {0x3d, 0x22},
    {0x3e, 0x03}, //  ;13

    {0xfd, 0x02},
    {0xc1, 0x05}, //;tx speed area sel
    {0x8c, 0x03}, //;CLK trail
    {0x8d, 0x01}, //;CLK prepare
    {0x95, 0x02}, //;HS prepare
    {0x98, 0x02}, //;HS trail
    {0x5e, 0x22}, //;dig windown en, auto first en
    {0xa1, 0x00}, //;dig v start 8lsb
    {0xa2, 0x01}, //;dig v size 3msb
    {0xa3, 0x68}, //;dig v size 8lsb
    {0xa5, 0x02}, //;dig h start 8lsb
    {0xa6, 0x02}, //;dig h size 4msb
    {0xa7, 0x80}, //;dig h size 8lsb
    {0x8e, 0x02}, //;mipi hsize 4msb
    {0x8f, 0x80}, // ;mipi hsize 8lsb
    {0x90, 0x01}, //;mipi v size 3msb
    {0x91, 0x68}, //;mipi vsize 8lsb
    {0xce, 0x65}, //;frame end dly
    {0xfd, 0x03},
    {0x03, 0x30}, //;vcap bias
    {0x05, 0x00}, //;adc range 551mv
    {0x12, 0x70}, //;rcnt_num[7:0]
    {0x13, 0x70}, //;scnt_num[7:0]
    {0x16, 0x13}, //;rst_num1_hcg_1x
    {0x21, 0xca}, //;vref_bsun_rst_hcg
    {0x27, 0x95},
    {0x2c, 0x55}, // ;vn1=vn2=-1.3v
    {0x2d, 0x08}, //;vh=3.4v
    {0x2e, 0xca}, //;vref_bsun_rst_lcg
    {0x3f, 0xe7}, // ;vcap=2.2v
    {0xfd, 0x00}, //
    {0x8b, 0x01}, //;mipi p2s rst=0
    {0x8d, 0x00}, //;mipi pwd sel=0
    {0xfd, 0x01},
    {0x01, 0x02},
    {0xfd, 0x05},
    {0xc4, 0x62}, //;blc k b
    {0xc5, 0x62}, //;blc k r
    {0xc6, 0x62}, //;blc k gr
    {0xc7, 0x62}, //;blc k gb
    {0xce, 0x3e}, //;blc
#if 0
    {0xf0, 0x40}, // ;blc Gb offset
    {0xf1, 0x40}, // ;blc B offset
    {0xf2, 0x40}, // ;blc R offset
    {0xf3, 0x40}, // ;blc Gr offset
#else
    {0xf0, 0x00}, // ;blc Gb offset
    {0xf1, 0x00}, // ;blc B offset
    {0xf2, 0x00}, // ;blc R offset
    {0xf3, 0x00}, // ;blc Gr offset
#endif
    {0xf4, 0x00}, // ;random gain limit
    {0xf9, 0x03}, // ;trig frame count
    {0xfa, 0x5d}, // ;blc trig en
    {0xfb, 0x6b}, // ;blc en, blc_bpc_en, blc_filter_en, random_en,4 frame average en

    {0xff, 0x00},

    //{0xb1, 0x01}, //;mipi en
    {0xfd, 0x01},
    {0x33, 0x03}, //init shutter/gain Effective immediately

    {0xfd, 0x00},
    {0x38, 0x35},
    {0xfd, 0x02},
    {0x81, 0x10},
    //{0x82,0x2f},//target ae
    {0x70, 0xcf},
    {0x7b, 0x2c},
    {0x77, 0xfe},

    //{0xfd, 0x01},
    //{0x33, 0x03},
    //{0x01, 0x02},
    {0xfd, 0x00},
    {0x20, 0x03},
};

I2C_ARRAY Sensor_init_table_360p_to_1440p[] = {
    {0xfd, 0x00},
    {0x20, 0x01},
    {0x38, 0x15},
    {0xfd, 0x02},
    {0x81, 0xff},
    {0x82, 0x80},
    {0x70, 0xc2},
    {0x7b, 0x34},

    {0xfd, 0x00},
    {0x31, 0x20},
    {0x35, 0x09},

    {0xfd, 0x01},
    {0x31, 0x00},

    {0xfd, 0x02},
    {0xc1, 0x03},
    {0x8c, 0x07},
    {0x8d, 0x05},
    {0x95, 0x06},
    {0x98, 0x08},
    {0x5e, 0x32},
    {0x92, 0x03},
    {0xa0, 0x00},
    {0xa1, 0x04},
    {0xa2, 0x05},
    {0xa3, 0xa0},
    {0xa4, 0x00},
    {0xa5, 0x04},
    {0xa6, 0x0a},
    {0xa7, 0x00},
    {0x8e, 0x0a},
    {0x8f, 0x00},
    {0x90, 0x05},
    {0x91, 0xb0},

    {0xfd, 0x05},
    {0xce, 0x0e},
#if 1
    {0xf0, 0x40}, // ;blc Gb offset
    {0xf1, 0x40}, // ;blc B offset
    {0xf2, 0x40}, // ;blc R offset
    {0xf3, 0x40}, // ;blc Gr offset
#else
    {0xf0, 0x00}, // ;blc Gb offset
    {0xf1, 0x00}, // ;blc B offset
    {0xf2, 0x00}, // ;blc R offset
    {0xf3, 0x00}, // ;blc Gr offset
#endif
    {0xfd, 0x05},
    {0xb1, 0x00}, //;mipi off
    {0xff, 0xff}, // aenotify flag
    {0xfd, 0x00},
    {0xe7, 0x03},
    {0xe7, 0x00},
    {0xfd, 0x01},
    {0x33, 0x03},
    {0x01, 0x02},
    {0x01, 0x02},

    {0xfd, 0x00},
    {0x20, 0x03},
    {0xfd, 0x05},
    {0xb1, 0x01}, //;mipi en
};
const I2C_ARRAY TriggerStartTbl[] = {

};

const I2C_ARRAY PatternTbl[] = {
    //{0xb2,0x40}, //colorbar pattern , bit 0 to enable
};

const I2C_ARRAY mirror_reg[] = {
    {0xfd, 0x01},
    {0x32, 0x00}, //P1 M0F0 [1]:F [0]:M
    {0xfd, 0x02}, //
    {0x5e, 0x22}, //mem down en + enable auto BR first
    {0xfd, 0x01},
    {0x01, 0x01},
};

/////////////////////////////////////////////////////////////////
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

typedef struct
{
    short reg;
    char  startbit;
    char  stopbit;
} COLLECT_REG_SET;

const static I2C_ARRAY gain_reg[] = {
    {0xfd, 0x01},
    {0x24, 0x20},//again 1x:0x10, 15.5x:0xf8
    {0xfd, 0x05},
    {0x39, 0x40},//dgain[7:0] 1x:0x40 32x:0x7ff
    {0x37, 0x00},//dgain[10:8]
    //{0xfd, 0x00},
};

const I2C_ARRAY expo_reg[] = {
    {0xfd, 0x01},
    {0x03, 0x01},
    {0x04, 0x00},
};

const I2C_ARRAY aec_expo_reg[] = {
    {0x22, 0x00},
    {0x23, 0x00},
};

const I2C_ARRAY vts_reg[] = {
    {0xfd, 0x01},
    {0x12, 0x21},//0x10 enable
    {0x0C, 0x04},//MSB
    {0x0D, 0x50},//LSB
};

const I2C_ARRAY lcg_hcg_reg[] = {
    {0xfd, 0x01},
    {0x45, 0x00},
    //{0x01, 0x01},
};

const I2C_ARRAY ae_target_reg[] = {
    {0xfd, 0x02},
    {0x82, 0x00},
};

CUS_INT_TASK_ORDER def_order = {
    .RunLength = 9,
    .Orders =
        {
            CUS_INT_TASK_AE | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_AWB | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_AE | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_AWB | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_AE | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_AWB | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
            CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        },
};

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME OS04D10

#define SensorReg_Read(_reg, _data)  (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg), _reg, _data))
#define SensorReg_Write(_reg, _data) (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg), _reg, _data))
#define SensorRegArrayW(_reg, _len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg), (_reg), (_len)))
#define SensorRegArrayR(_reg, _len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg), (_reg), (_len)))

int cus_camsensor_release_handle(ss_cus_sensor *handle);

/////////////////// sensor hardware dependent //////////////

// Sensor Sleep Mode
#define SENSOR_SLEEP_MODE_ENABLE  0
#define SENSOR_SLEEP_MODE_DISABLE 1

static int g_u8SuspendFlag = 0;

static int OS04D10_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    OS04D10_params *params   = (OS04D10_params *)handle->private_data;
    info->u8AEGainDelay      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min   = params->Line_Period * 2;
    info->u32AEShutter_step  = params->Line_Period;
    info->u32AEShutter_max   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

static int _pCus_SleepMode(ss_cus_sensor *handle, u32 sleepModeEnable, u32 idx)
{
    ISensorIfAPI *  sensor_if = handle->sensor_if_api;
    OS04D10_params *params    = (OS04D10_params *)handle->private_data;

    SENSOR_DMSG("line:%d, sleepModeEnable:%d !!!\n", __LINE__, sleepModeEnable);
    switch (sleepModeEnable)
    {
        // os04d10 hardware sleep cannot save register, use software sleep
        case SENSOR_SLEEP_MODE_ENABLE:
        {
            SensorReg_Write(0xfd, 0x00);
            SensorReg_Write(0x36, 0x07);
            SensorReg_Write(0x20, 0x01);
            params->stUserDefInfo.bSnrSleep = true;
            break;
        }
        case SENSOR_SLEEP_MODE_DISABLE:
        {
            OS04D10_params *params = (OS04D10_params *)handle->private_data;
            SensorReg_Write(0xfd, 0x00);
            SensorReg_Write(0x36, 0x00);
            CamOsMsDelay(1);
            if (params->reg_dirty)
            {
                // SENSOR_EMSG("setting expo !!!\n");
                // SensorReg_Write(0x01,0x00);//frame sync disable
                //  params->tExpo_reg[1].data = 0;
                //  params->tExpo_reg[2].data = 8;

                SensorRegArrayW((I2C_ARRAY *)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tLcg_Hcg_reg, ARRAY_SIZE(lcg_hcg_reg));
                params->reg_dirty = false;
            }
            SensorReg_Write(0xfd, 0x01); // page 1,
            //per resume,must set 0x33,0x01 reg value(enable quick expo),For dual sensor frame mode happend double vsync,rdm:146bcb2b-4132-42a1-88ca-5bf582b0c6fb
            SensorReg_Write(0x33, 0x03); // page 1
            SensorReg_Write(0x01, 0x02); // page 1
            SensorReg_Write(0xfd, 0x00);
            SensorReg_Write(0x20, 0x03);
            params->stUserDefInfo.bSnrSleep = false;
            break;
        }
        default:
            SENSOR_EMSG("do nothing ,Sleep Mode only On/Off !!!\n");
            break;
    }
    sensor_if->SetUserDefInfo(idx, params->stUserDefInfo);
    return 0;
}

static void OS04D10_GetOtpData(ss_cus_sensor *handle)
{
    // The i2c operation here takes 4ms
    // Follow-up check to see if you can speed up the earlyinit process
    u16             otp_data        = 0;
    OS04D10_params *params          = (OS04D10_params *)handle->private_data;
    static bool     is_get_otp_data = true;
    if (is_get_otp_data)
    {
        SensorReg_Write(0xfd, 0x00);
        SensorReg_Write(0x53, 0x01);
        SensorReg_Write(0xfd, 0x06);
        SensorReg_Write(0x9f, 0x00);
        SensorReg_Write(0x84, 0x40);

        SensorReg_Write(0xfd, 0x06);
        SensorReg_Write(0x89, 0x14);
        SensorReg_Write(0x8b, 0x14);
        SensorReg_Write(0x2f, 0x01);
        SensorReg_Read(0x31, &otp_data);
        if (otp_data)
        {
            params->otp_data = 4 * 64 * 1024 / otp_data;
        }
        //SENSOR_EMSG("\n>>>>>>>>>>>>>>[%s]:otp_data[%d] params->otp_data[%d] \n",__FUNCTION__, otp_data,params->otp_data);
        SensorReg_Write(0xfd, 0x01);
        SensorReg_Read(0x45, &linear_last_dcg_data);
        //SENSOR_EMSG("\n>>[%s] otp_data[%d] linear_last_dcg_data[%d]\n",__FUNCTION__,params->otp_data,linear_last_dcg_data);
        is_get_otp_data = false;
    }
}

static int pCus_Reopen(struct __ss_cus_sensor *handle, u32 idx)
{
    OS04D10_params *params   = (OS04D10_params *)handle->private_data;
    ISensorIfAPI  *sensor_if = handle->sensor_if_api;
    if(params->bFastAE_en == false)
    {
        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
        sensor_if->MCLK(idx, 1, handle->mclk);

        CamOsMsDelay(2); //xshutdown keep high, mclk on-> 2ms -> i2c
        _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_DISABLE, idx);
    }

    g_u8SuspendFlag = 0;

    SENSOR_DMSG("[%s] sensor reopen!\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    OS04D10_params *params    = (OS04D10_params *)handle->private_data;
    ISensorIfAPI *  sensor_if = handle->sensor_if_api;
    s32             res       = 0;
    // os04d10 not have pwdn pin
    // SENSOR_EMSG("[%s] ", __FUNCTION__);
    {
        SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
        res = sensor_if->Reset(idx, params->reset_POLARITY);
        if (res >= 0)
            SENSOR_USLEEP(5000);

        // Sensor power on sequence
        sensor_if->MCLK(idx, 1, handle->mclk);
        sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
        sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
        res = sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);

        SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
        res = sensor_if->Reset(idx, !params->reset_POLARITY); // 1  >  0  >  1
        if (res >= 0)
            SENSOR_USLEEP(8000);
    }
    params->stUserDefInfo.bSnrSleep = false;
    // handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    OS04D10_params *params = (OS04D10_params *)handle->private_data;
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    if (g_u8SuspendFlag == 0)
    {
        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
        sensor_if->MCLK(idx, 0, handle->mclk);
        SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
        sensor_if->PowerOff(idx, params->pwdn_POLARITY);
        sensor_if->Reset(idx, params->pwdn_POLARITY);
        SENSOR_USLEEP(5000);
    }
    return SUCCESS;
}

/////////////////// image function /////////////////////////
// Get and check sensor ID
// if i2c error or sensor id does not match then return FAIL
static int pCus_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
    int       i, n;
    int       table_length = ARRAY_SIZE(Sensor_id_table);
    I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    for (n = 0; n < table_length; ++n)
    {
        id_from_sensor[n].reg  = Sensor_id_table[n].reg;
        id_from_sensor[n].data = 0;
    }

    *id = 0;
    if (table_length > 8)
        table_length = 8;

    for (n = 0; n < 5; ++n)
    { // retry , until I2C success
        if (n > 3)
            return FAIL;

        if (SensorRegArrayR((I2C_ARRAY *)id_from_sensor, table_length) == SUCCESS) // read sensor ID from I2C
            break;
        else
            SENSOR_MSLEEP_(1);
    }

    for (i = 0; i < table_length; ++i)
    {
        if (id_from_sensor[i].data != Sensor_id_table[i].data)
        {
            SENSOR_DMSG("[%s]Read OS04D10 id: 0x%x 0x%x\n", __FUNCTION__, id_from_sensor[0].data,
                        id_from_sensor[1].data);
            return FAIL;
        }
        *id = id_from_sensor[i].data;
    }
    SENSOR_DMSG("[%s]Read OS04D10 id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}

static int OS04D10_SetPatternMode(ss_cus_sensor *handle, u32 mode)
{
    return SUCCESS;
}

static int OS04D10_SetFPS(ss_cus_sensor *handle, u32 fps);

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);

static int pCus_FastAe(ss_cus_sensor *handle)
{
    int             i      = 0;
    OS04D10_params *params = (OS04D10_params *)handle->private_data;
    while (1)
    {
        u16 u16AEC_done = 0;
        u16 u16exp_lsb = 0, u16exp_msb = 0, u16exp = 0, u16exp2 = 0;
        u16 again = 0, again2 = 0;
        SensorReg_Write(0xfd, 0x01);
        SensorReg_Read(0x20, &u16AEC_done);
        SensorReg_Read(0x04, &u16exp_msb);
        SensorReg_Read(0x24, &again);
        SENSOR_EMSG(">>>>>>>>>>>>>>>aec done %d  reg[0x04, 0x%x], reg[0x24, 0x%x],\n", u16AEC_done, u16exp_msb, again);
        if (u16AEC_done == 1)
        {
            SensorReg_Read(0x22, &u16exp_msb);
            SensorReg_Read(0x23, &u16exp_lsb);
            SensorReg_Read(0x24, &again);

            u16exp = (u16exp_msb & 0xff) << 8 | (u16exp_lsb & 0xff);
            again  = (again & 0xff) / 16;

            SENSOR_EMSG(">>>>>>expmsb 0x%x, lsb 0x%x, exp %d, gain %d, vts %d \n",u16exp_msb,u16exp_lsb,  u16exp, again, params->Init_Vts);
            if(u16exp * again <= (params->Init_Vts-16))
            {
                u16exp2 = u16exp * again;
                again2  = 0x10;
            }
            else
            {
                u16exp2 = params->Init_Vts - 16;
                again2  = (u16exp * again / (params->Init_Vts - 16)) * 16;
            }
            SENSOR_EMSG(">>>>>>>>exp2 %d, gain2 %d\n", u16exp2, again2);

            for (i = 0; i < ARRAY_SIZE(Sensor_init_table_360p_to_1440p); i++)
            {
                if (Sensor_init_table_360p_to_1440p[i].reg == expo_reg[1].reg)
                {
                    Sensor_init_table_360p_to_1440p[i].data = (u16exp2 >> 8) & 0x00ff;
                }

                if (Sensor_init_table_360p_to_1440p[i].reg == expo_reg[2].reg)
                {
                    Sensor_init_table_360p_to_1440p[i].data = u16exp2 & 0x00ff;
                }

                if (Sensor_init_table_360p_to_1440p[i].reg == gain_reg[1].reg)
                {
                    Sensor_init_table_360p_to_1440p[i].data = again2;
                }

                if(SensorReg_Write(Sensor_init_table_360p_to_1440p[i].reg,Sensor_init_table_360p_to_1440p[i].data) != SUCCESS) {
                    return FAIL;
                }
            }
            break;
        }
        else
        {
            SENSOR_USLEEP(1000);
        }
    }

    return 0;
}

static int pCus_init(ss_cus_sensor *handle)
{
    int i, cnt = 0;
    // int j,k,l;
    // int l;
    // u32 idx = handle->i2c_bus->nSensorID;
    OS04D10_params *params = (OS04D10_params *)handle->private_data;
    // u16 otp_data = 0;//,a,b,c,d;

    for (i = 0; i < params->Init_Array_Size; i++)
    {
        if (params->pTable_linear[i].reg == 0xff)
        {
            // SENSOR_EMSG("[DBG] reg 0x%x, 0x%x\n", params->pTable_linear[i].reg , params->pTable_linear[i].data);
            if (params->pTable_linear[i].data != 0 && params->pTable_linear[i].data != 0xFF)
            {
                SENSOR_MSLEEP_(params->pTable_linear[i].data);
            }
            if (params->pTable_linear[i].data == 0)
            {
                SensorRegArrayW((I2C_ARRAY *)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY *)params->tLcg_Hcg_reg, ARRAY_SIZE(lcg_hcg_reg));
                // Read gain for debug
                // for (l = 0; l < ARRAY_SIZE(gain_reg); l++)
                // {
                //    SENSOR_EMSG("[DBG] set gain 0x%x, 0x%x\n", params->tGain_reg[l].reg, params->tGain_reg[l].data);
                // }
                SensorRegArrayW((I2C_ARRAY *)params->tAETarget_reg, ARRAY_SIZE(ae_target_reg));
                // Read AEC target for debug
                //for (j = 0; j < ARRAY_SIZE(ae_target_reg); j++)
                //{
                //    SENSOR_EMSG("[DBG] set AeTarget 0x%x, 0x%x\n", params->tAETarget_reg[j].reg,
                //                params->tAETarget_reg[j].data);
                //}
                // SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                //  Read expo for debug
                // for(k = 0; k < ARRAY_SIZE(expo_reg); k++)
                //{
                //     SENSOR_EMSG("[DBG] set Expo 0x%x, 0x%x\n", params->tExpo_reg[k].reg, params->tExpo_reg[k].data);
                // }
            }
            if (params->pTable_linear[i].data == 0xFF)
            {
                SensorReg_Write(0xfd,0x00);
                SensorReg_Write(0x36,0x00);
                SENSOR_MSLEEP_(1);
                SensorRegArrayW((I2C_ARRAY *)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY *)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY *)params->tVts_reg, ARRAY_SIZE(vts_reg));

                // if(linear_last_dcg_data != params->tLcg_Hcg_reg[1].data)
                {
                    SensorRegArrayW((I2C_ARRAY *)params->tLcg_Hcg_reg, ARRAY_SIZE(lcg_hcg_reg));
                    // for(l = 0; l < ARRAY_SIZE(lcg_hcg_reg); l++)
                    // {
                    //     SENSOR_EMSG("[DBG] set hcg 0x%x, 0x%x\n", params->tLcg_Hcg_reg[l].reg,
                    //     params->tLcg_Hcg_reg[l].data);
                    // }
                }
                // linear_last_dcg_data = params->tLcg_Hcg_reg[1].data;
                SENSOR_EMSG(">>>>>>>>>>pcusinit write exp [0x%x,0x%x], agin 0x%x dgain [%x,%x] hcg_lcg 0x%x, vts[0x%x,0x%x,0x%x]\n ",
                            params->tExpo_reg[1].data, params->tExpo_reg[2].data, params->tGain_reg[1].data,
                            params->tGain_reg[3].data, params->tGain_reg[4].data, params->tLcg_Hcg_reg[1].data,
                            params->tVts_reg[1].data, params->tVts_reg[2].data, params->tVts_reg[3].data);
            }
        }
        else
        {
            cnt = 0;
            while (SensorReg_Write(params->pTable_linear[i].reg, params->pTable_linear[i].data) != SUCCESS
                   && params->pTable_linear[i].reg != 0x20)
            {
                cnt++;
                // SENSOR_DMSG("params->pTable_linear -> Retry %d...\n",cnt);
                if (cnt >= 10)
                {
                    // SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP_(10);
            }
        }
    }
    for (i = 0; i < ARRAY_SIZE(mirror_reg); i++)
    {
        if (SensorReg_Write(params->tMirror_reg[i].reg, params->tMirror_reg[i].data) != SUCCESS)
        {
            return FAIL;
        }
    }
    /*for(i = 0; i < 100; i++)
    {
        SENSOR_MSLEEP_(10);
        SensorReg_Write(0xfd, 0x06);
        SensorReg_Read(0x14, &otp_data);
        printk("fd0x06_0x14=%x\n",otp_data);
    } */
    // g_bSensorFirstInit = true;

    if (0)
    {
        pCus_FastAe(handle);
    }
    #if 0 //for debug
    {
        u16 u16data[10] = {0}, u16exp = 0, u16shutter = 0;
        SensorReg_Write(0xfd, 0x01);
        SensorReg_Read(0x03, &u16data[0]);
        SensorReg_Read(0x04, &u16data[1]);
        SensorReg_Read(0x24, &u16data[2]);
        SensorReg_Read(0x45, &u16data[3]);
        SensorReg_Write(0xfd, 0x05);
        SensorReg_Read(0x37, &u16data[4]);
        SensorReg_Read(0x39, &u16data[5]);
        u16exp     = (u16data[0] & 0xff) << 8 | (u16data[1] & 0xff);
        u16shutter = (u16exp * params->Line_Period) / 1000;
        SENSOR_DMSG(">>>>>>>>>>pcusinit read 0x%x, shtter %d, agin 0x%x dgain [%x,%x] hcg_lcg 0x%x\n", u16exp,
                    u16shutter, u16data[2], u16data[4], u16data[5], u16data[3]);
    }
    #endif
    return SUCCESS;
}

static int pCus_GetVideoResNum(ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int pCus_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res)
    {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int pCus_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res)
    {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    u32             num_res = handle->video_res_supported.num_res;
    OS04D10_params *params  = (OS04D10_params *)handle->private_data;
    if (res_idx >= num_res)
    {
        return FAIL;
    }

    switch (res_idx)
    {
        case LINEAR_RES_1:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init              = pCus_init;
            params->Init_Vts                      = vts_30fps;
            params->expo.fps                      = 30;
            handle->data_prec                     = SENSOR_DATAPREC;
            params->Line_Period                   = Preview_line_period_FPS_30;
            params->pTable_linear                 = Sensor_init_table_fps_30;
            params->Init_Array_Size               = ARRAY_SIZE(Sensor_init_table_fps_30);
            break;
        case LINEAR_RES_2:
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init              = pCus_init;
            params->Init_Vts                      = vts_15fps;
            params->expo.fps                      = 15;
            handle->data_prec                     = SENSOR_DATAPREC;
            params->Line_Period                   = Preview_line_period_FPS_15;
            params->pTable_linear                 = Sensor_init_table_fps_15;
            params->Init_Array_Size               = ARRAY_SIZE(Sensor_init_table_fps_15);
            break;
        case LINEAR_RES_3:
            handle->video_res_supported.ulcur_res = LINEAR_RES_3;
            handle->pCus_sensor_init              = pCus_init;
            params->Init_Vts                      = vts_30fps;
            params->expo.fps                      = 30;
            handle->data_prec                     = SENSOR_DATAPREC;
            params->Line_Period                   = Preview_line_period_FPS_30;
            params->expo.lines = ((1000 * (1000000 / 30) + (params->Line_Period >> 1)) / params->Line_Period);
            handle->sensor_ae_info_cfg.u32AEShutter_min = (params->Line_Period * 1);
            handle->sensor_ae_info_cfg.u32AEShutter_max =
                1000000000 / handle->video_res_supported.res[LINEAR_RES_3].u16min_fps;
            params->pTable_linear   = Sensor_init_table_360p_fps_120;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_init_table_360p_fps_120);
            break;
        case LINEAR_RES_4:
            handle->video_res_supported.ulcur_res = LINEAR_RES_4;
            handle->pCus_sensor_init              = pCus_init;
            params->Init_Vts                      = vts_30fps;
            params->expo.fps                      = 30;
            handle->data_prec                     = SENSOR_DATAPREC;
            params->Line_Period                   = Preview_line_period_FPS_30;
            params->expo.lines = ((1000 * (1000000 / 120) + (params->Line_Period >> 1)) / params->Line_Period);
            handle->sensor_ae_info_cfg.u32AEShutter_min = (params->Line_Period * 1);
            handle->sensor_ae_info_cfg.u32AEShutter_max =
                1000000000 / handle->video_res_supported.res[LINEAR_RES_4].u16min_fps;
            params->pTable_linear   = Sensor_init_table_360p_to_1440p;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_init_table_360p_to_1440p);
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    char            sen_data;
    OS04D10_params *params = (OS04D10_params *)handle->private_data;

    sen_data = params->tMirror_reg[1].data & 0x03;
    SENSOR_DMSG("\n\n[%s]:mirror:%x\r\n\n\n\n", __FUNCTION__, sen_data);
    switch (sen_data)
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
    OS04D10_params *params = (OS04D10_params *)handle->private_data;

    switch (orit)
    {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[1].data = 0x00;
            params->tMirror_reg[3].data = 0x22;
            params->orien_dirty         = true;
            break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[1].data = 0x01;
            params->tMirror_reg[3].data = 0x32;
            params->orien_dirty         = true;
            break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[1].data = 0x02;
            params->tMirror_reg[3].data = 0x32;
            params->orien_dirty         = true;
            break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[1].data = 0x03;
            params->tMirror_reg[3].data = 0x32;
            params->orien_dirty         = true;
            break;
    }

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    OS04D10_params *params  = (OS04D10_params *)handle->private_data;
    u32             max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32             tVts    = (params->tVts_reg[2].data << 8) | (params->tVts_reg[3].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (params->Init_Vts * max_fps * 1000) / tVts;
    else
        params->expo.preview_fps = (params->Init_Vts * max_fps) / tVts;

    // SENSOR_EMSG(">>>>>>>>>>>>>>>>[%s] tVts[%d] preview_fps[%d]\n", __FUNCTION__,tVts,params->expo.preview_fps);
    return params->expo.preview_fps;
}

static int OS04D10_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32             vts     = 0;
    OS04D10_params *params  = (OS04D10_params *)handle->private_data;
    u32             max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32             min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    if (fps >= min_fps && fps <= max_fps)
    {
        params->expo.fps = fps;
        params->expo.vts = (params->Init_Vts * max_fps) / fps;
    }
    else if ((fps >= (min_fps * 1000)) && (fps <= (max_fps * 1000)))
    {
        params->expo.fps = fps;
        params->expo.vts = (params->Init_Vts * (max_fps * 1000)) / fps;
    }
    else
    {
        SENSOR_DMSG("[%s] FPS %d out of range.\n", __FUNCTION__, fps);
        return FAIL;
    }

    if ((params->expo.lines) > (params->expo.vts - 9)) ///
        vts = params->expo.lines + 9;                  ///
    else
        vts = params->expo.vts;
    params->tVts_reg[2].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[3].data = (vts >> 0) & 0x00ff;
    if (params->PreSetFPS != fps)
    {
        params->PreSetFPS    = fps;
        params->reg_dirty = true;
    }
    SENSOR_DMSG(">>>>>>>>>>>>>>[%s] tVts[%d] fps[%d] expo.fps[%d], maxfps %d init vts %d\n", __FUNCTION__, vts, fps,
                params->expo.fps, max_fps, params->Init_Vts);
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
// AE status notification
// static long long int framecount = 3;
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    OS04D10_params *params = (OS04D10_params *)handle->private_data;
    // ISensorIfAPI *sensor_if = handle->sensor_if_api;
    if (params->stUserDefInfo.bSnrSleep == true) // isp resume set ae i2c err, wait sensor resume power on do aenotify
    {
        SENSOR_EMSG("[%s]:%d sensor sleep enable, no I2C write \n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }

    if (params->bFastAE_en == true)
    {
        return SUCCESS;
    }

    if(handle->bEarlyinitEnable == 1 && handle->bEarlyinitStreamOnoff ==  0)
    {
        SENSOR_EMSG("[%s]:%d sensor streamoff, no I2C write \n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if (params->orien_dirty)
            {
                // handle->sensor_if_api->SetSkipFrame(handle->snr_pad_group, params->expo.fps, 3);
                SensorRegArrayW((I2C_ARRAY *)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orien_dirty = false;
            }
            break;
        case CUS_FRAME_ACTIVE:
            if (params->reg_dirty)
            {
                bool dcg_change = false;
                if (linear_last_dcg_data != params->tLcg_Hcg_reg[1].data)
                {
                    dcg_change = true;
                    // printk("hcg_change = true!!\n");
                }

                SensorReg_Write(0xfd, 0x01); // page 1
                SensorReg_Write(0x01, 0x00); // frame sync disable
                SensorRegArrayW((I2C_ARRAY *)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY *)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorRegArrayW((I2C_ARRAY *)params->tGain_reg, ARRAY_SIZE(gain_reg));

                // if(framecount == 2)
                //{
                if (dcg_change == true)
                {
                    // framecount = 0;
                    // if(framecount == 1){
                    // printk("set dcg reg,%x",params->tLcg_Hcg_reg[1].data);
                    SensorRegArrayW((I2C_ARRAY *)params->tLcg_Hcg_reg, ARRAY_SIZE(lcg_hcg_reg));

                    //}
                    // framecount++;
                }
                linear_last_dcg_data = params->tLcg_Hcg_reg[1].data;
                //}
                SensorReg_Write(0xfd, 0x01); // page 1
                SensorReg_Write(0x01, 0x01); // frame sync enable
#if 0 //for debug

                {
                    u16 u16data[10] = {0}, u16exp = 0, u16shutter = 0;
                    SensorReg_Write(0xfd, 0x01);
                    SensorReg_Read(0x03, &u16data[0]);
                    SensorReg_Read(0x04, &u16data[1]);
                    SensorReg_Read(0x24, &u16data[2]);
                    SensorReg_Read(0x45, &u16data[3]);
                    SensorReg_Write(0xfd, 0x05);
                    SensorReg_Read(0x37, &u16data[4]);
                    SensorReg_Read(0x39, &u16data[5]);
                    u16exp     = (u16data[0] & 0xff) << 8 | (u16data[1] & 0xff);
                    u16shutter = (u16exp * params->Line_Period) / 1000;
                    SENSOR_DMSG(">>>>>>>>>>notify write exp [0x%x,0x%x], agin 0x%x dgain [%x,%x] hcg_lcg 0x%x\n ",
                                params->tExpo_reg[1].data, params->tExpo_reg[2].data, params->tGain_reg[1].data,
                                params->tGain_reg[3].data, params->tGain_reg[4].data, params->tLcg_Hcg_reg[1].data);
                    SENSOR_DMSG(">>>>>>>>>>notify exp 0x%x, shtter %d, agin 0x%x dgain [%x,%x] hcg_lcg 0x%x\n", u16exp,
                                u16shutter, u16data[2], u16data[4], u16data[5], u16data[3]);
                }
#endif
                params->reg_dirty = false;
            }
            break;
        default:
            break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    u32             lines    = 0;
    u16             u16value = 0;
    u8              i;
    OS04D10_params *params = (OS04D10_params *)handle->private_data;

    if (params->bFastAE_en)
    {
        // Read AEC shutter;
        for (i = 0; i < ARRAY_SIZE(expo_reg); i++)
        {
            if (expo_reg[i].reg == 0xFD)
            {
                SensorReg_Write(expo_reg[i].reg, expo_reg[i].data);
            }
            else
            {
                SensorReg_Read(expo_reg[i].reg, &u16value);
                params->tExpo_reg[i].data = u16value;
                // SENSOR_DMSG("[DBG] Shutter 0x%x get 0x%x\n", expo_reg[i].reg, u16value);
            }
        }

        // Read AEC shutter;
        for (i = 0; i < ARRAY_SIZE(aec_expo_reg); i++)
        {
            if (aec_expo_reg[i].reg == 0xFD)
            {
                SensorReg_Write(aec_expo_reg[i].reg, aec_expo_reg[i].data);
            }
            else
            {
                SensorReg_Read(aec_expo_reg[i].reg, &u16value);
                // params->tExpo_reg[i].data = u16value;
                // SENSOR_DMSG("[DBG] Shutter 0x%x get 0x%x\n", aec_expo_reg[i].reg, u16value);
            }
        }
    }
    lines = (u32)(params->tExpo_reg[2].data & 0xff);
    lines |= (u32)(params->tExpo_reg[1].data & 0xff) << 8;
    *us = (lines * (u32)params->Line_Period) / 1000;
    SENSOR_DMSG("[%s]--------%d,%x us:[%d]\n", __FUNCTION__, linear_last_dcg_data, params->tLcg_Hcg_reg[1].data, *us);
    return SUCCESS;
}

static int OS04D10_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32             lines = 0, vts = 0;
    OS04D10_params *params = (OS04D10_params *)handle->private_data;
    lines                  = (u32)((1000 * us + (params->Line_Period >> 1)) / params->Line_Period);



    if (lines < 1)
        lines = 1;                    ///
    if (lines > params->expo.vts - 9) ///
        vts = lines + 9;              ///
    else
        vts = params->expo.vts;

    SENSOR_DMSG(">>>>>>>>>>>>>>>[%s] us %d, line %d, line_period %d EXPOvts %d vts %d\n", __FUNCTION__, us, lines,
                params->Line_Period, params->expo.vts, vts);

    // lines <<= 4;
    params->tExpo_reg[1].data = (u16)((lines >> 8) & 0x00ff);
    params->tExpo_reg[2].data = (u16)((lines >> 0) & 0x00ff);

    params->tVts_reg[2].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[3].data = (vts >> 0) & 0x00ff;

    if (params->PreSetShutter != us)
    {
        params->PreSetShutter    = us;
        params->reg_dirty = true;
    }

    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32 *gain)
{
    int             again = 0, dgain = 0, lcg = 0;
    OS04D10_params *params   = (OS04D10_params *)handle->private_data;
    u16             u16value = 0;

    u8 i;

    if (params->bFastAE_en)
    {
        // Read AEC again;
        for (i = 0; i < ARRAY_SIZE(gain_reg); i++)
        {
            if (gain_reg[i].reg == 0xFD)
            {
                SensorReg_Write(gain_reg[i].reg, gain_reg[i].data);
            }
            else
            {
                SensorReg_Read(gain_reg[i].reg, &u16value);
                params->tGain_reg[i].data = u16value;
                //SENSOR_DMSG("[DBG] get tGain_reg 0x%x, 0x%x \n", params->tGain_reg[i].reg, params->tGain_reg[i].data);
            }
        }
        // Read AEC hcg;
        for (i = 0; i < ARRAY_SIZE(lcg_hcg_reg); i++)
        {
            if (lcg_hcg_reg[i].reg == 0xFD)
            {
                SensorReg_Write(lcg_hcg_reg[i].reg, lcg_hcg_reg[i].data);
            }
            else
            {
                SensorReg_Read(lcg_hcg_reg[i].reg, &u16value);
                params->tLcg_Hcg_reg[i].data = u16value;
                //SENSOR_DMSG("[DBG] get tLcg_Hcg_reg 0x%x, 0x%x \n", params->tLcg_Hcg_reg[i].reg, params->tLcg_Hcg_reg[i].data);
            }
        }
    }

    again = params->tGain_reg[1].data;
    dgain = (params->tGain_reg[4].data << 8) | params->tGain_reg[3].data;
    lcg   = params->tLcg_Hcg_reg[1].data == 0x02 ? 0 : 1;
    if (lcg == 1)
    {
        *gain = again * dgain * 1024 / (16 * 64);
    }
    else
    {
        *gain = again * dgain * params->otp_data / (16 * 64);
    }
    SENSOR_DMSG(">>>>get gain %d, again 0x%x, dgain 0x%x lcg %d\n", *gain, again, dgain, lcg);
    return SUCCESS;
}

static int OS04D10_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    OS04D10_params *params    = (OS04D10_params *)handle->private_data;
    u32             tmp_dgain = 0;
    u32             again     = 0;
    short           lcg       = 1;
    params->otp_data          = params->otp_data == 0 ? 3120 : params->otp_data;
    if (gain < 1024)
    {
        gain = 1024;
    }
    else if (params->otp_data > 0 && gain >= SENSOR_MAX_GAIN)
    {
        gain = SENSOR_MAX_GAIN - 1; // again max 15.5x,Without using digital gain
    }

    if (gain >= MAX_A_GAIN)
    {
        again     = MAX_A_GAIN;
        tmp_dgain = gain * 64 / again;
    }
    else
    {
        again     = gain;
        tmp_dgain = 0x40;
    }

    if (params->otp_data <= 0 || gain < (params->otp_data))
    {
        again = (again * 16 + 512) / 1024;
        lcg   = 1;
    }
    else
    {
        again = (again * 16 + (params->otp_data / 2)) / params->otp_data;
        lcg   = 0;
    }

    if (again >= 0xf8)
        again = 0xf8;
    params->tGain_reg[1].data = (u16)(again)&0xff; // low byte

    params->tGain_reg[4].data = (u16)(tmp_dgain >> 8 & 0x07);
    params->tGain_reg[3].data = (u16)(tmp_dgain & 0xFF);
    SENSOR_DMSG("set gain %d, again 0x%x, dgain 0x%x lcg %d\n", gain, again, tmp_dgain, lcg);
    SENSOR_DMSG("[%s] default tGain_reg.data 1:[%d], 3:[%d], 4:[%d]] !, gain %d again %d\n ", __FUNCTION__,
                params->tGain_reg[1].data, params->tGain_reg[3].data, params->tGain_reg[4].data, gain, again);
    params->tLcg_Hcg_reg[1].data = lcg ? 0x00 : 0x02;
    if (params->PreSetGain != gain)
    {
        params->PreSetGain    = gain;
        params->reg_dirty = true;
    }

    /* printk("[%s] set  gain/inputgain/AregH/DregH/DregL=%d/%d//%x/%x/%x;lcg=%d,reg=%x\n", __FUNCTION__,gain,
     * input_gain,params->tGain_reg[1].data,params->tGain_reg[3].data,params->tGain_reg[4].data,lcg,params->tLcg_Hcg_reg[1].data);
     */
    return SUCCESS;
}

static int pCus_post_init(ss_cus_sensor *handle, u32 idx)
{
    u32 gain    = 0;
    // u32 shutter = 0;
    // OS04D10_params *params = (OS04D10_params *)handle->private_data;

    if(handle->bEarlyinitEnable ==1 && handle->bEarlyinitStreamOnoff == 0)
    {
        return 0;
    }

    OS04D10_GetOtpData(handle);
#if 0 //for debug

    SENSOR_DMSG(">>>>>>>>>>>>>>[%s]:%d \n", __FUNCTION__, __LINE__);
    if (handle->video_res_supported.ulcur_res == 2 || handle->video_res_supported.ulcur_res == 3)
    {
        u16 u16data[4] = {0}, u16exp = 0, u16shutter = 0;
        SensorReg_Write(0xfd, 0x01);
        SensorReg_Read(0x03, &u16data[0]);
        SensorReg_Read(0x04, &u16data[1]);
        SensorReg_Read(0x24, &u16data[2]);
        SensorReg_Read(0x45, &u16data[3]);
        u16exp     = (u16data[0] & 0xff) << 8 | (u16data[1] & 0xff);
        u16shutter = (u16exp * params->Line_Period) / 1000;
        SENSOR_DMSG(">>>>>>>>>>exp 0x%x, shtter %d, agin 0x%x hcg_lcg 0x%x\n", u16exp, u16shutter, u16data[2], u16data[3]);

        SensorReg_Write(0xfd, 0x05);
        SensorReg_Read(0x37, &u16data[0]);
        SensorReg_Read(0x39, &u16data[1]);
        SENSOR_DMSG(">>>>>>>>>> dgain %d  0x%x\n", u16data[0], u16data[1]);
    }
#endif
    /*
    //for debug
    {
        int i=0;
        for(i=0;i< params->Init_Array_Size;i++)
        {
            if(params->pTable_linear[i].reg == 0xff)
            {
                CamOsPrintf("sleep: %d ms", params->pTable_linear[i].data);
            }
            else if (params->pTable_linear[i].reg == 0xfd)
            {
                SensorReg_Write(params->pTable_linear[i].reg,params->pTable_linear[i].data);
                CamOsPrintf("reg 0x%x -> 0x%x", params->pTable_linear[i].reg, params->pTable_linear[i].data);
            }
            else
            {
                SensorReg_Read(params->pTable_linear[i].reg, &u16_data);
                CamOsPrintf("reg 0x%x -> 0x%x", params->pTable_linear[i].reg, u16_data);
            }
        }
    }
*/
    /*  SensorReg_Write(0xfd, 0x00);
    SensorReg_Read(0x02, &a);
    SensorReg_Read(0x03, &b);
    SensorReg_Read(0x04, &c);
    SensorReg_Read(0x05, &d);

    printk("otp_data=%d;linear_last_dcg_data=%d;%x/%x/%x/%x\n", params->otp_data, linear_last_dcg_data,a,b,c,d); */

    if (handle->video_res_supported.ulcur_res != 2 && handle->video_res_supported.ulcur_res != 3)
    {
        pCus_GetAEGain(handle, &gain);
        if (gain > 0)
        {
            OS04D10_SetAEGain(handle, gain);
        }

        OS04D10_SetAEUSecs(handle, 25000);

        pCus_AEStatusNotify(handle, 0, CUS_FRAME_ACTIVE);
    }
    return SUCCESS;
}

typedef enum
{
    CMDID_FAE_SWITCH_SENSOR_AE     = 0,
    CMDID_FAE_GET_SENSOR_AE_STATE  = 1,
    CMDID_FAE_GET_ABORT_SENSOR_AE  = 2,
    CMDID_FAE_GET_SENSOR_SLEEPMODE = 3
} CMDID_e;

typedef struct
{
    u32 SensorAEswitch;
    u32 SensorAEled;
    u32 SensorAEfpsmax;
    u32 SensorAEdgain;
    u32 SensorAEagain;
    u32 SensorAEshutter;
    u32 SensorAEtarget;
} SensorAeSwitch_t;

typedef struct
{
    u32 state;   // 0 : AE working, 1 : AE done,
    u32 shutter; // us
    u32 again;   // 1X : 1024 (include hcg)
    u32 dgain;   // 1X : 1024
    u32 ymean;   // 0~255 : AE ROI average Y value
} SensorCusAeState_t;

typedef struct
{
    bool abort; // 0 : AE working, 1 : AE abort,
} SensorCusAeAbort_t;

typedef struct
{
    bool sleepmode; // 0 : sensor working, 1 : sensor sleep,
} SensorCusSleep_t;

static int pCus_sensor_CustDefineFunction(ss_cus_sensor *handle, u32 cmd_id, void *param)
{
    OS04D10_params *params = (OS04D10_params *)handle->private_data;
    u32             idx    = handle->i2c_bus->nSensorID;
    // u16 u16I2Cvalue;
    // u32 lines;
    // u32 u32cnt = 0;

    if (param == NULL || handle == NULL)
    {
        SENSOR_EMSG("param/handle data NULL \n");
        return FAIL;
    }

    switch (cmd_id)
    {
        case CMDID_FAE_SWITCH_SENSOR_AE:
        {
            SensorAeSwitch_t *pstParam  = (SensorAeSwitch_t *)param;
            ISensorIfAPI *    sensor_if = handle->sensor_if_api;
            // u16 againT;//, againH, againL;
            u16 dgainT;
            u16 lcg;

            if (pstParam->SensorAEswitch) // switch to little picture 640x360
            {
                SENSOR_EMSG("[%s] Select to little picture 640x360\n", __FUNCTION__);
                params->bFastAE_en = true;

                sensor_if->MCLK(idx, 1, handle->mclk);

                CamOsMsDelay(2); //xshutdown keep high, mclk on-> 2ms -> i2c
                // select little picture 640x360
                pCus_SetVideoRes(handle, LINEAR_RES_3);

                // Write again;
                // if (pstParam->SensorAEagain > 15872)
                //    pstParam->SensorAEagain = 15872;
                // againH = pstParam->SensorAEagain>>10;
                // againL = ((pstParam->SensorAEagain - againH*1024) * 16 >> 10);
                // againT = (againH & 0xF)<<4 | againL;
                // againT = (pstParam->SensorAEagain >> 10) << 4;
                // params->tGain_reg[2].data = againT & 0xFF;
                // SENSOR_EMSG("[DBG] Set again 0x%x (%d)\n", againT, pstParam->SensorAEagain);
                if (pstParam->SensorAEagain >= SENSOR_MIN_GAIN * 3)
                {
                    lcg = 0;
                }
                else
                {
                    lcg = 1;
                }
                params->tLcg_Hcg_reg[1].data = lcg ? 0x00 : 0x02;
                SENSOR_DMSG("[DBG] again (%d), lcg_hcg:0x%x\n", pstParam->SensorAEagain,params->tLcg_Hcg_reg[1].data);

                // Write dgain;
                dgainT = (pstParam->SensorAEdgain >> 10) << 6;

                if (dgainT > 0x7ff)
                {
                    dgainT = 0x7ff;
                }
                params->tGain_reg[4].data = (u16)(dgainT >> 8 & 0x07);
                params->tGain_reg[3].data = (u16)(dgainT & 0xFF);
                SENSOR_DMSG("[DBG] Set dgain 0x%x (%d), %x, %x\n", dgainT, pstParam->SensorAEdgain,
                            params->tGain_reg[4].data, params->tGain_reg[3].data);

                // Write AEC target;
                params->tAETarget_reg[1].data = pstParam->SensorAEtarget;
                SENSOR_DMSG("[DBG] Set AeTarget 0x%x, 0x%x\n", params->tAETarget_reg[1].data, pstParam->SensorAEtarget);

                // lines=(u32)((1000*pstParam->SensorAEshutter+(params->Line_Period>>1))/params->Line_Period);
                // if (lines < 1)
                //     lines = 1;

                // params->tExpo_reg[1].data =(u16)( (lines>>8) & 0x00ff);
                // params->tExpo_reg[2].data =(u16)( (lines>>0) & 0x00ff);
                // SENSOR_EMSG("[DBG] Set shutter 0x%x (%d)\n", lines, pstParam->SensorAEshutter);

                // params->reg_dirty = true;

                // sensor init
                pCus_init(handle);

                // pCus_FastAe(handle);
            }
            else
            {
                SENSOR_EMSG("[DBG] Select to normal picture 2560xx1440\n");
                if (params->bFastAE_en == false)
                {
                    SENSOR_EMSG("[ERROR] Please check config litte picture first!!\n");
                    return FAIL;
                }

                // select picture 2560xx1440
                pCus_SetVideoRes(handle, LINEAR_RES_4);

                // Write fps;
                //SENSOR_EMSG("[DBG] Set fps=%d\n", pstParam->SensorAEfpsmax);
                OS04D10_SetFPS(handle, pstParam->SensorAEfpsmax);

                // Write gain
                //SENSOR_EMSG("[DBG] Set gain %d\n", pstParam->SensorAEagain);
                OS04D10_SetAEGain(handle,  pstParam->SensorAEagain);

                // Write dgain;
                //SENSOR_EMSG("[DBG] Set dgain %d\n", pstParam->SensorAEdgain);

                // Write shutter
                //SENSOR_EMSG("[DBG] Set shutter %d, \n", pstParam->SensorAEshutter);
                OS04D10_SetAEUSecs(handle, pstParam->SensorAEshutter);
                sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
                sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
                sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
                sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);

                // sensor init
                pCus_init(handle);

                params->bFastAE_en = false;
            }
            break;
        }
        case CMDID_FAE_GET_SENSOR_AE_STATE:
        {
            SensorCusAeState_t *pstParam = (SensorCusAeState_t *)param;
            u16                 u16state = 0;
            u16                 u16value = 0;
            u32                 u32gain;
            u32                 u32shutter;

            pstParam->ymean   = 0;
            pstParam->again   = 0;
            pstParam->dgain   = 0;
            pstParam->shutter = 0;
            if(params->bFastAE_en == false)
            {
                SENSOR_EMSG("[DBG] sensor fastae not working, bFastAE_en %d\n", params->bFastAE_en);
                pstParam->state = 2;
                return FAIL;
            }
            SensorReg_Write(0xFD, 0x1);      // switch patch
            SensorReg_Read(0x20, &u16state); // Read AEC Done
            pstParam->state = u16state;

            if (u16state)
            {
                SENSOR_EMSG("[DBG] Sensor AEC done\n");
                // Get AEC statistics
                SensorReg_Read(0x21, &u16value);
                pstParam->ymean = u16value;
                SENSOR_DMSG("[DBG] Get ymean 0x%x\n", pstParam->ymean);

                // Get gain
                pCus_GetAEGain(handle, &u32gain);
                pstParam->again = u32gain;
                SENSOR_DMSG("[DBG] Get again 0x%x\n", pstParam->again);

                // Get shutter
                pCus_GetAEUSecs(handle, &u32shutter);
                pstParam->shutter = u32shutter;
                SENSOR_DMSG("[DBG] Get shutter time %d\n", pstParam->shutter);
            }
            else
            {
                // SENSOR_EMSG("[DBG] Wait Sensor AEC done\n");
                pstParam->ymean   = 0;
                pstParam->again   = 0;
                pstParam->dgain   = 0;
                pstParam->shutter = 0;
            }
            break;
        }
        case CMDID_FAE_GET_ABORT_SENSOR_AE:
        {
            SensorCusAeAbort_t *pstParam = (SensorCusAeAbort_t *)param;
            ISensorIfAPI *     sensor_if = handle->sensor_if_api;

            SENSOR_EMSG("[%s] Sensor AEC abort, bFastAE_en %d, g_u8SuspendFlag %d\n", __FUNCTION__,
                    params->bFastAE_en, g_u8SuspendFlag);

            if (params->bFastAE_en == true && pstParam->abort == true && g_u8SuspendFlag == 0)
            {
                // select picture 2560xx1440
                pCus_SetVideoRes(handle, LINEAR_RES_4);

                sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
                sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
                sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
                sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);

                // sensor init
                pCus_init(handle);
                params->bFastAE_en = false;
            }
            break;
        }
        case CMDID_FAE_GET_SENSOR_SLEEPMODE:
        {
            SensorCusSleep_t *pstParam = (SensorCusSleep_t *)param;

            pstParam->sleepmode = g_u8SuspendFlag;
            break;
        }
        default:
            SENSOR_EMSG("cmd id %d err \n", cmd_id);
            break;
    }

    return SUCCESS;
}

static int pCus_StreamOn(struct __ss_cus_sensor *handle, u32 idx)
{
    if(handle->bEarlyinitStreamOnoff == 0)
    {
        OS04D10_params *params = (OS04D10_params *)handle->private_data;
        SensorReg_Write(0xfd,0x00);
        SensorReg_Write(0x36,0x00);
        CamOsMsDelay(1);
        SensorReg_Write(0xfd,0x01);//page 1
        SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
        SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
        SensorRegArrayW((I2C_ARRAY*)params->tLcg_Hcg_reg, ARRAY_SIZE(lcg_hcg_reg));
        SensorReg_Write(0x33,0x03);//page 1
        SensorReg_Write(0x01,0x02);//page 1
        SensorReg_Write(0xfd,0x00);
        SensorReg_Write(0x20,0x03);
        SensorReg_Write(0xfd,0x01);
        SENSOR_EMSG("[%s] sensor stream on!\n", __FUNCTION__);

        handle->bEarlyinitStreamOnoff = 1;
        pCus_post_init(handle, idx);
    }
    return SUCCESS;
}

static int pCus_Suspend(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_ENABLE, idx);
    sensor_if->MCLK(idx, 0, handle->mclk);
    g_u8SuspendFlag = 1;

    SENSOR_EMSG("[%s] sensor suspend!\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_Resume(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    OS04D10_params *params = (OS04D10_params *)handle->private_data;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);
    sensor_if->MCLK(idx, 1, handle->mclk);

    CamOsMsDelay(2); //xshutdown keep high, mclk on-> 2ms -> i2c
    if(params->bFastAE_en == true)
    {
        SENSOR_EMSG("[%s] bFastAE_en=%d, select to picture 2560x1440\n", __FUNCTION__, params->bFastAE_en);
        //select picture 2560x1440
        pCus_SetVideoRes(handle, LINEAR_RES_4);

        // sensor init
        pCus_init(handle);
        params->bFastAE_en = false;
    }
    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_DISABLE, idx);

    g_u8SuspendFlag = 0;

    SENSOR_EMSG("[%s] sensor resume!\n", __FUNCTION__);
    return SUCCESS;
}
int cus_camsensor_init_handle(ss_cus_sensor *drv_handle)
{
    ss_cus_sensor * handle = drv_handle;
    int             res    = 0;
    OS04D10_params *params = (OS04D10_params *)handle->private_data;
    if (!handle)
    {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }

    // private data allocation & init
    if (handle->private_data == NULL)
    {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tLcg_Hcg_reg, lcg_hcg_reg, sizeof(lcg_hcg_reg));
    memcpy(params->tAETarget_reg, ae_target_reg, sizeof(ae_target_reg));
    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "OS04D10_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////

    handle->sif_bus                                               = SENSOR_IFBUS_TYPE; // CUS_SENIF_BUS_PARL;
    handle->data_prec                                             = SENSOR_DATAPREC;   // CUS_DATAPRECISION_8;
    handle->bayer_id                                              = SENSOR_BAYERID;    // CUS_BAYER_GB;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;
    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; // don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; // Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    // i2c
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < LINEAR_RES_END; res++)
    {
        handle->video_res_supported.num_res                  = res + 1;
        handle->video_res_supported.res[res].u16width        = os04d10_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height       = os04d10_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps      = os04d10_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps      = os04d10_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x = os04d10_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y = os04d10_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = os04d10_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = os04d10_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, os04d10_mipi_linear[res].senstr.strResDesc);
    }

    handle->i2c_cfg.mode    = SENSOR_I2C_LEGACY; //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt     = SENSOR_I2C_FMT;    // CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address = SENSOR_I2C_ADDR;   // 0x5a;
    handle->i2c_cfg.speed   = SENSOR_I2C_SPEED;  // 320000;

    // mclk
    handle->mclk            = Preview_MCLK_SPEED;
    params->Line_Period     = Preview_line_period_FPS_30;
    params->pTable_linear   = Sensor_init_table_fps_30;
    params->Init_Array_Size = ARRAY_SIZE(Sensor_init_table_fps_30);
    // polarity
    /////////////////////////////////////////////////////
    params->pwdn_POLARITY  = SENSOR_PWDN_POL; // CUS_CLK_POL_NEG;
    params->reset_POLARITY = SENSOR_RST_POL;  // CUS_CLK_POL_NEG;
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min      = SENSOR_MIN_GAIN;
    // handle->sensor_ae_info_cfg.u32AEGain_max                      = (32768*15750 / 1000000)*1024;
    handle->sensor_ae_info_cfg.u32AEGain_max      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min   = (params->Line_Period * 1);
    handle->sensor_ae_info_cfg.u32AEShutter_max   = 1000000000 / handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step  = params->Line_Period;

    handle->pCus_sensor_release   = cus_camsensor_release_handle;
    handle->pCus_sensor_init      = pCus_init;
    handle->pCus_sensor_post_init = pCus_post_init;
    handle->pCus_sensor_poweron   = pCus_poweron;
    handle->pCus_sensor_poweroff  = pCus_poweroff;
    handle->pCus_sensor_reopen    = pCus_Reopen;
    handle->pCus_sensor_streamon  = pCus_StreamOn;
    handle->pCus_sensor_suspend   = pCus_Suspend;
    handle->pCus_sensor_resume    = pCus_Resume;
    // Normal
    handle->pCus_sensor_GetSensorID        = pCus_GetSensorID;
    handle->pCus_sensor_GetVideoResNum     = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes        = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes     = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes        = pCus_SetVideoRes;
    handle->pCus_sensor_GetOrien           = pCus_GetOrien;
    handle->pCus_sensor_SetOrien           = pCus_SetOrien;
    handle->pCus_sensor_GetFPS             = pCus_GetFPS;
    handle->pCus_sensor_SetFPS             = OS04D10_SetFPS;
    handle->pCus_sensor_SetPatternMode     = OS04D10_SetPatternMode;

    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify     = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs         = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs         = OS04D10_SetAEUSecs;
    handle->pCus_sensor_GetAEGain          = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain          = OS04D10_SetAEGain;
    handle->pCus_sensor_GetAEInfo          = OS04D10_GetAEInfo;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;
    params->expo.vts                       = vts_30fps;
    params->expo.fps                       = 15;
    params->expo.lines                     = 1000;
    params->reg_dirty                      = false;
    params->orien_dirty                    = false;

    return SUCCESS;
}

int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  OS04D10,
                            cus_camsensor_init_handle,
                            NULL,
                            NULL,
                            OS04D10_params
                         );
