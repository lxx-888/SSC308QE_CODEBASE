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
#include <cam_drv_i2c.h>
#include "cam_os_wrapper.h"
#include "light_sensor.h"

#define CHECK_LIGHT_RESULT(result)\
    if (result != 0)\
    {\
        CamOsPrintf("[%s %d]i2c get light failed\n", __FUNCTION__, __LINE__);\
        return -1;\
    }

static int I2C_HACKING_I2CWriteRegisterPair(u16 nr, u16 slaveAddr, u16 reg, u16 value, ISP_I2C_FMT fmt)
{
    tI2cHandle tHandle;
    u8         data[4] = {0};

    CamI2cOpen(&tHandle, nr);

    switch (fmt)
    {
        case I2C_FMT_A8D8:
        {
            tI2cMsg msg;

            data[0] = reg & 0xff;
            data[1] = value & 0xff;

            msg.addr  = slaveAddr >> 1;
            msg.flags = 0;
            msg.buf   = data;
            msg.len   = 2;

            if (CamI2cTransfer(&tHandle, &msg, 1) < 0)
            {
                CamOsPrintf("[%s]%d i2c-%d timeout\n", __FUNCTION__, __LINE__, nr);
                CamI2cClose(&tHandle);
                return -1;
            }
        }
        break;
        default:
            break;
    }
    CamI2cClose(&tHandle);
    return 0;
}

static int I2C_HACKING_I2CReadRegisterPair(u16 nr, u16 slaveAddr, u16 reg, u16 *val, ISP_I2C_FMT fmt)
{
    tI2cHandle tHandle;
    u8         data[4] = {0};

    CamI2cOpen(&tHandle, nr);

    switch (fmt)
    {
        case I2C_FMT_A8D8:
        {
            tI2cMsg msg[2];

            data[0] = reg & 0xff;

            msg[0].addr  = slaveAddr >> 1;
            msg[0].flags = 0;
            msg[0].buf   = data;
            msg[0].len   = 1;

            msg[1].addr  = slaveAddr >> 1;
            msg[1].flags = 1;
            msg[1].buf   = data;
            msg[1].len   = 1;

            if (CamI2cTransfer(&tHandle, msg, 2) < 0)
            {
                CamOsPrintf("[%s]%d i2c-%d timeout\n", __FUNCTION__, __LINE__, nr);
                CamI2cClose(&tHandle);
                return -1;
            }
            *val = data[0];
        }
        break;
        default:
            break;
    }

    CamI2cClose(&tHandle);

    return 0;
}

#define LightSensorReg_Read(_reg,_data)     (I2C_HACKING_I2CReadRegisterPair(LIGHT_SENSOR_I2C_GROUP, LIGHT_SENSOR_I2C_SLAVE_ADDR, _reg, _data, I2C_FMT_A8D8))
#define LightSensorReg_Write(_reg,_data)    (I2C_HACKING_I2CWriteRegisterPair(LIGHT_SENSOR_I2C_GROUP, LIGHT_SENSOR_I2C_SLAVE_ADDR, _reg, _data, I2C_FMT_A8D8))
#define DIFF(a, b)                          (((a) > (b)) ? (a-b) : (b-a))

const static LightTable_t g_stLightTable[2][10] = {
    {
        // 100 TIG
        //{   1, 33311, 6623,1024}, // 1LUX
        {1, 30000, 262144, 1024}, // 1LUX
        {3, 30000, 8331, 1024},   // 3LUX
        {7, 30000, 4128, 1024},   // 7LUX
        {15, 10000, 5875, 1024},  // 15LUX
                                  //{  30, 12889, 1025,1024}, // 30LUX
        {30, 10000, 3040, 1024},  // 30LUX
                                  //{  63,  6400, 1025,1025}, // 63LUX
        {63, 10000, 1508, 1025},  // 63LUX
        {132, 7158, 1025, 1024},  // 132LUX
        {265, 3579, 1025, 1038},  // 265LUX
        {511, 1790, 1025, 1042},  // 511LUX
        {1638, 870, 1025, 1067},  // 1638LUX
    },
    {
        // 400 TIG
        {7, 33311, 6281, 1024},   // 7LUX
        {16, 33311, 3220, 1024},  // 16LUX
        {33, 33311, 1587, 1024},  // 33LUX
        {68, 25536, 1025, 1024},  // 68LUX
        {138, 12867, 1025, 1024}, // 138LUX
        {287, 6423, 1025, 1024},  // 287LUX
        {590, 3112, 1025, 1025},  // 590LUX
        {1182, 1578, 1025, 1025}, // 1182LUX
        {1638, 778, 1025, 1035},  // 1638LUX
        {1638, 778, 1025, 1038},
    }};


int LightSensorGetLux(unsigned long* getLux)
{
    u16 chn0_l = 0, chn0_h = 0, chn1_l = 0, chn1_h = 0, chn0 = 0, chn1 = 0;
    u16 gain = 0, gain_l = 0, gain_h = 0, lux= 0, update = 0;

    CHECK_LIGHT_RESULT(LightSensorReg_Read(0x20, &update));

    CHECK_LIGHT_RESULT(LightSensorReg_Read(0x21, &chn0_l));
    CHECK_LIGHT_RESULT(LightSensorReg_Read(0x22, &chn0_h));
    CHECK_LIGHT_RESULT(LightSensorReg_Read(0x23, &chn1_l));
    CHECK_LIGHT_RESULT(LightSensorReg_Read(0x24, &chn1_h));
    CHECK_LIGHT_RESULT(LightSensorReg_Read(0x05, &gain));
    gain_l = gain & 0xF;
    gain_h = (gain & 0xF0) >> 4;
    chn0 = chn0_h << 8 | chn0_l;
    chn1 = chn1_h << 8 | chn1_l;

    lux = chn0 < 65535 ? (chn0 * 15 * 25 / gain_l - chn1 * 15 * 25 / gain_h) / 1000:
            (chn0 * 15 * 25 / gain_l / 1000);

    *getLux = lux;
    CamOsPrintf("[%s]get lux:%d\n", __FUNCTION__, *getLux);
    return 0;
}

int Preload_LightSensorPowerOnOff(unsigned char bOnOff)
{
    CamOsPrintf("light sensor power onoff:%d\n", bOnOff);
    if (bOnOff == 0)
    {
        CHECK_LIGHT_RESULT(LightSensorReg_Write(0x03, 0x06));
    }
    else
    {
        CHECK_LIGHT_RESULT(LightSensorReg_Write(0x03, 0x04));
    }
    return 0;
}

int Preload_LightSensorGetLux(LightTable_t *pLightParam)
{
    unsigned long nLSlux = 0, i, min=0xFFFF, index=0;

    CHECK_LIGHT_RESULT(LightSensorGetLux(&nLSlux));

    for (i = 0; i < (sizeof(g_stLightTable[0]) / sizeof(g_stLightTable[0][0])); i++)
    {
        if(DIFF(nLSlux, g_stLightTable[0][i].lux) < min)
        {
            min = DIFF(nLSlux, g_stLightTable[0][i].lux);
            index = i;
        }
    }
    pLightParam->u32Shutter    = (unsigned int)g_stLightTable[0][index].u32Shutter;
    pLightParam->u32Sensorgain = (unsigned int)g_stLightTable[0][index].u32Sensorgain;
    pLightParam->u32Ispgain    = (unsigned int)g_stLightTable[0][index].u32Ispgain;
    pLightParam->lux = (unsigned int)g_stLightTable[0][index].lux;

    return 0;
}


