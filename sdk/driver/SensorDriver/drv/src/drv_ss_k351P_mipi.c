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

SENSOR_DRV_ENTRY_IMPL_BEGIN(K351P_HDR);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

//#define _DEBUG_
//c11 extern int usleep(u32 usec);
//int usleep(u32 usec);

#define SENSOR_CHANNEL_NUM (0)
//#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
//#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR
#define SENSOR_HDR_MODE             CUS_HDR_MODE_VC

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (4)
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
//                                                                                                    ��//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

//#define SENSOR_DATAFMT      CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_FOOTER_EDGE//PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

//#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
//#define long_packet_type_enable 0x00 //UD1~UD8 (user define)

#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period 15873                   // 33333/2100
#define Preview_line_period_hdr 15873               // 33333/2100
//#define Preview_line_period_hdr 7937               // 33333/2100
#define vts_30fps  2100

#define Preview_WIDTH       2000                    //resolution Width when preview
#define Preview_HEIGHT      2000                    //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     5                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR    0x80                   //I2C slave address
#define SENSOR_I2C_SPEED   200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A8D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS

#define K351P_HDR
#ifdef K351P_HDR
#define SENSOR_CHANNEL_MODE_HDR CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR
#define vts_30fps_hdr  2100
#define Preview_MAX_FPS_HDR     30                     //fastest preview FPS
#define Preview_MIN_FPS_HDR     5                      //slowest preview FPS
#endif

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             31744   //31*1024
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_GAIN_CTRL_NUM                        (1)
#define SENSOR_SHUTTER_CTRL_NUM                     (1)
#define SENSOR_GAIN_CTRL_NUM_HDR                    (1)
#define SENSOR_SHUTTER_CTRL_NUM_HDR                 (2)

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
CUS_MCLK_FREQ UseParaMclk(void);

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
        u32 sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 a_gain;//cch123
        u32 back_pv_us;
        u32 fps;
        u32 line;
        u32 sef_shutter;
    } expo;

    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool dirty;//same as lef
    bool orient_dirty;
    I2C_ARRAY tGain_reg[1];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tExpo_reg_hdr_sef[1];
} K351P_params;
// set sensor ID address and data,

static I2C_ARRAY Sensor_init_table[] =
{
    /*
    [SensorType]
    0=K351P#36-2000x2000x30fps-4Lmipi-20240105
    
    [K351P#36-2000x2000x30fps-4Lmipi-20240105]
    InitREGdef=INI_Register
    IniINFOdef=INI_Category
    IniVersion=20191104
    Project=144 ;K351P
    Width=2000
    Height=2000
    disWidth=2000
    disHeight=2000
    FrameWidth=2400
    FrameHeight=2100
    H_pad=0
    V_pad=0
    SrOutputFormats=1
    Interface=4
    HDR_Mode=0
    MclkRate=24.000
    DVPClkRate=37.80000
    SensorLineMode=0
    MipiImgVc=0
    MipiImgVc1=1
    LVDSformat=0
    PixelRate=151.20000     ;SysClk = 37.80000
    MipiClkRate=189.00000
    
    [INI_Category]
    FPS=30.00000
    DefaultReg_Ver=AB
    Start=K351_Start_2022111500.soi
    PLL=K351_PLL_2023011300.soi
    DAC=K351P_DAC_2023072505.soi
    Timing=K351P_Timing_2023120501.soi
    Interface=K351P_Interface_2023082205.soi
    Sampling=K351P_Sampling_2023112000.soi
    Power=K351P_Power_2023102600.soi
    AE=K351P_AE_2023112000.soi
    DisMode=K351_DisMode_2023011200.soi
    HDR=
    Fast_Boot=
    ISP=
    Customer=
    End=K351_End_2022111800.soi
    FT=
    Factory=
    CP_Factory=
    CP=
    
    ;PC:600222571
    */
    //[INI_Register]
    {0x12,0x40},
    {0xAD,0x01},
    {0xAD,0x00},
    {0x0E,0x13},
    {0x0F,0x0C},
    {0x10,0x3F},
    {0x0C,0x00},
    {0x67,0x91},
    {0x0D,0x10},
    {0x64,0x31},
    {0x65,0x9D},
    {0xBE,0x18},
    {0xBF,0x60},
    {0xBC,0xC0},
    {0x20,0x2C},
    {0x21,0x01},
    {0x22,0x34},
    {0x23,0x08},
    {0x24,0xF4},
    {0x25,0xD0},
    {0x26,0x71},
    {0x27,0x10},
    {0x28,0x0D},
    {0x29,0x00},
    {0x2B,0x10},
    {0x2C,0x00},
    {0x2D,0x06},
    {0x2E,0xFB},
    {0x2F,0x14},
    {0x30,0xF8},
    {0x87,0xC5},
    {0x9D,0xB9},
    {0xAC,0x00},
    {0x1D,0x00},
    {0x1E,0x10},
    {0x3A,0x49},
    {0x3B,0x2D},
    {0x3C,0x29},
    {0x3D,0x25},
    {0x3E,0x10},
    {0x3F,0x24},
    {0x42,0x12},
    {0x43,0x00},
    {0x70,0xA0},
    {0x71,0x24},
    {0x76,0x08},
    {0x06,0x00},
    {0x08,0x04},
    {0x9F,0x4C},
    {0x7E,0x0B},
    {0x31,0x08},
    {0x32,0x04},
    {0x33,0xCC},
    {0x38,0xCA},
    {0x6F,0x00},
    {0x78,0x5F},
    {0xB0,0x14},
    {0xB1,0xA0},
    {0xB2,0x24},
    {0xB3,0x2A},
    {0xB5,0x50},
    {0xB6,0x57},
    {0xB8,0x06},
    {0xB9,0x08},
    {0xBA,0x8B},
    {0xBB,0x8E},
    {0xC3,0x90},
    {0xF9,0x00},
    {0x56,0xF1},
    {0x57,0x40},
    {0x58,0x42},
    {0x59,0x66},
    {0x5A,0x80},
    {0x5B,0x10},
    {0x5C,0x10},
    {0x5D,0x49},
    {0x60,0x40},
    {0x61,0x00},
    {0x62,0x60},
    {0x68,0x00},
    {0x69,0x90},
    {0xA5,0x08},
    {0xAA,0x00},
    {0xC1,0xC0},
    {0xC4,0x00},
    {0xD4,0xFF},
    {0xEB,0x15},
    {0xEC,0x03},
    {0xE1,0xF2},
    {0x80,0x81},
    {0x81,0x44},
    {0xFB,0x20},
    {0xFC,0x32},
    {0xFA,0x01},
    {0x16,0xFF},
    {0x17,0x08},
    {0x49,0x10},
    {0x85,0x00},
    {0xB4,0x01},
    {0xD2,0x80},
    {0xD0,0x00},
    {0xD3,0x22},
    {0x39,0x8A},
    {0xFF,0x01},
    {0x74,0x04},
    {0xFF,0x00},
    {0x89,0x00},
    {0x12,0x00},
    
    //;PC:2107093303

};

#ifdef K351P_HDR
static I2C_ARRAY Sensor_init_table_hdr[] =
{
    
    /*
    [SensorType]
    0=K351P#31-2000x2000x30fpsHDR-4Lmipi-20240105
    
    [K351P#31-2000x2000x30fpsHDR-4Lmipi-20240105]
    InitREGdef=INI_Register
    IniINFOdef=INI_Category
    IniVersion=20191104
    Project=144 ;K351P
    Width=2000
    Height=2000
    disWidth=2000
    disHeight=2000
    FrameWidth=2400
    FrameHeight=2100
    H_pad=0
    V_pad=0
    SrOutputFormats=1
    Interface=4
    HDR_Mode=3
    MclkRate=24.000
    DVPClkRate=75.60000
    SensorLineMode=0
    MipiImgVc=0
    MipiImgVc1=1
    LVDSformat=0
    PixelRate=302.40000     ;SysClk = 75.60000
    MipiClkRate=378.00000
    
    [INI_Category]
    FPS=30.00000
    DefaultReg_Ver=AB
    Start=K351_Start_2022111500.soi
    PLL=K351_PLL_2022111501.soi
    DAC=K351P_DAC_2023072505.soi
    Timing=K351P_Timing_2023120602.soi
    Interface=K351P_Interface_2023082204.soi
    Sampling=K351P_Sampling_2023112000.soi
    Power=K351P_Power_2023102600.soi
    AE=K351P_AE_2023112000.soi
    DisMode=K351_DisMode_2023011200.soi
    HDR=K351P_HDR_2023080303.soi
    Fast_Boot=
    ISP=
    Customer=
    End=K351_End_2022111800.soi
    FT=
    Factory=
    CP_Factory=
    CP=
    
    ;PC:1606664246
    */
    //[INI_Register]
    {0x12,0x48},
    {0xAD,0x01},
    {0xAD,0x00},
    {0x0E,0x11},
    {0x0F,0x0C},
    {0x10,0x3F},
    {0x0C,0x00},
    {0x67,0xA2},
    {0x0D,0x20},
    {0x64,0x31},
    {0x65,0x9D},
    {0xBE,0x18},
    {0xBF,0x60},
    {0xBC,0xC0},
    {0x20,0x96},
    {0x21,0x00},
    {0x22,0x34},
    {0x23,0x08},
    {0x24,0xF4},
    {0x25,0xD0},
    {0x26,0x71},
    {0x27,0x10},
    {0x28,0x0C},
    {0x29,0x00},
    {0x2B,0x10},
    {0x2C,0x00},
    {0x2D,0x06},
    {0x2E,0xFB},
    {0x2F,0x14},
    {0x30,0xF8},
    {0x87,0xC5},
    {0x9D,0xB9},
    {0xAC,0x00},
    {0x1D,0x00},
    {0x1E,0x10},
    {0x3A,0xD5},
    {0x3B,0x9B},
    {0x3C,0x6D},
    {0x3D,0x59},
    {0x3E,0x10},
    {0x3F,0x24},
    {0x42,0x2E},
    {0x43,0x00},
    {0x70,0xA0},
    {0x71,0x24},
    {0x76,0x08},
    {0x06,0x00},
    {0x08,0x04},
    {0x9F,0x4C},
    {0x7E,0x0B},
    {0x31,0x08},
    {0x32,0x04},
    {0x33,0xCC},
    {0x38,0xCA},
    {0x6F,0x00},
    {0x78,0x5F},
    {0xB0,0x14},
    {0xB1,0xA0},
    {0xB2,0x24},
    {0xB3,0x2A},
    {0xB5,0x50},
    {0xB6,0x57},
    {0xB8,0x06},
    {0xB9,0x08},
    {0xBA,0x8B},
    {0xBB,0x8E},
    {0xC3,0x90},
    {0xF9,0x00},
    {0x56,0xF1},
    {0x57,0x40},
    {0x58,0x42},
    {0x59,0x66},
    {0x5A,0x80},
    {0x5B,0x10},
    {0x5C,0x10},
    {0x5D,0x49},
    {0x60,0x40},
    {0x61,0x00},
    {0x62,0x60},
    {0x68,0x00},
    {0x69,0x90},
    {0xA5,0x08},
    {0xAA,0x00},
    {0xC1,0xC0},
    {0xC4,0x00},
    {0xD4,0xFF},
    {0xEB,0x15},
    {0xEC,0x03},
    {0xE1,0xF2},
    {0x80,0x81},
    {0x81,0x44},
    {0xFB,0x20},
    {0xFC,0x32},
    {0xFA,0x01},
    {0x16,0xFF},
    {0x17,0x08},
    {0x49,0x10},
    {0x85,0x00},
    {0xB4,0x01},
    {0xD2,0x80},
    {0xD0,0x00},
    {0xD3,0x22},
    {0x39,0x8A},
    {0xFF,0x01},
    {0x74,0x04},
    {0xFF,0x00},
    {0x1B,0x07},
    {0x05,0x07},
    {0x7D,0x21},
    {0xA4,0x19},
    {0x88,0x08},
    {0x37,0x44},
    {0x44,0x64},
    {0x04,0xC0},
    {0x6F,0x00},
    {0x89,0x00},
    {0x12,0x08},
    
    //;PC:922370439

};
#endif

/////////////////////////////////////////////////////////////////
//       @@@@@@                                                                                    //
//                 @@                                                                                    //
//             @@@                                                                                      //
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

static I2C_ARRAY mirr_flip_table[] =
{
    {0x12, 0x20}, // mirror bit[5] flip bit[4]
};

static I2C_ARRAY mirr_flip_table_hdr[] =
{
    {0x12, 0x20}, // mirror bit[5]
};

const I2C_ARRAY gain_reg[] = {
    {0x00, 0x00},
};


const I2C_ARRAY expo_reg[] = {
    {0x02, 0x00}, //long expo[15:8] texp=expo[15:0]*Tline
    {0x01, 0xff}, //long expo[7:0]
};

const I2C_ARRAY expo_reg_sef[] = {
    {0x03, 0x00}, //short expo[7:0]
    {0x04, 0xC0}, //short expo[11:8]
};

const I2C_ARRAY vts_reg[] = {
    {0x23,0x08},
    {0x22,0x34},    //frame H 2100
};

int gain_table[] = {
    1024,  //1
    1088,  //1.0625
    1152,  //1.125
    1216,  //1.1875
    1280,  //1.25
    1344,  //1.3125
    1408,  //1.375
    1472,  //1.4375
    1536,  //1.5
    1600,  //1.5625
    1664,  //1.625
    1728,  //1.6875
    1792,  //1.75
    1856,  //1.8125
    1920,  //1.875
    1984,  //1.9375
    2048,  //2
    2176,  //2.125
    2304,  //2.25
    2432,  //2.375
    2560,  //2.5
    2688,  //2.625
    2816,  //2.75
    2944,  //2.875
    3072,  //3
    3200,  //3.125
    3328,  //3.25
    3456,  //3.375
    3584,  //3.5
    3712,  //3.625
    3840,  //3.75
    3968,  //3.875
    4096,  //4
    4352,  //4.25
    4608,  //4.5
    4864,  //4.75
    5120,  //5
    5376,  //5.25
    5632,  //5.5
    5888,  //5.75
    6144,  //6
    6400,  //6.25
    6656,  //6.5
    6912,  //6.75
    7168,  //7
    7424,  //7.25
    7680,  //7.5
    7936,  //7.75
    8192,  //8
    8704,  //8.5
    9216,  //9
    9728,  //9.5
    10240, //10
    10752, //10.5
    11264, //11
    11776, //11.5
    12288, //12
    12800, //12.5
    13312, //13
    13824, //13.5
    14336, //14
    14848, //14.5
    15360, //15
    15872, //15.5
    16384, //16
    17408, //17
    18432, //18
    19456, //19
    20480, //20
    21504, //21
    22528, //22
    23552, //23
    24576, //24
    25600, //25
    26624, //26
    27648, //27
    28672, //28
    29696, //29
    30720, //30
    31744, //31
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
#define SENSOR_NAME K351P


//#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
//#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
//#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))
//#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))


//static u32 timeGetTimeU(void)
//{
//    CamOsTimespec_t tRes;
//    CamOsGetMonotonicTime(&tRes);
//    return (tRes.nSec * 1000000)+(tRes.nNanoSec/1000);
//}
//static u32 TStart = 0;
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
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
        info->u32AEShutter_step                  = Preview_line_period_hdr;
        info->u32AEShutter_min                   = Preview_line_period_hdr;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;//&handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //TStart = timeGetTimeU();
    /*PAD and CSI*/
//    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
//    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);   ///???
//    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);  ///???
//    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);    //=========   ????

//  if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_DCG) {
//      sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, CUS_HDR_MODE_DCG);
//    }

#if 1
        /*Power ON*/
        //sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
        //CamOsMsSleep(5);
    
        /*Reset PIN*/
        SENSOR_EMSG("[%s] reset low\n", __FUNCTION__);
        sensor_if->Reset(idx, CUS_CLK_POL_NEG);
        CamOsMsSleep(10);
        sensor_if->Reset(idx, CUS_CLK_POL_POS);
        CamOsMsSleep(1);
    //    sensor_if->PowerOff(idx, handle->pwdn_POLARITY);

        /*MCLK ON*/
        sensor_if->MCLK(idx, 1, handle->mclk);
        SENSOR_EMSG("[%s] reset high\n", __FUNCTION__);
        CamOsMsSleep(1);
        sensor_if->Reset(idx, CUS_CLK_POL_NEG);
        CamOsMsSleep(11);

        /*Reset PIN*/
        sensor_if->Reset(idx, CUS_CLK_POL_POS);
        CamOsMsSleep(1);
    //    sensor_if->PowerOff(idx, handle->pwdn_POLARITY);
        //sensor_if->Reset(idx, CUS_CLK_POL_POS);
        //CamOsMsSleep(2);
    
        //sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
        //CamOsMsSleep(2);
        //CamOsPrintf("pCus_poweron = %d us \n",timeGetTimeU()-TStart);
    
        sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);   ///???
        sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);  ///???
        sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);    //=========   ????
    
        if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_DCG) {
                    sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, CUS_HDR_MODE_DCG);
        }
#endif
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;//&handle->sensor_if_api;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    //handle->i2c_bus->i2c_close(handle->i2c_bus);
    //CamOsMsSleep(20);
    //Set_csi_if(0, 0);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == SENSOR_HDR_MODE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    CamOsMsSleep(50);
    return SUCCESS;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
//static int pCus_SetAEGain_cal(ss_cus_sensor *handle, u32 gain);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init(ss_cus_sensor *handle)
{
    int i,cnt=0;
    K351P_params *params = (K351P_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    //ISensorIfAPI *sensor_if = &(handle->sensor_if_api);
    //sensor_if->PCLK(NULL,CUS_PCLK_MIPI_TOP);
    //TStart = timeGetTimeU();

        for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
        {
            if(Sensor_init_table[i].reg==0xffff)
            {
                CamOsMsSleep(Sensor_init_table[i].data);
            }
            else
            {
                cnt = 0;
                while(SensorReg_Write(Sensor_init_table[i].reg,Sensor_init_table[i].data) != SUCCESS)
                {
                    cnt++;
                    //SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                    if(cnt>=10)
                    {
                        //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                        return FAIL;
                    }
                    CamOsMsSleep(10);
                }
            }
        }

    //CamOsPrintf("pCus_init = %d us \n",timeGetTimeU()-TStart);
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;

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
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init;
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;

    sen_data = mirr_flip_table[0].data & 0x30;
    SENSOR_DMSG("\n\n[%s]:mirror:%x\r\n\n\n\n",__FUNCTION__, sen_data);
    switch(sen_data) {
        case 0x00:
        *orit = CUS_ORIT_M0F0;
        break;
        case 0x20:
        *orit = CUS_ORIT_M1F0;
        break;
        case 0x10:
        *orit = CUS_ORIT_M0F1;
        break;
        case 0x30:
        *orit = CUS_ORIT_M1F1;
        break;
    }

    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    K351P_params *params = (K351P_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    params->res.orit = orit;
    params->orient_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    K351P_params *params = (K351P_params *)handle->private_data;
    int table_length = ARRAY_SIZE(mirr_flip_table);
    int i;
    u16 sen_data=0;
    #define bit5 (0x01 << 5) //1=mirror 0=normal
    #define bit4 (0x01 << 4) //1=flip   0=normal

    SensorReg_Read(mirr_flip_table[0].reg, &sen_data);

     switch(orit) {
         case CUS_ORIT_M0F0:
            sen_data &= ~bit5;
            sen_data &= ~bit4;
            params->res.orit = CUS_ORIT_M0F0;
         break;

         case CUS_ORIT_M1F0:
            sen_data |= bit5;
            sen_data &= ~bit4;
            params->res.orit = CUS_ORIT_M1F0;
         break;

         case CUS_ORIT_M0F1:
            sen_data &= ~bit5;
            sen_data |= bit4;
            params->res.orit = CUS_ORIT_M0F1;
         break;

         case CUS_ORIT_M1F1:
            sen_data |= bit5;
            sen_data |= bit4;
            params->res.orit = CUS_ORIT_M1F1;
         break;
     }
    mirr_flip_table[0].data = sen_data;

    for(i=0;i<table_length;i++){
        SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
    }
     SENSOR_DMSG("pCus_SetOrien:%x,%x\r\n", orit,mirr_flip_table[0].data);
     return SUCCESS;

}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    K351P_params *params = (K351P_params *)handle->private_data;
    //SENSOR_DMSG("[%s]", __FUNCTION__);

    return  params->expo.fps;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    int vts=0;
    K351P_params *params = (K351P_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s][%d]", __FUNCTION__, fps);

    if(fps>=5 && fps <= 30){
      params->expo.fps = fps;
      params->expo.vts=  (vts_30fps*30)/fps;
    }else if(fps>=5000 && fps <= 30000){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*30000)/fps;
    }else{
      //params->expo.vts=vts_30fps;
      //params->expo.fps=30;
      //SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }

    if ((params->expo.line) > (params->expo.vts)-4) {
        vts = params->expo.line + 4;
    }else
        vts = params->expo.vts;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx,CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    K351P_params *params = (K351P_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->dirty)
        {

            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            //SENSOR_DMSG("[%s] gain:%x, exp:%x%x\n", "Linear:", params->tGain_reg[0], params->tExpo_reg[0], params->tExpo_reg[1]);
            params->dirty = false;
        }
        if(params->orient_dirty)
        {
            DoOrien(handle, params->res.orit);
            params->orient_dirty = false;
        }
        break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    K351P_params *params = (K351P_params *)handle->private_data;
    u32 lines = 0;

    lines  = (u32)(params->tExpo_reg[1].data&0xff);
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;

    *us = (lines*Preview_line_period)/1000;
    //SENSOR_DMSG("====================================================\n");
    //SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);
    //SENSOR_DMSG("====================================================\n");

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_GetAEUSecs_hdr(ss_cus_sensor *handle, u32 *us) {
    K351P_params *params = (K351P_params *)handle->private_data;
    u32 lines = 0;

    lines  = (u32)(params->tExpo_reg[0].data&0xff);
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;

    *us = (lines*Preview_line_period_hdr)/1000;
    //SENSOR_DMSG("====================================================\n");
    //SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);
    //SENSOR_DMSG("====================================================\n");

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    K351P_params *params = (K351P_params *)handle->private_data;
    SENSOR_DMSG("pCus_SetAEUSecs[%s] val=[%d]\n", __FUNCTION__,us);

    lines=(1000*us)/Preview_line_period; // Preview_line_period in ns
    if(lines<1)
    {
        lines=1;
    }

    if (lines > params->expo.vts-10)
    {
        vts = lines+10;
    } else
    {
        vts=params->expo.vts;
    }

    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->tExpo_reg[0].data = (lines>>8) & 0xff;
    params->tExpo_reg[1].data = (lines>>0) & 0xff;
    params->dirty = true;
    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld [0]=0x%x [1]=0x%x\n", __FUNCTION__, us, lines, params->expo.vts,params->tExpo_reg[0].data,params->tExpo_reg[1].data);
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    K351P_params *params = (K351P_params *)handle->private_data;

    *gain = params->expo.final_gain;
    SENSOR_DMSG("[%s] gain/reg [%ld]\n", __FUNCTION__,*gain);

    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    K351P_params *params = (K351P_params *)handle->private_data;
    int i=0;
    SENSOR_DMSG("[%s] pCus_SetAEGain:%ld\n", __FUNCTION__, gain);
    params->expo.final_gain = gain;

    if (gain < SENSOR_MIN_GAIN) {
        gain = SENSOR_MIN_GAIN;
    } else if (gain > SENSOR_MAX_GAIN) {
        gain = SENSOR_MAX_GAIN;
    }

    for(i=0; i < ARRAY_SIZE(gain_table); i++)
    {
        if(gain_table[i] >= gain || (i == ARRAY_SIZE(gain_table)-1) )
            break;
    }

    if(i!=0)
        i=i-1;
    SENSOR_DMSG("[%s]: inputgain=%d i=%d gaintable=%d\n", __FUNCTION__, gain,i,gain_table[i]);

    params->tGain_reg[0].data=i;

    SENSOR_DMSG("[%s] pCus_SetAEGain:%ld, %d %d\n", __FUNCTION__, gain, params->tGain_reg[0].data,gain_table[i]);
    params->dirty = true;
    return SUCCESS;
}
#if 0
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max) {
    SENSOR_DMSG("\n\n[%s]\n", __FUNCTION__);
    *min = 1;//30
    *max = 1000000/Preview_MIN_FPS;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max) {
    SENSOR_DMSG("\n\n[%s]\n", __FUNCTION__);
    *min =handle->sat_mingain;
    *max = SENSOR_MAX_GAIN;
    return SUCCESS;
}

static int jxf37_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info) {
    SENSOR_DMSG("\n\n[%s]\n", __FUNCTION__);
    info->max = 1000000000/Preview_MIN_FPS; ///12;
    info->min =  Preview_line_period*1;//2
    info->step = Preview_line_period*1;//2
    return SUCCESS;
}

static int jxf37_GetShutterInfo_hdr(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info) {
    SENSOR_DMSG("\n\n[%s]\n", __FUNCTION__);
    info->max = 1000000000/Preview_MIN_FPS; ///12;
    info->min =  Preview_line_period_hdr*1;//2
    info->step = Preview_line_period_hdr*1;//2
    return SUCCESS;
}

static int pCus_setCaliData_gain_linearity(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num) {
  //  u32 i, j;
    return SUCCESS;
}
#endif
#ifdef K351P_HDR
static int DoOrien_hdr(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    K351P_params *params = (K351P_params *)handle->private_data;
    int table_length = ARRAY_SIZE(mirr_flip_table_hdr);
    int i;
    u16 sen_data=0;
    #define bit5 (0x01 << 5) //1=mirror 0=normal
    #define bit4 (0x01 << 4) //1=flip   0=normal

    SensorReg_Read(mirr_flip_table[0].reg, &sen_data);
    SENSOR_EMSG("\n\n[%s][%d]\n", __FUNCTION__,orit);
    //return SUCCESS;

     switch(orit) {
         case CUS_ORIT_M0F0:
            sen_data &= ~bit5;
            sen_data &= ~bit4;
            params->res.orit = CUS_ORIT_M0F0;
         break;

         case CUS_ORIT_M1F0://OK
            sen_data |= bit5;
            sen_data &= ~bit4;
            params->res.orit = CUS_ORIT_M1F0;
         break;

         case CUS_ORIT_M0F1:
            sen_data &= ~bit5;
            sen_data |= bit4;
            params->res.orit = CUS_ORIT_M0F1;
         break;

         case CUS_ORIT_M1F1://ok
            sen_data |= bit5;
            sen_data |= bit4;
            params->res.orit = CUS_ORIT_M1F1;
         break;
     }
    mirr_flip_table_hdr[0].data = sen_data;
    for(i=0;i<table_length;i++){
        SensorReg_Write(mirr_flip_table_hdr[i].reg,mirr_flip_table_hdr[i].data);
    }
     SENSOR_DMSG("pCus_SetOrien:%x,%x\r\n", orit,mirr_flip_table_hdr[0].data);
     return SUCCESS;

}

static int pCus_AEStatusNotify_hdr_sef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    //return SUCCESS;
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

static int pCus_SetAEUSecs_hdr_lef(ss_cus_sensor *handle, u32 us) {
#if 1
    u32 vts = 0;
    u32 lef_shutter=0,sef_shutter=0;
    K351P_params *params = (K351P_params *)handle->private_data;


    sef_shutter = params->expo.sef_shutter;

    lef_shutter=(1000*us)/Preview_line_period_hdr; // Preview_line_period_hdr in ns

    if(lef_shutter<1)
    {
        lef_shutter=1;
    }

    if ((lef_shutter + sef_shutter)> params->expo.vts-4)
    {
        vts = (lef_shutter + sef_shutter)+4;
    } else
    {
        vts=params->expo.vts;
    }

/*     if(vts != params->expo.vts)
    {
        params->expo.fps = (vts_30fps*30+(vts>>1))/vts;
    } */

    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->tExpo_reg[0].data = (lef_shutter>>8) & 0xff;
    params->tExpo_reg[1].data = (lef_shutter>>0) & 0xff;
    params->dirty = true;
    SENSOR_DMSG("[%s] val=[%d] lines=%d\n", __FUNCTION__,us,lef_shutter);
    //SENSOR_EMSG("[%s] us %ld, lines %ld, vts %ld,%ld\n", __FUNCTION__, us, lines, params->expo.vts,vts);
#endif
    return SUCCESS;
}

static int pCus_SetAEUSecs_hdr_sef(ss_cus_sensor *handle, u32 us) {
#if 1
    u32 lines = 0;
    K351P_params *params = (K351P_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_hdr; // Preview_line_period_hdr in ns
    if(lines<1)
    {
        lines=1;
    }

    if (lines > 500)
        lines = 500;

    //params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    //params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->tExpo_reg_hdr_sef[0].data = lines & 0xff;
    params->tExpo_reg_hdr_sef[1].data = (lines>>8) & 0x0f;
    params->tExpo_reg_hdr_sef[1].data += 0xc0; //0x04 [4:7]=0xc0
    params->expo.sef_shutter=lines;
    params->dirty = true;
    //SENSOR_EMSG("[%s] val=[%d]  lines=%d\n", __FUNCTION__,us,lines);
    //SENSOR_EMSG("[%s] us %ld, lines %ld, vts %ld,%ld\n", __FUNCTION__, us, lines, params->expo.vts,vts);
#endif
    return SUCCESS;
}


static int pCus_AEStatusNotify_hdr_lef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    K351P_params *params = (K351P_params *)handle->private_data;
    //return SUCCESS;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        break;
        case CUS_FRAME_ACTIVE:
        if(params->dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_hdr_sef, ARRAY_SIZE(expo_reg_sef));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));

            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            //SENSOR_DMSG("[%s] gain:%x, l_exp:%x,%x, s_exp:%x\n", "HDR:", params->tGain_reg[0], params->tExpo_reg[0], params->tExpo_reg[1], params->tExpo_reg_hdr_sef[0]);
            params->dirty = false;
        }
        if(params->orient_dirty)
        {
            DoOrien_hdr(handle, params->res.orit);
            params->orient_dirty = false;
        }
        break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_init_hdr(ss_cus_sensor *handle)
{
    int i,cnt=0;
    K351P_params *params = (K351P_params *)handle->private_data;
    SENSOR_EMSG("\n\[%s]\n\n", __FUNCTION__);
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_hdr);i++)
    {
        if(Sensor_init_table_hdr[i].reg==0xffff)
        {
            CamOsMsSleep(Sensor_init_table_hdr[i].data);
        }
        else
        {
                cnt = 0;
                while(SensorReg_Write(Sensor_init_table_hdr[i].reg,Sensor_init_table_hdr[i].data) != SUCCESS)
                {
                    cnt++;
                    //SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                    if(cnt>=10)
                    {
                        //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                        return FAIL;
                    }
                    CamOsMsSleep(10);
                }
            }
        }
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;

    return SUCCESS;
}

static int pCus_SetVideoRes_hdr(ss_cus_sensor *handle, u32 res_idx)
{
    SENSOR_DMSG("\n\npCus_SetVideoRes_HDR_DOL\n");
    return SUCCESS;
}

static int pCus_SetFPS_hdr_lef(ss_cus_sensor *handle, u32 fps)
{
    int vts=0;
    K351P_params *params = (K351P_params *)handle->private_data;
    SENSOR_EMSG("\n\n[%s][%d]", __FUNCTION__, fps);

    if(fps>=5 && fps <= 30){
      params->expo.fps = fps;
      params->expo.vts=  (vts_30fps_hdr*30)/fps;
    }else if(fps>=5000 && fps <= 15000){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_hdr*15000)/fps;
    }else{
      fps =30;
      params->expo.fps = fps;
      params->expo.vts=  (vts_30fps_hdr*30)/fps;
      //return FAIL;
    }

    if ((params->expo.line) > (params->expo.vts)-4) {
        vts = params->expo.line + 4;
    }else
        vts = params->expo.vts;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGain(ss_cus_sensor *handle, u32 gain) {

    int i=0;

    if (gain < SENSOR_MIN_GAIN) {
        gain = SENSOR_MIN_GAIN;
    } else if (gain > SENSOR_MAX_GAIN) {
        gain = SENSOR_MAX_GAIN;
    }

    for(i=0; i < ARRAY_SIZE(gain_table); i++)
    {
        if(gain_table[i] >= gain || (i == ARRAY_SIZE(gain_table)-1) )
            break;
    }

    if(i!=0)
        i=i-1;
    SENSOR_DMSG("[%s]: inputgain=%d i=%d gaintable=%d\n", __FUNCTION__, gain,i,gain_table[i]);

    return  gain_table[i];
}
#endif

static int cus_camsensor_init_handle(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    K351P_params *params;
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    //handle->private_data = CamOsMemCalloc(1, sizeof(jxf37_params));
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }

    params = (K351P_params *)handle->private_data;
    /////copy para////
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_hdr_sef, expo_reg_sef, sizeof(expo_reg_sef));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"K351P_MIPI_Linear");

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
    params->res.orit      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
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

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;
    handle->video_res_supported.res[0].u16width = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height = Preview_HEIGHT;
    handle->video_res_supported.res[0].u16max_fps= Preview_MAX_FPS;
    handle->video_res_supported.res[0].u16min_fps= Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x= 0;//cch123
    handle->video_res_supported.res[0].u16crop_start_y= 0;
    handle->video_res_supported.res[0].u16OutputWidth= 2000;
    handle->video_res_supported.res[0].u16OutputHeight= 2000;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2000x2000@30fps");

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MAX_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

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


    //handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init    ;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien      ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS      ;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS      ;

    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGain;//cch123

    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;

    //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    //handle->pCus_sensor_GetShutterInfo = jxf37_GetShutterInfo;
    params->expo.vts=vts_30fps;
    params->expo.fps = 30;
    //params->expo.line = 100;
    //handle->channel_mode = SENSOR_CHANNEL_MODE_LINEAR;

    params->dirty = false;
    params->orient_dirty = false;
    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    K351P_params *params = NULL;

    cus_camsensor_init_handle(drv_handle);
    params = (K351P_params *)handle->private_data;

    //SENSOR_DMSG("[%s] HDR SEF INIT!\n", __FUNCTION__);
    sprintf(handle->strSensorStreamName,"K351P_MIPI_HDR_SEF");

    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_hdr_sef, expo_reg_sef, sizeof(expo_reg_sef));

    handle->bayer_id    = SENSOR_BAYERID;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->data_prec   = SENSOR_DATAPREC;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;//hdr_lane_num;
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = SENSOR_HDR_MODE;

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].u16width = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height = Preview_HEIGHT; //TBD. Workaround for Sony DOL HDR mode
    handle->video_res_supported.res[0].u16max_fps= Preview_MAX_FPS_HDR;
    handle->video_res_supported.res[0].u16min_fps= Preview_MIN_FPS_HDR;
    handle->video_res_supported.res[0].u16crop_start_x= Preview_CROP_START_X;
    handle->video_res_supported.res[0].u16crop_start_y= Preview_CROP_START_Y;
    handle->video_res_supported.res[0].u16OutputWidth = 2000;
    handle->video_res_supported.res[0].u16OutputHeight = 2000;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2000x2000@30fps_HDR");

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_hdr;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MAX_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_hdr;

    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_hdr;

    handle->pCus_sensor_init        = pCus_init_hdr;

    handle->pCus_sensor_SetFPS          = pCus_SetFPS_hdr_lef;

    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_hdr_sef;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_hdr;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_hdr_sef;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetShutterInfo = jxf37_GetShutterInfo_hdr;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    params->expo.vts = vts_30fps_hdr;
    params->expo.fps = 30;

//HDR    params->expo.expo_lines = 673;

//    handle->channel_mode = SENSOR_CHANNEL_MODE_HDR;
#if 1

    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    //handle->ae_gain_delay       = 2;//SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay    = 2;//SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 2;//2;//ccc
    params->dirty = false;
    params->orient_dirty = false;
#endif
    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    K351P_params *params;

    cus_camsensor_init_handle(drv_handle);
    //SENSOR_DMSG("[%s] HDR LEF INIT!\n", __FUNCTION__);
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
    params = (K351P_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_hdr_sef, expo_reg_sef, sizeof(expo_reg_sef));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"K351P_MIPI_HDR_LEF");

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
    params->res.orit      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = SENSOR_HDR_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num =  0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].u16width = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height = Preview_HEIGHT; //TBD. Workaround for Sony DOL HDR mode
    handle->video_res_supported.res[0].u16max_fps= Preview_MAX_FPS_HDR;
    handle->video_res_supported.res[0].u16min_fps= Preview_MIN_FPS_HDR;
    handle->video_res_supported.res[0].u16crop_start_x= Preview_CROP_START_X;
    handle->video_res_supported.res[0].u16crop_start_y= Preview_CROP_START_Y;
    handle->video_res_supported.res[0].u16OutputWidth = 2000;
    handle->video_res_supported.res[0].u16OutputHeight = 2000;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2000x2000@30fps_HDR");

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_hdr;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MAX_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_hdr;

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;//ParaMclk(SENSOR_DRV_PARAM_MCLK());

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
    //handle->ae_gain_delay       = 2;//2;//SENSOR_GAIN_DELAY_FRAME_COUNT;
    //handle->ae_shutter_delay    = 2;//SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 2;//ccc2;

    ///calibration
    //handle->sat_mingain = g_sensor_ae_min_gain;//g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    //handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_hdr;
    //handle->pCus_sensor_powerupseq  = pCus_powerupseq   ;
    handle->pCus_sensor_poweron     = pCus_poweron;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal

    handle->pCus_sensor_GetVideoResNum = NULL;
    handle->pCus_sensor_GetVideoRes       = NULL;
    handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes       = NULL;

    //handle->pCus_sensor_GetShutterInfo = jxf37_GetShutterInfo_hdr;
    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_hdr_lef;

    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_hdr_lef;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_hdr_lef;
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGain;//cch123
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->pCus_sensor_SetAEGain     = pCus_SetAEGain;
    params->expo.vts = vts_30fps_hdr;
    params->expo.fps = 30;

    params->dirty = false;
    params->orient_dirty = false;

//  params->expo.expo_lines = 673;
//  if (CUS_CMU_CLK_36MHZ == handle->mclk)
//      params->expo.fps = 29;
//  else
//      params->expo.fps = 30;

//  params->dirty = false;

    //handle->channel_num = SENSOR_CHANNEL_NUM + 1;
//  handle->channel_mode = SENSOR_CHANNEL_MODE_HDR;

    return SUCCESS;
}



#if 0
static int cus_camsensor_release_handle(ss_cus_sensor *handle) {
    //ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    //sensor_if-> PCLK(NULL,CUS_PCLK_OFF);
    //sensor_if->SetCSI_Clk(handle,CUS_CSI_CLK_DISABLE);
#if 0
    if (handle && handle->private_data) {
        SENSOR_EMSG("[%s] release handle, handle %x, private data %x",
                __FUNCTION__,
                (int)handle,
                (int)handle->private_data);
        CamOsMemRelease(handle->private_data);
        handle->private_data = NULL;
    }
#endif
    return SUCCESS;
}
#endif

SENSOR_DRV_ENTRY_IMPL_END_EX(  K351P_HDR,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_hdr_sef,
                            cus_camsensor_init_handle_hdr_lef,
                            K351P_params
                         );

