/*
 * drv_sensor_if.h - Sigmastar
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

#ifndef __DRV_SENSOR_IF__
#define __DRV_SENSOR_IF__
#include "hal_sensor.h"

void DrvSensorRegInit(void);
s32  DrvSensorSetVifChnBaseAddr(void);
s32 _DRV_SENSOR_IF_SetVifSetIoPad(u32 nSNRPadID, u32 eBusType, u8 SnrPadSel, u8 MclkPadSel, u8 RstPadSel, u8 PwnPadSel);
s32 _DRV_SENSOR_IF_SetSensorReset(u32 nSnrPadId, SensorPolarity_e ePOL);
s32 _DRV_SENSOR_IF_SetSensorPdwn(u32 nSNRPadID, SensorPolarity_e ePOL);
s32 _DRV_SENSOR_IF_SetSensorMCLK(u32 nSNRPadID, SnrMclk_e nMclkIdx);

#endif
