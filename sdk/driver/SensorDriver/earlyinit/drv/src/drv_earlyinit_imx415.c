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
#include "IMX415_init_table.h"         /* Sensor initial table */
#include "cam_os_wrapper.h"

#define IMX415_PRESET_4K30           0
#define IMX415_PRESET_4K15_HDR       1
#define IMX415_PRESET_4K30_HDR       2
#define IMX415_PRESET_2M60           3
#define IMX415_PRESET_2560x1440_30   4
#define IMX415_PRESET_3840x2160_24   5

typedef unsigned long long _u64;
typedef unsigned long _u32;
typedef unsigned short _u16;
extern _u64 intlog10(_u32 value);
extern _u64 EXT_log_2(_u32 value);
extern _u32 round_float(_u32 x, _u32 piont_offset);

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(IMX415)

typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

#if _IMX415_RES_4K30_
static EarlyInitSensorInfo_t _gIMX415Info_4K30 =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_GB,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 3840,
    .u32Height      = 2160,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 7,
    .u8HdrMode         = 0,
    .u32TimeoutMs   = 100,
    .u32CropX       = 12,
    .u32CropY       = 16,
    .u8NumLanes     = 4
};
#endif

#if _IMX415_RES_2M60_
static EarlyInitSensorInfo_t _gIMX415Info_2m60 =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_GB,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_12,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 60000,
    .u32Width       = 1920,
    .u32Height      = 1080,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 6,
    .u8HdrMode         = 0,
    .u32TimeoutMs   = 100,
    .u32CropX       = 0,
    .u32CropY       = 6,
    .u8NumLanes     = 4
};
#endif

#if _IMX415_RES_2560x1440_30_
static EarlyInitSensorInfo_t _gIMX415Info_2560x1440_30 =
{
    .eBayerID          = E_EARLYINIT_SNR_BAYER_GB,
    .ePixelDepth       = EARLYINIT_DATAPRECISION_10,
    .eIfBusType        = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000       = 30000,
    .u32Width          = 2560,
    .u32Height         = 1440,
    .u32GainX1024      = 1024,
    .u32ShutterUs      = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 5,
    .u8HdrMode         = 0,
    .u32TimeoutMs      = 100,
    .u32CropX          = 0,
    .u32CropY          = 0,
    .u8NumLanes        = 4
};
#endif

#if _IMX415_RES_3840x2160_24_
static EarlyInitSensorInfo_t _gIMX415Info_3840x2160_24 =
{
    .eBayerID          = E_EARLYINIT_SNR_BAYER_GB,
    .ePixelDepth       = EARLYINIT_DATAPRECISION_10,
    .eIfBusType        = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000       = 24000,
    .u32Width          = 3840,
    .u32Height         = 2160,
    .u32GainX1024      = 1024,
    .u32ShutterUs      = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 12,
    .u8HdrMode         = 0,
    .u32TimeoutMs      = 100,
    .u32CropX          = 0,
    .u32CropY          = 0,
    .u8NumLanes        = 4
};
#endif


#if _IMX415_RES_4K15_HDR
static EarlyInitSensorInfo_t _gIMX415Info_4K15_HDR =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_GB,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_12, //for RES ID 0
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 15000,
    .u32Width       = 3840,
    .u32Height      = 2160,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 80,
    .u32GainShortX1024 = 1024,
    .u8ResId           = 0,
    .u8HdrMode         = 1,
    .u32TimeoutMs   = 100,
    .u32CropX       = 12,
    .u32CropY       = 16,
    .u8NumLanes     = 4
};
#endif

#if _IMX415_RES_4K30_HDR
static EarlyInitSensorInfo_t _gIMX415Info_4K30_HDR =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_GB,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10, //for RES ID 2
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 30000,
    .u32Width       = 3840,
    .u32Height      = 2160,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 80,
    .u32GainShortX1024 = 1024,
    .u8ResId           = 2,
    .u8HdrMode         = 1,
    .u32TimeoutMs   = 100,
    .u32CropX       = 12,
    .u32CropY       = 16,
    .u8NumLanes     = 4
};
#endif

EarlyInitSensorInfo_t* pResList[] =
{
#if _IMX415_RES_4K30_
    &_gIMX415Info_4K30,
#endif
#if _IMX415_RES_4K15_HDR
    &_gIMX415Info_4K15_HDR,
#endif
#if _IMX415_RES_4K30_HDR
    &_gIMX415Info_4K30_HDR,
#endif
#if _IMX415_RES_2M60_
    &_gIMX415Info_2m60,
#endif
#if _IMX415_RES_2560x1440_30_
    &_gIMX415Info_2560x1440_30,
#endif
#if _IMX415_RES_3840x2160_24_
    &_gIMX415Info_3840x2160_24
#endif
};

/*****************  Interface VER2 ****************/
static void _memcpy(void* dest, void* src, unsigned int len)
{
    unsigned int n;
    for(n=0; n<len; ++n)
        ((char*)dest)[n] = ((char*)src)[n];
}

static unsigned int IMX415EarlyInitShutterAndFpsLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x3052, 0x00},
        {0x3051, 0x00},
        {0x3050, 0x08},
        /*vts*/
        {0x3026, 0x00},
        {0x3025, 0x08},
        {0x3024, 0xCA},
    };

    unsigned int _Preview_line_period = 14814; // hts=33.333/2250=14814
    unsigned int _vts_30fps = 2250; //for 29.091fps @ MCLK=36MHz

    if(nFpsX1000 > pHandle->tInfo.u32FpsX1000)
    {
        nFpsX1000 = pHandle->tInfo.u32FpsX1000;
    }
    switch(pHandle->uPresetId)
    {
    case IMX415_PRESET_4K30:
        _Preview_line_period = 14814;
        _vts_30fps = 2250;
        break;
    case IMX415_PRESET_2M60:
        _Preview_line_period = 7259;
        _vts_30fps = 2250;
        break;
    case IMX415_PRESET_2560x1440_30:
        _Preview_line_period = 14815;
        _vts_30fps = 2250;
        break;
    case IMX415_PRESET_3840x2160_24:
        _Preview_line_period = 18100;
        _vts_30fps = 2250;
        break;
    case IMX415_PRESET_4K15_HDR:
        _Preview_line_period = 14814;
        _vts_30fps = 2250;
        break;
    case IMX415_PRESET_4K30_HDR:
        _Preview_line_period = 7259;
        _vts_30fps = 2296;
        break;
    }

    switch(pHandle->uPresetId)
    {
    default:
        /*VTS*/
        vts =  (_vts_30fps*30000)/nFpsX1000;
        if(nFpsX1000<1000)    //for old method
            vts = (_vts_30fps*30)/nFpsX1000;
        break;
    case IMX415_PRESET_2M60:
        /*VTS*/
        vts =  (_vts_30fps*60000)/nFpsX1000;
        if(nFpsX1000<1000)    //for old method
            vts = (_vts_30fps*60)/nFpsX1000;
        break;
    }

    /*Exposure time*/
    lines = (1000*nShutterUsLef)/_Preview_line_period;

    if(lines>vts-4)
        vts = lines +4;

    lines = vts-lines-1;

    expo_reg[0].u16Data = (lines>>16) & 0x0003;
    expo_reg[1].u16Data = (lines>>8) & 0x00ff;
    expo_reg[2].u16Data = (lines>>0) & 0x00ff;

    expo_reg[3].u16Data = (vts >> 16) & 0x0003;
    expo_reg[4].u16Data = (vts >> 8) & 0x00ff;
    expo_reg[5].u16Data = (vts >> 0) & 0x00ff;

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

static unsigned int IMX415EarlyInitShutterAndFpsHdr( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned int expo_lines_lef = 0, vts = 0, fsc = 0;
    unsigned int expo_line_sef = 0;
    unsigned int _vts_30fps = 0;
    unsigned int _min_rhs1 = 0, _min_shr1 = 0, _max_shr0 = 0;
    int nNumRegs, n;
    unsigned int cur_line_period = 0;

    I2cCfg_t expo_reg[] =
    {
        /*SHR0*/
        {0x3052, 0x00}, //0
        {0x3051, 0x16}, //1
        {0x3050, 0x22}, //2

        /*VTS*/
        {0x3026, 0x00}, //3
        {0x3025, 0x08}, //4
        {0x3024, 0xCA}, //5

        /*RHS1 reg*/
        {0x3062, 0x00}, //6
        {0x3061, 0x00}, //7
        {0x3060, 0x11}, //8

        /*SHR1 reg*/
        {0x3056, 0x00}, //9
        {0x3055, 0x00}, //10
        {0x3054, 0x09}, //11
    };

    if(nFpsX1000 > pHandle->tInfo.u32FpsX1000)
    {
        nFpsX1000 = pHandle->tInfo.u32FpsX1000;
    }
    switch(pHandle->uPresetId)
    {
    case IMX415_PRESET_4K15_HDR:
        cur_line_period = 29630;
        _vts_30fps = 2250;
        _min_rhs1 = 437;
        break;
    case IMX415_PRESET_4K30_HDR:
        cur_line_period = 14493;
        _vts_30fps = 2300;
        _min_rhs1 = 437;
        break;
    }
    vts =  (_vts_30fps*pHandle->tInfo.u32FpsX1000)/nFpsX1000;
    /*LEF*/
    expo_lines_lef = (1000 * nShutterUsLef + (cur_line_period >> 1)) / cur_line_period;
    if(expo_lines_lef>vts-4)
        vts = expo_lines_lef +4;
    fsc = vts * 2;
    //params->fsc = ((fsc >> 2) << 2)+ 4;                  // 4n
    _max_shr0 = fsc - expo_lines_lef - 8;
    _max_shr0 = (fsc - 8) - expo_lines_lef;
    if(_max_shr0 < (_min_rhs1+9))
        _max_shr0 = _min_rhs1+9;
    _max_shr0 = ((_max_shr0 >> 1) << 1) + 2;

    if (expo_lines_lef > (fsc - _min_rhs1 - 9)) {
        vts = (expo_lines_lef + _min_rhs1 + 9) / 2;
    }
    else{
      vts = _vts_30fps;
    }
    //params->expo.expo_lines = expo_lines_lef;
    /*
    CamOsPrintf("[%s] us %u, expo_lines_lef %u, vts %u, SHR0 %u \n", __FUNCTION__,
                                                                     nShutterUsLef, \
                                                                     expo_lines_lef, \
                                                                     vts, \
                                                                     _max_shr0);
                                                                     */
    expo_reg[0].u16Data = (_max_shr0 >> 16) & 0x0003;
    expo_reg[1].u16Data = (_max_shr0 >> 8) & 0x00ff;
    expo_reg[2].u16Data = (_max_shr0 >> 0) & 0x00ff;

    expo_reg[3].u16Data = ((vts) >> 16) & 0x0003;
    expo_reg[4].u16Data = ((vts) >> 8) & 0x00ff;
    expo_reg[5].u16Data = ((vts) >> 0) & 0x00ff;


    /*SEF*/
    expo_line_sef = (1000 * nShutterUsSef + (cur_line_period >> 1)) / cur_line_period;
    //params->expo.expo_sef_us = us;
    //params->min_rhs1 = 437;     // 4n+1 fix to 269, 337 //5ms: 429

    _min_shr1 = _min_rhs1 - expo_line_sef;
    if((expo_line_sef > _min_rhs1) || ((_min_shr1) <  9))
        _min_shr1 = 9;
    _min_shr1 = ((_min_shr1 >> 1) << 1) + 1;
    /*
    CamOsPrintf("[%s] us %u, expo_line_sef %u rhs %u shr1 %u\n", __FUNCTION__,
                                                                 nShutterUsSef, \
                                                                 expo_line_sef, \
                                                                 _min_rhs1, \
                                                                 _min_shr1
               );
    */
    expo_reg[6].u16Data = (_min_rhs1 >>16) & 0x03;
    expo_reg[7].u16Data = (_min_rhs1 >>8) & 0xff;
    expo_reg[8].u16Data = (_min_rhs1 >>0) & 0xff;

    expo_reg[9].u16Data = (_min_shr1 >> 16) & 0x0003;
    expo_reg[10].u16Data = (_min_shr1 >> 8) & 0x00ff;
    expo_reg[11].u16Data = (_min_shr1 >> 0) & 0x00ff;

    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

    pHandle->tInfo.u32ShutterUs = nShutterUsLef;
    pHandle->tInfo.u32ShutterShortUs = nShutterUsSef;
    pHandle->tInfo.u32FpsX1000 = nFpsX1000;

    return nNumRegs;
}


/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int IMX415EarlyInitShutterAndFpsEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned int nFpsX1000, unsigned int nShutterUsLef, unsigned int nShutterUsSef, unsigned int nShutterUsMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    switch(pHandle->uPresetId)
    {
    case IMX415_PRESET_4K30:
    case IMX415_PRESET_2M60:
    case IMX415_PRESET_2560x1440_30:
    case IMX415_PRESET_3840x2160_24:
        return IMX415EarlyInitShutterAndFpsLinear(pHandle, nFpsX1000, nShutterUsLef, nShutterUsSef, pRegs, nMaxNumRegs);
        break;
    case IMX415_PRESET_4K15_HDR:
    case IMX415_PRESET_4K30_HDR:
        return IMX415EarlyInitShutterAndFpsHdr(pHandle, nFpsX1000, nShutterUsLef, nShutterUsSef, pRegs, nMaxNumRegs);
        break;
    }
    return 0;
}

static void IMX415EarlyInitAEGainCalculate(unsigned int gain, unsigned short *gain_reg_tmp)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     (3981 * 1024)                  // max sensor again, a-gain * conversion-gain*d-gain
    unsigned int gain_temp;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    gain_temp = (unsigned int)((200*(EXT_log_2(gain)-167772160ULL)) >> 24);
    *gain_reg_tmp = round_float(gain_temp, 1) & 0xFF;
}

static int IMX415EarlyInitSetAEGainHDR(EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    int n, nNumRegs = 0;
    unsigned short gain_reg_value = 0;
    I2cCfg_t gain_reg[] =
    {
        /*LEF gain reg*/
        {0x3090, 0x2A}, //0
        {0x3091, 0x00}, //1
        /*SEF gain reg*/
        {0x3092, 0x20}, //2
        {0x3093, 0x00}, //3
    };

    IMX415EarlyInitAEGainCalculate(u32GainLefX1024, &gain_reg_value);
    gain_reg[0].u16Data = gain_reg_value & 0xff;
    gain_reg[1].u16Data = (gain_reg_value >> 8) & 0xff;

    IMX415EarlyInitAEGainCalculate(u32GainSefX1024, &gain_reg_value);
    gain_reg[2].u16Data = gain_reg_value & 0xff;
    gain_reg[3].u16Data = (gain_reg_value >> 8) & 0xff;

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

static unsigned int IMX415EarlyInitGainLinear( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     (3981 * 1024)                  // max sensor again, a-gain * conversion-gain*d-gain
    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    _u64 nGainDouble;
    int nNumRegs = 0;
    I2cCfg_t gain_reg[] = {
        {0x3090, 0x2A},//low bit
        {0x3091, 0x00},//hcg mode,bit 4
    };

    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;

    if(u32GainLefX1024 < SENSOR_MIN_GAIN)
        u32GainLefX1024 = SENSOR_MIN_GAIN;
    else if(u32GainLefX1024 >= SENSOR_MAX_GAIN)
        u32GainLefX1024 = SENSOR_MAX_GAIN;

    nGainDouble = 20*(intlog10(u32GainLefX1024)-intlog10(1024));

    gain_reg[0].u16Data = (_u16)(((nGainDouble*10)>> 24)/3);

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;
    return nNumRegs;
}

static unsigned int IMX415EarlyInitGainEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    switch(pHandle->uPresetId)
    {
    case IMX415_PRESET_4K30:
    case IMX415_PRESET_2M60:
    case IMX415_PRESET_2560x1440_30:
    case IMX415_PRESET_3840x2160_24:
        return IMX415EarlyInitGainLinear(pHandle, u32GainLefX1024, u32GainSefX1024, pRegs, nMaxNumRegs);
        break;
    case IMX415_PRESET_4K15_HDR:
    case IMX415_PRESET_4K30_HDR:
        return IMX415EarlyInitSetAEGainHDR(pHandle, u32GainLefX1024, u32GainSefX1024, pRegs, nMaxNumRegs);
        break;
    }
    return 0;
}

static unsigned int IMX415EarlyInitSelPresetId(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uPresetId)
{
    pHandle->uPresetId = uPresetId;
    switch(uPresetId)
    {
    case IMX415_PRESET_4K30:
        pHandle->pTable = (void*) Sensor_init_table_4K30;
        pHandle->uTableSize = sizeof(Sensor_init_table_4K30);
        _memcpy(&pHandle->tInfo, &_gIMX415Info_4K30, sizeof(_gIMX415Info_4K30));
        break;
    case IMX415_PRESET_4K15_HDR:
        pHandle->pTable = (void*) Sensor_init_table_4K15_hdr;
        pHandle->uTableSize = sizeof(Sensor_init_table_4K15_hdr);
        _memcpy(&pHandle->tInfo, &_gIMX415Info_4K15_HDR, sizeof(_gIMX415Info_4K15_HDR));
        break;
    case IMX415_PRESET_4K30_HDR:
        pHandle->pTable  = (void*) Sensor_init_table_4k30_hdr;
        pHandle->uTableSize = sizeof(Sensor_init_table_4k30_hdr);
        _memcpy(&pHandle->tInfo, &_gIMX415Info_4K30_HDR, sizeof(_gIMX415Info_4K30_HDR));
        break;
    case IMX415_PRESET_2M60:
        pHandle->pTable  = (void*) Sensor_init_table_2M60;
        pHandle->uTableSize = sizeof(Sensor_init_table_2M60);
        _memcpy(&pHandle->tInfo, &_gIMX415Info_2m60, sizeof(_gIMX415Info_2m60));
        break;
    case IMX415_PRESET_2560x1440_30:
        pHandle->pTable  = (void*) Sensor_init_table_2560x1440_30_linear_;
        pHandle->uTableSize = sizeof(Sensor_init_table_2560x1440_30_linear_);
        _memcpy(&pHandle->tInfo, &_gIMX415Info_2560x1440_30, sizeof(_gIMX415Info_2560x1440_30));
        break;
    case IMX415_PRESET_3840x2160_24:
        pHandle->pTable  = (void*) Sensor_init_table_3840x2160_24_linear_;
        pHandle->uTableSize = sizeof(Sensor_init_table_3840x2160_24_linear_);
        _memcpy(&pHandle->tInfo, &_gIMX415Info_3840x2160_24, sizeof(_gIMX415Info_3840x2160_24));
        break;
    default:
        return 0;
        break;
    }
    return 0;
}

unsigned int IMX415EarlyInitSelResId(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId)
{
    if(uResId >= sizeof(pResList)/sizeof(pResList[0]))
    {
        return -1;
    }

    if(pResList[uResId]->u8HdrMode==uHdrMode)
    {
        IMX415EarlyInitSelPresetId(pHandle, uResId);
        return 0;
    }

    return -1;
}

static unsigned int IMX415EarlyInitSelOrientEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uMirror, unsigned char uFlip, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    /*TODO: Parsing mirror-flip to sensor i2c setting*/
    unsigned char n;
    I2cCfg_t orient_reg[] =
    {
        {0x3030, 0x00},//low bit
    };

    orient_reg[0].u16Data = ((uFlip << 1) & 0x2)| (uMirror & 0x1);

    /*copy result*/
    for(n=0;n<sizeof(orient_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)orient_reg)[n];
    }
    pHandle->tInfo.u8Mirror = uMirror;
    pHandle->tInfo.u8Flip = uFlip;
    return sizeof(orient_reg)/sizeof(orient_reg[0]);
}

static unsigned int IMX415EarlyInitSelStreamOnoff( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uStreamOnoff, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    I2cCfg_t stream_onoff_reg[] =
    {
        {0x3000, 0x00},  //Operating
        {0x3002, 0x00},  //Master mode start
    };
    if (uStreamOnoff == 0)
    {
        stream_onoff_reg[0].u16Data = 1;
        stream_onoff_reg[1].u16Data = 1;
    }

    /*copy result*/
    for(n=0;n<sizeof(stream_onoff_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)stream_onoff_reg)[n];
    }
    pHandle->tInfo.u8StreamOnoff = uStreamOnoff;
    return sizeof(stream_onoff_reg)/sizeof(stream_onoff_reg[0]);
}

unsigned int IMX415InitHandle( EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = IMX415EarlyInitGainEx;
    pHandle->fpShutterFpsParserEx = IMX415EarlyInitShutterAndFpsEx;
    pHandle->fpSelResId           = IMX415EarlyInitSelResId;
    pHandle->fpOrientParerEx      = IMX415EarlyInitSelOrientEx;
    pHandle->fpStreamOnoffParserEx= IMX415EarlyInitSelStreamOnoff;

    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = 0;
    pHandle->tCurCfg.pTable       = (void*) Sensor_init_table_4K30;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table_4K30);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gIMX415Info_4K30, sizeof(_gIMX415Info_4K30));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;
    return 0;
}

SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2( IMX415, IMX415InitHandle);
