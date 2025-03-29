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
#include "GC2053_MIPI_init_table.h"         /* Sensor initial table */

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(gc2053)


typedef struct {

      unsigned int gain;
      unsigned short again_reg_val_0;
      unsigned short again_reg_val_1;
      unsigned short again_reg_val_2;
      unsigned short again_reg_val_3;

} Gain_ARRAY;

static EarlyInitSensorInfo_t _ggc2053Info =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_RG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .u32FpsX1000    = 30000,
    .u32Width       = 1920,
    .u32Height      = 1080,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 5000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
};

// Davis 20181101

static Gain_ARRAY gain_table[]={
                                {1024 , 0x00, 0x00,0x01,0x00},
                                {1230 , 0x00, 0x10,0x01,0x0c},
                                {1440 , 0x00, 0x20,0x01,0x1b},
                                {1730 , 0x00, 0x30,0x01,0x2c},
                                {2032 , 0x00, 0x40,0x01,0x3f},
                                {2380 , 0x00, 0x50,0x02,0x16},
                                {2880 , 0x00, 0x60,0x02,0x35},
                                {3460 , 0x00, 0x70,0x03,0x16},
                                {4080 , 0x00, 0x80,0x04,0x02},
                                {4800 , 0x00, 0x90,0x04,0x31},
                                {5776 , 0x00, 0xa0,0x05,0x32},
                                {6760 , 0x00, 0xb0,0x06,0x35},
                                {8064 , 0x00, 0xc0,0x08,0x04},
                                {9500 , 0x00, 0x5a,0x09,0x19},
                                {11552, 0x00, 0x83,0x0b,0x0f},
                                {13600, 0x00, 0x93,0x0d,0x12},
                                {16132, 0x00, 0x84,0x10,0x00},
                                {18912, 0x00, 0x94,0x12,0x3a},
                                {22528, 0x01, 0x2c,0x1a,0x02},
                                {27036, 0x01, 0x3c,0x1b,0x20},
                                {32340, 0x00, 0x8c,0x20,0x0f},
                                {38256, 0x00, 0x9c,0x26,0x07},
                                {45600, 0x02, 0x64,0x36,0x21},
                                {53912, 0x02, 0x74,0x37,0x3a},
                                {63768, 0x00, 0xc6,0x3d,0x02},
                                {76880, 0x00, 0xdc,0x3f,0x3f},
                                {92300, 0x02, 0x85,0x3f,0x3f},
                                {108904, 0x02, 0x95,0x3f,0x3f},
                                {123568, 0x00, 0xce,0x3f,0x3f},
};


#define PREVIEW_LINE_PERIOD 29630 //Line per frame = Lpf+1 , line period = (1/30)/1125
#define VTS_30FPS  1125


/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int gc2053EarlyInitShutterAndFps( unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    /*ToDO: Parsing shutter to sensor i2c setting*/
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] = {
        {0x04, 0xd0},   //shutter [8:15]
        {0x03, 0x05},   //shutter [0:7]
        {0x41, 0x05},   //vts [8:15]
        {0x42, 0xdc},   //vts [0:7]
    };

    lines=(1000*nShutterUs)/PREVIEW_LINE_PERIOD;

    if(nFpsX1000<1000)    //for old method
        vts = (VTS_30FPS*30)/nFpsX1000;
    else    //new method, fps is 1000 based
        vts = (VTS_30FPS*30*1000)/nFpsX1000;

    if(lines<1) lines=1;

    if ( lines > (vts-4) ) //if shutter > frame interval
    {
        vts = lines +4;
    }

    expo_reg[0].u16Data = (lines) & 0x00ff;
    expo_reg[1].u16Data = (lines>>8) & 0x003f;
    expo_reg[2].u16Data = (vts >> 8) & 0x003f;
    expo_reg[3].u16Data = (vts >> 0) & 0x00ff;

    /*copy result*/
    //memcpy(pRegs,expo_reg,sizeof(expo_reg));
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

    _ggc2053Info.u32ShutterUs = nShutterUs;
    _ggc2053Info.u32FpsX1000 = nFpsX1000;

    return nNumRegs; //Return number of sensor registers to write
}

#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     (128*1024)                  // max sensor again, a-gain * conversion-gain*d-gain

/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int gc2053EarlyInitGain( unsigned int u32GainX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    int tmp = 0,i=0;
    int nNumRegs = 0;
    unsigned int dgain = 1;
    unsigned char dgain_0 = 1, dgain_1 = 0;
    I2cCfg_t gain_reg[] = {
        {0xfe, 0x00},
        {0xb4, 0x00},
        {0xb3, 0x00},
        {0xb8, 0x00},
        {0xb9, 0x00},
        {0xb1, 0x00},
        {0xb2, 0x00},
    };

    if (u32GainX1024 < 1024) {
        u32GainX1024 = 1024;
    } else if (u32GainX1024 > SENSOR_MAX_GAIN * 1024) {
        u32GainX1024 = SENSOR_MAX_GAIN  * 1024;
    }

    for(i = 0;i<sizeof(gain_table)/sizeof(Gain_ARRAY)-1;i++)
    {
        if((u32GainX1024 >= gain_table[i].gain) && (u32GainX1024 < gain_table[i + 1].gain))
        {
            tmp = i;
            break;
        }
        else
        {
            tmp = sizeof(gain_table)/sizeof(Gain_ARRAY) - 1;
        }
    }

    dgain =(u32GainX1024*64)/(gain_table[tmp].gain);
    dgain_0 = (dgain)>>6;
    dgain_1 =(dgain & 0x3f)<<2;
    gain_reg[1].u16Data  = gain_table[tmp].again_reg_val_0;
    gain_reg[2].u16Data  = gain_table[tmp].again_reg_val_1;
    gain_reg[3].u16Data  = gain_table[tmp].again_reg_val_2;
    gain_reg[4].u16Data  = gain_table[tmp].again_reg_val_3;

    gain_reg[5].u16Data  = dgain_0;
    gain_reg[6].u16Data  = dgain_1;

    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);

    _ggc2053Info.u32GainX1024 = u32GainX1024;
    return nNumRegs;
}

static unsigned int gc2053EarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID      = _ggc2053Info.eBayerID;
        pSnrInfo->ePixelDepth   = _ggc2053Info.ePixelDepth;
        pSnrInfo->u32FpsX1000   = _ggc2053Info.u32FpsX1000;
        pSnrInfo->u32Width      = _ggc2053Info.u32Width;
        pSnrInfo->u32Height     = _ggc2053Info.u32Height;
        pSnrInfo->u32GainX1024  = _ggc2053Info.u32GainX1024;
        pSnrInfo->u32ShutterUs  = _ggc2053Info.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _ggc2053Info.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _ggc2053Info.u32GainShortX1024;
        pSnrInfo->u8ResId       = _ggc2053Info.u8ResId;
        pSnrInfo->u8HdrMode     = _ggc2053Info.u8HdrMode;
        pSnrInfo->u32TimeoutMs  = 50;
     }
    return 0;
}

/* Sensor EarlyInit implementation end*/
SENSOR_EARLYINIY_ENTRY_IMPL_END( gc2053,
                                 Sensor_init_table,
                                 gc2053EarlyInitShutterAndFps,
                                 gc2053EarlyInitGain,
                                 gc2053EarlyInitGetSensorInfo
                                );
