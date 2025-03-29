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
#include "SC830AI_init_table.h"         /* Sensor initial table */
/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(SC830AI)

static EarlyInitSensorInfo_t _gSC830AIInfo =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_RG,
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
    .u32CropX       = 0,
    .u32CropY       = 0
};

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int SC830AIEarlyInitShutterAndFps( unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period 29629
unsigned int vts_30fps = 2250;

    unsigned char n;
    unsigned int lines = 0, half_lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x3e00, 0x00},
        {0x3e01, 0x5d},
        {0x3e02, 0x00},
        /*vts*/
        {0x320e, 0x05},
        {0x320f, 0xdc},

    };

    vts_30fps = (vts_30fps * 15000) / nFpsX1000;

    half_lines = (1000 * nShutterUs * 2) / Preview_line_period; // Preview_line_period in ns
    if(half_lines <= 3)
    {
        half_lines = 3;
    }
    if(half_lines > (2 * vts_30fps - 17))
    {
        vts = (half_lines + 18) / 2;
    }
    else
    {
        vts = vts_30fps;// 15fps ,meanwhile 0xa6a
    }
    lines = lines<<4;

    expo_reg[0].u16Data = (lines>>16) & 0x000f;
    expo_reg[1].u16Data = (lines>>8) & 0x00ff;
    expo_reg[2].u16Data = (lines>>0) & 0x00ff;

    expo_reg[3].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[4].u16Data = (vts >> 0) & 0x00ff;


    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

    _gSC830AIInfo.u32ShutterUs = nShutterUs;
    _gSC830AIInfo.u32FpsX1000 = nFpsX1000;
    return nNumRegs; //Return number of sensor registers to write
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
static unsigned int SC830AIEarlyInitGain( unsigned int u32GainX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAXGAIN     (32 * 3969)/1000// (32768*15 / 1000)//32XAGAIN 15XDGAIN
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    unsigned int ANA_gain = 1, DIG_gain = 1;
    unsigned char ANA_gain_reg = 0x40, DIG_gain_reg=0, DIG_Fine_gain_reg=0x80;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e09, 0x00},
    };

    _gSC830AIInfo.u32GainX1024 = u32GainX1024;

     if (u32GainX1024 <= 1024) {
        u32GainX1024 = 1024;
    } else if (u32GainX1024 > SENSOR_MAXGAIN*1024) {
        u32GainX1024 = SENSOR_MAXGAIN*1024;
    }

    if (u32GainX1024 < 2*1024)
    {
        ANA_gain = 1;        DIG_gain = 1;
        ANA_gain_reg = 0x40; DIG_gain_reg=0;
    }
    else if (u32GainX1024 < 4*1024)
    {
        ANA_gain = 2;        DIG_gain = 1;
        ANA_gain_reg = 0x48; DIG_gain_reg=0;
    }
    else if (u32GainX1024 < 8*1024)
    {
        ANA_gain = 4;        DIG_gain = 1;
        ANA_gain_reg = 0x49; DIG_gain_reg=0;
    }
    else if (u32GainX1024 < 16*1024)
    {
        ANA_gain = 8;        DIG_gain = 1;
        ANA_gain_reg = 0x4b; DIG_gain_reg=0;
    }
    else if (u32GainX1024 < 32*1024)
    {
        ANA_gain = 16;        DIG_gain = 1;
        ANA_gain_reg = 0x4f; DIG_gain_reg=0;
    }
    else if (u32GainX1024 < 64*1024)
    {
        ANA_gain = 32;       DIG_gain = 1;
        ANA_gain_reg = 0x5f; DIG_gain_reg=0;
    }
    else if (u32GainX1024 <= SENSOR_MAXGAIN * 1024)
    {
        ANA_gain = 32;       DIG_gain = 2;
        ANA_gain_reg = 0x5f; DIG_gain_reg=0x01;
    }

    DIG_Fine_gain_reg = u32GainX1024 / (ANA_gain*DIG_gain) / 8;

    gain_reg[2].u16Data = ANA_gain_reg; // 0x3e09
    gain_reg[1].u16Data = DIG_Fine_gain_reg; // 0x3e07
    gain_reg[0].u16Data = DIG_gain_reg; // 0x3e06

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

static unsigned int SC830AIEarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID      = _gSC830AIInfo.eBayerID;
        pSnrInfo->ePixelDepth   = _gSC830AIInfo.ePixelDepth;
        pSnrInfo->eIfBusType    = _gSC830AIInfo.eIfBusType;
        pSnrInfo->u32FpsX1000   = _gSC830AIInfo.u32FpsX1000;
        pSnrInfo->u32Width      = _gSC830AIInfo.u32Width;
        pSnrInfo->u32Height     = _gSC830AIInfo.u32Height;
        pSnrInfo->u32GainX1024  = _gSC830AIInfo.u32GainX1024;
        pSnrInfo->u32ShutterUs  = _gSC830AIInfo.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _gSC830AIInfo.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _gSC830AIInfo.u32GainShortX1024;
        pSnrInfo->u8ResId       = _gSC830AIInfo.u8ResId;
        pSnrInfo->u8HdrMode     = _gSC830AIInfo.u8HdrMode;
        pSnrInfo->u32TimeoutMs  = 200;
        pSnrInfo->u32CropX       = _gSC830AIInfo.u32CropX;
        pSnrInfo->u32CropY       = _gSC830AIInfo.u32CropY;
    }
    return 0;
}

/* Sensor EarlyInit implementation end*/
SENSOR_EARLYINIY_ENTRY_IMPL_END( SC830AI,
                                 Sensor_init_table,
                                 SC830AIEarlyInitShutterAndFps,
                                 SC830AIEarlyInitGain,
                                 SC830AIEarlyInitGetSensorInfo
                                );
