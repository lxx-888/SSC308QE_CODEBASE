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
   Date          : 2023/09/22
   Build on      : Master_V4 I6C
   Verified on   : mixer preview ok (linear)
               AE gain/shutter ok , IQ not verify OBC maybe cause pink.
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OG0VA1B);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

//#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
//#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE            CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL   CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (1)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3


//#undef SENSOR_DBG
#define SENSOR_DBG 0

#define DEBUG_INFO        0

#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
//#define SENSOR_DMSG(args...) printf(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif

#define EN_OG0VA1B_STROBE_SIGNAL  (1)  //2021/11/4, OV Jeff add for strobe


///////////////////////////////////////////////////////////////
//          @@@                                                                                       //
//       @   @@      ==  S t a r t * H e r e ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                           //
//         @@@@                                                                                  //
//                                                                                                     //
//      Start Step 1 --  show preview on LCM                                         //
//                                                                                                    �@//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

//#define SENSOR_ISP_TYPE     ISP_EXT                   //ISP_EXT, ISP_SOC
#define F_number  22                                  // CFG, demo module
//#define SENSOR_DATAFMT      CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE     CUS_SEN_10TO12_9098     //CFG
//#define SENSOR_MAXGAIN      128
#define SENSOR_BAYERID      CUS_BAYER_RG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_MAX_GAIN     128                 // max sensor again, a-gain
//#define SENSOR_YCORDER      CUS_SEN_YCODR_YC       //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
#define lane_number 1
#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
#define long_packet_type_enable 0x00 //UD1~UD8 (user define)

#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
//#define Preview_line_period 30000                  ////HTS/PCLK=4455 pixels/148.5MHZ=30usec @MCLK=36MHz
//#define vts_30fps 1125//1346,1616                 //for 29.1fps @ MCLK=36MHz
#define Preview_line_period 7848//7951//24969                  //
//#define Line_per_second     32727
#define vts_30fps  4248////1335//1090                              //for 29.091fps @ MCLK=36MHz
#define Prv_Max_line_number 480                    //maximum exposure line munber of sensor when preview
#define Preview_WIDTH       640                   //resolution Width when preview
#define Preview_HEIGHT      480                   //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     3                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_MAX_GAIN                             (128 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (1)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (1)

#define MAX_A_GAIN (31*1024)

#define SENSOR_I2C_ADDR     0xC0                    //I2C slave address
#define SENSOR_I2C_SPEED   200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
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
}og0va1b_mipi_linear[] = {
    {LINEAR_RES_1, {640, 480, 3, 30}, {0, 0, 640, 480}, {"640x480@30fps"}},
};

//static int  drv_Fnumber = 22;
//static volatile long long framecount=0;
static volatile int fps_delay=5;
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
//static int g_sensor_ae_min_gain = 1024;
int check_ae_diff = 0;

CUS_MCLK_FREQ UseParaMclk(void);

CUS_CAMSENSOR_CAP sensor_cap = {
    .length = sizeof(CUS_CAMSENSOR_CAP),
    .version = 0x0001,
};

typedef struct {
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
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
    } expo;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tMirror_reg[8];
    I2C_ARRAY tPatternTbl[1];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool reg_dirty;
    bool orien_dirty;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
    CUS_SNR_IR_WITH_SHUTTER_TYPE_e snr_ir_type;
} OG0VA1B_params;
// set sensor ID address and data

typedef struct {
    u64 total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

const I2C_ARRAY Sensor_id_table[] =
{
	    {0x300A, 0xC7},      // {address of ID, ID },
	    {0x300B, 0x56},      // {address of ID, ID },
};


const I2C_ARRAY Sensor_init_table[] =     // 1920*1080_60fps_27MCLK_756MMIPI_2840*1109*30
{

{0x0103, 0x01},
{0xffff, 30 },    //delay(100ms)
//{0x0100, 0x00},
//{0x0301, 0x29},
//{0x0326, 0x58},

{0x0302, 0x31},
{0x0304, 0x01},
{0x0305, 0xe0},
{0x0306, 0x00},
{0x0326, 0xd8},
#if (EN_OG0VA1B_STROBE_SIGNAL == 1)
{0x3006, 0x0e},  //0x02, //CSP & RAW package both case
{0x3020, 0x20},
#else
{0x3006, 0x02},
#endif
{0x3022, 0x01},
{0x3107, 0x40},
{0x3216, 0x01},
{0x3217, 0x00},
{0x3218, 0xc0},
{0x3219, 0x55},
{0x3500, 0x00},
{0x3501, 0x10},
{0x3502, 0x72},
{0x3506, 0x01},
{0x3507, 0x50},
{0x3508, 0x01},
{0x3509, 0x00},
{0x350a, 0x01},
{0x350b, 0x00},
{0x350c, 0x00},
{0x3541, 0x00},
{0x3542, 0x40},
{0x3605, 0x90},
{0x3606, 0x41},
{0x3612, 0x00},
{0x3620, 0x08},
{0x3630, 0x17},
{0x3631, 0x99},
{0x3639, 0x88},
#if (EN_OG0VA1B_STROBE_SIGNAL == 1)
{0x3668, 0x08},  //0x00,
#else
{0x3668, 0x00},
#endif
{0x3674, 0x00},
{0x3677, 0x3f},
{0x368f, 0x06},
{0x36a2, 0x19},
{0x36a4, 0xf1},
{0x36a5, 0x2d},
{0x3706, 0x30},
{0x370d, 0x72},
{0x3713, 0x86},
{0x3715, 0x03},
{0x3716, 0x00},
{0x376d, 0x24},
{0x3770, 0x3a},
{0x3778, 0x00},
{0x37a8, 0x03},
{0x37a9, 0x00},
{0x37df, 0x7d},
{0x3800, 0x00},
{0x3801, 0x00},
{0x3802, 0x00},
{0x3803, 0x00},
{0x3804, 0x02},
{0x3805, 0x8f},
{0x3806, 0x01},
{0x3807, 0xef},
{0x3808, 0x02},
{0x3809, 0x80},
{0x380a, 0x01},
{0x380b, 0xe0},
{0x380c, 0x01},
{0x380d, 0x78},
{0x380e, 0x10},
{0x380f, 0x80},
{0x3810, 0x00},
{0x3811, 0x08},
{0x3812, 0x00},
{0x3813, 0x08},
{0x3814, 0x11},
{0x3815, 0x11},
{0x3816, 0x00},
{0x3817, 0x01},
{0x3818, 0x00},
{0x3819, 0x05},
{0x3820, 0x40},
{0x3821, 0x04},
{0x3823, 0x00},
{0x3826, 0x00},
{0x3827, 0x00},
{0x382b, 0x52},
{0x384a, 0xa2},
{0x3858, 0x00},
{0x3859, 0x00},
{0x3860, 0x00},
{0x3861, 0x00},
{0x3866, 0x0c},
{0x3867, 0x07},
{0x3884, 0x00},
{0x3885, 0x08},
{0x3893, 0x6c},
{0x3898, 0x00},
{0x389a, 0x04},
{0x389b, 0x01},
{0x389c, 0x0b},
{0x389d, 0xdc},
{0x389f, 0x08},  //; Strobe positive/negative option [2]=0 positive; [2]=1 negative
{0x38a0, 0x00},  //; strb_line_adj (R380a|38a1)
{0x38a1, 0x00},
{0x38b1, 0x04},
{0x38b2, 0x00},
{0x38b3, 0x08},
{0x38c9, 0x02},
{0x38d4, 0x06},
{0x38d5, 0x5a},
{0x38d6, 0x08},
{0x38d7, 0x3a},
#if (EN_OG0VA1B_STROBE_SIGNAL == 1)
{0x391e, 0x01},  //0x00,  //; auto with exposure width
#else
{0x391e, 0x00},
#endif
{0x391f, 0x00},
#if (EN_OG0VA1B_STROBE_SIGNAL == 1)
{0x3920, 0xff},  //0xa5,  //; strobe_pattern; strobe on all frames
//{0x3920, 0xaa},  //0xa5,  //; strobe_pattern; strobe on alternative frames
{0x3921, 0x00},  //; Tnegative (R0x3921[6:0]|3822|3923|3924)
{0x3922, 0x00},
{0x3923, 0x00},
{0x3924, 0x00},  //0x05
{0x3925, 0x00},  //; Tstrobe width (R0x3925|3926|3927|3928)
{0x3926, 0x00},
{0x3927, 0x00},
{0x3928, 0x10},  //0x1a,
#else
{0x3920, 0xa5},
{0x3921, 0x00},
{0x3922, 0x00},
{0x3923, 0x00},
{0x3924, 0x05},
{0x3925, 0x00},
{0x3926, 0x00},
{0x3927, 0x00},
{0x3928, 0x1a},
#endif
{0x3929, 0x01},
{0x392a, 0xb4},
{0x392b, 0x00},
{0x392c, 0x10},
#if (EN_OG0VA1B_STROBE_SIGNAL == 1)
{0x392d, 0x01},  //; Strobe step width (R392d|392e), MUST be the same as (R380c, R380e)
{0x392e, 0x78},
{0x392f, 0x4a},  //0x40,  //; [3]=1 enable sub-row step size control; [2:0]=0b10 strobe reference to pre-charge start
#else
{0x392f, 0x40},
#endif
{0x3a06, 0x06},
{0x3a07, 0x78},
{0x3a08, 0x08},
{0x3a09, 0x80},
{0x3a52, 0x00},
{0x3a53, 0x01},
{0x3a54, 0x0c},
{0x3a55, 0x04},
{0x3a58, 0x0c},
{0x3a59, 0x04},
{0x4000, 0xcf},
{0x4003, 0x40},
{0x4008, 0x04},
{0x4009, 0x13},
{0x400a, 0x02},
{0x400b, 0x34},
{0x4010, 0x71},
{0x4042, 0xc3},
{0x4306, 0x04},
{0x4307, 0x12},
{0x4500, 0x70},
{0x4509, 0x00},
{0x450b, 0x83},
{0x4604, 0x68},
{0x481b, 0x44},
{0x481f, 0x30},
{0x4823, 0x44},
{0x4825, 0x35},
{0x4837, 0x11},
{0x4f00, 0x04},
{0x4f10, 0x04},
{0x4f21, 0x01},
{0x4f22, 0x00},
{0x4f23, 0x54},
{0x4f24, 0x51},
{0x4f25, 0x41},
{0x5000, 0x3f},
{0x5001, 0x80},
{0x500a, 0x00},
{0x5100, 0x00},
{0x5111, 0x20},
{0x0100, 0x01}


};

const I2C_ARRAY TriggerStartTbl[] = {
//{0x30f4,0x00},//Master mode start
};

const I2C_ARRAY PatternTbl[] = {
    {0xb2,0x40}, //colorbar pattern , bit 0 to enable
};

const I2C_ARRAY mirror_reg[] = {
/*
  {0xfd, 0x01}, //0
  {0x01, 0x00}, //1
  {0x3f, 0x03}, //Mirror/Flip
  {0xf8, 0x00},
  {0x01, 0x01}, //4
  {0xfd, 0x02}, //5
  {0x5e, 0x07}, //mem down en + enable auto BR first
  {0xa1, 0x08}, //vertical start

*/
};

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


const I2C_ARRAY mirr_flip_table[] =
{
    /*
       {0xEF, 0x01},//M0F0
       {0x1b, 0x00},//bit7,Hflip
    // {0x90, 0x00},//bit2 Cmd_ADC_Latency
    {0x1d, 0x00},//bit7,Vflip

    {0xEF, 0x01},//M1F0
    {0x1b, 0x80},//bit7,Hflip
    // {0x90, 0x04},//bit2 Cmd_ADC_Latency
    {0x1d, 0x00},//bit7,Vflip

    {0xEF, 0x01},//M0F1
    {0x1b, 0x00},//bit7,Hflip
    //{0x90, 0x00},//bit2 Cmd_ADC_Latency
    {0x1d, 0x80},//bit7,Vflip

    {0xEF, 0x01},//M1F1
    {0x1b, 0x80},//bit7,Hflip
    // {0x90, 0x04},//bit2 Cmd_ADC_Latency
    {0x1d, 0x80},//bit7,Vflip
     */
};

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const static I2C_ARRAY gain_reg[] = {
    // {0x38, 0x00},//bit0, high bit
    {0x3508, 0x01},//low byte
    {0x3509, 0x00},
};

const static Gain_ARRAY gain_table[]={
    {10000  ,0x100},
    {10625 	,0x110},
    {11250 	,0x120},
    {11875 	,0x130},
    {12500 	,0x140},
    {13125 	,0x150},
    {13750 	,0x160},
    {14375 	,0x170},
    {15000 	,0x180},
    {15625 	,0x190},
    {16250 	,0x1a0},
    {16875 	,0x1b0},
    {17500 	,0x1c0},
    {18125 	,0x1d0},
    {18750 	,0x1e0},
    {19375 	,0x1f0},
    {20000	,0x200},
    {21250	,0x220},
    {22500	,0x240},
    {23750	,0x260},
    {25000	,0x280},
    {26250	,0x2a0},
    {27500	,0x2c0},
    {28750	,0x2e0},
    {30000	,0x300},
    {31250	,0x320},
    {32500  ,0x340},
    {33750  ,0x360},
    {35000	,0x380},
    {36250  ,0x3a0},
    {37500	,0x3c0},
    {38750  ,0x3e0},
    {40000	,0x400},
    {42500	,0x440},
    {45000	,0x480},
    {47500	,0x4c0},
    {50000	,0x500},
    {52500	,0x540},
    {55000	,0x580},
    {57500	,0x5c0},
    {60000	,0x600},
    {62500	,0x640},
    {65000	,0x680},
    {67500	,0x6c0},
    {70000	,0x700},
    {72500	,0x740},
    {75000	,0x780},
    {77500	,0x7c0},
    {80000  ,0x800},
    {85000	,0x880},
    {90000	,0x900},
    {95000	,0x980},
    {100000	,0xa00},
    {105000	,0xa80},
    {110000	,0xb00},
    {115000	,0xb80},
    {120000	,0xc00},
    {125000	,0xc80},
    {130000	,0xd00},
    {135000	,0xd80},
    {140000	,0xe00},
    {145000	,0xe80},
    {150000	,0xf00},
    {155000	,0xf80},
};

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


const I2C_ARRAY expo_reg[] = {

    {0x3501, 0x01},
    {0x3502, 0xfe},
};

const I2C_ARRAY vts_reg[] = {
    //{0x0d,0x10},//0x10 enable
    //{0x0E, 0xFF&(vts_30fps>>8)},//MSB
    //{0x0F, 0xFF&vts_30fps},//LSB
    {0x380e, 0x10},
    {0x380f, 0x80}
};

#define SLAVE_PWM_ID 0
#define SLAVE_PWM_PERIOD_NS   66666666
#define SLAVE_PWM_DUTY_PERIOD 33333333

CUS_INT_TASK_ORDER def_order = {
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

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME OG0VA1B


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))//; CamOsPrintf("[R %x %x]\r\n", _reg, (void*)(_data));
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))//; CamOsPrintf("[W %x %x]\r\n", _reg, _data);
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period * 2;
    info->u32AEShutter_step                  = Preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

#if defined(CONFIG_SENSOR_SUPPORT_IR)
static int og0va1b_PWMCrtl(ss_cus_sensor *handle, u32 bEn)
{
    s32 ret  = SUCCESS;
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    CUS_SNR_IR_Attr_t stCfg;

    stCfg.bLedEn = bEn;
    stCfg.eShutterType = params->snr_ir_type;
    stCfg.nPwm_Duty = 1000000000ULL;
    stCfg.nPwm_Id = SLAVE_PWM_ID;
    stCfg.nPwm_Inv = 0;
    stCfg.nPwm_period = 1000000000ULL;
    ret = handle->sensor_if_api->SensorIrCtrl(handle->i2c_bus->nSensorID, &stCfg);

    return ret;
}

#endif
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence


    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, params->reset_POLARITY);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_USLEEP(1000);


	sensor_if->MCLK(idx, 1, handle->mclk);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    SENSOR_USLEEP(1000);

    //sensor_if->Set3ATaskOrder(handle, def_order);
    // pure power on
    //ISP_config_io(handle);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_USLEEP(5000);
    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);
    // pwm init
#if defined(CONFIG_SENSOR_SUPPORT_IR)
    og0va1b_PWMCrtl(handle, 1);
#endif
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
	sensor_if->Reset(idx, params->reset_POLARITY);
    //handle->i2c_bus->i2c_close(handle->i2c_bus);
    //SENSOR_USLEEP(1000);
    //Set_csi_if(0, 0);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
#if defined(CONFIG_SENSOR_SUPPORT_IR)
    og0va1b_PWMCrtl(handle,0);
#endif
    return SUCCESS;
}

static int OG0VA1B_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    int i=0;
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    switch(mode)
    {
        case 1:
            params->tPatternTbl[0].data |= 0x01; //enable
            break;
        case 0:
            params->tPatternTbl[0].data &= 0xfe; //disable
            break;
        default:
            params->tPatternTbl[0].data &= 0xfe; //disable
            break;
    }

    for(i=0;i< ARRAY_SIZE(PatternTbl);i++)
    {
        if(SensorReg_Write(params->tPatternTbl[i].reg,params->tPatternTbl[i].data) != SUCCESS)
        {
            return FAIL;
        }
    }
    return SUCCESS;
}
static ss_cus_sensor *uvc_sensor_handle = 0;
short uvc_sensor_readreg(short addr)
{   // A16D8 -> 2 bytes addr and 1 byte data
	short val=0;
	ss_cus_sensor *handle = uvc_sensor_handle;
    if(handle != 0)
        SensorReg_Read(addr, (void*)&val);
	else
		CamOsPrintf("err read:%x %x\r\n", handle, addr);
	return val;

}

/* IR LED driver with i2c interface */
ss_cus_sensor APW7502_handle={0};
short ir_led_driver_writereg(short addr, short val)
{ // A8D8 -> 1 byte addr and 1 byte data
    ss_cus_sensor *handle = &APW7502_handle;
    if(handle != 0x00){
        SensorReg_Write(addr, val);
        CamOsPrintf("Addr:%x Data:%x\r\n", addr, val);
    }
    else
        CamOsPrintf("err write: addr:%x data:%x\r\n", addr, val);

    return SUCCESS;
}

short ir_led_driver_readreg(short addr)
{  // A8D8 -> 1 byte addr and 1 byte data
	short val;
	ss_cus_sensor *handle = &APW7502_handle;
    if(handle != 0x00)
        SensorReg_Read(addr, (void*)&val);
	else
		CamOsPrintf("err read: addr:%x\r\n", addr);
	return val;
}


static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init(ss_cus_sensor *handle)
{
    int i,cnt=0;
    //unsigned short reg_data;
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
	uvc_sensor_handle = handle;

    /* For adjust the IR LED */
    APW7502_handle.i2c_cfg.mode = SENSOR_I2C_LEGACY;
    APW7502_handle.i2c_cfg.fmt = I2C_FMT_A8D8;
    APW7502_handle.i2c_cfg.address = 0x04;
    APW7502_handle.i2c_cfg.speed = SENSOR_I2C_SPEED;

    for(i=0; i < ARRAY_SIZE(Sensor_init_table); i++)
    {
        if(Sensor_init_table[i].reg == 0xff) {
            SENSOR_MSLEEP_(Sensor_init_table[i].data);
        } else {

#if(0)
			if(Sensor_init_table[i].reg == 0x0103){
				SensorReg_Write(Sensor_init_table[i].reg, Sensor_init_table[i].data);
				SENSOR_MSLEEP_(30);
			} else{
				while(SensorReg_Write(Sensor_init_table[i].reg, Sensor_init_table[i].data) != SUCCESS  && Sensor_init_table[i].reg != 0x20)
	            {
	                cnt++;
	                //SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
	                if(cnt>=10)
	                {
	                    //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
	                    return FAIL;
	                }
	                SENSOR_MSLEEP_(10);
	            }
			}
#else
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table[i].reg, Sensor_init_table[i].data) != SUCCESS  && Sensor_init_table[i].reg != 0x20)
            {
                cnt++;
                //SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    CamOsPrintf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP_(10);
            }

			//SensorReg_Read(Sensor_init_table[i].reg, (void*)&reg_data);
  			//CamOsPrintf("[%x %x %x]\r\n", Sensor_init_table[i].reg, Sensor_init_table[i].data, reg_data);
#endif
        }
    }


    for(i=0;i< ARRAY_SIZE(mirror_reg); i++) {
        if(SensorReg_Write(params->tMirror_reg[i].reg,params->tMirror_reg[i].data) != SUCCESS) {
            return FAIL;
        }
    }
    APW7502_handle.i2c_bus = handle->i2c_bus;

    pCus_SetAEUSecs(handle, 25000);
    pCus_AEStatusNotify(handle,0,CUS_FRAME_ACTIVE);

    return SUCCESS;
}




/*
int pCus_release(ss_cus_sensor *handle)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sensor_if->PCLK(NULL,CUS_PCLK_OFF);
    return SUCCESS;
}
*/

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
            CamOsPrintf( "\n\n[%s], res=xxxx, notsupport\n", __FUNCTION__);
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
#if 1
    return SUCCESS;
#else
    short HFlip,VFlip;
    SensorReg_Write(0xef,0x01);//page 1
    SensorReg_Read(0x1b, &HFlip);
    SensorReg_Read(0x1d, &VFlip);

    if(((HFlip&0x80)==0) &&((VFlip&0x80)==0))
       *orit = CUS_ORIT_M0F0;
    else if(((HFlip&0x80)!=0) &&((VFlip&0x80)==0))
       *orit = CUS_ORIT_M1F0;
    else if(((HFlip&0x80)==0) &&((VFlip&0x80)!=0))
       *orit = CUS_ORIT_M0F1;
    else if(((HFlip&0x80)!=0) &&((VFlip&0x80)!=0))
       *orit = CUS_ORIT_M1F1;

    return SUCCESS;
#endif
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    char index=0;
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    switch(orit) {
        case CUS_ORIT_M0F0:
            index = 0;
            params->tMirror_reg[3].data = 0x00;
            params->tMirror_reg[6].data = 0x07;
            params->tMirror_reg[7].data = 0x08;
            handle->bayer_id = CUS_BAYER_BG;
            break;
        case CUS_ORIT_M1F0:
            index = 0x01;
            params->tMirror_reg[3].data = 0x00;//Mirror/Flip
            params->tMirror_reg[6].data = 0x06; //mem down en + enable auto BR first
            params->tMirror_reg[7].data = 0x09; //vertical start
            handle->bayer_id = CUS_BAYER_RG;
            break;
        case CUS_ORIT_M0F1:
            index = 0x02;
            params->tMirror_reg[3].data = 0x02;
            params->tMirror_reg[6].data = 0x06;
            params->tMirror_reg[7].data = 0x09;
            handle->bayer_id = CUS_BAYER_BG;
            break;
        case CUS_ORIT_M1F1:
            index = 0x03;
            params->tMirror_reg[3].data = 0x02;
            params->tMirror_reg[6].data = 0x07;
            params->tMirror_reg[7].data = 0x08;//0x09;
            handle->bayer_id = CUS_BAYER_RG;
            break;
    }
    SENSOR_DMSG("pCus_SetOrien:%x\r\n", index);

    if (index != params->tMirror_reg[2].data) {
        params->tMirror_reg[2].data = index;
        params->orien_dirty = true;
    }
    //params->reg_dirty = true;

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;

    return params->expo.fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    //CamOsPrintf(KERN_DEBUG "\n\n ****************  [%s], fps=%d  **************** \n", __FUNCTION__, fps);

    if(fps>=3 && fps <= 30) {
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*30)/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        //CamOsPrintf( "\n\n[%s], fps=%d, lines=%d\n", __FUNCTION__, fps, params->expo.vts);
        return SUCCESS;
    }else if(fps>=3000 && fps <= 30000) {
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*30000)/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        //CamOsPrintf( "\n\n[%s], fps=%d, lines=%d\n", __FUNCTION__, fps, params->expo.vts);
        return SUCCESS;
    }else{
        CamOsPrintf("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

}

#if 0
static int pCus_GetSensorCap(ss_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap) {
    if (cap)
        memcpy(cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP));
    else     return FAIL;
    return SUCCESS;
}
#endif

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification


static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
     OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
     //u16 reg_data_1, reg_data_2, reg_data_8, reg_data_9, reg_data_e, reg_data_f;
	//ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            if(params->orien_dirty){
                 //sensor_if->SetSkipFrame(handle,2); //skip 2 frame to avoid bad frame after mirror/flip
                 SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                 //sensor_if->BayerFmt(handle, handle->bayer_id);
                 params->orien_dirty = false;
            }
            /*CamOsPrintf(KERN_DEBUG "%s Inact gain reg value:0x%x%x, expo_reg value:0x%x%x, vts_reg value:0x%x%x%x\n",
                         __func__, params->tGain_reg[0].data,params->tGain_reg[1].data,
                         params->tExpo_reg[0].data, params->tExpo_reg[1].data,
                         params->tVts_reg[0].data, params->tVts_reg[1].data, params->tVts_reg[2].data
                         );*/
        break;
        case CUS_FRAME_ACTIVE:
			//CamOsPrintf("%d\r\n", MMPF_BSP_GetTick());
            /*CamOsPrintf(KERN_DEBUG "%s Act gain reg value:0x%x%x, expo_reg value:0x%x%x, vts_reg value:0x%x%x\n",
                         __func__, params->tGain_reg[0].data,params->tGain_reg[1].data,
                         params->tExpo_reg[0].data, params->tExpo_reg[1].data,
                         params->tVts_reg[0].data, params->tVts_reg[1].data);*/
            if(params->reg_dirty)
            {
                //SensorReg_Write(0xfd,0x01);//page 1
                //SensorReg_Write(0x01,0x00);//frame sync disable
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));

                //SensorReg_Write(params->tGain_reg[0].reg, params->tGain_reg[0].data);
                //SensorReg_Write(params->tGain_reg[1].reg, params->tGain_reg[1].data);
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                //SensorReg_Write(0x01,0x01);//frame sync enable
				#if(0)
				SensorReg_Read(0x3501, (void*)&reg_data_1);
				SensorReg_Read(0x3502, (void*)&reg_data_2);
				SensorReg_Read(0x3508, (void*)&reg_data_8);
				SensorReg_Read(0x3509, (void*)&reg_data_9);
				SensorReg_Read(0x380e, (void*)&reg_data_e);
				SensorReg_Read(0x380f, (void*)&reg_data_f);
				if(check_ae_diff != reg_data_1){
					check_ae_diff = reg_data_1;
					CamOsPrintf("gain control(0x3508, 0x3509):(%x, %x)\r\n", reg_data_8, reg_data_9);
					CamOsPrintf("exposure time control(0x3501, 0x3502):(%x, %x)\r\n", reg_data_1, reg_data_2);
					CamOsPrintf("Vts(0x380e, 0x380f):(%x, %x)\r\n", reg_data_e, reg_data_f);
				}
				#endif
                params->reg_dirty = false;
            }

            break;
            default :
            break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    u32 lines = 0;
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    lines  =  (u32)(params->tExpo_reg[1].data&0xff);
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;

    *us = (lines*Preview_line_period)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0; //, vts = 0;
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;

    lines=(u32)((1000*us+(Preview_line_period>>1))/Preview_line_period);
    if (lines < 2) lines = 2;
    /*if (lines >params->expo.vts-1)
        vts = lines +1;
    else
        vts = params->expo.vts;
*/

    /*CamOsPrintf(KERN_DEBUG"[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );*/

   // lines <<= 4;
    params->tExpo_reg[0].data =(u16)( (lines>>8) & 0x00ff);
    params->tExpo_reg[1].data =(u16)( (lines>>0) & 0x00ff);

	//CamOsPrintf("[%s] Expo_reg[0]: %x, Expo_reg[1]: %x\r\n", __FUNCTION__,params->tExpo_reg[0].data, params->tExpo_reg[1].data);

    //params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    //params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

  params->reg_dirty = true;
  return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    int again;
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    again=params->tGain_reg[0].data;

    *gain = again<<6;
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    OG0VA1B_params *params = (OG0VA1B_params *)handle->private_data;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;
    u32 i; //, input_gain = 0;
    u16 gain8 = 0;
    u64 gain_double,total_gain_double;

    gain = (gain * SENSOR_MIN_GAIN + 512)>>10; // need to add min sat gain

    //input_gain = gain;

    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN){
        gain=MAX_A_GAIN;
    	}

    Sensor_Gain_Linearity = gain_gap_compensate;

    for(i = 0; i < sizeof(gain_gap_compensate)/sizeof(CUS_GAIN_GAP_ARRAY); i++){
        //LOGD("GAP:%x %x\r\n",Sensor_Gain_Linearity[i].gain, Sensor_Gain_Linearity[i].offset);

        if (Sensor_Gain_Linearity[i].gain == 0)
            break;
        if((gain>Sensor_Gain_Linearity[i].gain) && (gain < (Sensor_Gain_Linearity[i].gain + Sensor_Gain_Linearity[i].offset))){
              gain=Sensor_Gain_Linearity[i].gain;
              break;
        }
    }

    gain_double=(u64)gain;
    total_gain_double=(gain_double*10000)/1024;

    for(i=1;i<ARRAY_SIZE(gain_table);i++)
    {
        if(gain_table[i].total_gain>total_gain_double)
        {
            gain8=(gain_table[i].total_gain-total_gain_double > total_gain_double-gain_table[i-1].total_gain) ? gain_table[i-1].reg_val:gain_table[i].reg_val;
            break;
        }
        else if(i==ARRAY_SIZE(gain_table)-1)
        {
            gain8=gain_table[i].reg_val;
            break;
        }
    }

    params->tGain_reg[0].data =gain8 >>8;
	params->tGain_reg[1].data =gain8 & 0xFF;

	//CamOsPrintf("[%s] reg_gain[0]: %x, reg_gain[1]: %x\r\n", __FUNCTION__, params->tGain_reg[0].data, params->tGain_reg[1].data);
#if 0
    gain16=(u16)(gain>>6);
    if(gain16>255)
       gain16=255;
   // params->tGain_reg[0].data = (gain16>>8)&0x01;//high bit
    params->tGain_reg[0].data = gain16&0xff; //low byte
/*
    if(input_gain > MAX_A_GAIN){

            params->tGain_reg[1].data=(u16)( (input_gain/MAX_A_GAIN)*128)&0xFF;
        }
    else{

            params->tGain_reg[1].data=0x80;
        }

 */
#endif
    //CamOsPrintf(KERN_DEBUG"[%s] set gain/regH/regL=%d/0x%x/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data,params->tGain_reg[1].data);
    params->reg_dirty = true;
    return SUCCESS;
}

static int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OG0VA1B_params *params;
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

    params = (OG0VA1B_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"OG0VA1B_MIPI");

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
        handle->video_res_supported.res[res].u16width             = og0va1b_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = og0va1b_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = og0va1b_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = og0va1b_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = og0va1b_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = og0va1b_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = og0va1b_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = og0va1b_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, og0va1b_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = OG0VA1B_SetPatternMode;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/og0va1b_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps;
    params->expo.fps                                              = 30;
    params->reg_dirty                                             = false;
    params->orien_dirty                                           = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    params->snr_ir_type = CUS_SNR_IR_WITH_GLOBALSHUTTER_SENSOR;
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  OG0VA1B,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            OG0VA1B_params
                         );

