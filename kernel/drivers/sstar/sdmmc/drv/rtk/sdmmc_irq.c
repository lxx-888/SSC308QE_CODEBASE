/*
 * sdmmc_irq.c- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName sdmmc_irq.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#include "cam_os_wrapper.h"
#include "drv_int_ctrl_pub_api.h"
#include "hal_int_ctrl_pub.h"
#include "hal_sdmmc_intr.h"
#include "sdmmc_irq.h"
#include "sdio.h"
#include "sd.h"
#include "drv_sdmmc_rtk.h"
#include "hal_sdmmc_platform_pri_config.h"

CamOsCondition_t stSdSem[3];

extern sdio_irq_callback *    sdio_irq_cb[SDMMC_NUM_TOTAL];
extern struct sstar_mmc_priv *gp_mmc_priv[SDMMC_NUM_TOTAL];

static void _SdmmcIsr(U32_T d, void *dev_id)
{
    // irqreturn_t irq_t = IRQ_NONE;
    /*IntSourceStruct* pstIntSource = dev_id;
    IpOrder eIP  = pstIntSource->eIP;
    //struct ms_sdmmc_slot  *p_sdmmc_slot = pstIntSource->p_data;

    //gu32_sd_irq_count++;
    //MIEEvent = 1;
    //Trig_MIE_INTR = 1;
    if(HAL_CARD_INT_DetectSDIOInt(eIP))
    {
        sdio_irq_cb[pstIntSource->slotNo](pstIntSource->slotNo);
    }

    if(HAL_CARD_INT_SaveMIEEvent(eIP))
    {
        CamOsConditionWakeUpAll(&stSdSem[eIP]);
    }

    return;*/
    // return irq_t;

    return Hal_CARD_INT_MIE(d, dev_id);
}

int SdmmcIrqRequest(void *dev_id, U32_T u32_intrNo)
{
    IntSourceStruct *pstIntSource = dev_id;
    IpOrder          eIP          = pstIntSource->eIP;
    CamOsConditionInit(&stSdSem[eIP]);

    return CamOsIrqRequest(u32_intrNo, _SdmmcIsr, "sdmmc_mie", dev_id);
}

int SdmmcIrqFree(void *dev_id, U32_T u32_intrNo)
{
    IntSourceStruct *pstIntSource = dev_id;
    IpOrder          eIP          = pstIntSource->eIP;
    CamOsConditionDeinit(&stSdSem[eIP]);
    CamOsIrqFree(u32_intrNo, dev_id);
    return 0;
}

static void _sd_card_ins(void *dev_id)
{
    IntSourceStruct *pstIntSource = dev_id;
    U32_T            u32_slotNo   = pstIntSource->slotNo;
    sd_card_init(u32_slotNo);
}

static void _sd_card_rm(void *dev_id)
{
    IntSourceStruct *pstIntSource = dev_id;
    U32_T            u32_slotNo   = pstIntSource->slotNo;
    sd_card_remove(gp_mmc_priv[u32_slotNo]);
}

static void _SdmmcCdzIsr(void *dev_id)
{
    IntSourceStruct *pstIntSource = dev_id;
    SlotEmType       eSlot        = (SlotEmType)pstIntSource->slotNo;

    if (MS_SD_Card_Detect(pstIntSource->slotNo))
    {
        CamOsWorkQueueAdd(cdzWorkQueue[eSlot], (void *)_sd_card_ins, dev_id, 10);
        DrvIntcSetPolarity(INTC_POLARITY_HIGH, gp_mmc_priv[eSlot]->u32_cdzIrqNo);
    }
    else
    {
        CamOsWorkQueueAdd(cdzWorkQueue[eSlot], (void *)_sd_card_rm, dev_id, 10);
        DrvIntcSetPolarity(INTC_POLARITY_LOW, gp_mmc_priv[eSlot]->u32_cdzIrqNo);
    }
}

static void _SdmmcHotPlug(U32_T d, void *dev_id)
{
    IntSourceStruct *pstIntSource = dev_id;
    SlotEmType       eSlot        = (SlotEmType)pstIntSource->slotNo;
    CamOsWorkQueueAdd(hotplugWorkQueue[eSlot], (void *)_SdmmcCdzIsr, dev_id, 10);
}

int SdmmcCdzIrqRequest(void *dev_id, U32_T u32_intrNo)
{
    return CamOsIrqRequest(u32_intrNo, _SdmmcHotPlug, "sdmmc_cdz", dev_id);
}

int SdmmcCdzIrqFree(void *dev_id, U32_T u32_intrNo)
{
    CamOsIrqFree(u32_intrNo, dev_id);
    return 0;
}
