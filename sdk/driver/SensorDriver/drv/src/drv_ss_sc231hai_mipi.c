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

/* Sensor Porting on Master V5
   Porting owner :  
   Date :           9/26/2024
   Build on :       Master_V5
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC231HAI);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL


//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//MIPI config end.
//============================================

//#undef SENSOR_DBG
#define SENSOR_DBG 0

#define SENSOR_IFBUS_TYPE       CUS_SENIF_BUS_MIPI      //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC         CUS_DATAPRECISION_10    //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAMODE         CUS_SEN_10TO12_9000
#define SENSOR_BAYERID          CUS_BAYER_BG            //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID          CUS_RGBIR_NONE
#define SENSOR_ORIT             CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_MAXGAIN          (116550*15875)/1000000             // max sensor gain, a-gain*conversion-gain*d-gain
#define Preview_MCLK_SPEED      CUS_CMU_CLK_27MHZ       //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M

u32 Preview_line_period;
u32 vts_30fps;
#define Preview_WIDTH           1920                    //resolution Width when preview
#define Preview_HEIGHT          1080                    //resolution Height when preview
#define Preview_MAX_FPS         30  //25                //fastest preview FPS               
#define Preview_MIN_FPS         5                       //slowest preview FPS
#define Preview_CROP_START_X    0                       //CROP_START_X
#define Preview_CROP_START_Y    0                       //CROP_START_Y

#define SENSOR_I2C_ADDR         0x60                    //I2C slave address
#define SENSOR_I2C_SPEED        200000                  //I2C speed,60000~320000
#define SENSOR_I2C_LEGACY       I2C_NORMAL_MODE         //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT          I2C_FMT_A16D8           //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL         CUS_CLK_POL_NEG         // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL          CUS_CLK_POL_NEG         // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG            // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS         // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS         // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
////////////////////////////////////
// Image Info                     //
////////////////////////////////////
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
}SC231HAI_mipi_linear[] = {
    {LINEAR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},
 
};

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
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
        u32 line;
    } expo;
    struct {
        bool bVideoMode;
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tMirror_reg[1];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool orient_dirty;
    bool reg_dirty;
    bool vts_reg_dirty;
	CUS_CAMSENSOR_ORIT cur_orien;
} SC231HAI_params;

#if 0
// set sensor ID address and data,
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xCB},
    {0x3108, 0x3A},
};
#endif
const static I2C_ARRAY Sensor_init_table_2K30fps[] =
{
  //Cleaned_0x05_FT_sc231hai_MIPI_27Minput_2lane_371.25_10bit_1920x1080_30fps
	{0x0103,0x01},
	{0x36e9,0x80},
	{0x37f9,0x80},
	{0x301f,0x05},
	{0x3058,0x21},
	{0x3059,0x53},
	{0x305a,0x40},
	{0x3250,0x00},
	{0x3301,0x0a},
	{0x3302,0x20},
	{0x3304,0x90},
	{0x3305,0x00},
	{0x3306,0x68},
	{0x3309,0xd0},
	{0x330b,0xd8},
	{0x330d,0x08},
	{0x331c,0x04},
	{0x331e,0x81},
	{0x331f,0xc1},
	{0x3323,0x06},
	{0x3333,0x10},
	{0x3364,0x5e},
	{0x336c,0x8e},
	{0x337f,0x13},
	{0x3390,0x08},
	{0x3391,0x18},
	{0x3392,0xb8},
	{0x3393,0x0e},
	{0x3394,0x14},
	{0x3395,0x10},
	{0x3396,0x88},
	{0x3397,0x98},
	{0x3398,0xf8},
	{0x3399,0x0a},
	{0x339a,0x0e},
	{0x339b,0x10},
	{0x339c,0x3c},
	{0x33ae,0x80},
	{0x33af,0xc0},
	{0x33b2,0x50},
	{0x33b3,0x14},
	{0x33f8,0x00},
	{0x33f9,0x68},
	{0x33fa,0x00},
	{0x33fb,0x68},
	{0x33fc,0x48},
	{0x33fd,0x78},
	{0x349f,0x03},
	{0x34a6,0x40},
	{0x34a7,0x58},
	{0x34a8,0x10},
	{0x34a9,0x10},
	{0x34f8,0x78},
	{0x34f9,0x10},
	{0x3619,0x20},
	{0x361a,0x90},
	{0x3633,0x44},
	{0x3637,0x5c},
	{0x363c,0xc0},
	{0x363d,0x02},
	{0x3660,0x80},
	{0x3661,0x81},
	{0x3662,0x8f},
	{0x3663,0x81},
	{0x3664,0x81},
	{0x3665,0x82},
	{0x3666,0x8f},
	{0x3667,0x08},
	{0x3668,0x80},
	{0x3669,0x88},
	{0x366a,0x98},
	{0x366b,0xb8},
	{0x366c,0xf8},
	{0x3670,0xe2},
	{0x3671,0xe2},
	{0x3672,0x88},
	{0x3680,0x33},
	{0x3681,0x33},
	{0x3682,0x33},
	{0x36c0,0x80},
	{0x36c1,0x88},
	{0x36c8,0x88},
	{0x36c9,0xb8},
	{0x36ea,0x0b},
	{0x36eb,0x0c},
	{0x36ec,0x5c},
	{0x36ed,0x24},
	{0x3718,0x04},
	{0x3722,0x8b},
	{0x3724,0xd1},
	{0x3741,0x08},
	{0x3770,0x13},
	{0x3771,0x9b},
	{0x3772,0x9b},
	{0x37c0,0x88},
	{0x37c1,0xb8},
	{0x37fa,0x0b},
	{0x37fc,0x10},
	{0x37fd,0x24},
	{0x3902,0xc0},
	{0x3903,0x40},
	{0x3909,0x00},
	{0x391f,0x41},
	{0x3926,0xe0},
	{0x3933,0x80},
	{0x3934,0x02},
	{0x3937,0x6f},
	{0x3e00,0x00},
	{0x3e01,0x8b},
	{0x3e02,0xf0},
	{0x3e08,0x00},
	{0x4509,0x20},
	{0x450d,0x07},
	{0x4837,0x36},
	{0x5780,0x76},
	{0x5784,0x10},
	{0x5787,0x0a},
	{0x5788,0x0a},
	{0x5789,0x08},
	{0x578a,0x0a},
	{0x578b,0x0a},
	{0x578c,0x08},
	{0x578d,0x40},
	{0x5792,0x04},
	{0x5795,0x04},
	{0x57ac,0x00},
	{0x57ad,0x00},
	{0x36e9,0x20},
	{0x37f9,0x20},
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
    {0x3e08, 0x00|0x03},
    {0x3e09, 0x20}, //low bit, 0x40 - 0x7f, step 1/64
};

const static I2C_ARRAY expo_reg[] = {
    {0x3e00, 0x00}, //expo [20:17]
    {0x3e01, 0x46}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
};




const static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x04},
    {0x320f, 0x65},
};
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
     ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    sensor_if->PowerOff(idx,SENSOR_PWDN_POL );////pwd low
    sensor_if->Reset(idx, SENSOR_RST_POL);
    SENSOR_USLEEP(1000);
    //Sensor power on sequence

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
	sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_USLEEP(2000);
    sensor_if->Reset(idx, !SENSOR_RST_POL);
    SENSOR_USLEEP(1000);

    SENSOR_DMSG("[%s] pwd high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !SENSOR_PWDN_POL);
    SENSOR_USLEEP(2000);



    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
   // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
    sensor_if->Reset(idx,SENSOR_RST_POL );
    //handle->i2c_bus->i2c_close(handle->i2c_bus);
    CamOsMsSleep(1);
    //Set_csi_if(0, 0);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    
    sensor_if->MCLK(idx, 0, handle->mclk);

    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

static int SC231HAI_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
  return SUCCESS;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
//static int pCus_SetAEGain_cal(ss_cus_sensor *handle, u32 gain);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init_linear_2K30fps(ss_cus_sensor *handle)
{
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2K30fps);i++)
    {
        if(Sensor_init_table_2K30fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2K30fps[i].data);
        }else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2K30fps[i].reg, Sensor_init_table_2K30fps[i].data) != SUCCESS)
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
    return SUCCESS;
};
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
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"1920x1080@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_2K30fps;
            vts_30fps=1125;//1500
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            Preview_line_period  = 29630;
            break;
        default:
            break;
    }

    return SUCCESS;
}
static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
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
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;

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
    default :
        break;
    }

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
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
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
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

    if(params->expo.line > 2 * (params->expo.vts) -11){
        vts = (params->expo.line + 12)/2;
    }else{
        vts = params->expo.vts;
    }
    params->tVts_reg[0].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}



static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
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
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
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
    u32 half_lines = 0,vts = 0;
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x8b}, // expo[16:8]
    {0x3e02, 0xf0}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));

    half_lines = (1000*us*2)/Preview_line_period; // Preview_line_period in ns
    if(half_lines<=4) half_lines=4;
    if (half_lines >  2 * (params->expo.vts)-11) {
        vts = (half_lines+12)/2;
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

    for (i = 0; i < sizeof(expo_reg)/sizeof(I2C_ARRAY); i++)
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
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
    rc=params->expo.final_gain;
    return rc;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    SC231HAI_params *params = (SC231HAI_params *)handle->private_data;
   u8 i=0 , Coarse_gain = 1,DIG_gain=1;
    u32 Dcg_gainx100 = 1, ANA_Fine_gainx32 = 1;
    u8 Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x20,DIG_Fine_gain_reg=0x80;

	I2C_ARRAY gain_reg_temp[] = {
		{0x3e06, 0x00},
		{0x3e07, 0x80},
        {0x3e08, 0x00|0x03},
		{0x3e09, 0x20}, 
    };
    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAXGAIN*1024) {
        gain = SENSOR_MAXGAIN*1024;
    }

    if (gain < 2048) // start again  2 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x00; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
	else if (gain < 3789) // 3.7 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x01; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 7578) // 7.4 * 1024
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x80; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 15156) // 14.8 * 1024
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x81; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 30311)// 29.6 * 1024
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 4;     DIG_gain=1;
        Coarse_gain_reg = 0x83; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 60621)// 59.2 * 1024
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 8;     DIG_gain=1;
        Coarse_gain_reg = 0x87; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 121242)// 116.55 * 1024 // end again
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 16;     DIG_gain=1;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (gain < 121242 * 2) // start dgain
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 16;     DIG_gain=1;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x0;  ANA_Fine_gain_reg=0x3f;
    }
    else if (gain < 121242 * 4)
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 16;     DIG_gain=2;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x1;  ANA_Fine_gain_reg=0x3f;
    }
    else if (gain < 121242 * 8)
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 16;     DIG_gain=4;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x3;  ANA_Fine_gain_reg=0x3f;
    }
    else if (gain <= SENSOR_MAXGAIN*1024)
    {
        Dcg_gainx100 = 3700;      Coarse_gain = 16;     DIG_gain=8;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x7;  ANA_Fine_gain_reg=0x3f;
    }
#endif

    if(gain < 3700)
    {
        ANA_Fine_gain_reg = (u8)(1000 * gain / (Dcg_gainx100 * Coarse_gain) / 32);
    }
    else if(gain < 121241)
    {
        ANA_Fine_gain_reg = (u8)(1000ULL * gain / (Dcg_gainx100 * Coarse_gain) / 32);
    }else{
        DIG_Fine_gain_reg = (u8)(8000ULL * gain /(Dcg_gainx100 * Coarse_gain * DIG_gain) / ANA_Fine_gainx32);
    }
    params->expo.final_gain=gain;
    params->tGain_reg[3].data = ANA_Fine_gain_reg;
    params->tGain_reg[2].data = Coarse_gain_reg;
    params->tGain_reg[1].data = DIG_Fine_gain_reg;
    params->tGain_reg[0].data = DIG_gain_reg & 0xF;

    // printk("[%s]  params->tGain_reg : 0x%x ,0x%x ,0x%x, 0x%x\n\n", __FUNCTION__, 
	//	params->tGain_reg[2].data,params->tGain_reg[3].data,params->tGain_reg[0].data,params->tGain_reg[1].data);

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



static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    //SC231HAI_params *params;
	//params=(SC231HAI_params *)handle->private_data;
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
	info->u32AEShutter_min                   = Preview_line_period*2+999;
	info->u32AEShutter_step                  = Preview_line_period/2;
	info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
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
            SensorReg_Read(reg->reg, &reg->data);
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
    SC231HAI_params *params;
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
    params = (SC231HAI_params *)handle->private_data;
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
    handle->interface_attr.attr_mipi.mipi_lane_num    = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order   = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode    = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width        = SC231HAI_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height       = SC231HAI_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps      = SC231HAI_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps      = SC231HAI_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x = SC231HAI_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y = SC231HAI_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = SC231HAI_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = SC231HAI_mipi_linear[res].senout.height;
        strcpy(handle->video_res_supported.res[res].strResDesc, SC231HAI_mipi_linear[res].senstr.strResDesc);
    }

    // i2c
    handle->i2c_cfg.mode    = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt     = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed   = SENSOR_I2C_SPEED;     //320000;
    // mclk
    handle->mclk            = Preview_MCLK_SPEED;

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum = 1;
	handle->sensor_ae_info_cfg.u32AEGain_min                      = 1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAXGAIN*1024;
	handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period *2+999;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period/2 ;
    ///calibration
    handle->pCus_sensor_init           = pCus_init_linear_2K30fps;
    handle->pCus_sensor_poweron        = pCus_poweron ;
    handle->pCus_sensor_poweroff       = pCus_poweroff;
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes    = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes;
    handle->pCus_sensor_GetOrien       = pCus_GetOrien;
    handle->pCus_sensor_SetOrien       = pCus_SetOrien;
    handle->pCus_sensor_GetFPS         = pCus_GetFPS;
    handle->pCus_sensor_SetFPS         = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode = SC231HAI_SetPatternMode;

    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify     = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs         = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs         = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain          = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain          = pCus_SetAEGain;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;
	handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //sensor calibration
    params->expo.vts     =vts_30fps;
    params->expo.fps     = 30;
    params->expo.line    = 1000;
    params->reg_dirty    = false;
    params->orient_dirty = false;

    return SUCCESS;
}




SENSOR_DRV_ENTRY_IMPL_END_EX(SC231HAI,
                            cus_camsensor_init_handle,
							NULL,
							NULL,
                            SC231HAI_params
                         );

