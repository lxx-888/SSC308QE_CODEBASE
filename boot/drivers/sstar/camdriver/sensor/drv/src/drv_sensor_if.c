/*
 * drv_sensor_if.c - Sigmastar
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

#include "cam_os_wrapper.h"
#include <linux/types.h>
#include <common.h>
#include "sensor_datatype.h"
#include "sensor_log.h"
#include "drv_sensor_if.h"

static u8 g_u8Inited = 0;

void DrvSensorRegInit(void)
{
    if (!g_u8Inited)
    {
        SensorHandleId_e eHandleId;
        for (eHandleId = E_SENSOR_HANDLE_VIF_C0; eHandleId < E_SENSOR_HANDLE_ID_MAX; eHandleId++)
        {
            if (HalSensorGetHandleBasePa(eHandleId))
            {
                HalSensorSetHandle(eHandleId, HalSensorGetHandleBasePa(eHandleId));
            }
        }

        DrvSensorSetVifChnBaseAddr();
        // DrvSensorSetAllPadIn(1);
        g_u8Inited = 1;
    }
}

s32 DrvSensorSetVifChnBaseAddr(void)
{
    s32 ret = SENSOR_SUCCESS;
    u32 i   = 0;

    for (i = 0; i < VIF_MAX_CHANNEL; i++)
    {
        ret = HalSensorSetChnBaseAddr(i);
        if (ret != SENSOR_SUCCESS)
        {
            break;
        }
    }

    return ret;
}

s32 _DRV_SENSOR_IF_SetSensorReset(u32 nSNRPadID, SensorPolarity_e ePOL)
{
    u32 nChn;

    if (HalSensorGetVifChnBySnrPadId(nSNRPadID, &nChn))
    {
        return SENSOR_FAIL;
    }

    HalVifSensorReset(nChn, ePOL);
    return SENSOR_SUCCESS;
}

s32 _DRV_SENSOR_IF_SetSensorPdwn(u32 nSNRPadID, SensorPolarity_e ePOL)
{
    u32 nChn;

    if (HalSensorGetVifChnBySnrPadId(nSNRPadID, &nChn))
    {
        return SENSOR_FAIL;
    }

    HalSensorPowerDown(nChn, ePOL);
    return SENSOR_SUCCESS;
}

s32 _DRV_SENSOR_IF_SetSensorMCLK(u32 nSNRPadID, SnrMclk_e nMclkIdx)
{
    HalSensorSetVifMclk(nSNRPadID, nMclkIdx);

    return SENSOR_SUCCESS;
}

s32 _DRV_SENSOR_IF_SetVifSetIoPad(u32 nSNRPadID, u32 eBusType, u8 SnrPadSel, u8 MclkPadSel, u8 RstPadSel, u8 PwnPadSel)
{
    if (nSNRPadID >= VIF_MAX_SENSOR_PAD)
    {
        SENSORIFLogDbg("[%s] VIF group number %d not support\n", __FUNCTION__, nSNRPadID);
        return SENSOR_NOT_SUPPORT;
    }

    return HalSensorSetVifIoPad(nSNRPadID, eBusType, SnrPadSel, MclkPadSel, RstPadSel, PwnPadSel);
}
