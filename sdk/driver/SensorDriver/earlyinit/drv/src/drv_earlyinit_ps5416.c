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

#define SENSOR_EARLYINIT_PHASE
#define _PRE_ROLL_MODE_  1   // 1  SPI MODE,  0 DVP MODE
#define _PS5416_MIPI_SSI_SWITCH_IN_IPL_EARLYINIT_

#include <drv_sensor_earlyinit_common.h>
#include <drv_sensor_earlyinit_method_1.h>
#include "PS5416_init_table.h"

#define RATIO_OF_LEF_TO_SEF   16

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(PS5416)

static EarlyInitSensorInfo_t _gPS5416Info =
{
    .eBayerID           = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth        = EARLYINIT_DATAPRECISION_12,
    .u32FpsX1000        = 30000,
    .u32Width           = 2688,
    .u32Height          = 1520,
    .u32GainX1024       = 1024,
    .u32ShutterUs       = 8000,
    .u32ShutterShortUs  = 0,
    .u32GainShortX1024  = 0,
    .u8ResId            = 0,
    .u8HdrMode          = 0,
};
unsigned int gu32SnrGainCompensate128x = 128;

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int PS5416EarlyInitShutterAndFpsLinear(unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    /*ToDO: Parsing shutter to sensor i2c setting*/
    unsigned int uVtsMaxFps, u32ExpoLines, u32Vts;
    unsigned char n;

    I2cCfg_t expo_reg[] = {
        {0x014E, 0x3E},     // R_ExpLine_Stream[7:0]
        {0x014F, 0x06},     // R_ExpLine_Stream[15:8]
        {0x011E, 0x40},     // R_Lpf[7:0]
        {0x011F, 0x06},     // R_Lpf[15:8]
    };

    u32ExpoLines = (1000 * nShutterUs) / PREVIEW_LINE_PERIOD_LINEAR_30FPS;

    if(nFpsX1000 < 1000)
        uVtsMaxFps = VTS_LINEAR_30FPS * 30 / nFpsX1000;
    else
        uVtsMaxFps = VTS_LINEAR_30FPS * 30000 / nFpsX1000;

    if(u32ExpoLines < MIN_EXPO_LINE_COUNT)
        u32ExpoLines = MIN_EXPO_LINE_COUNT;

    if(u32ExpoLines > uVtsMaxFps - 2)
        u32Vts = u32ExpoLines + 2;
    else
        u32Vts = uVtsMaxFps;

    expo_reg[0].u16Data = (unsigned short)((u32ExpoLines >> 0) & 0xff);
    expo_reg[1].u16Data = (unsigned short)((u32ExpoLines >> 8) & 0xff);
    expo_reg[2].u16Data = (unsigned short)((u32Vts >> 0) & 0xff);
    expo_reg[3].u16Data = (unsigned short)((u32Vts >> 8) & 0xff);

    /*copy result*/
    for(n = 0; n < sizeof(expo_reg); n++)
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];

    _gPS5416Info.u32ShutterUs = nShutterUs;
    _gPS5416Info.u32FpsX1000 = nFpsX1000;

    //Return number of sensor registers to write
    return (sizeof(expo_reg)/sizeof(expo_reg[0]));
}
#if _PS5416_HDR_SUPPORT_
static unsigned int PS5416EarlyInitShutterAndFpsHdr(unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned int u32LefExpoLines, u32SefExpoLines, u32Vts, u32MaxSefExpoLines, u32MaxLefExpoLines;
    unsigned char n;

    I2cCfg_t expo_reg[] = {
        {0x014E, 0x3E},     // R_ExpLine_Stream[7:0]
        {0x014F, 0x06},     // R_ExpLine_Stream[15:8]
        {0x0152, 0x3E},     // R_ExpLine_short_Stream[7:0]
        {0x0153, 0x06},     // R_ExpLine_short_Stream[15:8]
        {0x011E, 0x40},     // R_Lpf[7:0]
        {0x011F, 0x06},     // R_Lpf[15:8]
        {0x0127, 0x0C},     // R_ExpLine_long_max[7:0]
        {0x0128, 0x0C},     // R_ExpLine_long_max[15:8]
        {0x0129, 0x6C},     // R_ExpLine_short_max[7:0]
        {0x012A, 0x00},     // R_ExpLine_short_max[15:8]
    };

    if(nFpsX1000 < 1000)
        nFpsX1000 *= 1000;

    u32Vts = VTS_HDR_30FPS * 30000 / nFpsX1000;

    u32LefExpoLines = (1000 * nShutterUs) / PREVIEW_LINE_PERIOD_HDR_30FPS;
    if(HDR_MAX_EXPO_LINE(u32Vts) / (RATIO_OF_LEF_TO_SEF + 1) > SEF_MAX_EXPO_LINE(u32Vts))
        u32MaxSefExpoLines = SEF_MAX_EXPO_LINE(u32Vts);
    else
        u32MaxSefExpoLines = HDR_MAX_EXPO_LINE(u32Vts) / (RATIO_OF_LEF_TO_SEF + 1);
    u32MaxLefExpoLines = HDR_MAX_EXPO_LINE(u32Vts) - u32MaxSefExpoLines;

    if(u32LefExpoLines > u32MaxLefExpoLines)
    {
        gu32SnrGainCompensate128x = (u32LefExpoLines << 7) / u32MaxLefExpoLines;
        u32LefExpoLines = u32MaxLefExpoLines;
        u32SefExpoLines = u32MaxSefExpoLines;
    }
    else
    {
        u32SefExpoLines = u32LefExpoLines / RATIO_OF_LEF_TO_SEF;
        if(u32SefExpoLines < MIN_EXPO_LINE_COUNT)
        {
            u32SefExpoLines = MIN_EXPO_LINE_COUNT;
            u32LefExpoLines = u32SefExpoLines * RATIO_OF_LEF_TO_SEF;
        }
    }

    expo_reg[0].u16Data = (unsigned short)((u32LefExpoLines >> 0) & 0xff);
    expo_reg[1].u16Data = (unsigned short)((u32LefExpoLines >> 8) & 0xff);
    expo_reg[2].u16Data = (unsigned short)((u32SefExpoLines >> 0) & 0xff);
    expo_reg[3].u16Data = (unsigned short)((u32SefExpoLines >> 8) & 0xff);
    expo_reg[4].u16Data = (unsigned short)((u32Vts >> 0) & 0xff);
    expo_reg[5].u16Data = (unsigned short)((u32Vts >> 8) & 0xff);
    expo_reg[6].u16Data = (unsigned short)((u32MaxLefExpoLines >> 0) & 0xff);
    expo_reg[7].u16Data = (unsigned short)((u32MaxLefExpoLines >> 8) & 0xff);
    expo_reg[8].u16Data = (unsigned short)((u32MaxSefExpoLines >> 0) & 0xff);
    expo_reg[9].u16Data = (unsigned short)((u32MaxSefExpoLines >> 8) & 0xff);

    /*copy result*/
    for(n = 0; n < sizeof(expo_reg); n++)
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];

    _gPS5416Info.u32ShutterUs = (u32LefExpoLines * PREVIEW_LINE_PERIOD_HDR_30FPS) / 1000;
    _gPS5416Info.u32ShutterShortUs = (u32SefExpoLines * PREVIEW_LINE_PERIOD_HDR_30FPS) / 1000;
    _gPS5416Info.u32FpsX1000 = nFpsX1000;
    _gPS5416Info.u8HdrMode = 1;

    //Return number of sensor registers to write
    return (sizeof(expo_reg)/sizeof(expo_reg[0]));
}
#endif
/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int PS5416EarlyInitGainLinear(unsigned int u32Gain1024x, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    int nGainTableSize = sizeof(gastGainTable) / sizeof(gastGainTable[0]);
    unsigned char i;
    I2cCfg_t gain_reg[] = {
        {0x0150, 0x40},     // R_GainIndex_Stream[7:0]
        {0x0151, 0x00},     // R_GainIndex_Stream[9:8]
    };

    if(u32Gain1024x < SNR_MIN_GAIN)
        u32Gain1024x = SNR_MIN_GAIN;
    else if(u32Gain1024x > SNR_MAX_GAIN)
        u32Gain1024x = SNR_MAX_GAIN;

    for(i = 1; i < nGainTableSize; i++)
    {
        if(gastGainTable[i].u64Gain1024x > u32Gain1024x)
            break;
    }

    gain_reg[0].u16Data = gastGainTable[i-1].u16RegVal & 0xFF;
    gain_reg[1].u16Data = (gastGainTable[i-1].u16RegVal >> 8) & 0x3;

    /*copy result*/
    for(i = 0; i < sizeof(gain_reg); i++)
        ((unsigned char*)pRegs)[i] = ((unsigned char*)gain_reg)[i];

    _gPS5416Info.u32GainX1024 = u32Gain1024x;
    return (sizeof(gain_reg)/sizeof(gain_reg[0]));
}
#if _PS5416_HDR_SUPPORT_
static unsigned int PS5416EarlyInitGainHdr( unsigned int u32Gain1024x, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    int nGainTableSize = sizeof(gastGainTable) / sizeof(gastGainTable[0]);
    unsigned char i;
    I2cCfg_t gain_reg[] = {
        {0x0150, 0x40},     // R_GainIndex_Stream[7:0]
        {0x0151, 0x00},     // R_GainIndex_Stream[9:8]
        {0x0154, 0x40},     // R_GainIndex_short_Stream[7:0]
        {0x0155, 0x00},     // R_GainIndex_short_Stream[9:8]
    };

    u32Gain1024x =  (u32Gain1024x * gu32SnrGainCompensate128x) >> 7;

    if(u32Gain1024x < SNR_MIN_GAIN)
        u32Gain1024x = SNR_MIN_GAIN;
    else if(u32Gain1024x > SNR_MAX_GAIN)
        u32Gain1024x = SNR_MAX_GAIN;

    for(i = 1; i < nGainTableSize; i++)
    {
        if(gastGainTable[i].u64Gain1024x > u32Gain1024x)
            break;
    }

    gain_reg[0].u16Data = gastGainTable[i-1].u16RegVal & 0xFF;
    gain_reg[1].u16Data = (gastGainTable[i-1].u16RegVal >> 8) & 0x3;
    gain_reg[2].u16Data = gastGainTable[i-1].u16RegVal & 0xFF;
    gain_reg[3].u16Data = (gastGainTable[i-1].u16RegVal >> 8) & 0x3;

    /*copy result*/
    for(i = 0; i < sizeof(gain_reg); i++)
        ((unsigned char*)pRegs)[i] = ((unsigned char*)gain_reg)[i];

    _gPS5416Info.u32GainX1024 = u32Gain1024x;
    _gPS5416Info.u32GainShortX1024 = u32Gain1024x;
    return (sizeof(gain_reg)/sizeof(gain_reg[0]));
}
#endif
static unsigned int PS5416EarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID          = E_EARLYINIT_SNR_BAYER_BG;
        pSnrInfo->ePixelDepth       = EARLYINIT_DATAPRECISION_12;
        pSnrInfo->u32FpsX1000       = _gPS5416Info.u32FpsX1000;
        pSnrInfo->u32Width          = _gPS5416Info.u32Width;
        pSnrInfo->u32Height         = _gPS5416Info.u32Height;
        pSnrInfo->u32GainX1024      = _gPS5416Info.u32GainX1024;
        pSnrInfo->u32ShutterUs      = _gPS5416Info.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _gPS5416Info.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _gPS5416Info.u32GainShortX1024;
        pSnrInfo->u8ResId           = _gPS5416Info.u8ResId;
        pSnrInfo->u8HdrMode         = _gPS5416Info.u8HdrMode;
        pSnrInfo->u32TimeoutMs      = 200;
        //CamOsPrintf("PS5416 , FPS=%u, Shutter=%u, Gain=%u \n",
        //        pSnrInfo->u32FpsX1000, pSnrInfo->u32ShutterUs, pSnrInfo->u32GainX1024);
    }
    return 0;
}

/* Sensor EarlyInit implementation end*/
#if _PS5416_HDR_SUPPORT_
SENSOR_EARLYINIY_ENTRY_IMPL_END( PS5416,
                                 Sensor_init_table_ps5416_linear_C,
                                 Sensor_init_table_ps5416_hdr_C,
                                 Sensor_init_table_ps5416_linear_D,
                                 Sensor_init_table_ps5416_hdr_D,
                                 PS5416EarlyInitShutterAndFpsLinear,
                                 PS5416EarlyInitShutterAndFpsHdr,
                                 PS5416EarlyInitGainLinear,
                                 PS5416EarlyInitGainHdr,
                                 PS5416EarlyInitGetSensorInfo
                                );
#else
SENSOR_EARLYINIY_ENTRY_IMPL_END(PS5416,
#ifdef _PS5416_MIPI_SSI_SWITCH_IN_IPL_EARLYINIT_
                                Sensor_switch_to_streaming_mode_ps5416,
#else
#if (PS5416_SNR_VER == PS5416_SNR_VER_C)
                                Sensor_init_table_ps5416_linear_C,
#endif
#if (PS5416_SNR_VER == PS5416_SNR_VER_D)
                                Sensor_init_table_ps5416_linear_D,
#endif
#endif
                                PS5416EarlyInitShutterAndFpsLinear,
                                PS5416EarlyInitGainLinear,
                                PS5416EarlyInitGetSensorInfo);
#endif
