/*
 * drv_ive_sys.c- Sigmastar
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

#include <linux/clk.h>
#include <linux/clk-provider.h>

#include "cam_sysfs.h"
#include "cam_os_wrapper.h"
#include "drv_ive_sys.h"
#include "drv_ive_datatype.h"
#include "drv_ive_clk.h"

extern u32 g_u32BurstOutstanding;

//-------------------------------------------------------------------------------------------------
// File operations
//-------------------------------------------------------------------------------------------------

static ssize_t _DrvSysIveAxiGet(struct device *pstDev, struct device_attribute *pstDevAttr, char *pu8Buf)
{
    ssize_t byteW = 0, ret = 0;

    pstDev     = (void *)pstDev;
    pstDevAttr = (void *)pstDevAttr;

    byteW =
        sprintf(pu8Buf, "r_burst=0x%x, w_burst=0x%x, r_out=0x%x, w_out=0x%x\n",
                IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 0), IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 8),
                IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 16), IVE_GET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 24));
    ret += byteW;
    pu8Buf += byteW;

    return ret;
}

static ssize_t _DrvSysIveAxiSet(struct device *pstDev, struct device_attribute *pstDevAttr, const char *pu8Buf,
                                size_t count)
{
    u8 u8ScanRet = 0;
    u8 u8RBurst = 0, u8WBurst = 0, u8ROut = 0, u8WOut = 0;

    pstDev     = (void *)pstDev;
    pstDevAttr = (void *)pstDevAttr;

    // example: echo r_burst w_burst r_out w_out > ive_axi
    if ((u8ScanRet = sscanf(pu8Buf, "%hhd %hhd %hhd %hhd", &u8RBurst, &u8WBurst, &u8ROut, &u8WOut)) >= 4)
    {
        g_u32BurstOutstanding = IVE_SET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 0, u8RBurst);
        g_u32BurstOutstanding = IVE_SET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 8, u8WBurst);
        g_u32BurstOutstanding = IVE_SET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 16, u8ROut);
        g_u32BurstOutstanding = IVE_SET_8BIT_BY_OFFSET(g_u32BurstOutstanding, 24, u8WOut);
    }
    else
    {
        IVE_MSG(IVE_MSG_ERR, "usage: echo r_burst w_burst r_out w_out > ive_axi.\n");
    }

    return count;
}

static ssize_t _DrvSysIveClkGet(struct device *pstDev, struct device_attribute *pstDevAttr, char *pu8Buf)
{
    ssize_t byteW = 0, ret = 0;

    ive_dev_data *pstDevData = dev_get_drvdata(pstDev);

    pstDevAttr = (void *)pstDevAttr;

    if (pstDevData->pstClk != NULL)
    {
        int i           = 0;
        u8  u8ParentNum = 0;
        if (0 < (u8ParentNum = clk_hw_get_num_parents(__clk_get_hw(pstDevData->pstClk))))
        {
            struct clk *pstClkParent   = NULL;
            u32         u32TempClkRate = 0;

            byteW = sprintf(pu8Buf, "\n---------- IVE clock -----------\n");
            ret += byteW;
            pu8Buf += byteW;
            for (i = 0; i < u8ParentNum; ++i)
            {
                pstClkParent   = clk_hw_get_parent_by_index(__clk_get_hw(pstDevData->pstClk), i)->clk;
                u32TempClkRate = clk_get_rate(pstClkParent);

                if (pstDevData->u32ClkRate == u32TempClkRate)
                {
                    if (u32TempClkRate > IVE_MAX_NON_OD_CLK_FREQ)
                    {
                        byteW = sprintf(pu8Buf, "%u od <--\n", pstDevData->u32ClkRate);
                    }
                    else
                    {
                        byteW = sprintf(pu8Buf, "%u <--\n", pstDevData->u32ClkRate);
                    }
                }
                else
                {
                    if (u32TempClkRate > IVE_MAX_NON_OD_CLK_FREQ)
                    {
                        byteW = sprintf(pu8Buf, "%u od\n", u32TempClkRate);
                    }
                    else
                    {
                        byteW = sprintf(pu8Buf, "%u\n", u32TempClkRate);
                    }
                }
                ret += byteW;
                pu8Buf += byteW;
            }
        }
        else
        {
            IVE_MSG(IVE_MSG_ERR, "err clk number(%d).\n", u8ParentNum);
        }
    }
    else
    {
        IVE_MSG(IVE_MSG_ERR, "empty clk in device data\n");
    }

    return ret;
}

static ssize_t _DrvSysIveClkSet(struct device *pstDev, struct device_attribute *pstDevAttr, const char *pu8Buf,
                                size_t count)
{
    u8            u8ScanRet     = 0;
    u32           u32SetClkRate = 0;
    ive_dev_data *pstDevData    = dev_get_drvdata(pstDev);

    pstDevAttr = (void *)pstDevAttr;

    // example: echo clk_rate > ive_clk
    if (pu8Buf == NULL || 1 > (u8ScanRet = sscanf(pu8Buf, "%u", &u32SetClkRate)))
    {
        IVE_MSG(IVE_MSG_ERR, "usage: echo clk_rate > ive_clk.\n");
    }
    else
    {
        CamOsMutexLock(&pstDevData->mutex);
        if (IVE_CLK_ENABLE_ON != CamOsAtomicRead(&pstDevData->bClkEnable))
        {
            u8 u8ParentNum = 0;
            if (0 < (u8ParentNum = clk_hw_get_num_parents(__clk_get_hw(pstDevData->pstClk))))
            {
                int         i            = 0;
                struct clk *pstClkParent = NULL;

                for (i = 0; i < u8ParentNum; ++i)
                {
                    pstClkParent = clk_hw_get_parent_by_index(__clk_get_hw(pstDevData->pstClk), i)->clk;
                    if (u32SetClkRate == clk_get_rate(pstClkParent))
                    {
                        if (u32SetClkRate > IVE_MAX_NON_OD_CLK_FREQ)
                        {
                            IVE_MSG(IVE_MSG_WRN, "set od clk(%u), be sure core voltage is correct.\n", u32SetClkRate);
                        }
                        pstDevData->u32ClkRate = u32SetClkRate;
                        break;
                    }
                }

                if (i == u8ParentNum)
                {
                    IVE_MSG(IVE_MSG_ERR, "err clk_rete(%u).\n", u32SetClkRate);
                }
            }
            else
            {
                IVE_MSG(IVE_MSG_ERR, "err clk number(%d).\n", u8ParentNum);
            }
        }
        else
        {
            IVE_MSG(IVE_MSG_ERR, "clock had been enabled, can't change rate.\n");
        }
        CamOsMutexUnlock(&pstDevData->mutex);
    }

    return count;
}

DEVICE_ATTR(ive_axi, S_IRUSR | S_IWUSR, _DrvSysIveAxiGet, _DrvSysIveAxiSet);
DEVICE_ATTR(ive_clk, S_IRUSR | S_IWUSR, _DrvSysIveClkGet, _DrvSysIveClkSet);

void DrvIveSysInit(void *pIveDevData)
{
    ive_dev_data *    pstIveDevData = (ive_dev_data *)pIveDevData;
    struct CamDevice *pstDev        = pstIveDevData->pstDev;

    if (pstDev != NULL)
    {
        if (CamDeviceCreateFile(pstDev, &dev_attr_ive_axi))
        {
            CamOsPrintf("ive_axi create fail\n");
        }
        if (CamDeviceCreateFile(pstDev, &dev_attr_ive_clk))
        {
            CamOsPrintf("ive_clk create fail\n");
        }
    }
    return;
}

void DrvIveSysDeinit(void *pIveDevData)
{
    ive_dev_data *    pstIveDevData = (ive_dev_data *)pIveDevData;
    struct CamDevice *pstDev        = pstIveDevData->pstDev;

    CamDeviceRemoveFile(pstDev, &dev_attr_ive_axi);
    CamDeviceRemoveFile(pstDev, &dev_attr_ive_clk);

    return;
}
