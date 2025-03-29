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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OS02H10);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
#define SENSOR_MIPI_LANE_NUM_HDR (2)
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
//                                                                                                    �@//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

//#define SENSOR_ISP_TYPE     ISP_EXT                 //ISP_EXT, ISP_SOC
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID      CUS_BAYER_GB            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR  CUS_BAYER_RG//CUS_BAYER_GB
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F1           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_ORIT_HDR     CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define SENSOR_MAX_GAIN     507903//(15.5x*32x)        // max sensor again, a-gain * conversion-gain*d-gain
#define MAX_A_GAIN          15872 //(15.5x)
#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ       //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period 14815                   // 1s/(vts*fps) ns
#define Preview_line_period_HDR 29630
#define vts_30fps           2250                    // VTS for 30fps
#define vts_30fps_HDR  1125
#define vBlankInit				1125
#define vBlankInit_HDR				0
#define Preview_WIDTH       1928                    //resolution Width when preview
#define Preview_HEIGHT      1088                    //resolution Height when preview
#define Preview_MAX_FPS     30                      //fastest preview FPS
#define Preview_MIN_FPS     3                       //slowest preview FPS
#define Preview_MAX_FPS_HDR 30  
#define SENSOR_I2C_ADDR     0x78                    //I2C slave address
#define SENSOR_I2C_SPEED    200000 //300000// 240000                  //I2C speed, 60000~320000

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
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int g_sensor_ae_min_gain = 1024;

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1=0, /*LINEAR_RES_2 = 1,*/ LINEAR_RES_END}mode;
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
}os02h10_mipi_linear[] = {
    //{LINEAR_RES_1, {1928, 1088, 3, 30}, {0, 0, 1928, 1088}, {"1928x1088@30fps"}},
    {LINEAR_RES_1, {1920, 1080, 3, 60}, {0, 0, 1920, 1080}, {"1920x1080@60fps"}},
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
        u32 usVts;
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
    CUS_CAMSENSOR_ORIT  orit;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tVBlank_reg[2];
    I2C_ARRAY tGain_reg[6];
    I2C_ARRAY tGain_reg_HDR_SEF[5];
    I2C_ARRAY tExpo_reg[2];
	I2C_ARRAY tExpo_reg_HDR_SEF[2];
    I2C_ARRAY tMirror_reg[6];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool reg_dirty;
    bool orien_dirty;
	int otp_data;

} OS02H10_params;
// set sensor ID address and data

const I2C_ARRAY Sensor_id_table[] =
{
    {0xfd, 0x00},
    {0x02, 0x53},      // {address of ID, ID },
    {0x03, 0x02},      // {address of ID, ID },
    {0x04, 0x48},      // {address of ID, ID },
    {0x05, 0x10},      // {address of ID, ID },
};

const I2C_ARRAY Sensor_2m30_init_table[] =     
{
   //OS02H10_1928x1088_mipi_2lane_raw10_30fps_linear_v3 - 24Mclk  HTS=1082 VTS=2250
    {0xfd,0x00},
	{0x20,0x00},
	{0x53,0xfe},
	{0x54,0x7f},
	{0x61,0x8c},
	{0x63,0x05},
	{0x64,0x00},
	{0x65,0x00},
	{0x66,0x02},
	{0x67,0x00},
	{0x8d,0x00},
	{0x8e,0x07},
	{0x8f,0x88},
	{0x90,0x04},
	{0x91,0x40},
	{0x95,0x88},
	{0x98,0x88},
	{0xb6,0x40},
	{0x23,0x00},
	{0x24,0x05},
	{0x26,0x4d},
	{0x29,0x00},
	{0x2a,0x03},
	{0x31,0x90},
	{0x32,0x04},
	{0x33,0x00},
	{0x38,0x00},
	{0x55,0x00},
	{0xf5,0x01},
	{0xfd,0x01},
	{0x03,0x00},
	{0x04,0x04},
	{0x05,0x04},
	{0x06,0x65},
	{0x09,0x01},
	{0x0a,0x00},
	{0x24,0xff},
	{0x3e,0x3c},
	{0x3f,0x01},
	{0x01,0x01},
	{0x11,0x40},
	{0x14,0x65},
	{0x15,0x00},
	{0x16,0x95},
	{0x18,0xf3},
	{0x19,0x47},
	{0x1a,0x03},
	{0x1b,0x50},
	{0x1c,0xf0},
	{0x1d,0x3d},
	{0x1e,0x00},
	{0x1f,0x27},
	{0x21,0x74},
	{0x22,0x90},
	{0x25,0x00},
	{0x26,0x77},
	{0x27,0xf9},
	{0x2a,0x4e},
	{0x2e,0x28},
	{0x34,0xf6},
	{0x35,0x22},
	{0x36,0xa2},
	{0x50,0x00},
	{0x51,0x08},
	{0x52,0x07},
	{0x53,0x00},
	{0x55,0x35},
	{0x56,0x01},
	{0x57,0x0a},
	{0x59,0x01},
	{0x5b,0x13},
	{0x5d,0x08},
	{0x5e,0x00},
	{0x5f,0x1d},
	{0x60,0x1e},
	{0x68,0x05},
	{0x69,0x04},
	{0x6a,0x00},
	{0x70,0x64},
	{0x71,0x15},
	{0x72,0x42},
	{0x7b,0x40},
	{0x80,0x08},
	{0x83,0x08},
	{0x85,0x14},
	{0x86,0x2d},
	{0x8a,0xf9},
	{0x95,0x25},
	{0x97,0x10},
	{0x99,0x30},
	{0x9f,0x10},
	{0xa8,0x10},
	{0xad,0x00},
	{0xb0,0x63},
	{0xb1,0x63},
	{0xb2,0x63},
	{0xb3,0x63},
	{0xb4,0x61},
	{0xb5,0x61},
	{0xb6,0x61},
	{0xb7,0x61},
	{0xb8,0x77},
	{0xb9,0x66},
	{0xba,0x07},
	{0xbb,0x07},
	{0xbc,0x07},
	{0xbd,0x07},
	{0x29,0x0a},
	{0x2b,0x03},
	{0x30,0x04},
	{0x92,0x05},
	{0xc4,0x00},
	{0xc5,0x00},
	{0xc6,0x13},
	{0xc7,0x6d},
	{0xc9,0x00},
	{0xd0,0x77},
	{0xd1,0x66},
	{0xd5,0x12},
	{0xd7,0x40},
	{0xdc,0x01},
	{0xdd,0x01},
	{0xde,0x04},
	{0xfd,0x02},
	{0x68,0x02},
	{0x6d,0x5d},
	{0x6e,0xdc},
	{0x78,0x70},
	{0x79,0x70},
	{0x7a,0x70},
	{0x7b,0x70},
	{0x80,0x60},
	{0x81,0x60},
	{0x82,0x12},
	{0x83,0x60},
	{0x84,0x60},
	{0x9f,0x12},
	{0xa1,0x00},
	{0xa2,0x04},
	{0xa3,0x40},
	{0xa5,0x00},
	{0xa6,0x07},
	{0xa7,0x88},
	{0x72,0x40},
	{0x73,0x40},
	{0x74,0x40},
	{0x75,0x40},
	{0xfd,0x06},
	{0x00,0x04},
	{0x01,0x7c},
	{0x02,0xbf},
	{0x03,0x86},
	{0x04,0xbd},
	{0x05,0x02},
	{0xfd,0x02},
	{0x52,0xff},
	{0xfd,0x01},
	{0xfd,0x00},
	{0xb1,0x02},
	{0xfd,0x01},
	{0xff,0x0a},
};

const I2C_ARRAY Sensor_2m60_init_table[] =     
{
    //@@ OS02H10_1920x1080_mipi_2lane_raw10_60fps_linear_v3.0

    //100 99 1920 1080

    //102 80 01
    //102 99b 11
    //110 80 10
    //110 40 00

    {0xfd, 0x00},
    {0x20, 0x00},
    //sl 5 5
    {0x53, 0xfe},
    {0x54, 0x7f},
    {0x61, 0x8c},  //mpll_divp_8lsb
    {0x63, 0x05},  //mpll_prediv
    {0x64, 0x00},  //mpll_divout
    {0x65, 0x00},  //mpll_divsys
    {0x66, 0x02},  //mpll_divbit
    {0x67, 0x00},  //mpll_predivp
    {0x8d, 0x00},  //mipi_rst.mipi_pd
    {0x8e, 0x07},  //mipi_hsize_4msb
    {0x8f, 0x80},  //mipi_hsize_8lsb
    {0x90, 0x04},  //mipi_vsize_4msb
    {0x91, 0x38},  //mipi_vsize_8lsb
    {0x95, 0x88},
    {0x98, 0x88},
    {0xb6, 0x40},  //mipi 2lane
    {0x23, 0x00},  //pll_predivp
    {0x24, 0x05},  //pll_prediv
    {0x26, 0x4d},  //pll_divp
    {0x29, 0x00},  //pll_divdac
    {0x2a, 0x03},  //pll_divpump
    {0x31, 0x90},  //clk_mode
    {0x32, 0x04},  //clk_mode2
    {0x33, 0x00},  //ao_mode_en
    {0x38, 0x00},  //lvds_en
    {0x55, 0x00},  //pclk_in_sel
    {0xf5, 0x01},  //osc_cal_en
    {0xfd, 0x01},
    {0x03, 0x00},  //exp1 msb
    {0x04, 0x04},  //exp1 lsb
    {0x09, 0x01},
    {0x0a, 0x00},
    {0x24, 0xff},  //again
    {0x3e, 0x3c},  //scg_en=1,col_gain
    {0x3f, 0x01},  //mirror
    {0x01, 0x01},
    {0x11, 0x40},  //[6]db_shutter
    {0x14, 0x65},
    {0x15, 0x00},
    {0x16, 0x95},
    {0x18, 0xf3},  //d3-->f3 ramp_psrr_act_en=1
    {0x19, 0x47},
    {0x1a, 0x03}, //01
    {0x1b, 0x50}, //70
    {0x1c, 0xf0}, //f2  ;icomp1/icomp2
    {0x1d, 0x3d},
    {0x1e, 0x00},  //col_comp1_cap_sel
    {0x1f, 0x27},  //ipix
    {0x21, 0x74},  //ramp bias sel
    {0x22, 0x90},  //vrhv
    {0x25, 0x00},
    {0x26, 0x77},  //vrnv1/vrnv2
    {0x27, 0xf9},  //vcap
    {0x2a, 0x4e},  //ramp_cur_code(adc_range)
    {0x2e, 0x28},  //2a-->28 pwd_psnc_ramp_reg=0
    {0x34, 0xf6},  //vbl
    {0x35, 0x22},
    {0x36, 0xa2},
    {0x50, 0x00},  //p0
    {0x51, 0x08},  //p1
    {0x52, 0x07},  //P2
    {0x53, 0x00},  //p3
    {0x55, 0x35},
    {0x56, 0x01},
    {0x57, 0x0a},
    {0x59, 0x01},
    {0x5b, 0x13},
    {0x5d, 0x08},
    {0x5e, 0x00},  //37  ;p12,rowsg
    {0x5f, 0x1d},  //p13,rowsel
    {0x60, 0x1e},  //p14,scg
    {0x68, 0x05},  //
    {0x69, 0x04},  //p24
    {0x6a, 0x00},  //
    {0x70, 0x64},  //p31,rst_d0
    {0x71, 0x15},  //p32,rst_d1
    {0x72, 0x42},  //p33,sig_d0
    {0x7b, 0x40},  //p43
    {0x80, 0x08},
    {0x83, 0x08},
    {0x85, 0x14},
    {0x86, 0x2d},
    {0x8a, 0xf9},
    {0x95, 0x25},
    {0x97, 0x10},
    {0x99, 0x30},
    {0x9f, 0x10},  //p90,rowsel
    {0xa8, 0x10},  //p99,rowsg
    {0xad, 0x00},  //p106,restg
    {0xb0, 0x63},  //sc1_1
    {0xb1, 0x63},  //sc1_2
    {0xb2, 0x63},  //sc1_3
    {0xb3, 0x63},  //sc1_4
    {0xb4, 0x61},  //sc2_1
    {0xb5, 0x61},  //sc2_2
    {0xb6, 0x61},  //sc2_3
    {0xb7, 0x61},  //sc2_4
    {0xb8, 0x77},  //vbl_1/2_scg0
    {0xb9, 0x66},  //vbl_3/4_scg0
    {0xba, 0x07},  //vofs_1
    {0xbb, 0x07},  //vofs_2
    {0xbc, 0x07},  //vofs_3
    {0xbd, 0x07},  //vofs_4
    //psrr
    {0x29, 0x0a},  //ramp_psrr_sw_gain
    {0x2b, 0x03},  //ramp_psrr_bias_sel
    {0x30, 0x04},  //ramp_psnc_cap2_ctrl
    {0x92, 0x05},
    {0xc4, 0x00},
    {0xc5, 0x00},
    {0xc6, 0x13},
    {0xc7, 0x6d},
    {0xc9, 0x00},

    {0xd0, 0x77},  //vbl_1/2_scg1
    {0xd1, 0x66},  //vbl_3/4_scg1
    {0xd5, 0x12},  //col_en1_num_8lsb
    {0xd7, 0x40},  //col_en2_num_8lsb
    {0xdc, 0x01},  //ulp pwd en
    {0xdd, 0x01},  //ulp start
    {0xde, 0x04},  //ulp end
    {0xfd, 0x02},
    {0x68, 0x02},  //[1]high/low 8 bit
    {0x6d, 0x5d},  //blc en
    {0x6e, 0xdc},
    {0x78, 0x70},  //blc k
    {0x79, 0x70},
    {0x7a, 0x70},
    {0x7b, 0x70},
    {0x80, 0x60},
    {0x81, 0x60},
    {0x82, 0x12},
    {0x83, 0x60},
    {0x84, 0x60},
    {0x9f, 0x12},
    {0xa1, 0x04},  //dig vstart
    {0xa2, 0x04},  //dig vsize
    {0xa3, 0x38},
    {0xa5, 0x04},  //dig hstart
    {0xa6, 0x07},  //dig hsize
    {0xa7, 0x80},
    {0x72, 0x40},  //offset
    {0x73, 0x40},
    {0x74, 0x40},
    {0x75, 0x40},
    //temperature sensor
    {0xfd, 0x06},
    {0x00, 0x04}, //r_tmp_slope_h[7:0]
    {0x01, 0x7c}, //r_tmp_slope_l[7:0]
    {0x02, 0xbf}, //r_tmp_offset_3[7:0]
    {0x03, 0x86}, //r_tmp_offset_2[7:0]
    {0x04, 0xbd}, //r_tmp_offset_1[7:0]
    {0x05, 0x02}, //r_tmp_offset_0[7:0]
    {0xfd, 0x02},
    {0x52, 0xff},
    {0xfd, 0x01},

    {0xfd, 0x00},
    {0xb1, 0x02},  //[1]mipi_en
    {0xfd, 0x01},

    //110 40 01
};

const I2C_ARRAY Sensor_2m30init_table_HDR[] ={
#if 0	
    //OS02H10_1920X1080_mipi_2lane_raw10_30fps_hdr_v3.0
	 {0xfd,0x00},
	 {0x20,0x00},
	 {0x53,0xfe},
	 {0x54,0x7f},
	 {0x61,0x8c},
	 {0x63,0x05},
	 {0x64,0x00},
	 {0x65,0x00},
	 {0x66,0x02},
	 {0x67,0x00},
	 {0x8d,0x00},
	 {0x8e,0x07},
	 {0x8f,0x88},
	 {0x90,0x04},
	 {0x91,0x40},
	 {0x95,0x88},
	 {0x98,0x88},
	 {0x9a,0x2b},
	 {0xb6,0x40},
	 {0x23,0x00},
	 {0x24,0x05},
	 {0x26,0x4d},
	 {0x29,0x00},
	 {0x2a,0x03},
	 {0x31,0x90},
	 {0x32,0x04},
	 {0x33,0x00},
	 {0x38,0x00},
	 {0x55,0x00},
	 {0xf5,0x01},
	 {0xfd,0x01},
	 {0x03,0x00},
	 {0x04,0x00},
	 /*
	 {0x05,0x04},
	 {0x06,0x65},*/
	 {0x4c,0x00},
	 {0x4d,0x04},
	 {0x24,0xff},
	 {0x45,0xff},
	 {0x09,0x00},
	 {0x0a,0x00},
	 {0x31,0x20},
	 {0x3e,0x3c},
	 {0x3f,0x01},
	 {0x01,0x01},
	 {0x11,0x40},
	 {0x14,0x65},
	 {0x15,0x00},
	 {0x16,0x95},
	 {0x18,0xf3},
	 {0x19,0x47},
	 {0x1a,0x03},
	 {0x1b,0x50},
	 {0x1c,0xf0},
	 {0x1d,0x3d},
	 {0x1e,0x00},
	 {0x1f,0x27},
	 {0x21,0x74},
	 {0x22,0x90},
	 {0x25,0x00},
	 {0x26,0x77},
	 {0x27,0xf9},
	 {0x2a,0x4e},
	 {0x2e,0x28},
	 {0x34,0xf6},
	 {0x35,0x22},
	 {0x36,0xa2},
	 {0x50,0x00},
	 {0x51,0x08},
	 {0x52,0x07},
	 {0x53,0x00},
	 {0x55,0x35},
	 {0x56,0x01},
	 {0x57,0x0a},
	 {0x59,0x01},
	 {0x5b,0x13},
	 {0x5d,0x08},
	 {0x5e,0x00},
	 {0x5f,0x1d},
	 {0x60,0x1e},
	 {0x68,0x05},
	 {0x69,0x04},
	 {0x6a,0x00},
	 {0x70,0x64},
	 {0x71,0x15},
	 {0x72,0x42},
	 {0x7b,0x40},
	 {0x80,0x08},
	 {0x86,0x2d},
	 {0x8a,0xf9},
	 {0x95,0x25},
	 {0x97,0x10},
	 {0x99,0x30},
	 {0x9f,0x10},
	 {0xa8,0x10},
	 {0xad,0x00},
	 {0xb0,0x63},
	 {0xb1,0x63},
	 {0xb2,0x63},
	 {0xb3,0x63},
	 {0xb4,0x61},
	 {0xb5,0x61},
	 {0xb6,0x61},
	 {0xb7,0x61},
	 {0xb8,0x77},
	 {0xb9,0x66},
	 {0xba,0x07},
	 {0xbb,0x07},
	 {0xbc,0x07},
	 {0xbd,0x07},
	 {0x29,0x0a},
	 {0x2b,0x03},
	 {0x30,0x04},
	 {0x92,0x05},
	 {0xc4,0x00},
	 {0xc5,0x00},
	 {0xc6,0x13},
	 {0xc7,0x6d},
	 {0xc9,0x00},
	 {0xd0,0x77},
	 {0xd1,0x66},
	 {0xd5,0x12},
	 {0xd7,0x40},
	 {0xdc,0x00},
	 {0xdd,0x01},
	 {0xde,0x04},
	 {0xfd,0x02},
	 {0x68,0x02},
	 {0x6d,0x5d},
	 {0x6e,0xdc},
	 {0x78,0x70},
	 {0x79,0x70},
	 {0x7a,0x70},
	 {0x7b,0x70},
	 {0x80,0x60},
	 {0x81,0x60},
	 {0x82,0x12},
	 {0x83,0x60},
	 {0x84,0x60},
	 {0x9f,0x12},
	 {0xa1,0x00},
	 {0xa2,0x04},
	 {0xa3,0x40},
	 {0xa5,0x00},
	 {0xa6,0x07},
	 {0xa7,0x88},
	 {0x72,0x40},
	 {0x73,0x40},
	 {0x74,0x40},
	 {0x75,0x40},
	 {0xf4,0x00},
	 {0xfd,0x06},
	 {0x00,0x04},
	 {0x01,0x7c},
	 {0x02,0xbf},
	 {0x03,0x86},
	 {0x04,0xbd},
	 {0x05,0x02},
	 {0xfd,0x02},
	 {0x52,0xff},
	 {0xfd,0x01},
	 /*
	 {0xfd,0x00},
	 {0x92,0x03},
	 {0xfd,0x01},
	 */
	 {0xfd,0x00},
	 {0xb1,0x02},
	 {0xfd,0x01},
	 {0xff,0x0a},
#endif
//@@ OS02H10_1920X1080_mipi_2lane_raw10_60fps_hdr_v3.0

//100 99 1920 1080

//102 80 01
//102 99b 11
//102 99c 11

//110 80 10
//110 58 8000000
//110 5a 8000000
//110 5c 8000000
//110 5e 8000000

//110 40 00

{0xfd, 0x00}, 
{0x20, 0x00},
{0x05, 0x05},
{0x53, 0xfe},
{0x54, 0x7f},
{0x61, 0x8c},//  ;mpll_divp_8lsb
{0x63, 0x05},//  ;mpll_prediv
{0x64, 0x00},//  ;mpll_divout
{0x65, 0x00},//  ;mpll_divsys
{0x66, 0x02},//  ;mpll_divbit
{0x67, 0x00},//  ;mpll_predivp
{0x8d, 0x00},//  ;mipi_rst.mipi_pd
{0x8e, 0x07},//  ;mipi_hsize_4msb
{0x8f, 0x80},//  ;mipi_hsize_8lsb
{0x90, 0x04},//  ;mipi_vsize_4msb
{0x91, 0x38},//  ;mipi_vsize_8lsb
{0x95, 0x88},
{0x98, 0x88},
{0x9a, 0x2b},
{0xb6, 0x40},//  ;mipi 2lane
{0x23, 0x00},//  ;pll_predivp
{0x24, 0x05},//  ;pll_prediv
{0x26, 0x4d},//  ;pll_divp
{0x29, 0x00},//  ;pll_divdac
{0x2a, 0x03},//  ;pll_divpump
{0x31, 0x90},//  ;clk_mode
{0x32, 0x04},//  ;clk_mode2
{0x33, 0x00},//  ;ao_mode_en
{0x38, 0x00},//  ;lvds_en
{0x55, 0x00},//  ;pclk_in_sel
{0xf5, 0x01},//  ;osc_cal_en
{0xfd, 0x01},
{0x03, 0x01},//  ;exp1 msb long
{0x04, 0x00},//  ;exp1 lsb long
{0x4c, 0x00},//  ;exp2 msb short
{0x4d, 0x04},//  ;exp2 lsb short
{0x24, 0xff},//   ;again long
{0x45, 0xff},//   ;again short
{0x09, 0x00},
{0x0a, 0x00},
{0x31, 0x20},//  ;hdr en
{0x3e, 0x3c},//  ;scg_en=1,col_gain
{0x3f, 0x00},//  ;mirror
{0x01, 0x01},
{0x11, 0x40},//  ;[6]db_shutter
{0x14, 0x65},
{0x15, 0x00},
{0x16, 0x95},
{0x18, 0xf3},//  ;d3-->f3 ramp_psrr_act_en=1
{0x19, 0x47},//  ;63
{0x1a, 0x03},
{0x1b, 0x50},
{0x1c, 0xf0},//  ;icomp1/icomp2
{0x1d, 0x3d},
{0x1e, 0x00},//  ;col_comp1_cap_sel
{0x1f, 0x27},//  ;ipix
{0x21, 0x74},//  ;ramp bias sel
{0x22, 0x90},//  ;vrhv
{0x25, 0x00},
{0x26, 0x77},//  ;vrnv1/vrnv2
{0x27, 0xf9},//  ;vcap
{0x2a, 0x4e},//  ;ramp_cur_code(adc_range)
{0x2e, 0x28},//  ;2a-->28 pwd_psnc_ramp_reg=0
{0x34, 0xf6},//  ;vbl up dn
{0x35, 0x22},
{0x36, 0xa2},
{0x50, 0x00},//  ;p0
{0x51, 0x08},//  ;p1
{0x52, 0x07},//  ;P2
{0x53, 0x00},//  ;p3
{0x55, 0x35},
{0x56, 0x01},
{0x57, 0x0a},
{0x59, 0x01},
{0x5b, 0x13},
{0x5d, 0x08},
{0x5e, 0x00},//  ;37  ;p12,rowsg
{0x5f, 0x1d},//  ;p13,rowsel
{0x60, 0x1e},//  ;p14,scg
{0x68, 0x05},
{0x69, 0x04},//  ;p24
{0x6a, 0x00},
{0x70, 0x64},//  ;p31,rst_d0
{0x71, 0x15},//  ;p32,rst_d1
{0x72, 0x42},//  ;p33,sig_d0
{0x7b, 0x40},//  ;p43
{0x80, 0x08},
{0x86, 0x2d},
{0x8a, 0xf9},
{0x95, 0x25},
{0x97, 0x10},
{0x99, 0x30},
{0x9f, 0x10},//  ;p90,rowsel
{0xa8, 0x10},//  ;p99,rowsg
{0xad, 0x00},//  ;p106,restg
{0xb0, 0x63},//  ;sc1_1
{0xb1, 0x63},//  ;sc1_2
{0xb2, 0x63},//  ;sc1_3
{0xb3, 0x63},//  ;sc1_4
{0xb4, 0x61},//  ;sc2_1
{0xb5, 0x61},//  ;sc2_2
{0xb6, 0x61},//  ;sc2_3
{0xb7, 0x61},//  ;sc2_4
{0xb8, 0x77},//  ;vbl_1/2_scg0
{0xb9, 0x66},//  ;vbl_3/4_scg0
{0xba, 0x07},//  ;vofs_1
{0xbb, 0x07},//  ;vofs_2
{0xbc, 0x07},//  ;vofs_3
{0xbd, 0x07},//  ;vofs_4

{0x29, 0x0a},//  ;ramp_psrr_sw_gain
{0x2b, 0x03},//  ;ramp_psrr_bias_sel
{0x30, 0x04},//  ;ramp_psnc_cap2_ctrl
{0x92, 0x05},
{0xc4, 0x00},
{0xc5, 0x00},
{0xc6, 0x13},
{0xc7, 0x6d},
{0xc9, 0x00},

{0xd0, 0x77},//  ;vbl_1/2_scg1
{0xd1, 0x66},//  ;vbl_3/4_scg1
{0xd5, 0x12},//  ;col_en1_num_8lsb
{0xd7, 0x40},//  ;col_en2_num_8lsb
{0xdc, 0x00},//  ;01  ;ulp pwd en
{0xdd, 0x01},//  ;ulp start
{0xde, 0x04},//  ;ulp end
{0xfd, 0x02},
{0x68, 0x02},//  ;[1]high/low 8 bit
{0x6d, 0x5d},//  ;blc en
{0x6e, 0xdc},//  ;auto trigger
{0x78, 0x70},//  ;blc k
{0x79, 0x70},
{0x7a, 0x70},
{0x7b, 0x70},
{0x80, 0x60},
{0x81, 0x60},
{0x82, 0x12},
{0x83, 0x60},
{0x84, 0x60},
{0x9f, 0x12},
{0xa1, 0x05},//  ;dig vstart
{0xa2, 0x04},//  ;dig vsize
{0xa3, 0x38},
{0xa5, 0x04},//  ;dig hstart
{0xa6, 0x07},//  ;dig hsize
{0xa7, 0x80},
{0x72, 0x40},//  ;offset
{0x73, 0x40},
{0x74, 0x40},
{0x75, 0x40},
{0xf4, 0x00},//  ;long and short frame output
//em er  ature sensor
{0xfd, 0x06},
{0x00, 0x04},//  ;r_tmp_slope_h[7:0]
{0x01, 0x7c},//  ;r_tmp_slope_l[7:0]
{0x02, 0xbf},//  ;r_tmp_offset_3[7:0]
{0x03, 0x86},//  ;r_tmp_offset_2[7:0]
{0x04, 0xbd},//  ;r_tmp_offset_1[7:0]
{0x05, 0x02},//  ;r_tmp_offset_0[7:0]
{0xfd, 0x02},
{0x52, 0xff},
{0xfd, 0x01},

{0xfd, 0x00},
{0xb1, 0x02},//  ;[1]mipi_en
{0xfd, 0x01},

//110 40 03

};



const I2C_ARRAY mirror_reg[] = {
    {0xfd, 0x01},
    {0x3f, 0x00}, //P1 M0F0 [1]:F [0]:M
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

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const static I2C_ARRAY gain_reg[] = {
    {0xfd, 0x01},
    {0x38, 0x10},//again msb
    {0x24, 0x20},//again 1x:0x10, 15.5x:0xf8
	{0xCD, 0x40},//dgain[7:0]  0x40-0xFF
    {0xCF, 0x00},//dgain Bit[6:4]: dig_gain_buf[10:8]
    {0x3e, 0x30},//linear ：lcg:0x30 hcg:0x34   hdr:lcg:0x30 hcg 0x3c
};
const I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0xfd, 0x01},
	{0x38, 0x10},//again msb
    {0x45, 0x10},//short a-gain[7:0]  1X:0X10 15.5:0XF8
    {0xCE, 0x40},// d-gain[7:0] 0X40-0XFF
    {0xCF, 0x00},// dgain Bit[2:0]: dig_gain_buf[10:8]
};
const I2C_ARRAY expo_reg[] = {
    {0x03, 0x01},
    {0x04, 0x00},
};
const I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x4C, 0x00},//short
    {0x4D, 0x04},
};
const I2C_ARRAY vts_reg[] = {
    /*{0x0d,0x10}0x10 enable*/
    {0x4E,0x07},//MSB
    {0x4F,0xc1},//LSB
};
const I2C_ARRAY vBlank_reg[] = {
    {0x05, 0x00},
    {0x06, 0x00},
};

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

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME OS02H10


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)     (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)     (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

int cus_camsensor_release_handle(ss_cus_sensor *handle);

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
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    int i=0;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);


    sensor_if->MCLK(idx, 0, handle->mclk);

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
	 if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }
    //Sensor power on sequence
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    //sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_384M);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);
    SENSOR_USLEEP(5000);

    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_USLEEP(1000);
    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(5000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(5050);

    //sensor_if->MCLK(idx, 1, handle->mclk);
    for(i=0;i<65600;i++)
    {}

    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
	if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(5000);
    return SUCCESS;
}
/*
static int pCus_poweron_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}
*/

/////////////////// image function /////////////////////////
//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
static int pCus_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
  int i,n;
  int table_length= ARRAY_SIZE(Sensor_id_table);
  I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

  SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
  for(n=0;n<table_length;++n) {
    id_from_sensor[n].reg = Sensor_id_table[n].reg;
    id_from_sensor[n].data = 0;
  }

  *id =0;
  if(table_length>8) table_length=8;

  for(n=0; n<5; ++n) {              //retry , until I2C success
    if(n>3) return FAIL;

    if(SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
    break;
    else
        SENSOR_MSLEEP_(1);
  }

  for(i=0; i<table_length; ++i) {
    if( id_from_sensor[i].data != Sensor_id_table[i].data ) {
      SENSOR_DMSG("[%s]Read OS02H10 id: 0x%x 0x%x\n", __FUNCTION__, id_from_sensor[0].data, id_from_sensor[1].data);
      return FAIL;
    }
    *id = id_from_sensor[i].data;
  }
  SENSOR_DMSG("[%s]Read OS02H10 id, get 0x%x Success\n", __FUNCTION__, (int)*id);
      return SUCCESS;
}

static int OS02H10_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    return SUCCESS;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
//static int pCus_SetAEGain_cal(ss_cus_sensor *handle, u32 gain);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_2m30_init(ss_cus_sensor *handle){
    int i,cnt=0;
	unsigned short otp_data = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
        for(i=0;i< ARRAY_SIZE(Sensor_2m30_init_table);i++)
        {
            if(Sensor_2m30_init_table[i].reg==0xff)
            {
                SENSOR_MSLEEP_(Sensor_2m30_init_table[i].data);
            }
            else
            {
                cnt = 0;
                while(SensorReg_Write(Sensor_2m30_init_table[i].reg,Sensor_2m30_init_table[i].data) != SUCCESS  && Sensor_2m30_init_table[i].reg != 0x20)
                {
                    cnt++;
                    //SENSOR_DMSG("Sensor_2m30_init_table -> Retry %d...\n",cnt);
                    if(cnt>=10)
                    {
                        //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                        return FAIL;
                    }
                    SENSOR_MSLEEP_(10);
                }
            }
        }


    for(i=0;i< ARRAY_SIZE(mirror_reg); i++) {
        if(SensorReg_Write(params->tMirror_reg[i].reg,params->tMirror_reg[i].data) != SUCCESS) {
            return FAIL;
        }
    }
	SensorReg_Write(0xfd, 0x04);
    SensorReg_Read(0x09, &otp_data);

	params->otp_data = (otp_data << 0)  ;

    pCus_SetAEUSecs(handle, 25000);
    //pCus_AEStatusNotify(handle,CUS_FRAME_ACTIVE);
    return SUCCESS;
}

static int pCus_2m60_init(ss_cus_sensor *handle){
    int i,cnt=0;
	unsigned short otp_data = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
        for(i=0;i< ARRAY_SIZE(Sensor_2m60_init_table);i++)
        {
            if(Sensor_2m60_init_table[i].reg==0xff)
            {
                SENSOR_MSLEEP_(Sensor_2m60_init_table[i].data);
            }
            else
            {
                cnt = 0;
                while(SensorReg_Write(Sensor_2m60_init_table[i].reg,Sensor_2m60_init_table[i].data) != SUCCESS  && Sensor_2m60_init_table[i].reg != 0x20)
                {
                    cnt++;
                    //SENSOR_DMSG("Sensor_2m60_init_table -> Retry %d...\n",cnt);
                    if(cnt>=10)
                    {
                        //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                        return FAIL;
                    }
                    SENSOR_MSLEEP_(10);
                }
            }
        }


    for(i=0;i< ARRAY_SIZE(mirror_reg); i++) {
        if(SensorReg_Write(params->tMirror_reg[i].reg,params->tMirror_reg[i].data) != SUCCESS) {
            return FAIL;
        }
    }
	SensorReg_Write(0xfd, 0x04);
    SensorReg_Read(0x09, &otp_data);


	params->otp_data = (otp_data << 0)  ;

    pCus_SetAEUSecs(handle, 25000);
    //pCus_AEStatusNotify(handle,CUS_FRAME_ACTIVE);
    return SUCCESS;
}

static int pCus_init_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;
	unsigned short otp_data = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    for(i=0;i< ARRAY_SIZE(Sensor_2m30init_table_HDR);i++)
    {
        if(Sensor_2m30init_table_HDR[i].reg==0xff)
        {
            SENSOR_MSLEEP(Sensor_2m30init_table_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_2m30init_table_HDR[i].reg,Sensor_2m30init_table_HDR[i].data) != SUCCESS  && Sensor_2m30init_table_HDR[i].reg != 0x20)
            {
                cnt++;
                //SENSOR_DMSG("Sensor_init_table_HDR -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
	SensorReg_Write(0xfd, 0x04);
    SensorReg_Read(0x09, &otp_data);

	params->otp_data = (otp_data << 0)  ;

	/*
	 for(i=0;i< ARRAY_SIZE(mirror_reg); i++) {
        if(SensorReg_Write(params->tMirror_reg[i].reg,params->tMirror_reg[i].data) != SUCCESS) {
            return FAIL;
        }
    }*/

    //pCus_SetAEUSecs(handle, 25000);
    //pCus_AEStatusNotify(handle,CUS_FRAME_ACTIVE);
    //CamOsPrintf("pCus_init = %d us \n",timeGetTimeU()-TStart);
    return SUCCESS;
}
static int pCus_init_HDR_LEF(ss_cus_sensor *handle)
{
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
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
#if 0
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_2m30_init;
            break;
#endif
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_2m60_init;
            break;
        default:
            break;
    }

    return SUCCESS;
}
static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short=256;
            break;
        default:
            break;
    }

    return SUCCESS;
}
static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;

    sen_data = params->tMirror_reg[1].data & 0x03;
    SENSOR_DMSG("\n\n[%s]:mirror:%x\r\n\n\n\n",__FUNCTION__, sen_data);
    switch(sen_data) {
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

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    OS02H10_params *params = (OS02H10_params *)handle->private_data;

   switch(orit) {
      case CUS_ORIT_M0F0:
            params->tMirror_reg[1].data = 0x00;
            //params->tMirror_reg[3].data = 0x22;
            params->orien_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[1].data = 0x01;
            //params->tMirror_reg[3].data = 0x32;
            params->orien_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[1].data = 0x02;
            //params->tMirror_reg[3].data = 0x32;
            params->orien_dirty = true;
        break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[1].data = 0x03;
            //params->tMirror_reg[3].data = 0x32;
            params->orien_dirty = true;
        break;
    }

    return SUCCESS;
}
static int pCus_SetOrien_HDR(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    OS02H10_params *params = (OS02H10_params *)handle->private_data;

   switch(orit) {
      case CUS_ORIT_M0F0:
            params->tMirror_reg[1].data = 0x00;
            //params->tMirror_reg[3].data = 0x22;
            params->orien_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[1].data = 0x01;
            //params->tMirror_reg[3].data = 0x32;

            params->orien_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[1].data = 0x02;
            //params->tMirror_reg[3].data = 0x32;

            params->orien_dirty = true;
        break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[1].data = 0x03;
            //params->tMirror_reg[3].data = 0x32;

            params->orien_dirty = true;
        break;
    }

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
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
    //u32 vts=0;
	u32 vBlank=0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
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
    if ((params->expo.lines) >= (params->expo.usVts - 9)){     //当shutter过大，需要vts降帧的时候
		//vts = params->expo.lines + 9;
		if((params->expo.vts)>(params->expo.usVts)){  //当设置的帧率小于当前帧率，会继续增大vlank
			vBlank=params->expo.vts-params->expo.usVts+vBlankInit;
		}else{ //当设置的帧率大于shutter对应的帧率，设置会不成功，维持当前帧率
			vBlank=vBlankInit;
		};
	}
    else{ //当shutter小于降帧对应的shutter，完全会按照setfps的来配置
		vBlank = params->expo.vts-vts_30fps+vBlankInit;
	}
	params->tVBlank_reg[0].data = (vBlank >> 8) & 0x00ff;
    params->tVBlank_reg[1].data = (vBlank >> 0) & 0x00ff;

    params->reg_dirty = true;
    return SUCCESS;
}
static int pCus_GetFPS_HDR_SEF(ss_cus_sensor *handle)
{
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_SEF(ss_cus_sensor *handle, u32 fps)
{
    //u32 vts=0;
	u32 vBlank=0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
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
    if ((params->expo.lines) >= (params->expo.usVts - 9)){     //当shutter过大，需要vts降帧的时候
		//vts = params->expo.lines + 9;
		if((params->expo.vts)>(params->expo.usVts)){  //当设置的帧率小于当前帧率，会继续增大vlank


			vBlank=params->expo.vts-params->expo.usVts+vBlankInit_HDR;
		}else{ //当设置的帧率大于shutter对应的帧率，设置会不成功，维持当前帧率
			vBlank=vBlankInit_HDR;
		};
	}    
    else{ //当shutter小于降帧对应的shutter，完全会按照setfps的来配置
		vBlank = params->expo.vts-vts_30fps_HDR+vBlankInit_HDR;
	}
	params->tVBlank_reg[0].data = (vBlank >> 8) & 0x00ff;
    params->tVBlank_reg[1].data = (vBlank >> 0) & 0x00ff;
	 params->tVBlank_reg[0].data = (vBlank >> 8) & 0x00ff;
    params->tVBlank_reg[1].data = (vBlank >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
     OS02H10_params *params = (OS02H10_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if(params->orien_dirty){
                handle->sensor_if_api->SetSkipFrame(idx, params->expo.fps, 3);
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orien_dirty = false;
            }
            break;
        case CUS_FRAME_ACTIVE:
            if(params->reg_dirty)
            {

                 SensorReg_Write(0xfd,0x01);//page 1
                SensorReg_Write(0x01,0x00);//frame sync disable
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
				//printk("%s\t%s\t%x\n",__FUNCTION__,"params->tVBlank_reg[0].data",params->tVBlank_reg[0].data);
				//printk("%s\t%s\t%x\n",__FUNCTION__,"params->tVBlank_reg[1].data",params->tVBlank_reg[1].data);
                //SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVBlank_reg, ARRAY_SIZE(vBlank_reg));
                SensorReg_Write(0x01,0x01);//frame sync enable
                params->reg_dirty = false;

            }
            break;
        default :
            break;
    }
    return SUCCESS;
}
static int pCus_AEStatusNotify_HDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
     OS02H10_params *params = (OS02H10_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if(params->orien_dirty){
                handle->sensor_if_api->SetSkipFrame(idx, params->expo.fps, 3);
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orien_dirty = false;
            }

            break;
        case CUS_FRAME_ACTIVE:

            if(params->reg_dirty)
            {

                SensorReg_Write(0xfd,0x01);//page 1
                SensorReg_Write(0x01,0x00);//frame sync disable
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
				SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
				SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
                //SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
               SensorRegArrayW((I2C_ARRAY*)params->tVBlank_reg, ARRAY_SIZE(vBlank_reg));
                SensorReg_Write(0x01,0x01);//frame sync enable

                params->reg_dirty = false;

            }
            break;
        default :
            break;
    }
    return SUCCESS;
}
static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    u32 lines = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    lines  =  (u32)(params->tExpo_reg[1].data&0xff);
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;

    *us = (lines*Preview_line_period)/1000;
    return SUCCESS;
}
static int pCus_GetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 *us) {
    u32 lines = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    lines  =  (u32)(params->tExpo_reg_HDR_SEF[1].data&0xff);
    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0xff)<<8;

    *us = (lines*Preview_line_period)/1000;
    return SUCCESS;
}
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
	//printk("%s\t%s\t%d\n",__FUNCTION__,"US",us);

    lines=(u32)((1000*us)/Preview_line_period);
	//printk("%s\t%s\t%d\n",__FUNCTION__,"lines",lines);
	//printk("%s\t%s\t%d\n",__FUNCTION__,"expo.vts",params->expo.vts);

    if (lines < 2) lines = 2;
    if (lines >params->expo.usVts - 9){
		vts = lines + 9;
		params->expo.usVts=vts;
	}else{
		vts=params->expo.vts;
		params->expo.usVts=vts_30fps;
	}

	params->expo.lines=lines;
	//printk("%s\t%s\t%d\n",__FUNCTION__,"vts",vts);
    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

   // lines <<= 4;
   //printk("%s\t%s\t%d\n",__FUNCTION__,"lines",lines);
    params->tExpo_reg[0].data =(u16)( (lines>>8) & 0x00ff);
    params->tExpo_reg[1].data =(u16)( (lines>>0) & 0x00ff);
	
    //params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    //params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
	params->reg_dirty = true;
  return SUCCESS;
}
static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;


    lines=(u32)((1000*us)/Preview_line_period_HDR);
	//printk("%s\t%s\t%d\n",__FUNCTION__,"lines",lines);
	//printk("%s\t%s\t%d\n",__FUNCTION__,"expo.vts",params->expo.vts);

    if (lines < 2) lines = 2;
    if (lines >params->expo.usVts - 9){
		vts = lines + 9;
		params->expo.usVts=vts;
	}else{
		vts=params->expo.vts;
		params->expo.usVts=vts_30fps_HDR;
	}

	params->expo.lines=lines;
	//printk("%s\t%s\t%d\n",__FUNCTION__,"vts",vts);
    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );


    params->tExpo_reg[0].data =(u16)( (lines>>8) & 0x00ff);
    params->tExpo_reg[1].data =(u16)( (lines>>0) & 0x00ff);;

    //params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    //params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
	params->reg_dirty = true;
  return SUCCESS;
}
static int pCus_SetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;

    lines=(u32)((1000*us)/Preview_line_period_HDR);

	//printk("%s\t%s\t%d\n",__FUNCTION__,"expo.vts",params->expo.vts);

    if (lines < 2) lines = 2;
	if (lines >= params->expo.max_short) {
        lines = params->expo.max_short;
    }else{
		vts = params->expo.vts;
	};


	params->expo.lines=lines;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

   // lines <<= 4;

    params->tExpo_reg_HDR_SEF[0].data =(u16)( (lines>>8) & 0x00ff);
    params->tExpo_reg_HDR_SEF[1].data =(u16)( (lines>>0) & 0x00ff);
    vts=vts;
    //params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    //params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
	params->reg_dirty = true;
  return SUCCESS;
}
// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    int again;
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    again=params->tGain_reg[2].data;

    *gain = again<<6;
    return SUCCESS;
}
/*
static int pCus_SetAEGain_cal(ss_cus_sensor *handle, u32 gain) {

    OS02H10_params *params = (OS02H10_params *)handle->private_data;
    u32 input_gain = 0;
    u16 gain16;

    gain = (gain * handle->sat_mingain + 512)>>10; // need to add min sat gain

    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    gain16=(u16)(gain>>6);
    params->tGain_reg[1].data = (gain16>>8)&0x01;//high bit
    params->tGain_reg[2].data = gain16&0xff; //low byte

    SENSOR_DMSG("[%s] set gain/regH/regL=%d/0x%x/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data,params->tGain_reg[1].data);
    //params->reg_dirty = true;
    return SUCCESS;
}
*/
u32 lcg=0;
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
	u32 input_gain = 0;
    u16 gain16 = 0;
	u16 dGain16=0;
	u16 dgain_fine=0;
	u16 tmp_dgain=0;
    gain = (gain * g_sensor_ae_min_gain + 512)>>10; // need to add min sat gain
	if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN) gain = SENSOR_MAX_GAIN;

	if (gain < (256 * 1024 / params->otp_data))
    {
        lcg = 1;
    }else
    {
        gain = gain * 1024/ (256 * 1024 / params->otp_data);
        lcg = 0;
    }
	input_gain = gain;
    /* A Gain */
	 if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < MAX_A_GAIN)) {
        gain = (gain>>9)<<9;
    }
	gain16=(u16)(gain>>6);
    params->tGain_reg[2].data = gain16&0xff; //low byte


	/* D Gain */
	if(input_gain>MAX_A_GAIN){
		tmp_dgain=(input_gain*1024)/MAX_A_GAIN;
		dGain16=(tmp_dgain>>4);
		dgain_fine=(tmp_dgain>>4)>>8;

		params->tGain_reg[2].data = 0xf8;
		params->tGain_reg[3].data=(dGain16) & 0xFF;
		params->tGain_reg[4].data=((dgain_fine<<4 ) & 0xf0);

	}else{
		params->tGain_reg[2].data = gain16&0xff;
		params->tGain_reg[3].data=(0x40);
		params->tGain_reg[4].data=(0x00);
	};
		//lcg hcg
	params->tGain_reg[5].data = lcg ? 0x30 : 0x34; //low byte
    params->reg_dirty = true;
    //pr_info("[%s] set input gain/gain/AregH/AregL/DregH/DregL=%d/%d/0x%x/0x%x/0x%x/0x%x\n", __FUNCTION__, input_gain,gain,gain_reg[0].data,gain_reg[1].data,gain_reg[2].data,gain_reg[3].data);
    return SUCCESS;
}
u16 hdrDGainLefFine=0;
u16 hdrDGainSefFine=0;
static int pCus_SetAEGain_HDR_LEF(ss_cus_sensor *handle, u32 gain) {
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
	u32 input_gain = 0;
    u16 gain16 = 0;
	u16 dGain16=0;
	u16 dgain_fine=0;
	u16 tmp_dgain=0;
    gain = (gain * g_sensor_ae_min_gain + 512)>>10; // need to add min sat gain
	if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN) gain = SENSOR_MAX_GAIN;
	if (gain < (256 * 1024 / params->otp_data))
    {
        lcg = 1;
    }else
    {
        gain = gain * 1024/ (256 * 1024 / params->otp_data);
        lcg = 0;
    }
    input_gain = gain;
    /* A Gain */
	 if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < MAX_A_GAIN)) {
        gain = (gain>>9)<<9;
    }
	gain16=(u16)(gain>>6);
    params->tGain_reg[2].data = gain16&0xff; //low byte


	/* D Gain */
	if(input_gain>MAX_A_GAIN){
		tmp_dgain=(input_gain*1024)/MAX_A_GAIN;
		dGain16=(tmp_dgain>>4);
		dgain_fine=(tmp_dgain>>4)>>8;

		params->tGain_reg[2].data = 0xf8;
		params->tGain_reg[3].data=(dGain16) & 0xFF;
		hdrDGainLefFine=(dgain_fine<<4)&0xf0;

		params->tGain_reg[4].data=((hdrDGainLefFine ) | hdrDGainSefFine);
		
	}else{
		params->tGain_reg[2].data = gain16&0xff;
		params->tGain_reg[3].data=(0x40);
		params->tGain_reg[4].data=(0x00);
	};
	params->tGain_reg[5].data = lcg ? 0x30:0x3C; //low byte
    params->reg_dirty = true;
    //pr_info("[%s] set input gain/gain/AregH/AregL/DregH/DregL=%d/%d/0x%x/0x%x/0x%x/0x%x\n", __FUNCTION__, input_gain,gain,gain_reg[0].data,gain_reg[1].data,gain_reg[2].data,gain_reg[3].data);
    return SUCCESS;
}
static int pCus_SetAEGain_HDR_SEF(ss_cus_sensor *handle, u32 gain) {
    OS02H10_params *params = (OS02H10_params *)handle->private_data;
	u32 input_gain = 0;
    u16 gain16 = 0;
	u16 dGain16=0;
	u16 dgain_fine=0;
	u16 tmp_dgain=0;
    gain = (gain * g_sensor_ae_min_gain + 512)>>10; // need to add min sat gain
	
    input_gain = gain;
	if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN) gain = SENSOR_MAX_GAIN;
    if (!lcg)
    {
        gain = gain * 1024/ (256 * 1024 / params->otp_data);
    };
    /* A Gain */
	 if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < MAX_A_GAIN)) {
        gain = (gain>>9)<<9;
    }
	gain16=(u16)(gain>>6);
    params->tGain_reg_HDR_SEF[2].data = gain16&0xff; //low byte

	/* D Gain */
	if(input_gain>MAX_A_GAIN){
		tmp_dgain=(input_gain*1024)/MAX_A_GAIN;
		dGain16=(tmp_dgain>>4);
		dgain_fine=(tmp_dgain>>4)>>8;
		params->tGain_reg_HDR_SEF[2].data = 0xf8;
		params->tGain_reg_HDR_SEF[3].data=(dGain16) & 0xFF;
		hdrDGainSefFine=(dgain_fine)&0x0f;
		params->tGain_reg_HDR_SEF[4].data=((hdrDGainLefFine ) | hdrDGainSefFine);
		
	}else{
		params->tGain_reg_HDR_SEF[2].data = gain16&0xff;
		params->tGain_reg_HDR_SEF[3].data=(0x40);
		params->tGain_reg_HDR_SEF[4].data=(0x00);
	};
    params->reg_dirty = true;
    //pr_info("[%s] set input gain/gain/AregH/AregL/DregH/DregL=%d/%d/0x%x/0x%x/0x%x/0x%x\n", __FUNCTION__, input_gain,gain,gain_reg[0].data,gain_reg[1].data,gain_reg[2].data,gain_reg[3].data);
    return SUCCESS;
}
/*

static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max) {
  *min = 1;
  *max = 1000000/Preview_MIN_FPS;
  return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max) {
  *min = 1024;
    *max = SENSOR_MAX_GAIN;
  return SUCCESS;
}


static int OS02H10_GetShutterInfo(struct __ms_cus_sensor* handle,CUS_SHUTTER_INFO *info){
    info->max = 1000000000/Preview_MIN_FPS;
    info->min = Preview_line_period * 2;
    info->step = Preview_line_period;
    return SUCCESS;
}
static int OS02H10_GetShutterInfo_HDR_SEF(struct __ms_cus_sensor* handle,CUS_SHUTTER_INFO *info){
	OS02H10_params *params = (OS02H10_params *)handle->private_data;
    info->max = Preview_line_period*params->expo.max_short;
    info->min = Preview_line_period * 2;
    info->step = Preview_line_period;
    return SUCCESS;
}
*/

int cus_camsensor_init_handle(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    OS02H10_params *params;
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
    params = (OS02H10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tVBlank_reg, vBlank_reg, sizeof(vBlank_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OS02H10_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = os02h10_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = os02h10_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = os02h10_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = os02h10_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = os02h10_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = os02h10_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth   = os02h10_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight  = os02h10_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, os02h10_mipi_linear[res].senstr.strResDesc);
    }

    // i2c

    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    //polarity
    /////////////////////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    //handle->ae_gain_delay       = 2;
    //handle->ae_shutter_delay    = 2;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 1;

    ///calibration
    //handle->sat_mingain=g_sensor_ae_min_gain;


    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_2m30_init    ;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID   ;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien      ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS      ;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS      ;
    handle->pCus_sensor_SetPatternMode = OS02H10_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;


    params->expo.vts=vts_30fps;
    params->expo.usVts=vts_30fps;
    params->expo.fps = 30;
    params->expo.lines = 1000;
    params->reg_dirty = false;
    params->orien_dirty = false;
    params->orit       = SENSOR_ORIT;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = 1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    return SUCCESS;
}
int cus_camsensor_init_handle_HDR_SEF(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    OS02H10_params *params;

    cus_camsensor_init_handle(drv_handle);
    params = (OS02H10_params *)handle->private_data;

    sprintf(handle->strSensorStreamName,"OS02H10_MIPI_HDR_SEF");

    handle->bayer_id    = SENSOR_BAYERID_HDR;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->data_prec   = SENSOR_DATAPREC_HDR;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].u16width = 1920;
    handle->video_res_supported.res[0].u16height = 1080;
    handle->video_res_supported.res[0].u16max_fps= Preview_MAX_FPS_HDR;
    handle->video_res_supported.res[0].u16min_fps= Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x= 0;
    handle->video_res_supported.res[0].u16crop_start_y= 0;
    handle->video_res_supported.res[0].u16OutputWidth = 1920;
    handle->video_res_supported.res[0].u16OutputHeight = 1080;
    sprintf(handle->video_res_supported.res[0].strResDesc, "1928x1088@30fps_HDR");

    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR;
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    handle->pCus_sensor_init        = pCus_init_HDR;

    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_SEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_SEF;
    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien_HDR;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_HDR_SEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_SEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_SEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    //handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_HDR_SEF;
    //handle->pCus_sensor_GetShutterInfo = OS02H10_GetShutterInfo_HDR_SEF;

    params->expo.vts = vts_30fps_HDR;
    params->expo.usVts = vts_30fps_HDR;
    params->expo.lines = 1000;
    params->expo.fps = 30;
    params->expo.max_short = 256;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame
    params->reg_dirty = false;
    params->orien_dirty = false;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = 1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period*params->expo.max_short;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    OS02H10_params *params;
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
    params = (OS02H10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OS02H10_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_HDR;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->orient      = SENSOR_ORIT_HDR;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;
    handle->video_res_supported.res[0].u16width             = 1920;
    handle->video_res_supported.res[0].u16height            = 1080;
    handle->video_res_supported.res[0].u16max_fps           = Preview_MAX_FPS_HDR;
    handle->video_res_supported.res[0].u16min_fps           = Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x      = 0;
    handle->video_res_supported.res[0].u16crop_start_y      = 0;
    handle->video_res_supported.res[0].u16OutputWidth       = 1920;
    handle->video_res_supported.res[0].u16OutputHeight      = 1080;
    sprintf(handle->video_res_supported.res[0].strResDesc, "1920x1088@30fps_HDR");

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //polarity
    /////////////////////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ///calibration
    //handle->sat_mingain=g_sensor_ae_min_gain;


    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_HDR_LEF;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID;

    handle->pCus_sensor_GetVideoResNum = NULL;
    handle->pCus_sensor_GetVideoRes       = NULL;
    handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes       = NULL;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien_HDR;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_SEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_SEF;
    handle->pCus_sensor_SetPatternMode = OS02H10_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_HDR_SEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_HDR_LEF;

    params->expo.vts=vts_30fps_HDR;
    params->expo.usVts=vts_30fps_HDR;
    params->expo.fps = 30;
    params->expo.max_short=256;
    params->reg_dirty = false;
    params->orien_dirty = false;
    params->orit       = SENSOR_ORIT;
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = 1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period*params->expo.max_short;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    return SUCCESS;
}

int cus_camsensor_release_handle(ss_cus_sensor *handle) {
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  OS02H10,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_HDR_SEF,
                            cus_camsensor_init_handle_HDR_LEF,
                            OS02H10_params
                         );

