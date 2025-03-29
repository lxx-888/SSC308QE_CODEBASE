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
#include <drv_sensor_init_table.h> //TODO: move this header to drv_sensor_common.h
#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX678_HDR);

#if defined(CAM_OS_RTK)
int sleep_mode = 0;
#elif defined(CAM_OS_LINUX_KERNEL)
int sleep_mode = 0;
module_param(sleep_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(sleep_mode, "sensor sleep mode 1:sleep reg, 2:pdn");
#endif

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

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
//#define SENSOR_DATAFMT             CUS_DATAFMT_BAYER    //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_DOL         CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9098  //CFG
#define SENSOR_BAYERID              CUS_BAYER_RG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_RG
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_YCORDER              CUS_SEN_YCODR_YC     //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define long_packet_type_enable     0x00 //UD1~UD8 (user define)

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_24MHZ

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
#define SENSOR_NAME     IMX678

#define CHIP_ID_r3F12   0x3F12
#define CHIP_ID_r3F13   0x3F13
#define CHIP_ID         0x0678

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    enum {LINEAR_RES_1 = 0 , LINEAR_RES_2 /*, LINEAR_RES_4, LINEAR_RES_5, LINEAR_RES_6, LINEAR_RES_7, LINEAR_RES_8*/, LINEAR_RES_END}mode;

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
}imx678_mipi_linear[] = {
    {LINEAR_RES_1, {3856, 2180, 1, 15}, {0, 0, 3840, 2160}, {"3840x2160@15fps"}}, // Modify it
    {LINEAR_RES_2, {3856, 2180, 1, 30}, {0, 0, 3840, 2160}, {"3840x2160@30fps"}}, // Modify it
};
static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_2 = 1, HDR_RES_END}mode;
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
}imx678_mipi_hdr[] = {
    {HDR_RES_1, {3856, 2180, 3, 30}, {2, 2, 3840, 2160}, {"IMX678 3840x2160@30fps_ClearHDR"}}, // Modify it
    {HDR_RES_2, {3856, 2180, 3, 30}, {0, 0, 3840, 2160}, {"IMX678 3840x2160@30fps_HDR"}}, // Modify it
};

u32 vts_30fps = 2250;
u32 vts_30fps_HDR_DOL_4lane = 2250;
u32 Preview_line_period = 17778;
u32 Preview_line_period_HDR_DOL_4LANE = 14814;

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (3981 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain, 20xlog(3981)=72db
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_CLR_HDR       (1)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)

#define SENSOR_CLR_HDR_MAX_GAIN                     (513228) // 54db
#define SENSOR_CLR_HDR_MAX_AGAIN_H                  (2754 * 1024 / 100) // 28.8db 28200
#define SENSOR_CLR_HDR_MAX_AGAIN_L                  (1584 * 1024 / 100) // 24db   16220

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR                                 0x3020
#define FLIP                                   0x3031

#define SENSOR_NOR                                  0x0
#define SENSOR_MIRROR_EN                            0x1
#define SENSOR_FLIP_EN                              0x2
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
    u32 max_rhs1;
    u32 min_shr0;
    u32 max_shr0;
    u32 fsc;
    u32 Clrhdr_mode_en;
    bool dirty;
    bool orien_dirty;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tVts_reg_hdr[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tSHR0_reg[3];
    I2C_ARRAY tExpo_shr_dol1_reg[3];
    I2C_ARRAY tExpo_rhs1_reg[3];
    I2C_ARRAY tGain_hdr_dol_lef_reg[2];
    I2C_ARRAY tGain_hdr_dol_sef_reg[2];
    I2C_ARRAY tGain_clr_hdr_AGH_reg[2];
    I2C_ARRAY tGain_clr_hdr_AGL_reg[2];
    I2C_ARRAY tGain_clr_hdr_DG_reg[2];
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
	CUS_UserDefSnrInfo_t stUserDefInfo;
} imx678_params;

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
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
const static I2C_ARRAY Sensor_8m_30fps_init_table_4lane_linear[] =
{
    /*
        IMX678 All-pixel scan
        CSI-2_4lane
        24MHz
        AD:10bit
        Output:10bit
        1440Mbps
        Master Mode 30fps
        Integration Time33.311ms
    */
    {0x3000, 0x01},    // Standby
    {0x3002, 0x01},    // Master mode stop
    {0x3014, 0x04},    // INCK_SEL[3:0], 4h:24MHz
    {0x3015, 0x03},    // DATATATE_SEL[3:0], 3h:1440 Mbps
    {0x3022, 0x00},    // ADBIT[1:0],   0h:AD 10bit
    {0x3023, 0x00},    // MDBIT[1:0],   0h:AD 10bit
    {0x3028, 0xCA},    // VMAX[19:0]
    {0x3029, 0x08},    // VMAX
    {0x302A, 0x00},    // VMAX         8CAh: 2250
    {0x302C, 0x4C},    // HMAX LSB
    {0x302D, 0x04},    // HMAX MSB     44Ch: 1100
    {0x3050, 0x03},    // SHR0
    {0x3051, 0x00},    // SHR0
    {0x3052, 0x00},    // SHR0
    {0x30A4, 0xAA},    // XVSOUTSEL XHSOUTSEL
    {0x30A6, 0x00},    // XVS_DRV XHS_DRV
    {0x30CC, 0x00},    // XVSLNG
    {0x30CD, 0x00},    // XHSLNG
    {0x3460, 0x22},
    {0x355A, 0x64},
    {0x3A02, 0x7A},
    {0x3A10, 0xEC},
    {0x3A12, 0x71},
    {0x3A14, 0xDE},
    {0x3A20, 0x2B},
    {0x3A24, 0x22},
    {0x3A25, 0x25},
    {0x3A26, 0x2A},
    {0x3A27, 0x2C},
    {0x3A28, 0x39},
    {0x3A29, 0x38},
    {0x3A30, 0x04},
    {0x3A31, 0x04},
    {0x3A32, 0x03},
    {0x3A33, 0x03},
    {0x3A34, 0x09},
    {0x3A35, 0x06},
    {0x3A38, 0xCD},
    {0x3A3A, 0x4C},
    {0x3A3C, 0xB9},
    {0x3A3E, 0x30},
    {0x3A40, 0x2C},
    {0x3A42, 0x39},
    {0x3A4E, 0x00},
    {0x3A52, 0x00},
    {0x3A56, 0x00},
    {0x3A5A, 0x00},
    {0x3A5E, 0x00},
    {0x3A62, 0x00},
    {0x3A6E, 0xA0},
    {0x3A70, 0x50},
    {0x3A8C, 0x04},
    {0x3A8D, 0x03},
    {0x3A8E, 0x09},
    {0x3A90, 0x38},
    {0x3A91, 0x42},
    {0x3A92, 0x3C},
    {0x3B0E, 0xF3},
    {0x3B12, 0xE5},
    {0x3B27, 0xC0},
    {0x3B2E, 0xEF},
    {0x3B30, 0x6A},
    {0x3B32, 0xF6},
    {0x3B36, 0xE1},
    {0x3B3A, 0xE8},
    {0x3B5A, 0x17},
    {0x3B5E, 0xEF},
    {0x3B60, 0x6A},
    {0x3B62, 0xF6},
    {0x3B66, 0xE1},
    {0x3B6A, 0xE8},
    {0x3B88, 0xEC},
    {0x3B8A, 0xED},
    {0x3B94, 0x71},
    {0x3B96, 0x72},
    {0x3B98, 0xDE},
    {0x3B9A, 0xDF},
    {0x3C0F, 0x06},
    {0x3C10, 0x06},
    {0x3C11, 0x06},
    {0x3C12, 0x06},
    {0x3C13, 0x06},
    {0x3C18, 0x20},
    {0x3C3A, 0x7A},
    {0x3C40, 0xF4},
    {0x3C48, 0xE6},
    {0x3C54, 0xCE},
    {0x3C56, 0xD0},
    {0x3C6C, 0x53},
    {0x3C6E, 0x55},
    {0x3C70, 0xC0},
    {0x3C72, 0xC2},
    {0x3C7E, 0xCE},
    {0x3C8C, 0xCF},
    {0x3C8E, 0xEB},
    {0x3C98, 0x54},
    {0x3C9A, 0x70},
    {0x3C9C, 0xC1},
    {0x3C9E, 0xDD},
    {0x3CB0, 0x7A},
    {0x3CB2, 0xBA},
    {0x3CC8, 0xBC},
    {0x3CCA, 0x7C},
    {0x3CD4, 0xEA},
    {0x3CD5, 0x01},
    {0x3CD6, 0x4A},
    {0x3CD8, 0x00},
    {0x3CD9, 0x00},
    {0x3CDA, 0xFF},
    {0x3CDB, 0x03},
    {0x3CDC, 0x00},
    {0x3CDD, 0x00},
    {0x3CDE, 0xFF},
    {0x3CDF, 0x03},
    {0x3CE4, 0x4C},
    {0x3CE6, 0xEC},
    {0x3CE7, 0x01},
    {0x3CE8, 0xFF},
    {0x3CE9, 0x03},
    {0x3CEA, 0x00},
    {0x3CEB, 0x00},
    {0x3CEC, 0xFF},
    {0x3CED, 0x03},
    {0x3CEE, 0x00},
    {0x3CEF, 0x00},
    {0x3E28, 0x82},
    {0x3E2A, 0x80},
    {0x3E30, 0x85},
    {0x3E32, 0x7D},
    {0x3E5C, 0xCE},
    {0x3E5E, 0xD3},
    {0x3E70, 0x53},
    {0x3E72, 0x58},
    {0x3E74, 0xC0},
    {0x3E76, 0xC5},
    {0x3E78, 0xC0},
    {0x3E79, 0x01},
    {0x3E7A, 0xD4},
    {0x3E7B, 0x01},
    {0x3EB4, 0x0B},
    {0x3EB5, 0x02},
    {0x3EB6, 0x4D},
    {0x3EEC, 0xF3},
    {0x3EEE, 0xE7},
    {0x3F01, 0x01},
    {0x3F28, 0x2D},
    {0x3F2A, 0x2D},
    {0x3F2C, 0x2D},
    {0x3F2E, 0x2D},
    {0x3F30, 0x23},
    {0x3F38, 0x2D},
    {0x3F3A, 0x2D},
    {0x3F3C, 0x2D},
    {0x3F3E, 0x28},
    {0x3F40, 0x1E},
    {0x3F48, 0x2D},
    {0x3F4A, 0x2D},
    {0x4004, 0xE4},
    {0x4006, 0xFF},
    {0x4018, 0x69},
    {0x401A, 0x84},
    {0x401C, 0xD6},
    {0x401E, 0xF1},
    {0x4038, 0xDE},
    {0x403A, 0x00},
    {0x403B, 0x01},
    {0x404C, 0x63},
    {0x404E, 0x85},
    {0x4050, 0xD0},
    {0x4052, 0xF2},
    {0x4108, 0xDD},
    {0x410A, 0xF7},
    {0x411C, 0x62},
    {0x411E, 0x7C},
    {0x4120, 0xCF},
    {0x4122, 0xE9},
    {0x4138, 0xE6},
    {0x413A, 0xF1},
    {0x414C, 0x6B},
    {0x414E, 0x76},
    {0x4150, 0xD8},
    {0x4152, 0xE3},
    {0x417E, 0x03},
    {0x417F, 0x01},
    {0x4186, 0xE0},
    {0x4190, 0xF3},
    {0x4192, 0xF7},
    {0x419C, 0x78},
    {0x419E, 0x7C},
    {0x41A0, 0xE5},
    {0x41A2, 0xE9},
    {0x41C8, 0xE2},
    {0x41CA, 0xFD},
    {0x41DC, 0x67},
    {0x41DE, 0x82},
    {0x41E0, 0xD4},
    {0x41E2, 0xEF},
    {0x4200, 0xDE},
    {0x4202, 0xDA},
    {0x4218, 0x63},
    {0x421A, 0x5F},
    {0x421C, 0xD0},
    {0x421E, 0xCC},
    {0x425A, 0x82},
    {0x425C, 0xEF},
    {0x4348, 0xFE},
    {0x4349, 0x06},
    {0x4352, 0xCE},
    {0x4420, 0x0B},
    {0x4421, 0x02},
    {0x4422, 0x4D},
    {0x4423, 0x0A},
    {0x4426, 0xF5},
    {0x442A, 0xE7},
    {0x4432, 0xF5},
    {0x4436, 0xE7},
    {0x4466, 0xB4},
    {0x446E, 0x32},
    {0x449F, 0x1C},
    {0x44A4, 0x2C},
    {0x44A6, 0x2C},
    {0x44A8, 0x2C},
    {0x44AA, 0x2C},
    {0x44B4, 0x2C},
    {0x44B6, 0x2C},
    {0x44B8, 0x2C},
    {0x44BA, 0x2C},
    {0x44C4, 0x2C},
    {0x44C6, 0x2C},
    {0x44C8, 0x2C},
    {0x4506, 0xF3},
    {0x450E, 0xE5},
    {0x4516, 0xF3},
    {0x4522, 0xE5},
    {0x4524, 0xF3},
    {0x452C, 0xE5},
    {0x453C, 0x22},
    {0x453D, 0x1B},
    {0x453E, 0x1B},
    {0x453F, 0x15},
    {0x4540, 0x15},
    {0x4541, 0x15},
    {0x4542, 0x15},
    {0x4543, 0x15},
    {0x4544, 0x15},
    {0x4548, 0x00},
    {0x4549, 0x01},
    {0x454A, 0x01},
    {0x454B, 0x06},
    {0x454C, 0x06},
    {0x454D, 0x06},
    {0x454E, 0x06},
    {0x454F, 0x06},
    {0x4550, 0x06},
    {0x4554, 0x55},
    {0x4555, 0x02},
    {0x4556, 0x42},
    {0x4557, 0x05},
    {0x4558, 0xFD},
    {0x4559, 0x05},
    {0x455A, 0x94},
    {0x455B, 0x06},
    {0x455D, 0x06},
    {0x455E, 0x49},
    {0x455F, 0x07},
    {0x4560, 0x7F},
    {0x4561, 0x07},
    {0x4562, 0xA5},
    {0x4564, 0x55},
    {0x4565, 0x02},
    {0x4566, 0x32},
    {0x4567, 0x05},
    {0x4568, 0xFD},
    {0x4569, 0x05},
    {0x456A, 0x94},
    {0x456B, 0x06},
    {0x456D, 0x06},
    {0x456E, 0x49},
    {0x456F, 0x07},
    {0x4572, 0xA5},
    {0x460C, 0x7D},
    {0x460E, 0xB1},
    {0x4614, 0xA8},
    {0x4616, 0xBe},
    {0x461C, 0x7E},
    {0x461E, 0xA7},
    {0x4624, 0xA8},
    {0x4626, 0xB2},
    {0x462C, 0x7E},
    {0x462E, 0x8A},
    {0x4630, 0x94},
    {0x4632, 0xA7},
    {0x4634, 0xFB},
    {0x4636, 0x2F},
    {0x4638, 0x81},
    {0x4639, 0x01},
    {0x463A, 0xB5},
    {0x463B, 0x01},
    {0x463C, 0x26},
    {0x463E, 0x30},
    {0x4640, 0xAC},
    {0x4641, 0x01},
    {0x4642, 0xB6},
    {0x4643, 0x01},
    {0x4644, 0xFC},
    {0x4646, 0x25},
    {0x4648, 0x82},
    {0x4649, 0x01},
    {0x464A, 0xAB},
    {0x464B, 0x01},
    {0x464C, 0x26},
    {0x464E, 0x30},
    {0x4654, 0xFC},
    {0x4656, 0x08},
    {0x4658, 0x12},
    {0x465A, 0x25},
    {0x4662, 0xFC},
    {0x46A2, 0xFB},
    {0x46D6, 0xF3},
    {0x46E6, 0x00},
    {0x46E8, 0xFF},
    {0x46E9, 0x03},
    {0x46EC, 0x7A},
    {0x46EE, 0xE5},
    {0x46F4, 0xEE},
    {0x46F6, 0xF2},
    {0x470C, 0xFF},
    {0x470D, 0x03},
    {0x470E, 0x00},
    {0x4714, 0xE0},
    {0x4716, 0xE4},
    {0x471E, 0xED},
    {0x472E, 0x00},
    {0x4730, 0xFF},
    {0x4731, 0x03},
    {0x4734, 0x7B},
    {0x4736, 0xDF},
    {0x4754, 0x7D},
    {0x4756, 0x8B},
    {0x4758, 0x93},
    {0x475A, 0xB1},
    {0x475C, 0xFB},
    {0x475E, 0x09},
    {0x4760, 0x11},
    {0x4762, 0x2F},
    {0x4766, 0xCC},
    {0x4776, 0xCB},
    {0x477E, 0x4A},
    {0x478E, 0x49},
    {0x4794, 0x7C},
    {0x4796, 0x8F},
    {0x4798, 0xB3},
    {0x4799, 0x00},
    {0x479A, 0xCC},
    {0x479C, 0xC1},
    {0x479E, 0xCB},
    {0x47A4, 0x7D},
    {0x47A6, 0x8E},
    {0x47A8, 0xB4},
    {0x47A9, 0x00},
    {0x47AA, 0xC0},
    {0x47AC, 0xFA},
    {0x47AE, 0x0D},
    {0x47B0, 0x31},
    {0x47B1, 0x01},
    {0x47B2, 0x4A},
    {0x47B3, 0x01},
    {0x47B4, 0x3F},
    {0x47B6, 0x49},
    {0x47BC, 0xFB},
    {0x47BE, 0x0C},
    {0x47C0, 0x32},
    {0x47C1, 0x01},
    {0x47C2, 0x3E},
    {0x47C3, 0x01},
    {0x4E00, 0x11},  //uncontinue mode
    {0xffff, 0x24},
    {0x3002, 0x00},  //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},  //Operating
};

const static I2C_ARRAY Sensor_8m_15fps_init_table_4lane_linear[] =
{
//  IMX678-AAQR1 All-pixel scan CSI-2_4lane 24MHz AD:10bit Output:10bit 720Mbps Master Mode LCG Mode 15fps Integration Time 66.613ms
     {0x3000, 0x01},
     {0x3001, 0x00},
     {0x3002, 0x01},
     {0x3014, 0x04},
     {0x3015, 0x06},
     {0x3018, 0x00},
     {0x3019, 0x00},
     {0x301A, 0x00},
     {0x301B, 0x00},
     {0x301C, 0x00},
     {0x301E, 0x01},
     {0x3020, 0x00},
     {0x3021, 0x00},
     {0x3022, 0x00},
     {0x3023, 0x00},
     {0x3028, 0xA6},
     {0x3029, 0x0E},
     {0x302A, 0x00},
     {0x302C, 0x28},
     {0x302D, 0x05},
     {0x3030, 0x00},
     {0x3031, 0x00},
     {0x3032, 0x00},
     {0x303C, 0x00},
     {0x303D, 0x00},
     {0x303E, 0x10},
     {0x303F, 0x0F},
     {0x3040, 0x03},
     {0x3042, 0x00},
     {0x3043, 0x00},
     {0x3044, 0x00},
     {0x3045, 0x00},
     {0x3046, 0x84},
     {0x3047, 0x08},
     {0x3050, 0x03},
     {0x3051, 0x00},
     {0x3052, 0x00},
     {0x3054, 0x0E},
     {0x3055, 0x00},
     {0x3056, 0x00},
     {0x3058, 0x8A},
     {0x3059, 0x01},
     {0x305A, 0x00},
     {0x3060, 0x16},
     {0x3061, 0x01},
     {0x3062, 0x00},
     {0x3064, 0xC4},
     {0x3065, 0x0C},
     {0x3066, 0x00},
     {0x3069, 0x00},
     {0x306B, 0x00},
     {0x3070, 0x00},
     {0x3071, 0x00},
     {0x3072, 0x00},
     {0x3073, 0x00},
     {0x3074, 0x00},
     {0x3075, 0x00},
     {0x3081, 0x00},
     {0x308C, 0x00},
     {0x308D, 0x01},
     {0x3094, 0x00},
     {0x3095, 0x00},
     {0x309C, 0x00},
     {0x309D, 0x00},
     {0x30A4, 0xAA},
     {0x30A6, 0x00},
     {0x30CC, 0x00},
     {0x30CD, 0x00},
     {0x30DC, 0x32},
     {0x30DD, 0x40},
     {0x3400, 0x01},
     {0x3460, 0x22},
     {0x355A, 0x64},
     {0x3A02, 0x7A},
     {0x3A10, 0xEC},
     {0x3A12, 0x71},
     {0x3A14, 0xDE},
     {0x3A20, 0x2B},
     {0x3A24, 0x22},
     {0x3A25, 0x25},
     {0x3A26, 0x2A},
     {0x3A27, 0x2C},
     {0x3A28, 0x39},
     {0x3A29, 0x38},
     {0x3A30, 0x04},
     {0x3A31, 0x04},
     {0x3A32, 0x03},
     {0x3A33, 0x03},
     {0x3A34, 0x09},
     {0x3A35, 0x06},
     {0x3A38, 0xCD},
     {0x3A3A, 0x4C},
     {0x3A3C, 0xB9},
     {0x3A3E, 0x30},
     {0x3A40, 0x2C},
     {0x3A42, 0x39},
     {0x3A4E, 0x00},
     {0x3A52, 0x00},
     {0x3A56, 0x00},
     {0x3A5A, 0x00},
     {0x3A5E, 0x00},
     {0x3A62, 0x00},
     {0x3A64, 0x00},
     {0x3A6E, 0xA0},
     {0x3A70, 0x50},
     {0x3A8C, 0x04},
     {0x3A8D, 0x03},
     {0x3A8E, 0x09},
     {0x3A90, 0x38},
     {0x3A91, 0x42},
     {0x3A92, 0x3C},
     {0x3B0E, 0xF3},
     {0x3B12, 0xE5},
     {0x3B27, 0xC0},
     {0x3B2E, 0xEF},
     {0x3B30, 0x6A},
     {0x3B32, 0xF6},
     {0x3B36, 0xE1},
     {0x3B3A, 0xE8},
     {0x3B5A, 0x17},
     {0x3B5E, 0xEF},
     {0x3B60, 0x6A},
     {0x3B62, 0xF6},
     {0x3B66, 0xE1},
     {0x3B6A, 0xE8},
     {0x3B88, 0xEC},
     {0x3B8A, 0xED},
     {0x3B94, 0x71},
     {0x3B96, 0x72},
     {0x3B98, 0xDE},
     {0x3B9A, 0xDF},
     {0x3C0F, 0x06},
     {0x3C10, 0x06},
     {0x3C11, 0x06},
     {0x3C12, 0x06},
     {0x3C13, 0x06},
     {0x3C18, 0x20},
     {0x3C37, 0x10},
     {0x3C3A, 0x7A},
     {0x3C40, 0xF4},
     {0x3C48, 0xE6},
     {0x3C54, 0xCE},
     {0x3C56, 0xD0},
     {0x3C6C, 0x53},
     {0x3C6E, 0x55},
     {0x3C70, 0xC0},
     {0x3C72, 0xC2},
     {0x3C7E, 0xCE},
     {0x3C8C, 0xCF},
     {0x3C8E, 0xEB},
     {0x3C98, 0x54},
     {0x3C9A, 0x70},
     {0x3C9C, 0xC1},
     {0x3C9E, 0xDD},
     {0x3CB0, 0x7A},
     {0x3CB2, 0xBA},
     {0x3CC8, 0xBC},
     {0x3CCA, 0x7C},
     {0x3CD4, 0xEA},
     {0x3CD5, 0x01},
     {0x3CD6, 0x4A},
     {0x3CD8, 0x00},
     {0x3CD9, 0x00},
     {0x3CDA, 0xFF},
     {0x3CDB, 0x03},
     {0x3CDC, 0x00},
     {0x3CDD, 0x00},
     {0x3CDE, 0xFF},
     {0x3CDF, 0x03},
     {0x3CE4, 0x4C},
     {0x3CE6, 0xEC},
     {0x3CE7, 0x01},
     {0x3CE8, 0xFF},
     {0x3CE9, 0x03},
     {0x3CEA, 0x00},
     {0x3CEB, 0x00},
     {0x3CEC, 0xFF},
     {0x3CED, 0x03},
     {0x3CEE, 0x00},
     {0x3CEF, 0x00},
     {0x3CF2, 0xFF},
     {0x3CF3, 0x03},
     {0x3CF4, 0x00},
     {0x3E28, 0x82},
     {0x3E2A, 0x80},
     {0x3E30, 0x85},
     {0x3E32, 0x7D},
     {0x3E5C, 0xCE},
     {0x3E5E, 0xD3},
     {0x3E70, 0x53},
     {0x3E72, 0x58},
     {0x3E74, 0xC0},
     {0x3E76, 0xC5},
     {0x3E78, 0xC0},
     {0x3E79, 0x01},
     {0x3E7A, 0xD4},
     {0x3E7B, 0x01},
     {0x3EB4, 0x0B},
     {0x3EB5, 0x02},
     {0x3EB6, 0x4D},
     {0x3EB7, 0x42},
     {0x3EEC, 0xF3},
     {0x3EEE, 0xE7},
     {0x3F01, 0x01},
     {0x3F24, 0x10},
     {0x3F28, 0x2D},
     {0x3F2A, 0x2D},
     {0x3F2C, 0x2D},
     {0x3F2E, 0x2D},
     {0x3F30, 0x23},
     {0x3F38, 0x2D},
     {0x3F3A, 0x2D},
     {0x3F3C, 0x2D},
     {0x3F3E, 0x28},
     {0x3F40, 0x1E},
     {0x3F48, 0x2D},
     {0x3F4A, 0x2D},
     {0x3F4C, 0x00},
     {0x4004, 0xE4},
     {0x4006, 0xFF},
     {0x4018, 0x69},
     {0x401A, 0x84},
     {0x401C, 0xD6},
     {0x401E, 0xF1},
     {0x4038, 0xDE},
     {0x403A, 0x00},
     {0x403B, 0x01},
     {0x404C, 0x63},
     {0x404E, 0x85},
     {0x4050, 0xD0},
     {0x4052, 0xF2},
     {0x4108, 0xDD},
     {0x410A, 0xF7},
     {0x411C, 0x62},
     {0x411E, 0x7C},
     {0x4120, 0xCF},
     {0x4122, 0xE9},
     {0x4138, 0xE6},
     {0x413A, 0xF1},
     {0x414C, 0x6B},
     {0x414E, 0x76},
     {0x4150, 0xD8},
     {0x4152, 0xE3},
     {0x417E, 0x03},
     {0x417F, 0x01},
     {0x4186, 0xE0},
     {0x4190, 0xF3},
     {0x4192, 0xF7},
     {0x419C, 0x78},
     {0x419E, 0x7C},
     {0x41A0, 0xE5},
     {0x41A2, 0xE9},
     {0x41C8, 0xE2},
     {0x41CA, 0xFD},
     {0x41DC, 0x67},
     {0x41DE, 0x82},
     {0x41E0, 0xD4},
     {0x41E2, 0xEF},
     {0x4200, 0xDE},
     {0x4202, 0xDA},
     {0x4218, 0x63},
     {0x421A, 0x5F},
     {0x421C, 0xD0},
     {0x421E, 0xCC},
     {0x425A, 0x82},
     {0x425C, 0xEF},
     {0x4348, 0xFE},
     {0x4349, 0x06},
     {0x4352, 0xCE},
     {0x4420, 0x0B},
     {0x4421, 0x02},
     {0x4422, 0x4D},
     {0x4423, 0x0A},
     {0x4426, 0xF5},
     {0x442A, 0xE7},
     {0x4432, 0xF5},
     {0x4436, 0xE7},
     {0x4466, 0xB4},
     {0x446E, 0x32},
     {0x449F, 0x1C},
     {0x44A4, 0x2C},
     {0x44A6, 0x2C},
     {0x44A8, 0x2C},
     {0x44AA, 0x2C},
     {0x44B4, 0x2C},
     {0x44B6, 0x2C},
     {0x44B8, 0x2C},
     {0x44BA, 0x2C},
     {0x44C4, 0x2C},
     {0x44C6, 0x2C},
     {0x44C8, 0x2C},
     {0x4506, 0xF3},
     {0x450E, 0xE5},
     {0x4516, 0xF3},
     {0x4522, 0xE5},
     {0x4524, 0xF3},
     {0x452C, 0xE5},
     {0x453C, 0x22},
     {0x453D, 0x1B},
     {0x453E, 0x1B},
     {0x453F, 0x15},
     {0x4540, 0x15},
     {0x4541, 0x15},
     {0x4542, 0x15},
     {0x4543, 0x15},
     {0x4544, 0x15},
     {0x4548, 0x00},
     {0x4549, 0x01},
     {0x454A, 0x01},
     {0x454B, 0x06},
     {0x454C, 0x06},
     {0x454D, 0x06},
     {0x454E, 0x06},
     {0x454F, 0x06},
     {0x4550, 0x06},
     {0x4554, 0x55},
     {0x4555, 0x02},
     {0x4556, 0x42},
     {0x4557, 0x05},
     {0x4558, 0xFD},
     {0x4559, 0x05},
     {0x455A, 0x94},
     {0x455B, 0x06},
     {0x455D, 0x06},
     {0x455E, 0x49},
     {0x455F, 0x07},
     {0x4560, 0x7F},
     {0x4561, 0x07},
     {0x4562, 0xA5},
     {0x4564, 0x55},
     {0x4565, 0x02},
     {0x4566, 0x42},
     {0x4567, 0x05},
     {0x4568, 0xFD},
     {0x4569, 0x05},
     {0x456A, 0x94},
     {0x456B, 0x06},
     {0x456D, 0x06},
     {0x456E, 0x49},
     {0x456F, 0x07},
     {0x4572, 0xA5},
     {0x460C, 0x7D},
     {0x460E, 0xB1},
     {0x4614, 0xA8},
     {0x4616, 0xB2},
     {0x461C, 0x7E},
     {0x461E, 0xA7},
     {0x4624, 0xA8},
     {0x4626, 0xB2},
     {0x462C, 0x7E},
     {0x462E, 0x8A},
     {0x4630, 0x94},
     {0x4632, 0xA7},
     {0x4634, 0xFB},
     {0x4636, 0x2F},
     {0x4638, 0x81},
     {0x4639, 0x01},
     {0x463A, 0xB5},
     {0x463B, 0x01},
     {0x463C, 0x26},
     {0x463E, 0x30},
     {0x4640, 0xAC},
     {0x4641, 0x01},
     {0x4642, 0xB6},
     {0x4643, 0x01},
     {0x4644, 0xFC},
     {0x4646, 0x25},
     {0x4648, 0x82},
     {0x4649, 0x01},
     {0x464A, 0xAB},
     {0x464B, 0x01},
     {0x464C, 0x26},
     {0x464E, 0x30},
     {0x4654, 0xFC},
     {0x4656, 0x08},
     {0x4658, 0x12},
     {0x465A, 0x25},
     {0x4662, 0xFC},
     {0x46A2, 0xFB},
     {0x46D6, 0xF3},
     {0x46E6, 0x00},
     {0x46E8, 0xFF},
     {0x46E9, 0x03},
     {0x46EC, 0x7A},
     {0x46EE, 0xE5},
     {0x46F4, 0xEE},
     {0x46F6, 0xF2},
     {0x470C, 0xFF},
     {0x470D, 0x03},
     {0x470E, 0x00},
     {0x4714, 0xE0},
     {0x4716, 0xE4},
     {0x471E, 0xED},
     {0x472E, 0x00},
     {0x4730, 0xFF},
     {0x4731, 0x03},
     {0x4734, 0x7B},
     {0x4736, 0xDF},
     {0x4754, 0x7D},
     {0x4756, 0x8B},
     {0x4758, 0x93},
     {0x475A, 0xB1},
     {0x475C, 0xFB},
     {0x475E, 0x09},
     {0x4760, 0x11},
     {0x4762, 0x2F},
     {0x4766, 0xCC},
     {0x4776, 0xCB},
     {0x477E, 0x4A},
     {0x478E, 0x49},
     {0x4794, 0x7C},
     {0x4796, 0x8F},
     {0x4798, 0xB3},
     {0x4799, 0x00},
     {0x479A, 0xCC},
     {0x479C, 0xC1},
     {0x479E, 0xCB},
     {0x47A4, 0x7D},
     {0x47A6, 0x8E},
     {0x47A8, 0xB4},
     {0x47A9, 0x00},
     {0x47AA, 0xC0},
     {0x47AC, 0xFA},
     {0x47AE, 0x0D},
     {0x47B0, 0x31},
     {0x47B1, 0x01},
     {0x47B2, 0x4A},
     {0x47B3, 0x01},
     {0x47B4, 0x3F},
     {0x47B6, 0x49},
     {0x47BC, 0xFB},
     {0x47BE, 0x0C},
     {0x47C0, 0x32},
     {0x47C1, 0x01},
     {0x47C2, 0x3E},
     {0x47C3, 0x01},
     {0x4E3C, 0x07},
     {0x4E00, 0x11},  //uncontinue mode
     {0xffff, 0x24},
     {0x3002, 0x00},  //Master mode start
     {0xffff, 0x10},
     {0x3000, 0x00},  //Operating
};

const static I2C_ARRAY Sensor_init_table_4lane_HDR_DOL[] =
{
  /*
IMX678-AAQR1
All-pixel scan
CSI-2_4lane
24MHz
AD:10bit Output:10bit
1440Mbps
Master Mode
LCG Mode
DOL HDR 2frame VC
30fps
Integration Time
LEF:0.993ms
SEF:0.104ms

   */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3014, 0x04},
    {0x3000, 0x01},
    {0x3001, 0x00},
    {0x3002, 0x01},
    {0x3014, 0x04},
    {0x3015, 0x03},
    {0x301A, 0x01},
    {0x301C, 0x01},
    {0x3022, 0x00},
    {0x3023, 0x00},
    {0x3028, 0xCA},		//VMAX 2250 08CA
    {0x3029, 0x08},
    {0x302C, 0x26},		//HMAX 550 0226
    {0x302D, 0x02},
    {0x3050, 0x0E},  //0x0e  0e
    {0x3051, 0x11},  //0x11  08
    {0x3054, 0x05},
    {0x3060, 0x13},  //0x83
    {0x3061, 0x00},  //0x00

    {0x3070, 0x6c},
    {0x3071, 0x00},

    {0x30A4, 0xAA},
    {0x30A6, 0x00},
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x3400, 0x00},
    {0x3460, 0x22},
    {0x355A, 0x64},
    {0x3A02, 0x7A},
    {0x3A10, 0xEC},
    {0x3A12, 0x71},
    {0x3A14, 0xDE},
    {0x3A20, 0x2B},
    {0x3A24, 0x22},
    {0x3A25, 0x25},
    {0x3A26, 0x2A},
    {0x3A27, 0x2C},
    {0x3A28, 0x39},
    {0x3A29, 0x38},
    {0x3A30, 0x04},
    {0x3A31, 0x04},
    {0x3A32, 0x03},
    {0x3A33, 0x03},
    {0x3A34, 0x09},
    {0x3A35, 0x06},
    {0x3A38, 0xCD},
    {0x3A3A, 0x4C},
    {0x3A3C, 0xB9},
    {0x3A3E, 0x30},
    {0x3A40, 0x2C},
    {0x3A42, 0x39},
    {0x3A4E, 0x00},
    {0x3A52, 0x00},
    {0x3A56, 0x00},
    {0x3A5A, 0x00},
    {0x3A5E, 0x00},
    {0x3A62, 0x00},
    {0x3A6E, 0xA0},
    {0x3A70, 0x50},
    {0x3A8C, 0x04},
    {0x3A8D, 0x03},
    {0x3A8E, 0x09},
    {0x3A90, 0x38},
    {0x3A91, 0x42},
    {0x3A92, 0x3C},
    {0x3B0E, 0xF3},
    {0x3B12, 0xE5},
    {0x3B27, 0xC0},
    {0x3B2E, 0xEF},
    {0x3B30, 0x6A},
    {0x3B32, 0xF6},
    {0x3B36, 0xE1},
    {0x3B3A, 0xE8},
    {0x3B5A, 0x17},
    {0x3B5E, 0xEF},
    {0x3B60, 0x6A},
    {0x3B62, 0xF6},
    {0x3B66, 0xE1},
    {0x3B6A, 0xE8},
    {0x3B88, 0xEC},
    {0x3B8A, 0xED},
    {0x3B94, 0x71},
    {0x3B96, 0x72},
    {0x3B98, 0xDE},
    {0x3B9A, 0xDF},
    {0x3C0F, 0x06},
    {0x3C10, 0x06},
    {0x3C11, 0x06},
    {0x3C12, 0x06},
    {0x3C13, 0x06},
    {0x3C18, 0x20},
    {0x3C3A, 0x7A},
    {0x3C40, 0xF4},
    {0x3C48, 0xE6},
    {0x3C54, 0xCE},
    {0x3C56, 0xD0},
    {0x3C6C, 0x53},
    {0x3C6E, 0x55},
    {0x3C70, 0xC0},
    {0x3C72, 0xC2},
    {0x3C7E, 0xCE},
    {0x3C8C, 0xCF},
    {0x3C8E, 0xEB},
    {0x3C98, 0x54},
    {0x3C9A, 0x70},
    {0x3C9C, 0xC1},
    {0x3C9E, 0xDD},
    {0x3CB0, 0x7A},
    {0x3CB2, 0xBA},
    {0x3CC8, 0xBC},
    {0x3CCA, 0x7C},
    {0x3CD4, 0xEA},
    {0x3CD5, 0x01},
    {0x3CD6, 0x4A},
    {0x3CD8, 0x00},
    {0x3CD9, 0x00},
    {0x3CDA, 0xFF},
    {0x3CDB, 0x03},
    {0x3CDC, 0x00},
    {0x3CDD, 0x00},
    {0x3CDE, 0xFF},
    {0x3CDF, 0x03},
    {0x3CE4, 0x4C},
    {0x3CE6, 0xEC},
    {0x3CE7, 0x01},
    {0x3CE8, 0xFF},
    {0x3CE9, 0x03},
    {0x3CEA, 0x00},
    {0x3CEB, 0x00},
    {0x3CEC, 0xFF},
    {0x3CED, 0x03},
    {0x3CEE, 0x00},
    {0x3CEF, 0x00},
    {0x3E28, 0x82},
    {0x3E2A, 0x80},
    {0x3E30, 0x85},
    {0x3E32, 0x7D},
    {0x3E5C, 0xCE},
    {0x3E5E, 0xD3},
    {0x3E70, 0x53},
    {0x3E72, 0x58},
    {0x3E74, 0xC0},
    {0x3E76, 0xC5},
    {0x3E78, 0xC0},
    {0x3E79, 0x01},
    {0x3E7A, 0xD4},
    {0x3E7B, 0x01},
    {0x3EB4, 0x0B},
    {0x3EB5, 0x02},
    {0x3EB6, 0x4D},
    {0x3EEC, 0xF3},
    {0x3EEE, 0xE7},
    {0x3F01, 0x01},
    {0x3F24, 0x10},
    {0x3F28, 0x2D},
    {0x3F2A, 0x2D},
    {0x3F2C, 0x2D},
    {0x3F2E, 0x2D},
    {0x3F30, 0x23},
    {0x3F38, 0x2D},
    {0x3F3A, 0x2D},
    {0x3F3C, 0x2D},
    {0x3F3E, 0x28},
    {0x3F40, 0x1E},
    {0x3F48, 0x2D},
    {0x3F4A, 0x2D},
    {0x4004, 0xE4},
    {0x4006, 0xFF},
    {0x4018, 0x69},
    {0x401A, 0x84},
    {0x401C, 0xD6},
    {0x401E, 0xF1},
    {0x4038, 0xDE},
    {0x403A, 0x00},
    {0x403B, 0x01},
    {0x404C, 0x63},
    {0x404E, 0x85},
    {0x4050, 0xD0},
    {0x4052, 0xF2},
    {0x4108, 0xDD},
    {0x410A, 0xF7},
    {0x411C, 0x62},
    {0x411E, 0x7C},
    {0x4120, 0xCF},
    {0x4122, 0xE9},
    {0x4138, 0xE6},
    {0x413A, 0xF1},
    {0x414C, 0x6B},
    {0x414E, 0x76},
    {0x4150, 0xD8},
    {0x4152, 0xE3},
    {0x417E, 0x03},
    {0x417F, 0x01},
    {0x4186, 0xE0},
    {0x4190, 0xF3},
    {0x4192, 0xF7},
    {0x419C, 0x78},
    {0x419E, 0x7C},
    {0x41A0, 0xE5},
    {0x41A2, 0xE9},
    {0x41C8, 0xE2},
    {0x41CA, 0xFD},
    {0x41DC, 0x67},
    {0x41DE, 0x82},
    {0x41E0, 0xD4},
    {0x41E2, 0xEF},
    {0x4200, 0xDE},
    {0x4202, 0xDA},
    {0x4218, 0x63},
    {0x421A, 0x5F},
    {0x421C, 0xD0},
    {0x421E, 0xCC},
    {0x425A, 0x82},
    {0x425C, 0xEF},
    {0x4348, 0xFE},
    {0x4349, 0x06},
    {0x4352, 0xCE},
    {0x4420, 0x0B},
    {0x4421, 0x02},
    {0x4422, 0x4D},
    {0x4426, 0xF5},
    {0x442A, 0xE7},
    {0x4432, 0xF5},
    {0x4436, 0xE7},
    {0x4466, 0xB4},
    {0x446E, 0x32},
    {0x449F, 0x1C},
    {0x44A4, 0x2C},
    {0x44A6, 0x2C},
    {0x44A8, 0x2C},
    {0x44AA, 0x2C},
    {0x44B4, 0x2C},
    {0x44B6, 0x2C},
    {0x44B8, 0x2C},
    {0x44BA, 0x2C},
    {0x44C4, 0x2C},
    {0x44C6, 0x2C},
    {0x44C8, 0x2C},
    {0x4506, 0xF3},
    {0x450E, 0xE5},
    {0x4516, 0xF3},
    {0x4522, 0xE5},
    {0x4524, 0xF3},
    {0x452C, 0xE5},
    {0x453C, 0x22},
    {0x453D, 0x1B},
    {0x453E, 0x1B},
    {0x453F, 0x15},
    {0x4540, 0x15},
    {0x4541, 0x15},
    {0x4542, 0x15},
    {0x4543, 0x15},
    {0x4544, 0x15},
    {0x4548, 0x00},
    {0x4549, 0x01},
    {0x454A, 0x01},
    {0x454B, 0x06},
    {0x454C, 0x06},
    {0x454D, 0x06},
    {0x454E, 0x06},
    {0x454F, 0x06},
    {0x4550, 0x06},
    {0x4554, 0x55},
    {0x4555, 0x02},
    {0x4556, 0x42},
    {0x4557, 0x05},
    {0x4558, 0xFD},
    {0x4559, 0x05},
    {0x455A, 0x94},
    {0x455B, 0x06},
    {0x455D, 0x06},
    {0x455E, 0x49},
    {0x455F, 0x07},
    {0x4560, 0x7F},
    {0x4561, 0x07},
    {0x4562, 0xA5},
    {0x4564, 0x55},
    {0x4565, 0x02},
    {0x4566, 0x42},
    {0x4567, 0x05},
    {0x4568, 0xFD},
    {0x4569, 0x05},
    {0x456A, 0x94},
    {0x456B, 0x06},
    {0x456D, 0x06},
    {0x456E, 0x49},
    {0x456F, 0x07},
    {0x4572, 0xA5},
    {0x460C, 0x7D},
    {0x460E, 0xB1},
    {0x4614, 0xA8},
    {0x4616, 0xB2},
    {0x461C, 0x7E},
    {0x461E, 0xA7},
    {0x4624, 0xA8},
    {0x4626, 0xB2},
    {0x462C, 0x7E},
    {0x462E, 0x8A},
    {0x4630, 0x94},
    {0x4632, 0xA7},
    {0x4634, 0xFB},
    {0x4636, 0x2F},
    {0x4638, 0x81},
    {0x4639, 0x01},
    {0x463A, 0xB5},
    {0x463B, 0x01},
    {0x463C, 0x26},
    {0x463E, 0x30},
    {0x4640, 0xAC},
    {0x4641, 0x01},
    {0x4642, 0xB6},
    {0x4643, 0x01},
    {0x4644, 0xFC},
    {0x4646, 0x25},
    {0x4648, 0x82},
    {0x4649, 0x01},
    {0x464A, 0xAB},
    {0x464B, 0x01},
    {0x464C, 0x26},
    {0x464E, 0x30},
    {0x4654, 0xFC},
    {0x4656, 0x08},
    {0x4658, 0x12},
    {0x465A, 0x25},
    {0x4662, 0xFC},
    {0x46A2, 0xFB},
    {0x46D6, 0xF3},
    {0x46E6, 0x00},
    {0x46E8, 0xFF},
    {0x46E9, 0x03},
    {0x46EC, 0x7A},
    {0x46EE, 0xE5},
    {0x46F4, 0xEE},
    {0x46F6, 0xF2},
    {0x470C, 0xFF},
    {0x470D, 0x03},
    {0x470E, 0x00},
    {0x4714, 0xE0},
    {0x4716, 0xE4},
    {0x471E, 0xED},
    {0x472E, 0x00},
    {0x4730, 0xFF},
    {0x4731, 0x03},
    {0x4734, 0x7B},
    {0x4736, 0xDF},
    {0x4754, 0x7D},
    {0x4756, 0x8B},
    {0x4758, 0x93},
    {0x475A, 0xB1},
    {0x475C, 0xFB},
    {0x475E, 0x09},
    {0x4760, 0x11},
    {0x4762, 0x2F},
    {0x4766, 0xCC},
    {0x4776, 0xCB},
    {0x477E, 0x4A},
    {0x478E, 0x49},
    {0x4794, 0x7C},
    {0x4796, 0x8F},
    {0x4798, 0xB3},
    {0x4799, 0x00},
    {0x479A, 0xCC},
    {0x479C, 0xC1},
    {0x479E, 0xCB},
    {0x47A4, 0x7D},
    {0x47A6, 0x8E},
    {0x47A8, 0xB4},
    {0x47A9, 0x00},
    {0x47AA, 0xC0},
    {0x47AC, 0xFA},
    {0x47AE, 0x0D},
    {0x47B0, 0x31},
    {0x47B1, 0x01},
    {0x47B2, 0x4A},
    {0x47B3, 0x01},
    {0x47B4, 0x3F},
    {0x47B6, 0x49},
    {0x47BC, 0xFB},
    {0x47BE, 0x0C},
    {0x47C0, 0x32},
    {0x47C1, 0x01},
    {0x47C2, 0x3E},
    {0x47C3, 0x01},
    {0x3002, 0x00},    //Master mode start
    {0x3000, 0x00},
};

const static I2C_ARRAY Sensor_init_table_4lane_Clear_HDR_VC[] =
{
  /*
      All-pixel scan
      CSI-2_4lane
      24MHz
      AD:10bit Output:10bit
      1440Mbps
      Master Mode
      Clear HDR VC
      30fps
      Integration Time
      LEF:33.289ms
   */
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    {0x3000, 0x01},
    {0x3001, 0x00},
    {0x3002, 0x01},
    {0x3014, 0x04},
    {0x3015, 0x03},
    {0x3018, 0x00},
    {0x301A, 0x08},    // Diff 0x01
    {0x301B, 0x00},
    {0x301C, 0x00},    // Diff 0x01
    {0x301E, 0x01},
    {0x3020, 0x00},
    {0x3021, 0x00},
    {0x3022, 0x00},
    {0x3023, 0x00},

    {0x3028, 0x94},
    {0x3029, 0x11},
    {0x302A, 0x00},
    {0x302C, 0x26},
    {0x302D, 0x02},
    {0x3030, 0x00},
    {0x3031, 0x00},
    {0x3032, 0x00},
    {0x303C, 0x00},
    {0x303D, 0x00},
    {0x303E, 0x10},
    {0x303F, 0x0F},
    {0x3040, 0x03},
    {0x3042, 0x00},
    {0x3043, 0x00},
    {0x3044, 0x00},
    {0x3045, 0x00},
    {0x3046, 0x84},
    {0x3047, 0x08},
    {0x3050, 0x06},
    {0x3051, 0x00},
    {0x3052, 0x01},
    {0x3054, 0x0E},
    {0x3055, 0x00},
    {0x3056, 0x00},
    {0x3058, 0x8A},
    {0x3059, 0x01},
    {0x305A, 0x00},
    {0x3060, 0x16},
    {0x3061, 0x01},
    {0x3062, 0x00},
    {0x3064, 0xC4},
    {0x3065, 0x0C},
    {0x3066, 0x00},
    {0x3069, 0x01},
    {0x306B, 0x04},    // Diff 0x00
    {0x3070, 0x00},
    {0x3071, 0x00},
    {0x3072, 0x00},
    {0x3073, 0x00},
    {0x3074, 0x00},
    {0x3075, 0x00},
    {0x3081, 0x02},    // Diff 0x00
    {0x308C, 0x00},
    {0x308D, 0x01},
    {0x3094, 0x00},
    {0x3095, 0x00},
    {0x309C, 0x00},
    {0x309D, 0x00},

    {0x30A4, 0xAA},
    {0x30A6, 0x00},
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x30DC, 0x32},
    {0x30DD, 0x40},
    {0x3400, 0x01},
    {0x3460, 0x22},
    {0x355A, 0x00},    // Diff 0x64
    {0x3A02, 0x7A},
    {0x3A10, 0xEC},
    {0x3A12, 0x71},
    {0x3A14, 0xDE},
    {0x3A20, 0x2B},
    {0x3A24, 0x22},
    {0x3A25, 0x25},
    {0x3A26, 0x2A},
    {0x3A27, 0x2C},
    {0x3A28, 0x39},
    {0x3A29, 0x38},
    {0x3A30, 0x04},
    {0x3A31, 0x04},
    {0x3A32, 0x03},
    {0x3A33, 0x03},
    {0x3A34, 0x09},
    {0x3A35, 0x06},
    {0x3A38, 0xCD},
    {0x3A3A, 0x4C},
    {0x3A3C, 0xB9},
    {0x3A3E, 0x30},
    {0x3A40, 0x2C},
    {0x3A42, 0x39},
    {0x3A4E, 0x00},
    {0x3A52, 0x00},
    {0x3A56, 0x00},
    {0x3A5A, 0x00},
    {0x3A5E, 0x00},
    {0x3A62, 0x00},
    {0x3A64, 0x01},    // Diff 0x00
    {0x3A6E, 0xA0},
    {0x3A70, 0x50},
    {0x3A8C, 0x04},
    {0x3A8D, 0x03},
    {0x3A8E, 0x09},
    {0x3A90, 0x38},
    {0x3A91, 0x42},
    {0x3A92, 0x3C},
    {0x3B0E, 0xF3},
    {0x3B12, 0xE5},
    {0x3B27, 0xC0},
    {0x3B2E, 0xEF},
    {0x3B30, 0x6A},
    {0x3B32, 0xF6},
    {0x3B36, 0xE1},
    {0x3B3A, 0xE8},
    {0x3B5A, 0x17},
    {0x3B5E, 0xEF},
    {0x3B60, 0x6A},
    {0x3B62, 0xF6},
    {0x3B66, 0xE1},
    {0x3B6A, 0xE8},
    {0x3B88, 0xEC},
    {0x3B8A, 0xED},
    {0x3B94, 0x71},
    {0x3B96, 0x72},
    {0x3B98, 0xDE},
    {0x3B9A, 0xDF},
    {0x3C0F, 0x06},
    {0x3C10, 0x06},
    {0x3C11, 0x06},
    {0x3C12, 0x06},
    {0x3C13, 0x06},
    {0x3C18, 0x20},
    {0x3C37, 0x30},    // Diff 0x10
    {0x3C3A, 0x7A},
    {0x3C40, 0xF4},
    {0x3C48, 0xE6},
    {0x3C54, 0xCE},
    {0x3C56, 0xD0},
    {0x3C6C, 0x53},
    {0x3C6E, 0x55},
    {0x3C70, 0xC0},
    {0x3C72, 0xC2},
    {0x3C7E, 0xCE},
    {0x3C8C, 0xCF},
    {0x3C8E, 0xEB},
    {0x3C98, 0x54},
    {0x3C9A, 0x70},
    {0x3C9C, 0xC1},
    {0x3C9E, 0xDD},
    {0x3CB0, 0x7A},
    {0x3CB2, 0xBA},
    {0x3CC8, 0xBC},
    {0x3CCA, 0x7C},
    {0x3CD4, 0xEA},
    {0x3CD5, 0x01},
    {0x3CD6, 0x4A},
    {0x3CD8, 0x00},
    {0x3CD9, 0x00},
    {0x3CDA, 0xFF},
    {0x3CDB, 0x03},
    {0x3CDC, 0x00},
    {0x3CDD, 0x00},
    {0x3CDE, 0xFF},
    {0x3CDF, 0x03},
    {0x3CE4, 0x4C},
    {0x3CE6, 0xEC},
    {0x3CE7, 0x01},
    {0x3CE8, 0xFF},
    {0x3CE9, 0x03},
    {0x3CEA, 0x00},
    {0x3CEB, 0x00},
    {0x3CEC, 0xFF},
    {0x3CED, 0x03},
    {0x3CEE, 0x00},
    {0x3CEF, 0x00},
    {0x3CF2, 0x78},    // Diff 0xFF
    {0x3CF3, 0x00},    // Diff 0x03
    {0x3CF4, 0xA5},    // Diff 0x00
    {0x3E28, 0x82},
    {0x3E2A, 0x80},
    {0x3E30, 0x85},
    {0x3E32, 0x7D},
    {0x3E5C, 0xCE},
    {0x3E5E, 0xD3},
    {0x3E70, 0x53},
    {0x3E72, 0x58},
    {0x3E74, 0xC0},
    {0x3E76, 0xC5},
    {0x3E78, 0xC0},
    {0x3E79, 0x01},
    {0x3E7A, 0xD4},
    {0x3E7B, 0x01},
    {0x3EB4, 0x7B},    // Diff 0x0B
    {0x3EB5, 0x00},    // Diff 0x02
    {0x3EB6, 0xA5},    // Diff 0x4D
    {0x3EB7, 0x40},    // Diff 0x42
    {0x3EEC, 0xF3},
    {0x3EEE, 0xE7},
    {0x3F01, 0x01},
    {0x3F24, 0x17},    // Diff 0x10
    {0x3F28, 0x2D},
    {0x3F2A, 0x2D},
    {0x3F2C, 0x2D},
    {0x3F2E, 0x2D},
    {0x3F30, 0x23},
    {0x3F38, 0x2D},
    {0x3F3A, 0x2D},
    {0x3F3C, 0x2D},
    {0x3F3E, 0x28},
    {0x3F40, 0x1E},
    {0x3F48, 0x2D},
    {0x3F4A, 0x2D},
    {0x3F4C, 0x2D},    // Diff 0x00
    {0x4004, 0xE4},
    {0x4006, 0xFF},
    {0x4018, 0x69},
    {0x401A, 0x84},
    {0x401C, 0xD6},
    {0x401E, 0xF1},
    {0x4038, 0xDE},
    {0x403A, 0x00},
    {0x403B, 0x01},
    {0x404C, 0x63},
    {0x404E, 0x85},
    {0x4050, 0xD0},
    {0x4052, 0xF2},
    {0x4108, 0xDD},
    {0x410A, 0xF7},
    {0x411C, 0x62},
    {0x411E, 0x7C},
    {0x4120, 0xCF},
    {0x4122, 0xE9},
    {0x4138, 0xE6},
    {0x413A, 0xF1},
    {0x414C, 0x6B},
    {0x414E, 0x76},
    {0x4150, 0xD8},
    {0x4152, 0xE3},
    {0x417E, 0x03},
    {0x417F, 0x01},
    {0x4186, 0xE0},
    {0x4190, 0xF3},
    {0x4192, 0xF7},
    {0x419C, 0x78},
    {0x419E, 0x7C},
    {0x41A0, 0xE5},
    {0x41A2, 0xE9},
    {0x41C8, 0xE2},
    {0x41CA, 0xFD},
    {0x41DC, 0x67},
    {0x41DE, 0x82},
    {0x41E0, 0xD4},
    {0x41E2, 0xEF},
    {0x4200, 0xDE},
    {0x4202, 0xDA},
    {0x4218, 0x63},
    {0x421A, 0x5F},
    {0x421C, 0xD0},
    {0x421E, 0xCC},
    {0x425A, 0x82},
    {0x425C, 0xEF},
    {0x4348, 0xFE},
    {0x4349, 0x06},
    {0x4352, 0xCE},
    {0x4420, 0xFF},    // Diff 0x0B
    {0x4421, 0x03},    // Diff 0x02
    {0x4422, 0x00},    // Diff 0x4D
    {0x4423, 0x08},    // Diff 0x0A
    {0x4426, 0xF5},
    {0x442A, 0xE7},
    {0x4432, 0xF5},
    {0x4436, 0xE7},
    {0x4466, 0xB4},
    {0x446E, 0x32},
    {0x449F, 0x1C},
    {0x44A4, 0x37},    // Diff 0x2C
    {0x44A6, 0x37},    // Diff 0x2C
    {0x44A8, 0x37},    // Diff 0x2C
    {0x44AA, 0x37},    // Diff 0x2C
    {0x44B4, 0x37},    // Diff 0x2C
    {0x44B6, 0x37},    // Diff 0x2C
    {0x44B8, 0x37},    // Diff 0x2C
    {0x44BA, 0x37},    // Diff 0x2C
    {0x44C4, 0x37},    // Diff 0x2C
    {0x44C6, 0x37},    // Diff 0x2C
    {0x44C8, 0x37},    // Diff 0x2C
    {0x4506, 0xF3},
    {0x450E, 0xE5},
    {0x4516, 0xF3},
    {0x4522, 0xE5},
    {0x4524, 0xF3},
    {0x452C, 0xE5},
    {0x453C, 0x22},
    {0x453D, 0x18},    // Diff 0x1B
    {0x453E, 0x18},    // Diff 0x1B
    {0x453F, 0x11},    // Diff 0x15
    {0x4540, 0x11},    // Diff 0x15
    {0x4541, 0x11},    // Diff 0x15
    {0x4542, 0x11},    // Diff 0x15
    {0x4543, 0x11},    // Diff 0x15
    {0x4544, 0x11},    // Diff 0x15
    {0x4548, 0x00},
    {0x4549, 0x00},    // Diff 0x01
    {0x454A, 0x00},    // Diff 0x01
    {0x454B, 0x04},    // Diff 0x06
    {0x454C, 0x04},    // Diff 0x06
    {0x454D, 0x04},    // Diff 0x06
    {0x454E, 0x04},    // Diff 0x06
    {0x454F, 0x04},    // Diff 0x06
    {0x4550, 0x04},    // Diff 0x06
    {0x4554, 0x55},
    {0x4555, 0x02},
    {0x4556, 0x42},
    {0x4557, 0x05},
    {0x4558, 0xFD},
    {0x4559, 0x05},
    {0x455A, 0x94},
    {0x455B, 0x06},
    {0x455D, 0x06},
    {0x455E, 0x49},
    {0x455F, 0x07},
    {0x4560, 0x7F},
    {0x4561, 0x07},
    {0x4562, 0xA5},
    {0x4564, 0x55},
    {0x4565, 0x02},
    {0x4566, 0x42},
    {0x4567, 0x05},
    {0x4568, 0xFD},
    {0x4569, 0x05},
    {0x456A, 0x94},
    {0x456B, 0x06},
    {0x456D, 0x06},
    {0x456E, 0x49},
    {0x456F, 0x07},
    {0x4572, 0xA5},
    {0x460C, 0x7D},
    {0x460E, 0xB1},
    {0x4614, 0xA8},
    {0x4616, 0xB2},
    {0x461C, 0x7E},
    {0x461E, 0xA7},
    {0x4624, 0xA8},
    {0x4626, 0xB2},
    {0x462C, 0x7E},
    {0x462E, 0x8A},
    {0x4630, 0x94},
    {0x4632, 0xA7},
    {0x4634, 0xFB},
    {0x4636, 0x2F},
    {0x4638, 0x81},
    {0x4639, 0x01},
    {0x463A, 0xB5},
    {0x463B, 0x01},
    {0x463C, 0x26},
    {0x463E, 0x30},
    {0x4640, 0xAC},
    {0x4641, 0x01},
    {0x4642, 0xB6},
    {0x4643, 0x01},
    {0x4644, 0xFC},
    {0x4646, 0x25},
    {0x4648, 0x82},
    {0x4649, 0x01},
    {0x464A, 0xAB},
    {0x464B, 0x01},
    {0x464C, 0x26},
    {0x464E, 0x30},
    {0x4654, 0xFC},
    {0x4656, 0x08},
    {0x4658, 0x12},
    {0x465A, 0x25},
    {0x4662, 0xFC},
    {0x46A2, 0xFB},
    {0x46D6, 0xF3},
    {0x46E6, 0x00},
    {0x46E8, 0xFF},
    {0x46E9, 0x03},
    {0x46EC, 0x7A},
    {0x46EE, 0xE5},
    {0x46F4, 0xEE},
    {0x46F6, 0xF2},
    {0x470C, 0xFF},
    {0x470D, 0x03},
    {0x470E, 0x00},
    {0x4714, 0xE0},
    {0x4716, 0xE4},
    {0x471E, 0xED},
    {0x472E, 0x00},
    {0x4730, 0xFF},
    {0x4731, 0x03},
    {0x4734, 0x7B},
    {0x4736, 0xDF},
    {0x4754, 0x7D},
    {0x4756, 0x8B},
    {0x4758, 0x93},
    {0x475A, 0xB1},
    {0x475C, 0xFB},
    {0x475E, 0x09},
    {0x4760, 0x11},
    {0x4762, 0x2F},
    {0x4766, 0xCC},
    {0x4776, 0xCB},
    {0x477E, 0x4A},
    {0x478E, 0x49},
    {0x4794, 0x7C},
    {0x4796, 0x8F},
    {0x4798, 0xB3},
    {0x4799, 0x00},
    {0x479A, 0xCC},
    {0x479C, 0xC1},
    {0x479E, 0xCB},
    {0x47A4, 0x7D},
    {0x47A6, 0x8E},
    {0x47A8, 0xB4},
    {0x47A9, 0x00},
    {0x47AA, 0xC0},
    {0x47AC, 0xFA},
    {0x47AE, 0x0D},
    {0x47B0, 0x31},
    {0x47B1, 0x01},
    {0x47B2, 0x4A},
    {0x47B3, 0x01},
    {0x47B4, 0x3F},
    {0x47B6, 0x49},
    {0x47BC, 0xFB},
    {0x47BE, 0x0C},
    {0x47C0, 0x32},
    {0x47C1, 0x01},
    {0x47C2, 0x3E},
    {0x47C3, 0x01},
    {0x3002, 0x00},    //Master mode start
    {0x3000, 0x00},
};

/* const static I2C_ARRAY Sensor_id_table[] = {
    {0x3F12, 0x00},      // {address of ID, ID },
    {0x3F13, 0x00},      // {address of ID, ID },
}; */

static I2C_ARRAY PatternTbl[] = {
    {0x0000,0x00},       // colorbar pattern , bit 0 to enable
};

static I2C_ARRAY expo_reg[] = {      // SHS0 (For Linear)
    {0x3052, 0x00},
    {0x3051, 0x00},
    {0x3050, 0x06},
};

static I2C_ARRAY expo_SHR0_reg[] = { // SHS0 (For LEF)
    {0x3052, 0x00},
    {0x3051, 0x16},
    {0x3050, 0x22},
};

static I2C_ARRAY expo_shr_dol1_reg[] = { // SHS1 (For SEF)
    //decreasing exposure ratio version.
    {0x3056, 0x00},
    {0x3055, 0x00},
    {0x3054, 0x0E},
};

static I2C_ARRAY vts_reg[] = {       //VMAX
    {0x302A, 0x00},
    {0x3029, 0x08},
    {0x3028, 0xCA},

};

static I2C_ARRAY vts_reg_hdr[] = {
    {0x302A, 0x00}, //bit0-3-->MSB
    {0x3029, 0x11},
    {0x3028, 0x94},
};

I2C_ARRAY expo_RHS1_reg[] = {
    //decreasing exposure ratio version.
    {0x3062, 0x00},
    {0x3061, 0x00},
    {0x3060, 0x11},
};

static I2C_ARRAY gain_reg[] = {
    {0x3070, 0x07},// LSB
    {0x3071, 0x00},// MSB
};

static I2C_ARRAY gain_HDR_DOL_LEF_reg[] = {
    {0x3070, 0x07},// LSB
    {0x3071, 0x00},// MSB
};

static I2C_ARRAY gain_Clr_HDR_AGH_reg[] = {
    {0x309C, 0x00},// LSB
    {0x309D, 0x00},// MSB
};

static I2C_ARRAY gain_Clr_HDR_AGL_reg[] = {
    {0x3094, 0x00},// LSB
    {0x3095, 0x00},// MSB
};

static I2C_ARRAY gain_Clr_HDR_DG_reg[] = {
    {0x308C, 0x00},// LSB
    {0x308D, 0x01},// MSB
};

static I2C_ARRAY gain_HDR_DOL_SEF_reg[] = {
    {0x3072, 0x00},// LSB
    {0x3073, 0x00},// MSB
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

#define SENSOR_SLEEP_MODE_ENABLE      0
#define SENSOR_SLEEP_MODE_DISABLE     1

u8 g_u8CurSleepMode = 255;
static int _pCus_SleepMode(ss_cus_sensor* handle, u32 sleepModeEnable ,u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx678_params *params = (imx678_params *)handle->private_data;

    if(sleep_mode == 1)
    {
        switch(sleepModeEnable)
        {
            case SENSOR_SLEEP_MODE_ENABLE:
                //enable sleep mode
                SensorReg_Write(0x3000, 0x01);
                SensorReg_Write(0x3002, 0x01);
                params->stUserDefInfo.bSnrSleep = true;
                break;
            case SENSOR_SLEEP_MODE_DISABLE:
                //disable sleep mode
                SensorReg_Write(0x3002, 0x00);
                SensorReg_Write(0x3000, 0x00);
                params->stUserDefInfo.bSnrSleep = false;
                break;
            default:
                SENSOR_EMSG("do nothing ,Sleep Mode [1] only On/Off !!!\n");
                break;
        }
    }
    else
    {
         SENSOR_EMSG("sleep_mode %d not avalibale\n", sleep_mode);
    }
    sensor_if->SetUserDefInfo(idx, params->stUserDefInfo);

    return 0;
}

/* static int IMX678_GetShutterInfo_HDR_DOL_SEF(ss_cus_sensor *handle, u32 *min, u32 *max, u32 *step)
{
    imx678_params *params = (imx678_params *)handle->private_data;

    if (params->Clrhdr_mode_en == 0)
    {
        *max = Preview_line_period_HDR_DOL_4LANE * params->max_rhs1;
        *min = (Preview_line_period_HDR_DOL_4LANE * 5);
        *step = Preview_line_period_HDR_DOL_4LANE * 2;
    }
    else
    {
        *max = 1000000000/imx678_mipi_hdr[2].senout.min_fps;
        *min = (7407 * 5);
        *step = 7407 * 2;
    }
    printk("###########################max = %d;min = %d;step = %d\n", *max,*min,*step);
    return SUCCESS;
} */

static int IMX678_GetShutterInfo_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *min, u32 *max, u32 *step)
{
    imx678_params *params = (imx678_params *)handle->private_data;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    if (params->Clrhdr_mode_en == 0)
    {
        *max = 1000000000/imx678_mipi_hdr[0].senout.min_fps;
        *min = (Preview_line_period_HDR_DOL_4LANE * 5);
        *step = Preview_line_period_HDR_DOL_4LANE * 2;
    }
    else
    {
        *max = 1000000000/min_fps;
        *min = (7407 * 5);
        *step = 7407 * 2;
    }
    return SUCCESS;
}
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    //imx678_params *params = (imx678_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    //printk("handle->interface_attr.attr_mipi.mipi_hdr_mode = %d\nhandle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num=%d\n", handle->interface_attr.attr_mipi.mipi_hdr_mode,handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num);
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode==CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
/*         if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            IMX678_GetShutterInfo_HDR_DOL_SEF(handle, &(handle->sensor_ae_info_cfg.u32AEShutter_min),
                                                      &(handle->sensor_ae_info_cfg.u32AEShutter_max),
                                                      &(handle->sensor_ae_info_cfg.u32AEShutter_step));
        }
        else
        { */
            IMX678_GetShutterInfo_HDR_DOL_LEF(handle, &(handle->sensor_ae_info_cfg.u32AEShutter_min),
                                                      &(handle->sensor_ae_info_cfg.u32AEShutter_max),
                                                      &(handle->sensor_ae_info_cfg.u32AEShutter_step));
        info->u32AEShutter_min                   = handle->sensor_ae_info_cfg.u32AEShutter_min;
        info->u32AEShutter_step                  = handle->sensor_ae_info_cfg.u32AEShutter_step;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        //}
    }
    //printk("min=%d;max=%d;step=%d",handle->sensor_ae_info_cfg.u32AEShutter_min,handle->sensor_ae_info_cfg.u32AEShutter_max,handle->sensor_ae_info_cfg.u32AEShutter_step);

    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx678_params *params = (imx678_params *)handle->private_data;

    memcpy(params->tVts_reg_hdr, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tExpo_rhs1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tExpo_shr_dol1_reg, expo_shr_dol1_reg, sizeof(expo_shr_dol1_reg));
    memcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));

    //Sensor power on sequence
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);   // Powerdn Pull Low
    sensor_if->Reset(idx, params->reset_POLARITY);     // Rst Pull Low
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_288M);
    //printk("-----------------------172M--------------------------\n");
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, ENABLE);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_DCG) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    //Sensor board PWDN Enable, 1.8V & 2.9V need 30ms then Pull High
    SENSOR_MSLEEP(40);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    SENSOR_UDELAY(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_UDELAY(20);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

u8 bAEStarted = false;
u8 g_u8SuspendFlag = 0;
static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx678_params *params = (imx678_params *)handle->private_data;

    memcpy(vts_reg_hdr, params->tVts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(expo_SHR0_reg, params->tSHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(expo_shr_dol1_reg, params->tExpo_shr_dol1_reg, sizeof(expo_shr_dol1_reg));
    memcpy(expo_RHS1_reg, params->tExpo_rhs1_reg, sizeof(expo_RHS1_reg));
    memcpy(gain_HDR_DOL_LEF_reg, params->tGain_hdr_dol_lef_reg, sizeof(gain_HDR_DOL_LEF_reg));
    memcpy(gain_HDR_DOL_SEF_reg, params->tGain_hdr_dol_sef_reg, sizeof(gain_HDR_DOL_SEF_reg));
    memcpy(vts_reg, params->tVts_reg, sizeof(vts_reg));
    memcpy(gain_reg, params->tGain_reg, sizeof(gain_reg));
    memcpy(expo_reg, params->tExpo_reg, sizeof(expo_reg));

    if (g_u8SuspendFlag == 0)
    {
        SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
        sensor_if->PowerOff(idx, params->pwdn_POLARITY);   // Powerdn Pull Low
        sensor_if->Reset(idx, params->reset_POLARITY);     // Rst Pull Low
        sensor_if->MCLK(idx, 0, handle->mclk);

        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
        if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_DCG) {
            sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
        }
    }
    g_u8SuspendFlag = 1;

    params->res.orit = SENSOR_ORIT;
    bAEStarted = false;

    return SUCCESS;
}


static int pCus_Suspend(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI  *sensor_if = handle->sensor_if_api;

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_ENABLE, idx);
    sensor_if->MCLK(idx, 0, handle->mclk); // handle->mclk);

    g_u8SuspendFlag = 1;

    SENSOR_EMSG("[%s] sensor suspend!\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_Resume(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI  *sensor_if = handle->sensor_if_api;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_288M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    sensor_if->MCLK(idx, 1, handle->mclk); //handle->mclk);

    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_DISABLE, idx);
    CamOsMsDelay(1);//or ae notify i2c err
    pCus_AEStatusNotify(handle, 0, CUS_FRAME_ACTIVE);

    g_u8SuspendFlag = 0;

    SENSOR_EMSG("[%s] sensor resume!\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_Reopen(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI  *sensor_if = handle->sensor_if_api;
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_288M);
    sensor_if->MCLK(idx, 1, handle->mclk); //handle->mclk);

    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_DISABLE, idx);

    g_u8SuspendFlag = 0;

    SENSOR_EMSG("[%s] sensor reopen!\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_StreamOn(struct __ss_cus_sensor *handle, u32 idx)
{
/*     I2C_ARRAY stream_onoff_reg = {0x0100, 0x00};

    stream_onoff_reg.data= 0x01;//disable sleep mode */
/*     SensorReg_Write(stream_onoff_reg.reg, stream_onoff_reg.data);
 */
    SensorReg_Write(0x3000, 0x00);
    SensorReg_Write(0x3002, 0x00);

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

static int imx678_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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
    int i,cnt=0;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_8m_30fps_init_table_4lane_linear);i++)
    {
        if(Sensor_8m_30fps_init_table_4lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_8m_30fps_init_table_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_8m_30fps_init_table_4lane_linear[i].reg, Sensor_8m_30fps_init_table_4lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
            //printk("\n reg 0x%x, 0x%x",Sensor_8m_30fps_init_table_4lane_linear[i].reg, Sensor_8m_30fps_init_table_4lane_linear[i].data);
#if 0
            SensorReg_Read(Sensor_8m_30fps_init_table_4lane_linear[i].reg, &sen_data );
            if(Sensor_8m_30fps_init_table_4lane_linear[i].data != sen_data)
                printk("R/W Differ Reg: 0x%x\n",Sensor_8m_30fps_init_table_4lane_linear[i].reg);
                //printk("IMX678 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_8m_30fps_init_table_4lane_linear[i].reg, Sensor_8m_30fps_init_table_4lane_linear[i].data, sen_data);
#endif
        }
    }

    return SUCCESS;
}

static int pCus_init_8m_15fps_mipi4lane_linear(ss_cus_sensor *handle)
{
    int i,cnt=0;
    imx678_params *params = (imx678_params *)handle->private_data;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_8m_15fps_init_table_4lane_linear);i++)
    {
        if(Sensor_8m_15fps_init_table_4lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_8m_15fps_init_table_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_8m_15fps_init_table_4lane_linear[i].reg, Sensor_8m_15fps_init_table_4lane_linear[i].data) != SUCCESS)
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
    if(params)
    {
        SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
        SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
    }
    return SUCCESS;
}

static int pCus_init_8m_30fps_mipi4lane_linear(ss_cus_sensor *handle)
{
    int i,cnt=0;
    imx678_params *params = (imx678_params *)handle->private_data;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_8m_30fps_init_table_4lane_linear);i++)
    {
        if(Sensor_8m_30fps_init_table_4lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_8m_30fps_init_table_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_8m_30fps_init_table_4lane_linear[i].reg, Sensor_8m_30fps_init_table_4lane_linear[i].data) != SUCCESS)
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
    if(params)
    {
        SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
        SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
    }
    return SUCCESS;
}

static int pCus_init_mipi4lane_HDR_DOL(ss_cus_sensor *handle)
{
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

        }
    }

    SENSOR_DMSG("Sensor IMX678 HDR MODE Initial Finished\n");
    return SUCCESS;
}

static int pCus_init_mipi4lane_Clear_HDR_VC(ss_cus_sensor *handle)
{
    int i,cnt=0;
    //s16 sen_data;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_4lane_Clear_HDR_VC);i++)
    {
        if(Sensor_init_table_4lane_Clear_HDR_VC[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_Clear_HDR_VC[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_Clear_HDR_VC[i].reg,Sensor_init_table_4lane_Clear_HDR_VC[i].data) != SUCCESS)
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

    SENSOR_DMSG("Sensor IMX678 HDR MODE Initial Finished\n");
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
    imx678_params *params = (imx678_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_8m_15fps_mipi4lane_linear;
            vts_30fps = 3750;//2250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 15;
            Preview_line_period = 17777;   // 14814 = 1/30/2250*1000000
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 1:
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_8m_30fps_mipi4lane_linear;
            vts_30fps = 2250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 14814;   // 14814 = 1/30/2250*1000000
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        default:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_8m_15fps_mipi4lane_linear;
            vts_30fps = 3750;//2250;
            params->expo.vts = vts_30fps;
            params->expo.fps = 15;
            Preview_line_period = 17777;   // 14814 = 1/30/2250*1000000
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
    }

    params->tVts_reg[0].data = (vts_30fps >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts_30fps >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts_30fps >> 0) & 0x00ff;
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int IMX678_GetAEMinMaxGain_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    imx678_params *params = (imx678_params *)handle->private_data;

    *min = SENSOR_MIN_GAIN;
    if (params->Clrhdr_mode_en == 0)
    {
        *max = SENSOR_MAX_GAIN;
    }
    else
    {
        *max = SENSOR_CLR_HDR_MAX_GAIN;
    }
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
        case HDR_RES_2:
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        default:
            break;
    }
    IMX678_GetAEMinMaxGain_HDR_DOL_LEF(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                               &(handle->sensor_ae_info_cfg.u32AEGain_max));
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}
static int IMX678_GetAEMinMaxGain_HDR_DOL_SEF(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    imx678_params *params = (imx678_params *)handle->private_data;

    *min = SENSOR_MIN_GAIN;
    if (params->Clrhdr_mode_en == 0)
    {
        *max = SENSOR_MAX_GAIN;
    }
    else
    {
        *max = SENSOR_CLR_HDR_MAX_AGAIN_L;
    }
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_SEF(ss_cus_sensor *handle, u32 res_idx)
{
    imx678_params *params = (imx678_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {

        case HDR_RES_2:
            handle->pCus_sensor_init = pCus_init_mipi4lane_HDR_DOL;
            vts_30fps_HDR_DOL_4lane = 2250;
            params->expo.vts = vts_30fps_HDR_DOL_4lane;
            params->expo.fps = 30;
            Preview_line_period_HDR_DOL_4LANE = 14814 / 2;
            handle->data_prec = CUS_DATAPRECISION_10;
            params->max_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            params->Clrhdr_mode_en = 0;
            break;
        case HDR_RES_1:
            handle->pCus_sensor_init = pCus_init_mipi4lane_Clear_HDR_VC;
            vts_30fps_HDR_DOL_4lane = 4500;
            params->expo.vts = vts_30fps_HDR_DOL_4lane;
            params->expo.fps = 30;
            Preview_line_period_HDR_DOL_4LANE = 7407 / 2;
            handle->data_prec = CUS_DATAPRECISION_10;
            params->max_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429
            handle->sensor_ae_info_cfg.u8AEGainDelay = SENSOR_GAIN_DELAY_FRAME_COUNT_CLR_HDR;
            params->Clrhdr_mode_en = 1;
            break;

        default:

            break;
    }
    IMX678_GetAEMinMaxGain_HDR_DOL_SEF(handle, &(handle->sensor_ae_info_cfg.u32AEGain_min),
                                               &(handle->sensor_ae_info_cfg.u32AEGain_max));
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);

    params->tVts_reg_hdr[0].data = (vts_30fps_HDR_DOL_4lane >> 16) & 0x000f;
    params->tVts_reg_hdr[1].data = (vts_30fps_HDR_DOL_4lane >> 8) & 0x00ff;
    params->tVts_reg_hdr[2].data = (vts_30fps_HDR_DOL_4lane >> 0) & 0x00ff;
    memcpy(vts_reg_hdr, params->tVts_reg_hdr, sizeof(vts_reg_hdr));

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    s16 sen_data_mirror;
    s16 sen_data_flip;

    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR, (void*)&sen_data_mirror);
    SensorReg_Read(FLIP, (void*)&sen_data_flip);

    switch(sen_data_mirror & (sen_data_flip << 1))
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
    imx678_params *params = (imx678_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx678_params *params = (imx678_params *)handle->private_data;
    s16 sen_data_mirror,sen_data_flip;
	return 0;
    switch(orit)
    {
        case CUS_ORIT_M0F0:
            sen_data_mirror = 0;
            sen_data_flip   = 0;
            params->res.orit = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M1F0:
            sen_data_mirror = 1;
            sen_data_flip   = 0;
            params->res.orit = CUS_ORIT_M1F0;
            break;
        case CUS_ORIT_M0F1:
            sen_data_mirror = 0;
            sen_data_flip    = 1;
            params->res.orit = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            sen_data_mirror  = 1;
            sen_data_flip    = 1;
            params->res.orit = CUS_ORIT_M1F1;
            break;
        default :
            sen_data_mirror = 0;
            sen_data_flip   = 0;
            params->res.orit = CUS_ORIT_M0F0;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(MIRROR, sen_data_mirror);
    SensorReg_Write(FLIP, sen_data_flip);
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx678_params *params = (imx678_params *)handle->private_data;
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
    imx678_params *params = (imx678_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

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

    if(bAEStarted == true)
        pCus_SetAEUSecs(handle, params->expo.expo_lef_us);

    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_SEF(ss_cus_sensor *handle)
{
    imx678_params *params = (imx678_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg_hdr[0].data << 16) | (params->tVts_reg_hdr[1].data << 8) | (params->tVts_reg_hdr[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_SEF(ss_cus_sensor *handle, u32 fps)
{
    u32 cur_vts_30fps = 0;
    imx678_params *params = (imx678_params *)handle->private_data;
       u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    //cur_vts_30fps = imx678_mipi_hdr[0].senout.height;
    cur_vts_30fps=vts_30fps_HDR_DOL_4lane;
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
    imx678_params *params = (imx678_params *)handle->private_data;
    //ISensorIfAPI2 *sensor_if1 = handle->sensor_if_api2;
    //return 0;
    if(params->stUserDefInfo.bSnrSleep == true)//isp resume set ae i2c err, wait sensor resume power on do aenotify
    {
        SENSOR_EMSG("[%s]:%d sensor sleep enable, no I2C write \n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SENSOR_DMSG("[%s] tExpo_reg %x %x %x  params->dirty %d\n",__FUNCTION__,params->tExpo_reg[0].data,params->tExpo_reg[1].data,params->tExpo_reg[2].data, params->dirty);
                SENSOR_DMSG("[%s] tGain_reg %x %x %x \n",__FUNCTION__,params->tGain_reg[0].data,params->tGain_reg[1].data);
                SENSOR_DMSG("[%s] tVts_reg %x %x %x \n",__FUNCTION__,params->tVts_reg[0].data,params->tVts_reg[1].data,params->tVts_reg[2].data);
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

    return SUCCESS;

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
    imx678_params *params = (imx678_params *)handle->private_data;

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
    imx678_params *params = (imx678_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    //    bAEStarted = true;
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

/*     SENSOR_DMSG("[%s] us %u, SHR0 %u, vts %u\n", __FUNCTION__,
                                                 us,  \
                                                 SHR0, \
                                                 vts
               ); */
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
    u32 cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    //u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    imx678_params *params = (imx678_params *)handle->private_data;

    cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    //cur_vts_30fps =vts_30fps_HDR_DOL_4lane;
    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    params->expo.expo_sef_us = us;
    //params->max_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429

    params->min_shr1 = params->max_rhs1 - expo_line_sef;
    if(params->min_shr1 > params->max_rhs1 - 2)
        params->min_shr1 = params->max_rhs1 - 2;
    else if(params->min_shr1 <  5)
        params->min_shr1 = 5;
    params->min_shr1 = ((params->min_shr1 >> 1) << 1) + 1;


    SENSOR_DMSG("[%s] us %u, expo_line_sef %u rhs %u shr1 %u cur_line_period %u\n", __FUNCTION__,
                                                                 us, \
                                                                 expo_line_sef, \
                                                                 params->max_rhs1, \
                                                                 params->min_shr1, cur_line_period
               );


    params->tExpo_shr_dol1_reg[0].data = (params->min_shr1 >> 16) & 0x000f;
    params->tExpo_shr_dol1_reg[1].data = (params->min_shr1 >> 8) & 0x00ff;
    params->tExpo_shr_dol1_reg[2].data = (params->min_shr1 >> 0) & 0x00ff;
    params->tExpo_rhs1_reg[0].data = (params->max_rhs1 >>16) & 0x0f;
    params->tExpo_rhs1_reg[1].data = (params->max_rhs1 >>8) & 0xff;
    params->tExpo_rhs1_reg[2].data = (params->max_rhs1 >>0) & 0xff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx678_params *params = (imx678_params *)handle->private_data;
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
    imx678_params *params = (imx678_params *)handle->private_data;
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

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data, gain,params->tGain_reg[1].data);
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
    imx678_params *params = (imx678_params *)handle->private_data;
    u16 gain_reg = 0;
    u64 Again_tmp=0;

    if (params->Clrhdr_mode_en == 0) // Nornal
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        params->tGain_hdr_dol_sef_reg[0].data = gain_reg & 0xff;
        params->tGain_hdr_dol_sef_reg[1].data = (gain_reg >> 8) & 0xff;
    }
    else // Formula (2048x(gainx10717/10000)-2048)/(gainx10717/10000)
    {
        if(gain < SENSOR_MIN_GAIN)
            gain = 1024;
        else if(gain >= SENSOR_CLR_HDR_MAX_AGAIN_L)
            gain = SENSOR_CLR_HDR_MAX_AGAIN_L;

        //gain_reg = ((2048*(gain*10717/10000)/1024)-2048)/(gain*10717/10000/1024);
        Again_tmp  = ((2048*(gain*10717/10000)/1024)-2048);
        gain_reg = (Again_tmp*9550)/(gain*10);
        CamOsPrintf(KERN_DEBUG "[DBG] AgainL_reg %lx , gain %d (%d) \n", gain_reg, gain, (2048*(gain*10717/10000)/1024)-2048);
        params->tGain_clr_hdr_AGL_reg[0].data = gain_reg & 0xff;
        params->tGain_clr_hdr_AGL_reg[1].data = (gain_reg >> 8) & 0xff;
    }

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
    imx678_params *params = (imx678_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_LEF(ss_cus_sensor *handle)
{
    imx678_params *params = (imx678_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg_hdr[0].data << 16) | (params->tVts_reg_hdr[1].data << 8) | (params->tVts_reg_hdr[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_LEF(ss_cus_sensor *handle, u32 fps)
{
    u32 cur_vts_30fps = 0;//vts = 0,
    imx678_params *params = (imx678_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
   // cur_vts_30fps = imx678_mipi_hdr[0].senout.height;
    cur_vts_30fps=vts_30fps_HDR_DOL_4lane;
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
    pCus_SetAEUSecsHDR_DOL_LEF(handle, params->expo.expo_lef_us);

    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    #if 1
    imx678_params *params = (imx678_params *)handle->private_data;

    //return 0;
    if(params->stUserDefInfo.bSnrSleep == true)//isp resume set ae i2c err, wait sensor resume power on do aenotify
    {
        SENSOR_EMSG("[%s]:%d sensor sleep enable, no I2C write \n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }

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
                        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg_hdr, ARRAY_SIZE(vts_reg_hdr));
                        SensorRegArrayW((I2C_ARRAY*)params->tExpo_shr_dol1_reg, ARRAY_SIZE(expo_shr_dol1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tExpo_rhs1_reg, ARRAY_SIZE(expo_RHS1_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                    }
                    else
                    {
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_clr_hdr_AGH_reg, ARRAY_SIZE(gain_Clr_HDR_AGH_reg));
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n", params->tGain_clr_hdr_AGH_reg[0].reg, params->tGain_clr_hdr_AGH_reg[0].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n", params->tGain_clr_hdr_AGH_reg[1].reg, params->tGain_clr_hdr_AGH_reg[1].data);
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_clr_hdr_AGL_reg, ARRAY_SIZE(gain_Clr_HDR_AGL_reg));
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tGain_clr_hdr_AGL_reg[0].reg, params->tGain_clr_hdr_AGL_reg[0].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tGain_clr_hdr_AGL_reg[1].reg, params->tGain_clr_hdr_AGL_reg[1].data);
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_clr_hdr_DG_reg, ARRAY_SIZE(gain_Clr_HDR_DG_reg));
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tGain_clr_hdr_DG_reg[0].reg, params->tGain_clr_hdr_DG_reg[0].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tGain_clr_hdr_DG_reg[1].reg, params->tGain_clr_hdr_DG_reg[1].data);
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg_hdr, ARRAY_SIZE(vts_reg_hdr));
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tSHR0_reg[0].reg, params->tSHR0_reg[0].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tSHR0_reg[1].reg, params->tSHR0_reg[1].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tSHR0_reg[2].reg, params->tSHR0_reg[2].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tVts_reg_hdr[0].reg, params->tVts_reg_hdr[0].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tVts_reg_hdr[1].reg, params->tVts_reg_hdr[1].data);
                        //CamOsPrintf(KERN_DEBUG" reg: 0x%x -> 0x%x\n",params->tVts_reg_hdr[2].reg, params->tVts_reg_hdr[2].data);
                    }

                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x3001,0);
            }
            break;
        default :
             break;
    }
    #endif
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
    u32 cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    //u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    imx678_params *params = (imx678_params *)handle->private_data;

    //printk("us=%d;mode=%d;\n",us,params->Clrhdr_mode_en);
    if (params->Clrhdr_mode_en == 0)
    {
        cur_line_period = Preview_line_period_HDR_DOL_4LANE;
        //cur_vts_30fps =vts_30fps_HDR_DOL_4lane;

        expo_lines_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;

        params->expo.expo_lef_us = us;
        fsc = params->expo.vts * 2;
        params->fsc = ((fsc >> 1) << 1)+ 2;                  // 2n

        //params->max_shr0 = fsc - expo_lines_lef - 8;
        params->max_shr0 = (fsc - 2) - expo_lines_lef;
        if(params->max_shr0 < 0 )
        {
            params->max_shr0 = fsc - 2;
        }
        if(params->max_shr0 < (params->max_rhs1 + 5))
            params->max_shr0 = params->max_rhs1 + 5;
        params->max_shr0 = ((params->max_shr0 >> 1) << 1);

        if (expo_lines_lef > (fsc - params->max_rhs1 - 5)) {
            vts = (expo_lines_lef + params->max_rhs1 + 5);
        }
        else{
          vts = params->expo.vts;
        }

        params->expo.expo_lines = expo_lines_lef;
        //params->expo.vts = vts;

        SENSOR_EMSG("[%s]3 us %u, expo_lines_lef %u, params->expo.vts %u vts %u, SHR0 %u rhs1 %u, params->fsc %u fsc %u \n", __FUNCTION__,
                                                                         us, \
                                                                         expo_lines_lef, \
                                                                         params->expo.vts, vts, \
                                                                         params->max_shr0, params->max_rhs1, \
                                                                         params->fsc, fsc
                    );
        params->tSHR0_reg[0].data = (params->max_shr0 >> 16) & 0x000f;
        params->tSHR0_reg[1].data = (params->max_shr0 >> 8) & 0x00ff;
        params->tSHR0_reg[2].data = (params->max_shr0 >> 0) & 0x00ff;

        params->tVts_reg_hdr[0].data = (vts >> 16) & 0x000f;
        params->tVts_reg_hdr[1].data = (vts >> 8) & 0x00ff;
        params->tVts_reg_hdr[2].data = (vts >> 0) & 0x00ff;
    }
    else
    {
        u32 expo_lines = 0, SHR0 = 0;

        params->expo.expo_lef_us = us;
        expo_lines = (1000*us)/Preview_line_period_HDR_DOL_4LANE;

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

        params->tSHR0_reg[0].data = (SHR0 >> 16) & 0x000f;
        params->tSHR0_reg[1].data = (SHR0 >> 8) & 0x00ff;
        params->tSHR0_reg[2].data = (SHR0 >> 0) & 0x00ff;

        params->tVts_reg_hdr[0].data = (vts >> 16) & 0x000f;
        params->tVts_reg_hdr[1].data = (vts >> 8) & 0x00ff;
        params->tVts_reg_hdr[2].data = (vts >> 0) & 0x00ff;
    }
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
    imx678_params *params = (imx678_params *)handle->private_data;
    u16 Again_reg = 0;
    u64 Again_tmp=0;
    u16 Dgain_reg = 0x100;
    if (params->Clrhdr_mode_en == 0)
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &Again_reg);
        params->tGain_hdr_dol_lef_reg[0].data = Again_reg & 0xff;
        params->tGain_hdr_dol_lef_reg[1].data = (Again_reg >> 8) & 0xff;

        SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data, params->tGain_hdr_dol_lef_reg[1].data);
    }
    else // Formula (2048x(gainx10717/10000)-2048)/(gainx10717/10000)
    {
        //H GAin
        if(gain <= SENSOR_MIN_GAIN)
        {
            Again_reg = 1024;
            Dgain_reg = 0x100;
        }

        if(gain >= SENSOR_CLR_HDR_MAX_AGAIN_H && gain <= SENSOR_CLR_HDR_MAX_GAIN)
        {
            Dgain_reg = gain * 0x100 / SENSOR_CLR_HDR_MAX_AGAIN_H;
            gain = SENSOR_CLR_HDR_MAX_AGAIN_H;
        }
        else if(gain > SENSOR_CLR_HDR_MAX_GAIN){
            Dgain_reg = SENSOR_CLR_HDR_MAX_GAIN *100 / SENSOR_CLR_HDR_MAX_AGAIN_H;
            gain = SENSOR_CLR_HDR_MAX_AGAIN_H;
        }

        //Again_reg = ((2048*(gain*10717/10000)/1024)-2048)/(gain*10717/10000/1024);
        Again_tmp = ((2048*(gain*10717/10000)/1024)-2048);
        //Again_reg = (Again_tmp*10000*1024)/(gain*10717);
        Again_reg = (Again_tmp*9550)/(gain*10);
        //CamOsPrintf(KERN_DEBUG "[DBG] Again_reg 0x%lx , gain %d (%d) \n", Again_reg, gain, (2048*(gain*10717/10000)/1024)-2048);
        //CamOsPrintf(KERN_DEBUG "[DBG] Dgain_reg 0x%lx , gain %d \n", Dgain_reg, gain);

        params->tGain_clr_hdr_AGH_reg[0].data = Again_reg & 0xff;
        params->tGain_clr_hdr_AGH_reg[1].data = (Again_reg >> 8) & 0xff;
        params->tGain_clr_hdr_DG_reg[0].data = Dgain_reg & 0xff; //Decimal
        params->tGain_clr_hdr_DG_reg[1].data = (Dgain_reg >> 8) & 0xff; //integer
    }

    params->dirty = true;
    return SUCCESS;
}

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx678_params *params;
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

    params = (imx678_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX678_MIPI");

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
    handle->pCus_sensor_suspend                                   = pCus_Suspend;
    handle->pCus_sensor_resume                                    = pCus_Resume;
    handle->pCus_sensor_reopen                                    = pCus_Reopen;
    handle->pCus_sensor_streamon                                  = pCus_StreamOn;
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
        handle->video_res_supported.res[res].u16width             = imx678_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx678_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx678_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx678_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx678_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx678_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx678_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx678_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx678_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi4lane_linear;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = imx678_SetPatternMode;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    Preview_line_period = 14814;
    Preview_line_period_HDR_DOL_4LANE = 14814;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx678_mipi_linear[0].senout.min_fps;
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
    imx678_params *params = NULL;
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

    params = (imx678_params *)handle->private_data;
    memcpy(params->tVts_reg_hdr, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tExpo_rhs1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tExpo_shr_dol1_reg, expo_shr_dol1_reg, sizeof(expo_shr_dol1_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));
    // Clear HDR
    memcpy(params->tGain_clr_hdr_AGH_reg, gain_Clr_HDR_AGH_reg, sizeof(gain_Clr_HDR_AGH_reg));
    memcpy(params->tGain_clr_hdr_AGL_reg, gain_Clr_HDR_AGL_reg, sizeof(gain_Clr_HDR_AGL_reg));
    memcpy(params->tGain_clr_hdr_DG_reg, gain_Clr_HDR_DG_reg, sizeof(gain_Clr_HDR_DG_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX678_MIPI_HDR_SEF");

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
    handle->pCus_sensor_suspend                                   = pCus_Suspend;
    handle->pCus_sensor_resume                                    = pCus_Resume;
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
        handle->video_res_supported.res[res].u16width             = imx678_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx678_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx678_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx678_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx678_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx678_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx678_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx678_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx678_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi4lane_HDR_DOL;
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
    params->max_rhs1 = 437;
    Preview_line_period_HDR_DOL_4LANE = 14814;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_4LANE * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR_DOL_4LANE * params->max_rhs1;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE * 2;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps_HDR_DOL_4lane * 2;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx678_params *params;
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
    params = (imx678_params *)handle->private_data;
    memcpy(params->tVts_reg_hdr, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX678_MIPI_HDR_LEF");

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
        handle->video_res_supported.res[res].u16width             = imx678_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx678_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx678_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx678_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx678_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx678_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx678_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx678_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx678_mipi_hdr[res].senstr.strResDesc);
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
    Preview_line_period_HDR_DOL_4LANE = 14814;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 2;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_4LANE * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx678_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE * 2;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps_HDR_DOL_4lane * 2;
    params->expo.expo_lines                                       = 673;
    params->dirty                                                 = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(IMX678_HDR,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            imx678_params
                         );
