/*
 * drv_sdmmc_common.h- Sigmastar
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
 * FileName drv_sdmmc_common.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#ifndef __SS_SDMMC_COMMON_H
#define __SS_SDMMC_COMMON_H

#include "hal_sdmmc_base.h"
#include "hal_sdmmc_platform.h"
#include "hal_sdmmc_base.h"
#if (D_OS == D_OS__LINUX)
#include "drv_sdmmc_lnx.h"
#elif (D_OS == D_OS__RTK)
#include "drv_sdmmc_rtk.h"
#endif

//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************

//***********************************************************************************************************
//***********************************************************************************************************

void Hal_CARD_SetGPIOIntAttr(GPIOOptEmType eGPIOOPT, unsigned int irq);
// extern void SwitchPAD(struct sstar_mmc_priv * p_mmc_priv);
void   sstar_sdmmc_enable(struct sstar_mmc_priv *p_mmc_priv);
void   sstar_sdmmc_setpower(struct sstar_mmc_priv *p_mmc_priv, U8_T u8PowerMode);
U32_T  sstar_sdmmc_setclock(struct sstar_mmc_priv *p_mmc_priv, unsigned int u32ReffClk);
void   sstar_sdmmc_setbuswidth(struct sstar_mmc_priv *p_mmc_priv, U8_T u8BusWidth);
void   sstar_sdmmc_setbustiming(struct sstar_mmc_priv *p_mmc_priv, U8_T u8BusTiming);
BOOL_T IsAdmaMode(struct sstar_mmc_priv *p_mmc_priv);

void sstar_sdmmc_request(struct mmc_host *p_mmc_host, struct mmc_request *p_mmc_req);

#endif // End of __SS_SDMMC_COMMON_H
