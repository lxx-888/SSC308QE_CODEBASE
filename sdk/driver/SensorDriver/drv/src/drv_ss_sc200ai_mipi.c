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
   Porting owner :Jilly
   Date          :23/09/19
   Build on      :Master_V4 i6c
   Verified on   :not yet
   Remark        :NA
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SC200ai);
#if defined(CAM_OS_RTK)
int sleep_mode = 0;
#elif defined(CAM_OS_LINUX_KERNEL)
int sleep_mode = 0;
module_param(sleep_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(sleep_mode, "sensor sleep mode 1:sleep reg, 2:pdn");
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//MIPI config end.
//============================================

//#undef SENSOR_DBG
#define SENSOR_DBG 0
#define SENSOR_HDR_MODE                       CUS_HDR_MODE_VC
//#define SENSOR_ISP_TYPE          ISP_EXT                   //ISP_EXT, ISP_SOC
#define SENSOR_IFBUS_TYPE        CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE   PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC          CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_HDR      CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE          CUS_SEN_10TO12_9000     //CFG

#define SENSOR_BAYERID           CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR       CUS_BAYER_BG//CUS_BAYER_GR
#define SENSOR_RGBIRID           CUS_RGBIR_NONE
#define SENSOR_ORIT              CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define Preview_MCLK_SPEED       CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR   CUS_CMU_CLK_27MHZ

#define Preview_line_period      29630
#define vts_30fps                1125//29630
#define Preview_line_period_HDR  14815             // 1000000000 / 30 / 1125 / 2 = 14815
#define vts_30fps_HDR            2250
#define Preview_MAX_FPS          30                     //fastest preview FPS
#define Preview_MIN_FPS          3                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR          0x60                   //I2C slave address
#define SENSOR_I2C_SPEED         200000                 //I2C speed, 60000~320000
#define SENSOR_I2C_LEGACY        I2C_NORMAL_MODE        //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT           I2C_FMT_A16D8          //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

//#define SENSOR_PWDN_POL          CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
//#define SENSOR_RST_POL           CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
//#define SENSOR_VSYNC_POL         CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_HSYNC_POL         CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_PCLK_POL          CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

//AE info

#define SENSOR_MAX_GAIN                       (53975*3175 / 100000)   //sensor again 15.875 dgain=31.5
#define SENSOR_MIN_GAIN                       (1)
#define SENSOR_GAIN_DELAY_FRAME_COUNT         (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_GAIN_CTRL_NUM                  (1)
#define SENSOR_SHUTTER_CTRL_NUM               (1)
#define SENSOR_SHUTTER_CTRL_NUM_HDR           (2)


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
}sc200ai_mipi_linear[] = {
    {LINEAR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},};

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
}sc200ai_mipi_hdr[] = {
   {HDR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
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
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
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
        u32 preview_fps;
        u32 fps;
        u32 max_short_exp;
        u32 line;
    } expo;
    struct {
        bool bVideoMode;
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[4];
    I2C_ARRAY tGain_reg_HDR_SEF[4];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tExpo_reg_HDR_SEF[3];
    I2C_ARRAY tMax_short_exp_reg[2];
    I2C_ARRAY tMirror_reg[1];
    I2C_ARRAY tTemperature_reg_1[1];
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool orient_dirty;
    CUS_CAMSENSOR_ORIT  orit;
    bool reg_dirty;
    bool temperature_reg_1_dirty;
	CUS_UserDefSnrInfo_t stUserDefInfo;
} sc200ai_params;

// set sensor ID address and data,
#if 0
const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3107, 0xCB},
    {0x3108, 0x1C},
};
#endif

const static I2C_ARRAY Sensor_init_table_2M30fps[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x301f,0x03},
    {0x3243,0x01},
    {0x3248,0x02},
    {0x3249,0x09},
    {0x3253,0x08},
    {0x3271,0x0a},
    {0x3301,0x20},
    {0x3304,0x40},
    {0x3306,0x32},
    {0x330b,0x88},
    {0x330f,0x02},
    {0x331e,0x39},
    {0x3333,0x10},
    {0x3621,0xe8},
    {0x3622,0x16},
    {0x3637,0x1b},
    {0x363a,0x1f},
    {0x363b,0xc6},
    {0x363c,0x0e},
    {0x3670,0x0a},
    {0x3674,0x82},
    {0x3675,0x76},
    {0x3676,0x78},
    {0x367c,0x48},
    {0x367d,0x58},
    {0x3690,0x34},
    {0x3691,0x33},
    {0x3692,0x44},
    {0x369c,0x40},
    {0x369d,0x48},
    {0x3901,0x02},
    {0x3904,0x04},
    {0x3908,0x41},
    {0x391d,0x14},
    {0x391f,0x18},
    {0x3e01,0x8c},
    {0x3e02,0x20},
    {0x3e16,0x00},
    {0x3e17,0x80},
    {0x3f09,0x48},
    {0x5787,0x10},
    {0x5788,0x06},
    {0x578a,0x10},
    {0x578b,0x06},
    {0x5790,0x10},
    {0x5791,0x10},
    {0x5792,0x00},
    {0x5793,0x10},
    {0x5794,0x10},
    {0x5795,0x00},
    {0x5799,0x00},
    {0x57c7,0x10},
    {0x57c8,0x06},
    {0x57ca,0x10},
    {0x57cb,0x06},
    {0x57d1,0x10},
    {0x57d4,0x10},
    {0x57d9,0x00},
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
    {0x59ee,0xa0},
    {0x59ef,0x08},
    {0x59f4,0x18},
    {0x59f5,0x10},
    {0x59f6,0x0c},
    {0x59f7,0x10},
    {0x59f8,0x06},
    {0x59f9,0x02},
    {0x59fa,0x18},
    {0x59fb,0x10},
    {0x59fc,0x0c},
    {0x59fd,0x10},
    {0x59fe,0x04},
    {0x59ff,0x02},
    {0x36e9,0x20},
    {0x36f9,0x27},
    {0x0100,0x01},
    {0xffff,0x0a},/////delay 10ms
};

const static I2C_ARRAY Sensor_init_table_HDR[] =
{
    {0x0103,0x01},
    {0x0100,0x00},
    {0x36e9,0x80},
    {0x36f9,0x80},
    {0x301f,0x02},
    {0x320e,0x08},
    {0x320f,0xca},
    {0x3220,0x53},
    {0x3243,0x01},
    {0x3248,0x02},
    {0x3249,0x09},
    {0x3250,0x3f},
    {0x3253,0x08},
    {0x3271,0x0a},
    {0x3301,0x06},
    {0x3302,0x0c},
    {0x3303,0x08},
    {0x3304,0x60},
    {0x3306,0x30},
    {0x3308,0x10},
    {0x3309,0x70},
    {0x330b,0x80},
    {0x330d,0x16},
    {0x330e,0x1c},
    {0x330f,0x02},
    {0x3310,0x02},
    {0x331c,0x04},
    {0x331e,0x51},
    {0x331f,0x61},
    {0x3320,0x07},
    {0x3333,0x10},
    {0x334c,0x08},
    {0x3356,0x09},
    {0x3364,0x17},
    {0x3390,0x08},
    {0x3391,0x18},
    {0x3392,0x38},
    {0x3393,0x06},
    {0x3394,0x06},
    {0x3395,0x06},
    {0x3396,0x08},
    {0x3397,0x18},
    {0x3398,0x38},
    {0x3399,0x06},
    {0x339a,0x0a},
    {0x339b,0x10},
    {0x339c,0x20},
    {0x33ac,0x08},
    {0x33ae,0x10},
    {0x33af,0x19},
    {0x3621,0xe8},
    {0x3622,0x16},
    {0x3630,0xa0},
    {0x3637,0x36},
    {0x363a,0x1f},
    {0x363b,0xc6},
    {0x363c,0x0e},
    {0x3670,0x0a},
    {0x3674,0x82},
    {0x3675,0x76},
    {0x3676,0x78},
    {0x367c,0x48},
    {0x367d,0x58},
    {0x3690,0x34},
    {0x3691,0x33},
    {0x3692,0x44},
    {0x369c,0x40},
    {0x369d,0x48},
    {0x36eb,0x0c},
    {0x36ec,0x0c},
    {0x36fd,0x14},
    {0x3901,0x02},
    {0x3904,0x04},
    {0x3908,0x41},
    {0x391f,0x10},
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
    {0x3e16,0x00},
    {0x3e17,0x80},
    {0x3e23,0x00},
    {0x3e24,0x88},
    {0x3f09,0x48},
    {0x4816,0xb1},
    {0x4819,0x09},
    {0x481b,0x05},
    {0x481d,0x14},
    {0x481f,0x04},
    {0x4821,0x0a},
    {0x4823,0x05},
    {0x4825,0x04},
    {0x4827,0x05},
    {0x4829,0x08},
    {0x5787,0x10},
    {0x5788,0x06},
    {0x578a,0x10},
    {0x578b,0x06},
    {0x5790,0x10},
    {0x5791,0x10},
    {0x5792,0x00},
    {0x5793,0x10},
    {0x5794,0x10},
    {0x5795,0x00},
    {0x5799,0x00},
    {0x57c7,0x10},
    {0x57c8,0x06},
    {0x57ca,0x10},
    {0x57cb,0x06},
    {0x57d1,0x10},
    {0x57d4,0x10},
    {0x57d9,0x00},
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
    {0x59ee,0xa0},
    {0x59ef,0x08},
    {0x59f4,0x18},
    {0x59f5,0x10},
    {0x59f6,0x0c},
    {0x59f7,0x10},
    {0x59f8,0x06},
    {0x59f9,0x02},
    {0x59fa,0x18},
    {0x59fb,0x10},
    {0x59fc,0x0c},
    {0x59fd,0x10},
    {0x59fe,0x04},
    {0x59ff,0x02},
    {0x36e9,0x20},
    {0x36f9,0x24},
    {0x0100,0x01},
    {0xffff,0x0a},/////delay 10ms
};

const static I2C_ARRAY mirror_reg[] =
{
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
    {0x3e08, 0x03},
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

const static I2C_ARRAY max_short_exp_reg[] = {
    {0x3e23, 0x00},
    {0x3e24, 0x88}
};

const static I2C_ARRAY temperature_reg_1[] = {
    {0x5799, 0x00},
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
#define SENSOR_NAME sc200ai

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

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
        info->u32AEShutter_min                   = Preview_line_period_HDR*2;
        info->u32AEShutter_step                  = Preview_line_period_HDR*2;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    return SUCCESS;
}
#define SENSOR_SLEEP_MODE_ENABLE      0
#define SENSOR_SLEEP_MODE_DISABLE     1
u8 g_u8CurSleepMode = 255;
static int _pCus_SleepMode(ss_cus_sensor* handle, u32 sleepModeEnable ,u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    sc200ai_params *params = (sc200ai_params *)handle->private_data;

    if(sleep_mode == 1)
    {
        I2C_ARRAY sleep_reg = {0x0100, 0x00};
        switch(sleepModeEnable)
        {
            case SENSOR_SLEEP_MODE_ENABLE:
                sleep_reg.data= 0x00;//enable sleep mode
                SensorReg_Write(sleep_reg.reg, sleep_reg.data);
                params->stUserDefInfo.bSnrSleep = true;
                break;
            case SENSOR_SLEEP_MODE_DISABLE:
                sleep_reg.data= 0x01;//disable sleep mode
                SensorReg_Write(sleep_reg.reg, sleep_reg.data);
                params->stUserDefInfo.bSnrSleep = false;
                break;
            default:
                SENSOR_EMSG("do nothing ,Sleep Mode [1] only On/Off !!!\n");
                break;
        }
    }
    else if(sleep_mode == 2)
    {
        switch(sleepModeEnable)
        {
            case SENSOR_SLEEP_MODE_ENABLE:
                sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
                params->stUserDefInfo.bSnrSleep = true;
                SENSOR_EMSG(">>>>>>>>>SLEEP MODE enable \n");
                break;
            case SENSOR_SLEEP_MODE_DISABLE:
                sensor_if->PowerOff(idx, !CUS_CLK_POL_NEG);
                params->stUserDefInfo.bSnrSleep = false;
                SENSOR_EMSG(">>>>>>>>>SLEEP MODE disable \n");
                break;
            default:
                SENSOR_EMSG("do nothing ,Sleep Mode [2] only On/Off !!!\n");
                break;
        }
    }
    else
    {
         SENSOR_EMSG("sleep_mode %d not avalibale\n", sleep_mode);
    }
    sensor_if->SetUserDefInfo(idx, params->stUserDefInfo);

    return 0;
}

/////////////////// sensor hardware dependent //////////////
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
    //Configuration Chip RX
    pCus_PowerOn_InitChipRX(handle, idx);
    sensor_if->MCLK(idx, 1, handle->mclk);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(5000);

    return SUCCESS;
}
u8 g_u8SuspendFlag = 0;
static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    if(g_u8SuspendFlag == 0){
		// power/reset low
		ISensorIfAPI *sensor_if = handle->sensor_if_api;
		sc200ai_params *params = (sc200ai_params *)handle->private_data;
		SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
		sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
		//handle->i2c_bus->i2c_close(handle->i2c_bus);
		CamOsMsSleep(1);
		//Set_csi_if(0, 0);
		sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
		if (handle->interface_attr.attr_mipi.mipi_hdr_mode != CUS_HDR_MODE_NONE) {
			sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
		}
		sensor_if->MCLK(idx, 0, handle->mclk);

		params->orit = CUS_ORIT_M0F0;
	}
	g_u8SuspendFlag = 1;
    return SUCCESS;
}
static int pCus_Suspend(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI  *sensor_if = handle->sensor_if_api;
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_ENABLE, idx);
    sensor_if->MCLK(idx, 0, handle->mclk); // handle->mclk);
    g_u8SuspendFlag = 1;
    SENSOR_EMSG("[%s] sensor suspend!\n", __FUNCTION__);
    return SUCCESS;
}

static int pCus_Resume(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI  *sensor_if = handle->sensor_if_api;
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    sensor_if->MCLK(idx, 1, handle->mclk); //handle->mclk);
    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_DISABLE, idx);
    CamOsMsDelay(1);//or ae notify i2c err
    pCus_AEStatusNotify(handle, 0, CUS_FRAME_ACTIVE);
    g_u8SuspendFlag = 0;
    SENSOR_EMSG("[%s] sensor resume!\n", __FUNCTION__);
    return SUCCESS;
}
static int pCus_Reopen(struct __ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI  *sensor_if = handle->sensor_if_api;
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->MCLK(idx, 1, handle->mclk); //handle->mclk);

    _pCus_SleepMode(handle, SENSOR_SLEEP_MODE_DISABLE, idx);

    g_u8SuspendFlag = 0;

    SENSOR_EMSG("[%s] sensor reopen!\n", __FUNCTION__);
    return SUCCESS;
}
static int pCus_StreamOn(struct __ss_cus_sensor *handle, u32 idx)
{
    I2C_ARRAY stream_onoff_reg = {0x0100, 0x00};

    stream_onoff_reg.data= 0x01;//disable sleep mode
    SensorReg_Write(stream_onoff_reg.reg, stream_onoff_reg.data);

    SENSOR_EMSG("[%s] sensor stream on!\n", __FUNCTION__);
    return SUCCESS;
}
/////////////////// image function /////////////////////////
//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
#if 0
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

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
        if( id_from_sensor[i].data != Sensor_id_table[i].data )
        return FAIL;
        *id = ((*id)+ id_from_sensor[i].data)<<8;
    }

    *id >>= 8;
    SENSOR_DMSG("[%s]sc200ai Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}
#endif

static int sc200ai_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    return SUCCESS;
}
static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps);
//static int pCus_SetAEGain_cal(ss_cus_sensor *handle, u32 gain);
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status);
static int pCus_init_mipi2lane_linear_2M30fps(ss_cus_sensor *handle)
{
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;
    //ISensorIfAPI *sensor_if = (handle->sensor_if_api);

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
static int pCus_init_mipi2lane_HDR(ss_cus_sensor *handle)
{
   sc200ai_params *params = (sc200ai_params *)handle->private_data;
    int i,cnt=0;
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR);i++)
    {
        if(Sensor_init_table_HDR[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR[i].data);
        }
        else
        {
            cnt = 0;
            SENSOR_DMSG("reg =  %x, data = %x\n", Sensor_init_table_HDR[i].reg, Sensor_init_table_HDR[i].data);
            while(SensorReg_Write(Sensor_init_table_HDR[i].reg,Sensor_init_table_HDR[i].data) != SUCCESS)
            {
                cnt++;
                 SENSOR_DMSG("Sensor_init_table_HDR -> Retry %d...\n",cnt);
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
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0: //"1920x1080@30fps"
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_linear_2M30fps;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;

        default:
            break;
    }

    return SUCCESS;
}
static int pCus_SetVideoRes_HDR(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    sc200ai_params *params = (sc200ai_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_mipi2lane_HDR;
            params->expo.vts = vts_30fps_HDR;
            params->expo.fps = 30;
            params->expo.max_short_exp=135;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
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
   sc200ai_params *params = (sc200ai_params *)handle->private_data;

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

    params->orit = orit;
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
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
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
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
static int pCus_SetFPS_HDR_SEF(ss_cus_sensor *handle, u32 fps)
{
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.max_short_exp = (((params->expo.vts)/17 - 1)>>1) << 1;
    params->tMax_short_exp_reg[0].data = (params->expo.max_short_exp >> 8) & 0x00ff;
    params->tMax_short_exp_reg[1].data = (params->expo.max_short_exp >> 0) & 0x00ff;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

static int pCus_SetFPS_hdr_lef(ss_cus_sensor *handle, u32 fps)
{
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.max_short_exp = (((params->expo.vts)/17 - 1)>>1) << 1;
    params->tMax_short_exp_reg[0].data = (params->expo.max_short_exp >> 8) & 0x00ff;
    params->tMax_short_exp_reg[1].data = (params->expo.max_short_exp >> 0) & 0x00ff;
    params->tVts_reg[0].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){

   sc200ai_params *params = (sc200ai_params *)handle->private_data;
	if(params->stUserDefInfo.bSnrSleep == true)//isp resume set ae i2c err, wait sensor resume power on do aenotify
    {
        SENSOR_EMSG("[%s]:%d sensor sleep enable, no I2C write \n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }
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
        if(params->temperature_reg_1_dirty) {
            SensorRegArrayW((I2C_ARRAY*)params->tTemperature_reg_1, ARRAY_SIZE(temperature_reg_1));
            params->temperature_reg_1_dirty = false;
        }

       break;
       default :
       break;
   }
   return SUCCESS;
}
static int pCus_AEStatusNotifyHDR_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
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

static int pCus_AEStatusNotifyHDR_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
   sc200ai_params *params = (sc200ai_params *)handle->private_data;
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
            SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_HDR_SEF, ARRAY_SIZE(expo_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_HDR_SEF, ARRAY_SIZE(gain_reg_HDR_SEF));
            SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
            SensorRegArrayW((I2C_ARRAY*)params->tMax_short_exp_reg, ARRAY_SIZE(max_short_exp_reg));
            params->reg_dirty = false;
        }
        if(params->temperature_reg_1_dirty) {
            SensorRegArrayW((I2C_ARRAY*)params->tTemperature_reg_1, ARRAY_SIZE(temperature_reg_1));
            params->temperature_reg_1_dirty = false;
        }
        break;
        default :
             break;
    }
    return SUCCESS;
}
static int pCus_SetAEUSecsHDR_LEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0;
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));
    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    half_lines = 4*dou_lines;
    if(half_lines<=4) half_lines=4;
    if (half_lines >  2 * (params->expo.vts-params->expo.max_short_exp-10)) {
        half_lines = 2 * (params->expo.vts-params->expo.max_short_exp-10);
    }
//  SENSOR_DMSG("[%s] us %ld, half_lines %ld, vts %ld\n", __FUNCTION__, us, half_lines, params->expo.vts);

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

static int pCus_GetAEUSecsHDR_SEF(ss_cus_sensor *handle, u32 *us) {
  int rc=0;
  u32 lines = 0;
  sc200ai_params *params = (sc200ai_params *)handle->private_data;
  lines |= (u32)(params->tExpo_reg_HDR_SEF[0].data&0x0f)<<16;
  lines |= (u32)(params->tExpo_reg_HDR_SEF[1].data&0xff)<<8;
  lines |= (u32)(params->tExpo_reg_HDR_SEF[2].data&0xf0)<<0;
  lines >>= 4;
  *us = (lines*Preview_line_period_HDR)/1000/2; //return us

  SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
  return rc;
}

static int pCus_SetAEUSecsHDR_SEF(ss_cus_sensor *handle, u32 us)
{
    int i;
    u32 half_lines = 0,dou_lines = 0;
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {
        {0x3e22, 0x00}, // expo[3:0]
        {0x3e04, 0x21}, // expo[7:0]
        {0x3e05, 0x00}, // expo[7:4]
    };
    memcpy(expo_reg_temp, params->tExpo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));

    dou_lines = (1000*us)/(Preview_line_period_HDR*2); // Preview_line_period in ns
    half_lines = 4*dou_lines;
    if(half_lines<=4) half_lines=4;
    if (half_lines >  2 * (params->expo.max_short_exp-10)) {
        half_lines = 2 * (params->expo.max_short_exp-10);
    }

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
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
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
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    I2C_ARRAY expo_reg_temp[] = {  // max expo line vts-4!
    {0x3e00, 0x00},//expo [20:17]
    {0x3e01, 0x00}, // expo[16:8]
    {0x3e02, 0x10}, // expo[7:0], [3:0] fraction of line
    };
    memcpy(expo_reg_temp, params->tExpo_reg, sizeof(expo_reg));

    half_lines = (1000*us*2)/Preview_line_period; // Preview_line_period in ns
    if(half_lines <= 1) half_lines=1;
    if (half_lines >  2 * (params->expo.vts)-10) {
        vts = (half_lines+11)/2;
    }
    else
        vts=params->expo.vts;

    params->expo.line = half_lines;
    SENSOR_DMSG("[%s] us %ld, half_lines %ld, vts %ld\n", __FUNCTION__, us, half_lines, params->expo.vts);

    half_lines = half_lines<<4;

    params->tExpo_reg[0].data = (half_lines>>16) & 0x0f;
    params->tExpo_reg[1].data =  (half_lines>>8) & 0xff;
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
#if 0
static int pCus_SetAEGain_cal(ss_cus_sensor *handle, u32 gain) {

    return SUCCESS;
}
#endif

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain) {
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    u8 i=0 , Coarse_gain = 1,DIG_gain=1;
    u32 Dcg_gainx100 = 100, ANA_Fine_gainx64 = 1;
    u8 Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x20,DIG_Fine_gain_reg=0x80;

    I2C_ARRAY gain_reg_temp[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e08, 0x00|0x03},
        {0x3e09, 0x40},
    };
    I2C_ARRAY temperature_reg_1_temp[] ={
        {0x5799, 0x00},
    };
    memcpy(gain_reg_temp, params->tGain_reg, sizeof(gain_reg));
    memcpy(temperature_reg_1_temp, params->tTemperature_reg_1, sizeof(temperature_reg_1_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN*1024) {
        gain = SENSOR_MAX_GAIN*1024;
    }

    if (gain < 2 * 1024) // start again
    {
        Dcg_gainx100 = 100;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x03; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <=  3456)
    {
        Dcg_gainx100 = 100;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x07; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 6908)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x23; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 13817)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x27; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 27635)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 4;     DIG_gain=1;
        Coarse_gain_reg = 0x2f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 55270) // end again
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=1;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (gain < 55270 * 2) // start dgain
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=1;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  ANA_Fine_gain_reg=0x7f;

    }
    else if (gain < 55270 * 4)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=2;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x1;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain < 55270 * 8)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=4;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x3;  ANA_Fine_gain_reg=0x7f;
    }
   else if (gain < 55270 * 16)
    {
       Dcg_gainx100 = 340;      Coarse_gain = 8;      DIG_gain=8;        ANA_Fine_gainx64=127;
       Coarse_gain_reg = 0x3f; DIG_gain_reg=0x7;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 1754822)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=16;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0xF;  ANA_Fine_gain_reg=0x7f;
    }
#endif

    if(gain < 3456)
    {
        ANA_Fine_gain_reg = abs(100 * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    }else if(gain == 3456) // || gain == 3456)
    {
        ANA_Fine_gain_reg = 0x6C;
    }
    else if(gain < 55270)
    {
        ANA_Fine_gain_reg = abs(100 * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    }else{
        DIG_Fine_gain_reg = abs(800 * gain /(Dcg_gainx100 * Coarse_gain * DIG_gain) / ANA_Fine_gainx64);
    }

    params->tGain_reg[3].data = ANA_Fine_gain_reg;   // 0x3e09
    params->tGain_reg[2].data = Coarse_gain_reg;     // 0x3e08
    params->tGain_reg[1].data = DIG_Fine_gain_reg;   // 0x3e07
    params->tGain_reg[0].data = DIG_gain_reg & 0xF;  // 0x3e06

    for (i = 0; i < ARRAY_SIZE(params->tGain_reg); i++)
    {
      if (params->tGain_reg[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

    // highTemp dpc
    if (gain >= 30 * 1024) {
        params->tTemperature_reg_1[0].data = 0x07;
    } else if (gain <= 20 * 1024) {
        params->tTemperature_reg_1[0].data = 0x00;
    }

    for (i = 0; i < ARRAY_SIZE(temperature_reg_1_temp); i++) {
        if (params->tTemperature_reg_1[i].data != temperature_reg_1_temp[i].data) {
            params->temperature_reg_1_dirty = true;
        break;
        }
    }
    return SUCCESS;
}

static int pCus_SetAEGainHDR_SEF(ss_cus_sensor *handle, u32 gain) {
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    u8 i=0 , Coarse_gain = 1,DIG_gain=1;
    u32 Dcg_gainx100 = 1, ANA_Fine_gainx64 = 1;
    u8 Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x20,DIG_Fine_gain_reg=0x80;
    I2C_ARRAY gain_reg_temp[] = {
        {0x3e10, 0x00},
        {0x3e11, 0x80},
        {0x3e12, 0x00|0x03},
        {0x3e13, 0x40},
    };
    I2C_ARRAY temperature_reg_1_temp[] ={
        {0x5799, 0x00},
    };
    memcpy(gain_reg_temp, params->tGain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(temperature_reg_1_temp, params->tTemperature_reg_1, sizeof(temperature_reg_1_temp));

    if (gain <= 1024) {
        gain = 1024;
    } else if (gain > SENSOR_MAX_GAIN*1024) {
        gain = SENSOR_MAX_GAIN*1024;
    }

    if (gain < 2 * 1024) // start again
    {
        Dcg_gainx100 = 100;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x03; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <=  3456)
    {
        Dcg_gainx100 = 100;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x07; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 6908)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x23; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 13817)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x27; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 27635)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 4;     DIG_gain=1;
        Coarse_gain_reg = 0x2f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (gain <= 55270) // end again
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=1;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (gain < 55270 * 2) // start dgain
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=1;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x0;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain < 55270 * 4)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=2;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x1;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain < 55270 * 8)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=4;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0x3;  ANA_Fine_gain_reg=0x7f;
    }
   else if (gain < 55270 * 16)
    {
       Dcg_gainx100 = 340;      Coarse_gain = 8;      DIG_gain=8;        ANA_Fine_gainx64=127;
       Coarse_gain_reg = 0x3f; DIG_gain_reg=0x7;  ANA_Fine_gain_reg=0x7f;
    }
    else if (gain <= 1754822)
    {
        Dcg_gainx100 = 340;      Coarse_gain = 8;     DIG_gain=16;       ANA_Fine_gainx64=127;
        Coarse_gain_reg = 0x3f; DIG_gain_reg=0xF;  ANA_Fine_gain_reg=0x7f;
    }
#endif

    if(gain < 3456)
    {
        //ANA_Fine_gainx64 = 1000 * 100 * gain / (Dcg_gainx100 * Coarse_gain) / 1024;
        ANA_Fine_gain_reg = abs(100 * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    }else if(gain == 3456) // || gain == 3456)
    {
        ANA_Fine_gain_reg = 0x6C;
    }
    else if(gain < 55270)
    {
        ANA_Fine_gain_reg = abs(100 * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    }else{
        DIG_Fine_gain_reg = abs(800 * gain /(Dcg_gainx100 * Coarse_gain * DIG_gain) / ANA_Fine_gainx64);
    }

    params->tGain_reg_HDR_SEF[3].data = ANA_Fine_gain_reg;
    params->tGain_reg_HDR_SEF[2].data = Coarse_gain_reg;
    params->tGain_reg_HDR_SEF[1].data = DIG_Fine_gain_reg;
    params->tGain_reg_HDR_SEF[0].data = DIG_gain_reg & 0xF;
    //printk("[%s]  gain_reg : %x ,%x ,%x, %x\n\n", __FUNCTION__,gain_reg_HDR_SEF[3].data,gain_reg_HDR_SEF[2].data,gain_reg_HDR_SEF[1].data,gain_reg_HDR_SEF[0].data);

    for (i = 0; i < ARRAY_SIZE(params->tGain_reg_HDR_SEF); i++)
    {
      if (params->tGain_reg_HDR_SEF[i].data != gain_reg_temp[i].data)
      {
        params->reg_dirty = true;
        break;
      }
    }

    // highTemp dpc
    if (gain >= 30 * 1024) {
        params->tTemperature_reg_1[0].data = 0x07;
    } else if (gain <= 20 * 1024) {
        params->tTemperature_reg_1[0].data = 0x00;
    }

    for (i = 0; i < ARRAY_SIZE(temperature_reg_1_temp); i++) {
        if (params->tTemperature_reg_1[i].data != temperature_reg_1_temp[i].data) {
            params->temperature_reg_1_dirty = true;
        break;
        }
    }
    return SUCCESS;
}

#if 0
static int pCus_GetAEMinMaxUSecs(ss_cus_sensor *handle, u32 *min, u32 *max) {
    *min = 1;
    *max = 1000000/Preview_MIN_FPS;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ss_cus_sensor *handle, u32 *min, u32 *max) {
  *min = 1024; // 1232 x1.2 Gain
  *max = SENSOR_MAX_GAIN*1024;
  return SUCCESS;
}

static int sc200ai_GetShutterInfo(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS;
    info->min = Preview_line_period;// /2;
    info->step = Preview_line_period /2;
    return SUCCESS;
}
static int pCus_GetShutterInfo_hdr_lef(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS;
    info->min = Preview_line_period_HDR*2;
    info->step = Preview_line_period_HDR*2;
    return SUCCESS;
}
static int pCus_GetShutterInfo_hdr_sef(struct __ss_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/(Preview_MIN_FPS*17);
    info->min = Preview_line_period_HDR*2;
    info->step = Preview_line_period_HDR*2;
    return SUCCESS;
}
#endif
static int pCus_poweron_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}
#if 0
static int pCus_GetSensorID_hdr_lef(ss_cus_sensor *handle, u32 *id)
{
    *id = 0;
     return SUCCESS;
}
#endif
static int pCus_init_hdr_lef(ss_cus_sensor *handle)
{
    return SUCCESS;
}

static int pCus_GetFPS_hdr_lef(ss_cus_sensor *handle)
{
    sc200ai_params *params = (sc200ai_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps_HDR*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps_HDR*max_fps)/tVts;

    return params->expo.preview_fps;
}
#if 0
static int pCus_setCaliData_gain_linearity_hdr_lef(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num)
{
    return SUCCESS;
}
static int pCus_SetAEGain_cal_hdr_lef(ss_cus_sensor *handle, u32 gain)
{
    return SUCCESS;
}
static int pCus_setCaliData_gain_linearity(ss_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num) {

    return SUCCESS;
}
#endif

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
int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle) {
   ss_cus_sensor *handle = drv_handle;
    sc200ai_params *params;
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
    params = (sc200ai_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMax_short_exp_reg, max_short_exp_reg, sizeof(max_short_exp_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tTemperature_reg_1, temperature_reg_1, sizeof(temperature_reg_1));
    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"sc200ai_MIPI_Linear");

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
    params->orit      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = sc200ai_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sc200ai_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sc200ai_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sc200ai_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sc200ai_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sc200ai_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sc200ai_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sc200ai_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc200ai_mipi_linear[res].senstr.strResDesc);
    }

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period*1;//2
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period*1;//2

    // i2c

    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    //polarity
    /////////////////////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
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

    //handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_mipi2lane_linear_2M30fps;
	handle->pCus_sensor_suspend     = pCus_Suspend;
    handle->pCus_sensor_resume      = pCus_Resume;
	handle->pCus_sensor_reopen      = pCus_Reopen;
    handle->pCus_sensor_streamon    = pCus_StreamOn;
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
    handle->pCus_sensor_SetPatternMode = sc200ai_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;

    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;

     //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    //handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    //handle->pCus_sensor_GetShutterInfo = sc200ai_GetShutterInfo;
    params->expo.vts=vts_30fps;
    params->expo.fps = 30;
    params->expo.line= 1000;
    params->reg_dirty = false;
    params->temperature_reg_1_dirty = false;
    params->orient_dirty = false;
    return SUCCESS;
}
int cus_camsensor_init_handle_hdr_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sc200ai_params *params = NULL;
    int res;

    cus_camsensor_init_handle_linear(drv_handle);
    params = (sc200ai_params *)handle->private_data;

    sprintf(handle->strSensorStreamName,"sc200ai_MIPI_HDR_SEF");

    handle->bayer_id    = SENSOR_BAYERID_HDR;
    handle->RGBIR_id    = SENSOR_RGBIRID;

    handle->data_prec   = SENSOR_DATAPREC_HDR;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = SENSOR_HDR_MODE;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = sc200ai_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = sc200ai_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = sc200ai_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = sc200ai_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = sc200ai_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = sc200ai_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = sc200ai_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = sc200ai_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc200ai_mipi_hdr[res].senstr.strResDesc);
    }

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/(Preview_MIN_FPS*17);
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR*2;

    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR;
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    handle->pCus_sensor_init        = pCus_init_mipi2lane_HDR;

    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_SEF; //TBD
    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotifyHDR_SEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecsHDR_SEF;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_SEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_SEF;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetShutterInfo = pCus_GetShutterInfo_hdr_sef;
    params->expo.vts = vts_30fps_HDR;
    params->expo.fps = 30;
    params->expo.max_short_exp=135;

    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    //handle->ae_gain_delay       = 2;
    //handle->ae_shutter_delay    = 2;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 2;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sc200ai_params *params;
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
    params = (sc200ai_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tGain_reg_HDR_SEF, gain_reg_HDR_SEF, sizeof(gain_reg_HDR_SEF));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_reg_HDR_SEF, expo_reg_HDR_SEF, sizeof(expo_reg_HDR_SEF));
    memcpy(params->tMax_short_exp_reg, max_short_exp_reg, sizeof(max_short_exp_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tTemperature_reg_1, temperature_reg_1, sizeof(temperature_reg_1));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SC200ai_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_HDR;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    params->orit      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = SENSOR_HDR_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num =  0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = sc200ai_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = sc200ai_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = sc200ai_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = sc200ai_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = sc200ai_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = sc200ai_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = sc200ai_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = sc200ai_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sc200ai_mipi_hdr[res].senstr.strResDesc);
    }

    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR*2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR*2;

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR;

    //polarity
    /////////////////////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////


    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    //handle->ae_gain_delay       = 2;//0;//1;
    //handle->ae_shutter_delay    = 2;//1;//2;

    //handle->ae_gain_ctrl_num = 1;
    //handle->ae_shutter_ctrl_num = 2;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    //handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_hdr_lef;
    handle->pCus_sensor_poweron     = pCus_poweron_hdr_lef;
    handle->pCus_sensor_poweroff    = pCus_poweroff_hdr_lef;

    // Normal
    //handle->pCus_sensor_GetSensorID       = pCus_GetSensorID_hdr_lef;
    handle->pCus_sensor_GetVideoResNum = NULL;
    handle->pCus_sensor_GetVideoRes       = NULL;
    handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes       = NULL;
    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_hdr_lef;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_hdr_lef;
    handle->pCus_sensor_SetPatternMode = sc200ai_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotifyHDR_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    //handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;

     //sensor calibration
    //handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal_hdr_lef;
    //handle->pCus_sensor_setCaliData_gain_linearity= pCus_setCaliData_gain_linearity_hdr_lef;
    //handle->pCus_sensor_GetShutterInfo = pCus_GetShutterInfo_hdr_lef;

    params->expo.vts = vts_30fps_HDR;
    params->expo.fps = 30;
    params->reg_dirty = false;
    params->temperature_reg_1_dirty = false;
    params->orient_dirty = false;

    return SUCCESS;
}

#if 0
static int cus_camsensor_release_handle(ss_cus_sensor *handle) {

    return SUCCESS;
}
#endif

SENSOR_DRV_ENTRY_IMPL_END_EX( SC200ai,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_sef,
                            cus_camsensor_init_handle_hdr_lef,
                            sc200ai_params
                         );
