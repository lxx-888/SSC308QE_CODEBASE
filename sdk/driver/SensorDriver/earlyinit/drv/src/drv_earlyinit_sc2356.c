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
#include "SC2356_init_table.h"         /* Sensor initial table */

//#define SC2356_PRESET 0

#define SC2356_PRESET_4_3        0
#define SC2356_PRESET_16_9       1

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(SC2356)

#if _SC2356_RES_4_3_
static EarlyInitSensorInfo_t _gSC2356Info_4_3 =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 1600,
    .u32Height      = 1200,
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
    .u8NumLanes     = 1,
    .u8StreamOnoff  = 1
};
#endif

#if _SC2356_RES_16_9_
static EarlyInitSensorInfo_t _gSC2356Info_16_9 =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 1600,
    .u32Height      = 900,
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
    .u8NumLanes     = 1,
    .u8StreamOnoff  = 1
};
#endif

static EarlyInitSensorInfo_t* pResList[] =
{
#if _SC2356_RES_4_3_
    &_gSC2356Info_4_3,
#endif
#if _SC2356_RES_16_9_
    &_gSC2356Info_16_9,
#endif
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
static unsigned int SC2356EarlyInitShutterAndFpsLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
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

    unsigned int _Preview_line_period = 26667;
    unsigned int _vts_linear = 1875;
    unsigned int _Max_fps = 20;

    switch(pHandle->uPresetId)
    {
        case SC2356_PRESET_4_3:
            _Preview_line_period = 26667;
            _vts_linear = 1875;
            break;
        case SC2356_PRESET_16_9:
            _Preview_line_period = 26667;
            _vts_linear = 1875;
            break;
    }

    switch(pHandle->uPresetId)
    {
        case SC2356_PRESET_4_3:
            /*VTS*/
            vts =  (_vts_linear * _Max_fps * 1000) / nFpsX1000;
            if(nFpsX1000 < 1000)    //for old method
                vts = (_vts_linear * _Max_fps) / nFpsX1000;
            break;
        case SC2356_PRESET_16_9:
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
static unsigned int SC2356EarlyInitGainLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAXGAIN     (16000*3938 / 1000000)// (32768*15 / 1000)//32XAGAIN 15XDGAIN
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    _u32 gain_factor = 0, gain_reg_0x3e09 = 0x00, gain_reg_0x3e06 = 0x00, gain_reg_0x3e07 = 0x80;

    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x00|0x80},
        {0x3e09, 0x00},
    };

    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;

    if (u32GainLefX1024 <= 1024) {
        u32GainLefX1024 = 1024;
    } else if (u32GainLefX1024 > SENSOR_MAXGAIN*1024) {
        u32GainLefX1024 = SENSOR_MAXGAIN*1024;
    }

    gain_factor = u32GainLefX1024 * 1000 / 1024;

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
    //params->tGain_reg[2].data = gain_reg_0x3e09;        // 3e09
    //params->tGain_reg[1].data = gain_reg_0x3e07;        // 3e07
    //params->tGain_reg[0].data = gain_reg_0x3e06;        // 3e06

    gain_reg[2].u16Data = gain_reg_0x3e09;
    gain_reg[1].u16Data = gain_reg_0x3e07;
    gain_reg[0].u16Data = gain_reg_0x3e06;
    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

static unsigned int SC2356EarlyInitSelPresetId(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uPresetId)
{
    pHandle->uPresetId = uPresetId;
    switch(uPresetId)
    {
    case SC2356_PRESET_4_3:
        pHandle->pTable = (void*) Sensor_init_table_4_3;
        pHandle->uTableSize = sizeof(Sensor_init_table_4_3);
        _memcpy(&pHandle->tInfo, &_gSC2356Info_4_3, sizeof(_gSC2356Info_4_3));
        break;
    case SC2356_PRESET_16_9:
        pHandle->pTable = (void*) Sensor_init_table_16_9;
        pHandle->uTableSize = sizeof(Sensor_init_table_16_9);
        _memcpy(&pHandle->tInfo, &_gSC2356Info_16_9, sizeof(_gSC2356Info_16_9));
        break;
    default:
        return 0;
        break;
    }
    return 0;
}

unsigned int SC2356EarlyInitSelResId(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId)
{
    int uId; //preset id
    for(uId=0;uId<sizeof(pResList)/sizeof(pResList[0]); ++uId)
    {
        if(pResList[uId]->u8HdrMode==uHdrMode && pResList[uId]->u8ResId==uResId)
        {
            SC2356EarlyInitSelPresetId(pHandle, uId);
            return 0;
        }
    }
    return -1;
}

static unsigned int SC2356EarlyInitSelOrientEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uMirror, unsigned char uFlip, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
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

static unsigned int SC2356EarlyInitSelStreamOnoff( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uStreamOnoff, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
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

unsigned int SC2356InitHandle( EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = SC2356EarlyInitGainLinear;
    pHandle->fpShutterFpsParserEx = SC2356EarlyInitShutterAndFpsLinear;
    pHandle->fpSelResId           = SC2356EarlyInitSelResId;
    pHandle->fpOrientParerEx      = SC2356EarlyInitSelOrientEx;
    pHandle->fpStreamOnoffParserEx= SC2356EarlyInitSelStreamOnoff;

    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = 0;
    pHandle->tCurCfg.pTable       = (void*) Sensor_init_table_4_3;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table_4_3);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gSC2356Info_4_3, sizeof(_gSC2356Info_4_3));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;

    return 0;
}

SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2( SC2356, SC2356InitHandle);
