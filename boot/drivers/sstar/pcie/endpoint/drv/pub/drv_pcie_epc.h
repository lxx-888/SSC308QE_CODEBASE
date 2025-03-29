/*
 * drv_pcie_epc.h - Sigmastar
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
#ifndef _DRV_PCIE_EPC_H_
#define _DRV_PCIE_EPC_H_

/* Synopsys specified PCIe registers */
#define SNPS_LINK_CONTROL             (0x710)
#define SNPS_LINK_MODE_MASK           (0x3f << 16)
#define SNPS_LINK_MODE_1_LANES        (0x1 << 16)
#define SNPS_LINK_MODE_2_LANES        (0x3 << 16)
#define SNPS_LINK_MODE_4_LANES        (0x7 << 16)
#define SNPS_LINK_MODE_8_LANES        (0xf << 16)
#define SNPS_LINK_WIDTH_SPEED_CONTROL (0x80C)
#define SNPS_SPEED_CHANGE             (0x1 << 17)
#define SNPS_AUTO_LANE_FLIP           (0x1 << 16)
#define SNPS_LINK_WIDTH_MASK          (0x1f << 8)
#define SNPS_LINK_WIDTH_1_LANES       (0x1 << 8)
#define SNPS_LINK_WIDTH_2_LANES       (0x2 << 8)
#define SNPS_LINK_WIDTH_4_LANES       (0x4 << 8)
#define SNPS_LINK_WIDTH_8_LANES       (0x8 << 8)
#define SNPS_FAST_TRAIN_SEQ(s)        ((s)&0xFF)
#define SNPS_TRGT_MAP_CTRL            (0x81C)
#define SNPS_MSI_ADDR_LOW             (0x820)
#define SNPS_MSI_ADDR_HIGH            (0x824)
#define SNPS_MSI_INTR0_ENABLE         (0x828)
#define SNPS_MSI_INTR0_MASK           (0x82C)
#define SNPS_MSI_INTR0_STATUS         (0x830)

#define SNPS_MISC_CONTROL_1_OFF (0x8BC)
#define SNPS_DBI_RO_WR_EN       (0x1)

/*
 * iATU Unroll-specific register definitions
 * From 4.80 core version the address translation will be made by unroll
 */
#define PCIE_ATU_UNR_REGION_CTRL1 0x00
#define PCIE_ATU_TYPE_MEM         (0x0)
#define PCIE_ATU_TYPE_IO          (0x2)
#define PCIE_ATU_TYPE_CFG0        (0x4)
#define PCIE_ATU_TYPE_CFG1        (0x5)
#define PCIE_ATU_UNR_REGION_CTRL2 0x04
#define PCIE_ATU_ENABLE           (0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE  (0x1 << 30)
#define PCIE_ATU_UNR_LOWER_BASE   0x08
#define PCIE_ATU_UNR_UPPER_BASE   0x0C
#define PCIE_ATU_UNR_LIMIT        0x10
#define PCIE_ATU_UNR_LOWER_TARGET 0x14
#define PCIE_ATU_UNR_UPPER_TARGET 0x18

/* Register address builder */
#define EPC_ATU_OB_UNR_REG_OFFSET(region) ((region) << 9)
#define EPC_ATU_IB_UNR_REG_OFFSET(region) (((region) << 9) | (0x1 << 8))

/* Parameters for the waiting for iATU enabled routine */
#define WAIT_IATU_RETRIES 5
#define WAIT_IATU_DELAY   9

static inline void _pcie_dbi_ro_wr_en(u8 id)
{
    u32 reg, val;

    reg = SNPS_MISC_CONTROL_1_OFF;
    val = sstar_pcieif_readl_dbi(id, reg);
    val |= SNPS_DBI_RO_WR_EN;
    sstar_pcieif_writel_dbi(id, reg, val);
}

static inline void _pcie_dbi_ro_wr_dis(u8 id)
{
    u32 reg, val;

    reg = SNPS_MISC_CONTROL_1_OFF;
    val = sstar_pcieif_readl_dbi(id, reg);
    val &= ~SNPS_DBI_RO_WR_EN;
    sstar_pcieif_writel_dbi(id, reg, val);
}

#define DEFAULT_DBI2_OFFSET (0x1 << 20)

static inline void _pcie_writel_dbi2(u8 id, u32 reg, u32 val)
{
    sstar_pcieif_writel_dbi(id, reg + DEFAULT_DBI2_OFFSET, val);
}

#define DEFAULT_ATU_OFFSET (0x3 << 20)

static inline void _pcie_writel_atu(u8 id, u32 reg, u32 val)
{
    sstar_pcieif_writel_dbi(id, reg + DEFAULT_ATU_OFFSET, val);
}

static inline u32 _pcie_readl_atu(u8 id, u32 reg)
{
    return sstar_pcieif_readl_dbi(id, reg + DEFAULT_ATU_OFFSET);
}

#define DEFAULT_DMA_OFFSET ((0x3 << 20) | (0x1 << 19))

static inline void _pcie_writel_dma(u8 id, u32 reg, u32 val)
{
    sstar_pcieif_writel_dbi(id, reg + DEFAULT_DMA_OFFSET, val);
}

static inline u32 _pcie_readl_dma(u8 id, u32 reg)
{
    return sstar_pcieif_readl_dbi(id, reg + DEFAULT_DMA_OFFSET);
}

#endif //_DRV_PCIE_EPC_H_
