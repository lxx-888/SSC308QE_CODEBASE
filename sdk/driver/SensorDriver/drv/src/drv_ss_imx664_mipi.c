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

// SYSDESC
//#include "drv_sysdesc.h"
//#include "device_id.h"
//#include "property_id.h"

//#include "mdrv_gpio_io.h"
#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX664);
/*
extern s32 DRV_SENSOR_IF_Release(u32 nSNRPadID);
extern s32 DRV_SENSOR_IF_SensorIFVer(u32 version_major, u32 version_minor);
extern s32 DRV_SENSOR_IF_RegisterSensorDriverEx(u32 nSNRPadID, SensorInitHandle pfnSensorInitHandle, void *pPrivateData);
extern s32 DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(u32 nSNRPadID, u32 nPlaneID, u32 Slaveid);
extern s32 DRV_SENSOR_IF_SensorHandleVer(u32 version_major, u32 version_minor);
extern s32 DRV_SENSOR_IF_RegisterPlaneDriverEx(u32 nSNRPadID, u32 nPlaneID, SensorInitHandle pfnSensorInitHandle);
extern s32 DRV_SENSOR_IF_RegisterSensorI2CSlaveID(u32 nSNRPadID, u32 Slaveid);
extern s32 DRV_SENSOR_IF_SensorI2CVer(u32 version_major, u32 version_minor);
extern u64 intlog10(u32 value);
*/
extern u64 base2_exp_float_pow(u64 x);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

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
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_24MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x34                //I2C slave address
#define SENSOR_I2C_SPEED             200000              //200000 //300000 //240000                  //I2C speed, 60000~320000
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
#define SENSOR_NAME     IMX664

#define CHIP_ID_r3F12   0x3F12
#define CHIP_ID_r3F13   0x3F13
#define CHIP_ID         0x0664

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
}imx664_mipi_linear[] = {
    {LINEAR_RES_1, {2688, 1520, 3, 30}, {0, 0, 2688, 1520}, {"2688x1520@30fps"}},
};

static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_2, HDR_RES_END}mode;
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
}imx664_mipi_hdr[] = {
    {HDR_RES_1, {2688, 1520, 3, 30}, {0, 0, 2688, 1520}, {"2688x1520@30fps_HDR"}},
    {HDR_RES_2, {2688, 1520, 3, 30}, {0, 0, 2688, 1520}, {"2688x1520@30fps_ClearHDR"}},
};

u32 vts_30fps_linear = 3300;
u32 vts_30fps_DOL_4lane = 1650;           //TBD
u32 Preview_line_period_linear = 10101; 
u32 Preview_line_period_DOL_4LANE = 10101; //TBD

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (3981 * 1024)        // TBD max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_DOL           (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_CLRHDR        (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)
#define SENSOR_GAIN_CTRL_NUM                        (1)
#define SENSOR_SHUTTER_CTRL_NUM                     (1)
#define SENSOR_GAIN_CTRL_NUM_DOL                    (2)
#define SENSOR_SHUTTER_CTRL_NUM_DOL                 (2)

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define SENSOR_FLIP                                 0x3021
#define SENSOR_MIRROR                               0x3020
#define SENSOR_NOR                                  0x0
#define SENSOR_MIRROR_EN                            0x1 << (0)
#define SENSOR_FLIP_EN                              0x1 << (1)
#define SENSOR_MIRROR_FLIP_EN                       0x3

#define SENSOR_HCG_GAIN                             (582 * 1024 / 100) // 15.3db(HCG)
#define SENSOR_CLR_HDR_MAX_GAIN_HG                  (291742 * 1024 / 100) // 54db+15.3db(HCG)=69.3db
#define SENSOR_CLR_HDR_MIN_GAIN_HG                  (1884 * 1024 / 100) // 10.2db+15.3db(HCG)=25.5db
#define SENSOR_CLR_HDR_MAX_GAIN_LG                  (1584 * 1024 / 100) // 24db
#define SENSOR_CLR_HDR_MIN_GAIN_LG                  (1024) // 0db

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
    CUS_CAMSENSOR_ORIT  orit;

    u32 min_shr1;
    u32 min_rhs1;
    u32 min_shr0;
    u32 max_shr0;
    u32 fsc;
    u32 Clrhdr_mode_en;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tVts_reg_hdr[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tSHR0_reg[3];
    I2C_ARRAY tSHR1_reg[3];
    I2C_ARRAY tRHS1_reg[3];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tGain_hdr_dol_lef_reg[2];
    I2C_ARRAY tGain_hdr_dol_sef_reg[2];
    I2C_ARRAY tGain_Clr_hDR_HG_reg[2];
    I2C_ARRAY tGain_Clr_hDR_LG_reg[2];
    bool dirty;
    bool orien_dirty;
} imx664_params;

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

const static I2C_ARRAY Sensor_4m_30fps_init_table_4lane_linear[] =
{
/*
IMX664LQJ All-pixel scan CSI-2_4lane 27MHz AD:12bit Output:12bit 1440Mbps Master Mode LCG Mode 30fps
Integration Time 33.244ms 
*/
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop

    {0x3014, 0x04},
    //{0x3028, 0xE4},
    //{0x3029, 0x0C},
    {0x3028, 0xbc},//Vmax 1980
    {0x3029, 0x07},//Vmax
    {0x302C, 0xe2},//Hmax 884
    {0x302D, 0x04},//Hmax


    //{0x3230, 0x01},
    //{0x3236, 0xB0},


    {0x3050, 0x08},
    {0x30A6, 0x00},
    {0x3148, 0x00},
    {0x3412, 0x01},
    {0x3460, 0x21},
    {0x3492, 0x08},
    {0x3930, 0x01},
    {0x3B1D, 0x17},
    {0x3C00, 0x0C},
    {0x3C01, 0x0C},
    {0x3C04, 0x02},
    {0x3C08, 0x1E},
    {0x3C09, 0x1D},
    {0x3C0A, 0x1D},
    {0x3C0B, 0x1D},
    {0x3C0C, 0x1D},
    {0x3C0D, 0x1D},
    {0x3C0E, 0x1C},
    {0x3C0F, 0x1B},
    {0x3C30, 0x73},
    {0x3C34, 0x03},
    {0x3C3C, 0x20},
    {0x3CB8, 0x00},
    {0x3CBA, 0xFF},
    {0x3CBB, 0x03},
    {0x3CBC, 0xFF},
    {0x3CBD, 0x03},
    {0x3CC2, 0xFF},
    {0x3CC8, 0xFF},
    {0x3CC9, 0x03},
    {0x3CCA, 0x00},
    {0x3CCE, 0xFF},
    {0x3CCF, 0x03},
    {0x3CD0, 0xFF},
    {0x3E00, 0x31},
    {0x3E02, 0x04},
    {0x3E03, 0x00},
    {0x3E20, 0x04},
    {0x3E21, 0x00},
    {0x3E22, 0x31},
    {0x3E24, 0xB0},
    {0x3E2C, 0x38},
    {0x4470, 0xFF},
    {0x4471, 0x00},
    {0x4478, 0x00},
    {0x4479, 0xFF},
    {0x4490, 0x05},
    {0x4494, 0x19},
    {0x4495, 0x00},
    {0x4496, 0x55},
    {0x4497, 0x01},
    {0x4498, 0xA1},
    {0x449A, 0xA0},
    {0x449C, 0x9B},
    {0x449E, 0x96},
    {0x44A0, 0x91},
    {0x44A2, 0x87},
    {0x44A4, 0x7D},
    {0x44A6, 0x78},
    {0x44B8, 0xA1},
    {0x44BA, 0xA0},
    {0x44BC, 0x9B},
    {0x44BE, 0x96},
    {0x44C0, 0x91},
    {0x44C2, 0x87},
    {0x44C4, 0x7D},
    {0x44C6, 0x78},
    {0x44C8, 0xB6},
    {0x44C9, 0x01},
    {0x44CA, 0xB6},
    {0x44CB, 0x01},
    {0x44CC, 0xB1},
    {0x44CD, 0x01},
    {0x44CE, 0xB1},
    {0x44CF, 0x01},
    {0x44D0, 0xA7},
    {0x44D1, 0x01},
    {0x44D2, 0xA2},
    {0x44D3, 0x01},
    {0x44D4, 0xA2},
    {0x44D5, 0x01},
    {0x44D6, 0x93},
    {0x44D7, 0x01},
    {0x44E8, 0xB6},
    {0x44E9, 0x01},
    {0x44EA, 0xB6},
    {0x44EB, 0x01},
    {0x44EC, 0xB1},
    {0x44ED, 0x01},
    {0x44EE, 0xB1},
    {0x44EF, 0x01},
    {0x44F0, 0xA7},
    {0x44F1, 0x01},
    {0x44F2, 0xA2},
    {0x44F3, 0x01},
    {0x44F4, 0xA2},
    {0x44F5, 0x01},
    {0x44F6, 0x93},
    {0x44F7, 0x01},
    {0x4534, 0x22},
    {0x4538, 0x22},
    {0x4539, 0x22},
    {0x453A, 0x22},
    {0x453B, 0x22},
    {0x453C, 0x22},
    {0x453D, 0x22},
    {0x453E, 0x22},
    {0x453F, 0x22},
    {0x4540, 0x22},
    {0x4544, 0x22},
    {0x4545, 0x22},
    {0x4546, 0x22},
    {0x4547, 0x22},
    {0x4548, 0x22},
    {0x4549, 0x22},
    {0x454A, 0x22},
    {0x454B, 0x22},
    {0x454C, 0x22},
    {0x4550, 0x22},
    {0x4551, 0x22},
    {0x4552, 0x22},
    {0x4553, 0x22},
    {0x4554, 0x22},
    {0x4555, 0x22},
    {0x4556, 0x22},
    {0x4557, 0x22},
    {0x4558, 0x22},
    {0x455C, 0x22},
    {0x455D, 0x22},
    {0x455E, 0x22},
    {0x455F, 0x22},
    {0x4560, 0x22},
    {0x4561, 0x22},
    {0x4562, 0x22},
    {0x4563, 0x22},
    {0x4564, 0x22},
    {0x4E30, 0x01},
    {0x4E31, 0x00},
    {0x4E32, 0x03},
    {0x4E33, 0x02},

    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_4m_30fps_init_table_4lane_HDR_DOL[] =
{
    {0x3000, 0x01},    //Standby
    {0x3002, 0x01},    //Master mode stop
    
    {0x3014, 0x04},
    {0x301a, 0x01},
    {0x301c, 0x01},
    {0x3050, 0x80},
    {0x3051, 0x0c},
    {0x3054, 0x0a},
    {0x3060, 0x16},
    {0x30a6, 0x00},
    {0x3148, 0x00},
    {0x3412, 0x01},
    {0x3460, 0x21},
    {0x3492, 0x08},
    {0x3930, 0x01},
    {0x3B1D, 0x17},
    {0x3C00, 0x0C},
    {0x3C01, 0x0C},
    {0x3C04, 0x02},
    {0x3C08, 0x1E},
    {0x3C09, 0x1D},
    {0x3C0A, 0x1D},
    {0x3C0B, 0x1D},
    {0x3C0C, 0x1D},
    {0x3C0D, 0x1D},
    {0x3C0E, 0x1C},
    {0x3C0F, 0x1B},
    {0x3C30, 0x73},
    {0x3C34, 0x03},
    {0x3C3C, 0x20},
    {0x3CB8, 0x00},
    {0x3CBA, 0xFF},
    {0x3CBB, 0x03},
    {0x3CBC, 0xFF},
    {0x3CBD, 0x03},
    {0x3CC2, 0xFF},
    {0x3CC8, 0xFF},
    {0x3CC9, 0x03},
    {0x3CCA, 0x00},
    {0x3CCE, 0xFF},
    {0x3CCF, 0x03},
    {0x3CD0, 0xFF},
    {0x3E00, 0x31},
    {0x3E02, 0x04},
    {0x3E03, 0x00},
    {0x3E20, 0x04},
    {0x3E21, 0x00},
    {0x3E22, 0x31},
    {0x3E24, 0xB0},
    {0x3E2C, 0x38},
    {0x4470, 0xFF},
    {0x4471, 0x00},
    {0x4478, 0x00},
    {0x4479, 0xFF},
    {0x4490, 0x05},
    {0x4494, 0x19},
    {0x4495, 0x00},
    {0x4496, 0x55},
    {0x4497, 0x01},
    {0x4498, 0xA1},
    {0x449A, 0xA0},
    {0x449C, 0x9B},
    {0x449E, 0x96},
    {0x44A0, 0x91},
    {0x44A2, 0x87},
    {0x44A4, 0x7D},
    {0x44A6, 0x78},
    {0x44B8, 0xA1},
    {0x44BA, 0xA0},
    {0x44BC, 0x9B},
    {0x44BE, 0x96},
    {0x44C0, 0x91},
    {0x44C2, 0x87},
    {0x44C4, 0x7D},
    {0x44C6, 0x78},
    {0x44C8, 0xB6},
    {0x44C9, 0x01},
    {0x44CA, 0xB6},
    {0x44CB, 0x01},
    {0x44CC, 0xB1},
    {0x44CD, 0x01},
    {0x44CE, 0xB1},
    {0x44CF, 0x01},
    {0x44D0, 0xA7},
    {0x44D1, 0x01},
    {0x44D2, 0xA2},
    {0x44D3, 0x01},
    {0x44D4, 0xA2},
    {0x44D5, 0x01},
    {0x44D6, 0x93},
    {0x44D7, 0x01},
    {0x44E8, 0xB6},
    {0x44E9, 0x01},
    {0x44EA, 0xB6},
    {0x44EB, 0x01},
    {0x44EC, 0xB1},
    {0x44ED, 0x01},
    {0x44EE, 0xB1},
    {0x44EF, 0x01},
    {0x44F0, 0xA7},
    {0x44F1, 0x01},
    {0x44F2, 0xA2},
    {0x44F3, 0x01},
    {0x44F4, 0xA2},
    {0x44F5, 0x01},
    {0x44F6, 0x93},
    {0x44F7, 0x01},
    {0x4534, 0x22},
    {0x4538, 0x22},
    {0x4539, 0x22},
    {0x453A, 0x22},
    {0x453B, 0x22},
    {0x453C, 0x22},
    {0x453D, 0x22},
    {0x453E, 0x22},
    {0x453F, 0x22},
    {0x4540, 0x22},
    {0x4544, 0x22},
    {0x4545, 0x22},
    {0x4546, 0x22},
    {0x4547, 0x22},
    {0x4548, 0x22},
    {0x4549, 0x22},
    {0x454A, 0x22},
    {0x454B, 0x22},
    {0x454C, 0x22},
    {0x4550, 0x22},
    {0x4551, 0x22},
    {0x4552, 0x22},
    {0x4553, 0x22},
    {0x4554, 0x22},
    {0x4555, 0x22},
    {0x4556, 0x22},
    {0x4557, 0x22},
    {0x4558, 0x22},
    {0x455C, 0x22},
    {0x455D, 0x22},
    {0x455E, 0x22},
    {0x455F, 0x22},
    {0x4560, 0x22},
    {0x4561, 0x22},
    {0x4562, 0x22},
    {0x4563, 0x22},
    {0x4564, 0x22},
    {0x4E00, 0x11}, //conti mode
    {0x4E30, 0x01},
    {0x4E31, 0x00},
    {0x4E32, 0x03},
    {0x4E33, 0x02},
    //pattern mode
    //{0x30E4 ,0x01},
    //{0x30E2 ,0x0a},
    //{0x30E0 ,0x01},
      
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY Sensor_4m_30fps_init_table_4lane_ClearHDR_VC[] =
{
    {0x3000, 0x01},    //Standby
    {0x3000, 0x00},
    {0x3002, 0x01},    //Master mode stop

    {0x3014, 0x04},
    {0x3015, 0x04},
    {0x3018, 0x00},
    {0x301a, 0x08},    //Clear HDR mode
    {0x301B, 0x00},
    {0x301C, 0x00},
    {0x301E, 0x01},
    {0x3020, 0x00},
    {0x3021, 0x00},
    {0x3022, 0x02},
    {0x3023, 0x01},
    {0x3028, 0xE4},    //VMAX LSB
    {0x3029, 0x0C},    //VMAX
    {0x302A, 0x00},    //VMAX MSB
    {0x302C, 0xEE},    //HMAX LSB
    {0x302D, 0x02},    //HMAX MSB
    {0x3030, 0x02},
    {0x3031, 0x00},
    {0x3032, 0x00},
    {0x303C, 0x00},
    {0x303D, 0x00},
    {0x303E, 0x90},
    {0x303F, 0x0A},
    {0x3040, 0x03},
    {0x3044, 0x00},
    {0x3045, 0x00},
    {0x3046, 0x04},
    {0x3047, 0x06},
    {0x304C, 0x22},    //ClearHDR mode High Gain
    {0x304D, 0x00},
    {0x3050, 0x10},    //SHR0, ClearHDR mode >= 16
    {0x3051, 0x00},
    {0x3052, 0x00},
    {0x3054, 0x1A},
    {0x3055, 0x00},
    {0x3056, 0x00},
    {0x3058, 0x4C},
    {0x3059, 0x00},
    {0x305A, 0x00},
    {0x3060, 0x32},
    {0x3061, 0x00},
    {0x3062, 0x00},
    {0x3064, 0x6A},
    {0x3065, 0x00},
    {0x3066, 0x00},
    {0x3070, 0x00},
    {0x3071, 0x00},
    {0x3072, 0x00},
    {0x3073, 0x00},
    {0x3074, 0x00},
    {0x3075, 0x00},
    {0x30A4, 0xAA},
    {0x30A6, 0x00},
    {0x30CC, 0x00},
    {0x30CD, 0x00},
    {0x30DC, 0x32},
    {0x30DD, 0x40},
    {0x310C, 0x01},
    {0x3130, 0x01},
    {0x3148, 0x00},
    {0x315E, 0x10},
    {0x3400, 0x01},
    {0x3412, 0x01},
    {0x3460, 0x22},
    {0x3492, 0x08},
    {0x3890, 0x08},
    {0x3891, 0x00},
    {0x3893, 0x00},
    {0x3930, 0x01},
    {0x3B1D, 0x17},
    {0x3C00, 0x09},
    {0x3C01, 0x09},
    {0x3C04, 0x02},
    {0x3C08, 0x00},
    {0x3C09, 0x00},
    {0x3C0A, 0x00},
    {0x3C0B, 0x00},
    {0x3C0C, 0x00},
    {0x3C0D, 0x00},
    {0x3C0E, 0x1F},
    {0x3C0F, 0x1E},
    {0x3C30, 0x73},
    {0x3C34, 0x03},
    {0x3C3C, 0x20},
    {0x3C44, 0x05},
    {0x3CB8, 0x00},
    {0x3CBA, 0xFF},
    {0x3CBB, 0x03},
    {0x3CBC, 0xFF},
    {0x3CBD, 0x03},
    {0x3CC2, 0xFF},
    {0x3CC8, 0xFF},
    {0x3CC9, 0x03},
    {0x3CCA, 0x00},
    {0x3CCE, 0xFF},
    {0x3CCF, 0x03},
    {0x3CD0, 0xFF},
    {0x3E00, 0x31},
    {0x3E02, 0x04},
    {0x3E03, 0x00},
    {0x3E20, 0x04},
    {0x3E21, 0x00},
    {0x3E22, 0x31},
    {0x3E24, 0xB0},
    {0x3E2C, 0x38},
    {0x4470, 0xFF},
    {0x4471, 0x00},
    {0x4478, 0x00},
    {0x4479, 0xFF},
    {0x4490, 0x05},
    {0x4494, 0x19},
    {0x4495, 0x00},
    {0x4496, 0x55},
    {0x4497, 0x01},
    {0x4498, 0xA1},
    {0x449A, 0xA0},
    {0x449C, 0x9B},
    {0x449E, 0x96},
    {0x44A0, 0x91},
    {0x44A2, 0x87},
    {0x44A4, 0x7D},
    {0x44A6, 0x78},
    {0x44B8, 0xA1},
    {0x44BA, 0xA0},
    {0x44BC, 0x9B},
    {0x44BE, 0x96},
    {0x44C0, 0x91},
    {0x44C2, 0x87},
    {0x44C4, 0x7D},
    {0x44C6, 0x78},
    {0x44C8, 0xB6},
    {0x44C9, 0x01},
    {0x44CA, 0xB6},
    {0x44CB, 0x01},
    {0x44CC, 0xB1},
    {0x44CD, 0x01},
    {0x44CE, 0xB1},
    {0x44CF, 0x01},
    {0x44D0, 0xA7},
    {0x44D1, 0x01},
    {0x44D2, 0xA2},
    {0x44D3, 0x01},
    {0x44D4, 0xA2},
    {0x44D5, 0x01},
    {0x44D6, 0x93},
    {0x44D7, 0x01},
    {0x44E8, 0xB6},
    {0x44E9, 0x01},
    {0x44EA, 0xB6},
    {0x44EB, 0x01},
    {0x44EC, 0xB1},
    {0x44ED, 0x01},
    {0x44EE, 0xB1},
    {0x44EF, 0x01},
    {0x44F0, 0xA7},
    {0x44F1, 0x01},
    {0x44F2, 0xA2},
    {0x44F3, 0x01},
    {0x44F4, 0xA2},
    {0x44F5, 0x01},
    {0x44F6, 0x93},
    {0x44F7, 0x01},
    {0x4534, 0x22},
    {0x4538, 0x22},
    {0x4539, 0x22},
    {0x453A, 0x22},
    {0x453B, 0x22},
    {0x453C, 0x22},
    {0x453D, 0x22},
    {0x453E, 0x22},
    {0x453F, 0x22},
    {0x4540, 0x22},
    {0x4544, 0x22},
    {0x4545, 0x22},
    {0x4546, 0x22},
    {0x4547, 0x22},
    {0x4548, 0x22},
    {0x4549, 0x22},
    {0x454A, 0x22},
    {0x454B, 0x22},
    {0x454C, 0x22},
    {0x4550, 0x22},
    {0x4551, 0x22},
    {0x4552, 0x22},
    {0x4553, 0x22},
    {0x4554, 0x22},
    {0x4555, 0x22},
    {0x4556, 0x22},
    {0x4557, 0x22},
    {0x4558, 0x22},
    {0x455C, 0x22},
    {0x455D, 0x22},
    {0x455E, 0x22},
    {0x455F, 0x22},
    {0x4560, 0x22},
    {0x4561, 0x22},
    {0x4562, 0x22},
    {0x4563, 0x22},
    {0x4564, 0x22},
    {0x4E00, 0x11}, //conti mode
    {0x4E30, 0x01},
    {0x4E31, 0x00},
    {0x4E32, 0x03},
    {0x4E33, 0x02},
    //pattern mode
    //{0x30E4 ,0x01},
    //{0x30E2 ,0x0a},
    //{0x30E0 ,0x01},
      
    {0x3002, 0x00},    //Master mode start
    {0xffff, 0x10},
    {0x3000, 0x00},    //Operating
};

const static I2C_ARRAY vts_reg[] = {       //VMAX
    {0x302a, 0x00}, //bit0-3-->MSB
    {0x3029, 0x0C},
    {0x3028, 0xE4},
};

const static I2C_ARRAY expo_SHR0_reg[] = { // SHS0 (For LEF)
    {0x3052, 0x00},
    {0x3051, 0x00},
    {0x3050, 0x84},
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
    {0x3070, 0x2A},//low bit
    {0x3071, 0x00},
};

const static I2C_ARRAY gain_HDR_DOL_LEF_reg[] = {
    {0x3070, 0x2A},
    {0x3071, 0x00},
};

const static I2C_ARRAY gain_HDR_DOL_SEF_reg[] = {
    {0x3072, 0x20},
    {0x3073, 0x00},
};

const static I2C_ARRAY gain_Clr_HDR_LG_reg[] = {
    {0x3070, 0x00},
    {0x3071, 0x00},
};

const static I2C_ARRAY gain_Clr_HDR_HG_reg[] = {
    {0x304C, 0x00},
    {0x304D, 0x00},
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
static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    //Sensor power on sequence
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);   // Powerdn Pull Low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);     // Rst Pull Low
    sensor_if->MCLK(idx, 0, handle->mclk);
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, ENABLE);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }
    
    SENSOR_MSLEEP(10);

    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    //Sensor board PWDN Enable, 1.8V & 2.9V need 30ms then Pull High
    SENSOR_MSLEEP(10);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    //SENSOR_UDELAY(1);
    SENSOR_MSLEEP(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_UDELAY(30);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx664_params *params = (imx664_params *)handle->private_data;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);   // Powerdn Pull Low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);     // Rst Pull Low
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }

    params->orit = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// Check Sensor Product ID /////////////////////////
static int pCus_CheckSensorProductID(ss_cus_sensor *handle)
{
    u16 sen_id_msb = 0, sen_id_lsb = 0;

    /* Read Product ID */
    SENSOR_MSLEEP(25);
    SensorReg_Read(0x4D1C, (void*)&sen_id_lsb);
    SensorReg_Read(0x4D1D, (void*)&sen_id_msb);
    //Product ID = 0x0298
    SENSOR_EMSG("===== IMX664 Product ID = 0x%x =====\n", sen_id_lsb + (sen_id_msb << 8));
    
    return SUCCESS;
}
//Get and check sensor ID
static int pCus_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
    return SUCCESS;
}

static int imx664_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    return SUCCESS;
}

static int pCus_init_4m_30fps_mipi4lane_linear(ss_cus_sensor *handle)
{
    int i,cnt=0;

    for(i=0;i< ARRAY_SIZE(Sensor_4m_30fps_init_table_4lane_linear);i++)
    {
        if(Sensor_4m_30fps_init_table_4lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_4m_30fps_init_table_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_4m_30fps_init_table_4lane_linear[i].reg, Sensor_4m_30fps_init_table_4lane_linear[i].data) != SUCCESS)
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
    
    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }
    
    return SUCCESS;
}

static int pCus_init_4m_30fps_mipi4lane_HDR_DOL(ss_cus_sensor *handle)
{
    int i,cnt=0;

    for(i=0;i< ARRAY_SIZE(Sensor_4m_30fps_init_table_4lane_HDR_DOL);i++)
    {
        if(Sensor_4m_30fps_init_table_4lane_HDR_DOL[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_4m_30fps_init_table_4lane_HDR_DOL[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_4m_30fps_init_table_4lane_HDR_DOL[i].reg,Sensor_4m_30fps_init_table_4lane_HDR_DOL[i].data) != SUCCESS)
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
    
    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }
    
    return SUCCESS;
}

static int pCus_init_4m_30fps_mipi4lane_HDR_ClearHDR(ss_cus_sensor *handle)
{
    int i,cnt=0;

    for(i=0;i< ARRAY_SIZE(Sensor_4m_30fps_init_table_4lane_ClearHDR_VC);i++)
    {
        if(Sensor_4m_30fps_init_table_4lane_ClearHDR_VC[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_4m_30fps_init_table_4lane_ClearHDR_VC[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_4m_30fps_init_table_4lane_ClearHDR_VC[i].reg,Sensor_4m_30fps_init_table_4lane_ClearHDR_VC[i].data) != SUCCESS)
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
    
    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
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
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_4m_30fps_mipi4lane_linear;
            vts_30fps_linear = 1980; //2700;
            params->expo.vts = vts_30fps_linear;
            params->expo.fps = 30;
            Preview_line_period_linear = 16835;
            break;
        default:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_4m_30fps_mipi4lane_linear;
            vts_30fps_linear = 1980; //2700;
            params->expo.vts = vts_30fps_linear;
            params->expo.fps = 30;
            Preview_line_period_linear = 16835;
            break;
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
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_SEF(ss_cus_sensor *handle, u32 res_idx)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    switch (res_idx) {
        case HDR_RES_1:
            handle->pCus_sensor_init = pCus_init_4m_30fps_mipi4lane_HDR_DOL;
            vts_30fps_DOL_4lane = 1650;
            params->expo.vts = vts_30fps_DOL_4lane;
            params->expo.fps = 30;
            Preview_line_period_DOL_4LANE = 10101;
            params->min_rhs1 = 430; //4n+2
            params->Clrhdr_mode_en = 0;
            break;
        case HDR_RES_2:
            handle->pCus_sensor_init = pCus_init_4m_30fps_mipi4lane_HDR_ClearHDR;
            vts_30fps_DOL_4lane = 3300;
            params->expo.vts = vts_30fps_DOL_4lane;
            params->expo.fps = 30;
            handle->sensor_ae_info_cfg.u8AEGainDelay = SENSOR_GAIN_DELAY_FRAME_COUNT_CLRHDR;
            Preview_line_period_DOL_4LANE = 10101;
            params->min_rhs1 = 430; //4n+2
            params->Clrhdr_mode_en = 1;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    //s16 sen_data;
    u32 IsMirror = 0;
    u32 IsFlip = 0;
    //Read SENSOR MIRROR-FLIP STATUS
    //SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);
    
    SensorReg_Read(SENSOR_FLIP, (void*)&IsFlip);
    SensorReg_Read(SENSOR_MIRROR, (void*)&IsMirror);
    
    if(IsFlip && IsMirror){
        *orit = CUS_ORIT_M1F1;
    }else if(IsFlip == 1 && IsMirror == 0){
        *orit = CUS_ORIT_M0F1;
    }else if(IsFlip == 0 && IsMirror == 1){
        *orit = CUS_ORIT_M1F0;
    }else{
        *orit = CUS_ORIT_M0F0;
    }
    
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx664_params *params = (imx664_params *)handle->private_data;

    params->orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 IsMirror = 0;
    u32 IsFlip = 0;
    //Read SENSOR MIRROR-FLIP STATUS
    //SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);
    //sen_data &= ~(SENSOR_MIRROR_FLIP_EN);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            //sen_data |= SENSOR_NOR;
            IsMirror = 0;
            IsFlip   = 0;
            params->orit = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M1F0:
            //sen_data |= SENSOR_MIRROR_EN;
            IsMirror = 1;
            IsFlip   = 0;
            params->orit = CUS_ORIT_M1F0;
            break;
        case CUS_ORIT_M0F1:
            //sen_data |= SENSOR_FLIP_EN;
            IsMirror = 0;
            IsFlip   = 1;
            params->orit = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            //sen_data |= SENSOR_MIRROR_FLIP_EN;
            IsMirror = 1;
            IsFlip   = 1;
            params->orit = CUS_ORIT_M1F1;
            break;
        default :
            params->orit = CUS_ORIT_M0F0;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(SENSOR_FLIP, IsFlip);
    SensorReg_Write(SENSOR_MIRROR, IsMirror);
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_linear*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_linear*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (vts_30fps_linear*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts=  (vts_30fps_linear*(max_fps*1000) + (fps>>1))/fps;
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
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_DOL_4lane;
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
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    cur_vts_30fps=vts_30fps_DOL_4lane;
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
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_DOL_4lane;
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
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    cur_vts_30fps=vts_30fps_DOL_4lane;
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
    imx664_params *params = (imx664_params *)handle->private_data;
    //return SUCCESS;

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
                    DoOrien(handle, params->orit);
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
    //imx664_params *params = (imx664_params *)handle->private_data;

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
    imx664_params *params = (imx664_params *)handle->private_data;
    //return SUCCESS;

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
                    }
                    else // Clear HDR
                    {
                        SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tSHR0_reg, ARRAY_SIZE(expo_SHR0_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_Clr_hDR_HG_reg, ARRAY_SIZE(gain_Clr_HDR_HG_reg));
                        SensorRegArrayW((I2C_ARRAY*)params->tGain_Clr_hDR_LG_reg, ARRAY_SIZE(gain_Clr_HDR_LG_reg));
                        
        SENSOR_DMSG("[%s] tSHR0_reg %02x %02x %02x, tVts_reg %02x %02x %02x\n", __FUNCTION__,
                   params->tSHR0_reg[0].data, params->tSHR0_reg[1].data, params->tSHR0_reg[2].data, \
                   params->tVts_reg[0].data, params->tVts_reg[1].data, params->tVts_reg[2].data \
                );
                    }
                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    handle->sensor_if_api->SetSkipFrame(idx, params->expo.fps, 1);
                    DoOrien(handle, params->orit);
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
    imx664_params *params = (imx664_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data & 0x03) << 16;
    lines |= (u32)(params->tExpo_reg[1].data & 0xff) << 8;
    lines |= (u32)(params->tExpo_reg[2].data & 0xff) << 0;

    *us = (lines * Preview_line_period_linear) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);
    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines = 0, vts = 0, SHR0 = 0;
    imx664_params *params = (imx664_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000*us)/Preview_line_period_linear;
    expo_lines = (expo_lines>>1)<<1;

    /* SHR0: 8 to vts -2, multiple of 2 */
    if (expo_lines <= 2) expo_lines = 2;
    if (expo_lines > params->expo.vts - 8) {
        vts = expo_lines + 8;
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

static u32 pCus_TryAEShutter(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines = 0, vts = 0, SHR0 = 0;
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 actual_us = us;

    expo_lines = (1000*us)/Preview_line_period_linear;
    expo_lines = (expo_lines>>1)<<1;

    /* SHR0: 8 to vts -2, multiple of 2 */
    if (expo_lines <= 2) expo_lines = 2;
    if (expo_lines > params->expo.vts - 8) {
        vts = expo_lines + 8;
    }
    else
        vts = params->expo.vts;
    SHR0 =  vts - expo_lines;
    actual_us = (vts - SHR0) * Preview_line_period_linear/1000;

    //CamOsPrintf("try_Exp us=%d act_us=%d\n",us,actual_us);
    return actual_us;
}

static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_sef = 0;
    u32 cur_line_period = Preview_line_period_DOL_4LANE;
    imx664_params *params = (imx664_params *)handle->private_data;

    params->expo.expo_sef_us = us;
    cur_line_period = Preview_line_period_DOL_4LANE;
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

    params->tRHS1_reg[0].data = (params->min_rhs1 >>16) & 0x0f;
    params->tRHS1_reg[1].data = (params->min_rhs1 >>8) & 0xff;
    params->tRHS1_reg[2].data = (params->min_rhs1 >>0) & 0xff;

    params->tSHR1_reg[0].data = (params->min_shr1 >> 16) & 0x000f;
    params->tSHR1_reg[1].data = (params->min_shr1 >> 8) & 0x00ff;
    params->tSHR1_reg[2].data = (params->min_shr1 >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_sef = 0, SHR1 = 0;
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 actual_us = us;
    u32 cur_line_period = Preview_line_period_DOL_4LANE;

    expo_line_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    expo_line_sef = (expo_line_sef >> 2)<<2;

    if(expo_line_sef < 4) expo_line_sef = 4;
    SHR1 = params->min_rhs1 - expo_line_sef;

    actual_us = (params->min_rhs1 - SHR1) * Preview_line_period_DOL_4LANE/1000;

    //CamOsPrintf("SEF try_Exp us=%d act_us=%d\n",us,actual_us);
    return actual_us;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_lef = 0, fsc = 0, vts=0;
    u32 cur_line_period = Preview_line_period_DOL_4LANE;
    imx664_params *params = (imx664_params *)handle->private_data;

    if (params->Clrhdr_mode_en == 0)
    {
        params->expo.expo_lef_us = us;
        //cur_line_period = Preview_line_period_DOL_4LANE;
        expo_line_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
        expo_line_lef = (expo_line_lef >> 2)<<2; //shr0: 4n

        if(expo_line_lef < 4) expo_line_lef = 4;
        fsc = params->expo.vts * 2;   //fsc: 4n
        params->max_shr0 = fsc - expo_line_lef;

        if(params->max_shr0 < (params->min_rhs1+10))
            params->max_shr0 = params->min_rhs1+10;
        
        params->expo.expo_lines = expo_line_lef;

        if (expo_line_lef > (fsc - params->min_rhs1 - 10)) {
            vts = (expo_line_lef + params->min_rhs1 + 10) / 2;
        }
        else{
          vts = params->expo.vts;
        }

        SENSOR_DMSG("[%s] DOL us %u, expo_lines_lef %u, vts %u, SHR0 %u \n", __FUNCTION__,
                                                                     us, \
                                                                     expo_line_lef, \
                                                                     vts, \
                                                                     params->max_shr0
                );
        params->tSHR0_reg[0].data = (params->max_shr0 >> 16) & 0x000f;
        params->tSHR0_reg[1].data = (params->max_shr0 >> 8) & 0x00ff;
        params->tSHR0_reg[2].data = (params->max_shr0 >> 0) & 0x00ff;

        params->tVts_reg[0].data = (vts >> 16) & 0x000F;
        params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (vts >> 0) & 0x00ff;
    }
    else
    {
        u32 expo_lines = 0, vts = 0, SHR0 = 0;

        params->expo.expo_lef_us = us;
        expo_lines = (1000*us)/Preview_line_period_DOL_4LANE;
        expo_lines = (expo_lines>>2)<<2;

        /* SHR0: 16 to vts -2, multiple of 4 */
        if (expo_lines <= 4) expo_lines = 4;
        if (expo_lines > params->expo.vts - 16) {
            vts = expo_lines + 16;
        }
        else
            vts = params->expo.vts;
        SHR0 =  vts - expo_lines;

        params->expo.expo_lines = expo_lines;

        SENSOR_DMSG("[%s] ClearHDR us %u, expo_lines %u, vts %u, SHR0 %u \n", __FUNCTION__,
                                                                     us, \
                                                                     expo_lines, \
                                                                     vts, \
                                                                     SHR0
                );
        params->tSHR0_reg[0].data = (SHR0 >> 16) & 0x000f;
        params->tSHR0_reg[1].data = (SHR0 >> 8) & 0x00ff;
        params->tSHR0_reg[2].data = (SHR0 >> 0) & 0x00ff;

        params->tVts_reg[0].data = (vts >> 16) & 0x000F;
        params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (vts >> 0) & 0x00ff;
        SENSOR_DMSG("[%s] tSHR0_reg %02x %02x %02x, tVts_reg %02x %02x %02x\n", __FUNCTION__,
                   params->tSHR0_reg[0].data, params->tSHR0_reg[1].data, params->tSHR0_reg[2].data, \
                   params->tVts_reg[0].data, params->tVts_reg[1].data, params->tVts_reg[2].data \
                );

    }


    params->dirty = true;
    return SUCCESS;
}
static u32 pCus_TryAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_line_lef = 0, vts = 0, SHR0 = 0, fsc = 0;
    u32 cur_line_period = Preview_line_period_DOL_4LANE;
    imx664_params *params = (imx664_params *)handle->private_data;
    u32 actual_us = us;

    if (params->Clrhdr_mode_en == 0)
    {
        expo_line_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
        expo_line_lef = (expo_line_lef >> 2)<<2; //shr0: 4n

        if(expo_line_lef < 4) expo_line_lef = 4;
        fsc = params->expo.vts * 2;   //fsc: 4n
        params->max_shr0 = fsc - expo_line_lef;

        if(params->max_shr0 < (params->min_rhs1+10))
            params->max_shr0 = params->min_rhs1+10;

        if (expo_line_lef > (fsc - params->min_rhs1 - 10)) {
            vts = (expo_line_lef + params->min_rhs1 + 10) / 2;
        }
        else{
          vts = params->expo.vts;
        }

        SHR0 =  fsc - expo_line_lef;
        actual_us = (fsc - SHR0) * Preview_line_period_linear/1000;
    }
    else
    {
        expo_line_lef = (1000*us)/Preview_line_period_DOL_4LANE;
        expo_line_lef = (expo_line_lef>>2)<<2;

        /* SHR0: 16 to vts -2, multiple of 4 */
        if (expo_line_lef <= 4) expo_line_lef = 4;
        if (expo_line_lef > params->expo.vts - 16) {
            vts = expo_line_lef + 16;
        }
        else
            vts = params->expo.vts;
        SHR0 =  vts - expo_line_lef;

        actual_us = (vts - SHR0) * Preview_line_period_linear/1000;
    }
    //CamOsPrintf("LEf try_Exp us=%d act_us=%d\n",us,actual_us);
    return actual_us;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx664_params *params = (imx664_params *)handle->private_data;

    *gain = params->expo.final_gain;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);
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

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u64 gain_double;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    gain_double = (u16)((gain_double * 10) >> 24) / 3;

    params->tGain_reg[0].data = gain_double & 0xff;
    params->tGain_reg[1].data = (gain_double >> 8) & 0x07;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data, gain,params->tGain_reg[1].data);
    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGain(ss_cus_sensor *handle, u32 gain)
{
    u32 valid_gain=0;
    u64 gain_double;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    gain_double = (u16)((gain_double * 10) >> 24) / 3;

    pCus_GetAEGain_Calculate(gain_double, &valid_gain);
    //CamOsPrintf("try_gain %d act_gain=%d\n",gain,valid_gain);

    return valid_gain;
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
    imx664_params *params = (imx664_params *)handle->private_data;
    u16 gain_reg = 0;

    if (params->Clrhdr_mode_en == 0)
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        params->tGain_hdr_dol_sef_reg[0].data = gain_reg & 0xff;
        params->tGain_hdr_dol_sef_reg[1].data = (gain_reg >> 8) & 0xff;
        
        SENSOR_DMSG("[%s] set DOL gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data, params->tGain_hdr_dol_sef_reg[1].data);
    }
    else
    {        
        if(gain < SENSOR_CLR_HDR_MIN_GAIN_LG)
            gain = 1024;
        else if(gain >= SENSOR_CLR_HDR_MAX_GAIN_LG)
            gain = SENSOR_CLR_HDR_MAX_GAIN_LG;
        
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        
        params->tGain_Clr_hDR_LG_reg[0].data = gain_reg & 0xff;
        params->tGain_Clr_hDR_LG_reg[1].data = (gain_reg >> 8) & 0xff;
        
        SENSOR_DMSG("[%s] set CLRHDR gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_Clr_hDR_LG_reg[0].data, params->tGain_Clr_hDR_LG_reg[1].data);
    }

    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGainHDR_DOL_SEF1(ss_cus_sensor *handle, u32 gain)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u16 gain_reg = 0;
    u32 valid_gain=0;

    if (params->Clrhdr_mode_en == 0)
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        pCus_GetAEGain_Calculate(gain_reg, &valid_gain);

    }
    else
    {
        //u64 gain_double = gain;
        if(gain < SENSOR_CLR_HDR_MIN_GAIN_LG)
            gain = 1024;
        else if(gain >= SENSOR_CLR_HDR_MAX_GAIN_LG)
            gain = SENSOR_CLR_HDR_MAX_GAIN_LG;

        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        pCus_GetAEGain_Calculate(gain_reg, &valid_gain);

    }
    //CamOsPrintf("SEF try_gain %d act_gain=%d\n",gain,valid_gain);

    return valid_gain;
}

static int pCus_SetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u16 gain_reg = 0;

    if (params->Clrhdr_mode_en == 0)
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        params->tGain_hdr_dol_lef_reg[0].data = gain_reg & 0xff;
        params->tGain_hdr_dol_lef_reg[1].data = (gain_reg >> 8) & 0xff;
    
        SENSOR_DMSG("[%s] set DOL gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data, params->tGain_hdr_dol_lef_reg[1].data);
    }
    else
    {
        u64 gain_double = gain;
        if(gain_double < SENSOR_CLR_HDR_MIN_GAIN_HG)
            gain_double = 1024 * (u64)SENSOR_CLR_HDR_MIN_GAIN_HG / (u64)SENSOR_HCG_GAIN;
        else if(gain_double >= SENSOR_CLR_HDR_MAX_GAIN_HG)
            gain_double = 1024 * (u64)SENSOR_CLR_HDR_MAX_GAIN_HG / (u64)SENSOR_HCG_GAIN;
        else
            gain_double = 1024 * gain_double / (u64)SENSOR_HCG_GAIN;
        
        gain = (u32)gain_double;
        
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        
        params->tGain_Clr_hDR_HG_reg[0].data = gain_reg & 0xff;
        params->tGain_Clr_hDR_HG_reg[1].data = (gain_reg >> 8) & 0xff;
        SENSOR_DMSG("[%s] set CLRHDR gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_Clr_hDR_HG_reg[0].data, params->tGain_Clr_hDR_HG_reg[1].data);
    }

    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    u16 gain_reg = 0;
    u32 valid_gain=0;

    if (params->Clrhdr_mode_en == 0)
    {
        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        pCus_GetAEGain_Calculate(gain_reg, &valid_gain);

        //SENSOR_DMSG("[%s] set DOL gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data, params->tGain_hdr_dol_lef_reg[1].data);
    }
    else
    {
        u64 gain_double = gain;
        if(gain_double < SENSOR_CLR_HDR_MIN_GAIN_HG)
            gain_double = 1024 * (u64)SENSOR_CLR_HDR_MIN_GAIN_HG / (u64)SENSOR_HCG_GAIN;
        else if(gain_double >= SENSOR_CLR_HDR_MAX_GAIN_HG)
            gain_double = 1024 * (u64)SENSOR_CLR_HDR_MAX_GAIN_HG / (u64)SENSOR_HCG_GAIN;
        else
            gain_double = 1024 * gain_double / (u64)SENSOR_HCG_GAIN;

        gain = (u32)gain_double;

        pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
        pCus_GetAEGain_Calculate(gain_reg, &valid_gain);

        //SENSOR_DMSG("[%s] set CLRHDR gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_Clr_hDR_HG_reg[0].data, params->tGain_Clr_hDR_HG_reg[1].data);
    }


    //CamOsPrintf("LEF try_gain %d act_gain=%d\n",gain,valid_gain);

    return valid_gain;
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
    imx664_params *params = (imx664_params *)handle->private_data;

    params->orit = orit;
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
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/imx664_mipi_linear[0].senout.min_fps;
    return SUCCESS;
}

static int pCus_GetAEMinMaxUSecs_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/imx664_mipi_hdr[0].senout.min_fps;
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

static int IMX664_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/imx664_mipi_linear[0].senout.min_fps;
    info->min = (Preview_line_period * 2);
    info->step = Preview_line_period *2;
    return SUCCESS;
}

static int IMX664_GetShutterInfo_HDR_DOL_SEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    imx664_params *params = (imx664_params *)handle->private_data;
    info->max = Preview_line_period_HDR_DOL_4LANE * params->min_rhs1;
    info->min = (Preview_line_period_HDR_DOL_4LANE * 4);
    info->step = Preview_line_period_HDR_DOL_4LANE *4;
    return SUCCESS;
}

static int IMX664_GetShutterInfo_HDR_DOL_LEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/imx664_mipi_hdr[0].senout.min_fps;
    info->min = (Preview_line_period_HDR_DOL_4LANE * 4);
    info->step = Preview_line_period_HDR_DOL_4LANE *4;
    return SUCCESS;
}
#endif

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
            SENSOR_DMSG("cmdid %d, unknow \n");
            break;
    }
    
    return SUCCESS;
}

static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    ss_cus_sensor *cus_handle = handle;
    imx664_params *params = (imx664_params *)cus_handle->private_data;
    
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period_linear*2;
        info->u32AEShutter_step                  = Preview_line_period_linear;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_min                   = Preview_line_period_DOL_4LANE * 4;
        if (params->Clrhdr_mode_en == 0)
        {
            info->u32AEShutter_max               = 1000000000/imx664_mipi_hdr[0].senout.min_fps;
        }
        else
        {
            info->u32AEShutter_max               = 1000000000/imx664_mipi_hdr[1].senout.min_fps;
            if (handle->pCus_sensor_SetVideoRes == pCus_SetVideoRes_HDR_DOL_SEF)
            {
                info->u32AEGain_min                      = SENSOR_CLR_HDR_MIN_GAIN_LG;
                info->u32AEGain_max                      = SENSOR_CLR_HDR_MAX_GAIN_LG;
            }
            else if (handle->pCus_sensor_SetVideoRes == pCus_SetVideoRes_HDR_DOL_LEF)
            {
                info->u32AEGain_min                      = SENSOR_CLR_HDR_MIN_GAIN_HG;
                info->u32AEGain_max                      = SENSOR_CLR_HDR_MAX_GAIN_HG;
            }
        }
        info->u32AEShutter_step                  = Preview_line_period_DOL_4LANE * 4;
    }
    return SUCCESS;
}

int cus_camsensor_init_handle_linear_imx664(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx664_params *params;
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

    params = (imx664_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"IMX664_MIPI");

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
    params->orit                  = SENSOR_ORIT;
    //handle->YC_ODER               = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num    = SENSOR_MIPI_LANE_NUM;
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
        handle->video_res_supported.res[res].u16width         = imx664_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx664_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx664_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx664_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx664_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx664_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx664_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx664_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx664_mipi_linear[res].senstr.strResDesc);
    }

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
    handle->pCus_sensor_release        = cus_camsensor_release_handle;
    handle->pCus_sensor_init           = pCus_init_4m_30fps_mipi4lane_linear;
    handle->pCus_sensor_poweron        = pCus_poweron;
    handle->pCus_sensor_poweroff       = pCus_poweroff;
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
    handle->pCus_sensor_SetPatternMode = imx664_SetPatternMode; //NONE

    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    //handle->ae_gain_delay              = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay           = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    //handle->ae_gain_ctrl_num           = 1;
    //handle->ae_shutter_ctrl_num        = 1;
    //handle->sat_mingain                = SENSOR_MIN_GAIN;//g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_linear * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx664_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_linear * 2;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGain;
    handle->pCus_sensor_TryAEShutter    = pCus_TryAEShutter;

    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    //handle->pCus_sensor_GetShutterInfo  = IMX664_GetShutterInfo;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    params->expo.vts = vts_30fps_linear;

    return SUCCESS;
}

int cus_camsensor_init_handle_imx664_hdr_dol_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx664_params *params = NULL;
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

    params = (imx664_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tRHS1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tSHR1_reg, expo_SHR1_reg, sizeof(expo_SHR1_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));
    memcpy(params->tGain_Clr_hDR_LG_reg, gain_Clr_HDR_LG_reg, sizeof(gain_Clr_HDR_LG_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "IMX664_MIPI_HDR_SEF");

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

    handle->interface_attr.attr_mipi.mipi_lane_num                = 4; // hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    //handle->interface_attr.attr_mipi.mipi_hsync_mode              = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;  // SONY IMX664 as VC mode
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx664_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx664_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx664_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx664_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx664_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx664_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx664_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx664_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx664_mipi_hdr[res].senstr.strResDesc);
    }

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
    handle->pCus_sensor_release        = cus_camsensor_release_handle;
    handle->pCus_sensor_init           = pCus_init_4m_30fps_mipi4lane_HDR_DOL;
    handle->pCus_sensor_poweron        = pCus_poweron;               // Need to check
    handle->pCus_sensor_poweroff       = pCus_poweroff;              // Need to check
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
    //handle->ae_gain_delay              = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay           = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    //handle->ae_gain_ctrl_num           = 2;
    //handle->ae_shutter_ctrl_num        = 2;
    //handle->sat_mingain                = SENSOR_MIN_GAIN;      //g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_DOL_4LANE * 4;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx664_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_DOL_4LANE * 4;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_SEF;  // Need to check
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_SEF;      // Need to check
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_SEF1;      // Need to check
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGainHDR_DOL_SEF1;
    handle->pCus_sensor_TryAEShutter    = pCus_TryAEUSecsHDR_DOL_SEF;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;

    //handle->pCus_sensor_GetShutterInfo  = IMX664_GetShutterInfo_HDR_DOL_SEF;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;
    params->expo.vts = vts_30fps_DOL_4lane;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx664_params *params;
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
    params = (imx664_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tSHR0_reg, expo_SHR0_reg, sizeof(expo_SHR0_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));
    memcpy(params->tGain_Clr_hDR_HG_reg, gain_Clr_HDR_HG_reg, sizeof(gain_Clr_HDR_HG_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "IMX664_MIPI_HDR_LEF");

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
    params->orit                   = SENSOR_ORIT;        //CUS_ORIT_M1F1;
    handle->interface_attr.attr_mipi.mipi_lane_num                = 4;//hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode              = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    //handle->video_res_supported.num_res = HDR_RES_END;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = imx664_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx664_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx664_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx664_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx664_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx664_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx664_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx664_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx664_mipi_hdr[res].senstr.strResDesc);
    }

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
    //    AE parameters               //
    ////////////////////////////////////
    //handle->ae_gain_delay               = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay            = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    //handle->ae_gain_ctrl_num            = 2;
    //handle->ae_shutter_ctrl_num         = 2;
    //handle->sat_mingain                 = SENSOR_MIN_GAIN;
    //handle->dgain_remainder = 0;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_DOL_4LANE * 4;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx664_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_DOL_4LANE * 4;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_LEF;
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGainHDR_DOL_LEF;
    handle->pCus_sensor_TryAEShutter    = pCus_TryAEUSecsHDR_DOL_LEF;

    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain_HDR_DOL_LEF;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs_HDR_DOL_LEF;

    //handle->pCus_sensor_GetShutterInfo  = IMX664_GetShutterInfo_HDR_DOL_LEF;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;
    
    params->expo.vts = vts_30fps_DOL_4lane;

    params->dirty = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(IMX664,
                            cus_camsensor_init_handle_linear_imx664,
                            cus_camsensor_init_handle_imx664_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            imx664_params
                         );
