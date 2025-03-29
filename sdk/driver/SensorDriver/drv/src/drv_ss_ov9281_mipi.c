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
   Date : 2023/11/7
   Build on : Master V4  I6C
   Verified on : mixer preview ok (linear). black-and-white sensor.
               AE gain/shutter ok , IQ not need.
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OV9281);

#define SENSOR_MIPI_LANE_NUM (2)
#define SENSOR_MODEL_ID     "OV9281_MIPI"
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_DELAY   0x1212                  //CFG
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10
#define SENSOR_BAYERID      CUS_BAYER_BG            //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG

#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_GAIN_STEP     (((16*1024) - 64))                  // max sensor gain 15.9375x
#define SENSOR_MAX_GAIN     (SENSOR_GAIN_STEP *8 /8)               // max sensor gain 15.9375x * 8
#define SENSOR_MIN_GAIN      (1 * 1024)


#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ         //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_27M
#define Preview_MAX_FPS     120                      //fastest preview FPS
#define Preview_MIN_FPS     5                       //slowest preview FPS

#define hts_120fps_Linear_1280           (728)
#define vts_120fps_Linear_1280           (910)                    //for 60fps 2124
#define PREVIEW_LINE_PERIOD(vts, fps)  (1000000000 / (vts) / (fps))                   //2124

#define Preview_WIDTH       1280                    //resolution Width when preview
#define Preview_HEIGHT      80                   //resolution Height when preview

#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL       (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)
//#define Preview_HEIGHT_HDR_DOL        2080


#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR     0xC0
#define SENSOR_I2C_FMT      I2C_FMT_A16D8
#define SENSOR_I2C_SPEED    200000
#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG
#define SENSOR_RST_POL      CUS_CLK_POL_NEG
#define SENSOR_PCLK_POL     CUS_CLK_POL_NEG        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
typedef enum
{
    E_CAMERA_UPDATE_SEF_HW,
    E_CAMERA_UPDATE_SEF_SET,
    E_CAMERA_UPDATE_SEF_WAIT,
} UpdateSef_e;

typedef struct {
    struct {
        float sclk;
        u32 hts;
        u32 preview_vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 Default_Res_fps;
        u32 preview_fps;
        u32 expo_lines;
        u32 vts_lef;
    } expo;
    bool reg_dirty;
    bool gain_dirty;
    bool orien_dirty;
    I2C_ARRAY tmirror_reg[2];
    I2C_ARRAY tVts_reg[4];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tExpo_reg[4];
    u32 Default_Res_line_period;
    u32 Default_Res_vts;
    CUS_CAMSENSOR_ORIT orit;
} ov9281_params;
typedef struct {
    float total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

typedef struct {
    u32 u32AgainMain;
    u32 u32AgainFloat;
    u32 u32DgainMain;
    u32 u32DgainFloat;
} Gain_Param;

I2C_ARRAY mirror_reg[] = {
    {0x3820, 0x40},//Flip bit2
    {0x3821, 0x00},//mirror bit2
};

const I2C_ARRAY gain_reg[] = {
    {0x3509, 0x00},            //long a-gain[7:0]
    {0x3507, 0x03},            //gain shift[1:0]
};

const I2C_ARRAY expo_reg[] = {
    {0x3500, 0x00},            //long exp[19,16]
    {0x3501, 0x2a},            //long exp[15,8]
    {0x3502, 0x90},            //long exp[7,0]
};

const I2C_ARRAY vts_reg[] = {
    {0x380e, ((vts_120fps_Linear_1280 >> 8) & 0xff)},
    {0x380f, (vts_120fps_Linear_1280 &0xff)},
};

const I2C_ARRAY Sensor_init_table_1280x800p120[] =
{
    {0x0103, 0x01},
    {0x0302, 0x32},
    {0x030d, 0x50},
    {0x030e, 0x02},
    {0x3001, 0x00},
    {0x3004, 0x00},
    {0x3005, 0x00},
    {0x3006, 0x04},
    {0x3011, 0x0a},
    {0x3013, 0x18},
    {0x301c, 0xf0},
    {0x3022, 0x01},
    {0x3030, 0x10},
    {0x3039, 0x32},
    {0x303a, 0x00},
    {0x3500, 0x00},
    {0x3501, 0x2a},
    {0x3502, 0x90},
    {0x3503, 0x08},
    {0x3505, 0x8c},
    {0x3507, 0x03},
    //{0x3507, 0x00},
    {0x3508, 0x00},
    {0x3509, 0x10},
    {0x3610, 0x80},
    {0x3611, 0xa0},
    {0x3620, 0x6e},
    {0x3632, 0x56},
    {0x3633, 0x78},
    {0x3662, 0x05},
    {0x3666, 0x00},
    {0x366f, 0x5a},
    {0x3680, 0x84},
    {0x3707, 0x56},
    {0x370d, 0x00},
    {0x370e, 0xfa},
    {0x3712, 0x80},
    {0x372d, 0x22},
    {0x3731, 0x80},
    {0x3732, 0x30},
    {0x3778, 0x00},
    {0x377d, 0x22},
    {0x3788, 0x02},
    {0x3789, 0xa4},
    {0x378a, 0x00},
    {0x378b, 0x4a},
    {0x3799, 0x20},
    {0x379c, 0x01},
    {0x3800, 0x00},
    {0x3801, 0x00},
    {0x3802, 0x00},
    {0x3803, 0x00},
    {0x3804, 0x05},
    {0x3805, 0x0f},
    {0x3806, 0x03},
    {0x3807, 0x2f},
    {0x3808, 0x05},
    {0x3809, 0x00},
    {0x380a, 0x03},
    {0x380b, 0x20},
    {0x380c, ((hts_120fps_Linear_1280 >> 8) & 0xff)},
    {0x380d, (hts_120fps_Linear_1280 &0xff)},
    {0x380e, ((vts_120fps_Linear_1280 >> 8) & 0xff)},
    {0x380f, (vts_120fps_Linear_1280 &0xff)},
    {0x3810, 0x00},
    {0x3811, 0x08},
    {0x3812, 0x00},
    {0x3813, 0x08},
    {0x3814, 0x11},
    {0x3815, 0x11},
    {0x3820, 0x40},
    {0x3821, 0x00},
    {0x382b, 0x3a},
    {0x382c, 0x06},
    {0x382d, 0xc2},
    {0x389d, 0x00},
    {0x3881, 0x42},
    {0x3882, 0x02},
    {0x3883, 0x12},
    {0x3885, 0x07},
    {0x38a8, 0x02},
    {0x38a9, 0x80},
    {0x38b1, 0x03},
    {0x38b3, 0x07},
    {0x38c4, 0x00},
    {0x38c5, 0xc0},
    {0x38c6, 0x04},
    {0x38c7, 0x80},
    {0x3920, 0xff},
    {0x4003, 0x40},
    {0x4008, 0x04},
    {0x4009, 0x0b},
    {0x400c, 0x00},
    {0x400d, 0x07},
    {0x4010, 0xf0},
    {0x4011, 0x3b},
    {0x4043, 0x40},
    {0x4307, 0x30},
    {0x4317, 0x00},
    {0x4501, 0x00},
    {0x4507, 0x00},
    {0x4509, 0x00},
    {0x450a, 0x08},
    {0x4601, 0x04},
    {0x470f, 0x00},
    {0x4f07, 0x00},
    {0x4800, 0x00},
    {0x5000, 0x9f},
    {0x5001, 0x00},
    {0x5e00, 0x00},
    {0x5d00, 0x07},
    {0x5d01, 0x00},
    {0x4f00, 0x04},
    {0x4f10, 0x00},
    {0x4f11, 0x98},
    {0x4f12, 0x0f},
    {0x4f13, 0xc4},
    {0x3501, 0x37},
    {0x3502, 0x50},
    {0x0100, 0x01},
};
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
}OV9281_mipi_linear[] = {
    {LINEAR_RES_1, {1280, 800, 3, 120}, {0, 0, 1280, 800}, {"1280x800@120fps"}},
};

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    ov9281_params *params = (ov9281_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = params->Default_Res_line_period * 2;
    info->u32AEShutter_step                  = params->Default_Res_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] ", __FUNCTION__);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, handle->mclk);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 2);
    }
    sensor_if->Reset(idx, SENSOR_RST_POL);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
    SENSOR_USLEEP(1000);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !SENSOR_PWDN_POL);
    SENSOR_USLEEP(5000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !SENSOR_RST_POL);
    SENSOR_USLEEP(1000);

    //sensor_if->Set3ATaskOrder(handle, def_order);
    // pure power on
    //ISP_config_io(handle);
    //sensor_if->PowerOff(idx, !SENSOR_PWDN_POL);
    //SENSOR_MSLEEP(10);
    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;

}
static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
     // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
    SENSOR_USLEEP(2000);//mantis:1690203
    return SUCCESS;
}


static int pCus_init(ss_cus_sensor *handle)
{
    ov9281_params *params = (ov9281_params *)handle->private_data;
    int i,cnt = 0;
    u32 size = 0;
    const I2C_ARRAY *sensor_init_table = NULL;

    size = ARRAY_SIZE(Sensor_init_table_1280x800p120);
    sensor_init_table = Sensor_init_table_1280x800p120;
    SENSOR_DMSG(KERN_DEBUG "[%s] %px\n", __FUNCTION__,handle);
    //UartSendTrace("OV9281 Sensor_init_table_1280x800p120\n");
    for(i=0; i< size; i++)
    {
        if(sensor_init_table[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(sensor_init_table[i].reg, sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                //printf("Sensor_init_table_1280x800p120 -> Retry %d...\n",cnt);
                if(cnt >= 10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //usleep(10*1000);
            }
            //SensorReg_Read( Sensor_init_table_1280x800p120[i].reg, &sen_data );
            //UartSendTrace("OV9281 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_1280x800p120[i].reg, Sensor_init_table_1280x800p120[i].data, sen_data);
        }
    }
    params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0x00ff;
    params->expo.final_gain = 1024;
    params->expo.expo_lines = 0x2a9;
    params->reg_dirty = true;
    params->gain_dirty = true;
    params->orien_dirty = true;
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
    ov9281_params *params = (ov9281_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = res_idx;
            handle->pCus_sensor_init = pCus_init;
            params->expo.preview_vts = vts_120fps_Linear_1280;
            params->expo.vts_lef = vts_120fps_Linear_1280;
            params->Default_Res_line_period = PREVIEW_LINE_PERIOD((params->expo.preview_vts), Preview_MAX_FPS);
            params->expo.us_per_line = params->Default_Res_line_period / 1000;
            params->expo.Default_Res_fps = Preview_MAX_FPS;
            params->Default_Res_vts = params->expo.preview_vts;
            params->expo.preview_fps = Preview_MAX_FPS;
            params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0x00ff;
            params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0x00ff;
            break;

        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    ov9281_params *params = (ov9281_params *)handle->private_data;

    *orit = params->orit;
    SENSOR_DMSG("mirror:%x\r\n", params->orit);

    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    ov9281_params *params = (ov9281_params *)handle->private_data;

    switch(orit) {
        case CUS_ORIT_M0F0:
            params->tmirror_reg[0].data = 0x40;
            params->tmirror_reg[1].data = 0;
        break;
        case CUS_ORIT_M1F0:
            params->tmirror_reg[0].data = 0x40;
            params->tmirror_reg[1].data = 0x4;
        break;
        case CUS_ORIT_M0F1:
            params->tmirror_reg[0].data = 0x44;
            params->tmirror_reg[1].data = 0;
        break;
        case CUS_ORIT_M1F1:
            params->tmirror_reg[0].data = 0x44;
            params->tmirror_reg[1].data = 0x4;
        break;
    }
    params->orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    ov9281_params *params = (ov9281_params *)handle->private_data;

    return  params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    ov9281_params *params = (ov9281_params *)handle->private_data;
    u16 height;
    u16 u16max = 60;
    u16 u16min = 5;

    u16min = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    u16max = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    if(fps>=u16min && fps <= u16max) {
        params->expo.preview_vts =  (params->Default_Res_vts * params->expo.Default_Res_fps)/fps;
        params->expo.vts_lef = params->expo.preview_vts;
        params->expo.preview_fps = fps;
        params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0xff;
        params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0xff;
        params->reg_dirty = true;
        height = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16height;
        SENSOR_DMSG("\n\n[%s], fps=%d, lines=%d\n", __FUNCTION__, fps, params->expo.preview_vts);
        return SUCCESS;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }
}

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    ov9281_params *params = (ov9281_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    //ISensorIfAPI2 *sensor_if1 = handle->sensor_if_api2;
    switch(status) {
        case CUS_FRAME_ACTIVE:
            if(params->reg_dirty || params->orien_dirty || params->gain_dirty) {
                SensorReg_Write(0x3208, 0x00);
                if(params->orien_dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tmirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
                    params->orien_dirty = false;
                }
                if(params->gain_dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
                    params->gain_dirty = false;
                }
                if(params->reg_dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, sizeof(expo_reg)/sizeof(I2C_ARRAY));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));
                    params->reg_dirty = false;
                }
                SensorReg_Write(0x3208, 0x10);
                SensorReg_Write(0x3208, 0xa0);
            }
            SENSOR_DMSG(KERN_DEBUG "[%s] set  exp:(0x%x%x)gain:(%x<<%d)(%d) vts_reg value:(0x%x%x)\n", __FUNCTION__,
                params->tExpo_reg[1].data,params->tExpo_reg[2].data>>4, params->tGain_reg[0].data, params->tGain_reg[1].data,
                params->expo.final_gain, params->tVts_reg[0].data,params->tVts_reg[1].data);
        break;
        case CUS_FRAME_INACTIVE:
        default :
        break;
    }

    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    int rc = SUCCESS;
    ov9281_params *params = (ov9281_params *)handle->private_data;

    *gain = params->expo.final_gain;

  return rc;
}
int pCus_GetSensorGainParam(u32 gain, Gain_Param *pGain)
{
    if(gain > SENSOR_MAX_GAIN)
    {
        gain = SENSOR_MAX_GAIN;
    }
    else if (gain < SENSOR_MIN_GAIN)
    {
        gain = SENSOR_MIN_GAIN;
    }
    pGain->u32DgainMain = 0;
    /*while(gain >= 0x4000)
    {
        gain = gain >> 1;
        pGain->u32DgainMain++;
    }
    if(pGain->u32DgainMain > 3)
    {
        pGain->u32DgainMain = 3;
        pGain->u32AgainMain = 0xF;
        pGain->u32AgainFloat = 0xF * 64;
    }
    else*/
    {
        pGain->u32AgainMain = gain / 1024;
        pGain->u32AgainFloat = gain % 1024;
    }
    return SUCCESS;
}
static int pCus_SetAEGainReg(Gain_Param *pGain,I2C_ARRAY *ptGain_reg)
{
    ptGain_reg[0].data = ((pGain->u32AgainMain<<4) | (pGain->u32AgainFloat/64)) &0xff; // A
    //ptGain_reg[1].data = ((pGain->u32DgainMain)) &0xff; // D
    return SUCCESS;
}
static int pCus_GetAEGainFinal(Gain_Param *pGain,u32 *final_gain)
{
    *final_gain = (pGain->u32AgainMain * 1024 + ((pGain->u32AgainFloat/64) * 64)) << pGain->u32DgainMain;
    return SUCCESS;
}
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    Gain_Param stGain;
    ov9281_params *params = (ov9281_params *)handle->private_data;
    u32 u32Gain = params->expo.final_gain;

    pCus_GetSensorGainParam(gain, &stGain);
    pCus_SetAEGainReg(&stGain, params->tGain_reg);
    pCus_GetAEGainFinal(&stGain, &params->expo.final_gain);

    if(u32Gain != params->expo.final_gain)
    {
        params->gain_dirty = true;
        SENSOR_DMSG(KERN_DEBUG "[%s] set gain %d,final gain %d, reg=0x%x << %d\n", __FUNCTION__, gain,params->expo.final_gain,
        params->tGain_reg[0].data, params->tGain_reg[1].data);
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    int rc=0;
    u32 lines = 0;
    ov9281_params *params = (ov9281_params *)handle->private_data;

    lines  = (u32)(params->tExpo_reg[2].data)>>4;
    lines |= (u32)(params->tExpo_reg[1].data)<<4;
    lines |= (u32)(params->tExpo_reg[0].data)<<12;

    *us = params->expo.final_us;

    SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
    return rc;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    ov9281_params *params = (ov9281_params *)handle->private_data;
    u32 u32preexpoline = params->expo.expo_lines;

    params->expo.final_us = us;
    lines = (1000*us)/params->Default_Res_line_period;
    if(lines < 2)
    {
        lines = 2;
    }
    if (lines >params->expo.preview_vts-26) {
        vts = lines +26;
    }
    else
        vts=params->expo.preview_vts;


    params->expo.expo_lines = lines;
    params->tExpo_reg[0].data = (lines>>12) & 0xff;
    params->tExpo_reg[1].data = (lines>>4) & 0xff;
    params->tExpo_reg[2].data = (lines<<4) & 0xff;

    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;
    if(params->expo.expo_lines != u32preexpoline)
    {
        params->reg_dirty = true;
        SENSOR_DMSG(KERN_DEBUG"[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                    us,
                    lines,
                    vts
                    );
    }
  return SUCCESS;
}

static int pCus_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    return SUCCESS;
}
int cus_camsensor_init_handle(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    ov9281_params *params;
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
    params = (ov9281_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tmirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"OV9281_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//;
    handle->data_prec   = SENSOR_DATAPREC;  //;
    handle->bayer_id    = SENSOR_BAYERID;   //;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res +1;
        handle->video_res_supported.res[res].u16width = OV9281_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height = OV9281_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps= OV9281_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps= OV9281_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x= OV9281_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y= OV9281_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth = OV9281_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = OV9281_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, OV9281_mipi_linear[res].senstr.strResDesc);
    }
    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    handle->pCus_sensor_init        = pCus_init;
    handle->pCus_sensor_poweron     = pCus_poweron;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS            = pCus_GetFPS;
    handle->pCus_sensor_SetFPS            = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode    = pCus_SetPatternMode;

    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    params->expo.preview_vts = vts_120fps_Linear_1280;
    params->expo.vts_lef = vts_120fps_Linear_1280;
    params->expo.Default_Res_fps = Preview_MAX_FPS;
    params->reg_dirty = false;
    params->gain_dirty = false;
    params->orien_dirty = false;
    params->expo.preview_fps = params->expo.Default_Res_fps;
    params->expo.us_per_line = PREVIEW_LINE_PERIOD(vts_120fps_Linear_1280, Preview_MAX_FPS)/1000;
    params->Default_Res_vts = vts_120fps_Linear_1280;
    params->Default_Res_line_period = PREVIEW_LINE_PERIOD(vts_120fps_Linear_1280, Preview_MAX_FPS);
    params->orit = SENSOR_ORIT;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = params->Default_Res_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Default_Res_line_period;
    //SENSOR_DMSG("[%s] VTS=%d!\n", __FUNCTION__, params->expo.vts);
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  OV9281,
                            cus_camsensor_init_handle,
                            NULL,
                            NULL,
                            ov9281_params
                         );
