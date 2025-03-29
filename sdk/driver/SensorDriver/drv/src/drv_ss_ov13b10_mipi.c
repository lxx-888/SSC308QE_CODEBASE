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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OV13B10_LINEAR);

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
#define SENSOR_MIPI_LANE_NUM        (4)          //OV13B10 Linear mode supports MIPI 2/4 Lane
//#define SENSOR_MIPI_HDR_MODE        (0) //0: Non-HDR mode. 1:Sony DOL mode

//#define SENSOR_DATAFMT             CUS_DATAFMT_BAYER    //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
#define SENSOR_BAYERID              CUS_BAYER_BG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x20                //I2C slave address
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
#define SENSOR_NAME     OV13B10

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_3, LINEAR_RES_4, LINEAR_RES_5, LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _sensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    u32 MinFrameLengthLine;
    u32 RowTime;
    //u16 PixelMode;
    // Show resolution string
    struct _senstr{
        const char* strResDesc;
    }senstr;
}ov13b10_mipi_linear[] = {
    {LINEAR_RES_1, {4208, 3120, 3, 15}, {0, 0, 4208, 3120}, 6346, 10499, {"4208x3120@15fps"}},
    {LINEAR_RES_2, {4208, 3120, 3, 30}, {0, 0, 4208, 3120}, 3196, 10499, {"4208x3120@30fps"}},
    {LINEAR_RES_3, {3840, 2160, 3, 30}, {0, 0, 3840, 2160}, 3196, 10499, {"3840x2160@30fps"}},
    {LINEAR_RES_4, {2104, 1560, 3, 30}, {0, 0, 2104, 1560}, 3174, 10501, {"2104x1560@30fps"}},
	{LINEAR_RES_5, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, 3174, 10501, {"2104x1560@30fps"}},
};

u32 Preview_line_period = 10499;
u32 vts_30fps = 3196;

//#define SENSOR_YCORDER      CUS_SEN_YCODR_YC       //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
//#define long_packet_type_enable 0x00 //UD1~UD8 (user define)
////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (63430)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_MAX_A_GAIN                           (15.5 * 1024) //
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

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
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    struct {
        float sclk;
        u32 hts;
        u32 vts;
        u32 Max_vts;
        u32 lines;
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
    u32 max_rhs1;
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool dirty;
    bool orien_dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
    CUS_CAMSENSOR_ORIT cur_orien;
} ov13b10_params;
// set sensor ID address and data,

const static I2C_ARRAY Sensor_init_table_4lane_13m15fps_linear[] =
{
    //@@OV13B10_mclk24M_4208x3120_MIPI_4LANE_1120Mbpslane_linear_raw10_15fps_AM10_Toby.Li_20240925
    //;----------------------------------------------
    //; MCLK: 24Mhz
    //; resolution: 4208x3120
    //; Mipi : 4 lane
    //; Mipi data rate: 1120Mbps/Lane
    //; SystemCLK   :112Mhz 
    //; FPS         : 29.8fps
    //; HTS     :1176(R380c:R380d)
    //; VTS     :3196(R380e:R380f)
    //; Tline   :10.5us
    //; mipi is gated clock mode
    //; mirror/flip normal
    //;---------------------------------------------
    {0x0103, 0x01}, 
    {0xffff, 0x1f},
    {0x0303, 0x01}, 
    {0x0305, 0x46}, 
    {0x0321, 0x00}, 
    {0x0323, 0x04}, 
    {0x0324, 0x01}, 
    {0x0325, 0x50}, 
    {0x0326, 0x81}, 
    {0x0327, 0x04},
    {0x3011, 0x7c},
    {0x3012, 0x07}, 
    {0x3013, 0x32}, 
    {0x3107, 0x23}, 
    {0x3501, 0x0c}, 
    {0x3502, 0x10}, 
    {0x3504, 0x08}, 
    {0x3508, 0x07}, 
    {0x3509, 0xc0}, 
    {0x3600, 0x1a},
    {0x3601, 0x54}, 
    {0x3612, 0x4e},
    {0x3620, 0x00},
    {0x3621, 0x68}, 
    {0x3622, 0x66}, 
    {0x3623, 0x03}, 
    {0x3662, 0x92}, 
    {0x3666, 0xbb},
    {0x3667, 0x44}, 
    {0x366e, 0xff}, 
    {0x366f, 0xf3}, 
    {0x3675, 0x44}, 
    {0x3676, 0x00}, 
    {0x367f, 0xe9},
    {0x3681, 0x32},
    {0x3682, 0x1f},
    {0x3683, 0x0b},
    {0x3684, 0x0b},
    {0x3704, 0x0f}, 
    {0x3706, 0x40}, 
    {0x3708, 0x44}, 
    {0x3709, 0x72}, 
    {0x370b, 0xa2}, 
    {0x3714, 0x24}, 
    {0x371a, 0x3e}, 
    {0x3725, 0x42}, 
    {0x3739, 0x12}, 
    {0x3767, 0x00}, 
    {0x3774, 0x30}, 
    {0x377a, 0x0d},
    {0x3789, 0x18},
    {0x3790, 0x40}, 
    {0x3791, 0xa2}, 
    {0x37c2, 0x04}, 
    {0x37c3, 0xf1}, 
    {0x37d9, 0x0c}, 
    {0x37da, 0x02}, 
    {0x37dc, 0x02}, 
    {0x37e1, 0x04}, 
    {0x37e2, 0x0a}, 
    {0x3800, 0x00}, 
    {0x3801, 0x00}, 
    {0x3802, 0x00}, 
    {0x3803, 0x08}, 
    {0x3804, 0x10}, 
    {0x3805, 0x8f}, 
    {0x3806, 0x0c}, 
    {0x3807, 0x47}, 
    {0x3808, 0x10}, 
    {0x3809, 0x70}, 
    {0x380a, 0x0c}, 
    {0x380b, 0x30}, 
    {0x380c, 0x04}, 
    {0x380d, 0x98}, 
    {0x380e, 0x18},
    {0x380f, 0xca}, 
    {0x3811, 0x0f}, 
    {0x3813, 0x08}, 
    {0x3814, 0x01}, 
    {0x3815, 0x01}, 
    {0x3816, 0x01}, 
    {0x3817, 0x01},
    {0x381f, 0x08}, 
    {0x3820, 0x88}, 
    {0x3821, 0x00},
    {0x3822, 0x14}, // if would like to support long exposure mode, set r3822=0x04 
    {0x3823, 0x18}, 
    {0x3827, 0x01}, 
    {0x382e, 0xe6}, 
    {0x3c80, 0x00}, 
    {0x3c87, 0x01}, 
    {0x3c8c, 0x19}, 
    {0x3c8d, 0x1c}, 
    {0x3ca0, 0x00}, 
    {0x3ca1, 0x00}, 
    {0x3ca2, 0x00}, 
    {0x3ca3, 0x00}, 
    {0x3ca4, 0x50}, 
    {0x3ca5, 0x11}, 
    {0x3ca6, 0x01}, 
    {0x3ca7, 0x00}, 
    {0x3ca8, 0x00}, 
    {0x4008, 0x02}, 
    {0x4009, 0x0f}, 
    {0x400a, 0x01}, 
    {0x400b, 0x19},
    {0x4011, 0x21},
    {0x4017, 0x08},
    {0x4019, 0x04}, 
    {0x401a, 0x58}, 
    {0x4032, 0x1e}, 
    {0x4050, 0x02}, 
    {0x4051, 0x09}, 
    {0x405e, 0x00}, 
    {0x4066, 0x02},
    {0x4501, 0x00},
    {0x4502, 0x10}, 
    {0x4505, 0x00}, 
    {0x4800, 0x64},
    {0x481b, 0x3e},
    {0x481f, 0x30},
    {0x4825, 0x34},
    {0x4837, 0x0e}, 
    {0x484b, 0x01},
    {0x4883, 0x02},
    {0x5000, 0xff}, 
    {0x5001, 0x0f},
    {0x5045, 0x20}, 
    {0x5046, 0x20}, 
    {0x5047, 0xa4}, 
    {0x5048, 0x20}, 
    {0x5049, 0xa4},  
    {0x0100, 0x01},
    {0xffff, 0x0a},
};

const static I2C_ARRAY Sensor_init_table_4lane_13m30fps_linear[] =
{
    //@@OV13B10_mclk24M_4208x3120_MIPI_4LANE_1120Mbpslane_linear_raw10_30fps_AM10_Toby.Li_20240925
    //;----------------------------------------------
    //; MCLK: 24Mhz
    //; resolution: 4208x3120
    //; Mipi : 4 lane
    //; Mipi data rate: 1120Mbps/Lane
    //; SystemCLK   :112Mhz 
    //; FPS         : 29.8fps
    //; HTS     :1176(R380c:R380d)
    //; VTS     :3196(R380e:R380f)
    //; Tline   :10.5us
    //; mipi is gated clock mode
    //; mirror/flip normal
    //;---------------------------------------------
    {0x0103, 0x01}, 
    {0xffff, 0x1f},
    {0x0303, 0x01}, 
    {0x0305, 0x46}, 
    {0x0321, 0x00}, 
    {0x0323, 0x04}, 
    {0x0324, 0x01}, 
    {0x0325, 0x50}, 
    {0x0326, 0x81}, 
    {0x0327, 0x04},
    {0x3011, 0x7c},
    {0x3012, 0x07}, 
    {0x3013, 0x32}, 
    {0x3107, 0x23}, 
    {0x3501, 0x0c}, 
    {0x3502, 0x10}, 
    {0x3504, 0x08}, 
    {0x3508, 0x07}, 
    {0x3509, 0xc0}, 
    {0x3600, 0x1a},
    {0x3601, 0x54}, 
    {0x3612, 0x4e},
    {0x3620, 0x00},
    {0x3621, 0x68}, 
    {0x3622, 0x66}, 
    {0x3623, 0x03}, 
    {0x3662, 0x92}, 
    {0x3666, 0xbb},
    {0x3667, 0x44}, 
    {0x366e, 0xff}, 
    {0x366f, 0xf3}, 
    {0x3675, 0x44}, 
    {0x3676, 0x00}, 
    {0x367f, 0xe9},
    {0x3681, 0x32},
    {0x3682, 0x1f},
    {0x3683, 0x0b},
    {0x3684, 0x0b},
    {0x3704, 0x0f}, 
    {0x3706, 0x40}, 
    {0x3708, 0x44}, 
    {0x3709, 0x72}, 
    {0x370b, 0xa2}, 
    {0x3714, 0x24}, 
    {0x371a, 0x3e}, 
    {0x3725, 0x42}, 
    {0x3739, 0x12}, 
    {0x3767, 0x00}, 
    {0x3774, 0x30}, 
    {0x377a, 0x0d},
    {0x3789, 0x18},
    {0x3790, 0x40}, 
    {0x3791, 0xa2}, 
    {0x37c2, 0x04}, 
    {0x37c3, 0xf1}, 
    {0x37d9, 0x0c}, 
    {0x37da, 0x02}, 
    {0x37dc, 0x02}, 
    {0x37e1, 0x04}, 
    {0x37e2, 0x0a}, 
    {0x3800, 0x00}, 
    {0x3801, 0x00}, 
    {0x3802, 0x00}, 
    {0x3803, 0x08}, 
    {0x3804, 0x10}, 
    {0x3805, 0x8f}, 
    {0x3806, 0x0c}, 
    {0x3807, 0x47}, 
    {0x3808, 0x10}, 
    {0x3809, 0x70}, 
    {0x380a, 0x0c}, 
    {0x380b, 0x30}, 
    {0x380c, 0x04}, 
    {0x380d, 0x98}, 
    {0x380e, 0x0c},
    {0x380f, 0x7c}, 
    {0x3811, 0x0f}, 
    {0x3813, 0x08}, 
    {0x3814, 0x01}, 
    {0x3815, 0x01}, 
    {0x3816, 0x01}, 
    {0x3817, 0x01},
    {0x381f, 0x08}, 
    {0x3820, 0x88}, 
    {0x3821, 0x00},
    {0x3822, 0x14}, // if would like to support long exposure mode, set r3822=0x04 
    {0x3823, 0x18}, 
    {0x3827, 0x01}, 
    {0x382e, 0xe6}, 
    {0x3c80, 0x00}, 
    {0x3c87, 0x01}, 
    {0x3c8c, 0x19}, 
    {0x3c8d, 0x1c}, 
    {0x3ca0, 0x00}, 
    {0x3ca1, 0x00}, 
    {0x3ca2, 0x00}, 
    {0x3ca3, 0x00}, 
    {0x3ca4, 0x50}, 
    {0x3ca5, 0x11}, 
    {0x3ca6, 0x01}, 
    {0x3ca7, 0x00}, 
    {0x3ca8, 0x00}, 
    {0x4008, 0x02}, 
    {0x4009, 0x0f}, 
    {0x400a, 0x01}, 
    {0x400b, 0x19},
    {0x4011, 0x21},
    {0x4017, 0x08},
    {0x4019, 0x04}, 
    {0x401a, 0x58}, 
    {0x4032, 0x1e}, 
    {0x4050, 0x02}, 
    {0x4051, 0x09}, 
    {0x405e, 0x00}, 
    {0x4066, 0x02},
    {0x4501, 0x00},
    {0x4502, 0x10}, 
    {0x4505, 0x00}, 
    {0x4800, 0x64},
    {0x481b, 0x3e},
    {0x481f, 0x30},
    {0x4825, 0x34},
    {0x4837, 0x0e}, 
    {0x484b, 0x01},
    {0x4883, 0x02},
    {0x5000, 0xff}, 
    {0x5001, 0x0f},
    {0x5045, 0x20}, 
    {0x5046, 0x20}, 
    {0x5047, 0xa4}, 
    {0x5048, 0x20}, 
    {0x5049, 0xa4},  
    {0x0100, 0x01},
    {0xffff, 0x0a},
};

const static I2C_ARRAY Sensor_init_table_4lane_8m30fps_linear[] =
{
    //@@ OV13B10_mclk24M_3840x2160_MIPI_4LANE_1120Mbpslane_linear_raw10_30fps_AM10_Toby.Li_20240926
    //;----------------------------------------------
    //; MCLK: 24Mhz
    //; resolution: 3840x2160
    //; Mipi : 4 lane
    //; Mipi data rate: 1120Mbps/Lane
    //; SystemCLK   :112Mhz 
    //; FPS         : 29.8fps
    //; HTS     :1176(R380c:R380d)
    //; VTS     :3196(R380e:R380f)
    //; Tline   :10.5us
    //; mipi is gated clock mode
    //; mirror/flip normal
    //;---------------------------------------------
    {0x0103, 0x01}, 
    {0xffff, 0x1f},
    {0x0303, 0x01}, 
    {0x0305, 0x46}, 
    {0x0321, 0x00}, 
    {0x0323, 0x04}, 
    {0x0324, 0x01}, 
    {0x0325, 0x50}, 
    {0x0326, 0x81}, 
    {0x0327, 0x04},
    {0x3011, 0x7c},
    {0x3012, 0x07}, 
    {0x3013, 0x32}, 
    {0x3107, 0x23}, 
    {0x3501, 0x0c}, 
    {0x3502, 0x10}, 
    {0x3504, 0x08}, 
    {0x3508, 0x07}, 
    {0x3509, 0xc0}, 
    {0x3600, 0x1a},
    {0x3601, 0x54}, 
    {0x3612, 0x4e},
    {0x3620, 0x00},
    {0x3621, 0x68},
    {0x3622, 0x66},
    {0x3623, 0x03},
    {0x3662, 0x92}, 
    {0x3666, 0xbb},
    {0x3667, 0x44}, 
    {0x366e, 0xff}, 
    {0x366f, 0xf3}, 
    {0x3675, 0x44}, 
    {0x3676, 0x00}, 
    {0x367f, 0xe9},
    {0x3681, 0x32},
    {0x3682, 0x1f},
    {0x3683, 0x0b},
    {0x3684, 0x0b}, 
    {0x3704, 0x0f}, 
    {0x3706, 0x40}, 
    {0x3708, 0x44}, 
    {0x3709, 0x72}, 
    {0x370b, 0xa2}, 
    {0x3714, 0x24}, 
    {0x371a, 0x3e}, 
    {0x3725, 0x42}, 
    {0x3739, 0x12}, 
    {0x3767, 0x00}, 
    {0x3774, 0x30}, 
    {0x377a, 0x0d},
    {0x3789, 0x18},
    {0x3790, 0x40}, 
    {0x3791, 0xa2}, 
    {0x37c2, 0x04}, 
    {0x37c3, 0xf1}, 
    {0x37d9, 0x0c}, 
    {0x37da, 0x02}, 
    {0x37dc, 0x02}, 
    {0x37e1, 0x04}, 
    {0x37e2, 0x0a}, 
    {0x3800, 0x00}, 
    {0x3801, 0xc0}, 
    {0x3802, 0x01}, 
    {0x3803, 0xe0}, 
    {0x3804, 0x0f}, 
    {0x3805, 0xcf}, 
    {0x3806, 0x0a}, 
    {0x3807, 0x6f}, 
    {0x3808, 0x0f}, 
    {0x3809, 0x00}, 
    {0x380a, 0x08}, 
    {0x380b, 0x70}, 
    {0x380c, 0x04}, 
    {0x380d, 0x98}, 
    {0x380e, 0x0c}, 
    {0x380f, 0x7c}, 
    {0x3811, 0x07},  
    {0x3813, 0x10},  
    {0x3814, 0x01}, 
    {0x3815, 0x01}, 
    {0x3816, 0x01}, 
    {0x3817, 0x01},
    {0x381f, 0x08}, 
    {0x3820, 0x88}, 
    {0x3821, 0x00},
    {0x3822, 0x14}, // if would like to support long exposure mode, set r3822=0x04 
    {0x3823, 0x18}, 
    {0x3827, 0x01}, 
    {0x382e, 0xe6}, 
    {0x3c80, 0x00}, 
    {0x3c87, 0x01}, 
    {0x3c8c, 0x19}, 
    {0x3c8d, 0x1c}, 
    {0x3ca0, 0x00}, 
    {0x3ca1, 0x00}, 
    {0x3ca2, 0x00}, 
    {0x3ca3, 0x00}, 
    {0x3ca4, 0x50}, 
    {0x3ca5, 0x11}, 
    {0x3ca6, 0x01}, 
    {0x3ca7, 0x00}, 
    {0x3ca8, 0x00}, 
    {0x4008, 0x02}, 
    {0x4009, 0x0f}, 
    {0x400a, 0x01}, 
    {0x400b, 0x19},
    {0x4011, 0x21},
    {0x4017, 0x08},
    {0x4019, 0x04}, 
    {0x401a, 0x58}, 
    {0x4032, 0x1e}, 
    {0x4050, 0x02}, 
    {0x4051, 0x09}, 
    {0x405e, 0x00}, 
    {0x4066, 0x02},
    {0x4501, 0x00}, 
    {0x4502, 0x10}, 
    {0x4505, 0x00}, 
    {0x4800, 0x64},
    {0x481b, 0x3e},
    {0x481f, 0x30},
    {0x4825, 0x34},
    {0x4837, 0x0e},
    {0x484b, 0x01}, 
    {0x4883, 0x02},
    {0x5000, 0xff}, 
    {0x5001, 0x0f},
    {0x5045, 0x20}, 
    {0x5046, 0x20}, 
    {0x5047, 0xa4}, 
    {0x5048, 0x20}, 
    {0x5049, 0xa4},  
    {0x0100, 0x01},
    {0xffff, 0x0a},
};

const static I2C_ARRAY Sensor_init_table_4lane_3m30fps_linear[] =
{
    //@@OV13B10_mclk24M_2104x1560_MIPI_4LANE_560Mbpslane_linear_raw10_30fps_AM10_Toby.Li_20240927
    //;----------------------------------------------
    //; MCLK: 24Mhz
    //; resolution: 2104x1560
    //; Mipi : 4 lane
    //; Mipi data rate: 560Mbps/Lane
    //; SystemCLK   :112Mhz 
    //; FPS         : 29.8fps
    //; HTS     :1176(R380c:R380d)
    //; VTS     :3174(R380e:R380f)
    //; Tline   :10.5us
    //; mipi is gated clock mode
    //; mirror/flip normal
    //;---------------------------------------------
    {0x0103, 0x01}, 
    {0xffff, 0x1f},
    {0x0303, 0x01}, 
    {0x0305, 0x23}, 
    {0x0321, 0x00}, 
    {0x0323, 0x04}, 
    {0x0324, 0x01}, 
    {0x0325, 0x50}, 
    {0x0326, 0x81}, 
    {0x0327, 0x04},
    {0x3011, 0x7c},
    {0x3012, 0x07}, 
    {0x3013, 0x32}, 
    {0x3107, 0x23}, 
    {0x3501, 0x06}, 
    {0x3502, 0x10}, 
    {0x3504, 0x08}, 
    {0x3508, 0x07}, 
    {0x3509, 0xc0}, 
    {0x3600, 0x1a}, 
    {0x3601, 0x54}, 
    {0x3612, 0x4e},  
    {0x3620, 0x00},
    {0x3621, 0x68},
    {0x3622, 0x66},
    {0x3623, 0x03}, 
    {0x3662, 0x88}, 
    {0x3666, 0xbb},
    {0x3667, 0x44}, 
    {0x366e, 0xff}, 
    {0x366f, 0xf3}, 
    {0x3675, 0x44}, 
    {0x3676, 0x00}, 
    {0x367f, 0xe9},
    {0x3681, 0x32},
    {0x3682, 0x1f},
    {0x3683, 0x0b},
    {0x3684, 0x0b},
    {0x3704, 0x0f}, 
    {0x3706, 0x40}, 
    {0x3708, 0x44}, 
    {0x3709, 0x72}, 
    {0x370b, 0xa2}, 
    {0x3714, 0x28}, 
    {0x371a, 0x3e}, 
    {0x3725, 0x42}, 
    {0x3739, 0x10}, 
    {0x3767, 0x00}, 
    {0x3774, 0x30}, 
    {0x377a, 0x0d},
    {0x3789, 0x18}, 
    {0x3790, 0x40}, 
    {0x3791, 0xa2}, 
    {0x37c2, 0x14}, 
    {0x37c3, 0xf1}, 
    {0x37d9, 0x06}, 
    {0x37da, 0x02}, 
    {0x37dc, 0x02}, 
    {0x37e1, 0x04}, 
    {0x37e2, 0x0c},
    {0x37e4, 0x00},
    {0x3800, 0x00}, 
    {0x3801, 0x00}, 
    {0x3802, 0x00}, 
    {0x3803, 0x08}, 
    {0x3804, 0x10}, 
    {0x3805, 0x8f}, 
    {0x3806, 0x0c}, 
    {0x3807, 0x47}, 
    {0x3808, 0x08}, 
    {0x3809, 0x38}, 
    {0x380a, 0x06}, 
    {0x380b, 0x18}, 
    {0x380c, 0x04}, 
    {0x380d, 0x98}, 
    {0x380e, 0x0c},
    {0x380f, 0x66}, 
    {0x3811, 0x07}, 
    {0x3813, 0x04}, 
    {0x3814, 0x03}, 
    {0x3815, 0x01}, 
    {0x3816, 0x03}, 
    {0x3817, 0x01},
    {0x381f, 0x08}, 
    {0x3820, 0x8b}, 
    {0x3821, 0x00},
    {0x3822, 0x14}, //if would like to support long exposure mode, set r3822=0x04 
    {0x3823, 0x18}, 
    {0x3827, 0x01}, 
    {0x382e, 0xe6}, 
    {0x3c80, 0x00}, 
    {0x3c87, 0x01}, 
    {0x3c8c, 0x18}, 
    {0x3c8d, 0x1c}, 
    {0x3ca0, 0x00}, 
    {0x3ca1, 0x00}, 
    {0x3ca2, 0x00}, 
    {0x3ca3, 0x00}, 
    {0x3ca4, 0x50}, 
    {0x3ca5, 0x11}, 
    {0x3ca6, 0x01}, 
    {0x3ca7, 0x00}, 
    {0x3ca8, 0x00}, 
    {0x4008, 0x00}, 
    {0x4009, 0x05}, 
    {0x400a, 0x01}, 
    {0x400b, 0x19},
    {0x4011, 0x21},
    {0x4017, 0x08},
    {0x4019, 0x04}, 
    {0x401a, 0x58}, 
    {0x4032, 0x1e},
    {0x4050, 0x00}, 
    {0x4051, 0x05}, 
    {0x405e, 0x00}, 
    {0x4066, 0x02}, 
    {0x4501, 0x08},
    {0x4502, 0x10},
    {0x4505, 0x04}, 
    {0x4800, 0x64},
    {0x481b, 0x3e},
    {0x481f, 0x30},
    {0x4825, 0x34},
    {0x4837, 0x1d},
    {0x484b, 0x01},
    {0x4883, 0x02},
    {0x5000, 0xfd}, 
    {0x5001, 0x0d},
    {0x5045, 0x20}, 
    {0x5046, 0x20}, 
    {0x5047, 0xa4}, 
    {0x5048, 0x20}, 
    {0x5049, 0xa4},  
    {0x0100, 0x01},
    {0xffff, 0x0a},
};

const static I2C_ARRAY Sensor_init_table_4lane_2m30fps_linear[] =
{
	//@@OV13B10_mclk24M_1920x1080_MIPI_4LANE_560Mbpslane_linear_raw10_30fps_AM10_Toby.Li_20250214
	//;----------------------------------------------
	//; MCLK: 24Mhz
	//; resolution: 1920x1080
	//; Mipi : 4 lane
	//; Mipi data rate: 560Mbps/Lane
	//; SystemCLK   :112Mhz 
	//; FPS	        : 30.01fps
	//; HTS		:1176(R380c:R380d)
	//; VTS		:3174(R380e:R380f)
	//; Tline 	:10.5us
	//; mipi is gated clock mode
	//; mirror/flip normal
	//;---------------------------------------------
	{0x0103, 0x01}, // 
	{0xffff, 0x1f}, 
	{0x0303, 0x01}, // 
	{0x0305, 0x23}, // 
	{0x0321, 0x00}, // 
	{0x0323, 0x04}, // 
	{0x0324, 0x01}, // 
	{0x0325, 0x50}, // 
	{0x0326, 0x81}, // 
	{0x0327, 0x04}, //
	{0x3011, 0x7c}, //
	{0x3012, 0x07}, // 
	{0x3013, 0x32}, // 
	{0x3107, 0x23}, // 
	{0x3501, 0x06}, // 
	{0x3502, 0x00}, // 
	{0x3504, 0x08}, // 
	{0x3508, 0x07}, // 
	{0x3509, 0xc0}, // 
	{0x3600, 0x1a}, // ; 
	{0x3601, 0x54}, // 
	{0x3612, 0x4e}, //  
	{0x3620, 0x00}, //
	{0x3621, 0x68}, //
	{0x3622, 0x66}, //
	{0x3623, 0x03}, //
	{0x3662, 0x88}, // 
	{0x3666, 0xbb}, //
	{0x3667, 0x44}, // 
	{0x366e, 0xff}, // 
	{0x366f, 0xf3}, // 
	{0x3675, 0x44}, // 
	{0x3676, 0x00}, // 
	{0x367f, 0xe9}, //
	{0x3681, 0x32}, //
	{0x3682, 0x1f}, //
	{0x3683, 0x0b}, //
	{0x3684, 0x0b}, //
	{0x3704, 0x0f}, // 
	{0x3706, 0x40}, // 
	{0x3708, 0x44}, // ;  
	{0x3709, 0x72}, // 
	{0x370b, 0xa2}, // 
	{0x3714, 0x28}, // 
	{0x371a, 0x3e}, // 
	{0x3725, 0x42}, // 
	{0x3739, 0x10}, // 
	{0x3767, 0x00}, // 
	{0x3774, 0x30}, // ;
	{0x377a, 0x0d}, //
	{0x3789, 0x18}, // 
	{0x3790, 0x40}, // 
	{0x3791, 0xa2}, // 
	{0x37c2, 0x14}, // 
	{0x37c3, 0xf1}, //  
	{0x37d9, 0x06}, // 
	{0x37da, 0x02}, // 
	{0x37dc, 0x02}, // 
	{0x37e1, 0x04}, // 
	{0x37e2, 0x0c}, //
	{0x37e4, 0x00}, //
	{0x3800, 0x00}, // 
	{0x3801, 0xb0}, // 
	{0x3802, 0x01}, // 
	{0x3803, 0xe0}, // 
	{0x3804, 0x0f}, // 
	{0x3805, 0xdf}, // 
	{0x3806, 0x0a}, // 
	{0x3807, 0x6f}, // 
	{0x3808, 0x07}, // 
	{0x3809, 0x80}, // 
	{0x380a, 0x04}, // 
	{0x380b, 0x38}, // 
	{0x380c, 0x04}, // 
	{0x380d, 0x98}, // 
	{0x380e, 0x0c}, //
	{0x380f, 0x66}, //  
	{0x3811, 0x0b}, // 
	{0x3813, 0x08}, // 
	{0x3814, 0x03}, // 
	{0x3815, 0x01}, // 
	{0x3816, 0x03}, // 
	{0x3817, 0x01}, //
	{0x381f, 0x08}, //  
	{0x3820, 0x8b}, // 
	{0x3821, 0x00}, //
	{0x3822, 0x14}, // ; if would like to support long exposure mode, set r3822=0x04 
	{0x3823, 0x18}, // ;
	{0x3827, 0x01}, // ;
	{0x382e, 0xe6}, // 
	{0x3c80, 0x00}, // 
	{0x3c87, 0x01}, // 
	{0x3c8c, 0x18}, // 
	{0x3c8d, 0x1c}, // 
	{0x3ca0, 0x00}, // 
	{0x3ca1, 0x00}, // 
	{0x3ca2, 0x00}, // 
	{0x3ca3, 0x00}, // 
	{0x3ca4, 0x50}, // 
	{0x3ca5, 0x11}, // 
	{0x3ca6, 0x01}, // 
	{0x3ca7, 0x00}, // 
	{0x3ca8, 0x00}, // 
	{0x4008, 0x00}, // 
	{0x4009, 0x05}, // 
	{0x400a, 0x01}, // 
	{0x400b, 0x19}, //
	{0x4011, 0x21}, //
	{0x4017, 0x08}, //
	{0x4019, 0x04}, // 
	{0x401a, 0x58}, // 
	{0x4032, 0x1e}, //
	{0x4050, 0x00}, // 
	{0x4051, 0x05}, // 
	{0x405e, 0x00}, // 
	{0x4066, 0x02}, // 
	{0x4501, 0x08}, //
	{0x4502, 0x10}, //
	{0x4505, 0x04}, // 
	{0x4800, 0x64}, //
	{0x481b, 0x3e}, //
	{0x481f, 0x30}, //
	{0x4825, 0x34}, //
	{0x4837, 0x1d}, //
	{0x484b, 0x01}, //
	{0x4883, 0x02}, //
	{0x5000, 0xfd}, // 
	{0x5001, 0x0d}, //
	{0x5045, 0x20}, // 
	{0x5046, 0x20}, // 
	{0x5047, 0xa4}, // 
	{0x5048, 0x20}, // 
	{0x5049, 0xa4}, // 
	{0x0100, 0x01}, //
	{0xffff, 0x0a},
};

static I2C_ARRAY PatternTbl[] = {
    {0x5080,0x00}, //colorbar pattern , bit 0 to enable , bit[3:2] color bar style
};

const static I2C_ARRAY mirror_reg[] = {
    {0x3820, 0x88},//[3] mirror , [4] flip
};

const static I2C_ARRAY gain_reg[] = {
    {0x3508, 0x01},//long a-gain[14:8] bit [6:0]
    {0x3509, 0x00},//long a-gain[7:1] bit[7:1]
    {0x350a, 0x01},//dig_gain_coarse_b [3:0]
    {0x350b, 0x00},//dig_gain_fine_b [9:2]
    {0x350c, 0x00},//dig_gain_fine_b [1:0]
};

const static I2C_ARRAY expo_reg[] = {
    {0x3500, 0x00},//long exp[23,16]
    {0x3501, 0x02},//long exp[15,8]
    {0x3502, 0x00},//long exp[7,0]
};

const static I2C_ARRAY vts_reg[] = {
    {0x380e, 0x0c},
    {0x380f, 0x7c},
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

/////////// function definition ///////////////////
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
/******* Support MCLK List*******
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

static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period * 9;
    info->u32AEShutter_step                  = Preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    //Sensor board PWDN Enable, 1.8V & 2.9V need 30ms then Pull High
    SENSOR_MDELAY(10);// SENSOR_MSLEEP(31);
    sensor_if->Reset(idx,CUS_CLK_POL_POS);
    SENSOR_MDELAY(5);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_MDELAY(10);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    ov13b10_params *params = (ov13b10_params *)handle->private_data;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    params->cur_orien = SENSOR_ORIT;

    return SUCCESS;
}

//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL

static int ov13b10_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    int i;

    for(i = 0; i < ARRAY_SIZE(PatternTbl); i++)
    {
        if(SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
        {
            //MSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }
    return SUCCESS;

}

static int pCus_init_mipi4lane_13m15fps_linear(ss_cus_sensor *handle)
{
    int i,cnt = 0;
    
    SENSOR_DMSG("\n[%s]", __FUNCTION__);

    for(i = 0; i< ARRAY_SIZE(Sensor_init_table_4lane_13m15fps_linear); i++)
    {
        if(Sensor_init_table_4lane_13m15fps_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_13m15fps_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_13m15fps_linear[i].reg,Sensor_init_table_4lane_13m15fps_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt >= 10)
                {
                    return FAIL;
                }
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_mipi4lane_13m30fps_linear(ss_cus_sensor *handle)
{
    int i,cnt = 0;
    
    SENSOR_DMSG("\n[%s]", __FUNCTION__);

    for(i = 0; i< ARRAY_SIZE(Sensor_init_table_4lane_13m30fps_linear); i++)
    {
        if(Sensor_init_table_4lane_13m30fps_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_13m30fps_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_13m30fps_linear[i].reg,Sensor_init_table_4lane_13m30fps_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt >= 10)
                {
                    return FAIL;
                }
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_mipi4lane_8m30fps_linear(ss_cus_sensor *handle)
{
    int i, cnt = 0;
    
    SENSOR_DMSG("\n[%s]", __FUNCTION__);

    for(i = 0; i < ARRAY_SIZE(Sensor_init_table_4lane_8m30fps_linear); i++)
    {
        if(Sensor_init_table_4lane_8m30fps_linear[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_8m30fps_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_8m30fps_linear[i].reg,Sensor_init_table_4lane_8m30fps_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt >= 10)
                {
                    return FAIL;
                }
                //usleep(10*1000);
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_mipi4lane_3m30fps_linear(ss_cus_sensor *handle)
{
    int i, cnt = 0;
    
    SENSOR_DMSG("\n[%s]", __FUNCTION__);

    for(i = 0; i < ARRAY_SIZE(Sensor_init_table_4lane_3m30fps_linear); i++)
    {
        if(Sensor_init_table_4lane_3m30fps_linear[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_3m30fps_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_3m30fps_linear[i].reg,Sensor_init_table_4lane_3m30fps_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt >= 10)
                {
                    return FAIL;
                }
                //usleep(10*1000);
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_mipi4lane_2m30fps_linear(ss_cus_sensor *handle)
{
    int i, cnt = 0;
    
    SENSOR_DMSG("\n[%s]", __FUNCTION__);

    for(i = 0; i < ARRAY_SIZE(Sensor_init_table_4lane_2m30fps_linear); i++)
    {
        if(Sensor_init_table_4lane_2m30fps_linear[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane_2m30fps_linear[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane_2m30fps_linear[i].reg,Sensor_init_table_4lane_2m30fps_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt >= 10)
                {
                    return FAIL;
                }
                //usleep(10*1000);
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
    ov13b10_params *params = (ov13b10_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }

    switch (res_idx) {
        case LINEAR_RES_1:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi4lane_13m15fps_linear;
            params->expo.Max_vts = 6346;
            Preview_line_period = 10499;
            break;
        case LINEAR_RES_2:
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_mipi4lane_13m30fps_linear;
            params->expo.Max_vts = 3196;
            Preview_line_period = 10499;
            break;
        case LINEAR_RES_3:
            handle->video_res_supported.ulcur_res = 2;
            handle->pCus_sensor_init = pCus_init_mipi4lane_8m30fps_linear;
            params->expo.Max_vts = 3196;
            Preview_line_period = 10499;
            break;
        case LINEAR_RES_4:
            handle->video_res_supported.ulcur_res = 3;
            handle->pCus_sensor_init = pCus_init_mipi4lane_3m30fps_linear;
            params->expo.Max_vts = 3174;
            Preview_line_period = 10501;
            break;
		case LINEAR_RES_5:
            handle->video_res_supported.ulcur_res = 4;
            handle->pCus_sensor_init = pCus_init_mipi4lane_2m30fps_linear;
            params->expo.Max_vts = 3174;
            Preview_line_period = 10501;
            break;
        default:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi4lane_13m15fps_linear;
            params->expo.Max_vts = 6346;
            Preview_line_period = 10499;
            break;
    }

    handle->video_res_supported.res[res_idx].u16MinFrameLengthLine = params->expo.Max_vts;
    handle->video_res_supported.res[res_idx].u16RowTime = Preview_line_period;
    params->expo.vts = params->expo.Max_vts;
    params->expo.fps = ov13b10_mipi_linear[res_idx].senout.max_fps;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    ov13b10_params *params = (ov13b10_params *)handle->private_data;

    *orit = params->cur_orien;

    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    ov13b10_params *params = (ov13b10_params *)handle->private_data;

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[0].data = 0x88;
            params->orien_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[0].data = 0x80;
            params->orien_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[0].data = 0x98;
            params->orien_dirty = true;
            break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[0].data = 0x90;
            params->orien_dirty = true;
            break;
        default :
            break;
    }
    params->orien_dirty = true;
    params->cur_orien = orit;
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    ov13b10_params *params = (ov13b10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (params->expo.Max_vts*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (params->expo.Max_vts*max_fps)/tVts;

    return params->expo.preview_fps;

}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts = 0;
    ov13b10_params *params = (ov13b10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>= min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts =  (params->expo.Max_vts * max_fps) / fps;
    }else if(fps >= (min_fps * 1000) && fps <= (max_fps * 1000)){
        params->expo.fps = fps;
        params->expo.vts = (params->expo.Max_vts * (max_fps * 1000)) / fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if ((params->expo.lines) > (params->expo.vts - 16))
        vts = params->expo.lines + 16;
    else
        vts = params->expo.vts;

    handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16MinFrameLengthLine = vts;

    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    printk("380ereg = %02x \n", params->tVts_reg[0].data);
    printk("380freg = %02x \n", params->tVts_reg[1].data);
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
    ov13b10_params *params = (ov13b10_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty) {
                SensorReg_Write(0x3208, 0x00);
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorReg_Write(0x3208, 0x10);
                SensorReg_Write(0x3208, 0xa0);
                params->dirty = false;
            }
            if(params->orien_dirty) {
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orien_dirty = false;
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
    ov13b10_params *params = (ov13b10_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xff)<<0;

    *us = (lines*Preview_line_period) / 1000;

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    ov13b10_params *params = (ov13b10_params *)handle->private_data;

    lines = (1000 * us) / Preview_line_period;
    if (lines > params->expo.vts - 4)
        vts = lines + 4;
    else
        vts = params->expo.vts;
    
    params->expo.lines = lines;

    /*printk(" us %d, lines %d, vts %d\n",
                us,
                lines,
                vts
                );*/
    // lines <<= 4;
    params->tExpo_reg[0].data = (lines>>16) & 0x00ff;
    params->tExpo_reg[1].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[2].data = (lines>>0) & 0x00ff;


    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;

    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    ov13b10_params *params = (ov13b10_params *)handle->private_data;
    
    *gain = params->expo.final_gain;
    
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    //extern DBG_ITEM Dbg_Items[DBG_TAG_MAX];
    ov13b10_params *params = (ov13b10_params *)handle->private_data;
    u32 input_gain = 0;
    u16 gain16 = 0, tmp_dgain = 1024;

    SENSOR_DMSG("\n\n[%s], gain %u \n", __FUNCTION__,gain);

    if (gain < 1024)
        gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    input_gain = gain;
    params->expo.final_gain = gain;
    
    if(gain < 1024)
        gain = 1024;
    else if(gain >= SENSOR_MAX_A_GAIN)
        gain = SENSOR_MAX_A_GAIN;

    gain16=(u16)(gain >> 2);
    params->tGain_reg[0].data = (gain16 >> 8) & 0x0f;//high bit
    params->tGain_reg[1].data = gain16 & 0xfe; //low byte

    if(input_gain > SENSOR_MAX_A_GAIN)
    {
        tmp_dgain = ((input_gain*1024)/gain);
        params->tGain_reg[2].data = (u16)(tmp_dgain >> 10) & 0x0f;
        params->tGain_reg[3].data = (u16)(tmp_dgain >> 2) & 0xff;
        params->tGain_reg[4].data = (u16)((tmp_dgain & 0x03) << 6);
    }
    else
    {
        params->tGain_reg[2].data=0x01;
        params->tGain_reg[3].data=0x00;
        params->tGain_reg[4].data=0x00;
    }

    SENSOR_DMSG("\n\n[%s], gain %u, dig_gain %u, a_gain %u \n", __FUNCTION__,gain, dig_gain, a_gain);
    params->dirty = true;
    return SUCCESS;

}


int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    ov13b10_params *params;
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

    params = (ov13b10_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));


    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OV13B10_MIPI");

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
    handle->mclk                    = Preview_MCLK_SPEED;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //handle->data_fmt              = SENSOR_DATAFMT;
    handle->sif_bus               = SENSOR_IFBUS_TYPE;
    handle->data_prec             = SENSOR_DATAPREC;
    handle->bayer_id              = SENSOR_BAYERID;
    handle->RGBIR_id              = SENSOR_RGBIRID;
    //handle->YC_ODER               = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num    = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order   = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode    = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
   //handle->video_res_supported.num_res = LINEAR_RES_END;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = ov13b10_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = ov13b10_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = ov13b10_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = ov13b10_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = ov13b10_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = ov13b10_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = ov13b10_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = ov13b10_mipi_linear[res].senout.height;
		handle->video_res_supported.res[res].u16MinFrameLengthLine =ov13b10_mipi_linear[res].MinFrameLengthLine;
		handle->video_res_supported.res[res].u16RowTime =ov13b10_mipi_linear[res].RowTime;
        sprintf(handle->video_res_supported.res[res].strResDesc, ov13b10_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    handle->pCus_sensor_init            = pCus_init_mipi4lane_13m30fps_linear;
    //handle->pCus_sensor_powerupseq      = pCus_powerupseq;
    handle->pCus_sensor_poweron         = pCus_poweron;
    handle->pCus_sensor_poweroff        = pCus_poweroff;
    handle->pCus_sensor_GetVideoResNum  = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes     = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes     = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode  = ov13b10_SetPatternMode;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period * 2);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000 / ov13b10_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;

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

    params->cur_orien                = SENSOR_ORIT;
    params->res.orit                = SENSOR_ORIT;
    params->expo.vts        = vts_30fps;
    params->expo.expo_lines = 1000;
    params->dirty           = false;

    return SUCCESS;
}


SENSOR_DRV_ENTRY_IMPL_END_EX(OV13B10_LINEAR,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            ov13b10_params
                         );
