/*
 * drv_vif.c - Sigmastar
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
#include "vif_datatype.h"
#include "hal_vif.h"
#include "vif_log.h"
#include "drv_vif.h"

u8 g_u8Inited = 0;

void DrvVifInit(void)
{
    if (!g_u8Inited)
    {
        VifHandleId_e eHandleId;
        for (eHandleId = E_VIF_HANDLE_VIF_C0; eHandleId < E_VIF_HANDLE_ID_MAX; eHandleId++)
        {
            if (HalVifGetHandleBasePa(eHandleId))
            {
                HalVifSetHandle(eHandleId, HalVifGetHandleBasePa(eHandleId));
            }
        }

        DrvVifSetChnBaseAddr();
        g_u8Inited = 1;
    }
}

s32 DrvVifSetChnBaseAddr(void)
{
    s32 ret = VIF_SUCCESS;
    u32 i   = 0;

    for (i = 0; i < VIF_MAX_CHANNEL; i++)
    {
        ret = HalVifSetChnBaseAddr(i);
        if (ret != VIF_SUCCESS)
        {
            break;
        }
    }

    return ret;
}

s32 DrvVifSensorReset(u32 nSNRPadID, u32 eBusType, u32 ePOL)
{
    u32 nChn;

    if (HalVifGetChnBySnrPadId(nSNRPadID, &nChn))
    {
        return VIF_FAIL;
    }

    HalVifSensorReset(nChn, eBusType, (ePOL == E_VIF_CLK_POL_POS) ? 1 : 0);
    return VIF_SUCCESS;
}

s32 DrvVifSensorPdwn(u32 nSNRPadID, u32 eBusType, u32 ePOL)
{
    u32 nChn;

    if (HalVifGetChnBySnrPadId(nSNRPadID, &nChn))
    {
        return VIF_FAIL;
    }

    HalVifSensorPowerDown(nChn, eBusType, (ePOL == E_VIF_CLK_POL_POS) ? 1 : 0);
    return VIF_SUCCESS;
}

s32 DrvVifSetMclk(u32 nSNRPadID, u32 eBusType, u8 nOnOff, u32 nMclk)
{
#ifdef CONFIG_CAM_CLK
    static void *        pMclkHandle[VIF_MAX_SENSOR_PAD] = {0};
    s32                  ret;
    u8                   nGating = 0;
    u32                  nFnlMclk;
    CAMCLK_Set_Attribute stSetCfg;
    extern const u32     g_au32VifCamClkId[VIF_MAX_SENSOR_PAD];

    if (nSNRPadID >= VIF_MAX_SENSOR_PAD)
    {
        return VIF_FAIL;
    }

    if (pMclkHandle[nSNRPadID] == NULL)
    {
        CamClkRegister((u8 *)"sstar,vif", g_au32VifCamClkId[nSNRPadID], &pMclkHandle[nSNRPadID]);
    }

    ret = HalVifConvertMclk(nMclk, &nFnlMclk);

    if (ret)
    {
        /* unknown mclk */
        nGating = 1;
    }

    if (nGating || !nOnOff)
    {
        CamClkSetOnOff(pMclkHandle[nSNRPadID], 0);
        CamClkUnregister(pMclkHandle[nSNRPadID]);
        pMclkHandle[nSNRPadID] = NULL;
    }
    else
    {
        CAMCLK_SETRATE_ROUND(stSetCfg, nFnlMclk);
        CamClkAttrSet(pMclkHandle[nSNRPadID], &stSetCfg);
        CamClkSetOnOff(pMclkHandle[nSNRPadID], 1);
    }
    return VIF_SUCCESS;

#else

    if (!nOnOff)
    {
        HalVifSetMclk(nSNRPadID, eBusType, 1, 0);
        return VIF_SUCCESS;
    }

    HalVifSetMclk(nSNRPadID, eBusType, 0, nMclk);

    return VIF_SUCCESS;

#endif
}

s32 DrvVifSetIoPad(u32 nSNRPadID, u32 eBusType, u32 nPara)
{
    if (nSNRPadID >= VIF_MAX_SENSOR_PAD)
    {
        VIFLogDbg("[%s] VIF group number %d not support\n", __FUNCTION__, nSNRPadID);
        return VIF_NOT_SUPPORT;
    }

    DrvVifSetAllPadIn(nSNRPadID, 0);

    return HalVifSetIoPad(nSNRPadID, eBusType, nPara);
}
