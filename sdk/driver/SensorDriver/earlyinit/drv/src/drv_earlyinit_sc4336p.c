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
#include "SC4336P_init_table.h"         /* Sensor initial table */


#define SC4336P_PRESET_4M30 0

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(SC4336P)

static EarlyInitSensorInfo_t _gSC4336PInfo_4M30 =
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

EarlyInitSensorInfo_t* pResList[] =
{
    &_gSC4336PInfo_4M30,
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
static unsigned int SC4336PEarlyInitShutterAndFpsLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
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
        /*expo logic*/
        {0x338f, 0x20},
    };

    unsigned int _Preview_line_period = 22222;
    unsigned int _vts_30fps = 1500;

    switch(pHandle->uPresetId)
    {
        case SC4336P_PRESET_4M30:
            _Preview_line_period = 22222;
            _vts_30fps = 1500;
            break;
    }

    switch(pHandle->uPresetId)
    {
        case SC4336P_PRESET_4M30:
            /*VTS*/
            vts =  (_vts_30fps*30000)/nFpsX1000;
            if(nFpsX1000<1000)    //for old method
                vts = (_vts_30fps*30)/nFpsX1000;
            break;
    }

    lines = (1000*nShutterUsLef)/_Preview_line_period; // Preview_line_period in ns
    if(lines <= 1)
        lines=1;
    if (lines > vts-8) {
        vts = (lines+8);
    }

    lines = lines<<4;

    expo_reg[0].u16Data = (lines>>16) & 0x0003;
    expo_reg[1].u16Data = (lines>>8) & 0x00ff;
    expo_reg[2].u16Data = (lines>>0) & 0x00ff;

    expo_reg[3].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[4].u16Data = (vts >> 0) & 0x00ff;

    ///////////expo logic
    if (nShutterUsLef < 5000) {    //5ms
        expo_reg[5].u16Data = 0xa0;
    }
    else if (nShutterUsLef > 10000) {  //10ms
        expo_reg[5].u16Data = 0x20;
    }

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
static unsigned int SC4336PEarlyInitGainLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAXGAIN     (32768*15750 / 1000000)// (32768*15 / 1000)//32XAGAIN 15XDGAIN
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    _u64 gain_factor;
    _u64 gain_reg_0x3e09,gain_reg_0x3e06,gain_reg_0x3e07;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
        {0x3e09, 0x00},
    };

    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;

    if (u32GainLefX1024 <= 1024) {
        u32GainLefX1024 = 1024;
    } else if (u32GainLefX1024 > SENSOR_MAXGAIN*1024) {
        u32GainLefX1024 = SENSOR_MAXGAIN*1024;
    }

    gain_factor = u32GainLefX1024 * 1000 / 1024;

    if(gain_factor < 1000)
    {
        gain_factor = 1000;
    }
    if(gain_factor > 480*1000)
    {
        gain_factor =480*1000;
    }

    if (gain_factor < 2000)
    {
        gain_reg_0x3e09 = 0x00;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 =  (gain_factor * 128 / 1000 / 4 ) * 4 ; //update fine dgain accuracy
    }
    else if( gain_factor < 4000)
    {
        gain_reg_0x3e09 = 0x08;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = (gain_factor * 128 / 2000 / 4) * 4; //update fine dgain accuracy
    }
    else if( gain_factor < 8000)
    {
        gain_reg_0x3e09 = 0x09;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = (gain_factor * 128 / 4000 / 4 ) * 4 ;//update fine dgain accuracy
    }
    else if( gain_factor < 16000)
    {
        gain_reg_0x3e09 = 0x0b;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = (gain_factor * 128 / 8000 / 4 ) * 4 ;//update fine dgain accuracy ;
    }
    else if( gain_factor < 32000)
    {
        gain_reg_0x3e09 = 0x0f;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = (gain_factor * 128 / 16000 / 4 ) * 4 ;//update fine dgain accuracy ;
    }
    else if( gain_factor < 32000*2)
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x00;
        gain_reg_0x3e07 = (gain_factor * 128 / 32000 / 4 ) * 4 ;//update fine dgain accuracy ;
    }
    else if( gain_factor < 32000*4)//open dgain begin  max digital gain 4X
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x01;
        gain_reg_0x3e07 = (gain_factor * 128 / 32000/2 / 4 ) * 4 ;//update fine dgain accuracy ;
    }
    else if( gain_factor < 32000*8)
     {
         gain_reg_0x3e09 = 0x1f;
         gain_reg_0x3e06 = 0x03;
         gain_reg_0x3e07 = (gain_factor * 128 / 32000 / 4 / 4 ) * 4 ;//update fine dgain accuracy ;
     }
    else if( gain_factor < 504000)//32000*15.75
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x07;
        gain_reg_0x3e07 = (gain_factor * 128 / 32000 / 8 / 4 ) * 4 ;//update fine dgain accuracy ;
    }
    else
    {
        gain_reg_0x3e09 = 0x1f;
        gain_reg_0x3e06 = 0x07;
        gain_reg_0x3e07 = 0xfc;
    }

    gain_reg[2].u16Data = gain_reg_0x3e09; // 0x3e09
    gain_reg[1].u16Data = gain_reg_0x3e07; // 0x3e07
    gain_reg[0].u16Data = gain_reg_0x3e06; // 0x3e06

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

static unsigned int SC4336PEarlyInitSelPresetId(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uPresetId)
{
    pHandle->uPresetId = uPresetId;
    switch(uPresetId)
    {
    case SC4336P_PRESET_4M30:
        pHandle->pTable = (void*) Sensor_init_table;
        pHandle->uTableSize = sizeof(Sensor_init_table);
        _memcpy(&pHandle->tInfo, &_gSC4336PInfo_4M30, sizeof(_gSC4336PInfo_4M30));
        break;
    default:
        return 0;
        break;
    }
    return 0;
}

unsigned int SC4336PEarlyInitSelResId(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId)
{
    int uId; //preset id
    for(uId=0;uId<sizeof(pResList)/sizeof(pResList[0]); ++uId)
    {
        if(pResList[uId]->u8HdrMode==uHdrMode && pResList[uId]->u8ResId==uResId)
        {
            SC4336PEarlyInitSelPresetId(pHandle, uId);
            return 0;
        }
    }
    return -1;
}

static unsigned int SC4336PEarlyInitSelOrientEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uMirror, unsigned char uFlip, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
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

static unsigned int SC4336PEarlyInitSelStreamOnoff( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uStreamOnoff, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
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

unsigned int SC4336PInitHandle( EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = SC4336PEarlyInitGainLinear;
    pHandle->fpShutterFpsParserEx = SC4336PEarlyInitShutterAndFpsLinear;
    pHandle->fpSelResId           = SC4336PEarlyInitSelResId;
    pHandle->fpOrientParerEx      = SC4336PEarlyInitSelOrientEx;
    pHandle->fpStreamOnoffParserEx= SC4336PEarlyInitSelStreamOnoff;

    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = 0;
    pHandle->tCurCfg.pTable       = (void*) Sensor_init_table;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gSC4336PInfo_4M30, sizeof(_gSC4336PInfo_4M30));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;

    return 0;
}

SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2( SC4336P, SC4336PInitHandle);
