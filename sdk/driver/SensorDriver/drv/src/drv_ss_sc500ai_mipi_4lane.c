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
   Date : 2023/9/25
   Build on : Master V4  I6C
   Verified on : not yet。
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(sc500ai);

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
#define SENSOR_NAME                           sc500ai

#define SENSOR_IFBUS_TYPE                     CUS_SENIF_BUS_MIPI
#define SENSOR_HDR_MODE                       CUS_HDR_MODE_VC
//Linear Mode
#define SENSOR_MIPI_LANE_NUM                  (4)
#define Preview_MCLK_SPEED                    CUS_CMU_CLK_24MHZ

//HDR mode
#define SENSOR_MIPI_LANE_NUM_HDR              (4)

//I2C bus
#define SENSOR_I2C_ADDR                       0x60
#define SENSOR_I2C_SPEED                      20000
#define SENSOR_I2C_LEGACY                     I2C_NORMAL_MODE
#define SENSOR_I2C_FMT                        I2C_FMT_A16D8

#define SENSOR_RGBIRID                        CUS_RGBIR_NONE

#define SENSOR_DATAPREC                       CUS_DATAPRECISION_10
#define SENSOR_BAYERID                        CUS_BAYER_BG

#define SENSOR_DATAPREC_HDR                   CUS_DATAPRECISION_10
#define SENSOR_BAYERID_HDR                    CUS_BAYER_BG

//AE info
#define SENSOR_MAX_GAIN                       ((24067*3175)/100000)
#define SENSOR_MIN_GAIN                       (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (1)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

#define SENSOR_GAIN_DELAY_FRAME_COUNT_HDR     (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR  (2)
#define SENSOR_GAIN_CTRL_NUM_DOL              (2)
#define SENSOR_SHUTTER_CTRL_NUM_DOL           (2)

#define ENABLE_NR 1
u32 Preview_line_period_HDR = 10101;
u32 vts_30fps_HDR = 3300;
u32 Preview_line_period = 20202;
u32 vts_30fps = 1650;


//============================================
//
//    SENSOR Resolution List
//
//============================================

static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2, LINEAR_RES_END}mode;
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
    {LINEAR_RES_1, {2880, 1620, 3, 30}, {0, 0, 2880, 1620}, {"2880x1620@30fps"}},
    {LINEAR_RES_2, {2880, 1620, 3, 60}, {480, 270, 1920, 1080}, {"1920x1080@60fps"}},
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
    {HDR_RES_1, {2880, 1620, 3, 30}, {0, 0, 2880, 1620}, {"2880x1620@30fps_HDR"}}, // Modify it
};

typedef struct {
    struct {
        u32 expo_lines;
        u32 expo_lef_us;
        u32 expo_sef_us;
        u32 vts;
        u32 final_gain;
        u32 fps;
        u32 preview_fps;
        u32 line;
        u32 max_short_exp;
    } expo;

    u32 skip_cnt;
    CUS_CAMSENSOR_ORIT  orit;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tVts_reg_HDR[3];
    I2C_ARRAY tMax_short_exp_reg[3];
    I2C_ARRAY tGain_reg_HDR_SEF[4];
    I2C_ARRAY tGain_reg_HDR_LEF[3];
    I2C_ARRAY tExpo_reg_HDR_SEF[3];
    I2C_ARRAY tExpo_reg_HDR_LEF[3];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
#if ENABLE_NR
    I2C_ARRAY tTemperature_reg[1];
#endif
    bool orien_dirty;
    bool reg_dirty;
    bool vts_reg_dirty;
    bool dirty;
    bool change;
    bool temperature_dirty;
} sc500ai_params;

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)     (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)     (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

// set sensor ID address and data,
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xce},
    {0x3108, 0x39},
};

const static I2C_ARRAY Sensor_init_table_5M30fps[] =
{//cleaned_0x3c_FT_SC500AI_24MInput_MIPI_4lane_396Mbps_10bit_2880x1620_30fps
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x301f,0x3c},
    {0x3250,0x40},
    {0x3253,0x0a},
    {0x3301,0x0a},
    {0x3302,0x18},
    {0x3303,0x10},
    {0x3304,0x60},
    {0x3306,0x60},
    {0x3308,0x10},
    {0x3309,0x70},
    {0x330a,0x00},
    {0x330b,0xf0},
    {0x330d,0x18},
    {0x330e,0x20},
    {0x330f,0x02},
    {0x3310,0x02},
    {0x331c,0x04},
    {0x331e,0x51},
    {0x331f,0x61},
    {0x3320,0x09},
    {0x3333,0x10},
    {0x334c,0x08},
    {0x3356,0x09},
    {0x3364,0x17},
    {0x336d,0x03},
    {0x3390,0x08},
    {0x3391,0x18},
    {0x3392,0x38},
    {0x3393,0x0a},
    {0x3394,0x20},
    {0x3395,0x20},
    {0x3396,0x08},
    {0x3397,0x18},
    {0x3398,0x38},
    {0x3399,0x0a},
    {0x339a,0x20},
    {0x339b,0x20},
    {0x339c,0x20},
    {0x33ac,0x10},
    {0x33ae,0x10},
    {0x33af,0x19},
    {0x360f,0x01},
    {0x3622,0x03},
    {0x363a,0x1f},
    {0x363c,0x40},
    {0x3651,0x7d},
    {0x3670,0x0a},
    {0x3671,0x07},
    {0x3672,0x17},
    {0x3673,0x1e},
    {0x3674,0x82},
    {0x3675,0x64},
    {0x3676,0x66},
    {0x367a,0x48},
    {0x367b,0x78},
    {0x367c,0x58},
    {0x367d,0x78},
    {0x3690,0x34},
    {0x3691,0x34},
    {0x3692,0x54},
    {0x369c,0x48},
    {0x369d,0x78},
    {0x36ea,0x35},
    {0x36eb,0x0c},
    {0x36ec,0x1a},
    {0x36ed,0x34},
    {0x36fa,0x35},
    {0x36fb,0x35},
    {0x36fc,0x10},
    {0x36fd,0x34},
    {0x3904,0x04},
    {0x3908,0x41},
    {0x391d,0x04},
    {0x39c2,0x30},
    {0x3e01,0xcd},
    {0x3e02,0xc0},
    {0x3e16,0x00},
    {0x3e17,0x80},
    {0x4500,0x88},
    {0x4509,0x20},
    {0x5799,0x00},
    {0x59e0,0x60},
    {0x59e1,0x08},
    {0x59e2,0x3f},
    {0x59e3,0x18},
    {0x59e4,0x18},
    {0x59e5,0x3f},
    {0x59e7,0x02},
    {0x59e8,0x38},
    {0x59e9,0x20},
    {0x59ea,0x0c},
    {0x59ec,0x08},
    {0x59ed,0x02},
    {0x59ee,0xa0},
    {0x59ef,0x08},
    {0x59f4,0x18},
    {0x59f5,0x10},
    {0x59f6,0x0c},
    {0x59f9,0x02},
    {0x59fa,0x18},
    {0x59fb,0x10},
    {0x59fc,0x0c},
    {0x59ff,0x02},
    {0x36e9,0x20},
    {0x36f9,0x53},
    {0x0100,0x01},
    {0xffff,0x0a},
};

const static I2C_ARRAY Sensor_init_table_5M60fps[] =
{//cleaned_0x05_SC500AI_MIPI_27Minput_4lane_792Mbps_10bit_2880x1620_60fps_continue_mode   update 20220425
	{0x0103,0x01},
	{0x0100,0x00},
	{0x36e9,0x80},
	{0x36f9,0x80},
	{0x301f,0x05},
	{0x3106,0x01},
	{0x3250,0x40},
	{0x3253,0x0a},
	{0x3301,0x0b},
	{0x3302,0x20},
	{0x3303,0x10},
	{0x3304,0x70},
	{0x3306,0x50},
	{0x3308,0x18},
	{0x3309,0x80},
	{0x330a,0x00},
	{0x330b,0xe8},
	{0x330d,0x30},
	{0x330e,0x30},
	{0x330f,0x02},
	{0x3310,0x02},
	{0x331c,0x08},
	{0x331e,0x61},
	{0x331f,0x71},
	{0x3320,0x11},
	{0x3333,0x10},
	{0x334c,0x10},
	{0x3356,0x11},
	{0x3364,0x17},
	{0x336d,0x03},
	{0x3390,0x08},
	{0x3391,0x18},
	{0x3392,0x38},
	{0x3393,0x0a},
	{0x3394,0x0a},
	{0x3395,0x12},
	{0x3396,0x08},
	{0x3397,0x18},
	{0x3398,0x38},
	{0x3399,0x0a},
	{0x339a,0x0a},
	{0x339b,0x0a},
	{0x339c,0x12},
	{0x33ac,0x10},
	{0x33ae,0x20},
	{0x33af,0x21},
	{0x360f,0x01},
	{0x3621,0xe8},
	{0x3622,0x06},
	{0x3630,0x82},
	{0x3633,0x33},
	{0x3634,0x64},
	{0x3637,0x50},
	{0x363a,0x1f},
	{0x363c,0x40},
	{0x3651,0x7d},
	{0x3670,0x0a},
	{0x3671,0x06},
	{0x3672,0x16},
	{0x3673,0x17},
	{0x3674,0x82},
	{0x3675,0x62},
	{0x3676,0x44},
	{0x367a,0x48},
	{0x367b,0x78},
	{0x367c,0x48},
	{0x367d,0x58},
	{0x3690,0x34},
	{0x3691,0x34},
	{0x3692,0x54},
	{0x369c,0x48},
	{0x369d,0x78},
	{0x36ea,0x35},
	{0x36eb,0x04},
	{0x36ec,0x0a},
	{0x36ed,0x14},
	{0x36fa,0x35},
	{0x36fb,0x04},
	{0x36fc,0x00},
	{0x36fd,0x16},
	{0x3904,0x04},
	{0x3908,0x41},
	{0x391f,0x10},
	{0x39c2,0x30},
	{0x3e01,0xcd},
	{0x3e02,0xc0},
	{0x4500,0x88},
	{0x4509,0x20},
	{0x4800,0x04},
	{0x4837,0x15},
	{0x36e9,0x44},
	{0x36f9,0x44},
	{0x0100,0x01},
    {0xffff,0x0a},
};

//2880x1620@30fps_HDR
const static I2C_ARRAY Sensor_init_table_HDR_4lane[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x301f,0x23},
    {0x3106,0x01},
    {0x320e,0x0c},
    {0x320f,0xe4},
    {0x3220,0x53},
    {0x3250,0x3f},
    {0x3253,0x0a},
    {0x3301,0x0b},
    {0x3302,0x20},
    {0x3303,0x10},
    {0x3304,0x70},
    {0x3306,0x50},
    {0x3308,0x18},
    {0x3309,0x80},
    {0x330a,0x00},
    {0x330b,0xe8},
    {0x330d,0x30},
    {0x330e,0x30},
    {0x330f,0x02},
    {0x3310,0x02},
    {0x331c,0x08},
    {0x331e,0x61},
    {0x331f,0x71},
    {0x3320,0x11},
    {0x3333,0x10},
    {0x334c,0x10},
    {0x3356,0x11},
    {0x3364,0x17},
    {0x336d,0x03},
    {0x3390,0x08},
    {0x3391,0x18},
    {0x3392,0x38},
    {0x3393,0x0a},
    {0x3394,0x0a},
    {0x3395,0x12},
    {0x3396,0x08},
    {0x3397,0x18},
    {0x3398,0x38},
    {0x3399,0x0a},
    {0x339a,0x0a},
    {0x339b,0x0a},
    {0x339c,0x12},
    {0x33ac,0x10},
    {0x33ae,0x20},
    {0x33af,0x21},
    {0x360f,0x01},
    {0x3621,0xe8},
    {0x3622,0x06},
    {0x3630,0x82},
    {0x3633,0x33},
    {0x3634,0x64},
    {0x3637,0x50},
    {0x363a,0x1f},
    {0x363c,0x40},
    {0x3651,0x7d},
    {0x3670,0x0a},
    {0x3671,0x06},
    {0x3672,0x16},
    {0x3673,0x17},
    {0x3674,0x82},
    {0x3675,0x62},
    {0x3676,0x44},
    {0x367a,0x48},
    {0x367b,0x78},
    {0x367c,0x48},
    {0x367d,0x58},
    {0x3690,0x34},
    {0x3691,0x34},
    {0x3692,0x54},
    {0x369c,0x48},
    {0x369d,0x78},
    {0x36ea,0x2d},
    {0x36eb,0x04},
    {0x36ec,0x0a},
    {0x36ed,0x14},
    {0x36fa,0x35},
    {0x36fb,0x04},
    {0x36fc,0x00},
    {0x36fd,0x16},
    {0x3904,0x04},
    {0x3908,0x41},
    {0x391f,0x10},
    {0x39c2,0x30},
    {0x3e00,0x01},
    {0x3e01,0x82},
    {0x3e02,0x00},
    {0x3e04,0x18},
    {0x3e05,0x20},
    {0x3e23,0x00},
    {0x3e24,0xc6},
    {0x4500,0x88},
    {0x4509,0x20},
    {0x4800,0x24}, //bit5; 0:continous-mode; 1:non
    {0x4837,0x14},
    {0x4853,0xfd},
    {0x36e9,0x30},
    {0x36f9,0x44},
    {0x0100,0x01},
    {0xffff,0x0a},
};

const static I2C_ARRAY mirror_reg[] =
{
    {0x3221, 0x00}, // mirror[2:1], flip[6:5]
};

const static I2C_ARRAY gain_reg[] = {
    {0x3e06, 0x00},
    {0x3e07, 0x80},
    {0x3e08, 0x03},
    {0x3e09, 0x40}, //low bit, 0x40 - 0x7f, step 1/64
};

const static I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x3e10, 0x00},
    {0x3e11, 0x00},
    {0x3e12, 0x03},
    {0x3e13, 0x40}, //low bit, 0x40 - 0x7f, step 1/64
};

const static I2C_ARRAY expo_reg[] = {
    {0x3e00, 0x00}, //expo [20:17]
    {0x3e01, 0x30}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
};

const static I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x3e04, 0x00}, // expo[16:8]
    {0x3e05, 0x80}, // expo[7:0], [3:0] fraction of line
};
#if 0
const static I2C_ARRAY max_short_exp_reg[] = {
    {0x3e23, 0x00}, // expo[16:8]
    {0x3e24, 0xc7}, // expo[7:0], [3:0] fraction of line
};
#endif
const static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x05},
    {0x320f, 0xdc},
};

const static I2C_ARRAY vts_reg_HDR[] = {
    {0x320e, 0x05},
    {0x320f, 0xdc},
};
#if ENABLE_NR
const I2C_ARRAY temperature_reg[] = {
    {0x5799, 0x00},
};
#endif

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
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    sc500ai_params *params = (sc500ai_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period *3/2;
        info->u32AEShutter_step                  = Preview_line_period/2;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_min                   = Preview_line_period_HDR*2;
        info->u32AEShutter_step                  = Preview_line_period_HDR/2;
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
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sc500ai_params *params = (sc500ai_params *)handle->private_data;

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
    sc500ai_params *params = (sc500ai_params *)handle->private_data;

    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);

    SENSOR_MSLEEP(1);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    sensor_if->MCLK(idx, 0, UseParaMclk(SENSOR_DRV_PARAM_MCLK()));

    params->orit = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    sen_data = params->tMirror_reg[0].data;
    SENSOR_DMSG("[%s] mirror:%x\r\n", __FUNCTION__, sen_data & 0x66);
    switch(sen_data & 0x66)
    {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
            break;
        case 0x06:
            *orit = CUS_ORIT_M1F0;
            break;
        case 0x60:
            *orit = CUS_ORIT_M0F1;
            break;
        case 0x66:
            *orit = CUS_ORIT_M1F1;
            break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    sc500ai_params *params = (sc500ai_params *)handle->private_data;

    switch(orit) {
    case CUS_ORIT_M0F0:
        params->tMirror_reg[0].data = 0;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F0:
        params->tMirror_reg[0].data = 6;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M0F1:
        params->tMirror_reg[0].data = 0x60;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F1:
        params->tMirror_reg[0].data = 0x66;
        params->orien_dirty = true;
        break;
    default :
        break;
    }

    params->orit = orit;
    return SUCCESS;
}


/////////////////// image function /////////////////////////

static int sc500ai_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

  return SUCCESS;
}

static int pCus_init_linear_5M30fps(ss_cus_sensor *handle)
{
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_5M30fps);i++)
    {
        if(Sensor_init_table_5M30fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_5M30fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_5M30fps[i].reg, Sensor_init_table_5M30fps[i].data) != SUCCESS)
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

static int pCus_init_linear_5M60fps(ss_cus_sensor *handle)
{
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_5M60fps);i++)
    {
        if(Sensor_init_table_5M60fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_5M60fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_5M60fps[i].reg, Sensor_init_table_5M60fps[i].data) != SUCCESS)
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
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
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
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"2880x1620@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_5M30fps;
            vts_30fps = 1650;
            params->expo.vts = vts_30fps;
            Preview_line_period = 20202;
            params->expo.fps = 30;
            break;
        case 1: //"MAX: 2880x1620@60fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_5M60fps;
            vts_30fps = 1650;
            params->expo.vts = vts_30fps;
            Preview_line_period = 10101;
            params->expo.fps = 30;
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
    sc500ai_params *params = (sc500ai_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            Preview_line_period_HDR = 10101;
            vts_30fps_HDR = 3300;
            handle->pCus_sensor_init = pCus_init_mipi4lane_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short_exp=198;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
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
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
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

    if(params->expo.line > 2 * (params->expo.vts) -10){
        vts = (params->expo.line + 11)/2;
    }else{
        vts = params->expo.vts;
    }
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_GetFPS_HDR(ss_cus_sensor *handle)
{
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
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
    #if 0
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
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
    params->tVts_reg_HDR[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg_HDR[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->tMax_short_exp_reg[0].data = (params->expo.max_short_exp >> 8) & 0x00ff;
    params->tMax_short_exp_reg[1].data = (params->expo.max_short_exp >> 0) & 0x00ff;
    params->vts_reg_dirty = true;
    return SUCCESS;
    #endif
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
      params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->tVts_reg_HDR[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg_HDR[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        return SUCCESS;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->tVts_reg_HDR[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg_HDR[1].data = (params->expo.vts >> 0) & 0x00ff;
        params->reg_dirty = true;
        //pr_info("[%s]  vts_reg_sef : %x , %x\n\n", __FUNCTION__,vts_reg[0].data,vts_reg[1].data);
        return SUCCESS;
    }else{
      return FAIL;
    }
}

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        break;
        case CUS_FRAME_ACTIVE:
        if(params->orien_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->orien_dirty = false;
        }
        if(params->reg_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            params->reg_dirty = false;
        }
        if(params->temperature_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tTemperature_reg, ARRAY_SIZE(temperature_reg));
            params->temperature_dirty = false;
        }
        break;
        default :
        break;
    }
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
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
        if(params->orien_dirty){
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
            params->orien_dirty = false;
        }
        if(params->reg_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            params->reg_dirty = false;
        }
        if(params->temperature_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tTemperature_reg, ARRAY_SIZE(temperature_reg));
            params->temperature_dirty = false;
        }
        break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0,vts = 0;
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));
    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    half_lines = 4*dou_lines;
    if(half_lines<8) half_lines=8;
    if (half_lines >  2 * (params->expo.vts - params->expo.max_short_exp) - 18) {
        half_lines = 2 * (params->expo.vts - params->expo.max_short_exp) - 18;
    }
    else
        vts=params->expo.vts;

    half_lines = half_lines<<4;
    params->tExpo_reg[0].data = (half_lines>>16) & 0x0f;
    params->tExpo_reg[1].data =  (half_lines>>8) & 0xff;
    params->tExpo_reg[2].data = (half_lines>>0) & 0xf0;

    for (i = 0; i < ARRAY_SIZE(expo_reg); i++)
    {
      if (params->tExpo_reg[i].data != expo_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 *us) {

    u32 lines = 0;
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period_HDR)/1000/2;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0,vts = 0;
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {
        {0x3e22, 0x00}, // expo[7:0]
        {0x3e04, 0x18}, // expo[7:0]
        {0x3e05, 0x00}, // expo[7:4]
    };
    memcpy(expo_reg_temp, params->tExpo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));

    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    half_lines = 4*dou_lines;
    if(half_lines<8) half_lines=8;
    if (half_lines >  2 * (params->expo.max_short_exp)-14) {
        half_lines = 2 * (params->expo.max_short_exp)-14;
    }else
        vts=params->expo.vts;

    half_lines = half_lines<<4;
    params->tExpo_reg_HDR_SEF[0].data =  (half_lines>>8) & 0xff;
    params->tExpo_reg_HDR_SEF[1].data = (half_lines>>0) & 0xf0;
    for (i = 0; i < ARRAY_SIZE(expo_reg_HDR_SEF); i++)
    {
      if (params->tExpo_reg_HDR_SEF[i].data != expo_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    int rc=0;
    u32 lines = 0;
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0x0f)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period)/1000/2; //return us

  SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
  return rc;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    int i;
    u32 half_lines = 0, lines = 0, vts = 0;
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
        {0x3e00, 0x00}, //expo [20:17]
        {0x3e01, 0xcd}, // expo[16:8]
        {0x3e02, 0xa0}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg_temp));

    lines = (1000*us)/Preview_line_period; // Preview_line_period in ns
    half_lines = 2*lines;
    if(half_lines<=3) half_lines=3;
    if (half_lines > 2*(params->expo.vts)-10) {
        vts = (half_lines+11)/2;
    }
    else
        vts=params->expo.vts;
    params->expo.line = half_lines;

    SENSOR_DMSG("[%s] us %d, half_lines %d, vts %d\n", __FUNCTION__, us, half_lines, params->expo.vts);

    half_lines = half_lines<<4;
    params->tExpo_reg[0].data = (half_lines>>16) & 0x0f;
    params->tExpo_reg[1].data = (half_lines>>8) & 0xff;
    params->tExpo_reg[2].data = (half_lines>>0) & 0xf0;
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    for (i = 0; i < ARRAY_SIZE(expo_reg); i++)
    {
      if (params->tExpo_reg[i].data != expo_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
     }
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    int rc = 0;

    return rc;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    u8 i=0 , Coarse_gain = 1,DIG_gain=1;
    u32 Dcg_gainx100 = 1, ANA_Fine_gainx64 = 1,DIG_Fine_gainx1000 =1;
    u8 Dcg_gain_reg = 0, Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x40,DIG_Fine_gain_reg=0x80;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e08, 0x00|0x03},
        {0x3e09, 0x40},
    };
#if ENABLE_NR
    I2C_ARRAY temperature_reg_temp[] ={
        {0x5799, 0x00},
    };
#endif
    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg_temp));
    memcpy(temperature_reg_temp, params->tTemperature_reg, sizeof(temperature_reg_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN*1024) {
        gain = SENSOR_MAX_GAIN*1024;
    }

    if (gain < 1552) // start again  1.516 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 1;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 0;  Coarse_gain_reg = 0x03; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 3104) // 3.032 * 1024
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 1;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x23; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 6209) // 6.064 * 1024
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 2;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x27; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 12419)// 12.128 * 1024
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 4;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x2f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 24644)// 24.067 * 1024 // end again
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (gain <= 24644 * 2) // start dgain
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=1;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 24644 * 4)
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=2;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x1;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 24644 * 8)
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=4;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x3;  ANA_Fine_gain_reg=0x7f;
    }
   else if (gain <= 24644 * 16)
    {
       Dcg_gainx100 = 1516;      Coarse_gain = 8;      DIG_gain=8;        ANA_Fine_gainx64=127;
       Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x7;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 782466)
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=16;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0xF;  ANA_Fine_gain_reg=0x7f;
    }
#endif

    if(gain == 1552){
        ANA_Fine_gain_reg = 0x40;
    }else if(gain <= 24644)
    {
        ANA_Fine_gain_reg = abs(1000ULL * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    }else{
        DIG_Fine_gain_reg = abs(1000ULL * gain /(Dcg_gainx100 * DIG_gain) / ANA_Fine_gainx64);//*8000/coarse_gain = *1000
    }
    params->tGain_reg[3].data = ANA_Fine_gain_reg;
    params->tGain_reg[2].data = Coarse_gain_reg;
    params->tGain_reg[1].data = DIG_Fine_gain_reg;
    params->tGain_reg[0].data = DIG_gain_reg & 0xF;

    for (i = 0; i < ARRAY_SIZE(params->tGain_reg); i++)
    {
      if (params->tGain_reg[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }
#if 1
    if(gain >= 30 * 1024)
    {
        params->tTemperature_reg[0].data = 0x07;
    }
    else if(gain <= 20 * 1024)
    {
        params->tTemperature_reg[0].data = 0x00;
    }
    for (i = 0; i < ARRAY_SIZE(temperature_reg_temp); i++)
    {
      if (params->tTemperature_reg[i].data != temperature_reg_temp[i].data)
      {
        params->temperature_dirty = true;
        break;
      }
    }
#endif
    return SUCCESS;
}

static int pCus_SetAEGain_HDR_SEF(ss_cus_sensor *handle, u32 gain) {
    sc500ai_params *params = (sc500ai_params *)handle->private_data;
    u8 i=0 , Coarse_gain = 1,DIG_gain=1;
    u32 Dcg_gainx100 = 1, ANA_Fine_gainx64 = 1,DIG_Fine_gainx1000 =1;
    u8 Dcg_gain_reg = 0, Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x40,DIG_Fine_gain_reg=0x80;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e10, 0x00},
        {0x3e11, 0x80},
        {0x3e12, 0x00|0x03},
        {0x3e13, 0x40},
    };
    memcpy(gain_reg_temp, params->tGain_reg_HDR_SEF, sizeof(gain_reg_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN*1024) {
        gain = SENSOR_MAX_GAIN*1024;
    }

    if (gain < 1552) // start again  1.516 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 1;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 0;  Coarse_gain_reg = 0x03; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 3104) // 3.032 * 1024
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 1;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x23; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 6209) // 6.064 * 1024
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 2;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x27; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 12419)// 12.128 * 1024
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 4;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x2f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 24644)// 24.067 * 1024 // end again
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=1;       DIG_Fine_gainx1000=1000;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (gain <= 24644 * 2) // start dgain
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=1;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 24644 * 4)
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=2;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x1;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 24644 * 8)
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=4;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x3;  ANA_Fine_gain_reg=0x7f;
    }
   else if (gain <= 24644 * 16)
    {
       Dcg_gainx100 = 1516;      Coarse_gain = 8;      DIG_gain=8;        ANA_Fine_gainx64=127;
       Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0x7;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 782466)
    {
        Dcg_gainx100 = 1516;      Coarse_gain = 8;     DIG_gain=16;       ANA_Fine_gainx64=127;
        Dcg_gain_reg = 1;  Coarse_gain_reg = 0x3f; DIG_gain_reg=0xF;  ANA_Fine_gain_reg=0x7f;
    }
#endif

    if(gain == 1552) // || gain == 1552)
    {
        ANA_Fine_gain_reg = 0x40;
    }
    else if(gain <= 24644)
    {
        ANA_Fine_gain_reg = abs(1000ULL * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    }else{
        DIG_Fine_gain_reg = abs(1000ULL * gain /(Dcg_gainx100 * DIG_gain) / ANA_Fine_gainx64);
    }

    params->tGain_reg_HDR_SEF[3].data = ANA_Fine_gain_reg;
    params->tGain_reg_HDR_SEF[2].data = Coarse_gain_reg;
    params->tGain_reg_HDR_SEF[1].data = DIG_Fine_gain_reg;
    params->tGain_reg_HDR_SEF[0].data = DIG_gain_reg & 0xF;

    for (i = 0; i < ARRAY_SIZE(gain_reg_HDR_SEF); i++)
    {
      if (params->tGain_reg_HDR_SEF[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }
#if 1
    if(gain >= 16 * 1024)
    {
        params->tTemperature_reg[0].data = 0x07;
    }
    else if(gain <= 10 * 1024)
    {
        params->tTemperature_reg[0].data = 0x00;
    }
    for (i = 0; i < ARRAY_SIZE(temperature_reg); i++)
    {
      if (params->tTemperature_reg[i].data != temperature_reg[i].data)
      {
        params->temperature_dirty = true;
        break;
      }
    }
#endif

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
    sc500ai_params *params;
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

    params = (sc500ai_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
#if ENABLE_NR
    memcpy(params->tTemperature_reg, temperature_reg, sizeof(temperature_reg));
#endif
    ///////////////////////////////////////////////////////
    // Sensor stream name
    ///////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"sc500ai_MIPI");

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
    handle->pCus_sensor_init                                      = pCus_init_linear_5M30fps;

    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;

    handle->pCus_sensor_SetPatternMode                            = sc500ai_SetPatternMode;

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
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period *3/2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period / 2;

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
    sc500ai_params *params;
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
    params = (sc500ai_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"sc500ai_MIPI_HDR_SEF");


    memcpy(params->tVts_reg_HDR, vts_reg_HDR, sizeof(vts_reg_HDR));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    //memcpy(params->tMax_short_exp_reg, max_short_exp_reg, sizeof(max_short_exp_reg));
#if ENABLE_NR
    memcpy(params->tTemperature_reg, temperature_reg, sizeof(temperature_reg));
#endif
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
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR/2;
    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    sc500ai_params *params;
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
    params = (sc500ai_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"sc500ai_MIPI_HDR_LEF");
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
#if ENABLE_NR
    memcpy(params->tTemperature_reg, temperature_reg, sizeof(temperature_reg));
#endif
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
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR/2;

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

SENSOR_DRV_ENTRY_IMPL_END_EX( sc500ai,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_HDR_SEF,
                            cus_camsensor_init_handle_HDR_LEF,
                            sc500ai_params
                         );

