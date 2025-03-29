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
   Porting owner :  mountain.li
   Date :           9/26/2023
   Build on :       Master_V4 I6C
   Verified on :    not yet
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC4336);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

//#undef SENSOR_DBG
#define SENSOR_DBG 0

#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_MAXGAIN      (32768*15 / 1000)//32XAGAIN 15XDGAIN
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define Preview_MCLK_SPEED  CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M

u32 Preview_line_period = 22222;
u32 vts_30fps = 1500;

#define Preview_WIDTH       2560                    //resolution Width when preview
#define Preview_HEIGHT      1440                    //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     3                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR    0x60                   //I2C slave address
#define SENSOR_I2C_SPEED   200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
//static int cus_camsensor_release_handle(ss_cus_sensor *handle);
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
//static int g_sensor_ae_min_gain = 1024;

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
        u32 sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 final_us;
        u32 final_gain;
        u32 back_pv_us;
        u32 preview_fps;
        u32 fps;
        u32 line;
    } expo;
    struct {
        bool bVideoMode;
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool orient_dirty;
    bool reg_dirty;
    CUS_CAMSENSOR_ORIT cur_orien;
} SC4336_params;
// set sensor ID address and data,
const I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xdc},
    {0x3108, 0x42},
};

const I2C_ARRAY Sensor_init_table_4M30fps[] =
{
    //cleaned_0x01_FT_SC4336_MIPI_27Minput_2lane_10bit_630Mbps_2560x1440_30fps.ini
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x37f9,0x80},
    {0x301f,0x01},
    {0x30b8,0x44},
    {0x3253,0x10},
    {0x3301,0x0a},
    {0x3302,0xff},
    {0x3305,0x00},
    {0x3306,0x90},
    {0x3308,0x20},
    {0x330a,0x01},
    {0x330b,0xb0},
    {0x330d,0xf0},
    {0x3333,0x10},
    {0x335e,0x06},
    {0x335f,0x0a},
    {0x3364,0x5e},
    {0x337d,0x0e},
    {0x3390,0x08},
    {0x3391,0x09},
    {0x3392,0x0f},
    {0x3393,0x18},
    {0x3394,0x60},
    {0x3395,0xff},
    {0x3396,0x08},
    {0x3397,0x09},
    {0x3398,0x0f},
    {0x3399,0x0a},
    {0x339a,0x18},
    {0x339b,0x60},
    {0x339c,0xff},
    {0x33a2,0x04},
    {0x33ad,0x3c},
    {0x33b2,0x40},
    {0x33b3,0x30},
    {0x33f8,0x00},
    {0x33f9,0xa0},
    {0x33fa,0x00},
    {0x33fb,0xe0},
    {0x33fc,0x09},
    {0x33fd,0x1f},
    {0x349f,0x03},
    {0x34a6,0x09},
    {0x34a7,0x1f},
    {0x34a8,0x28},
    {0x34a9,0x28},
    {0x34aa,0x01},
    {0x34ab,0xd0},
    {0x34ac,0x02},
    {0x34ad,0x10},
    {0x34f8,0x1f},
    {0x34f9,0x20},
    {0x3630,0xc0},
    {0x3631,0x84},
    {0x3633,0x44},
    {0x3637,0x4c},
    {0x3641,0x38},
    {0x3670,0x56},
    {0x3674,0xc0},
    {0x3675,0xa0},
    {0x3676,0xa0},
    {0x3677,0x84},
    {0x3678,0x88},
    {0x3679,0x8a},
    {0x367c,0x09},
    {0x367d,0x0b},
    {0x367e,0x08},
    {0x367f,0x0f},
    {0x3696,0x44},
    {0x3697,0x54},
    {0x3698,0x54},
    {0x36a0,0x0f},
    {0x36a1,0x1f},
    {0x36b0,0x81},
    {0x36b1,0x83},
    {0x36b2,0x85},
    {0x36b3,0x8b},
    {0x36b4,0x09},
    {0x36b5,0x0b},
    {0x36b6,0x0f},
    {0x370f,0x01},
    {0x3722,0x09},
    {0x3724,0x21},
    {0x3771,0x09},
    {0x3772,0x05},
    {0x3773,0x05},
    {0x377a,0x0f},
    {0x377b,0x1f},
    {0x3905,0x8c},
    {0x391d,0x04},
    {0x3926,0x21},
    {0x3933,0x80},
    {0x3934,0x03},
    {0x3935,0x00},
    {0x3936,0x1b},
    {0x3937,0x76},
    {0x3938,0x75},
    {0x3939,0x00},
    {0x393a,0x00},
    {0x39dc,0x02},
    {0x3e00,0x00},
    {0x3e01,0x5d},
    {0x3e02,0x40},
    {0x440e,0x02},
    {0x4509,0x28},
    {0x450d,0x32},
    {0x5000,0x06},
    {0x5799,0x46},
    {0x579a,0x77},
    {0x57d9,0x46},
    {0x57da,0x77},
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
    {0x36e9,0x44},
    {0x37f9,0x44},
    {0x0100,0x01},
    {0xffff,0x0a},
};

const static I2C_ARRAY mirror_reg[] = {
    {0x3221, 0x00}, // mirror[2:1], flip[6:5]
};

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const static I2C_ARRAY gain_reg[] = {
    {0x3e06, 0x00},
    {0x3e07, 0x80},
    {0x3e09, 0x00},
};

const static I2C_ARRAY expo_reg[] = {
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x5D}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
};

const static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x05},
    {0x320f, 0xDC}
};

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME SC4336

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////

//Sensor Sleep Mode
#define SENSOR_SLEEP_MODE_ENABLE      0
#define SENSOR_SLEEP_MODE_DISABLE     1
bool    g_bSensorFirstPn   = false;
bool    g_bSensorFirstInit = false;
int sleep_mode = 0;
module_param(sleep_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(sleep_mode, "sensor sleep mode");

static int _pCus_SleepMode(ss_cus_sensor* handle, u32 sleepModeEnable ,u32 idx)
{
    I2C_ARRAY sleep_reg = {0x0100, 0x00};
     switch(sleepModeEnable)
     {
         case 0:
             sleep_reg.data= 0x00;   //enable sleep mode
             SensorReg_Write(sleep_reg.reg, sleep_reg.data);
             break;
         case 1:
             sleep_reg.data= 0x01;   //disable sleep mode
             SensorReg_Write(sleep_reg.reg, sleep_reg.data);
             break;
         default:
             SENSOR_EMSG("do nothing ,Sleep Mode only On/Off !!!\n");
             break;
     }
     return 0;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    //ISP_config_io(handle);
    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    if(!g_bSensorFirstPn || !sleep_mode)
    {
        sensor_if->Reset(idx, SENSOR_RST_POL);
        SENSOR_USLEEP(1000);
        SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
        sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
        SENSOR_USLEEP(1000);

        //Sensor power on sequence
        sensor_if->MCLK(idx, 1, handle->mclk);

        sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
        sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
        sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

        // power -> high, reset -> high
        SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
        sensor_if->PowerOff(idx, !SENSOR_PWDN_POL);
        CamOsMsSleep(1);
        SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
        sensor_if->Reset(idx, !SENSOR_RST_POL);
        CamOsMsSleep(1);
        g_bSensorFirstPn = true;
    }
    else
    {
        sensor_if->MCLK(idx, 1, handle->mclk);
        sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
        sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
        CamOsMsSleep(1);
        _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_DISABLE, idx);
    }

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    if(!sleep_mode)
    {
        ISensorIfAPI *sensor_if = handle->sensor_if_api;
        SC4336_params *params = (SC4336_params *)handle->private_data;
        SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
        sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
        //handle->i2c_bus->i2c_close(handle->i2c_bus);
        CamOsMsSleep(1);
        //Set_csi_if(0, 0);
        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);

        sensor_if->MCLK(idx, 0, handle->mclk);

        params->cur_orien = CUS_ORIT_M0F0;
    }
    else
    {
        _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_ENABLE ,idx);
    }

    return SUCCESS;
}

static int SC4336_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    return SUCCESS;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init_mipi2lane_linear_4M30fps(ss_cus_sensor *handle)
{
    SC4336_params *params = (SC4336_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;
    if((!g_bSensorFirstInit) || !sleep_mode)
    {
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

        pCus_SetOrien(handle, params->cur_orien);
        params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
        g_bSensorFirstInit = true;
    }
    else
    {
        SENSOR_EMSG("Sleep Mode Just Need Init Once !!!\n");
    }
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
    SC4336_params *params = (SC4336_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"2560x1440@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_linear_4M30fps;
            vts_30fps=1500;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period  = 22222;
            break;

        default:
            break;
    }

    return SUCCESS;
}


static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    SC4336_params *params = (SC4336_params *)handle->private_data;
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
   SC4336_params *params = (SC4336_params *)handle->private_data;

    switch(orit) {
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
}

  SENSOR_DMSG("pCus_SetOrien:%x\r\n", orit);
  return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    SC4336_params *params = (SC4336_params *)handle->private_data;
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
    SC4336_params *params = (SC4336_params *)handle->private_data;
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

    if(params->expo.line > (params->expo.vts) - 8){
        vts = (params->expo.line + 8);
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

    SC4336_params *params = (SC4336_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        break;
        case CUS_FRAME_ACTIVE:
        if(params->orient_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
            params->orient_dirty = false;
        }
        if(params->reg_dirty)
        {
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, sizeof(expo_reg)/sizeof(I2C_ARRAY));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));
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
    SC4336_params *params = (SC4336_params *)handle->private_data;
    lines |= (u32)(params->tExpo_reg[0].data&0x0f)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xf0)<<0;
    lines >>= 4;
    *us = (lines*Preview_line_period)/1000; //return us

    SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
    return rc;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us) {
    int i;
    u32 expo_lines = 0,vts = 0;
    SC4336_params *params = (SC4336_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x46}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, expo_reg, sizeof(expo_reg));

    expo_lines = (1000*us)/Preview_line_period; // Preview_line_period in ns
    if(expo_lines <= 1)
        expo_lines=1;
    if (expo_lines > params->expo.vts-8) {
        vts = (expo_lines+8);
    }
    else
        vts=params->expo.vts;

    params->expo.line = expo_lines;
    SENSOR_DMSG("[%s] us %ld, expo_lines %ld, vts %ld\n", __FUNCTION__, us, expo_lines, params->expo.vts);

    expo_lines = expo_lines<<4;
    params->tExpo_reg[0].data = (expo_lines>>16) & 0x0f;
    params->tExpo_reg[1].data = (expo_lines>>8) & 0xff;
    params->tExpo_reg[2].data = (expo_lines>>0) & 0xf0;
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
    SC4336_params *params = (SC4336_params *)handle->private_data;
    u8 i=0 ;
    u64 gain_factor;
    u64 gain_reg_0x3e09,gain_reg_0x3e06,gain_reg_0x3e07;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e09, 0x00},
    };
    memcpy(gain_reg_temp, gain_reg, sizeof(gain_reg));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAXGAIN*1024) {
        gain = SENSOR_MAXGAIN*1024;
    }

    gain_factor = gain * 1000 / 1024;

    if(gain_factor < 1000) gain_factor = 1000;
    if(gain_factor > 480*1000) gain_factor =480*1000;

    if (gain_factor < 2000)
    {
        gain_reg_0x3e09 = 0x00;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = gain_factor * 128 / 1000;
    }
    else if( gain_factor < 4000)
    {
        gain_reg_0x3e09 = 0x08;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = gain_factor * 128 / 2000;
    }
    else if( gain_factor < 8000)
    {
        gain_reg_0x3e09 = 0x09;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = gain_factor * 128 / 4000;
    }
    else if( gain_factor < 16000)
    {
        gain_reg_0x3e09 = 0x0b;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = gain_factor * 128 / 8000;
    }
    else if( gain_factor < 32000)
    {
        gain_reg_0x3e09 = 0x0f;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = gain_factor * 128 / 16000;
    }
    else if( gain_factor < 32000*2)
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = gain_factor * 128 / 32000;
    }
    else if( gain_factor < 32000*4)//open dgain begin  max digital gain 4X
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x01;
        gain_reg_0x3e07 = gain_factor * 128 / 32000/2;
    }
    else if( gain_factor < 32000*8)
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x03;
        gain_reg_0x3e07 = gain_factor * 128 / 32000 / 4;
    }
     else if( gain_factor < 32000*15)
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x07;
        gain_reg_0x3e07 = gain_factor * 128 / 32000 / 8;
    }
    else
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x07;
        gain_reg_0x3e07 = 0xf0;
    }

    params->tGain_reg[2].data = gain_reg_0x3e09; // 0x3e09
    params->tGain_reg[1].data = gain_reg_0x3e07; // 0x3e07
    params->tGain_reg[0].data = gain_reg_0x3e06; // 0x3e06

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
    SC4336_params *params;
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
    params = (SC4336_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
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
    handle->video_res_supported.res[0].u16OutputWidth= 2560;
    handle->video_res_supported.res[0].u16OutputHeight= 1440;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2560x1440@30fps");

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum = 1;

    ///calibration
    //handle->sat_mingain=g_sensor_ae_min_gain;

    //handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_mipi2lane_linear_4M30fps;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal
    //handle->pCus_sensor_GetSensorID       = pCus_GetSensorID   ;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien      ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS      ;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS      ;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = SC4336_SetPatternMode;
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
    params->expo.line= 1500;
    params->reg_dirty = false;
    params->orient_dirty = false;
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX( SC4336,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            SC4336_params
                         );
