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
   Verified on :    not
   Remark :
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC3335);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

//#undef SENSOR_DBG
#define SENSOR_DBG 0

#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI     //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000
#define SENSOR_BAYERID      CUS_BAYER_BG            //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_MAX_GAIN      (15875*31875)/1000000*1024   //max sensor gain, a-gain*conversion-gain*d-gain
#define SENSOR_MIN_GAIN      1024
#define Preview_MCLK_SPEED  CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define SENSOR_HDR_MODE             CUS_HDR_MODE_NONE

u32 Preview_line_period;
u32 vts_30fps;
#define Preview_WIDTH               2304                        //resolution Width when preview
#define Preview_HEIGHT              1296                        //resolution Height when preview
#define Preview_MAX_FPS             30                          //fastest preview FPS
#define Preview_MIN_FPS             5                           //slowest preview FPS
#define Preview_CROP_START_X        0                           //CROP_START_X
#define Preview_CROP_START_Y        0                           //CROP_START_Y

#define SENSOR_I2C_ADDR             0x60                        //I2C slave address
#define SENSOR_I2C_SPEED            240000                      //I2C speed,60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG


#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
#define ENABLE_NR 1

typedef struct {
    u32 expo_lines;
    u32 vts;
    u32 final_gain;
    u32 fps;
    u32 preview_fps;
    u32 line;
    u32 max_short_exp;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
    I2C_ARRAY tNr_reg[3];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool orient_dirty;
    bool reg_dirty;
    bool nr_dirty;
    CUS_CAMSENSOR_ORIT cur_orien;
} sc3335_params;
// set sensor ID address and data,

typedef struct {
    u64 gain;
    u8 fine_gain_reg;
} FINE_GAIN;

const I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xcc},
    {0x3108, 0x1a},
};

const I2C_ARRAY Sensor_init_table_3M30fps[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x301f,0x01},
    {0x3253,0x04},
    {0x3301,0x04},
    {0x3302,0x10},
    {0x3304,0x40},
    {0x3306,0x40},
    {0x3309,0x50},
    {0x330b,0xb6},
    {0x330e,0x29},
    {0x3310,0x06},
    {0x3314,0x96},
    {0x331e,0x39},
    {0x331f,0x49},
    {0x3320,0x09},
    {0x3333,0x10},
    {0x334c,0x01},
    {0x3364,0x17},
    {0x3367,0x01},
    {0x3390,0x04},
    {0x3391,0x08},
    {0x3392,0x38},
    {0x3393,0x05},
    {0x3394,0x09},
    {0x3395,0x16},
    {0x33ac,0x0c},
    {0x33ae,0x1c},
    {0x3622,0x16},
    {0x3637,0x22},
    {0x363a,0x1f},
    {0x363c,0x05},
    {0x3670,0x0e},
    {0x3674,0xb0},
    {0x3675,0x88},
    {0x3676,0x68},
    {0x3677,0x84},
    {0x3678,0x85},
    {0x3679,0x86},
    {0x367c,0x18},
    {0x367d,0x38},
    {0x367e,0x08},
    {0x367f,0x18},
    {0x3690,0x43},
    {0x3691,0x43},
    {0x3692,0x44},
    {0x369c,0x18},
    {0x369d,0x38},
    {0x36ea,0x3b},
    {0x36eb,0x0d},
    {0x36ec,0x1c},
    {0x36ed,0x24},
    {0x36fa,0x3b},
    {0x36fb,0x00},
    {0x36fc,0x10},
    {0x36fd,0x24},
    {0x3908,0x82},
    {0x391f,0x18},
    {0x3e01,0xa8},
    {0x3e02,0x20},
    {0x3f09,0x48},
    {0x4505,0x08},
    {0x4509,0x20},
    {0x5799,0x00},
    {0x59e0,0x60},
    {0x59e1,0x08},
    {0x59e2,0x3f},
    {0x59e3,0x18},
    {0x59e4,0x18},
    {0x59e5,0x3f},
    {0x59e6,0x06},
    {0x59e7,0x02},
    {0x59e8,0x38},
    {0x59e9,0x10},
    {0x59ea,0x0c},
    {0x59eb,0x10},
    {0x59ec,0x04},
    {0x59ed,0x02},
    {0x36e9,0x23},
    {0x36f9,0x23},
    {0x0100,0x01},
    {0xffff,0x0a},
};

I2C_ARRAY mirror_reg[] = {
    {0x3221, 0x00}, // mirror[2:1], flip[6:5]
};


typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

static I2C_ARRAY gain_reg[] = {
    {0x3e06, 0x00},
    {0x3e07, 0x80},
    {0x3e08, 0x00|0x03},
    {0x3e09, 0x40}, //low bit, 0x40 - 0x7f, step 1/64
};

I2C_ARRAY expo_reg[] = {
    {0x3e00, 0x00}, //expo [20:17]
    {0x3e01, 0x30}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
};

I2C_ARRAY vts_reg[] = {
    {0x320e, 0x05},
    {0x320f, 0x46},
};

#if ENABLE_NR
I2C_ARRAY nr_reg[] = {
{0x363c,0x05},
{0x330e,0x29},
{0x5799,0x07},
};
#endif

I2C_ARRAY PatternTbl[] = {
    {0x4501,0xc8}, //testpattern , bit 3 to enable
};

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

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
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
    info->u32AEShutter_min                   = Preview_line_period *3/2;
    info->u32AEShutter_step                  = Preview_line_period / 2;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);////pwd low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    //Sensor power on sequence

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
	sensor_if->MCLK(idx, 1, Preview_MCLK_SPEED);
    //sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    //sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    //sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    SENSOR_USLEEP(2000);
    sensor_if->Reset(idx, !CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);

    SENSOR_DMSG("[%s] pwd high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !CUS_CLK_POL_NEG);
    SENSOR_USLEEP(2000);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sc3335_params *params = (sc3335_params *)handle->private_data;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    //handle->i2c_bus->i2c_close(handle->i2c_bus);
    SENSOR_USLEEP(1000);
    //Set_csi_if(0, 0);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, Preview_MCLK_SPEED);

    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int sc3335_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
  int i;
  SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

  switch(mode) {
  case 1:
    PatternTbl[0].data = 0xc8; //enable
    break;
  case 0:
    PatternTbl[0].data = 0xc0; //disable
    break;
  default:
    PatternTbl[0].data = 0xc0; //disable
    break;
  }
  for(i=0; i< ARRAY_SIZE(PatternTbl); i++) {
      if(SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
            return FAIL;
  }

  return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{

    sc3335_params *params = (sc3335_params *)handle->private_data;

    switch(orit) {
    case CUS_ORIT_M0F0:
    {
        params->tMirror_reg[0].data = 0;
        params->orient_dirty = true;
    }
    break;
    case CUS_ORIT_M1F0:
    {
        params->tMirror_reg[0].data = 6;
        params->orient_dirty = true;
    }
    break;
    case CUS_ORIT_M0F1:
    {
        params->tMirror_reg[0].data = 0x60;
        params->orient_dirty = true;
    }
    break;
    case CUS_ORIT_M1F1:
    {
        params->tMirror_reg[0].data = 0x66;
        params->orient_dirty = true;
    }
    break;
}

  SENSOR_DMSG("pCus_SetOrien:%x\r\n", orit);

  return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    sc3335_params *params = (sc3335_params *)handle->private_data;
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

static u16 reg3109;
static int pCus_init_linear_3M30fps(ss_cus_sensor *handle)
{
    sc3335_params *params = (sc3335_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SensorReg_Read(0x3109, &reg3109);

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_3M30fps);i++)
    {
        if(Sensor_init_table_3M30fps[i].reg==0x0100 && 0x01 == Sensor_init_table_3M30fps[i].data)
        {
            sensor_if->SetCSI_Clk(0, CUS_CSI_CLK_216M);
            sensor_if->SetCSI_Lane(0, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
            sensor_if->SetCSI_LongPacketType(0, 0, 0x1C00, 0);
            SENSOR_MSLEEP(50);
        }

        if(Sensor_init_table_3M30fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_3M30fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_3M30fps[i].reg, Sensor_init_table_3M30fps[i].data) != SUCCESS)
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

    pCus_SetOrien(handle, params->cur_orien);
   // pr_info("cur_orien %s pCus_SetOrien %x\n",__FUNCTION__, params->cur_orien);
    params->tVts_reg[0].data = (params->vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->vts >> 0) & 0x00ff;
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
    sc3335_params *params = (sc3335_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"2304x1296@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_3M30fps;
            vts_30fps=1350;//1500
            params->vts = vts_30fps;
            params->fps = 30;
            Preview_line_period  = 24691;
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    sc3335_params *params = (sc3335_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->fps >= 1000)
        params->preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->preview_fps = (vts_30fps*max_fps)/tVts;

    return params->preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts=0;
    sc3335_params *params = (sc3335_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->fps = fps;
        params->vts=  (vts_30fps*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->fps = fps;
        params->vts=  (vts_30fps*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->line > 2 * (params->vts) -8){
        vts = (params->line + 9)/2;
    }else{
        vts = params->vts;
    }
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    sc3335_params *params = (sc3335_params *)handle->private_data;
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
#if ENABLE_NR
        if(params->nr_dirty)
        {
            SensorReg_Write(0x3812,0x00);
            if(params->nr_dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tNr_reg, ARRAY_SIZE(nr_reg));
                params->nr_dirty = false;
            }
            SensorReg_Write(0x3812,0x30);

        }
#endif
        break;
        default :
        break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us) {
    int rc=0;
    u32 lines = 0;
    sc3335_params *params = (sc3335_params *)handle->private_data;
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
    sc3335_params *params = (sc3335_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));

    half_lines = (1000*us*2)/Preview_line_period; // Preview_line_period in ns
    if(half_lines<3) half_lines=3;
    if (half_lines >  2 * (params->vts)-8) {
        vts = (half_lines+9)/2;
    }
    else
        vts=params->vts;
    params->line = half_lines;
    SENSOR_DMSG("[%s] us %ld, half_lines %ld, vts %ld\n", __FUNCTION__, us, half_lines, params->vts);

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
    sc3335_params *params = (sc3335_params *)handle->private_data;

    u8 i=0 ,Dgain = 1,  Coarse_gain = 1;
    u32 Fine_againx64 = 64,Fine_dgainx128 = 128;
    u8 Dgain_reg = 0, Coarse_gain_reg = 0, Fine_again_reg= 0x10,Fine_dgain_reg= 0x80;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e08, (0x00|0x03)},
        {0x3e09, 0x40},
    };
    I2C_ARRAY nr_reg_temp[] = {
        {0x363c,0x05},
        {0x330e,0x29},
        {0x5799,0x07},
    };

    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg));
    memcpy(nr_reg_temp, params->tNr_reg, sizeof(nr_reg));

    if (gain < 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN) {
        gain = SENSOR_MAX_GAIN;
    }

    if (gain < 2 * 1024)
    {
        Dgain = 1;      Fine_dgainx128 = 128;         Coarse_gain = 1;
        Dgain_reg = 0;  Fine_dgain_reg = 0x80;  Coarse_gain_reg = 0x3;
    }
    else if (gain <  4 * 1024)
    {
        Dgain = 1;      Fine_dgainx128 = 128;         Coarse_gain = 2;
        Dgain_reg = 0;  Fine_dgain_reg = 0x80;  Coarse_gain_reg = 0x7;
    }
    else if (gain <  8 * 1024)
    {
        Dgain = 1;      Fine_dgainx128 = 128;         Coarse_gain = 4;
        Dgain_reg = 0;  Fine_dgain_reg = 0x80;  Coarse_gain_reg = 0xf;
    }
    else if (gain <=  16256)
    {
        Dgain = 1;      Fine_dgainx128 = 128;         Coarse_gain = 8;
        Dgain_reg = 0;  Fine_dgain_reg = 0x80;  Coarse_gain_reg = 0x1f;
    }
    else if (gain <  32512)
    {
        Dgain = 1;      Fine_againx64 = 127;    Coarse_gain = 8;
        Dgain_reg = 0;  Fine_again_reg = 0x7f;  Coarse_gain_reg = 0x1f;
    }
    else if (gain <  65024)
    {
        Dgain = 2;      Fine_againx64 = 127;    Coarse_gain = 8;
        Dgain_reg = 1;  Fine_again_reg = 0x7f;  Coarse_gain_reg = 0x1f;
    }
    else if (gain < 127 * 1024)
    {
        Dgain = 4;         Fine_againx64 = 127;    Coarse_gain = 8;
        Dgain_reg = 0x03;  Fine_again_reg = 0x7f;  Coarse_gain_reg = 0x1f;
    }
    else if (gain < 254 * 1024)
    {
        Dgain = 8;         Fine_againx64 = 127;    Coarse_gain = 8;
        Dgain_reg = 0x07;  Fine_again_reg = 0x7f;  Coarse_gain_reg = 0x1f;
    }
    else if (gain <= SENSOR_MAX_GAIN)
    {
        Dgain = 16;        Fine_againx64 = 127;    Coarse_gain = 8;
        Dgain_reg = 0x0f;  Fine_again_reg = 0x7f;  Coarse_gain_reg = 0x1f;
    }

    if (gain <=  16256){
        Fine_againx64 = abs(8 * gain / (Dgain * Coarse_gain * Fine_dgainx128));
        Fine_again_reg = Fine_againx64;
    }else{
        Fine_dgain_reg = abs(8 * gain / (Dgain * Coarse_gain * Fine_againx64));
    }

    params->tGain_reg[3].data = Fine_again_reg;   // 9
    params->tGain_reg[2].data = Coarse_gain_reg;  // 8
    params->tGain_reg[1].data = Fine_dgain_reg;   // 7
    params->tGain_reg[0].data = Dgain_reg;        // 6

    for (i = 0; i < ARRAY_SIZE(gain_reg); i++)
    {
      if (params->tGain_reg[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

#if ENABLE_NR
    // logic
    if(reg3109 == 1)
    {
        if (gain_reg_temp[2].data < 0x07) {         // gain<2
            params->tNr_reg[0].data = 0x05;
            params->tNr_reg[1].data = 0x29;
        } else if (gain_reg_temp[2].data < 0x0F) {  // 2<=gain<4
            params->tNr_reg[0].data = 0x07;
            params->tNr_reg[1].data = 0x25;
        } else if (gain_reg_temp[2].data < 0x1F) {  // 4<=gain<8
            params->tNr_reg[0].data = 0x07;
            params->tNr_reg[1].data = 0x25;
        } else if (gain_reg_temp[2].data == 0x1F) { // gain >= 8
            params->tNr_reg[0].data = 0x07;
            params->tNr_reg[1].data = 0x18;
        }
    }
    else
    {
        if (gain_reg_temp[2].data < 0x07) {         // gain<2
            params->tNr_reg[0].data = 0x06;
            params->tNr_reg[1].data = 0x29;
        } else if (gain_reg_temp[2].data < 0x0F) {  // 2<=gain<4
            params->tNr_reg[0].data = 0x08;
            params->tNr_reg[1].data = 0x25;
        } else if (gain_reg_temp[2].data < 0x1F) {  // 4<=gain<8
            params->tNr_reg[0].data = 0x08;
            params->tNr_reg[1].data = 0x25;
        } else if (gain_reg_temp[2].data == 0x1F) { // gain >= 8
            params->tNr_reg[0].data = 0x08;
            params->tNr_reg[1].data = 0x18;
        }
    }
    // highTemp dpc
    if (gain >= 6144) // x16, x6
    {
        params->tNr_reg[2].data = 0x07;
    }
    else if (gain <= 4096) // x10, x4
    {
        params->tNr_reg[2].data = 0x00;
    }
    for (i = 0; i < ARRAY_SIZE(nr_reg); i++)
    {
      if (params->tNr_reg[i].data != nr_reg_temp[i].data)
      {
        params->nr_dirty = true;
        break;
      }
    }
#endif

    return SUCCESS;
}

int cus_camsensor_init_handle(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    sc3335_params *params;
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
    params = (sc3335_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tNr_reg, nr_reg, sizeof(nr_reg));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"sc3335_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    handle->sif_bus                                                 = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec                                               = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id                                                = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id                                                = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                  = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format               = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order                 = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                  = SENSOR_HDR_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num   = 0;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;
    handle->video_res_supported.res[0].u16width             = Preview_WIDTH;
    handle->video_res_supported.res[0].u16height            = Preview_HEIGHT;
    handle->video_res_supported.res[0].u16max_fps           = Preview_MAX_FPS;
    handle->video_res_supported.res[0].u16min_fps           = Preview_MIN_FPS;
    handle->video_res_supported.res[0].u16crop_start_x      = Preview_CROP_START_X;
    handle->video_res_supported.res[0].u16crop_start_y      = Preview_CROP_START_Y;
    handle->video_res_supported.res[0].u16OutputWidth       = 2304;
    handle->video_res_supported.res[0].u16OutputHeight      = 1296;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2304x1296@30fps");

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period * 3) / 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period / 2;
    ///calibration
    handle->pCus_sensor_init                    = pCus_init_linear_3M30fps;

    handle->pCus_sensor_poweron                 = pCus_poweron ;
    handle->pCus_sensor_poweroff                = pCus_poweroff;

    // Normal

    handle->pCus_sensor_GetVideoResNum          = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes             = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes          = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes             = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien                = pCus_SetOrien      ;
    handle->pCus_sensor_GetFPS                  = pCus_GetFPS      ;
    handle->pCus_sensor_SetFPS                  = pCus_SetFPS      ;
    handle->pCus_sensor_SetPatternMode          = sc3335_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify          = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs              = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs              = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain               = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain               = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    params->vts                             = vts_30fps;
    params->fps                             = 30;
    params->line                            = 1000;
    params->reg_dirty                       = false;
    params->nr_dirty                        = false;
    params->orient_dirty                    = false;

    //handle->snr_pad_group = SENSOR_PAD_GROUP_SET;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX( SC3335,
                            cus_camsensor_init_handle,
                            NULL,
                            NULL,
                            sc3335_params
                         );

