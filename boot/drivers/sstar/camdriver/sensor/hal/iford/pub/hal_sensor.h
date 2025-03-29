/*
 * hal_sensor.h - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifndef __HAL_SENSOR__
#define __HAL_SENSOR__

#include "hal_sensor_reg.h"
#include "sensor_datatype.h"

typedef enum
{
    E_MCLK_27M        = 0x00,
    E_MCLK_72M        = 0x01,
    E_MCLK_61P7M      = 0x02,
    E_MCLK_54M        = 0x03,
    E_MCLK_48M        = 0x04,
    E_MCLK_43P2M      = 0x05,
    E_MCLK_36M        = 0x06,
    E_MCLK_24M        = 0x07,
    E_MCLK_21P6M      = 0x08,
    E_MCLK_12M        = 0x09,
    E_MCLK_5P4M       = 0x0A,
    E_MCLK_LPLL_DIV2  = 0x0B,
    E_MCLK_LPLL_DIV4  = 0x0C,
    E_MCLK_LPLL_DIV8  = 0x0D,
    E_MCLK_LPLL_DIV16 = 0x0E,
    E_MCK_MAX,
} SnrMclk_e;

void      HalSensorSetHandle(SensorHandleId_e nHandleId, uintptr_t nHandle);
uintptr_t HalSensorGetHandle(SensorHandleId_e nHandleId);
u32       HalSensorGetHandleBasePa(SensorHandleId_e nHandleId);
s32       HalSensorSetChnBaseAddr(VifChannel_e ch);
void      HalSensorSetAllPadIn(u32 nMode);
s32       HalSensorGetVifChnBySnrPadId(u32 nSNRPadID, u32 *nPhyChn);
void      HalSensorReset(VifChannel_e nPhyChn, SensorPolarity_e OnOff);
void      HalSensorPowerDown(VifChannel_e nPhyChn, SensorPolarity_e OnOff);
void      HalSensorSetVifMclk(u32 nSNRPadID, SnrMclk_e nMclk);
s32       HalSensorSetVifIoPad(u32 nSNRPadID, u32 eBusType, u8 SnrPadSel, u8 MclkPadSel, u8 RstPadSel, u8 PwnPadSel);

#define DrvSensorSetAllPadIn(b) HalVifSetAllPadIn(b)

#endif
