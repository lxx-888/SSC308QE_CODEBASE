/*
 * hal_sensor.c - Sigmastar
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

#include "cam_os_wrapper.h"
#include <common.h>
#include "sensor_datatype.h"
#include "hal_sensor.h"
#include "reg_sensor.h"
#include <sensor_log.h>

u32 *g_au32VifReg[E_VIF_CHANNEL_NUM] = {0};

SensorHandle_t g_astVifHandle[E_SENSOR_HANDLE_ID_MAX] = {
    {0, SENSOR_BASE_ADDR_W_BANK(0x1308), 0x800}, // 0 0x1308 VIF Core0
    {0, SENSOR_BASE_ADDR_W_BANK(0x103C), 0x200}, // 4 0x103C PADTOP
    {0, SENSOR_BASE_ADDR_W_BANK(0x1038), 0x200}, // 6 0x1038 CLKGEN
    {0, SENSOR_BASE_ADDR_W_BANK(0x101E), 0x200}, // 6 0x101E CHIPTOP
};

void HalSensorSetHandle(SensorHandleId_e nHandleId, uintptr_t nHandle)
{
    g_astVifHandle[nHandleId].nHandle = nHandle;
}

uintptr_t HalSensorGetHandle(SensorHandleId_e nHandleId)
{
    return g_astVifHandle[nHandleId].nHandle;
}

u32 HalSensorGetHandleBasePa(SensorHandleId_e nHandleId)
{
    return g_astVifHandle[nHandleId].nBasePA;
}

void HalVifSetAllPadIn(u32 nMode)
{
    volatile reg_chiptop *pChipTopHnd = (reg_chiptop *)HalSensorGetHandle(E_SENSOR_HANDLE_CHIPTOP);

    pChipTopHnd->reg_allpad_in = nMode;
}
/*
    This function directly map senor pad and channel for reset and powerdown pin controlling.
*/
s32 HalSensorGetVifChnBySnrPadId(u32 nSNRPadID, u32 *nPhyChn)
{
    switch (nSNRPadID)
    {
        case 0:
            *nPhyChn = 0;
            break;
        case 2:
            *nPhyChn = 4;
            break;
        default:
            SENSORIFLogDbg("[%s] Error, No such sensor pad %d\n", __func__, nSNRPadID);
            return SENSOR_FAIL;
    }
    return SENSOR_SUCCESS;
}

s32 HalSensorSetChnBaseAddr(VifChannel_e nPhyChn)
{
    s32 ret = SENSOR_SUCCESS;

    switch (nPhyChn)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            g_au32VifReg[nPhyChn] = (u32 *)HalSensorGetHandle(E_SENSOR_HANDLE_VIF_C0);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            g_au32VifReg[nPhyChn] = (u32 *)(HalSensorGetHandle(E_SENSOR_HANDLE_VIF_C0) + 0x200);
            break;
        default:
            SENSORIFLogDbg("[%s] VIF chanel number %d not support\n", __func__, nPhyChn);
            ret = SENSOR_NOT_SUPPORT;
            break;
    }

    if (SENSOR_SUCCESS == ret)
    {
        SENSORIFLogDbg("[%s] g_au32VifReg[%d]: %p\n", __func__, nPhyChn, g_au32VifReg[nPhyChn]);
    }
    return ret;
}

void HalSensorSetMipiPad(u32 nSNRPadID, u32 nMode)
{
    volatile reg_padtop *pstPadTopHandle = (reg_padtop *)HalSensorGetHandle(E_SENSOR_HANDLE_PADTOP);

    switch (nSNRPadID)
    {
        case 0:
            pstPadTopHandle->reg_sr0_mipi_mode = nMode;
            break;
        case 2:
            pstPadTopHandle->reg_sr0_mipi_mode = nMode;
            break;
        default:
            SENSORIFLogDbg("[%s] err, over VIF group number \n", __func__);
            break;
    }
}

void HalSensorSetVifSnrRstMode(u32 nSNRPadID, u8 nMode)
{
    volatile reg_padtop *pstPadTopHandle = (reg_padtop *)HalSensorGetHandle(E_SENSOR_HANDLE_PADTOP);

    switch (nSNRPadID)
    {
        case 0:
            pstPadTopHandle->reg_sr00_rst_mode = nMode;
            break;
        case 2:
            pstPadTopHandle->reg_sr01_rst_mode = nMode;
            break;
        default:
            SENSORLogErr("[%s] Error, No such sensor pad %d\n", __func__, nSNRPadID);
            break;
    }
}

void HalVifSensorReset(VifChannel_e nPhyChn, SensorPolarity_e OnOff) // sensor rest
{
    volatile reg_isp_vif *pstVifHandle = (reg_isp_vif *)g_au32VifReg[nPhyChn];

    pstVifHandle->reg_vif_ch0_sensor_rst = OnOff;
}

void HalSensorPowerDown(VifChannel_e nPhyChn, SensorPolarity_e OnOff) // sensor pwrdn
{
    volatile reg_isp_vif *pstVifHandle = (reg_isp_vif *)g_au32VifReg[nPhyChn];

    pstVifHandle->reg_vif_ch0_sensor_pwrdn = OnOff;
}

void HalSensorSetVifMclk(u32 nSNRPadID, SnrMclk_e nMclkIdx)
{
    volatile reg_CLKGEN *pClkgenHnd = (reg_CLKGEN *)HalSensorGetHandle(E_SENSOR_HANDLE_CLKGEN);
    u32                  nVal;

    nVal = nMclkIdx << 2;

    switch (nSNRPadID)
    {
        case 0:
            pClkgenHnd->reg_ckg_sr00_mclk = nVal;
            break;
        case 2:
            pClkgenHnd->reg_ckg_sr01_mclk = nVal;
            break;
        default:
            SENSORIFLogDbg("[VIF] Invalid VIF group for MCLK.");
            break;
    }
}

void HalSensorSetVifSnrMclkMode(u32 nSNRPadID, u8 nMode)
{
    volatile reg_padtop *pstPadTopHandle = (reg_padtop *)HalSensorGetHandle(E_SENSOR_HANDLE_PADTOP);

    switch (nSNRPadID)
    {
        case 0:
            pstPadTopHandle->reg_sr00_mclk_mode = nMode;
            break;
        case 2:
            pstPadTopHandle->reg_sr01_mclk_mode = nMode;
            break;
        default:
            SENSORLogErr("[%s] Error, No such sensor pad %d\n", __func__, nSNRPadID);
            break;
    }
}

void HalSensorSetVifSnrPdnMode(u32 nSNRPadID, u8 nMode)
{
    volatile reg_padtop *pstPadTopHandle = (reg_padtop *)HalSensorGetHandle(E_SENSOR_HANDLE_PADTOP);

    switch (nSNRPadID)
    {
        case 0:
            pstPadTopHandle->reg_sr00_pdn_mode = nMode;
            break;
        case 2:
            pstPadTopHandle->reg_sr01_pdn_mode = nMode;
            break;
        default:
            SENSORLogErr("[%s] Error, No such sensor pad %d\n", __func__, nSNRPadID);
            break;
    }
}

s32 HalSensorSetVifIoPad(u32 nSNRPadID, u32 eBusType, u8 SnrPadSel, u8 MclkPadSel, u8 RstPadSel, u8 PwnPadSel)
{
    s32                  ret             = SENSOR_SUCCESS;
    volatile reg_padtop *pstPadTopHandle = (reg_padtop *)HalSensorGetHandle(E_SENSOR_HANDLE_PADTOP);

    switch (eBusType)
    {
        case E_SENSOR_BUS_TYPE_MIPI:
            HalSensorSetMipiPad(nSNRPadID, SnrPadSel);
            HalSensorSetVifSnrPdnMode(nSNRPadID, PwnPadSel);
            HalSensorSetVifSnrRstMode(nSNRPadID, RstPadSel);
            HalSensorSetVifSnrMclkMode(nSNRPadID, MclkPadSel);
            break;
        case E_SENSOR_BUS_TYPE_PARL:
        case E_SENSOR_BUS_TYPE_BT656:
        case E_SENSOR_BUS_TYPE_BT1120:
        default:
            SENSORIFLogDbg("[%s] Sensor bus type %d not support\n", __FUNCTION__, eBusType);
            ret = SENSOR_NOT_SUPPORT;
            break;
    }
    return ret;
}
