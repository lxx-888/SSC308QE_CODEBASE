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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(SP2309);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_CHANNEL_MODE            CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL   CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
#define ENABLE            1
#define DISABLE           0
#undef SENSOR_DBG
#define SENSOR_DBG        0

#define DEBUG_INFO        0

#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) printf(args)
#elif SENSOR_DBG == 0
#define SENSOR_DMSK(args...) //printk(args)
#endif
///////////////////////////////////////////////////////////////
//          @@@                                              //
//         @ @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//          @@@@                                             //
//                                                           //
//      Start Step 1 --  show preview on LCM                 //
//                                                           //
//  Fill these #define value and table with correct settings //
//      camera can work and show preview on LCM              //
//                                                           //
///////////////////////////////////////////////////////////////

////////////////////////////////////
// Sensor-If Info                 //
////////////////////////////////////
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM        2                   // SP2309 Linear mode supports MIPI 2/4 Lane
#define SENSOR_MIPI_LANE_NUM_DOL    2                   // (hdr_lane_num)//SP2309 DOL mode supports MIPI 4 Lane

#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  // CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
#define SENSOR_BAYERID              CUS_BAYER_RG        // 0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M1F1       // CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ   // CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x78               // I2C slave address
#define SENSOR_I2C_SPEED             200000             // 200000 //300000 //240000                  //I2C speed, 60000~320000
#define SENSOR_I2C_LEGACY            I2C_NORMAL_MODE    // usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT               I2C_FMT_A8D8       // CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

////////////////////////////////////
// Sensor Signal                  //
////////////////////////////////////
#define SENSOR_PWDN_POL              CUS_CLK_POL_NEG     // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL               CUS_CLK_POL_NEG     // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG
                                                         // VSYNC/HSYNC POL can be found in data sheet timing diagram,
                                                         // Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL             CUS_CLK_POL_POS     // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL             CUS_CLK_POL_POS     // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL              CUS_CLK_POL_POS     // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

////////////////////////////////////
// Sensor ID                      //
////////////////////////////////////

#undef SENSOR_NAME
#define SENSOR_NAME     SP2309

////////////////////////////////////
// Image Info                     //
////////////////////////////////////

#define VTS_30FPS                   1205
#define PREVIEW_LINE_PERIOD_30FPS   27662 // (1/30)/VTS_30FPS = 0.00002766251 sec

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
}sp2309_mipi_linear[] = {
    {LINEAR_RES_1, {1932, 1090, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},
};

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (32 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
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

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);

typedef struct {
    struct {
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    struct {
        u32 vts_min;                // What the vts is set in init table. The min vts which a good frame requires.
        u32 target_vts;             // What the vts should be according to the fps.
        u32 cur_vts;                // What the vts is actually. The current vts can be greater than the target vts because the large expo time.
        u32 preview_line_period;    // How long one line takes.
        u32 fps;
        u32 expo_time_us;
    } expo;
    bool dirty;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[2];
} sp2309_params;

typedef struct {
    u64 total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

const static I2C_ARRAY Sensor_init_table_2lane_2m30fps[] =
{
    // 1080p 30fps data format raw10
    {0xfd, 0x00},
    {0x36, 0x01},
    {0xfd, 0x00},
    {0x36, 0x00},
    {0xfd, 0x00},
    {0x20, 0x00},
    {0xFF, 0x14},   //delay 14ms
    {0xfd, 0x00},
    {0x41, 0x05},
    {0xfd, 0x01},
    {0x03, 0x01},   // Expo time high byte (unit: line)
    {0x04, 0x69},   // Expo time low byte  (unit: line)
    {0x06, 0x58},
    {0x0a, 0xa0},
    {0x0e, 0x04},   // VTS high byte
    {0x0f, 0xb5},   // VTS low byte
    {0x24, 0x40},
    {0x01, 0x01},
    {0x12, 0x01},
    {0x11, 0x4f},
    {0x19, 0xa0},
    {0xd5, 0x44},
    {0xd6, 0x01},
    {0xd7, 0x19},
    {0x16, 0x38},
    {0x1d, 0xa1},
    {0x1f, 0x23},
    {0x21, 0x05},
    {0x25, 0x0f},
    {0x27, 0x46},
    {0x2a, 0x03},
    {0x2c, 0x05},
    {0x2b, 0x22},
    {0x20, 0x08},
    {0x38, 0x10},
    {0x45, 0xce},
    {0x51, 0x2a},
    {0x52, 0x2a},
    {0x55, 0x15},
    {0x57, 0x20},
    {0x66, 0x58},
    {0x68, 0x50},
    {0x71, 0xf0},
    {0x72, 0x25},
    {0x73, 0x30},
    {0x74, 0x25},
    {0x77, 0x02},
    {0x79, 0x01},
    {0x7a, 0x6f},
    {0x8a, 0x11},
    {0x8b, 0x11},
    {0xb1, 0x01},
    {0xc4, 0x7a},
    {0xc5, 0x7a},
    {0xc6, 0x7a},
    {0xc7, 0x7a},
    {0xf0, 0x40},
    {0xf1, 0x40},
    {0xf2, 0x40},
    {0xf3, 0x40},
    {0xf4, 0x08},
    {0xf7, 0xf7},
    {0xfe, 0xf7},
    {0x48, 0xf7},
    {0xfa, 0x10},
    {0xfb, 0x58},
    {0xb1, 0x03},
    {0xfd, 0x02},
    {0x34, 0xff},
    {0xa0, 0x00},
    {0xa1, 0x08},
    {0xa2, 0x04},
    {0xa3, 0x38},
    {0xa4, 0x00},
    {0xa5, 0x08},
    {0xa6, 0x07},
    {0xa7, 0x80},
    {0xfd, 0x01},
    {0x8e, 0x07},
    {0x8f, 0x80},
    {0x90, 0x04},
    {0x91, 0x38},
    {0x01, 0x01},
    {0xfd, 0x01},
    {0x3F, 0x03},
	{0xfd, 0x01},
};

const static I2C_ARRAY mirr_flip_table[] =
{
    {0x3F, 0x00},
    {0x3F, 0x01},
    {0x3F, 0x02},
    {0x3F, 0x03},
};

const static Gain_ARRAY gain_table[] = {
    {10000  ,0x10},
    {10625  ,0x11},
    {11250  ,0x12},
    {11875  ,0x13},
    {12500  ,0x14},
    {13125  ,0x15},
    {13750  ,0x16},
    {14375  ,0x17},
    {15000  ,0x18},
    {15625  ,0x19},
    {16250  ,0x1a},
    {16875  ,0x1b},
    {17500  ,0x1c},
    {18125  ,0x1d},
    {18750  ,0x1e},
    {19375  ,0x1f},
    {20000  ,0x20},
    {21250  ,0x22},
    {22500  ,0x24},
    {23750  ,0x26},
    {25000  ,0x28},
    {26250  ,0x2a},
    {27500  ,0x2c},
    {28750  ,0x2e},
    {30000  ,0x30},
    {31250  ,0x32},
    {32500  ,0x34},
    {33750  ,0x36},
    {35000  ,0x38},
    {36250  ,0x3a},
    {37500  ,0x3c},
    {38750  ,0x3e},
    {40000  ,0x40},
    {42500  ,0x44},
    {45000  ,0x48},
    {47500  ,0x4c},
    {50000  ,0x50},
    {52500  ,0x54},
    {55000  ,0x58},
    {57500  ,0x5c},
    {60000  ,0x60},
    {62500  ,0x64},
    {65000  ,0x68},
    {67500  ,0x6c},
    {70000  ,0x70},
    {72500  ,0x74},
    {75000  ,0x78},
    {77500  ,0x7c},
    {80000  ,0x80},
    {85000  ,0x88},
    {90000  ,0x90},
    {95000  ,0x98},
    {100000 ,0xa0},
    {105000 ,0xa8},
    {110000 ,0xb0},
    {115000 ,0xb8},
    {120000 ,0xc0},
    {125000 ,0xc8},
    {130000 ,0xd0},
    {135000 ,0xd8},
    {140000 ,0xe0},
    {145000 ,0xe8},
    {150000 ,0xf0},
    {155000 ,0xf8},
};

const static I2C_ARRAY expo_reg[] = {
    {0x03, 0x01},
    {0x04, 0x00},
};
const static I2C_ARRAY gain_reg[] = {
    {0x24, 0x00},
    {0x38, 0x10},
    {0x39, 0x08},
};
const static I2C_ARRAY vts_reg[] = {

    {0x0D, 0x10},
    {0x0E, 0x04},
    {0x0F, 0xb5},
    //{0x0D, 0x01}, //test pettern
};

I2C_ARRAY Current_Mirror_Flip_Tbl[] = {
     {0x3F, 0x03},    // bit[1:0]
};
/////////////////////////////////////////////////////////////////
//       @@@@@@@                                               //
//           @@                                                //
//          @@                                                 //
//          @@@                                                //
//       @   @@                                                //
//        @@@@                                                 //
//                                                             //
//      Step 3 --  complete camera features                    //
//                                                             //
//  camera set EV, MWB, orientation, contrast, sharpness       //
//   , saturation, and Denoise can work correctly.             //
//                                                             //
/////////////////////////////////////////////////////////////////

/////////// function definition ///////////////////
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg), _reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg), _reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg), (_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg), (_reg),(_len)))

/////////////////// sensor hardware dependent //////////////


/*******I5/I6 Support MCLK List*******
 *    CUS_CMU_CLK_27MHZ,
 *    CUS_CMU_CLK_21P6MHZ,
 *    CUS_CMU_CLK_12MHZ,
 *    CUS_CMU_CLK_5P4MHZ,
 *    CUS_CMU_CLK_36MHZ,
 *    CUS_CMU_CLK_54MHZ,
 *    CUS_CMU_CLK_43P2MHZ,
 *    CUS_CMU_CLK_61P7MHZ,
 *    CUS_CMU_CLK_72MHZ,
 *    CUS_CMU_CLK_48MHZ,
 *    CUS_CMU_CLK_24MHZ,
 *    CUS_CMU_CLK_37P125MHZ,
 ******End of Support MCLK List*******/
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    sp2309_params *params = (sp2309_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = (params->expo.preview_line_period * 9);
    info->u32AEShutter_step                  = params->expo.preview_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSK("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL)
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);

    SENSOR_MSLEEP(5);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_UDELAY(6000);    // T3, Delay from sensor power up to Reset pull up need to be longer than 4 ms.
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_UDELAY(7000);    // T5, Delay from sensor power up to I2C initialization need to be longer than 5 ms.

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSK("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL)
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);

    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    return SUCCESS;
}

/////////////////// Check Sensor Product ID /////////////////////////
//Get and check sensor ID

static int sp2309_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    sp2309_params *params = (sp2309_params *)handle->private_data;
    switch(mode)
    {
        case 1:
            params->tVts_reg[0].data |= 0x01; //enable
            break;
        case 0:
            params->tVts_reg[0].data &= 0xFE; //disable
            break;
        default:
            params->tVts_reg[0].data &= 0xFE; //disable
            break;
    }

    SensorReg_Write(0xfd,0x01);
    if(SensorReg_Write(params->tVts_reg[0].reg, params->tVts_reg[0].data) != SUCCESS)
    {
        SENSOR_DMSK("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
        return FAIL;
    }
    return SUCCESS;
}

static int pCus_init_mipi2lane_2m30fps_linear(ss_cus_sensor *handle)
{
    int i,cnt=0, page=0, reg, data;
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2lane_2m30fps);i++)
    {
        if(Sensor_init_table_2lane_2m30fps[i].reg==0xff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2lane_2m30fps[i].data);
        }
        else
        {
            cnt = 0;
            reg = Sensor_init_table_2lane_2m30fps[i].reg;
            data = Sensor_init_table_2lane_2m30fps[i].data;
            while(SensorReg_Write(reg, data) != SUCCESS && !(!page && reg == 0x20))
            {
                cnt++;
                if(cnt>=10)
                {
                    return FAIL;
                }
            }
            if(reg == 0xfd)
                page = data;
        }
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

    if (res_idx >= num_res)
        return FAIL;

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int pCus_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res)
        return FAIL;

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    sp2309_params *params = (sp2309_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res)
        return FAIL;

    switch (res_idx)
    {
        case 0:
            handle->video_res_supported.ulcur_res   = 0;
            handle->data_prec                       = CUS_DATAPRECISION_10;
            handle->pCus_sensor_init                = pCus_init_mipi2lane_2m30fps_linear;
            params->expo.vts_min                    = VTS_30FPS;
            params->expo.target_vts                 = VTS_30FPS;
            params->expo.cur_vts                    = VTS_30FPS;
            params->expo.fps                        = 30;
            params->expo.preview_line_period        = PREVIEW_LINE_PERIOD_30FPS;
            params->expo.expo_time_us               = PREVIEW_LINE_PERIOD_30FPS * 0x169; // 0x169 comes from init table
            break;
        default:
            SENSOR_DMSK("[WARN]Set resolution fail in error parameters\n");
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    switch(Current_Mirror_Flip_Tbl[0].data)
    {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
            break;
        case 0x01:
            *orit = CUS_ORIT_M1F0;
            break;
        case 0x02:
            *orit = CUS_ORIT_M0F1;
            break;
        case 0x03:
            *orit = CUS_ORIT_M1F1;
            break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    SENSOR_DMSK("[%s] orit: %d\n", __FUNCTION__, orit);
    SensorReg_Write(0xfd,0x01);//page 1
    switch(orit)
    {
        case CUS_ORIT_M0F0:

            SensorReg_Write(mirr_flip_table[0].reg,mirr_flip_table[0].data);
            Current_Mirror_Flip_Tbl[0].reg = mirr_flip_table[0].reg;
            Current_Mirror_Flip_Tbl[0].data = mirr_flip_table[0].data;
            break;
        case CUS_ORIT_M1F0:
            SensorReg_Write(mirr_flip_table[1].reg,mirr_flip_table[1].data);
            Current_Mirror_Flip_Tbl[0].reg = mirr_flip_table[1].reg;
            Current_Mirror_Flip_Tbl[0].data = mirr_flip_table[1].data;
            break;
        case CUS_ORIT_M0F1:
            SensorReg_Write(mirr_flip_table[2].reg,mirr_flip_table[2].data);
            Current_Mirror_Flip_Tbl[0].reg = mirr_flip_table[2].reg;
            Current_Mirror_Flip_Tbl[0].data = mirr_flip_table[2].data;
            break;
        case CUS_ORIT_M1F1:
            SensorReg_Write(mirr_flip_table[3].reg,mirr_flip_table[3].data);
            Current_Mirror_Flip_Tbl[0].reg = mirr_flip_table[3].reg;
            Current_Mirror_Flip_Tbl[0].data = mirr_flip_table[3].data;
            break;
        default :
            SensorReg_Write(mirr_flip_table[0].reg,mirr_flip_table[0].data);
            Current_Mirror_Flip_Tbl[0].reg = mirr_flip_table[0].reg;
            Current_Mirror_Flip_Tbl[0].data = mirr_flip_table[0].data;
            break;
    }
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    sp2309_params *params = (sp2309_params *)handle->private_data;
    return params->expo.fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    sp2309_params *params = (sp2309_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    SENSOR_DMSK("[%s]\n", __FUNCTION__);

    if(fps >= min_fps && fps <= max_fps)
    {
        params->expo.target_vts = (params->expo.vts_min * max_fps) / fps;
    }
    else if((fps >= (min_fps * 1000)) && (fps <= (max_fps * 1000)))
    {
        params->expo.target_vts = (params->expo.vts_min * max_fps * 1000) / fps;
    }
    else
    {
        SENSOR_DMSK("[%s] FPS %d out of range.\n", __FUNCTION__, fps);
        return FAIL;
    }
    pCus_SetAEUSecs(handle, params->expo.expo_time_us);

    params->dirty = true;
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    sp2309_params *params = (sp2309_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty)
            {
                SensorReg_Write(0xfd,0x01);
                SensorReg_Write(0x01,0x00);//frame sync disable
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorReg_Write(0x01,0x01);//frame sync enable
                params->dirty = false;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    u32 lines = 0;
    sp2309_params *params = (sp2309_params *)handle->private_data;
    lines  =  (u32)(params->tExpo_reg[1].data&0xff);
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;

    *us = (lines * params->expo.preview_line_period)/1000;

    SENSOR_DMSK("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    sp2309_params *params = (sp2309_params *)handle->private_data;
    params->expo.expo_time_us = us;

    lines = (u32)( (1000*us) / params->expo.preview_line_period);

    if (lines < 2)
        lines = 2;

    if (lines > params->expo.target_vts - 1)
        params->expo.cur_vts = lines + 1;
    else
        params->expo.cur_vts = params->expo.target_vts;

    params->tExpo_reg[0].data =(u16)((lines >> 8) & 0x00ff);
    params->tExpo_reg[1].data =(u16)((lines >> 0) & 0x00ff);

    params->tVts_reg[1].data = (params->expo.cur_vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.cur_vts >> 0) & 0x00ff;

    params->expo.fps = (params->expo.cur_vts * max_fps * 1000) / params->expo.vts_min;

    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    int again = 0;
    sp2309_params *params = (sp2309_params *)handle->private_data;

    again=params->tGain_reg[0].data;

    *gain = again<<6;
    SENSOR_DMSK("[%s] Gain: %d \n", __FUNCTION__, (again<<6));
    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    sp2309_params *params = (sp2309_params *)handle->private_data;
    u8 Dgain = 1;
    u16 gain8 = 0;
    u32 i = 0;
    u64 gain_double = 0, total_gain_double = 0;

    gain = (gain * SENSOR_MIN_GAIN + 512)>>10; // need to add min sat gain

    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;


    gain_double = (u64)gain;
    total_gain_double = (gain_double*10000)/1024;

    if(total_gain_double > 155000)
    {
        if(total_gain_double / 2 > 155000)
            Dgain = 3;
        else
            Dgain = 2;
        total_gain_double /= Dgain;
    }

    for(i=1; i<ARRAY_SIZE(gain_table); i++)
    {
        if(gain_table[i].total_gain > total_gain_double)
        {
            gain8 = (gain_table[i].total_gain - total_gain_double > total_gain_double - gain_table[i-1].total_gain) ? gain_table[i-1].reg_val : gain_table[i].reg_val;
            break;
        }
        else if(i == ARRAY_SIZE(gain_table)-1)
        {
            gain8 = gain_table[i].reg_val;
            break;
        }
    }

    params->tGain_reg[0].data = gain8 & 0xFF;
    switch(Dgain)
    {
        case 1:
            params->tGain_reg[2].data = 0x08;
            break;
        case 2:
            params->tGain_reg[2].data = 0x10;
            break;
        case 3:
            params->tGain_reg[2].data = 0x18;
            break;
    }

    params->dirty = true;
    return SUCCESS;
}

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    sp2309_params *params;
    int res;

    if (!handle)
    {
        SENSOR_DMSK("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSK("[%s]", __FUNCTION__);
    ////////////////////////////////////
    // private data allocation & init //
    ////////////////////////////////////
    if (handle->private_data == NULL)
    {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }

    params = (sp2309_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"SP2309_MIPI");

    ////////////////////////////////////
    //    i2c config                  //
    ////////////////////////////////////
    handle->i2c_cfg.mode          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed         = SENSOR_I2C_SPEED;

    ////////////////////////////////////
    //    mclk                        //
    ////////////////////////////////////
    handle->mclk                  = Preview_MCLK_SPEED;

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    handle->sif_bus               = SENSOR_IFBUS_TYPE;
    handle->data_prec             = SENSOR_DATAPREC;
    handle->bayer_id              = SENSOR_BAYERID;
    handle->RGBIR_id              = SENSOR_RGBIRID;
    handle->interface_attr.attr_mipi.mipi_lane_num    = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order   = 0;                        //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode    = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;            //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < LINEAR_RES_END; res++)
    {
        handle->video_res_supported.num_res = res + 1;
        handle->video_res_supported.res[res].u16width         = sp2309_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = sp2309_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = sp2309_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = sp2309_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = sp2309_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = sp2309_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = sp2309_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = sp2309_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, sp2309_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    handle->pCus_sensor_init            = pCus_init_mipi2lane_2m30fps_linear;
    handle->pCus_sensor_poweron         = pCus_poweron;
    handle->pCus_sensor_poweroff        = pCus_poweroff;
    handle->pCus_sensor_GetVideoResNum  = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes     = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes     = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode  = sp2309_SetPatternMode;


    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    params->expo.vts_min                = VTS_30FPS;
    params->expo.target_vts             = VTS_30FPS;
    params->expo.cur_vts                = VTS_30FPS;
    params->expo.preview_line_period    = PREVIEW_LINE_PERIOD_30FPS;
    params->expo.expo_time_us           = PREVIEW_LINE_PERIOD_30FPS * 0x169; // 0x169 comes from init table
    params->expo.fps                    = 30;
    params->dirty                       = false;
    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (params->expo.preview_line_period * 9);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/sp2309_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = (params->expo.preview_line_period);
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(SP2309,
                            cus_camsensor_init_handle_linear,
                            NULL,
                            NULL,
                            sp2309_params
                         );
