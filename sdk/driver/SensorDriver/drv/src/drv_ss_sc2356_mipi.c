/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(sc2356);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL

//============================================
#define SENSOR_MIPI_LANE_NUM (1)
//============================================

#define SENSOR_DBG 0

#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI

#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_MAXGAIN      (16000*3938 / 1000000)
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define Preview_MCLK_SPEED  CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M

#define Preview_WIDTH       1600                    //resolution Width when preview
#define Preview_HEIGHT      1200                    //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     5                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR    0x6c                   //I2C slave address
#define SENSOR_I2C_SPEED   200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

static u32 Preview_line_period = 26667;
static u32 vts_linear = 1250;

static struct {     // LINEAR
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2 = 1, LINEAR_RES_END}mode;
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    struct _sensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    struct _senstr{
        const char* strResDesc;
    }senstr;
}sc202cs_mipi_linear[] = {
    {LINEAR_RES_1, {1600, 1200, 5, 20}, {0, 0, 1600, 1200}, {"1600x1200@20fps"}},
    {LINEAR_RES_2, {1600,  900, 5, 20}, {0, 0, 1600,  900}, {"1600x900@20fps"}},
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
        u32 line_freq;
        u32 us_per_line;
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
} sc2356_params;

typedef struct {
    u64 gain;
    u8 fine_gain_reg;
} FINE_GAIN;

static I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xeb},
    {0x3108, 0x52},
};

static I2C_ARRAY Sensor_init_table_2M30fps_4_3[] =
{
    //cleaned_0x1b_SC202CS_27Minput_720Mbps_1lane_10bit_1600x1200_30fps
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36ea,0x0a},
    {0x36eb,0x0c},
    {0x36ec,0x01},
    {0x36ed,0x18},
    {0x36e9,0x10},
    {0x301f,0x1b},
    {0x320e,0x07},//set to 20fps ;0x%x4e2
    {0x320f,0x53},//
    {0x3301,0xff},
    {0x3304,0x68},
    {0x3306,0x40},
    {0x3308,0x08},
    {0x3309,0xa8},
    {0x330b,0xb0},
    {0x330c,0x18},
    {0x330d,0xff},
    {0x330e,0x20},
    {0x331e,0x59},
    {0x331f,0x99},
    {0x3333,0x10},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x1f},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x338f,0xa0},
    {0x3390,0x01},
    {0x3391,0x03},
    {0x3392,0x1f},
    {0x3393,0xff},
    {0x3394,0xff},
    {0x3395,0xff},
    {0x33a2,0x04},
    {0x33ad,0x0c},
    {0x33b1,0x20},
    {0x33b3,0x38},
    {0x33f9,0x40},
    {0x33fb,0x48},
    {0x33fc,0x0f},
    {0x33fd,0x1f},
    {0x349f,0x03},
    {0x34a6,0x03},
    {0x34a7,0x1f},
    {0x34a8,0x38},
    {0x34a9,0x30},
    {0x34ab,0xb0},
    {0x34ad,0xb0},
    {0x34f8,0x1f},
    {0x34f9,0x20},
    {0x3630,0xa0},
    {0x3631,0x92},
    {0x3632,0x64},
    {0x3633,0x43},
    {0x3637,0x49},
    {0x363a,0x85},
    {0x363c,0x0f},
    {0x3650,0x31},
    {0x3670,0x0d},
    {0x3674,0xc0},
    {0x3675,0xa0},
    {0x3676,0xa0},
    {0x3677,0x92},
    {0x3678,0x96},
    {0x3679,0x9a},
    {0x367c,0x03},
    {0x367d,0x0f},
    {0x367e,0x01},
    {0x367f,0x0f},
    {0x3698,0x83},
    {0x3699,0x86},
    {0x369a,0x8c},
    {0x369b,0x94},
    {0x36a2,0x01},
    {0x36a3,0x03},
    {0x36a4,0x07},
    {0x36ae,0x0f},
    {0x36af,0x1f},
    {0x36bd,0x22},
    {0x36be,0x22},
    {0x36bf,0x22},
    {0x36d0,0x01},
    {0x370f,0x02},
    {0x3721,0x6c},
    {0x3722,0x8d},
    {0x3725,0xc5},
    {0x3727,0x14},
    {0x3728,0x04},
    {0x37b7,0x04},
    {0x37b8,0x04},
    {0x37b9,0x06},
    {0x37bd,0x07},
    {0x37be,0x0f},
    {0x3901,0x02},
    {0x3903,0x40},
    {0x3905,0x8d},
    {0x3907,0x00},
    {0x3908,0x41},
    {0x391f,0x41},
    {0x3933,0x80},
    {0x3934,0x02},
    {0x3937,0x6f},
    {0x393a,0x01},
    {0x393d,0x01},
    {0x393e,0xc0},
    {0x39dd,0x41},
    {0x3e00,0x00},
    {0x3e01,0x4d},
    {0x3e02,0xc0},
    {0x3e09,0x00},
    {0x4509,0x28},
    {0x450d,0x61},
    {0x0100,0x01},
};

static I2C_ARRAY Sensor_init_table_2M30fps_16_9[] =
{//cleaned_0x1b_SC202CS_27Minput_720Mbps_1lane_10bit_1600x900_30fps
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36ea,0x0a},
    {0x36eb,0x0c},
    {0x36ec,0x01},
    {0x36ed,0x18},
    {0x36e9,0x10},
    {0x301f,0x1b},
//1600X900
    {0x3200,0x00},
    {0x3201,0x00},
    {0x3202,0x00},
    {0x3203,0x96},
    {0x3204,0x06},
    {0x3205,0x47},
    {0x3206,0x04},
    {0x3207,0x21},
    {0x3208,0x06},
    {0x3209,0x40},
    {0x320a,0x03},
    {0x320b,0x84},
    {0x320e,0x07},//set to 20fps ;0x%x4e2
    {0x320f,0x53},//
    {0x3210,0x00},
    {0x3211,0x04},
    {0x3212,0x00},
    {0x3213,0x04},
    {0x3301,0xff},
    {0x3304,0x68},
    {0x3306,0x40},
    {0x3308,0x08},
    {0x3309,0xa8},
    {0x330b,0xb0},
    {0x330c,0x18},
    {0x330d,0xff},
    {0x330e,0x20},
    {0x331e,0x59},
    {0x331f,0x99},
    {0x3333,0x10},
    {0x335e,0x06},
    {0x335f,0x08},
    {0x3364,0x1f},
    {0x337c,0x02},
    {0x337d,0x0a},
    {0x338f,0xa0},
    {0x3390,0x01},
    {0x3391,0x03},
    {0x3392,0x1f},
    {0x3393,0xff},
    {0x3394,0xff},
    {0x3395,0xff},
    {0x33a2,0x04},
    {0x33ad,0x0c},
    {0x33b1,0x20},
    {0x33b3,0x38},
    {0x33f9,0x40},
    {0x33fb,0x48},
    {0x33fc,0x0f},
    {0x33fd,0x1f},
    {0x349f,0x03},
    {0x34a6,0x03},
    {0x34a7,0x1f},
    {0x34a8,0x38},
    {0x34a9,0x30},
    {0x34ab,0xb0},
    {0x34ad,0xb0},
    {0x34f8,0x1f},
    {0x34f9,0x20},
    {0x3630,0xa0},
    {0x3631,0x92},
    {0x3632,0x64},
    {0x3633,0x43},
    {0x3637,0x49},
    {0x363a,0x85},
    {0x363c,0x0f},
    {0x3650,0x31},
    {0x3670,0x0d},
    {0x3674,0xc0},
    {0x3675,0xa0},
    {0x3676,0xa0},
    {0x3677,0x92},
    {0x3678,0x96},
    {0x3679,0x9a},
    {0x367c,0x03},
    {0x367d,0x0f},
    {0x367e,0x01},
    {0x367f,0x0f},
    {0x3698,0x83},
    {0x3699,0x86},
    {0x369a,0x8c},
    {0x369b,0x94},
    {0x36a2,0x01},
    {0x36a3,0x03},
    {0x36a4,0x07},
    {0x36ae,0x0f},
    {0x36af,0x1f},
    {0x36bd,0x22},
    {0x36be,0x22},
    {0x36bf,0x22},
    {0x36d0,0x01},
    {0x370f,0x02},
    {0x3721,0x6c},
    {0x3722,0x8d},
    {0x3725,0xc5},
    {0x3727,0x14},
    {0x3728,0x04},
    {0x37b7,0x04},
    {0x37b8,0x04},
    {0x37b9,0x06},
    {0x37bd,0x07},
    {0x37be,0x0f},
    {0x3901,0x02},
    {0x3903,0x40},
    {0x3905,0x8d},
    {0x3907,0x00},
    {0x3908,0x41},
    {0x391f,0x41},
    {0x3933,0x80},
    {0x3934,0x02},
    {0x3937,0x6f},
    {0x393a,0x01},
    {0x393d,0x01},
    {0x393e,0xc0},
    {0x39dd,0x41},
    {0x3e00,0x00},
    {0x3e01,0x4d},
    {0x3e02,0xc0},
    {0x3e09,0x00},
    {0x4509,0x28},
    {0x450d,0x61},
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
    {0x3e09, 0x00}, //low bit, 0x10 - 0x3e0, step 1/64
};

static I2C_ARRAY expo_reg[] = {  // max expo line vts*2-6!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x46}, // expo[16:8]
    {0x3e02, 0x00}, // expo[7:0], [3:0] fraction of line
};

static I2C_ARRAY vts_reg[] = {
    {0x320e, 0x07},
    {0x320f, 0x53},
};

static I2C_ARRAY PatternTbl[] = {
    {0x4501, 0xa4}, //testpattern , bit 3 to enable
};

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
#endif
#undef SENSOR_NAME
#define SENSOR_NAME sc2356

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    sensor_if->PowerOff(idx, SENSOR_PWDN_POL);////pwd low
    SENSOR_USLEEP(1000);
    sensor_if->Reset(idx, SENSOR_RST_POL);
    SENSOR_USLEEP(1000);
    //Sensor power on sequence
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !SENSOR_PWDN_POL);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !SENSOR_RST_POL);
    SENSOR_USLEEP(1000);
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sc2356_params *params = (sc2356_params *)handle->private_data;
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
    sensor_if->Reset(idx,SENSOR_RST_POL );
    CamOsMsSleep(1);
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

        if( SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
            break;
         else
            continue;
    }

    for(i=0;i<table_length;++i)
    {
        if( id_from_sensor[i].data != Sensor_id_table[i].data )
            return FAIL;
        *id = ((*id)+ id_from_sensor[i].data)<<8;
    }

    *id >>= 8;
    SENSOR_DMSG("[%s]sc2356 Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);

    return SUCCESS;
}

static int sc2356_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    int i;
    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    switch(mode) {
    case 1:
        PatternTbl[0].data = 0xac; //enable
        break;
    case 0:
        PatternTbl[0].data = 0xa4; //disable
        break;
    default:
        PatternTbl[0].data = 0xa4; //disable
        break;
    }
    for(i=0; i< ARRAY_SIZE(PatternTbl); i++) {
        if(SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
            return FAIL;
    }

    return SUCCESS;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
//static int pCus_SetAEGain_cal(ss_cus_sensor *handle, u32 gain);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init_mipi1lane_linear_2M30fps_4_3(ss_cus_sensor *handle)
{
    sc2356_params *params = (sc2356_params *)handle->private_data;
    int i,cnt;
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2M30fps_4_3);i++)
    {
        if(Sensor_init_table_2M30fps_4_3[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2M30fps_4_3[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2M30fps_4_3[i].reg, Sensor_init_table_2M30fps_4_3[i].data) != SUCCESS)
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
}

static int pCus_init_mipi1lane_linear_2M30fps_16_9(ss_cus_sensor *handle)
{
    sc2356_params *params = (sc2356_params *)handle->private_data;
    int i,cnt;
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2M30fps_16_9);i++)
    {
        if(Sensor_init_table_2M30fps_16_9[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2M30fps_16_9[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2M30fps_16_9[i].reg, Sensor_init_table_2M30fps_16_9[i].data) != SUCCESS)
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
    sc2356_params *params = (sc2356_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"1600x1200@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi1lane_linear_2M30fps_4_3;
            vts_linear = 1875;
            params->expo.vts = vts_linear;
            params->expo.fps = 20;
            Preview_line_period  = 26667;
            break;
        case 1: //"1600x900@30fps"
            handle->video_res_supported.ulcur_res = 1;
            handle->pCus_sensor_init = pCus_init_mipi1lane_linear_2M30fps_16_9;
            vts_linear = 1875;
            params->expo.vts = vts_linear;
            params->expo.fps = 20;
            Preview_line_period  = 26667;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    sc2356_params *params = (sc2356_params *)handle->private_data;
    char sen_data;
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
    sc2356_params *params = (sc2356_params *)handle->private_data;

    switch(orit) {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[0].data = 0x00;
            params->orient_dirty = true;
            break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[0].data = 0x06;
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
    sc2356_params *params = (sc2356_params *)handle->private_data;
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
    sc2356_params *params = (sc2356_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_linear*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts = (vts_linear*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.line > (params->expo.vts) - 6){
        vts = (params->expo.line + 6);
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
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
   sc2356_params *params = (sc2356_params *)handle->private_data;
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
    sc2356_params *params = (sc2356_params *)handle->private_data;
    int rc=0;
    u32 lines = 0;
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
    sc2356_params *params = (sc2356_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
        {0x3e00, 0x00},//expo [20:17]
        {0x3e01, 0x4d}, // expo[16:8]
        {0x3e02, 0xc0}, // expo[7:4], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));

    expo_lines = (1000*us)/Preview_line_period; // Preview_line_period in ns
    if(expo_lines <= 2)
        expo_lines = 2;
    if (expo_lines >   (params->expo.vts) - 6) {
        vts = (expo_lines + 6);
    } else {
        vts=params->expo.vts;
    }
    params->expo.line = expo_lines;
    SENSOR_DMSG("[%s] us %ld, expo_lines %ld, vts %ld\n", __FUNCTION__, us, expo_lines, params->expo.vts);

    expo_lines = expo_lines<<4;

    params->tExpo_reg[0].data = (expo_lines>>16) & 0x0f;
    params->tExpo_reg[1].data =  (expo_lines>>8) & 0xff;
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
    int rc = 0;
    sc2356_params *params = (sc2356_params *)handle->private_data;
    *gain = params->expo.final_gain;
    return rc;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    sc2356_params *params = (sc2356_params *)handle->private_data;
    u8 i=0 ;// , Coarse_gain = 1,DIG_gain=1;
    u32 gain_factor = 0, gain_reg_0x3e09 = 0x00, gain_reg_0x3e06 = 0x00, gain_reg_0x3e07 = 0x80;

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
    gain_factor = gain * 1000 / 1024;

    if(gain_factor < 1000) gain_factor = 1000;
    if(gain_factor > 64*1000) gain_factor =64*1000;

     if (gain_factor < 2000)
        {
            gain_reg_0x3e09 = 0x00;
            gain_reg_0x3e06 = 0x00;
            gain_reg_0x3e07 = (gain_factor * 128 / 1000 / 4) * 4;
        }
     else if( gain_factor < 4000)
        {
        gain_reg_0x3e09 = 0x01;
            gain_reg_0x3e06 = 0x00;
            gain_reg_0x3e07 = (gain_factor * 128 / 2000 / 4) * 4;
        }
     else if( gain_factor < 8000)
        {
        gain_reg_0x3e09 = 0x03;
            gain_reg_0x3e06 = 0x00;
            gain_reg_0x3e07 = (gain_factor * 128 / 4000 / 4) * 4 ;
        }
     else if( gain_factor < 16000)
        {
        gain_reg_0x3e09 = 0x07;
            gain_reg_0x3e06 = 0x00;
            gain_reg_0x3e07 = (gain_factor * 128 / 8000 / 4) * 4 ;
        }
    else if( gain_factor < 16000*2)//open dgain begin  max digital gain 4X
        {
            gain_reg_0x3e09 = 0x0f;
            gain_reg_0x3e06 = 0x00;
            gain_reg_0x3e07 = (gain_factor * 128 / 16000 / 4) * 4 ;
        }
    else if( gain_factor < SENSOR_MAXGAIN*1024)
    {
        gain_reg_0x3e09 = 0x0f;
        gain_reg_0x3e06 = 0x01;
        gain_reg_0x3e07 = (gain_factor * 128 / 16000 / 2 / 4) * 4 ;
    }
      else
     {
         gain_reg_0x3e09 = 0x0f;
         gain_reg_0x3e06 = 0x01;
         gain_reg_0x3e07 = 0xfc;
     }
    params->expo.final_gain=gain;
    params->tGain_reg[2].data = gain_reg_0x3e09;        // 3e09
    params->tGain_reg[1].data = gain_reg_0x3e07;        // 3e07
    params->tGain_reg[0].data = gain_reg_0x3e06;        // 3e06

    for (i = 0; i < sizeof(gain_reg)/sizeof(I2C_ARRAY); i++)
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
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = handle->sensor_ae_info_cfg.u32AEShutter_min;
    info->u32AEShutter_step                  = handle->sensor_ae_info_cfg.u32AEShutter_step;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

int cus_camsensor_sc2356_init_handle_linear(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    sc2356_params *params;
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
    params = (sc2356_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
//    SENSOR_DMSG(handle->model_id,"sc202cs_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
//    handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
//    handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
 //   handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
//   handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width        = sc202cs_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height       = sc202cs_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps      = sc202cs_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps      = sc202cs_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x = sc202cs_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y = sc202cs_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sc202cs_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sc202cs_mipi_linear[res].senout.height;
        strcpy(handle->video_res_supported.res[res].strResDesc, sc202cs_mipi_linear[res].senstr.strResDesc);
    }

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

    handle->sensor_ae_info_cfg.u32AEGain_min                      = 1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAXGAIN * 1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 1 + 999;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

//    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_mipi1lane_linear_2M30fps_4_3;

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
    handle->pCus_sensor_SetPatternMode = sc2356_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;

    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;


    params->expo.vts=vts_linear;
    params->expo.fps = 30;
    params->expo.line = 26667;
    params->reg_dirty = false;
    params->orient_dirty = false;
    return SUCCESS;
}


SENSOR_DRV_ENTRY_IMPL_END_EX( SC2356,
                            cus_camsensor_sc2356_init_handle_linear,
                            NULL,
                            NULL,
                            sc2356_params
                         );
