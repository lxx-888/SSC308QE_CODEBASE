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

typedef enum
{
    // for write
    MIU_CLIENTW_NULL        = 0x00,
    MIU_CLIENTW_XZ_DECODE   = 0x01,
    MIU_CLIENTW_XZ_DECODE2  = 0x02,
    MIU_CLIENTW_BACH        = 0x03,
    MIU_CLIENTW_BDMA        = 0x05,
    MIU_CLIENTW_BDMA2       = 0x06,
    MIU_CLIENTW_JPE         = 0x07,
    MIU_CLIENTW_LOW_SPEED1  = 0x08,
    MIU_CLIENTW_SD          = 0x09,
    MIU_CLIENTW_USB20       = 0x0b,
    MIU_CLIENTW_LOW_SPEED0  = 0x0d,
    MIU_CLIENTW_RIU_RECODER = 0x0e,
    MIU_CLIENTW_BIST0       = 0x0f,
    MIU_CLIENTW_PMIMI       = 0x10,
    MIU_CLIENTW_VENC0_CODEC = 0x11,
    MIU_CLIENTW_VENC0_SAD   = 0x12,
    MIU_CLIENTW_AESDMA      = 0x14,
    MIU_CLIENTW_LDC         = 0x15,
    MIU_CLIENTW_SC_WDMA0    = 0x16,
    MIU_CLIENTW_SC_WDMA2    = 0x17,
    MIU_CLIENTW_MIIC        = 0x18,
    MIU_CLIENTW_SC_WDMA1    = 0x1a,
    MIU_CLIENTW_IVE         = 0x1c,
    MIU_CLIENTW_EMAC        = 0x1d,
    MIU_CLIENTW_EMAC1       = 0x1e,
    MIU_CLIENTW_BIST1       = 0x1f,
    MIU_CLIENTW_ISP_3DNR    = 0x20,
    MIU_CLIENTW_ISP_DMAG0   = 0x21,
    MIU_CLIENTW_ISP_DMAG1   = 0x22,
    MIU_CLIENTW_ISP_STA     = 0x23,
    MIU_CLIENTW_ISP_IMG     = 0x24,
    MIU_CLIENTW_ISP_ROT     = 0x25,
    MIU_CLIENTW_ISP_WDR     = 0x26,
    MIU_CLIENTW_ISP_IIR     = 0x27,
    MIU_CLIENTW_BIST2       = 0x2f,
    MIU_CLIENTW_IPU         = 0x71,
    MIU_CLIENTW_CPU_CA32    = 0x72,
    MIU_CLIENTW_BWLA        = 0x75,
    // for read
    MIU_CLIENTR_XZ_DECODE   = 0x02,
    MIU_CLIENTR_BACH        = 0x03,
    MIU_CLIENTR_CMDQ        = 0x04,
    MIU_CLIENTR_BDMA        = 0x05,
    MIU_CLIENTR_BDMA2       = 0x06,
    MIU_CLIENTR_JPE         = 0x07,
    MIU_CLIENTR_LOW_SPEED1  = 0x08,
    MIU_CLIENTR_SD          = 0x09,
    MIU_CLIENTR_CMDQ1       = 0x0a,
    MIU_CLIENTR_USB20       = 0x0b,
    MIU_CLIENTR_GOP_SC0     = 0x0c,
    MIU_CLIENTR_LOW_SPEED0  = 0x0d,
    MIU_CLIENTR_BIST0       = 0x0f,
    MIU_CLIENTR_PMIMI       = 0x10,
    MIU_CLIENTR_VENC0_CODEC = 0x11,
    MIU_CLIENTR_VENC0_SAD   = 0x12,
    MIU_CLIENTR_ISP0_CMDQ   = 0x13,
    MIU_CLIENTR_AESDMA      = 0x14,
    MIU_CLIENTR_LDC         = 0x15,
    MIU_CLIENTR_SC_RDMA0    = 0x17,
    MIU_CLIENTR_MIIC        = 0x18,
    MIU_CLIENTR_SC_RDMA1    = 0x19,
    MIU_CLIENTR_GOP_JPE0    = 0x1b,
    MIU_CLIENTR_IVE         = 0x1c,
    MIU_CLIENTR_EMAC        = 0x1d,
    MIU_CLIENTR_EMAC1       = 0x1e,
    MIU_CLIENTR_BIST1       = 0x1f,
    MIU_CLIENTR_ISP_3DNR    = 0x20,
    MIU_CLIENTR_ISP_DMAG0   = 0x21,
    MIU_CLIENTR_ISP_ROT     = 0x25,
    MIU_CLIENTR_ISP_WDR     = 0x26,
    MIU_CLIENTR_ISP_IIR     = 0x27,
    MIU_CLIENTR_ISP_MLOAD   = 0x28,
    MIU_CLIENTR_ISP_LDC     = 0x29,
    MIU_CLIENTR_BIST2       = 0x2f,
    MIU_CLIENTR_IPU         = 0x71,
    MIU_CLIENTR_CPU_CA32    = 0x72,
    MIU_CLIENTR_BWLA        = 0x75,
} miu_client_id;

#endif // #ifndef __DRV_MIU_DATATYPE_H__
