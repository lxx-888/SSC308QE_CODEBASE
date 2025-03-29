/*
 * drv_miu_datatype.h - Sigmastar
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

#ifndef __DRV_MIU_DATATYPE_H__
#define __DRV_MIU_DATATYPE_H__

// only write
typedef enum
{
    MIU_CLIENTW_NULL        = 0x00,
    MIU_CLIENTW_SC_WDMA0    = 0x10,
    MIU_CLIENTW_SC_WDMA1    = 0x11,
    MIU_CLIENTW_SC_WDMA2    = 0x12,
    MIU_CLIENTW_SC_WDMA3    = 0x13,
    MIU_CLIENTW_USB30       = 0x14,
    MIU_CLIENTW_SC_WDMA4    = 0x15,
    MIU_CLIENTW_SC_WDMA5    = 0x16,
    MIU_CLIENTW_JPE         = 0x17,
    MIU_CLIENTW_SC_WDMA6    = 0x18,
    MIU_CLIENTW_JPD0        = 0x19,
    MIU_CLIENTW_IVE         = 0x1a,
    MIU_CLIENTW_AESDMA      = 0x1b,
    MIU_CLIENTW_SC_COL_INV  = 0x1c,
    MIU_CLIENTW_MIIC_TOP    = 0x1e,
    MIU_CLIENTW_BIST_SC0    = 0x1f,
    MIU_CLIENTW_GMAC0       = 0x30,
    MIU_CLIENTW_GMAC1       = 0x31,
    MIU_CLIENTW_GMAC_TOE0   = 0x32,
    MIU_CLIENTW_GMAC_TOE1   = 0x33,
    MIU_CLIENTW_GMAC_TOE2   = 0x34,
    MIU_CLIENTW_GMAC_TOE3   = 0x35,
    MIU_CLIENTW_GMAC_TOE4   = 0x36,
    MIU_CLIENTW_GMAC_TOE5   = 0x3a,
    MIU_CLIENTW_GMAC_TOE6   = 0x3b,
    MIU_CLIENTW_GMAC_TOE7   = 0x3c,
    MIU_CLIENTW_SGDMA       = 0x3d,
    MIU_CLIENTW_BIST_SC1    = 0x3f,
    MIU_CLIENTW_XZ_DECODE0  = 0x40,
    MIU_CLIENTW_XZ_DECODE1  = 0x41,
    MIU_CLIENTW_BACH0       = 0x43,
    MIU_CLIENTW_DBG_WDMA    = 0x44,
    MIU_CLIENTW_BDMA        = 0x45,
    MIU_CLIENTW_BDMA2       = 0x46,
    MIU_CLIENTW_BDMA3       = 0x47,
    MIU_CLIENTW_LOW_SPEED1  = 0x48,
    MIU_CLIENTW_SD          = 0x49,
    MIU_CLIENTW_FCIE        = 0x4a,
    MIU_CLIENTW_SDIO        = 0x4b,
    MIU_CLIENTW_CVS         = 0x4c,
    MIU_CLIENTW_BACH1       = 0x4d,
    MIU_CLIENTW_LOW_SPEED0  = 0x4e,
    MIU_CLIENTW_LDC         = 0x4f,
    MIU_CLIENTW_BIST_MISC   = 0x4f,
    MIU_CLIENTW_ISP_DMAG0   = 0x50,
    MIU_CLIENTW_ISP_DMAG1   = 0x51,
    MIU_CLIENTW_ISP_3DNR    = 0x52,
    MIU_CLIENTW_ISP_WDR     = 0x53,
    MIU_CLIENTW_ISP_ROT     = 0x54,
    MIU_CLIENTW_ISP_STA     = 0x55,
    MIU_CLIENTW_ISP_IMG     = 0x56,
    MIU_CLIENTW_ISP_DMAG2   = 0x57,
    MIU_CLIENTW_ISP_DMAG3   = 0x58,
    MIU_CLIENTW_ISP_TNR     = 0x59,
    MIU_CLIENTW_ISP_VIF_STA = 0x5a,
    MIU_CLIENTW_VENC0       = 0x5e,
    MIU_CLIENTW_BIST_ISP    = 0x5f,
    MIU_CLIENTW_IPU         = 0x71,
    MIU_CLIENTW_CA55        = 0x72,
    MIU_CLIENTW_VENC        = 0x73,
    MIU_CLIENTW_BWLA        = 0x75,
} miu_clientw_id;

#endif // #ifndef __DRV_MIU_DATATYPE_H__
