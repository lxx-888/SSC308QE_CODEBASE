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
   Verified on : mixer preview ok (linear /Hdr ),
               AE gain/shutter ok , IQ not verify,sensor module w/o lens so just check hist.
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC230ai);

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
#define SENSOR_NAME                           SC230ai

#define SENSOR_IFBUS_TYPE                     CUS_SENIF_BUS_MIPI
#define SENSOR_HDR_MODE                       CUS_HDR_MODE_VC
//Linear Mode
#define SENSOR_MIPI_LANE_NUM                  (2)
#define Preview_MCLK_SPEED                    CUS_CMU_CLK_27MHZ

//HDR mode
#define SENSOR_MIPI_LANE_NUM_HDR              (2)

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
#define SENSOR_MAX_GAIN                       (53975*3175 / 100000)
#define SENSOR_MIN_GAIN                       (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

#define SENSOR_GAIN_DELAY_FRAME_COUNT_HDR     (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR  (2)
#define SENSOR_GAIN_CTRL_NUM_DOL              (2)
#define SENSOR_SHUTTER_CTRL_NUM_DOL           (2)

u32 Preview_line_period_HDR = 14815;
u32 vts_30fps_HDR = 2250;
u32 Preview_line_period  = 29630;
u32 vts_30fps = 1125;


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
    {LINEAR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},
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
   {HDR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
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
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tVts_reg_HDR[3];
    I2C_ARRAY tMax_short_exp_reg[3];
    I2C_ARRAY tGain_reg_HDR_SEF[3];
    I2C_ARRAY tGain_reg_HDR_LEF[3];
    I2C_ARRAY tExpo_reg_HDR_SEF[3];
    I2C_ARRAY tExpo_reg_HDR_LEF[3];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
    bool orien_dirty;
    bool reg_dirty;
    bool vts_reg_dirty;
    bool dirty;
    bool change;
} SC230ai_params;

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

const static I2C_ARRAY Sensor_init_table_2M30fps[] =
{//cleaned_0x01_FT_SC230AI_MIPI_27Minput_2lane_371.25Mbps_10bit_1920x1080_30fps
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x37f9,0x80},
    {0x301f,0x01},
    {0x3301,0x07},
    {0x3304,0x50},
    {0x3306,0x70},
    {0x3308,0x18},
    {0x3309,0x68},
    {0x330a,0x01},
    {0x330b,0x20},
    {0x3314,0x15},
    {0x331e,0x41},
    {0x331f,0x59},
    {0x3333,0x10},
    {0x3334,0x40},
    {0x335d,0x60},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x5e},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x3390,0x01},
    {0x3391,0x0b},
    {0x3392,0x0f},
    {0x3393,0x09},
    {0x3394,0x0d},
    {0x3395,0x60},
    {0x3396,0x48},
    {0x3397,0x49},
    {0x3398,0x4b},
    {0x3399,0x06},
    {0x339a,0x0a},
    {0x339b,0x0d},
    {0x339c,0x60},
    {0x33a2,0x04},
    {0x33ad,0x2c},
    {0x33af,0x40},
    {0x33b1,0x80},
    {0x33b3,0x40},
    {0x33b9,0x0a},
    {0x33f9,0x78},
    {0x33fb,0xa0},
    {0x33fc,0x4f},
    {0x33fd,0x5f},
    {0x349f,0x03},
    {0x34a6,0x4b},
    {0x34a7,0x5f},
    {0x34a8,0x30},
    {0x34a9,0x20},
    {0x34aa,0x01},
    {0x34ab,0x28},
    {0x34ac,0x01},
    {0x34ad,0x58},
    {0x34f8,0x7f},
    {0x34f9,0x10},
    {0x3630,0xc0},
    {0x3632,0x54},
    {0x3633,0x44},
    {0x363b,0x20},
    {0x363c,0x08},
    {0x3670,0x09},
    {0x3674,0xb0},
    {0x3675,0x80},
    {0x3676,0x88},
    {0x367c,0x40},
    {0x367d,0x49},
    {0x3690,0x33},
    {0x3691,0x33},
    {0x3692,0x43},
    {0x369c,0x49},
    {0x369d,0x4f},
    {0x36ae,0x4b},
    {0x36af,0x4f},
    {0x36b0,0x87},
    {0x36b1,0x9b},
    {0x36b2,0xb7},
    {0x36d0,0x01},
    {0x3722,0x97},
    {0x3724,0x22},
    {0x3728,0x90},
    {0x3901,0x02},
    {0x3902,0xc5},
    {0x3904,0x04},
    {0x3907,0x00},
    {0x3908,0x41},
    {0x3909,0x00},
    {0x390a,0x00},
    {0x3933,0x84},
    {0x3934,0x0a},
    {0x3940,0x64},
    {0x3941,0x00},
    {0x3942,0x04},
    {0x3943,0x0b},
    {0x3e00,0x00},
    {0x3e01,0x8c},
    {0x3e02,0x10},//update for danny 20220505
    {0x440e,0x02},
    {0x450d,0x11},
    {0x5010,0x01},
    {0x5787,0x08},
    {0x5788,0x03},
    {0x5789,0x00},
    {0x578a,0x10},
    {0x578b,0x08},
    {0x578c,0x00},
    {0x5790,0x08},
    {0x5791,0x04},
    {0x5792,0x00},
    {0x5793,0x10},
    {0x5794,0x08},
    {0x5795,0x00},
    {0x5799,0x06},
    {0x57ad,0x00},
    {0x5ae0,0xfe},
    {0x5ae1,0x40},
    {0x5ae2,0x3f},
    {0x5ae3,0x38},
    {0x5ae4,0x28},
    {0x5ae5,0x3f},
    {0x5ae6,0x38},
    {0x5ae7,0x28},
    {0x5ae8,0x3f},
    {0x5ae9,0x3c},
    {0x5aea,0x2c},
    {0x5aeb,0x3f},
    {0x5aec,0x3c},
    {0x5aed,0x2c},
    {0x5af4,0x3f},
    {0x5af5,0x38},
    {0x5af6,0x28},
    {0x5af7,0x3f},
    {0x5af8,0x38},
    {0x5af9,0x28},
    {0x5afa,0x3f},
    {0x5afb,0x3c},
    {0x5afc,0x2c},
    {0x5afd,0x3f},
    {0x5afe,0x3c},
    {0x5aff,0x2c},
    {0x36e9,0x20},
    {0x37f9,0x27},
    {0x0100,0x01},
    {0xffff,0x0a},/////delay 10ms
};

const static I2C_ARRAY Sensor_init_table_HDR_2M30fps[] =
{//cleaned_0x06_SC230AI_MIPI_27Minput_2lane_742.5Mbps_10bit_1920x1080_SHDR_VC_30fps
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x37f9,0x80},
    {0x301f,0x06},
    {0x320e,0x08},
    {0x320f,0xca},
    {0x3250,0xff},
    {0x3281,0x01},
    {0x3301,0x09},
    {0x3304,0x50},
    {0x3306,0x48},
    {0x3308,0x18},
    {0x3309,0x68},
    {0x330a,0x00},
    {0x330b,0xc0},
    {0x331e,0x41},
    {0x331f,0x59},
    {0x3333,0x10},
    {0x3334,0x40},
    {0x335d,0x60},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x5e},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x3390,0x01},
    {0x3391,0x0b},
    {0x3392,0x0f},
    {0x3393,0x0c},
    {0x3394,0x0d},
    {0x3395,0x60},
    {0x3396,0x48},
    {0x3397,0x49},
    {0x3398,0x4f},
    {0x3399,0x0a},
    {0x339a,0x0f},
    {0x339b,0x14},
    {0x339c,0x60},
    {0x33a2,0x04},
    {0x33af,0x40},
    {0x33b1,0x80},
    {0x33b3,0x40},
    {0x33b9,0x0a},
    {0x33f9,0x70},
    {0x33fb,0x90},
    {0x33fc,0x4b},
    {0x33fd,0x5f},
    {0x349f,0x03},
    {0x34a6,0x4b},
    {0x34a7,0x4f},
    {0x34a8,0x30},
    {0x34a9,0x20},
    {0x34aa,0x00},
    {0x34ab,0xe0},
    {0x34ac,0x01},
    {0x34ad,0x00},
    {0x34f8,0x5f},
    {0x34f9,0x10},
    {0x3630,0xc0},
    {0x3633,0x44},
    {0x3637,0x29},
    {0x363b,0x20},
    {0x3670,0x09},
    {0x3674,0xb0},
    {0x3675,0x80},
    {0x3676,0x88},
    {0x367c,0x40},
    {0x367d,0x49},
    {0x3690,0x44},
    {0x3691,0x44},
    {0x3692,0x54},
    {0x369c,0x49},
    {0x369d,0x4f},
    {0x36ae,0x4b},
    {0x36af,0x4f},
    {0x36b0,0x87},
    {0x36b1,0x9b},
    {0x36b2,0xb7},
    {0x36d0,0x01},
    {0x36ea,0x0b},
    {0x36eb,0x04},
    {0x36ec,0x0c},
    {0x36ed,0x24},
    {0x370f,0x01},
    {0x3722,0x17},
    {0x3728,0x90},
    {0x37b0,0x17},
    {0x37b1,0x17},
    {0x37b2,0x97},
    {0x37b3,0x4b},
    {0x37b4,0x4f},
    {0x37fa,0x0b},
    {0x37fb,0x24},
    {0x37fc,0x10},
    {0x37fd,0x22},
    {0x3901,0x02},
    {0x3902,0xc5},
    {0x3904,0x04},
    {0x3907,0x00},
    {0x3908,0x41},
    {0x3909,0x00},
    {0x390a,0x00},
    {0x391f,0x04},
    {0x3933,0x84},
    {0x3934,0x02},
    {0x3940,0x62},
    {0x3941,0x00},
    {0x3942,0x04},
    {0x3943,0x03},
    {0x3e00,0x01},
    {0x3e01,0x06},
    {0x3e02,0x00},
    {0x3e04,0x10},
    {0x3e05,0x60},
    {0x3e06,0x00},
    {0x3e07,0x80},
    {0x3e08,0x03},
    {0x3e09,0x40},
    {0x3e10,0x00},
    {0x3e11,0x80},
    {0x3e12,0x03},
    {0x3e13,0x40},
    {0x3e23,0x00},
    {0x3e24,0x88},
    {0x440e,0x02},
    {0x450d,0x11},
    {0x4816,0x71},
    {0x5010,0x00},
    {0x5787,0x08},
    {0x5788,0x03},
    {0x5789,0x00},
    {0x578a,0x10},
    {0x578b,0x08},
    {0x578c,0x00},
    {0x5790,0x08},
    {0x5791,0x04},
    {0x5792,0x00},
    {0x5793,0x10},
    {0x5794,0x08},
    {0x5795,0x00},
    {0x5799,0x06},
    {0x57ad,0x00},
    {0x5ae0,0xfe},
    {0x5ae1,0x40},
    {0x5ae2,0x3f},
    {0x5ae3,0x38},
    {0x5ae4,0x28},
    {0x5ae5,0x3f},
    {0x5ae6,0x38},
    {0x5ae7,0x28},
    {0x5ae8,0x3f},
    {0x5ae9,0x3c},
    {0x5aea,0x2c},
    {0x5aeb,0x3f},
    {0x5aec,0x3c},
    {0x5aed,0x2c},
    {0x5af4,0x3f},
    {0x5af5,0x38},
    {0x5af6,0x28},
    {0x5af7,0x3f},
    {0x5af8,0x38},
    {0x5af9,0x28},
    {0x5afa,0x3f},
    {0x5afb,0x3c},
    {0x5afc,0x2c},
    {0x5afd,0x3f},
    {0x5afe,0x3c},
    {0x5aff,0x2c},
    {0x36e9,0x20},
    {0x37f9,0x24},
    {0x0100,0x01},
    {0xffff,0x0a},/////delay 10ms
};

const static I2C_ARRAY mirror_reg[] =
{
    {0x3221, 0x00}, // mirror[2:1], flip[6:5]
};

const static I2C_ARRAY gain_reg[] = {
    {0x3e06, 0x00},
    {0x3e07, 0x80},
    {0x3e09, 0x40}, //low bit
};

const static I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x3e10, 0x00},
    {0x3e11, 0x80},
    {0x3e13, 0x40}, //low bit
};

const static I2C_ARRAY expo_reg[] = {
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x8c}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
};

const static I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x3e22, 0x00}, // expo[3:0]
    {0x3e04, 0x21}, // expo[7:0]
    {0x3e05, 0x00}, // expo[7:4]
};

const static I2C_ARRAY max_short_exp_reg[] = {
    {0x3e23, 0x00},
    {0x3e24, 0x88}
};

const static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x04},
    {0x320f, 0x65}
};

const static I2C_ARRAY vts_reg_HDR[] = {
    {0x320e, 0x04},
    {0x320f, 0x65}
};

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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;

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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;

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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;

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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;

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

static int SC230ai_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

  return SUCCESS;
}

static int pCus_init_linear_2M30fps(ss_cus_sensor *handle)
{
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2M30fps);i++)
    {
        if(Sensor_init_table_2M30fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2M30fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2M30fps[i].reg, Sensor_init_table_2M30fps[i].data) != SUCCESS)
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

static int pCus_init_mipi2lane_HDR_2M30fps(ss_cus_sensor *handle)
{
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
    int i,cnt=0;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR_2M30fps);i++)
    {
        if(Sensor_init_table_HDR_2M30fps[i].reg==0xffff)
        {
            //MsSleep(RTK_MS_TO_TICK(1));//usleep(1000*Sensor_init_table_2560_1440_30_HDR[i].data);
            SENSOR_MSLEEP(Sensor_init_table_HDR_2M30fps[i].data);
        }else
        {
            cnt = 0;
            SENSOR_DMSG("reg =  %x, data = %x\n", Sensor_init_table_HDR_2M30fps[i].reg, Sensor_init_table_HDR_2M30fps[i].data);
            while(SensorReg_Write(Sensor_init_table_HDR_2M30fps[i].reg,Sensor_init_table_HDR_2M30fps[i].data) != SUCCESS)
            {
                cnt++;
                 SENSOR_DMSG("Sensor_init_table_HDR_2M30fps -> Retry %d...\n",cnt);
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"1920x1080@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_2M30fps;
            vts_30fps=1125;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period  = 29630;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_HDR_2M30fps;
            vts_30fps_HDR = 2250;
            Preview_line_period_HDR = 14815;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short_exp=133;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
}

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
        break;
        default :
        break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotify_HDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
            params->reg_dirty = false;
        }
        if(params->vts_reg_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg_HDR, ARRAY_SIZE(vts_reg_HDR));
            SensorRegArrayW((I2C_ARRAY*)params->tMax_short_exp_reg, ARRAY_SIZE(max_short_exp_reg));
            params->vts_reg_dirty = false;
        }
        break;
        default :
        break;
    }

    return SUCCESS;
}

static int pCus_AEStatusNotify_HDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
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

static int pCus_SetAEUSecs_HDR_LEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0,vts = 0;
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x01},//expo [20:17]
    {0x3e01, 0x80}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0x0f)<<16;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg_HDR_SEF[2].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period_HDR)/1000/2;

    SENSOR_DMSG("[%s] sensor expo lines/us %ld,%ld us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs_HDR_SEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0,vts = 0;
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
    params->tExpo_reg_HDR_SEF[0].data =  (half_lines>>16) & 0x0f;
    params->tExpo_reg_HDR_SEF[1].data =  (half_lines>>8) & 0xff;
    params->tExpo_reg_HDR_SEF[2].data = (half_lines>>0) & 0xf0;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
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
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
    u8 i=0 ;
    u16 reg_gain, ana_real_gain,  dcg = 100, DIG_Fine_gain_reg = 0;
    u32 tmp_gain = 0;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e09, 0x00},
    };
    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN*1024) {
        gain = SENSOR_MAX_GAIN*1024;
    }

    if (gain <= 1024)//64
        gain = 1024;
    else if (gain >= 138000)
        gain = 138000;

    tmp_gain = gain;

    if((tmp_gain >= 1024)&&(tmp_gain < 2048)) // x1 ~ x2
    {
        dcg = 1000;
        reg_gain = 0x00;
        ana_real_gain = 1;
    }
    else if((tmp_gain >= 2048)&&(tmp_gain < 3175)) // 2 ~ 3.1
    {
        dcg = 1000;
        reg_gain = 0x01;
        ana_real_gain = 2;
    }
    else if((tmp_gain >= 3175)&&(tmp_gain < 6350)) // 3.1 ~ 6.2
    {
        dcg = 3100;
        reg_gain = 0x40;
        ana_real_gain = 1;
    }
    else if((tmp_gain >= 6350)&&(tmp_gain < 12700)) // 6.2 ~ 12.4
    {
        dcg = 3100;
        reg_gain = 0x48;
        ana_real_gain = 2;
    }
    else if((tmp_gain >= 12700)&&(tmp_gain < 25400)) // 12.4 ~ 24.8
    {
        dcg = 3100;
        reg_gain = 0x49;
        ana_real_gain = 4;
    }
    else if((tmp_gain >= 25400)&&(tmp_gain < 50800)) // 24.8 ~ 49.6
    {
        dcg = 3100;
        reg_gain = 0x4b;
        ana_real_gain = 8;
    }
    else if((tmp_gain >= 50800)&&(tmp_gain < 101600)) // 49.6 ~ 99.2
    {
        dcg = 3100;
        reg_gain = 0x4f;
        ana_real_gain = 16;
    }
    else if((tmp_gain >= 101600)&&(tmp_gain < 203200)) // 99.2 ~ 99.2 * 2
    {
        dcg = 3100;
        reg_gain = 0x5f;
        ana_real_gain = 32;
        DIG_Fine_gain_reg = 0xFF;
    }

    if(gain < 3175)
    {
        DIG_Fine_gain_reg = abs(gain/ana_real_gain/8);
    }else if(gain >= 3175)
    {
        DIG_Fine_gain_reg = abs(8000*gain/dcg/ana_real_gain/64);
    }

    if (DIG_Fine_gain_reg >= 0xFF) // Never delete it. // icekirin
    {
        DIG_Fine_gain_reg = 0xFF;
    }
    params->tGain_reg[2].data = reg_gain; // // 0x3e09
    params->tGain_reg[1].data = DIG_Fine_gain_reg;   // 0x3e07
    params->tGain_reg[0].data = 0x0; // // 0x3e06

    for (i = 0; i < ARRAY_SIZE(gain_reg); i++)
    {
      if (params->tGain_reg[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

    return SUCCESS;
}

static int pCus_SetAEGain_HDR_SEF(ss_cus_sensor *handle, u32 gain) {
    SC230ai_params *params = (SC230ai_params *)handle->private_data;
    u8 i=0 ;
    u16 reg_gain, ana_real_gain,  dcg = 100, DIG_Fine_gain_reg = 0;
    u32 tmp_gain = 0;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e10, 0x00},
        {0x3e11, 0x80},
        {0x3e13, 0x40},
    };
    memcpy(gain_reg_temp, params->tGain_reg_HDR_SEF, sizeof(gain_reg_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN*1024) {
        gain = SENSOR_MAX_GAIN*1024;
    }

    if (gain <= 1024)//64
        gain = 1024;
    else if (gain >= 138000)
        gain = 138000;

    tmp_gain = gain;

    if((tmp_gain >= 1024)&&(tmp_gain < 2048)) // x1 ~ x2
    {
        dcg = 1000;
        reg_gain = 0x00;
        ana_real_gain = 1;
    }
    else if((tmp_gain >= 2048)&&(tmp_gain < 3175)) // 2 ~ 3.1
    {
        dcg = 1000;
        reg_gain = 0x01;
        ana_real_gain = 2;
    }
    else if((tmp_gain >= 3175)&&(tmp_gain < 6350)) // 3.1 ~ 6.2
    {
        dcg = 3100;
        reg_gain = 0x40;
        ana_real_gain = 1;
    }
    else if((tmp_gain >= 6350)&&(tmp_gain < 12700)) // 6.2 ~ 12.4
    {
        dcg = 3100;
        reg_gain = 0x48;
        ana_real_gain = 2;
    }
    else if((tmp_gain >= 12700)&&(tmp_gain < 25400)) // 12.4 ~ 24.8
    {
        dcg = 3100;
        reg_gain = 0x49;
        ana_real_gain = 4;
    }
    else if((tmp_gain >= 25400)&&(tmp_gain < 50800)) // 24.8 ~ 49.6
    {
        dcg = 3100;
        reg_gain = 0x4b;
        ana_real_gain = 8;
    }
    else if((tmp_gain >= 50800)&&(tmp_gain < 101600)) // 49.6 ~ 99.2
    {
        dcg = 3100;
        reg_gain = 0x4f;
        ana_real_gain = 16;
    }
    else if((tmp_gain >= 101600)&&(tmp_gain < 203200)) // 99.2 ~ 99.2 * 2
    {
        dcg = 3100;
        reg_gain = 0x5f;
        ana_real_gain = 32;
        DIG_Fine_gain_reg = 0xFF;
    }

    if(gain < 3175)
    {
        DIG_Fine_gain_reg = abs(gain/ana_real_gain/8);
    }else if(gain >= 3175) // || gain == 3456)
    {
        DIG_Fine_gain_reg = abs(8000*gain/dcg/ana_real_gain/64);
    }

    if (DIG_Fine_gain_reg >= 0xFF) // Never delete it. // icekirin
    {
        DIG_Fine_gain_reg = 0xFF;
    }
    params->tGain_reg_HDR_SEF[2].data = reg_gain; // // 0x3e09
    params->tGain_reg_HDR_SEF[1].data = DIG_Fine_gain_reg;   // 0x3e07
    params->tGain_reg_HDR_SEF[0].data = 0x0; // // 0x3e06

    for (i = 0; i < ARRAY_SIZE(gain_reg_HDR_SEF); i++)
    {
      if (params->tGain_reg_HDR_SEF[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

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
    SC230ai_params *params;
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

    params = (SC230ai_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ///////////////////////////////////////////////////////
    // Sensor stream name
    ///////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SC230ai_MIPI");

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
    handle->pCus_sensor_init                                      = pCus_init_linear_2M30fps;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;

    handle->pCus_sensor_SetPatternMode                            = SC230ai_SetPatternMode;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////

    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN *1024;
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
    SC230ai_params *params;
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
    params = (SC230ai_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SC230ai_MIPI_HDR_SEF");


    memcpy(params->tVts_reg_HDR, vts_reg_HDR, sizeof(vts_reg_HDR));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMax_short_exp_reg, max_short_exp_reg, sizeof(max_short_exp_reg));

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
    handle->pCus_sensor_init                                      = pCus_init_mipi2lane_HDR_2M30fps;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
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
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN *1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR * params->expo.max_short_exp;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR/2;
    return SUCCESS;
}

int cus_camsensor_init_handle_HDR_LEF(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    SC230ai_params *params;
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
    params = (SC230ai_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SC230ai_MIPI_HDR_LEF");
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
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
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
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

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN *1024;
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

SENSOR_DRV_ENTRY_IMPL_END_EX( SC230ai,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_HDR_SEF,
                            cus_camsensor_init_handle_HDR_LEF,
                            SC230ai_params
                         );

