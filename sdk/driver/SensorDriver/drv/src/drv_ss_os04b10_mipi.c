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
   Porting owner : ming-en.lu
   Date          : 2023/09/25
   Build on      : Master_V4 I6C
   Verified on   : Not yet
   Remark        : NA
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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OS04B10);

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

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3


//#undef SENSOR_DBG
#define SENSOR_DBG 0

///////////////////////////////////////////////////////////////
//          @@@                                                                                       //
//       @   @@      ==  S t a r t * H e r e ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                           //
//         @@@@                                                                                  //
//                                                                                                     //
//      Start Step 1 --  show preview on LCM                                         //
//                                                                                                    �@//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

//#define SENSOR_ISP_TYPE     ISP_EXT                   //ISP_EXT, ISP_SOC
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID      CUS_BAYER_GB            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

//#define SENSOR_MAX_GAIN     128//(15.5*15.9)                  // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MAX_GAIN                             (128 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)

#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period 21987//16801//17814                           // MCLK=21.6 HTS/PCLK=3080 pixels/97.2MHZ=31.687us                              // 3126 for 25fps
#define vts_30fps  1514                                     // VTS for 20fps

#define Preview_WIDTH       2560//2688                    //resolution Width when preview
#define Preview_HEIGHT      1440//1520                    //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     3                      //slowest preview FPS

#define Cap_Max_line_number 1440//1520                   //maximum exposure line munber of sensor when capture

#define SENSOR_I2C_ADDR    0x78                   //I2C slave address
#define SENSOR_I2C_SPEED   200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A8D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_POS        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

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
}os04b10_mipi_linear[] = {
    {LINEAR_RES_1, {2592, 1440, 3, 30}, {4, 4, 2568, 1448}, {"2560x1440@30fps"}},
};

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
static int OS04B10_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int OS04B10_SetAEUSecs(ss_cus_sensor *handle, u32 us);


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
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
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
        u32 lines;
    } expo;

    int sen_init;
    int still_min_fps;
    int video_min_fps;
    u32 gain;
    bool reg_dirty;
    bool ori_dirty;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
} OS04B10_params;

// set sensor ID address and data,
I2C_ARRAY Sensor_id_table[] =
{//P0
    {0xfd, 0x00},
    {0x02, 0x43},      // {address of ID, ID },
    {0x03, 0x08},      // {address of ID, ID },
};

I2C_ARRAY Sensor_init_table[] =
{
    {0xfd, 0x00},
    {0x34, 0x71},
    {0x32, 0x01},
    {0x33, 0x01},
    {0x2e, 0x0c},
    {0xfd, 0x01},
    {0x03, 0x01},
    {0x04, 0xc6},
    {0x06, 0x0b},
    {0x0a, 0x50},
    {0x0d, 0x10},
    {0x0e, 0x05},
    {0x0f, 0xea},
    {0x38, 0x20},
    {0x39, 0x08},
    {0x31, 0x01},
    {0x24, 0xff},
    {0x01, 0x01},
    {0x11, 0x59},
    {0x13, 0xf4},
    {0x14, 0xff},
    {0x19, 0xf2},
    {0x16, 0x68},
    {0x1a, 0x5e},
    {0x1c, 0x1a},
    {0x1d, 0xd6},
    {0x1f, 0x17},
    {0x20, 0x99},
    {0x26, 0x76},
    {0x27, 0x0c},
    {0x29, 0x3b},
    {0x2a, 0x00},
    {0x2b, 0x8e},
    {0x2c, 0x0b},
    {0x2e, 0x02},
    {0x44, 0x03},
    {0x45, 0xbe},
    {0x50, 0x06},
    {0x51, 0x10},
    {0x52, 0x0d},
    {0x53, 0x08},
    {0x55, 0x15},
    {0x56, 0x00},
    {0x57, 0x09},
    {0x59, 0x00},
    {0x5a, 0x04},
    {0x5b, 0x00},
    {0x5c, 0xe0},
    {0x5d, 0x00},
    {0x65, 0x00},
    {0x67, 0x00},
    {0x66, 0x2a},
    {0x68, 0x2c},
    {0x69, 0x0c},
    {0x6a, 0x0a},
    {0x6b, 0x03},
    {0x6c, 0x18},
    {0x71, 0x42},
    {0x72, 0x04},
    {0x73, 0x30},
    {0x74, 0x03},
    {0x77, 0x28},
    {0x7b, 0x00},
    {0x7f, 0x18},
    {0x83, 0xf0},
    {0x85, 0x10},
    {0x86, 0xf0},
    {0x8a, 0x33},
    {0x8b, 0x33},
    {0x28, 0x04},
    {0x34, 0x00},
    {0x35, 0x04},
    {0x36, 0x0a},
    {0x37, 0x08},
    {0x4a, 0x00},
    {0x4b, 0x00},
    {0x4c, 0x05},
    {0x4d, 0xa8},
    {0x01, 0x01},
    {0x8e, 0x0a},
    {0x8f, 0x08},
    {0x90, 0x05},
    {0x91, 0xa8},
    {0xa1, 0x04},
    {0xc4, 0x80},
    {0xc5, 0x80},
    {0xc6, 0x80},
    {0xc7, 0x80},
    {0xfb, 0x00},
    {0xfa, 0x16},
    {0xfa, 0x14},
    {0xfa, 0x02},
    {0xf0, 0x40},
    {0xf1, 0x40},
    {0xf2, 0x40},
    {0xf3, 0x40},
    {0xb1, 0x01},
    {0xb6, 0x80},
    {0xb1, 0x03},
    {0xfd, 0x00},
    {0x36, 0x01},
    {0x34, 0x72},
    {0x34, 0x71},
    {0x36, 0x00},
    {0xfd, 0x01},
    {0xfb, 0x03},
    {0xfd, 0x03},
    {0xc0, 0x01},

//For OTP_BPC
    {0xfd, 0x02},
    {0xa8, 0x01},
    {0xa9, 0x00},
    {0xaa, 0x04},
    {0xab, 0x00},
    {0xac, 0x04},
    {0xad, 0x05},
    {0xae, 0xa8},
    {0xaf, 0x0a},
    {0xb0, 0x08},
    {0x62, 0x05},
    {0x63, 0x00},
    {0xfd, 0x01},
    {0xb1, 0x03},
};


I2C_ARRAY TriggerStartTbl[] = {
    // {0x0100,0x01},//normal mode
};

I2C_ARRAY PatternTbl[] = {
    // {0x5081,0x00}, //colorbar pattern , bit 7 to enable
};

I2C_ARRAY mirror_reg[] = {
    {0xfd, 0x01},
    {0x3f, 0x01},//P1 M0F0 [1]:F [0]:M
    {0x01, 0x01},
    {0xfd, 0x02},
    {0x62, 0x05},
    {0x63, 0x00},
    {0xfd, 0x01},
};

/////////////////////////////////////////////////////////////////
//       @@@@@@                                                                                    //
//                 @@                                                                                    //
//             @@@                                                                                      //
//       @       @@                                                                                    //
//         @@@@                                                                                        //
//                                                                                                          //
//      Step 3 --  complete camera features                                              //
//                                                                                                         //
//                                                                                                         //
//  camera set EV, MWB, orientation, contrast, sharpness                          //
//   , saturation, and Denoise can work correctly.                                     //
//                                                                                                          //
/////////////////////////////////////////////////////////////////



typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

static I2C_ARRAY gain_reg[] = {
    {0xfd, 0x01},
    {0x38, 0x00},//long a-gain[8]
    {0x24, 0x20},//long a-gain[7:0]
    {0x39, 0x08},// d-gain[7:0] 1x:0x08 32x:0xff
};

I2C_ARRAY expo_reg[] = {
    {0xfd, 0x01},//
    {0x03, 0x00},//long exp[15,8]
    {0x04, 0x9a},//long exp[7,0]
};

I2C_ARRAY vts_reg[] = {
    {0x0d,0x10},//0x10 enable
    {0x0E,0x07},//MSB
    {0x0F,0xc1},//LSB
};

#if 0
static CUS_INT_TASK_ORDER def_order = {
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
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME OS04B10


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
#if 0
static int ISP_config_io(ss_cus_sensor *handle) {
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s]", __FUNCTION__);

    sensor_if->HsyncPol(handle, handle->HSYNC_POLARITY);
    sensor_if->VsyncPol(handle, handle->VSYNC_POLARITY);
    sensor_if->ClkPol(handle, handle->PCLK_POLARITY);
    sensor_if->BayerFmt(handle, handle->bayer_id);
    sensor_if->DataBus(handle, handle->sif_bus);

    sensor_if->DataPrecision(handle, handle->data_prec);
    sensor_if->FmtConv(handle,  handle->data_mode);
    return SUCCESS;
}
#endif
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = Preview_line_period * 2;
    info->u32AEShutter_step                  = Preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}
static int OS04B10_poweron(ss_cus_sensor *handle, u32 idx)
{
    OS04B10_params *params = (OS04B10_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);

    sensor_if->MCLK(idx, 1, handle->mclk);

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, params->reset_POLARITY);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_USLEEP(1000);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    SENSOR_USLEEP(1000);

    //sensor_if->Set3ATaskOrder(handle, def_order);
    // pure power on
    //ISP_config_io(handle);
    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    SENSOR_USLEEP(5000);
    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;
}

static int OS04B10_poweroff(ss_cus_sensor *handle, u32 idx)
{
    OS04B10_params *params = (OS04B10_params *)handle->private_data;
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);
    SENSOR_USLEEP(500);
    return SUCCESS;
}

static int OS04B10_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);
    return SUCCESS;
}

static int OS04B10_init(ss_cus_sensor *handle){
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {

        if(Sensor_init_table[i].reg==0xff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table[i].reg, Sensor_init_table[i].data) != SUCCESS)
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

    return SUCCESS;
}

static int OS04B10_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int OS04B10_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int OS04B10_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int OS04B10_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = OS04B10_init;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int OS04B10_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    //OS04B10_params *params = (OS04B10_params *)handle->private_data;

    sen_data = mirror_reg[1].data & 0x03;
    SENSOR_DMSG("\n\n[%s]:mirror:%x\r\n\n\n\n",__FUNCTION__, sen_data);
    switch(sen_data) {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
        break;
        case 0x10:
            *orit = CUS_ORIT_M1F0;
        break;
        case 0x20:
            *orit = CUS_ORIT_M0F1;
        break;
        case 0x30:
            *orit = CUS_ORIT_M1F1;
        break;
    }
    return SUCCESS;
}


static int OS04B10_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    OS04B10_params *params = (OS04B10_params *)handle->private_data;
    switch(orit) {
        case CUS_ORIT_M0F0:
            mirror_reg[1].data = 0x01;
            mirror_reg[4].data = 0x05;
            mirror_reg[5].data = 0x00;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            mirror_reg[1].data = 0x00;
            mirror_reg[4].data = 0x05;
            mirror_reg[5].data = 0x00;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            mirror_reg[1].data = 0x03;
            mirror_reg[4].data = 0xac;
            mirror_reg[5].data = 0x05;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F1:
            mirror_reg[1].data = 0x02;
            mirror_reg[4].data = 0xac;
            mirror_reg[5].data = 0x05;
            params->ori_dirty = true;
        break;
    }
     return SUCCESS;
}

static int OS04B10_GetFPS(ss_cus_sensor *handle)
{
    OS04B10_params *params = (OS04B10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (vts_reg[1].data << 8) | (vts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int OS04B10_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts=0;
    OS04B10_params *params = (OS04B10_params *)handle->private_data;
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

    if ((params->expo.lines) > (params->expo.vts - 4))
        vts = params->expo.lines + 4;
    else
        vts = params->expo.vts;
    vts_reg[1].data = (vts >> 8) & 0x00ff;
    vts_reg[2].data = (vts >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}

#if 0
static int pCus_GetSensorCap(ss_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap) {
    if (cap)
        memcpy(cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP));
    else
        return FAIL;

    return SUCCESS;
}
#endif

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int OS04B10_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    OS04B10_params *params = (OS04B10_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
            if(params->ori_dirty){
                SensorRegArrayW((I2C_ARRAY*)mirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
                //sensor_if->BayerFmt(handle, handle->bayer_id);
                params->ori_dirty = false;
            }
            if(params->reg_dirty)
            {
                SensorReg_Write(0xfd,0x01);//page 1
                SensorReg_Write(0x01,0x00);//frame sync disable
                SensorRegArrayW((I2C_ARRAY*)expo_reg, sizeof(expo_reg)/sizeof(I2C_ARRAY));
                SensorRegArrayW((I2C_ARRAY*)gain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
                SensorRegArrayW((I2C_ARRAY*)vts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));
                SensorReg_Write(0x01,0x01);//frame sync enable

                params->reg_dirty = false;
            }
            break;
            default :
            break;
    }
    return SUCCESS;
}

static int OS04B10_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    int rc = SUCCESS;
    u32 lines = 0;
    //rc = SensorRegArrayR((I2C_ARRAY*)expo_reg, ARRAY_SIZE(expo_reg));

    lines  =  (u32)(expo_reg[2].data&0xff);
    lines |= (u32)(expo_reg[1].data&0xff)<<8;

    *us = (lines*Preview_line_period)/1000;

    return rc;
}

static int OS04B10_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    OS04B10_params *params = (OS04B10_params *)handle->private_data;

    lines=(u32)((1000*us+(Preview_line_period>>1))/Preview_line_period);
    if (lines < 2) lines = 2;
    if (lines >params->expo.vts - 4)
        vts = lines + 4;
    else
        vts=params->expo.vts;

    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
   // lines <<= 4;
    expo_reg[1].data = (lines>>8) & 0x00ff;
    expo_reg[2].data = (lines>>0) & 0x00ff;

    vts_reg[1].data = (vts >> 8) & 0x00ff;
    vts_reg[2].data = (vts >> 0) & 0x00ff;

    params->reg_dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int OS04B10_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    int again;
    //unsigned short temp_gain;

    again=gain_reg[0].data;

    *gain = again<<6;
    return SUCCESS;
}
#define MAX_A_GAIN 31744//(31*1024)
static int OS04B10_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    //OS04B10_params *params = (OS04B10_params *)handle->private_data;
    u32 input_gain = 0;
    u16 gain16;
    gain = (gain * SENSOR_MIN_GAIN + 512)>>10; // need to add min sat gain

    input_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=MAX_A_GAIN)
        gain=MAX_A_GAIN;

    /* A Gain */
    if (gain < 1024) {
        gain=1024;
    } else if ((gain >=1024) && (gain < 2048)) {
        gain = (gain>>6)<<6;
    } else if ((gain >=2048) && (gain < 4096)) {
        gain = (gain>>7)<<7;
    } else if ((gain >= 4096) && (gain < 8192)) {
        gain = (gain>>8)<<8;
    } else if ((gain >= 8192) && (gain < 16384)) {
        gain = (gain>>9)<<9;
    } else if ((gain >= 16384) && (gain < MAX_A_GAIN)) {
        gain = (gain>>10)<<10;
    } else {
        gain = MAX_A_GAIN;
    }

    gain16=(u16)(gain>>6);
    gain_reg[1].data = (gain16>>8)&0x01;//high bit
    gain_reg[2].data = gain16&0xff; //low byte

    //pr_info("[%s] set input gain/gain/AregH/AregL/DregH/DregL=%d/%d/0x%x/0x%x/0x%x/0x%x\n", __FUNCTION__, input_gain,gain,gain_reg[0].data,gain_reg[1].data,gain_reg[2].data,gain_reg[3].data);
    return SUCCESS;
}

#if 0
static int OS04B10_setCaliData_mingain(ss_cus_sensor* handle,  u32 gain) {
    handle->sat_mingain=gain;
    return SUCCESS;
}
#endif

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    OS04B10_params *params;
    int res;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    ////////////////////////////////////
    // private data allocation & init //
    ////////////////////////////////////
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }

    params = (OS04B10_params *)handle->private_data;

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"OS04B10_MIPI");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC;
    handle->bayer_id                                              = SENSOR_BAYERID;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_NONE;
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
    handle->pCus_sensor_poweron                                   = OS04B10_poweron;
    handle->pCus_sensor_poweroff                                  = OS04B10_poweroff;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    handle->mclk                                                  = Preview_MCLK_SPEED;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = os04b10_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = os04b10_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = os04b10_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = os04b10_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = os04b10_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = os04b10_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = os04b10_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = os04b10_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, os04b10_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = OS04B10_init;
    handle->pCus_sensor_GetVideoResNum                            = OS04B10_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = OS04B10_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = OS04B10_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = OS04B10_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = OS04B10_GetOrien;
    handle->pCus_sensor_SetOrien                                  = OS04B10_SetOrien;
    handle->pCus_sensor_GetFPS                                    = OS04B10_GetFPS;
    handle->pCus_sensor_SetFPS                                    = OS04B10_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = OS04B10_SetPatternMode;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = OS04B10_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = OS04B10_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = OS04B10_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = OS04B10_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = OS04B10_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/os04b10_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps;
    params->expo.fps                                              = 25;
    params->reg_dirty                                             = false;
    params->ori_dirty                                             = false;
    params->expo.lines                                            = 1000;
    params->gain                                                  = 1024;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  OS04B10,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            OS04B10_params
                         );
