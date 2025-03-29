/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
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
#include "OS02N10_init_table.h"         /* Sensor initial table */
#include <cam_os_wrapper.h>
#define ENABLE_NR 0
/* Sensor EarlyInit implementation */
SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(OS02N10)
#define abs(a)   ((a)>(0) ? (a) : (-(a)))
typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

static EarlyInitSensorInfo_t _gOS02N10Info =
{
    .eBayerID       = E_EARLYINIT_SNR_BAYER_BG,
    .ePixelDepth    = EARLYINIT_DATAPRECISION_10,
    .u32FpsX1000    = 15000,
    .u32Width       = 1920,
    .u32Height      = 1080,
    .u32GainX1024   = 1024,
    .u32ShutterUs   = 8000,
    .u32ShutterShortUs = 0,
    .u32GainShortX1024 = 0,
    .u8ResId           = 0,
    .u8HdrMode         = 0,
};

typedef struct {
    unsigned int gain;
    unsigned short fine_gain_reg;
} FINE_GAIN;



/** @brief Convert shutter to sensor register setting
@param[in] nShutterUs target shutter in us
@param[in] nFps x  target shutter in us
@param[out] pRegs I2C data buffer
@param[in] nMaxNumRegs pRegs buffer length
@retval Return the number of I2C data in pRegs
*/
static unsigned int OS02N10EarlyInitShutterAndFps( unsigned int nFpsX1000, unsigned int nShutterUs, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{
#define Preview_line_period 30222                           // hts
#define vts_30fps  1109//1090                              //for 30fps
#define Preview_MAX_FPS     30
#define Preview_MIN_FPS     3
    unsigned char n;
    unsigned int lines = 0;
    unsigned int vts = 0;
	unsigned int vb = 0;
    int nNumRegs = 0;
    I2cCfg_t expo_reg[] =
    {
        /*exposure*/
        {0x0e, 0x00},//expo [20:17]
        {0x0f, 0x00}, // expo[16:8]
        /*vts*/
        {0x14, 0x05},
        {0x15, 0x46},
    };
    //CamOsPrintf("[%s] num shutter:%u fps:%u\n", __func__, nShutterUs, nFpsX1000);

    if(nFpsX1000<1000){
        _gOS02N10Info.u32FpsX1000 = nFpsX1000*1000;
        nFpsX1000 = nFpsX1000*1000;
    }
    _gOS02N10Info.u32ShutterUs = nShutterUs;

    /*VTS*/
    //-----------------from pCus_SetFPS s-----------------------
    if(15 == nFpsX1000 || 15000 == nFpsX1000){
        vts = ((vts_30fps*Preview_MAX_FPS * 10000)/142857);
    }else if(nFpsX1000>=Preview_MIN_FPS && nFpsX1000 <= Preview_MAX_FPS){
        vts =  (vts_30fps*Preview_MAX_FPS)/nFpsX1000;
    }else if((nFpsX1000 >= (Preview_MIN_FPS*1000)) && (nFpsX1000 <= (Preview_MAX_FPS*1000))){
        vts =  (vts_30fps*(Preview_MAX_FPS*1000))/nFpsX1000;
    }else{
       // CamOsPrintf("[%s]ERR nFpsX1000 %d out of range.\n",__FUNCTION__,nFpsX1000);
    }
    //-----------------from pCus_SetFPS e-----------------------

    //SHUTTER
    //-----------------from pCus_SetAEUSecs s-----------------------
	lines=((1000*nShutterUs+(Preview_line_period>>1))/Preview_line_period);
    if(lines<=2) lines=2;
    if (lines > (vts-9)) {
        vts = (lines + 9);
    }
    else{
        vts = vts;
    }
	vb=vts-vts_30fps;

    expo_reg[0].u16Data = (lines>>8) & 0x00ff;//0x00;
    expo_reg[1].u16Data = (lines>>0) & 0x00ff; //0x00;
    expo_reg[2].u16Data = (vb >> 8) & 0x00ff;// 0x05;//
    expo_reg[3].u16Data = (vb >> 0) & 0x00ff;//0x46;//
    //-----------------from pCus_SetAEUSecs e-----------------------
    /*copy result*/
    for(n=0;n<sizeof(expo_reg);++n)
    {
        ((unsigned char*)pRegs)[n] = ((unsigned char*)expo_reg)[n];
    }

    //CamOsPrintf("[%s] expo:0x%06x vts:0x%04x\n", __func__, ((pRegs+2)->u16Data) | ((pRegs+1)->u16Data << 8) | (pRegs->u16Data << 16), ((pRegs+4)->u16Data) | ((pRegs+3)->u16Data << 8));

    nNumRegs = sizeof(expo_reg)/sizeof(expo_reg[0]);

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
extern _u64 EXT_log_2(_u32 value);

static unsigned int OS02N10EarlyInitGain( unsigned int u32GainX1024, I2cCfg_t *pRegs, unsigned int nMaxNumRegs)
{

#define SENSOR_MAXGAIN      496
#define MAX_A_GAIN          (15872)      //15.5 * 1024

    int nNumRegs = 0;
    unsigned char i;

    I2cCfg_t gain_reg[] = {
        {0x24, 0xf8},
        {0x1f, 0x00},
        {0x20, 0x40},
    };
	u32 dgain = 0, again;
    unsigned int gain = u32GainX1024;
	if(gain<1024)
        gain=1024;
    else if(gain>=SENSOR_MAXGAIN*1024)
        gain=SENSOR_MAXGAIN*1024;


    /* A Gain */
    if (gain <= 1024) {
        again=1024;
    } else if (gain < 2048) {
        again = (gain>>6)<<6;
    } else if (gain < 4096) {
        again = (gain>>7)<<7;
    } else if (gain < 8192) {
        again = (gain>>8)<<8;
    } else if (gain < MAX_A_GAIN) {
        again = (gain>>9)<<9;
    } else {
        again = MAX_A_GAIN;
    }
    dgain = gain*64/again;
    again = again>>6;
    gain_reg[0].u16Data = again*0xff; // 0x3e09
    gain_reg[1].u16Data = (dgain>>8)&0x07;; // 0x3e08
    gain_reg[2].u16Data = dgain&0xff; // 0x3e07
    for (i = 0; i < sizeof(gain_reg); i++)
    {
        ((unsigned char*)pRegs)[i] = ((unsigned char*)gain_reg)[i];
    }
    //CamOsPrintf("[%s] gain:%06x\n", __func__, ((pRegs+2)->u16Data) | ((pRegs+1)->u16Data << 8) | (pRegs->u16Data << 16));

    nNumRegs = sizeof(gain_reg)/sizeof(gain_reg[0]);
    return nNumRegs;
}



static unsigned int OS02N10EarlyInitGetSensorInfo(EarlyInitSensorInfo_t* pSnrInfo)
{
    if(pSnrInfo)
    {
        pSnrInfo->eBayerID      = E_EARLYINIT_SNR_BAYER_BG;
        pSnrInfo->ePixelDepth   = EARLYINIT_DATAPRECISION_10;
        pSnrInfo->eIfBusType    = EARLYINIT_BUS_TYPE_MIPI;
        pSnrInfo->u32FpsX1000   = _gOS02N10Info.u32FpsX1000;
        pSnrInfo->u32Width      = _gOS02N10Info.u32Width;
        pSnrInfo->u32Height     = _gOS02N10Info.u32Height;
        pSnrInfo->u32GainX1024  = _gOS02N10Info.u32GainX1024;
        pSnrInfo->u32ShutterUs  = _gOS02N10Info.u32ShutterUs;
        pSnrInfo->u32ShutterShortUs = _gOS02N10Info.u32ShutterShortUs;
        pSnrInfo->u32GainShortX1024 = _gOS02N10Info.u32GainShortX1024;
        pSnrInfo->u8ResId       = _gOS02N10Info.u8ResId;
        pSnrInfo->u8HdrMode     = _gOS02N10Info.u8HdrMode;
        pSnrInfo->u32TimeoutMs  = 200;
    }
    return 0;
}

/* Sensor EarlyInit implementation end*/
SENSOR_EARLYINIY_ENTRY_IMPL_END( OS02N10,
                                 Sensor_init_table,
                                 OS02N10EarlyInitShutterAndFps,
                                 OS02N10EarlyInitGain,
                                 OS02N10EarlyInitGetSensorInfo
                                );
