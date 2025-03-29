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
   Date          : 2023/09/14
   Build on      : Master_V4 I6C
   Verified on   : mixer preview ok (linear/hdr)
               AE gain/shutter ok , IQ not verify, hdr ae hard to converge. hdr colors are darker.
   Remark        : NA
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX675_HDR);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

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

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_12
#define SENSOR_DATAPREC_DOL         CUS_DATAPRECISION_12
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9098  //CFG
#define SENSOR_BAYERID              CUS_BAYER_RG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_RG
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_72MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_72MHZ
#define Preview_MCLK_SPEED_CLR_HDR  CUS_CMU_CLK_37P125MHZ
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
#define SENSOR_NAME     IMX675

#define CHIP_ID_r3F12   0x3F12
#define CHIP_ID_r3F13   0x3F13
#define CHIP_ID         0x0485

extern u64 base2_exp_float_pow(u64 x);

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_END}mode;
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
}IMX675_mipi_linear[] = {
    {LINEAR_RES_1, {2592, 1944, 3, 30}, {0, 0, 2592, 1944}, {"2592x1944@30fps"}},
    {LINEAR_RES_2, {2592, 1440, 3, 60}, {0, 0, 2592, 1440}, {"2592x1440@60fps"}},
};

static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_2 = 1, HDR_RES_3, HDR_RES_END}mode;
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
}IMX675_mipi_hdr[] = {
    {HDR_RES_1, {2592, 1944, 3, 30}, {0, 0, 2592, 1944}, {"2592x1944@30fps_HDR"}}, // Modify it
    {HDR_RES_2, {2592, 1944, 3, 30}, {0, 0, 2592, 1944}, {"2592x1944@30fps_CLR_HDR"}}, // Modify it
    {HDR_RES_3, {2592, 1440, 3, 30}, {0, 0, 2592, 1440}, {"2592x1440@30fps_HDR"}},
};

//u32 vts_30fps = 3667;
//u32 vts_30fps_HDR_DOL_4lane = 2250;
//u32 vts_30fps_HDR_DOL_2lane = 1672;
//u32 Preview_line_period = 9090;
//u32 Preview_line_period_HDR_DOL_4LANE = 8106;
//u32 Preview_line_period_HDR_DOL_2LANE = 8106;

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (3981 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_CLR_HDR_HG_MAX_GAIN                  (513216) //54dB
#define SENSOR_CLR_HDR_HG_MIN_GAIN                  (3430) //9dB ==>use10.5dB tranfer formula difference
#define SENSOR_CLR_HDR_LG_MAX_GAIN                  (16229) //24dB
#define SENSOR_CLR_HDR_LG_MIN_GAIN                  (1024) //0dB
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_CLR_HDR       (3)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR_FLIP                                 0x3030
#define SENSOR_NOR                                  0x0
#define SENSOR_MIRROR_EN                            0x1 << (0)
#define SENSOR_FLIP_EN                              0x1 << (1)
#define SENSOR_MIRROR_FLIP_EN                       0x3

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
    u32 Init_Vts;
    u32 Line_Period;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tVts_reg_hdr[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tSHR0_reg[3];
    I2C_ARRAY tSHR1_reg[3];
    I2C_ARRAY tRHS1_reg[3];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tGain_hdr_dol_lef_reg[2];
    I2C_ARRAY tGain_hdr_dol_sef_reg[2];
    I2C_ARRAY tGain_clr_hdr_AGH_reg[2];
    I2C_ARRAY tGain_clr_hdr_AGL_reg[2];
    I2C_ARRAY tGain_clr_hdr_DG_reg[2];
    I2C_ARRAY tGain_CLR_HDR_HG_reg[2];
    I2C_ARRAY tGain_CLR_HDR_LG_reg[2];
    bool dirty;
    bool orien_dirty;
    bool Clrhdr_mode_en;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
} IMX675_params;

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

const static I2C_ARRAY Sensor_6m_30fps_init_table_4lane_linear[] =
{
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop

    {0x3014, 0x02},
    {0x3022, 0x01},
    {0x3028, 0x53},
    {0x3029, 0x0E},
    {0x302C, 0xA3},
    {0x302D, 0x02},
    {0x3050, 0x04},
    {0x30A6, 0x00},
    {0x30CE, 0x02},
    {0x3148, 0x00},
    {0x3460, 0x22},
    {0x3492, 0x08},
    {0x3B1D, 0x17},
    {0x3B44, 0x3F},
    {0x3B60, 0x03},
    {0x3C03, 0x04},
    {0x3C04, 0x04},
    {0x3C0B, 0x00},
    {0x3C0C, 0x00},
    {0x3C0D, 0x00},
    {0x3C0E, 0x00},
    {0x3C0F, 0x00},
    {0x3C30, 0x73},
    {0x3C3C, 0x20},
    {0x3C7C, 0xB9},
    {0x3C7D, 0x01},
    {0x3C7E, 0xB7},
    {0x3C7F, 0x01},
    {0x3CB0, 0x00},
    {0x3CB2, 0xFF},
    {0x3CB3, 0x03},
    {0x3CB4, 0xFF},
    {0x3CB5, 0x03},
    {0x3CBA, 0xFF},
    {0x3CBB, 0x03},
    {0x3CC0, 0xFF},
    {0x3CC1, 0x03},
    {0x3CC2, 0x00},
    {0x3CC6, 0xFF},
    {0x3CC7, 0x03},
    {0x3CC8, 0xFF},
    {0x3CC9, 0x03},
    {0x3E00, 0x1E},
    {0x3E02, 0x04},
    {0x3E03, 0x00},
    {0x3E20, 0x04},
    {0x3E21, 0x00},
    {0x3E22, 0x1E},
    {0x3E24, 0xBA},
    {0x3E72, 0x85},
    {0x3E76, 0x0C},
    {0x3E77, 0x01},
    {0x3E7A, 0x85},
    {0x3E7E, 0x1F},
    {0x3E82, 0xA6},
    {0x3E86, 0x2D},
    {0x3EE2, 0x33},
    {0x3EE3, 0x03},
    {0x4490, 0x07},
    {0x4494, 0x19},
    {0x4495, 0x00},
    {0x4496, 0xBB},
    {0x4497, 0x00},
    {0x4498, 0x55},
    {0x449A, 0x50},
    {0x449C, 0x50},
    {0x449E, 0x50},
    {0x44A0, 0x3C},
    {0x44A2, 0x19},
    {0x44A4, 0x19},
    {0x44A6, 0x19},
    {0x44A8, 0x4B},
    {0x44AA, 0x4B},
    {0x44AC, 0x4B},
    {0x44AE, 0x4B},
    {0x44B0, 0x3C},
    {0x44B2, 0x19},
    {0x44B4, 0x19},
    {0x44B6, 0x19},
    {0x44B8, 0x4B},
    {0x44BA, 0x4B},
    {0x44BC, 0x4B},
    {0x44BE, 0x4B},
    {0x44C0, 0x3C},
    {0x44C2, 0x19},
    {0x44C4, 0x19},
    {0x44C6, 0x19},
    {0x44C8, 0xF0},
    {0x44CA, 0xEB},
    {0x44CC, 0xEB},
    {0x44CE, 0xE6},
    {0x44D0, 0xE6},
    {0x44D2, 0xBB},
    {0x44D4, 0xBB},
    {0x44D6, 0xBB},
    {0x44D8, 0xE6},
    {0x44DA, 0xE6},
    {0x44DC, 0xE6},
    {0x44DE, 0xE6},
    {0x44E0, 0xE6},
    {0x44E2, 0xBB},
    {0x44E4, 0xBB},
    {0x44E6, 0xBB},
    {0x44E8, 0xE6},
    {0x44EA, 0xE6},
    {0x44EC, 0xE6},
    {0x44EE, 0xE6},
    {0x44F0, 0xE6},
    {0x44F2, 0xBB},
    {0x44F4, 0xBB},
    {0x44F6, 0xBB},
    {0x4538, 0x15},
    {0x4539, 0x15},
    {0x453A, 0x15},
    {0x4544, 0x15},
    {0x4545, 0x15},
    {0x4546, 0x15},
    {0x4550, 0x11},
    {0x4551, 0x11},
    {0x4552, 0x11},
    {0x4553, 0x11},
    {0x4554, 0x11},
    {0x4555, 0x11},
    {0x4556, 0x11},
    {0x4557, 0x11},
    {0x4558, 0x11},
    {0x455C, 0x11},
    {0x455D, 0x11},
    {0x455E, 0x11},
    {0x455F, 0x11},
    {0x4560, 0x11},
    {0x4561, 0x11},
    {0x4562, 0x11},
    {0x4563, 0x11},
    {0x4564, 0x11},

    {0x4E00, 0x11},  // 0h=continuous mode, 1h=non-continuous mode

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_5m_60fps_init_table_2lane_linear[] =
{
    //"IMX675-AAQR / AATN Window cropping 2592x1440 CSI-2_2lane 72MHz AD:10bit Output:10bit 1440Mbps Master Mode LCG Mode 60.011fps Integration Time 16.624ms"
    //  Tool ver : Ver5.0

     {0x3000, 0x01},  // STANDBY
     {0x3001, 0x00},  // REGHOLD
     {0x3002, 0x01},  // XMSTA
     {0x3014, 0x02},  // INCK_SEL[3:0]
     {0x3015, 0x03},  // DATARATE_SEL[3:0]
     {0x3018, 0x04},  // WINMODE[3:0]
     {0x3019, 0x00},  // CFMODE
     {0x301A, 0x00},  // WDMODE[7:0]
     {0x301B, 0x00},  // ADDMODE[1:0]
     {0x301C, 0x00},  // THIN_V_EN[7:0]
     {0x301E, 0x01},  // VCMODE[7:0]
     {0x3020, 0x00},  // HREVERSE
     {0x3021, 0x00},  // VREVERSE
     {0x3022, 0x00},  // ADBIT[1:0]
     {0x3023, 0x00},  // MDBIT
     {0x3028, 0x88},  // VMAX[19:0]
     {0x3029, 0x06},  // VMAX[19:0]
     {0x302A, 0x00},  // VMAX[19:0]
     {0x302C, 0xE4},  // HMAX[15:0]
     {0x302D, 0x02},  // HMAX[15:0]
     {0x3030, 0x00},  // FDG_SEL0[1:0]
     {0x3031, 0x00},  // FDG_SEL1[1:0]
     {0x3032, 0x00},  // FDG_SEL2[1:0]
     {0x303C, 0x08},  // PIX_HST[12:0]
     {0x303D, 0x00},  // PIX_HST[12:0]
     {0x303E, 0x20},  // PIX_HWIDTH[12:0]
     {0x303F, 0x0A},  // PIX_HWIDTH[12:0]
     {0x3040, 0x01},  // LANEMODE[2:0]
     {0x3044, 0x06},  // PIX_VST[11:0]
     {0x3045, 0x01},  // PIX_VST[11:0]
     {0x3046, 0xA0},  // PIX_VWIDTH[11:0]
     {0x3047, 0x05},  // PIX_VWIDTH[11:0]
     {0x304C, 0x00},  // GAIN_HG0[10:0]
     {0x304D, 0x00},  // GAIN_HG0[10:0]
     {0x3050, 0x04},  // SHR0[19:0]
     {0x3051, 0x00},  // SHR0[19:0]
     {0x3052, 0x00},  // SHR0[19:0]
     {0x3054, 0x93},  // SHR1[19:0]
     {0x3055, 0x00},  // SHR1[19:0]
     {0x3056, 0x00},  // SHR1[19:0]
     {0x3058, 0x53},  // SHR2[19:0]
     {0x3059, 0x00},  // SHR2[19:0]
     {0x305A, 0x00},  // SHR2[19:0]
     {0x3060, 0x95},  // RHS1[19:0]
     {0x3061, 0x00},  // RHS1[19:0]
     {0x3062, 0x00},  // RHS1[19:0]
     {0x3064, 0x56},  // RHS2[19:0]
     {0x3065, 0x00},  // RHS2[19:0]
     {0x3066, 0x00},  // RHS2[19:0]
     {0x3070, 0x00},  // GAIN_0[10:0]
     {0x3071, 0x00},  // GAIN_0[10:0]
     {0x3072, 0x00},  // GAIN_1[10:0]
     {0x3073, 0x00},  // GAIN_1[10:0]
     {0x3074, 0x00},  // GAIN_2[10:0]
     {0x3075, 0x00},  // GAIN_2[10:0]
     {0x30A4, 0xAA},  // XVSOUTSEL[1:0]
     {0x30A6, 0x00},  // XVS_DRV[1:0]
     {0x30CC, 0x00},  // -
     {0x30CD, 0x00},  // -
     {0x30CE, 0x02},  // -
     {0x30DC, 0x32},  // BLKLEVEL[9:0]
     {0x30DD, 0x40},  // BLKLEVEL[9:0]
     {0x310C, 0x01},  // -
     {0x3130, 0x01},  // -
     {0x3148, 0x00},  // -
     {0x315E, 0x10},  // -
     {0x3400, 0x01},  // GAIN_PGC_FIDMD
     {0x3460, 0x22},  // -
     {0x347B, 0x02},  // -
     {0x3492, 0x08},  // -
     {0x3890, 0x08},  // HFR_EN[3:0]
     {0x3891, 0x00},  // HFR_EN[3:0]
     {0x3893, 0x00},  // -
     {0x3B1D, 0x17},  // -
     {0x3B44, 0x3F},  // -
     {0x3B60, 0x03},  // -
     {0x3C03, 0x04},  // -
     {0x3C04, 0x04},  // -
     {0x3C0A, 0x03},  // -
     {0x3C0B, 0x03},  // -
     {0x3C0C, 0x03},  // -
     {0x3C0D, 0x03},  // -
     {0x3C0E, 0x03},  // -
     {0x3C0F, 0x03},  // -
     {0x3C30, 0x73},  // -
     {0x3C3C, 0x20},  // -
     {0x3C44, 0x06},  // -
     {0x3C7C, 0xB9},  // -
     {0x3C7D, 0x01},  // -
     {0x3C7E, 0xB7},  // -
     {0x3C7F, 0x01},  // -
     {0x3CB0, 0x00},  // -
     {0x3CB2, 0xFF},  // -
     {0x3CB3, 0x03},  // -
     {0x3CB4, 0xFF},  // -
     {0x3CB5, 0x03},  // -
     {0x3CBA, 0xFF},  // -
     {0x3CBB, 0x03},  // -
     {0x3CC0, 0xFF},  // -
     {0x3CC1, 0x03},  // -
     {0x3CC2, 0x00},  // -
     {0x3CC6, 0xFF},  // -
     {0x3CC7, 0x03},  // -
     {0x3CC8, 0xFF},  // -
     {0x3CC9, 0x03},  // -
     {0x3E00, 0x1E},  // -
     {0x3E02, 0x04},  // -
     {0x3E03, 0x00},  // -
     {0x3E20, 0x04},  // -
     {0x3E21, 0x00},  // -
     {0x3E22, 0x1E},  // -
     {0x3E24, 0xBA},  // -
     {0x3E72, 0x85},  // -
     {0x3E76, 0x0C},  // -
     {0x3E77, 0x01},  // -
     {0x3E7A, 0x85},  // -
     {0x3E7E, 0x1F},  // -
     {0x3E82, 0xA6},  // -
     {0x3E86, 0x2D},  // -
     {0x3EE2, 0x33},  // -
     {0x3EE3, 0x03},  // -
     {0x4490, 0x07},  // -
     {0x4494, 0x19},  // -
     {0x4495, 0x00},  // -
     {0x4496, 0xBB},  // -
     {0x4497, 0x00},  // -
     {0x4498, 0x55},  // -
     {0x449A, 0x50},  // -
     {0x449C, 0x50},  // -
     {0x449E, 0x50},  // -
     {0x44A0, 0x3C},  // -
     {0x44A2, 0x19},  // -
     {0x44A4, 0x19},  // -
     {0x44A6, 0x19},  // -
     {0x44A8, 0x4B},  // -
     {0x44AA, 0x4B},  // -
     {0x44AC, 0x4B},  // -
     {0x44AE, 0x4B},  // -
     {0x44B0, 0x3C},  // -
     {0x44B2, 0x19},  // -
     {0x44B4, 0x19},  // -
     {0x44B6, 0x19},  // -
     {0x44B8, 0x4B},  // -
     {0x44BA, 0x4B},  // -
     {0x44BC, 0x4B},  // -
     {0x44BE, 0x4B},  // -
     {0x44C0, 0x3C},  // -
     {0x44C2, 0x19},  // -
     {0x44C4, 0x19},  // -
     {0x44C6, 0x19},  // -
     {0x44C8, 0xF0},  // -
     {0x44CA, 0xEB},  // -
     {0x44CC, 0xEB},  // -
     {0x44CE, 0xE6},  // -
     {0x44D0, 0xE6},  // -
     {0x44D2, 0xBB},  // -
     {0x44D4, 0xBB},  // -
     {0x44D6, 0xBB},  // -
     {0x44D8, 0xE6},  // -
     {0x44DA, 0xE6},  // -
     {0x44DC, 0xE6},  // -
     {0x44DE, 0xE6},  // -
     {0x44E0, 0xE6},  // -
     {0x44E2, 0xBB},  // -
     {0x44E4, 0xBB},  // -
     {0x44E6, 0xBB},  // -
     {0x44E8, 0xE6},  // -
     {0x44EA, 0xE6},  // -
     {0x44EC, 0xE6},  // -
     {0x44EE, 0xE6},  // -
     {0x44F0, 0xE6},  // -
     {0x44F2, 0xBB},  // -
     {0x44F4, 0xBB},  // -
     {0x44F6, 0xBB},  // -
     {0x4538, 0x15},  // -
     {0x4539, 0x15},  // -
     {0x453A, 0x15},  // -
     {0x4544, 0x15},  // -
     {0x4545, 0x15},  // -
     {0x4546, 0x15},  // -
     {0x4550, 0x10},  // -
     {0x4551, 0x10},  // -
     {0x4552, 0x10},  // -
     {0x4553, 0x10},  // -
     {0x4554, 0x10},  // -
     {0x4555, 0x10},  // -
     {0x4556, 0x10},  // -
     {0x4557, 0x10},  // -
     {0x4558, 0x10},  // -
     {0x455C, 0x10},  // -
     {0x455D, 0x10},  // -
     {0x455E, 0x10},  // -
     {0x455F, 0x10},  // -
     {0x4560, 0x10},  // -
     {0x4561, 0x10},  // -
     {0x4562, 0x10},  // -
     {0x4563, 0x10},  // -
     {0x4564, 0x10},  // -
     {0x4569, 0x01},  // -
     {0x456A, 0x01},  // -
     {0x456B, 0x06},  // -
     {0x456C, 0x06},  // -
     {0x456D, 0x06},  // -
     {0x456E, 0x06},  // -
     {0x456F, 0x06},  // -
     {0x4570, 0x06},  // -

    {0x4E00, 0x11},  // 0h=continuous mode, 1h=non-continuous mode

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_6m_30fps_init_table_4lane_HDR_DOL[] =
{
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop

    {0x3014, 0x02},
    {0x301A, 0x01},
    {0x301C, 0x01},
    {0x3022, 0x01},
    {0x3028, 0x08},
    {0x3029, 0x08},
    {0x302C, 0x5A},
    {0x302D, 0x02},
    {0x3050, 0x94},
    {0x3051, 0x0f},
    {0x3054, 0x05},
    {0x3060, 0xC1},
    {0x30A6, 0x00},
    {0x30CE, 0x02},
    {0x3148, 0x00},
    {0x3460, 0x22},
    {0x3492, 0x08},
    {0x3B1D, 0x17},
    {0x3B44, 0x3F},
    {0x3B60, 0x03},
    {0x3C03, 0x04},
    {0x3C04, 0x04},
    {0x3C0B, 0x00},
    {0x3C0C, 0x00},
    {0x3C0D, 0x00},
    {0x3C0E, 0x00},
    {0x3C0F, 0x00},
    {0x3C30, 0x73},
    {0x3C3C, 0x20},
    {0x3C7C, 0xB9},
    {0x3C7D, 0x01},
    {0x3C7E, 0xB7},
    {0x3C7F, 0x01},
    {0x3CB0, 0x00},
    {0x3CB2, 0xFF},
    {0x3CB3, 0x03},
    {0x3CB4, 0xFF},
    {0x3CB5, 0x03},
    {0x3CBA, 0xFF},
    {0x3CBB, 0x03},
    {0x3CC0, 0xFF},
    {0x3CC1, 0x03},
    {0x3CC2, 0x00},
    {0x3CC6, 0xFF},
    {0x3CC7, 0x03},
    {0x3CC8, 0xFF},
    {0x3CC9, 0x03},
    {0x3E00, 0x1E},
    {0x3E02, 0x04},
    {0x3E03, 0x00},
    {0x3E20, 0x04},
    {0x3E21, 0x00},
    {0x3E22, 0x1E},
    {0x3E24, 0xBA},
    {0x3E72, 0x85},
    {0x3E76, 0x0C},
    {0x3E77, 0x01},
    {0x3E7A, 0x85},
    {0x3E7E, 0x1F},
    {0x3E82, 0xA6},
    {0x3E86, 0x2D},
    {0x3EE2, 0x33},
    {0x3EE3, 0x03},
    {0x4490, 0x07},
    {0x4494, 0x19},
    {0x4495, 0x00},
    {0x4496, 0xBB},
    {0x4497, 0x00},
    {0x4498, 0x55},
    {0x449A, 0x50},
    {0x449C, 0x50},
    {0x449E, 0x50},
    {0x44A0, 0x3C},
    {0x44A2, 0x19},
    {0x44A4, 0x19},
    {0x44A6, 0x19},
    {0x44A8, 0x4B},
    {0x44AA, 0x4B},
    {0x44AC, 0x4B},
    {0x44AE, 0x4B},
    {0x44B0, 0x3C},
    {0x44B2, 0x19},
    {0x44B4, 0x19},
    {0x44B6, 0x19},
    {0x44B8, 0x4B},
    {0x44BA, 0x4B},
    {0x44BC, 0x4B},
    {0x44BE, 0x4B},
    {0x44C0, 0x3C},
    {0x44C2, 0x19},
    {0x44C4, 0x19},
    {0x44C6, 0x19},
    {0x44C8, 0xF0},
    {0x44CA, 0xEB},
    {0x44CC, 0xEB},
    {0x44CE, 0xE6},
    {0x44D0, 0xE6},
    {0x44D2, 0xBB},
    {0x44D4, 0xBB},
    {0x44D6, 0xBB},
    {0x44D8, 0xE6},
    {0x44DA, 0xE6},
    {0x44DC, 0xE6},
    {0x44DE, 0xE6},
    {0x44E0, 0xE6},
    {0x44E2, 0xBB},
    {0x44E4, 0xBB},
    {0x44E6, 0xBB},
    {0x44E8, 0xE6},
    {0x44EA, 0xE6},
    {0x44EC, 0xE6},
    {0x44EE, 0xE6},
    {0x44F0, 0xE6},
    {0x44F2, 0xBB},
    {0x44F4, 0xBB},
    {0x44F6, 0xBB},
    {0x4538, 0x15},
    {0x4539, 0x15},
    {0x453A, 0x15},
    {0x4544, 0x15},
    {0x4545, 0x15},
    {0x4546, 0x15},
    {0x4550, 0x11},
    {0x4551, 0x11},
    {0x4552, 0x11},
    {0x4553, 0x11},
    {0x4554, 0x11},
    {0x4555, 0x11},
    {0x4556, 0x11},
    {0x4557, 0x11},
    {0x4558, 0x11},
    {0x455C, 0x11},
    {0x455D, 0x11},
    {0x455E, 0x11},
    {0x455F, 0x11},
    {0x4560, 0x11},
    {0x4561, 0x11},
    {0x4562, 0x11},
    {0x4563, 0x11},
    {0x4564, 0x11},

    {0x3230, 0x00},  //GLOBAL_TIMING_MANEN
    {0x3236, 0xE7},  //TCKPOST
    {0x3237, 0x00},  //TCKPOST
    {0x3238, 0x8F},  //TCKPREPARE
    {0x3239, 0x00},  //TCKPREPARE
    {0x323A, 0x8F},  //TCKTRAIL
    {0x323B, 0x00},  //TCKTRAIL
    {0x323C, 0x7F},  //TCKZERO
    {0x323D, 0x02},  //TCKZERO
    {0x323E, 0x97},  //THSPREPARE
    {0x323F, 0x00},  //THSPREPARE
    {0x3240, 0x0F},  //THSZERO
    {0x3241, 0x01},  //THSZERO
    {0x3242, 0x97},  //THSTRAIL
    {0x3243, 0x00},  //THSTRAIL
    {0x3244, 0xF7},  //THSEXIT
    {0x3245, 0x00},  //THSEXIT

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
const static I2C_ARRAY Sensor_5m_30fps_init_table_4lane_CLR_HDR[] =
{
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop

    {0x3014, 0x01},
    {0x301A, 0x08},
    {0x3022, 0x01},
    {0x3028, 0x0F},
    {0x3029, 0x10},
    {0x302c, 0x5A},
    {0x302D, 0x02},
    {0x3030, 0x02},
    {0x304c, 0x1E},
    {0x3050, 0x08},
    {0x30A6, 0x00},
    {0x30CE, 0x02},
    {0x3148, 0x00},
    {0x3460, 0x22},
    {0x347B, 0x02},
    {0x3492, 0x08},
    {0x3B1D, 0x17},
    {0x3B44, 0x3F},
    {0x3B60, 0x03},
    {0x3C03, 0x04},
    {0x3C04, 0x04},
    {0x3C0A, 0x1F},
    {0x3C0B, 0x1F},
    {0x3C0C, 0x1F},
    {0x3C0D, 0x1F},
    {0x3C0E, 0x1F},
    {0x3C0F, 0x1F},
    {0x3C30, 0x73},
    {0x3C3C, 0x20},
    {0x3C44, 0x05},
    {0x3C7C, 0xB9},
    {0x3C7D, 0x01},
    {0x3C7E, 0xB7},
    {0x3C7F, 0x01},
    {0x3CB0, 0x00},
    {0x3CB2, 0xFF},
    {0x3CB3, 0x03},
    {0x3CB4, 0xFF},
    {0x3CB5, 0x03},
    {0x3CBA, 0xFF},
    {0x3CBB, 0x03},
    {0x3CC0, 0xFF},
    {0x3CC1, 0x03},
    {0x3CC2, 0x00},
    {0x3CC6, 0xFF},
    {0x3CC7, 0x03},
    {0x3CC8, 0xFF},
    {0x3CC9, 0x03},
    {0x3E00, 0x1E},
    {0x3E02, 0x04},
    {0x3E03, 0x00},
    {0x3E20, 0x04},
    {0x3E21, 0x00},
    {0x3E22, 0x1E},
    {0x3E24, 0xBA},
    {0x3E72, 0x85},
    {0x3E76, 0x0C},
    {0x3E77, 0x01},
    {0x3E7A, 0x85},
    {0x3E7E, 0x1F},
    {0x3E82, 0xA6},
    {0x3E86, 0x2D},
    {0x3EE2, 0x33},
    {0x3EE3, 0x03},
    {0x4490, 0x07},
    {0x4494, 0x19},
    {0x4495, 0x00},
    {0x4496, 0xBB},
    {0x4497, 0x00},
    {0x4498, 0x5A},
    {0x449A, 0x50},
    {0x449C, 0x50},
    {0x449E, 0x50},
    {0x44A0, 0x3C},
    {0x44A2, 0x19},
    {0x44A4, 0x19},
    {0x44A6, 0x19},
    {0x44A8, 0x50},
    {0x44AA, 0x50},
    {0x44AC, 0x50},
    {0x44AE, 0x46},
    {0x44B0, 0x46},
    {0x44B2, 0x19},
    {0x44B4, 0x19},
    {0x44B6, 0x19},
    {0x44B8, 0x50},
    {0x44BA, 0x50},
    {0x44BC, 0x50},
    {0x44BE, 0x46},
    {0x44C0, 0x46},
    {0x44C2, 0x19},
    {0x44C4, 0x19},
    {0x44C6, 0x19},
    {0x44C8, 0xF0},
    {0x44CA, 0xEB},
    {0x44CC, 0xEB},
    {0x44CE, 0xE6},
    {0x44D0, 0xE6},
    {0x44D2, 0xBB},
    {0x44D4, 0xBB},
    {0x44D6, 0xBB},
    {0x44D8, 0xE6},
    {0x44DA, 0xE6},
    {0x44DC, 0xE6},
    {0x44DE, 0xE6},
    {0x44E0, 0xE6},
    {0x44E2, 0xBB},
    {0x44E4, 0xBB},
    {0x44E6, 0xBB},
    {0x44E8, 0xE6},
    {0x44EA, 0xE6},
    {0x44EC, 0xE6},
    {0x44EE, 0xE6},
    {0x44F0, 0xE6},
    {0x44F2, 0xBB},
    {0x44F4, 0xBB},
    {0x44F6, 0xBB},
    {0x4538, 0x15},
    {0x4539, 0x15},
    {0x453A, 0x15},
    {0x4544, 0x15},
    {0x4545, 0x15},
    {0x4546, 0x15},
    {0x4550, 0x10},
    {0x4551, 0x10},
    {0x4552, 0x10},
    {0x4553, 0x10},
    {0x4554, 0x10},
    {0x4555, 0x10},
    {0x4556, 0x10},
    {0x4557, 0x10},
    {0x4558, 0x10},
    {0x455C, 0x10},
    {0x455D, 0x10},
    {0x455E, 0x10},
    {0x455F, 0x10},
    {0x4560, 0x10},
    {0x4561, 0x10},
    {0x4562, 0x10},
    {0x4563, 0x10},
    {0x4564, 0x10},
    {0x4569, 0x00},
    {0x456A, 0x00},
    {0x456B, 0x04},
    {0x456C, 0x04},
    {0x456D, 0x04},
    {0x456E, 0x04},
    {0x456F, 0x04},
    {0x4570, 0x04},

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_5m_30fps_init_table_2lane_DOL_HDR[] =
{
    //"IMX675-AAQR / AATN Window cropping 2592x1440 CSI-2_2lane 72MHz AD:10bit Output:10bit 1440Mbps Master Mode LCG Mode 60.011fps Integration Time 16.624ms"
    //  Tool ver : Ver5.0

     {0x3000, 0x01},  // STANDBY
     {0x3001, 0x00},  // REGHOLD
     {0x3002, 0x01},  // XMSTA
     {0x3014, 0x02},  // INCK_SEL[3:0]
     {0x3015, 0x03},  // DATARATE_SEL[3:0]
     {0x3018, 0x04},  // WINMODE[3:0]
     {0x3019, 0x00},  // CFMODE
     {0x301A, 0x01},  // WDMODE[7:0]        0x00
     {0x301B, 0x00},  // ADDMODE[1:0]
     {0x301C, 0x01},  // THIN_V_EN[7:0]     0x00
     {0x301E, 0x01},  // VCMODE[7:0]
     {0x3020, 0x00},  // HREVERSE
     {0x3021, 0x00},  // VREVERSE
     {0x3022, 0x00},  // ADBIT[1:0]
     {0x3023, 0x00},  // MDBIT
     {0x3028, 0x88},  // VMAX[19:0]
     {0x3029, 0x06},  // VMAX[19:0]
     {0x302A, 0x00},  // VMAX[19:0]
     {0x302C, 0xE4},  // HMAX[15:0]
     {0x302D, 0x02},  // HMAX[15:0]
     {0x3030, 0x00},  // FDG_SEL0[1:0]
     {0x3031, 0x00},  // FDG_SEL1[1:0]
     {0x3032, 0x00},  // FDG_SEL2[1:0]
     {0x303C, 0x08},  // PIX_HST[12:0]
     {0x303D, 0x00},  // PIX_HST[12:0]
     {0x303E, 0x20},  // PIX_HWIDTH[12:0]
     {0x303F, 0x0A},  // PIX_HWIDTH[12:0]
     {0x3040, 0x01},  // LANEMODE[2:0]
     {0x3044, 0x06},  // PIX_VST[11:0]
     {0x3045, 0x01},  // PIX_VST[11:0]
     {0x3046, 0xA0},  // PIX_VWIDTH[11:0]
     {0x3047, 0x05},  // PIX_VWIDTH[11:0]
     {0x304C, 0x00},  // GAIN_HG0[10:0]
     {0x304D, 0x00},  // GAIN_HG0[10:0]
     {0x3050, 0xC6},  // SHR0[19:0]          0x04
     {0x3051, 0x00},  // SHR0[19:0]          0x00
     {0x3052, 0x00},  // SHR0[19:0]
     {0x3054, 0x05},  // SHR1[19:0]          0x93
     {0x3055, 0x00},  // SHR1[19:0]
     {0x3056, 0x00},  // SHR1[19:0]
     {0x3058, 0x53},  // SHR2[19:0]
     {0x3059, 0x00},  // SHR2[19:0]
     {0x305A, 0x00},  // SHR2[19:0]
     {0x3060, 0xC1},  // RHS1[19:0]          0x95
     {0x3061, 0x00},  // RHS1[19:0]
     {0x3062, 0x00},  // RHS1[19:0]
     {0x3064, 0x56},  // RHS2[19:0]
     {0x3065, 0x00},  // RHS2[19:0]
     {0x3066, 0x00},  // RHS2[19:0]
     {0x3070, 0x00},  // GAIN_0[10:0]
     {0x3071, 0x00},  // GAIN_0[10:0]
     {0x3072, 0x00},  // GAIN_1[10:0]
     {0x3073, 0x00},  // GAIN_1[10:0]
     {0x3074, 0x00},  // GAIN_2[10:0]
     {0x3075, 0x00},  // GAIN_2[10:0]
     {0x30A4, 0xAA},  // XVSOUTSEL[1:0]
     {0x30A6, 0x00},  // XVS_DRV[1:0]
     {0x30CC, 0x00},  // -
     {0x30CD, 0x00},  // -
     {0x30CE, 0x02},  // -
     {0x30DC, 0x32},  // BLKLEVEL[9:0]
     {0x30DD, 0x40},  // BLKLEVEL[9:0]
     {0x310C, 0x01},  // -
     {0x3130, 0x01},  // -
     {0x3148, 0x00},  // -
     {0x315E, 0x10},  // -
     {0x3400, 0x01},  // GAIN_PGC_FIDMD
     {0x3460, 0x22},  // -
     {0x347B, 0x02},  // -
     {0x3492, 0x08},  // -
     {0x3890, 0x08},  // HFR_EN[3:0]
     {0x3891, 0x00},  // HFR_EN[3:0]
     {0x3893, 0x00},  // -
     {0x3B1D, 0x17},  // -
     {0x3B44, 0x3F},  // -
     {0x3B60, 0x03},  // -
     {0x3C03, 0x04},  // -
     {0x3C04, 0x04},  // -
     {0x3C0A, 0x03},  // -
     {0x3C0B, 0x03},  // -
     {0x3C0C, 0x03},  // -
     {0x3C0D, 0x03},  // -
     {0x3C0E, 0x03},  // -
     {0x3C0F, 0x03},  // -
     {0x3C30, 0x73},  // -
     {0x3C3C, 0x20},  // -
     {0x3C44, 0x06},  // -
     {0x3C7C, 0xB9},  // -
     {0x3C7D, 0x01},  // -
     {0x3C7E, 0xB7},  // -
     {0x3C7F, 0x01},  // -
     {0x3CB0, 0x00},  // -
     {0x3CB2, 0xFF},  // -
     {0x3CB3, 0x03},  // -
     {0x3CB4, 0xFF},  // -
     {0x3CB5, 0x03},  // -
     {0x3CBA, 0xFF},  // -
     {0x3CBB, 0x03},  // -
     {0x3CC0, 0xFF},  // -
     {0x3CC1, 0x03},  // -
     {0x3CC2, 0x00},  // -
     {0x3CC6, 0xFF},  // -
     {0x3CC7, 0x03},  // -
     {0x3CC8, 0xFF},  // -
     {0x3CC9, 0x03},  // -
     {0x3E00, 0x1E},  // -
     {0x3E02, 0x04},  // -
     {0x3E03, 0x00},  // -
     {0x3E20, 0x04},  // -
     {0x3E21, 0x00},  // -
     {0x3E22, 0x1E},  // -
     {0x3E24, 0xBA},  // -
     {0x3E72, 0x85},  // -
     {0x3E76, 0x0C},  // -
     {0x3E77, 0x01},  // -
     {0x3E7A, 0x85},  // -
     {0x3E7E, 0x1F},  // -
     {0x3E82, 0xA6},  // -
     {0x3E86, 0x2D},  // -
     {0x3EE2, 0x33},  // -
     {0x3EE3, 0x03},  // -
     {0x4490, 0x07},  // -
     {0x4494, 0x19},  // -
     {0x4495, 0x00},  // -
     {0x4496, 0xBB},  // -
     {0x4497, 0x00},  // -
     {0x4498, 0x55},  // -
     {0x449A, 0x50},  // -
     {0x449C, 0x50},  // -
     {0x449E, 0x50},  // -
     {0x44A0, 0x3C},  // -
     {0x44A2, 0x19},  // -
     {0x44A4, 0x19},  // -
     {0x44A6, 0x19},  // -
     {0x44A8, 0x4B},  // -
     {0x44AA, 0x4B},  // -
     {0x44AC, 0x4B},  // -
     {0x44AE, 0x4B},  // -
     {0x44B0, 0x3C},  // -
     {0x44B2, 0x19},  // -
     {0x44B4, 0x19},  // -
     {0x44B6, 0x19},  // -
     {0x44B8, 0x4B},  // -
     {0x44BA, 0x4B},  // -
     {0x44BC, 0x4B},  // -
     {0x44BE, 0x4B},  // -
     {0x44C0, 0x3C},  // -
     {0x44C2, 0x19},  // -
     {0x44C4, 0x19},  // -
     {0x44C6, 0x19},  // -
     {0x44C8, 0xF0},  // -
     {0x44CA, 0xEB},  // -
     {0x44CC, 0xEB},  // -
     {0x44CE, 0xE6},  // -
     {0x44D0, 0xE6},  // -
     {0x44D2, 0xBB},  // -
     {0x44D4, 0xBB},  // -
     {0x44D6, 0xBB},  // -
     {0x44D8, 0xE6},  // -
     {0x44DA, 0xE6},  // -
     {0x44DC, 0xE6},  // -
     {0x44DE, 0xE6},  // -
     {0x44E0, 0xE6},  // -
     {0x44E2, 0xBB},  // -
     {0x44E4, 0xBB},  // -
     {0x44E6, 0xBB},  // -
     {0x44E8, 0xE6},  // -
     {0x44EA, 0xE6},  // -
     {0x44EC, 0xE6},  // -
     {0x44EE, 0xE6},  // -
     {0x44F0, 0xE6},  // -
     {0x44F2, 0xBB},  // -
     {0x44F4, 0xBB},  // -
     {0x44F6, 0xBB},  // -
     {0x4538, 0x15},  // -
     {0x4539, 0x15},  // -
     {0x453A, 0x15},  // -
     {0x4544, 0x15},  // -
     {0x4545, 0x15},  // -
     {0x4546, 0x15},  // -
     {0x4550, 0x10},  // -
     {0x4551, 0x10},  // -
     {0x4552, 0x10},  // -
     {0x4553, 0x10},  // -
     {0x4554, 0x10},  // -
     {0x4555, 0x10},  // -
     {0x4556, 0x10},  // -
     {0x4557, 0x10},  // -
     {0x4558, 0x10},  // -
     {0x455C, 0x10},  // -
     {0x455D, 0x10},  // -
     {0x455E, 0x10},  // -
     {0x455F, 0x10},  // -
     {0x4560, 0x10},  // -
     {0x4561, 0x10},  // -
     {0x4562, 0x10},  // -
     {0x4563, 0x10},  // -
     {0x4564, 0x10},  // -
     {0x4569, 0x01},  // -
     {0x456A, 0x01},  // -
     {0x456B, 0x06},  // -
     {0x456C, 0x06},  // -
     {0x456D, 0x06},  // -
     {0x456E, 0x06},  // -
     {0x456F, 0x06},  // -
     {0x4570, 0x06},  // -

    {0x3230, 0x01},  //GLOBAL_TIMING_MANEN
    {0x3236, 0xE7},  //TCKPOST
    {0x3237, 0x00},  //TCKPOST
    {0x3238, 0x95},  //TCKPREPARE
    {0x3239, 0x00},  //TCKPREPARE
    {0x323A, 0x38},  //TCKTRAIL
    {0x323B, 0x00},  //TCKTRAIL
    {0x323C, 0x7F},  //TCKZERO
    {0x323D, 0x02},  //TCKZERO
    {0x323E, 0x95},  //THSPREPARE
    {0x323F, 0x00},  //THSPREPARE
    {0x3240, 0x0F},  //THSZERO
    {0x3241, 0x01},  //THSZERO
    {0x3242, 0x38},  //THSTRAIL
    {0x3243, 0x00},  //THSTRAIL
    {0x3244, 0xF7},  //THSEXIT
    {0x3245, 0x00},  //THSEXIT

     {0x4E00, 0x11},  // 0h=continuous mode, 1h=non-continuous mode

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
/*
const static I2C_ARRAY Sensor_id_table[] = {
    {0x3F12, 0x14},      // {address of ID, ID },
    {0x3F13, 0x75},      // {address of ID, ID },
};
*/
static I2C_ARRAY PatternTbl[] = {
    {0x0000,0x00},       // colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY vts_reg[] = {       //VMAX
    {0x302A, 0x00}, //bit0-3-->MSB
    {0x3029, 0x08},
    {0x3028, 0xCA},
};

const static I2C_ARRAY expo_SHR0_reg[] = { // SHS0 (For LEF)
    {0x3052, 0x00},
    {0x3051, 0x00},
    {0x3050, 0xC6},
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
    {0x3060, 0xC1},
};

const static I2C_ARRAY gain_reg[] = {
    {0x3070, 0x00},//low bit
    {0x3071, 0x00},//hcg mode,bit 4
};

const static I2C_ARRAY gain_HDR_DOL_LEF_reg[] = {
    {0x3070, 0x2A},
    {0x3071, 0x00},
};

const static I2C_ARRAY gain_HDR_DOL_SEF_reg[] = {
    {0x3072, 0x20},
    {0x3073, 0x00},
};

const static I2C_ARRAY gain_CLR_HDR_HG_reg[] = {
    {0x304C, 0x34},
    {0x304D, 0x00},
};

const static I2C_ARRAY gain_CLR_HDR_LG_reg[] = {
    {0x3070, 0x00},
    {0x3071, 0x00},
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

#define I2C_READ  (0x01)
#define I2C_WRITE  (0x02)

typedef struct stI2CRegData_s
{
    u16 u16Reg;
    u16 u16Data;
}stI2CRegData_t;

static int pCus_sensor_CustDefineFunction(ss_cus_sensor *handle, u32 cmd_id, void *i2c_cmd)
{
    stI2CRegData_t *pRegData = (stI2CRegData_t *)i2c_cmd;

    switch(cmd_id)
    {
        case I2C_READ:
            SensorReg_Read(pRegData->u16Reg, &(pRegData->u16Data));
            break;
        case I2C_WRITE:
            SensorReg_Write(pRegData->u16Reg, pRegData->u16Data);
            break;
        default:
            SENSOR_DMSG("cmdid %d, unknow \n", cmd_id);
            break;
    }
    SENSOR_EMSG("[%s]Snr %s reg 0x%x data 0x%x_%d\n", __func__, (cmd_id == I2C_READ) ? "READ" : "WRITE",  pRegData->u16Reg, pRegData->u16Data, pRegData->u16Data);

    return SUCCESS;
}


/////////////////// sensor hardware dependent ///////////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = params->Line_Period * 2;
    info->u32AEShutter_step                  = params->Line_Period * 2;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;

    return SUCCESS;
}

static int pCus_sensor_GetAEInfo_SEF(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;

    SENSOR_DMSG("%s Clrhdr %d lane num %d\r\n", __func__, params->Clrhdr_mode_en, handle->interface_attr.attr_mipi.mipi_lane_num);
    if (params->Clrhdr_mode_en == 0)
    {
        info->u32AEShutter_min  = params->Line_Period * 2;
        info->u32AEShutter_step = params->Line_Period;
        info->u32AEShutter_max  = params->Line_Period * (params->min_rhs1 - 5);
    }
    else
    {
        info->u32AEShutter_min  = params->Line_Period * 2;
        info->u32AEShutter_step = params->Line_Period;
        info->u32AEShutter_max  = 1000000000/IMX675_mipi_hdr[0].senout.min_fps;
    }

    SENSOR_DMSG("%s min %d step %d max %d min_rhs1 %d\r\n", __func__, info->u32AEShutter_min, info->u32AEShutter_step, info->u32AEShutter_max, params->min_rhs1);
    return SUCCESS;
}


static int pCus_sensor_GetAEInfo_LEF(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;

    SENSOR_DMSG("%s Clrhdr %d lane num %d\r\n", __func__, params->Clrhdr_mode_en, handle->interface_attr.attr_mipi.mipi_lane_num);
    if (params->Clrhdr_mode_en == 0)
    {
        info->u32AEShutter_min  = params->Line_Period * 2;
        info->u32AEShutter_step = params->Line_Period;
        info->u32AEShutter_max  = params->Line_Period * (params->fsc - params->min_rhs1 - 5);
    }
    else
    {
        info->u32AEShutter_min  = params->Line_Period * 2;
        info->u32AEShutter_step = params->Line_Period;
        info->u32AEShutter_max  = 1000000000/IMX675_mipi_hdr[0].senout.min_fps;
    }

    SENSOR_DMSG("%s min %d step %d max %d fsc %d max_shr0 %d\r\n", __func__, info->u32AEShutter_min, info->u32AEShutter_step, info->u32AEShutter_max, params->fsc, params->max_shr0);
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    IMX675_params *params = (IMX675_params *)handle->private_data;

    //Sensor power on sequence
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);   // Powerdn Pull Low
    sensor_if->Reset(idx, params->reset_POLARITY);     // Rst Pull Low
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_288M);//216 fail
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, ENABLE);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    //Sensor board PWDN Enable, 1.8V & 2.9V need 30ms then Pull High
    SENSOR_MSLEEP(31);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    SENSOR_UDELAY(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    IMX675_params *params = (IMX675_params *)handle->private_data;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);   // Powerdn Pull Low
    sensor_if->Reset(idx, params->reset_POLARITY);     // Rst Pull Low
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }

    params->res.orit = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// Check Sensor Product ID /////////////////////////
static int pCus_CheckSensorProductID(ss_cus_sensor *handle)
{
    //u16 sen_id_msb, sen_id_lsb;

    /* Read Product ID */
    //SensorReg_Read(0x3f12, (void*)&sen_id_lsb);
    //SensorReg_Read(0x3f13, (void*)&sen_id_msb);//CHIP_ID_r3F13
#if 0
    if (sen_data != CHIP_ID) {
        printk("[***ERROR***]Check Product ID Fail: 0x%x\n", sen_data);
        return FAIL;
    }
#endif
    return SUCCESS;
}

static int IMX675_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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

static int pCus_init_6m_30fps_mipi4lane_linear(ss_cus_sensor *handle)
{
    //IMX675_params *params = (IMX675_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    //if (pCus_CheckSensorProductID(handle) == FAIL) {
    //    return FAIL;
    //}

    for(i=0;i< ARRAY_SIZE(Sensor_6m_30fps_init_table_4lane_linear);i++)
    {
        SENSOR_DMSG("[%s] addr:0x%x, data:0x%x\n", __FUNCTION__, Sensor_6m_30fps_init_table_4lane_linear[i].reg, Sensor_6m_30fps_init_table_4lane_linear[i].data);
        if(Sensor_6m_30fps_init_table_4lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_6m_30fps_init_table_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_6m_30fps_init_table_4lane_linear[i].reg, Sensor_6m_30fps_init_table_4lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!! %d\n", __FUNCTION__, __LINE__, i);
                    return FAIL;
                }
                SENSOR_UDELAY(1);
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_2560x1440_60fps_2lane_linear(ss_cus_sensor *handle)
{
    //IMX675_params *params = (IMX675_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    //if (pCus_CheckSensorProductID(handle) == FAIL) {
    //    return FAIL;
    //}

    for(i=0;i< ARRAY_SIZE(Sensor_5m_60fps_init_table_2lane_linear);i++)
    {
        SENSOR_DMSG("[%s] addr:0x%x, data:0x%x\n", __FUNCTION__, Sensor_5m_60fps_init_table_2lane_linear[i].reg, Sensor_5m_60fps_init_table_2lane_linear[i].data);
        if(Sensor_5m_60fps_init_table_2lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_5m_60fps_init_table_2lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_5m_60fps_init_table_2lane_linear[i].reg, Sensor_5m_60fps_init_table_2lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!! %d\n", __FUNCTION__, __LINE__, i);
                    return FAIL;
                }
                SENSOR_UDELAY(1);
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_6m_30fps_mipi4lane_HDR_DOL(ss_cus_sensor *handle)
{
    int i,cnt=0;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_6m_30fps_init_table_4lane_HDR_DOL);i++)
    {
        if(Sensor_6m_30fps_init_table_4lane_HDR_DOL[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_6m_30fps_init_table_4lane_HDR_DOL[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_6m_30fps_init_table_4lane_HDR_DOL[i].reg,Sensor_6m_30fps_init_table_4lane_HDR_DOL[i].data) != SUCCESS)
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

static int pCus_init_5m_30fps_mipi4lane_CLR_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_5m_30fps_init_table_4lane_CLR_HDR);i++)
    {
        if(Sensor_5m_30fps_init_table_4lane_CLR_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_5m_30fps_init_table_4lane_CLR_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_5m_30fps_init_table_4lane_CLR_HDR[i].reg,Sensor_5m_30fps_init_table_4lane_CLR_HDR[i].data) != SUCCESS)
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

static int pCus_init_5m_30fps_mipi2lane_DOL_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_5m_30fps_init_table_2lane_DOL_HDR);i++)
    {
        if(Sensor_5m_30fps_init_table_2lane_DOL_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_5m_30fps_init_table_2lane_DOL_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_5m_30fps_init_table_2lane_DOL_HDR[i].reg,Sensor_5m_30fps_init_table_2lane_DOL_HDR[i].data) != SUCCESS)
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

static int IMX675_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    *min = SENSOR_MIN_GAIN;//handle->sat_mingain;
    if (params->Clrhdr_mode_en == 0)
        *max = SENSOR_MAX_GAIN;//10^(72db/20)*1024;
    else
        *max = SENSOR_CLR_HDR_LG_MAX_GAIN;
    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_6m_30fps_mipi4lane_linear;
            params->Init_Vts = 3667;
            ///vts_30fps = 3667;
            params->expo.vts = params->Init_Vts;
            params->expo.fps = 30;
            params->Line_Period = 9090;
            //Preview_line_period = 9090;
            IMX675_GetAEMinMaxGain(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                         &(handle->sensor_ae_info_cfg.u32AEGain_max));
            handle->interface_attr.attr_mipi.mipi_lane_num = 4;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;
        case 1:
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_2560x1440_60fps_2lane_linear;
            params->Init_Vts = 1672;
            //vts_30fps = 1672;
            params->expo.vts = params->Init_Vts;
            params->expo.fps = 60;
            params->Line_Period = 9968;
            //Preview_line_period = 19936;
            IMX675_GetAEMinMaxGain(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                         &(handle->sensor_ae_info_cfg.u32AEGain_max));
            handle->interface_attr.attr_mipi.mipi_lane_num = 2;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;

        default:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_6m_30fps_mipi4lane_linear;
            params->Init_Vts = 3667;
            //vts_30fps = 3667;
            params->expo.vts = params->Init_Vts;
            params->expo.fps = 30;
            params->Line_Period = 9090;
            //Preview_line_period = 9090;
            IMX675_GetAEMinMaxGain(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                         &(handle->sensor_ae_info_cfg.u32AEGain_max));
            break;
    }
    //pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    params->expo.vts = params->Init_Vts;
    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x000f;
    params->tVts_reg[1].data = (params->expo.vts >>  8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >>  0) & 0x00ff;
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_LEF(ss_cus_sensor *handle, u32 res_idx)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    switch (res_idx) {
        case HDR_RES_1:
            handle->interface_attr.attr_mipi.mipi_lane_num = 4;
            params->Init_Vts = 2056;
            //vts_30fps_HDR_DOL_4lane = 2056;
            params->fsc = params->Init_Vts * 2;
            params->min_rhs1 = 193;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;

        case HDR_RES_2:
            handle->interface_attr.attr_mipi.mipi_lane_num = 4;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;

        case HDR_RES_3:
            handle->interface_attr.attr_mipi.mipi_lane_num = 2;
            params->Init_Vts = 1672;
            //vts_30fps_HDR_DOL_2lane = 1672;
            params->fsc = params->Init_Vts * 2;
            params->min_rhs1 = 193;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;

        default:
            break;
    }
    return SUCCESS;
}


static int pCus_SetVideoRes_HDR_DOL_SEF(ss_cus_sensor *handle, u32 res_idx)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    switch (res_idx) {
        case HDR_RES_1:
            handle->pCus_sensor_init = pCus_init_6m_30fps_mipi4lane_HDR_DOL;
            params->Init_Vts = 2056;
            //vts_30fps_HDR_DOL_4lane = 2056;
            params->expo.vts = params->Init_Vts;
            params->expo.fps = 30;
            params->Line_Period = 8106;
            //Preview_line_period_HDR_DOL_4LANE = 8106;
            params->min_rhs1 = 193;
            params->fsc = params->Init_Vts * 2;
            params->Clrhdr_mode_en = 0;
            handle->sensor_ae_info_cfg.u8AEGainDelay = SENSOR_GAIN_DELAY_FRAME_COUNT;
            IMX675_GetAEMinMaxGain(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                         &(handle->sensor_ae_info_cfg.u32AEGain_max));
            handle->mclk = Preview_MCLK_SPEED_HDR_DOL;
            handle->interface_attr.attr_mipi.mipi_lane_num = 4;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;

        case HDR_RES_2:
            handle->pCus_sensor_init = pCus_init_5m_30fps_mipi4lane_CLR_HDR;
            params->Init_Vts = 4111;
            //vts_30fps_HDR_DOL_4lane = 4111;
            params->expo.vts = params->Init_Vts;
            params->expo.fps = 30;
            params->Line_Period = 8108;
            //Preview_line_period_HDR_DOL_4LANE = 8108;
            params->min_rhs1 = 17;
            params->Clrhdr_mode_en = 1;
            handle->sensor_ae_info_cfg.u8AEGainDelay = SENSOR_GAIN_DELAY_FRAME_COUNT_CLR_HDR;
            IMX675_GetAEMinMaxGain(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                         &(handle->sensor_ae_info_cfg.u32AEGain_max));
            handle->mclk = Preview_MCLK_SPEED_CLR_HDR;
            handle->interface_attr.attr_mipi.mipi_lane_num = 4;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;

        case HDR_RES_3:
            handle->pCus_sensor_init = pCus_init_5m_30fps_mipi2lane_DOL_HDR;
            params->Init_Vts = 1672;
            //vts_30fps_HDR_DOL_2lane = 1672;
            params->expo.vts = params->Init_Vts;
            params->expo.fps = 30;
            params->Line_Period = 9968;
            //Preview_line_period_HDR_DOL_2LANE = 9968;
            params->min_rhs1 = 193;
            params->fsc = params->Init_Vts * 2;
            params->Clrhdr_mode_en = 0;
            handle->sensor_ae_info_cfg.u8AEGainDelay = SENSOR_GAIN_DELAY_FRAME_COUNT;
            IMX675_GetAEMinMaxGain(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                         &(handle->sensor_ae_info_cfg.u32AEGain_max));
            handle->mclk = Preview_MCLK_SPEED_HDR_DOL;
            handle->interface_attr.attr_mipi.mipi_lane_num = 2;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;

        default:
            break;
    }
    //pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    params->expo.vts = params->Init_Vts;
    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x000f;
    params->tVts_reg[1].data = (params->expo.vts >>  8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >>  0) & 0x00ff;
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
    IMX675_params *params = (IMX675_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    s16 sen_data;
    //Read SENSOR MIRROR-FLIP STATUS
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
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (params->Init_Vts*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (params->Init_Vts*max_fps)/tVts;

    SENSOR_DMSG("[%s] FPS %d tVts %d\n",__FUNCTION__, params->expo.preview_fps, tVts);

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);
    SENSOR_DMSG("[%s] FPS %d tVts %d\n",__FUNCTION__,fps, tVts);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (tVts*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts=  (tVts*(max_fps*1000) + (fps>>1))/fps;
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
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 cur_vts_30fps = params->Init_Vts;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    SENSOR_DMSG("[%s] FPS %d cur_vts_30fps %d tVts %d\n",__FUNCTION__, params->expo.preview_fps, cur_vts_30fps, tVts);

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_SEF(ss_cus_sensor *handle, u32 fps)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 cur_vts_30fps = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    SENSOR_DMSG("[%s] FPS %d cur_vts_30fps %d\n",__FUNCTION__,fps, cur_vts_30fps);

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
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 cur_vts_30fps = params->Init_Vts;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    SENSOR_DMSG("[%s] FPS %d cur_vts_30fps %d tVts %d\n",__FUNCTION__, params->expo.preview_fps, cur_vts_30fps, tVts);

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_LEF(ss_cus_sensor *handle, u32 fps)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 cur_vts_30fps = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    SENSOR_DMSG("[%s] FPS %d cur_vts_30fps %d\n",__FUNCTION__,fps, cur_vts_30fps);

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
    pCus_SetAEUSecsHDR_DOL_LEF(handle, params->expo.expo_lef_us);
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
    IMX675_params *params = (IMX675_params *)handle->private_data;

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
                    if(0)
                    {
                        SENSOR_EMSG(" vts: (%x, %x, %x) ", params->tVts_reg[0].data,params->tVts_reg[1].data,params->tVts_reg[2].data);
                        SENSOR_EMSG("expo:(%x, %x, %x) ", params->tExpo_reg[0].data,params->tExpo_reg[1].data,params->tExpo_reg[2].data);
                        SENSOR_EMSG("gain:(%x, %x)\n", params->tGain_reg[0].data,params->tGain_reg[1].data);
                    }
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
    //IMX675_params *params = (IMX675_params *)handle->private_data;

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
    IMX675_params *params = (IMX675_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1);
                if(params->dirty) {
                    if (params->Clrhdr_mode_en == 0) // Normal hdr
                    {
                        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR1_reg, ARRAY_SIZE(expo_SHR1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tRHS1_reg, ARRAY_SIZE(expo_RHS1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                        if(0)
                        {
                            SENSOR_EMSG("[%s]", __FUNCTION__);
                            SENSOR_EMSG("vts:(%x, %x, %x) ", params->tVts_reg[0].data,params->tVts_reg[1].data,params->tVts_reg[2].data);
                            SENSOR_EMSG("shr0:(%x, %x, %x) ", params->tSHR0_reg[0].data,params->tSHR0_reg[1].data,params->tSHR0_reg[2].data);
                            SENSOR_EMSG("shr1:(%x, %x, %x) ", params->tSHR1_reg[0].data,params->tSHR1_reg[1].data,params->tSHR1_reg[2].data);
                            SENSOR_EMSG("rhs1:(%x, %x, %x)\n", params->tRHS1_reg[0].data,params->tRHS1_reg[1].data,params->tRHS1_reg[2].data);
                            //SENSOR_EMSG("gainL:(%x, %x) ", params->tGain_hdr_dol_lef_reg[0].data,params->tGain_hdr_dol_lef_reg[1].data);
                            //SENSOR_EMSG("gainS:(%x, %x)\n", params->tGain_hdr_dol_sef_reg[0].data,params->tGain_hdr_dol_sef_reg[1].data);
                        }
                    }
                    else
                    {
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_CLR_HDR_HG_reg, ARRAY_SIZE(gain_CLR_HDR_HG_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_CLR_HDR_LG_reg, ARRAY_SIZE(gain_CLR_HDR_LG_reg));
                        //SENSOR_EMSG("[%s] tVts_reg 0x%x 0x%x 0x%x\n", __FUNCTION__, params->tVts_reg[0].data, params->tVts_reg[1].data, params->tVts_reg[2].data);
                        //SENSOR_EMSG("[%s] tSHR0_reg 0x%x 0x%x 0x%x\n", __FUNCTION__, params->tSHR0_reg[0].data, params->tSHR0_reg[1].data, params->tSHR0_reg[2].data);
                        //SENSOR_EMSG("[%s] tGain_CLR_HDR_HG_reg 0x%x 0x%x \n", __FUNCTION__, params->tGain_CLR_HDR_HG_reg[0].data, params->tGain_CLR_HDR_HG_reg[1].data);
                        //SENSOR_EMSG("[%s] tGain_CLR_HDR_LG_reg 0x%x 0x%x \n", __FUNCTION__, params->tGain_CLR_HDR_LG_reg[0].data, params->tGain_CLR_HDR_LG_reg[1].data);
                    }
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
    IMX675_params *params = (IMX675_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data & 0x0f) << 16;
    lines |= (u32)(params->tExpo_reg[1].data & 0xff) << 8;
    lines |= (u32)(params->tExpo_reg[2].data & 0xff) << 0;

    *us = (lines * params->Line_Period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);
    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines = 0, vts = 0, SHR0 = 0;
    IMX675_params *params = (IMX675_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000*us)/params->Line_Period;
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
    params->tExpo_reg[0].data = (SHR0>>16) & 0x000f;
    params->tExpo_reg[1].data = (SHR0>>8) & 0x00ff;
    params->tExpo_reg[2].data = (SHR0>>0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 16) & 0x000f;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_sef = 0;
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 cur_line_period = params->Line_Period;

    params->expo.expo_sef_us = us;
    cur_line_period = params->Line_Period;
    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    expo_line_sef = ((expo_line_sef >> 1) << 1) + 1; //shr1 restriction: 2n + 1

    if(expo_line_sef < 2) expo_line_sef = 3;
    params->min_shr1 = params->min_rhs1 - expo_line_sef;
    if((expo_line_sef > params->min_rhs1) || ((params->min_shr1) < 10))
        params->min_shr1 = 10;

    SENSOR_DMSG("[%s] us %u, expo_line_sef %u rhs %u shr1 %u\n", __FUNCTION__,
                                                                 us, \
                                                                 expo_line_sef, \
                                                                 params->min_rhs1, \
                                                                 params->min_shr1
               );

    params->tRHS1_reg[0].data = (params->min_rhs1 >>16) & 0x0f;
    params->tRHS1_reg[1].data = (params->min_rhs1 >>8) & 0xff;
    params->tRHS1_reg[2].data = (params->min_rhs1 >>0) & 0xff;

    params->tSHR1_reg[0].data = (params->min_shr1 >> 16) & 0x000f;
    params->tSHR1_reg[1].data = (params->min_shr1 >> 8) & 0x00ff;
    params->tSHR1_reg[2].data = (params->min_shr1 >> 0) & 0x00ff;
    params->dirty = true;

    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_lef = 0, fsc = 0;
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u32 cur_line_period = params->Line_Period;
    if (params->Clrhdr_mode_en == 0)
    {
        cur_line_period = params->Line_Period;
        params->expo.expo_lef_us = us;
        expo_line_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
        expo_line_lef = ((expo_line_lef >> 1) << 1); //shr0 restriction: 2n

        if(expo_line_lef < 2) expo_line_lef = 3;
        fsc = params->expo.vts * 2;   //fsc: 4n
        params->max_shr0 = fsc - expo_line_lef;

        if(params->max_shr0 < (params->min_rhs1+10))
            params->max_shr0 = params->min_rhs1+10;

        params->expo.expo_lines = expo_line_lef;
        SENSOR_DMSG("[%s] us %u, expo_lines_lef %u, fsc %u, SHR0 %u \n", __FUNCTION__,
                                                                         us, \
                                                                         expo_line_lef, \
                                                                         fsc, \
                                                                         params->max_shr0
                    );
        params->tSHR0_reg[0].data = (params->max_shr0 >> 16) & 0x000f;
        params->tSHR0_reg[1].data = (params->max_shr0 >> 8) & 0x00ff;
        params->tSHR0_reg[2].data = (params->max_shr0 >> 0) & 0x00ff;

        params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x000f;
        params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
    }
    else
    {
        u32 expo_lines = 0, SHR0 = 0;
        u32 vts = 0;
        params->expo.expo_lef_us = us;
        expo_lines = (1000*us)/params->Line_Period;

        if (expo_lines > params->expo.vts) {
            vts = expo_lines + 8;
        }
        else
          vts = params->expo.vts;

        SHR0 =  vts - expo_lines;

        if (SHR0 <= 12 )  // 8+4
            SHR0 = 8;
        else
            SHR0 -= 4;
        //CamOsPrintf(KERN_DEBUG" SHR0 %d vts %d expo_lines %d\n",SHR0, vts, expo_lines);
        params->expo.expo_lines = expo_lines;
        SENSOR_DMSG("[%s] us %u, expo_lines_lef %u, vts %u, SHR0 %u \n", __FUNCTION__,
                                                                         us, \
                                                                         expo_lines, \
                                                                         vts, \
                                                                         SHR0 );
        //TBD
        params->tSHR0_reg[0].data = (SHR0 >> 16) & 0x000f;
        params->tSHR0_reg[1].data = (SHR0 >> 8) & 0x00ff;
        params->tSHR0_reg[2].data = (SHR0 >> 0) & 0x00ff;

        params->tVts_reg[0].data = (vts >> 16) & 0x000f;
        params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (vts >> 0) & 0x00ff;
    }
    params->dirty = true;
    return SUCCESS;
}

/*
This API is for converting the gain(dB unit) to the gain(ISP 1024 unit).
*/
static void pCus_GetAEGain_Calculate(u16 gain_reg_value, u32 *gain_value){
    u64 gain_int,gain_deci,i;
    u64 power_int=1,power_deci,power_temp,power_times;
    u64 bits=20, times = 1<<bits;

    power_times = gain_reg_value*times/20;
    power_temp = power_times;
    while(power_temp > times){
        power_int = power_int << 1;
        power_temp = power_temp >> 1;
    }
    power_deci = power_temp;

    gain_deci = base2_exp_float_pow(power_deci);
    gain_int = gain_deci;
    for(i=1; i < power_int;i++){
        gain_int *= gain_deci;
        gain_int = gain_int >> bits;
    }

    *gain_value = (u32)((gain_int*1024)>>bits);

}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;

    *gain = params->expo.final_gain;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
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

    SENSOR_DMSG("[%s] set gain %d =%d 0x%x,  %d 0x%x\n", __FUNCTION__, gain, params->tGain_reg[0].data, params->tGain_reg[0].data, params->tGain_reg[1].data, params->tGain_reg[1].data);

    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGain(ss_cus_sensor *handle, u32 gain)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u64 gain_double;
    u32 try_gain;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    gain_double = (u16)((gain_double * 10) >> 24) / 3;

    pCus_GetAEGain_Calculate(gain_double, &try_gain);

    SENSOR_DMSG("[%s_#%d] set gain %d, try gain %d\n", __FUNCTION__, __LINE__, gain, try_gain);
    //params->dirty = true;
    return try_gain;
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
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u16 gain_reg = 0;
    if (params->Clrhdr_mode_en == 0) // Nornal
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        params->tGain_hdr_dol_sef_reg[0].data = gain_reg & 0xff;
        params->tGain_hdr_dol_sef_reg[1].data = (gain_reg >> 8) & 0xff;
    }
    else
    {
        u64 gain_double;
        if(gain < SENSOR_CLR_HDR_LG_MIN_GAIN){
            gain = SENSOR_CLR_HDR_LG_MIN_GAIN;
        }
        else if(gain > SENSOR_CLR_HDR_LG_MAX_GAIN){
            gain = SENSOR_CLR_HDR_LG_MAX_GAIN;
        }
        gain_double = 20 * (intlog10(gain) - intlog10(1024));
        gain_reg =(u16)((gain_double * 10) >> 24) / 3;
        params->tGain_CLR_HDR_LG_reg[0].data = gain_reg & 0xff;
        params->tGain_CLR_HDR_LG_reg[1].data = (gain_reg >> 8) & 0xff;

    }
    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data, params->tGain_hdr_dol_sef_reg[1].data);
    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGainHDR_DOL_SEF1(ss_cus_sensor *handle, u32 gain)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u16 gain_reg = 0;
    u32 try_gain;

    if (params->Clrhdr_mode_en == 0) // Nornal
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        pCus_GetAEGain_Calculate(gain_reg, &try_gain);
        SENSOR_DMSG("[%s_#%d] set gain %d, try gain %d\n", __FUNCTION__, __LINE__, gain, try_gain);
    }
    else
    {
        u64 gain_double;
        if(gain < SENSOR_CLR_HDR_LG_MIN_GAIN){
            gain = SENSOR_CLR_HDR_LG_MIN_GAIN;
        }
        else if(gain > SENSOR_CLR_HDR_LG_MAX_GAIN){
            gain = SENSOR_CLR_HDR_LG_MAX_GAIN;
        }
        gain_double = 20 * (intlog10(gain) - intlog10(1024));
        gain_reg =(u32)((gain_double * 10) >> 24) / 3;
        pCus_GetAEGain_Calculate(gain_reg, &try_gain);
        SENSOR_DMSG("[%s_#%d] set gain %d, try gain %d\n", __FUNCTION__, __LINE__, gain, try_gain);

    }
    return try_gain;
}


static int pCus_SetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u16 gain_reg = 0;

    if (params->Clrhdr_mode_en == 0)
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        params->tGain_hdr_dol_lef_reg[0].data = gain_reg & 0xff;
        params->tGain_hdr_dol_lef_reg[1].data = (gain_reg >> 8) & 0xff;

        SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data, params->tGain_hdr_dol_lef_reg[1].data);
    }
    else
    {
        u64 gain_double;
        if(gain < SENSOR_CLR_HDR_HG_MIN_GAIN){
            gain = SENSOR_CLR_HDR_HG_MIN_GAIN;
        }
        else if(gain > SENSOR_CLR_HDR_HG_MAX_GAIN){
            gain = SENSOR_CLR_HDR_HG_MAX_GAIN;
        }
        gain_double = 20 * (intlog10(gain) - intlog10(1024));
        gain_reg =(u16)((gain_double * 10) >> 24) / 3;
        SENSOR_DMSG("[%s] set gain/reg=%u/0x%x, %llu\n", __FUNCTION__, gain, gain_reg, gain_double);
        params->tGain_CLR_HDR_HG_reg[0].data = gain_reg & 0xff;
        params->tGain_CLR_HDR_HG_reg[1].data = (gain_reg >> 8) & 0xff;
    }
    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    IMX675_params *params = (IMX675_params *)handle->private_data;
    u16 gain_reg = 0;
    u32 try_gain;

    SENSOR_DMSG("[%s_#%d] Clr_en %d, set gain %d, try gain %d\n", __FUNCTION__, __LINE__, params->Clrhdr_mode_en, gain, gain_reg);

    if (params->Clrhdr_mode_en == 0)
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        pCus_GetAEGain_Calculate(gain_reg, &try_gain);

        SENSOR_DMSG("[%s_#%d] set gain %d, try gain %d\n", __FUNCTION__, __LINE__, gain, try_gain);
    }
    else
    {
        u64 gain_double;
        if(gain < SENSOR_CLR_HDR_HG_MIN_GAIN){
            gain = SENSOR_CLR_HDR_HG_MIN_GAIN;
        }
        else if(gain > SENSOR_CLR_HDR_HG_MAX_GAIN){
            gain = SENSOR_CLR_HDR_HG_MAX_GAIN;
        }
        gain_double = 20 * (intlog10(gain) - intlog10(1024));
        gain_reg =(u16)((gain_double * 10) >> 24) / 3;
        pCus_GetAEGain_Calculate(gain_reg, &try_gain);
        SENSOR_DMSG("[%s_#%d] set gain %d, try gain %d\n", __FUNCTION__, __LINE__, gain, try_gain);
    }
    return try_gain;
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
    IMX675_params *params = (IMX675_params *)handle->private_data;

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

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    IMX675_params *params;
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

    params = (IMX675_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX675_MIPI");

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
        handle->video_res_supported.res[res].u16width             = IMX675_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = IMX675_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = IMX675_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = IMX675_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = IMX675_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = IMX675_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = IMX675_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = IMX675_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, IMX675_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_6m_30fps_mipi4lane_linear;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = IMX675_SetPatternMode;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->pCus_sensor_TryAEGain                                 = pCus_TryAEGain;
    //Preview_line_period = 9090;
    params->Line_Period = 19936;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = params->Line_Period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/IMX675_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Line_Period * 2;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    //params->expo.vts                                              = vts_30fps;
    params->dirty                                                 = false;
    params->expo.expo_lines                                       = 5000;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    IMX675_params *params = NULL;
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

    params = (IMX675_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tRHS1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tSHR1_reg, expo_SHR1_reg, sizeof(expo_SHR1_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));
    memcpy(params->tGain_CLR_HDR_HG_reg, gain_CLR_HDR_HG_reg, sizeof(gain_CLR_HDR_HG_reg));
    memcpy(params->tGain_CLR_HDR_LG_reg, gain_CLR_HDR_LG_reg, sizeof(gain_CLR_HDR_LG_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX675_MIPI_HDR_SEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DOL;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_DOL;
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
    handle->mclk                                                  = Preview_MCLK_SPEED_HDR_DOL;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = IMX675_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = IMX675_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = IMX675_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = IMX675_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = IMX675_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = IMX675_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = IMX675_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = IMX675_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, IMX675_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_6m_30fps_mipi4lane_HDR_DOL;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL_SEF;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_SEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_SEF;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo_SEF;
    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    params->Line_Period = 8106;
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_SEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_SEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_SEF1;
    handle->pCus_sensor_TryAEGain                                 = pCus_TryAEGainHDR_DOL_SEF1;

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = params->Line_Period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/IMX675_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Line_Period * 2;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    //params->expo.vts                                              = vts_30fps_HDR_DOL_4lane;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    IMX675_params *params;
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
    params = (IMX675_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX675_MIPI_HDR_LEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DOL;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_DOL;
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
    handle->pCus_sensor_poweron                                   = pCus_poweron_HDR_DOL_LEF;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff_HDR_DOL_LEF;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    handle->mclk                                                  = Preview_MCLK_SPEED_HDR_DOL;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = IMX675_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = IMX675_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = IMX675_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = IMX675_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = IMX675_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = IMX675_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = IMX675_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = IMX675_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, IMX675_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    params->Line_Period = 8106;
    handle->pCus_sensor_init                                      = pCus_init_HDR_DOL_LEF;
    handle->pCus_sensor_GetVideoResNum                            = NULL;
    handle->pCus_sensor_GetVideoRes                               = NULL;
    handle->pCus_sensor_GetCurVideoRes                            = NULL;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL_LEF;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_LEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_LEF;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo_LEF;
    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_LEF;
    handle->pCus_sensor_TryAEGain                                 = pCus_TryAEGainHDR_DOL_LEF;

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = params->Line_Period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/IMX675_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Line_Period * 2;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    //params->expo.vts                                              = vts_30fps_HDR_DOL_4lane;
    params->dirty                                                 = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(IMX675_HDR,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            IMX675_params
                         );
