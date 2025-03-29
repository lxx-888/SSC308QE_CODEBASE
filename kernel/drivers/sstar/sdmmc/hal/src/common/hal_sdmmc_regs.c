/*
 * hal_sdmmc_regs.c- Sigmastar
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

#include "hal_sdmmc_base.h"
#if (D_OS == D_OS__LINUX)
#include "drv_sdmmc_lnx.h"
#elif (D_OS == D_OS__RTK)
#include "drv_sdmmc_rtk.h"
#endif
#include "hal_sdmmc_platform_pri_config.h"

void* pIPBANKArr[SDMMC_NUM_TOTAL];
void* pPLLIPBANKArr[SDMMC_NUM_TOTAL];
void* pCIFDIPBANKArr[SDMMC_NUM_TOTAL];
void* pPWRSAVEIPBANKArr[SDMMC_NUM_TOTAL];

void Hal_CREG_SET_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv)
{
    pIPBANKArr[eIP] = (void*)(uintptr_t)p_mmc_priv->pIPBANKArr[0];
}

volatile void* Hal_CREG_GET_REG_BANK(IpOrder eIP)
{
    return pIPBANKArr[eIP];
}

void Hal_CREG_SET_PLL_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv)
{
#if (D_OS == D_OS__LINUX)
    pPLLIPBANKArr[eIP] = (void*)(uintptr_t)p_mmc_priv->pPLLIPBANKArr[0];
#endif
}

volatile void* Hal_CREG_GET_PLL_REG_BANK(IpOrder eIP)
{
    return pPLLIPBANKArr[eIP];
}

void Hal_CREG_SET_CIFD_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv)
{
#if (D_OS == D_OS__LINUX)
    pCIFDIPBANKArr[eIP] = (void*)(uintptr_t)p_mmc_priv->pCIFDIPBANKArr[0];
#endif
}

volatile void* Hal_CREG_GET_CIFD_REG_BANK(IpOrder eIP)
{
    return pCIFDIPBANKArr[eIP];
}

void Hal_CREG_SET_PWR_SAVE_REG_BANK(IpOrder eIP, struct sstar_mmc_priv* p_mmc_priv)
{
#if (D_OS == D_OS__LINUX)
    pPWRSAVEIPBANKArr[eIP] = (void*)(uintptr_t)p_mmc_priv->pPWRSAVEIPBANKArr[0];
#endif
}

volatile void* Hal_CREG_GET_PWR_SAVE_REG_BANK(IpOrder eIP)
{
    return pPWRSAVEIPBANKArr[eIP];
}
