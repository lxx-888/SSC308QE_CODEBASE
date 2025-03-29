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
#include "SC535HAI_init_table.h"         /* Sensor initial table */


#define SC535HAI_PRESET 0

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(SC535HAI)

static EarlyInitSensorInfo_t _gSC535HAIInfo =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 2560,
    .u32Height      = 1440,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
    .u32CropX       = 0,
    .u32CropY       = 0,
    .u8Mirror       = 0,
    .u8Flip         = 0,
    .u32TimeoutMs   = 200,
    .u8NumLanes     = 2,
    .u8StreamOnoff  = 1
};

static EarlyInitSensorInfo_t* pResList[] =
{
    &_gSC535HAIInfo,
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
static unsigned int SC535HAIEarlyInitShutterAndFpsLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    unsigned int lines = 0;
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

    unsigned int _Preview_line_period = 16667;
    unsigned int _vts_linear = 3000;
    unsigned int _Max_fps = 20;

    switch(pHandle->uPresetId)
    {
        case SC535HAI_PRESET:
            _Preview_line_period = 16667;
            _vts_linear = 3000;
            break;
    }

    switch(pHandle->uPresetId)
    {
        case SC535HAI_PRESET:
            /*VTS*/
            vts =  (_vts_linear * _Max_fps * 1000) / nFpsX1000;
            if(nFpsX1000 < 1000)    //for old method
                vts = (_vts_linear * _Max_fps) / nFpsX1000;
            break;
    }

    lines = (1000 * nShutterUsLef) / _Preview_line_period; // Preview_line_period in ns
    if(lines <= 2) {
        lines = 2;
    }

    if (lines > vts - 8) {
        vts = (lines + 8);
    }

    lines = lines<<4;

    expo_reg[0].u16Data = (lines>>16) & 0x0003;
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

    pHandle->tInfo.u32ShutterUs = nShutterUsLef;
    pHandle->tInfo.u32FpsX1000  = nFpsX1000;
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
static unsigned int SC535HAIEarlyInitGainLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAXGAIN     (79695*1575)/100000// (32768*15 / 1000)//32XAGAIN 15XDGAIN
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    unsigned char Coarse_gain = 1,DIG_gain=1;
    unsigned int Dcg_gainx100 = 1, ANA_Fine_gainx32 = 1;
    unsigned char Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x20,DIG_Fine_gain_reg=0x80;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e08, 0x00|0x03},
        {0x3e09, 0x20}, //low bit, 0x40 - 0x7f, step 1/64
    };

    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;

    if (u32GainLefX1024 <= 1024) {
        u32GainLefX1024 = 1024;
    } else if (u32GainLefX1024 > SENSOR_MAXGAIN*1024) {
        u32GainLefX1024 = SENSOR_MAXGAIN*1024;
    }

    if (u32GainLefX1024 < 2048) // start again  2 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x00; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (u32GainLefX1024 < 2591) // 2.53 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x01; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (u32GainLefX1024 < 5182) // 5.06 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 1;     DIG_gain=1;
        Coarse_gain_reg = 0x80; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (u32GainLefX1024 < 10363) // 10.12 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 2;     DIG_gain=1;
        Coarse_gain_reg = 0x81; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (u32GainLefX1024 < 20726)// 20.24 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 4;     DIG_gain=1;
        Coarse_gain_reg = 0x83; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (u32GainLefX1024 < 41452)// 40.48 * 1024
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 8;     DIG_gain=1;
        Coarse_gain_reg = 0x87; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
    else if (u32GainLefX1024 < 82904)// 80.96 * 1024 // end again
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=1;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x0;  DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (u32GainLefX1024 < 164511 ) // start dgain
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=1;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x0;  ANA_Fine_gain_reg=0x3f;
    }
    else if (u32GainLefX1024 < 164511 * 2)
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=2;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x1;  ANA_Fine_gain_reg=0x3f;
    }
    else if (u32GainLefX1024 < (164511 * 4 - 1))
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=4;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x3;  ANA_Fine_gain_reg=0x3f;
    }
    else if (u32GainLefX1024 <= SENSOR_MAXGAIN*1024)
    {
        Dcg_gainx100 = 2530;      Coarse_gain = 16;     DIG_gain=8;       ANA_Fine_gainx32=127;
        Coarse_gain_reg = 0x8f; DIG_gain_reg=0x7;  ANA_Fine_gain_reg=0x3f;
    }
#endif

    if(u32GainLefX1024 < 2530)
    {
        ANA_Fine_gain_reg = (1000ULL * u32GainLefX1024 / (Dcg_gainx100 * Coarse_gain) / 32);
    }
    else if(u32GainLefX1024 < 82903)
    {
        ANA_Fine_gain_reg = (1000ULL * u32GainLefX1024 / (Dcg_gainx100 * Coarse_gain) / 32);
    }else{
        DIG_Fine_gain_reg = (8000ULL * u32GainLefX1024 /(Dcg_gainx100 * Coarse_gain * DIG_gain) / ANA_Fine_gainx32 / 4 * 4);
    }

    gain_reg[3].u16Data = ANA_Fine_gain_reg;
    gain_reg[2].u16Data = Coarse_gain_reg;
    gain_reg[1].u16Data = DIG_Fine_gain_reg;
    gain_reg[0].u16Data = DIG_gain_reg & 0xF;
    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

static unsigned int SC535HAIEarlyInitSelPresetId(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uPresetId)
{
    pHandle->uPresetId = uPresetId;
    switch(uPresetId)
    {
    case SC535HAI_PRESET:
        pHandle->pTable = (void*) Sensor_init_table;
        pHandle->uTableSize = sizeof(Sensor_init_table);
        _memcpy(&pHandle->tInfo, &_gSC535HAIInfo, sizeof(_gSC535HAIInfo));
        break;
    default:
        return 0;
        break;
    }
    return 0;
}

unsigned int SC535HAIEarlyInitSelResId(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId)
{
    int uId; //preset id
    for(uId=0;uId<sizeof(pResList)/sizeof(pResList[0]); ++uId)
    {
        if(pResList[uId]->u8HdrMode==uHdrMode && pResList[uId]->u8ResId==uResId)
        {
            SC535HAIEarlyInitSelPresetId(pHandle, uId);
            return 0;
        }
    }
    return -1;
}

static unsigned int SC535HAIEarlyInitSelOrientEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uMirror, unsigned char uFlip, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    I2cCfg_t orient_reg[] =
    {
        {0x3221, 0x00},//low bit
    };

    if(uFlip)
    {
        orient_reg[0].u16Data  |= 0x60;
    }
    if(uMirror)
    {
        orient_reg[0].u16Data  |= 0x6;
    }

    /*copy result*/
    for(n=0;n<sizeof(orient_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)orient_reg)[n];
    }
    pHandle->tInfo.u8Mirror = uMirror;
    pHandle->tInfo.u8Flip   = uFlip;
    return sizeof(orient_reg)/sizeof(orient_reg[0]);
}

static unsigned int SC535HAIEarlyInitSelStreamOnoff( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uStreamOnoff, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    I2cCfg_t stream_onoff_reg[] =
    {
        {0x0100,0x01},//0x01 stream on,0x0 stream off
    };

    stream_onoff_reg[0].u16Data = uStreamOnoff;

    /*copy result*/
    for(n=0;n<sizeof(stream_onoff_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)stream_onoff_reg)[n];
    }
    pHandle->tInfo.u8StreamOnoff = uStreamOnoff;
    return sizeof(stream_onoff_reg)/sizeof(stream_onoff_reg[0]);
}

unsigned int SC535HAIInitHandle( EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = SC535HAIEarlyInitGainLinear;
    pHandle->fpShutterFpsParserEx = SC535HAIEarlyInitShutterAndFpsLinear;
    pHandle->fpSelResId           = SC535HAIEarlyInitSelResId;
    pHandle->fpOrientParerEx      = SC535HAIEarlyInitSelOrientEx;
    pHandle->fpStreamOnoffParserEx= SC535HAIEarlyInitSelStreamOnoff;

    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = 0;
    pHandle->tCurCfg.pTable       = (void*) Sensor_init_table;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gSC535HAIInfo, sizeof(_gSC535HAIInfo));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;

    return 0;
}

SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2( SC535HAI, SC535HAIInitHandle);
