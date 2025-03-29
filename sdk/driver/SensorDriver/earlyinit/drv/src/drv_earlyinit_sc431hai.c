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
#include "SC431HAI_init_table.h"         /* Sensor initial table */
#include "cam_os_wrapper.h"
#define SC431HAI_PRESET_4M30 0
#define SC431HAI_PRESET_4K25_HDR 1
/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(SC431HAI)
#if _SC431HAI_RES_4M30_
	static EarlyInitSensorInfo_t _gSC431HAIInfo_4M30 =
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
		.u32CropY       = 0
	};
#endif
#if _SC431HAI_RES_4M25_HDR
	static EarlyInitSensorInfo_t _gSC431HAIInfo_4M25_HDR =
	{
		.eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
		.ePixelDepth    = EARLYINIT_DATAPRECISION_10,
		.eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
		.u32FpsX1000    = 25000,
		.u32Width       = 2560,
		.u32Height      = 1440,
		.u32GainX1024   = 1024,
		.u32ShutterUs   = 8000,
		.u32ShutterShortUs = 100,
		.u32GainShortX1024 = 1024,
		.u8ResId           = 0,
		.u8HdrMode         = 1,
		.u32CropX       = 0,
		.u32CropY       = 0
	};
#endif
EarlyInitSensorInfo_t* pResList[] ={
	#if _SC431HAI_RES_4M30_
		&_gSC431HAIInfo_4M30,
	#endif
	#if _SC431HAI5_RES_4M25_HDR
		&_gSC431HAIInfo_4M25_HDR,
	#endif
};

/*****************  Interface VER2 ****************/
static void _memcpy(void* dest, void* src, unsigned int len)
{
    unsigned int n;
    for(n=0; n<len; ++n)
        ((char*)dest)[n] = ((char*)src)[n];
}

static unsigned int SC431HAIEarlyInitShutterAndFpsLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period 22222
unsigned int vts_30fps = 1500;

    unsigned char n;
    unsigned int half_lines = 0;
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

    vts_30fps = (vts_30fps * 30000) / nFpsX1000;

    half_lines = (1000 * nShutterUsLef*2 ) / Preview_line_period; // Preview_line_period in ns
    if(half_lines <= 4)
    {
        half_lines = 4;
    }
    if(half_lines > 2*(vts_30fps - 11))
    {
        vts = (half_lines + 12)/2 ;
    }
    else
    {
        vts = vts_30fps;// 15fps ,meanwhile 0xa6a
    }
    half_lines = half_lines<<4;

    expo_reg[0].u16Data = (half_lines>>16) & 0x000f;
    expo_reg[1].u16Data = (half_lines>>8) & 0x00ff;
    expo_reg[2].u16Data = (half_lines>>0) & 0x00f0;

    expo_reg[3].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[4].u16Data = (vts >> 0) & 0x00ff;


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
static unsigned int SC431HAIEarlyInitShutterAndFpsHdr( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period_HDR 11111
unsigned int vtsHdr =3000;

    unsigned char n;
    unsigned int half_lines_lef = 0;
	unsigned int half_lines_sef = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
	unsigned int max_short_exp=184;
    I2cCfg_t expo_reg[] =
    {
        /*longExposure*/
        {0x3e00, 0x00},
        {0x3e01, 0x5d},
        {0x3e02, 0x00},
		 /*VTS*/
        {0x320e, 0x0B}, //3
        {0x320f, 0xB8}, //4
        /*shortExposure*/
        {0x3e22, 0x00}, // expo[3:0]
        {0x3e04, 0x21}, // expo[7:0]
        {0x3e05, 0x00}, // expo[7:4]

    };
	 if(nFpsX1000 > pHandle->tInfo.u32FpsX1000)
    {
        nFpsX1000 = pHandle->tInfo.u32FpsX1000;
    }
	vts=(vtsHdr*pHandle->tInfo.u32FpsX1000)/nFpsX1000;
	//long exposure
    half_lines_lef = (1000 * nShutterUsLef ) / (Preview_line_period_HDR*2); // Preview_line_period in ns
	half_lines_lef=4*half_lines_lef;
    if(half_lines_lef <= 6)
    {
        half_lines_lef = 6;
    }
	if (half_lines_lef >  2 * (vtsHdr - max_short_exp) - 21) {
        half_lines_lef = 2 * (vtsHdr - max_short_exp) - 21;
    }
    half_lines_lef = half_lines_lef<<4;
	//short exposure
	 half_lines_sef = (1000 * nShutterUsSef ) / (Preview_line_period_HDR*2); // Preview_line_period in ns
	 half_lines_sef=4*half_lines_sef;
    if(half_lines_sef <= 6)
    {
        half_lines_sef = 6;
    }
   if (half_lines_sef >  2 * (max_short_exp) - 19) {
        half_lines_sef = 2 * (max_short_exp) - 19;
    }
    half_lines_sef = half_lines_sef<<4;
	
    expo_reg[0].u16Data = (half_lines_lef>>16) & 0x000f;
    expo_reg[1].u16Data = (half_lines_lef>>8) & 0x00ff;
    expo_reg[2].u16Data = (half_lines_lef>>0) & 0x00f0;
    expo_reg[3].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[4].u16Data = (vts >> 0) & 0x00ff;
	expo_reg[5].u16Data = (half_lines_sef>>16) & 0x000f;
    expo_reg[6].u16Data = (half_lines_sef>>8) & 0x00ff;
    expo_reg[7].u16Data = (half_lines_sef>>0) & 0x00f0;

    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);
	
	pHandle->tInfo.u32ShutterUs = nShutterUsLef;
    pHandle->tInfo.u32ShutterShortUs = nShutterUsSef;
    pHandle->tInfo.u32FpsX1000 = nFpsX1000;
    return nNumRegs; //Return number of sensor registers to write
}

static unsigned int SC431HAIEarlyInitShutterAndFpsEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    switch(pHandle->uPresetId)
    {
    case SC431HAI_PRESET_4M30:
        return SC431HAIEarlyInitShutterAndFpsLinear(pHandle, nFpsX1000, nShutterUsLef, nShutterUsSef, pRegs, nMaxNumRegs);
        break;
    case SC431HAI_PRESET_4K25_HDR:
        return SC431HAIEarlyInitShutterAndFpsHdr(pHandle, nFpsX1000, nShutterUsLef, nShutterUsSef, pRegs, nMaxNumRegs);
        break;
    }
    return 0;
}

typedef unsigned long long _u64;
typedef unsigned long _u32;
typedef unsigned short _u16;
typedef unsigned char _u8;
static void SC431HAIEarlyInitAEGainCalculate(unsigned int *gain, u8 *ANA_Fine_gain_reg,u8 *Coarse_gain_reg,u8 *DIG_Fine_gain_reg,u8 *DIG_gain_reg){
    #define SENSOR_MIN_GAIN      (1 * 1024)
	#define SENSOR_MAX_GAIN     764 
    /*TODO: Parsing gain to sensor i2c setting*/
	_u8 Coarse_gain = 1,DIG_gain=1;
    _u32 Dcg_gainx100 = 1, ANA_Fine_gainx32 = 1;
    
	if (*gain <= 1024) {
        *gain = 1024;
    } else if (*gain > (SENSOR_MAX_GAIN*1024)) {
        *gain = SENSOR_MAX_GAIN*1024;
    }

    if (*gain < 1577) // start again  1.540 * 1024
    {
        Dcg_gainx100 = 1000;      Coarse_gain = 1;     DIG_gain=1;
        *Coarse_gain_reg = 0x00; *DIG_gain_reg=0x0;  *DIG_Fine_gain_reg=0x80;
    }
    else if (*gain < 3154) // 3.080 * 1024
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 1;     DIG_gain=1;
        *Coarse_gain_reg = 0x80; *DIG_gain_reg=0x0;  *DIG_Fine_gain_reg=0x80;
    }
    else if (*gain < 6308) // 6.16 * 1024
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 2;     DIG_gain=1;
        *Coarse_gain_reg = 0x81; *DIG_gain_reg=0x0;  *DIG_Fine_gain_reg=0x80;
    }
    else if (*gain < 12616)// 12.32 * 1024
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 4;     DIG_gain=1;
        *Coarse_gain_reg = 0x83; *DIG_gain_reg=0x0;  *DIG_Fine_gain_reg=0x80;
    }
    else if (*gain < 25232)// 24.64 * 1024 // end again
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 8;     DIG_gain=1;
        *Coarse_gain_reg = 0x87; *DIG_gain_reg=0x0;  *DIG_Fine_gain_reg=0x80;
    }
    else if (*gain < 50463)// 48.51 * 1024 // end again
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 16;     DIG_gain=1;
        *Coarse_gain_reg = 0x8f; *DIG_gain_reg=0x0;  *DIG_Fine_gain_reg=0x80;
    }
#if 1
    else if (*gain < 50463 * 2) // start dgain
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 16;     DIG_gain=1;       ANA_Fine_gainx32=127;
        *Coarse_gain_reg = 0x8f; *DIG_gain_reg=0x0;  *ANA_Fine_gain_reg=0x3f;
    }
    else if (*gain < 50463 * 4)
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 16;     DIG_gain=2;       ANA_Fine_gainx32=127;
        *Coarse_gain_reg = 0x8f; *DIG_gain_reg=0x1;  *ANA_Fine_gain_reg=0x3f;
    }
    else if (*gain < 50463 * 8)
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 16;     DIG_gain=4;       ANA_Fine_gainx32=127;
        *Coarse_gain_reg = 0x8f; *DIG_gain_reg=0x3;  *ANA_Fine_gain_reg=0x3f;
    }
    else if (*gain <= SENSOR_MAX_GAIN)
    {
        Dcg_gainx100 = 1540;      Coarse_gain = 16;     DIG_gain=8;       ANA_Fine_gainx32=127;
        *Coarse_gain_reg = 0x8f; *DIG_gain_reg=0x7; *ANA_Fine_gain_reg=0x3f;
    }
#endif

    if(*gain < 1577)
    {
        *ANA_Fine_gain_reg = (_u8)(1000ULL * (*gain) / (Dcg_gainx100 * Coarse_gain) / 32);
    }
    else if(*gain < 50463)
    {
        *ANA_Fine_gain_reg = (_u8)(1000ULL * (*gain) / (Dcg_gainx100 * Coarse_gain) / 32);
    }else{
        *DIG_Fine_gain_reg = (_u8)(8000ULL * (*gain) /(Dcg_gainx100 * Coarse_gain * DIG_gain) / ANA_Fine_gainx32);
	}
}
static unsigned int SC431HAIEarlyInitGainLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     764 
unsigned char n;
    /*TODO: Parsing gain to sensor i2c setting*/
    _u8 Coarse_gain_reg = 0,DIG_gain_reg=0, ANA_Fine_gain_reg= 0x20,DIG_Fine_gain_reg=0x80;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3e06, 0x00},
        {0x3e07, 0x80},
		{0x3e08, 0x00|0x03},
        {0x3e09, 0x00},
    };
    SC431HAIEarlyInitAEGainCalculate(&u32GainLefX1024,&ANA_Fine_gain_reg,&Coarse_gain_reg,&DIG_Fine_gain_reg,&DIG_gain_reg);
	
	gain_reg[3].u16Data = ANA_Fine_gain_reg; // 0x3e09
    gain_reg[2].u16Data = Coarse_gain_reg; // 0x3e08
    gain_reg[1].u16Data = DIG_Fine_gain_reg; // 0x3e07
    gain_reg[0].u16Data = DIG_gain_reg; // 0x3e06

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;
    return nNumRegs;
}
static unsigned int SC431HAIEarlyInitGainHDR( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     764 
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    _u8 Coarse_gain_regLong = 0,DIG_gain_regLong=0, ANA_Fine_gain_regLong= 0x20,DIG_Fine_gain_regLong=0x80;
	_u8 Coarse_gain_regShort = 0,DIG_gain_regShort=0, ANA_Fine_gain_regShort= 0x20,DIG_Fine_gain_regShort=0x80;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
		//longExposureGain
        {0x3e06, 0x00},
        {0x3e07, 0x80},
		{0x3e08, 0x00|0x03},
        {0x3e09, 0x00},
		//shortExposureGain
		{0x3e10, 0x00},
        {0x3e11, 0x80},
        {0x3e12, 0x00},
        {0x3e13, 0x20},
    };
    SC431HAIEarlyInitAEGainCalculate(&u32GainLefX1024,&ANA_Fine_gain_regLong,&Coarse_gain_regLong,&DIG_Fine_gain_regLong,&DIG_gain_regLong);
	SC431HAIEarlyInitAEGainCalculate(&u32GainSefX1024,&ANA_Fine_gain_regShort,&Coarse_gain_regShort,&DIG_Fine_gain_regShort,&DIG_gain_regShort);
	gain_reg[3].u16Data = ANA_Fine_gain_regLong; // 0x3e09
    gain_reg[2].u16Data = Coarse_gain_regLong; // 0x3e08
    gain_reg[1].u16Data = DIG_Fine_gain_regLong; // 0x3e07
    gain_reg[0].u16Data = DIG_gain_regLong; // 0x3e06
	
	gain_reg[7].u16Data = ANA_Fine_gain_regShort; // 0x3e13
    gain_reg[6].u16Data = Coarse_gain_regShort; // 0x3e12
    gain_reg[5].u16Data = DIG_Fine_gain_regShort; // 0x3e11
    gain_reg[4].u16Data = DIG_gain_regShort; // 0x3e10

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;
	pHandle->tInfo.u32GainShortX1024 = u32GainSefX1024;
    return nNumRegs;
}
static unsigned int SC431HAIEarlyInitGainEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    switch(pHandle->uPresetId)
    {
    case SC431HAI_PRESET_4M30:
        return SC431HAIEarlyInitGainLinear(pHandle, u32GainLefX1024, u32GainSefX1024, pRegs, nMaxNumRegs);
        break;
    case SC431HAI_PRESET_4K25_HDR:
        return SC431HAIEarlyInitGainHDR(pHandle, u32GainLefX1024, u32GainSefX1024, pRegs, nMaxNumRegs);
        break;
    }
    return 0;
}
static unsigned int SC431HAIEarlyInitSelPresetId(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uPresetId)
{
    pHandle->uPresetId = uPresetId;
    switch(uPresetId)
    {
    case SC431HAI_PRESET_4M30:
        pHandle->pTable = (void*) Sensor_init_table_4M30;
        pHandle->uTableSize = sizeof(Sensor_init_table_4M30);
        _memcpy(&pHandle->tInfo, &_gSC431HAIInfo_4M30, sizeof(_gSC431HAIInfo_4M30));
        break;
    case SC431HAI_PRESET_4K25_HDR:
        pHandle->pTable = (void*) Sensor_init_table_4M25_hdr;
        pHandle->uTableSize = sizeof(Sensor_init_table_4M25_hdr);
        _memcpy(&pHandle->tInfo, &_gSC431HAIInfo_4M25_HDR, sizeof(_gSC431HAIInfo_4M25_HDR));
        break;
   
    default:
        return 0;
        break;
    }
    return 0;
}
unsigned int SC431HAIEarlyInitSelResId(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId)
{
    if(uResId > sizeof(pResList)/sizeof(pResList[0]))
    {
        return -1;
    }

    if(pResList[uResId]->u8HdrMode==uHdrMode)
    {
        SC431HAIEarlyInitSelPresetId(pHandle, uResId);
        return 0;
    }

    return -1;
}

unsigned int SC431HAIInitHandle( EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = SC431HAIEarlyInitGainEx;
    pHandle->fpShutterFpsParserEx = SC431HAIEarlyInitShutterAndFpsEx;
    pHandle->fpSelResId           = SC431HAIEarlyInitSelResId;


    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = 0;
    pHandle->tCurCfg.pTable       = (void*) Sensor_init_table_4M30;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table_4M30);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gSC431HAIInfo_4M30, sizeof(_gSC431HAIInfo_4M30));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;
    return 0;
}
SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2( SC431HAI, SC431HAIInitHandle);
/* Sensor EarlyInit implementation end*/
