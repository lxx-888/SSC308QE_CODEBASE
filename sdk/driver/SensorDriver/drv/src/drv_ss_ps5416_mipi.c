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
#define _LPC_SUPPORT_ (1)
#define _PRE_ROLL_MODE_  1   // 1  SPI MODE,  0 DVP MODE

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>
#include <drv_sensor_init_table.h>
#include <PS5416_init_table.h>
//#include "drv_gpio.h"
//#include <gpio.h>
//#include <padmux.h>

#ifdef __cplusplus
}
#endif


SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(PS5416);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

//============================================
//#undef SENSOR_DBG
#define SENSOR_DBG 0
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif

////////////////////////////////////
// Sensor-If Info                 //
////////////////////////////////////
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM    (4)

#define SENSOR_IFBUS_TYPE       CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC_LINEAR  CUS_DATAPRECISION_12
#define SENSOR_DATAPREC_HDR     CUS_DATAPRECISION_10
#define SENSOR_BAYERID          CUS_BAYER_BG
#define SENSOR_RGBIRID          CUS_RGBIR_NONE

#if SNR_ORIT_INIT_VAL == ORIT_M0F0_VAL
#define SENSOR_ORIT             CUS_ORIT_M0F0
#elif SNR_ORIT_INIT_VAL == ORIT_M1F0_VAL
#define SENSOR_ORIT             CUS_ORIT_M1F0
#elif SNR_ORIT_INIT_VAL == ORIT_M0F1_VAL
#define SENSOR_ORIT             CUS_ORIT_M0F1
#elif SNR_ORIT_INIT_VAL == ORIT_M1F1_VAL
#define SENSOR_ORIT             CUS_ORIT_M1F1
#endif

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_SPEED   200000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8
#define SENSOR_I2C_ADDR    0x98 //ver B

////////////////////////////////////
// Sensor Signal                  //
////////////////////////////////////
#define SENSOR_PWDN_POL     CUS_CLK_POL_POS        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

////////////////////////////////////
// Sensor ID                      //
////////////////////////////////////
#undef SENSOR_NAME
#define SENSOR_NAME         ps5416
#define SNR_VER_ID_REG      0x0002
#define SNR_VER_ID_C        0x22
#define SNR_VER_ID_D        0x33

////////////////////////////////////
// Others                         //
////////////////////////////////////
#define SEL_OP_MODE_REG     0x0008
#define CUR_OP_MODE_REG     0x0009

#define PCLK                24      //24MHz
#define PIXEL_PER_LINE      576

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
typedef struct {
    // Modify it based on number of support resolution
    enum {RES_1 = 0, RES_END}mode;
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
} PS5416_INFO;

static PS5416_INFO ps5416_mipi_linear[] = {
    {RES_1, {2688, 1520, 5, 30},  {0, 0, 2688, 1520}, {"2688x1520@30fps LINEAR"}},
}, ps5416_mipi_hdr[] = {
    {RES_1, {2688, 1520, 5, 30},  {0, 0, 2688, 1520}, {"2688x1520@30fps HDR"}},
};

typedef struct {
    struct {
        u32 u32VtsMin;
        u32 u32VtsTarget;
        u32 u32VtsCur;              // Not used in HDR mode since AE cannot increase VTS in HDR mode.
        u32 u32PreviewLinePeriod;   // How long one line takes.
        u32 u32Fps;
        u32 u32LefMaxExpoLines;
        u32 u32SefMaxExpoLines;
        u32 u32LefExpoLines;
        u32 u32SefExpoLines;
        u32 u32LefGainIndex;
        u32 u32SefGainIndex;
    } expo;
    bool dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tMax_expo_reg[4];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tGain_reg_sef[2];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tExpo_reg_sef[2];
    CUS_CAMSENSOR_ORIT orient;
} ps5416_params;

static CUS_GAIN_GAP_ARRAY gain_gap_compensate[16] =
{
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
    {0x00, 0x00},
};

const static I2C_ARRAY gstOritReg =
{
    0x01CE, SNR_ORIT_INIT_VAL
};

const static I2C_ARRAY gastMaxExpoReg[] =
{
    {0x0127, 0x04},     // R_ExpLine_long_max[7:0]
    {0x0128, 0x0c},     // R_ExpLine_long_max[15:8]
    {0x0129, 0x74},     // R_ExpLine_short_max[7:0]
    {0x012A, 0x00},     // R_ExpLine_short_max[15:8]
};

const static I2C_ARRAY gastExpoReg[] =
{
    {0x014E, 0x3E},     // R_ExpLine_Stream[7:0]
    {0x014F, 0x06},     // R_ExpLine_Stream[15:8]
};

const static I2C_ARRAY gastExpoSefReg[] =
{
    {0x0152, 0x3E},     // R_ExpLine_short_Stream[7:0]
    {0x0153, 0x06},     // R_ExpLine_short_Stream[15:8]
};

const static I2C_ARRAY gastGainReg[] =
{
    {0x0150, 0x40},     // R_GainIndex_Stream[7:0]
    {0x0151, 0x00},     // R_GainIndex_Stream[9:8]
};

const static I2C_ARRAY gastGainSefReg[] =
{
    {0x0154, 0x40},     // R_GainIndex_short_Stream[7:0]
    {0x0155, 0x00},     // R_GainIndex_short_Stream[9:8]
};

const static I2C_ARRAY gastVtsReg[] =
{
    {0x011E, 0x40},     // R_Lpf[7:0]
    {0x011F, 0x06},     // R_Lpf[15:8]
};

const static I2C_ARRAY gastSnrIDReg[] =
{
    {0x0000, 0x16},     // SensorID[7:0]
    {0x0001, 0x54},     // SensorID[15:8]
};

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 u32ExpoUs);
static volatile u16 gu16SnrVerId = 0xFFFF;
static int pCus_SetAEUSecsHdrLef(ss_cus_sensor *handle, u32 u32ExpoUs);
static int pCus_SetAEUSecsHdrSef(ss_cus_sensor *handle, u32 u32ExpoUs);

#define SensorReg_Read(_reg,_data)  (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data) (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

static int cus_camsensor_release_handle(ss_cus_sensor *handle)
{
    if (handle && handle->private_data) {
        SENSOR_DMSG("[%s] release handle, handle %x, private data %x",
                __FUNCTION__,
                (int)handle,
                (int)handle->private_data);
        handle->private_data = NULL;
    }
    return SUCCESS;
}


#if _LPC_SUPPORT_
static u16 _ps5416_check_op_mode(ss_cus_sensor *handle,u8 u8Mode)
{
    volatile u16 u16SnrMode = 0;

    SensorReg_Read(CUR_OP_MODE_REG, &u16SnrMode);
    if(u16SnrMode != u8Mode)
        return (0xFF00 | u16SnrMode);
    return 0;
}

#define POINT_CONVERT_STREAMING_TO_PREROLL_H(x) ((((x)+12) >> 2) - 6)
#define POINT_CONVERT_PREROLL_TO_STREAMING_H(x) ((((x)+6) << 2) - 12)
#define POINT_CONVERT_STREAMING_TO_PREROLL_V(y) ((((y)+8) >> 2) - 2)
#define POINT_CONVERT_PREROLL_TO_STREAMING_V(y) ((((y)+2) << 2) - 8)
#define PREROLL_MODE_HSTART_MAX 162
#define PREROLL_MODE_VSTART_MAX 12

static u16 _ps5416_set_preroll_hvstart(ss_cus_sensor *handle)
{
    //u16 u16StmMdCropX = 0, u16StmMdCropY = 0, u16StmMdCropW = 648, u16StmMdCropH = 360;
    u16 u16StmMdCropX = 0, u16StmMdCropY = 0, u16StmMdCropW = PREROLL_MODE_HSIZE, u16StmMdCropH = PREROLL_MODE_VSIZE;
    u16 u16PrlMdVStart = 6, u16PrlMdHStart = 148;   // Make default center equal to the Optical center

    //No implementation,use default value if(!MHalLpc_GetStreamingModeCropping(&u16StmMdCropX, &u16StmMdCropY, &u16StmMdCropW, &u16StmMdCropH))
    {
        u16 u16StmMdCenterX = u16StmMdCropX + (u16StmMdCropW >> 1);
        u16 u16StmMdCenterY = u16StmMdCropY + (u16StmMdCropH >> 1);
        u16 u16PrlMdCenterH = POINT_CONVERT_STREAMING_TO_PREROLL_H(u16StmMdCenterX);
        u16 u16PrlMdCenterV = POINT_CONVERT_STREAMING_TO_PREROLL_V(u16StmMdCenterY);

        if(u16PrlMdCenterH < (PREROLL_MODE_HSIZE >> 1))
        {
            CamOsPrintf("[WARNING] %s: Target Preroll HStart is less than 0, overwrite it to 0\n", __func__);
            u16PrlMdHStart = 0;
        }
        else if(u16PrlMdCenterH > PREROLL_MODE_HSTART_MAX + (PREROLL_MODE_HSIZE >> 1))
        {
            CamOsPrintf("[WARNING] %s: Target Preroll HStart exceeds the max value, overwrite it to %u\n", __func__, PREROLL_MODE_HSTART_MAX);
            u16PrlMdHStart = PREROLL_MODE_HSTART_MAX;
        }
        else
        {
            u16PrlMdCenterH &= 0xFFFE;
            u16PrlMdHStart = u16PrlMdCenterH - (PREROLL_MODE_HSIZE >> 1);

            if((u16StmMdCenterX - POINT_CONVERT_PREROLL_TO_STREAMING_H(u16PrlMdCenterH)) >
                    (POINT_CONVERT_PREROLL_TO_STREAMING_H(u16PrlMdCenterH + 2) - u16StmMdCenterX))
            {
                u16PrlMdHStart += 2;
            }
        }

        if(u16PrlMdCenterV < (PREROLL_MODE_VSIZE >> 1))
        {
            CamOsPrintf("[WARNING] %s: Target Preroll VStart is less than 0, overwrite it to 0\n", __func__);
            u16PrlMdVStart = 0;
        }
        else if(u16PrlMdCenterV > PREROLL_MODE_VSTART_MAX + (PREROLL_MODE_VSIZE >> 1))
        {
            CamOsPrintf("[WARNING] %s: Target Preroll VStart exceeds the max value, overwrite it to %u\n", __func__, PREROLL_MODE_VSTART_MAX);
            u16PrlMdVStart = PREROLL_MODE_VSTART_MAX;
        }
        else
        {
            u16PrlMdCenterV &= 0xFFFE;
            u16PrlMdVStart = u16PrlMdCenterV - (PREROLL_MODE_VSIZE >> 1);

            if((u16StmMdCenterY - POINT_CONVERT_PREROLL_TO_STREAMING_V(u16PrlMdCenterV)) >
                    (POINT_CONVERT_PREROLL_TO_STREAMING_V(u16PrlMdCenterV + 2) - u16StmMdCenterY))
            {
                u16PrlMdVStart += 2;
            }
        }
    }

    // Preroll HStart = WOI_HSTART
    //         VStart = WOI_VSTART + 2*VSTART_SEL, where WOI is always equal to 0
    SensorReg_Write(WOI_HSTART_REG_LBYTE, u16PrlMdHStart & 0xFF);
    SensorReg_Write(WOI_HSTART_REG_HBYTE, u16PrlMdHStart >> 8);
    SensorReg_Write(VSTART_SEL_LBYTE, (u16PrlMdVStart >> 1) & 0xFF);
    SensorReg_Write(VSTART_SEL_HBYTE, (u16PrlMdVStart >> 1) >> 8);

    return 0;
}

static u16 _ps5416_switch_into_preroll_mode(ss_cus_sensor *handle)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;

    SensorReg_Write(0x0126, 0x00);      //R_DOL_mode=0
    SensorReg_Write(0x0105, 0x15);
    SensorReg_Write(0x0106, 0x0b);
    SensorReg_Write(0x00eb, 0x01);

    SensorReg_Write(params->tExpo_reg[0].reg, MIN_EXPO_LINE_COUNT & 0xFF);
    SensorReg_Write(params->tExpo_reg[1].reg, (MIN_EXPO_LINE_COUNT >> 8) & 0xFF);
    SensorReg_Write(0x0156, 0x03);      //gain_exp update
    SENSOR_MSLEEP(200);
    _ps5416_set_preroll_hvstart(handle);
    SensorReg_Write(SEL_OP_MODE_REG, 0x80 | OP_MODE_PREROLL);
    SENSOR_MSLEEP(200);
    return _ps5416_check_op_mode(handle,OP_MODE_PREROLL);
}

static u16 _ps5416_switch_into_streaming_mode_fix_patch(ss_cus_sensor *handle)
{
    u16 u16SnrMode = 0, i = 0;

    SensorReg_Write(0x00F4, 0x01);
    SensorReg_Write(0x00F4, 0x00);

    for(i=0; i<125; i++)
    {
        if(!(u16SnrMode = _ps5416_check_op_mode(handle, OP_MODE_STREAMING)))
            break;
        SENSOR_MSLEEP(2);
    }

    return i == 125 ? (0xFF00 | u16SnrMode) : 0;
}


#endif

#if _LPC_SUPPORT_
#if 0
static void pCus_Request_I2c(void)
{
    //can not use padmux get mode/pad since dts set none for pm/non-pm share this i2c1
    drv_gpio_pad_val_set(PAD_PM_MI2C1_SCL, PINMUX_FOR_I2C0_MODE_4);
    drv_gpio_pad_val_set(PAD_PM_MI2C1_SDA, PINMUX_FOR_I2C0_MODE_4);
    drv_gpio_pad_val_set(PAD_PM_SR1_RST, PINMUX_FOR_GPIO_MODE);
}

static void pCus_Release_I2c(void)
{
    //can not use padmux get mode/pad since dts set none for pm/non-pm share this i2c1
    drv_gpio_pad_val_set(PAD_PM_MI2C1_SCL, PINMUX_FOR_I2CM1_MODE_1);
    drv_gpio_pad_val_set(PAD_PM_MI2C1_SDA, PINMUX_FOR_I2CM1_MODE_1);
    //drv_gpio_pad_val_set(PAD_PM_SR1_RST, PINMUX_FOR_SR_RST_MODE_1);//pm should not touch reset,still control by non-pm
}
#endif
#if 0
static void pCus_PS5416_reset(u32 idx, u32 gpio_value)
{
    idx = idx;
    if(CUS_CLK_POL_NEG == gpio_value)
    {
        drv_gpio_set_low(PAD_SSI_RST);
    }
    else
    {
        drv_gpio_set_high(PAD_SSI_RST);
    }
}
#endif
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    CamOsPrintf("[%s] ", __FUNCTION__);

    /*PAD and CSI*/
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    //if(sensor_if->Reset(idx, SENSOR_RST_POL) >= 0)
    sensor_if->Reset(idx, SENSOR_RST_POL);
    //pCus_PS5416_reset(idx, SENSOR_RST_POL);
    SENSOR_UDELAY(1000);

    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !SENSOR_RST_POL);
    //pCus_PS5416_reset(idx, !SENSOR_RST_POL);

    if(sensor_if->MCLK(idx, 1, handle->mclk) >= 0)
    SENSOR_MSLEEP(2);

    SensorReg_Read(SNR_VER_ID_REG, &gu16SnrVerId);
    CamOsPrintf("%s: Sensor Version ID = 0x%02X\n", __func__, gu16SnrVerId);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    u16 u16SnrMode = 0, i = 0;

    CamOsPrintf("[%s] power low\n", __FUNCTION__);
    //sensor_if->Reset(idx, SENSOR_RST_POL);
    //SENSOR_UDELAY(100);
    //sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if((u16SnrMode = _ps5416_switch_into_preroll_mode(handle)))
    {
        CamOsPrintf("%s: Fail to switch into Pre-roll mode: %u\n", __func__, u16SnrMode & 0xFF);
        SensorReg_Write(0x00F4, 0x03);
        SensorReg_Write(0x00F4, 0x00);
        for(i=0; i<50; i++)
        {
            if(!_ps5416_check_op_mode(handle, OP_MODE_PREROLL))
                break;
            SENSOR_MSLEEP(20);
        }
        if(i == 50)
        {
            CamOsPrintf("%s: Try the fix patch but still fail to enter Pre-roll\n", __func__);

            SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
            //if(sensor_if->Reset(idx, SENSOR_RST_POL) >= 0)
            sensor_if->Reset(idx, SENSOR_RST_POL);
            //pCus_PS5416_reset(idx, SENSOR_RST_POL);
            SENSOR_UDELAY(1000);

            SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
            sensor_if->Reset(idx, !SENSOR_RST_POL);
            //pCus_PS5416_reset(idx, !SENSOR_RST_POL);
            SENSOR_MSLEEP(3);
            handle->pCus_sensor_init(handle);
            if((u16SnrMode = _ps5416_switch_into_preroll_mode(handle)))
            {
                CamOsPrintf("%s: Re-init sensor but still fail: %u\n", __func__, u16SnrMode & 0xFF);
                return FAIL;
            }
        }
    }
    SensorReg_Write(0x0105, 0x14);
    SensorReg_Write(0x0106, 0x0a);
    SensorReg_Write(0x00eb, 0x01);
    sensor_if->MCLK(idx, 0, handle->mclk);
    CamOsPrintf("[%s] power low finish\n", __FUNCTION__);

    params->dirty = false;
    //pCus_Release_I2c();

    return SUCCESS;
}
static int PS5416_WarmBoot(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    sensor_if->MCLK(idx, 1, handle->mclk);
    return SUCCESS;
}
// ssi to mipi set in ipl earlyinit
// only check whether ssi to mipi is success or not
// if still ssi, then run fix patch
static int PS5416_FixPatch(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    u16 u16ParallelIfPolar, u16SnrMode;

    SensorReg_Read(0x00AF, &u16ParallelIfPolar);
    CamOsPrintf("%s: Sensor polar = 0x%02X\n", __func__, u16ParallelIfPolar);
    // when read correct polar value, means sensor already init
    if((u16ParallelIfPolar & 0xFF) == 0x01)
    {
        SensorReg_Read(SNR_VER_ID_REG, &gu16SnrVerId);
        CamOsPrintf("%s: Sensor Version ID = 0x%02X\n", __func__, gu16SnrVerId);
        if(gu16SnrVerId != SNR_VER_ID_C && gu16SnrVerId != SNR_VER_ID_D)
            return FAIL;

        // if it is streaming, then switch ssi to mipi successfully
        if(!_ps5416_check_op_mode(handle, OP_MODE_STREAMING))
        {
            CamOsPrintf("[%s] finish 1", __FUNCTION__);
            return SUCCESS;
        }

        // if switch fail, then run fix patch
#if 0
        sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
        sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
        sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
        sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

        sensor_if->MCLK(idx, 1, handle->mclk);
        SENSOR_MSLEEP(1);
#endif

        CamOsPrintf("%s: Sensor driver fail to make PS5416 into streaming mode\n", __func__);
        if((u16SnrMode = _ps5416_switch_into_streaming_mode_fix_patch(handle)))
        {
            CamOsPrintf("%s: Try the fix patch but still fail to enter streaming mode: %u\n", __func__, u16SnrMode & 0xFF);
        }
        else
        {
            // recover for ssi to mipi successfully
            CamOsPrintf("[%s] finish 2", __FUNCTION__);
            return SUCCESS;
        }

        sensor_if->MCLK(idx, 0, handle->mclk);
    }
    else
    {
        // Haven't init sensor, HDR mode change or earlyinit was failed
    }
    return FAIL;

}
static int pCus_WarmBootInit(ss_cus_sensor *handle, u32 idx)
{
    PS5416_WarmBoot(handle, idx);
    PS5416_FixPatch(handle, idx);
    return SUCCESS;
}
#else
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    /*PAD and CSI*/
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    //if(sensor_if->Reset(idx, SENSOR_RST_POL) >= 0)
    sensor_if->Reset(idx, SENSOR_RST_POL);
    //pCus_PS5416_reset(idx, SENSOR_RST_POL);
    SENSOR_UDELAY(1000);

    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !SENSOR_RST_POL);
    //pCus_PS5416_reset(idx, !SENSOR_RST_POL);

    if(sensor_if->MCLK(idx, 1, handle->mclk) >= 0)
    SENSOR_MSLEEP(2);

    SensorReg_Read(SNR_VER_ID_REG, &gu16SnrVerId);
    CamOsPrintf("%s: Sensor Version ID = 0x%02X\n", __func__, gu16SnrVerId);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;

    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->Reset(idx, SENSOR_RST_POL);
    //pCus_PS5416_reset(idx, SENSOR_RST_POL);
    SENSOR_UDELAY(100);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);

    params->dirty = false;
    return SUCCESS;
}
#endif

static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    info->u8AEGainDelay       = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay    = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum     = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum  = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min       = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max       = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min    = handle->sensor_ae_info_cfg.u32AEShutter_min;
    info->u32AEShutter_step   = handle->sensor_ae_info_cfg.u32AEShutter_step;
    info->u32AEShutter_max    = handle->sensor_ae_info_cfg.u32AEShutter_max;
    return SUCCESS;
}

static int pCus_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
    volatile u16 hb = 0, lb = 0;

    SensorReg_Read(gastSnrIDReg[0].reg, &lb);
    SensorReg_Read(gastSnrIDReg[1].reg, &hb);

    if((lb != gastSnrIDReg[0].data) || (hb != gastSnrIDReg[1].data))
    {
        SENSOR_DMSG("%s: Get wrong sensor id: 0x%X\n", __func__, (hb << 8) | lb);
        return FAIL;
    }

    *id = (hb << 8) | lb;
    return SUCCESS;
}

static int pCus_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    u16 u16Val;
    if(mode == 1)
    {
        SENSOR_DMSG("[%s] Random Test\n", __FUNCTION__);
        u16Val = 0x03;
    }
    else if(mode == 2)
    {
        SENSOR_DMSG("[%s] Color Bar Test\n", __FUNCTION__);
        u16Val = 0x0b;
    }
    else
    {
        SENSOR_DMSG("[%s] Unknown mode\n", __FUNCTION__);
        return FAIL;
    }

    if(SensorReg_Write(0x040A, u16Val) != SUCCESS)
    {
        SENSOR_DMSG("[%s] FAIL!!\n", __FUNCTION__);
        return FAIL;
    }

    return SUCCESS;
}

static bool ps5416_SendInitCmd(ss_cus_sensor *handle, SENSOR_INIT_TABLE* pstInitTable, u32 u32TableSize, u8 bHdr)
{
    u32 i, u32RetryCnt;

    for(i = 0; i < u32TableSize; i++)
    {
        if(pstInitTable[i].reg == 0xffff)
            SENSOR_MSLEEP(pstInitTable[i].data);
        else
        {
            u32RetryCnt = 0;
            while(SensorReg_Write(pstInitTable[i].reg,pstInitTable[i].data) != SUCCESS)
            {
                u32RetryCnt++;
                SENSOR_DMSG("Sensor_init_table -> Retry %u...\n",u32RetryCnt);
                if(u32RetryCnt >= 10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
            SENSOR_DMSG("[%s:%d]Sensor init %d !!\n", __FUNCTION__, __LINE__, i);
        }
    }
    return SUCCESS;
}

static int pCus_init_ps5416_4lanes_linear(ss_cus_sensor *handle)
{

    CamOsPrintf("\n\n[%s]\n", __FUNCTION__);

    //if(gu16SnrVerId == SNR_VER_ID_C)
    //{
#if (PS5416_SNR_VER == PS5416_SNR_VER_C)
        return ps5416_SendInitCmd(handle,
                Sensor_init_table_ps5416_linear_C,
                ARRAY_SIZE(Sensor_init_table_ps5416_linear_C), 0);
#endif
    //}
    //else if(gu16SnrVerId == SNR_VER_ID_D)
    //{
#if (PS5416_SNR_VER == PS5416_SNR_VER_D)
        return ps5416_SendInitCmd(handle,
                Sensor_init_table_ps5416_linear_D,
                ARRAY_SIZE(Sensor_init_table_ps5416_linear_D), 0);
#endif
    //}

    //CamOsPrintf("%s: Unexpected Sensor version id %u\n", __func__, gu16SnrVerId);
    //return FAIL;
}

static int pCus_init_ps5416_4lanes_hdr(ss_cus_sensor *handle)
{
    SENSOR_DMSG("\n\n[%s]\n", __FUNCTION__);
    
#if _PS5416_HDR_SUPPORT_
    if(gu16SnrVerId == SNR_VER_ID_C)
    {
        return ps5416_SendInitCmd(handle,
                Sensor_init_table_ps5416_hdr_C,
                ARRAY_SIZE(Sensor_init_table_ps5416_hdr_C), 1);
    }
    else if(gu16SnrVerId == SNR_VER_ID_D)
    {
        return ps5416_SendInitCmd(handle,
                Sensor_init_table_ps5416_hdr_D,
                ARRAY_SIZE(Sensor_init_table_ps5416_hdr_D), 1);
    }

    CamOsPrintf("%s: Unexpected Sensor version id %u\n", __func__, gu16SnrVerId);
    return FAIL;
#else
    return SUCCESS;
#endif
}

static int pCus_GetVideoResNum(ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int pCus_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res)
    {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int pCus_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res)
    {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res)
        return FAIL;

    switch (res_idx)
    {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->data_prec                   = SENSOR_DATAPREC_LINEAR;
            handle->pCus_sensor_init            = pCus_init_ps5416_4lanes_linear;
            params->expo.u32VtsMin              = VTS_LINEAR_30FPS;
            params->expo.u32VtsTarget           = VTS_LINEAR_30FPS;
            params->expo.u32VtsCur              = VTS_LINEAR_30FPS;
            params->expo.u32PreviewLinePeriod   = PREVIEW_LINE_PERIOD_LINEAR_30FPS;
            
            handle->sensor_ae_info_cfg.u32AEShutter_min  = MIN_EXPO_LINE_COUNT * params->expo.u32PreviewLinePeriod;
            handle->sensor_ae_info_cfg.u32AEShutter_max  = 1000000000 / handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
            handle->sensor_ae_info_cfg.u32AEShutter_step = params->expo.u32PreviewLinePeriod;
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_SetVideoResHdr(ss_cus_sensor *handle, u32 res_idx)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res)
        return FAIL;

    switch (res_idx)
    {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->data_prec                   = SENSOR_DATAPREC_HDR;
            handle->pCus_sensor_init            = handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num ? pCus_init_ps5416_4lanes_hdr : NULL;
            params->expo.u32VtsMin              = VTS_HDR_30FPS;
            params->expo.u32VtsTarget           = VTS_HDR_30FPS;
            params->expo.u32PreviewLinePeriod   = PREVIEW_LINE_PERIOD_HDR_30FPS;

            handle->sensor_ae_info_cfg.u32AEShutter_min  = MIN_EXPO_LINE_COUNT * params->expo.u32PreviewLinePeriod;
            handle->sensor_ae_info_cfg.u32AEShutter_max  = params->expo.u32LefMaxExpoLines * params->expo.u32PreviewLinePeriod;
            handle->sensor_ae_info_cfg.u32AEShutter_step = params->expo.u32PreviewLinePeriod;
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;
    *orit = params->orient;
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;
    u16 u16OritVal;
    CUS_CAMSENSOR_ORIT eNewOrit;

    SENSOR_DMSG("[%s] orit: %d\n", __FUNCTION__, orit);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            u16OritVal = ORIT_M0F0_VAL;
            eNewOrit = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M0F1:
            u16OritVal = ORIT_M0F1_VAL;
            eNewOrit = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            u16OritVal = ORIT_M1F1_VAL;
            eNewOrit = CUS_ORIT_M1F1;
            break;
        case CUS_ORIT_M1F0: default:
            u16OritVal = ORIT_M1F0_VAL;
            eNewOrit = CUS_ORIT_M1F0;
            break;
    }

    if((SensorReg_Write(gstOritReg.reg, u16OritVal) != SUCCESS) ||
       (SensorReg_Write(0x00EB, 0x01) != SUCCESS))
    {
        SENSOR_DMSG("%s: FAIL\n", __func__);
        return FAIL;
    }
    params->orient = eNewOrit;
    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;
    return params->expo.u32Fps;
}

static bool ps5416_SetTargetVts(ss_cus_sensor *handle, u32 u32Fps)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(u32Fps < 1000)
        u32Fps *= 1000;

    if((u32Fps >= (min_fps * 1000)) && (u32Fps <= (max_fps * 1000)))
        params->expo.u32VtsTarget = (params->expo.u32VtsMin * max_fps * 1000) / u32Fps;
    else
    {
        SENSOR_DMSG("[%s] FPS %d out of range.\n", __FUNCTION__, u32Fps);
        return 1;
    }

    if(handle->interface_attr.attr_mipi.mipi_hdr_mode)
    {
        params->expo.u32Fps = u32Fps;
        params->tVts_reg[0].data = (params->expo.u32VtsTarget >> 0) & 0x00ff;
        params->tVts_reg[1].data = (params->expo.u32VtsTarget >> 8) & 0x00ff;
    }

    return 0;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;

    SENSOR_DMSG("[%s]\n", __FUNCTION__);

    if(ps5416_SetTargetVts(handle, fps))
        return FAIL;

    pCus_SetAEUSecs(handle, params->expo.u32LefExpoLines * params->expo.u32PreviewLinePeriod / 1000);

    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetFPSHdr(ss_cus_sensor *handle, u32 fps)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;

    SENSOR_DMSG("[%s]\n", __FUNCTION__);

    if(ps5416_SetTargetVts(handle, fps))
        return FAIL;

    if(HDR_MAX_EXPO_LINE(params->expo.u32VtsTarget)/17 > SEF_MAX_EXPO_LINE(params->expo.u32VtsTarget))
    {
        params->expo.u32SefMaxExpoLines = SEF_MAX_EXPO_LINE(params->expo.u32VtsTarget);
        params->expo.u32LefMaxExpoLines = LEF_MAX_EXPO_LINE(params->expo.u32VtsTarget);
    }
    else
    {
        params->expo.u32SefMaxExpoLines = HDR_MAX_EXPO_LINE(params->expo.u32VtsTarget)/17;
        params->expo.u32LefMaxExpoLines = HDR_MAX_EXPO_LINE(params->expo.u32VtsTarget) - params->expo.u32SefMaxExpoLines;
    }
    params->tMax_expo_reg[0].data = params->expo.u32LefMaxExpoLines & 0xFF;
    params->tMax_expo_reg[1].data = params->expo.u32LefMaxExpoLines >> 8;
    params->tMax_expo_reg[2].data = params->expo.u32SefMaxExpoLines & 0xFF;
    params->tMax_expo_reg[3].data = params->expo.u32SefMaxExpoLines >> 8;

    pCus_SetAEUSecsHdrLef(handle, params->expo.u32LefExpoLines * params->expo.u32PreviewLinePeriod / 1000);
    pCus_SetAEUSecsHdrSef(handle, params->expo.u32SefExpoLines * params->expo.u32PreviewLinePeriod / 1000);

    params->dirty = true;
    return SUCCESS;
}

static int pCus_AEStatusNotify(struct __ss_cus_sensor* handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(gastExpoReg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gastGainReg));
                SensorReg_Write(0x0156, 0x03);  // Gain & ExpLine update flag
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(gastVtsReg));
                SensorReg_Write(0x00EB, 0x01);  // Global update flag for updating VTS
                params->dirty = false;
            }
            break;
        default :
            break;
    }

    return SUCCESS;
}

static int pCus_AEStatusNotifyHdrSef(struct __ss_cus_sensor* handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    return SUCCESS;
}

static int pCus_AEStatusNotifyHdrLef(struct __ss_cus_sensor* handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    ps5416_params *params = (ps5416_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty)
            {
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(gastExpoReg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_sef, ARRAY_SIZE(gastExpoSefReg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gastGainReg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg_sef, ARRAY_SIZE(gastGainSefReg));
                SensorReg_Write(0x0156, 0x03);  // Gain & ExpLine update flag
                SensorRegArrayW((I2C_ARRAY*)params->tMax_expo_reg, ARRAY_SIZE(gastMaxExpoReg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(gastVtsReg));
                SensorReg_Write(0x00EB, 0x01);  // Global update flag
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
    ps5416_params* params = (ps5416_params*)handle->private_data;

    *us = (params->expo.u32LefExpoLines * params->expo.u32PreviewLinePeriod) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, u32Lines, *us);

    return SUCCESS;
}

static int pCus_GetAEUSecsHdrSef(ss_cus_sensor *handle, u32 *us)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;

    *us = (params->expo.u32SefExpoLines * params->expo.u32PreviewLinePeriod) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, u32Lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 u32ExpoUs)
{
    u32 u32Lines = 0;
    u32 max_fps  = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    ps5416_params* params = (ps5416_params*)handle->private_data;

    u32Lines = (u32)((1000 * u32ExpoUs) / params->expo.u32PreviewLinePeriod);

    if(u32Lines < MIN_EXPO_LINE_COUNT)
        u32Lines = MIN_EXPO_LINE_COUNT;

    if(u32Lines > params->expo.u32VtsTarget - 2)
        params->expo.u32VtsCur = u32Lines + 2;
    else
        params->expo.u32VtsCur = params->expo.u32VtsTarget;

    params->tExpo_reg[0].data = (u16)((u32Lines >> 0) & 0x00ff);
    params->tExpo_reg[1].data = (u16)((u32Lines >> 8) & 0x00ff);

    params->tVts_reg[0].data = (params->expo.u32VtsCur >> 0) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.u32VtsCur >> 8) & 0x00ff;

    params->expo.u32LefExpoLines = u32Lines;
    params->expo.u32Fps = (params->expo.u32VtsMin * max_fps * 1000) / params->expo.u32VtsCur;

    params->dirty = true;

    return SUCCESS;
}

static int pCus_SetAEUSecsHdrSef(ss_cus_sensor *handle, u32 u32ExpoUs)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;
    u32 u32SefLines = (u32)((1000 * u32ExpoUs) / params->expo.u32PreviewLinePeriod);
    u32 u32SefMaxLine = params->expo.u32SefMaxExpoLines > (params->expo.u32LefExpoLines - 1) ?
        (params->expo.u32LefExpoLines - 1) : params->expo.u32SefMaxExpoLines;

    if(u32SefLines > u32SefMaxLine)
        u32SefLines = u32SefMaxLine;
    if(u32SefLines < MIN_EXPO_LINE_COUNT)
        u32SefLines = MIN_EXPO_LINE_COUNT;

    params->tExpo_reg_sef[0].data = (u16)((u32SefLines >> 0) & 0x00ff);
    params->tExpo_reg_sef[1].data = (u16)((u32SefLines >> 8) & 0x00ff);

    params->expo.u32SefExpoLines = u32SefLines;

    params->dirty = true;

    return SUCCESS;
}

static int pCus_SetAEUSecsHdrLef(ss_cus_sensor *handle, u32 u32ExpoUs)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;
    u32 u32LefLines = (u32)((1000 * u32ExpoUs) / params->expo.u32PreviewLinePeriod);

    if(u32LefLines <= params->expo.u32SefExpoLines)
        u32LefLines = params->expo.u32SefExpoLines + 1;
    else if(u32LefLines > params->expo.u32LefMaxExpoLines)
        u32LefLines = params->expo.u32LefMaxExpoLines;

    params->tExpo_reg[0].data = (u16)((u32LefLines >> 0) & 0x00ff);
    params->tExpo_reg[1].data = (u16)((u32LefLines >> 8) & 0x00ff);

    params->expo.u32LefExpoLines = u32LefLines;

    params->dirty = true;

    return SUCCESS;
}

static int pCus_GetAEGain(ss_cus_sensor *handle, u32* pu32Gain1024x)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;

    *pu32Gain1024x = gastGainTable[params->expo.u32LefGainIndex].u64Gain1024x;

    SENSOR_DMSG("[%s] Gain: %u\n", __FUNCTION__, *pu32Gain1024x);
    return SUCCESS;
}

static int pCus_GetAEGainHdrSef(ss_cus_sensor *handle, u32* pu32Gain1024x)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;

    *pu32Gain1024x = gastGainTable[params->expo.u32SefGainIndex].u64Gain1024x;

    SENSOR_DMSG("[%s] Gain: %u\n", __FUNCTION__, *pu32Gain1024x);
    return SUCCESS;
}

static int ps5416_getClosestIndexInGainTable(ss_cus_sensor *handle, u32 u32Gain1024x)
{
    u32 i;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;

    if(u32Gain1024x < handle->sensor_ae_info_cfg.u32AEGain_min)
        u32Gain1024x = handle->sensor_ae_info_cfg.u32AEGain_min;
    else if(u32Gain1024x >= SNR_MAX_GAIN)
        u32Gain1024x = SNR_MAX_GAIN;

    Sensor_Gain_Linearity = gain_gap_compensate;

    for(i = 0; i < sizeof(gain_gap_compensate) / sizeof(CUS_GAIN_GAP_ARRAY); i++)
    {
        if(Sensor_Gain_Linearity[i].gain == 0)
            break;
        if((u32Gain1024x > Sensor_Gain_Linearity[i].gain) &&
           (u32Gain1024x < (Sensor_Gain_Linearity[i].gain + Sensor_Gain_Linearity[i].offset)))
        {
            u32Gain1024x = Sensor_Gain_Linearity[i].gain;
            break;
        }
    }

    for(i = 1; i < ARRAY_SIZE(gastGainTable); i++)
    {
        if(gastGainTable[i].u64Gain1024x > u32Gain1024x)
            break;
    }
    return i-1;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 u32Gain1024x)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;
    int i = ps5416_getClosestIndexInGainTable(handle, u32Gain1024x);

    params->tGain_reg[0].data = gastGainTable[i].u16RegVal & 0xFF;
    params->tGain_reg[1].data = (gastGainTable[i].u16RegVal >> 8) & 0x3;

    params->expo.u32LefGainIndex = i;
    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEGainHdrSef(ss_cus_sensor *handle, u32 u32Gain1024x)
{
    ps5416_params* params = (ps5416_params*)handle->private_data;
    int i = ps5416_getClosestIndexInGainTable(handle, u32Gain1024x);

    params->tGain_reg_sef[0].data = gastGainTable[i].u16RegVal & 0xFF;
    params->tGain_reg_sef[1].data = (gastGainTable[i].u16RegVal >> 8) & 0x3;

    params->expo.u32SefGainIndex = i;
    params->dirty = true;
    return SUCCESS;
}

static u32 pCus_TryAEGain(ss_cus_sensor *handle, u32 u32Gain1024x)
{
    int i = ps5416_getClosestIndexInGainTable(handle, u32Gain1024x);
    return gastGainTable[i].u64Gain1024x;
}

#define I2C_READ  (0x01)
#define I2C_WRITE  (0x02)

typedef struct stI2CRegData_s
{
    u16 u16Reg;
    u16 u16Data;
}stI2CRegData_t;

static int pCus_sensor_CustDefineFunction(ss_cus_sensor *handle, u32 cmd_id, void *param)
{

    switch(cmd_id)
    {
        case I2C_READ:
        {
            stI2CRegData_t *pRegData = (stI2CRegData_t *)param;
            SensorReg_Read(pRegData->u16Reg, &(pRegData->u16Data));
            break;
        }
        case I2C_WRITE:
        {
            stI2CRegData_t *pRegData = (stI2CRegData_t *)param;
            SensorReg_Write(pRegData->u16Reg, pRegData->u16Data);
            break;
        }
        default:
            SENSOR_DMSG("cmdid %d, unknow \n");
            break;
    }

    return SUCCESS;
}

static void _ps5416_handle_init(ss_cus_sensor* handle, bool bHdr, bool bSef)
{
    int i;
    ps5416_params *params = (ps5416_params *)handle->private_data;
    PS5416_INFO* pstPs5416Info = bHdr ? ps5416_mipi_hdr : ps5416_mipi_linear;
    ////////////////////////////////////
    //    i2c config                  //
    ////////////////////////////////////
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;

    ////////////////////////////////////
    //    mclk                        //
    ////////////////////////////////////
    handle->mclk                        = Preview_MCLK_SPEED;

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    handle->sif_bus     = SENSOR_IFBUS_TYPE;
    handle->data_prec   = bHdr ? SENSOR_DATAPREC_HDR : SENSOR_DATAPREC_LINEAR;
    handle->bayer_id    = SENSOR_BAYERID;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    params->orient      = SENSOR_ORIT;
    handle->interface_attr.attr_mipi.mipi_lane_num                  = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format               = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order                 = 0;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                  = bHdr ? CUS_HDR_MODE_DCG : CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num   = bHdr ? bSef : 0;

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (i = 0; i < RES_END; i++)
    {
        handle->video_res_supported.num_res                         = i + 1;
        handle->video_res_supported.res[i].u16width                 = pstPs5416Info[i].senif.preview_w;
        handle->video_res_supported.res[i].u16height                = pstPs5416Info[i].senif.preview_h;
        handle->video_res_supported.res[i].u16max_fps               = pstPs5416Info[i].senout.max_fps;
        handle->video_res_supported.res[i].u16min_fps               = pstPs5416Info[i].senout.min_fps;
        handle->video_res_supported.res[i].u16crop_start_x          = pstPs5416Info[i].senif.crop_start_X;
        handle->video_res_supported.res[i].u16crop_start_y          = pstPs5416Info[i].senif.crop_start_y;
        handle->video_res_supported.res[i].u16OutputWidth           = pstPs5416Info[i].senout.width;
        handle->video_res_supported.res[i].u16OutputHeight          = pstPs5416Info[i].senout.height;
        sprintf(handle->video_res_supported.res[i].strResDesc, pstPs5416Info[i].senstr.strResDesc);
    }

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = bHdr ? 2 : 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = bHdr ? 2 : 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SNR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SNR_MAX_GAIN;

    handle->pCus_sensor_release         = cus_camsensor_release_handle;
    handle->pCus_sensor_init            = bHdr ? (bSef ? pCus_init_ps5416_4lanes_hdr : NULL) : pCus_init_ps5416_4lanes_linear;
    handle->pCus_sensor_poweron         = (bHdr & !bSef) ? NULL : pCus_poweron;
    handle->pCus_sensor_poweroff        = (bHdr & !bSef) ? NULL : pCus_poweroff;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    handle->pCus_sensor_GetSensorID     = pCus_GetSensorID;

    handle->pCus_sensor_GetVideoResNum  = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes     = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes     = bHdr ? pCus_SetVideoResHdr : pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien        = pCus_GetOrien;
    handle->pCus_sensor_SetOrien        = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = bHdr ? (bSef ? NULL : pCus_SetFPSHdr) : pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode  = pCus_SetPatternMode;

    ////////////////////////////////////////////////////
    // CustDefineFunction
    ////////////////////////////////////////////////////
    handle->pCus_sensor_CustDefineFunction = pCus_sensor_CustDefineFunction;
    handle->pCus_sensor_WarmBootInit       = pCus_WarmBootInit;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify  = bHdr ? (bSef ? pCus_AEStatusNotifyHdrSef : pCus_AEStatusNotifyHdrLef) : pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = (bHdr & bSef) ? pCus_GetAEUSecsHdrSef : pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = bHdr ? (bSef ? pCus_SetAEUSecsHdrSef : pCus_SetAEUSecsHdrLef) : pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = (bHdr & bSef) ? pCus_GetAEGainHdrSef : pCus_GetAEGain;
    handle->pCus_sensor_TryAEGain       = pCus_TryAEGain;
    handle->pCus_sensor_SetAEGain       = (bHdr & bSef) ? pCus_SetAEGainHdrSef : pCus_SetAEGain;

}

static int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    ps5416_params *params;
    if (!handle)
    {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    params = (ps5416_params *)handle->private_data;

    memcpy(params->tGain_reg, gastGainReg, sizeof(gastGainReg));
    memcpy(params->tExpo_reg, gastExpoReg, sizeof(gastExpoReg));
    memcpy(params->tVts_reg, gastVtsReg, sizeof(gastVtsReg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    strcpy(handle->strSensorStreamName,"PS5416_MIPI_LINEAR");

    _ps5416_handle_init(handle, 0, 0);
    ////////////////////////////////////
    //  Private Data                  //
    ////////////////////////////////////
    params->expo.u32VtsMin              = VTS_LINEAR_30FPS;
    params->expo.u32VtsTarget           = VTS_LINEAR_30FPS;
    params->expo.u32VtsCur              = VTS_LINEAR_30FPS;
    params->expo.u32PreviewLinePeriod   = PREVIEW_LINE_PERIOD_LINEAR_30FPS;
    params->expo.u32LefExpoLines        = SNR_LEF_INIT_EXPO_LINES;
    params->expo.u32LefGainIndex        = SNR_LEF_INIT_GAIN_INDEX;
    params->expo.u32Fps                 = 30000;
    params->dirty                       = false;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    ps5416_params *params = NULL;
    if(!handle)
    {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }

    // private_data is shared between Lef and Sef handles, so we
    // don't need to init it again in cus_camsensor_init_handle_hdr_lef.
    params = (ps5416_params *)handle->private_data;

    memcpy(params->tVts_reg, gastVtsReg, sizeof(gastVtsReg));
    memcpy(params->tMax_expo_reg, gastMaxExpoReg, sizeof(gastMaxExpoReg));
    memcpy(params->tGain_reg, gastGainReg, sizeof(gastGainReg));
    memcpy(params->tGain_reg_sef, gastGainSefReg, sizeof(gastGainSefReg));
    memcpy(params->tExpo_reg, gastExpoReg, sizeof(gastExpoReg));
    memcpy(params->tExpo_reg_sef, gastExpoSefReg, sizeof(gastExpoSefReg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "PS5416_MIPI_HDR_SEF");

    _ps5416_handle_init(handle, 1, 1);

    ////////////////////////////////////
    //  Private Data                  //
    ////////////////////////////////////
    params->expo.u32VtsMin              = VTS_HDR_30FPS;
    params->expo.u32VtsTarget           = VTS_HDR_30FPS;
    params->expo.u32PreviewLinePeriod   = PREVIEW_LINE_PERIOD_HDR_30FPS;
    params->expo.u32LefMaxExpoLines     = LEF_MAX_EXPO_LINE(VTS_HDR_30FPS);
    params->expo.u32SefMaxExpoLines     = SEF_MAX_EXPO_LINE(VTS_HDR_30FPS);
    params->expo.u32LefExpoLines        = SNR_LEF_INIT_EXPO_LINES;
    params->expo.u32SefExpoLines        = MIN_EXPO_LINE_COUNT;
    params->expo.u32LefGainIndex        = SNR_LEF_INIT_GAIN_INDEX;
    params->expo.u32SefGainIndex        = SNR_SEF_INIT_GAIN_INDEX;
    params->expo.u32Fps                 = 30000;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    if(!handle)
    {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName, "PS5416_MIPI_HDR_LEF");

    _ps5416_handle_init(handle, 1, 0);

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(PS5416,
        cus_camsensor_init_handle_linear,
        cus_camsensor_init_handle_hdr_sef,
        cus_camsensor_init_handle_hdr_lef,
        ps5416_params);
