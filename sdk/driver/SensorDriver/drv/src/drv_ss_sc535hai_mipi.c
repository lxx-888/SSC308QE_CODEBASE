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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(sc535hai);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3


//#undef SENSOR_DBG
#define SENSOR_DBG 0

#define SENSOR_ISP_TYPE     ISP_EXT                 //ISP_EXT, ISP_SOC
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI     //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000
#define SENSOR_BAYERID      CUS_BAYER_BG            //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG          //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_MAXGAIN      (79695*1575)/100000         // max sensor gain, a-gain*conversion-gain*d-gain
#define Preview_MCLK_SPEED  CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M

#define Preview_WIDTH       2688                   //resolution Width when preview
#define Preview_HEIGHT      1944                  //resolution Height when preview
#define Preview_MAX_FPS     20  //25                     //fastest preview FPS
#define Preview_MIN_FPS     5                       //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR    0x60                   //I2C slave address
#define SENSOR_I2C_SPEED    240000                  //I2C speed,60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

//#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
//#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

//#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG            // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS         // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_PCLK_POL     CUS_CLK_POL_POS         // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

static u32 Preview_line_period;
static u32 vts_linear;
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
}sc535iot_mipi_linear[] = {
    {LINEAR_RES_1, {2688, 1944, 3, 20}, {0, 0, 2688, 1944}, {"2688x1944@20fps"}},
};

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
static int cus_camsensor_release_handle(ss_cus_sensor *handle);
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
//#define ABS(a)   ((a)>(0) ? (a) : (-(a)))
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
        u32 final_us;
        u32 final_gain;
        u32 fps;
        u32 preview_fps;
        u32 line;
    } expo;
    struct {
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
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
    CUS_CAMSENSOR_ORIT cur_orien;
} sc535hai_params;
// set sensor ID address and data,

const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xce},
    {0x3108, 0x78},
};

const static I2C_ARRAY Sensor_init_table_5M30fps[] =
{//Cleaned_0x20_SC535HAI_raw_MIPI_27Minput_2Lane_10bit_900Mbps_2688x1944_30fps
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x37f9,0x80},
    {0x23b0,0x00},
    {0x23b1,0x08},
    {0x23b2,0x00},
    {0x23b3,0x18},
    {0x23b4,0x00},
    {0x23b5,0x38},
    {0x23b6,0x04},
    {0x23b7,0x08},
    {0x23b8,0x04},
    {0x23b9,0x18},
    {0x23ba,0x04},
    {0x23bb,0x38},
    {0x23c0,0x04},
    {0x23c1,0x00},
    {0x23c2,0x04},
    {0x23c3,0x18},
    {0x23c4,0x04},
    {0x23c5,0x78},
    {0x23c6,0x04},
    {0x23c7,0x08},
    {0x23c8,0x04},
    {0x23c9,0x78},
    {0x3018,0x3b},
    {0x3019,0x0c},
    {0x301e,0xf0},
    {0x301f,0x20},
    {0x302c,0x00},
    {0x30b8,0x44},
    {0x3200,0x00},
    {0x3201,0x00},
    {0x3202,0x00},
    {0x3203,0x00},
    {0x3204,0x0a},
    {0x3205,0x87},
    {0x3206,0x07},
    {0x3207,0x9f},
    {0x3208,0x0a},
    {0x3209,0x80},
    {0x320a,0x07},
    {0x320b,0x98},
    {0x320c,0x05},//HTS=1500*2=3000
    {0x320d,0xdc},
    {0x320e,0x0b},//VTS=2000 -> 3000 fps 30->20
    {0x320f,0xb8},
    {0x3210,0x00},
    {0x3211,0x04},
    {0x3212,0x00},
    {0x3213,0x04},
    {0x3214,0x11},
    {0x3215,0x11},
    {0x3223,0xc0},
    {0x3250,0x40},
    {0x327f,0x3f},
    {0x32e0,0x00},
    {0x3301,0x12},
    {0x3302,0x20},
    {0x3304,0xc0},
    {0x3306,0xb0},
    {0x3309,0xf0},
    {0x330a,0x01},
    {0x330b,0x70},
    {0x330d,0x10},
    {0x3310,0x18},
    {0x331e,0xb1},
    {0x331f,0xe1},
    {0x3333,0x10},
    {0x3334,0x40},
    {0x3364,0x56},
    {0x338f,0x80},
    {0x3393,0x1c},
    {0x3394,0x2c},
    {0x3395,0x3c},
    {0x3399,0x0c},
    {0x339a,0x10},
    {0x339b,0x18},
    {0x339c,0x80},
    {0x33ac,0x10},
    {0x33ad,0x2c},
    {0x33ae,0xb0},
    {0x33af,0xe0},
    {0x33b0,0x0f},
    {0x33b2,0x2c},
    {0x33b3,0x02},
    {0x349f,0x03},
    {0x34a8,0x02},
    {0x34a9,0x08},
    {0x34aa,0x01},
    {0x34ab,0x70},
    {0x34ac,0x01},
    {0x34ad,0x70},
    {0x34f9,0x12},
    {0x3631,0x0f},
    {0x3632,0x8d},
    {0x3633,0x4d},
    {0x363b,0x58},
    {0x363c,0xd8},
    {0x363d,0x20},
    {0x3641,0x08},
    {0x3670,0x32},
    {0x3671,0x34},
    {0x3672,0x26},
    {0x3673,0x04},
    {0x3674,0x08},
    {0x3675,0x04},
    {0x3676,0x18},
    {0x367e,0x49},
    {0x367f,0x49},
    {0x3680,0x49},
    {0x3681,0x04},
    {0x3682,0x08},
    {0x3683,0x04},
    {0x3684,0x38},
    {0x3685,0xc1},
    {0x3686,0xc2},
    {0x3687,0xc1},
    {0x3688,0xc1},
    {0x3689,0xc1},
    {0x368a,0xc1},
    {0x368b,0xc4},
    {0x368c,0xc1},
    {0x368d,0x00},
    {0x368e,0x08},
    {0x368f,0x00},
    {0x3690,0x18},
    {0x3691,0x04},
    {0x3692,0x00},
    {0x3693,0x04},
    {0x3694,0x08},
    {0x3695,0x04},
    {0x3696,0x18},
    {0x3697,0x04},
    {0x3698,0x38},
    {0x3699,0x04},
    {0x369a,0x78},
    {0x36d0,0x0d},
    {0x36ea,0x0a},
    {0x36eb,0x0c},
    {0x36ec,0x43},
    {0x36ed,0xaa},
    {0x370f,0x13},
    {0x3721,0x6c},
    {0x3722,0x8b},
    {0x3724,0xd1},
    {0x3729,0x34},
    {0x37b0,0x17},
    {0x37b1,0x17},
    {0x37b2,0x13},
    {0x37b3,0x04},
    {0x37b4,0x08},
    {0x37b5,0x04},
    {0x37b6,0x38},
    {0x37b7,0x1d},
    {0x37b8,0x1f},
    {0x37b9,0x1f},
    {0x37ba,0x04},
    {0x37bb,0x04},
    {0x37bc,0x04},
    {0x37bd,0x04},
    {0x37be,0x08},
    {0x37bf,0x04},
    {0x37c0,0x38},
    {0x37c1,0x04},
    {0x37c2,0x08},
    {0x37c3,0x04},
    {0x37c4,0x38},
    {0x37fa,0x0a},
    {0x37fb,0x22},
    {0x37fc,0x30},
    {0x37fd,0x16},
    {0x3900,0x05},
    {0x3901,0x00},
    {0x3902,0xc0},
    {0x3903,0x40},
    {0x3905,0x2d},
    {0x391a,0x72},
    {0x391b,0x39},
    {0x391c,0x22},
    {0x391d,0x00},
    {0x391f,0x41},
    {0x3926,0xe0},
    {0x3933,0x80},
    {0x3934,0x03},
    {0x3935,0x01},
    {0x3936,0xc0},
    {0x3937,0x6a},
    {0x3938,0x6b},
    {0x3939,0x0f},
    {0x393a,0xf6},
    {0x393d,0x05},
    {0x393e,0x50},
    {0x39dd,0x00},
    {0x39de,0x06},
    {0x39e7,0x04},
    {0x39e8,0x04},
    {0x39e9,0x80},
    {0x3e00,0x00},
    {0x3e01,0x7c},
    {0x3e02,0x80},
    {0x3e03,0x0b},
    {0x3e16,0x01},
    {0x3e17,0x44},
    {0x3e18,0x01},
    {0x3e19,0x44},
    {0x440e,0x02},
    {0x4509,0x18},
    {0x450d,0x07},
    {0x4800,0x24},//non_continuous mode
    {0x480f,0x03},
    {0x5000,0x06},
    {0x5780,0x76},
    {0x5784,0x10},
    {0x5785,0x08},
    {0x5787,0x16},
    {0x5788,0x16},
    {0x5789,0x15},
    {0x578a,0x16},
    {0x578b,0x16},
    {0x578c,0x15},
    {0x578d,0x41},
    {0x5790,0x11},
    {0x5791,0x0f},
    {0x5792,0x0f},
    {0x5793,0x11},
    {0x5794,0x0f},
    {0x5795,0x0f},
    {0x5799,0x46},
    {0x579a,0x77},
    {0x57a1,0x04},
    {0x57a8,0xd2},
    {0x57aa,0x2a},
    {0x57ab,0x7f},
    {0x57ac,0x00},
    {0x57ad,0x00},
    {0x36e9,0x44},
    {0x37f9,0x04},
    {0x0100,0x01},
};

static I2C_ARRAY mirror_reg[] =
{
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
    {0x3e09, 0x20}, //low bit, 0x40 - 0x7f, step 1/64
};

static I2C_ARRAY expo_reg[] = {
    {0x3e00, 0x00}, //expo [20:17]
    {0x3e01, 0x7c}, // expo[16:8]
    {0x3e02, 0x80}, // expo[7:0], [3:0] fraction of line
};

static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x07},
    {0x320f, 0xd0},
};

static I2C_ARRAY PatternTbl[] = {
    {0x4501,0xa4}, //testpattern , bit 3 to enable
};

/*
/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) printf(args)
#elif SENSOR_DBG == 0
#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME sc535hai
*/
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

//int cus_camsensor_release_handle(ss_cus_sensor *handle);

/////////////////// sensor hardware dependent //////////////
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);////pwd low
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    //Sensor power on sequence

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_USLEEP(2000);
    sensor_if->Reset(idx, !CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);

    SENSOR_DMSG("[%s] pwd high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !CUS_CLK_POL_NEG);
    SENSOR_USLEEP(2000);

    //sensor_if->Set3ATaskOrder(handle, def_order);
    // pure power on
    //ISP_config_io(handle);
   // sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
   // SENSOR_USLEEP(5000);
    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    //handle->i2c_bus->i2c_close(handle->i2c_bus);
    SENSOR_USLEEP(1000);
    //Set_csi_if(0, 0);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);

    params->cur_orien = CUS_ORIT_M0F0;

    return SUCCESS;
}

/////////////////// image function /////////////////////////
//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
static int pCus_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
    int i,n;
    int table_length= ARRAY_SIZE(Sensor_id_table);
    I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    for(n=0;n<table_length;++n) {
      id_from_sensor[n].reg = Sensor_id_table[n].reg;
      id_from_sensor[n].data = 0;
    }

    *id =0;
    if(table_length>8) table_length=8;
    for(n=0; n<4; ++n) {              //retry , until I2C success
      if(n>2) return FAIL;
      if(SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
          break;
      else
          SENSOR_USLEEP(1000);
    }

    for(i=0; i<table_length; ++i) {
    if (id_from_sensor[i].data != Sensor_id_table[i].data)
            return FAIL;
    *id = ((*id)+ id_from_sensor[i].data)<<8;
  }
  *id >>= 8;
    SENSOR_DMSG("[%s]Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);

    return SUCCESS;
}

static int sc535iot_SetPatternMode(ss_cus_sensor *handle,u32 mode)
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
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init_linear_5M30fps(ss_cus_sensor *handle)
{
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;
    // u16 reg3109, reg3109_1, reg3040, reg3040_l;
    //sensor_if->PCLK(NULL,CUS_PCLK_MIPI_TOP);
    u16 Reg_sensor_id;
    CamOsMsSleep(1);
    SensorReg_Read(0x8037,&Reg_sensor_id);
    if (Reg_sensor_id == 0x00)
    {
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
    }
    else
            {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
            }
    pCus_SetOrien(handle, params->cur_orien);
   // pr_info("cur_orien %s pCus_SetOrien %x\n",__FUNCTION__, params->cur_orien);
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
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"2688x1944@20fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear_5M30fps;
            vts_linear = 3000;//1500
            params->expo.vts = vts_linear;
            params->expo.fps = 20;
            Preview_line_period  = 16667;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
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
    sc535hai_params *params = (sc535hai_params *)handle->private_data;

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
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_linear*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_linear*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts=0;
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_linear*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_linear*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.line > (params->expo.vts) -8){
        vts = (params->expo.line + 8);
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
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
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
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
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
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x7c}, // expo[16:8]
    {0x3e02, 0x80}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, expo_reg, sizeof(expo_reg));

    expo_lines = (1000*us)/Preview_line_period; // Preview_line_period in ns
    if(expo_lines <= 2) //1
        expo_lines=2;
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
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
    int rc = 0;
    *gain = params->expo.final_gain;
    return rc;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    sc535hai_params *params = (sc535hai_params *)handle->private_data;
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

    params->expo.final_gain = gain;

    if (gain < 2048) // start again  2 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x00; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 2591) // 2.53 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x01; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 5182) // 5.06 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x80; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 10363) // 10.12 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x81; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 20726)// 20.24 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 4;     DIG_gain=1;
        Coarse_gain_reg = 0x83; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 41452)// 40.48 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 8;     DIG_gain=1;
        Coarse_gain_reg = 0x87; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain < 82904)// 80.96 * 1024 // end again
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=1;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (gain < 164511 ) // start dgain
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=1;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x0;  ANA_Fine_gain_reg=0x3f;
    }
    else if (gain < 164511 * 2)
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=2;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x1;  ANA_Fine_gain_reg=0x3f;
    }
    else if (gain < (164511 * 4 - 1))
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=4;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x3;  ANA_Fine_gain_reg=0x3f;
    }
    else if (gain <= SENSOR_MAXGAIN*1024)
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=8;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x7;  ANA_Fine_gain_reg=0x3f;
    }
#endif

    if(gain < 2530)
    {
        ANA_Fine_gain_reg = (1000ULL * gain / (Dcg_gainx100 * Coarse_gain) / 32);
    }
    else if(gain < 82903)
    {
        ANA_Fine_gain_reg = (1000ULL * gain / (Dcg_gainx100 * Coarse_gain) / 32);
    }else{
        DIG_Fine_gain_reg = (8000ULL * gain /(Dcg_gainx100 * Coarse_gain * DIG_gain) / ANA_Fine_gainx32 / 4 * 4);
    }

    params->tGain_reg[3].data = ANA_Fine_gain_reg;
    params->tGain_reg[2].data = Coarse_gain_reg;
    params->tGain_reg[1].data = DIG_Fine_gain_reg;
    params->tGain_reg[0].data = DIG_gain_reg & 0xF;
    // printk("[%s]  params->tGain_reg : 0x%x ,0x%x ,0x%x, 0x%x\n\n", __FUNCTION__,
    //  params->tGain_reg[2].data,params->tGain_reg[3].data,params->tGain_reg[0].data,params->tGain_reg[1].data);

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
    {/*
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
        }*/
    }
    return SUCCESS;
}

int cus_camsensor_sc535hai__init_handle(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    sc535hai_params *params;
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
    params = (sc535hai_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    SENSOR_DMSG(handle->model_id,"sc535iot_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    params->res.orit      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = sc535iot_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sc535iot_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sc535iot_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sc535iot_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sc535iot_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sc535iot_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sc535iot_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sc535iot_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc535iot_mipi_linear[res].senstr.strResDesc);
    }

    // i2c

    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    //polarity
    /////////////////////////////////////////////////////
    //handle->pwdn_POLARITY               = CUS_CLK_POL_NEG;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = CUS_CLK_POL_NEG;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    //handle->ae_gain_delay       = 2;
    //handle->ae_shutter_delay    = 2;
    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 1;

    ///calibration
    //handle->sat_mingain = g_sensor_ae_min_gain;


    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_linear_5M30fps;

    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID   ;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien      ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS      ;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS      ;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = sc535iot_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = 1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAXGAIN * 1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (Preview_line_period * 2 + 999);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

     //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal; // NULL
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    //handle->pCus_sensor_GetShutterInfo = sc535iot_GetShutterInfo;
    params->expo.vts=vts_linear;
    params->expo.fps = 30;
    params->expo.line= 2000;
    params->reg_dirty = false;
    params->orient_dirty = false;

    //handle->snr_pad_group = SENSOR_PAD_GROUP_SET;

    return SUCCESS;
}

static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX( SC535HAI,
                            cus_camsensor_sc535hai__init_handle,
                            NULL,
                            NULL,
                            sc535hai_params
                         );

