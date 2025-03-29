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
#include "GC4653_MIPI_init_table.h"         /* Sensor initial table */

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(gc4653)


typedef struct {

      unsigned int gain;
      unsigned short again_reg_val_0;
      unsigned short again_reg_val_1;
      unsigned short again_reg_val_2;
      unsigned short again_reg_val_3;
      unsigned short again_reg_val_4;

} Gain_ARRAY;

static EarlyInitSensorInfo_t _ggc4653Info =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_GR,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .u32FpsX1000    = 30000,
    .u32Width       = 2560,
    .u32Height      = 1440,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
};

// Davis 20181101

static Gain_ARRAY gain_table[]={
    { 1024, 0x00, 0x04, 0x42, 0x01, 0x00},
    { 1213, 0x20, 0x05, 0x43, 0x01, 0x0B},
    { 1434, 0x01, 0x04, 0x42, 0x01, 0x19},
    { 1699, 0x21, 0x05, 0x43, 0x01, 0x2a},
    { 2048, 0x02, 0x04, 0x42, 0x02, 0x00},
    { 2427, 0x22, 0x05, 0x43, 0x02, 0x17},
    { 2867, 0x03, 0x05, 0x43, 0x02, 0x33},
    { 3398, 0x23, 0x06, 0x44, 0x03, 0x14},
    { 4096, 0x04, 0x06, 0x44, 0x04, 0x00},
    { 4854, 0x24, 0x08, 0x46, 0x04, 0x2f},
    { 5734, 0x05, 0x08, 0x46, 0x05, 0x26},
    { 6795, 0x25, 0x0a, 0x48, 0x06, 0x28},
    { 8192, 0x06, 0x0c, 0x4a, 0x08, 0x00},
    { 9708, 0x26, 0x0d, 0x4b, 0x09, 0x1E},
    {11469, 0x46, 0x0f, 0x4d, 0x0B, 0x0C},
    {13591, 0x66, 0x11, 0x4f, 0x0D, 0x11},
    {16384, 0x0e, 0x13, 0x51, 0x10, 0x00},
    {19415, 0x2e, 0x16, 0x54, 0x12, 0x3D},
    {22938, 0x4e, 0x19, 0x57, 0x16, 0x19},
    {27181, 0x6e, 0x1b, 0x59, 0x1A, 0x22},
    {32768, 0x1e, 0x1e, 0x5c, 0x20, 0x00},
    {38830, 0x3e, 0x21, 0x5f, 0x25, 0x3A},
    {45875, 0x5e, 0x25, 0x63, 0x2C, 0x33},
    {54362, 0x7e, 0x29, 0x67, 0x35, 0x05},
    {65536, 0x9e, 0x2d, 0x6b, 0x40, 0x00},
    {77660, 0xbe, 0x40, 0x78, 0x4B, 0x35},
};


#define PREVIEW_LINE_PERIOD 22222 //Line per frame = Lpf+1 , line period = (1/30)/1125
#define VTS_30FPS  1500


/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int gc4653EarlyInitShutterAndFps( unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    /*ToDO: Parsing shutter to sensor i2c setting*/
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] = {
        {0x0203, 0xd0},   //shutter [0:7]
        {0x0202, 0x05},   //shutter [8:15]
        {0x0340, 0x05},   //vts [8:15]
        {0x0341, 0xdc},   //vts [0:7]
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

    _ggc4653Info.u32ShutterUs = nShutterUs;
    _ggc4653Info.u32FpsX1000 = nFpsX1000;

    return nNumRegs; //Return number of sensor registers to write
}

#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     (75*1024)                  // max sensor again, a-gain * conversion-gain*d-gain

/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int gc4653EarlyInitGain( unsigned int u32GainX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    int tmp = 0,i=0;
    int nNumRegs = 0;
    unsigned int dgain = 1;
    unsigned char dgain_0 = 1, dgain_1 = 0;
    I2cCfg_t gain_reg[] = {
        {0x02b3, 0x00},
        {0x0519, 0x00},
        {0x02d9, 0x00},
        {0x02b8, 0x00},
        {0x02b9, 0x00},
        {0x020e, 0x00},
        {0x020f, 0x00},
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
    gain_reg[0].u16Data  = gain_table[tmp].again_reg_val_0;
    gain_reg[1].u16Data  = gain_table[tmp].again_reg_val_1;
    gain_reg[2].u16Data  = gain_table[tmp].again_reg_val_2;
    gain_reg[3].u16Data  = gain_table[tmp].again_reg_val_3;
    gain_reg[4].u16Data  = gain_table[tmp].again_reg_val_4;

    gain_reg[5].u16Data  = dgain_0;
    gain_reg[6].u16Data  = dgain_1;

    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);

    _ggc4653Info.u32GainX1024 = u32GainX1024;
    return nNumRegs;
}

static unsigned int gc4653EarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID      = _ggc4653Info.eBayerID;
        pSnrInfo->ePixelDepth   = _ggc4653Info.ePixelDepth;
        pSnrInfo->u32FpsX1000   = _ggc4653Info.u32FpsX1000;
        pSnrInfo->u32Width      = _ggc4653Info.u32Width;
        pSnrInfo->u32Height     = _ggc4653Info.u32Height;
        pSnrInfo->u32GainX1024  = _ggc4653Info.u32GainX1024;
        pSnrInfo->u32ShutterUs  = _ggc4653Info.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _ggc4653Info.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _ggc4653Info.u32GainShortX1024;
        pSnrInfo->u8ResId       = _ggc4653Info.u8ResId;
        pSnrInfo->u8HdrMode     = _ggc4653Info.u8HdrMode;
        pSnrInfo->u32TimeoutMs  = 50;
     }
    return 0;
}

/* Sensor EarlyInit implementation end*/
SENSOR_EARLYINIY_ENTRY_IMPL_END( gc4653,
                                 Sensor_init_table,
                                 gc4653EarlyInitShutterAndFps,
                                 gc4653EarlyInitGain,
                                 gc4653EarlyInitGetSensorInfo
                                );
