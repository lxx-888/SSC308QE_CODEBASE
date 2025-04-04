/*
 * drv_pnl_os.c- Sigmastar
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

#define _DRV_PNL_OS_C_

#include "drv_pnl_os.h"

void *DrvPnlOsMemAlloc(u32 u32Size)
{
    return malloc(u32Size);
}

void DrvPnlOsMemRelease(void *pPtr)
{
    free(pPtr);
}

void DrvPnlOsMsSleep(u32 u32Msec) {}

void DrvPnlOsUsSleep(u32 u32Usec) {}

bool DrvPnlOsPadMuxActive(void)
{
    return 0;
}

int DrvPnlOsPadMuxGetMode(void)
{
    return 0xFF;
}

int DrvPnlOsGetMode(u16 u16LinkType, u16 u16OutputFormatBitMode)
{
    int ret     = 0;
    u32 u32Mode = 0;

    if (u16LinkType == DRV_PNL_OS_LINK_TTL)
    {
        u32Mode = 1;
    }

    return (ret == 0) ? u32Mode : 0;
}

bool DrvPnlOsSetDeviceNode(void *pPlatFormDev)
{
    return 0;
}

bool DrvPnlOsSetClkOn(void *pClkEn, void *pClkRate, u32 u32ClkRateSize)
{
    return 0;
}

bool DrvPnlOsSetClkOff(void *pClkEn, void *pClkRate, u32 u32ClkRateSize)
{
    return 0;
}
