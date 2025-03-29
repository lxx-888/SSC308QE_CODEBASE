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

#include <drv_sensor_earlyinit_common.h>
#include <drv_sensor_earlyinit_method_1.h>
#include "IMX678_init_table.h"         /* Sensor initial table */

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(IMX678)

typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

static EarlyInitSensorInfo_t _gIMX678Info =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_GB,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 15000,
    .u32Width       = 3840,
    .u32Height      = 2160,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
    .u32TimeoutMs   = 100,
    .u32CropX       = 0,
    .u32CropY       = 0,
    .u8NumLanes     = 4
};

/*****************  Interface VER2 ****************/
static void _memcpy(void* dest, void* src, unsigned int len)
{
    unsigned int n;
    for(n=0; n<len; ++n)
        ((char*)dest)[n] = ((char*)src)[n];
}

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int IMX678EarlyInitShutterAndFpsLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period 17777                           // hts=33.333/1125=25629
#define vts_30fps  3750//1090                              //for 29.091fps @ MCLK=36MHz
#define max_fps  15000
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0, SHR0 = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x3052, 0x00},
        {0x3051, 0x00},
        {0x3050, 0x06},
        /*vts*/
        {0x302A, 0x00},
        {0x3029, 0x0E},
        {0x3028, 0xA6},
    };

    /*VTS*/
    if(nFpsX1000 >= max_fps && nFpsX1000 > 1000){
        nFpsX1000 = max_fps;
    }else if(nFpsX1000 >= (max_fps / 1000) && nFpsX1000 < 100){
        nFpsX1000 = max_fps / 1000;
    }

    if(nFpsX1000 <= max_fps && nFpsX1000 > 1000){
        vts = (vts_30fps * max_fps) / nFpsX1000;
    }else if(nFpsX1000 <= (max_fps / 1000) && nFpsX1000 < 100){
        vts = (vts_30fps * (max_fps / 1000)) / nFpsX1000;
    }

    /*Exposure time*/
    lines = (1000 * nShutterUsLef) / Preview_line_period;

    if(lines > vts)
        vts = lines + 8;

    SHR0 =  vts - lines;
    if (SHR0 <= 12 )  // 8+4
        SHR0 = 12 - 4;
    else
        SHR0 -= 4;

    expo_reg[0].u16Data = (SHR0>>16) & 0x0003;
    expo_reg[1].u16Data = (SHR0>>8) & 0x00ff;
    expo_reg[2].u16Data = (SHR0>>0) & 0x00ff;

    expo_reg[3].u16Data = (vts >> 16) & 0x0003;
    expo_reg[4].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[5].u16Data = (vts >> 0) & 0x00ff;

    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

    pHandle->tInfo.u32ShutterUs = nShutterUsLef;
    pHandle->tInfo.u32FpsX1000 = nFpsX1000;

    return nNumRegs; //Return number of sensor registers to write
}

static unsigned int IMX678EarlyInitShutterAndFpsEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    return IMX678EarlyInitShutterAndFpsLinear(pHandle, nFpsX1000, nShutterUsLef, nShutterUsSef, pRegs, nMaxNumRegs);
}

/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
typedef unsigned long long _u64;
typedef unsigned long _u32;
typedef unsigned short _u16;
extern _u64 intlog10(_u32 value);
extern _u64 EXT_log_2(_u32 value);

static unsigned int IMX678EarlyInitGainLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     (3981 * 1024)                  // max sensor again, a-gain * conversion-gain*d-gain
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    _u64 nGainDouble;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3070, 0x07},// LSB
        {0x3071, 0x00},// MSB
    };

    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;

    if(u32GainLefX1024 < SENSOR_MIN_GAIN)
        u32GainLefX1024 = SENSOR_MIN_GAIN;
    else if(u32GainLefX1024 >= SENSOR_MAX_GAIN)
        u32GainLefX1024 = SENSOR_MAX_GAIN;

    nGainDouble = 20*(intlog10(u32GainLefX1024)-intlog10(1024));

    gain_reg[0].u16Data = (_u16)(((nGainDouble*10)>> 24)/3);

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;
    return nNumRegs;
}

static unsigned int IMX678EarlyInitGainEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    return IMX678EarlyInitGainLinear(pHandle, u32GainLefX1024, u32GainSefX1024, pRegs, nMaxNumRegs);
}

static unsigned int IMX678EarlyInitSelStreamOnoff( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uStreamOnoff, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    I2cCfg_t stream_onoff_reg[] =
    {
        {0x3000, 0x00},  //Operating
        {0x3002, 0x00},  //Master mode start
    };
    if (uStreamOnoff == 0)
    {
        stream_onoff_reg[0].u16Data = 1;
        stream_onoff_reg[1].u16Data = 1;
    }

    /*copy result*/
    for(n=0;n<sizeof(stream_onoff_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)stream_onoff_reg)[n];
    }
    pHandle->tInfo.u8StreamOnoff = uStreamOnoff;
    return sizeof(stream_onoff_reg)/sizeof(stream_onoff_reg[0]);
}

unsigned int IMX678InitHandle( EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = IMX678EarlyInitGainEx;
    pHandle->fpShutterFpsParserEx = IMX678EarlyInitShutterAndFpsEx;
    //pHandle->fpSelResId           = IMX415EarlyInitSelResId;
   // pHandle->fpOrientParerEx      = IMX415EarlyInitSelOrientEx;
    pHandle->fpStreamOnoffParserEx= IMX678EarlyInitSelStreamOnoff;

    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = 0;
    pHandle->tCurCfg.pTable       = (void*) Sensor_init_table;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gIMX678Info, sizeof(_gIMX678Info));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;
    return 0;
}

SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2( IMX678, IMX678InitHandle);

