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

#ifndef _DRV_SENSOR_EARLYINIY_COMMON_H_
#define _DRV_SENSOR_EARLYINIY_COMMON_H_

#include <drv_sensor_earlyinit_method_1.h>
#include <drv_sensor_earlyinit_datatype.h>

typedef unsigned int (*ShutterFpsParser_fp)( unsigned int, unsigned int, I2cCfg_t *, unsigned int);
typedef unsigned int (*GainParser_fp)( unsigned int, I2cCfg_t *, unsigned int);
typedef unsigned int (*GetSnrInfo_fp)( EarlyInitSensorInfo_t* pSnrInfo);
//Ver2
typedef unsigned int (*SetResId_fp)(EarlyInitSensorDrvCfg_t *pHandle, unsigned char uHdrMode, unsigned int uResId);
typedef unsigned int (*ShutterFpsParserEx_fp)(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uFpsX1000, unsigned int uShtterLef, unsigned int uShtterSef, unsigned int uShtterMef, I2cCfg_t *, unsigned int nMaxNumRegs);
typedef unsigned int (*GainParserEx_fp)(EarlyInitSensorDrvCfg_t *pHandle, unsigned int uGainLef, unsigned int uGainSef, unsigned int uGainMef,I2cCfg_t *, unsigned int nMaxNumRegs);
typedef unsigned int (*OrientationParserEx_fp)(EarlyInitSensorDrvCfg_t *pHandle, unsigned char nMirror, unsigned char nFlip, I2cCfg_t *, unsigned int nMaxNumRegs);
typedef unsigned int (*StreamOnOffParserEx_fp)(EarlyInitSensorDrvCfg_t *pHandle, unsigned char nStreamOnff, I2cCfg_t *, unsigned int nMaxNumRegs);
typedef unsigned int (*FastAeEx_fp)( EarlyInitSensorDrvCfg_t *pHandle, unsigned long *pShutter, unsigned long *pGain);


typedef enum
{
    E_EARLYINIT_SENSORIF_V1 = 1,
    E_EARLYINIT_SENSORIF_V2 = 2
}EarlyInitSensorIfVer_e;

typedef struct
{
    void *pInitTable;            //sensor EarlyInit data
    unsigned int nInitTableSize; //sensor EarlyInit data size
    ShutterFpsParser_fp fpShutterFpsParser;
    GainParser_fp fpGainParser;
    GetSnrInfo_fp fpGetSnrInfo;
    EarlyInitHwRes_t tHwRes; //Hardware resource
    //earlyinit sensor interface ver2
    SetResId_fp fpSelResId;
    ShutterFpsParserEx_fp fpShutterFpsParserEx; //shutter and fps control for linear and HDR
    GainParserEx_fp fpGainParserEx;             //gain control for linear and HDR
    OrientationParserEx_fp fpOrientParerEx;     //mirror-flip control
    StreamOnOffParserEx_fp fpStreamOnoffParserEx; // streamonoff control
    FastAeEx_fp fpFastAeParseEx; //sensor Fast AE control
    EarlyInitSensorDrvCfg_t tCurCfg;            //sensor driver handle
    EarlyInitSensorIfVer_e eIfVer;              //support interface version
}EarlyInitEntry_t;

typedef unsigned int (*InitHandle_fp)( EARLY_INIT_SN_PAD_e eSrPad, EarlyInitEntry_t *pHandle);

#define SENSOR_EARLYINIY_ENTRY_IMPL_BEGIN(Name)

#define SENSOR_EARLYINIY_ENTRY_IMPL_END(Name, EarlyInitTable, ShutterFpsParser, GainParser, GetSnrInfo) \
void Name##_EarlyInitReg(unsigned int nChMap)\
{\
    int n;\
    for(n=0;n<4;++n)\
    {\
        if( nChMap&(0x1<<n) )\
        {\
            DrvEarlyInitSetInitParam( n,\
                                      &EarlyInitTable[0],\
                                      sizeof(EarlyInitTable),\
                                      ShutterFpsParser,\
                                      GainParser,\
                                      GetSnrInfo\
            );\
        }\
    }\
}

#define SENSOR_EARLYINIY_ENTRY_IMPL_END_VER2(Name, InitHandle) \
void Name##_EarlyInitReg(unsigned int nChMap)\
{\
    int n;\
    for(n=0;n<4;++n)\
    {\
        if( nChMap&(0x1<<n) )\
        {\
            DrvEarlyInitSetInitParamVer2( n, InitHandle);\
        }\
    }\
}
void DrvEarlyInitSetInitParam(EARLY_INIT_SN_PAD_e eSensorPad,
                              void* pTable, unsigned int nTableSize,
                              ShutterFpsParser_fp fpShutterFpsParser,
                              GainParser_fp fpGainParser,
                              GetSnrInfo_fp fpGetSnrInfo
                              );

void DrvEarlyInitSetInitParamVer2(EARLY_INIT_SN_PAD_e eSensorPad, InitHandle_fp fpInitHandle);

#define SENSOR_EARLYINIY_REG_DRV(Name, Config) \
extern void Name##_EarlyInitReg(unsigned int);\
if(Config == 0)\
    Name##_EarlyInitReg(_SENSOR0_CHMAP_);\
else if(Config == 1)\
    Name##_EarlyInitReg(_SENSOR1_CHMAP_);
#endif //end of _DRV_SENSOR_EARLYINIY_COMMON_H_
