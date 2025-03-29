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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OS05A10);

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
#define SENSOR_NAME                           OS05A10

#define SENSOR_IFBUS_TYPE                     CUS_SENIF_BUS_MIPI
//Linear Mode
#define SENSOR_MIPI_LANE_NUM                  (4)
#define Preview_MCLK_SPEED                    CUS_CMU_CLK_24MHZ

//I2C bus
#define SENSOR_I2C_ADDR                       0x6c
#define SENSOR_I2C_SPEED                      20000
#define SENSOR_I2C_LEGACY                     I2C_NORMAL_MODE
#define SENSOR_I2C_FMT                        I2C_FMT_A16D8

#define SENSOR_RGBIRID                        CUS_RGBIR_NONE

#define SENSOR_DATAPREC                       CUS_DATAPRECISION_12
#define SENSOR_BAYERID                        CUS_BAYER_BG

//AE info
#define SENSOR_MAX_GAIN                       (246 * 1024)
#define SENSOR_MIN_GAIN                       (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)

u32 Preview_line_period;
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
    {LINEAR_RES_1, {2592, 1944, 3, 30}, {0, 0, 2592, 1944}, {"2592x1944@30fps"}},
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
    I2C_ARRAY tVts_reg_HDR[2];
    I2C_ARRAY tMax_short_exp_reg[3];
    I2C_ARRAY tGain_reg_HDR_SEF[5];
    I2C_ARRAY tGain_reg_HDR_LEF[5];
    I2C_ARRAY tExpo_reg_HDR_SEF[2];
    I2C_ARRAY tExpo_reg_HDR_LEF[2];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[2];
    bool orien_dirty;
    bool reg_dirty;
    bool vts_reg_dirty;
    bool dirty;
    bool change;
} OS05A10_params;

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)     (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)     (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

// set sensor ID address and data,
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x300a, 0x53},      // {address of ID, ID },
    {0x300b, 0x05},      // {address of ID, ID },
};

const static I2C_ARRAY Sensor_init_table_5M30fps[] =
{
//@@ Res 2592X1944 4lane MIPI0512Mbps Linear12 30fps MCLK24M VTS1984
//;version = OS05A10_R1A_AM14
    {0x0100, 0x00},
    {0x0103, 0x01},
    {0x0303, 0x01},
    {0x0305, 0x27},
    {0x0306, 0x00},
    {0x0307, 0x00},
    {0x0308, 0x03},
    {0x0309, 0x04},
    {0x032a, 0x00},
    {0x031e, 0x0a},
    {0x0325, 0x48},
    {0x0328, 0x07},
    {0x300d, 0x11},
    {0x300e, 0x11},
    {0x300f, 0x11},
    {0x3010, 0x01},
    {0x3012, 0x41},
    {0x3016, 0xf0},
    {0x3018, 0xf0},
    {0x3028, 0xf0},
    {0x301e, 0x98},
    {0x3010, 0x04},
    {0x3011, 0x06},
    {0x3031, 0xa9},
    {0x3103, 0x48},
    {0x3104, 0x01},
    {0x3106, 0x10},
    {0x3501, 0x09},
    {0x3502, 0x2c},
    {0x3505, 0x83},
    {0x3508, 0x00},
    {0x3509, 0x80},
    {0x350a, 0x04},
    {0x350b, 0x00},
    {0x350c, 0x00},
    {0x350d, 0x80},
    {0x350e, 0x04},
    {0x350f, 0x00},
    {0x3600, 0x00},
    {0x3626, 0xff},
    {0x3605, 0x50},
    {0x3609, 0xdb},
    {0x3610, 0x69},
    {0x360c, 0x01},
    {0x3628, 0xa4},
    {0x3629, 0x6a},
    {0x362d, 0x10},
    {0x3660, 0xd3},
    {0x3661, 0x06},
    {0x3662, 0x00},
    {0x3663, 0x28},
    {0x3664, 0x0d},
    {0x366a, 0x38},
    {0x366b, 0xa0},
    {0x366d, 0x00},
    {0x366e, 0x00},
    {0x3680, 0x00},
    {0x36c0, 0x00},
    {0x3621, 0x81},
    {0x3634, 0x31},
    {0x3620, 0x00},
    {0x3622, 0x00},
    {0x362a, 0xd0},
    {0x362e, 0x8c},
    {0x362f, 0x98},
    {0x3630, 0xb0},
    {0x3631, 0xd7},
    {0x3701, 0x0f},
    {0x3737, 0x02},
    {0x3740, 0x18},
    {0x3741, 0x04},
    {0x373c, 0x0f},
    {0x373b, 0x02},
    {0x3705, 0x00},
    {0x3706, 0xa0},
    {0x370a, 0x01},
    {0x370b, 0xc8},
    {0x3709, 0x4a},
    {0x3714, 0x21},
    {0x371c, 0x00},
    {0x371d, 0x08},
    {0x375e, 0x0e},
    {0x3760, 0x13},
    {0x3776, 0x10},
    {0x3781, 0x02},
    {0x3782, 0x04},
    {0x3783, 0x02},
    {0x3784, 0x08},
    {0x3785, 0x08},
    {0x3788, 0x01},
    {0x3789, 0x01},
    {0x3797, 0x84},
    {0x3798, 0x01},
    {0x3799, 0x00},
    {0x3761, 0x02},
    {0x3762, 0x0d},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x0c},
    {0x3804, 0x0e},
    {0x3805, 0xff},
    {0x3806, 0x08},
    {0x3807, 0x6f},
    {0x3808, 0x0a},
    {0x3809, 0x20},
    {0x380a, 0x07},
    {0x380b, 0x98},
    {0x380c, 0x03},
    {0x380d, 0xf0},
    {0x380e, 0x09},
    {0x380f, 0x4c},
    {0x3813, 0x04},
    {0x3814, 0x01},
    {0x3815, 0x01},
    {0x3816, 0x01},
    {0x3817, 0x01},
    {0x381c, 0x00},
    {0x3820, 0x00},
    {0x3821, 0x04},
    {0x3823, 0x18},
    {0x3826, 0x00},
    {0x3827, 0x01},
    {0x3833, 0x00},
    {0x3832, 0x02},
    {0x383c, 0x48},
    {0x383d, 0xff},
    {0x3843, 0x20},
    {0x382d, 0x08},
    {0x3d85, 0x0b},
    {0x3d84, 0x40},
    {0x3d8c, 0x63},
    {0x3d8d, 0x00},
    {0x4000, 0x78},
    {0x4001, 0x2b},
    {0x4004, 0x01},
    {0x4005, 0x00},
    {0x4028, 0x2f},
    {0x400a, 0x01},
    {0x4010, 0x12},
    {0x4008, 0x02},
    {0x4009, 0x0d},
    {0x401a, 0x58},
    {0x4050, 0x00},
    {0x4051, 0x01},
    {0x4052, 0x00},
    {0x4053, 0x80},
    {0x4054, 0x00},
    {0x4055, 0x80},
    {0x4056, 0x00},
    {0x4057, 0x80},
    {0x4058, 0x00},
    {0x4059, 0x80},
    {0x430b, 0xff},
    {0x430c, 0xff},
    {0x430d, 0x00},
    {0x430e, 0x00},
    {0x4501, 0x18},
    {0x4502, 0x00},
    {0x4600, 0x00},
    {0x4601, 0x10},
    {0x4603, 0x01},
    {0x4643, 0x00},
    {0x4640, 0x01},
    {0x4641, 0x04},
    {0x480e, 0x00},
    {0x4813, 0x00},
    {0x4815, 0x2b},
    {0x486e, 0x36},
    {0x486f, 0x84},
    {0x4860, 0x00},
    {0x4861, 0xa0},
    {0x484b, 0x05},
    {0x4850, 0x00},
    {0x4851, 0xaa},
    {0x4852, 0xff},
    {0x4853, 0x8a},
    {0x4854, 0x08},
    {0x4855, 0x30},
    {0x4800, 0x60},
    {0x4837, 0x19},
    {0x484a, 0x3f},
    {0x5000, 0xc9},
    {0x5001, 0x43},
    {0x5002, 0x00},
    {0x5211, 0x03},
    {0x5291, 0x03},
    {0x520d, 0x0f},
    {0x520e, 0xfd},
    {0x520f, 0xa5},
    {0x5210, 0xa5},
    {0x528d, 0x0f},
    {0x528e, 0xfd},
    {0x528f, 0xa5},
    {0x5290, 0xa5},
    {0x5004, 0x40},
    {0x5005, 0x00},
    {0x5180, 0x00},
    {0x5181, 0x10},
    {0x5182, 0x0f},
    {0x5183, 0xff},
    {0x580b, 0x03},
    {0x4d00, 0x03},
    {0x4d01, 0xe9},
    {0x4d02, 0xba},
    {0x4d03, 0x66},
    {0x4d04, 0x46},
    {0x4d05, 0xa5},
    {0x3603, 0x3c},
    {0x3703, 0x26},
    {0x3709, 0x49},
    {0x3708, 0x2d},
    {0x3719, 0x1c},
    {0x371a, 0x06},
    {0x4000, 0x79},
    {0x0305, 0x20},
    {0x4837, 0x1f},
    {0x380c, 0x07},
    {0x380d, 0x17},
    {0x380e, 0x07},
    {0x380f, 0xc0},
    {0x3501, 0x05},
    {0x3502, 0xc0},
    {0x0100, 0x01},
    {0x0100, 0x01},
    {0x0100, 0x01},
    {0x0100, 0x01},
};

const static I2C_ARRAY mirror_reg[] =
{
    {0x3820, 0x00},//M0F0
    {0x3821, 0x00},
};

const static I2C_ARRAY gain_reg[] = {
    {0x3508, 0x01},//long a-gain [8:4] bit[7:0]
    {0x3509, 0x00},//long a-gain [3:0] bit[7:4]
    {0x350A, 0x01},// d-gain[13:10]
    {0x350B, 0x00},// d-gain[9:2]
    {0x350C, 0x00},// d-gain[1:0] bit[7:6]
};


const static I2C_ARRAY expo_reg[] = {
    {0x3208, 0x00},//Group 0 hold start
    {0x3501, 0x02},//long exp[15,8]
    {0x3502, 0x00},//long exp[7,0]
};

const static I2C_ARRAY vts_reg[] = {
    {0x380e, 0x06},
    {0x380f, 0x66},
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
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period *3/2;
    info->u32AEShutter_step                  = Preview_line_period / 2;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    OS05A10_params *params = (OS05A10_params *)handle->private_data;

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
    OS05A10_params *params = (OS05A10_params *)handle->private_data;

    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);

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
    u8 sen_data[2] = {0};
    OS05A10_params *params = (OS05A10_params *)handle->private_data;

    sen_data[0] = params->tMirror_reg[0].data;
    sen_data[1] = params->tMirror_reg[1].data;
    if(sen_data[0] == 0x00 && sen_data[1] == 0x00){
        *orit = CUS_ORIT_M0F0;
    }
    else if(sen_data[0] == 0x00 && sen_data[1] == 0x04){
        *orit = CUS_ORIT_M1F0;
    }
    else if(sen_data[0] == 0x24 && sen_data[1] == 0x00){
        *orit = CUS_ORIT_M0F1;
    }
    else if(sen_data[0] == 0x24 && sen_data[1] == 0x04){
        *orit = CUS_ORIT_M1F1;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    OS05A10_params *params = (OS05A10_params *)handle->private_data;

    switch(orit) {
    case CUS_ORIT_M0F0:
        params->tMirror_reg[0].data = 0x00;
        params->tMirror_reg[1].data = 0x00;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F0:
        params->tMirror_reg[0].data = 0x00;
        params->tMirror_reg[1].data = 0x04;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M0F1:
        params->tMirror_reg[0].data = 0x24;
        params->tMirror_reg[1].data = 0x00;
        params->orien_dirty = true;
    break;
    case CUS_ORIT_M1F1:
        params->tMirror_reg[0].data = 0x24;
        params->tMirror_reg[1].data = 0x04;
        params->orien_dirty = true;
        break;
    default :
        break;
    }

    params->orit = orit;
    return SUCCESS;
}


/////////////////// image function /////////////////////////

static int OS05A10_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

  return SUCCESS;
}

static int pCus_init_linear_5M30fps(ss_cus_sensor *handle)
{
    OS05A10_params *params = (OS05A10_params *)handle->private_data;
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
    OS05A10_params *params = (OS05A10_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_5M30fps;
            vts_30fps=1984;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period  = 16801;
            break;
        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    OS05A10_params *params = (OS05A10_params *)handle->private_data;
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
    OS05A10_params *params = (OS05A10_params *)handle->private_data;
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

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    OS05A10_params *params = (OS05A10_params *)handle->private_data;
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

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    int rc = SUCCESS;
    u32 lines = 0;
    OS05A10_params *params = (OS05A10_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;

    return rc;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0;
    OS05A10_params *params = (OS05A10_params *)handle->private_data;

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
    params->tExpo_reg[0].data = (lines>>16) & 0x000f;
    params->tExpo_reg[1].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[2].data = (lines>>0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;

    params->reg_dirty = true;

    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    OS05A10_params *params = (OS05A10_params *)handle->private_data;

    if(params->tGain_reg[0].data==0)
	*gain=(u32)((params->tGain_reg[1].data/128)<<10);
    else if(params->tGain_reg[0].data==1)
       *gain=(u32)(((params->tGain_reg[1].data+8)/64)<<10);
    else if(params->tGain_reg[0].data==3)
       *gain=(u32)(((params->tGain_reg[1].data+12)/32)<<10);
    else if(params->tGain_reg[0].data==7)
       *gain=(u32)(((params->tGain_reg[1].data+8)/16)<<10);


    SENSOR_DMSG("[%s] get gain/reg0/reg1 (1024=1X)= %d/0x%x/0x%x\n", __FUNCTION__, *gain,params->tGain_reg[0].data,params->tGain_reg[1].data);
    return SUCCESS;
}
#define MAX_A_GAIN 15872//(16*1024)
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    OS05A10_params *params = (OS05A10_params *)handle->private_data;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;
    u32 i,input_gain = 0;
    u16 gain16;

    if (gain < 1024) gain = 1024;
    else if (gain >= SENSOR_MAX_GAIN * 1024) gain = SENSOR_MAX_GAIN * 1024;

    gain = (gain * 1024 + 512)>>10; // need to add min sat gain

    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;


    Sensor_Gain_Linearity = gain_gap_compensate;

    for(i = 0; i < sizeof(gain_gap_compensate)/sizeof(CUS_GAIN_GAP_ARRAY); i++){

        if (Sensor_Gain_Linearity[i].gain == 0)
            break;
        if((gain>Sensor_Gain_Linearity[i].gain) && (gain < (Sensor_Gain_Linearity[i].gain + Sensor_Gain_Linearity[i].offset))){
              gain=Sensor_Gain_Linearity[i].gain;
              break;
        }
    }

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

    gain16=(u16)(gain>>3);
    params->tGain_reg[0].data = (gain16>>8)&0x3f;//high bit
    params->tGain_reg[1].data = gain16&0xff; //low byte

    if(input_gain > MAX_A_GAIN){
        params->tGain_reg[2].data=(u16)((input_gain*4)/MAX_A_GAIN)&0x3F;
        params->tGain_reg[3].data=(u16)((input_gain*1024)/MAX_A_GAIN)&0xFF;
    }
    else{
        u16 tmp_dgain = ((input_gain*1024)/gain);
        params->tGain_reg[2].data=(u16)((tmp_dgain >> 8) & 0x3F);
        params->tGain_reg[3].data=(u16)(tmp_dgain & 0xFF);
    }

    params->dirty = true;
    //pr_info("[%s] set input gain/gain/AregH/AregL/DregH/DregL=%d/%d/0x%x/0x%x/0x%x/0x%x\n", __FUNCTION__, input_gain,gain,params->tGain_reg[0].data,params->tGain_reg[1].data,params->tGain_reg[2].data,params->tGain_reg[3].data);

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
    OS05A10_params *params;
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

    params = (OS05A10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ///////////////////////////////////////////////////////
    // Sensor stream name
    ///////////////////////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OS05A10_MIPI");

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

    handle->pCus_sensor_SetPatternMode                            = OS05A10_SetPatternMode;

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
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period *3/2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
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

SENSOR_DRV_ENTRY_IMPL_END_EX( OS05A10,
                            cus_camsensor_init_handle,
                            NULL,
                            NULL,
                            OS05A10_params
                         );

