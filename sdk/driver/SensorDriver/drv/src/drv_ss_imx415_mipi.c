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
   Porting owner :  Lance.Lan
   Date :           9/26/2023
   Build on :       Master_V4 I6C
   Verified on :    not
   Remark :         rename only
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX415_HDR);
extern u64 EXT_log_2(u32 value);
extern u32 round_float(u32 x, u32 piont_offset);
extern u64 base2_exp_float_pow(u64 x);

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

#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_DOL         CUS_DATAPRECISION_12

#define SENSOR_BAYERID              CUS_BAYER_GB         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_GB
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_YCORDER              CUS_SEN_YCODR_YC     //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define long_packet_type_enable     0x00 //UD1~UD8 (user define)

#define SENSOR_GAIN_DELAY_FRAME_COUNT         (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

#define SENSOR_GAIN_DELAY_FRAME_COUNT_DOL     (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_DOL  (2)
#define SENSOR_GAIN_CTRL_NUM_DOL              (2)
#define SENSOR_SHUTTER_CTRL_NUM_DOL           (2)

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_27MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_27MHZ
#define Preview_CSI_CLK_SPEED       CUS_CSI_CLK_432M

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
#define SENSOR_NAME     IMX415

#define CHIP_ID_r3F12   0x3F12
#define CHIP_ID_r3F13   0x3F13
#define CHIP_ID         0x0415

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_3, LINEAR_RES_4, LINEAR_RES_5, LINEAR_RES_6, LINEAR_RES_7, LINEAR_RES_8, LINEAR_RES_9, LINEAR_RES_10, LINEAR_RES_11, LINEAR_RES_12, LINEAR_RES_13, LINEAR_RES_END}mode;
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
}imx415_mipi_linear[] = {
    {LINEAR_RES_1, {3860, 2250, 3, 20}, {0, 0, 3840, 2160}, {"3840x2160@20fps"}},
    {LINEAR_RES_2, {3096, 2190, 3, 30}, {0, 0, 3072, 2048}, {"3072x2048@30fps"}}, // Modify it
    {LINEAR_RES_3, {3096, 1758, 3, 30}, {0, 0, 3072, 1728}, {"3072x1728@30fps"}}, // Modify it
    {LINEAR_RES_4, {2616, 1974, 3, 30}, {0, 0, 2592, 1944}, {"2592x1944@30fps"}}, // Modify it
    {LINEAR_RES_5, {2976, 1686, 3, 30}, {0, 0, 2944, 1656}, {"2944x1656@30fps"}}, // Modify it
    {LINEAR_RES_6, {2592, 1470, 3, 30}, {0, 0, 2560, 1440}, {"2560x1440@30fps"}}, // Modify it
    {LINEAR_RES_7, {1920, 1080, 3, 60}, {0, 6, 1920, 1080}, {"1920x1080@60fps"}}, // Modify it
    {LINEAR_RES_8, {3864, 2192, 3, 30}, {0, 0, 3840, 2160}, {"3840x2160@30fps"}}, // Modify it
    {LINEAR_RES_9, {3864, 2192, 3, 60}, {12, 16, 3840, 2160}, {"3840x2160@60fps"}}, // Modify it
    {LINEAR_RES_10, {3864, 2250, 3, 10}, {0, 0, 3840, 2160}, {"3840x2160@10fps_2lane"}}, //Modify it
    {LINEAR_RES_11, {2688, 1520, 3, 30}, {0, 0, 2688, 1520}, {"2688x1520@30fps_2lane"}}, // Modify it
    {LINEAR_RES_12, {1920, 1080, 3, 20}, {0, 0, 1920, 1080}, {"1920x1080@20fps"}}, // Modify it
    {LINEAR_RES_13, {3840, 2160, 3, 24}, {0, 0, 3840, 2160}, {"3840x2160@24fps"}}, // Modify it
};

static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_2, HDR_RES_3, HDR_RES_4, HDR_RES_5, HDR_RES_6, HDR_RES_7, HDR_RES_8, HDR_RES_9, HDR_RES_10, HDR_RES_11, HDR_RES_12, HDR_RES_13,  HDR_RES_14,HDR_RES_END}mode;
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
}imx415_mipi_hdr[] = {
    {HDR_RES_1, {3864, 2192, 3, 15}, {0, 0, 3840, 2160}, {"3840x2160@15fps_HDR"}}, // Modify it
    {HDR_RES_2, {3864, 2192, 3, 20}, {0, 0, 3840, 2160}, {"3840x2160@20fps_HDR"}}, // Modify it
    {HDR_RES_3, {3864, 2192, 3, 30}, {0, 0, 3840, 2160}, {"3840x2160@30fps_HDR"}}, // Modify it
    {HDR_RES_4, {2592, 1944, 3, 30}, {0, 0, 2592, 1944}, {"2592x1944@30fps_HDR"}}, // Modify it
    {HDR_RES_5, {2976, 1688, 3, 30}, {0, 0, 2944, 1656}, {"2944x1656@30fps_HDR"}}, // Modify it
    {HDR_RES_6, {2688, 1520, 3, 30}, {0, 0, 2688, 1520}, {"2688x1520@30fps_HDR"}}, // Modify it
    {HDR_RES_7, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
    {HDR_RES_8, {3840, 2160, 3, 24}, {0, 0, 3840, 2160}, {"3840x2160@24fps_HDR"}}, // Modify it
    {HDR_RES_9, {3864, 2185, 3, 30}, {0, 0, 3840, 2160}, {"3840x2160@30fps_HDR_3F"}},// Modify it
    {HDR_RES_10, {2592, 1944, 3, 25}, {0, 0, 2592, 1944}, {"2592x1944@25fps_HDR_3F"}},// Modify it
    {HDR_RES_11, {2952, 1656, 3, 25}, {0, 0, 2952, 1656}, {"2952x1656@25fps_HDR_3F"}},// Modify it
    {HDR_RES_12, {2688, 1520, 3, 25}, {0, 0, 2688, 1520}, {"2688x1520@25fps_HDR_3F"}},// Modify it
    {HDR_RES_13, {1944, 1097, 3, 22}, {0, 0, 1920, 1080}, {"1920x1080@22fps_HDR_3F"}},// Modify it
    {HDR_RES_14, {1944, 1097, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR_3F"}},// Modify it
};

#define IMX415_HDR_BRL                              2228

u32 vts_30fps = 2250;
u32 vts_30fps_HDR_DOL_4lane = 2250;
u32 Preview_line_period = 17778;
u32 Preview_line_period_HDR_DOL_4LANE = 29630;

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (3981 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1097)
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
        bool bVideoMode;
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    struct {
        float sclk;
        u32 hts;
        u32 vts;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 fps;
        u32 preview_fps;
        u32 expo_lines;
        u32 expo_lef_us;
        u32 expo_sef_us;
        u32 expo_sef2_us;
    } expo;

    u32 min_shr1;
    u32 min_rhs1;
    u32 min_shr2;
    u32 min_rhs2;
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
    I2C_ARRAY tSHR2_reg[3];
    I2C_ARRAY tRHS2_reg[3];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tGain_hdr_dol_lef_reg[2];
    I2C_ARRAY tGain_hdr_dol_sef_reg[2];
    I2C_ARRAY tGain_hdr_dol_sef2_reg[2];
    bool dirty;
    bool orien_dirty;
    u8 snr_pad;
    const I2C_ARRAY *pTable_linear;
    const I2C_ARRAY *pTable_HDR;
    u32 Init_Array_Size;
    u32 Init_Vts;
    u32 Line_Period;
    u32 csi_clk;
    u32 PowerOnOff;
    CUS_CAMSENSOR_ORIT orient;
} imx415_params;

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us);
static int pCus_AEStatusNotifyHDR_DOL_LEF(struct __ss_cus_sensor* handle, u32 idx,CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_AEStatusNotifyHDR_DOL3_LEF(struct __ss_cus_sensor* handle, u32 idx,CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_SetAEUSecsHDR_DOL3_SEF2(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL3_LEF(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL3_SEF(ss_cus_sensor *handle, u32 us);
static int pCus_GetAEUSecs_HDR_DOL_SEF2(ss_cus_sensor *handle, u32 *us);
static int pCus_GetAEUSecs_HDR_DOL3_LEF(ss_cus_sensor *handle, u32 *us);
static int pCus_GetAEUSecs_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *us);
static void pCus_SetAEGainHDR_DOL_Calculate(u32 gain, u16 *gain_reg_tmp);
static void pCus_GetAEGain_Calculate(u16 gain_reg_value, u32 *gain_value);


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

const static I2C_ARRAY Sensor_init_table_4lane_linear[] =
{
    {0x3000, 0x01},    //Standby
    //{0xffff, 0x24},    //delay (ms)
    {0x3002, 0x01},    //Master mode stop
    //{0xffff, 0x10},    //delay (ms)
    {0x3008, 0x7F},    //BCwait time
    {0x3009, 0x00},
    {0x300A, 0x5B},
    {0x300B, 0xA0},
    //{0x3023, 0x0},
    {0x3024, 0xCA},    //VMAX LSB
    {0x3025, 0x08},    //VMAX MSB     8CAh: 2250
    {0x3026, 0x00},    //VMAX MSB [0:3]
    {0x3028, 0x4C},    //HMAX LSB
    {0x3029, 0x04},    //HMAX MSB     44Ch: 1100
    {0x3031, 0x00},    //ADBIT    0h: AD 10bit; 1h: AD 12bit
    {0x3032, 0x00},    //MDBIT    0h: 10bit; 1h: 12bit
    {0x3033, 0x05},    //SYS MODE 05h: 891Mbps
    {0x3050, 0x08},    //SHR0 Shutter Setting
    {0x3051, 0x00},
    {0x3052, 0x00},
    {0x30C0, 0x2A},    //XVSOUTSEL
    {0x30C1, 0x00},
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x30CF, 0x00},    //DOL setting
    {0x3116, 0x24},    //INCKSEL2
    {0x311E, 0x24},    //INCKSEL5
    {0x32D4, 0x21},
    {0x32EC, 0xA1},
    {0x3452, 0x7F},
    {0x3453, 0x03},
    {0x358A, 0x04},
    {0x35A1, 0x02},
    {0x36BC, 0x0C},
    {0x36CC, 0x53},
    {0x36CD, 0x00},
    {0x36CE, 0x3C},
    {0x36D0, 0x8C},
    {0x36D1, 0x00},
    {0x36D2, 0x71},
    {0x36D4, 0x3C},
    {0x36D6, 0x53},
    {0x36D7, 0x00},
    {0x36D8, 0x71},
    {0x36DA, 0x8C},
    {0x36DB, 0x00},
    {0x37D1, 0x00},    //ADBIT1
    {0x3724, 0x02},
    {0x3726, 0x02},
    {0x3732, 0x02},
    {0x3734, 0x03},
    {0x3736, 0x03},
    {0x3742, 0x03},
    {0x3862, 0xE0},
    {0x38CC, 0x30},
    {0x38CD, 0x2F},
    {0x395C, 0x0C},
    {0x3A42, 0xD1},
    {0x3A4C, 0x77},
    {0x3AE0, 0x02},
    {0x3AEC, 0x0C},
    {0x3B00, 0x2E},
    {0x3B06, 0x29},
    {0x3B98, 0x25},
    {0x3B99, 0x21},
    {0x3B9B, 0x13},
    {0x3B9C, 0x13},
    {0x3B9D, 0x13},
    {0x3B9E, 0x13},
    {0x3BA1, 0x00},
    {0x3BA2, 0x06},
    {0x3BA3, 0x0B},
    {0x3BA4, 0x10},
    {0x3BA5, 0x14},
    {0x3BA6, 0x18},
    {0x3BA7, 0x1A},
    {0x3BA8, 0x1A},
    {0x3BA9, 0x1A},
    {0x3BAC, 0xED},
    {0x3BAD, 0x01},
    {0x3BAE, 0xF6},
    {0x3BAF, 0x02},
    {0x3BB0, 0xA2},
    {0x3BB1, 0x03},
    {0x3BB2, 0xE0},
    {0x3BB3, 0x03},
    {0x3BB4, 0xE0},
    {0x3BB5, 0x03},
    {0x3BB6, 0xE0},
    {0x3BB7, 0x03},
    {0x3BB8, 0xE0},
    {0x3BBA, 0xE0},
    {0x3BBC, 0xDA},
    {0x3BBE, 0x88},
    {0x3BC0, 0x44},
    {0x3BC2, 0x7B},
    {0x3BC4, 0xA2},
    {0x3BC8, 0xBD},
    {0x3BCA, 0xBD},
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0x48},    //TXCLKSEC_FREQ LSB
    {0x4005, 0x09},    //TXCLKSEC_FREQ MSB
    {0x400C, 0x00},
    {0x4018, 0x7F},    //TCLKPOST    MIPI global setting
    {0x401A, 0x37},    //TCLKPREPARE MIPI global setting
    {0x401C, 0x37},    //TCLKTRAIL   MIPI global setting
    {0x401E, 0xF7},    //TCLKZERO    MIPI global setting
    {0x401F, 0x00},    //TCLKZERO
    {0x4020, 0x3F},    //THSPREPARE
    {0x4022, 0x6F},    //THSZERO
    {0x4024, 0x3F},    //THSTRAIL
    {0x4026, 0x5F},    //THSEXT
    {0x4028, 0x2F},    //TLPX
    {0x4074, 0x01},
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
//8M20 594Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_8m_20fps_init_table_4lane_linear[] =
{
    /*
    IMX415-AAQR All-pixel scan CSI-2_4lane 27MHz AD:10bit Output:10bit 594Mbps Master Mode 19.996fps Integration Time 49.86ms
    Tool ver : Ver6.0  hts:1320  vts:2813
    */
    {0x3000, 0x01},    //Standby
    //{0xffff, 0x24},    //delay (ms)
    {0x3002, 0x01},    //Master mode stop
    //{0xffff, 0x10},    //delay (ms)
    {0x3008, 0x5D},    // BCWAIT_TIME[9:0]
    {0x300A, 0x42},    // CPWAIT_TIME[9:0]
    {0x3024, 0xFD},    // VMAX[19:0]
    {0x3025, 0x0A},    //
    {0x3028, 0x28},    // HMAX[15:0]
    {0x3029, 0x05},    //
    {0x3031, 0x00},    // ADBIT[1:0]
    {0x3032, 0x00},    // MDBIT
    {0x3033, 0x07},    // SYS_MODE[3:0]
    {0x3050, 0x08},    // SHR0[19:0]
    {0x30C1, 0x00},    // XVS_DRV[1:0]
    {0x3116, 0x23},    // INCKSEL2[7:0]
    {0x3118, 0x84},    // INCKSEL3[10:0]
    {0x311A, 0xE7},    // INCKSEL4[10:0]
    {0x311E, 0x23},    // INCKSEL5[7:0]
    {0x32D4, 0x21},    // -
    {0x32EC, 0xA1},    // -
    {0x3452, 0x7F},    // -
    {0x3453, 0x03},    // -
    {0x358A, 0x04},    // -
    {0x35A1, 0x02},    // -
    {0x36BC, 0x0C},    // -
    {0x36CC, 0x53},    // -
    {0x36CD, 0x00},    // -
    {0x36CE, 0x3C},    // -
    {0x36D0, 0x8C},    // -
    {0x36D1, 0x00},    // -
    {0x36D2, 0x71},    // -
    {0x36D4, 0x3C},    // -
    {0x36D6, 0x53},    // -
    {0x36D7, 0x00},    // -
    {0x36D8, 0x71},    // -
    {0x36DA, 0x8C},    // -
    {0x36DB, 0x00},    // -
    {0x3701, 0x00},    // ADBIT1[7:0]
    {0x3724, 0x02},    // -
    {0x3726, 0x02},    // -
    {0x3732, 0x02},    // -
    {0x3734, 0x03},    // -
    {0x3736, 0x03},    // -
    {0x3742, 0x03},    // -
    {0x3862, 0xE0},    // -
    {0x38CC, 0x30},    // -
    {0x38CD, 0x2F},    // -
    {0x395C, 0x0C},    // -
    {0x3A42, 0xD1},    // -
    {0x3A4C, 0x77},    // -
    {0x3AE0, 0x02},    // -
    {0x3AEC, 0x0C},    // -
    {0x3B00, 0x2E},    // -
    {0x3B06, 0x29},    // -
    {0x3B98, 0x25},    // -
    {0x3B99, 0x21},    // -
    {0x3B9B, 0x13},    // -
    {0x3B9C, 0x13},    // -
    {0x3B9D, 0x13},    // -
    {0x3B9E, 0x13},    // -
    {0x3BA1, 0x00},    // -
    {0x3BA2, 0x06},    // -
    {0x3BA3, 0x0B},    // -
    {0x3BA4, 0x10},    // -
    {0x3BA5, 0x14},    // -
    {0x3BA6, 0x18},    // -
    {0x3BA7, 0x1A},    // -
    {0x3BA8, 0x1A},    // -
    {0x3BA9, 0x1A},    // -
    {0x3BAC, 0xED},    // -
    {0x3BAD, 0x01},    // -
    {0x3BAE, 0xF6},    // -
    {0x3BAF, 0x02},    // -
    {0x3BB0, 0xA2},    // -
    {0x3BB1, 0x03},    // -
    {0x3BB2, 0xE0},    // -
    {0x3BB3, 0x03},    // -
    {0x3BB4, 0xE0},    // -
    {0x3BB5, 0x03},    // -
    {0x3BB6, 0xE0},    // -
    {0x3BB7, 0x03},    // -
    {0x3BB8, 0xE0},    // -
    {0x3BBA, 0xE0},    // -
    {0x3BBC, 0xDA},    // -
    {0x3BBE, 0x88},    // -
    {0x3BC0, 0x44},    // -
    {0x3BC2, 0x7B},    // -
    {0x3BC4, 0xA2},    // -
    {0x3BC8, 0xBD},    // -
    {0x3BCA, 0xBD},    // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},    // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},    //
    {0x400C, 0x00},    // INCKSEL6
    {0x4018, 0x67},    // TCLKPOST[15:0]
    {0x401A, 0x27},    // TCLKPREPARE[15:0]
    {0x401C, 0x27},    // TCLKTRAIL[15:0]
    {0x401E, 0xB7},    // TCLKZERO[15:0]
    {0x401F, 0x00},    //
    {0x4020, 0x2F},    // THSPREPARE[15:0]
    {0x4022, 0x4F},    // THSZERO[15:0]
    {0x4024, 0x2F},    // THSTRAIL[15:0]
    {0x4026, 0x47},    // THSEXIT[15:0]
    {0x4028, 0x27},    // TLPX[15:0]
    {0x4074, 0x01},    // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
//3096x2190 891Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_6m_30fps_init_table_4lane_linear[] =
{
    /*
    IMX415-AAQR Window cropping 3096x2190 CSI-2_4lane 27MHz AD:10bit Output:10bit 891Mbps Master Mode 30fps Integration Time 33.217ms
    Tool ver : Ver6.0  vts:2250 hts:1100
    */
    {0x3000, 0x01},    //Standby
    //{0xffff, 0x24},    //delay (ms)
    {0x3002, 0x01},    //Master mode stop
    //{0xffff, 0x10},    //delay (ms)
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3028, 0x4C},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3040, 0x80},  // PIX_HST[12:0]
    {0x3041, 0x01},  //
    {0x3043, 0x0C},  //
    {0x3044, 0x00},  // PIX_VST[12:0]//02
    {0x3045, 0x00},  //
    {0x3046, 0x1C},  // PIX_VWIDTH[12:0]
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
//3096x1758@30fps
const static I2C_ARRAY Sensor_5p3m_30fps_init_table_4lane_linear[] =
{
    /*
    IMX415-AAQR Window cropping 3096x1758 CSI-2_4lane 27MHz AD:10bit Output:10bit 891Mbps Master Mode 30fps Integration Time 33.217ms
    Tool ver : Ver6.0
    */
    {0x3000, 0x01},    //Standby
    //{0xffff, 0x24},     //delay (ms)
    {0x3002, 0x01},    //Master mode stop
    //{0xffff, 0x10},     //delay (ms)
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3028, 0x4C},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3040, 0x80},  // PIX_HST[12:0]
    {0x3041, 0x01},  //
    {0x3043, 0x0C},  //
    {0x3044, 0xB0},  // PIX_VST[12:0]  //01b2
    {0x3045, 0x01},  //
    {0x3046, 0xBC},  // PIX_VWIDTH[12:0]
    {0x3047, 0x0D},  //
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

//5M30 891Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_5m_30fps_init_table_4lane_linear[] =
{
    /*
    IMX415-AAQR Window cropping 2616x1974 CSI-2_4lane 27MHz AD:10bit Output:10bit 891Mbps Master Mode 30fps Integration Time 33.217ms
    Tool ver : Ver6.0
    */
    {0x3000, 0x01},    //Standby
    //{0xffff, 0x24},    //delay (ms)
    {0x3002, 0x01},    //Master mode stop
    //{0xffff, 0x10},    //delay (ms)
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3024, 0x60},  //VMAX LSB
    {0x3025, 0x08},  //VMAX MSB     8CAh: 2250
    {0x3026, 0x00},  //VMAX MSB [0:3]
    {0x3028, 0x82},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3040, 0x70},  // PIX_HST[12:0]
    {0x3041, 0x02},  //
    {0x3042, 0x38},  // PIX_HWIDTH[12:0]
    {0x3043, 0x0A},  //
    {0x3044, 0xD8},  // PIX_VST[12:0] //DA
    {0x3045, 0x00},  //
    {0x3046, 0x6C},  // PIX_VWIDTH[12:0]
    {0x3047, 0x0F},  //
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
//4p8M30 891Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_4p8m_30fps_init_table_4lane_linear[] =
{
    /*
    IMX415-AAQR Window cropping 2976x1686 CSI-2_4lane 27MHz AD:10bit Output:10bit 891Mbps Master Mode 30fps Integration Time 33.217ms
    Tool ver : Ver6.0
    */
    {0x3000, 0x01},    //Standby
    //{0xffff, 0x24},    //delay (ms)
    {0x3002, 0x01},    //Master mode stop
    //{0xffff, 0x10},    //delay (ms)
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3028, 0x4C},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3040, 0xBC},  // PIX_HST[12:0]
    {0x3041, 0x01},  //
    {0x3042, 0xA0},  // PIX_HWIDTH[12:0]
    {0x3043, 0x0B},  //
    {0x3044, 0xF8},  // PIX_VST[12:0] //FA
    {0x3045, 0x01},  //
    {0x3046, 0x2C},  // PIX_VWIDTH[12:0]
    {0x3047, 0x0D},  //
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
//2592x1470@30fps
const static I2C_ARRAY Sensor_3p6m_30fps_init_table_4lane_linear[] =
{
    /*
    IMX415-AAQR Window cropping 2592x1470 CSI-2_4lane 27MHz AD:10bit Output:10bit 891Mbps Master Mode 30fps Integration Time 33.217ms
    Tool ver : Ver6.0
    */
    {0x3000, 0x01},    //Standby
    //{0xffff, 0x24},    //delay (ms)
    {0x3002, 0x01},    //Master mode stop
    //{0xffff, 0x10},    //delay (ms)
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3028, 0x4C},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3040, 0x7C},  // PIX_HST[12:0]
    {0x3041, 0x02},  //
    {0x3042, 0x20},  // PIX_HWIDTH[12:0]
    {0x3043, 0x0A},  //
    {0x3044, 0xD0},  // PIX_VST[12:0] //D2
    {0x3045, 0x02},  //
    {0x3046, 0x7C},  // PIX_VWIDTH[12:0]
    {0x3047, 0x0B},  //
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};
//8M30 891Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_8m_30fps_init_table_4lane_linear[] =
{
    /*
        IMX415-AAQRAll-pixel scan
        CSI-2_4lane
        27MHz
        AD:10bit
        Output:10bit
        891Mbps
        Master Mode 30fps
        Integration Time33.217ms
    */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},    // BCWAIT_TIME[9:0]
    {0x300A, 0x42},    // CPWAIT_TIME[9:0]
    {0x3024, 0xCA},    // VMAX[19:0]
    {0x3025, 0x08},    //
    {0x3028, 0x4C},    // HMAX[15:0]
    {0x3029, 0x04},    //
    {0x3031, 0x00},    // ADBIT[1:0]
    {0x3032, 0x00},    // MDBIT
    {0x3033, 0x05},    // SYS_MODE[3:0]
    {0x3050, 0x08},    // SHR0[19:0]
    {0x30C1, 0x00},    // XVS_DRV[1:0]
    {0x3116, 0x23},    // INCKSEL2[7:0]
    {0x3118, 0xC6},    // INCKSEL3[10:0]
    {0x311A, 0xE7},    // INCKSEL4[10:0]
    {0x311E, 0x23},    // INCKSEL5[7:0]
    {0x32D4, 0x21},    // -
    {0x32EC, 0xA1},    // -
    {0x3452, 0x7F},    // -
    {0x3453, 0x03},    // -
    {0x358A, 0x04},    // -
    {0x35A1, 0x02},    // -
    {0x36BC, 0x0C},    // -
    {0x36CC, 0x53},    // -
    {0x36CD, 0x00},    // -
    {0x36CE, 0x3C},    // -
    {0x36D0, 0x8C},    // -
    {0x36D1, 0x00},    // -
    {0x36D2, 0x71},    // -
    {0x36D4, 0x3C},    // -
    {0x36D6, 0x53},    // -
    {0x36D7, 0x00},    // -
    {0x36D8, 0x71},    // -
    {0x36DA, 0x8C},    // -
    {0x36DB, 0x00},    // -
    {0x3701, 0x00},    // ADBIT1[7:0]
    {0x3724, 0x02},    // -
    {0x3726, 0x02},    // -
    {0x3732, 0x02},    // -
    {0x3734, 0x03},    // -
    {0x3736, 0x03},    // -
    {0x3742, 0x03},    // -
    {0x3862, 0xE0},    // -
    {0x38CC, 0x30},    // -
    {0x38CD, 0x2F},    // -
    {0x395C, 0x0C},    // -

    {0x3A42, 0xD1},    // -
    {0x3A4C, 0x77},    // -
    {0x3AE0, 0x02},    // -
    {0x3AEC, 0x0C},    // -
    {0x3B00, 0x2E},    // -
    {0x3B06, 0x29},    // -
    {0x3B98, 0x25},    // -
    {0x3B99, 0x21},    // -
    {0x3B9B, 0x13},    // -
    {0x3B9C, 0x13},    // -
    {0x3B9D, 0x13},    // -
    {0x3B9E, 0x13},    // -
    {0x3BA1, 0x00},    // -
    {0x3BA2, 0x06},    // -
    {0x3BA3, 0x0B},    // -
    {0x3BA4, 0x10},    // -
    {0x3BA5, 0x14},    // -
    {0x3BA6, 0x18},    // -
    {0x3BA7, 0x1A},    // -
    {0x3BA8, 0x1A},    // -
    {0x3BA9, 0x1A},    // -
    {0x3BAC, 0xED},    // -
    {0x3BAD, 0x01},    // -
    {0x3BAE, 0xF6},    // -
    {0x3BAF, 0x02},    // -
    {0x3BB0, 0xA2},    // -
    {0x3BB1, 0x03},    // -
    {0x3BB2, 0xE0},    // -
    {0x3BB3, 0x03},    // -
    {0x3BB4, 0xE0},    // -
    {0x3BB5, 0x03},    // -
    {0x3BB6, 0xE0},    // -
    {0x3BB7, 0x03},    // -
    {0x3BB8, 0xE0},    // -
    {0x3BBA, 0xE0},    // -
    {0x3BBC, 0xDA},    // -
    {0x3BBE, 0x88},    // -
    {0x3BC0, 0x44},    // -
    {0x3BC2, 0x7B},    // -
    {0x3BC4, 0xA2},    // -
    {0x3BC8, 0xBD},    // -
    {0x3BCA, 0xBD},    // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},    // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},    //
    {0x400C, 0x00},    // INCKSEL6
    {0x4018, 0x7F},    // TCLKPOST[15:0]
    {0x401A, 0x37},    // TCLKPREPARE[15:0]
    {0x401C, 0x37},    // TCLKTRAIL[15:0]
    {0x401E, 0xF7},    // TCLKZERO[15:0]
    {0x401F, 0x00},    //
    {0x4020, 0x3F},    // THSPREPARE[15:0]
    {0x4022, 0x6F},    // THSZERO[15:0]
    {0x4024, 0x3F},    // THSTRAIL[15:0]
    {0x4026, 0x5F},    // THSEXIT[15:0]
    {0x4028, 0x2F},    // TLPX[15:0]
    {0x4074, 0x01},    // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

//8M60 1485Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_8m_60fps_init_table_4lane_linear[] =
{
/*
IMX415-AAQR All-pixel scan CSI-2_4lane 27MHz AD:10bit Output:10bit 1485Mbps Master Mode 60fps Integration Time 16.609ms
Tool ver : Ver9.0
*/
    {0x3000, 0x01}, //Standby
    {0x3002, 0x01}, //Master mode stop
    {0x3008, 0x5D}, // BCWAIT_TIME[9:0]
    {0x300A, 0x42}, // CPWAIT_TIME[9:0]
    {0x3031, 0x00}, // ADBIT[1:0]
    {0x3032, 0x00}, // MDBIT
    {0x3033, 0x08}, // SYS_MODE[3:0]
    {0x3050, 0x08}, // SHR0[19:0]
    {0x30C1, 0x00}, // XVS_DRV[1:0]
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xA5}, // INCKSEL3[10:0]
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3701, 0x00}, // ADBIT1[7:0]
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
    {0x4018, 0xA7}, // TCLKPOST[15:0]
    {0x401A, 0x57}, // TCLKPREPARE[15:0]
    {0x401C, 0x5F}, // TCLKTRAIL[15:0]
    {0x401E, 0x97}, // TCLKZERO[15:0]
    {0x4020, 0x5F}, // THSPREPARE[15:0]
    {0x4022, 0xAF}, // THSZERO[15:0]
    {0x4024, 0x5F}, // THSTRAIL[15:0]
    {0x4026, 0x9F}, // THSEXIT[15:0]
    {0x4028, 0x4F}, // TLPX[15:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

//8M5 891Mbps, 10bits, Mclk 27M
const static I2C_ARRAY Sensor_8m_10fps_init_table_2lane_linear[] =
{
#if 1// for 8M11fps 12bit
    /*
    IMX415-AAQR All-pixel scan CSI-2_2lane 27MHz AD:12bit Output:12bit 891Mbps Master Mode 11.201fps Integration Time 89.051ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01}, //Standby
    {0x3002, 0x01}, //Master mode stop
    {0x3008, 0x5D}, // BCWAIT_TIME[9:0]
    {0x300A, 0x42}, // CPWAIT_TIME[9:0]
    //{0x3024, 0xC5}, // VMAX[19:0] --> 11 fps
    //{0x3025, 0x0B}, //
    {0x3024, 0xF3}, // VMAX[19:0] --> 10 fps
    {0x3025, 0x0C}, //
    {0x3028, 0xBF}, // HMAX[15:0]
    {0x3029, 0x08}, //
    {0x3033, 0x05}, // SYS_MODE[3:0]
    {0x3050, 0x08}, // SHR0[19:0]
    {0x30C1, 0x00}, // XVS_DRV[1:0]
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xC6}, // INCKSEL3[10:0]
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4001, 0x01}, // LANEMODE[2:0]
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
    {0x400C, 0x00}, // INCKSEL6
    {0x4018, 0x7F}, // TCLKPOST[15:0]
    {0x401A, 0x37}, // TCLKPREPARE[15:0]
    {0x401C, 0x37}, // TCLKTRAIL[15:0]
    {0x401E, 0xF7}, // TCLKZERO[15:0]
    {0x401F, 0x00}, //
    {0x4020, 0x3F}, // THSPREPARE[15:0]
    {0x4022, 0x6F}, // THSZERO[15:0]
    {0x4024, 0x3F}, // THSTRAIL[15:0]
    {0x4026, 0x5F}, // THSEXIT[15:0]
    {0x4028, 0x2F}, // TLPX[15:0]
    {0x4074, 0x01}, // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating

#endif

    #if 0 // for 5 fps
    /*
    IMX415-AAQR All-pixel scan CSI-2_2lane 27MHz AD:12bit Output:12bit 891Mbps Master Mode 11.201fps Integration Time 89.051ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01}, //Standby
    {0x3002, 0x01}, //Master mode stop
    {0x3008, 0x5D}, // BCWAIT_TIME[9:0]
    {0x300A, 0x42}, // CPWAIT_TIME[9:0]
    {0x3024, 0x9d}, // VMAX[19:0]
    {0x3025, 0x13}, //
    {0x3028, 0x8e}, // HMAX[15:0]
    {0x3029, 0x0b}, //
    {0x3033, 0x05}, // SYS_MODE[3:0]
    {0x3050, 0x08}, // SHR0[19:0]
    {0x30C1, 0x00}, // XVS_DRV[1:0]
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xC6}, // INCKSEL3[10:0]
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4001, 0x01}, // LANEMODE[2:0]
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
    {0x400C, 0x00}, // INCKSEL6
    {0x4018, 0x7F}, // TCLKPOST[15:0]
    {0x401A, 0x37}, // TCLKPREPARE[15:0]
    {0x401C, 0x37}, // TCLKTRAIL[15:0]
    {0x401E, 0xF7}, // TCLKZERO[15:0]
    {0x401F, 0x00}, //
    {0x4020, 0x3F}, // THSPREPARE[15:0]
    {0x4022, 0x6F}, // THSZERO[15:0]
    {0x4024, 0x3F}, // THSTRAIL[15:0]
    {0x4026, 0x5F}, // THSEXIT[15:0]
    {0x4028, 0x2F}, // TLPX[15:0]
    {0x4074, 0x01}, // INCKSEL7 [2:0]

    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
#endif

};

//1920x1080@60fps
const static I2C_ARRAY Sensor_2m_60fps_init_table_4lane_linear[] =
{
/*
IMX415-AAQR 2/2-line binning CSI-2_4lane 27MHz AD:10bit Output:12bit 891Mbps Master Mode 59.998fps Integration Time 16.61ms
Tool ver : Ver6.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x3020, 0x01},  // HADD
    {0x3021, 0x01},  // VADD
    {0x3022, 0x01},  // ADDMODE[1:0]
    {0x3024, 0xF8},  // VMAX[19:0]
    {0x3028, 0x1B},  // HMAX[15:0]
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30D9, 0x02},  // DIG_CLP_VSTART[4:0]
    {0x30DA, 0x01},  // DIG_CLP_VNUM[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

//1920x1080@20fps
const static I2C_ARRAY Sensor_2m_20fps_init_table_4lane_linear[] =
{
/*
IMX415-AAQR 2/2-line binning CSI-2_4lane 27MHz AD:10bit Output:12bit 594Mbps Master Mode 20fps Integration Time 16.61ms
Tool ver : Ver6.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x3020, 0x01},  // HADD
    {0x3021, 0x01},  // VADD
    {0x3022, 0x01},  // ADDMODE[1:0]
    {0x3024, 0xCA},    //VMAX LSB
    {0x3025, 0x08},    //VMAX MSB     8CAh: 2250
    {0x3026, 0x00},    //VMAX MSB [0:3]
    {0x3028, 0x72},    //HMAX LSB
    {0x3029, 0x06},    //HMAX MSB     672h: 1650
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3033, 0x07},  // SYS_MODE[3:0]
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30D9, 0x02},  // DIG_CLP_VSTART[4:0]
    {0x30DA, 0x01},  // DIG_CLP_VNUM[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0x84},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x67},  // TCLKPOST[15:0]
    {0x401A, 0x27},  // TCLKPREPARE[15:0]
    {0x401C, 0x27},  // TCLKTRAIL[15:0]
    {0x401E, 0xB7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x2F},  // THSPREPARE[15:0]
    {0x4022, 0x4F},  // THSZERO[15:0]
    {0x4024, 0x2F},  // THSTRAIL[15:0]
    {0x4026, 0x47},  // THSEXIT[15:0]
    {0x4028, 0x27},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

//3840x2160@24fps
const static I2C_ARRAY Sensor_8m_24fps_init_table_4lane_linear[] =
{
/*
IMX415-AAQR 2/2-line binning CSI-2_4lane 27MHz AD:10bit Output:12bit 594Mbps Master Mode 20fps Integration Time 16.61ms
Tool ver : Ver6.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x54},  // BCWAIT_TIME[9:0]
    {0x300A, 0x3B},  // CPWAIT_TIME[9:0]
    {0x3024, 0xFE},    //VMAX LSB
    {0x3025, 0x08},    //VMAX MSB    8FE:2302 9FE:2558 AFEh: 2814
    {0x3026, 0x00},    //VMAX MSB [0:3]
    {0x3028, 0x17},    //HMAX LSB
    {0x3029, 0x05},    //HMAX MSB    517:1303 494:1172 42Ah: 1066
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT[1:0]
    {0x3033, 0x09},  // SYS_MODE[3:0]
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xB4},  // INCKSEL3[10:0]
    {0x311A, 0xFC},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x344C, 0x2B},  // -
    {0x344D, 0x01},  // -
    {0x344E, 0xED},  // -
    {0x344F, 0x01},  // -
    {0x3450, 0xF6},  // -
    {0x3451, 0x02},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x35EC, 0x27},  // -
    {0x35EE, 0x8D},  // -
    {0x35F0, 0x8D},  // -
    {0x35F2, 0x29},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3720, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x39A4, 0x07},  // -
    {0x39A8, 0x32},  // -
    {0x39AA, 0x32},  // -
    {0x39AC, 0x32},  // -
    {0x39AE, 0x32},  // -
    {0x39B0, 0x32},  // -
    {0x39B2, 0x2F},  // -
    {0x39B4, 0x2D},  // -
    {0x39B6, 0x28},  // -
    {0x39B8, 0x30},  // -
    {0x39BA, 0x30},  // -
    {0x39BC, 0x30},  // -
    {0x39BE, 0x30},  // -
    {0x39C0, 0x30},  // -
    {0x39C2, 0x2E},  // -
    {0x39C4, 0x2B},  // -
    {0x39C6, 0x25},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0x00},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x6F},  // TCLKPOST[15:0]
    {0x401A, 0x2F},  // TCLKPREPARE[15:0]
    {0x401C, 0x2F},  // TCLKTRAIL[15:0]
    {0x401E, 0xBF},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x2F},  // THSPREPARE[15:0]
    {0x4022, 0x57},  // THSZERO[15:0]
    {0x4024, 0x2F},  // THSTRAIL[15:0]
    {0x4026, 0x4F},  // THSEXIT[15:0]
    {0x4028, 0x27},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_init_table_4lane_HDR_DOL[] =
{
    /*
    IMX415-AAQR All-pixel scan CSI-2_4lane 27MHz AD:12bit Output:12bit 891Mbps Master Mode DOL HDR 2frame VC 15fps Integration Time LEF:9.988ms SEF:0.121ms
    Tool ver : Ver6.0  vts:2250 hts:1100
    */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x3028, 0x4C},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3050, 0xF2},  // SHR0[19:0]
    {0x3051, 0x0E},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x11},  // RHS1[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

//5M30 1440Mbps, 10bits, Mclk 24M
const static I2C_ARRAY Sensor_4m_30fps_init_table_2lane_linear[] =
{
/*
IMX415-AAQR Window cropping 2688x1520 CSI-2_2lane 24MHz AD:10bit Output:10bit 1440Mbps Master Mode 30.005fps Integration Time 33.217ms
Tool ver : Ver9.0
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x54},  // 24M 1440
    {0x300A, 0x3B},  // 24M 1440
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3024, 0xCB},  //VMAX LSB
    {0x3025, 0x08},  //VMAX MSB     8CAh: 2250
    {0x3026, 0x00},  //VMAX MSB [0:3]
    {0x3028, 0x34},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    {0x3033, 0x08},  // SYS_MODE[3:0]
    {0x3040, 0x4C},  // PIX_HST[12:0]
    {0x3041, 0x02},  //
    {0x3042, 0x80},  // PIX_HWIDTH[12:0]
    {0x3043, 0x0A},  //
    {0x3044, 0xA0},  // PIX_VST[12:0]
    {0x3045, 0x02},  //
    {0x3046, 0xE0},  // PIX_VWIDTH[12:0]
    {0x3047, 0x0B},  //
    {0x3050, 0x08},  // SHR0[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xB4},  // INCKSEL3[10:0]
    {0x311A, 0xFC},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x344C, 0x2B},  // -
    {0x344D, 0x01},  // -
    {0x344E, 0xED},  // -
    {0x344F, 0x01},  // -
    {0x3450, 0xF6},  // -
    {0x3451, 0x02},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x35EC, 0x27},  // -
    {0x35EE, 0x8D},  // -
    {0x35F0, 0x8D},  // -
    {0x35F2, 0x29},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3720, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x39A4, 0x07},  // -
    {0x39A8, 0x32},  // -
    {0x39AA, 0x32},  // -
    {0x39AC, 0x32},  // -
    {0x39AE, 0x32},  // -
    {0x39B0, 0x32},  // -
    {0x39B2, 0x2F},  // -
    {0x39B4, 0x2D},  // -
    {0x39B6, 0x28},  // -
    {0x39B8, 0x30},  // -
    {0x39BA, 0x30},  // -
    {0x39BC, 0x30},  // -
    {0x39BE, 0x30},  // -
    {0x39C0, 0x30},  // -
    {0x39C2, 0x2E},  // -
    {0x39C4, 0x2B},  // -
    {0x39C6, 0x25},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4001, 0x01},  // LANEMODE[2:0]
    {0x4004, 0x00},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x4018, 0x9F},  // TCLKPOST[15:0]
    {0x401A, 0x57},  // TCLKPREPARE[15:0]
    {0x401C, 0x57},  // TCLKTRAIL[15:0]
    {0x401E, 0x87},  // TCLKZERO[15:0]
    {0x4020, 0x5F},  // THSPREPARE[15:0]
    {0x4022, 0xA7},  // THSZERO[15:0]
    {0x4024, 0x5F},  // THSTRAIL[15:0]
    {0x4026, 0x97},  // THSEXIT[15:0]
    {0x4028, 0x4f},  // TLPX[15:0]
    {0x4074, 0x00},  // INCKSEL7 [2:0]
    {0xffff, 0x24},
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_8m_20fps_init_table_4lane_HDR_DOL[] =
{
    /*
        IMX415-AAQRAll-pixel scan
        CSI-2_4lane
        27MHz
        AD:10bit Output:10bit
        1485Mbps
        Master Mode DOL HDR 2frame VC
        20.002fps
        Integration Time LEF:1.002ms SEF:0.118ms
    */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x3024, 0x7A},  // VMAX[15:0]
    {0x3025, 0x0D},  //
    {0x3028, 0x1A},  // HMAX[15:0]
    {0x3029, 0x02},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3031, 0x00},  //
    {0x3032, 0x00},  //
    {0x3033, 0x08},  // SYS_MODE[3:0]
    {0x3050, 0x6A},  // SHR0[19:0]
    {0x3051, 0x1A},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x19},  // RHS1[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xA5},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x4018, 0xA7},  // TCLKPOST[15:0]
    {0x401A, 0x57},  // TCLKPREPARE[15:0]
    {0x401C, 0x5F},  // TCLKTRAIL[15:0]
    {0x401E, 0x97},  // TCLKZERO[15:0]
    {0x4020, 0x5F},  // THSPREPARE[15:0]
    {0x4022, 0xAF},  // THSZERO[15:0]
    {0x4024, 0x5F},  // THSTRAIL[15:0]
    {0x4026, 0x9F},  // THSEXIT[15:0]
    {0x4028, 0x4F},  // TLPX[15:0]
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_8m_30fps_init_table_4lane_HDR_DOL[] =
{
    /*
        IMX415-AAQRAll-pixel scan
        CSI-2_4lane
        27MHz
        AD:10bit Output:10bit
        1485Mbps
        Master Mode DOL HDR 2frame VC
        30.002fps
        Integration Time LEF:1.002ms SEF:0.118ms
    */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x3024, 0xFC},  // VMAX[15:0]
    {0x3025, 0x08},  //
    {0x3028, 0x1A},  // HMAX[15:0]
    {0x3029, 0x02},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3031, 0x00},  //
    {0x3032, 0x00},  //
    {0x3033, 0x08},  // SYS_MODE[3:0]
    {0x3050, 0x6E},  // SHR0[19:0]
    {0x3051, 0x11},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x19},  // RHS1[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xA5},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x4018, 0xA7},  // TCLKPOST[15:0]
    {0x401A, 0x57},  // TCLKPREPARE[15:0]
    {0x401C, 0x5F},  // TCLKTRAIL[15:0]
    {0x401E, 0x97},  // TCLKZERO[15:0]
    {0x4020, 0x5F},  // THSPREPARE[15:0]
    {0x4022, 0xAF},  // THSZERO[15:0]
    {0x4024, 0x5F},  // THSTRAIL[15:0]
    {0x4026, 0x9F},  // THSEXIT[15:0]
    {0x4028, 0x4F},  // TLPX[15:0]
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_5m_30fps_init_table_4lane_HDR_DOL[] =
{
    /*
        IMX415-AAQR
        Window cropping2592x1944
        CSI-2_4lane27MHz
        AD:10bit Output:10bit
        1485Mbps
        Master Mode DOL HDR 2frame VC
        30.002fps
        Integration Time LEF:1.002ms SEF:0.118ms

    */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  //
    {0x3024, 0xFC},  // VMAX[15:0]
    {0x3025, 0x08},  //
    {0x3028, 0x1A},  // HMAX[15:0]
    {0x3029, 0x02},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3031, 0x00},  //
    {0x3032, 0x00},  //
    {0x3033, 0x08},  // SYS_MODE[3:0]
    {0x3040, 0x7C},  //
    {0x3041, 0x02},  //
    {0x3042, 0x20},  //
    {0x3043, 0x0A},  //
    {0x3044, 0xF8},  //
    {0x3045, 0x00},  //
    {0x3046, 0x30},  //
    {0x3047, 0x0F},  //
    {0x3050, 0x6E},  // SHR0[19:0]
    {0x3051, 0x11},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x19},  // RHS1[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xA5},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x4018, 0xA7},  // TCLKPOST[15:0]
    {0x401A, 0x57},  // TCLKPREPARE[15:0]
    {0x401C, 0x5F},  // TCLKTRAIL[15:0]
    {0x401E, 0x97},  // TCLKZERO[15:0]
    {0x4020, 0x5F},  // THSPREPARE[15:0]
    {0x4022, 0xAF},  // THSZERO[15:0]
    {0x4024, 0x5F},  // THSTRAIL[15:0]
    {0x4026, 0x9F},  // THSEXIT[15:0]
    {0x4028, 0x4F},  // TLPX[15:0]
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

#if 0
const static I2C_ARRAY Sensor_5m_15fps_init_table_4lane_HDR_DOL[] =
{
    /*
    IMX415-AAQR window cropping2616x1974 scan CSI-2_4lane 27MHz AD:12bit Output:12bit 891Mbps Master Mode DOL HDR 2frame VC 15fps Integration Time LEF:9.988ms SEF:0.121ms
    Tool ver : Ver6.0
    */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04}, // WINMODE[3:0]
    {0x3028, 0x4C},  // HMAX[15:0]
    {0x3029, 0x04},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3040, 0x70},  // PIX_HST[12:0]
    {0x3041, 0x02},  //
    {0x3042, 0x38},  // PIX_HWIDTH[12:0]
    {0x3043, 0x0A},  //
    {0x3044, 0xDA},  // PIX_VST[12:0]
    {0x3045, 0x00},  //
    {0x3046, 0x6C},  // PIX_VWIDTH[12:0]
    {0x3047, 0x0F},  //
    {0x3050, 0xF2},  // SHR0[19:0]
    {0x3051, 0x0E},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x11},  // RHS1[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},  // INCKSEL6
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},  //
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // INCKSEL7 [2:0]
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};
#endif

const static I2C_ARRAY Sensor_4p8m_30fps_init_table_4lane_HDR_DOL[] =
{
    /*
    "IMX415-AAQR Window cropping 2976x1686 CSI-2_4lane 27MHz
     AD:10bit Output:10bit 1485Mbps
     Master Mode DOL HDR 2frame VC 30.002fps
     Integration Time LEF:1.002ms SEF:0.118ms"
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  //
    {0x3024, 0xFC},  // VMAX[15:0] FC
    {0x3025, 0x08},  //
    {0x3028, 0x1A},  // HMAX[15:0] 1A
    {0x3029, 0x02},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3031, 0x00},  //
    {0x3032, 0x00},  //
    {0x3033, 0x08},  // SYS_MODE[3:0]
    {0x3040, 0xBC},  //
    {0x3041, 0x01},  //            01
    {0x3042, 0xA0},  //
    {0x3043, 0x0B},  //
    {0x3044, 0xF8},  //
    {0x3045, 0x01},  //            01
    {0x3046, 0x30},  //
    {0x3047, 0x0D},  //
    {0x3050, 0x6E},  // SHR0[19:0]
    {0x3051, 0x11},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x19},  // RHS1[19:0]
    {0x30C0, 0x2A},
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xA5},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x4018, 0xA7},  // TCLKPOST[15:0]
    {0x401A, 0x57},  // TCLKPREPARE[15:0]
    {0x401C, 0x5F},  // TCLKTRAIL[15:0]
    {0x401E, 0x97},  // TCLKZERO[15:0]
    {0x4020, 0x5F},  // THSPREPARE[15:0]
    {0x4022, 0xAF},  // THSZERO[15:0]
    {0x4024, 0x5F},  // THSTRAIL[15:0]
    {0x4026, 0x9F},  // THSEXIT[15:0]
    {0x4028, 0x4F},  // TLPX[15:0]
    {0xffff, 0x10},
    {0x3000, 0x00},  //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},  //Master mode start
    {0x3260, 0x00},  //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_4m_30fps_init_table_4lane_HDR_DOL[] =
{
    /*
        "IMX415-AAQR Window cropping 2688x1520
        CSI-2_4lane 27MHz AD:10bit Output:10bit 1485Mbps
        Master Mode DOL HDR 2frame VC
        30.002fps Integration Time LEF:1.002ms SEF:0.118ms"
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  //
    {0x3024, 0xFC},  // VMAX[15:0]
    {0x3025, 0x08},  //
    {0x3028, 0x1A},  // HMAX[15:0]
    {0x3029, 0x02},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3031, 0x00},  //
    {0x3032, 0x00},  //
    {0x3033, 0x08},  // SYS_MODE[3:0]
    {0x3040, 0x4C},  //
    {0x3041, 0x01},  //
    {0x3042, 0x80},  //
    {0x3043, 0x0A},  //
    {0x3044, 0xA0},  //
    {0x3045, 0x01},  //
    {0x3046, 0xE0},  //
    {0x3047, 0x0B},  //
    {0x3050, 0x6E},  // SHR0[19:0]
    {0x3051, 0x11},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x19},  // RHS1[19:0]
    {0x30C0, 0x2A},
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xA5},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x4018, 0xA7},  // TCLKPOST[15:0]
    {0x401A, 0x57},  // TCLKPREPARE[15:0]
    {0x401C, 0x5F},  // TCLKTRAIL[15:0]
    {0x401E, 0x97},  // TCLKZERO[15:0]
    {0x4020, 0x5F},  // THSPREPARE[15:0]
    {0x4022, 0xAF},  // THSZERO[15:0]
    {0x4024, 0x5F},  // THSTRAIL[15:0]
    {0x4026, 0x9F},  // THSEXIT[15:0]
    {0x4028, 0x4F},  // TLPX[15:0]
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_2m_30fps_init_table_4lane_HDR_DOL[] =
{
    /*
    "IMX415-AAQR 2/2-line binning CSI-2_4lane 27MHz
     AD:10bit Output:12bit 891Mbps
     Master Mode DOL HDR 2frame VC
     29.999fps Integration Time LEF:1.004ms SEF:0.118ms"
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]

    {0x3020, 0x01},  // 01h: Horizontal 2 binning
    {0x3021, 0x01},  // 01h: Vertical 2 binning
    {0x3022, 0x01},  // 01h: Horizontal/Vertical 2x2-line binning(RGB only)
    {0x3024, 0xF8},  // MAX[15:0]
    {0x3025, 0x08},  //
    {0x3028, 0x1B},  // HMAX[15:0]
    {0x3029, 0x02},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3031, 0x00},  //
    {0x3033, 0x05},  // SYS_MODE[3:0]
    {0x3050, 0x66},  // SHR0[19:0]
    {0x3051, 0x11},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x19},  // RHS1[19:0]
    {0x30C0, 0x2A},
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x30D9, 0x02},
    {0x30DA, 0x01},
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0xC6},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // --
    {0x32EC, 0xA1},  // --
    {0x3452, 0x7F},  // --
    {0x3453, 0x03},  // --
    {0x358A, 0x04},  // --
    {0x35A1, 0x02},  // --
    {0x36BC, 0x0C},  // --
    {0x36CC, 0x53},  // --
    {0x36CD, 0x00},  // --
    {0x36CE, 0x3C},  // --
    {0x36D0, 0x8C},  // --
    {0x36D1, 0x00},  // --
    {0x36D2, 0x71},  // --
    {0x36D4, 0x3C},  // --
    {0x36D6, 0x53},  // --
    {0x36D7, 0x00},  // --
    {0x36D8, 0x71},  // --
    {0x36DA, 0x8C},  // --
    {0x36DB, 0x00},  // --
    {0x3701, 0x00},  // --
    {0x3724, 0x02},  // --
    {0x3726, 0x02},  // --
    {0x3732, 0x02},  // --
    {0x3734, 0x03},  // --
    {0x3736, 0x03},  // --
    {0x3742, 0x03},  // --
    {0x3862, 0xE0},  // --
    {0x38CC, 0x30},  // --
    {0x38CD, 0x2F},  // --
    {0x395C, 0x0C},  // --
    {0x3A42, 0xD1},  // --
    {0x3A4C, 0x77},  // --
    {0x3AE0, 0x02},  // --
    {0x3AEC, 0x0C},  // --
    {0x3B00, 0x2E},  // --
    {0x3B06, 0x29},  // --
    {0x3B98, 0x25},  // --
    {0x3B99, 0x21},  // --
    {0x3B9B, 0x13},  // --
    {0x3B9C, 0x13},  // --
    {0x3B9D, 0x13},  // --
    {0x3B9E, 0x13},  // --
    {0x3BA1, 0x00},  // --
    {0x3BA2, 0x06},  // --
    {0x3BA3, 0x0B},  // --
    {0x3BA4, 0x10},  // --
    {0x3BA5, 0x14},  // --
    {0x3BA6, 0x18},  // --
    {0x3BA7, 0x1A},  // --
    {0x3BA8, 0x1A},  // --
    {0x3BA9, 0x1A},  // --
    {0x3BAC, 0xED},  // --
    {0x3BAD, 0x01},  // --
    {0x3BAE, 0xF6},  // --
    {0x3BAF, 0x02},  // --
    {0x3BB0, 0xA2},  // --
    {0x3BB1, 0x03},  // --
    {0x3BB2, 0xE0},  // --
    {0x3BB3, 0x03},  // --
    {0x3BB4, 0xE0},  // --
    {0x3BB5, 0x03},  // --
    {0x3BB6, 0xE0},  // --
    {0x3BB7, 0x03},  // --
    {0x3BB8, 0xE0},  // --
    {0x3BBA, 0xE0},  // --
    {0x3BBC, 0xDA},  // --
    {0x3BBE, 0x88},  // --
    {0x3BC0, 0x44},  // --
    {0x3BC2, 0x7B},  // --
    {0x3BC4, 0xA2},  // --
    {0x3BC8, 0xBD},  // --
    {0x3BCA, 0xBD},  // --
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},
    {0x4018, 0x7F},  // TCLKPOST[15:0]
    {0x401A, 0x37},  // TCLKPREPARE[15:0]
    {0x401C, 0x37},  // TCLKTRAIL[15:0]
    {0x401E, 0xF7},  // TCLKZERO[15:0]
    {0x401F, 0x00},
    {0x4020, 0x3F},  // THSPREPARE[15:0]
    {0x4022, 0x6F},  // THSZERO[15:0]
    {0x4024, 0x3F},  // THSTRAIL[15:0]
    {0x4026, 0x5F},  // THSEXIT[15:0]
    {0x4028, 0x2F},  // TLPX[15:0]
    {0x4074, 0x01},  // TLPX[15:0]
    {0xffff, 0x10},
    {0x3000, 0x00},  //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},  //Master mode start
    {0x3260, 0x00},  //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_8m_24fps_init_table_4lane_HDR_DOL[] =
{
    /*
        IMX415-AAQRAll-pixel scan
        CSI-2_4lane 27MHz
        AD:10bit Output:10bit 1188Mbps
        Master Mode DOL HDR 2frame VC
        23.997fps
        Integration Time LEF:0.997ms SEF:0.108ms
    */

    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]

    {0x3024, 0x28},  // VMAX[15:0]
    {0x3025, 0x09},  //
    {0x3028, 0x94},  // HMAX[15:0]
    {0x3029, 0x02},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x01},  // WDSEL[1:0]
    {0x3031, 0x00},  //
    {0x3032, 0x00},  //
    {0x3033, 0x06},  // SYS_MODE[3:0]
    {0x3050, 0xE0},  // SHR0[19:0]
    {0x3051, 0x11},  //
    {0x3054, 0x09},  // SHR1[19:0]
    {0x3060, 0x15},  // RHS1[19:0]
    {0x30C0, 0x2A},
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x30CF, 0x01},  // XVSMSKCNT_INT[1:0]
    {0x3116, 0x23},  // INCKSEL2[7:0]
    {0x3118, 0x84},  // INCKSEL3[10:0]
    {0x311A, 0xE7},  // INCKSEL4[10:0]
    {0x311E, 0x23},  // INCKSEL5[7:0]
    {0x32D4, 0x21},  // --
    {0x32EC, 0xA1},  // --
    {0x344C, 0x2B},  // --
    {0x344D, 0x01},  // --
    {0x344E, 0xED},  // --
    {0x344F, 0x01},  // --
    {0x3450, 0xF6},  // --
    {0x3451, 0x02},  // --
    {0x3452, 0x7F},  // --
    {0x3453, 0x03},  // --
    {0x358A, 0x04},  // --
    {0x35A1, 0x02},  // --
    {0x35EC, 0x27},  // --
    {0x35EE, 0x8D},  // --
    {0x35F0, 0x8D},  // --
    {0x35F2, 0x29},  // --
    {0x36BC, 0x0C},  // --
    {0x36CC, 0x53},  // --
    {0x36CD, 0x00},  // --
    {0x36CE, 0x3C},  // --
    {0x36D0, 0x8C},  // --
    {0x36D1, 0x00},  // --
    {0x36D2, 0x71},  // --
    {0x36D4, 0x3C},  // --
    {0x36D6, 0x53},  // --
    {0x36D7, 0x00},  // --
    {0x36D8, 0x71},  // --
    {0x36DA, 0x8C},  // --
    {0x36DB, 0x00},  // --

    {0x3701, 0x00},  // --
    {0x3720, 0x07},  // --
    {0x3724, 0x02},  // --
    {0x3726, 0x02},  // --
    {0x3732, 0x02},  // --
    {0x3734, 0x03},  // --
    {0x3736, 0x03},  // --
    {0x3742, 0x03},  // --
    {0x3862, 0xE0},  // --
    {0x38CC, 0x30},  // --
    {0x38CD, 0x2F},  // --
    {0x395C, 0x0C},  // --
    {0x39A4, 0x07},  // --
    {0x39A8, 0x32},  // --
    {0x39AA, 0x32},  // --
    {0x39AC, 0x32},  // --
    {0x39AE, 0x32},  // --
    {0x39B0, 0x32},  // --
    {0x39B2, 0x2F},  // --
    {0x39B4, 0x2D},  // --
    {0x39B6, 0x28},  // --
    {0x39B8, 0x30},  // --
    {0x39BA, 0x30},  // --
    {0x39BC, 0x30},  // --
    {0x39BE, 0x30},  // --
    {0x39C0, 0x30},  // --
    {0x39C2, 0x2E},  // --
    {0x39C4, 0x2B},  // --
    {0x39C6, 0x25},  // --

    {0x3A42, 0xD1},  // --
    {0x3A4C, 0x77},  // --
    {0x3AE0, 0x02},  // --
    {0x3AEC, 0x0C},  // --
    {0x3B00, 0x2E},  // --
    {0x3B06, 0x29},  // --
    {0x3B98, 0x25},  // --
    {0x3B99, 0x21},  // --
    {0x3B9B, 0x13},  // --
    {0x3B9C, 0x13},  // --
    {0x3B9D, 0x13},  // --
    {0x3B9E, 0x13},  // --
    {0x3BA1, 0x00},  // --
    {0x3BA2, 0x06},  // --
    {0x3BA3, 0x0B},  // --
    {0x3BA4, 0x10},  // --
    {0x3BA5, 0x14},  // --
    {0x3BA6, 0x18},  // --
    {0x3BA7, 0x1A},  // --
    {0x3BA8, 0x1A},  // --
    {0x3BA9, 0x1A},  // --
    {0x3BAC, 0xED},  // --
    {0x3BAD, 0x01},  // --
    {0x3BAE, 0xF6},  // --
    {0x3BAF, 0x02},  // --
    {0x3BB0, 0xA2},  // --
    {0x3BB1, 0x03},  // --
    {0x3BB2, 0xE0},  // --
    {0x3BB3, 0x03},  // --
    {0x3BB4, 0xE0},  // --
    {0x3BB5, 0x03},  // --
    {0x3BB6, 0xE0},  // --
    {0x3BB7, 0x03},  // --
    {0x3BB8, 0xE0},  // --
    {0x3BBA, 0xE0},  // --
    {0x3BBC, 0xDA},  // --
    {0x3BBE, 0x88},  // --
    {0x3BC0, 0x44},  // --
    {0x3BC2, 0x7B},  // --
    {0x3BC4, 0xA2},  // --
    {0x3BC8, 0xBD},  // --
    {0x3BCA, 0xBD},  // --
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0},  // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06},  //
    {0x400C, 0x00},
    // MIPI CLK Timing
    {0x4018, 0x8F},  // TCLKPOST[15:0]
    {0x401A, 0x4F},  // TCLKPREPARE[15:0]
    {0x401C, 0x47},  // TCLKTRAIL[15:0]
    {0x401E, 0x37},  // TCLKZERO[15:0]
    {0x4020, 0x4F},  // THSPREPARE[15:0]
    {0x4022, 0x87},  // THSZERO[15:0]
    {0x4024, 0x4F},  // THSTRAIL[15:0]
    {0x4026, 0x7F},  // THSEXIT[15:0]
    {0x4028, 0x3F},  // TLPX[15:0]

    {0xffff, 0x10},
    {0x3000, 0x00},  //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},  //Master mode start
    {0x3260, 0x00},  //Gain_Pgc_Fidmd

#if 0
    {0x4018, 0xA7},  // TCLKPOST[15:0]
    {0x401A, 0x57},  // TCLKPREPARE[15:0]
    {0x401C, 0x5F},  // TCLKTRAIL[15:0]
    {0x401E, 0x97},  // TCLKZERO[15:0]
    {0x4020, 0x5F},  // THSPREPARE[15:0]
    {0x4022, 0xAF},  // THSZERO[15:0]
    {0x4024, 0x5F},  // THSTRAIL[15:0]
    {0x4026, 0x9F},  // THSEXIT[15:0]
    {0x4028, 0x4F},  // TLPX[15:0]
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
#endif
};

const static I2C_ARRAY Sensor_8M_30fps_init_table_4lane_HDR_DOL3[] = {
    /*
    IMX415-AAQR All-pixel scan CSI-2_4lane 27MHz AD:10bit
    Output:10bit 2376Mbps Master Mode DOL HDR 3frame VC 30.004fps
    Integration Time LEF:10.001ms SEF1:0.208ms SEF2:0.09ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop
    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]

    {0x3024, 0x9F},  // VMAX[19:0]
    {0x3025, 0x06},
    {0x3028, 0x6D},  // HMAX[15:0]
    {0x3029, 0x01},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x02},  // WDSEL[1:0]
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    {0x3033, 0x00},  // SYS_MODE[3:0]
    {0x3050, 0x8A},  // SHR0[19:0]
    {0x3051, 0x12},  //
    {0x3054, 0x0D},  // SHR1[19:0]
    {0x3058, 0x44},  // SHR2[19:0]
    {0x3060, 0x37},  // RHS1[19:0]
    {0x3064, 0x56},  // RHS2[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CF, 0x03},  // XVSMSKCNT_INT[1:0]
#if 0 // data rate 2079Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xE7}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 1 //data rate 2376Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0x08}, // INCKSEL3[10:0]
    {0x3119, 0x01}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3701, 0x00}, // ADBIT1[7:0]
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
#if 1  // data lane 2376Mbps
    {0x4018, 0xE7}, // TCLKPOST[15:0]
    {0x401A, 0x8F}, // TCLKPREPARE[15:0]
    {0x401C, 0x8F}, // TCLKTRAIL[15:0]
    {0x401E, 0x7F}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x97}, // THSPREPARE[15:0]
    {0x4022, 0x0F}, // THSZERO[15:0]
    {0x4023, 0x01}, //
    {0x4024, 0x97}, // THSTRAIL[15:0]
    {0x4026, 0xF7}, // THSEXIT[15:0]
    {0x4028, 0x7F}, // TLPX[15:0]
#endif
#if 0 // data lane setting to 2079Mbps
    {0x4018, 0xD7}, // TCLKPOST[15:0]
    {0x401A, 0x7F}, // TCLKPREPARE[15:0]
    {0x401C, 0x7F}, // TCLKTRAIL[15:0]
    {0x401E, 0x37}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x87}, // THSPREPARE[15:0]
    {0x4022, 0xEF}, // THSZERO[15:0]
    {0x4023, 0x00}, //
    {0x4024, 0x87}, // THSTRAIL[15:0]
    {0x4026, 0xDF}, // THSEXIT[15:0]
    {0x4028, 0x6F}, // TLPX[15:0]
#endif
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_5m_25fps_init_table_4lane_HDR_DOL3[] = {
    /*
    IMX415-AAQR Window cropping 2592x1944 CSI-2_4lane 27MHz
    AD:10bit Output:10bit 2376Mbps Master Mode DOL HDR 3frame VC
    22.513fps Integration Time LEF:10.001ms SEF1:0.208ms SEF2:0.09ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop

    {0x3008, 0x5D}, // BCWAIT_TIME[9:0]
    {0x300A, 0x42}, // CPWAIT_TIME[9:0]
    {0x301C, 0x04}, // WINMODE[3:0]
    {0x3024, 0xD3}, // VMAX[19:0]
    {0x3028, 0x6D}, // HMAX[15:0]
    {0x3029, 0x01}, //
    {0x302C, 0x01}, // WDMODE[1:0]
    {0x302D, 0x02}, // WDSEL[1:0]
    {0x3031, 0x00}, // ADBIT[1:0]
    {0x3032, 0x00}, // MDBIT
    // {0x3033, 0x04}, // SYS_MODE[3:0]  for 1782Mbps data rate
    {0x3033, 0x02}, // SYS_MODE[3:0]  for 2079Mbps data rate
    {0x3040, 0x7C}, // PIX_HST[12:0]
    {0x3041, 0x02}, //
    {0x3042, 0x20}, // PIX_HWIDTH[12:0]
    {0x3043, 0x0A}, //
    {0x3044, 0xF8}, // PIX_VST[12:0]
    {0x3046, 0x30}, // PIX_VWIDTH[12:0]
    {0x3047, 0x0F}, //
    {0x3050, 0x5A}, // SHR0[19:0]
    {0x3051, 0x1B}, //
    {0x3054, 0x0D}, // SHR1[19:0]
    {0x3058, 0x44}, // SHR2[19:0]
    {0x3060, 0x37}, // RHS1[19:0]
    {0x3064, 0x56}, // RHS2[19:0]
    {0x30C1, 0x00}, // XVS_DRV[1:0]
    {0x30CF, 0x03}, // XVSMSKCNT_INT[1:0]
#if 0 // data rate 2079Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xE7}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 1 //data rate 2376Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0x08}, // INCKSEL3[10:0]
    {0x3119, 0x01}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 0  // for 1782Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xC6}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3701, 0x00}, // ADBIT1[7:0]
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
#if 1  // data lane setting to 2376Mbps
    {0x4018, 0xE7}, // TCLKPOST[15:0]
    {0x401A, 0x8F}, // TCLKPREPARE[15:0]
    {0x401C, 0x8F}, // TCLKTRAIL[15:0]
    {0x401E, 0x7F}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x97}, // THSPREPARE[15:0]
    {0x4022, 0x0F}, // THSZERO[15:0]
    {0x4023, 0x01}, //
    {0x4024, 0x97}, // THSTRAIL[15:0]
    {0x4026, 0xF7}, // THSEXIT[15:0]
    {0x4028, 0x7F}, // TLPX[15:0]
#endif
#if 0 // data lane setting to 2079Mbps
    {0x4018, 0xD7}, // TCLKPOST[15:0]
    {0x401A, 0x7F}, // TCLKPREPARE[15:0]
    {0x401C, 0x7F}, // TCLKTRAIL[15:0]
    {0x401E, 0x37}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x87}, // THSPREPARE[15:0]
    {0x4022, 0xEF}, // THSZERO[15:0]
    {0x4023, 0x00}, //
    {0x4024, 0x87}, // THSTRAIL[15:0]
    {0x4026, 0xDF}, // THSEXIT[15:0]
    {0x4028, 0x6F}, // TLPX[15:0]
#endif
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_4p8m_25fps_init_table_4lane_HDR_DOL3[] = {
    /*
    IMX415-AAQR Window cropping 2952x1656 CSI-2_4lane 27MHz
    AD:10bit Output:10bit 2376Mbps Master Mode DOL HDR 3frame VC
    22.513fps Integration Time LEF:10.001ms SEF1:0.208ms SEF2:0.09ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop

    {0x3008, 0x5D}, // BCWAIT_TIME[9:0]
    {0x300A, 0x42}, // CPWAIT_TIME[9:0]
    {0x301C, 0x04}, // WINMODE[3:0]
    {0x3024, 0xD3}, // VMAX[19:0]
    {0x3028, 0x6D}, // HMAX[15:0]
    {0x3029, 0x01}, //
    {0x302C, 0x01}, // WDMODE[1:0]
    {0x302D, 0x02}, // WDSEL[1:0]
    {0x3031, 0x00}, // ADBIT[1:0]
    {0x3032, 0x00}, // MDBIT
    // {0x3033, 0x04}, // SYS_MODE[3:0]  for 1782Mbps
    {0x3033, 0x02}, // SYS_MODE[3:0]  for 2079Mbps
    {0x3040, 0xC8}, // PIX_HST[12:0]
    {0x3041, 0x01}, //
    {0x3042, 0x88}, // PIX_HWIDTH[12:0]
    {0x3043, 0x0B}, //
    {0x3044, 0x18}, // PIX_VST[12:0]
    {0x3045, 0x02}, //
    {0x3046, 0xF0}, // PIX_VWIDTH[12:0]
    {0x3047, 0x0C}, //
    {0x3050, 0x5A}, // SHR0[19:0]
    {0x3051, 0x1B}, //
    {0x3054, 0x0D}, // SHR1[19:0]
    {0x3058, 0x44}, // SHR2[19:0]
    {0x3060, 0x37}, // RHS1[19:0]
    {0x3064, 0x56}, // RHS2[19:0]
    {0x30C1, 0x00}, // XVS_DRV[1:0]
    {0x30CF, 0x03}, // XVSMSKCNT_INT[1:0]
#if 0 // data rate 2079Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xE7}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 0  //data rate 1782Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xC6}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 1 //data rate 2376Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0x08}, // INCKSEL3[10:0]
    {0x3119, 0x01}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3701, 0x00}, // ADBIT1[7:0]
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
#if 1  // data lane 2376Mbps
    {0x4018, 0xE7}, // TCLKPOST[15:0]
    {0x401A, 0x8F}, // TCLKPREPARE[15:0]
    {0x401C, 0x8F}, // TCLKTRAIL[15:0]
    {0x401E, 0x7F}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x97}, // THSPREPARE[15:0]
    {0x4022, 0x0F}, // THSZERO[15:0]
    {0x4023, 0x01}, //
    {0x4024, 0x97}, // THSTRAIL[15:0]
    {0x4026, 0xF7}, // THSEXIT[15:0]
    {0x4028, 0x7F}, // TLPX[15:0]
#endif
#if 0 // data lane setting to 2079Mbps
    {0x4018, 0xD7}, // TCLKPOST[15:0]
    {0x401A, 0x7F}, // TCLKPREPARE[15:0]
    {0x401C, 0x7F}, // TCLKTRAIL[15:0]
    {0x401E, 0x37}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x87}, // THSPREPARE[15:0]
    {0x4022, 0xEF}, // THSZERO[15:0]
    {0x4023, 0x00}, //
    {0x4024, 0x87}, // THSTRAIL[15:0]
    {0x4026, 0xDF}, // THSEXIT[15:0]
    {0x4028, 0x6F}, // TLPX[15:0]
#endif
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_4m_25fps_init_table_4lane_HDR_DOL3[] = {
    /*
    IMX415-AAQR Window cropping 2688x1520 CSI-2_4lane 27MHz
    AD:10bit Output:10bit 2376Mbps Master Mode DOL HDR 3frame VC 22.513fps
    Integration Time LEF:10.001ms SEF1:0.208ms SEF2:0.12ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop

    {0x3008, 0x5D},  // BCWAIT_TIME[9:0]
    {0x300A, 0x42},  // CPWAIT_TIME[9:0]
    {0x301C, 0x04},  // WINMODE[3:0]
    {0x3024, 0xD3},  // VMAX[19:0]
    {0x3028, 0x6D},  // HMAX[15:0]
    {0x3029, 0x01},  //
    {0x302C, 0x01},  // WDMODE[1:0]
    {0x302D, 0x02},  // WDSEL[1:0]
    {0x3031, 0x00},  // ADBIT[1:0]
    {0x3032, 0x00},  // MDBIT
    // {0x3033, 0x04},  // SYS_MODE[3:0] for 1782 Mbps
    {0x3033, 0x02},  // SYS_MODE[3:0] for 2079Mbps
    {0x3040, 0x4C},  // PIX_HST[12:0]
    {0x3041, 0x02},  //
    {0x3042, 0x80},  // PIX_HWIDTH[12:0]
    {0x3043, 0x0A},  //
    {0x3044, 0xA0},  // PIX_VST[12:0]
    {0x3045, 0x02},
    {0x3046, 0xE0},  // PIX_VWIDTH[12:0]
    {0x3047, 0x0B},  //
    {0x3050, 0x5A},  // SHR0[19:0]
    {0x3051, 0x1B},  //
    {0x3054, 0x0D},  // SHR1[19:0]
    {0x3058, 0x44},  // SHR2[19:0]
    {0x3060, 0x37},  // RHS1[19:0]
    {0x3064, 0x5C},  // RHS2[19:0]
    {0x30C1, 0x00},  // XVS_DRV[1:0]
    {0x30CF, 0x03},  // XVSMSKCNT_INT[1:0]
#if 0 // data rate 2079Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xE7}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 0  // for data rate 1782Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xC6}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
//#if 0 //data rate 2376Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0x08}, // INCKSEL3[10:0]
    {0x3119, 0x01}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
//#endif
    {0x32D4, 0x21},  // -
    {0x32EC, 0xA1},  // -
    {0x344C, 0x2B},  // -
    {0x344D, 0x01},  // -
    {0x344E, 0xED},  // -
    {0x344F, 0x01},  // -
    {0x3450, 0xF6},  // -
    {0x3451, 0x02},  // -
    {0x3452, 0x7F},  // -
    {0x3453, 0x03},  // -
    {0x358A, 0x04},  // -
    {0x35A1, 0x02},  // -
    {0x35EC, 0x27},  // -
    {0x35EE, 0x8D},  // -
    {0x35F0, 0x8D},  // -
    {0x35F2, 0x29},  // -
    {0x36BC, 0x0C},  // -
    {0x36CC, 0x53},  // -
    {0x36CD, 0x00},  // -
    {0x36CE, 0x3C},  // -
    {0x36D0, 0x8C},  // -
    {0x36D1, 0x00},  // -
    {0x36D2, 0x71},  // -
    {0x36D4, 0x3C},  // -
    {0x36D6, 0x53},  // -
    {0x36D7, 0x00},  // -
    {0x36D8, 0x71},  // -
    {0x36DA, 0x8C},  // -
    {0x36DB, 0x00},  // -
    {0x3701, 0x00},  // ADBIT1[7:0]
    {0x3720, 0x00},  // -
    {0x3724, 0x02},  // -
    {0x3726, 0x02},  // -
    {0x3732, 0x02},  // -
    {0x3734, 0x03},  // -
    {0x3736, 0x03},  // -
    {0x3742, 0x03},  // -
    {0x3862, 0xE0},  // -
    {0x38CC, 0x30},  // -
    {0x38CD, 0x2F},  // -
    {0x395C, 0x0C},  // -
    {0x39A4, 0x07},  // -
    {0x39A8, 0x32},  // -
    {0x39AA, 0x32},  // -
    {0x39AC, 0x32},  // -
    {0x39AE, 0x32},  // -
    {0x39B0, 0x32},  // -
    {0x39B2, 0x2F},  // -
    {0x39B4, 0x2D},  // -
    {0x39B6, 0x28},  // -
    {0x39B8, 0x30},  // -
    {0x39BA, 0x30},  // -
    {0x39BC, 0x30},  // -
    {0x39BE, 0x30},  // -
    {0x39C0, 0x30},  // -
    {0x39C2, 0x2E},  // -
    {0x39C4, 0x2B},  // -
    {0x39C6, 0x25},  // -
    {0x3A42, 0xD1},  // -
    {0x3A4C, 0x77},  // -
    {0x3AE0, 0x02},  // -
    {0x3AEC, 0x0C},  // -
    {0x3B00, 0x2E},  // -
    {0x3B06, 0x29},  // -
    {0x3B98, 0x25},  // -
    {0x3B99, 0x21},  // -
    {0x3B9B, 0x13},  // -
    {0x3B9C, 0x13},  // -
    {0x3B9D, 0x13},  // -
    {0x3B9E, 0x13},  // -
    {0x3BA1, 0x00},  // -
    {0x3BA2, 0x06},  // -
    {0x3BA3, 0x0B},  // -
    {0x3BA4, 0x10},  // -
    {0x3BA5, 0x14},  // -
    {0x3BA6, 0x18},  // -
    {0x3BA7, 0x1A},  // -
    {0x3BA8, 0x1A},  // -
    {0x3BA9, 0x1A},  // -
    {0x3BAC, 0xED},  // -
    {0x3BAD, 0x01},  // -
    {0x3BAE, 0xF6},  // -
    {0x3BAF, 0x02},  // -
    {0x3BB0, 0xA2},  // -
    {0x3BB1, 0x03},  // -
    {0x3BB2, 0xE0},  // -
    {0x3BB3, 0x03},  // -
    {0x3BB4, 0xE0},  // -
    {0x3BB5, 0x03},  // -
    {0x3BB6, 0xE0},  // -
    {0x3BB7, 0x03},  // -
    {0x3BB8, 0xE0},  // -
    {0x3BBA, 0xE0},  // -
    {0x3BBC, 0xDA},  // -
    {0x3BBE, 0x88},  // -
    {0x3BC0, 0x44},  // -
    {0x3BC2, 0x7B},  // -
    {0x3BC4, 0xA2},  // -
    {0x3BC8, 0xBD},  // -
    {0x3BCA, 0xBD},  // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
#if 1  // data lane 2376Mbps
    {0x4018, 0xE7}, // TCLKPOST[15:0]
    {0x401A, 0x8F}, // TCLKPREPARE[15:0]
    {0x401C, 0x8F}, // TCLKTRAIL[15:0]
    {0x401E, 0x7F}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x97}, // THSPREPARE[15:0]
    {0x4022, 0x0F}, // THSZERO[15:0]
    {0x4023, 0x01}, //
    {0x4024, 0x97}, // THSTRAIL[15:0]
    {0x4026, 0xF7}, // THSEXIT[15:0]
    {0x4028, 0x7F}, // TLPX[15:0]
#endif
#if 0 // data lane setting to 2079Mbps
    {0x4018, 0xD7}, // TCLKPOST[15:0]
    {0x401A, 0x7F}, // TCLKPREPARE[15:0]
    {0x401C, 0x7F}, // TCLKTRAIL[15:0]
    {0x401E, 0x37}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x87}, // THSPREPARE[15:0]
    {0x4022, 0xEF}, // THSZERO[15:0]
    {0x4023, 0x00}, //
    {0x4024, 0x87}, // THSTRAIL[15:0]
    {0x4026, 0xDF}, // THSEXIT[15:0]
    {0x4028, 0x6F}, // TLPX[15:0]
#endif
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},    //Master mode start
    {0x3260, 0x00},    //Gain_Pgc_Fidmd

};

const static I2C_ARRAY Sensor_2m_22fps_init_table_4lane_HDR_DOL3[] = {
    /*
    IMX415-AAQR 2/2-line binning CSI-2_4lane 27MHz
    AD:10bit Output:12bit 2376Mbps Master Mode DOL HDR 3frame VC 22.513fps
    Integration Time LEF:10.001ms SEF1:0.208ms SEF2:0.09ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop

    {0x3008, 0x5D}, // BCWAIT_TIME[9:0]
    {0x300A, 0x42}, // CPWAIT_TIME[9:0]
    {0x3020, 0x01}, // HADD
    {0x3021, 0x01}, // VADD
    {0x3022, 0x01}, // ADDMODE[1:0]
    {0x3024, 0xD3}, // VMAX[19:0] for 22.5fps, tested
    {0x3025, 0x08}, //
    {0x3028, 0x6D}, // HMAX[15:0]
    {0x3029, 0x01}, //
    {0x302C, 0x01}, // WDMODE[1:0]
    {0x302D, 0x02}, // WDSEL[1:0]
    {0x3031, 0x00}, // ADBIT[1:0]
    {0x3033, 0x00}, // SYS_MODE[3:0] 2376 Mbps
//    {0x3033, 0x04}, // SYS_MODE[3:0] 1782Mbps
    {0x3050, 0x5A}, // SHR0[19:0]
    {0x3051, 0x1B}, //
    {0x3054, 0x0D}, // SHR1[19:0]
    {0x3058, 0x44}, // SHR2[19:0]
    {0x3060, 0x37}, // RHS1[19:0]
    {0x3064, 0x56}, // RHS2[19:0]
    {0x30C1, 0x00}, // XVS_DRV[1:0]
    {0x30CF, 0x03}, // XVSMSKCNT_INT[1:0]
    {0x30D9, 0x02}, // DIG_CLP_VSTART[4:0]
    {0x30DA, 0x01}, // DIG_CLP_VNUM[1:0]
#if 0 //data rate 1782Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xC6}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 1 //data rate 2376Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0x08}, // INCKSEL3[10:0]
    {0x3119, 0x01}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3701, 0x00}, // ADBIT1[7:0]
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
#if 1 // for 2376Mbps
    {0x4018, 0xE7}, // TCLKPOST[15:0]
    {0x401A, 0x8F}, // TCLKPREPARE[15:0]
    {0x401C, 0x8F}, // TCLKTRAIL[15:0]
    {0x401E, 0x7F}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x97}, // THSPREPARE[15:0]
    {0x4022, 0x0F}, // THSZERO[15:0]
    {0x4023, 0x01}, //
    {0x4024, 0x97}, // THSTRAIL[15:0]
    {0x4026, 0xF7}, // THSEXIT[15:0]
    {0x4028, 0x7F}, // TLPX[15:0]
#endif
    {0xffff, 0x10},
    {0x3000, 0x00},  //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},  //Master mode start
    {0x3260, 0x00},  //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_2m_30fps_init_table_4lane_HDR_DOL3[] = {
    /*
    IMX415-AAQR 2/2-line binning CSI-2_4lane 27MHz
    AD:10bit Output:12bit 2376Mbps Master Mode DOL HDR 3frame VC 22.513fps
    Integration Time LEF:10.001ms SEF1:0.208ms SEF2:0.09ms
    Tool ver : Ver10.0
    */
    {0x3000, 0x01},  //Standby
    {0x3002, 0x01},  //Master mode stop

    {0x3008, 0x5D}, // BCWAIT_TIME[9:0]
    {0x300A, 0x42}, // CPWAIT_TIME[9:0]
    {0x3020, 0x01}, // HADD
    {0x3021, 0x01}, // VADD
    {0x3022, 0x01}, // ADDMODE[1:0]
    {0x3024, 0x9E}, // VMAX[19:0] for 30fps, tested
    {0x3025, 0x06}, //
    {0x3028, 0x6D}, // HMAX[15:0]
    {0x3029, 0x01}, //
    {0x302C, 0x01}, // WDMODE[1:0]
    {0x302D, 0x02}, // WDSEL[1:0]
    {0x3031, 0x00}, // ADBIT[1:0]
    {0x3033, 0x00}, // SYS_MODE[3:0] 2376 Mbps
//    {0x3033, 0x04}, // SYS_MODE[3:0] 1782Mbps
    {0x3050, 0x5A}, // SHR0[19:0]
    {0x3051, 0x1B}, //
    {0x3054, 0x0D}, // SHR1[19:0]
    {0x3058, 0x44}, // SHR2[19:0]
    {0x3060, 0x37}, // RHS1[19:0]
    {0x3064, 0x56}, // RHS2[19:0]
    {0x30C1, 0x00}, // XVS_DRV[1:0]
    {0x30CF, 0x03}, // XVSMSKCNT_INT[1:0]
    {0x30D9, 0x02}, // DIG_CLP_VSTART[4:0]
    {0x30DA, 0x01}, // DIG_CLP_VNUM[1:0]
#if 0 //data rate 1782Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0xC6}, // INCKSEL3[10:0]
    {0x3119, 0x00}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
#if 1 //data rate 2376Mbps
    {0x3116, 0x23}, // INCKSEL2[7:0]
    {0x3118, 0x08}, // INCKSEL3[10:0]
    {0x3119, 0x01}, //
    {0x311A, 0xE7}, // INCKSEL4[10:0]
    {0x311E, 0x23}, // INCKSEL5[7:0]
#endif
    {0x32D4, 0x21}, // -
    {0x32EC, 0xA1}, // -
    {0x344C, 0x2B}, // -
    {0x344D, 0x01}, // -
    {0x344E, 0xED}, // -
    {0x344F, 0x01}, // -
    {0x3450, 0xF6}, // -
    {0x3451, 0x02}, // -
    {0x3452, 0x7F}, // -
    {0x3453, 0x03}, // -
    {0x358A, 0x04}, // -
    {0x35A1, 0x02}, // -
    {0x35EC, 0x27}, // -
    {0x35EE, 0x8D}, // -
    {0x35F0, 0x8D}, // -
    {0x35F2, 0x29}, // -
    {0x36BC, 0x0C}, // -
    {0x36CC, 0x53}, // -
    {0x36CD, 0x00}, // -
    {0x36CE, 0x3C}, // -
    {0x36D0, 0x8C}, // -
    {0x36D1, 0x00}, // -
    {0x36D2, 0x71}, // -
    {0x36D4, 0x3C}, // -
    {0x36D6, 0x53}, // -
    {0x36D7, 0x00}, // -
    {0x36D8, 0x71}, // -
    {0x36DA, 0x8C}, // -
    {0x36DB, 0x00}, // -
    {0x3701, 0x00}, // ADBIT1[7:0]
    {0x3720, 0x00}, // -
    {0x3724, 0x02}, // -
    {0x3726, 0x02}, // -
    {0x3732, 0x02}, // -
    {0x3734, 0x03}, // -
    {0x3736, 0x03}, // -
    {0x3742, 0x03}, // -
    {0x3862, 0xE0}, // -
    {0x38CC, 0x30}, // -
    {0x38CD, 0x2F}, // -
    {0x395C, 0x0C}, // -
    {0x39A4, 0x07}, // -
    {0x39A8, 0x32}, // -
    {0x39AA, 0x32}, // -
    {0x39AC, 0x32}, // -
    {0x39AE, 0x32}, // -
    {0x39B0, 0x32}, // -
    {0x39B2, 0x2F}, // -
    {0x39B4, 0x2D}, // -
    {0x39B6, 0x28}, // -
    {0x39B8, 0x30}, // -
    {0x39BA, 0x30}, // -
    {0x39BC, 0x30}, // -
    {0x39BE, 0x30}, // -
    {0x39C0, 0x30}, // -
    {0x39C2, 0x2E}, // -
    {0x39C4, 0x2B}, // -
    {0x39C6, 0x25}, // -
    {0x3A42, 0xD1}, // -
    {0x3A4C, 0x77}, // -
    {0x3AE0, 0x02}, // -
    {0x3AEC, 0x0C}, // -
    {0x3B00, 0x2E}, // -
    {0x3B06, 0x29}, // -
    {0x3B98, 0x25}, // -
    {0x3B99, 0x21}, // -
    {0x3B9B, 0x13}, // -
    {0x3B9C, 0x13}, // -
    {0x3B9D, 0x13}, // -
    {0x3B9E, 0x13}, // -
    {0x3BA1, 0x00}, // -
    {0x3BA2, 0x06}, // -
    {0x3BA3, 0x0B}, // -
    {0x3BA4, 0x10}, // -
    {0x3BA5, 0x14}, // -
    {0x3BA6, 0x18}, // -
    {0x3BA7, 0x1A}, // -
    {0x3BA8, 0x1A}, // -
    {0x3BA9, 0x1A}, // -
    {0x3BAC, 0xED}, // -
    {0x3BAD, 0x01}, // -
    {0x3BAE, 0xF6}, // -
    {0x3BAF, 0x02}, // -
    {0x3BB0, 0xA2}, // -
    {0x3BB1, 0x03}, // -
    {0x3BB2, 0xE0}, // -
    {0x3BB3, 0x03}, // -
    {0x3BB4, 0xE0}, // -
    {0x3BB5, 0x03}, // -
    {0x3BB6, 0xE0}, // -
    {0x3BB7, 0x03}, // -
    {0x3BB8, 0xE0}, // -
    {0x3BBA, 0xE0}, // -
    {0x3BBC, 0xDA}, // -
    {0x3BBE, 0x88}, // -
    {0x3BC0, 0x44}, // -
    {0x3BC2, 0x7B}, // -
    {0x3BC4, 0xA2}, // -
    {0x3BC8, 0xBD}, // -
    {0x3BCA, 0xBD}, // -
    {0x4000, 0x11},    //mipi clk non-continouns mode
    {0x4004, 0xC0}, // TXCLKESC_FREQ[15:0]
    {0x4005, 0x06}, //
#if 1 // for 2376Mbps
    {0x4018, 0xE7}, // TCLKPOST[15:0]
    {0x401A, 0x8F}, // TCLKPREPARE[15:0]
    {0x401C, 0x8F}, // TCLKTRAIL[15:0]
    {0x401E, 0x7F}, // TCLKZERO[15:0]
    {0x401F, 0x02}, //
    {0x4020, 0x97}, // THSPREPARE[15:0]
    {0x4022, 0x0F}, // THSZERO[15:0]
    {0x4023, 0x01}, //
    {0x4024, 0x97}, // THSTRAIL[15:0]
    {0x4026, 0xF7}, // THSEXIT[15:0]
    {0x4028, 0x7F}, // TLPX[15:0]
#endif
    {0xffff, 0x10},
    {0x3000, 0x00},  //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},  //Master mode start
    {0x3260, 0x00},  //Gain_Pgc_Fidmd
};

const static I2C_ARRAY Sensor_id_table[] = {
    {0x3F12, 0x14},      // {address of ID, ID },
    {0x3F13, 0x75},      // {address of ID, ID },
};

static I2C_ARRAY PatternTbl[] = {
    {0x0000, 0x00},       // colorbar pattern , bit 0 to enable
};

static I2C_ARRAY StreamOnOffTbl[] = {
    {0x3000, 0x00},  //Operating
    {0xffff, 0x10},
    {0x3002, 0x00},  //Master mode start
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

const I2C_ARRAY expo_IMX415_RHS1_reg[] = {
    //decreasing exposure ratio version.
    {0x3062, 0x00},
    {0x3061, 0x00},
    {0x3060, 0x11},
};

const static I2C_ARRAY expo_DOL3_SHR0_reg[] = { // DOL3 SHS0 (For LEF)
    {0x3052, 0x00},
    {0x3051, 0x12},
    {0x3050, 0x8A},
};

const static I2C_ARRAY expo_DOL3_SHR1_reg[] = { // DOL3 SHS1 (For SEF)
    //decreasing exposure ratio version.
    {0x3056, 0x00},
    {0x3055, 0x00},
    {0x3054, 0x0D},
};

const static I2C_ARRAY expo_DOL3_SHR2_reg[] = { // DOL3 SHS1 (For SEF2)
    //decreasing exposure ratio version.
    {0x305A, 0x00},
    {0x3059, 0x00},
    {0x3058, 0x44},
};

const I2C_ARRAY expo_IMX415_DOL3_RHS1_reg[] = { // DOL3 RHS1 (For SEF)
    {0x3062, 0x00},
    {0x3061, 0x00},
    {0x3060, 0x37},
};

const I2C_ARRAY expo_IMX415_DOL3_RHS2_reg[] = { // DOL3 RHS2 (For SEF2)
    {0x3066, 0x00},
    {0x3065, 0x00},
    {0x3064, 0x56},
};

const static I2C_ARRAY gain_reg[] = {
    {0x3090, 0x2A},//low bit
    {0x3091, 0x00},//hcg mode,bit 4
};

const static I2C_ARRAY gain_HDR_DOL_LEF_reg[] = {
    {0x3090, 0x2A},
    {0x3091, 0x00},
};

const static I2C_ARRAY gain_HDR_DOL_SEF_reg[] = {
    {0x3092, 0x20},
    {0x3093, 0x00},
};

const static I2C_ARRAY gain_HDR_DOL_SEF2_reg[] = {
    {0x3094, 0x20},
    {0x3095, 0x00},
};

//static int g_sensor_ae_min_gain = 1024;
//static CUS_GAIN_GAP_ARRAY gain_gap_compensate[16] = {  //compensate  gain gap
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0},
//    {0, 0}
//};

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

#define IMX415_AE_DEBUG_MSG(handle) {                               \
    imx415_params *params = (imx415_params *)handle->private_data;  \
    SENSOR_DMSG(KERN_DEBUG "vts %x%x%x"                             \
        ,params->tVts_reg[0].data                                   \
        ,params->tVts_reg[1].data                                   \
        ,params->tVts_reg[2].data);                                 \
    SENSOR_DMSG(KERN_DEBUG "shr0 %x%x%x"                            \
        ,params->tSHR0_reg[0].data                                  \
        ,params->tSHR0_reg[1].data                                  \
        ,params->tSHR0_reg[2].data);                                \
    SENSOR_DMSG(KERN_DEBUG "shr1 %x%x%x"                            \
        ,params->tSHR1_reg[0].data                                  \
        ,params->tSHR1_reg[1].data                                  \
        ,params->tSHR1_reg[2].data);                                \
    SENSOR_DMSG(KERN_DEBUG "shr2 %x%x%x"                            \
        ,params->tSHR2_reg[0].data                                  \
        ,params->tSHR2_reg[1].data                                  \
        ,params->tSHR2_reg[2].data);                                \
    SENSOR_DMSG(KERN_DEBUG "rhs1 %x%x%x"                            \
        ,params->tRHS1_reg[0].data                                  \
        ,params->tRHS1_reg[1].data                                  \
        ,params->tRHS1_reg[2].data);                                \
    SENSOR_DMSG(KERN_DEBUG "rhs2 %x%x%x"                            \
        ,params->tRHS2_reg[0].data                                  \
        ,params->tRHS2_reg[1].data                                  \
        ,params->tRHS2_reg[2].data);                                \
}                                                                   \
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

static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

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

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    //Sensor power on sequence
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);   // Powerdn Pull Low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);     // Rst Pull Low
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, params->csi_clk);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    //Sensor board PWDN Enable, 1.8V & 2.9V need 30ms then Pull High
    SENSOR_MSLEEP(40);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_UDELAY(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_UDELAY(20);
    SENSOR_DMSG("Sensor Power On finished\n");
    params->PowerOnOff  = 1;

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx415_params *params = (imx415_params *)handle->private_data;
    params->PowerOnOff  = 0;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);     // Rst Pull Low
    SENSOR_MSLEEP(1);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);   // Powerdn Pull Low
    SENSOR_MSLEEP(1);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }

    params->orient = SENSOR_ORIT;

    return SUCCESS;
}

int pCus_sensor_str(struct __ss_cus_sensor* handle, u32 idx, CUS_SENSOR_STR_MODE mode)
{
    if(mode == CUS_SENSOR_RESUME)
    {
        handle->pCus_sensor_poweron(handle, idx);
        handle->pCus_sensor_init(handle);
    }
    else if(mode == CUS_SENSOR_SUSPEND)
    {
        handle->pCus_sensor_poweroff(handle, idx);
    }
    else
    {
        SENSOR_EMSG(">>>>>>>[%s] mode %d err \n", __FUNCTION__, mode);
    }
    return SUCCESS;
}

static int pCus_StreamOn(struct __ss_cus_sensor *handle, u32 idx)
{
    int i;

    for(i=0;i< ARRAY_SIZE(StreamOnOffTbl);i++)
    {
        if(StreamOnOffTbl[i].reg==0xffff)
        {
            SENSOR_MSLEEP(StreamOnOffTbl[i].data);
        }

        if(SensorReg_Write(StreamOnOffTbl[i].reg,StreamOnOffTbl[i].data) != SUCCESS)
        {
            SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }

    SENSOR_EMSG("[%s] sensor stream on!\n", __FUNCTION__);
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
            SENSOR_DMSG("[%s] Please Check IMX415 Sensor Insert!!\n", __FUNCTION__);
            return FAIL;
       }
       *id = id_from_sensor[i].data;
    }

    SENSOR_DMSG("[%s]IMX415 sensor ,Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}

static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    imx415_params *params = (imx415_params *)handle->private_data;

    info->u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    info->u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    info->u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    info->u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    info->u32AEGain_min                      = SENSOR_MIN_GAIN;
    info->u32AEGain_max                      = SENSOR_MAX_GAIN;
    info->u32AEShutter_min                   = params->Line_Period * 1;
    info->u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    info->u32AEShutter_step                  = params->Line_Period;
    return SUCCESS;
}

static int pCus_sensor_GetAEInfo_SEF(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    imx415_params *params = (imx415_params *)handle->private_data;

    info->u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_DOL;
    info->u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_DOL;
    info->u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    info->u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    info->u32AEGain_min                      = SENSOR_MIN_GAIN;
    info->u32AEGain_max                      = SENSOR_MAX_GAIN;
    info->u32AEShutter_min                   = params->Line_Period * 5;
    info->u32AEShutter_max                   = params->Line_Period * params->min_rhs1;
    info->u32AEShutter_step                  = params->Line_Period;
    return SUCCESS;
}

static int pCus_sensor_GetAEInfo_LEF(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    imx415_params *params = (imx415_params *)handle->private_data;

    info->u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_DOL;
    info->u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_DOL;
    info->u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    info->u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    info->u32AEGain_min                      = SENSOR_MIN_GAIN;
    info->u32AEGain_max                      = SENSOR_MAX_GAIN;
    info->u32AEShutter_min                   = params->Line_Period * 5;
    info->u32AEShutter_max                   = 1000000000/imx415_mipi_hdr[0].senout.min_fps;
    info->u32AEShutter_step                  = params->Line_Period;
    return SUCCESS;
}

static int imx415_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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

static int pCus_init_mipi4lane_linear(ss_cus_sensor *handle)
{
    //imx415_params *params = (imx415_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_4lane_linear);i++)
    {
        if(Sensor_init_table_4lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_linear[i].reg, Sensor_init_table_4lane_linear[i].data) != SUCCESS)
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
                //printk("IMX415 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_4lane_linear[i].reg, Sensor_init_table_4lane_linear[i].data, sen_data);
#endif
        }
    }

    return SUCCESS;
}

static int pCus_init_mipi4lane_HDR_DOL(ss_cus_sensor *handle)
{
    //imx415_params *params = (imx415_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_4lane_HDR_DOL);i++)
    {
        if(Sensor_init_table_4lane_HDR_DOL[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_HDR_DOL[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_HDR_DOL[i].reg,Sensor_init_table_4lane_HDR_DOL[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
            //printk("\n reg 0x%x, 0x%x",Sensor_init_table_4lane_HDR_DOL[i].reg, Sensor_init_table_4lane_HDR_DOL[i].data);
#if 0
            SensorReg_Read(Sensor_init_table_4lane_HDR_DOL[i].reg, &sen_data );
            if(Sensor_init_table_4lane_linear[i].data != sen_data)
                printk("HDR R/W Differ Reg: 0x%x\n",Sensor_init_table_4lane_HDR_DOL[i].reg);
                //printk("IMX415 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_4lane_HDR_DOL[i].reg, Sensor_init_table_4lane_HDR_DOL[i].data, sen_data);
#endif
        }
    }

    SENSOR_DMSG("Sensor IMX415 HDR MODE Initial Finished\n");
    return SUCCESS;
}

static void imx415_AEparam_init(ss_cus_sensor *handle)
{
    u16 Hsb, Msb, Lsb;
    imx415_params *params = (imx415_params *)handle->private_data;

    SensorReg_Read(params->tVts_reg[0].reg, (void*)&Hsb);
    SensorReg_Read(params->tVts_reg[1].reg, (void*)&Msb);
    SensorReg_Read(params->tVts_reg[2].reg, (void*)&Lsb);

    params->tVts_reg[0].data = Hsb & 0x0003;
    params->tVts_reg[1].data = Msb & 0x00ff;
    params->tVts_reg[2].data = Lsb & 0x00ff;

    SensorReg_Read(params->tSHR0_reg[0].reg, (void*)&Hsb);
    SensorReg_Read(params->tSHR0_reg[1].reg, (void*)&Msb);
    SensorReg_Read(params->tSHR0_reg[2].reg, (void*)&Lsb);

    params->tSHR0_reg[0].data = Hsb & 0x0003;
    params->tSHR0_reg[1].data = Msb & 0x00ff;
    params->tSHR0_reg[2].data = Lsb & 0x00ff;

    SensorReg_Read(params->tSHR1_reg[0].reg, (void*)&Hsb);
    SensorReg_Read(params->tSHR1_reg[1].reg, (void*)&Msb);
    SensorReg_Read(params->tSHR1_reg[2].reg, (void*)&Lsb);

    params->tSHR1_reg[0].data = Hsb & 0x0003;
    params->tSHR1_reg[1].data = Msb & 0x00ff;
    params->tSHR1_reg[2].data = Lsb & 0x00ff;

    SensorReg_Read(params->tSHR2_reg[0].reg, (void*)&Hsb);
    SensorReg_Read(params->tSHR2_reg[1].reg, (void*)&Msb);
    SensorReg_Read(params->tSHR2_reg[2].reg, (void*)&Lsb);

    params->tSHR2_reg[0].data = Hsb & 0x0003;
    params->tSHR2_reg[1].data = Msb & 0x00ff;
    params->tSHR2_reg[2].data = Lsb & 0x00ff;


    SensorReg_Read(params->tRHS1_reg[0].reg, (void*)&Hsb);
    SensorReg_Read(params->tRHS1_reg[1].reg, (void*)&Msb);
    SensorReg_Read(params->tRHS1_reg[2].reg, (void*)&Lsb);

    params->tRHS1_reg[0].data = Hsb & 0x0003;
    params->tRHS1_reg[1].data = Msb & 0x00ff;
    params->tRHS1_reg[2].data = Lsb & 0x00ff;

    SensorReg_Read(params->tRHS2_reg[0].reg, (void*)&Hsb);
    SensorReg_Read(params->tRHS2_reg[1].reg, (void*)&Msb);
    SensorReg_Read(params->tRHS2_reg[2].reg, (void*)&Lsb);

    params->tRHS2_reg[0].data = Hsb & 0x0003;
    params->tRHS2_reg[1].data = Msb & 0x00ff;
    params->tRHS2_reg[2].data = Lsb & 0x00ff;
    return;
}

static int pCus_init_HDR(ss_cus_sensor *handle)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    int i,cnt=0;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    CUS_CSI_PARAM_t csi;
    u32 i2c_size = params->Init_Array_Size;

    const I2C_ARRAY *pTable = params->pTable_HDR;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }

    csi.tVCEn.uVCEn = 0x00; /** Mask all of virtual channel input before sensor streaming */
    csi.tVCPd.uPDEn = 0;  /** Turn off atop data lane inflow switch*/
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_PD, &csi);
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_EN, &csi);

    for(i=0;i< i2c_size;i++)
    {
        if(pTable[i].reg==0xffff)
        {
            SENSOR_MDELAY(pTable[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(pTable[i].reg, pTable[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
            }

        }
    }

    CamOsMsDelay(10);               //Wait 10ms for sensor start
    csi.tReset.uReset = 1;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_RESET, &csi);   //Now sensor is streaming, CSI front end is working now, this is time to reset CSI
    csi.tReset.uReset = 0;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_RESET, &csi);   //Just reset pulse
    csi.tVCPd.uPDEn = 1;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_PD, &csi);   //Turn on all data lane inflow switch
    csi.tVCEn.uVCEn = 0xFF;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_EN, &csi);   //Unmask all virtual channel input

    imx415_AEparam_init(handle);

    return SUCCESS;
}

static int pCus_init_Linear(ss_cus_sensor *handle)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    int i,cnt=0;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    CUS_CSI_PARAM_t csi;
    u32 i2c_size = params->Init_Array_Size;

    const I2C_ARRAY *pTable = params->pTable_linear;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }

    csi.tVCEn.uVCEn = 0x00; /** Mask all of virtual channel input before sensor streaming */
    csi.tVCPd.uPDEn = 0;  /** Turn off atop data lane inflow switch*/
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_PD, &csi);
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_EN, &csi);

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }
    for(i=0;i< i2c_size;i++)
    {
        if(pTable[i].reg==0xffff)
        {
            SENSOR_MDELAY(pTable[i].data);
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
            }

        }
    }

    CamOsMsDelay(10);               //Wait 10ms for sensor start
    csi.tReset.uReset = 1;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_RESET, &csi);   //Now sensor is streaming, CSI front end is working now, this is time to reset CSI
    csi.tReset.uReset = 0;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_RESET, &csi);   //Just reset pulse
    csi.tVCPd.uPDEn = 1;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_PD, &csi);   //Turn on all data lane inflow switch
    csi.tVCEn.uVCEn = 0xFF;
    sensor_if->CsiCtrl(params->snr_pad, CUS_CSI_CTRL_ID_VC_EN, &csi);   //Unmask all virtual channel input

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
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    params->csi_clk               = CUS_CSI_CLK_288M;
    handle->video_res_supported.ulcur_res = res_idx;
    handle->pCus_sensor_init = pCus_init_Linear;
    handle->mclk      = Preview_MCLK_SPEED;
    switch (res_idx) {
        case 0:
            params->pTable_linear = Sensor_8m_20fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_20fps_init_table_4lane_linear);
            params->Init_Vts = 2813;
            params->expo.fps = 20;
            params->Line_Period = 17778; // 49.86ms/2813 = 17725ns
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 1:
            params->pTable_linear = Sensor_6m_30fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_6m_30fps_init_table_4lane_linear);
            params->Init_Vts = 2250;
            params->expo.fps = 30;
            params->Line_Period = 14815;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 2:
            params->pTable_linear = Sensor_5p3m_30fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_5p3m_30fps_init_table_4lane_linear);
            params->Init_Vts = 2250;
            params->expo.fps = 30;
            params->Line_Period = 14815;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 3:
            params->pTable_linear = Sensor_5m_30fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_5m_30fps_init_table_4lane_linear);
            params->Init_Vts = 2144;
            params->expo.fps = 25;
            params->Line_Period = 15547;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 4:
            params->pTable_linear = Sensor_4p8m_30fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_4p8m_30fps_init_table_4lane_linear);
            params->Init_Vts = 2250;
            params->expo.fps = 25;
            params->Line_Period = 14815;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 5:
            params->pTable_linear = Sensor_3p6m_30fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_3p6m_30fps_init_table_4lane_linear);
            params->Init_Vts = 2250;
            params->expo.fps = 30;
            params->Line_Period = 14815;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 6:
            params->pTable_linear = Sensor_2m_60fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_2m_60fps_init_table_4lane_linear);
            params->Init_Vts = 2296;
            params->expo.fps = 60;
            params->Line_Period = 7259;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;
        case 7:
            params->pTable_linear = Sensor_8m_30fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_30fps_init_table_4lane_linear);
            params->Init_Vts = 2250;
            params->expo.fps = 30;
            params->Line_Period = 14814;
            params->csi_clk = CUS_CSI_CLK_384M;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 8:
           params->pTable_linear = Sensor_8m_60fps_init_table_4lane_linear;
           params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_60fps_init_table_4lane_linear);
           params->Init_Vts = 2250;
           params->expo.fps = 30; //max 60,default 30
           params->Line_Period = 7407;
           params->csi_clk               = CUS_CSI_CLK_384M;
           handle->data_prec = CUS_DATAPRECISION_10;
           break;
        case 9:
            params->pTable_linear = Sensor_8m_10fps_init_table_2lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_10fps_init_table_2lane_linear);
            handle->interface_attr.attr_mipi.mipi_lane_num    = 2;
            params->Init_Vts = 3315;
            params->expo.fps = 10;
            params->Line_Period = 30165;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;
        case 10:
            params->pTable_linear = Sensor_4m_30fps_init_table_2lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_4m_30fps_init_table_2lane_linear);
            handle->interface_attr.attr_mipi.mipi_lane_num    = 2;
            params->Init_Vts = 2508;
            params->expo.fps = 30; //max 60,default 30
            params->Line_Period = 13290;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
       case 11:
            params->pTable_linear = Sensor_2m_20fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_2m_20fps_init_table_4lane_linear);
            params->Init_Vts = 2250;
            params->expo.fps = 20;
            params->Line_Period = 22222;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;
       case 12:
            params->pTable_linear = Sensor_8m_24fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_24fps_init_table_4lane_linear);
            params->Init_Vts = 2302; //2558; //2686; //2814;
            params->expo.fps = 24;
            params->Line_Period = 18100; //16288;//15512;//14806;
            handle->data_prec = CUS_DATAPRECISION_10;
            handle->mclk      = CUS_CMU_CLK_24MHZ;

            break;

        default:
            handle->video_res_supported.ulcur_res = 0;
            params->pTable_linear = Sensor_8m_20fps_init_table_4lane_linear;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_20fps_init_table_4lane_linear);
            vts_30fps = 2813;
            params->Init_Vts = vts_30fps;
            params->expo.fps = 20;
            params->Line_Period = 17725; // 49.86ms/2813 = 17725ns
            handle->data_prec = CUS_DATAPRECISION_10;
            handle->mclk      = Preview_MCLK_SPEED;
            break;
    }

    params->expo.vts = params->Init_Vts;
    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;

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
    handle->mclk      = Preview_MCLK_SPEED_HDR_DOL;
    switch (res_idx) {
        case HDR_RES_1:
        case HDR_RES_7:
        case HDR_RES_12:
        case HDR_RES_13:
            handle->data_prec = CUS_DATAPRECISION_12;
            break;
        default:
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
    }

    switch (res_idx) {
        case HDR_RES_9:
        case HDR_RES_10:
        case HDR_RES_11:
        case HDR_RES_12:
        case HDR_RES_13:
        case HDR_RES_14:
            handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL3_LEF;
            handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL3_LEF;
            handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_DOL3_LEF;
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_3T1;
            break;
        default:
            handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_LEF;
            handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_LEF;
            handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_DOL_LEF;
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_2T1;

            break;
    }
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_SEF2(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    SENSOR_EMSG("\r \n \033[1;33m  [%s] (%d)  \033[0m \r \n", __func__, __LINE__);

    handle->video_res_supported.ulcur_res = res_idx;
    handle->mclk      = Preview_MCLK_SPEED_HDR_DOL;

    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_SEF(ss_cus_sensor *handle, u32 res_idx)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res, fsc = 0;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    handle->pCus_sensor_init = pCus_init_HDR;
    handle->mclk      = Preview_MCLK_SPEED_HDR_DOL;
    switch (res_idx) {
        case HDR_RES_1:
            vts_30fps_HDR_DOL_4lane = 2250;
            params->pTable_HDR = Sensor_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_init_table_4lane_HDR_DOL);
            params->Init_Vts   = vts_30fps_HDR_DOL_4lane;
            params->expo.fps   = 15;
            params->Line_Period = 29630;
            params->csi_clk    = CUS_CSI_CLK_384M;
            params->min_rhs1   = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_2:
            params->pTable_HDR = Sensor_8m_20fps_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_20fps_init_table_4lane_HDR_DOL);
            params->Init_Vts   = 3450;
            params->expo.fps   = 20;
            params->Line_Period = 14493;
            params->csi_clk    = CUS_CSI_CLK_384M;
            params->min_rhs1   = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_3:
            params->pTable_HDR = Sensor_8m_30fps_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_30fps_init_table_4lane_HDR_DOL);
            params->Init_Vts   = 2300;
            params->expo.fps   = 30;
            params->Line_Period = 14493;
            params->csi_clk    = CUS_CSI_CLK_384M;
            params->min_rhs1   = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_4:
            params->pTable_HDR = Sensor_5m_30fps_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_5m_30fps_init_table_4lane_HDR_DOL);
            params->Init_Vts   = 2300;
            params->expo.fps   = 30;
            params->Line_Period = 14493;
            params->csi_clk    = CUS_CSI_CLK_384M;
            params->min_rhs1   = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_5:
            params->pTable_HDR = Sensor_4p8m_30fps_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_4p8m_30fps_init_table_4lane_HDR_DOL);
            params->Init_Vts   = 2300;
            params->expo.fps   = 30;
            params->Line_Period = 14493;
            params->csi_clk    = CUS_CSI_CLK_384M;
            params->min_rhs1   = 365;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_6:
            params->pTable_HDR = Sensor_4m_30fps_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_4m_30fps_init_table_4lane_HDR_DOL);
            params->Init_Vts   = 2300;
            params->expo.fps   = 30;
            params->Line_Period = 14493;
            params->csi_clk    = CUS_CSI_CLK_288M;
            params->min_rhs1   = 365;     // 4ms: 343 //5ms: 429
            break;
        case HDR_RES_7:
            params->pTable_HDR = Sensor_2m_30fps_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_2m_30fps_init_table_4lane_HDR_DOL);
            params->Init_Vts   = 2296;
            params->expo.fps   = 30;
            params->Line_Period = 14518;
            params->csi_clk    = CUS_CSI_CLK_288M;
            params->min_rhs1   = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_8:
            params->pTable_HDR = Sensor_8m_24fps_init_table_4lane_HDR_DOL;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8m_24fps_init_table_4lane_HDR_DOL);
            params->Init_Vts   = 2344;
            params->expo.fps   = 24;
            params->Line_Period = 17775 >> 1;
            params->csi_clk    = CUS_CSI_CLK_288M;
            params->min_rhs1   = 365;     // 6n+1 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_9:
            params->pTable_HDR = Sensor_8M_30fps_init_table_4lane_HDR_DOL3;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_8M_30fps_init_table_4lane_HDR_DOL3);
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_3T1;
            params->Init_Vts   = 1695;
            params->expo.fps   = 30;
            params->Line_Period = 19666;
            params->csi_clk    = CUS_CSI_CLK_432M;
            params->min_rhs1   = 439;     // 6n+1 fix to 269, 337 //5ms: 429
            params->min_rhs2   = 878;     // 6n+2 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_10:
            params->pTable_HDR = Sensor_5m_25fps_init_table_4lane_HDR_DOL3;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_5m_25fps_init_table_4lane_HDR_DOL3);
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_3T1;
            params->Init_Vts   = 2259;
            params->expo.fps   = 25;
            params->Line_Period = 17211;
            params->csi_clk    = CUS_CSI_CLK_432M;
            params->min_rhs1   = 439;     // 6n+1 fix to 269, 337 //5ms: 429
            params->min_rhs2   = 878;     // 6n+2 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_11:
            params->pTable_HDR = Sensor_4p8m_25fps_init_table_4lane_HDR_DOL3;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_4p8m_25fps_init_table_4lane_HDR_DOL3);
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_3T1;
            params->Init_Vts   = 2259;
            params->expo.fps   = 25;
            params->Line_Period = 17211;
            params->csi_clk    = CUS_CSI_CLK_432M;
            params->min_rhs1   = 439;     // 6n+1 fix to 269, 337 //5ms: 429
            params->min_rhs2   = 878;     // 6n+2 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_12:
            params->pTable_HDR = Sensor_4m_25fps_init_table_4lane_HDR_DOL3;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_4m_25fps_init_table_4lane_HDR_DOL3);
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_3T1;
            params->Init_Vts   = 2259;
            params->expo.fps   = 25;
            params->Line_Period = 17211;
            params->csi_clk    = CUS_CSI_CLK_432M;
            params->min_rhs1   = 439;     // 6n+1 fix to 269, 337 //5ms: 429
            params->min_rhs2   = 878;     // 6n+2 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_13:
            params->pTable_HDR = Sensor_2m_22fps_init_table_4lane_HDR_DOL3;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_2m_22fps_init_table_4lane_HDR_DOL3);
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_3T1;
            params->Init_Vts = 2259;
            params->expo.fps = 22;
            params->Line_Period = 19665;
            params->csi_clk   = CUS_CSI_CLK_432M;
            params->min_rhs1   = 439;     // 6n+1 fix to 269, 337 //5ms: 429
            params->min_rhs2   = 878;     // 6n+2 fix to 269, 337 //5ms: 429
            break;
        case HDR_RES_14:
            params->pTable_HDR = Sensor_2m_30fps_init_table_4lane_HDR_DOL3;
            params->Init_Array_Size = ARRAY_SIZE(Sensor_2m_30fps_init_table_4lane_HDR_DOL3);
            handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = CUS_HDR_FUSION_TYPE_3T1;
            params->Init_Vts = 1694;
            params->expo.fps = 30;
            params->Line_Period = 19677;
            params->csi_clk   = CUS_CSI_CLK_432M;
            params->min_rhs1   = 439;     // 6n+1 fix to 269, 337 //5ms: 429
            params->min_rhs2   = 878;     // 6n+2 fix to 269, 337 //5ms: 429
            break;
        default:
            break;
    }
    params->expo.vts = params->Init_Vts;
    if(handle->interface_attr.attr_mipi.mipi_hdr_fusion_type == CUS_HDR_FUSION_TYPE_3T1)
    {
        fsc = params->expo.vts * 4;
        params->fsc = ((fsc * 6) / 6)+ 6;                  // 6n
    }
    else
    {
        fsc = params->expo.vts * 2;
        params->fsc = ((fsc >> 2) << 2)+ 4;                  // 4n
    }

    switch(res_idx){
        case HDR_RES_1:
        case HDR_RES_7:
            handle->data_prec  = CUS_DATAPRECISION_12;
            handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_SEF; //2F hdr AE USECS control
            break;
        case HDR_RES_9:
        case HDR_RES_10:
        case HDR_RES_11:
        case HDR_RES_12:
            handle->data_prec  = CUS_DATAPRECISION_10;
            handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL3_SEF; //3F hdr AE USECS contr
            break;
        case HDR_RES_13:
        case HDR_RES_14:
            handle->data_prec  = CUS_DATAPRECISION_12;
            handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL3_SEF; //3F hdr AE USECS contro
            break;
        default:
            handle->data_prec  = CUS_DATAPRECISION_10;
            handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_SEF; //2F hdr AE USECS control
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
    imx415_params *params = (imx415_params *)handle->private_data;

    params->orient = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    s16 sen_data;
    imx415_params *params = (imx415_params *)handle->private_data;
    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);
    sen_data &= ~(SENSOR_MIRROR_FLIP_EN);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            //sen_data |= SENSOR_NOR;
            params->orient = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M1F0:
            sen_data |= SENSOR_MIRROR_EN;
            params->orient = CUS_ORIT_M1F0;
            break;
        case CUS_ORIT_M0F1:
            sen_data |= SENSOR_FLIP_EN;
            params->orient = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            sen_data |= SENSOR_MIRROR_FLIP_EN;
            params->orient = CUS_ORIT_M1F1;
            break;
        default :
            params->orient = CUS_ORIT_M0F0;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(MIRROR_FLIP, sen_data);

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);
    u32 InitVts = params->Init_Vts;

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (InitVts*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (InitVts*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    //u32 vts = 0, cur_vts_30fps = 0;
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    u32 InitVts = params->Init_Vts;

    //pr_info("[%s]  leslie_fps,maxfps,minfps : %d,%d,%d\n\n", __FUNCTION__,fps,max_fps,min_fps);
    //cur_vts_30fps = vts_30fps;
    //pr_info("[%s]  leslie_vts_30fps : %u\n\n", __FUNCTION__,vts_30fps);
    if(fps>=min_fps && fps <= max_fps){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps>29?29:fps;              //limit fps at 29 fps due to MCLK=36MHz
            params->expo.vts=  (InitVts*29091 + fps * 500)/(fps * 1000);
        }
        else
            params->expo.vts=  (InitVts*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        if (CUS_CMU_CLK_36MHZ == handle->mclk) {
            fps = fps>29091?29091:fps;       //limit fps at 29.091 fps due to MCLK=36MHz
            params->expo.vts=  (InitVts*29091 + (fps>>1))/fps;
        }
        else
            params->expo.vts=  (InitVts*(max_fps*1000) + (fps>>1))/fps;
    }else{
      SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }
    //pr_info("[%s]  leslie_vts : %u\n\n", __FUNCTION__,params->expo.vts);
    if(params->expo.expo_lines > params->expo.vts - 4){
        //vts = params->expo.expo_lines + 4;
#if 0        //Update FPS Status
        if(fps>=3 && fps <= 30)
            fps = (InitVts*30000)/(params->expo.vts * 1000 - 500);
        else if(fps>=3000 && fps <= 30000)
            fps = (InitVts*30000)/(params->expo.vts - (500 / 1000));
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
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 cur_vts_30fps = params->Init_Vts;
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
    imx415_params *params = (imx415_params *)handle->private_data;
       u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    //cur_vts_30fps = imx415_mipi_hdr[0].senout.height;
    cur_vts_30fps=params->Init_Vts;
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
static int pCus_AEStatusNotify(struct __ss_cus_sensor*handle,u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

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
                    DoOrien(handle, params->orient);
                    params->orien_dirty = false;
                    params->skip_cnt = true;
                }
                SensorReg_Write(0x3001,0); // Global hold off
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

static int pCus_AEStatusNotifyHDR_DOL_SEF(struct __ss_cus_sensor*handle,u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    //imx415_params *params = (imx415_params *)handle->private_data;

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
                    DoOrien(handle, params->orient);
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
    u32 expo_lines = 0, SHR0 = 0, vts = 0;
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 Hsb, Msb, Lsb;

    if (params->PowerOnOff == 0)
    {
        SHR0 |= (u32)(params->tExpo_reg[0].data & 0x03) << 16;
        SHR0 |= (u32)(params->tExpo_reg[1].data & 0xff) << 8;
        SHR0 |= (u32)(params->tExpo_reg[2].data & 0xff) << 0;

        vts |= (u32)(params->tVts_reg[0].data & 0x03) << 16;
        vts |= (u32)(params->tVts_reg[1].data & 0xff) << 8;
        vts |= (u32)(params->tVts_reg[2].data & 0xff) << 0;

    }
    else
    {
        SensorReg_Read(params->tExpo_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tExpo_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tExpo_reg[2].reg, (void*)&Lsb);

        SHR0 |= (u32)(Hsb & 0x03) << 16;
        SHR0 |= (u32)(Msb & 0xff) << 8;
        SHR0 |= (u32)(Lsb & 0xff) << 0;

        SensorReg_Read(params->tVts_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tVts_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tVts_reg[2].reg, (void*)&Lsb);
        vts |= (u32)(Hsb & 0x03) << 16;
        vts |= (u32)(Msb & 0xff) << 8;
        vts |= (u32)(Lsb & 0xff) << 0;
    }
    expo_lines =  vts - SHR0;

    *us = (expo_lines * params->Line_Period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, expo_lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines = 0, vts = 0, SHR0 = 0;
    imx415_params *params = (imx415_params *)handle->private_data;

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

static int pCus_GetAEUSecs_HDR_DOL_SEF(ss_cus_sensor *handle, u32 *us)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 RHS_lines = 0, SHR_lines = 0, expo_line_sef = 0;
    u16 Hsb, Msb, Lsb;

    if (params->PowerOnOff == 0)
    {
        RHS_lines |= (u32)(params->tRHS1_reg[0].data & 0x03) << 16;
        RHS_lines |= (u32)(params->tRHS1_reg[1].data & 0xff) << 8;
        RHS_lines |= (u32)(params->tRHS1_reg[2].data & 0xff) << 0;

        SHR_lines |= (u32)(params->tSHR1_reg[0].data & 0x03) << 16;
        SHR_lines |= (u32)(params->tSHR1_reg[1].data & 0xff) << 8;
        SHR_lines |= (u32)(params->tSHR1_reg[2].data & 0xff) << 0;
    }
    else
    {
        SensorReg_Read(params->tRHS1_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tRHS1_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tRHS1_reg[2].reg, (void*)&Lsb);

        RHS_lines |= (u32)(Hsb & 0x03) << 16;
        RHS_lines |= (u32)(Msb & 0xff) << 8;
        RHS_lines |= (u32)(Lsb & 0xff) << 0;

        SensorReg_Read(params->tSHR1_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tSHR1_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tSHR1_reg[2].reg, (void*)&Lsb);

        SHR_lines |= (u32)(Hsb & 0x03) << 16;
        SHR_lines |= (u32)(Msb & 0xff) << 8;
        SHR_lines |= (u32)(Lsb & 0xff) << 0;
    }

    expo_line_sef  = RHS_lines - SHR_lines;

    *us = (expo_line_sef * params->Line_Period) / 1000;

    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_DOL_SEF2(ss_cus_sensor *handle, u32 *us)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 RHS2_lines = 0, SHR2_lines = 0, expo_line_sef2 = 0;
    u16 Hsb, Msb, Lsb;

    if (params->PowerOnOff == 0)
    {
        RHS2_lines |= (u32)(params->tRHS2_reg[0].data & 0x03) << 16;
        RHS2_lines |= (u32)(params->tRHS2_reg[1].data & 0xff) << 8;
        RHS2_lines |= (u32)(params->tRHS2_reg[2].data & 0xff) << 0;

        SHR2_lines |= (u32)(params->tSHR2_reg[0].data & 0x03) << 16;
        SHR2_lines |= (u32)(params->tSHR2_reg[1].data & 0xff) << 8;
        SHR2_lines |= (u32)(params->tSHR2_reg[2].data & 0xff) << 0;
    }
    else
    {
        SensorReg_Read(params->tRHS2_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tRHS2_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tRHS2_reg[2].reg, (void*)&Lsb);

        RHS2_lines |= (u32)(Hsb & 0x03) << 16;
        RHS2_lines |= (u32)(Msb & 0xff) << 8;
        RHS2_lines |= (u32)(Lsb & 0xff) << 0;

        SensorReg_Read(params->tSHR2_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tSHR2_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tSHR2_reg[2].reg, (void*)&Lsb);

        SHR2_lines |= (u32)(Hsb & 0x03) << 16;
        SHR2_lines |= (u32)(Msb & 0xff) << 8;
        SHR2_lines |= (u32)(Lsb & 0xff) << 0;
    }
    expo_line_sef2 = RHS2_lines - SHR2_lines;

    *us = (expo_line_sef2 * params->Line_Period) / 1000;

    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_sef = 0;
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 cur_line_period = params->Line_Period;

    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    params->expo.expo_sef_us = us;
    //params->min_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429
    if(expo_line_sef>= params->min_rhs1)
    {
        SENSOR_EMSG("[%s] us %u, expo_line_sef %u rhs %u\n", __FUNCTION__, us,  expo_line_sef,  params->min_rhs1 );
        return FAIL;
    }
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

static int pCus_SetAEUSecsHDR_DOL3_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_sef = 0;
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 cur_line_period = params->Line_Period;

    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    params->expo.expo_sef_us = us;
    if(expo_line_sef>= params->min_rhs1)
    {
        SENSOR_EMSG("[%s] us %u, expo_line_sef %u rhs %u\n", __FUNCTION__,us, expo_line_sef, params->min_rhs1);
        return FAIL;
    }
    params->min_shr1 = params->min_rhs1 - expo_line_sef;

    if((expo_line_sef > params->min_rhs1) || ((params->min_shr1) < 13))
        params->min_shr1 = 13;

    if(params->min_shr1 > params->min_rhs1 -12)
        params->min_shr1 = params->min_rhs1 - 12;

    params->min_shr1 = ((params->min_shr1 * 3) /3) + 1;

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

static int pCus_SetAEUSecsHDR_DOL3_SEF2(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_sef = 0;
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 cur_line_period = params->Line_Period;

    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    params->expo.expo_sef2_us = us;
    if(expo_line_sef>= params->min_rhs2)
    {
        SENSOR_EMSG("[%s] us %u, expo_line_sef %u rhs %u\n", __FUNCTION__, us,expo_line_sef, params->min_rhs2);
        return FAIL;
    }
    params->min_shr2 = params->min_rhs2 - expo_line_sef;

    if((expo_line_sef > params->min_rhs2) || ((params->min_shr2) <  params->min_rhs1 + 13))
        params->min_shr2 = params->min_rhs1 + 13;

    if(params->min_shr2 > params->min_rhs2 - 12)
        params->min_shr2 = params->min_rhs2 - 12;

    params->min_shr2 = ((params->min_shr2 * 6)/ 6) + 2;

    SENSOR_DMSG("[%s] us %u, expo_line_sef %u rhs %u shr1 %u\n", __FUNCTION__,
                                                                 us, \
                                                                 expo_line_sef, \
                                                                 params->min_rhs1, \
                                                                 params->min_shr1
               );

    params->tRHS2_reg[0].data = (params->min_rhs2 >>16) & 0x03;
    params->tRHS2_reg[1].data = (params->min_rhs2 >>8) & 0xff;
    params->tRHS2_reg[2].data = (params->min_rhs2 >>0) & 0xff;

    params->tSHR2_reg[0].data = (params->min_shr2 >> 16) & 0x0003;
    params->tSHR2_reg[1].data = (params->min_shr2 >> 8) & 0x00ff;
    params->tSHR2_reg[2].data = (params->min_shr2 >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 gain_reg_temp = params->tGain_reg[0].data;

    pCus_GetAEGain_Calculate(gain_reg_temp, gain);

    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);
    return SUCCESS;
}

static int pCus_GetAEGain_HDR_DOL_SEF(ss_cus_sensor *handle, u32* gain)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 gain_reg_temp = params->tGain_hdr_dol_sef_reg[0].data;

    pCus_GetAEGain_Calculate(gain_reg_temp, gain);

    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}

static int pCus_GetAEGain_HDR_DOL_SEF2(ss_cus_sensor *handle, u32* gain)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 gain_reg_temp = params->tGain_hdr_dol_sef2_reg[0].data;

    pCus_GetAEGain_Calculate(gain_reg_temp, gain);

    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 gain_temp,gain_reg_value;
    //params->expo.final_gain[0] = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    // dB_reg = 20log10(gain)/log10(2) = 20log2(gain)
    gain_temp = (u32)((200*(EXT_log_2(gain)-167772160ULL)) >> 24);
    gain_reg_value = round_float(gain_temp, 1);

    params->tGain_reg[0].data = gain_reg_value & 0xff;
    params->tGain_reg[1].data = (gain_reg_value >> 8) & 0xff;

    SENSOR_DMSG("gain:%u  gain_reg_value:%u \n", gain, gain_reg_value);
    params->dirty = true;
    return SUCCESS;
}

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

static u32 pCus_TryAEGain(ss_cus_sensor *handle, u32 gain)
{
    u32 valid_gain;
    u16 gain_reg_tmp;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg_tmp);
    pCus_GetAEGain_Calculate(gain_reg_tmp, &valid_gain);

    return valid_gain;
}

static void pCus_SetAEGainHDR_DOL_Calculate(u32 gain, u16 *gain_reg_tmp)
{
    u32 gain_temp;
    //params->expo.final_gain[0] = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    gain_temp = (u32)((200*(EXT_log_2(gain)-167772160ULL)) >> 24);
    *gain_reg_tmp = round_float(gain_temp, 1) & 0xFF;
}

static int pCus_SetAEGainHDR_DOL_SEF(ss_cus_sensor *handle, u32 gain)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 gain_reg_value = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg_value);
    //params->expo.final_gain[1] = gain;
    params->tGain_hdr_dol_sef_reg[0].data = gain_reg_value & 0xff;
    params->tGain_hdr_dol_sef_reg[1].data = (gain_reg_value >> 8) & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data, params->tGain_hdr_dol_sef_reg[1].data);

    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGainHDR_DOL_SEF(ss_cus_sensor *handle, u32 gain)
{
    u32 valid_gain;
    u16 gain_reg_tmp;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg_tmp);
    pCus_GetAEGain_Calculate(gain_reg_tmp, &valid_gain);

    return valid_gain;
}

static int pCus_SetAEGainHDR_DOL_SEF2(ss_cus_sensor *handle, u32 gain)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 gain_reg_value = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg_value);
    //params->expo.final_gain[2] = gain;
    params->tGain_hdr_dol_sef2_reg[0].data = gain_reg_value & 0xff;
    params->tGain_hdr_dol_sef2_reg[1].data = (gain_reg_value >> 8) & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef2_reg[0].data, params->tGain_hdr_dol_sef2_reg[1].data);

    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGainHDR_DOL_SEF2(ss_cus_sensor *handle, u32 gain)
{
    u32 valid_gain;
    u16 gain_reg_tmp;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg_tmp);
    pCus_GetAEGain_Calculate(gain_reg_tmp, &valid_gain);

    return valid_gain;
}

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx415_params *params;
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

    params = (imx415_params *)handle->private_data;
    CamOsMemcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    CamOsMemcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    CamOsMemcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    CamOsSnprintf(handle->strSensorStreamName,sizeof(handle->strSensorStreamName),"IMX415_MIPI");

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
    //handle->data_fmt              = SENSOR_DATAFMT;
    handle->sif_bus               = SENSOR_IFBUS_TYPE;
    handle->data_prec             = SENSOR_DATAPREC;
    handle->bayer_id              = SENSOR_BAYERID;
    handle->RGBIR_id              = SENSOR_RGBIRID;
    params->csi_clk               = CUS_CSI_CLK_288M;
    //handle->YC_ODER               = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num    = SENSOR_MIPI_LANE_NUM_DOL;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order   = 0; //don't care in RGB pattern.
//    handle->interface_attr.attr_mipi.mipi_hsync_mode  = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode    = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
   //handle->video_res_supported.num_res = LINEAR_RES_END;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx415_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx415_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx415_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx415_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx415_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx415_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx415_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx415_mipi_linear[res].senout.height;
        CamOsStrncpy(handle->video_res_supported.res[res].strResDesc, imx415_mipi_linear[res].senstr.strResDesc,CamOsStrlen(imx415_mipi_linear[res].senstr.strResDesc));
    }

    ////////////////////////////////////
    //    Sensor polarity             //
    ////////////////////////////////////
//    handle->pwdn_POLARITY              = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
//    handle->reset_POLARITY             = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY             = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY             = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY              = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    handle->pCus_sensor_release        = cus_camsensor_release_handle;
    handle->pCus_sensor_init           = pCus_init_mipi4lane_linear;
    //handle->pCus_sensor_powerupseq     = pCus_powerupseq;
    handle->pCus_sensor_poweron        = pCus_poweron;
    handle->pCus_sensor_poweroff       = pCus_poweroff;
    handle->pCus_sensor_streamon       = pCus_StreamOn;
    handle->pCus_sensor_GetSensorID    = pCus_GetSensorID;
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes    = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien       = pCus_GetOrien;
    handle->pCus_sensor_SetOrien       = pCus_SetOrien;
    handle->pCus_sensor_GetFPS         = pCus_GetFPS;
    handle->pCus_sensor_SetFPS         = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = imx415_SetPatternMode; //NONE


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
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    handle->pCus_sensor_str = pCus_sensor_str;

    params->expo.vts = vts_30fps;
    params->Init_Vts = vts_30fps;
    params->Line_Period = Preview_line_period;
    params->pTable_linear = Sensor_init_table_4lane_linear;
    params->Init_Array_Size = ARRAY_SIZE(Sensor_init_table_4lane_linear);
    //params->expo.final_gain[0] = 1060;
    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx415_params *params = NULL;
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

    params = (imx415_params *)handle->private_data;
    CamOsMemcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    CamOsMemcpy(params->tRHS1_reg, expo_IMX415_RHS1_reg, sizeof(expo_IMX415_RHS1_reg));
    CamOsMemcpy(params->tSHR1_reg, expo_SHR1_reg, sizeof(expo_SHR1_reg));
    CamOsMemcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    CamOsSnprintf(handle->strSensorStreamName, sizeof(handle->strSensorStreamName),"IMX415_MIPI_HDR_SEF");

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
    handle->bayer_id              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id              = SENSOR_RGBIRID;
    params->csi_clk               = CUS_CSI_CLK_288M;

    handle->interface_attr.attr_mipi.mipi_lane_num                = 4; // hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
//    handle->interface_attr.attr_mipi.mipi_hsync_mode              = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;  // SONY IMX415 as VC mode
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx415_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx415_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx415_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx415_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx415_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx415_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx415_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx415_mipi_hdr[res].senout.height;
        CamOsStrncpy(handle->video_res_supported.res[res].strResDesc, imx415_mipi_hdr[res].senstr.strResDesc,CamOsStrlen(imx415_mipi_hdr[res].senstr.strResDesc));
    }


    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    handle->pCus_sensor_release        = cus_camsensor_release_handle;
    handle->pCus_sensor_init           = pCus_init_mipi4lane_HDR_DOL;
    handle->pCus_sensor_poweron        = pCus_poweron;               // Need to check
    handle->pCus_sensor_poweroff       = pCus_poweroff;              // Need to check
    handle->pCus_sensor_streamon       = pCus_StreamOn;
    handle->pCus_sensor_GetSensorID    = pCus_GetSensorID;           // Need to check
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
//    handle->ae_gain_delay              = SENSOR_GAIN_DELAY_FRAME_COUNT;
//    handle->ae_shutter_delay           = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
//    handle->ae_gain_ctrl_num           = 2;
//    handle->ae_shutter_ctrl_num        = 2;
//    handle->sat_mingain                = SENSOR_MIN_GAIN;      //g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_SEF;  // Need to check
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_DOL_SEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_SEF;      // Need to check
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_HDR_DOL_SEF;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_SEF;      // Need to che
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGainHDR_DOL_SEF;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo_SEF;
    handle->pCus_sensor_str = pCus_sensor_str;


    params->expo.vts = vts_30fps_HDR_DOL_4lane;
    params->pTable_HDR = Sensor_init_table_4lane_HDR_DOL;
    params->Init_Array_Size = ARRAY_SIZE(Sensor_init_table_4lane_HDR_DOL);
    //params->expo.final_gain[1] = 1060;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_sef2(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx415_params *params;
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
    params = (imx415_params *)handle->private_data;
    CamOsMemcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    CamOsMemcpy(params->tSHR2_reg, expo_DOL3_SHR2_reg, sizeof(expo_DOL3_SHR2_reg));
    CamOsMemcpy(params->tRHS2_reg, expo_IMX415_DOL3_RHS2_reg, sizeof(expo_IMX415_DOL3_RHS2_reg));
    CamOsMemcpy(params->tGain_hdr_dol_sef2_reg, gain_HDR_DOL_SEF2_reg, sizeof(gain_HDR_DOL_SEF2_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    CamOsSnprintf(handle->strSensorStreamName,sizeof(handle->strSensorStreamName),"IMX415_MIPI_HDR_SEF2");

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
//    handle->data_prec              = SENSOR_DATAPREC_DOL;  //CUS_DATAPRECISION_8;
//    handle->data_mode              = SENSOR_DATAMODE;
    handle->bayer_id               = SENSOR_BAYERID_HDR_DOL;   //CUS_BAYER_GB;
    handle->RGBIR_id               = SENSOR_RGBIRID;
//    params->orient                 = SENSOR_ORIT;        //CUS_ORIT_M1F1;
    //handle->YC_ODER                = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num                = 4;//hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
//    handle->interface_attr.attr_mipi.mipi_hsync_mode              = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 2; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx415_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx415_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx415_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx415_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx415_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx415_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx415_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx415_mipi_hdr[res].senout.height;
        CamOsStrncpy(handle->video_res_supported.res[res].strResDesc, imx415_mipi_hdr[res].senstr.strResDesc,CamOsStrlen(imx415_mipi_hdr[res].senstr.strResDesc));
    }



    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes_HDR_DOL_SEF2;   // Need to check

    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_SEF;  // Need to check
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_DOL_SEF2;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL3_SEF2;      // Need to check
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_HDR_DOL_SEF2;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_SEF2;      // Need to check
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGainHDR_DOL_SEF2;
    handle->pCus_sensor_str = pCus_sensor_str;
    params->expo.vts = vts_30fps_HDR_DOL_4lane;
    //params->expo.final_gain[2] = 1060;

    params->dirty = false;

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
    handle->interface_attr.attr_mipi.mipi_hdr_fusion_type = 0;
    return SUCCESS;
}

static int pCus_GetSensorID_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *id)
{
     return SUCCESS;
}

static int pCus_GetOrien_HDR_DOL_LEF(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    return SUCCESS;
}

static int pCus_SetOrien_HDR_DOL_LEF(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx415_params *params = (imx415_params *)handle->private_data;

//    params->orient = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_LEF(ss_cus_sensor *handle)
{
    imx415_params *params = (imx415_params *)handle->private_data;
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
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    cur_vts_30fps=params->Init_Vts;
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

static int pCus_AEStatusNotifyHDR_DOL_LEF(struct __ss_cus_sensor* handle, u32 idx,CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

             break;
        case CUS_FRAME_ACTIVE:
            if(handle->interface_attr.attr_mipi.mipi_hdr_fusion_type == CUS_HDR_FUSION_TYPE_2T1){
                if(params->dirty || params->orien_dirty) {
                    SensorReg_Write(0x3001,1);
                    if(params->dirty) {
                        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg_hdr));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR1_reg, ARRAY_SIZE(expo_SHR1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tRHS1_reg, ARRAY_SIZE(expo_IMX415_RHS1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                        params->dirty = false;
                    }
                    if(params->orien_dirty) {
                        DoOrien(handle, params->orient);
                        params->orien_dirty = false;
                        params->skip_cnt = true;
                    }
                    SensorReg_Write(0x3001,0);
                }
                if(params->skip_cnt){
                    sensor_if->SetSkipFrame(idx, params->expo.fps, params->skip_cnt);
                    params->skip_cnt = 0;
                }
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL3_LEF(struct __ss_cus_sensor *handle,u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

             break;
        case CUS_FRAME_ACTIVE:
            if(handle->interface_attr.attr_mipi.mipi_hdr_fusion_type == CUS_HDR_FUSION_TYPE_3T1){
                if(params->dirty || params->orien_dirty) {
                    SensorReg_Write(0x3001,1);
                    if(params->dirty) {
                        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg_hdr));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_DOL3_SHR0_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR1_reg, ARRAY_SIZE(expo_DOL3_SHR1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tRHS1_reg, ARRAY_SIZE(expo_IMX415_DOL3_RHS1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR2_reg, ARRAY_SIZE(expo_DOL3_SHR2_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tRHS2_reg, ARRAY_SIZE(expo_IMX415_DOL3_RHS2_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef2_reg, ARRAY_SIZE(gain_HDR_DOL_SEF2_reg));
                        params->dirty = false;
                    }
                    if(params->orien_dirty) {
                        DoOrien(handle, params->orient);
                        params->orien_dirty = false;
                        params->skip_cnt = true;
                    }
                    SensorReg_Write(0x3001,0);
                }

                if(params->skip_cnt){
                    sensor_if->SetSkipFrame(idx, params->expo.fps, params->skip_cnt);
                    params->skip_cnt = 0;
                }
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *us)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 Hsb, Msb, Lsb;
    u32 Lef_shr0;

    if (params->PowerOnOff == 0)
    {
        Hsb = params->tSHR0_reg[0].data;
        Msb = params->tSHR0_reg[1].data;
        Lsb = params->tSHR0_reg[2].data;
    }
    else
    {
        SensorReg_Read(params->tSHR0_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tSHR0_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tSHR0_reg[2].reg, (void*)&Lsb);
    }

    Lef_shr0 = (Hsb & 0x03 ) << 16 | (Msb & 0xFF ) << 8 | (Lsb & 0xFF);
    *us = (params->fsc - 8) - Lef_shr0;
    *us =  (*us) * params->Line_Period /1000;
    SENSOR_DMSG("[%s] sensor Lef_shr0: %u,%u us\n", __FUNCTION__,  Lef_shr0, *us);
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_DOL3_LEF(ss_cus_sensor *handle, u32 *us)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 Hsb, Msb, Lsb;
    u32 Lef_shr0;

    if (params->PowerOnOff == 0)
    {
        Hsb = params->tSHR0_reg[0].data;
        Msb = params->tSHR0_reg[1].data;
        Lsb = params->tSHR0_reg[2].data;
    }
    else
    {
        SensorReg_Read(params->tSHR0_reg[0].reg, (void*)&Hsb);
        SensorReg_Read(params->tSHR0_reg[1].reg, (void*)&Msb);
        SensorReg_Read(params->tSHR0_reg[2].reg, (void*)&Lsb);
    }
    Lef_shr0 = (Hsb & 0x03 ) << 16 | (Msb & 0xFF ) << 8 | (Lsb & 0xFF);
    *us = (params->fsc - 8) - Lef_shr0;
    *us =  (*us) * params->Line_Period /1000;
    SENSOR_DMSG("[DBG] Get LEF_AEUsc %d\n", *us);
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines_lef = 0, vts = 0, fsc = 0;
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 cur_line_period = params->Line_Period;
    expo_lines_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;

    params->expo.expo_lef_us = us;
    fsc = params->expo.vts * 2;
    params->fsc = ((fsc >> 2) << 2)+ 4;                  // 4n
    if(expo_lines_lef+8>= fsc)
    {
        SENSOR_EMSG("[%s] us %u, expo_lines_lef %u fsc %u\n", __FUNCTION__, us,expo_lines_lef, fsc);
        return FAIL;
    }
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
                                                                     params->max_shr0);
    params->tSHR0_reg[0].data = (params->max_shr0 >> 16) & 0x0003;
    params->tSHR0_reg[1].data = (params->max_shr0 >> 8) & 0x00ff;
    params->tSHR0_reg[2].data = (params->max_shr0 >> 0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL3_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines_lef = 0, vts = 0, fsc = 0;
    imx415_params *params = (imx415_params *)handle->private_data;
    u32 cur_line_period = params->Line_Period;

    expo_lines_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;

    params->expo.expo_lef_us = us;
    fsc = params->expo.vts * 4;
    params->fsc = ((fsc * 6) / 6)+ 6;                  // 4n
    if(expo_lines_lef+12>= fsc)
    {
        SENSOR_EMSG("[%s] us %u, expo_lines_lef %u fsc %u\n", __FUNCTION__, us, expo_lines_lef, fsc );
        return FAIL;
    }
    params->max_shr0 = (fsc - 12) - expo_lines_lef;
    if(params->max_shr0 < (params->min_rhs2 + 13))
        params->max_shr0 = params->min_rhs2 + 13;
    params->max_shr0 = ((params->max_shr0 * 3) / 3) + 3;

    if (expo_lines_lef > (fsc - params->min_rhs2 - 12)) {
        vts = (expo_lines_lef + params->min_rhs2 + 12) / 4;
    }
    else{
      vts = params->expo.vts;
    }
    params->expo.expo_lines = expo_lines_lef;

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
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 gain_reg_temp = params->tGain_hdr_dol_lef_reg[0].data;

    pCus_GetAEGain_Calculate(gain_reg_temp, gain);

    return SUCCESS;
}

static int pCus_SetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    imx415_params *params = (imx415_params *)handle->private_data;
    u16 gain_reg_value = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg_value);
    //params->expo.final_gain[0] = gain;
    params->tGain_hdr_dol_lef_reg[0].data = gain_reg_value & 0xff;
    params->tGain_hdr_dol_lef_reg[1].data = (gain_reg_value >> 8) & 0xff;

    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    u32 valid_gain;
    u16 gain_reg_tmp;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg_tmp);
    pCus_GetAEGain_Calculate(gain_reg_tmp, &valid_gain);

    return valid_gain;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx415_params *params;
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
    params = (imx415_params *)handle->private_data;
    CamOsMemcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    CamOsMemcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    CamOsMemcpy(params->tSHR1_reg, expo_SHR1_reg, sizeof(expo_SHR1_reg));
    CamOsMemcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    CamOsSnprintf(handle->strSensorStreamName,sizeof(handle->strSensorStreamName),"IMX415_MIPI_HDR_LEF");

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
//    handle->data_mode              = SENSOR_DATAMODE;
    handle->bayer_id               = SENSOR_BAYERID_HDR_DOL;   //CUS_BAYER_GB;
    handle->RGBIR_id               = SENSOR_RGBIRID;
//    params->orient                 = SENSOR_ORIT;        //CUS_ORIT_M1F1;
    //handle->YC_ODER                = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num                = 4;//hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
//    handle->interface_attr.attr_mipi.mipi_hsync_mode              = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx415_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx415_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx415_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx415_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx415_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx415_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx415_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx415_mipi_hdr[res].senout.height;
        CamOsStrncpy(handle->video_res_supported.res[res].strResDesc, imx415_mipi_hdr[res].senstr.strResDesc,sizeof(imx415_mipi_hdr[res].senstr.strResDesc));
    }

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_release         = cus_camsensor_release_handle;
    handle->pCus_sensor_init            = pCus_init_HDR_DOL_LEF;
    //handle->pCus_sensor_powerupseq      = pCus_powerupseq   ;
    handle->pCus_sensor_poweron         = pCus_poweron_HDR_DOL_LEF;
    handle->pCus_sensor_poweroff        = pCus_poweroff_HDR_DOL_LEF;
    handle->pCus_sensor_GetSensorID     = pCus_GetSensorID_HDR_DOL_LEF;
    //handle->pCus_sensor_GetVideoResNum  = NULL;
    //handle->pCus_sensor_GetVideoRes     = NULL;
    //handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes_HDR_DOL_LEF;   // Need to check


    handle->pCus_sensor_GetOrien        = pCus_GetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_DOL_LEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_DOL_LEF;


    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_LEF;
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGainHDR_DOL_LEF;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo_LEF;
    handle->pCus_sensor_str = pCus_sensor_str;

    params->expo.vts = vts_30fps_HDR_DOL_4lane;
    params->Init_Vts = vts_30fps_HDR_DOL_4lane;
    params->Line_Period = Preview_line_period_HDR_DOL_4LANE;
    //params->expo.final_gain[0] = 1060;

    params->dirty = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX2(IMX415_HDR,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            cus_camsensor_init_handle_hdr_dol_sef2,
                            imx415_params
                         );

