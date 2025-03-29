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
   Porting owner :  Lance.Lan
   Date :           9/26/2023
   Build on :       Master_V4 I6C

    Verified owner:Jilly
    Date          :23/11/22
    Verify Info   : verify on imx307 sensor board
                    linear pass
                    shutter/gain pass
                    HDR pass
                    shutter/gain pass
                    disable then re-initial sensor pass
    Remark :        rename only

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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX307_HDR);

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
#define SENSOR_NAME                           IMX307

#define SENSOR_IFBUS_TYPE                     CUS_SENIF_BUS_MIPI
#define SENSOR_HDR_MODE                       CUS_HDR_MODE_SONY_DOL

//Linear Mode
#define SENSOR_MIPI_LANE_NUM                  (2)
#define Preview_MCLK_SPEED                    CUS_CMU_CLK_37P125MHZ

//HDR mode
#define SENSOR_MIPI_LANE_NUM_DOL              (4)
#define SENSOR_MIPI_HDR_DOL_10bit             (0)
#define SENSOR_MIPI_HDR_DOL_12bit             (1)

//I2C bus
#define SENSOR_I2C_ADDR                       0x34
#define SENSOR_I2C_SPEED                      20000
#define SENSOR_I2C_LEGACY                     I2C_NORMAL_MODE
#define SENSOR_I2C_FMT                        I2C_FMT_A16D8

#define SENSOR_RGBIRID                        CUS_RGBIR_NONE

#define SENSOR_DATAPREC                       CUS_DATAPRECISION_12
#define SENSOR_BAYERID                        CUS_BAYER_RG
#define SENSOR_BAYERID_HDR_DOL                CUS_BAYER_RG
#if SENSOR_MIPI_HDR_DOL_10bit
#define SENSOR_DATAPREC_DOL                   CUS_DATAPRECISION_10
#endif
#if SENSOR_MIPI_HDR_DOL_12bit
#define SENSOR_DATAPREC_DOL                   CUS_DATAPRECISION_12
#endif

//AE info
#define SENSOR_MAX_GAIN                       (2818 * 1024)
#define SENSOR_MIN_GAIN                       (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (1)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (2)

#define SENSOR_GAIN_DELAY_FRAME_COUNT_DOL     (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_DOL  (2)
#define SENSOR_GAIN_CTRL_NUM_DOL              (2)
#define SENSOR_SHUTTER_CTRL_NUM_DOL           (2)

#define Preview_line_period                   29629
#define vts_30fps                             1125

#if SENSOR_MIPI_HDR_DOL_10bit
#define Preview_line_period_HDR_DOL_4LANE     (27322 / 2)
#define vts_30fps_HDR_DOL_4lane               1220
#endif
#if SENSOR_MIPI_HDR_DOL_12bit
#define Preview_line_period_HDR_DOL_4LANE     (28784 / 2)
#define Preview_line_period_HDR_DOL_2LANE     (28784 / 2)

#define vts_30fps_HDR_DOL_4lane               1158
#define vts_30fps_HDR_DOL_2lane               1158
#endif
#define MAX_DOL_RHS1                          ((1158 * 2) - (1097 * 2) - 21)

//mirror-flip
#define SENSOR_ORIT                           CUS_ORIT_M0F0

static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_0 = 0,  LINEAR_RES_END}mode;
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
    {LINEAR_RES_0, {1948, 1097, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},
};

static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_0 = 0, HDR_RES_END}mode;
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
   {HDR_RES_0, {1948, 1089, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
};

typedef struct {
    struct {
        u32 expo_lines;
        u32 expo_lef_us;
        u32 expo_sef_us;
        //u32 hts;
        u32 vts;
        //u32 final_us;
        u32 final_gain;
        u32 fps;
        u32 preview_fps;
        u32 lines;
    } expo;
    bool dirty;
    bool change;
    u32 max_rhs1;
    u32 lef_shs2;
    u32 skip_cnt;
    CUS_CAMSENSOR_ORIT  orit;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tShs2_reg[3];
    I2C_ARRAY tRhs1_reg[3];
    I2C_ARRAY tGain_hdr_dol_lef_reg[1];
    I2C_ARRAY tGain_hdr_dol_sef_reg[1];
    bool orien_dirty;
} imx307_params;

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)     (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)     (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#if 0
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3004, 0x10},      // {address of ID, ID },
    {0x3009, 0x01},      // {address of ID, ID },
};
#endif
const static I2C_ARRAY Sensor_init_table_4lane[] =
{
    {0x3002, 0x01},   //Master mode stop
    {0xffff, 0x14},   //delay
    {0x3000, 0x01},   // standby
    {0xffff, 0x14},   //delay
    {0x3005, 0x01},
    {0x3007, 0x00},   //mirror/flip
    {0x3009, 0x02},   //FRSEL
    {0x3011, 0x0A},
    {0x3012, 0x64},
    {0x3013, 0x00},
    {0x3018, 0x65},   //VMAX
    {0x3019, 0x04},
    {0x301a, 0x00},
    {0x301c, 0x30},   //0x1167 HMAX,for 25fps
    {0x301d, 0x11},
    {0x3046, 0x01},
    {0x305C, 0x18},   //INCK
    {0x305D, 0x03},
    {0x305E, 0x20},
    {0x305F, 0x01},
    {0x309E, 0x4A},
    {0x309F, 0x4A},
    {0x311C, 0x0E},
    {0x3128, 0x04},
    {0x3129, 0x00},
    {0x313B, 0x41},
    {0x315E, 0x1A},   //INCKSEL5
    {0x3164, 0x1A},   //INCKSEL6
    {0x317C, 0x00},   //ADBIT2
    {0x317E, 0x00},
    {0x31EC, 0x0E},
    {0x3405, 0x20},
    {0x3407, 0x03},   // 4 lane for phy
    {0x3418, 0x49},   //Y-out
    {0x3419, 0x04},
    {0x3441, 0x0C},
    {0x3442, 0x0C},
    {0x3443, 0x03},   // 4 lane
    {0x3444, 0x20},
    {0x3445, 0x25},
    {0x3446, 0x47},
    {0x3447, 0x00},
    {0x3448, 0x1F},
    {0x3449, 0x00},
    {0x344A, 0x17},
    {0x344B, 0x00},
    {0x344C, 0x0F},
    {0x344D, 0x00},
    {0x344E, 0x17},
    {0x344F, 0x00},
    {0x3450, 0x47},
    {0x3451, 0x00},
    {0x3452, 0x0F},
    {0x3453, 0x00},
    {0x3454, 0x0F},
    {0x3455, 0x00},
    {0x3472, 0x9c},   //x-out size
    {0x3473, 0x07},
    {0x3480, 0x49},   //0x9c
    {0x3000, 0x00},   // operating
    {0x3002, 0x00},   //Master mode start
};

const static I2C_ARRAY Sensor_init_table_2lane[] =
{
    {0x3002, 0x00},   //Master mode stop
    {0xffff, 0x14},   //delay
    {0x3000, 0x01},   // standby
    {0xffff, 0x14},   //delay
    {0x3005, 0x01},
    {0x3009, 0x02},   //FRSEL
    {0x3011, 0x0A},
    {0x3016, 0x08},   //yc modify
    {0x3018, 0x65},   //VMAX
    {0x3019, 0x04},
    {0x301c, 0x30},   //0x1167 HMAX,for 25fps
    {0x301d, 0x11},
    {0x305C, 0x18},   //INCK
    {0x305D, 0x03},
    {0x305E, 0x20},
    {0x309E, 0x4A},
    {0x309F, 0x4A},
    {0x311C, 0x0E},
    {0x3128, 0x04},
    {0x3129, 0x00},
    {0x313B, 0x41},
    {0x315E, 0x1A},   //INCKSEL5
    {0x3164, 0x1A},   //INCKSEL6
    {0x317C, 0x00},   //ADBIT2
    {0x317E, 0x00},
    {0x31EC, 0x0E},
    {0x3405, 0x10},
    {0x3407, 0x01},   // 2 lane for phy
    {0x3418, 0x49},   //Y-out
    {0x3419, 0x04},
    {0x3443, 0x01},   // 2 lane
    {0x3444, 0x20},
    {0x3445, 0x25},
    {0x3446, 0x57},
    {0x3448, 0x37},
    {0x344A, 0x1F},
    {0x344C, 0x1F},
    {0x344E, 0x1f},
    {0x3450, 0x77},
    {0x3452, 0x1F},
    {0x3454, 0x17},
    {0x3480, 0x49},   //0x9c
    {0x3000, 0x00},   // operating
    {0xffff, 0x14},
    //{0x3002, 0x00},   //Master mode start
};

const static I2C_ARRAY Sensor_init_table_HDR_DOL_4lane[] =
{
#if SENSOR_MIPI_HDR_DOL_10bit
    //DOL 2frame 1080p 10bit
    {0x3002, 0x01},
    {0xffff, 0x14},
    {0x3000, 0x01},
    {0xffff, 0x14},
    {0x3005, 0x00},
    //{0x3007, 0x00},   //WINMODE
    {0x3007, 0x40},
    {0x3009, 0x01},
    {0x300A, 0x3C},
    {0x300C, 0x11},
    {0x3011, 0x0A},
    //{0x3018, 0x65},   //VMAX
    //{0x3019, 0x04},
    {0x3018, 0xC4},
    {0x3019, 0x04},
    //{0x301c, 0x98},   //HMAX
    //{0x301d, 0x08},
    {0x301c, 0xEC},
    {0x301d, 0x07},
    {0x3020, 0x02},   // SHS1
    {0x3021, 0x00},   // SHS1
    {0x3022, 0x00},   // SHS1
    {0x3024, 0x53},   // SHS2
    {0x3025, 0x04},   // SHS2
    {0x3026, 0x00},   // SHS2
    {0x3028, 0x00},   // SHS3
    {0x3029, 0x00},   // SHS3
    {0x302A, 0x00},   // SHS3
    {0x3030, 0xe1},   //RHS1
    {0x3031, 0x00},
    {0x3032, 0x00},
    {0x3034, 0x00},   //RHS2
    {0x3035, 0x00},
    {0x3036, 0x00},
    {0x303A, 0x08},   // WIN
    {0x303C, 0x04},   // WIN
    {0x303E, 0x41},   // WIN
    {0x303F, 0x04},   // WIN
    {0x3045, 0x05},
    {0x3046, 0x00},
    {0x304B, 0x0A},
    {0x305C, 0x18},
    {0x305D, 0x03},
    {0x305E, 0x20},
    {0x305F, 0x01},
    {0x309E, 0x4A},
    {0x309F, 0x4A},
    {0x3106, 0x11},
    {0x311C, 0x0E},
    {0x3128, 0x04},
    {0x3129, 0x1D},
    {0x313B, 0x41},
    {0x315E, 0x1A},
    {0x3164, 0x1A},
    {0x317C, 0x12},
    {0x31EC, 0x37},
    {0x3405, 0x10},
    {0x3407, 0x03},
    //{0x3414, 0x0A},   //opb_size_v
    {0x3414, 0x00},
    {0x3415, 0x00},
    //{0x3418, 0x9C},   //y_out_size
    //{0x3419, 0x08},
    {0x3418, 0x7A},
    {0x3419, 0x09},
    {0x3441, 0x0A},
    {0x3442, 0x0A},
    {0x3443, 0x03},
    {0x3444, 0x20},
    {0x3445, 0x25},
    {0x3446, 0x57},
    {0x3447, 0x00},
    {0x3448, 0x37},
    {0x3449, 0x00},
    {0x344A, 0x1F},
    {0x344B, 0x00},
    {0x344C, 0x1F},
    {0x344D, 0x00},
    {0x344E, 0x1F},
    {0x344F, 0x00},
    {0x3450, 0x77},
    {0x3451, 0x00},
    {0x3452, 0x1F},
    {0x3453, 0x00},
    {0x3454, 0x17},
    {0x3455, 0x00},
    {0x3472, 0xA0},
    {0x3473, 0x07},
    {0x347B, 0x23},
    {0x3480, 0x49},
    {0x3000, 0x00},   // operating
    //{0xffff, 0x14},
    {0x3002, 0x00},   //Master mode start
#endif

#if SENSOR_MIPI_HDR_DOL_12bit
    //DOL 2frame 1080p 12bit
    {0x3002, 0x01},
    {0xffff, 0x14},
    {0x3000, 0x01},
    {0xffff, 0x14},
    {0x3005, 0x01},
    //{0x3007, 0x00},   //WINMODE
    {0x3007, 0x40},
    {0x3009, 0x01},
    {0x300A, 0xF0},
    {0x300C, 0x11},
    {0x3011, 0x0A},
    //{0x3018, 0x65},   //VMAX
    //{0x3019, 0x04},
    {0x3018, 0x86},
    {0x3019, 0x04},
    //{0x301c, 0x98},   //HMAX
    //{0x301d, 0x08},
    {0x301c, 0x58},
    {0x301d, 0x08},
    {0x3020, 0x02},   // SHS1
    {0x3021, 0x00},   // SHS1
    {0x3022, 0x00},   // SHS1
    {0x3024, 0x73},   // SHS2
    {0x3025, 0x04},   // SHS2
    {0x3026, 0x00},   // SHS2
    {0x3028, 0x00},   // SHS3
    {0x3029, 0x00},   // SHS3
    {0x302A, 0x00},   // SHS3
    {0x3030, 0x65},   //RHS1
    {0x3031, 0x00},
    {0x3032, 0x00},
    {0x3034, 0x00},   //RHS2
    {0x3035, 0x00},
    {0x3036, 0x00},
    {0x303A, 0x08},   // WIN
    {0x303C, 0x04},   // WIN
    {0x303E, 0x41},   // WIN
    {0x303F, 0x04},   // WIN
    {0x3045, 0x05},
    {0x3046, 0x01},
    {0x304B, 0x0A},
    {0x305C, 0x18},
    {0x305D, 0x03},
    {0x305E, 0x20},
    {0x305F, 0x01},
    {0x309E, 0x4A},
    {0x309F, 0x4A},
    {0x3106, 0x11},
    {0x311C, 0x0E},
    {0x3128, 0x04},
    {0x3129, 0x00},
    {0x313B, 0x41},
    {0x315E, 0x1A},
    {0x3164, 0x1A},
    {0x317C, 0x00},
    {0x31EC, 0x0E},
    {0x3204, 0x4A},
    {0x320A, 0x22},
    {0x3344, 0x38},
    {0x3405, 0x10},
    {0x3407, 0x03},
    //{0x3414, 0x0A},   //opb_size_v
    {0x3414, 0x00},
    {0x3415, 0x00},
    //{0x3418, 0x9C},   //y_out_size
    //{0x3419, 0x08},
    {0x3418, 0x7A},
    {0x3419, 0x09},

    {0x3441, 0x0C},
    {0x3442, 0x0C},
    {0x3443, 0x03},
    {0x3444, 0x20},
    {0x3445, 0x25},
    {0x3446, 0x57},
    {0x3447, 0x00},
    {0x3448, 0x37},
    {0x3449, 0x00},
    {0x344A, 0x1F},
    {0x344B, 0x00},
    {0x344C, 0x1F},
    {0x344D, 0x00},
    {0x344E, 0x1F},
    {0x344F, 0x00},
    {0x3450, 0x77},
    {0x3451, 0x00},
    {0x3452, 0x1F},
    {0x3453, 0x00},
    {0x3454, 0x17},
    {0x3455, 0x00},
    {0x3472, 0xA0},
    {0x3473, 0x07},
    {0x347B, 0x23},
    {0x3480, 0x49},

    {0x3000, 0x00},   // operating
    //{0xffff, 0x14},
    {0x3002, 0x00},   //Master mode start
#endif
};

const static I2C_ARRAY Sensor_init_table_HDR_DOL_2lane[] =
{
    {0x3002, 0x01},
    {0xffff, 0x14},   //test
    {0x3000, 0x01},   //test
    {0xffff, 0x14},   //test
    {0x3005, 0x01},
    {0x3007, 0x00},

    {0x3009, 0x01},
    {0x300A, 0xF0},
    {0x300C, 0x11},
    {0x3011, 0x0A},
    {0x3018, 0x86},

    {0x3019, 0x04},
    {0x301C, 0x58},
    {0x301D, 0x08},
    {0x3020, 0x02},   // SHS1
    {0x3021, 0x00},   // SHS1

    {0x3024, 0xC9},   // SHS2
    {0x3025, 0x06},   // SHS2
    //{0x3026, 0x06},   // SHS2
    {0x3030, 0x0B},
    {0x3031, 0x00},
    //{0x3032, 0x00},
    {0x3045, 0x05},
    {0x3046, 0x01},
    //{0x3048, 0x00},
    //{0x3049, 0x00},
    {0x304B, 0x0A},
    {0x305C, 0x18},
    {0x305D, 0x03},
    {0x305E, 0x20},
    {0x305F, 0x01},
    {0x309E, 0x4A},
    {0x309F, 0x4A},
    {0x3106, 0x11},
    {0x311C, 0x0E},
    {0x311D, 0x08},

    {0x3128, 0x04},
    {0x3129, 0x00},
    {0x313B, 0x41},
    {0x315E, 0x1A},
    {0x3164, 0x1A},
    {0x317C, 0x00},
    {0x31EC, 0x0E},
    {0x3405, 0x00},
    {0x3407, 0x01},
    {0x3414, 0x0A},
    {0x3415, 0x00},
    //{0x3418, 0x9C},
    //{0x3419, 0x08},
    {0x3418, 0x7A},
    {0x3419, 0x09},
    {0x3441, 0x0C},
    {0x3442, 0x0C},
    {0x3443, 0x01},
    {0x3444, 0x20},
    {0x3445, 0x25},
    {0x3446, 0x77},
    {0x3447, 0x00},
    {0x3448, 0x67},
    {0x3449, 0x00},
    {0x344A, 0x47},
    {0x344B, 0x00},
    {0x344C, 0x37},
    {0x344D, 0x00},
    {0x344E, 0x3F},
    {0x344F, 0x00},
    {0x3450, 0xFF},
    {0x3451, 0x00},
    {0x3452, 0x3F},
    {0x3453, 0x00},
    {0x3454, 0x37},
    {0x3455, 0x00},
    {0x3472, 0xA0},
    {0x3473, 0x07},
    {0x347B, 0x23},
    {0x3480, 0x49},
    {0x3000, 0x00},   //test
    {0x3002, 0x00},
};

const static I2C_ARRAY TriggerStartTbl[] = {
    {0x3002, 0x00},   //Master mode start
};

static I2C_ARRAY PatternTbl[] = {
    {0x308c, 0x20},   //colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY gain_HDR_DOL_LEF_reg[] =
{
    {0x3014, 0x00},
};

const static I2C_ARRAY gain_HDR_DOL_SEF1_reg[] =
{
    {0x30F2, 0x00},
};

const static I2C_ARRAY expo_SHS2_reg[] =
{
#if 1 //decreasing exposure ratio version.
    {0x3026, 0x00},
    {0x3025, 0x08},
    {0x3024, 0x49},
#else
    {0x3026, 0x00},
    {0x3025, 0x07},
    {0x3024, 0xc9},
#endif
};

const I2C_ARRAY expo_RHS1_reg[] =
{
#if 1 //decreasing exposure ratio version.
    {0x3032, 0x00},
    {0x3031, 0x00},
    {0x3030, 0x65}, /*101*/
#else
    {0x3032, 0x00},
    {0x3031, 0x00},
    {0x3030, 0x0b},
#endif
};

const static I2C_ARRAY mirr_flip_table[] =
{
    {0x3007, 0x00},   //M0F0
    {0x3007, 0x02},   //M1F0
    {0x3007, 0x01},   //M0F1
    {0x3007, 0x03},   //M1F1
};

const static I2C_ARRAY gain_reg[] = {
    //{0x350A, 0x00},   //bit0, high bit
    {0x3014, 0x00},   //low bit
    {0x3009, 0x02},   //hcg mode,bit 4
    {0x3016, 0x08},
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

const static I2C_ARRAY expo_reg[] = {
    {0x3022, 0x00},
    {0x3021, 0x00},
    {0x3020, 0x00},
};

const static I2C_ARRAY vts_reg[] = {
    {0x301a, 0x00},
    {0x3019, 0x04},
    {0x3018, 0xC4},
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

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_SEF1(ss_cus_sensor *handle, u32 us);

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
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_min                   = Preview_line_period_HDR_DOL_4LANE * 2;
            info->u32AEShutter_max                   = (Preview_line_period_HDR_DOL_4LANE * 211);
        }
        else
        {
            info->u32AEShutter_min                   = (Preview_line_period_HDR_DOL_4LANE * 5);
            info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        }
    }
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    //imx307_params *params = (imx307_params *)handle->private_data;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Configuration Chip RX
    pCus_PowerOn_InitChipRX(handle, idx);

    //Sensor power on sequence
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->MCLK(idx, 1, UseParaMclk(SENSOR_DRV_PARAM_MCLK()));
    SENSOR_UDELAY(20); //TLOW

    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_UDELAY(20);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_UDELAY(20);

    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_UDELAY(20); //TXCE

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
	ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx307_params *params = (imx307_params *)handle->private_data;

    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->MCLK(idx, 0, UseParaMclk(SENSOR_DRV_PARAM_MCLK()));

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }

    //Init orient status
    params->orit = SENSOR_ORIT;
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    imx307_params *params = (imx307_params *)handle->private_data;

    *orit = params->orit;
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx307_params *params = (imx307_params *)handle->private_data;

    params->orit        = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

/////////////////// image function /////////////////////////
#if 0

static int pCus_CheckSonyProductID(ss_cus_sensor *handle)
{
    u16 sen_data;

    /* Read Product ID */
    if (SensorReg_Read(0x31DC, (void*)&sen_data)) {
        return FAIL;
    }

    if ((sen_data & 0x0006) != 0x4) {
        SENSOR_EMSG("[***ERROR***]Check Product ID Fail: 0x%x\n", sen_data);
        return FAIL;
    }

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


    SENSOR_DMSG("[%s]IMX307 sensor ,Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}
#endif
static int imx307_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    return SUCCESS;
}

static int pCus_init_mipi4lane_linear(ss_cus_sensor *handle)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    int i, cnt=0;

    //if (pCus_CheckSonyProductID(handle)) {
    //    return FAIL;
    //}

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_4lane);i++)
    {
        if(Sensor_init_table_4lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane[i].data);
        }
        else
        {
            while(SensorReg_Write(Sensor_init_table_4lane[i].reg,Sensor_init_table_4lane[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    CamOsPrintf("[ERROR] Sensor Init Fail(I2C nak)!!\n");
                    return FAIL;
                }
            }
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
    pCus_SetOrien(handle, params->orit);
    return SUCCESS;
}

#if (SENSOR_MIPI_LANE_NUM_DOL == 4)
static int pCus_init_mipi4lane_HDR_DOL(ss_cus_sensor *handle)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    int i, cnt=0;

    //if (pCus_CheckSonyProductID(handle)) {
    //    return FAIL;
    //}

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR_DOL_4lane);i++)
    {
        if(Sensor_init_table_HDR_DOL_4lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR_DOL_4lane[i].data);
        }
        else
        {
            while(SensorReg_Write(Sensor_init_table_HDR_DOL_4lane[i].reg,Sensor_init_table_HDR_DOL_4lane[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    CamOsPrintf("[ERROR] Sensor Init Fail(I2C nak)!!\n");
                    return FAIL;
                }
            }
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;

    return SUCCESS;
}
#endif

static int pCus_init_mipi2lane_HDR_DOL(ss_cus_sensor *handle)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    int i, cnt=0;

    //if (pCus_CheckSonyProductID(handle)) {
    //    return FAIL;
    //}

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR_DOL_2lane);i++)
    {
        if(Sensor_init_table_HDR_DOL_2lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR_DOL_2lane[i].data);
        }
        else
        {
            while(SensorReg_Write(Sensor_init_table_HDR_DOL_2lane[i].reg,Sensor_init_table_HDR_DOL_2lane[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    CamOsPrintf("[ERROR] Sensor Init Fail(I2C nak)!!\n");
                    return FAIL;
                }
            }
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;

    return SUCCESS;
}

#if 1//(SENSOR_MIPI_LANE_NUM == 2)
static int pCus_init_mipi2lane_linear(ss_cus_sensor *handle)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    int i,cnt=0;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    //if (pCus_CheckSonyProductID(handle)) {
    //    return FAIL;
    //}

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2lane);i++)
    {
		if(Sensor_init_table_2lane[i].reg==0xffff)
		{
			SENSOR_MSLEEP_(Sensor_init_table_2lane[i].data);
		}
		else
		{
			while(SensorReg_Write(Sensor_init_table_2lane[i].reg,Sensor_init_table_2lane[i].data) != SUCCESS)
			{
				cnt++;
				if(cnt>=10)
				{
                    CamOsPrintf("[ERROR] Sensor Init Fail(I2C nak)!!\n");
					return FAIL;
				}
			}
		}
	}

    for(i=0;i< ARRAY_SIZE(PatternTbl);i++)
    {
        if(SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
        {
            CamOsPrintf("[ERROR] SENSOR PatternGen Init Fail(I2C nak)!!\n");
            return FAIL;
        }
    }

    for(i=0;i< ARRAY_SIZE(TriggerStartTbl);i++)
    {
        if(SensorReg_Write(TriggerStartTbl[i].reg,TriggerStartTbl[i].data) != SUCCESS)
        {
            CamOsPrintf("[ERROR] Sensor Trigger Start Init Fail(I2C nak)!!\n");
            return FAIL;
        }
    }

    pCus_SetOrien(handle, params->orit);

    return SUCCESS;
}
#endif


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
    imx307_params *params = (imx307_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;

            if(lane_num == 2)
                 handle->pCus_sensor_init = pCus_init_mipi2lane_linear;
            else if (lane_num == 4)
                handle->pCus_sensor_init = pCus_init_mipi4lane_linear;
            else
              handle->pCus_sensor_init = pCus_init_mipi2lane_linear;

            params->expo.vts=vts_30fps;
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL(ss_cus_sensor *handle, u32 res_idx)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            if(hdr_lane_num == 2) {
                handle->pCus_sensor_init = pCus_init_mipi2lane_HDR_DOL;
                params->expo.vts = vts_30fps_HDR_DOL_2lane;
            } else if(hdr_lane_num == 4) {
                handle->pCus_sensor_init = pCus_init_mipi4lane_HDR_DOL;
                params->expo.vts = vts_30fps_HDR_DOL_4lane;
            }

            break;

        default:
            break;
    }

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    int table_length = ARRAY_SIZE(mirr_flip_table);
    imx307_params *params = (imx307_params *)handle->private_data;
    int seg_length=table_length/4;
    int i,j;


    switch(orit)
    {
        case CUS_ORIT_M0F0:
            for(i=0,j=0;i<seg_length;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                params->orit  = CUS_ORIT_M0F0;
            }
            break;

        case CUS_ORIT_M1F0:
            for(i=seg_length,j=0;i<seg_length*2;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                params->orit  = CUS_ORIT_M1F0;
            }
            break;

        case CUS_ORIT_M0F1:
            for(i=seg_length*2,j=0;i<seg_length*3;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                params->orit  = CUS_ORIT_M0F1;
            }
            break;

        case CUS_ORIT_M1F1:
            for(i=seg_length*3,j=0;i<seg_length*4;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                params->orit  = CUS_ORIT_M1F1;
            }
            break;

        default :
            for(i=0,j=0;i<seg_length;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                params->orit  = CUS_ORIT_M0F0;
            }
            break;
    }

    return SUCCESS;
}


static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 3000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    u32 vts = 0;

    if(fps>=3 && fps <= 30){
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            fps = fps>29?29:fps;//limit fps at 29 fps due to MCLK=36MHz
        params->expo.fps = fps;
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            params->expo.vts=  (vts_30fps*29091 + fps * 500)/(fps * 1000);
        else
            params->expo.vts=  (vts_30fps*30000 + fps * 500)/(fps * 1000);
        params->dirty = true;
    }else if(fps>=3000 && fps <= 30000){
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            fps = fps>29091?29091:fps;//limit fps at 29.091 fps due to MCLK=36MHz
        params->expo.fps = fps;
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            params->expo.vts=  (vts_30fps*29091 + (fps>>1))/fps;
        else
            params->expo.vts=  (vts_30fps*30000 + (fps>>1))/fps;
        params->dirty = true;
    }else{
      SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }

    if(params->expo.expo_lines > params->expo.vts -2){
        vts = params->expo.expo_lines + 8;
    }else{
        vts = params->expo.vts;
    }

    params->expo.vts = vts;

    pCus_SetAEUSecs(handle, params->expo.expo_lef_us);

    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_SEF1(ss_cus_sensor *handle)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if(2 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_vts_30fps = vts_30fps_HDR_DOL_2lane;
    }
    else if(4 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    }

    if (params->expo.fps >= 3000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_SEF1(ss_cus_sensor *handle, u32 fps)
{
    u32 vts = 0, cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    imx307_params *params = (imx307_params *)handle->private_data;

    if(2 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_vts_30fps =vts_30fps_HDR_DOL_2lane;
    }
    else if(4 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    }

    params->expo.fps = fps;

    if(fps>=3 && fps <= 30){
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            fps = fps>29?29:fps;//limit fps at 29 fps due to MCLK=36MHz
        params->expo.fps = fps;
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            params->expo.vts= (cur_vts_30fps*29091 + fps * 500 )/ (fps * 1000);
        else
            params->expo.vts= (cur_vts_30fps*30000 + fps * 500 )/ (fps * 1000);
        params->dirty = true;
    }else if(fps>=3000 && fps <= 30000){
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            fps = fps>29091?29091:fps;//limit fps at 29.091 fps due to MCLK=36MHz
        params->expo.fps = fps;
        if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
            params->expo.vts= (cur_vts_30fps*29091 + (fps >> 1))/fps;
        else
            params->expo.vts= (cur_vts_30fps*30000 + (fps >> 1))/fps;
        params->dirty = true;
    }else{
        //params->expo.vts=vts_30fps;
        //params->expo.fps=30;
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.expo_lines > 2 * params->expo.vts - params->max_rhs1 -3){
       vts = (params->expo.expo_lines + params->max_rhs1 + 3 + 1) / 2;
    }else{
       vts = params->expo.vts;
    }

    params->expo.vts = vts;

    if (params->expo.expo_sef_us == 0)
        params->skip_cnt = 0;
    else
        params->skip_cnt = 1;

    pCus_SetAEUSecsHDR_DOL_SEF1(handle, params->expo.expo_sef_us);

    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             //SensorReg_Write(0x3001,0);
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1);

                if(params->dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                    params->dirty = false;
                }

                if(params->orien_dirty)
                {
                    DoOrien(handle, params->orit);
                    params->orien_dirty = false;
                }

                SensorReg_Write(0x3001,0);         // printf("0x3009=0x%x,0x3014=0x%x,0x3016=0x%x,0x3020=0x%x,0x3021=0x%x\n", params->tGain_reg[1].data,params->tGain_reg[0].data,params->tGain_reg[2].data,params->tExpo_reg[2].data,params->tExpo_reg[1].data);
            }

            if(params->skip_cnt){
                sensor_if->SetSkipFrame(idx, params->expo.fps, params->skip_cnt);
                params->skip_cnt = 0;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_SEF1(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
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

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    u32 lines = 0;
    imx307_params *params = (imx307_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;
    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);
    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    imx307_params *params = (imx307_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    lines = (1000*us)/Preview_line_period;

    if (lines <1 ) lines = 1;

    params->expo.expo_lines = lines;

    if (lines >params->expo.vts-2) {
        vts = lines +2;
    }
    else
      vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %u, lines %u, vts %u\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    lines=vts-lines-1;
    params->tExpo_reg[0].data = (lines>>16) & 0x0003;
    params->tExpo_reg[1].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[2].data = (lines>>0) & 0x00ff;
    //CamOsPrintf("3022=%d  3021=%d  3020=%d\n",params->tExpo_reg[0].data,params->tExpo_reg[1].data,params->tExpo_reg[2].data);

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}



// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    unsigned short temp_gain;

    temp_gain=gain_reg[0].data;

    *gain=(u32)(10^((temp_gain*3)/200))*1024;
    if (gain_reg[1].data & 0x10)
       *gain = (*gain) * 2;

    SENSOR_DMSG("[%s] get gain/reg (1024=1X)= %u/0x%x\n", __FUNCTION__, *gain,gain_reg[0].data);
    //return rc;
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    u32 i;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;
    u64 gain_double;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;
    Sensor_Gain_Linearity = gain_gap_compensate;

    for(i = 0; i < sizeof(gain_gap_compensate)/sizeof(CUS_GAIN_GAP_ARRAY); i++){
        if (Sensor_Gain_Linearity[i].gain == 0)
            break;
        if((gain>Sensor_Gain_Linearity[i].gain) && (gain < (Sensor_Gain_Linearity[i].gain + Sensor_Gain_Linearity[i].offset))){
              gain=Sensor_Gain_Linearity[i].gain;
              break;
        }
    }

    if(gain>=22925)//if gain exceed 2x , enable high conversion gain, >27DB, enable HCG
    {
       if(params->tGain_reg[1].data==0x02){
           //params->change = true;
           //gain_reg[2].data=0x08;
       }
       else{
           //params->change = false;
           //gain_reg[2].data=0x09;
       }
           //gain_before=gain;
       params->tGain_reg[1].data |= 0x10;
       // gain_reg[2].data=0x08;
        gain /= 2;
    }
    else{
       if(params->tGain_reg[1].data==0x12){
           //params->change = true;
           //gain_reg[2].data=0x08;
       }
       else{
           //params->change = false;
           //gain_reg[2].data=0x09;
       }

       //gain_before=gain;
       params->tGain_reg[1].data &= ~0x10;
       //gain_reg[2].data=0x09;

   }
    gain_double = 20*(intlog10(gain)-intlog10(1024));
    params->tGain_reg[0].data=(u16)(((gain_double*10)>> 24)/3);
    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data);

    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_SEF1(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, cur_line_period = Preview_line_period_HDR_DOL_4LANE, cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 rhs1 = 0, shs1 = 0, vts = 0;
    imx307_params *params = (imx307_params *)handle->private_data;

    params->expo.expo_sef_us = us;

    if(2 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_line_period = Preview_line_period_HDR_DOL_2LANE;
    }
    else if(4 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    }

    if(2 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_vts_30fps =vts_30fps_HDR_DOL_2lane;
    }
    else if(4 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    }

    lines = (1000 * us + (cur_line_period >> 1)) / cur_line_period;

    //check fps to set rhs1

    if(params->expo.fps > 3000){ //3fps*1000
        if(params->expo.fps > 25000){
            if(lines > 98) {
                params->max_rhs1 = 211;
                params->expo.vts = cur_vts_30fps * 30000 / 25000;
            }
            else{
                params->expo.vts = cur_vts_30fps * 30000 / params->expo.fps;
                params->max_rhs1 = 101;
            }
        }
        else
    	    params->max_rhs1 = 211;
    }
    else {
        if(params->expo.fps > 25){
            if(lines > 98) {
                params->max_rhs1 = 211;
                params->expo.vts = cur_vts_30fps * 30 / 25;
            }
            else{
                params->expo.vts = cur_vts_30fps * 30 / params->expo.fps;
                params->max_rhs1 = 101;
            }
        }
        else
    	    params->max_rhs1 = 211;
    }

    if (params->expo.expo_lines > 2 * params->expo.vts - params->max_rhs1 - 3) {
        vts = (params->expo.expo_lines + params->max_rhs1 + 3 + 1) / 2;
    }
    else{
        vts = params->expo.vts;
    }

    rhs1 = params->max_rhs1;
    //Check boundary
	if(lines < 1)
		lines = 1;
	if(lines > rhs1- 3)
    	lines = rhs1 - 3;

    if((rhs1 - 1 - 2) <= lines){
        shs1 = 2;
    }
    else if((rhs1 <= params->max_rhs1) && (rhs1 <= params->lef_shs2 - 2)){
        shs1 = rhs1 - 1 - lines;
        if((shs1 < 2) || (shs1 > (rhs1 - 2))){ //Check boundary
            //shs1 = 0;
            //UartSendTrace("[SEF1 NG1]");
        }
    }
    else{
        //UartSendTrace("[SEF1 NG2]");
    }
    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->tExpo_reg[0].data = (shs1 >> 16) & 0x0003;
    params->tExpo_reg[1].data = (shs1 >> 8) & 0x00ff;
    params->tExpo_reg[2].data = (shs1 >> 0) & 0x00ff;

    params->tRhs1_reg[0].data = (params->max_rhs1 >> 16) & 0x0003;
    params->tRhs1_reg[1].data = (params->max_rhs1 >> 8) & 0x00ff;
    params->tRhs1_reg[2].data = (params->max_rhs1 >> 0) & 0x00ff;

    return SUCCESS;
}

static void pCus_SetAEGainHDR_DOL_Calculate(u32 gain, u16 *gain_reg)
{
    u64 gain_double;

    if(gain < SENSOR_MIN_GAIN){
      gain = SENSOR_MIN_GAIN;
    }
    else if(gain >= SENSOR_MAX_GAIN){
      gain = SENSOR_MAX_GAIN;
    }
    gain_double = 20*(intlog10(gain)-intlog10(1024));
    *gain_reg=(u16)(((gain_double*10)>> 24)/3);
}

static int pCus_SetAEGainHDR_DOL_SEF1(ss_cus_sensor *handle, u32 gain)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    u16 gain_reg = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_sef_reg[0].data = gain_reg;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data);

    params->dirty = true;
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             //SensorReg_Write(0x3001,0);
             break;
        case CUS_FRAME_ACTIVE:
        	if(params->dirty || params->orien_dirty)
            {
                SensorReg_Write(0x3001,1);
                SensorRegArrayW((I2C_ARRAY*)params->tShs2_reg, ARRAY_SIZE(expo_SHS2_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tRhs1_reg, ARRAY_SIZE(expo_RHS1_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF1_reg));
                params->dirty = false;

                if(params->orien_dirty)
                {
                    DoOrien(handle, params->orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x3001,0);
            }

            if(params->skip_cnt){
            	sensor_if->SetSkipFrame(idx, params->expo.fps, params->skip_cnt);
                params->skip_cnt = 0;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 *us)
{
    *us = 0;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 vts = 0, fsc = 0, cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    //u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    imx307_params *params = (imx307_params *)handle->private_data;

    params->expo.expo_lef_us = us;

    if(2 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_line_period = Preview_line_period_HDR_DOL_2LANE;
        //cur_vts_30fps =vts_30fps_HDR_DOL_2lane;
    }
    else if(4 == handle->interface_attr.attr_mipi.mipi_lane_num)
    {
        cur_line_period = Preview_line_period_HDR_DOL_4LANE;
        //cur_vts_30fps =vts_30fps_HDR_DOL_4lane;
    }

    params->expo.expo_lines = (1000 * us + (cur_line_period >> 1)) / cur_line_period;

    if (params->expo.expo_lines > 2 * params->expo.vts - params->max_rhs1 - 3) {
        vts = (params->expo.expo_lines + params->max_rhs1 + 3 + 1) / 2;
    }
    else{
        vts = params->expo.vts;
    }

   // lines=us/Preview_line_period_HDR_DOL_4LANE;
    SENSOR_DMSG("[%s] us %u, lines %u, vts %u\n", __FUNCTION__,
                us,
                params->expo.expo_lines,
                params->expo.vts
                );

    fsc = vts * 2;

    if(params->expo.expo_lines <= 1) {
    	params->expo.expo_lines = 1;
    }

	if(params->expo.expo_lines > fsc - 104) {
		params->expo.expo_lines = fsc - 104;
	}

	params->lef_shs2 = fsc - params->expo.expo_lines - 1;

    params->tShs2_reg[0].data = (params->lef_shs2 >> 16) & 0x0003;
    params->tShs2_reg[1].data = (params->lef_shs2 >> 8) & 0x00ff;
    params->tShs2_reg[2].data = (params->lef_shs2 >> 0) & 0x00ff;
    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32* gain)
{
    *gain = 0;
    return SUCCESS;
}

static int pCus_SetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    imx307_params *params = (imx307_params *)handle->private_data;
    u16 gain_reg = 0;


    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_lef_reg[0].data = gain_reg;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data);

    params->dirty = true;
    return SUCCESS;
}

int cus_camsensor_init_handle(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx307_params *params;
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

    params = (imx307_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));

    ///////////////////////////////////////////////////////
    // Sensor stream name
    ///////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"IMX307_MIPI_LINEAR");

    ///////////////////////////////////////////////////////
    // Sensor interface info
    ///////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC;
    handle->bayer_id                                              = SENSOR_BAYERID;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = lane_num;
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
    if(lane_num == 2)
        handle->pCus_sensor_init                                  = pCus_init_mipi2lane_linear;
    else if(lane_num == 4)
        handle->pCus_sensor_init                                  = pCus_init_mipi4lane_linear;
    else
        handle->pCus_sensor_init                                  = pCus_init_mipi2lane_linear;

    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;

    handle->pCus_sensor_SetPatternMode                            = imx307_SetPatternMode;

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
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period * 1);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    //nPixelSize
    handle->sensor_ae_info_cfg.u32PixelSize                       = 2900;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps;
    params->expo.expo_lines                                       = 673;
    if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
        params->expo.fps                                          = 29;
    else
        params->expo.fps                                          = 30;
    params->dirty = false;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef1(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx307_params *params = NULL;
    u8 res=0;

    params = (imx307_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tRhs1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tShs2_reg, expo_SHS2_reg, sizeof(expo_SHS2_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF1_reg, sizeof(gain_HDR_DOL_SEF1_reg));

    ///////////////////////////////////////////////////////
    // Sensor stream name
    ///////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"IMX307_MIPI_HDR_SEF");

    ///////////////////////////////////////////////////////
    // Sensor interface info
    ///////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DOL;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = SENSOR_HDR_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1;

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
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    if(hdr_lane_num == 2)
        handle->pCus_sensor_init                                  = pCus_init_mipi2lane_HDR_DOL;
    else if(hdr_lane_num == 4)
        handle->pCus_sensor_init                                  = pCus_init_mipi4lane_HDR_DOL;
    else
        handle->pCus_sensor_init                                  = pCus_init_mipi4lane_HDR_DOL;

    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_SEF1;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_SEF1;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_SEF1;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_SEF1;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_SEF1;

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_4LANE * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = (Preview_line_period_HDR_DOL_4LANE * 211);
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    if(hdr_lane_num == 4)
        params->expo.vts                                          = vts_30fps_HDR_DOL_4lane;
    else if(hdr_lane_num == 2)
        params->expo.vts                                          = vts_30fps_HDR_DOL_2lane;
    else
        params->expo.vts                                          = vts_30fps_HDR_DOL_4lane;

    params->expo.expo_lines                                       = 673;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx307_params *params;
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
    params = (imx307_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX307_MIPI_HDR_LEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DOL;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = hdr_lane_num;
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
    if(lane_num == 2)
        handle->pCus_sensor_init                                  = NULL;
    else if(lane_num == 4)
        handle->pCus_sensor_init                                  = NULL;
    else
        handle->pCus_sensor_init                                  = NULL;

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
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGainHDR_DOL_LEF;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_LEF;

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period_HDR_DOL_4LANE * 5);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts = vts_30fps_HDR_DOL_4lane;
    params->expo.expo_lines = 673;
    if (CUS_CMU_CLK_36MHZ == UseParaMclk(SENSOR_DRV_PARAM_MCLK()))
        params->expo.fps = 29;
    else
        params->expo.fps = 30;

    params->dirty = false;

    return SUCCESS;
}

///////////////////////////////////////////////////////
// SensorIF Entry
///////////////////////////////////////////////////////
SENSOR_DRV_ENTRY_IMPL_END_EX(  IMX307_HDR,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_hdr_dol_sef1,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            imx307_params
                         );
