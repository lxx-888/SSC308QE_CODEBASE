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
#include "IMX664_init_table.h"         /* Sensor initial table */

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(IMX664)

typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

static EarlyInitSensorInfo_t _gIMX664Info =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_RG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_12,
    .u32FpsX1000    = 30000,
    .u32Width       = 2688,
    .u32Height      = 1520,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
};

#define PREVIEW_LINE_PERIOD 10101
#define VTS_30FPS  3300

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int IMX664EarlyInitShutterAndFps( unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period 10101
#define vts_30fps  3300
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    int SHR0=0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x3052, 0x00},
        {0x3051, 0x00},
        {0x3050, 0x00},
        /*vts*/
        {0x302a, 0x00},
        {0x3029, 0x0C},
        {0x3028, 0xE4},
    };

    lines = (1000*nShutterUs)/Preview_line_period;
    lines = (lines>>1)<<1;

    /* SHR0: 8 to vts -2, multiple of 2 */
    vts =  (VTS_30FPS*30000)/nFpsX1000;

    if(nFpsX1000<1000)    //for old method
        vts = (VTS_30FPS*30)/nFpsX1000;

    if (lines <= 2) lines = 2;
    if (lines > vts - 8) {
        vts = lines + 8;
    }
    //else
    //    vts = VTS_30FPS;
    SHR0 =  vts - lines;

#if 0
    /*VTS*/
    vts =  (vts_30fps*30000)/nFpsX1000;

    if(nFpsX1000<1000)    //for old method
        vts = (vts_30fps*30)/nFpsX1000;

    /*Exposure time*/
    lines = (1000*nShutterUs)/Preview_line_period;

    if(lines>vts-2)
        vts = lines +2;

    lines = vts-lines-1;
#endif

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

    _gIMX664Info.u32ShutterUs = nShutterUs;
    _gIMX664Info.u32FpsX1000 = nFpsX1000;

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
extern _u64 intlog10(_u32 value);
extern _u64 EXT_log_2(_u32 value);
static unsigned int IMX664EarlyInitGain( unsigned int u32GainX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (   1 * 1024)
#define SENSOR_MAX_GAIN      (3981 * 1024)                  // max sensor again, a-gain * conversion-gain*d-gain
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    _u64 nGainDouble;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3070, 0x2A},//low bit
        {0x3071, 0x00},//hcg mode,bit 4
        //{0x3016, 0x08},//
    };

    _gIMX664Info.u32GainX1024 = u32GainX1024;

    if(u32GainX1024 < SENSOR_MIN_GAIN)
        u32GainX1024 = SENSOR_MIN_GAIN;
    else if(u32GainX1024 >= SENSOR_MAX_GAIN)
        u32GainX1024 = SENSOR_MAX_GAIN;
/*
    if(u32GainX1024>=22925)//if gain exceed 2x , enable high conversion gain, >27DB, enable HCG
    {
        gain_reg[1].u16Data |= 0x10;
        u32GainX1024 /= 2;
    }
    else
    {
        gain_reg[1].u16Data &= ~0x10;
    }
*/
    //nGainDouble = 20*(intlog10(u32GainX1024)-intlog10(1024));
    //gain_reg[0].u16Data = (_u16)(((nGainDouble*10)>> 24)/3);

    nGainDouble = 20 * (intlog10(u32GainX1024) - intlog10(1024));
    nGainDouble = (_u16)((nGainDouble * 10) >> 24) / 3;

    gain_reg[0].u16Data = nGainDouble & 0xff;
    gain_reg[1].u16Data = (nGainDouble >> 8) & 0xff;

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

static unsigned int IMX664EarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID      = E_EARLYINIT_SNR_BAYER_RG;
        pSnrInfo->ePixelDepth   = EARLYINIT_DATAPRECISION_12;
        pSnrInfo->eIfBusType    = EARLYINIT_BUS_TYPE_PARL;
        pSnrInfo->u32FpsX1000   = _gIMX664Info.u32FpsX1000;
        pSnrInfo->u32Width      = _gIMX664Info.u32Width;
        pSnrInfo->u32Height     = _gIMX664Info.u32Height;
        pSnrInfo->u32GainX1024  = _gIMX664Info.u32GainX1024;
        pSnrInfo->u32ShutterUs  = _gIMX664Info.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _gIMX664Info.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _gIMX664Info.u32GainShortX1024;
        pSnrInfo->u8ResId       = _gIMX664Info.u8ResId;
        pSnrInfo->u8HdrMode     = _gIMX664Info.u8HdrMode;
        pSnrInfo->u32TimeoutMs  = 200;
    }
    return 0;
}

/* Sensor EarlyInit implementation end*/
SENSOR_EARLYINIY_ENTRY_IMPL_END( IMX664,
                                 Sensor_init_table,
                                 IMX664EarlyInitShutterAndFps,
                                 IMX664EarlyInitGain,
                                 IMX664EarlyInitGetSensorInfo
                                );
