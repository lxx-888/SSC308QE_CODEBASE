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
   Porting owner : eddie.liang
   Date : 2023/10/30
   Build on : Master V4  I6C
   Verified on : mixer preview ok (linear/hdr),
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OS04A10);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE  CAM_OS_ARRAY_SIZE
#endif

//============================================
//
//    SENSOR STATUS
//
//============================================
//MIPI config begin.
#undef SENSOR_NAME
#define SENSOR_NAME                           OS04A10

#define SENSOR_IFBUS_TYPE                     CUS_SENIF_BUS_MIPI
#define SENSOR_HDR_MODE                       CUS_HDR_MODE_VC
//Linear Mode
#define SENSOR_MIPI_LANE_NUM                  (2)
#define Preview_MCLK_SPEED                    CUS_CMU_CLK_24MHZ

//HDR mode
#define SENSOR_MIPI_LANE_NUM_HDR              (2)

//I2C bus
#define SENSOR_I2C_ADDR                       0x6c
#define SENSOR_I2C_SPEED                      20000
#define SENSOR_I2C_LEGACY                     I2C_NORMAL_MODE
#define SENSOR_I2C_FMT                        I2C_FMT_A16D8

#define SENSOR_RGBIRID                        CUS_RGBIR_NONE
#define SENSOR_DATAPREC                       CUS_DATAPRECISION_10
#define SENSOR_BAYERID                        CUS_BAYER_BG

#define SENSOR_DATAPREC_HDR                   CUS_DATAPRECISION_10
#define SENSOR_BAYERID_HDR                    CUS_BAYER_BG

//AE info
#define SENSOR_MAX_GAIN                       (255 * 1024)
#define SENSOR_MIN_GAIN                       (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

#define SENSOR_GAIN_DELAY_FRAME_COUNT_HDR     (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR  (2)
#define SENSOR_GAIN_CTRL_NUM_DOL              (2)
#define SENSOR_SHUTTER_CTRL_NUM_DOL           (2)

#define Preview_line_period_HDR               24630
#define vts_30fps_HDR                         1624
u32 Preview_line_period  = 20350;
u32 vts_30fps;


//============================================
//
//    SENSOR Resolution List
//
//============================================

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
}mipi_linear[] = {
    {LINEAR_RES_1, {2688, 1520, 3, 30}, {0, 0, 2688, 1520}, {"2688x1520@30fps"}},
};

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
}mipi_hdr[] = {
   {HDR_RES_1, {2688, 1520, 3, 25}, {0, 0, 2688, 1520}, {"2688x1520@25fps_HDR"}}, // Modify it
};

typedef struct {
    struct {
        u32 expo_lines;
        u32 expo_lef_us;
        u32 expo_sef_us;
        u32 vts;
        u32 final_gain;
        u32 final_gain_sef;
        u32 fps;
        u32 preview_fps;
        u32 line;
        u32 max_short_exp;
    } expo;

    u32 skip_cnt;
    CUS_CAMSENSOR_ORIT  orit;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tVts_reg_HDR[2];
    I2C_ARRAY tMax_short_exp_reg[3];
    I2C_ARRAY tGain_reg_HDR_SEF[5];
    I2C_ARRAY tGain_reg_HDR_LEF[5];
    I2C_ARRAY tExpo_reg_HDR_SEF[2];
    I2C_ARRAY tExpo_reg_HDR_LEF[2];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tMirror_reg[1];
    bool orien_dirty;
    bool reg_dirty;
    bool vts_reg_dirty;
    bool change;
} OS04A10_params;

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)     (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)     (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

// set sensor ID address and data,
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x300a, 0x53},      // {address of ID, ID },
    {0x300b, 0x04},      // {address of ID, ID },
};
const static I2C_ARRAY Sensor_init_table_HDR_4lane[] =
{
    {0x0103, 0x01},
    {0x0109, 0x01},
    {0x0104, 0x02},
    {0x0102, 0x00},
    {0x0305, 0x60}, //;6c
    {0x0306, 0x00},
    {0x0307, 0x00},
    {0x0308, 0x04},
    {0x030a, 0x01},
    {0x0317, 0x09},
    {0x0322, 0x01},
    {0x0323, 0x02},
    {0x0324, 0x00},
    {0x0325, 0xd8},// ;b0
    {0x0327, 0x05},
    {0x0329, 0x02},
    {0x032c, 0x02},
    {0x032d, 0x02},
    {0x032e, 0x02},
    {0x300f, 0x11},
    {0x3012, 0x21},
    {0x3026, 0x10},
    {0x3027, 0x08},
    {0x302d, 0x24},
    {0x3104, 0x01},
    {0x3106, 0x11},
    {0x3400, 0x00},
    {0x3408, 0x05},
    {0x340c, 0x0c},
    {0x340d, 0xb0},
    {0x3425, 0x51},
    {0x3426, 0x10},//;50
    {0x3427, 0x14},//;15
    {0x3428, 0x10},//;50
    {0x3429, 0x10},
    {0x342a, 0x10},
    {0x342b, 0x04},
    {0x3501, 0x02},
    {0x3504, 0x08},
    {0x3508, 0x01},
    {0x3509, 0x00},
    {0x350a, 0x01},
    {0x3541, 0x00},
    {0x3542, 0x20},
    {0x3581, 0x00},
    {0x3582, 0x20},
    {0x3544, 0x08},
    {0x3548, 0x01},
    {0x3549, 0x00},
    {0x3584, 0x08},
    {0x3588, 0x01},
    {0x3589, 0x00},
    {0x3601, 0x70},
    {0x3604, 0xe3},
    {0x3605, 0x7f},
    {0x3606, 0x80},
    {0x3608, 0xa8},
    {0x360a, 0xd0},
    {0x360b, 0x08},
    {0x360e, 0xc8},
    {0x360f, 0x66},
    {0x3610, 0x89},
    {0x3611, 0x8a},
    {0x3612, 0x4e},
    {0x3613, 0xbd},
    {0x3614, 0x9b},
    {0x362a, 0x0e},
    {0x362b, 0x0e},
    {0x362c, 0x0e},
    {0x362d, 0x0e},
    {0x362e, 0x1a},
    {0x362f, 0x34},
    {0x3630, 0x67},
    {0x3631, 0x7f},
    {0x3638, 0x00},
    {0x3643, 0x00},
    {0x3644, 0x00},
    {0x3645, 0x00},
    {0x3646, 0x00},
    {0x3647, 0x00},
    {0x3648, 0x00},
    {0x3649, 0x00},
    {0x364a, 0x04},
    {0x364c, 0x0e},
    {0x364d, 0x0e},
    {0x364e, 0x0e},
    {0x364f, 0x0e},
    {0x3650, 0xff},
    {0x3651, 0xff},
    {0x365a, 0x00},
    {0x365b, 0x00},
    {0x365c, 0x00},
    {0x365d, 0x00},
    {0x3661, 0x07},
    {0x3662, 0x02},
    {0x3663, 0x20},
    {0x3665, 0x12},
    {0x3667, 0x54},
    {0x3668, 0x80},
    {0x366c, 0x00},
    {0x366d, 0x00},
    {0x366e, 0x00},
    {0x366f, 0x00},
    {0x3671, 0x09},
    {0x3673, 0x2a},
    {0x3681, 0x80},
    {0x3700, 0x2d},
    {0x3701, 0x22},
    {0x3702, 0x25},
    {0x3703, 0x20},
    {0x3705, 0x00},
    {0x3706, 0x72},
    {0x3707, 0x0a},
    {0x3708, 0x36},
    {0x3709, 0x57},
    {0x370a, 0x01},
    {0x370b, 0x14},
    {0x3714, 0x01},
    {0x3719, 0x1f},
    {0x371b, 0x16},
    {0x371c, 0x00},
    {0x371d, 0x08},
    {0x373f, 0x63},
    {0x3740, 0x63},
    {0x3741, 0x63},
    {0x3742, 0x63},
    {0x3756, 0x9d},
    {0x3757, 0x9d},
    {0x3762, 0x1c},
    {0x376c, 0x04},
    {0x3776, 0x05},
    {0x3777, 0x22},
    {0x3779, 0x60},
    {0x377c, 0x48},
    {0x3784, 0x06},
    {0x3785, 0x0a},
    {0x3790, 0x10},
    {0x3793, 0x04},
    {0x3794, 0x07},
    {0x3796, 0x00},
    {0x3797, 0x02},
    {0x379c, 0x4d},
    {0x37a1, 0x80},
    {0x37bb, 0x88},
    {0x37be, 0x48},
    {0x37bf, 0x01},
    {0x37c0, 0x01},
    {0x37c4, 0x72},
    {0x37c5, 0x72},
    {0x37c6, 0x72},
    {0x37ca, 0x21},
    {0x37cc, 0x13},
    {0x37cd, 0x90},
    {0x37cf, 0x02},
    {0x37d0, 0x00},
    {0x37d1, 0x72},
    {0x37d2, 0x01},
    {0x37d3, 0x14},
    {0x37d4, 0x00},
    {0x37d5, 0x6c},
    {0x37d6, 0x00},
    {0x37d7, 0xf7},
    {0x37d8, 0x01},
    {0x37dc, 0x00},
    {0x37dd, 0x00},
    {0x37da, 0x00},
    {0x37db, 0x00},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x8f},
    {0x3806, 0x05},
    {0x3807, 0xff},
    {0x3808, 0x0a},
    {0x3809, 0x80},
    {0x380a, 0x05},
    {0x380b, 0xf0},
    {0x380e, 0x06},
    {0x380f, 0x58},//;1e
    {0x3811, 0x08},
    {0x3813, 0x08},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3816, 0x01},
    {0x3817, 0x01},
    {0x381c, 0x08},
    {0x3820, 0x03},
    {0x3821, 0x00},
    {0x3822, 0x14},
    {0x3823, 0x18},
    {0x3826, 0x00},
    {0x3827, 0x00},
    {0x3833, 0x41},
    {0x380c, 0x05},//;04
    {0x380d, 0x32},//;64
    {0x384c, 0x05},//;04
    {0x384d, 0x32},//;64
    {0x3858, 0x3c},
    {0x3865, 0x02},
    {0x3866, 0x00},
    {0x3867, 0x00},
    {0x3868, 0x02},
    {0x3900, 0x13},
    {0x3940, 0x13},
    {0x3980, 0x13},
    {0x3c01, 0x11},
    {0x3c05, 0x00},
    {0x3c0f, 0x1c},
    {0x3c12, 0x0d},
    {0x3c19, 0x00},
    {0x3c21, 0x00},
    {0x3c3a, 0x10},
    {0x3c3b, 0x18},
    {0x3c3d, 0xc6},
    {0x3c55, 0xcb},
    {0x3c5a, 0x55},
    {0x3c5d, 0xcf},
    {0x3c5e, 0xcf},
    {0x3d8c, 0x70},
    {0x3d8d, 0x10},
    {0x4000, 0xf9},
    {0x4001, 0xef},
    {0x4004, 0x00},
    {0x4005, 0x40},
    {0x4008, 0x02},
    {0x4009, 0x11},
    {0x400a, 0x06},
    {0x400b, 0x40},
    {0x400e, 0x40},
    {0x402e, 0x00},
    {0x402f, 0x40},
    {0x4030, 0x00},
    {0x4031, 0x40},
    {0x4032, 0x0f},
    {0x4033, 0x80},
    {0x4050, 0x00},
    {0x4051, 0x07},
    {0x4011, 0xbb},
    {0x410f, 0x01},
    {0x4288, 0xce},
    {0x4289, 0x00},
    {0x428a, 0x46},
    {0x430b, 0x0f},
    {0x430c, 0xfc},
    {0x430d, 0x00},
    {0x430e, 0x00},
    {0x4314, 0x04},
    {0x4500, 0x18},
    {0x4501, 0x18},
    {0x4503, 0x10},
    {0x4504, 0x00},
    {0x4506, 0x32},
    {0x4507, 0x03},
    {0x4601, 0x30},
    {0x4603, 0x00},
    {0x460a, 0x50},
    {0x460c, 0x60},
    {0x4640, 0x62},
    {0x4646, 0xaa},
    {0x4647, 0x55},
    {0x4648, 0x99},
    {0x4649, 0x66},
    {0x464d, 0x00},
    {0x4654, 0x11},
    {0x4655, 0x22},
    {0x4800, 0x44},
    {0x480e, 0x04},
    {0x4810, 0xff},
    {0x4811, 0xff},
    {0x4813, 0x84},
    {0x481f, 0x30},
    {0x4837, 0x0d},//;0e
    {0x484b, 0x67},
    {0x4d00, 0x4d},
    {0x4d01, 0x9d},
    {0x4d02, 0xb9},
    {0x4d03, 0x2e},
    {0x4d04, 0x4a},
    {0x4d05, 0x3d},
    {0x4d09, 0x4f},
    {0x5000, 0x1f},
    {0x5001, 0x0c},
    {0x5080, 0x00},
    {0x50c0, 0x00},
    {0x5100, 0x00},
    {0x5200, 0x00},
    {0x5201, 0x00},
    {0x5202, 0x03},
    {0x5203, 0xff},
    {0x5780, 0x53},
    {0x5782, 0x18},
    {0x5783, 0x3c},
    {0x5786, 0x01},
    {0x5788, 0x18},
    {0x5789, 0x3c},
    {0x5792, 0x11},
    {0x5793, 0x33},
    {0x5857, 0xff},
    {0x5858, 0xff},
    {0x5859, 0xff},
    {0x58d7, 0xff},
    {0x58d8, 0xff},
    {0x58d9, 0xff},
    {0x0100, 0x01},
    {0x0100, 0x01},

};

const static I2C_ARRAY Sensor_init_table_4M30fps[] =
{
    {0x0103, 0x01},
    {0x0109, 0x01},
    {0x0104, 0x02},
    {0x0102, 0x00},
    {0x0305, 0x3c},
    {0x0306, 0x00},
    {0x0307, 0x00},
    {0x0308, 0x04},
    {0x030a, 0x01},
    {0x0317, 0x09},
    {0x0322, 0x01},
    {0x0323, 0x02},
    {0x0324, 0x00},
    {0x0325, 0x90},
    {0x0327, 0x05},
    {0x0329, 0x02},
    {0x032c, 0x02},
    {0x032d, 0x02},
    {0x032e, 0x02},
    {0x300f, 0x11},
    {0x3012, 0x21},
    {0x3026, 0x10},
    {0x3027, 0x08},
    {0x302d, 0x24},
    {0x3104, 0x01},
    {0x3106, 0x11},
    {0x3400, 0x00},
    {0x3408, 0x05},
    {0x340c, 0x0c},
    {0x340d, 0xb0},
    {0x3425, 0x51},
    {0x3426, 0x10},
    {0x3427, 0x14},
    {0x3428, 0x10},
    {0x3429, 0x10},
    {0x342a, 0x10},
    {0x342b, 0x04},
    {0x3501, 0x02},
    {0x3504, 0x08},
    {0x3508, 0x01},
    {0x3509, 0x00},
    {0x350a, 0x01},
    {0x3544, 0x08},
    {0x3548, 0x01},
    {0x3549, 0x00},
    {0x3584, 0x08},
    {0x3588, 0x01},
    {0x3589, 0x00},
    {0x3601, 0x70},
    {0x3604, 0xe3},
    {0x3605, 0x7f},
    {0x3606, 0x80},
    {0x3608, 0xa8},
    {0x360a, 0xd0},
    {0x360b, 0x08},
    {0x360e, 0xc8},
    {0x360f, 0x66},
    {0x3610, 0x89},
    {0x3611, 0x8a},
    {0x3612, 0x4e},
    {0x3613, 0xbd},
    {0x3614, 0x9b},
    {0x362a, 0x0e},
    {0x362b, 0x0e},
    {0x362c, 0x0e},
    {0x362d, 0x0e},
    {0x362e, 0x1a},
    {0x362f, 0x34},
    {0x3630, 0x67},
    {0x3631, 0x7f},
    {0x3638, 0x00},
    {0x3643, 0x00},
    {0x3644, 0x00},
    {0x3645, 0x00},
    {0x3646, 0x00},
    {0x3647, 0x00},
    {0x3648, 0x00},
    {0x3649, 0x00},
    {0x364a, 0x04},
    {0x364c, 0x0e},
    {0x364d, 0x0e},
    {0x364e, 0x0e},
    {0x364f, 0x0e},
    {0x3650, 0xff},
    {0x3651, 0xff},
    {0x365a, 0x00},
    {0x365b, 0x00},
    {0x365c, 0x00},
    {0x365d, 0x00},
    {0x3661, 0x07},
    {0x3662, 0x02},
    {0x3663, 0x20},
    {0x3665, 0x12},
    {0x3667, 0xd4},
    {0x3668, 0x80},
    {0x366c, 0x00},
    {0x366d, 0x00},
    {0x366e, 0x00},
    {0x366f, 0x00},
    {0x3671, 0x08},
    {0x3673, 0x2a},
    {0x3681, 0x80},
    {0x3700, 0x2d},
    {0x3701, 0x22},
    {0x3702, 0x25},
    {0x3703, 0x20},
    {0x3705, 0x00},
    {0x3706, 0x72},
    {0x3707, 0x0a},
    {0x3708, 0x36},
    {0x3709, 0x57},
    {0x370a, 0x01},
    {0x370b, 0x14},
    {0x3714, 0x01},
    {0x3719, 0x1f},
    {0x371b, 0x16},
    {0x371c, 0x00},
    {0x371d, 0x08},
    {0x373f, 0x63},
    {0x3740, 0x63},
    {0x3741, 0x63},
    {0x3742, 0x63},
    {0x3756, 0x9d},
    {0x3757, 0x9d},
    {0x3762, 0x1c},
    {0x376c, 0x04},
    {0x3776, 0x05},
    {0x3777, 0x22},
    {0x3779, 0x60},
    {0x377c, 0x48},
    {0x3784, 0x06},
    {0x3785, 0x0a},
    {0x3790, 0x10},
    {0x3793, 0x04},
    {0x3794, 0x07},
    {0x3796, 0x00},
    {0x3797, 0x02},
    {0x379c, 0x4d},
    {0x37a1, 0x80},
    {0x37bb, 0x88},
    {0x37be, 0x48},
    {0x37bf, 0x01},
    {0x37c0, 0x01},
    {0x37c4, 0x72},
    {0x37c5, 0x72},
    {0x37c6, 0x72},
    {0x37ca, 0x21},
    {0x37cc, 0x13},
    {0x37cd, 0x90},
    {0x37cf, 0x02},
    {0x37d0, 0x00},
    {0x37d1, 0x72},
    {0x37d2, 0x01},
    {0x37d3, 0x14},
    {0x37d4, 0x00},
    {0x37d5, 0x6c},
    {0x37d6, 0x00},
    {0x37d7, 0xf7},
    {0x37d8, 0x01},
    {0x37dc, 0x00},
    {0x37dd, 0x00},
    {0x37da, 0x00},
    {0x37db, 0x00},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x0a},
    {0x3805, 0x8f},
    {0x3806, 0x05},
    {0x3807, 0xff},
    {0x3808, 0x0a},
    {0x3809, 0x80},
    {0x380a, 0x05},
    {0x380b, 0xf0},
    {0x380c, 0x05},
    {0x380d, 0xb8},
    {0x380e, 0x06},
    {0x380f, 0x66},
    {0x3811, 0x08},
    {0x3813, 0x08},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3816, 0x01},
    {0x3817, 0x01},
    {0x381c, 0x00},
    {0x3820, 0x02},
    {0x3821, 0x00},
    {0x3822, 0x14},
    {0x3823, 0x18},
    {0x3826, 0x00},
    {0x3827, 0x00},
    {0x3833, 0x40},
    {0x384c, 0x02},
    {0x384d, 0xdc},
    {0x3858, 0x3c},
    {0x3865, 0x02},
    {0x3866, 0x00},
    {0x3867, 0x00},
    {0x3868, 0x02},
    {0x3900, 0x13},
    {0x3940, 0x13},
    {0x3980, 0x13},
    {0x3c01, 0x11},
    {0x3c05, 0x00},
    {0x3c0f, 0x1c},
    {0x3c12, 0x0d},
    {0x3c19, 0x00},
    {0x3c21, 0x00},
    {0x3c3a, 0x10},
    {0x3c3b, 0x18},
    {0x3c3d, 0xc6},
    {0x3c55, 0xcb},
    {0x3c5a, 0x55},
    {0x3c5d, 0xcf},
    {0x3c5e, 0xcf},
    {0x3d8c, 0x70},
    {0x3d8d, 0x10},
    {0x4000, 0xf9},
    {0x4001, 0x2f},
    {0x4004, 0x00},
    {0x4005, 0x40},
    {0x4008, 0x02},
    {0x4009, 0x11},
    {0x400a, 0x06},
    {0x400b, 0x40},
    {0x400e, 0x40},
    {0x402e, 0x00},
    {0x402f, 0x40},
    {0x4030, 0x00},
    {0x4031, 0x40},
    {0x4032, 0x0f},
    {0x4033, 0x80},
    {0x4050, 0x00},
    {0x4051, 0x07},
    {0x4011, 0xbb},
    {0x410f, 0x01},
    {0x4288, 0xcf},
    {0x4289, 0x00},
    {0x428a, 0x46},
    {0x430b, 0x0f},
    {0x430c, 0xfc},
    {0x430d, 0x00},
    {0x430e, 0x00},
    {0x4314, 0x04},
    {0x4500, 0x18},
    {0x4501, 0x18},
    {0x4503, 0x10},
    {0x4504, 0x00},
    {0x4506, 0x32},
    {0x4507, 0x02},
    {0x4601, 0x30},
    {0x4603, 0x00},
    {0x460a, 0x50},
    {0x460c, 0x60},
    {0x4640, 0x62},
    {0x4646, 0xaa},
    {0x4647, 0x55},
    {0x4648, 0x99},
    {0x4649, 0x66},
    {0x464d, 0x00},
    {0x4654, 0x11},
    {0x4655, 0x22},
    {0x4800, 0x44},
    {0x480e, 0x00},
    {0x4810, 0xff},
    {0x4811, 0xff},
    {0x4813, 0x00},
    {0x481f, 0x30},
    {0x4837, 0x0e},
    {0x484b, 0x27},
    {0x4d00, 0x4d},
    {0x4d01, 0x9d},
    {0x4d02, 0xb9},
    {0x4d03, 0x2e},
    {0x4d04, 0x4a},
    {0x4d05, 0x3d},
    {0x4d09, 0x4f},
    {0x5000, 0x1f},
    {0x5001, 0x0d},
    {0x5080, 0x00},
    {0x50c0, 0x00},
    {0x5100, 0x00},
    {0x5200, 0x00},
    {0x5201, 0x00},
    {0x5202, 0x03},
    {0x5203, 0xff},
    {0x5780, 0x53},
    {0x5782, 0x18},
    {0x5783, 0x3c},
    {0x5786, 0x01},
    {0x5788, 0x18},
    {0x5789, 0x3c},
    {0x5792, 0x11},
    {0x5793, 0x33},
    {0x5857, 0xff},
    {0x5858, 0xff},
    {0x5859, 0xff},
    {0x58d7, 0xff},
    {0x58d8, 0xff},
    {0x58d9, 0xff},
    {0x0100, 0x01},
    {0x0100, 0x01},

/* //@@ 10 1001 LinearHCG
    {0x376c, 0x04},
    {0x3c55, 0xcb},
//@@ 10 1002 LinearLCG
    {0x376c, 0x14},
    {0x3c55, 0x08}, */
};



const static I2C_ARRAY mirror_reg[] =
{
    {0x3820, 0x02},//[2]Flip [1]mirror
};

const static I2C_ARRAY gain_reg[] = {
    {0x3508, 0x01},//long a-gain [8:4] bit[7:0]
    {0x3509, 0x00},//long a-gain [3:0] bit[7:4]
    {0x350A, 0x01},// d-gain[13:10]
    {0x350B, 0x00},// d-gain[9:2]
    {0x350C, 0x00},// d-gain[1:0] bit[7:6]
};

const static I2C_ARRAY expo_reg[] = {
    //{0x3208, 0x00},//Group 0 hold start
    {0x3501, 0x02},//long exp[15,8]
    {0x3502, 0x00},//long exp[7,0]
};
const static I2C_ARRAY vts_reg[] = {
    {0x380e, 0x06},
    {0x380f, 0x66},
};

const static I2C_ARRAY vts_reg_HDR[] = {
    {0x380e, 0x06},
    {0x380f, 0x66},
};
const static I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x3548, 0x10},//long a-gain [8:4] bit[7:0]
    {0x3549, 0x00},//long a-gain [3:0] bit[7:4]
    {0x354A, 0x10},// d-gain[13:10]
    {0x354B, 0x00},// d-gain[9:2]
    {0x354C, 0x00},// d-gain[1:0] bit[7:6]
};



const static I2C_ARRAY expo_reg_HDR_LEF[] = {
    {0x3501, 0x00},//long
    {0x3502, 0x40},
};

const static I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x3541, 0x00},//short
    {0x3542, 0x20},
};
#if 0
const static I2C_ARRAY max_short_exp_reg[] = {
    {0x3e23, 0x00}, // expo[16:8]
    {0x3e24, 0xc7}, // expo[7:0], [3:0] fraction of line
};
#endif
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period *3;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_min                   = Preview_line_period_HDR * 4;
        info->u32AEShutter_step                  = Preview_line_period_HDR * 2;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
        }
        else
        {
            info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        }
    }
    return SUCCESS;
}

static CUS_MCLK_FREQ UseParaMclk(const char *mclk)
{
/*
    CUS_CMU_CLK_27MHZ,
    CUS_CMU_CLK_21P6MHZ,
    CUS_CMU_CLK_12MHZ,
    CUS_CMU_CLK_5P4MHZ,
    CUS_CMU_CLK_36MHZ,
    CUS_CMU_CLK_54MHZ,
    CUS_CMU_CLK_43P2MHZ,
    CUS_CMU_CLK_61P7MHZ,
    CUS_CMU_CLK_72MHZ,
    CUS_CMU_CLK_48MHZ,
    CUS_CMU_CLK_24MHZ,
    CUS_CMU_CLK_37P125MHZ,
*/
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

static void pCus_PowerOn_InitChipRX(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);

    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }
}

/////////////////// sensor hardware dependent //////////////
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);////pwd low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);

    SENSOR_MSLEEP(1);
    //Configuration Chip RX
    pCus_PowerOn_InitChipRX(handle, idx);

    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, UseParaMclk(SENSOR_DRV_PARAM_MCLK()));
    SENSOR_MSLEEP(2);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_MSLEEP(1);

    SENSOR_DMSG("[%s] pwd high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_MSLEEP(2);

    params->orit = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);

    SENSOR_MSLEEP(1);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == SENSOR_HDR_MODE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->MCLK(idx, 0, UseParaMclk(SENSOR_DRV_PARAM_MCLK()));

    params->orit = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    sen_data = params->tMirror_reg[0].data;
    SENSOR_DMSG("[%s] mirror:%x\r\n", __FUNCTION__, sen_data & 0x66);
    switch(sen_data)
    {
        case 0x02:
            *orit = CUS_ORIT_M0F0;
            break;
        case 0x00:
            *orit = CUS_ORIT_M1F0;
            break;
        case 0x06:
            *orit = CUS_ORIT_M0F1;
            break;
        case 0x04:
            *orit = CUS_ORIT_M1F1;
            break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    SENSOR_DMSG(KERN_DEBUG "[%s] mirror:%x now:%d\r\n", __FUNCTION__, params->tMirror_reg[0].data & 0x66,orit);
    switch(orit) {
    case CUS_ORIT_M0F0:
        params->tMirror_reg[0].data = 0x02;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F0:
        params->tMirror_reg[0].data = 0x00;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M0F1:
        params->tMirror_reg[0].data = 0x06;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F1:
        params->tMirror_reg[0].data = 0x04;
        params->orien_dirty = true;
        break;
    default :
        break;
    }
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == SENSOR_HDR_MODE) {
        params->tMirror_reg[0].data |= 0x1;
    }
    params->orit = orit;
    return SUCCESS;
}


/////////////////// image function /////////////////////////

static int OS04A10_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

  return SUCCESS;
}

static int pCus_init_linear_4M30fps(ss_cus_sensor *handle)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_4M30fps);i++)
    {
        if(Sensor_init_table_4M30fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4M30fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4M30fps[i].reg, Sensor_init_table_4M30fps[i].data) != SUCCESS)
            {
                cnt++;
                SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }

    pCus_SetOrien(handle, params->orit);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

static int pCus_init_mipi4lane_HDR(ss_cus_sensor *handle)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    int i,cnt=0;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR_4lane);i++)
    {
        if(Sensor_init_table_HDR_4lane[i].reg==0xffff)
        {
            //MsSleep(RTK_MS_TO_TICK(1));//usleep(1000*Sensor_init_table_2560_1440_30_HDR[i].data);
            SENSOR_MSLEEP(Sensor_init_table_HDR_4lane[i].data);
        }else
        {
            cnt = 0;
            SENSOR_DMSG("reg =  %x, data = %x\n", Sensor_init_table_HDR_4lane[i].reg, Sensor_init_table_HDR_4lane[i].data);
            while(SensorReg_Write(Sensor_init_table_HDR_4lane[i].reg,Sensor_init_table_HDR_4lane[i].data) != SUCCESS)
            {
                cnt++;
                 SENSOR_DMSG("Sensor_init_table_HDR_4lane -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }

    pCus_SetOrien(handle, params->orit);
    params->tVts_reg_HDR[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg_HDR[1].data = (params->expo.vts >> 0) & 0x00ff;

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
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"2688x1520@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_4M30fps;
            vts_30fps=1638;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period  = 20350;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0://2688x1520@25fps_HDR
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi4lane_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 25;
            params->expo.max_short_exp=95;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u16 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
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
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u16 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u16 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

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

    if ((params->expo.line) > (params->expo.vts - 8))
        vts = params->expo.line +8;
    else
        vts = params->expo.vts;

    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_GetFPS_HDR(ss_cus_sensor *handle)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u16 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg_HDR[0].data << 8) | (params->tVts_reg_HDR[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR(ss_cus_sensor *handle, u32 fps)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u16 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u16 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
        return FAIL;
    }

    params->expo.max_short_exp = (((params->expo.vts)/17 - 1)>>1) << 1;
    handle->sensor_ae_info_cfg.u32AEShutter_max = Preview_line_period_HDR * params->expo.max_short_exp;
    params->tVts_reg_HDR[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg_HDR[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->vts_reg_dirty = true;
    return SUCCESS;
}

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        break;
        case CUS_FRAME_ACTIVE:
        if(params->orien_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->orien_dirty = false;
            SENSOR_DMSG(KERN_DEBUG "[%s] mirror:%x now:%d\r\n", __FUNCTION__, params->tMirror_reg[0].data, params->orit);
        }
        if(params->reg_dirty)
        {
            SensorReg_Write(0x3208, 0x00);
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            SensorReg_Write(0x3208,0x10);
            SensorReg_Write(0x3208,0xa0);
            params->reg_dirty = false;
        }
        break;
        default :
        break;
    }
    SENSOR_DMSG(KERN_DEBUG "[%s] set  exp:(0x%x%x) gain:(%x%x,%x%x%x)(%d) vts_reg value:(0x%x%x) drity:%d\n", __FUNCTION__,
        params->tExpo_reg[0].data,params->tExpo_reg[1].data, params->tGain_reg[0].data, params->tGain_reg[1].data,
        params->tGain_reg[2].data, params->tGain_reg[3].data,params->tGain_reg[4].data, params->expo.final_gain,
        params->tVts_reg[0].data,params->tVts_reg[1].data,params->reg_dirty);
    return SUCCESS;
}

static int pCus_AEStatusNotify_HDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){

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

static int pCus_AEStatusNotify_HDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->orien_dirty){
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->orien_dirty = false;
        }
        if(params->reg_dirty || params->vts_reg_dirty)
        {
            SENSOR_DMSG(KERN_DEBUG "[%s] set  lsh:(0x%x%x) ssh:(0x%x%x) lgain:(%x,%x,%x)(%d) sgain:(%x,%x,%x)(%d) vts_reg value:(0x%x%x) reg_dirty:%d vts_dirty:%d\n", __FUNCTION__,
                params->tExpo_reg[0].data, params->tExpo_reg[1].data,
                params->tExpo_reg_HDR_SEF[0].data, params->tExpo_reg_HDR_SEF[1].data,
                params->tGain_reg[0].data, params->tGain_reg[1].data,
                params->tGain_reg[2].data, params->expo.final_gain,
                params->tGain_reg_HDR_SEF[0].data, params->tGain_reg_HDR_SEF[1].data,
                params->tGain_reg_HDR_SEF[2].data, params->expo.final_gain_sef,
                params->tVts_reg_HDR[0].data, params->tVts_reg_HDR[1].data, params->reg_dirty, params->vts_reg_dirty);
            SensorReg_Write(0x3208, 0x00);
            if(params->reg_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
                params->reg_dirty = false;
            }
            if(params->vts_reg_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg_HDR, ARRAY_SIZE(vts_reg_HDR));
                //SensorRegArrayW((I2C_ARRAY*)params->tMax_short_exp_reg, ARRAY_SIZE(max_short_exp_reg));
                params->vts_reg_dirty = false;
            }
            SensorReg_Write(0x3208,0x10);
            SensorReg_Write(0x3208,0xa0);
        }
        break;
        default :
        break;
    }
    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_HDR;
    if (lines < 2) lines = 2;
    if (lines > ((params->expo.vts) - (params->expo.max_short_exp) - 8))
        lines = (params->expo.vts) - (params->expo.max_short_exp) - 8;
    else
        vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );

    params->tExpo_reg[0].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[1].data = (lines>>0) & 0x00ff;

    //pr_info("[%s] shutter %d  0x3e01 0x%x  0x3e02 0x%x\n", __FUNCTION__, us, params->tExpo_vc0_reg[0].data,params->tExpo_vc0_reg[1].data);
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xff)<<0;
    *us = (lines*Preview_line_period_HDR)/1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period_HDR;
    if (lines < 2) lines = 2;
    if (lines > ((params->expo.max_short_exp) - 2))
        lines = (params->expo.max_short_exp) - 2;
    else
        vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    // lines <<= 4;
    params->tExpo_reg_HDR_SEF[0].data = (lines>>8) & 0x00ff;
    params->tExpo_reg_HDR_SEF[1].data = (lines>>0) & 0x00ff;

    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    int rc = SUCCESS;
    u32 lines = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;

    return rc;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    OS04A10_params *params = (OS04A10_params *)handle->private_data;

    lines=(1000*us)/Preview_line_period;
    if (lines >params->expo.vts-8)
        vts = lines +8;
    else
        vts=params->expo.vts;
    params->expo.line = lines;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    // lines <<= 4;
    params->tExpo_reg[0].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[1].data = (lines>>0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->reg_dirty = true;

    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    int rc = 0;

    return rc;
}
#define MAX_A_GAIN 16384//(16*1024)
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 input_gain = 0;
    u16 gain16 = 0, tmp_dgain = 1024;

    if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN) gain = SENSOR_MAX_GAIN;

    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    /* A Gain */
    if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < MAX_A_GAIN)) {
        gain = (gain>>9)<<9;
    } else {
        gain = MAX_A_GAIN;
    }

    gain16=(u16)(gain>>2);
    tmp_dgain = ((input_gain*1024)/gain);
    params->tGain_reg[0].data = (gain16>>8)&0x1f;//high bit
    params->tGain_reg[1].data = gain16&0xf0; //low byte

    params->tGain_reg[2].data=(u16)((tmp_dgain >> 10) & 0x0F);
    params->tGain_reg[3].data=(u16)((tmp_dgain >> 2) & 0xFF);
    params->tGain_reg[4].data=(u16)((tmp_dgain&0x03)<<6);
    params->expo.final_gain = gain;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_SetAEGain_HDR_SEF(ss_cus_sensor *handle, u32 gain) {
    OS04A10_params *params = (OS04A10_params *)handle->private_data;
    u32 input_gain = 0;
    u16 gain16 = 0, tmp_dgain = 1024;

    if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN) gain = SENSOR_MAX_GAIN;

    //gain = 4*gain;
    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    /* A Gain */
    if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < MAX_A_GAIN)) {
        gain = (gain>>9)<<9;
    } else {
        gain = MAX_A_GAIN;
    }

    gain16=(u16)(gain>>2);
    tmp_dgain = ((input_gain*1024)/gain);
    params->tGain_reg_HDR_SEF[0].data = (gain16>>8)&0x1f;//high bit
    params->tGain_reg_HDR_SEF[1].data = gain16&0xf0; //low byte

    params->tGain_reg_HDR_SEF[2].data=(u16)((tmp_dgain >> 10) & 0x0F);
    params->tGain_reg_HDR_SEF[3].data=(u16)((tmp_dgain >> 2) & 0xFF);
    params->tGain_reg_HDR_SEF[4].data=(u16)((tmp_dgain&0x03)<<6);
    params->expo.final_gain_sef = gain;
    params->reg_dirty = true;
    return SUCCESS;
}


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
            SensorReg_Read(reg->reg, (void*)&reg->data);
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

int cus_camsensor_init_handle(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    OS04A10_params *params;
    u8 res=0;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }

    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }

    params = (OS04A10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ///////////////////////////////////////////////////////
    // Sensor stream name
    ///////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OS04A10_MIPI");

    ///////////////////////////////////////////////////////
    // Sensor interface info
    ///////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC;
    handle->bayer_id                                              = SENSOR_BAYERID;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;

    ///////////////////////////////////////////////////////
    // Sensor i2c configuration
    ///////////////////////////////////////////////////////
    handle->i2c_cfg.mode                                          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                                           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address                                       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed                                         = SENSOR_I2C_SPEED;

    ///////////////////////////////////////////////////////
    // Sensor Power configuration
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_poweron                                   = pCus_poweron;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff;

    ///////////////////////////////////////////////////////
    // Sensor resolution capability
    ///////////////////////////////////////////////////////
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_linear_4M30fps;

    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;

    handle->pCus_sensor_SetPatternMode                            = OS04A10_SetPatternMode;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////

    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    Preview_line_period  = 20350;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period *3;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    //nPixelSize
    handle->sensor_ae_info_cfg.u32PixelSize                       = 0;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_CustDefineFunction                        = pCus_sensor_CustDefineFunction;

    params->expo.vts                                              = vts_30fps;
    params->expo.fps                                              = 30;
    params->expo.line                                             = 1000;
    params->reg_dirty                                             = false;
    params->orien_dirty                                           = false;

    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_SEF(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OS04A10_params *params;
    u8 res=0;

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
    params = (OS04A10_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OS04A10_MIPI_HDR_SEF");


    memcpy(params->tVts_reg_HDR, vts_reg_HDR, sizeof(vts_reg_HDR));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    //memcpy(params->tMax_short_exp_reg, max_short_exp_reg, sizeof(max_short_exp_reg));

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_HDR;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = SENSOR_HDR_MODE;
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

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_mipi4lane_HDR;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify_HDR_SEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs_HDR_SEF;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs_HDR_SEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain_HDR_SEF;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;

    ///////////////////////////////////////////////////////
    // Private area
    ////////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps_HDR;
    params->expo.line                                             = 1000;
    params->expo.fps                                              = 30;
    params->expo.max_short_exp                                    = 199;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*4;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR*2;

    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    OS04A10_params *params;
    u8 res=0;

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
    params = (OS04A10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OS04A10_MIPI_HDR_LEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_HDR;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_HDR;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = SENSOR_HDR_MODE;
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

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = NULL;
    handle->pCus_sensor_GetVideoResNum                            = NULL;
    handle->pCus_sensor_GetVideoRes                               = NULL;
    handle->pCus_sensor_GetCurVideoRes                            = NULL;
    handle->pCus_sensor_SetVideoRes                               = NULL;

    handle->pCus_sensor_GetOrien                                  = NULL;
    handle->pCus_sensor_SetOrien                                  = NULL;
    handle->pCus_sensor_GetFPS                                    = NULL;
    handle->pCus_sensor_SetFPS                                    = NULL;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify_HDR_LEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs_HDR_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*4;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR*2;

    ///////////////////////////////////////////////////////
    // Private area
    ////////////////////////////////////////////////////////
    handle->pCus_sensor_CustDefineFunction                        = pCus_sensor_CustDefineFunction;
    params->expo.vts                                              = vts_30fps_HDR;
    params->expo.fps                                              = 30;
    params->expo.max_short_exp                                    = 384;
    params->reg_dirty                                             = false;
    params->orien_dirty                                           = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX( OS04A10,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_HDR_SEF,
                            cus_camsensor_init_handle_HDR_LEF,
                            OS04A10_params
                         );

