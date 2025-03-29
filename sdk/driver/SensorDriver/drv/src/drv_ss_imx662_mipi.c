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
#include <drv_sensor_init_table.h> //TODO: move this header to drv_sensor_common.h
#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX662_HDR);

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
#define SENSOR_MIPI_LANE_NUM        (2)
#define SENSOR_MIPI_LANE_NUM_DOL    (2)
//#define SENSOR_MIPI_HDR_MODE        (0) //0: Non-HDR mode. 1:Sony DOL mode

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
//#define SENSOR_DATAFMT             CUS_DATAFMT_BAYER    //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_DOL         CUS_DATAPRECISION_12
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9098  //CFG
#define SENSOR_BAYERID              CUS_BAYER_GB         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_GB
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_YCORDER              CUS_SEN_YCODR_YC     //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define long_packet_type_enable     0x00 //UD1~UD8 (user define)

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_72MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_72MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x34                //I2C slave address
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
#define SENSOR_NAME     IMX662

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
}IMX662_mipi_linear[] = {
    {LINEAR_RES_1, {1920, 1080, 3, 25}, {0, 0, 1920, 1080}, {"1920x1080@25fps_10bit"}},
    {LINEAR_RES_2, {1920, 1080, 3, 25}, {0, 0, 1920, 1080}, {"1920x1080@25fps_12bit"}}, // Modify it
};

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
}imx662_mipi_hdr[] = {
    {HDR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
    {HDR_RES_2, {1920, 1080, 3, 20}, {0, 0, 1920, 1080}, {"1920x1080@20fps_HDR_3DOL"}}, // Modify it
    {HDR_RES_3, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR_3DOL"}},
};

#define IMX662_HDR_BRL                              2228

u32 vts_30fps = 1250;
u32 vts_30fps_HDR_DOL_2lane = 1250;
u32 Preview_line_period = 26672;
u32 Preview_line_period_HDR_DOL_2LANE = 26672;

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (3981 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
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
    u32 skip_cnt;
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
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
} imx662_params;

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us);
static int pCus_AEStatusNotifyHDR_DOL_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_AEStatusNotifyHDR_DOL_LEF3(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
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
const static I2C_ARRAY Sensor_init_table_2lane_linear[] =
{
    /*All-pixel scan, CSI-2_2lane, 74.25MHz
      AD:10bit Output:10bit, 594Mbps, Master Mode
      LCG Mode, 25fps, Integration Time,39.872ms*/
    {0x3000, 0x01},    //Standby
    {0xffff, 0x14},//delay
    {0x3002, 0x01},    //Master mode stop
    {0xffff, 0x14},//delay
    {0x3015, 0x07},
    {0x3022, 0x01},    //ADBIT    0h: AD 10bit; 1h: AD 12bit
    {0x3023, 0x01},    //MDBIT    0h: 10bit; 1h: 12bit
    {0x3029, 0xA6},    //VMAX
    {0x302A, 0x0E},    //VMAX
    {0x302C, 0x48},    //HMAX
    {0x302D, 0x09},    //HMAX
    {0x3040, 0x01},    //LANE MODE, 1h 2Lane, 3h 4Lane
    {0x3050, 0x04},    //SHR0 Shutter Setting
    {0x3051, 0x00},
    {0x3052, 0x00},
    {0x30A6, 0x0F},    //XVS/XHS PIN setting 0h:XVS outpush(master Mode), 3h:HIZ(Slave Mode)
    {0x3460, 0x21},
    {0x3492, 0x08},
    {0x3A50, 0x62},
    {0x3A51, 0x01},
    {0x3A52, 0x19},
    {0x3B00, 0x39},
    {0x3B23, 0x2D},
    {0x3B45, 0x04},
    {0x3C0A, 0x1F},
    {0x3C0B, 0x1E},
    {0x3C38, 0x21},
    {0x3C44, 0x00},
    {0x3CB6, 0xD8},
    {0x3CC4, 0xDA},
    {0x3E24, 0x79},
    {0x3E2C, 0x15},
    {0x3EDC, 0x2D},
    {0x4498, 0x05},
    {0x449C, 0x19},
    {0x449D, 0x00},
    {0x449E, 0x32},
    {0x449F, 0x01},
    {0x44A0, 0x92},
    {0x44A2, 0x91},
    {0x44A4, 0x8C},
    {0x44A6, 0x87},
    {0x44A8, 0x82},
    {0x44AA, 0x78},
    {0x44AC, 0x6E},
    {0x44AE, 0x69},
    {0x44B0, 0x92},
    {0x44B2, 0x91},
    {0x44B4, 0x8C},
    {0x44B6, 0x87},
    {0x44B8, 0x82},
    {0x44BA, 0x78},
    {0x44BC, 0x6E},
    {0x44BE, 0x69},
    {0x44C0, 0x7F},
    {0x44C1, 0x01},
    {0x44C2, 0x7F},
    {0x44C3, 0x01},
    {0x44C4, 0x7A},
    {0x44C5, 0x01},
    {0x44C6, 0x7A},
    {0x44C7, 0x01},
    {0x44C8, 0x70},
    {0x44C9, 0x01},
    {0x44CA, 0x6B},
    {0x44CB, 0x01},
    {0x44CC, 0x6B},
    {0x44CD, 0x01},
    {0x44CE, 0x5C},
    {0x44CF, 0x01},
    {0x44D0, 0x7F},
    {0x44D1, 0x01},
    {0x44D2, 0x7F},
    {0x44D3, 0x01},
    {0x44D4, 0x7A},
    {0x44D5, 0x01},
    {0x44D6, 0x7A},
    {0x44D7, 0x01},
    {0x44D8, 0x70},
    {0x44D9, 0x01},
    {0x44DA, 0x6B},
    {0x44DB, 0x01},
    {0x44DC, 0x6B},
    {0x44DD, 0x01},
    {0x44DE, 0x5C},
    {0x44DF, 0x01},
    {0x4534, 0x1C},
    {0x4535, 0x03},
    {0x4538, 0x1C},
    {0x4539, 0x1C},
    {0x453A, 0x1C},
    {0x453B, 0x1C},
    {0x453C, 0x1C},
    {0x453D, 0x1C},
    {0x453E, 0x1C},
    {0x453F, 0x1C},
    {0x4540, 0x1C},
    {0x4541, 0x03},
    {0x4542, 0x03},
    {0x4543, 0x03},
    {0x4544, 0x03},
    {0x4545, 0x03},
    {0x4546, 0x03},
    {0x4547, 0x03},
    {0x4548, 0x03},
    {0x4549, 0x03},
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
//8M20 594Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_2m_20fps_init_table_2lane_linear[] =
{
    /*
    All-pixel scan
    CSI-2_2lane
    74.25MHz
    AD:12bit Output:12bit
    720Mbps
    Master Mode
    LCG Mode
    25fps
    Integration Time
    39.872ms
    */
    {0x3000, 0x01},    //Standby
    {0xffff, 0x14},//delay
    {0x3002, 0x01},    //Master mode stop
    {0xffff, 0x14},//delay
    {0x3015, 0x06},
    {0x302C, 0x48},
    {0x302D, 0x09},
    {0x3040, 0x01},
    {0x3050, 0x04},
    {0x30A6, 0x00},
    {0x3444, 0xAC},
    {0x3460, 0x21},
    {0x3492, 0x08},
    {0x3B00, 0x39},
    {0x3B23, 0x2D},
    {0x3B45, 0x04},
    {0x3C0A, 0x1F},
    {0x3C0B, 0x1E},
    {0x3C38, 0x21},
    {0x3C44, 0x00},
    {0x3CB6, 0xD8},
    {0x3CC4, 0xDA},
    {0x3E24, 0x79},
    {0x3E2C, 0x15},
    {0x3EDC, 0x2D},
    {0x4498, 0x05},
    {0x449C, 0x19},
    {0x449D, 0x00},
    {0x449E, 0x32},
    {0x449F, 0x01},
    {0x44A0, 0x92},
    {0x44A2, 0x91},
    {0x44A4, 0x8C},
    {0x44A6, 0x87},
    {0x44A8, 0x82},
    {0x44AA, 0x78},
    {0x44AC, 0x6E},
    {0x44AE, 0x69},
    {0x44B0, 0x92},
    {0x44B2, 0x91},
    {0x44B4, 0x8C},
    {0x44B6, 0x87},
    {0x44B8, 0x82},
    {0x44BA, 0x78},
    {0x44BC, 0x6E},
    {0x44BE, 0x69},
    {0x44C0, 0x7F},
    {0x44C1, 0x01},
    {0x44C2, 0x7F},
    {0x44C3, 0x01},
    {0x44C4, 0x7A},
    {0x44C5, 0x01},
    {0x44C6, 0x7A},
    {0x44C7, 0x01},
    {0x44C8, 0x70},
    {0x44C9, 0x01},
    {0x44CA, 0x6B},
    {0x44CB, 0x01},
    {0x44CC, 0x6B},
    {0x44CD, 0x01},
    {0x44CE, 0x5C},
    {0x44CF, 0x01},
    {0x44D0, 0x7F},
    {0x44D1, 0x01},
    {0x44D2, 0x7F},
    {0x44D3, 0x01},
    {0x44D4, 0x7A},
    {0x44D5, 0x01},
    {0x44D6, 0x7A},
    {0x44D7, 0x01},
    {0x44D8, 0x70},
    {0x44D9, 0x01},
    {0x44DA, 0x6B},
    {0x44DB, 0x01},
    {0x44DC, 0x6B},
    {0x44DD, 0x01},
    {0x44DE, 0x5C},
    {0x44DF, 0x01},
    {0x4534, 0x1C},
    {0x4535, 0x03},
    {0x4538, 0x1C},
    {0x4539, 0x1C},
    {0x453A, 0x1C},
    {0x453B, 0x1C},
    {0x453C, 0x1C},
    {0x453D, 0x1C},
    {0x453E, 0x1C},
    {0x453F, 0x1C},
    {0x4540, 0x1C},
    {0x4541, 0x03},
    {0x4542, 0x03},
    {0x4543, 0x03},
    {0x4544, 0x03},
    {0x4545, 0x03},
    {0x4546, 0x03},
    {0x4547, 0x03},
    {0x4548, 0x03},
    {0x4549, 0x03},
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_init_table_2lane_HDR_DOL[] =
{
    /*
    All-pixel scan
    CSI-2_2lane
    74.25MHz
    AD:10bit Output:10bit
    891Mbps
    Master Mode
    LCG Mode
    DOL HDR 2frame VC
    30fps
    Integration Time
    LEF:20ms
    SEF:2.027ms

    */
    {0x3000, 0x01},    //Standby
    {0xffff, 0x14},//delay
    {0x3002, 0x01},    //Master mode stop
    {0xffff, 0x14},//delay
    {0x3015, 0x05},
    {0x301A, 0x01},
    {0x301C, 0x01},
    {0x3022, 0x00},
    {0x3023, 0x00},
    {0x3040, 0x01},
    {0x3050, 0xE8},
    {0x3051, 0x03},
    {0x3054, 0x05},
    {0x3060, 0x9D},
    {0x3061, 0x00},
    {0x30A6, 0x00},
    {0x3400, 0x00},
    {0x3444, 0xAC},
    {0x3460, 0x21},
    {0x3492, 0x08},
    {0x3A50, 0x62},
    {0x3A51, 0x01},
    {0x3A52, 0x19},
    {0x3B00, 0x39},
    {0x3B23, 0x2D},
    {0x3B45, 0x04},
    {0x3C0A, 0x1F},
    {0x3C0B, 0x1E},
    {0x3C38, 0x21},
    {0x3C44, 0x00},
    {0x3CB6, 0xD8},
    {0x3CC4, 0xDA},
    {0x3E24, 0x79},
    {0x3E2C, 0x15},
    {0x3EDC, 0x2D},
    {0x4498, 0x05},
    {0x449C, 0x19},
    {0x449D, 0x00},
    {0x449E, 0x32},
    {0x449F, 0x01},
    {0x44A0, 0x92},
    {0x44A2, 0x91},
    {0x44A4, 0x8C},
    {0x44A6, 0x87},
    {0x44A8, 0x82},
    {0x44AA, 0x78},
    {0x44AC, 0x6E},
    {0x44AE, 0x69},
    {0x44B0, 0x92},
    {0x44B2, 0x91},
    {0x44B4, 0x8C},
    {0x44B6, 0x87},
    {0x44B8, 0x82},
    {0x44BA, 0x78},
    {0x44BC, 0x6E},
    {0x44BE, 0x69},
    {0x44C0, 0x7F},
    {0x44C1, 0x01},
    {0x44C2, 0x7F},
    {0x44C3, 0x01},
    {0x44C4, 0x7A},
    {0x44C5, 0x01},
    {0x44C6, 0x7A},
    {0x44C7, 0x01},
    {0x44C8, 0x70},
    {0x44C9, 0x01},
    {0x44CA, 0x6B},
    {0x44CB, 0x01},
    {0x44CC, 0x6B},
    {0x44CD, 0x01},
    {0x44CE, 0x5C},
    {0x44CF, 0x01},
    {0x44D0, 0x7F},
    {0x44D1, 0x01},
    {0x44D2, 0x7F},
    {0x44D3, 0x01},
    {0x44D4, 0x7A},
    {0x44D5, 0x01},
    {0x44D6, 0x7A},
    {0x44D7, 0x01},
    {0x44D8, 0x70},
    {0x44D9, 0x01},
    {0x44DA, 0x6B},
    {0x44DB, 0x01},
    {0x44DC, 0x6B},
    {0x44DD, 0x01},
    {0x44DE, 0x5C},
    {0x44DF, 0x01},
    {0x4534, 0x1C},
    {0x4535, 0x03},
    {0x4538, 0x1C},
    {0x4539, 0x1C},
    {0x453A, 0x1C},
    {0x453B, 0x1C},
    {0x453C, 0x1C},
    {0x453D, 0x1C},
    {0x453E, 0x1C},
    {0x453F, 0x1C},
    {0x4540, 0x1C},
    {0x4541, 0x03},
    {0x4542, 0x03},
    {0x4543, 0x03},
    {0x4544, 0x03},
    {0x4545, 0x03},
    {0x4546, 0x03},
    {0x4547, 0x03},
    {0x4548, 0x03},
    {0x4549, 0x03},

    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    //{0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_2m_20fps_init_table_2lane_HDR_DOL3[] =
{
    /*
        All-pixel scan
        CSI-2_2lane
        74.25MHz
        AD:10bit Output:10bit
        891Mbps
        Master Mode
        LCG Mode
        DOL HDR 3frame VC
        20fps
        Integration Time
        LEF:20ms
        SEF1:2ms
        SEF2:0.24ms

    */
    {0x3000, 0x01},    //Standby
    {0xffff, 0x14},//delay
    {0x3002, 0x01},    //Master mode stop
    {0xffff, 0x14},//delay
    {0x3015, 0x05},
    {0x301A, 0x02},
    {0x301C, 0x01},
    {0x3022, 0x00},
    {0x3023, 0x00},
    {0x3040, 0x01},
    {0x3050, 0xCA},
    {0x3051, 0x08},
    {0x3054, 0x07},
    {0x3058, 0xA7},
    {0x3059, 0x00},
    {0x3060, 0x9D},
    {0x3061, 0x00},
    {0x3064, 0xB9},
    {0x3065, 0x00},
    {0x30A6, 0x00},
    {0x3400, 0x00},
    {0x3444, 0xAC},
    {0x3460, 0x21},
    {0x3492, 0x08},
    {0x3A50, 0x62},
    {0x3A51, 0x01},
    {0x3A52, 0x19},
    {0x3B00, 0x39},
    {0x3B23, 0x2D},
    {0x3B45, 0x04},
    {0x3C0A, 0x1F},
    {0x3C0B, 0x1E},
    {0x3C38, 0x21},
    {0x3C44, 0x00},
    {0x3CB6, 0xD8},
    {0x3CC4, 0xDA},
    {0x3E24, 0x79},
    {0x3E2C, 0x15},
    {0x3EDC, 0x2D},
    {0x4498, 0x05},
    {0x449C, 0x19},
    {0x449D, 0x00},
    {0x449E, 0x32},
    {0x449F, 0x01},
    {0x44A0, 0x92},
    {0x44A2, 0x91},
    {0x44A4, 0x8C},
    {0x44A6, 0x87},
    {0x44A8, 0x82},
    {0x44AA, 0x78},
    {0x44AC, 0x6E},
    {0x44AE, 0x69},
    {0x44B0, 0x92},
    {0x44B2, 0x91},
    {0x44B4, 0x8C},
    {0x44B6, 0x87},
    {0x44B8, 0x82},
    {0x44BA, 0x78},
    {0x44BC, 0x6E},
    {0x44BE, 0x69},
    {0x44C0, 0x7F},
    {0x44C1, 0x01},
    {0x44C2, 0x7F},
    {0x44C3, 0x01},
    {0x44C4, 0x7A},
    {0x44C5, 0x01},
    {0x44C6, 0x7A},
    {0x44C7, 0x01},
    {0x44C8, 0x70},
    {0x44C9, 0x01},
    {0x44CA, 0x6B},
    {0x44CB, 0x01},
    {0x44CC, 0x6B},
    {0x44CD, 0x01},
    {0x44CE, 0x5C},
    {0x44CF, 0x01},
    {0x44D0, 0x7F},
    {0x44D1, 0x01},
    {0x44D2, 0x7F},
    {0x44D3, 0x01},
    {0x44D4, 0x7A},
    {0x44D5, 0x01},
    {0x44D6, 0x7A},
    {0x44D7, 0x01},
    {0x44D8, 0x70},
    {0x44D9, 0x01},
    {0x44DA, 0x6B},
    {0x44DB, 0x01},
    {0x44DC, 0x6B},
    {0x44DD, 0x01},
    {0x44DE, 0x5C},
    {0x44DF, 0x01},
    {0x4534, 0x1C},
    {0x4535, 0x03},
    {0x4538, 0x1C},
    {0x4539, 0x1C},
    {0x453A, 0x1C},
    {0x453B, 0x1C},
    {0x453C, 0x1C},
    {0x453D, 0x1C},
    {0x453E, 0x1C},
    {0x453F, 0x1C},
    {0x4540, 0x1C},
    {0x4541, 0x03},
    {0x4542, 0x03},
    {0x4543, 0x03},
    {0x4544, 0x03},
    {0x4545, 0x03},
    {0x4546, 0x03},
    {0x4547, 0x03},
    {0x4548, 0x03},
    {0x4549, 0x03},
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    //{0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_2m_30fps_init_table_2lane_HDR_DOL3[] =
{
    /*
        All-pixel scan
        CSI-2_2lane
        74.25MHz
        AD:10bit Output:10bit
        1440Mbps
        Master Mode
        LCG Mode
        DOL HDR 3frame VC
        30fps
        Integration Time
        LEF:20ms
        SEF1:2ms
        SEF2:0.213ms

    */
    {0x3000, 0x01},    //Standby
    {0xffff, 0x14},//delay
    {0x3002, 0x01},    //Master mode stop
    {0xffff, 0x14},//delay
    {0x3015, 0x03},
    {0x301A, 0x02},
    {0x301C, 0x01},
    {0x3022, 0x00},
    {0x3023, 0x00},
    {0x302C, 0x94},
    {0x302D, 0x02},
    {0x3040, 0x01},
    {0x3050, 0xDC},
    {0x3051, 0x05},
    {0x3054, 0x07},
    {0x3058, 0xF2},
    {0x3059, 0x00},
    {0x3060, 0xE8},
    {0x3061, 0x00},
    {0x3064, 0x0A},
    {0x3065, 0x01},
    {0x30A6, 0x00},
    {0x3400, 0x00},
    {0x3444, 0xAC},
    {0x3460, 0x21},
    {0x3492, 0x08},
    {0x3A50, 0x62},
    {0x3A51, 0x01},
    {0x3A52, 0x19},
    {0x3B00, 0x39},
    {0x3B23, 0x2D},
    {0x3B45, 0x04},
    {0x3C0A, 0x1F},
    {0x3C0B, 0x1E},
    {0x3C38, 0x21},
    {0x3C44, 0x00},
    {0x3CB6, 0xD8},
    {0x3CC4, 0xDA},
    {0x3E24, 0x79},
    {0x3E2C, 0x15},
    {0x3EDC, 0x2D},
    {0x4498, 0x05},
    {0x449C, 0x19},
    {0x449D, 0x00},
    {0x449E, 0x32},
    {0x449F, 0x01},
    {0x44A0, 0x92},
    {0x44A2, 0x91},
    {0x44A4, 0x8C},
    {0x44A6, 0x87},
    {0x44A8, 0x82},
    {0x44AA, 0x78},
    {0x44AC, 0x6E},
    {0x44AE, 0x69},
    {0x44B0, 0x92},
    {0x44B2, 0x91},
    {0x44B4, 0x8C},
    {0x44B6, 0x87},
    {0x44B8, 0x82},
    {0x44BA, 0x78},
    {0x44BC, 0x6E},
    {0x44BE, 0x69},
    {0x44C0, 0x7F},
    {0x44C1, 0x01},
    {0x44C2, 0x7F},
    {0x44C3, 0x01},
    {0x44C4, 0x7A},
    {0x44C5, 0x01},
    {0x44C6, 0x7A},
    {0x44C7, 0x01},
    {0x44C8, 0x70},
    {0x44C9, 0x01},
    {0x44CA, 0x6B},
    {0x44CB, 0x01},
    {0x44CC, 0x6B},
    {0x44CD, 0x01},
    {0x44CE, 0x5C},
    {0x44CF, 0x01},
    {0x44D0, 0x7F},
    {0x44D1, 0x01},
    {0x44D2, 0x7F},
    {0x44D3, 0x01},
    {0x44D4, 0x7A},
    {0x44D5, 0x01},
    {0x44D6, 0x7A},
    {0x44D7, 0x01},
    {0x44D8, 0x70},
    {0x44D9, 0x01},
    {0x44DA, 0x6B},
    {0x44DB, 0x01},
    {0x44DC, 0x6B},
    {0x44DD, 0x01},
    {0x44DE, 0x5C},
    {0x44DF, 0x01},
    {0x4534, 0x1C},
    {0x4535, 0x03},
    {0x4538, 0x1C},
    {0x4539, 0x1C},
    {0x453A, 0x1C},
    {0x453B, 0x1C},
    {0x453C, 0x1C},
    {0x453D, 0x1C},
    {0x453E, 0x1C},
    {0x453F, 0x1C},
    {0x4540, 0x1C},
    {0x4541, 0x03},
    {0x4542, 0x03},
    {0x4543, 0x03},
    {0x4544, 0x03},
    {0x4545, 0x03},
    {0x4546, 0x03},
    {0x4547, 0x03},
    {0x4548, 0x03},
    {0x4549, 0x03},
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    //{0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_id_table[] = {
    {0x3F12, 0x14},      // {address of ID, ID },
    {0x3F13, 0x75},      // {address of ID, ID },
};

static I2C_ARRAY PatternTbl[] = {
    {0x0000,0x00},       // colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY expo_reg[] = {      // SHS0 (For Linear)
    {0x3052, 0x00},
    {0x3051, 0x00},
    {0x3050, 0x08},
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

const static I2C_ARRAY vts_reg[] = {       //VMAX
    {0x3026, 0x00}, //bit0-3-->MSB
    {0x3025, 0x08},
    {0x3024, 0xCA},
};

const static I2C_ARRAY vts_reg_hdr[] = {
    {0x3026, 0x00}, //bit0-3-->MSB
    {0x3025, 0x0B},
    {0x3024, 0x3B},
};

const I2C_ARRAY expo_IMX662_RHS1_reg[] = {
    //decreasing exposure ratio version.
    {0x3062, 0x00},
    {0x3061, 0x00},
    {0x3060, 0x11},
};

const static I2C_ARRAY gain_reg[] = {
    {0x3070, 0x2A},//low bit
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

//static int g_sensor_ae_min_gain = 1024;
static CUS_GAIN_GAP_ARRAY gain_gap_compensate[16] = {  //compensate  gain gap
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

/////////////////// I2C function definition ///////////////////
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent ///////////////////
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

/*******I5/I6 Support MCLK List*******
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
 #if 0
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
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    imx662_params *params = (imx662_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode==CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_step                  = Preview_line_period_HDR_DOL_2LANE;
        info->u32AEShutter_min                   = Preview_line_period_HDR_DOL_2LANE * 5;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num > 0)
        {
            info->u32AEShutter_max                   = Preview_line_period_HDR_DOL_2LANE * params->min_rhs1;
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
    imx662_params *params = (imx662_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    //sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_384M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

    sensor_if->Reset(idx, params->reset_POLARITY );
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_UDELAY(20); //TLOW

    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_UDELAY(20);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_UDELAY(20);

    sensor_if->Reset(idx, !params->reset_POLARITY );
    SENSOR_UDELAY(20); //TXCE

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, params->reset_POLARITY);     // Rst Pull Low
    SENSOR_MSLEEP(1);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);   // Powerdn Pull Low
    SENSOR_MSLEEP(1);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
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
static int IMX662_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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

static int pCus_init_mipi2lane_linear(ss_cus_sensor *handle)
{
    //imx662_params *params = (imx662_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2lane_linear);i++)
    {
        if(Sensor_init_table_2lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2lane_linear[i].reg, Sensor_init_table_2lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
            //printk("\n reg 0x%x, 0x%x",Sensor_init_table_4lane_linear[i].reg, Sensor_init_table_4lane_linear[i].data);
#if 0
            SensorReg_Read(Sensor_init_table_4lane_linear[i].reg, &sen_data );
            if(Sensor_init_table_4lane_linear[i].data != sen_data)
                printk("R/W Differ Reg: 0x%x\n",Sensor_init_table_4lane_linear[i].reg);
                //printk("IMX662 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_4lane_linear[i].reg, Sensor_init_table_4lane_linear[i].data, sen_data);
#endif
        }
    }

    return SUCCESS;
}

static int pCus_init_2m_20fps_mipi2lane_linear(ss_cus_sensor *handle)
{
    //imx662_params *params = (imx662_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_2m_20fps_init_table_2lane_linear);i++)
    {
        if(Sensor_2m_20fps_init_table_2lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_2m_20fps_init_table_2lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_2m_20fps_init_table_2lane_linear[i].reg, Sensor_2m_20fps_init_table_2lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
            //printk("\n reg 0x%x, 0x%x",Sensor_2m_20fps_init_table_4lane_linear[i].reg, Sensor_2m_20fps_init_table_4lane_linear[i].data);
#if 0
            SensorReg_Read(Sensor_2m_20fps_init_table_4lane_linear[i].reg, &sen_data );
            if(Sensor_2m_20fps_init_table_4lane_linear[i].data != sen_data)
                printk("R/W Differ Reg: 0x%x\n",Sensor_2m_20fps_init_table_4lane_linear[i].reg);
                //printk("IMX662 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_2m_20fps_init_table_4lane_linear[i].reg, Sensor_2m_20fps_init_table_4lane_linear[i].data, sen_data);
#endif
        }
    }

    return SUCCESS;
}

static int pCus_init_mipi2lane_HDR_DOL(ss_cus_sensor *handle)
{
    //imx662_params *params = (imx662_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2lane_HDR_DOL);i++)
    {
        if(Sensor_init_table_2lane_HDR_DOL[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2lane_HDR_DOL[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2lane_HDR_DOL[i].reg,Sensor_init_table_2lane_HDR_DOL[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
            //printk("\n reg 0x%x, 0x%x",Sensor_init_table_2lane_HDR_DOL[i].reg, Sensor_init_table_2lane_HDR_DOL[i].data);
#if 0
            SensorReg_Read(Sensor_init_table_2lane_HDR_DOL[i].reg, &sen_data );
            if(Sensor_init_table_4lane_linear[i].data != sen_data)
                printk("HDR R/W Differ Reg: 0x%x\n",Sensor_init_table_2lane_HDR_DOL[i].reg);
                //printk("IMX662 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_2lane_HDR_DOL[i].reg, Sensor_init_table_2lane_HDR_DOL[i].data, sen_data);
#endif
        }
    }

    SENSOR_DMSG("Sensor IMX662 HDR MODE Initial Finished\n");
    return SUCCESS;
}

static int pCus_init_2m_20fps_mipi2lane_HDR_DOL3(ss_cus_sensor *handle)
{
    int i,cnt=0;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_2m_20fps_init_table_2lane_HDR_DOL3);i++)
    {
        if(Sensor_2m_20fps_init_table_2lane_HDR_DOL3[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_2m_20fps_init_table_2lane_HDR_DOL3[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_2m_20fps_init_table_2lane_HDR_DOL3[i].reg,Sensor_2m_20fps_init_table_2lane_HDR_DOL3[i].data) != SUCCESS)
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

static int pCus_init_2m_30fps_mipi2lane_HDR_DOL3(ss_cus_sensor *handle)
{
    int i,cnt=0;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_2m_30fps_init_table_2lane_HDR_DOL3);i++)
    {
        if(Sensor_2m_30fps_init_table_2lane_HDR_DOL3[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_2m_30fps_init_table_2lane_HDR_DOL3[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_2m_30fps_init_table_2lane_HDR_DOL3[i].reg,Sensor_2m_30fps_init_table_2lane_HDR_DOL3[i].data) != SUCCESS)
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
    imx662_params *params = (imx662_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_2m_20fps_mipi2lane_linear;
            vts_30fps = 1250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 20;
            Preview_line_period = 17778; // 49.86ms/2813 = 17725ns
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 1:
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_2m_20fps_mipi2lane_linear;
            vts_30fps = 1250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 14815;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;

        default:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_2m_20fps_mipi2lane_linear;
            vts_30fps = 1250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 20;
            Preview_line_period = 17725; // 49.86ms/2813 = 17725ns
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
    }

    params->tVts_reg[0].data = (vts_30fps >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts_30fps >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts_30fps >> 0) & 0x00ff;

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

    switch (res_idx) {
        case HDR_RES_1:
            handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_LEF;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case HDR_RES_2:
        case HDR_RES_3:
            handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_LEF3;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        default:
            break;
    }
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_SEF(ss_cus_sensor *handle, u32 res_idx)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case HDR_RES_1:
            handle->pCus_sensor_init = pCus_init_mipi2lane_HDR_DOL;
            vts_30fps_HDR_DOL_2lane = 1250;
            params->expo.vts = vts_30fps_HDR_DOL_2lane;
            params->expo.fps = 15;
            Preview_line_period_HDR_DOL_2LANE = 26672;
            handle->data_prec = CUS_DATAPRECISION_10;
            params->min_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_2:
            handle->pCus_sensor_init = pCus_init_2m_20fps_mipi2lane_HDR_DOL3;
            vts_30fps_HDR_DOL_2lane = 1250;
            params->expo.vts = vts_30fps_HDR_DOL_2lane;
            params->expo.fps = 20;
            Preview_line_period_HDR_DOL_2LANE = 26672;
            handle->data_prec = CUS_DATAPRECISION_10;
            params->min_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_3:
            handle->pCus_sensor_init = pCus_init_2m_30fps_mipi2lane_HDR_DOL3;
            vts_30fps_HDR_DOL_2lane = 1250;
            params->expo.vts = vts_30fps_HDR_DOL_2lane;
            params->expo.fps = 30;
            Preview_line_period_HDR_DOL_2LANE = 26672;
            handle->data_prec = CUS_DATAPRECISION_10;
            params->min_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429
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
    imx662_params *params = (imx662_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx662_params *params = (imx662_params *)handle->private_data;
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
    imx662_params *params = (imx662_params *)handle->private_data;
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
    //u32 vts = 0, cur_vts_30fps = 0;
    imx662_params *params = (imx662_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    //pr_info("[%s]  leslie_fps,maxfps,minfps : %d,%d,%d\n\n", __FUNCTION__,fps,max_fps,min_fps);
    //cur_vts_30fps = vts_30fps;
    //pr_info("[%s]  leslie_vts_30fps : %u\n\n", __FUNCTION__,vts_30fps);
    if(fps>=min_fps && fps <= max_fps){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps>29?29:fps;              //limit fps at 29 fps due to MCLK=36MHz
            params->expo.vts=  (vts_30fps*29091 + fps * 500)/(fps * 1000);
        }
        else
            params->expo.vts=  (vts_30fps*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps>29091?29091:fps;       //limit fps at 29.091 fps due to MCLK=36MHz
            params->expo.vts=  (vts_30fps*29091 + (fps>>1))/fps;
        }
        else
            params->expo.vts=  (vts_30fps*(max_fps*1000) + (fps>>1))/fps;
    }else{
      SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }
    //pr_info("[%s]  leslie_vts : %u\n\n", __FUNCTION__,params->expo.vts);
    if(params->expo.expo_lines > params->expo.vts - 4){
        //vts = params->expo.expo_lines + 4;
#if 0        //Update FPS Status
        if(fps>=3 && fps <= 30)
            fps = (vts_30fps*30000)/(params->expo.vts * 1000 - 500);
        else if(fps>=3000 && fps <= 30000)
            fps = (vts_30fps*30000)/(params->expo.vts - (500 / 1000));
#endif
    }else{
        //vts = params->expo.vts;
    }

    params->expo.fps = fps;
    params->dirty = true;

    pCus_SetAEUSecs(handle, params->expo.expo_lef_us);

    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_SEF(ss_cus_sensor *handle)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_2lane;
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
    imx662_params *params = (imx662_params *)handle->private_data;
       u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    //cur_vts_30fps = imx662_mipi_hdr[0].senout.height;
    cur_vts_30fps=vts_30fps_HDR_DOL_2lane;
    if(fps>=min_fps && fps <= max_fps){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps > 14 ? 14 : fps;//limit fps at 29 fps due to MCLK=36MHz
            params->expo.vts= (cur_vts_30fps*14545 + fps * 500 )/ (fps * 1000);
        }
        else
            params->expo.vts= (cur_vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);

    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps > 14545 ? 14545 : fps;//limit fps at 29.091 fps due to MCLK=36MHz
            params->expo.vts= (cur_vts_30fps*14545 + (fps >> 1))/fps;
        }
        else
            params->expo.vts= (cur_vts_30fps*(max_fps*1000) + (fps >> 1))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.expo_lines > params->expo.vts - 4){
        //vts = params->expo.expo_lines + 4;
#if 0        //Update FPS Status
        if(fps>=3 && fps <= 30)
            fps = (vts_30fps*30000)/(params->expo.vts * 1000 - 500);
        else if(fps>=3000 && fps <= 30000)
            fps = (vts_30fps*30000)/(params->expo.vts - (500 / 1000));
#endif
    }else{
        //vts = params->expo.vts;
    }

    params->expo.fps = fps;
    //params->expo.vts = vts;
    params->dirty = true;

    pCus_SetAEUSecsHDR_DOL_SEF(handle, params->expo.expo_sef_us);

    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    //ISensorIfAPI2 *sensor_if1 = handle->sensor_if_api2;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1); // Global hold on
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
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
    //imx662_params *params = (imx662_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
#if 0
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1);

                if(params->dirty) {
                    //SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                    //SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                    //SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x3001,0);
            }
#endif
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    u32 lines = 0;
    imx662_params *params = (imx662_params *)handle->private_data;

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
    imx662_params *params = (imx662_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000*us)/Preview_line_period;

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

    params->expo.expo_lines = expo_lines;
    //params->expo.vts = vts;

    SENSOR_DMSG("[%s] us %u, SHR0 %u, vts %u\n", __FUNCTION__,
                                                 us,  \
                                                 SHR0, \
                                                 vts
               );
    //pr_info("[%s]  leslie_shutter,expo_lines,params_expo_lines : %d,%d,%d\n\n", __FUNCTION__,us,expo_lines,params->expo.expo_lines);
    //pr_info("[%s]  leslie_shutter_vts : %u,%u\n\n", __FUNCTION__,params->expo.vts,vts);
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
    u32 cur_line_period = Preview_line_period_HDR_DOL_2LANE;
    //u32 cur_vts_30fps = vts_30fps_HDR_DOL_2lane;
    imx662_params *params = (imx662_params *)handle->private_data;

    cur_line_period = Preview_line_period_HDR_DOL_2LANE;
    //cur_vts_30fps =vts_30fps_HDR_DOL_2lane;
    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    params->expo.expo_sef_us = us;
    //params->min_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429

    params->min_shr1 = params->min_rhs1 - expo_line_sef;
    if((expo_line_sef > params->min_rhs1) || ((params->min_shr1) <  9))
        params->min_shr1 = 9;
    params->min_shr1 = ((params->min_shr1 >> 1) << 1) + 1;

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

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx662_params *params = (imx662_params *)handle->private_data;
#if 0
    u16 temp_gain;

    temp_gain=gain_reg[0].data;
    *gain=(u32)(10^((temp_gain*3)/200))*1024;
    if (gain_reg[1].data & 0x10)
       *gain = (*gain) * 2;
#endif
    *gain = params->expo.final_gain;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    u32 i;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;
    u64 gain_double;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    Sensor_Gain_Linearity = gain_gap_compensate;

    for(i = 0; i < sizeof(gain_gap_compensate)/sizeof(CUS_GAIN_GAP_ARRAY); i++){
        if (Sensor_Gain_Linearity[i].gain == 0)
            break;
        if((gain>Sensor_Gain_Linearity[i].gain) && (gain < (Sensor_Gain_Linearity[i].gain + Sensor_Gain_Linearity[i].offset))){
              gain=Sensor_Gain_Linearity[i].gain;
              break;
        }
    }
    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    gain_double = (u16)((gain_double * 10) >> 24) / 3;

    params->tGain_reg[0].data = gain_double & 0xff;
    params->tGain_reg[1].data = (gain_double >> 8) & 0xff;

#if DEBUG_INFO
    SENSOR_DMSG("[%s]gain %u gain_double %llu\n",__FUNCTION__, gain, gain_double);
#endif

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
    imx662_params *params = (imx662_params *)handle->private_data;
    u16 gain_reg = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_sef_reg[0].data = gain_reg & 0xff;
    params->tGain_hdr_dol_sef_reg[1].data = (gain_reg >> 8) & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data, params->tGain_hdr_dol_sef_reg[1].data);

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
    imx662_params *params = (imx662_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_LEF(ss_cus_sensor *handle)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_2lane;
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
    imx662_params *params = (imx662_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
   // cur_vts_30fps = imx662_mipi_hdr[0].senout.height;
    cur_vts_30fps=vts_30fps_HDR_DOL_2lane;
    if(fps>=min_fps && fps <= max_fps){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps>14?14:fps;//limit fps at 29 fps due to MCLK=36MHz
            params->expo.vts=  (cur_vts_30fps*14545 + fps * 500 )/ (fps * 1000);
        }
        else
            params->expo.vts=  (cur_vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);

    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps>14?14:fps;//limit fps at 29.091 fps due to MCLK=36MHz
            params->expo.vts=  (cur_vts_30fps*14545 + (fps >> 1))/fps;
        }
        else
            params->expo.vts=  (cur_vts_30fps*(max_fps*1000) + (fps >> 1))/fps;

    }else {
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.expo_lines > params->expo.vts - 4) {
        //vts = params->expo.expo_lines + 4;
    }else {
        //vts = params->expo.vts;
    }

    params->dirty = true;
    params->expo.fps = fps;
    pCus_SetAEUSecsHDR_DOL_LEF(handle, params->expo.expo_sef_us);

    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

             break;
        case CUS_FRAME_ACTIVE:

            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1);
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg_hdr));
                    SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tSHR1_reg, ARRAY_SIZE(expo_SHR1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tRHS1_reg, ARRAY_SIZE(expo_IMX662_RHS1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                    params->skip_cnt = true;
                }
                SensorReg_Write(0x3001,0);
            }
            if(params->skip_cnt){
                sensor_if->SetSkipFrame(idx, params->expo.fps, params->skip_cnt);
                params->skip_cnt = 0;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_LEF3(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

             break;
        case CUS_FRAME_ACTIVE:

            if((params->dirty || params->orien_dirty)&& 0) {
                SensorReg_Write(0x3001,1);
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg_hdr));
                    SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tSHR1_reg, ARRAY_SIZE(expo_SHR1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tRHS1_reg, ARRAY_SIZE(expo_IMX662_RHS1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                    params->skip_cnt = true;
                }
                SensorReg_Write(0x3001,0);
            }
            if(params->skip_cnt){
                sensor_if->SetSkipFrame(idx, params->expo.fps, params->skip_cnt);
                params->skip_cnt = 0;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *us)
{
    *us = 0;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines_lef = 0, vts = 0, fsc = 0;
    u32 cur_line_period = Preview_line_period_HDR_DOL_2LANE;
    //u32 cur_vts_30fps = vts_30fps_HDR_DOL_2lane;
    imx662_params *params = (imx662_params *)handle->private_data;
    cur_line_period = Preview_line_period_HDR_DOL_2LANE;
    //cur_vts_30fps =vts_30fps_HDR_DOL_2lane;
    expo_lines_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;

    params->expo.expo_lef_us = us;
    fsc = params->expo.vts * 2;
    params->fsc = ((fsc >> 2) << 2)+ 4;                  // 4n

    //params->max_shr0 = fsc - expo_lines_lef - 8;
    params->max_shr0 = (fsc - 8) - expo_lines_lef;
    if(params->max_shr0 < (params->min_rhs1+9))
        params->max_shr0 = params->min_rhs1+9;
    params->max_shr0 = ((params->max_shr0 >> 1) << 1) + 2;

    if (expo_lines_lef > (fsc - params->min_rhs1 - 9)) {
        vts = (expo_lines_lef + params->min_rhs1 + 9) / 2;
    }
    else{
      vts = params->expo.vts;
    }
    params->expo.expo_lines = expo_lines_lef;
    //params->expo.vts = vts;

    SENSOR_DMSG("[%s] us %u, expo_lines_lef %u, vts %u, SHR0 %u \n", __FUNCTION__,
                                                                     us, \
                                                                     expo_lines_lef, \
                                                                     vts, \
                                                                     params->max_shr0
                );
    params->tSHR0_reg[0].data = (params->max_shr0 >> 16) & 0x0003;
    params->tSHR0_reg[1].data = (params->max_shr0 >> 8) & 0x00ff;
    params->tSHR0_reg[2].data = (params->max_shr0 >> 0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEGain_HDR_DOL_LEF(ss_cus_sensor *handle, u32* gain)
{
    *gain = 0;
    return SUCCESS;
}

static int pCus_SetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    imx662_params *params = (imx662_params *)handle->private_data;
    u16 gain_reg = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_lef_reg[0].data = gain_reg & 0xff;
    params->tGain_hdr_dol_lef_reg[1].data = (gain_reg >> 8) & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data, params->tGain_hdr_dol_lef_reg[1].data);

    params->dirty = true;
    return SUCCESS;
}

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx662_params *params;
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

    params = (imx662_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX662_MIPI");

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
        handle->video_res_supported.res[res].u16width             = IMX662_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = IMX662_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = IMX662_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = IMX662_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = IMX662_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = IMX662_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = IMX662_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = IMX662_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, IMX662_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi2lane_linear;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = IMX662_SetPatternMode;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/IMX662_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps;
    params->dirty                                                 = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx662_params *params = NULL;
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

    params = (imx662_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tRHS1_reg, expo_IMX662_RHS1_reg, sizeof(expo_IMX662_RHS1_reg));
    memcpy(params->tSHR1_reg, expo_SHR1_reg, sizeof(expo_SHR1_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"imx662_mipi_hdr_SEF");

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
        handle->video_res_supported.res[res].u16width             = imx662_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx662_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx662_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx662_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx662_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx662_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx662_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx662_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx662_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi2lane_HDR_DOL;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL_SEF;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_SEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_SEF;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_SEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_SEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_SEF1;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_2LANE * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR_DOL_2LANE * params->min_rhs1;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_2LANE;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps_HDR_DOL_2lane;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx662_params *params;
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
    params = (imx662_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"imx662_mipi_hdr_LEF");

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
        handle->video_res_supported.res[res].u16width             = imx662_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx662_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx662_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx662_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx662_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx662_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx662_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx662_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx662_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_HDR_DOL_LEF;
    handle->pCus_sensor_GetVideoResNum                            = NULL;
    handle->pCus_sensor_GetVideoRes                               = NULL;
    handle->pCus_sensor_GetCurVideoRes                            = NULL;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL_LEF;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_LEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_LEF;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_LEF;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_2LANE * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx662_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_2LANE;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps_HDR_DOL_2lane;
    params->dirty                                                 = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef2(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx662_params *params = NULL;
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

    params = (imx662_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"imx662_mipi_hdr_SEF2");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DOL;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = 2;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 2;

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
        handle->video_res_supported.res[res].u16width             = imx662_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx662_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx662_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx662_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx662_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx662_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx662_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx662_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx662_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi2lane_HDR_DOL;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL_SEF;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_SEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_SEF;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_SEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_SEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_SEF1;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_2LANE * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR_DOL_2LANE * params->min_rhs1;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_2LANE;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps_HDR_DOL_2lane;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX2(IMX662_HDR,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            cus_camsensor_init_handle_hdr_dol_sef2,
                            imx662_params
                         );
