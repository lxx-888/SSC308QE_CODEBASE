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
Remark        :NA
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>
#include <drv_sensor_init_table.h> //TODO: move this header to drv_sensor_common.h
#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX485_HDR);

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
#define SENSOR_MIPI_LANE_NUM        (4)
#define SENSOR_MIPI_LANE_NUM_DOL    (4)
//#define SENSOR_MIPI_HDR_MODE        (0) //0: Non-HDR mode. 1:Sony DOL mode
#define SENSOR_HDR_MODE             CUS_HDR_MODE_VC

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
//#define SENSOR_DATAFMT             CUS_DATAFMT_BAYER    //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_12
#define SENSOR_DATAPREC_DOL         CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9098  //CFG
#define SENSOR_BAYERID              CUS_BAYER_RG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_RG
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_27MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_27MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x34                //I2C slave address
#define SENSOR_I2C_SPEED             300000              //200000 //300000 //240000                  //I2C speed, 60000~320000
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
#define SENSOR_NAME     IMX485

#define CHIP_ID_r3F12   0x3F12
#define CHIP_ID_r3F13   0x3F13
#define CHIP_ID         0x0485

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
}imx485_mipi_linear[] = {
    {LINEAR_RES_1, {3864, 2180, 3, 30}, {12, 10, 3840, 2160}, {"3840x2160@30fps"}},
    {LINEAR_RES_2, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}}, // Modify it
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
}imx485_mipi_hdr[] = {
    {HDR_RES_1, {3864, 2192, 3, 30}, {12, 10, 3840, 2160}, {"3840x2160@30fps_HDR_4lane"}}, // Modify it
    //{HDR_RES_2, {1944, 1097, 3, 30}, {12, 8, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
};

u32 vts_30fps = 2250;
u32 vts_30fps_HDR_DOL_4lane = 2250;
u32 Preview_line_period = 14814;
u32 Preview_line_period_HDR_DOL_4LANE = 7407;

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR_FLIP                                 0x3030
#define SENSOR_NOR                                  0x0
#define SENSOR_MIRROR_EN                            0x1 << (0)
#define SENSOR_FLIP_EN                              0x1 << (1)
#define SENSOR_MIRROR_FLIP_EN                       0x3

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (3981 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)
#define SENSOR_GAIN_CTRL_NUM                        (1)
#define SENSOR_SHUTTER_CTRL_NUM                     (1)
#define SENSOR_GAIN_CTRL_NUM_HDR                    (2)
#define SENSOR_SHUTTER_CTRL_NUM_HDR                 (2)

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif

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

    u32 min_shr1;
    u32 min_rhs1;
    u32 min_shr0;
    u32 max_shr0;
    u32 fsc;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tVts_reg_hdr[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tSHR0_reg[3];
    I2C_ARRAY tSHR1_reg[3];
    I2C_ARRAY tRHS1_reg[3];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tGain_hdr_dol_lef_reg[2];
    I2C_ARRAY tGain_hdr_dol_sef_reg[2];
    bool dirty;
    bool orien_dirty;
    bool dual_4lane_mode;
} imx485_params;

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us);
///////////////////////////////////////////////////////////////
//          @@@                                              //
//         @  @@                                             //
//           @@                                              //
//          @@                                               //
//         @@@@@                                             //
//                                                           //
//      Start Step 2 --  set Sensor initial and              //
//                       adjust parameter                    //
//  Fill these register table with resolution settings       //
//      camera can work and show preview on LCM              //
//                                                           //
///////////////////////////////////////////////////////////////

//8M30 891Mbps, 12bits, Mclk 27M
const static I2C_ARRAY Sensor_8m_30fps_init_table_4lane_linear[] =
{
/*
IMX485LQJ All-pixel scan CSI-2_4lane 27MHz AD:12bit Output:12bit 1440Mbps Master Mode LCG Mode 30fps
Integration Time 33.244ms
Tool ver : Ver7.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5C},// BCWAIT_TIME[9:0]
    {0x300A, 0x42},// CPWAIT_TIME[9:0]
    {0x300B, 0x40},//
    {0x3028, 0x4C},// HMAX[15:0]
    {0x3029, 0x04},//
    {0x30A5, 0x00},// XVS_DRV[1:0]
    {0x3114, 0x02},// INCKSEL1[1:0]
    {0x311C, 0xD5},// INCKSEL3[8:0]
    {0x3260, 0x22},
    {0x3262, 0x02},
    {0x3278, 0xA2},
    {0x3324, 0x00},
    {0x3366, 0x31},
    {0x340C, 0x4D},
    {0x3416, 0x10},
    {0x3417, 0x13},
    {0x3432, 0x93},
    {0x34CE, 0x1E},
    {0x34CF, 0x1E},
    {0x34DC, 0x80},
    {0x351C, 0x03},
    {0x359E, 0x70},
    {0x35A2, 0x9C},
    {0x35AC, 0x08},
    {0x35C0, 0xFA},
    {0x35C2, 0x4E},
    {0x3608, 0x41},
    {0x360A, 0x47},
    {0x361E, 0x4A},
    {0x3630, 0x43},
    {0x3632, 0x47},
    {0x363C, 0x41},
    {0x363E, 0x4A},
    {0x3648, 0x41},
    {0x364A, 0x47},
    {0x3660, 0x04},
    {0x3676, 0x3F},
    {0x367A, 0x3F},
    {0x36A4, 0x41},
    {0x3798, 0x82},
    {0x379A, 0x82},
    {0x379C, 0x82},
    {0x379E, 0x82},
    {0x3804, 0x22},// INCKSEL4[1:0]
    {0x3807, 0x84},// INCKSEL5[7:0]
    {0x3888, 0xA8},
    {0x388C, 0xA6},
    {0x3914, 0x15},
    {0x3915, 0x15},
    {0x3916, 0x15},
    {0x3917, 0x14},
    {0x3918, 0x14},
    {0x3919, 0x14},
    {0x391A, 0x13},
    {0x391B, 0x13},
    {0x391C, 0x13},
    {0x391E, 0x00},
    {0x391F, 0xA5},
    {0x3920, 0xED},
    {0x3921, 0x0E},
    {0x39A2, 0x0C},
    {0x39A4, 0x16},
    {0x39A6, 0x2B},
    {0x39A7, 0x01},
    {0x39D2, 0x2D},
    {0x39D3, 0x00},
    {0x39D8, 0x37},
    {0x39D9, 0x00},
    {0x39DA, 0x9B},
    {0x39DB, 0x01},
    {0x39E0, 0x28},
    {0x39E1, 0x00},
    {0x39E2, 0x2C},
    {0x39E3, 0x00},
    {0x39E8, 0x96},
    {0x39EA, 0x9A},
    {0x39EB, 0x01},
    {0x39F2, 0x27},
    {0x39F3, 0x00},
    {0x3A00, 0x38},
    {0x3A01, 0x00},
    {0x3A02, 0x95},
    {0x3A03, 0x01},
    {0x3A18, 0x9B},
    {0x3A2A, 0x0C},
    {0x3A30, 0x15},
    {0x3A32, 0x31},
    {0x3A33, 0x01},
    {0x3A36, 0x4D},
    {0x3A3E, 0x11},
    {0x3A40, 0x31},
    {0x3A42, 0x4C},
    {0x3A43, 0x01},
    {0x3A44, 0x47},
    {0x3A46, 0x4B},
    {0x3A4E, 0x11},
    {0x3A50, 0x32},
    {0x3A52, 0x46},
    {0x3A53, 0x01},
    {0x3D01, 0x03},//MIPI 4Lane mode
    {0x3D04, 0xC0},// TXCLKESC_FREQ[15:0]
    {0x3D05, 0x06},//
    {0x3D18, 0x9F},// TCLKPOST[15:0]
    {0x3D1A, 0x57},// TCLKPREPARE[15:0]
    {0x3D1C, 0x57},// TCLKTRAIL[15:0]
    {0x3D1E, 0x87},// TCLKZERO[15:0]
    {0x3D20, 0x5F},// THSPREPARE[15:0]
    {0x3D22, 0xA7},// THSZERO[15:0]
    {0x3D24, 0x5F},// THSTRAIL [15:0]
    {0x3D26, 0x97},// THSEXIT [15:0]
    {0x3D28, 0x4F},// TLPX[15:0]

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

#if 0
const static I2C_ARRAY Sensor_8m_30fps_init_table_8lane_linear[] =
{
/*
 * ALL pixel mode
IMX485LQJ All-pixel scan CSI-2_4lane 27MHz AD:12bit Output:12bit 1440Mbps Master Mode LCG Mode 30fps
Integration Time 33.244ms
Tool ver : Ver7.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5C},// BCWAIT_TIME[9:0]
    {0x300A, 0x42},// CPWAIT_TIME[9:0]
    {0x300B, 0x40},//
    {0x3028, 0x4C},// HMAX[15:0]
    {0x3029, 0x04},//
    {0x30A5, 0x00},// XVS_DRV[1:0]
    {0x3114, 0x02},// INCKSEL1[1:0]
    {0x311C, 0xD5},// INCKSEL3[8:0]
    {0x3260, 0x22},
    {0x3262, 0x02},
    {0x3278, 0xA2},
    {0x3324, 0x00},
    {0x3366, 0x31},
    {0x340C, 0x4D},
    {0x3416, 0x10},
    {0x3417, 0x13},
    {0x3432, 0x93},
    {0x34CE, 0x1E},
    {0x34CF, 0x1E},
    {0x34DC, 0x80},
    {0x351C, 0x03},
    {0x359E, 0x70},
    {0x35A2, 0x9C},
    {0x35AC, 0x08},
    {0x35C0, 0xFA},
    {0x35C2, 0x4E},
    {0x3608, 0x41},
    {0x360A, 0x47},
    {0x361E, 0x4A},
    {0x3630, 0x43},
    {0x3632, 0x47},
    {0x363C, 0x41},
    {0x363E, 0x4A},
    {0x3648, 0x41},
    {0x364A, 0x47},
    {0x3660, 0x04},
    {0x3676, 0x3F},
    {0x367A, 0x3F},
    {0x36A4, 0x41},
    {0x3798, 0x82},
    {0x379A, 0x82},
    {0x379C, 0x82},
    {0x379E, 0x82},
    {0x3804, 0x22},// INCKSEL4[1:0]
    {0x3807, 0x84},// INCKSEL5[7:0]
    {0x3888, 0xA8},
    {0x388C, 0xA6},
    {0x3914, 0x15},
    {0x3915, 0x15},
    {0x3916, 0x15},
    {0x3917, 0x14},
    {0x3918, 0x14},
    {0x3919, 0x14},
    {0x391A, 0x13},
    {0x391B, 0x13},
    {0x391C, 0x13},
    {0x391E, 0x00},
    {0x391F, 0xA5},
    {0x3920, 0xED},
    {0x3921, 0x0E},
    {0x39A2, 0x0C},
    {0x39A4, 0x16},
    {0x39A6, 0x2B},
    {0x39A7, 0x01},
    {0x39D2, 0x2D},
    {0x39D3, 0x00},
    {0x39D8, 0x37},
    {0x39D9, 0x00},
    {0x39DA, 0x9B},
    {0x39DB, 0x01},
    {0x39E0, 0x28},
    {0x39E1, 0x00},
    {0x39E2, 0x2C},
    {0x39E3, 0x00},
    {0x39E8, 0x96},
    {0x39EA, 0x9A},
    {0x39EB, 0x01},
    {0x39F2, 0x27},
    {0x39F3, 0x00},
    {0x3A00, 0x38},
    {0x3A01, 0x00},
    {0x3A02, 0x95},
    {0x3A03, 0x01},
    {0x3A18, 0x9B},
    {0x3A2A, 0x0C},
    {0x3A30, 0x15},
    {0x3A32, 0x31},
    {0x3A33, 0x01},
    {0x3A36, 0x4D},
    {0x3A3E, 0x11},
    {0x3A40, 0x31},
    {0x3A42, 0x4C},
    {0x3A43, 0x01},
    {0x3A44, 0x47},
    {0x3A46, 0x4B},
    {0x3A4E, 0x11},
    {0x3A50, 0x32},
    {0x3A52, 0x46},
    {0x3A53, 0x01},
    {0x3D01, 0x07},//MIPI 8Lane mode
    {0x3D04, 0xC0},// TXCLKESC_FREQ[15:0]
    {0x3D05, 0x06},//
    {0x3D18, 0x9F},// TCLKPOST[15:0]
    {0x3D1A, 0x57},// TCLKPREPARE[15:0]
    {0x3D1C, 0x57},// TCLKTRAIL[15:0]
    {0x3D1E, 0x87},// TCLKZERO[15:0]
    {0x3D20, 0x5F},// THSPREPARE[15:0]
    {0x3D22, 0xA7},// THSZERO[15:0]
    {0x3D24, 0x5F},// THSTRAIL [15:0]
    {0x3D26, 0x97},// THSEXIT [15:0]
    {0x3D28, 0x4F},// TLPX[15:0]

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
#endif

const static I2C_ARRAY Sensor_8m_30fps_init_table_8lane_linear[] =
{
/*
IMX485LQJ Window cropping 3840x2160 CSI-2_8lane 27MHz AD:12bit Output:12bit 720Mbps Master Mode LCG Mode 30fps
Integration Time 33.244ms
ver7.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5C},// BCWAIT_TIME[9:0]
    {0x300A, 0x42},// CPWAIT_TIME[9:0]
    {0x300B, 0x40},//

    {0x301C, 0x04},// 0: all pixel, 4 window cropping mode
    //{0x301C, 0x00},// 0: all pixel, 4 window cropping mode

    {0x3028, 0x4C},// HMAX[15:0]

    {0x3029, 0x04},//
    {0x303C, 0x0C},// H start position
    {0x303E, 0x00},// H crop width
    {0x303F, 0x0F},// H crop width
    //{0x303E, 0x8C},// H crop width
    //{0x303F, 0x07},// H crop width

    {0x3044, 0x0A},// V start position
    {0x3046, 0x70},// V crop width
    {0x3047, 0x08},// V crop width
    //{0x3046, 0x44},// V crop width
    //{0x3047, 0x04},// V crop width


    {0x30A5, 0x00},// XVS_DRV[1:0]
    {0x3114, 0x02},// INCKSEL1[1:0]
    {0x3119, 0x01},// INCKSEL2[1:0]
    {0x311C, 0xD5},// INCKSEL3[8:0]
    {0x3260, 0x22},
    {0x3262, 0x02},
    {0x3278, 0xA2},
    {0x3324, 0x00},
    {0x3366, 0x31},
    {0x340C, 0x4D},
    {0x3416, 0x10},
    {0x3417, 0x13},
    {0x3432, 0x93},
    {0x34CE, 0x1E},
    {0x34CF, 0x1E},
    {0x34DC, 0x80},
    {0x351C, 0x03},
    {0x359E, 0x70},
    {0x35A2, 0x9C},
    {0x35AC, 0x08},
    {0x35C0, 0xFA},
    {0x35C2, 0x4E},
    {0x3608, 0x41},
    {0x360A, 0x47},
    {0x361E, 0x4A},
    {0x3630, 0x43},
    {0x3632, 0x47},
    {0x363C, 0x41},
    {0x363E, 0x4A},
    {0x3648, 0x41},
    {0x364A, 0x47},
    {0x3660, 0x04},
    {0x3676, 0x3F},
    {0x367A, 0x3F},
    {0x36A4, 0x41},
    {0x3798, 0x82},
    {0x379A, 0x82},
    {0x379C, 0x82},
    {0x379E, 0x82},
    {0x3804, 0x22},// INCKSEL4[1:0]
    {0x3807, 0x84},// INCKSEL5[7:0]
    {0x3888, 0xA8},
    {0x388C, 0xA6},
    {0x3914, 0x15},
    {0x3915, 0x15},
    {0x3916, 0x15},
    {0x3917, 0x14},
    {0x3918, 0x14},
    {0x3919, 0x14},
    {0x391A, 0x13},
    {0x391B, 0x13},
    {0x391C, 0x13},
    {0x391E, 0x00},
    {0x391F, 0xA5},
    {0x3920, 0xED},
    {0x3921, 0x0E},
    {0x39A2, 0x0C},
    {0x39A4, 0x16},
    {0x39A6, 0x2B},
    {0x39A7, 0x01},
    {0x39D2, 0x2D},
    {0x39D3, 0x00},
    {0x39D8, 0x37},
    {0x39D9, 0x00},
    {0x39DA, 0x9B},
    {0x39DB, 0x01},
    {0x39E0, 0x28},
    {0x39E1, 0x00},
    {0x39E2, 0x2C},
    {0x39E3, 0x00},
    {0x39E8, 0x96},
    {0x39EA, 0x9A},
    {0x39EB, 0x01},
    {0x39F2, 0x27},
    {0x39F3, 0x00},
    {0x3A00, 0x38},
    {0x3A01, 0x00},
    {0x3A02, 0x95},
    {0x3A03, 0x01},
    {0x3A18, 0x9B},
    {0x3A2A, 0x0C},
    {0x3A30, 0x15},
    {0x3A32, 0x31},
    {0x3A33, 0x01},
    {0x3A36, 0x4D},
    {0x3A3E, 0x11},
    {0x3A40, 0x31},
    {0x3A42, 0x4C},
    {0x3A43, 0x01},
    {0x3A44, 0x47},
    {0x3A46, 0x4B},
    {0x3A4E, 0x11},
    {0x3A50, 0x32},
    {0x3A52, 0x46},
    {0x3A53, 0x01},
    {0x3D00, 0x11},// non-continues mode , 10h (LP mode disable at V blanking. Default setting), 11h (LP mode enable at V blanking) Reflection timing:Set the register during standby mode and reflected after standby cancel.
    {0x3D01, 0x07},// MIPI 8Lane mode
    {0x3D04, 0xC0},// TXCLKESC_FREQ[15:0]
    {0x3D05, 0x06},//
    {0x3D0C, 0x00},// INCKSEL6
    {0x3D18, 0x6F},// TCLKPOST[15:0]
    {0x3D1A, 0x2F},// TCLKPREPARE[15:0]
    {0x3D1C, 0x2F},// TCLKTRAIL[15:0]
    {0x3D1E, 0xBF},// TCLKZERO[15:0]
    {0x3D1F, 0x00},// TCLKZERO[15:0]
    {0x3D20, 0x2F},// THSPREPARE[15:0]
    {0x3D22, 0x57},// THSZERO[15:0]
    {0x3D24, 0x2F},// THSTRAIL [15:0]
    {0x3D26, 0x4F},// THSEXIT [15:0]
    {0x3D28, 0x27},// TLPX[15:0]

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};


const static I2C_ARRAY Sensor_8m_30fps_init_table_dual_4lane_linear[] =
{
/*
IMX485LQJ Window cropping 3840x2160 CSI-2_4lane x 2ch 27MHz AD:12bit Output:12bit 891Mbps Master Mode LCG Mode 30fps Integration Time 33.244ms
Tool ver : Ver7.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop

    {0x3008, 0x5C},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x300B, 0x40},  //
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3028, 0x4C},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x303C, 0x0C},  // PIX_HST[12:0]
    {0x303E, 0x00},  // PIX_HWIDTH[12:0]
    {0x3044, 0x0A},  // PIX_VST[11:0]
    {0x3046, 0x70},  // PIX_VWIDTH[11:0]
    {0x30A5, 0x00},  // XVS_DRV[1:0]
    {0x3114, 0x02},  // INCKSEL1[1:0]
    {0x3119, 0x01},  // INCKSEL2[7:0]
    {0x311C, 0x08},  // INCKSEL3[8:0]
    {0x311D, 0x01},  //
    {0x3260, 0x22},  // -
    {0x3262, 0x02},  // -
    {0x3278, 0xA2},  // -
    {0x3324, 0x00},  // -
    {0x3366, 0x31},  // -
    {0x340C, 0x4D},  // -
    {0x3416, 0x10},  // -
    {0x3417, 0x13},  // -
    {0x3432, 0x93},  // -
    {0x34CE, 0x1E},  // -
    {0x34CF, 0x1E},  // -
    {0x34DC, 0x80},  // -
    {0x351C, 0x03},  // -
    {0x359E, 0x70},  // -
    {0x35A2, 0x9C},  // -
    {0x35AC, 0x08},  // -
    {0x35C0, 0xFA},  // -
    {0x35C2, 0x4E},  // -
    {0x3608, 0x41},  // -
    {0x360A, 0x47},  // -
    {0x361E, 0x4A},  // -
    {0x3630, 0x43},  // -
    {0x3632, 0x47},  // -
    {0x363C, 0x41},  // -
    {0x363E, 0x4A},  // -
    {0x3648, 0x41},  // -
    {0x364A, 0x47},  // -
    {0x3660, 0x04},  // -
    {0x3676, 0x3F},  // -
    {0x367A, 0x3F},  // -
    {0x36A4, 0x41},  // -
    {0x3798, 0x82},  // -
    {0x379A, 0x82},  // -
    {0x379C, 0x82},  // -
    {0x379E, 0x82},  // -
    {0x3804, 0x22},  // INCKSEL4[1:0]
    {0x3807, 0x84},  // INCKSEL5[7:0]
    {0x3888, 0xA8},  // -
    {0x388C, 0xA6},  // -
    {0x3914, 0x15},  // -
    {0x3915, 0x15},  // -
    {0x3916, 0x15},  // -
    {0x3917, 0x14},  // -
    {0x3918, 0x14},  // -
    {0x3919, 0x14},  // -
    {0x391A, 0x13},  // -
    {0x391B, 0x13},  // -
    {0x391C, 0x13},  // -
    {0x391E, 0x00},  // -
    {0x391F, 0xA5},  // -
    {0x3920, 0xED},  // -
    {0x3921, 0x0E},  // -
    {0x39A2, 0x0C},  // -
    {0x39A4, 0x16},  // -
    {0x39A6, 0x2B},  // -
    {0x39A7, 0x01},  // -
    {0x39D2, 0x2D},  // -
    {0x39D3, 0x00},  // -
    {0x39D8, 0x37},  // -
    {0x39D9, 0x00},  // -
    {0x39DA, 0x9B},  // -
    {0x39DB, 0x01},  // -
    {0x39E0, 0x28},  // -
    {0x39E1, 0x00},  // -
    {0x39E2, 0x2C},  // -
    {0x39E3, 0x00},  // -
    {0x39E8, 0x96},  // -
    {0x39EA, 0x9A},  // -
    {0x39EB, 0x01},  // -
    {0x39F2, 0x27},  // -
    {0x39F3, 0x00},  // -
    {0x3A00, 0x38},  // -
    {0x3A01, 0x00},  // -
    {0x3A02, 0x95},  // -
    {0x3A03, 0x01},  // -
    {0x3A18, 0x9B},  // -
    {0x3A2A, 0x0C},  // -
    {0x3A30, 0x15},  // -
    {0x3A32, 0x31},  // -
    {0x3A33, 0x01},  // -
    {0x3A36, 0x4D},  // -
    {0x3A3E, 0x11},  // -
    {0x3A40, 0x31},  // -
    {0x3A42, 0x4C},  // -
    {0x3A43, 0x01},  // -
    {0x3A44, 0x47},  // -
    {0x3A46, 0x4B},  // -
    {0x3A4E, 0x11},  // -
    {0x3A50, 0x32},  // -
    {0x3A52, 0x46},  // -
    {0x3A53, 0x01},  // -
    {0x3D01, 0x06},  // LANEMODE[2:0]
    {0x3D04, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x3D05, 0x06},  //
    {0x3D0C, 0x00},  // INCKSEL6
    {0x3D18, 0x7F},  // TCLKPOST[15:0]
    {0x3D1A, 0x37},  // TCLKPREPARE[15:0]
    {0x3D1C, 0x37},  // TCLKTRAIL[15:0]
    {0x3D1E, 0xF7},  // TCLKZERO[15:0]
    {0x3D1F, 0x00},  //
    {0x3D20, 0x3F},  // THSPREPARE[15:0]
    {0x3D22, 0x6F},  // THSZERO[15:0]
    {0x3D24, 0x3F},  // THSTRAIL [15:0]
    {0x3D26, 0x5F},  // THSEXIT [15:0]
    {0x3D28, 0x2F},  // TLPX[15:0]

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

//1920x1080@60fps
#if 0
const static I2C_ARRAY Sensor_2m_50fps_init_table_4lane_linear[] =
{

};
#endif

const static I2C_ARRAY Sensor_8m_30fps_init_table_4lane_HDR_DOL[] =
{
/*
IMX485LQJ All-pixel scan CSI-2_4lane 27MHz AD:10bit Output:10bit 1440Mbps Master Mode LCG Mode DOL HDR 2frame VC 30fps
Integration Time LEF:32ms SEF:0.207ms
Tool ver : Ver7.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5C},// BCWAIT_TIME[9:0]
    {0x300A, 0x42},// CPWAIT_TIME[9:0]
    {0x300B, 0x40},//
    {0x302C, 0x01},// WDMODE
    {0x302D, 0x01},// WDSEL[1:0]
    {0x3031, 0x00},// ADBIT
    {0x3032, 0x00},// MDBIT
    {0x3050, 0xB4},// SHR0[19:0]
    {0x3054, 0x0A},// SHR1[19:0]
    {0x3060, 0x26},// RHS1[19:0]
    {0x3061, 0x00},//
    {0x30A5, 0x00},// XVS_DRV[1:0]
    {0x30CF, 0x01},// XVSMSKCNT_INT
    {0x3114, 0x02},// INCKSEL1[1:0]
    {0x311C, 0xD5},// INCKSEL3[8:0]
    {0x3260, 0x22},
    {0x3262, 0x02},
    {0x3278, 0xA2},
    {0x3324, 0x00},
    {0x3366, 0x31},
    {0x340C, 0x4D},
    {0x3416, 0x10},
    {0x3417, 0x13},
    {0x3432, 0x93},
    {0x34CE, 0x1E},
    {0x34CF, 0x1E},
    {0x34DC, 0x80},
    {0x34E1, 0x19},// RAMPSHSWSEL
    {0x351C, 0x03},
    {0x359E, 0x70},
    {0x35A2, 0x9C},
    {0x35AC, 0x08},
    {0x35C0, 0xFA},
    {0x35C2, 0x4E},
    {0x3608, 0x41},
    {0x360A, 0x47},
    {0x361E, 0x4A},
    {0x3630, 0x43},
    {0x3632, 0x47},
    {0x363C, 0x41},
    {0x363E, 0x4A},
    {0x3648, 0x41},
    {0x364A, 0x47},
    {0x3660, 0x04},
    {0x3676, 0x3F},
    {0x367A, 0x3F},
    {0x36A4, 0x41},
    {0x3798, 0x82},
    {0x379A, 0x82},
    {0x379C, 0x82},
    {0x379E, 0x82},
    {0x3804, 0x22},// INCKSEL4[1:0]
    {0x3807, 0x84},// INCKSEL5[7:0]
    {0x3888, 0xA8},
    {0x388C, 0xA6},
    {0x3914, 0x15},
    {0x3915, 0x15},
    {0x3916, 0x15},
    {0x3917, 0x14},
    {0x3918, 0x14},
    {0x3919, 0x14},
    {0x391A, 0x13},
    {0x391B, 0x13},
    {0x391C, 0x13},
    {0x391E, 0x00},
    {0x391F, 0xA5},
    {0x3920, 0xED},
    {0x3921, 0x0E},
    {0x39A2, 0x0C},
    {0x39A4, 0x16},
    {0x39A6, 0x2B},
    {0x39A7, 0x01},
    {0x39D2, 0x2D},
    {0x39D3, 0x00},
    {0x39D8, 0x37},
    {0x39D9, 0x00},
    {0x39DA, 0x9B},
    {0x39DB, 0x01},
    {0x39E0, 0x28},
    {0x39E1, 0x00},
    {0x39E2, 0x2C},
    {0x39E3, 0x00},
    {0x39E8, 0x96},
    {0x39EA, 0x9A},
    {0x39EB, 0x01},
    {0x39F2, 0x27},
    {0x39F3, 0x00},
    {0x3A00, 0x38},
    {0x3A01, 0x00},
    {0x3A02, 0x95},
    {0x3A03, 0x01},
    {0x3A18, 0x9B},
    {0x3A2A, 0x0C},
    {0x3A30, 0x15},
    {0x3A32, 0x31},
    {0x3A33, 0x01},
    {0x3A36, 0x4D},
    {0x3A3E, 0x11},
    {0x3A40, 0x31},
    {0x3A42, 0x4C},
    {0x3A43, 0x01},
    {0x3A44, 0x47},
    {0x3A46, 0x4B},
    {0x3A4E, 0x11},
    {0x3A50, 0x32},
    {0x3A52, 0x46},
    {0x3A53, 0x01},
    {0x3D01, 0x03},// MIPI 4Lane
    {0x3D04, 0xC0},// TXCLKESC_FREQ[15:0]
    {0x3D05, 0x06},//
    {0x3D18, 0x9F},// TCLKPOST[15:0]
    {0x3D1A, 0x57},// TCLKPREPARE[15:0]
    {0x3D1C, 0x57},// TCLKTRAIL[15:0]
    {0x3D1E, 0x87},// TCLKZERO[15:0]
    {0x3D20, 0x5F},// THSPREPARE[15:0]
    {0x3D22, 0xA7},// THSZERO[15:0]
    {0x3D24, 0x5F},// THSTRAIL [15:0]
    {0x3D26, 0x97},// THSEXIT [15:0]
    {0x3D28, 0x4F},// TLPX[15:0]

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_8m_30fps_init_table_8lane_HDR_DOL[] =
{
/*
IMX485LQJ All-pixel scan CSI-2_4lane 27MHz AD:10bit Output:10bit 1440Mbps Master Mode LCG Mode DOL HDR 2frame VC 30fps
Integration Time LEF:32ms SEF:0.207ms
Tool ver : Ver7.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5C},// BCWAIT_TIME[9:0]
    {0x300A, 0x42},// CPWAIT_TIME[9:0]
    {0x300B, 0x40},//
    {0x302C, 0x01},// WDMODE
    {0x302D, 0x01},// WDSEL[1:0]
    {0x3031, 0x00},// ADBIT
    {0x3032, 0x00},// MDBIT
    {0x3050, 0xB4},// SHR0[19:0]
    {0x3054, 0x0A},// SHR1[19:0]
    {0x3060, 0x26},// RHS1[19:0]
    {0x3061, 0x00},//
    {0x30A5, 0x00},// XVS_DRV[1:0]
    {0x30CF, 0x01},// XVSMSKCNT_INT
    {0x3114, 0x02},// INCKSEL1[1:0]
    {0x311C, 0xD5},// INCKSEL3[8:0]
    {0x3260, 0x22},
    {0x3262, 0x02},
    {0x3278, 0xA2},
    {0x3324, 0x00},
    {0x3366, 0x31},
    {0x340C, 0x4D},
    {0x3416, 0x10},
    {0x3417, 0x13},
    {0x3432, 0x93},
    {0x34CE, 0x1E},
    {0x34CF, 0x1E},
    {0x34DC, 0x80},
    {0x34E1, 0x19},// RAMPSHSWSEL
    {0x351C, 0x03},
    {0x359E, 0x70},
    {0x35A2, 0x9C},
    {0x35AC, 0x08},
    {0x35C0, 0xFA},
    {0x35C2, 0x4E},
    {0x3608, 0x41},
    {0x360A, 0x47},
    {0x361E, 0x4A},
    {0x3630, 0x43},
    {0x3632, 0x47},
    {0x363C, 0x41},
    {0x363E, 0x4A},
    {0x3648, 0x41},
    {0x364A, 0x47},
    {0x3660, 0x04},
    {0x3676, 0x3F},
    {0x367A, 0x3F},
    {0x36A4, 0x41},
    {0x3798, 0x82},
    {0x379A, 0x82},
    {0x379C, 0x82},
    {0x379E, 0x82},
    {0x3804, 0x22},// INCKSEL4[1:0]
    {0x3807, 0x84},// INCKSEL5[7:0]
    {0x3888, 0xA8},
    {0x388C, 0xA6},
    {0x3914, 0x15},
    {0x3915, 0x15},
    {0x3916, 0x15},
    {0x3917, 0x14},
    {0x3918, 0x14},
    {0x3919, 0x14},
    {0x391A, 0x13},
    {0x391B, 0x13},
    {0x391C, 0x13},
    {0x391E, 0x00},
    {0x391F, 0xA5},
    {0x3920, 0xED},
    {0x3921, 0x0E},
    {0x39A2, 0x0C},
    {0x39A4, 0x16},
    {0x39A6, 0x2B},
    {0x39A7, 0x01},
    {0x39D2, 0x2D},
    {0x39D3, 0x00},
    {0x39D8, 0x37},
    {0x39D9, 0x00},
    {0x39DA, 0x9B},
    {0x39DB, 0x01},
    {0x39E0, 0x28},
    {0x39E1, 0x00},
    {0x39E2, 0x2C},
    {0x39E3, 0x00},
    {0x39E8, 0x96},
    {0x39EA, 0x9A},
    {0x39EB, 0x01},
    {0x39F2, 0x27},
    {0x39F3, 0x00},
    {0x3A00, 0x38},
    {0x3A01, 0x00},
    {0x3A02, 0x95},
    {0x3A03, 0x01},
    {0x3A18, 0x9B},
    {0x3A2A, 0x0C},
    {0x3A30, 0x15},
    {0x3A32, 0x31},
    {0x3A33, 0x01},
    {0x3A36, 0x4D},
    {0x3A3E, 0x11},
    {0x3A40, 0x31},
    {0x3A42, 0x4C},
    {0x3A43, 0x01},
    {0x3A44, 0x47},
    {0x3A46, 0x4B},
    {0x3A4E, 0x11},
    {0x3A50, 0x32},
    {0x3A52, 0x46},
    {0x3A53, 0x01},
    {0x3D01, 0x07},// MIPI 8Lane
    {0x3D04, 0xC0},// TXCLKESC_FREQ[15:0]
    {0x3D05, 0x06},//
    {0x3D18, 0x9F},// TCLKPOST[15:0]
    {0x3D1A, 0x57},// TCLKPREPARE[15:0]
    {0x3D1C, 0x57},// TCLKTRAIL[15:0]
    {0x3D1E, 0x87},// TCLKZERO[15:0]
    {0x3D20, 0x5F},// THSPREPARE[15:0]
    {0x3D22, 0xA7},// THSZERO[15:0]
    {0x3D24, 0x5F},// THSTRAIL [15:0]
    {0x3D26, 0x97},// THSEXIT [15:0]
    {0x3D28, 0x4F},// TLPX[15:0]

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

#if 0
const static I2C_ARRAY Sensor_2m_30fps_init_table_4lane_HDR_DOL[] =
{

};
#endif

const static I2C_ARRAY Sensor_id_table[] = {
    {0x3F12, 0x14},      // {address of ID, ID },
    {0x3F13, 0x75},      // {address of ID, ID },
};

static I2C_ARRAY PatternTbl[] = {
    {0x0000,0x00},       // colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY vts_reg[] = {       //VMAX
    {0x3026, 0x00}, //bit0-3-->MSB
    {0x3025, 0x08},
    {0x3024, 0xCA},
};

const static I2C_ARRAY expo_SHR0_reg[] = { // SHS0 (For LEF)
    {0x3052, 0x00},
    {0x3051, 0x16},
    {0x3050, 0x22},
};

const static I2C_ARRAY expo_SHR1_reg[] = { // SHS1 (For SEF)
    //decreasing exposure ratio version.
    {0x3056, 0x00},
    {0x3055, 0x00},
    {0x3054, 0x09},
};

const I2C_ARRAY expo_RHS1_reg[] = {
    //decreasing exposure ratio version.
    {0x3062, 0x00},
    {0x3061, 0x00},
    {0x3060, 0x11},
};

const static I2C_ARRAY gain_reg[] = {
    {0x3084, 0x2A},//low bit
    {0x3085, 0x00},//hcg mode,bit 4
};

const static I2C_ARRAY gain_HDR_DOL_LEF_reg[] = {
    {0x3084, 0x2A},
    {0x3085, 0x00},
};

const static I2C_ARRAY gain_HDR_DOL_SEF_reg[] = {
    {0x3086, 0x20},
    {0x3087, 0x00},
};

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


/////////////////// I2C function definition ///////////////////
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent ///////////////////
#if 0

static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

/*******Support MCLK List*******
 *    CUS_CMU_CLK_27MHZ,
 *    CUS_CMU_CLK_21P6MHZ,
 *    CUS_CMU_CLK_12MHZ,
 *    CUS_CMU_CLK_5P4MHZ,
 *    CUS_CMU_CLK_36MHZ,
 *    CUS_CMU_CLK_54MHZ,
 *    CUS_CMU_CLK_43P2MHZ,
 *    CUS_CMU_CLK_61P7MHZ,
 *    CUS_CMU_CLK_72MHZ,
 *    CUS_CMU_CLK_48MHZ,
 *    CUS_CMU_CLK_24MHZ,
 *    CUS_CMU_CLK_37P125MHZ,
 ******End of Support MCLK List*******/

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
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 mipi_lane_num;

    /*If 8 lane is enabled, change to 4+4 lane*/
    if(params->dual_4lane_mode)
        mipi_lane_num = 4;
    else
        mipi_lane_num = handle->interface_attr.attr_mipi.mipi_lane_num;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, mipi_lane_num, ENABLE);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    SENSOR_DMSG("IMX485 mipi_lane_num=%d, dual 4 lane mode is %d \n", handle->interface_attr.attr_mipi.mipi_lane_num, params->dual_4lane_mode);
    if(params->dual_4lane_mode)
    {
        sensor_if->SetIOPad(idx+1, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
        sensor_if->SetCSI_Clk(idx+1, CUS_CSI_CLK_216M);
        sensor_if->SetCSI_Lane(idx+1, mipi_lane_num, ENABLE);
        sensor_if->SetCSI_LongPacketType(idx+1, 0, 0x1C00, 0);
    }

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }
}
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    imx485_params *params = (imx485_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode==CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period * 2;
        info->u32AEShutter_step                  = Preview_line_period * 2;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE* 4;
        info->u32AEShutter_min                   = (Preview_line_period_HDR_DOL_4LANE * 4);
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_max                   = Preview_line_period_HDR_DOL_4LANE * params->min_rhs1;
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
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    //Sensor power on sequence
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);   // Powerdn Pull Low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);     // Rst Pull Low

    //Configuration Chip RX
    pCus_PowerOn_InitChipRX(handle, idx);

    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    //Sensor board PWDN Enable, 1.8V & 2.9V need 30ms then Pull High
    SENSOR_MSLEEP(31);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    //SENSOR_UDELAY(1);
    SENSOR_MSLEEP(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_DMSG("Sensor Power On finished\n");
    SENSOR_MSLEEP(5);
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx485_params *params = (imx485_params *)handle->private_data;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);   // Powerdn Pull Low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);     // Rst Pull Low
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }

    if(params->dual_4lane_mode)
    {
        sensor_if->SetCSI_Clk(idx+1, CUS_CSI_CLK_DISABLE);
        if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
            sensor_if->SetCSI_hdr_mode(idx+1, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
        }
    }

    params->res.orit = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// Check Sensor Product ID /////////////////////////

static int pCus_CheckSensorProductID(ss_cus_sensor *handle)
{
    u16 sen_id_msb, sen_id_lsb;

    /* Read Product ID */
    SensorReg_Read(0x3f12, (void*)&sen_id_lsb);
    SensorReg_Read(0x3f13, (void*)&sen_id_msb);//CHIP_ID_r3F13
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
#if 0
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
    if(table_length>8)
        table_length=8;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(n=0;n<4;++n) //retry , until I2C success
    {
      if(n>3)
          return FAIL;

      if(SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
          break;
      else
          SENSOR_MSLEEP(1);
    }

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
       if( id_from_sensor[i].data != Sensor_id_table[i].data ){
            SENSOR_DMSG("[%s] Please Check IMX485 Sensor Insert!!\n", __FUNCTION__);
            return FAIL;
       }
       *id = id_from_sensor[i].data;
    }

    SENSOR_DMSG("[%s]IMX485 sensor ,Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}
#endif
static int imx485_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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
            SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }
    return SUCCESS;
}

static int pCus_init_8m_30fps_mipi_linear(ss_cus_sensor *handle)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;
    int res_idx = handle->video_res_supported.ulcur_res;
    const I2C_ARRAY *pTable = NULL;
    unsigned int uTableSize = 0;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    switch(res_idx)
    {
    case LINEAR_RES_1:
        if(lane_num == 8)
        {
            if(params->dual_4lane_mode)
            {
                /*initialize dual 4 lane mode*/
                pTable = Sensor_8m_30fps_init_table_dual_4lane_linear;
                uTableSize = ARRAY_SIZE(Sensor_8m_30fps_init_table_dual_4lane_linear);
            }
            else
            {
                /*initialize 8 lane mode*/
                pTable = Sensor_8m_30fps_init_table_8lane_linear;
                uTableSize = ARRAY_SIZE(Sensor_8m_30fps_init_table_8lane_linear);
            }
        }
        else
        {
            pTable = Sensor_8m_30fps_init_table_4lane_linear;
            uTableSize = ARRAY_SIZE(Sensor_8m_30fps_init_table_4lane_linear);
        }
        break;
    default:
        break;
    }

    for(i=0;i< uTableSize;i++)
    {
        if(pTable[i].reg==0xffff)
        {
            SENSOR_MSLEEP(pTable[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(pTable[i].reg, pTable[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_8m_30fps_mipi_HDR_DOL(ss_cus_sensor *handle)
{
    int i,cnt=0;
    int res_idx = handle->video_res_supported.ulcur_res;
    const I2C_ARRAY *pTable = NULL;
    unsigned int uTableSize = 0;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    switch(res_idx)
    {
    case LINEAR_RES_1:
        if(hdr_lane_num == 8)
        {
            pTable = Sensor_8m_30fps_init_table_8lane_HDR_DOL;
            uTableSize = ARRAY_SIZE(Sensor_8m_30fps_init_table_8lane_HDR_DOL);
        }
        else
        {
            pTable = Sensor_8m_30fps_init_table_4lane_HDR_DOL;
            uTableSize = ARRAY_SIZE(Sensor_8m_30fps_init_table_4lane_HDR_DOL);
        }
        break;
    default:
        break;
    }

    for(i=0;i< uTableSize;i++)
    {
        if(pTable[i].reg==0xffff)
        {
            SENSOR_MSLEEP(pTable[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(pTable[i].reg,pTable[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
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
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
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
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case 0: /*3840x2160 12bits , 4/8/4+4 lane*/
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_8m_30fps_mipi_linear;
            vts_30fps = 2250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 14814;
            break;
        case 1: /*crop 1920x1080*/
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_8m_30fps_mipi_linear;
            vts_30fps = 2250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 14814;
            break;
        default:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_8m_30fps_mipi_linear;
            vts_30fps = 2250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 14814;
            break;
    }

    /*workaround: use 4+4 lane mode to instead of 8 lane mode*/
    if(handle->interface_attr.attr_mipi.mipi_lane_num==8)
        params->dual_4lane_mode = 1;
    else
        params->dual_4lane_mode = 0;

    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_LEF(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_SEF(ss_cus_sensor *handle, u32 res_idx)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    switch (res_idx) {
        case HDR_RES_1:
            handle->pCus_sensor_init = pCus_init_8m_30fps_mipi_HDR_DOL;
            vts_30fps_HDR_DOL_4lane = 2250;
            params->expo.vts = vts_30fps_HDR_DOL_4lane;
            params->expo.fps = 30;
            Preview_line_period_HDR_DOL_4LANE = 7407;
            params->min_rhs1 = 430; //4n+2
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    s16 sen_data;

    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);

    switch(sen_data & SENSOR_MIRROR_FLIP_EN)
    {
        case SENSOR_NOR:
            *orit = CUS_ORIT_M0F0;
            break;
        case SENSOR_FLIP_EN:
            *orit = CUS_ORIT_M0F1;
            break;
        case SENSOR_MIRROR_EN:
            *orit = CUS_ORIT_M1F0;
            break;
        case SENSOR_MIRROR_FLIP_EN:
            *orit = CUS_ORIT_M1F1;
            break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx485_params *params = (imx485_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    s16 sen_data;
    //Read SENSOR MIRROR-FLIP STATUS
    imx485_params *params = (imx485_params *)handle->private_data;
    SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);
    sen_data &= ~(SENSOR_MIRROR_FLIP_EN);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            //sen_data |= SENSOR_NOR;
            params->res.orit = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M1F0:
            sen_data |= SENSOR_MIRROR_EN;
            params->res.orit = CUS_ORIT_M1F0;
            break;
        case CUS_ORIT_M0F1:
            sen_data |= SENSOR_FLIP_EN;
            params->res.orit = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            sen_data |= SENSOR_MIRROR_FLIP_EN;
            params->res.orit = CUS_ORIT_M1F1;
            break;
        default :
            params->res.orit = CUS_ORIT_M0F0;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(MIRROR_FLIP, sen_data);

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (vts_30fps*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts=  (vts_30fps*(max_fps*1000) + (fps>>1))/fps;
    }else{
      SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }

    params->expo.vts = (params->expo.vts>>1)<<1; //set vts multiple of 2
    params->expo.fps = fps;
    params->dirty = true;
    pCus_SetAEUSecs(handle, params->expo.expo_lef_us);
    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_SEF(ss_cus_sensor *handle)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_SEF(ss_cus_sensor *handle, u32 fps)
{
    u32 cur_vts_30fps = 0;
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    cur_vts_30fps=vts_30fps_HDR_DOL_4lane;
    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts= (cur_vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts= (cur_vts_30fps*(max_fps*1000) + (fps >> 1))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.vts = (params->expo.vts>>1)<<1; //set vts multiple of 2
    params->expo.fps = fps;
    params->dirty = true;
    pCus_SetAEUSecsHDR_DOL_SEF(handle, params->expo.expo_sef_us);
    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_LEF(ss_cus_sensor *handle)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_LEF(ss_cus_sensor *handle, u32 fps)
{
    u32 cur_vts_30fps = 0;//vts = 0,
    imx485_params *params = (imx485_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    cur_vts_30fps=vts_30fps_HDR_DOL_4lane;
    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (cur_vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts=  (cur_vts_30fps*(max_fps*1000) + (fps >> 1))/fps;
    }else {
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.vts = (params->expo.vts>>1)<<1; //set vts multiple of 2
    params->expo.fps = fps;
    pCus_SetAEUSecsHDR_DOL_LEF(handle, params->expo.expo_sef_us);
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
    imx485_params *params = (imx485_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1); // Global hold on
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_SHR0_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                    params->dirty = false;
                }

                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x3001,0); // Global hold off
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    //imx485_params *params = (imx485_params *)handle->private_data;

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

static int pCus_AEStatusNotifyHDR_DOL_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx485_params *params = (imx485_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1);
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tSHR1_reg, ARRAY_SIZE(expo_SHR1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tRHS1_reg, ARRAY_SIZE(expo_RHS1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    handle->sensor_if_api->SetSkipFrame(idx, params->expo.fps, 1);
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x3001,0);
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
    imx485_params *params = (imx485_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data & 0x03) << 16;
    lines |= (u32)(params->tExpo_reg[1].data & 0xff) << 8;
    lines |= (u32)(params->tExpo_reg[2].data & 0xff) << 0;

    *us = (lines * Preview_line_period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);
    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines = 0, vts = 0, SHR0 = 0;
    imx485_params *params = (imx485_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000*us)/Preview_line_period;
    expo_lines = (expo_lines>>1)<<1;

    /* SHR0: 6 to vts -2, multiple of 2 */
    if (expo_lines <= 2) expo_lines = 2;
    if (expo_lines > params->expo.vts - 6) {
        vts = expo_lines + 6;
    }
    else
        vts = params->expo.vts;
    SHR0 =  vts - expo_lines;

    params->expo.expo_lines = expo_lines;

    SENSOR_DMSG("[%s] us %u, SHR0 %u, vts %u\n", __FUNCTION__,
                                                 us,  \
                                                 SHR0, \
                                                 vts
               );
    params->tExpo_reg[0].data = (SHR0>>16) & 0x0003;
    params->tExpo_reg[1].data = (SHR0>>8) & 0x00ff;
    params->tExpo_reg[2].data = (SHR0>>0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_sef = 0;
    u32 cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    imx485_params *params = (imx485_params *)handle->private_data;

    params->expo.expo_sef_us = us;
    cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    expo_line_sef = (expo_line_sef >> 2)<<2;

    if(expo_line_sef < 4) expo_line_sef = 4;
    params->min_shr1 = params->min_rhs1 - expo_line_sef;
    if((expo_line_sef > params->min_rhs1) || ((params->min_shr1) < 10))
        params->min_shr1 = 10;

    SENSOR_DMSG("[%s] us %u, expo_line_sef %u rhs %u shr1 %u\n", __FUNCTION__,
                                                                 us, \
                                                                 expo_line_sef, \
                                                                 params->min_rhs1, \
                                                                 params->min_shr1
               );

    params->tRHS1_reg[0].data = (params->min_rhs1 >>16) & 0x03;
    params->tRHS1_reg[1].data = (params->min_rhs1 >>8) & 0xff;
    params->tRHS1_reg[2].data = (params->min_rhs1 >>0) & 0xff;

    params->tSHR1_reg[0].data = (params->min_shr1 >> 16) & 0x0003;
    params->tSHR1_reg[1].data = (params->min_shr1 >> 8) & 0x00ff;
    params->tSHR1_reg[2].data = (params->min_shr1 >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_lef = 0, fsc = 0;
    u32 cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    imx485_params *params = (imx485_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    expo_line_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    expo_line_lef = (expo_line_lef >> 2)<<2; //shr0: 4n

    if(expo_line_lef < 4) expo_line_lef = 4;
    fsc = params->expo.vts * 2;   //fsc: 4n
    params->max_shr0 = fsc - expo_line_lef;

    if(params->max_shr0 < (params->min_rhs1+10))
        params->max_shr0 = params->min_rhs1+10;

    params->expo.expo_lines = expo_line_lef;
    SENSOR_DMSG("[%s] us %u, expo_lines_lef %u, vts %u, SHR0 %u \n", __FUNCTION__,
                                                                     us, \
                                                                     expo_lines_lef, \
                                                                     vts, \
                                                                     params->max_shr0
                );
    params->tSHR0_reg[0].data = (params->max_shr0 >> 16) & 0x0003;
    params->tSHR0_reg[1].data = (params->max_shr0 >> 8) & 0x00ff;
    params->tSHR0_reg[2].data = (params->max_shr0 >> 0) & 0x00ff;

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx485_params *params = (imx485_params *)handle->private_data;

    *gain = params->expo.final_gain;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u64 gain_double;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    gain_double = (u16)((gain_double * 10) >> 24) / 3;

    params->tGain_reg[0].data = gain_double & 0xff;
    params->tGain_reg[1].data = (gain_double >> 8) & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data, gain,params->tGain_reg[1].dat);
    params->dirty = true;
    return SUCCESS;
}

static void pCus_SetAEGainHDR_DOL_Calculate(u32 gain, u16 *gain_reg)
{
    //double gain_double;
    u64 gain_double;

    if(gain < SENSOR_MIN_GAIN){
      gain = SENSOR_MIN_GAIN;
    }
    else if(gain >= SENSOR_MAX_GAIN){
      gain = SENSOR_MAX_GAIN;
    }
    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    *gain_reg =(u16)((gain_double * 10) >> 24) / 3;
}

static int pCus_SetAEGainHDR_DOL_SEF1(ss_cus_sensor *handle, u32 gain)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u16 gain_reg = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_sef_reg[0].data = gain_reg & 0xff;
    params->tGain_hdr_dol_sef_reg[1].data = (gain_reg >> 8) & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data, params->tGain_hdr_dol_sef_reg[1].data);
    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    u16 gain_reg = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_lef_reg[0].data = gain_reg & 0xff;
    params->tGain_hdr_dol_lef_reg[1].data = (gain_reg >> 8) & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data, params->tGain_hdr_dol_lef_reg[1].data);
    params->dirty = true;
    return SUCCESS;
}

//lef functions
static int pCus_init_HDR_DOL_LEF(ss_cus_sensor *handle)
{
    return SUCCESS;
}

static int pCus_poweron_HDR_DOL_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_HDR_DOL_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_GetOrien_HDR_DOL_LEF(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    return SUCCESS;
}

static int pCus_SetOrien_HDR_DOL_LEF(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx485_params *params = (imx485_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *us)
{
    *us = 0;
    return SUCCESS;
}

static int pCus_GetAEGain_HDR_DOL_LEF(ss_cus_sensor *handle, u32* gain)
{
    *gain = 0;
    return SUCCESS;
}
#if 0
static int pCus_GetSensorID_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *id)
{
     return SUCCESS;
}

static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/imx485_mipi_linear[0].senout.min_fps;
    return SUCCESS;
}

static int pCus_GetAEMinMaxUSecs_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/imx485_mipi_hdr[0].senout.min_fps;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = SENSOR_MIN_GAIN;//handle->sat_mingain;
    *max = SENSOR_MAX_GAIN;//10^(72db/20)*1024;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = handle->sat_mingain;
    *max = SENSOR_MAX_GAIN;
    return SUCCESS;
}

static int IMX485_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/imx485_mipi_linear[0].senout.min_fps;
    info->min = (Preview_line_period * 2);
    info->step = Preview_line_period *2;
    return SUCCESS;
}

static int IMX485_GetShutterInfo_HDR_DOL_SEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    imx485_params *params = (imx485_params *)handle->private_data;
    info->max = Preview_line_period_HDR_DOL_4LANE * params->min_rhs1;
    info->min = (Preview_line_period_HDR_DOL_4LANE * 4);
    info->step = Preview_line_period_HDR_DOL_4LANE *4;
    return SUCCESS;
}

static int IMX485_GetShutterInfo_HDR_DOL_LEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/imx485_mipi_hdr[0].senout.min_fps;
    info->min = (Preview_line_period_HDR_DOL_4LANE * 4);
    info->step = Preview_line_period_HDR_DOL_4LANE *4;
    return SUCCESS;
}
#endif
int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx485_params *params;
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

    params = (imx485_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"IMX485_MIPI_Linear");

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
   // handle->mclk                  = UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    handle->mclk                    =Preview_MCLK_SPEED;

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

    handle->interface_attr.attr_mipi.mipi_lane_num    = (lane_num==8)?8:SENSOR_MIPI_LANE_NUM;
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
        handle->video_res_supported.res[res].u16width         = imx485_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx485_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx485_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx485_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx485_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx485_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx485_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx485_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx485_mipi_linear[res].senstr.strResDesc);
    }
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx485_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period * 2;

    ////////////////////////////////////
    //    Sensor polarity             //
    ////////////////////////////////////
    //handle->pwdn_POLARITY              = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY             = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY             = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY             = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY              = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    //handle->pCus_sensor_release        = cus_camsensor_release_handle;
    handle->pCus_sensor_init           = pCus_init_8m_30fps_mipi_linear;
    handle->pCus_sensor_poweron        = pCus_poweron;
    handle->pCus_sensor_poweroff       = pCus_poweroff;
    //handle->pCus_sensor_GetSensorID    = pCus_GetSensorID;
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes    = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien       = pCus_GetOrien;
    handle->pCus_sensor_SetOrien       = pCus_SetOrien;
    handle->pCus_sensor_GetFPS         = pCus_GetFPS;
    handle->pCus_sensor_SetFPS         = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = imx485_SetPatternMode; //NONE

    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    //handle->ae_gain_delay              = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay           = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    //handle->ae_gain_ctrl_num           = 1;
    //handle->ae_shutter_ctrl_num        = 1;
    //handle->sat_mingain                = SENSOR_MIN_GAIN;//g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    //handle->pCus_sensor_GetShutterInfo  = IMX485_GetShutterInfo;

    params->expo.vts = vts_30fps;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx485_params *params = NULL;
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

    params = (imx485_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tRHS1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tSHR1_reg, expo_SHR1_reg, sizeof(expo_SHR1_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"IMX485_MIPI_HDR_SEF");

    ////////////////////////////////////
    //    i2c config                  //
    ////////////////////////////////////
    handle->i2c_cfg.mode          = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt           = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address       = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed         = SENSOR_I2C_SPEED;     //320000;

    ////////////////////////////////////
    //    mclk                        //
    ////////////////////////////////////
    handle->mclk                  = Preview_MCLK_SPEED_HDR_DOL;

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //handle->isp_type              = SENSOR_ISP_TYPE;
    //handle->data_fmt              = SENSOR_DATAFMT;
    handle->sif_bus               = SENSOR_IFBUS_TYPE;
    handle->data_prec             = SENSOR_DATAPREC_DOL;
    //handle->data_mode             = SENSOR_DATAMODE;
    handle->bayer_id              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num    = hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    //handle->interface_attr.attr_mipi.mipi_hsync_mode              = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = SENSOR_HDR_MODE;  // SONY IMX485 as VC mode
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx485_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx485_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx485_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx485_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx485_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx485_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx485_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx485_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx485_mipi_hdr[res].senstr.strResDesc);
    }
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period_HDR_DOL_4LANE * 4);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR_DOL_4LANE * params->min_rhs1;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE *4;

    ////////////////////////////////////
    //    Sensor polarity             //
    ////////////////////////////////////
    //handle->pwdn_POLARITY              = SENSOR_PWDN_POL;      //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY             = SENSOR_RST_POL;       //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY             = SENSOR_VSYNC_POL;     //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY             = SENSOR_HSYNC_POL;     //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY              = SENSOR_PCLK_POL;      //CUS_CLK_POL_POS);  // use '!' to clear board latch error

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    //handle->pCus_sensor_release        = cus_camsensor_release_handle;
    handle->pCus_sensor_init           = pCus_init_8m_30fps_mipi_HDR_DOL;
    handle->pCus_sensor_poweron        = pCus_poweron;               // Need to check
    handle->pCus_sensor_poweroff       = pCus_poweroff;              // Need to check
    //handle->pCus_sensor_GetSensorID    = pCus_GetSensorID;           // Need to check
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes_HDR_DOL_SEF;   // Need to check
    handle->pCus_sensor_GetVideoRes    = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes = pCus_GetCurVideoRes;

    handle->pCus_sensor_GetOrien       = pCus_GetOrien;              // Need to check
    handle->pCus_sensor_SetOrien       = pCus_SetOrien;              // Need to check
    handle->pCus_sensor_GetFPS         = pCus_GetFPS_HDR_DOL_SEF;    // Need to check
    handle->pCus_sensor_SetFPS         = pCus_SetFPS_HDR_DOL_SEF;    // Need to check

    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    //handle->ae_gain_delay              = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay           = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    //handle->ae_gain_ctrl_num           = 2;
    //handle->ae_shutter_ctrl_num        = 2;
    //handle->sat_mingain                = SENSOR_MIN_GAIN;      //g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_SEF;  // Need to check
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_SEF;      // Need to check
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_SEF1;      // Need to check
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;

    //handle->pCus_sensor_GetShutterInfo  = IMX485_GetShutterInfo_HDR_DOL_SEF;
    params->expo.vts = vts_30fps_HDR_DOL_4lane;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx485_params *params;
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
    params = (imx485_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"IMX485_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    i2c config                  //
    ////////////////////////////////////
    handle->i2c_cfg.mode          = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt           = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address       = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed         = SENSOR_I2C_SPEED;     //320000;

    ////////////////////////////////////
    //    mclk                        //
    ////////////////////////////////////
   // handle->mclk                   = UseParaMclk(SENSOR_DRV_PARAM_MCLK());
      handle->mclk                   =Preview_MCLK_SPEED_HDR_DOL;
    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //handle->isp_type             = SENSOR_ISP_TYPE;    //ISP_SOC;
    //handle->data_fmt             = SENSOR_DATAFMT;     //CUS_DATAFMT_YUV;
    handle->sif_bus                = SENSOR_IFBUS_TYPE;  //CUS_SENIF_BUS_PARL;
    handle->data_prec              = SENSOR_DATAPREC_DOL;  //CUS_DATAPRECISION_8;
    //handle->data_mode              = SENSOR_DATAMODE;
    handle->bayer_id               = SENSOR_BAYERID_HDR_DOL;   //CUS_BAYER_GB;
    handle->RGBIR_id               = SENSOR_RGBIRID;
    params->res.orit                 = SENSOR_ORIT;        //CUS_ORIT_M1F1;
    handle->interface_attr.attr_mipi.mipi_lane_num                = 4;//hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode              = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = SENSOR_HDR_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx485_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx485_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx485_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx485_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx485_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx485_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx485_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx485_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx485_mipi_hdr[res].senstr.strResDesc);
    }
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period_HDR_DOL_4LANE * 4);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx485_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE *4;

    ////////////////////////////////////
    //    Sensor polarity             //
    ////////////////////////////////////
    //handle->pwdn_POLARITY              = SENSOR_PWDN_POL;      //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY             = SENSOR_RST_POL;       //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY             = SENSOR_VSYNC_POL;     //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY             = SENSOR_HSYNC_POL;     //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY              = SENSOR_PCLK_POL;      //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    //handle->pCus_sensor_release         = cus_camsensor_release_handle;
    handle->pCus_sensor_init            = pCus_init_HDR_DOL_LEF;
    //handle->pCus_sensor_powerupseq      = pCus_powerupseq   ;
    handle->pCus_sensor_poweron         = pCus_poweron_HDR_DOL_LEF;
    handle->pCus_sensor_poweroff        = pCus_poweroff_HDR_DOL_LEF;
    //handle->pCus_sensor_GetSensorID     = pCus_GetSensorID_HDR_DOL_LEF;
    //handle->pCus_sensor_GetVideoResNum  = NULL;
    //handle->pCus_sensor_GetVideoRes     = NULL;
    //handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes_HDR_DOL_LEF;   // Need to check


    handle->pCus_sensor_GetOrien        = pCus_GetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_DOL_LEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_DOL_LEF;

    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    //handle->ae_gain_delay               = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay            = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    //handle->ae_gain_ctrl_num            = 2;
    //handle->ae_shutter_ctrl_num         = 2;
    //handle->sat_mingain                 = SENSOR_MIN_GAIN;
    //handle->dgain_remainder = 0;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_LEF;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain_HDR_DOL_LEF;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs_HDR_DOL_LEF;

    //handle->pCus_sensor_GetShutterInfo  = IMX485_GetShutterInfo_HDR_DOL_LEF;

    params->expo.vts = vts_30fps_HDR_DOL_4lane;

    params->dirty = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(IMX485_HDR,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            imx485_params
                         );
