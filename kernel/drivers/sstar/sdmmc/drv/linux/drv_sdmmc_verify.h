/*
 * drv_sdmmc_verify.h- Sigmastar
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
 * FileName drv_sdmmc_verify.h
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 * 	   This file is the header file of ms_sdmmc_verify.c.
 *
 ***************************************************************************************************************/

#ifndef __SS_SDMMC_VERIFY_H
#define __SS_SDMMC_VERIFY_H

#include "drv_sdmmc_lnx.h"
#include "hal_sdmmc_base.h"
#include <linux/platform_device.h>

#define A_DMA_W_BASE 0x20006000
#define A_DMA_R_BASE 0x20008000
#define A_ADMA_BASE  0x2000A000 // Not Support

//***********************************************************************************************************

void IPV_SDMMC_TimerTest(U8_T u8Slot, U8_T u8Sec);
void IPV_SDMMC_CardDetect(struct sstar_sdmmc_slot *p_sdmmc_slot);
void IPV_SDMMC_Init(struct sstar_sdmmc_slot *p_sdmmc_slot);
void IPV_SDMMC_SetWideBus(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8BusWidth);
void IPV_SDMMC_SetHighBus(struct sstar_sdmmc_slot *p_sdmmc_slot);
void IPV_SDMMC_SetClock(struct sstar_mmc_priv *p_mmc_priv, U32_T u32ReffClk);
void IPV_SDMMC_SetBusTiming(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8BusTiming);
void IPV_SDMMC_CIFD_RW(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32SecBlkAddr, BOOL_T bHidden);
void IPV_SDMMC_DMA_RW(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32SecBlkAddr, U16_T u16SecCount, BOOL_T bHidden);

//###########################################################################################################

//###########################################################################################################
void IPV_SDMMC_ADMA_RW(struct sstar_sdmmc_slot *p_sdmmc_slot, U32_T u32SecBlkAddr, BOOL_T bHidden);
//###########################################################################################################

void IPV_SDMMC_BurnRW(struct sstar_sdmmc_slot *p_sdmmc_slot, U8_T u8TransType, U32_T u32StartSec, U32_T u32EndSec);
void IPV_SDMMC_TestPattern(U8_T u8Slot, U8_T u8Pattern);

void IPV_SDMMC_RW_Verify(struct device *dev, U8_T u8Val1);

#endif // End of __SS_SDMMC_VERIFY_H
