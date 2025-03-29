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
Remark        :base on master_v3 imx681
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(imx681);

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
#define SENSOR_MIPI_LANE_NUM        2//(lane_num)          //imx681 Linear mode supports MIPI 4-Lane
//#define SENSOR_MIPI_LANE_NUM_DOL    (hdr_lane_num)      //imx681 DOL mode supports MIPI 4-Lane
//#define SENSOR_MIPI_HDR_MODE        (0) //0: Non-HDR mode. 1:Sony DOL mode

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
//#define SENSOR_DATAFMT             CUS_DATAFMT_BAYER    //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9098  //CFG
#define SENSOR_BAYERID              CUS_BAYER_RG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_YCORDER              CUS_SEN_YCODR_YC     //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define long_packet_type_enable     0x00 //UD1~UD8 (user define)

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ//CUS_CMU_CLK_12MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x20                //I2C slave address
//#define SENSOR_I2C_ADDR              0x34                //I2C slave address
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
#define SENSOR_NAME                   imx681

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    /*enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_END}mode;*/
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_3, LINEAR_RES_4, LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _sensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
   // u32 nMinFrameLengthLine;
    // Show resolution string
    struct _senstr{
        const char* strResDesc;
    }senstr;
}imx681_mipi_linear[] = {
    {LINEAR_RES_1, {4032,3024, 3, 15}, {0, 0, 4032, 3024}, {"4032x3024@15fps"}},
    {LINEAR_RES_2, {3840,2160, 3, 21}, {0, 0, 3840, 2160}, {"3840x2160@21fps"}},
	{LINEAR_RES_3, {2016,1512, 3, 30}, {0, 0, 2016, 1512}, {"2016x1512@30fps"}},
	{LINEAR_RES_4, {1920,1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},
};


u32 Preview_line_period;
u32 vts_30fps;
u32 Preview_MAX_FPS;


//#define SENSOR_YCORDER      CUS_SEN_YCODR_YC       //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
//#define long_packet_type_enable 0x00 //UD1~UD8 (user define)
////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (16 * 16 * 1024)        // max sensor again, a-gain*d-gain
#define SENSOR_MAX_A_GAIN                           (16 * 1024)
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_CTRL_NUM                        (1)
#define SENSOR_SHUTTER_CTRL_NUM                     (1)

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR_FLIP_REG                             0x0101

#define SENSOR_NOR                                  0x0
#define MIRROR_EN                                   0x1 << (0)
#define FLIP_EN                                     0x1 << (1)

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif

//static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);

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
    u32 max_rhs1;
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool dirty;
    bool orien_dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tExpo_reg[3];
    u8 uSnrPad;
} imx681_params;
// set sensor ID address and data,

const static I2C_ARRAY Sensor_id_table[] =
{
    //reserved
};

/* 4032x3024@12.42 fps master mode , ResID 0 */
const static I2C_ARRAY Sensor_init_table_2lane_12m30fps[] =
{
//External Clock Setting
	{0x0136,	0x18},
	{0x0137,	0x00},
//Register version
	{0x002C,	0x02},
	{0x002D,	0x05},
//Signaling mode setting
	{0x0111,	0x02},
//Global Setting1
	{0x30EB,	0x05},
	{0x30EB,	0x0C},
	{0x300A,	0xFF},
	{0x300B,	0xFF},
	{0x3532,	0xFF},
	{0x3533,	0xFF},
//Global Setting
	{0x051E,	0x00},
	{0x0905,	0x04},
	{0x2029,	0x01},
	{0x202A,	0x11},
	{0x20A1,	0x00},
	{0x20A2,	0x02},
	{0x20A3,	0x03},
	{0x20AC,	0x01},
	{0x20AD,	0x01},
	{0x20AE,	0x01},
	{0x20AF,	0x01},
	{0x20B0,	0x00},
	{0x20B1,	0x01},
	{0x20B2,	0x02},
	{0x20B3,	0x03},
	{0x706F,	0x00},
	{0x7130,	0x08},
	{0x7131,	0x08},
	{0x7408,	0x89},
	{0x7437,	0x3D},
	{0x7439,	0x29},
	{0x7443,	0x38},
	{0x7447,	0x55},
	{0x744B,	0x00},
	{0x7451,	0x8E},
	{0x746D,	0x29},
	{0x747D,	0x68},
	{0x7481,	0x60},
	{0x7491,	0x2D},
	{0x7493,	0x31},
	{0x74A5,	0x52},
	{0x74AF,	0x4A},
	{0x74B5,	0x1F},
	{0x74B7,	0x31},
	{0x74BD,	0x75},
	{0x74C5,	0x06},
	{0x74C9,	0x52},
	{0x74D3,	0x4A},
	{0x74D9,	0x1F},
	{0x74DB,	0x31},
	{0x74E1,	0x75},
	{0x74E9,	0x06},
	{0x74ED,	0x52},
	{0x74F7,	0x4A},
	{0x74FD,	0x1F},
	{0x74FF,	0x31},
	{0x7505,	0x75},
	{0x750D,	0x06},
	{0x7537,	0x38},
	{0x753D,	0x4A},
	{0x753F,	0x4A},
	{0x7541,	0x4A},
	{0x7549,	0x8E},
	{0x754F,	0x75},
	{0x7551,	0x75},
	{0x7553,	0x75},
	{0x792B,	0x39},
	{0x792D,	0x43},
	{0x79D3,	0x25},
	{0x79D6,	0x8E},
	{0x79D7,	0x01},
	{0x79D8,	0xE7},
	{0x79D9,	0x25},
	{0x79DB,	0x76},
	{0x79DC,	0x8E},
	{0x79DD,	0x01},
	{0x79DE,	0xE7},
	{0x79DF,	0x25},
	{0x79E1,	0x76},
	{0x79E2,	0x8E},
	{0x79E3,	0x01},
	{0x79E4,	0xE7},
	{0x79E5,	0x25},
	{0x79E7,	0x76},
	{0x79E8,	0x8E},
	{0x7A01,	0xFF},
	{0x7A29,	0x6C},
	{0x7A2B,	0xDA},
	{0x7A34,	0x6C},
	{0x7A37,	0xDA},
	{0x7A40,	0x6C},
	{0x7A43,	0xDA},
	{0x7B08,	0x00},
	{0x7B09,	0x01},
	{0x7C03,	0x38},
	{0x7C09,	0x4A},
	{0x7C0B,	0x4A},
	{0x7C0D,	0x4A},
	{0x7C13,	0x8E},
	{0x7C19,	0x75},
	{0x7C1B,	0x75},
	{0x7C1D,	0x75},
	{0x7C90,	0x00},
	{0x7C91,	0x00},
	{0x7C92,	0x00},
	{0x7C9D,	0x01},
	{0x7C9E,	0x01},
	{0x7C9F,	0x01},
	{0x7E9B,	0x07},
	{0x7F09,	0x00},
	{0x7F36,	0x00},
	{0x7F4F,	0x0A},
	{0x7F50,	0x0A},
	{0x7F51,	0x0A},
	{0x7F55,	0x05},
	{0x7F56,	0x05},
	{0x7F57,	0x05},
	{0x7F5B,	0x03},
	{0x7F5C,	0x03},
	{0x7F5D,	0x03},
	{0x7F61,	0x03},
	{0x7F62,	0x03},
	{0x7F63,	0x03},
	{0x7F67,	0x03},
	{0x7F68,	0x03},
	{0x7F69,	0x03},
	{0x7F6A,	0x05},
	{0x7F6B,	0x05},
	{0x7F6C,	0x05},
	{0x7F6D,	0x11},
	{0x7F6E,	0x14},
	{0x7F6F,	0x14},
	{0x7F73,	0x14},
	{0x7F74,	0x1C},
	{0x7F75,	0x14},
	{0x7F76,	0x08},
	{0x7F79,	0x14},
	{0x7F7A,	0x1C},
	{0x7F7B,	0x14},
	{0x7F7F,	0x1C},
	{0x7F80,	0x1C},
	{0x7F81,	0x1C},
	{0x7F85,	0x1C},
	{0x7F86,	0x1C},
	{0x7F87,	0x1C},
	{0x7F9D,	0x09},
	{0x7F9E,	0x09},
	{0x7F9F,	0x09},
	{0x7FA3,	0x09},
	{0x7FA4,	0x09},
	{0x7FA5,	0x09},
	{0x7FAC,	0x00},
	{0x7FAD,	0x00},
	{0x7FAE,	0x00},
	{0x7FAF,	0x00},
	{0x7FB0,	0x00},
	{0x7FB1,	0x00},
	{0x7FB2,	0x00},
	{0x7FB3,	0x00},
	{0x7FB4,	0x00},
	{0x7FB5,	0x00},
	{0x7FB6,	0x00},
	{0x7FB7,	0x00},
	{0x7FB8,	0x00},
	{0x7FB9,	0x00},
	{0x7FBA,	0x00},
	{0x7FBB,	0x00},
	{0x7FBC,	0x00},
	{0x7FBD,	0x00},
	{0x7FBE,	0x00},
	{0x7FBF,	0x00},
	{0x7FC0,	0x00},
	{0x7FC1,	0x00},
	{0x7FC2,	0x00},
	{0x7FC3,	0x00},
	{0x7FCB,	0x37},
	{0x7FCD,	0x37},
	{0x7FCF,	0x37},
	{0x7FD7,	0x44},
	{0x7FD9,	0x44},
	{0x7FDB,	0x44},
	{0x7FDD,	0x38},
	{0x7FE3,	0x4A},
	{0x7FE5,	0x4A},
	{0x7FE7,	0x4A},
	{0x7FEF,	0x4A},
	{0x7FF1,	0x4A},
	{0x7FF3,	0x4A},
	{0x7FFB,	0x4A},
	{0x7FFD,	0x4A},
	{0x7FFF,	0x4A},
	{0x8007,	0x62},
	{0x8009,	0x62},
	{0x800B,	0x62},
	{0x8013,	0x6F},
	{0x8015,	0x6F},
	{0x8017,	0x6F},
	{0x8019,	0x8E},
	{0x801F,	0x75},
	{0x8021,	0x75},
	{0x8023,	0x75},
	{0x802B,	0x75},
	{0x802D,	0x75},
	{0x802F,	0x75},
	{0x8037,	0x75},
	{0x8039,	0x75},
	{0x803B,	0x75},
	{0x803C,	0x13},
	{0x803D,	0x17},
	{0x803E,	0x15},
	{0x803F,	0x11},
	{0x8040,	0x0A},
	{0x8041,	0x08},
	{0x8047,	0x17},
	{0x80F0,	0x24},
	{0x80F1,	0x1B},
	{0x80F2,	0x1A},
	{0x80F3,	0x14},
	{0x80F4,	0x14},
	{0x80F5,	0x12},
	{0x80F6,	0x25},
	{0x80F7,	0x1C},
	{0x80F8,	0x1B},
	{0x80F9,	0x18},
	{0x80FA,	0x17},
	{0x80FB,	0x18},
	{0x80FC,	0x26},
	{0x80FD,	0x1E},
	{0x80FE,	0x1D},
	{0x80FF,	0x1C},
	{0x8100,	0x1B},
	{0x8101,	0x1C},
	{0x8102,	0x27},
	{0x8103,	0x1E},
	{0x8104,	0x1D},
	{0x8105,	0x1E},
	{0x8106,	0x1E},
	{0x8107,	0x1E},
	{0x8108,	0x27},
	{0x8109,	0x1E},
	{0x810A,	0x1E},
	{0x810B,	0x1E},
	{0x810C,	0x1E},
	{0x810D,	0x1F},
	{0x810E,	0x00},
	{0x8168,	0x0B},
	{0x8169,	0x0B},
	{0x816A,	0x09},
	{0x816B,	0x0F},
	{0x816C,	0x0F},
	{0x816D,	0x0F},
	{0x816E,	0x0B},
	{0x816F,	0x0B},
	{0x8170,	0x0A},
	{0x8171,	0x0F},
	{0x8172,	0x0F},
	{0x8173,	0x0F},
	{0x8174,	0x0D},
	{0x8175,	0x0C},
	{0x8176,	0x09},
	{0x8177,	0x0F},
	{0x8178,	0x0F},
	{0x8179,	0x0F},
	{0x817A,	0x0C},
	{0x817B,	0x0D},
	{0x817C,	0x09},
	{0x817D,	0x0F},
	{0x817E,	0x0F},
	{0x817F,	0x0F},
	{0x8180,	0x0D},
	{0x8181,	0x0D},
	{0x8182,	0x09},
	{0x8183,	0x0F},
	{0x8184,	0x0F},
	{0x8185,	0x0F},
	{0x81B0,	0x03},
	{0x81E3,	0x04},
	{0x81E4,	0x04},
	{0x81E9,	0x04},
	{0x81EA,	0x04},
	{0x81EF,	0x04},
	{0x81F0,	0x04},
	{0x9186,	0x00},
	{0xD030,	0x01},
	{0xD04C,	0x10},
	{0xD123,	0x75},
	{0xD144,	0x10},
	{0xD1AF,	0x08},
	{0xD1BD,	0x67},
	{0xD1D4,	0x04},
	{0xD1D5,	0x04},
	{0xD1D6,	0x07},
	{0xD1D7,	0x07},
	{0xD1D9,	0x40},
	{0xD1DB,	0x58},
	{0xD1DD,	0xD4},
	{0xD1DF,	0xD4},
	{0xD1E1,	0xD4},
	{0xD348,	0x0F},
	{0xD357,	0x00},
	{0xD3AE,	0x11},
	{0xD3AF,	0x44},
	{0xD3B1,	0x7D},
	{0xD803,	0xF0},
	{0xD80B,	0xF0},
	{0xD813,	0xF1},
	{0xD81B,	0xF0},
	{0xD843,	0xF1},
	{0xD84F,	0xF0},
	{0xD934,	0x23},
	{0xD935,	0xC8},
	{0xD938,	0x27},
	{0xD939,	0x10},
	{0xD93A,	0x23},
	{0xD93B,	0xC8},
	{0xD955,	0x07},
	{0xD95A,	0x04},
	{0xD95B,	0x0A},
	{0xD95C,	0x1E},
	{0xD95D,	0x00},
	{0xD95E,	0x14},
	{0xD95F,	0x21},
	{0xD960,	0x00},
	{0xD961,	0x00},
	{0xD962,	0x0A},
	{0xD963,	0x50},
	{0xD964,	0x0A},
	{0xD965,	0xA0},
	{0xD966,	0x00},
	{0xD967,	0x28},
	{0xD968,	0x0A},
	{0xD969,	0x50},
	{0xD96A,	0x0A},
	{0xD96B,	0xA0},
	{0xD96C,	0x00},
	{0xD96D,	0x00},
	{0xD96E,	0x0A},
	{0xD96F,	0x44},
	{0xD970,	0x0A},
	{0xD971,	0x50},
	{0xD972,	0x00},
	{0xD973,	0x00},
	{0xD974,	0x0A},
	{0xD975,	0x44},
	{0xD976,	0x0A},
	{0xD977,	0x50},
	{0xDA10,	0x00},
	{0xDA11,	0x14},
	{0xDA12,	0x64},
	{0xDA13,	0x00},
	{0xDA14,	0x14},
	{0xDA15,	0xC8},
	{0xDA22,	0x00},
	{0xDA23,	0x56},
	{0xDA24,	0x00},
	{0xDA25,	0xB5},
	{0xDA26,	0x00},
	{0xDA27,	0xE8},
	{0xDA28,	0x08},
	{0xDA29,	0xA6},
	{0xDA2A,	0x00},
	{0xDA2B,	0xA2},
	{0xDA2F,	0x01},
//MIPI output setting
	{0x0110,	0x00},
	{0x0112,	0x0A},
	{0x0113,	0x0A},
	{0x0114,	0x01},
//Line Length PCK Setting
	{0x0342,	0x3E},
	{0x0343,	0x56},
//Frame Length Lines Setting
	{0x033D,	0x00},
	{0x033E,	0x0C},
	{0x033F,	0x45},
//ROI Setting
	{0x0344,	0x00},
	{0x0345,	0x08},
	{0x0346,	0x00},
	{0x0347,	0x40},
	{0x0348,	0x0F},
	{0x0349,	0xC7},
	{0x034A,	0x0C},
	{0x034B,	0x0F},
//Mode Setting
	{0x017C,	0x01},
	{0x017D,	0x01},
	{0x017E,	0x00},
	{0x017F,	0x01},
	{0x0180,	0x00},
	{0x038C,	0x13},
	{0x038D,	0x33},
	{0x2000,	0x01},
//Digital Crop & Scaling
	{0x0408,	0x00},
	{0x0409,	0x00},
	{0x040A,	0x00},
	{0x040B,	0x00},
	{0x040C,	0x0F},
	{0x040D,	0xC0},
	{0x040E,	0x0B},
	{0x040F,	0xD0},
//Output Size Setting
	{0x034C,	0x0F},
	{0x034D,	0xC0},
	{0x034E,	0x0B},
	{0x034F,	0xD0},
//Clock Setting
	{0x0301,	0x06},
	{0x0303,	0x02},
	{0x0305,	0x04},
	{0x0306,	0x01},
	{0x0307,	0x68},
	{0x030D,	0x03},
	{0x030E,	0x00},
	{0x030F,	0xFA},
	{0x0323,	0x01},
//Integration Setting
	{0x0229,	0x00},
	{0x022A,	0x0C},
	{0x022B,	0x3D},
//Gain Setting
	{0x0204,	0x00},
	{0x0205,	0x00},
	{0x020E,	0x01},
	{0x020F,	0x00},
	{0x0210,	0x01},
	{0x0211,	0x00},
	{0x0212,	0x01},
	{0x0213,	0x00},
	{0x0214,	0x01},
	{0x0215,	0x00},
//Additional Mode Setting
	{0x6A83,	0x03},
	{0x7E9B,	0x02},
	{0xD1CE,	0x00},
	{0xDC3C,	0x01},
//HiSpeed Mode Setting

	{0x0368,	0x00},
	{0x036A,	0x08},
	{0x036B,	0x70},
//MIPI Setting
	{0x0115,	0x01},
//Streaming setting
	{0x0100,	0x01},
}; //end of master mode


/* 3840x2160@12.42 fps mode , ResID 1 */
const static I2C_ARRAY Sensor_init_table_2lane_8m30fps[] =
{
//External Clock Setting
	{0x0136,	0x18},
	{0x0137,	0x00},
//Register version
	{0x002C,	0x02},
	{0x002D,	0x05},
//Signaling mode setting
	{0x0111,	0x02},
//Global Setting1
	{0x30EB,	0x05},
	{0x30EB,	0x0C},
	{0x300A,	0xFF},
	{0x300B,	0xFF},
	{0x3532,	0xFF},
	{0x3533,	0xFF},
//Global Setting
	{0x051E,	0x00},
	{0x0905,	0x04},
	{0x2029,	0x01},
	{0x202A,	0x11},
	{0x20A1,	0x00},
	{0x20A2,	0x02},
	{0x20A3,	0x03},
	{0x20AC,	0x01},
	{0x20AD,	0x01},
	{0x20AE,	0x01},
	{0x20AF,	0x01},
	{0x20B0,	0x00},
	{0x20B1,	0x01},
	{0x20B2,	0x02},
	{0x20B3,	0x03},
	{0x706F,	0x00},
	{0x7130,	0x08},
	{0x7131,	0x08},
	{0x7408,	0x89},
	{0x7437,	0x3D},
	{0x7439,	0x29},
	{0x7443,	0x38},
	{0x7447,	0x55},
	{0x744B,	0x00},
	{0x7451,	0x8E},
	{0x746D,	0x29},
	{0x747D,	0x68},
	{0x7481,	0x60},
	{0x7491,	0x2D},
	{0x7493,	0x31},
	{0x74A5,	0x52},
	{0x74AF,	0x4A},
	{0x74B5,	0x1F},
	{0x74B7,	0x31},
	{0x74BD,	0x75},
	{0x74C5,	0x06},
	{0x74C9,	0x52},
	{0x74D3,	0x4A},
	{0x74D9,	0x1F},
	{0x74DB,	0x31},
	{0x74E1,	0x75},
	{0x74E9,	0x06},
	{0x74ED,	0x52},
	{0x74F7,	0x4A},
	{0x74FD,	0x1F},
	{0x74FF,	0x31},
	{0x7505,	0x75},
	{0x750D,	0x06},
	{0x7537,	0x38},
	{0x753D,	0x4A},
	{0x753F,	0x4A},
	{0x7541,	0x4A},
	{0x7549,	0x8E},
	{0x754F,	0x75},
	{0x7551,	0x75},
	{0x7553,	0x75},
	{0x792B,	0x39},
	{0x792D,	0x43},
	{0x79D3,	0x25},
	{0x79D6,	0x8E},
	{0x79D7,	0x01},
	{0x79D8,	0xE7},
	{0x79D9,	0x25},
	{0x79DB,	0x76},
	{0x79DC,	0x8E},
	{0x79DD,	0x01},
	{0x79DE,	0xE7},
	{0x79DF,	0x25},
	{0x79E1,	0x76},
	{0x79E2,	0x8E},
	{0x79E3,	0x01},
	{0x79E4,	0xE7},
	{0x79E5,	0x25},
	{0x79E7,	0x76},
	{0x79E8,	0x8E},
	{0x7A01,	0xFF},
	{0x7A29,	0x6C},
	{0x7A2B,	0xDA},
	{0x7A34,	0x6C},
	{0x7A37,	0xDA},
	{0x7A40,	0x6C},
	{0x7A43,	0xDA},
	{0x7B08,	0x00},
	{0x7B09,	0x01},
	{0x7C03,	0x38},
	{0x7C09,	0x4A},
	{0x7C0B,	0x4A},
	{0x7C0D,	0x4A},
	{0x7C13,	0x8E},
	{0x7C19,	0x75},
	{0x7C1B,	0x75},
	{0x7C1D,	0x75},
	{0x7C90,	0x00},
	{0x7C91,	0x00},
	{0x7C92,	0x00},
	{0x7C9D,	0x01},
	{0x7C9E,	0x01},
	{0x7C9F,	0x01},
	{0x7E9B,	0x07},
	{0x7F09,	0x00},
	{0x7F36,	0x00},
	{0x7F4F,	0x0A},
	{0x7F50,	0x0A},
	{0x7F51,	0x0A},
	{0x7F55,	0x05},
	{0x7F56,	0x05},
	{0x7F57,	0x05},
	{0x7F5B,	0x03},
	{0x7F5C,	0x03},
	{0x7F5D,	0x03},
	{0x7F61,	0x03},
	{0x7F62,	0x03},
	{0x7F63,	0x03},
	{0x7F67,	0x03},
	{0x7F68,	0x03},
	{0x7F69,	0x03},
	{0x7F6A,	0x05},
	{0x7F6B,	0x05},
	{0x7F6C,	0x05},
	{0x7F6D,	0x11},
	{0x7F6E,	0x14},
	{0x7F6F,	0x14},
	{0x7F73,	0x14},
	{0x7F74,	0x1C},
	{0x7F75,	0x14},
	{0x7F76,	0x08},
	{0x7F79,	0x14},
	{0x7F7A,	0x1C},
	{0x7F7B,	0x14},
	{0x7F7F,	0x1C},
	{0x7F80,	0x1C},
	{0x7F81,	0x1C},
	{0x7F85,	0x1C},
	{0x7F86,	0x1C},
	{0x7F87,	0x1C},
	{0x7F9D,	0x09},
	{0x7F9E,	0x09},
	{0x7F9F,	0x09},
	{0x7FA3,	0x09},
	{0x7FA4,	0x09},
	{0x7FA5,	0x09},
	{0x7FAC,	0x00},
	{0x7FAD,	0x00},
	{0x7FAE,	0x00},
	{0x7FAF,	0x00},
	{0x7FB0,	0x00},
	{0x7FB1,	0x00},
	{0x7FB2,	0x00},
	{0x7FB3,	0x00},
	{0x7FB4,	0x00},
	{0x7FB5,	0x00},
	{0x7FB6,	0x00},
	{0x7FB7,	0x00},
	{0x7FB8,	0x00},
	{0x7FB9,	0x00},
	{0x7FBA,	0x00},
	{0x7FBB,	0x00},
	{0x7FBC,	0x00},
	{0x7FBD,	0x00},
	{0x7FBE,	0x00},
	{0x7FBF,	0x00},
	{0x7FC0,	0x00},
	{0x7FC1,	0x00},
	{0x7FC2,	0x00},
	{0x7FC3,	0x00},
	{0x7FCB,	0x37},
	{0x7FCD,	0x37},
	{0x7FCF,	0x37},
	{0x7FD7,	0x44},
	{0x7FD9,	0x44},
	{0x7FDB,	0x44},
	{0x7FDD,	0x38},
	{0x7FE3,	0x4A},
	{0x7FE5,	0x4A},
	{0x7FE7,	0x4A},
	{0x7FEF,	0x4A},
	{0x7FF1,	0x4A},
	{0x7FF3,	0x4A},
	{0x7FFB,	0x4A},
	{0x7FFD,	0x4A},
	{0x7FFF,	0x4A},
	{0x8007,	0x62},
	{0x8009,	0x62},
	{0x800B,	0x62},
	{0x8013,	0x6F},
	{0x8015,	0x6F},
	{0x8017,	0x6F},
	{0x8019,	0x8E},
	{0x801F,	0x75},
	{0x8021,	0x75},
	{0x8023,	0x75},
	{0x802B,	0x75},
	{0x802D,	0x75},
	{0x802F,	0x75},
	{0x8037,	0x75},
	{0x8039,	0x75},
	{0x803B,	0x75},
	{0x803C,	0x13},
	{0x803D,	0x17},
	{0x803E,	0x15},
	{0x803F,	0x11},
	{0x8040,	0x0A},
	{0x8041,	0x08},
	{0x8047,	0x17},
	{0x80F0,	0x24},
	{0x80F1,	0x1B},
	{0x80F2,	0x1A},
	{0x80F3,	0x14},
	{0x80F4,	0x14},
	{0x80F5,	0x12},
	{0x80F6,	0x25},
	{0x80F7,	0x1C},
	{0x80F8,	0x1B},
	{0x80F9,	0x18},
	{0x80FA,	0x17},
	{0x80FB,	0x18},
	{0x80FC,	0x26},
	{0x80FD,	0x1E},
	{0x80FE,	0x1D},
	{0x80FF,	0x1C},
	{0x8100,	0x1B},
	{0x8101,	0x1C},
	{0x8102,	0x27},
	{0x8103,	0x1E},
	{0x8104,	0x1D},
	{0x8105,	0x1E},
	{0x8106,	0x1E},
	{0x8107,	0x1E},
	{0x8108,	0x27},
	{0x8109,	0x1E},
	{0x810A,	0x1E},
	{0x810B,	0x1E},
	{0x810C,	0x1E},
	{0x810D,	0x1F},
	{0x810E,	0x00},
	{0x8168,	0x0B},
	{0x8169,	0x0B},
	{0x816A,	0x09},
	{0x816B,	0x0F},
	{0x816C,	0x0F},
	{0x816D,	0x0F},
	{0x816E,	0x0B},
	{0x816F,	0x0B},
	{0x8170,	0x0A},
	{0x8171,	0x0F},
	{0x8172,	0x0F},
	{0x8173,	0x0F},
	{0x8174,	0x0D},
	{0x8175,	0x0C},
	{0x8176,	0x09},
	{0x8177,	0x0F},
	{0x8178,	0x0F},
	{0x8179,	0x0F},
	{0x817A,	0x0C},
	{0x817B,	0x0D},
	{0x817C,	0x09},
	{0x817D,	0x0F},
	{0x817E,	0x0F},
	{0x817F,	0x0F},
	{0x8180,	0x0D},
	{0x8181,	0x0D},
	{0x8182,	0x09},
	{0x8183,	0x0F},
	{0x8184,	0x0F},
	{0x8185,	0x0F},
	{0x81B0,	0x03},
	{0x81E3,	0x04},
	{0x81E4,	0x04},
	{0x81E9,	0x04},
	{0x81EA,	0x04},
	{0x81EF,	0x04},
	{0x81F0,	0x04},
	{0x9186,	0x00},
	{0xD030,	0x01},
	{0xD04C,	0x10},
	{0xD123,	0x75},
	{0xD144,	0x10},
	{0xD1AF,	0x08},
	{0xD1BD,	0x67},
	{0xD1D4,	0x04},
	{0xD1D5,	0x04},
	{0xD1D6,	0x07},
	{0xD1D7,	0x07},
	{0xD1D9,	0x40},
	{0xD1DB,	0x58},
	{0xD1DD,	0xD4},
	{0xD1DF,	0xD4},
	{0xD1E1,	0xD4},
	{0xD348,	0x0F},
	{0xD357,	0x00},
	{0xD3AE,	0x11},
	{0xD3AF,	0x44},
	{0xD3B1,	0x7D},
	{0xD803,	0xF0},
	{0xD80B,	0xF0},
	{0xD813,	0xF1},
	{0xD81B,	0xF0},
	{0xD843,	0xF1},
	{0xD84F,	0xF0},
	{0xD934,	0x23},
	{0xD935,	0xC8},
	{0xD938,	0x27},
	{0xD939,	0x10},
	{0xD93A,	0x23},
	{0xD93B,	0xC8},
	{0xD955,	0x07},
	{0xD95A,	0x04},
	{0xD95B,	0x0A},
	{0xD95C,	0x1E},
	{0xD95D,	0x00},
	{0xD95E,	0x14},
	{0xD95F,	0x21},
	{0xD960,	0x00},
	{0xD961,	0x00},
	{0xD962,	0x0A},
	{0xD963,	0x50},
	{0xD964,	0x0A},
	{0xD965,	0xA0},
	{0xD966,	0x00},
	{0xD967,	0x28},
	{0xD968,	0x0A},
	{0xD969,	0x50},
	{0xD96A,	0x0A},
	{0xD96B,	0xA0},
	{0xD96C,	0x00},
	{0xD96D,	0x00},
	{0xD96E,	0x0A},
	{0xD96F,	0x44},
	{0xD970,	0x0A},
	{0xD971,	0x50},
	{0xD972,	0x00},
	{0xD973,	0x00},
	{0xD974,	0x0A},
	{0xD975,	0x44},
	{0xD976,	0x0A},
	{0xD977,	0x50},
	{0xDA10,	0x00},
	{0xDA11,	0x14},
	{0xDA12,	0x64},
	{0xDA13,	0x00},
	{0xDA14,	0x14},
	{0xDA15,	0xC8},
	{0xDA22,	0x00},
	{0xDA23,	0x56},
	{0xDA24,	0x00},
	{0xDA25,	0xB5},
	{0xDA26,	0x00},
	{0xDA27,	0xE8},
	{0xDA28,	0x08},
	{0xDA29,	0xA6},
	{0xDA2A,	0x00},
	{0xDA2B,	0xA2},
	{0xDA2F,	0x01},
//MIPI output setting
	{0x0110,	0x00},
	{0x0112,	0x0A},
	{0x0113,	0x0A},
	{0x0114,	0x01},
//Line Length PCK Setting
	{0x0342,	0x3B},
	{0x0343,	0xA2},
//Frame Length Lines Setting
	{0x033D,	0x00},
	{0x033E,	0x08},
	{0x033F,	0xC9},
//ROI Setting
	{0x0344,	0x00},
	{0x0345,	0x68},
	{0x0346,	0x01},
	{0x0347,	0xF0},
	{0x0348,	0x0F},
	{0x0349,	0x67},
	{0x034A,	0x0A},
	{0x034B,	0x5F},
//Mode Setting
	{0x017C,	0x01},
	{0x017D,	0x01},
	{0x017E,	0x00},
	{0x017F,	0x01},
	{0x0180,	0x00},
	{0x038C,	0x13},
	{0x038D,	0x33},
	{0x2000,	0x01},
//Digital Crop & Scaling
	{0x0408,	0x00},
	{0x0409,	0x00},
	{0x040A,	0x00},
	{0x040B,	0x00},
	{0x040C,	0x0F},
	{0x040D,	0x00},
	{0x040E,	0x08},
	{0x040F,	0x70},
//Output Size Setting
	{0x034C,	0x0F},
	{0x034D,	0x00},
	{0x034E,	0x08},
	{0x034F,	0x70},
//Clock Setting
	{0x0301,	0x06},
	{0x0303,	0x02},
	{0x0305,	0x04},
	{0x0306,	0x01},
	{0x0307,	0x68},
	{0x030D,	0x03},
	{0x030E,	0x00},
	{0x030F,	0xFA},
	{0x0323,	0x01},
//Integration Setting
	{0x0229,	0x00},
	{0x022A,	0x08},
	{0x022B,	0xC1},
//Gain Setting
	{0x0204,	0x00},
	{0x0205,	0x00},
	{0x020E,	0x01},
	{0x020F,	0x00},
	{0x0210,	0x01},
	{0x0211,	0x00},
	{0x0212,	0x01},
	{0x0213,	0x00},
	{0x0214,	0x01},
	{0x0215,	0x00},
//Additional Mode Setting
	{0x6A83,	0x03},
	{0x7E9B,	0x02},
	{0xD1CE,	0x00},
	{0xDC3C,	0x01},
//HiSpeed Mode Setting

	{0x0368,	0x01},
	{0x036A,	0x08},
	{0x036B,	0x70},
//MIPI Setting
	{0x0115,	0x01},
//Streaming setting
	{0x0100,	0x01},
}; //end of master mode

/* 2016x1512@30 fps , ResID 1 */
const static I2C_ARRAY Sensor_init_table_2lane_3m30fps[] =
{
//External Clock Setting
	{0x0136,	0x18},
	{0x0137,	0x00},
//Register version
	{0x002C,	0x02},
	{0x002D,	0x05},
//Signaling mode setting
	{0x0111,	0x02},
//Global Setting1
	{0x30EB,	0x05},
	{0x30EB,	0x0C},
	{0x300A,	0xFF},
	{0x300B,	0xFF},
	{0x3532,	0xFF},
	{0x3533,	0xFF},
//Global Setting
	{0x051E,	0x00},
	{0x0905,	0x04},
	{0x2029,	0x01},
	{0x202A,	0x11},
	{0x20A1,	0x00},
	{0x20A2,	0x02},
	{0x20A3,	0x03},
	{0x20AC,	0x01},
	{0x20AD,	0x01},
	{0x20AE,	0x01},
	{0x20AF,	0x01},
	{0x20B0,	0x00},
	{0x20B1,	0x01},
	{0x20B2,	0x02},
	{0x20B3,	0x03},
	{0x706F,	0x00},
	{0x7130,	0x08},
	{0x7131,	0x08},
	{0x7408,	0x89},
	{0x7437,	0x3D},
	{0x7439,	0x29},
	{0x7443,	0x38},
	{0x7447,	0x55},
	{0x744B,	0x00},
	{0x7451,	0x8E},
	{0x746D,	0x29},
	{0x747D,	0x68},
	{0x7481,	0x60},
	{0x7491,	0x2D},
	{0x7493,	0x31},
	{0x74A5,	0x52},
	{0x74AF,	0x4A},
	{0x74B5,	0x1F},
	{0x74B7,	0x31},
	{0x74BD,	0x75},
	{0x74C5,	0x06},
	{0x74C9,	0x52},
	{0x74D3,	0x4A},
	{0x74D9,	0x1F},
	{0x74DB,	0x31},
	{0x74E1,	0x75},
	{0x74E9,	0x06},
	{0x74ED,	0x52},
	{0x74F7,	0x4A},
	{0x74FD,	0x1F},
	{0x74FF,	0x31},
	{0x7505,	0x75},
	{0x750D,	0x06},
	{0x7537,	0x38},
	{0x753D,	0x4A},
	{0x753F,	0x4A},
	{0x7541,	0x4A},
	{0x7549,	0x8E},
	{0x754F,	0x75},
	{0x7551,	0x75},
	{0x7553,	0x75},
	{0x792B,	0x39},
	{0x792D,	0x43},
	{0x79D3,	0x25},
	{0x79D6,	0x8E},
	{0x79D7,	0x01},
	{0x79D8,	0xE7},
	{0x79D9,	0x25},
	{0x79DB,	0x76},
	{0x79DC,	0x8E},
	{0x79DD,	0x01},
	{0x79DE,	0xE7},
	{0x79DF,	0x25},
	{0x79E1,	0x76},
	{0x79E2,	0x8E},
	{0x79E3,	0x01},
	{0x79E4,	0xE7},
	{0x79E5,	0x25},
	{0x79E7,	0x76},
	{0x79E8,	0x8E},
	{0x7A01,	0xFF},
	{0x7A29,	0x6C},
	{0x7A2B,	0xDA},
	{0x7A34,	0x6C},
	{0x7A37,	0xDA},
	{0x7A40,	0x6C},
	{0x7A43,	0xDA},
	{0x7B08,	0x00},
	{0x7B09,	0x01},
	{0x7C03,	0x38},
	{0x7C09,	0x4A},
	{0x7C0B,	0x4A},
	{0x7C0D,	0x4A},
	{0x7C13,	0x8E},
	{0x7C19,	0x75},
	{0x7C1B,	0x75},
	{0x7C1D,	0x75},
	{0x7C90,	0x00},
	{0x7C91,	0x00},
	{0x7C92,	0x00},
	{0x7C9D,	0x01},
	{0x7C9E,	0x01},
	{0x7C9F,	0x01},
	{0x7E9B,	0x07},
	{0x7F09,	0x00},
	{0x7F36,	0x00},
	{0x7F4F,	0x0A},
	{0x7F50,	0x0A},
	{0x7F51,	0x0A},
	{0x7F55,	0x05},
	{0x7F56,	0x05},
	{0x7F57,	0x05},
	{0x7F5B,	0x03},
	{0x7F5C,	0x03},
	{0x7F5D,	0x03},
	{0x7F61,	0x03},
	{0x7F62,	0x03},
	{0x7F63,	0x03},
	{0x7F67,	0x03},
	{0x7F68,	0x03},
	{0x7F69,	0x03},
	{0x7F6A,	0x05},
	{0x7F6B,	0x05},
	{0x7F6C,	0x05},
	{0x7F6D,	0x11},
	{0x7F6E,	0x14},
	{0x7F6F,	0x14},
	{0x7F73,	0x14},
	{0x7F74,	0x1C},
	{0x7F75,	0x14},
	{0x7F76,	0x08},
	{0x7F79,	0x14},
	{0x7F7A,	0x1C},
	{0x7F7B,	0x14},
	{0x7F7F,	0x1C},
	{0x7F80,	0x1C},
	{0x7F81,	0x1C},
	{0x7F85,	0x1C},
	{0x7F86,	0x1C},
	{0x7F87,	0x1C},
	{0x7F9D,	0x09},
	{0x7F9E,	0x09},
	{0x7F9F,	0x09},
	{0x7FA3,	0x09},
	{0x7FA4,	0x09},
	{0x7FA5,	0x09},
	{0x7FAC,	0x00},
	{0x7FAD,	0x00},
	{0x7FAE,	0x00},
	{0x7FAF,	0x00},
	{0x7FB0,	0x00},
	{0x7FB1,	0x00},
	{0x7FB2,	0x00},
	{0x7FB3,	0x00},
	{0x7FB4,	0x00},
	{0x7FB5,	0x00},
	{0x7FB6,	0x00},
	{0x7FB7,	0x00},
	{0x7FB8,	0x00},
	{0x7FB9,	0x00},
	{0x7FBA,	0x00},
	{0x7FBB,	0x00},
	{0x7FBC,	0x00},
	{0x7FBD,	0x00},
	{0x7FBE,	0x00},
	{0x7FBF,	0x00},
	{0x7FC0,	0x00},
	{0x7FC1,	0x00},
	{0x7FC2,	0x00},
	{0x7FC3,	0x00},
	{0x7FCB,	0x37},
	{0x7FCD,	0x37},
	{0x7FCF,	0x37},
	{0x7FD7,	0x44},
	{0x7FD9,	0x44},
	{0x7FDB,	0x44},
	{0x7FDD,	0x38},
	{0x7FE3,	0x4A},
	{0x7FE5,	0x4A},
	{0x7FE7,	0x4A},
	{0x7FEF,	0x4A},
	{0x7FF1,	0x4A},
	{0x7FF3,	0x4A},
	{0x7FFB,	0x4A},
	{0x7FFD,	0x4A},
	{0x7FFF,	0x4A},
	{0x8007,	0x62},
	{0x8009,	0x62},
	{0x800B,	0x62},
	{0x8013,	0x6F},
	{0x8015,	0x6F},
	{0x8017,	0x6F},
	{0x8019,	0x8E},
	{0x801F,	0x75},
	{0x8021,	0x75},
	{0x8023,	0x75},
	{0x802B,	0x75},
	{0x802D,	0x75},
	{0x802F,	0x75},
	{0x8037,	0x75},
	{0x8039,	0x75},
	{0x803B,	0x75},
	{0x803C,	0x13},
	{0x803D,	0x17},
	{0x803E,	0x15},
	{0x803F,	0x11},
	{0x8040,	0x0A},
	{0x8041,	0x08},
	{0x8047,	0x17},
	{0x80F0,	0x24},
	{0x80F1,	0x1B},
	{0x80F2,	0x1A},
	{0x80F3,	0x14},
	{0x80F4,	0x14},
	{0x80F5,	0x12},
	{0x80F6,	0x25},
	{0x80F7,	0x1C},
	{0x80F8,	0x1B},
	{0x80F9,	0x18},
	{0x80FA,	0x17},
	{0x80FB,	0x18},
	{0x80FC,	0x26},
	{0x80FD,	0x1E},
	{0x80FE,	0x1D},
	{0x80FF,	0x1C},
	{0x8100,	0x1B},
	{0x8101,	0x1C},
	{0x8102,	0x27},
	{0x8103,	0x1E},
	{0x8104,	0x1D},
	{0x8105,	0x1E},
	{0x8106,	0x1E},
	{0x8107,	0x1E},
	{0x8108,	0x27},
	{0x8109,	0x1E},
	{0x810A,	0x1E},
	{0x810B,	0x1E},
	{0x810C,	0x1E},
	{0x810D,	0x1F},
	{0x810E,	0x00},
	{0x8168,	0x0B},
	{0x8169,	0x0B},
	{0x816A,	0x09},
	{0x816B,	0x0F},
	{0x816C,	0x0F},
	{0x816D,	0x0F},
	{0x816E,	0x0B},
	{0x816F,	0x0B},
	{0x8170,	0x0A},
	{0x8171,	0x0F},
	{0x8172,	0x0F},
	{0x8173,	0x0F},
	{0x8174,	0x0D},
	{0x8175,	0x0C},
	{0x8176,	0x09},
	{0x8177,	0x0F},
	{0x8178,	0x0F},
	{0x8179,	0x0F},
	{0x817A,	0x0C},
	{0x817B,	0x0D},
	{0x817C,	0x09},
	{0x817D,	0x0F},
	{0x817E,	0x0F},
	{0x817F,	0x0F},
	{0x8180,	0x0D},
	{0x8181,	0x0D},
	{0x8182,	0x09},
	{0x8183,	0x0F},
	{0x8184,	0x0F},
	{0x8185,	0x0F},
	{0x81B0,	0x03},
	{0x81E3,	0x04},
	{0x81E4,	0x04},
	{0x81E9,	0x04},
	{0x81EA,	0x04},
	{0x81EF,	0x04},
	{0x81F0,	0x04},
	{0x9186,	0x00},
	{0xD030,	0x01},
	{0xD04C,	0x10},
	{0xD123,	0x75},
	{0xD144,	0x10},
	{0xD1AF,	0x08},
	{0xD1BD,	0x67},
	{0xD1D4,	0x04},
	{0xD1D5,	0x04},
	{0xD1D6,	0x07},
	{0xD1D7,	0x07},
	{0xD1D9,	0x40},
	{0xD1DB,	0x58},
	{0xD1DD,	0xD4},
	{0xD1DF,	0xD4},
	{0xD1E1,	0xD4},
	{0xD348,	0x0F},
	{0xD357,	0x00},
	{0xD3AE,	0x11},
	{0xD3AF,	0x44},
	{0xD3B1,	0x7D},
	{0xD803,	0xF0},
	{0xD80B,	0xF0},
	{0xD813,	0xF1},
	{0xD81B,	0xF0},
	{0xD843,	0xF1},
	{0xD84F,	0xF0},
	{0xD934,	0x23},
	{0xD935,	0xC8},
	{0xD938,	0x27},
	{0xD939,	0x10},
	{0xD93A,	0x23},
	{0xD93B,	0xC8},
	{0xD955,	0x07},
	{0xD95A,	0x04},
	{0xD95B,	0x0A},
	{0xD95C,	0x1E},
	{0xD95D,	0x00},
	{0xD95E,	0x14},
	{0xD95F,	0x21},
	{0xD960,	0x00},
	{0xD961,	0x00},
	{0xD962,	0x0A},
	{0xD963,	0x50},
	{0xD964,	0x0A},
	{0xD965,	0xA0},
	{0xD966,	0x00},
	{0xD967,	0x28},
	{0xD968,	0x0A},
	{0xD969,	0x50},
	{0xD96A,	0x0A},
	{0xD96B,	0xA0},
	{0xD96C,	0x00},
	{0xD96D,	0x00},
	{0xD96E,	0x0A},
	{0xD96F,	0x44},
	{0xD970,	0x0A},
	{0xD971,	0x50},
	{0xD972,	0x00},
	{0xD973,	0x00},
	{0xD974,	0x0A},
	{0xD975,	0x44},
	{0xD976,	0x0A},
	{0xD977,	0x50},
	{0xDA10,	0x00},
	{0xDA11,	0x14},
	{0xDA12,	0x64},
	{0xDA13,	0x00},
	{0xDA14,	0x14},
	{0xDA15,	0xC8},
	{0xDA22,	0x00},
	{0xDA23,	0x56},
	{0xDA24,	0x00},
	{0xDA25,	0xB5},
	{0xDA26,	0x00},
	{0xDA27,	0xE8},
	{0xDA28,	0x08},
	{0xDA29,	0xA6},
	{0xDA2A,	0x00},
	{0xDA2B,	0xA2},
	{0xDA2F,	0x01},
//MIPI output setting
	{0x0110,	0x00},
	{0x0112,	0x0A},
	{0x0113,	0x0A},
	{0x0114,	0x01},
//Line Length PCK Setting
	{0x0342,	0x21},
	{0x0343,	0xFC},
//Frame Length Lines Setting
	{0x033D,	0x00},
	{0x033E,	0x0A},
	{0x033F,	0xC6},
//ROI Setting
	{0x0344,	0x00},
	{0x0345,	0x08},
	{0x0346,	0x00},
	{0x0347,	0x40},
	{0x0348,	0x0F},
	{0x0349,	0xC7},
	{0x034A,	0x0C},
	{0x034B,	0x0F},
//Mode Setting
	{0x017C,	0x02},
	{0x017D,	0x02},
	{0x017E,	0x00},
	{0x017F,	0x01},
	{0x0180,	0x00},
	{0x038C,	0x13},
	{0x038D,	0x33},
	{0x2000,	0x01},
//Digital Crop & Scaling
	{0x0408,	0x00},
	{0x0409,	0x00},
	{0x040A,	0x00},
	{0x040B,	0x00},
	{0x040C,	0x07},
	{0x040D,	0xE0},
	{0x040E,	0x05},
	{0x040F,	0xE8},
//Output Size Setting
	{0x034C,	0x07},
	{0x034D,	0xE0},
	{0x034E,	0x05},
	{0x034F,	0xE8},
//Clock Setting
	{0x0301,	0x06},
	{0x0303,	0x02},
	{0x0305,	0x04},
	{0x0306,	0x01},
	{0x0307,	0x68},
	{0x030D,	0x03},
	{0x030E,	0x00},
	{0x030F,	0xFA},
	{0x0323,	0x01},
//Integration Setting
	{0x0229,	0x00},
	{0x022A,	0x0A},
	{0x022B,	0xB4},
//Gain Setting
	{0x0204,	0x00},
	{0x0205,	0x00},
	{0x020E,	0x01},
	{0x020F,	0x00},
	{0x0210,	0x01},
	{0x0211,	0x00},
	{0x0212,	0x01},
	{0x0213,	0x00},
	{0x0214,	0x01},
	{0x0215,	0x00},
//Additional Mode Setting
	{0x6A83,	0x03},
	{0x7E9B,	0x06},
	{0xD1CE,	0x00},
	{0xDC3C,	0x01},
//HiSpeed Mode Setting

	{0x0368,	0x01},
	{0x036A,	0x08},
	{0x036B,	0x70},
//MIPI Setting
	{0x0115,	0x01},
//Streaming setting
	{0x0100,	0x01},
}; //end of master mode

/* 1920x1080@60 fps , ResID 1 */
const static I2C_ARRAY Sensor_init_table_2lane_2m30fps[] =
{
//External Clock Setting
	{0x0136,	0x18},
	{0x0137,	0x00},
//Register version
	{0x002C,	0x02},
	{0x002D,	0x05},
//Signaling mode setting
	{0x0111,	0x02},
//Global Setting1
	{0x30EB,	0x05},
	{0x30EB,	0x0C},
	{0x300A,	0xFF},
	{0x300B,	0xFF},
	{0x3532,	0xFF},
	{0x3533,	0xFF},
//Global Setting
	{0x051E,	0x00},
	{0x0905,	0x04},
	{0x2029,	0x01},
	{0x202A,	0x11},
	{0x20A1,	0x00},
	{0x20A2,	0x02},
	{0x20A3,	0x03},
	{0x20AC,	0x01},
	{0x20AD,	0x01},
	{0x20AE,	0x01},
	{0x20AF,	0x01},
	{0x20B0,	0x00},
	{0x20B1,	0x01},
	{0x20B2,	0x02},
	{0x20B3,	0x03},
	{0x706F,	0x00},
	{0x7130,	0x08},
	{0x7131,	0x08},
	{0x7408,	0x89},
	{0x7437,	0x3D},
	{0x7439,	0x29},
	{0x7443,	0x38},
	{0x7447,	0x55},
	{0x744B,	0x00},
	{0x7451,	0x8E},
	{0x746D,	0x29},
	{0x747D,	0x68},
	{0x7481,	0x60},
	{0x7491,	0x2D},
	{0x7493,	0x31},
	{0x74A5,	0x52},
	{0x74AF,	0x4A},
	{0x74B5,	0x1F},
	{0x74B7,	0x31},
	{0x74BD,	0x75},
	{0x74C5,	0x06},
	{0x74C9,	0x52},
	{0x74D3,	0x4A},
	{0x74D9,	0x1F},
	{0x74DB,	0x31},
	{0x74E1,	0x75},
	{0x74E9,	0x06},
	{0x74ED,	0x52},
	{0x74F7,	0x4A},
	{0x74FD,	0x1F},
	{0x74FF,	0x31},
	{0x7505,	0x75},
	{0x750D,	0x06},
	{0x7537,	0x38},
	{0x753D,	0x4A},
	{0x753F,	0x4A},
	{0x7541,	0x4A},
	{0x7549,	0x8E},
	{0x754F,	0x75},
	{0x7551,	0x75},
	{0x7553,	0x75},
	{0x792B,	0x39},
	{0x792D,	0x43},
	{0x79D3,	0x25},
	{0x79D6,	0x8E},
	{0x79D7,	0x01},
	{0x79D8,	0xE7},
	{0x79D9,	0x25},
	{0x79DB,	0x76},
	{0x79DC,	0x8E},
	{0x79DD,	0x01},
	{0x79DE,	0xE7},
	{0x79DF,	0x25},
	{0x79E1,	0x76},
	{0x79E2,	0x8E},
	{0x79E3,	0x01},
	{0x79E4,	0xE7},
	{0x79E5,	0x25},
	{0x79E7,	0x76},
	{0x79E8,	0x8E},
	{0x7A01,	0xFF},
	{0x7A29,	0x6C},
	{0x7A2B,	0xDA},
	{0x7A34,	0x6C},
	{0x7A37,	0xDA},
	{0x7A40,	0x6C},
	{0x7A43,	0xDA},
	{0x7B08,	0x00},
	{0x7B09,	0x01},
	{0x7C03,	0x38},
	{0x7C09,	0x4A},
	{0x7C0B,	0x4A},
	{0x7C0D,	0x4A},
	{0x7C13,	0x8E},
	{0x7C19,	0x75},
	{0x7C1B,	0x75},
	{0x7C1D,	0x75},
	{0x7C90,	0x00},
	{0x7C91,	0x00},
	{0x7C92,	0x00},
	{0x7C9D,	0x01},
	{0x7C9E,	0x01},
	{0x7C9F,	0x01},
	{0x7E9B,	0x07},
	{0x7F09,	0x00},
	{0x7F36,	0x00},
	{0x7F4F,	0x0A},
	{0x7F50,	0x0A},
	{0x7F51,	0x0A},
	{0x7F55,	0x05},
	{0x7F56,	0x05},
	{0x7F57,	0x05},
	{0x7F5B,	0x03},
	{0x7F5C,	0x03},
	{0x7F5D,	0x03},
	{0x7F61,	0x03},
	{0x7F62,	0x03},
	{0x7F63,	0x03},
	{0x7F67,	0x03},
	{0x7F68,	0x03},
	{0x7F69,	0x03},
	{0x7F6A,	0x05},
	{0x7F6B,	0x05},
	{0x7F6C,	0x05},
	{0x7F6D,	0x11},
	{0x7F6E,	0x14},
	{0x7F6F,	0x14},
	{0x7F73,	0x14},
	{0x7F74,	0x1C},
	{0x7F75,	0x14},
	{0x7F76,	0x08},
	{0x7F79,	0x14},
	{0x7F7A,	0x1C},
	{0x7F7B,	0x14},
	{0x7F7F,	0x1C},
	{0x7F80,	0x1C},
	{0x7F81,	0x1C},
	{0x7F85,	0x1C},
	{0x7F86,	0x1C},
	{0x7F87,	0x1C},
	{0x7F9D,	0x09},
	{0x7F9E,	0x09},
	{0x7F9F,	0x09},
	{0x7FA3,	0x09},
	{0x7FA4,	0x09},
	{0x7FA5,	0x09},
	{0x7FAC,	0x00},
	{0x7FAD,	0x00},
	{0x7FAE,	0x00},
	{0x7FAF,	0x00},
	{0x7FB0,	0x00},
	{0x7FB1,	0x00},
	{0x7FB2,	0x00},
	{0x7FB3,	0x00},
	{0x7FB4,	0x00},
	{0x7FB5,	0x00},
	{0x7FB6,	0x00},
	{0x7FB7,	0x00},
	{0x7FB8,	0x00},
	{0x7FB9,	0x00},
	{0x7FBA,	0x00},
	{0x7FBB,	0x00},
	{0x7FBC,	0x00},
	{0x7FBD,	0x00},
	{0x7FBE,	0x00},
	{0x7FBF,	0x00},
	{0x7FC0,	0x00},
	{0x7FC1,	0x00},
	{0x7FC2,	0x00},
	{0x7FC3,	0x00},
	{0x7FCB,	0x37},
	{0x7FCD,	0x37},
	{0x7FCF,	0x37},
	{0x7FD7,	0x44},
	{0x7FD9,	0x44},
	{0x7FDB,	0x44},
	{0x7FDD,	0x38},
	{0x7FE3,	0x4A},
	{0x7FE5,	0x4A},
	{0x7FE7,	0x4A},
	{0x7FEF,	0x4A},
	{0x7FF1,	0x4A},
	{0x7FF3,	0x4A},
	{0x7FFB,	0x4A},
	{0x7FFD,	0x4A},
	{0x7FFF,	0x4A},
	{0x8007,	0x62},
	{0x8009,	0x62},
	{0x800B,	0x62},
	{0x8013,	0x6F},
	{0x8015,	0x6F},
	{0x8017,	0x6F},
	{0x8019,	0x8E},
	{0x801F,	0x75},
	{0x8021,	0x75},
	{0x8023,	0x75},
	{0x802B,	0x75},
	{0x802D,	0x75},
	{0x802F,	0x75},
	{0x8037,	0x75},
	{0x8039,	0x75},
	{0x803B,	0x75},
	{0x803C,	0x13},
	{0x803D,	0x17},
	{0x803E,	0x15},
	{0x803F,	0x11},
	{0x8040,	0x0A},
	{0x8041,	0x08},
	{0x8047,	0x17},
	{0x80F0,	0x24},
	{0x80F1,	0x1B},
	{0x80F2,	0x1A},
	{0x80F3,	0x14},
	{0x80F4,	0x14},
	{0x80F5,	0x12},
	{0x80F6,	0x25},
	{0x80F7,	0x1C},
	{0x80F8,	0x1B},
	{0x80F9,	0x18},
	{0x80FA,	0x17},
	{0x80FB,	0x18},
	{0x80FC,	0x26},
	{0x80FD,	0x1E},
	{0x80FE,	0x1D},
	{0x80FF,	0x1C},
	{0x8100,	0x1B},
	{0x8101,	0x1C},
	{0x8102,	0x27},
	{0x8103,	0x1E},
	{0x8104,	0x1D},
	{0x8105,	0x1E},
	{0x8106,	0x1E},
	{0x8107,	0x1E},
	{0x8108,	0x27},
	{0x8109,	0x1E},
	{0x810A,	0x1E},
	{0x810B,	0x1E},
	{0x810C,	0x1E},
	{0x810D,	0x1F},
	{0x810E,	0x00},
	{0x8168,	0x0B},
	{0x8169,	0x0B},
	{0x816A,	0x09},
	{0x816B,	0x0F},
	{0x816C,	0x0F},
	{0x816D,	0x0F},
	{0x816E,	0x0B},
	{0x816F,	0x0B},
	{0x8170,	0x0A},
	{0x8171,	0x0F},
	{0x8172,	0x0F},
	{0x8173,	0x0F},
	{0x8174,	0x0D},
	{0x8175,	0x0C},
	{0x8176,	0x09},
	{0x8177,	0x0F},
	{0x8178,	0x0F},
	{0x8179,	0x0F},
	{0x817A,	0x0C},
	{0x817B,	0x0D},
	{0x817C,	0x09},
	{0x817D,	0x0F},
	{0x817E,	0x0F},
	{0x817F,	0x0F},
	{0x8180,	0x0D},
	{0x8181,	0x0D},
	{0x8182,	0x09},
	{0x8183,	0x0F},
	{0x8184,	0x0F},
	{0x8185,	0x0F},
	{0x81B0,	0x03},
	{0x81E3,	0x04},
	{0x81E4,	0x04},
	{0x81E9,	0x04},
	{0x81EA,	0x04},
	{0x81EF,	0x04},
	{0x81F0,	0x04},
	{0x9186,	0x00},
	{0xD030,	0x01},
	{0xD04C,	0x10},
	{0xD123,	0x75},
	{0xD144,	0x10},
	{0xD1AF,	0x08},
	{0xD1BD,	0x67},
	{0xD1D4,	0x04},
	{0xD1D5,	0x04},
	{0xD1D6,	0x07},
	{0xD1D7,	0x07},
	{0xD1D9,	0x40},
	{0xD1DB,	0x58},
	{0xD1DD,	0xD4},
	{0xD1DF,	0xD4},
	{0xD1E1,	0xD4},
	{0xD348,	0x0F},
	{0xD357,	0x00},
	{0xD3AE,	0x11},
	{0xD3AF,	0x44},
	{0xD3B1,	0x7D},
	{0xD803,	0xF0},
	{0xD80B,	0xF0},
	{0xD813,	0xF1},
	{0xD81B,	0xF0},
	{0xD843,	0xF1},
	{0xD84F,	0xF0},
	{0xD934,	0x23},
	{0xD935,	0xC8},
	{0xD938,	0x27},
	{0xD939,	0x10},
	{0xD93A,	0x23},
	{0xD93B,	0xC8},
	{0xD955,	0x07},
	{0xD95A,	0x04},
	{0xD95B,	0x0A},
	{0xD95C,	0x1E},
	{0xD95D,	0x00},
	{0xD95E,	0x14},
	{0xD95F,	0x21},
	{0xD960,	0x00},
	{0xD961,	0x00},
	{0xD962,	0x0A},
	{0xD963,	0x50},
	{0xD964,	0x0A},
	{0xD965,	0xA0},
	{0xD966,	0x00},
	{0xD967,	0x28},
	{0xD968,	0x0A},
	{0xD969,	0x50},
	{0xD96A,	0x0A},
	{0xD96B,	0xA0},
	{0xD96C,	0x00},
	{0xD96D,	0x00},
	{0xD96E,	0x0A},
	{0xD96F,	0x44},
	{0xD970,	0x0A},
	{0xD971,	0x50},
	{0xD972,	0x00},
	{0xD973,	0x00},
	{0xD974,	0x0A},
	{0xD975,	0x44},
	{0xD976,	0x0A},
	{0xD977,	0x50},
	{0xDA10,	0x00},
	{0xDA11,	0x14},
	{0xDA12,	0x64},
	{0xDA13,	0x00},
	{0xDA14,	0x14},
	{0xDA15,	0xC8},
	{0xDA22,	0x00},
	{0xDA23,	0x56},
	{0xDA24,	0x00},
	{0xDA25,	0xB5},
	{0xDA26,	0x00},
	{0xDA27,	0xE8},
	{0xDA28,	0x08},
	{0xDA29,	0xA6},
	{0xDA2A,	0x00},
	{0xDA2B,	0xA2},
	{0xDA2F,	0x01},
//MIPI output setting
	{0x0110,	0x00},
	{0x0112,	0x0A},
	{0x0113,	0x0A},
	{0x0114,	0x01},
//Line Length PCK Setting
	{0x0342,	0x21},
	{0x0343,	0xFC},
//Frame Length Lines Setting
	{0x033D,	0x00},
	{0x033E,	0x0A},
	{0x033F,	0xC6},
//ROI Setting
	{0x0344,	0x00},
	{0x0345,	0x68},
	{0x0346,	0x01},
	{0x0347,	0xF0},
	{0x0348,	0x0F},
	{0x0349,	0x67},
	{0x034A,	0x0A},
	{0x034B,	0x5F},
//Mode Setting
	{0x017C,	0x02},
	{0x017D,	0x02},
	{0x017E,	0x00},
	{0x017F,	0x01},
	{0x0180,	0x00},
	{0x038C,	0x13},
	{0x038D,	0x33},
	{0x2000,	0x01},
//Digital Crop & Scaling
	{0x0408,	0x00},
	{0x0409,	0x00},
	{0x040A,	0x00},
	{0x040B,	0x00},
	{0x040C,	0x07},
	{0x040D,	0x80},
	{0x040E,	0x04},
	{0x040F,	0x38},
//Output Size Setting
	{0x034C,	0x07},
	{0x034D,	0x80},
	{0x034E,	0x04},
	{0x034F,	0x38},
//Clock Setting
	{0x0301,	0x06},
	{0x0303,	0x02},
	{0x0305,	0x04},
	{0x0306,	0x01},
	{0x0307,	0x68},
	{0x030D,	0x03},
	{0x030E,	0x00},
	{0x030F,	0xFA},
	{0x0323,	0x01},
//Integration Setting
	{0x0229,	0x00},
	{0x022A,	0x0A},
	{0x022B,	0xB4},
//Gain Setting
	{0x0204,	0x00},
	{0x0205,	0x00},
	{0x020E,	0x01},
	{0x020F,	0x00},
	{0x0210,	0x01},
	{0x0211,	0x00},
	{0x0212,	0x01},
	{0x0213,	0x00},
	{0x0214,	0x01},
	{0x0215,	0x00},
//Additional Mode Setting
	{0x6A83,	0x03},
	{0x7E9B,	0x06},
	{0xD1CE,	0x00},
	{0xDC3C,	0x01},
//HiSpeed Mode Setting

	{0x0368,	0x01},
	{0x036A,	0x08},
	{0x036B,	0x70},
//MIPI Setting
	{0x0115,	0x01},
//Streaming setting
	{0x0100,	0x01},
};

static I2C_ARRAY PatternTbl[] = {
    {0x0000, 0x00}, //colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY expo_reg[] =
{
	{0x0229, 0x00},  //
    {0x022a, 0x05},  // bit8-15
    {0x022b, 0x48},  // bit0-7
};

const static I2C_ARRAY gain_reg[] = {
    {0x0204, 0x00}, // again bit0-1(8-10)
    {0x0205, 0x00}, // again bit0-7 low
	{0xd383, 0x00}, //register 0xd383 for dgain using
	{0x020e, 0x01}, // dgain bit0-1(8-10)
    {0x020f, 0x00}, // dgain bit0-7 low
};

const static I2C_ARRAY vts_reg[] = {
    {0x033E, 0x07},  // bit0-7(8-15)
    {0x033F, 0x53},  // bit0-7
};

//static int g_sensor_ae_min_gain = 1024;
/*static CUS_GAIN_GAP_ARRAY gain_gap_compensate[16] = {  //compensate  gain gap
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
};*/

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

/////////// function definition ///////////////////
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
#if 0
static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

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
    info->u32AEShutter_min                   = Preview_line_period * 4;
    info->u32AEShutter_step                  = Preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx681_params *param = (imx681_params*) handle->private_data;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    param->uSnrPad = idx;

    //Sensor power on sequence
    //Sensor board PWDN/RST Pull Low
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);

    //Configuration Chip RX
    pCus_PowerOn_InitChipRX(handle, idx);

    SENSOR_UDELAY(100);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    //Sensor board RST Pull High after PWDN Enable
    SENSOR_UDELAY(100);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_UDELAY(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_MDELAY(2);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    imx681_params *params = (imx681_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    params->res.orit = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// Check Sensor Product ID /////////////////////////
#if 0
static int pCus_CheckSensorProductID(ss_cus_sensor *handle)
{
    u16 sen_id_msb, sen_id_lsb, sen_data;

    /* Read Product ID */
    SensorReg_Read(0x3f12, &sen_id_lsb);
    SensorReg_Read(0x3f13, &sen_id_msb);//CHIP_ID_r3F13
    sen_data = ((sen_id_lsb & 0x0F) << 8) | (sen_id_lsb & 0xF0) | (sen_id_msb & 0x0F);
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
    if(table_length>8) table_length=8;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(n=0;n<4;++n) //retry , until I2C success
    {
      if(n>2) return FAIL;

      if(/* SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == */SUCCESS) //read sensor ID from I2C
          break;
      else
          SENSOR_MSLEEP(1);
    }

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
      if( id_from_sensor[i].data != Sensor_id_table[i].data )
        return FAIL;
      *id = id_from_sensor[i].data;
    }

    SENSOR_DMSG("[%s]imx681 sensor ,Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}
#endif

static int imx681_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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
            //MSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }
    return SUCCESS;
}

static int pCus_init_mipi2lane_8m30fps_linear(ss_cus_sensor *handle)
{
    int i,cnt=0;
    int nArraySize = 0;
    const I2C_ARRAY *pInitTable = NULL;

    switch(handle->video_res_supported.ulcur_res)
    {
    case LINEAR_RES_1: /*4032x3024@15fps master mode*/
        pInitTable = Sensor_init_table_2lane_12m30fps;
        nArraySize = ARRAY_SIZE(Sensor_init_table_2lane_12m30fps);
        break;
    case LINEAR_RES_2: /*3840x2160@21fps master mode*/
        pInitTable = Sensor_init_table_2lane_8m30fps;
        nArraySize = ARRAY_SIZE(Sensor_init_table_2lane_8m30fps);
        break;
	case LINEAR_RES_3: /*2016x1512@30fps master mode*/
        pInitTable = Sensor_init_table_2lane_3m30fps;
        nArraySize = ARRAY_SIZE(Sensor_init_table_2lane_3m30fps);
        break;
	case LINEAR_RES_4: /*1920x1080@30fps master mode*/
        pInitTable = Sensor_init_table_2lane_2m30fps;
        nArraySize = ARRAY_SIZE(Sensor_init_table_2lane_2m30fps);
        break;
    default:
        SENSOR_EMSG("imx681 res id %d out of range.\n", handle->video_res_supported.ulcur_res);
        break;
    }

    for(i=0;i< nArraySize;i++)
    {
        if(pInitTable[i].reg==0xffff)
        {
            SENSOR_MSLEEP(pInitTable[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(pInitTable[i].reg,pInitTable[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    return FAIL;
                }
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
    imx681_params *params = (imx681_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case LINEAR_RES_1: /*4032x3024 15fps master mode*/
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_8m30fps_linear;
            vts_30fps = 3141;
            Preview_MAX_FPS = imx681_mipi_linear[handle->video_res_supported.ulcur_res].senout.max_fps;
            Preview_line_period  = 22160;//16236;
            break;
        case LINEAR_RES_2: /*3840x2160 21fps master mode*/
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_mipi2lane_8m30fps_linear;
            vts_30fps = 2249;
            Preview_MAX_FPS = imx681_mipi_linear[handle->video_res_supported.ulcur_res].senout.max_fps;
            Preview_line_period  = 21203;//16236;
			break;
		case LINEAR_RES_3: /*2016x1512 30fps master mode*/
            handle->video_res_supported.ulcur_res = 2;
            handle->pCus_sensor_init = pCus_init_mipi2lane_8m30fps_linear;
            vts_30fps = 2758;
            Preview_MAX_FPS = imx681_mipi_linear[handle->video_res_supported.ulcur_res].senout.max_fps;
            Preview_line_period  = 12086;//16236;
            break;
		case LINEAR_RES_4: /*1920x1080 30fps master mode*/
            handle->video_res_supported.ulcur_res = 3;
            handle->pCus_sensor_init = pCus_init_mipi2lane_8m30fps_linear;
            vts_30fps = 2758;
            Preview_MAX_FPS = imx681_mipi_linear[handle->video_res_supported.ulcur_res].senout.max_fps;
            Preview_line_period  = 12086;//16236;
            break;
    }
    params->expo.vts = vts_30fps;
    params->expo.fps = Preview_MAX_FPS;

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    s16 sen_data;
    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP_REG, &sen_data);

    switch(sen_data)
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
    imx681_params *params = (imx681_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    s16 sen_data;
    imx681_params *params = (imx681_params *)handle->private_data;
    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP_REG, (void*)&sen_data);
    sen_data &= ~(MIRROR_EN | FLIP_EN);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            //sen_data |= SENSOR_NOR;
            params->res.orit = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M1F0:
            sen_data |= MIRROR_EN;
            params->res.orit = CUS_ORIT_M1F0;
            break;
        case CUS_ORIT_M0F1:
            sen_data |= FLIP_EN;
            params->res.orit = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            sen_data |= MIRROR_EN;
            sen_data |= FLIP_EN;
            params->res.orit = CUS_ORIT_M1F1;
            break;
        default :
            params->res.orit = CUS_ORIT_M0F0;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(MIRROR_FLIP_REG, sen_data);

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx681_params *params = (imx681_params *)handle->private_data;
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
    u32 vts = 0;
    imx681_params *params = (imx681_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);
    }else if((fps>=(min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts = (vts_30fps*(max_fps*1000) + (fps>>1))/fps;
    }else{
        //params->expo.vts=vts_30fps;
        //params->expo.fps=25;
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.expo_lines > (params->expo.vts - 18)){
        vts = params->expo.expo_lines + 18;
    }else{
        vts = params->expo.vts;
    }
    //params->expo.vts = vts;
    //pCus_SetAEUSecs(handle, params->expo.expo_lef_us);

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

    imx681_params *params = (imx681_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             //SensorReg_Write(0x0104,1);
             break;
        case CUS_FRAME_ACTIVE:

            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x0104,1);

                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));

                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x0104,0);
                params->dirty = false;
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
    imx681_params *params = (imx681_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;
    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    imx681_params *params = (imx681_params *)handle->private_data;

    params->expo.expo_lef_us = us;

    lines = (1000 * us) / Preview_line_period;
    if(lines < 4)
	{
		lines = 4;
	}

    if (lines > (params->expo.vts-18))
	{
        vts = lines + 18;
	}
    else
	{
        vts = params->expo.vts;
	}

	params->expo.expo_lines = lines;

    params->tExpo_reg[1].data = (lines>>8) & 0xff;
    params->tExpo_reg[2].data = (lines>>0) & 0xff;

    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;

	/*printk("[%s] us %u, lines %u, vts %u , Expo_reg[1] %x, Expo_reg[2] %x\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts,
				params->tExpo_reg[1].data,
				params->tExpo_reg[2].data
                );*/

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx681_params *params = (imx681_params *)handle->private_data;

	*gain=params->expo.final_gain;

    return SUCCESS;
}



static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    //extern DBG_ITEM Dbg_Items[DBG_TAG_MAX];
    imx681_params *params = (imx681_params *)handle->private_data;

    u32 gain_cal;


    if(gain < SENSOR_MIN_GAIN)
	{
        gain = SENSOR_MIN_GAIN;
	}
    else if(gain > SENSOR_MAX_GAIN)
	{
        gain = SENSOR_MAX_GAIN;
	}

	params->expo.final_gain = gain;

	if(gain <= SENSOR_MAX_A_GAIN)
	{
		gain_cal = (u32)(1024 - ((1024*1024)/gain));

		params->tGain_reg[0].data = (gain_cal >> 8) & 0xff;
		params->tGain_reg[1].data = gain_cal & 0xff;

		params->tGain_reg[2].data = 0x01; //0xd383
		params->tGain_reg[3].data = 0x01;
		params->tGain_reg[4].data = 0x00;
	}
	else
	{
		params->tGain_reg[0].data = 0x03;//max a gain = 960
		params->tGain_reg[1].data = 0xc0;

		gain_cal = ((gain/1024)*256)/(SENSOR_MAX_A_GAIN/1024);
		params->tGain_reg[2].data = 0x01; //0xd383
		params->tGain_reg[3].data = (gain_cal >> 8) & 0xff;
		params->tGain_reg[4].data = gain_cal & 0xff;
	}

	//printk("\ngain[%d] gain_cal[%d] gain0[0x%x] | gain1[0x%x], gain2[0x%x] | gain3[0x%x]\n",gain,gain_cal,params->tGain_reg[0].data,params->tGain_reg[1].data,params->tGain_reg[3].data,params->tGain_reg[4].data);

    params->dirty = true;
    return SUCCESS;
}

#if 0
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/imx681_mipi_linear[0].senout.min_fps;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = SENSOR_MIN_GAIN;//handle->sat_mingain;
    *max = SENSOR_MAX_GAIN;//3980*1024;
    return SUCCESS;
}

static int imx681_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/imx681_mipi_linear[0].senout.min_fps;
    info->min = (Preview_line_period * 4);
    info->step = Preview_line_period;
    return SUCCESS;
}

/*static int pCus_setCaliData_gain_linearity(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num) {
    u32 i, j;

    for(i=0,j=0;i< num ;i++,j+=2){
        gain_gap_compensate[i].gain=pArray[i].gain;
        gain_gap_compensate[i].offset=pArray[i].offset;
    }
    return SUCCESS;
}*/
#endif

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx681_params *params;
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

    params = (imx681_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"imx681_MIPI_Linear");

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
    handle->mclk                  = Preview_MCLK_SPEED;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());

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
        handle->video_res_supported.res[res].u16width         = imx681_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = imx681_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = imx681_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = imx681_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = imx681_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = imx681_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = imx681_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = imx681_mipi_linear[res].senout.height;
        //handle->video_res_supported.res[res].nMinFrameLengthLine = imx681_mipi_linear[res].nMinFrameLengthLine;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx681_mipi_linear[res].senstr.strResDesc);
    }
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period * 4);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx681_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ////////////////////////////////////
    //    Sensor polarity             //
    ////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    //handle->pCus_sensor_release         = cus_camsensor_release_handle;
    handle->pCus_sensor_init            = pCus_init_mipi2lane_8m30fps_linear;
    //handle->pCus_sensor_powerupseq      = pCus_powerupseq;
    handle->pCus_sensor_poweron         = pCus_poweron;
    handle->pCus_sensor_poweroff        = pCus_poweroff;
    //handle->pCus_sensor_GetSensorID     = pCus_GetSensorID;
    handle->pCus_sensor_GetVideoResNum  = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes     = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes     = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode  = imx681_SetPatternMode;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    //handle->ae_gain_delay              = SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay           = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    //handle->ae_gain_ctrl_num           = 1;
    //handle->ae_shutter_ctrl_num        = 1;
    //handle->sat_mingain                = SENSOR_MIN_GAIN;  //calibration
    //handle->dgain_remainder = 0;
    //nPixelSize
   // handle->nPixelSize                  = 1450;
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
    //handle->pCus_sensor_GetDGainRemainder = pCus_GetDGainRemainder;

    //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    //handle->pCus_sensor_GetShutterInfo = imx681_GetShutterInfo;

    params->expo.vts        = vts_30fps;
    params->expo.expo_lines = 5000;
    params->dirty           = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(imx681_HDR,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            imx681_params
                         );
