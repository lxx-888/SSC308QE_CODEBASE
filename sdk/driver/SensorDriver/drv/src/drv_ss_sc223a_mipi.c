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
   Date : 2023/9/25
   Build on : Master V4  I6C
   Verified on : mixer preview ok (linear only),
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC223A);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL

//============================================
//MIPI config begin.
//#define SENSOR_MIPI_LANE_NUM (2)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode 2:DCG&VC mode
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3


//#undef SENSOR_DBG
#define SENSOR_DBG 0

#define SENSOR_IFBUS_TYPE          CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC            CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10

#define SENSOR_BAYERID             CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID             CUS_RGBIR_NONE
#define SENSOR_ORIT                CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define Preview_MCLK_SPEED         CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M

#define SENSOR_MAXGAIN             (57920*4 / 1000)
u32 Preview_line_period = 29630;
u32 vts_30fps = 1125;
#define Preview_WIDTH               1920    //resolution Width when preview
#define Preview_HEIGHT              1080    //resolution Height when preview
#define Preview_MAX_FPS             30      //fastest preview FPS
#define Preview_MIN_FPS             3       //slowest preview FPS
#define Preview_CROP_START_X        0       //CROP_START_X
#define Preview_CROP_START_Y        0       //CROP_START_Y

#define SENSOR_I2C_ADDR             0x60    //I2C slave address
#define SENSOR_I2C_SPEED            200000  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY           I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT              I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL             CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL              CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL            CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL            CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL             CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
#define SENSOR_MAX_GAIN     SENSOR_MAXGAIN*1024                  // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN     1024                  // max sensor again, a-gain * conversion-gain*d-gain
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
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);

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
        float sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 preview_fps;
        u32 fps;
        u32 line;
        u32 max_short_exp;
    } expo;
    struct {
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT  orit;
    } res;

    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool orient_dirty;
    bool reg_dirty;
    CUS_CAMSENSOR_ORIT cur_orien;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
    I2C_ARRAY tGain_reg_HDR_SEF[4];
    I2C_ARRAY tExpo_reg_HDR_SEF[3];
} sc223A_params;



const static I2C_ARRAY Sensor_init_table_2M30fps_2lane[] =
{
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x37f9,0x80},
    {0x301f,0x01},
    {0x3253,0x0c},
    {0x3281,0x80},
    {0x3301,0x08},
    {0x3302,0x12},
    {0x3306,0xa0},
    {0x3309,0x60},
    {0x330a,0x01},
    {0x330b,0x58},
    {0x330d,0x20},
    {0x331e,0x41},
    {0x331f,0x51},
    {0x3320,0x0a},
    {0x3326,0x0e},
    {0x3333,0x10},
    {0x3334,0x40},
    {0x335d,0x60},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x56},
    {0x337a,0x06},
    {0x337b,0x0e},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x3390,0x03},
    {0x3391,0x0f},
    {0x3392,0x1f},
    {0x3393,0x08},
    {0x3394,0x08},
    {0x3395,0x08},
    {0x3396,0x41},
    {0x3397,0x47},
    {0x3398,0x5f},
    {0x3399,0x08},
    {0x339a,0x10},
    {0x339b,0x38},
    {0x339c,0x40},
    {0x33a2,0x04},
    {0x33a3,0x0a},
    {0x33ad,0x18},
    {0x33af,0x40},
    {0x33b1,0x80},
    {0x33b3,0x20},
    {0x349f,0x02},
    {0x34a6,0x41},
    {0x34a7,0x47},
    {0x34a8,0x28},
    {0x34a9,0x28},
    {0x34f8,0x5f},
    {0x34f9,0x20},
    {0x3630,0xc0},
    {0x3631,0x86},
    {0x3632,0x26},
    {0x3633,0x32},
    {0x363a,0x84},
    {0x3641,0x02},
    {0x3670,0x4e},
    {0x3674,0xc0},
    {0x3675,0xc0},
    {0x3676,0xc0},
    {0x3677,0x86},
    {0x3678,0x8c},
    {0x3679,0x8c},
    {0x367c,0x47},
    {0x367d,0x5f},
    {0x367e,0x47},
    {0x367f,0x5f},
    {0x3690,0x32},
    {0x3691,0x33},
    {0x3692,0x43},
    {0x3699,0x84},
    {0x369a,0x8c},
    {0x369b,0xa4},
    {0x369c,0x41},
    {0x369d,0x47},
    {0x36a2,0x41},
    {0x36a3,0x47},
    {0x370f,0x01},
    {0x3721,0x6c},
    {0x3722,0x09},
    {0x3725,0xa4},
    {0x37b0,0x09},
    {0x37b1,0x09},
    {0x37b2,0x05},
    {0x37b3,0x41},
    {0x37b4,0x5f},
    {0x3901,0x02},
    {0x3e01,0x8c},
    {0x4509,0x28},
    {0x4518,0x00},
    {0x5799,0x06},
    {0x5ae0,0xfe},
    {0x5ae1,0x40},
    {0x5ae2,0x38},
    {0x5ae3,0x30},
    {0x5ae4,0x28},
    {0x5ae5,0x38},
    {0x5ae6,0x30},
    {0x5ae7,0x28},
    {0x5ae8,0x3f},
    {0x5ae9,0x34},
    {0x5aea,0x2c},
    {0x5aeb,0x3f},
    {0x5aec,0x34},
    {0x5aed,0x2c},
    {0x5aee,0xfe},
    {0x5aef,0x40},
    {0x5af4,0x38},
    {0x5af5,0x30},
    {0x5af6,0x28},
    {0x5af7,0x38},
    {0x5af8,0x30},
    {0x5af9,0x28},
    {0x5afa,0x3f},
    {0x5afb,0x34},
    {0x5afc,0x2c},
    {0x5afd,0x3f},
    {0x5afe,0x34},
    {0x5aff,0x2c},
    {0x36e9,0x20},
    {0x37f9,0x27},
    {0x0100,0x01},
    {0xffff,0x0a}, //mdelay(10),
};

const static I2C_ARRAY Sensor_init_table_2M30fps_1lane[] =
{
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x3018,0x1a},
    {0x3019,0x02},
    {0x301f,0x02},
    {0x320c,0x08},
    {0x320d,0x98},
    {0x3253,0x0c},
    {0x3281,0x80},
    {0x3301,0x08},
    {0x3302,0x12},
    {0x3306,0xa0},
    {0x3309,0x60},
    {0x330a,0x01},
    {0x330b,0x58},
    {0x330d,0x20},
    {0x331e,0x41},
    {0x331f,0x51},
    {0x3320,0x0a},
    {0x3326,0x0e},
    {0x3333,0x10},
    {0x3334,0x40},
    {0x335d,0x60},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x56},
    {0x337a,0x06},
    {0x337b,0x0e},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x3390,0x03},
    {0x3391,0x0f},
    {0x3392,0x1f},
    {0x3393,0x08},
    {0x3394,0x08},
    {0x3395,0x08},
    {0x3396,0x41},
    {0x3397,0x47},
    {0x3398,0x5f},
    {0x3399,0x08},
    {0x339a,0x10},
    {0x339b,0x38},
    {0x339c,0x40},
    {0x33a2,0x04},
    {0x33a3,0x0a},
    {0x33ad,0x18},
    {0x33af,0x40},
    {0x33b1,0x80},
    {0x33b3,0x20},
    {0x349f,0x02},
    {0x34a6,0x41},
    {0x34a7,0x47},
    {0x34a8,0x28},
    {0x34a9,0x28},
    {0x34f8,0x5f},
    {0x34f9,0x20},
    {0x3630,0xc0},
    {0x3631,0x86},
    {0x3632,0x26},
    {0x3633,0x32},
    {0x363a,0x84},
    {0x3641,0x02},
    {0x3670,0x4e},
    {0x3674,0xc0},
    {0x3675,0xc0},
    {0x3676,0xc0},
    {0x3677,0x86},
    {0x3678,0x8c},
    {0x3679,0x8c},
    {0x367c,0x47},
    {0x367d,0x5f},
    {0x367e,0x47},
    {0x367f,0x5f},
    {0x3690,0x32},
    {0x3691,0x33},
    {0x3692,0x43},
    {0x3699,0x84},
    {0x369a,0x8c},
    {0x369b,0xa4},
    {0x369c,0x41},
    {0x369d,0x47},
    {0x36a2,0x41},
    {0x36a3,0x47},
    {0x36ec,0x0c},
    {0x370f,0x01},
    {0x3721,0x6c},
    {0x3722,0x09},
    {0x3725,0xa4},
    {0x37b0,0x09},
    {0x37b1,0x09},
    {0x37b2,0x05},
    {0x37b3,0x41},
    {0x37b4,0x5f},
    {0x3901,0x02},
    {0x3e01,0x8c},
    {0x4509,0x28},
    {0x4518,0x00},
    {0x4819,0x09},
    {0x481b,0x05},
    {0x481d,0x14},
    {0x481f,0x04},
    {0x4821,0x0a},
    {0x4823,0x05},
    {0x4825,0x04},
    {0x4827,0x05},
    {0x4829,0x08},
    {0x5799,0x06},
    {0x5ae0,0xfe},
    {0x5ae1,0x40},
    {0x5ae2,0x38},
    {0x5ae3,0x30},
    {0x5ae4,0x28},
    {0x5ae5,0x38},
    {0x5ae6,0x30},
    {0x5ae7,0x28},
    {0x5ae8,0x3f},
    {0x5ae9,0x34},
    {0x5aea,0x2c},
    {0x5aeb,0x3f},
    {0x5aec,0x34},
    {0x5aed,0x2c},
    {0x5aee,0xfe},
    {0x5aef,0x40},
    {0x5af4,0x38},
    {0x5af5,0x30},
    {0x5af6,0x28},
    {0x5af7,0x38},
    {0x5af8,0x30},
    {0x5af9,0x28},
    {0x5afa,0x3f},
    {0x5afb,0x34},
    {0x5afc,0x2c},
    {0x5afd,0x3f},
    {0x5afe,0x34},
    {0x5aff,0x2c},
    {0x36e9,0x20},
    {0x0100,0x01},
    {0xffff,0x0a}, //mdelay(10),
};

const static I2C_ARRAY mirror_reg[] =
{
    {0x3221, 0x00}, // mirror[2:1], flip[6:5]
};

const static I2C_ARRAY gain_reg[] = {
    {0x3e06, 0x00},
    {0x3e07, 0x80},
    // {0x3e08, 0x03},
    {0x3e09, 0x40}, //low bit, 0x10 - 0x3e0, step 1/64
};

const static I2C_ARRAY gain_reg_HDR_SEF[] = {
    {0x3e10, 0x00},
    {0x3e11, 0x80},
    {0x3e12, 0x03},
    {0x3e13, 0x40}, //low bit, 0x10 - 0x3e0, step 1/16
    };

const static I2C_ARRAY expo_reg[] = {  // max expo line vts*2-6!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x8c}, // expo[16:8]
    {0x3e02, 0x40}, // expo[7:0], [3:0] fraction of line
};

const static I2C_ARRAY expo_reg_HDR_SEF[] = {
    {0x3e22, 0x00}, // expo[3:0]
    {0x3e04, 0x21}, // expo[7:0]
    {0x3e05, 0x00}, // expo[7:4]
};

const static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x04},
    {0x320f, 0x65}
};

/* static I2C_ARRAY max_short_exp_reg[] = {
    {0x3e23, 0x00},
    {0x3e24, 0x87}
}; */

#if 0
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
#endif

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME sc223A

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period / 2;
    info->u32AEShutter_step                  = Preview_line_period / 2;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    //ISP_config_io(handle);
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);

    //Sensor power on sequence
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    sensor_if->MCLK(idx, 1, handle->mclk);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_MSLEEP(1);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_MSLEEP(1);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sc223A_params *params = (sc223A_params *)handle->private_data;

    // power/reset low
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_MSLEEP(1);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);

    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

/////////////////// image function /////////////////////////
//Get and check sensor ID
static int sc223A_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);
    return SUCCESS;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init_mipi2lane_linear_2M30fps_2lane(ss_cus_sensor *handle)
{
    sc223A_params *params = (sc223A_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2M30fps_2lane);i++)
    {
        if(Sensor_init_table_2M30fps_2lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2M30fps_2lane[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2M30fps_2lane[i].reg, Sensor_init_table_2M30fps_2lane[i].data) != SUCCESS)
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

    //pCus_SetOrien(handle, params->cur_orien);
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    return SUCCESS;
}

static int pCus_init_mipi2lane_linear_2M30fps_1lane(ss_cus_sensor *handle)
{
    sc223A_params *params = (sc223A_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2M30fps_1lane);i++)
    {
        if(Sensor_init_table_2M30fps_1lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2M30fps_1lane[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2M30fps_1lane[i].reg, Sensor_init_table_2M30fps_1lane[i].data) != SUCCESS)
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

    //pCus_SetOrien(handle, params->cur_orien);
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
    sc223A_params *params = (sc223A_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"1920x1080@30fps"
            handle->video_res_supported.ulcur_res = 0;
            if(lane_num == 2){
                handle->pCus_sensor_init = pCus_init_mipi2lane_linear_2M30fps_2lane;
            }else if(lane_num == 1){
                handle->pCus_sensor_init = pCus_init_mipi2lane_linear_2M30fps_1lane;
            }else{
                handle->pCus_sensor_init = pCus_init_mipi2lane_linear_2M30fps_2lane;
            }
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

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    sc223A_params *params = (sc223A_params *)handle->private_data;
    sen_data = params->tMirror_reg[0].data;
    SENSOR_DMSG("mirror:%x\r\n", sen_data);
    switch(sen_data) {
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
    sc223A_params *params = (sc223A_params *)handle->private_data;

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[0].data = 0;
            params->orient_dirty = true;
            break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[0].data = 6;
            params->orient_dirty = true;
            break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[0].data = 0x60;
            params->orient_dirty = true;
            break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[0].data = 0x66;
            params->orient_dirty = true;
            break;
        default :

            break;
    }
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    sc223A_params *params = (sc223A_params *)handle->private_data;
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
    sc223A_params *params = (sc223A_params *)handle->private_data;
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

    if(params->expo.line > 2* (params->expo.vts) -10){
        vts = (params->expo.line + 11)/2;
    }else{
        vts = params->expo.vts;
    }
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;

}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
   sc223A_params *params = (sc223A_params *)handle->private_data;

   switch(status)
   {
        case CUS_FRAME_INACTIVE:
            break;
        case CUS_FRAME_ACTIVE:
            if(params->orient_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->orient_dirty = false;
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

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    int rc=0;
    u32 lines = 0;
    sc223A_params *params = (sc223A_params *)handle->private_data;

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
    u32 half_lines = 0,vts = 0;
    sc223A_params *params = (sc223A_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {
    {0x3e00, 0x00}, //expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));

    half_lines = (1000*us*2)/Preview_line_period; // Preview_line_period in ns
    if(half_lines <= 1) half_lines=1;
    if (half_lines >  2 * (params->expo.vts)-8) {
        vts = (half_lines+9)/2;
    }
    else
        vts=params->expo.vts;

    params->expo.line = half_lines;
    SENSOR_DMSG("[%s] us %ld, half_lines %ld, vts %ld\n", __FUNCTION__, us, half_lines, params->expo.vts);

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
    sc223A_params *params = (sc223A_params *)handle->private_data;
    u8 i=0 ;
    u16 reg_gain, ana_real_gain, dcg = 100, DIG_Fine_gain_reg = 0;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e09, 0x00},
    };
    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAXGAIN*1024) {
        gain = SENSOR_MAXGAIN*1024;
    }

    if (gain <= 1024)//64
        gain = 1024;
    else if (gain >= 138000)
        gain = 138000;

    if((gain >= 1024)&&(gain < 1854)) // x1.81
    {
        dcg = 100;
        reg_gain = 0x00;
        ana_real_gain = 1;
    }
    else if((gain >= 1854)&&(gain < 3707)) // 1.81 ~ 3.62
    {
        dcg = 181;
        reg_gain = 0x40;
        ana_real_gain = 1;
    }
    else if((gain >= 3707)&&(gain < 7414)) // 3.62 ~ 7.24
    {
        dcg = 181;
        reg_gain = 0x41;
        ana_real_gain = 2;
    }
    else if((gain >= 7414)&&(gain < 14828)) // 7.24 ~ 14.48
    {
        dcg = 181;
        reg_gain = 0x43;
        ana_real_gain = 4;
    }
    else if((gain >= 14828)&&(gain < 29656)) // 14.48 ~ 28.96
    {
        dcg = 181;
        reg_gain = 0x47;
        ana_real_gain = 8;
    }
    else if((gain >= 29656)&&(gain < 59311)) // 28.96 ~ 57.92
    {
        dcg = 181;
        reg_gain = 0x4f;
        ana_real_gain = 16;
    }
    else if((gain >= 59311)&&(gain < 118621)) // 57.92 ~ 57.92 * 2
    {
        dcg = 181;
        reg_gain = 0x5f;
        ana_real_gain = 32;
    }
    else
    {
        dcg = 181;
        reg_gain = 0x5f;
        ana_real_gain = 32;
        DIG_Fine_gain_reg = 0xFF;
    }

    if(gain < 1854)
    {
        DIG_Fine_gain_reg = abs(gain/ana_real_gain/8);
    }else if(gain >= 1854) // || gain == 3456)
    {
        DIG_Fine_gain_reg = abs(800*gain/dcg/ana_real_gain/63);
    }

    if (DIG_Fine_gain_reg >= 0xFF) // Never delete it. // icekirin
    {
        DIG_Fine_gain_reg = 0xFF;
    }
    params->tGain_reg[2].data = reg_gain;  // 0x3e09
    params->tGain_reg[1].data = DIG_Fine_gain_reg;  // 0x3e07
    params->tGain_reg[0].data = 0x0;  // 0x3e06

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

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    sc223A_params *params;
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
    params = (sc223A_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"sc223A_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->interface_attr.attr_mipi.mipi_lane_num = lane_num;//SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].u16width = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height = Preview_HEIGHT;
    handle->video_res_supported.res[0].u16max_fps= Preview_MAX_FPS;
    handle->video_res_supported.res[0].u16min_fps= Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x= 0;
    handle->video_res_supported.res[0].u16crop_start_y= 0;
    handle->video_res_supported.res[0].u16OutputWidth= 1920;
    handle->video_res_supported.res[0].u16OutputHeight= 1080;
    sprintf(handle->video_res_supported.res[0].strResDesc, "1920x1080@30fps");


    // i2c

    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period / 2; // half line
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period /2; // half line
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    if(lane_num == 2){
        handle->pCus_sensor_init        = pCus_init_mipi2lane_linear_2M30fps_2lane;
    }else if(lane_num == 1){
        handle->pCus_sensor_init        = pCus_init_mipi2lane_linear_2M30fps_1lane;
    }else{
        handle->pCus_sensor_init        = pCus_init_mipi2lane_linear_2M30fps_2lane;
    }

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
    handle->pCus_sensor_SetPatternMode = sc223A_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    params->expo.vts=vts_30fps;
    params->expo.fps = 30;
    params->expo.line= 1000;
    params->reg_dirty = false;
    params->orient_dirty = false;
    return SUCCESS;
}


SENSOR_DRV_ENTRY_IMPL_END_EX( SC223A,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            sc223A_params
                         );
