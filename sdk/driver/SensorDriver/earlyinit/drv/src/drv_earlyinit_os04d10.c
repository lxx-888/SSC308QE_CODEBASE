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
#include "earlyinit_drv_iic.h"
#include "cam_os_wrapper.h"
#include "OS04D10_init_table.h"         /* Sensor initial table */

/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(OS04D10)

typedef unsigned long long _u64;
typedef unsigned long _u32;
typedef unsigned short _u16;

static EarlyInitHwRes_t* gHwRes[_MAX_SENSOR_PADID];

static EarlyInitSensorInfo_t _gOS04D10Info_4M30 =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 15000,
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

static EarlyInitSensorInfo_t _gOS04D10Info_4M15=
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 15000,
    .u32Width       = 2560,
    .u32Height      = 1440,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 1,
    .u8HdrMode         = 0,
    .u32CropX       = 0,
    .u32CropY       = 0,
    .u8Mirror       = 0,
    .u8Flip         = 0,
    .u32TimeoutMs   = 200,
    .u8NumLanes     = 2,
    .u8StreamOnoff  = 0
};

static EarlyInitSensorInfo_t _gOS04D10Info_360P_FASTAE =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 15000,
    .u32Width       = 2560,
    .u32Height      = 1440,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 2,
    .u8HdrMode         = 0,
    .u32CropX       = 0,
    .u32CropY       = 0,
    .u8Mirror       = 0,
    .u8Flip         = 0,
    .u32TimeoutMs   = 2000,
    .u8NumLanes     = 2,
    .u8StreamOnoff  = 1
};

static EarlyInitSensorInfo_t _gOS04D10Info_360P_Change_4M30 =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .eIfBusType     = EARLYINIT_BUS_TYPE_MIPI,
    .u32FpsX1000    = 15000,
    .u32Width       = 2560,
    .u32Height      = 1440,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 3,
    .u8HdrMode         = 0,
    .u32CropX       = 0,
    .u32CropY       = 0,
    .u8Mirror       = 0,
    .u8Flip         = 0,
    .u32TimeoutMs   = 2000,
    .u8NumLanes     = 2,
    .u8StreamOnoff  = 1
};

EarlyInitSensorInfo_t* pResList[] =
{
#if _OS04D10_RES_4M30_
    &_gOS04D10Info_4M30,
#endif
#if _OS04D10_RES_4M15_
    &_gOS04D10Info_4M15,
#endif
#if _OS04D10_RES_360P_FASTAE_
    &_gOS04D10Info_360P_FASTAE,
#endif
#if _OS04D10_RES_360P_FASTAE_4M_
    &_gOS04D10Info_360P_Change_4M30
#endif
};

/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/

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

static unsigned int OS04D10EarlyInitShutterAndFps(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uFpsX1000, unsigned int uShtterLef, unsigned int uShtterSef, unsigned int uShtterMef, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
    int nNumRegs = 0;
    unsigned int Preview_line_period = 0;
    unsigned int max_fps_time = 0;
    unsigned int vts_maxfps = 0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x03, 0x01},
        {0x04, 0x00},
        /*vts*/
        {0x12, 0x01},//0x10 enable
        {0x0C, 0x04},//MSB
        {0x0D, 0x50},//LSB
    };
    switch(pHandle->uPresetId)
    {
        case OS04D10_PRESET_4M15:
            Preview_line_period = 43802; // 1s/(vts*vts_maxfps) ns
            vts_maxfps = 1552;      // VTS for 15fps(init table)
            max_fps_time = 15*1000;  //max_fps*1000ms
            break;
        case OS04D10_PRESET_4M30:
        case OS04D10_PRESET_360P_FASTAE:
        case OS04D10_PRESET_360P_FASTAE_4M:
            Preview_line_period = 22629;
            vts_maxfps = 1473;
            max_fps_time = 30*1000;
            break;
    }
    lines = (unsigned int)((1000*uShtterLef+(Preview_line_period>>1))/Preview_line_period); // Preview_line_period in ns
    if(lines <= 1)
        lines=1;
    if (lines > vts_maxfps-9) {
        vts = lines+9;
    }
    else
        vts = (vts_maxfps*max_fps_time + (uFpsX1000>>1))/ uFpsX1000;

    expo_reg[0].u16Data = (_u16)((lines>>8) & 0x00ff); //0x450>>8 & 0x00ff=>0x04
    expo_reg[1].u16Data = (_u16)((lines>>0) & 0x00ff);//0x450>>0 & 0x00ff=> 50 0x01 b8  //0x01b8=>440

    expo_reg[3].u16Data = (_u16)((vts >> 8) & 0x00ff);
    expo_reg[4].u16Data = (_u16)((vts >> 0) & 0x00ff);

    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }
    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);
    pHandle->tInfo.u32ShutterUs = uShtterLef;
    pHandle->tInfo.u32FpsX1000 = uFpsX1000;

    return nNumRegs; //Return number of sensor registers to write
}


/** @brief Convert gain to sensor register setting
@param[in] nGainX1024 target sensor gain x 1024
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/

static unsigned int OS04D10_GetOtpData(EarlyInitSensorDrvCfg_t *pHandle)
{
    _u16 I2cId = gHwRes[pHandle->uSensorPadId]->eI2cPort;
    _u16 otp_data = 0;
    static _u16 otpdata_1024[_MAX_SENSOR_PADID] = {0, 0, 0};
    _u16 slave_addr = 0x78;
    EARLYINIT_I2C_FMT fmt = I2C_FMT_A8D8;
    if ((I2cId <= sizeof(otpdata_1024)/sizeof(otpdata_1024[0]) && !otpdata_1024[I2cId]))
    {
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd,0x00, fmt);
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0x53,0x01, fmt);
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd,0x06, fmt);
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0x9f,0x00, fmt);
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0x84,0x40, fmt);

        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x06, fmt);
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0x89, 0x14, fmt);
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0x8b, 0x14, fmt);
        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0x2f, 0x01, fmt);
        earlyinit_i2c_read_register_pair(I2cId, slave_addr,0x31, &otp_data, fmt);
        if (otp_data)
        {
            otpdata_1024[pHandle->uSensorPadId] = 4 * 64 * 1024 / otp_data;
        }
    }
    CamOsPrintf(">>>>>>otpdata1024 %d===>>>>>>\n", otpdata_1024[pHandle->uSensorPadId]);
    return otpdata_1024[pHandle->uSensorPadId];
}


static unsigned int OS04D10EarlyInitGain(EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_MAX_GAIN     (MAX_A_GAIN * 32) // max sensor again, a-gain * conversion-gain*d-gain
//#define MAX_A_GAIN          (15872)      //15.5 * 1024
#define MAX_A_GAIN          (49725)      //255/16 * 3120/1024 * 1024

    /*TODO: Parsing gain to sensor i2c setting*/
    unsigned char n;
    int nNumRegs = 0;
    int again = 0;
    _u16 otpdata = 0;
    unsigned char lcg = 0;
    _u32 tmp_dgain = 0;
    I2cCfg_t gain_reg[] = {
        {0xfd, 0x01},
        {0x24, 0x20},//again 1x:0x10, 15.5x:0xf8
        {0x45, 0x01},
        {0xfd, 0x05},
        {0x39, 0x40},//dgain[7:0] 1x:0x40 32x:0x7ff
        {0x37, 0x00},//dgain[10:8]
        {0xfd, 0x01},
        {0x33, 0x03},
        {0x01, 0x02},
    };

    if(pHandle->uPresetId == OS04D10_PRESET_360P_FASTAE
        || pHandle->uPresetId == OS04D10_PRESET_360P_FASTAE_4M)
    {
        otpdata = OS04D10_GetOtpData(pHandle);
    }
    else
    {
        otpdata = 4*1024; // default opt data: 4x1024
    }

    //u32GainLefX1024 = (u32GainLefX1024 * SENSOR_MIN_GAIN + 512)>>10;


    if (u32GainLefX1024 <= 1024) {
        u32GainLefX1024 = 1024;
    } else if (u32GainLefX1024 > SENSOR_MAX_GAIN) {
        u32GainLefX1024 = SENSOR_MAX_GAIN - 1;
    }

    if(u32GainLefX1024 >= MAX_A_GAIN)
    {
        again = MAX_A_GAIN;
        tmp_dgain = u32GainLefX1024 *64/again;
    }
    else
    {
        again = u32GainLefX1024;
        tmp_dgain = 0x40;
    }

    if(again >= otpdata)
    {
        again = (again*16+(otpdata/2))/otpdata;
        lcg=0;
    }
    else
    {
        again = (again*16+512)/1024;
        lcg=1;
    }

    gain_reg[1].u16Data = (_u16)(again)&0xff; //low byte
    gain_reg[2].u16Data = lcg ? 0x00: 0x02;

    gain_reg[5].u16Data=(_u16)(tmp_dgain>>8&0x07);
    gain_reg[4].u16Data=(_u16)(tmp_dgain&0xFF);

    CamOsPrintf(">>>>>>>>>>>>otpdata %d, LCG %d  again %d dgain %d\n", otpdata, lcg, again, tmp_dgain);

    /*copy result*/
    for(n=0;n<sizeof(gain_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)gain_reg)[n];
    }
    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    if(nNumRegs > nMaxNumRegs)
    {
        CamOsPrintf(">>>>>>>>>>err gain reg num over range\n");
    }
    pHandle->tInfo.u32GainX1024 = u32GainLefX1024;
    return nNumRegs;
}


static unsigned int OS04D10EarlyInitGainEx(EarlyInitSensorDrvCfg_t *pHandle, unsigned int u32GainLefX1024, unsigned int u32GainSefX1024, unsigned int u32GainMefX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    return  OS04D10EarlyInitGain(pHandle, u32GainLefX1024, u32GainSefX1024, pRegs, nMaxNumRegs);
}

static unsigned int Os04D10EarlyInitSelPresetId(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uPresetId)
{
    pHandle->uPresetId = uPresetId;
    switch(uPresetId)
    {
    case OS04D10_PRESET_4M15:
        pHandle->pTable = (void*) Sensor_init_table_4M15;
        pHandle->uTableSize = sizeof(Sensor_init_table_4M15);
        _memcpy(&pHandle->tInfo, &_gOS04D10Info_4M15, sizeof(_gOS04D10Info_4M15));
        break;
    case OS04D10_PRESET_4M30:
        pHandle->pTable = (void*) Sensor_init_table_4M30;
        pHandle->uTableSize = sizeof(Sensor_init_table_4M30);
        _memcpy(&pHandle->tInfo, &_gOS04D10Info_4M30, sizeof(_gOS04D10Info_4M30));
        break;
    case OS04D10_PRESET_360P_FASTAE:
        pHandle->pTable = (void*) Sensor_init_table_360P120_FastAe;
        pHandle->uTableSize = sizeof(Sensor_init_table_360P120_FastAe);
        _memcpy(&pHandle->tInfo, &_gOS04D10Info_360P_FASTAE, sizeof(_gOS04D10Info_360P_FASTAE));
        break;
    case OS04D10_PRESET_360P_FASTAE_4M:
        pHandle->pTable = (void*) Sensor_init_table_360P120_FastAe_changeto_1440P;
        pHandle->uTableSize = sizeof(Sensor_init_table_360P120_FastAe_changeto_1440P);
        _memcpy(&pHandle->tInfo, &_gOS04D10Info_360P_Change_4M30, sizeof(_gOS04D10Info_360P_Change_4M30));
        break;
    default:
        break;
    }
    return 0;
}

unsigned int OS04D10EarlyInitSelResId(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId)
{
    if(uResId > sizeof(pResList)/sizeof(pResList[0]))
    {
        return -1;
    }
    if(pResList[uResId]->u8HdrMode == uHdrMode)
    {
        Os04D10EarlyInitSelPresetId(pHandle, uResId);
        return 0;
    }

    return -1;
}


static unsigned int OS04D10EarlyInitSelOrientEx( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uMirror, unsigned char uFlip, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    /*TODO: Parsing mirror-flip to sensor i2c setting*/
    unsigned char n;
    I2cCfg_t orient_reg[] =
    {
        {0xfd, 0x01},
        {0x32, 0x00}, //P1 M0F0 [1]:F [0]:M
        {0xfd, 0x02}, //
        {0x5e, 0x22}, //mem down en + enable auto BR first
        {0xfd, 0x01},
        {0x01, 0x01},
    };

    orient_reg[1].u16Data = ((uFlip << 1) & 0x2)| (uMirror & 0x1); //filp/mirror 00, 01 , 11
    orient_reg[3].u16Data = 0x22;
    if (uMirror == 0 && uFlip == 0)
    {
        orient_reg[3].u16Data = 0x32;
    }
    /*copy result*/
    for(n=0;n<sizeof(orient_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)orient_reg)[n];
    }
    pHandle->tInfo.u8Mirror = uMirror;
    pHandle->tInfo.u8Flip = uFlip;
    return sizeof(orient_reg)/sizeof(orient_reg[0]);
}

static unsigned int OS04D10EarlyInitSelStreamOnoff( EarlyInitSensorDrvCfg_t *pHandle, unsigned char uStreamOnoff, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
    unsigned char n;
    I2cCfg_t stream_onoff_reg[] =
    {
        {0xfd,0x00},
        {0x36,0x07},
        {0x20,0x01} //enter sleep
    };

    pHandle->tInfo.u8StreamOnoff = uStreamOnoff;
    if(uStreamOnoff == 1)
    {
        return 0;
    }
    /*copy result*/
    //for(n=0;n<sizeof(stream_onoff_reg)/sizeof(stream_onoff_reg[0]);++n)
    for(n=0;n<sizeof(stream_onoff_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)stream_onoff_reg)[n];
    }
    pHandle->tInfo.u8StreamOnoff = uStreamOnoff;
    return sizeof(stream_onoff_reg)/sizeof(stream_onoff_reg[0]);
}

static unsigned int OS04D10EarlyInitFastAe(EarlyInitSensorDrvCfg_t *pHandle, unsigned long *pShutter, unsigned long *pGain)
{
    _u16 I2cId = gHwRes[pHandle->uSensorPadId]->eI2cPort ;//HwRes->eI2cPort;
    _u16 slave_addr = 0x78;
    EARLYINIT_I2C_FMT fmt = I2C_FMT_A8D8;
    _u16 timeout_cnt = 100;
    _u32 ret = 0;

    while(1)
    {
        _u16 u16AEC_done = 0;
        _u16 data[10] = {0};

        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x01, fmt);
        earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x20, &u16AEC_done, fmt);

        earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x03, &data[0], fmt);
        earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x04, &data[1], fmt);
        earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x24, &data[2], fmt);

        earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x21, &data[3], fmt);

        earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x02, fmt);
        earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x82, &data[4], fmt);

        CamOsPrintf("I2cId[%d] aec done %d [0x03 0x%x], [0x04, 0x%x],[0x24, 0x%x],[P1 0x21, 0x%x], [P2 0X82 0x%x]\n",I2cId, u16AEC_done,
            data[0], data[1],data[2], data[3], data[4]);

        if(u16AEC_done == 1)
        {
            _u16 u16exp_lsb = 0, u16exp_msb = 0, u16exp =0, u16exp2=0;
            _u16 again1=0, again16 = 0;
            _u32 again2_1024=0;
            _u16 otpdata1024=0;
            _u16 debug_77 = 0, debug_7b=0, debug_21=0;
            _u32 VTS = 1473-16, VTS_temp =0;
            int line_period = 22629;
            _u16 AETarget=0, AvgY=0;

            earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x01, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x22, &u16exp_msb, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x23, &u16exp_lsb, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x24, &again16, fmt);
            otpdata1024 = OS04D10_GetOtpData(pHandle);
            earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x02, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x82, &AETarget, fmt);

            earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x01, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x21, &AvgY, fmt);

            u16exp = (u16exp_msb&0xff)<<8|(u16exp_lsb&0xff);
            again16 = (again16 &0xff);
            again1 = again16/16;

            VTS_temp = (u16exp*again16*otpdata1024+(16*1024/2))/(16*1024);//include float param

            CamOsPrintf(">>>>>>exp %d, again16 %d, agin1 %d vts %d otpdata1024 %d, VTS_temp %d\n", u16exp, again16,again1,VTS, otpdata1024, VTS_temp);

            if(VTS_temp <= VTS)
            {
                u16exp2 = VTS_temp;
                again2_1024 = 1024;
            }
            else
            {
                u16exp2 = VTS;
                if(again16 == 0xff && (AvgY < (AETarget-1)) && AvgY != 0)
                {
                    _u64 temp = u16exp*again16*otpdata1024;
                    again2_1024 = (temp*AETarget+VTS*8*AvgY)/(VTS*16*AvgY);
                }
                else
                {
                    again2_1024 = (u16exp*again16*otpdata1024+VTS*8)/(VTS*16);
                }

            }

            CamOsPrintf(">>>>>>>>exp2 %d, again2_1024 %d \n", u16exp2,again2_1024);

            earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x02, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x77, &debug_77, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x7b, &debug_7b, fmt);

            earlyinit_i2c_write_register_pair(I2cId, slave_addr,0xfd, 0x01, fmt);
            earlyinit_i2c_read_register_pair(I2cId, slave_addr, 0x21, &debug_21, fmt);
            CamOsPrintf(">>>>>>>>debug_77 0x%x, debug_7b 0x%x, debug_21 0x%x\n", debug_77,debug_7b, debug_21);


            *pShutter    = (u16exp2*line_period)/1000;
            *pGain = again2_1024;

            CamOsPrintf(">>>>>>>>init shutter %d gain %d\n",*pShutter, *pGain);
            break;
        }
        else
        {
            CamOsUsDelay(1000);
            timeout_cnt -=1;
            if(timeout_cnt == 0)
            {
                CamOsPrintf(">>>>>>>>fast ae wait aec done timeout \n");
                ret = -1;
                break;
            }
        }
    }

    return ret;
}

static unsigned int os04d10InitHandle(EARLY_INIT_SN_PAD_e eSnrId, EarlyInitEntry_t *pHandle)
{
    //interface ver2
    pHandle->fpGainParserEx       = OS04D10EarlyInitGainEx;
    pHandle->fpShutterFpsParserEx = OS04D10EarlyInitShutterAndFps;
    pHandle->fpSelResId           = OS04D10EarlyInitSelResId;
    pHandle->fpOrientParerEx      = OS04D10EarlyInitSelOrientEx;
    pHandle->fpStreamOnoffParserEx= OS04D10EarlyInitSelStreamOnoff;
    pHandle->fpFastAeParseEx      = OS04D10EarlyInitFastAe;
    gHwRes[eSnrId]               = &pHandle->tHwRes;
    //default setting is preset id 0
    pHandle->tCurCfg.pData        = 0; //private data
    pHandle->tCurCfg.uPresetId    = OS04D10_PRESET_4M30;
    pHandle->tCurCfg.pTable       = (void*)Sensor_init_table_4M30;
    pHandle->tCurCfg.uTableSize   = sizeof(Sensor_init_table_4M30);
    _memcpy(&pHandle->tCurCfg.tInfo, &_gOS04D10Info_4M30, sizeof(_gOS04D10Info_4M30));

    //default setting for interface v1
    pHandle->pInitTable           = pHandle->tCurCfg.pTable;
    pHandle->nInitTableSize       = pHandle->tCurCfg.uTableSize;

    return 0;
}
SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2(OS04D10, os04d10InitHandle)
