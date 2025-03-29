/*
 * drv_vif.h - Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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

#ifndef __DRV_VIF__
#define __DRV_VIF__

void DrvVifInit(void);
s32  DrvVifSetChnBaseAddr(void);
s32  DrvVifSetIoPad(u32 nSnrPadId, u32 eBusType, u32 nPara);
s32  DrvVifSensorReset(u32 nSnrPadId, u32 eBusType, u32 ePOL);
s32  DrvVifSensorPdwn(u32 nSnrPadId, u32 eBusType, u32 ePOL);
s32  DrvVifSetMclk(u32 nSnrPadId, u32 eBusType, u8 nOnOff, u32 nMclk);

#endif
