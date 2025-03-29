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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX708);

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
//#define SENSOR_MIPI_LANE_NUM_DOL    (4)
//#define SENSOR_MIPI_HDR_MODE        (0) //0: Non-HDR mode. 1:Sony DOL mode

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9098  //CFG
#define SENSOR_BAYERID              CUS_BAYER_RG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,


////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x20                //I2C slave address//0x34
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
#define SENSOR_NAME     IMX708

#define CHIP_ID         0x0708

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_3, LINEAR_RES_END}mode;
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
}imx708_mipi_linear[] = {
    {LINEAR_RES_1, {4608, 2592, 3, 30}, {200, 0, 4208, 2592}, {"4208x2592@30fps"}},
    {LINEAR_RES_2, {4608, 2592, 3, 30}, {200, 112, 4208, 2368}, {"4208x2368@30fps"}},
    {LINEAR_RES_3, {2304, 1296, 3, 30}, {192, 108, 1920, 1080}, {"1920x1080@30fps"}},
};

u32 vts_30fps;
u32 Preview_line_period;

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
u32 SENSOR_MAX_GAIN = 16 * 16380;// 16*(15+255/256)*1024 //max sensor again, a-gain *d-gain
u32 SENSOR_MIN_GAIN = 1024;
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_CTRL_NUM                        (1)
#define SENSOR_SHUTTER_CTRL_NUM                     (1)

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR_FLIP                                 0x0101
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
        u32 binning_mode;
    } expo;

    u32 min_shr1;
    u32 min_rhs1;
    u32 min_shr0;
    u32 max_shr0;
    u32 fsc;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tGain_reg[4];
    bool dirty;
    bool orien_dirty;
} imx708_params;

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
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
const static I2C_ARRAY Sensor_init_table_11M_4lane_linear[] =
{

    {0x0136, 0x18}, //External Clock Setting
    {0x0137, 0x00},

    {0x33F0, 0x02}, //Register version
    {0x33F1, 0x05},
    {0x3068, 0x00}, //AE-Hist1 data type Setting
    {0x3069, 0x30},
    {0x3070, 0x00}, //Flicker data type Setting
    {0x3071, 0x30},
    {0x3062, 0x00}, //PDAF TYPE1 data type Setting
    {0x3063, 0x30},
    {0x3076, 0x00}, //PDAF TYPE2 data type Setting
    {0x3077, 0x30},
    {0x3078, 0x00},
    {0x3079, 0x30},
    {0x5E54, 0x0C}, //Global Setting
    {0x6E44, 0x00},
    {0xB0B6, 0x01},
    {0xBCF1, 0x00},

    //Image Quality adjustment setting
    {0xE829, 0x00},
    {0xF001, 0x08},
    {0xF003, 0x08},
    {0xF00D, 0x10},
    {0xF00F, 0x10},
    {0xF031, 0x08},
    {0xF033, 0x08},
    {0xF03D, 0x10},
    {0xF03F, 0x10},

    //MIPI output setting
    {0x0112, 0x0A},
    {0x0113, 0x0A},
    {0x0114, 0x03},

    //Line Length PCK Setting
    {0x0342, 0x14},
    {0x0343, 0x60},

    //Frame Length Lines Setting
    {0x0340, 0x0A},
    {0x0341, 0xC8},

    //ROI Setting
    {0x0344, 0x00},
    {0x0345, 0x00},
    {0x0346, 0x00},
    {0x0347, 0x00},
    {0x0348, 0x11},//
    {0x0349, 0xff},//4608
    {0x034A, 0x0a},//
    {0x034B, 0x1f},//2592

    //Mode Setting
    {0x0220, 0x62},
    {0x0222, 0x01},
    {0x0900, 0x00},
    {0x0901, 0x11},
    {0x0902, 0x0A},
    {0x3200, 0x01},
    {0x3201, 0x01},
    {0x32D5, 0x01},
    {0x32D6, 0x01},
    {0x32DB, 0x01},
    {0x32DF, 0x00},
    {0x350C, 0x00},
    {0x350D, 0x00},

    //Digital Crop & Scaling
    {0x0408, 0x00},
    {0x0409, 0x00},
    {0x040A, 0x00},
    {0x040B, 0x00},
    {0x040C, 0x12},
    {0x040D, 0x00},
    {0x040E, 0x0a},
    {0x040F, 0x20},

    //Output Size Setting
    {0x034C, 0x12},
    {0x034D, 0x00},
    {0x034E, 0x0a},
    {0x034F, 0x20},

    //Clock Setting
    {0x0301, 0x05},
    {0x0303, 0x02},
    {0x0305, 0x04},
    {0x0306, 0x00},
    {0x0307, 0xB4},
    {0x030B, 0x01},
    {0x030D, 0x04},
    {0x030E, 0x00},
    {0x030F, 0xD0},
    {0x0310, 0x01},

    //Other Setting
    {0x3CA0, 0x00},
    {0x3CA1, 0x64},
    {0x3CA4, 0x00},
    {0x3CA5, 0x00},
    {0x3CA6, 0x00},
    {0x3CA7, 0x00},
    {0x3CAA, 0x00},
    {0x3CAB, 0x00},
    {0x3CB8, 0x00},
    {0x3CB9, 0x08},
    {0x3CBA, 0x00},
    {0x3CBB, 0x00},
    {0x3CBC, 0x00},
    {0x3CBD, 0x3C},
    {0x3CBE, 0x00},
    {0x3CBF, 0x00},

    //Integration Setting
    {0x0202, 0x0A},
    {0x0203, 0x98},
    {0x0224, 0x01},
    {0x0225, 0xF4},
    {0x3116, 0x01},
    {0x3117, 0xF4},

    //Gain Setting
    {0x0204, 0x00},
    {0x0205, 0x00},
    {0x0216, 0x00},
    {0x0217, 0x00},
    {0x0218, 0x01},
    {0x0219, 0x00},
    {0x020E, 0x01},
    {0x020F, 0x00},
    {0x3118, 0x00},
    {0x3119, 0x00},
    {0x311A, 0x01},
    {0x311B, 0x00},

    {0x3400, 0x01}, //PDAF TYPE Setting
    {0x3091, 0x00}, //PDAF TYPE1 Setting
    {0x0100, 0x01}, //Streaming setting

};

const static I2C_ARRAY Sensor_init_table_3M_4lane_linear[] =
{

    {0x0136, 0x18}, //External Clock Setting
    {0x0137, 0x00},

    {0x33F0, 0x02}, //Register version
    {0x33F1, 0x05},
    {0x3068, 0x00}, //AE-Hist1 data type Setting
    {0x3069, 0x30},
    {0x3070, 0x00}, //Flicker data type Setting
    {0x3071, 0x30},
    {0x3062, 0x00}, //PDAF TYPE1 data type Setting
    {0x3063, 0x30},
    {0x3076, 0x00}, //PDAF TYPE2 data type Setting
    {0x3077, 0x30},
    {0x3078, 0x00},
    {0x3079, 0x30},
    {0x5E54, 0x0C}, //Global Setting
    {0x6E44, 0x00},
    {0xB0B6, 0x01},
    {0xBCF1, 0x00},

    //Image Quality adjustment setting
    {0xE829, 0x00},
    {0xF001, 0x08},
    {0xF003, 0x08},
    {0xF00D, 0x10},
    {0xF00F, 0x10},
    {0xF031, 0x08},
    {0xF033, 0x08},
    {0xF03D, 0x10},
    {0xF03F, 0x10},

    //MIPI output setting
    {0x0112, 0x0A},
    {0x0113, 0x0A},
    {0x0114, 0x03},

    //Line Length PCK Setting
    {0x0342, 0x25},
    {0x0343, 0x20},

    //Frame Length Lines Setting
    {0x0340, 0x0b},
    {0x0341, 0xd6},

    {0x0344, 0x00},
    {0x0345, 0x00},
    {0x0346, 0x00},
    {0x0347, 0x00},
    {0x0348, 0x11},//
    {0x0349, 0xff},//4608
    {0x034A, 0x0a},//
    {0x034B, 0x1f},//2592

    //Mode Setting
    {0x0220, 0x62},
    {0x0222, 0x01},
    {0x0900, 0x01},
    {0x0901, 0x22},
    {0x0902, 0x09},
    {0x3200, 0x41},
    {0x3201, 0x41},
    {0x32D5, 0x00},
    {0x32D6, 0x00},
    {0x32DB, 0x01},
    {0x32DF, 0x00},
    {0x350C, 0x00},
    {0x350D, 0x00},

    //Digital Crop & Scaling
    {0x0408, 0x00},
    {0x0409, 0x00},
    {0x040A, 0x00},
    {0x040B, 0x00},
    {0x040C, 0x09},
    {0x040D, 0x00},
    {0x040E, 0x05},
    {0x040F, 0x10},

    //Output Size Setting
    {0x034C, 0x09},
    {0x034D, 0x00},
    {0x034E, 0x05},
    {0x034F, 0x10},

    //Clock Setting
    {0x0301, 0x05},
    {0x0303, 0x02},
    {0x0305, 0x04},
    {0x0306, 0x01},
    {0x0307, 0x68},
    {0x030B, 0x02},
    {0x030D, 0x04},
    {0x030E, 0x01},
    {0x030F, 0x0a},
    {0x0310, 0x01},

    //Other Setting
    {0x3CA0, 0x00},
    {0x3CA1, 0x3c},
    {0x3CA4, 0x00},
    {0x3CA5, 0x3c},
    {0x3CA6, 0x00},
    {0x3CA7, 0x00},
    {0x3CAA, 0x00},
    {0x3CAB, 0x00},
    {0x3CB8, 0x00},
    {0x3CB9, 0x1c},
    {0x3CBA, 0x00},
    {0x3CBB, 0x08},
    {0x3CBC, 0x00},
    {0x3CBD, 0x1e},
    {0x3CBE, 0x00},
    {0x3CBF, 0x0a},

    //Integration Setting
    {0x0202, 0x0b},
    {0x0203, 0xa6},
    {0x0224, 0x01},
    {0x0225, 0xF4},
    {0x3116, 0x01},
    {0x3117, 0xF4},

    //Gain Setting
    {0x0204, 0x00},
    {0x0205, 0x70},
    {0x0216, 0x00},
    {0x0217, 0x70},
    {0x0218, 0x01},
    {0x0219, 0x00},
    {0x020E, 0x01},
    {0x020F, 0x00},
    {0x3118, 0x00},
    {0x3119, 0x70},
    {0x311A, 0x01},
    {0x311B, 0x00},

    {0x3400, 0x01}, //PDAF TYPE Setting
    {0x3091, 0x00}, //PDAF TYPE1 Setting
    {0x0100, 0x01}, //Streaming setting
};

const static I2C_ARRAY Sensor_id_table[] = {
    {0x0016, 0x02},      // {address of ID, ID },
    {0x0017, 0x58},      // {address of ID, ID },
};

static I2C_ARRAY PatternTbl[] = {
    {0x0000,0x00},       // colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY expo_reg[] = {      // SHS0 (For Linear)
    {0x0202, 0x00},
    {0x0203, 0x00},
};

const static I2C_ARRAY vts_reg[] = {       //VMAX
    //{0x3026, 0x00}, //bit0-3-->MSB
    {0x0340, 0x0C},
    {0x0341, 0x98},
};

const static I2C_ARRAY gain_reg[] = {
    {0x0204, 0x00}, //again 0-24db or 1-36db
    {0x0205, 0x00},
    {0x020E, 0x01}, //dgain 24db
    {0x020F, 0x00},
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
#endif

static void pCus_PowerOn_InitChipRX(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }
}
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period;
    info->u32AEShutter_step                  = Preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
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
    SENSOR_MSLEEP(40);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_UDELAY(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_UDELAY(20);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx708_params *params = (imx708_params *)handle->private_data;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);   // Powerdn Pull Low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);     // Rst Pull Low
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
    SensorReg_Read(0x0016, (void*)&sen_id_lsb);
    SensorReg_Read(0x0017, (void*)&sen_id_msb);//CHIP_ID_r3F13
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
    int i, n;
    int table_length = ARRAY_SIZE(Sensor_id_table);
    I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

    for(n=0;n<table_length;++n)
    {
      id_from_sensor[n].reg = Sensor_id_table[n].reg;
      id_from_sensor[n].data = 0;
    }

    *id = 0;
    if(table_length > 8)
        table_length = 8;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(n = 0; n < 4; ++n) //retry , until I2C success
    {
      if(n > 3)
          return FAIL;

      if(SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
          break;
      else
          SENSOR_MSLEEP(1);
    }

    //convert sensor id to u32 format
    for(i = 0; i < table_length; ++i)
    {
       if( id_from_sensor[i].data != Sensor_id_table[i].data ){
            SENSOR_DMSG("[%s] Please Check IMX708 Sensor Insert!!\n", __FUNCTION__);
            return FAIL;
       }
       *id = id_from_sensor[i].data;
    }

    SENSOR_DMSG("[%s]IMX708 sensor ,Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}
#endif

static int imx708_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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

static int pCus_init_11M_mipi4lane_linear(ss_cus_sensor *handle)
{
    //imx708_params *params = (imx708_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    if(pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i = 0; i < ARRAY_SIZE(Sensor_init_table_11M_4lane_linear); i++)
    {
        if(Sensor_init_table_11M_4lane_linear[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_11M_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_11M_4lane_linear[i].reg, Sensor_init_table_11M_4lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt >= 10)
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

static int pCus_init_3M_mipi4lane_linear(ss_cus_sensor *handle)
{
    //imx708_params *params = (imx708_params *)handle->private_data;
    int i,cnt=0;
    //s16 sen_data;

    if(pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i = 0; i < ARRAY_SIZE(Sensor_init_table_3M_4lane_linear); i++)
    {
        if(Sensor_init_table_3M_4lane_linear[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_3M_4lane_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_3M_4lane_linear[i].reg, Sensor_init_table_3M_4lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt >= 10)
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
    imx708_params *params = (imx708_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_11M_mipi4lane_linear;
            vts_30fps = 2760;//2691;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 12074;
            SENSOR_MIN_GAIN = 1024;
            SENSOR_MAX_GAIN = 16*16380;
            params->expo.binning_mode = 0;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 1:
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_11M_mipi4lane_linear;
            vts_30fps = 2760;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 12074;
            SENSOR_MIN_GAIN = 1024;
            SENSOR_MAX_GAIN = 16*16380;
            params->expo.binning_mode = 0;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        case 2:
            handle->video_res_supported.ulcur_res = 2;
            handle->pCus_sensor_init = pCus_init_3M_mipi4lane_linear;
            vts_30fps = 3030;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period = 22000; //11000*2
            SENSOR_MIN_GAIN = 1150;
            SENSOR_MAX_GAIN = 64*16380;
            params->expo.binning_mode = 1;
            handle->data_prec = CUS_DATAPRECISION_10;
            break;
        default:
            break;
    }

    params->tVts_reg[0].data = (vts_30fps >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts_30fps >> 0) & 0x00ff;
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
    imx708_params *params = (imx708_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    s16 sen_data;
    imx708_params *params = (imx708_params *)handle->private_data;
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
    imx708_params *params = (imx708_params *)handle->private_data;
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
    imx708_params *params = (imx708_params *)handle->private_data;
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
    //pr_info("[%s]  leslie_vts : %u\n\n", __FUNCTION__,params->expo.vts);

    params->expo.fps = fps;
    params->dirty = true;

    pCus_SetAEUSecs(handle, params->expo.expo_lef_us);

    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx708_params *params = (imx708_params *)handle->private_data;
    //ISensorIfAPI2 *sensor_if1 = handle->sensor_if_api2;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x0104,1); // Global hold on
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
                SensorReg_Write(0x0104,0); // Global hold off
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
    imx708_params *params = (imx708_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data & 0xff) << 8;
    lines |= (u32)(params->tExpo_reg[1].data & 0xff) << 0;

    *us = (lines * Preview_line_period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines = 0, vts = 0;
    imx708_params *params = (imx708_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000 * us) / Preview_line_period;

    if (expo_lines < 4) expo_lines = 4;
    if (params->expo.binning_mode)
    {
        expo_lines = 2*expo_lines;
    }

    if (expo_lines > params->expo.vts)
        vts = expo_lines + 48;
    else
        vts = params->expo.vts;

    params->expo.expo_lines = expo_lines;
    //params->expo.vts = vts;

    //x = us * 160 / 4296;//exp * pixel_rate / line_length_pck
    /*printk("[%s] us %u, vts %u\n", __FUNCTION__,
                                                 us,  \
                                                 vts
               );*/

    params->tExpo_reg[0].data = (unsigned char)((0xff00 & expo_lines) >> 8);
    params->tExpo_reg[1].data = (unsigned char)((0x00ff & expo_lines));
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx708_params *params = (imx708_params *)handle->private_data;
    *gain = params->expo.final_gain;

    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    imx708_params *params = (imx708_params *)handle->private_data;
    u32 Again=0, Dgain_up=1, Dgain_low=0;

    if(gain < SENSOR_MIN_GAIN) {
        gain = SENSOR_MIN_GAIN;
    } else if(gain >= SENSOR_MAX_GAIN) {
        gain = SENSOR_MAX_GAIN;
    }

    if (params->expo.binning_mode)
    {
        if(gain <= 64*1024) {
            Again = 1024 - (1024 * 1024) / gain;
            Dgain_up = 1;
            Dgain_low = 0;
        } else if (gain <= SENSOR_MAX_GAIN) {
            Again = 1008;
            Dgain_up = gain/64/1024;
            Dgain_low = (gain  - (int)Dgain_up * 64 * 1024) / 64;
        }
    }else
    {
        if(gain <= 16*1024) {
            Again = 1024 - (1024 * 1024) / gain;
            Dgain_up = 1;
            Dgain_low = 0;
        } else if (gain <= SENSOR_MAX_GAIN) {
            Again = 960;
            Dgain_up = gain/16/1024;
            Dgain_low = (gain  - (int)Dgain_up * 16 * 1024) / 64;
        }
    }

    params->expo.final_gain = gain;
    params->tGain_reg[0].data = (unsigned char)((Again >> 8) & 0xff);
    params->tGain_reg[1].data = (unsigned char)(Again & 0xff);
    params->tGain_reg[2].data = (unsigned char)(Dgain_up & 0xff);
    params->tGain_reg[3].data = (unsigned char)(Dgain_low & 0xff);
    params->dirty = true;
    return SUCCESS;
}
#if 0
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000 / imx708_mipi_linear[0].senout.min_fps;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = SENSOR_MIN_GAIN;//handle->sat_mingain;
    *max = SENSOR_MAX_GAIN;//10^(72db/20)*1024;
    return SUCCESS;
}

static int IMX708_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000 / imx708_mipi_linear[0].senout.min_fps;//
    info->min = (Preview_line_period * 1);
    info->step = Preview_line_period;
    return SUCCESS;
}
#endif
int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx708_params *params;
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

    params = (imx708_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"IMX708_MIPI_Linear");

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
        handle->video_res_supported.res[res].u16width         = imx708_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx708_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx708_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx708_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx708_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx708_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx708_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx708_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx708_mipi_linear[res].senstr.strResDesc);
    }
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000 / imx708_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
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
    handle->pCus_sensor_init           = pCus_init_11M_mipi4lane_linear;
    //handle->pCus_sensor_powerupseq     = pCus_powerupseq;
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
    handle->pCus_sensor_SetPatternMode = imx708_SetPatternMode; //NONE

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
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    //handle->pCus_sensor_GetShutterInfo  = IMX708_GetShutterInfo;

    params->expo.vts = vts_30fps;
    params->expo.binning_mode = 0;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(IMX708,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            imx708_params
                         );
