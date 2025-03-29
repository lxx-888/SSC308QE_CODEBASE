/*
 * sdmmc_irq.h- Sigmastar
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
 * FileName sdmmc_irq.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#ifndef __SDMMC_IRQ_H
#define __SDMMC_IRQ_H

typedef void irqreturn_t;

extern int SdmmcIrqRequest(void *dev_id, U32_T u32_intrNo);
extern int SdmmcIrqFree(void *dev_id, U32_T u32_intrNo);

// cdz irq
extern int SdmmcCdzIrqRequest(void *dev_id, U32_T u32_intrNo);
extern int SdmmcCdzIrqFree(void *dev_id, U32_T u32_intrNo);

#endif // __SDMMC_IRQ_H
