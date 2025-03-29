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
#include "K351P_init_table.h"         /* Sensor initial table */

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(K351P)

typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

static EarlyInitSensorInfo_t _gK351PInfo =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .u32FpsX1000    = 30000,
    .u32Width       = 2000,
    .u32Height      = 2000,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
};
static int gain_table[]={
    1024,  //1
    1088,  //1.0625
    1152,  //1.125
    1216,  //1.1875
    1280,  //1.25
    1344,  //1.3125
    1408,  //1.375
    1472,  //1.4375
    1536,  //1.5
    1600,  //1.5625
    1664,  //1.625
    1728,  //1.6875
    1792,  //1.75
    1856,  //1.8125
    1920,  //1.875
    1984,  //1.9375
    2048,  //2
    2176,  //2.125
    2304,  //2.25
    2432,  //2.375
    2560,  //2.5
    2688,  //2.625
    2816,  //2.75
    2944,  //2.875
    3072,  //3
    3200,  //3.125
    3328,  //3.25
    3456,  //3.375
    3584,  //3.5
    3712,  //3.625
    3840,  //3.75
    3968,  //3.875
    4096,  //4
    4352,  //4.25
    4608,  //4.5
    4864,  //4.75
    5120,  //5
    5376,  //5.25
    5632,  //5.5
    5888,  //5.75
    6144,  //6
    6400,  //6.25
    6656,  //6.5
    6912,  //6.75
    7168,  //7
    7424,  //7.25
    7680,  //7.5
    7936,  //7.75
    8192,  //8
    8704,  //8.5
    9216,  //9
    9728,  //9.5
    10240, //10
    10752, //10.5
    11264, //11
    11776, //11.5
    12288, //12
    12800, //12.5
    13312, //13
    13824, //13.5
    14336, //14
    14848, //14.5
    15360, //15
    15872, //15.5
    16384, //16
    17408, //17
    18432, //18
    19456, //19
    20480, //20
    21504, //21
    22528, //22
    23552, //23
    24576, //24
    25600, //25
    26624, //26
    27648, //27
    28672, //28
    29696, //29
    30720, //30
    31744, //31    
};

#define PREVIEW_LINE_PERIOD 15873 //Line per frame = Lpf+1 , line period = (1/30)/2100
#define VTS_30FPS  2100

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int K351P_EarlyInitShutterAndFps( unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period 15873                           // hts=33.333/2100=15873
#define vts_30fps  2100
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x02, 0x00},//expo[15:8] texp=expo[15:0]*Tline
        {0x01, 0xff},//expo[7:0]
        /*vts*/
        {0x23, 0x08},
        {0x22, 0x34},
    };

    /*VTS*/
    vts =  (vts_30fps*30000)/nFpsX1000;

    if(nFpsX1000<1000)    //for old method
        vts = (vts_30fps*30)/nFpsX1000;

    /*Exposure time*/
    lines = (1000*nShutterUs)/Preview_line_period;

    if(lines>vts-10)
        vts = lines +10;

    //lines = vts-lines-1;

    expo_reg[0].u16Data = (lines>>8) & 0x00ff;
    expo_reg[1].u16Data = (lines>>0) & 0x00ff;

    expo_reg[2].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[3].u16Data = (vts >> 0) & 0x00ff;

    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

    _gK351PInfo.u32ShutterUs = nShutterUs;
    _gK351PInfo.u32FpsX1000 = nFpsX1000;

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
static unsigned int K351P_EarlyInitGain( unsigned int u32GainX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN      31744   //31*1024
    /*TODO: Parsing gain to sensor i2c setting*/
    int nNumRegs = 0,i=0;
    unsigned int gain=u32GainX1024;
    unsigned char n;

    I2cCfg_t gain_reg[] = {
        {0x00, 0x00},   //again:2^PGA[6:4]*(1+PGA[3:0]/16)
    };

    _gK351PInfo.u32GainX1024 = u32GainX1024;

    if (gain < SENSOR_MIN_GAIN) {
        gain = SENSOR_MIN_GAIN;
    } else if (gain > SENSOR_MAX_GAIN) {
        gain = SENSOR_MAX_GAIN;
    }

    for(i=0; i < 80; i++)
    {
        if(gain_table[i] >= gain)
            break;
    }
    if(i!=0 && i!=80 && gain!=gain_table[i])
        i=i-1;
    gain_reg[0].u16Data = i;

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}

static unsigned int K351P_EarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID      = E_EARLYINIT_SNR_BAYER_BG;
        pSnrInfo->ePixelDepth   = EARLYINIT_DATAPRECISION_10;
        pSnrInfo->eIfBusType    = EARLYINIT_BUS_TYPE_MIPI;
        pSnrInfo->u32FpsX1000   = _gK351PInfo.u32FpsX1000;
        pSnrInfo->u32Width      = _gK351PInfo.u32Width;
        pSnrInfo->u32Height     = _gK351PInfo.u32Height;
        pSnrInfo->u32GainX1024  = _gK351PInfo.u32GainX1024;
        pSnrInfo->u32ShutterUs  = _gK351PInfo.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _gK351PInfo.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _gK351PInfo.u32GainShortX1024;
        pSnrInfo->u8ResId       = _gK351PInfo.u8ResId;
        pSnrInfo->u8HdrMode     = _gK351PInfo.u8HdrMode;
        pSnrInfo->u32TimeoutMs  = 200;
    }
    return 0;
}

/* Sensor EarlyInit implementation end*/
SENSOR_EARLYINIY_ENTRY_IMPL_END( K351P,
                                 Sensor_init_table,
                                 K351P_EarlyInitShutterAndFps,
                                 K351P_EarlyInitGain,
                                 K351P_EarlyInitGetSensorInfo
                                );
