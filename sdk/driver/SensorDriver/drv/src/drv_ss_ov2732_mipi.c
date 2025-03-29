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
   Date          : 2023/09/26
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

#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OV2732);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
#define SENSOR_MIPI_LANE_NUM_HDR (2)
#define SENSOR_MIPI_HDR_MODE (2) //0: Non-HDR mode. 1:Sony DOL mode
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

//#define SENSOR_ISP_TYPE            ISP_EXT                   //ISP_EXT, ISP_SOC
//#define SENSOR_DATAFMT           CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE          CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE     PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC            CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR        CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE            CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID             CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR         CUS_BAYER_BG//CUS_BAYER_GR
#define SENSOR_RGBIRID             CUS_RGBIR_NONE
#define SENSOR_ORIT                CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_MAX_GAIN            15872                 // max sensor again, a-gain
#define SENSOR_MIN_GAIN            1024//                     // max sensor again, a-gain * conversion-gain*d-gain

#define Preview_MCLK_SPEED         CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR     CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period        14076//30580                  //(36M/37.125M)*30fps=29.091fps(34.375msec), hts=34.375/1125=30556,
#define Preview_line_period_HDR    26666//30580           //(36M/37.125M)*30fps=29.091fps(34.375msec), hts=34.375/1125=30556,
#define vts_30fps                  1184                   //for 30fps @ MCLK=24MHz
#define vts_30fps_HDR              1250                   //for 30fps @ MCLK=24MHz

#define Preview_WIDTH              1920//2688             //resolution Width when preview
#define Preview_HEIGHT             1080//1520             //resolution Height when preview
#define Preview_MAX_FPS            60                     //fastest preview FPS
#define Preview_MAX_FPS_HDR        30                     //fastest preview FPS
#define Preview_MIN_FPS            5                      //slowest preview FPS

#define SENSOR_I2C_ADDR            0x6c                   //I2C slave address
#define SENSOR_I2C_SPEED           240000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY          I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT             I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL            CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL             CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL           CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL           CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL            CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);

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
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
        u32 max_short;
        u32 line;
    } expo;

    int sen_init;
    int still_min_fps;
    int video_min_fps;
    u32 skip_cnt;
    bool ori_dirty;
    bool dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tGain_reg_HDR_SEF[4];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tExpo_reg_HDR_SEF[3];
    I2C_ARRAY tMirror_reg[4];
    CUS_CAMSENSOR_ORIT cur_orien;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
} OV2732_params;
// set sensor ID address and data,

const I2C_ARRAY Sensor_id_table[] =
{
    {0x300a, 0x00},      // {address of ID, ID },
    {0x300b, 0x27},      // {address of ID, ID },
    {0x300c, 0x32},      // {address of ID, ID },
    // max 8 sets in this table
};

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
}ov2732_mipi_linear[] = {
    {LINEAR_RES_1, {1920, 1080, 3, 60}, {0, 0, 1920, 1080}, {"1920x1080@60fps"}},};
/*
static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_END}mode;
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
}ov2732_mipi_hdr[] = {
   {HDR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
};
*/
const I2C_ARRAY Sensor_init_table[] =
{
//SIZE_1920X1080_60FPS TWO_LANE; MIRROR
   {0x0103, 0x01},
   {0x0305, 0x3c},
   {0x0307, 0x00},
   {0x0308, 0x03},
   {0x0309, 0x03},
   {0x0327, 0x07},
   {0x3016, 0x32},
   {0x3000, 0x00},
   {0x3001, 0x00},
   {0x3002, 0x00},
   {0x3013, 0x00},
   {0x301f, 0xf0},
   {0x3023, 0xf0},
   {0x3020, 0x9b},
   {0x3022, 0x51},
   {0x3106, 0x11},
   {0x3107, 0x01},
   {0x3500, 0x00},
   {0x3501, 0x40},
   {0x3502, 0x00},
   {0x3503, 0x88},
   {0x3505, 0x83},
   {0x3508, 0x01},
   {0x3509, 0x80},
   {0x350a, 0x04},
   {0x350b, 0x00},
   {0x350c, 0x00},
   {0x350d, 0x80},
   {0x350e, 0x04},
   {0x350f, 0x00},
   {0x3510, 0x00},
   {0x3511, 0x00},
   {0x3512, 0x20},
   {0x3600, 0x55},
   {0x3601, 0x52},
   {0x3612, 0xb5},
   {0x3613, 0xb3},
   {0x3616, 0x83},
   {0x3621, 0x00},
   {0x3624, 0x06},
   {0x3642, 0x88},
   {0x3660, 0x00},
   {0x3661, 0x00},
   {0x366a, 0x64},
   {0x366c, 0x00},
   {0x366e, 0xff},
   {0x366f, 0xff},
   {0x3677, 0x11},
   {0x3678, 0x11},
   {0x3679, 0x0c},
   {0x3680, 0xff},
   {0x3681, 0x16},
   {0x3682, 0x16},
   {0x3683, 0x90},
   {0x3684, 0x90},
   {0x3768, 0x04},
   {0x3769, 0x20},
   {0x376a, 0x04},
   {0x376b, 0x20},
   {0x3620, 0x80},
   {0x3662, 0x10},
   {0x3663, 0x24},
   {0x3665, 0xa0},
   {0x3667, 0xa6},
   {0x3674, 0x01},
   {0x373d, 0x24},
   {0x3741, 0x28},
   {0x3743, 0x28},
   {0x3745, 0x28},
   {0x3747, 0x28},
   {0x3748, 0x00},
   {0x3749, 0x78},
   {0x374a, 0x00},
   {0x374b, 0x78},
   {0x374c, 0x00},
   {0x374d, 0x78},
   {0x374e, 0x00},
   {0x374f, 0x78},
   {0x3766, 0x12},
   {0x37e0, 0x00},
   {0x37e6, 0x04},
   {0x37e5, 0x04},
   {0x37e1, 0x04},
   {0x3737, 0x04},
   {0x37d0, 0x0a},
   {0x37d8, 0x04},
   {0x37e2, 0x08},
   {0x3739, 0x10},
   {0x37e4, 0x18},
   {0x37e3, 0x04},
   {0x37d9, 0x10},
   {0x4040, 0x04},
   {0x4041, 0x0f},
   {0x4008, 0x00},
   {0x4009, 0x0d},
   {0x37a1, 0x14},
   {0x37a8, 0x16},
   {0x37ab, 0x10},
   {0x37c2, 0x04},
   {0x3705, 0x00},
   {0x3706, 0x28},
   {0x370a, 0x00},
   {0x370b, 0x78},
   {0x3714, 0x24},
   {0x371a, 0x1e},
   {0x372a, 0x03},
   {0x3756, 0x00},
   {0x3757, 0x0e},
   {0x377b, 0x00},
   {0x377c, 0x0c},
   {0x377d, 0x20},
   {0x3790, 0x28},
   {0x3791, 0x78},
   {0x3800, 0x00},
   {0x3801, 0x00},
   {0x3802, 0x00},
   {0x3803, 0x04},
   {0x3804, 0x07},
   {0x3805, 0x8f},
   {0x3806, 0x04},
   {0x3807, 0x43},
   {0x3808, 0x07},
   {0x3809, 0x80},
   {0x380a, 0x04},
   {0x380b, 0x38},
   {0x380c, 0x02},
   {0x380d, 0x78},
   {0x380e, 0x09},
   {0x380f, 0x40},
   {0x3811, 0x08},
   {0x3813, 0x04},
   {0x3814, 0x01},
   {0x3815, 0x01},
   {0x3816, 0x01},
   {0x3817, 0x01},
   {0x381d, 0x40},
   {0x381e, 0x02},
   {0x3820, 0x88},
   {0x3821, 0x00},
   {0x3822, 0x04},
   {0x3835, 0x00},
   {0x4303, 0x19},
   {0x4304, 0x19},
   {0x4305, 0x03},
   {0x4306, 0x81},
   {0x4503, 0x00},
   {0x4508, 0x14},
   {0x450a, 0x00},
   {0x450b, 0x40},
   {0x4833, 0x08},
   {0x5000, 0xa9},
   {0x5001, 0x09},
   {0x3b00, 0x00},
   {0x3b02, 0x00},
   {0x3b03, 0x00},
   {0x3c80, 0x08},
   {0x3c82, 0x00},
   {0x3c83, 0xb1},
   {0x3c87, 0x08},
   {0x3c8c, 0x10},
   {0x3c8d, 0x00},
   {0x3c90, 0x00},
   {0x3c91, 0x00},
   {0x3c92, 0x00},
   {0x3c93, 0x00},
   {0x3c94, 0x00},
   {0x3c95, 0x00},
   {0x3c96, 0x00},
   {0x3c97, 0x00},
   {0x3c98, 0x00},
   {0x4000, 0xf3},
   {0x4001, 0x60},
   {0x4002, 0x00},
   {0x4003, 0x40},
   {0x4090, 0x14},
   {0x4601, 0x10},
   {0x4701, 0x00},
   {0x4708, 0x09},
   {0x470a, 0x00},
   {0x470b, 0x40},
   {0x470c, 0x81},
   {0x480c, 0x12},
   {0x4710, 0x06},
   {0x4711, 0x00},
   {0x4837, 0x12},
   {0x4800, 0x20},//bit5 1:non-con, 0:continous mode
   {0x4c01, 0x00},
   {0x5036, 0x00},
   {0x5037, 0x00},
   {0x580b, 0x0f},
   {0x4903, 0x80},
   {0x484b, 0x05},
   {0x400a, 0x00},
   {0x400b, 0x90},
   {0x4003, 0x40},
   {0x5000, 0xf9},
   {0x5200, 0x1b},
   {0x4837, 0x16},
   {0x380e, 0x04},
   {0x380f, 0xa0},
   {0x3500, 0x00},
   {0x3501, 0x49},
   {0x3502, 0x80},
   {0x3508, 0x02},
   {0x3509, 0x80},
   {0x3d8c, 0x11},
   {0x3d8d, 0xf0},
   {0x5180, 0x00},
   {0x5181, 0x10},
   {0x36a0, 0x16},
   {0x36a1, 0x50},
   {0x36a2, 0x60},
   {0x36a3, 0x80},
   {0x36a4, 0x00},
   {0x36a5, 0xa0},
   {0x36a6, 0x00},
   {0x36a7, 0x50},
   {0x36a8, 0x00},
   {0x36a9, 0x50},
   {0x36aa, 0x00},
   {0x36ab, 0x50},
   {0x36ac, 0x00},
   {0x36ad, 0x50},
   {0x36ae, 0x00},
   {0x36af, 0x50},
   {0x36b0, 0x00},
   {0x36b1, 0x50},
   {0x36b2, 0x00},
   {0x36b3, 0x50},
   {0x36b4, 0x00},
   {0x36b5, 0x50},
   {0x36b9, 0xee},
   {0x36ba, 0xee},
   {0x36bb, 0xee},
   {0x36bc, 0xee},
   {0x36bd, 0x0e},
   {0x36b6, 0x08},
   {0x36b7, 0x08},
   {0x36b8, 0x10},
   {0x0100, 0x01},
};

const I2C_ARRAY Sensor_init_table_HDR[] =
{

};

I2C_ARRAY TriggerStartTbl[] = {
//{0x30f4,0x00},//Master mode start
};

I2C_ARRAY PatternTbl[] = {
    //pattern mode
};

static I2C_ARRAY mirror_reg[] =
{
    {0x0100, 0x00},
    {0x3820, 0x08},//M0F0 [4]flip [3]mirror
    {0x373d, 0x00},
    {0x0100, 0x01},
};

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

//static int g_sensor_ae_min_gain = 1024;//1280;

const I2C_ARRAY gain_reg[] = {
    {0x3508, 0x00},//long a-gain[13:8]
    {0x3509, 0x80},//long a-gain[7:0]
    {0x350A, 0x04},// d-gain[13:8]
    {0x350B, 0x00},// d-gain[7:0]
};

const I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x350C, 0x00},//short a-gain[13:8]
    {0x350D, 0x80},//short a-gain[7:0]
    {0x350E, 0x04},// d-gain[13:8]
    {0x350F, 0x00},// d-gain[7:0]
};

const I2C_ARRAY expo_reg[] = {
    {0x3500, 0x00},//long exp[19,16]
    {0x3501, 0x02},//long exp[15,8]
    {0x3502, 0x00},//long exp[7,0]
};

const I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x3510, 0x00},//short
    {0x3511, 0x00},//short
    {0x3512, 0x20},
};

const I2C_ARRAY vts_reg[] = {
    {0x380E, 0x04},
    {0x380F, 0xa0},
};

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
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME OV2732


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
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

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    sensor_if->Reset(idx, !params->reset_POLARITY);

    //Sensor power on sequence
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, params->reset_POLARITY);
    SENSOR_UDELAY(20);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_UDELAY(20);
    ///////////////////

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_UDELAY(20);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    SENSOR_USLEEP(6000);
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, params->reset_POLARITY);
    SENSOR_UDELAY(30);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_UDELAY(30);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->MCLK(idx, 0, handle->mclk);

//    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init(ss_cus_sensor *handle)
{
    int i,cnt=0;
    OV2732_params *params = (OV2732_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {
        if(Sensor_init_table[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
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
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}
/*
static int pCus_init_HDR(ss_cus_sensor *handle)
{
    int i,cnt=0;
    OV2732_params *params = (OV2732_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR);i++)
    {
        if(Sensor_init_table_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_HDR[i].reg,Sensor_init_table_HDR[i].data) != SUCCESS)
            {
                cnt++;
                //SENSOR_DMSG("Sensor_init_table_HDR -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    //pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
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
    OV2732_params *params = (OV2732_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;
        default:
            break;
    }

    return SUCCESS;
}
/*
static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    OV2732_params *params = (OV2732_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short=105;
            break;
        default:
            break;
    }

    return SUCCESS;
}
*/
static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    OV2732_params *params = (OV2732_params *)handle->private_data;

    return params->cur_orien;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit) {
    OV2732_params *params = (OV2732_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    switch(orit) {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[1].data = 0x08;
            params->tMirror_reg[2].data = 0x00;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[1].data = 0x00;
            params->tMirror_reg[2].data = 0x00;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[1].data = 0x38;
            params->tMirror_reg[2].data = 0x02;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[1].data = 0x30;
            params->tMirror_reg[2].data = 0x02;
            params->ori_dirty = true;
        break;
    }
    params->cur_orien = orit;
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
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
    u32 vts=0;
    OV2732_params *params = (OV2732_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if ((params->expo.line) > (params->expo.vts)-8) {
        vts = params->expo.line + 8;
    }else
        vts = params->expo.vts;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}
/*
static int pCus_GetFPS_HDR_SEF(ss_cus_sensor *handle)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_SEF(ss_cus_sensor *handle, u32 fps)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
      return FAIL;
    }

    params->expo.max_short = (((params->expo.vts-10)/17 - 1)>>1) << 1;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x007f;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetFPS_HDR_LEF(ss_cus_sensor *handle, u32 fps)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
      return FAIL;
    }

    params->expo.max_short = (((params->expo.vts-10)/17 - 1)>>1) << 1;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x007f;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->dirty = true;
    return SUCCESS;
}
*/
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
    OV2732_params *params = (OV2732_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        if(params->ori_dirty){
            //handle->sensor_if_api->SetSkipFrame(handle->snr_pad_group, params->expo.fps, 3);
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->ori_dirty = false;
        }
        break;
        case CUS_FRAME_ACTIVE:
        if(params->dirty)
        {
            SensorReg_Write(0x3208,0x00);
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            SensorReg_Write(0x3208,0x10);
            SensorReg_Write(0x3208,0xa0);
            params->dirty = false;
        }
        break;
        default :
        break;
    }

    return SUCCESS;
}
/*
static int pCus_AEStatusNotify_HDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    //OV2732_params *params = (OV2732_params *)handle->private_data;
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
*/
#if 0
static int pCus_AEStatusNotify_HDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    //OV2732_params *params = (OV2732_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
/*         if(params->ori_dirty){
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->ori_dirty = false;
        }
        if(params->dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            params->dirty = false;
        } */
        break;
        default :
        break;
    }

    return SUCCESS;
}
#endif
/*
static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us) {
    u32 vts = 0, lines = 0;
    OV2732_params *params = (OV2732_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_HDR;

    if(lines<2) lines=2;
    if (lines > (params->expo.vts - params->expo.max_short - 2)) {
        lines = params->expo.vts - params->expo.max_short - 2;
    }else
        vts = params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    lines = lines<<4;
    params->tExpo_reg[0].data = (lines>>16) & 0xff;
    params->tExpo_reg[1].data = (lines>>8) & 0xff;
    params->tExpo_reg[2].data = (lines>>0) & 0xff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 *us) {
    u32 lines = 0;
    OV2732_params *params = (OV2732_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0xff)<<16;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[2].data&0xff)<<0;

    lines = lines>>4;
    *us = (lines*Preview_line_period_HDR)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 us) {
    u32 vts=0, lines = 0;
    OV2732_params *params = (OV2732_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_HDR;
    if(lines<2) lines=2;
    if (lines > params->expo.max_short - 2) {
        lines = params->expo.max_short - 2;
    }else
        vts = params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    lines = lines<<4;
    params->tExpo_reg_HDR_SEF[0].data = (lines>>16) & 0xff;
    params->tExpo_reg_HDR_SEF[1].data = (lines>>8) & 0xff;
    params->tExpo_reg_HDR_SEF[2].data = (lines>>0) & 0xff;

    params->dirty = true;
    return SUCCESS;
}
*/
static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    OV2732_params *params = (OV2732_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xff)<<0;
    lines = lines>>4;
    *us = (lines*Preview_line_period)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    OV2732_params *params = (OV2732_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period;

    if(lines<2) lines=2;
    if (lines > params->expo.vts-8) {
        vts = lines + 8;
    }else
        vts = params->expo.vts;

    params->expo.line = lines;
    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    lines = lines<<4;
    params->tExpo_reg[0].data = (lines>>16) & 0xff;
    params->tExpo_reg[1].data = (lines>>8) & 0xff;
    params->tExpo_reg[2].data = (lines>>0) & 0xff;

    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
  //  OV2732_params *params = (OV2732_params *)handle->private_data;
    return SUCCESS;
}

#define MAX_A_GAIN 15872//(15.5*1024)
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    OV2732_params *params = (OV2732_params *)handle->private_data;

    if (gain<1024) {
        gain = 1024;
    } else if (gain>= MAX_A_GAIN) {
        gain = MAX_A_GAIN; //again max 15.5x,Without using digital gain
    }

    gain = gain>>3;
    params->tGain_reg[0].data = gain>>8;
    params->tGain_reg[1].data = gain&0xff; //low byte

    SENSOR_DMSG("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}
/*
static int pCus_SetAEGain_HDR_SEF(ss_cus_sensor *handle, u32 gain) {
    OV2732_params *params = (OV2732_params *)handle->private_data;

    if (gain<1024) {
        gain = 1024;
    } else if (gain>= MAX_A_GAIN) {
        gain = MAX_A_GAIN; //again max 15.5x,Without using digital gain
    }

    gain = gain>>3;
    params->tGain_reg_HDR_SEF[0].data = gain>>8;//high bit
    params->tGain_reg_HDR_SEF[1].data = gain&0xff; //low byte

    SENSOR_DMSG("[%s] set gain =%d ,0x%x\n", __FUNCTION__, gain,gain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}

static int OV2732_GetShutterInfo_HDR_LEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS; ///12;
    info->min =  Preview_line_period_HDR*1;//2
    info->step = Preview_line_period_HDR*1;//2
    return SUCCESS;
}

static int OV2732_GetShutterInfo_HDR_SEF(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
    info->max = (Preview_line_period_HDR * params->expo.max_short);
    info->min =  Preview_line_period_HDR*1;//2
    info->step = Preview_line_period_HDR*1;//2
    return SUCCESS;
}

static int pCus_poweron_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_HDR_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}
static int pCus_GetSensorID_HDR_LEF(ss_cus_sensor *handle, u32 *id)
{
    *id = 0;
     return SUCCESS;
}
static int pCus_init_HDR_LEF(ss_cus_sensor *handle)
{
    return SUCCESS;
}
*/
#if 0
static int pCus_GetVideoRes_HDR_LEF( ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res )
{
    *res = &handle->video_res_supported.res[res_idx];
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_LEF( ss_cus_sensor *handle, u32 res )
{
    handle->video_res_supported.ulcur_res = 0; //TBD
    return SUCCESS;
}
#endif
/*
static int pCus_GetFPS_HDR_LEF(ss_cus_sensor *handle)
{
    OV2732_params *params = (OV2732_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_setCaliData_gain_linearity_HDR_LEF(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num)
{
    return SUCCESS;
}
static int pCus_SetAEGain_cal_HDR_LEF(ss_cus_sensor *handle, u32 gain)
{
    return SUCCESS;
}
*/
#define CMDID_I2C_READ   (0x01)
#define CMDID_I2C_WRITE  (0x02)

static int pCus_sensor_CustDefineFunction(ss_cus_sensor* handle,u32 cmd_id, void *param) {

    if(param == NULL || handle == NULL)
    {
        SENSOR_EMSG("param/handle data NULL \n");
        return FAIL;
    }

    switch(cmd_id)
    {
        case CMDID_I2C_READ:
        {
            I2C_ARRAY *reg = (I2C_ARRAY *)param;
            SensorReg_Read(reg->reg, &reg->data);
            SENSOR_EMSG("reg %x, read data %x \n", reg->reg, reg->data);
            break;
        }
        case CMDID_I2C_WRITE:
        {
            I2C_ARRAY *reg = (I2C_ARRAY *)param;
            SENSOR_EMSG("reg %x, write data %x \n", reg->reg, reg->data);
            SensorReg_Write(reg->reg, reg->data);
            break;
        }
        default:
            SENSOR_EMSG("cmd id %d err \n", cmd_id);
            break;
    }

    return SUCCESS;
}
int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OV2732_params *params;
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

    params = (OV2732_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"OV2732_MIPI");

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
        handle->video_res_supported.res[res].u16width             = ov2732_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = ov2732_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = ov2732_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = ov2732_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = ov2732_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = ov2732_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = ov2732_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = ov2732_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, ov2732_mipi_linear[res].senstr.strResDesc);
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
    handle->pCus_sensor_CustDefineFunction                        = pCus_sensor_CustDefineFunction;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/ov2732_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps;
    params->expo.fps                                              = 30;
    params->dirty                                                 = false;
    params->ori_dirty                                             = false;
    params->expo.line                                             = 100;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}
/*
int cus_camsensor_init_handle_HDR_SEF(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OV2732_params *params = NULL;
    int res;

    cus_camsensor_init_handle(drv_handle);
    params = (OV2732_params *)handle->private_data;

    sprintf(handle->model_id,"OV2732_MIPI_HDR_SEF");

    handle->bayer_id    = SENSOR_BAYERID_HDR;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->data_prec   = SENSOR_DATAPREC_HDR;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].width         = ov2732_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].height        = ov2732_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].max_fps       = ov2732_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].min_fps       = ov2732_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].crop_start_x  = ov2732_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].crop_start_y  = ov2732_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].nOutputWidth  = ov2732_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].nOutputHeight = ov2732_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, ov2732_mipi_hdr[res].senstr.strResDesc);
    }

    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR;
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    handle->pCus_sensor_init        = pCus_init_HDR;

    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_SEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_SEF;
    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_HDR_SEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_HDR_SEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_SEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    //handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_HDR_SEF;
    handle->pCus_sensor_GetShutterInfo = OV2732_GetShutterInfo_HDR_SEF;

    params->expo.vts = vts_30fps_HDR;
    params->expo.line = 1000;
    params->expo.fps = 30;
    params->expo.max_short = 105;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    handle->ae_gain_delay       = 2;
    handle->ae_shutter_delay    = 2;
    handle->ae_gain_ctrl_num = 1;
    handle->ae_shutter_ctrl_num = 2;

    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    OV2732_params *params;
    int res;
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
    params = (OV2732_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->model_id,"OV2732_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_HDR;  //CUS_DATAPRECISION_8;
    handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].width         = ov2732_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].height        = ov2732_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].max_fps       = ov2732_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].min_fps       = ov2732_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].crop_start_x  = ov2732_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].crop_start_y  = ov2732_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].nOutputWidth  = ov2732_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].nOutputHeight = ov2732_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, ov2732_mipi_hdr[res].senstr.strResDesc);
    }

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //polarity
    /////////////////////////////////////////////////////
    params->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////


    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->ae_gain_delay       = 2;
    handle->ae_shutter_delay    = 2;

    handle->ae_gain_ctrl_num = 1;
    handle->ae_shutter_ctrl_num = 2;

    ///calibration
    handle->sat_mingain=SENSOR_MIN_GAIN;


    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_HDR_LEF;

    handle->pCus_sensor_poweron     = pCus_poweron_HDR_LEF ;
    handle->pCus_sensor_poweroff    = pCus_poweroff_HDR_LEF;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID_HDR_LEF;

    handle->pCus_sensor_GetVideoResNum = NULL;
    handle->pCus_sensor_GetVideoRes       = NULL;
    handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_HDR_LEF;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_LEF;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = OV2732_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_HDR_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_HDR_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;

    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;

     //sensor calibration
    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal_HDR_LEF;
    handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity_HDR_LEF;
    handle->pCus_sensor_GetShutterInfo = OV2732_GetShutterInfo_HDR_LEF;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

    params->expo.vts=vts_30fps_HDR;
    params->expo.fps = 30;
    params->expo.max_short=105;
    params->dirty = false;
    params->ori_dirty = false;
    return SUCCESS;
}
*/
SENSOR_DRV_ENTRY_IMPL_END_EX(  OV2732,
                            cus_camsensor_init_handle_linear,
                            NULL,//cus_camsensor_init_handle_HDR_SEF,
                            NULL,//cus_camsensor_init_handle_HDR_LEF,
                            OV2732_params
                         );
