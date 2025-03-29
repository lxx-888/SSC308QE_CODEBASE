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
   Porting owner : paul-pc.wang
   Date : 2023/10/31
   Build on : Master V4  I6C
   Verified on : mixer preview ok (linear only),
               AE gain/shutter ok , IQ not verify.
   Remark : NA
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OV4688);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif


#if 1//(SENSOR0 == OV4688_MIPI)
#define BIND_SENSOR_OV4688 (1)
#define SENSOR_ROTATE_180 0
#define TEST_PATTERN_EN 0
#define ADD_VTS 0
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//MIPI config end.
//============================================

#if 1//defined(__MV5_FPGA__)
#define SLOW_CLK  (0) //Verify FPGA
#if (SENSOR_MIPI_LANE_NUM == 4)
#define OV4688_4LANE (1) //Verify FPGA
#elif (SENSOR_MIPI_LANE_NUM == 2)
#define OV4688_4LANE (0) //Verify FPGA
#endif
#define OV4688_2432x1368_60P_HDR (1) //Verify FPGA
#else //#if defined(__MV5_FPGA__)
#define SLOW_CLK  (0) //Verify FPGA
#define OV4688_4LANE (0) //Verify FPGA
#define OV4688_2432x1368_60P_HDR (0) //Verify FPGA
#endif //#if defined(__MV5_FPGA__)

//#define RES_IDX_2304x1296_60P_HDR   (1)     // mode 0,  2304*1296 60P
#define RES_IDX_2688x1520_30P      (0)     // mode 0,  2304*1296 60P
#define RES_IDX_1920x1080_30P       (1)     // mode 4,  1920*1080 30P       // Video (16:9)
#define RES_IDX_1280x720_60P        (2)     // mode 9,  1280*720  60P       // Video (16:9)

#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_DELAY   0x0E06                  //CFG
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#if (OV4688_4LANE == 1)
#define SENSOR_BAYERID      CUS_BAYER_GB            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#else
#define SENSOR_BAYERID      CUS_BAYER_BG            //CUS_BAYER_RG , GB,GR=>Pink,RG=>Blue
#endif
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define SENSOR_MAX_GAIN     (15874)                // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN     (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT      (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)

#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ//CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define MIPI_CLK 783 //MCLK 783MHz
#define Preview_line_period 21401     // 33.3ms/vts                      // MCLK=21.6 HTS/PCLK=3080 pixels/97.2MHZ=31.687us
#define vts_30fps  1560 //    0x614 380E/380F                                 // VTS for 20fps

#define Preview_WIDTH       1928//2688//2688                    //resolution Width when preview
#define Preview_HEIGHT      1092//1520//1520                    //resolution Height when preview

#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     5                      //slowest preview FPS

#define Preview_CROP_START_X     1                      //CROP_START_X
#define Preview_CROP_START_Y     1                      //CROP_START_Y

#define Cap_Max_line_number 1520//1520                   //maximum exposure line munber of sensor when capture

#define SENSOR_I2C_ADDR     0x6c                   //I2C slave address
#define SENSOR_I2C_SPEED    20000   //200KHz
#define SENSOR_I2C_CHANNEL  1                           //I2C Channel
#define SENSOR_I2C_PAD_MODE 2                           //Pad/Mode Number

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
//SCD define
#define SCD_SENSOR_MIPI_LANE_NUM (2)
#define SCD_SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_B
#define SCD_SENSOR_PWDN_POL      CUS_CLK_POL_POS
#define SCD_SENSOR_CHANNEL_NUM   (1)
//#define SCD_SENSOR_CHANNEL_MODE CUS_SENSOR_CHANNEL_MODE_RAW_STORE_FETCH
#define SCD_SENSOR_BAYERID      CUS_BAYER_RG
#define SCD_Preview_WIDTH       1928//2688//2688                    //resolution Width when preview
#define SCD_Preview_HEIGHT      1092//1520//1520
//

static int OV4688_SetAEGain( ss_cus_sensor *handle, u32 gain );
static int OV4688_SetAEUSecs( ss_cus_sensor *handle, u32 us );


typedef struct
{
    struct
    {
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
    struct
    {
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT orit;
    } res;
    struct
    {
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
    } expo;
    I2C_ARRAY tVts_reg[5];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tExpo_reg[4];
    I2C_ARRAY tMirror_reg[2];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    u32 gain;
    CUS_CAMSENSOR_ORIT cur_orien;
    bool mirror_dirty;
    bool reg_dirty;            //sensor setting need to update through I2C

} ov4688_params;

// set sensor ID address and data,
const static I2C_ARRAY Sensor_id_table[] =
                {
                {0x300a, 0x46},            // {address of ID, ID },
                {0x300b, 0x88},            // {address of ID, ID },
//{0x7001, 0x46},      // {address of ID, ID },
//{0x7002, 0x89},      // {address of ID, ID },

// max 8 sets in this table
                };
const I2C_ARRAY mirror_reg[] = {
    {0x3820, 0x00},//M0F0
    {0x3821, 0x06},
};

const static I2C_ARRAY Sensor_init_table_2lane[] =
{

#if 0 // 1920x1080 30fps
	{0x0103, 0x01},
    {0x3638, 0x00},
    {0x0300, 0x00},
    {0x0302, 0x1d},
    {0x0303, 0x00},
    {0x0304, 0x03},
    {0x030b, 0x00},
    {0x030d, 0x1e},
    {0x030e, 0x04},
    {0x030f, 0x01},
    {0x0312, 0x01},
    {0x031e, 0x00},
    {0x3000, 0x20},
    {0x3002, 0x00},
    {0x3018, 0x32},
    {0x3020, 0x93},
    {0x3021, 0x03},
    {0x3022, 0x01},
    {0x3031, 0x0a},
    //{0x303f, 0x0c},
    {0x3305, 0xf1},
    {0x3307, 0x04},
    {0x3309, 0x29},
    {0x3500, 0x00},
    {0x3501, 0x4c},
    {0x3502, 0x00},
    {0x3503, 0x04},
    {0x3504, 0x00},
    {0x3505, 0x00},
    {0x3506, 0x00},
    {0x3507, 0x00},
    {0x3508, 0x00},
    {0x3509, 0x80},
    {0x350a, 0x00},
    {0x350b, 0x00},
    {0x350c, 0x00},
    {0x350d, 0x00},
    {0x350e, 0x00},
    {0x350f, 0x80},
    {0x3510, 0x00},
    {0x3511, 0x00},
    {0x3512, 0x00},
    {0x3513, 0x00},
    {0x3514, 0x00},
    {0x3515, 0x80},
    {0x3516, 0x00},
    {0x3517, 0x00},
    {0x3518, 0x00},
    {0x3519, 0x00},
    {0x351a, 0x00},
    {0x351b, 0x80},
    {0x351c, 0x00},
	{0x351d, 0x00},
    {0x351e, 0x00},
    {0x351f, 0x00},
    {0x3520, 0x00},
    {0x3521, 0x80},
    {0x3522, 0x08},
    {0x3524, 0x08},
    {0x3526, 0x08},
    {0x3528, 0x08},
    {0x352a, 0x08},
    {0x3602, 0x00},
    //{0x3603, 0x40},
    {0x3604, 0x02},
    {0x3605, 0x00},
    {0x3606, 0x00},
    {0x3607, 0x00},
    {0x3609, 0x12},
    {0x360a, 0x40},
    {0x360c, 0x08},
    {0x360f, 0xe5},
    {0x3608, 0x8f},
    {0x3611, 0x00},
    {0x3613, 0xf7},
    {0x3616, 0x58},
    {0x3619, 0x99},
    {0x361b, 0x60},
    {0x361c, 0x7a},
    {0x361e, 0x79},
    {0x361f, 0x02},
    {0x3632, 0x10},
    {0x3633, 0x10},
    {0x3634, 0x10},
    {0x3635, 0x10},
    {0x3636, 0x15},
    {0x3646, 0x86},
    {0x364a, 0x0b},
    {0x3700, 0x17},
    {0x3701, 0x22},
    {0x3703, 0x10},
    {0x370a, 0x37},
    {0x3705, 0x00},
    {0x3706, 0x63},
    {0x3709, 0x3c},
    {0x370b, 0x01},
    {0x370c, 0x30},
    {0x3710, 0x24},
    {0x3711, 0x0c},
    {0x3716, 0x00},
    {0x3720, 0x28},
    {0x3729, 0x7b},
    {0x372a, 0x84},
    {0x372b, 0xbd},
    {0x372c, 0xbc},
    {0x372e, 0x52},
    {0x373c, 0x0e},
    {0x373e, 0x33},
    {0x3743, 0x10},
    {0x3744, 0x88},
    //{0x3745, 0xc0},
    {0x374a, 0x43},
    {0x374c, 0x00},
    {0x374e, 0x23},
    {0x3751, 0x7b},
    {0x3752, 0x84},
    {0x3753, 0xbd},
    {0x3754, 0xbc},
    {0x3756, 0x52},
    {0x375c, 0x00},
    {0x3760, 0x00},
    {0x3761, 0x00},
    {0x3762, 0x00},
    {0x3763, 0x00},
    {0x3764, 0x00},
    {0x3767, 0x04},
    {0x3768, 0x04},
    {0x3769, 0x08},
    {0x376a, 0x08},
    {0x376b, 0x20},
    {0x376c, 0x00},
    {0x376d, 0x00},
    {0x376e, 0x00},
    {0x3773, 0x00},
    {0x3774, 0x51},
    {0x3776, 0xbd},
    {0x3777, 0xbd},
    {0x3781, 0x18},
    {0x3783, 0x25},
   // {0x3798, 0x1b},
    {0x3800, 0x01},
    {0x3801, 0x88},
    {0x3802, 0x00},
    {0x3803, 0xe0},
    {0x3804, 0x09},
    {0x3805, 0x17},
    {0x3806, 0x05},
    {0x3807, 0x1f},
    {0x3808, 0x07},
    {0x3809, 0x80},
    {0x380a, 0x04},
    {0x380b, 0x3c},
    {0x380c, 0x0a},
    {0x380d, 0x0a},
    {0x380e, 0x06},
    {0x380f, 0x14},
    {0x3810, 0x00},
    {0x3811, 0x08},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3820, 0x00},
    {0x3821, 0x06},
    {0x3829, 0x00},
    {0x382a, 0x01},
    {0x382b, 0x01},
    {0x382d, 0x7f},
    {0x3830, 0x04},
    {0x3836, 0x01},
    {0x3837, 0x00},
    {0x3841, 0x02},
    {0x3846, 0x08},
    {0x3847, 0x07},
    {0x3d85, 0x36},
    {0x3d8c, 0x71},
    {0x3d8d, 0xcb},
    {0x3f0a, 0x00},
    {0x4000, 0x71},
    {0x4001, 0x40},
    {0x4002, 0x04},
    {0x4003, 0x14},
    {0x400e, 0x00},
    {0x4011, 0x00},
    {0x401a, 0x00},
    {0x401b, 0x00},
    {0x401c, 0x00},
    {0x401f, 0x00},
    {0x4020, 0x00},
    {0x4021, 0x10},
    {0x4022, 0x06},
    {0x4023, 0x13},
    {0x4024, 0x07},
    {0x4025, 0x40},
    {0x4026, 0x07},
    {0x4027, 0x50},
    {0x4028, 0x00},
    {0x4029, 0x02},
    {0x402a, 0x06},
    {0x402b, 0x04},
    {0x402c, 0x02},
    {0x402d, 0x02},
    {0x402e, 0x0e},
    {0x402f, 0x04},
    {0x4302, 0xff},
    {0x4303, 0xff},
    {0x4304, 0x00},
    {0x4305, 0x00},
    {0x4306, 0x00},
    {0x4308, 0x02},
    {0x4500, 0x6c},
    {0x4501, 0xc4},
    {0x4502, 0x40},
    {0x4503, 0x02},
    {0x4601, 0x77},
    {0x4800, 0x04},
    {0x4813, 0x08},
    {0x481f, 0x40},
    {0x4829, 0x78},
    {0x4837, 0x14},
    {0x4b00, 0x2a},
    {0x4b0d, 0x00},
    {0x4d00, 0x04},
    {0x4d01, 0x42},
    {0x4d02, 0xd1},
    {0x4d03, 0x93},
    {0x4d04, 0xf5},
    {0x4d05, 0xc1},
    {0x5000, 0xf3},
    {0x5001, 0x11},
    {0x5004, 0x00},
    {0x500a, 0x00},
    {0x500b, 0x00},
    {0x5032, 0x00},
    {0x5040, 0x00},
    {0x5050, 0x0c},
    {0x5500, 0x00},
    {0x5501, 0x10},
    {0x5502, 0x01},
    {0x5503, 0x0f},
    {0x8000, 0x00},
    {0x8001, 0x00},
    {0x8002, 0x00},
    {0x8003, 0x00},
    {0x8004, 0x00},
    {0x8005, 0x00},
    {0x8006, 0x00},
    {0x8007, 0x00},
    {0x8008, 0x00},
    {0x3638, 0x00},
    {0x3105, 0x31},
    {0x301a, 0xf9},
    {0x3508, 0x07},
    {0x484b, 0x05},
    {0x4805, 0x03},
    {0x3601, 0x01},
    {0x0100, 0x01},
    {0x3105, 0x11},
    {0x301a, 0xf1},
    {0x4805, 0x00},
    {0x301a, 0xf0},
    {0x3208, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x3601, 0x00},
    {0x3638, 0x00},
    {0x3208, 0x10},
    {0x3208, 0xa0},
    //{0x0100, 0x01},
#else // 2688x1520 R2A 30fps
    {0x0103, 0x01},
    {0x3638, 0x00},
    {0x0300, 0x00},
    {0x0302, 0x1d},
    {0x0303, 0x00},
    {0x0304, 0x03},
    {0x030b, 0x00},
    {0x030d, 0x1e},
    {0x030e, 0x04},
    {0x030f, 0x01},
    {0x0312, 0x01},
    {0x031e, 0x00},
    {0x3000, 0x20},
    {0x3002, 0x00},
    {0x3018, 0x32},
    {0x3020, 0x93},
    {0x3021, 0x03},
    {0x3022, 0x01},
    {0x3031, 0x0a},
    //{0x303f, 0x0c}, // should be mark
    {0x3305, 0xf1},
    {0x3307, 0x04},
    {0x3309, 0x29},
    {0x3500, 0x00},
    {0x3501, 0x60},
    {0x3502, 0x00},
    {0x3503, 0x04},
    {0x3504, 0x00},
    {0x3505, 0x00},
    {0x3506, 0x00},
    {0x3507, 0x00},
    {0x3508, 0x00},
    {0x3509, 0x80},
    {0x350a, 0x00},
    {0x350b, 0x00},
    {0x350c, 0x00},
    {0x350d, 0x00},
    {0x350e, 0x00},
    {0x350f, 0x80},
    {0x3510, 0x00},
    {0x3511, 0x00},
    {0x3512, 0x00},
    {0x3513, 0x00},
    {0x3514, 0x00},
    {0x3515, 0x80},
    {0x3516, 0x00},
    {0x3517, 0x00},
    {0x3518, 0x00},
    {0x3519, 0x00},
    {0x351a, 0x00},
    {0x351b, 0x80},
    {0x351c, 0x00},
	{0x351d, 0x00},
    {0x351e, 0x00},
    {0x351f, 0x00},
    {0x3520, 0x00},
    {0x3521, 0x80},
    {0x3522, 0x08},
    {0x3524, 0x08},
    {0x3526, 0x08},
    {0x3528, 0x08},
    {0x352a, 0x08},
    {0x3602, 0x00},
    //{0x3603, 0x40}, // should be mark
    {0x3604, 0x02},
    {0x3605, 0x00},
    {0x3606, 0x00},
    {0x3607, 0x00},
    {0x3609, 0x12},
    {0x360a, 0x40},
    {0x360c, 0x08},
    {0x360f, 0xe5},
    {0x3608, 0x8f},
    {0x3611, 0x00},
    {0x3613, 0xf7},
    {0x3616, 0x58},
    {0x3619, 0x99},
    {0x361b, 0x60},
    {0x361c, 0x7a},
    {0x361e, 0x79},
    {0x361f, 0x02},
    {0x3632, 0x00}, // should be 0x00
    {0x3633, 0x10},
    {0x3634, 0x10},
    {0x3635, 0x10},
    {0x3636, 0x15},
    {0x3646, 0x86},
    {0x364a, 0x0b},
    {0x3700, 0x17},
    {0x3701, 0x22},
    {0x3703, 0x10},
    {0x370a, 0x37},
    {0x3705, 0x00},
    {0x3706, 0x63},
    {0x3709, 0x3c},
    {0x370b, 0x01},
    {0x370c, 0x30},
    {0x3710, 0x24},
    {0x3711, 0x0c},
    {0x3716, 0x00},
    {0x3720, 0x28},
    {0x3729, 0x7b},
    {0x372a, 0x84},
    {0x372b, 0xbd},
    {0x372c, 0xbc},
    {0x372e, 0x52},
    {0x373c, 0x0e},
    {0x373e, 0x33},
    {0x3743, 0x10},
    {0x3744, 0x88},
    {0x3745, 0xc0}, //
    {0x374a, 0x43},
    {0x374c, 0x00},
    {0x374e, 0x23},
    {0x3751, 0x7b},
    {0x3752, 0x84},
    {0x3753, 0xbd},
    {0x3754, 0xbc},
    {0x3756, 0x52},
    {0x375c, 0x00},
    {0x3760, 0x00},
    {0x3761, 0x00},
    {0x3762, 0x00},
    {0x3763, 0x00},
    {0x3764, 0x00},
    {0x3767, 0x04},
    {0x3768, 0x04},
    {0x3769, 0x08},
    {0x376a, 0x08},
    {0x376b, 0x20},
    {0x376c, 0x00},
    {0x376d, 0x00},
    {0x376e, 0x00},
    {0x3773, 0x00},
    {0x3774, 0x51},
    {0x3776, 0xbd},
    {0x3777, 0xbd},
    {0x3781, 0x18},
    {0x3783, 0x25},
    {0x3798, 0x1b}, //
    {0x3800, 0x00},
    {0x3801, 0x08},
    {0x3802, 0x00},
    {0x3803, 0x04},
    {0x3804, 0x0a},
    {0x3805, 0x97},
    {0x3806, 0x05},
    {0x3807, 0xfb},
    {0x3808, 0x0a},
    {0x3809, 0x80},
    {0x380a, 0x05},
    {0x380b, 0xf0},
    {0x380c, 0x0a},
    {0x380d, 0x04},
    {0x380e, (vts_30fps >> 8)&0xFF},
    {0x380f, vts_30fps &0xFF},
    {0x3810, 0x00},
    {0x3811, 0x08},
    {0x3812, 0x00},
    {0x3813, 0x04},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3820, 0x00},
    {0x3821, 0x06},
    {0x3829, 0x00},
    {0x382a, 0x01},
    {0x382b, 0x01},
    {0x382d, 0x7f},
    {0x3830, 0x04},
    {0x3836, 0x01},
    {0x3837, 0x00},
    {0x3841, 0x02},
    {0x3846, 0x08},
    {0x3847, 0x07},
    {0x3d85, 0x36},
    {0x3d8c, 0x71},
    {0x3d8d, 0xcb},
    {0x3f0a, 0x00},
    {0x4000, 0x71}, // should be 0x71
    {0x4001, 0x40},
    {0x4002, 0x04},
    {0x4003, 0x14},
    {0x400e, 0x00},
    {0x4011, 0x00},
    {0x401a, 0x00},
    {0x401b, 0x00},
    {0x401c, 0x00},
    {0x401f, 0x00},
    {0x4020, 0x00},
    {0x4021, 0x10},
    {0x4022, 0x07},
    {0x4023, 0xcf},
    {0x4024, 0x09},
    {0x4025, 0x60},
    {0x4026, 0x09},
    {0x4027, 0x6f},
    {0x4028, 0x00},
    {0x4029, 0x02},
    {0x402a, 0x06},
    {0x402b, 0x04},
    {0x402c, 0x02},
    {0x402d, 0x02},
    {0x402e, 0x0e},
    {0x402f, 0x04},
    {0x4302, 0xff},
    {0x4303, 0xff},
    {0x4304, 0x00},
    {0x4305, 0x00},
    {0x4306, 0x00},
    {0x4308, 0x02},
    {0x4500, 0x6c},
    {0x4501, 0xc4},
    {0x4502, 0x40},
    {0x4503, 0x02}, // should be 0x02
    {0x4601, 0xA7},
    {0x4800, 0x04},
    {0x4813, 0x08},
    {0x481f, 0x40},
    {0x4829, 0x78},
    {0x4837, 0x16}, //
    {0x4b00, 0x2a},
    {0x4b0d, 0x00},
    {0x4d00, 0x04},
    {0x4d01, 0x42},
    {0x4d02, 0xd1},
    {0x4d03, 0x93},
    {0x4d04, 0xf5},
    {0x4d05, 0xc1},
    {0x5000, 0xf3},
    {0x5001, 0x11},
    {0x5004, 0x00},
    {0x500a, 0x00},
    {0x500b, 0x00},
    {0x5032, 0x00},
    {0x5040, 0x00},
    {0x5050, 0x0c},
    {0x5500, 0x00},
    {0x5501, 0x10},
    {0x5502, 0x01},
    {0x5503, 0x0f},
    {0x8000, 0x00},
    {0x8001, 0x00},
    {0x8002, 0x00},
    {0x8003, 0x00},
    {0x8004, 0x00},
    {0x8005, 0x00},
    {0x8006, 0x00},
    {0x8007, 0x00},
    {0x8008, 0x00},
    {0x3638, 0x00},

    {0x3105, 0x31},
    {0x301a, 0xf9},
    {0x3508, 0x07},
    {0x484b, 0x05},
    {0x4805, 0x03},
    {0x3601, 0x01},

    {0x0100, 0x01},

    {0xFFFF, 0x0A},
    {0x3105, 0x31}, //
    {0x301a, 0xf9}, //
    {0x3508, 0x07}, //
    {0x484b, 0x05}, //
    {0x4805, 0x03}, //
    {0x3601, 0x01}, //
    {0x3105, 0x11}, //
    {0x301a, 0xf1}, //
    {0x4805, 0x00}, //
    {0x301a, 0xf0},
    {0x3208, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x302a, 0x00},
    {0x3601, 0x00},
    {0x3638, 0x00},
    {0x3208, 0x10},
    {0x3208, 0xa0},
    //{0x0100, 0x01},

#endif
};

/*
const static I2C_ARRAY TriggerStartTbl[] =
{
    {0x0100, 0x01},            //normal mode
};
*/
/*
static I2C_ARRAY BadPixelTbl[] = {

// {0x5000,0x5F&(~0x04)},

                };*/

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
I2C_ARRAY AWB_GAIN_REG[] =
                {
                };

typedef struct
{
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const static I2C_ARRAY gain_reg[] =
{
{0x3508, 0x00},            //long a-gain[15:8]
{0x3509, 0x70},            //long a-gain[7:0]
{0x352A, 0x00},            // d-gain[14:8]
{0x352B, 0x00},            // d-gain[7:0]

};

//static int g_sensor_ae_min_gain = 1024;

const static I2C_ARRAY expo_reg[] =
{
{0x3208, 0x00},            //Group 0 hold start
{0x3500, 0x00},            //long exp[19,16]
{0x3501, 0x02},            //long exp[15,8]
{0x3502, 0x00},            //long exp[7,0]
};

const static I2C_ARRAY vts_reg[] =
{
    {0x380E, 0x7F & ( vts_30fps >> 8 )},
		{0x380F, 0xFF & ( vts_30fps )},
		{0x3208, 0x10},            //Group 0 hold end
	  {0x320D, 0x00},//manual launch
		{0x3208, 0xA0},            // Group delay laun
};
/*
static CUS_INT_TASK_ORDER def_order =
{
    .RunLength = 9,
    .Orders =
    {
        CUS_INT_TASK_AE | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_AWB | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_AE | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_AWB | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_AE | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_AWB | CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
        CUS_INT_TASK_VDOS | CUS_INT_TASK_AF,
    },
};*/

/////////// function definition ///////////////////
#define SENSOR_NAME ov4688

static int OV4688_SetFPS( ss_cus_sensor *handle, u32 fps );
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// image function /////////////////////////
//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL

/////////////////// sensor hardware dependent //////////////
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
static int OV4688_poweron( ss_cus_sensor *handle,  u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] ", __FUNCTION__);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, handle->mclk);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(1000);

    //sensor_if->Set3ATaskOrder(handle, def_order);
    // pure power on
    //ISP_config_io(handle);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_MSLEEP(10);
    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;

}

static int OV4688_poweroff( ss_cus_sensor *handle,  u32 idx)
{
     // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(2000);//mantis:1690203
    return SUCCESS;
}



static int OV4688_SetPatternMode( ss_cus_sensor *handle, u32 mode )
{
#if 1//defined(__MV5_FPGA__)
    return 0; //test only
#else //#if defined(__MV5_FPGA__)
    int i;

    switch( mode )
    {
        case 1:
            PatternTbl[0].data = 0x82;            //enable color square
            break;
        case 2:
            PatternTbl[0].data = 0x80;            //enable color bar
            break;
        case 0:
            PatternTbl[0].data &= 0x7F;            //disable
            break;
        default:
            PatternTbl[0].data &= 0x7F;            //disable
            break;
    }

    for( i = 0; i < ARRAY_SIZE( PatternTbl ); i++ )
    {
        if( SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS )
        {
            return FAIL;
        }
    }
    return SUCCESS;
#endif
}


static int OV4688_init( ss_cus_sensor *handle )
{
    //SENSOR_DMSG( "\n\n[%s]", __FUNCTION__ );
    int i,cnt = 0;
    const I2C_ARRAY *pSensor_init_table = NULL;
    int ArraySize=0;

    if(handle->interface_attr.attr_mipi.mipi_lane_num == 2)
    {
        pSensor_init_table = Sensor_init_table_2lane;
        ArraySize = ARRAY_SIZE(Sensor_init_table_2lane);
    }

    //UartSendTrace("OV4688 ARRAY_SIZE( pSensor_init_table) %d\n",ARRAY_SIZE( &pSensor_init_table ));
    for( i = 0; i < ArraySize; i++ )
    {
        if( (pSensor_init_table+i)->reg == 0xffff ){
            SENSOR_MSLEEP((pSensor_init_table+i)->data);//MsSleep(RTK_MS_TO_TICK(1));//usleep( 1000*Sensor_init_table_R1C[i].data );
            CamOsPrintf( "[%s]I2C 0xFFFF, sleep %d ms\r\n", __FUNCTION__, (pSensor_init_table+i)->data);
        }
        else{
            while(SensorReg_Write((pSensor_init_table+i)->reg,(pSensor_init_table+i)->data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_UDELAY(10);
            }

        }
    }


    //OV4688_SetFPS(handle,19);//init fps
    OV4688_SetAEGain( handle, 2048 );
    OV4688_SetAEUSecs( handle, 25000 );
    return SUCCESS;
}

static int OV4688_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    SENSOR_DMSG( "[%s] %d\n", __FUNCTION__, *ulres_num);
    return SUCCESS;
}

static int OV4688_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int OV4688_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int OV4688_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    //TODO: Set sensor output resolution
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(res_idx){
        case 0:
            handle->video_res_supported.ulcur_res = RES_IDX_2688x1520_30P;
            handle->pCus_sensor_init = OV4688_init;
            break;

        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    //sensor_if->WaitVEnd( 500 );
    return SUCCESS;
}

static int OV4688_GetOrien( ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit )
{
    ov4688_params *params = ( ov4688_params * ) handle->private_data;
    return params->cur_orien;
}

static int DoMirror( ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit )
{
    ov4688_params *params = (ov4688_params *)handle->private_data;

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[0].data = 0x00;
            params->tMirror_reg[1].data = 0x06;
            params->mirror_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[0].data = 0x00;
            params->tMirror_reg[1].data = 0x00;
            params->mirror_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[0].data = 0x06;
            params->tMirror_reg[1].data = 0x06;
            params->mirror_dirty = true;
            break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[0].data = 0x06;
            params->tMirror_reg[1].data = 0x00;
            params->mirror_dirty = true;
            break;
        default :
            break;
    }

    params->cur_orien = orit;
    return SUCCESS;
}

static int OV4688_SetOrien( ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit )
{
    DoMirror( handle, orit);
    return SUCCESS;
}

static int OV4688_GetFPS( ss_cus_sensor *handle )
{
    ov4688_params *params = ( ov4688_params * ) handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int OV4688_SetFPS( ss_cus_sensor *handle, u32 fps )
{
    u32 vts = 0;
    ov4688_params *params = (ov4688_params *)handle->private_data;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=5 && fps <= 30){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*30)/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->tVts_reg[0].data = (vts >> 8) & 0x007f;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int OV4688_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    ov4688_params *params = (ov4688_params *)handle->private_data;


    switch( status )
    {
        case CUS_FRAME_INACTIVE:
            break;
        case CUS_FRAME_ACTIVE:
            if(params->mirror_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->mirror_dirty = false;
            }
            if (params->reg_dirty == true)
            {
                SensorRegArrayW( ( I2C_ARRAY* )params->tExpo_reg, ARRAY_SIZE(expo_reg) );
                SensorRegArrayW( ( I2C_ARRAY* )params->tGain_reg, ARRAY_SIZE(gain_reg) );
                SensorRegArrayW( ( I2C_ARRAY* )params->tVts_reg, ARRAY_SIZE(vts_reg) );
                params->reg_dirty = false;
            }
            break;
        default:
            break;
    }
    return SUCCESS;
}

static int OV4688_GetAEUSecs( ss_cus_sensor *handle, u32 *us )
{
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
    ov4688_params *params = (ov4688_params *)handle->private_data;

    lines |= ( u32 ) ( params->tExpo_reg[1].data & 0xff ) << 16;
    lines |= ( u32 ) ( params->tExpo_reg[2].data & 0xff ) << 8;
    lines |= ( u32 ) ( params->tExpo_reg[3].data & 0xff ) << 0;
    lines >>= 4;

    *us = ( lines * Preview_line_period );

    return SUCCESS;
}

static int OV4688_SetAEUSecs( ss_cus_sensor *handle, u32 us )
{
    u32 lines = 0, vts = 0;
    ov4688_params *params = ( ov4688_params * ) handle->private_data;
    // return SUCCESS;

    lines = ( 1000 * us ) / Preview_line_period;

    if (lines <1 ) lines = 1;

    if( lines > params->expo.vts - 4 )
        vts = lines + 4;
    else
        vts = params->expo.vts;


    SENSOR_DMSG( "[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                    us,
                    lines,
                    params->expo.vts
                    );
    lines <<= 4;
    params->tExpo_reg[1].data = ( lines >> 16 ) & 0x000f;
    params->tExpo_reg[2].data = ( lines >> 8 ) & 0x00ff;
    params->tExpo_reg[3].data = ( lines >> 0 ) & 0x00ff;

    params->tVts_reg[0].data = ( vts >> 8 ) & 0x007f;
    params->tVts_reg[1].data = ( vts >> 0 ) & 0x00ff;

    params->reg_dirty = true;

    return 0;

}

//#define SENSOR_MAX_GAIN (15.5*1024)
// Gain: 1x = 1024
static int OV4688_GetAEGain( ss_cus_sensor *handle, u32* gain )
{
//=IF((B18=0),HEX2DEC(B19)/128,IF((B18=1),(HEX2DEC(B19)+8)/64,IF((B18=3),(HEX2DEC(B19)+12)/32,IF((B18=7),(HEX2DEC(B19)+8)/16,"Error"))))
    ov4688_params *params = (ov4688_params *)handle->private_data;
    if( params->tGain_reg[0].data == 0 )
        *gain = ( u32 ) ( ( params->tGain_reg[1].data / 128 ) << 10 );
    else if( params->tGain_reg[0].data == 1 )
        *gain = ( u32 ) ( ( ( params->tGain_reg[1].data + 8 ) / 64 ) << 10 );
    else if( params->tGain_reg[0].data == 3 )
        *gain = ( u32 ) ( ( ( params->tGain_reg[1].data + 12 ) / 32 ) << 10 );
    else if( params->tGain_reg[0].data == 7 )
        *gain = ( u32 ) ( ( ( params->tGain_reg[1].data + 8 ) / 16 ) << 10 );

    SENSOR_DMSG( "[%s] get gain/reg0/reg1 (1024=1X)= %d/0x%x/0x%x\n", __FUNCTION__, *gain, params->tGain_reg[0].data, params->tGain_reg[1].data );
    return 0;
}

static int OV4688_SetAEGain( ss_cus_sensor *handle, u32 gain )
{
    ov4688_params *params = ( ov4688_params * ) handle->private_data;
    u32 input_gain = 0;

    gain = ( gain * SENSOR_MIN_GAIN + 512 ) >> 10;            // need to add min sat gain
    //  params->expo.final_gain = gain;
    input_gain = gain;
    if( gain < SENSOR_MIN_GAIN )
        gain = SENSOR_MIN_GAIN;
    else if( gain >= SENSOR_MAX_GAIN )
        gain = SENSOR_MAX_GAIN;


    if( ( gain >> 10 ) < 2 )
    {
        params->tGain_reg[0].data = 0;
        params->tGain_reg[1].data = ( gain * 128 ) >> 10;                 // 1X ~ 2X
    }
    else if( ( gain >> 10 ) < 4 )
    {
        params->tGain_reg[0].data = 1;
        params->tGain_reg[1].data = ( ( gain * 64 ) - ( 256 * 8 ) ) >> 10;            // 2X ~ 4X
    }
    else if( ( gain >> 10 ) < 8 )
    {
        params->tGain_reg[0].data = 3;
        params->tGain_reg[1].data = ( ( gain * 32 ) - ( 256 * 12 ) ) >> 10;            // 4X ~ 8X
    }
    else
    {
        params->tGain_reg[0].data = 7;
        params->tGain_reg[1].data = ( ( gain * 16 ) - ( 256 * 8 ) ) >> 10;            // 8X ~16X
    }

    if( input_gain > SENSOR_MAX_GAIN )
    {
        params->tGain_reg[2].data = ( u16 ) ( ( input_gain / SENSOR_MAX_GAIN ) * 8 ) & 0x7F;
        params->tGain_reg[3].data = ( u16 ) ( ( input_gain / SENSOR_MAX_GAIN ) * 2048 ) & 0xFF;
    }
    else
    {

        params->tGain_reg[2].data = 0x08;
        params->tGain_reg[3].data = 0;

    }

    SENSOR_DMSG( "[%s] set input gain/gain/regH/regL=%d/%d/0x%x/0x%x\n",
                    __FUNCTION__,
                    input_gain,
                    gain,
                    params->tGain_reg[0].data,
                    params->tGain_reg[1].data );
    params->reg_dirty = true;
    //CamOsPrintf("pCus_SetAEGain,Gain[0][1][2][3] = 0x%x,0x%x,0x%x,0x%x\r\n",params->tGain_reg[0].data,params->tGain_reg[1].data,params->tGain_reg[2].data,params->tGain_reg[3].data);


    return SUCCESS;

}


int ov4688_MIPI_cus_camsensor_init_handle( ss_cus_sensor* drv_handle )
{
    ss_cus_sensor *handle = drv_handle;
    ov4688_params *params;

    CamOsPrintf("Enter OV4688_MIPI_cus_camsensor_init_handle\r\n");

    if( !handle )
    {
        SENSOR_DMSG( "[%s] not enough memory!\n", __FUNCTION__ );
        return FAIL;
    }
    SENSOR_DMSG( "[%s]\n", __FUNCTION__ );
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (ov4688_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf( handle->strSensorStreamName, "OV4688_MIPI" );

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus = SENSOR_IFBUS_TYPE;            //CUS_SENIF_BUS_PARL;
    handle->data_prec = SENSOR_DATAPREC;            //CUS_DATAPRECISION_8;
    handle->RGBIR_id = CUS_RGBIR_NONE;
    handle->bayer_id = SENSOR_BAYERID;            //CUS_BAYER_GB;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.

    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16width = 2688;
    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16height = 1520;
    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16max_fps = 30;
    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16min_fps = 3;
    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16crop_start_x = 0;
    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16crop_start_y = 0;
    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16OutputWidth= 2688;
    handle->video_res_supported.res[RES_IDX_2688x1520_30P].u16OutputHeight= 1520;
    sprintf(handle->video_res_supported.res[RES_IDX_2688x1520_30P].strResDesc, "2688x1520@30fps");

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;            //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;            //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;            //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //200000;

    // mclk
    handle->mclk = Preview_MCLK_SPEED;

    //Mirror / Flip
    params->cur_orien = SENSOR_ORIT;
    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init = OV4688_init;
    handle->pCus_sensor_poweron = OV4688_poweron;
    handle->pCus_sensor_poweroff = OV4688_poweroff;

    // Normal
    handle->pCus_sensor_GetVideoResNum = OV4688_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes = OV4688_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = OV4688_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes = OV4688_SetVideoRes;

    handle->pCus_sensor_GetOrien = OV4688_GetOrien;
    handle->pCus_sensor_SetOrien = OV4688_SetOrien;
    handle->pCus_sensor_GetFPS = OV4688_GetFPS;
    handle->pCus_sensor_SetFPS = OV4688_SetFPS;
    handle->pCus_sensor_SetPatternMode = OV4688_SetPatternMode;
    handle->pCus_sensor_GetAEInfo  = pCus_sensor_GetAEInfo;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = OV4688_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs = OV4688_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs = OV4688_SetAEUSecs;
    handle->pCus_sensor_GetAEGain = OV4688_GetAEGain;
    handle->pCus_sensor_SetAEGain = OV4688_SetAEGain;

    params->expo.vts = vts_30fps;
    params->expo.fps = 30;
    params->gain = SENSOR_MIN_GAIN;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 4;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  OV4688,
                            ov4688_MIPI_cus_camsensor_init_handle,
                            NULL,
                            NULL,
                            ov4688_params
                         );
