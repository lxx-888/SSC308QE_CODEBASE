/*
 * hal_pcieif.h- Sigmastar
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

#ifndef _HAL_PCIEIF_H_
#define _HAL_PCIEIF_H_

/*
 * platform dependent
 */
#define PCIE_INTF_NUM (2)

/*
 * registers
 */
/* riu mac reg */
#define REG_PCIE_DEVICE_TYPE  (0x02 << 2)
#define PCIE_DEVICE_EP        (0)      // PCI Express Endpoint
#define PCIE_DEVICE_LEGACY_EP (1 << 0) // Legacy PCI Express Endpoint
#define PCIE_DEVICE_RC        (1 << 2) // Root port of PCIe Express Root Complex
#define REG_PCIE_IRQ_MASK     (0x06 << 2)
#define REG_PCIE_IRQ2_MASK    (0x14 << 2)
#define REG_PCIE_IRQ2_CLR     (0x16 << 2)
#define REG_PCIE_IRQ2_TYPE    (0x18 << 2)
#define REG_PCIE_IRQ2_STAT    (0x19 << 2)
#define IRQ2_INTR_MSI         (0x100)
#define IRQ2_INTR_ADDR_MATCH  (0x200)
#define REG_SLV_ARMISC_INFO   (0x28 << 2) // AXI: slave status count slv_armisc_info[4:0] is the TLP type
#define TLP_TYPE_MEM          (0)         // 5'b00000 => Memory
#define TLP_TYPE_IO           (2)         // 5'b00010 => I/O
#define TLP_TYPE0_CFG         (4)         // 5'b00100 => Type0 Configuration
#define TLP_TYPE1_CFG         (5)         // 5'b00101 => Type1 Configuration
#define REG_SLV_WSTRB_MASK    (0x2B << 2) // reg_slv_wstrb_mask
#define REG_DBI_ADDR_20to12   (0x3D << 2) // reg_dbi_addr_20to12
#define REG_DBI_ADDR_32to21   (0x3E << 2) // reg_dbi_addr_32to21
#define REG_PCIE_MAC_CTRL     (0x60 << 2) // PCIe MAC Control Registers
#define PCIE_LTSSM_EN         (1 << 4)    // reg_app_ltssm_enable
#define REG_WR_MATCH_ADDR0    (0x7D << 2) // reg_pcie_mstwr_addr15to0_match
#define REG_WR_MATCH_ADDR1    (0x7E << 2) // reg_pcie_mstwr_addr31to16_match
#define REG_WR_MATCH_ADDR2    (0x7F << 2) // reg_pcie_mstwr_addr35to32_match
#define REG_PHYVREG_BPS       (0x05 << 2) // reg_phy_vreg_bypass,1=1.8V;0=3.3V
/* riu phy reg */
#define REG_SLV_READ_BYPASS (0x27 << 2) // reg_slv_read_bypass

#define REG_ADDR_BASE_PCIE_DBI(n) (void *)(IO_VIRT + (0x1A8000 + (n * 0x800)) * 2)
#define REG_ADDR_BASE_PCIE_PHY(n) (void *)(IO_VIRT + (0x163A00 + (n * 0x100)) * 2)
#define REG_ADDR_BASE_PCIE_MAC(n) (void *)(IO_VIRT + (0x163C00 + (n * 0x100)) * 2)

/* DBI address bus layout
 *
 * Due the 32-bit data type length, reg addr bit[21:20] are used as CDM/ELBI & CS2 select bits
 *
 * In the design:
 * type | 32 | 31 | 30-20 | 19 | 18-2 | 1 | 0 |
 *  CMD |  0 | CS2|       |  0 |      | 0 | 0 |
 *  ATU |  1 |  1 |       |  0 | addr | 0 | 0 |
 *  DMA |  1 |  1 |       |  1 | addr | 0 | 0 |
 *
 *
 * reg addr layout:
 * type | 31-22 | 21 | 20 | 19 | 18-2 | 1 | 0 |
 *  CMD |       |  0 | CS2|  0 |      | 0 | 0 |
 *  ATU |       |  1 |  1 |  0 | addr | 0 | 0 |
 *  DMA |       |  1 |  1 |  1 | addr | 0 | 0 |
 */
static inline u32 hal_prog_dbi_addr(void __iomem *mac_base, u32 reg)
{
    u32 bits = 0;

    bits = reg >> 12;
    if (bits)
    {
        // bit[31:12] are not zeros
        // here is a trick, bit[21:20] is actually for bit[32:31]
        bits = (bits & 0xFFCFF) | ((reg & 0x300000) >> 1);
        writew(bits & 0x1FF, mac_base + REG_DBI_ADDR_20to12);
        writew((bits >> 9) & 0xFFF, mac_base + REG_DBI_ADDR_32to21);
    }
    return reg & 0xFFF; // dbi addr bit[11:0]
}

static inline void hal_rst_dbi_addr(void __iomem *mac_base)
{
    writew(0, mac_base + REG_DBI_ADDR_20to12);
    writew(0, mac_base + REG_DBI_ADDR_32to21);
}

static inline void hal_clk_enable(u8 id)
{
#define REG_ADDR_BASE_CLKGEN  (void *)(IO_VIRT + (0x103800) * 2)
#define REG_CKG_IPUFF         (0x50 << 2)
#define REG_ADDR_BASE_CLKGEN2 (void *)(IO_VIRT + (0x103F00) * 2)
#define REG_CKG_PCIE0         (0x45 << 2)
#define REG_CKG_PCIE1         (0x4B << 2)

#define REG_ADDR_BASE_XTAL_ATOP (void *)(IO_VIRT + (0x111B00) * 2)
#define REG_ADDR_BASE_MPLL      (void *)(IO_VIRT + (0x103000) * 2)
#define REG_ADDR_BASE_UPLL1     (void *)(IO_VIRT + (0x141F00) * 2)
#define REG_ADDR_BASE_SATA_MAC  (void *)(IO_VIRT + (0x143A00) * 2)
#define REG_ADDR_BASE_SATA_MAC2 (void *)(IO_VIRT + (0x144100) * 2)
#define REG_ADDR_BASE_SATA_PHY2 (void *)(IO_VIRT + (0x143D00) * 2)

    void __iomem *addr;
    u16           reg;

    /*  reg_ckg_ipuff, 0x1038 0x50[8] = 1'b0 */
    addr = (void *)(REG_ADDR_BASE_CLKGEN + REG_CKG_IPUFF);
    reg  = readw(addr);
    reg &= ~(0x0100);
    writew(reg, addr);

    if (id == 0)
    {
        /*  reg_ckg_pcie0, 0x103F 0x45[0] = 1'b0 */
        addr = (void *)(REG_ADDR_BASE_CLKGEN2 + REG_CKG_PCIE0);
    }
    else
    {
        /*  reg_ckg_pcie1, 0x103F 0x4B[0] = 1'b0 */
        addr = (void *)(REG_ADDR_BASE_CLKGEN2 + REG_CKG_PCIE1);
    }
    reg = readw(addr);
    reg &= ~(0x0001);
    writew(reg, addr);
}

void hal_internalclk_en(u8 id)
{
    void __iomem *addr;
    u16           u16regdat = 0;

    if (id == 1)
        return;

    // upll1 for pcie//
    //  xtal_atop wriu 0x00111b12 0x00
    addr      = (REG_ADDR_BASE_XTAL_ATOP + 0x12 * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF00;
    writew(u16regdat, addr);

    //+++wriu 0x00103003 0x00
    addr      = (REG_ADDR_BASE_MPLL + 0x02 * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF;
    writew(u16regdat, addr);

    // wriu 0x00141f46 0x32
    addr      = (REG_ADDR_BASE_UPLL1 + 0x46 * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF00;
    writew(u16regdat | 0x32, addr);

    // wriu 0x00141f4e 0xbc
    addr      = (REG_ADDR_BASE_UPLL1 + 0x4e * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF00;
    writew(u16regdat | 0xbc, addr);

    // wriu 0x00141f00 0x00
    addr      = (REG_ADDR_BASE_UPLL1 + 0x00 * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF00;
    writew(u16regdat, addr);

    // wriu 0x00141f40 0x20
    addr      = (REG_ADDR_BASE_UPLL1 + 0x40 * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF00;
    writew(u16regdat | 0x20, addr);

    // wriu 0x00141f4a 0xaf
    addr      = (REG_ADDR_BASE_UPLL1 + 0x4a * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF00;
    writew(u16regdat | 0xaf, addr);

    // wriu 0x00141f4c 0x80
    addr      = (REG_ADDR_BASE_UPLL1 + 0x4c * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF00;
    writew(u16regdat | 0x80, addr);

    //+++wriu 0x00141f4d 0x81
    addr      = (REG_ADDR_BASE_UPLL1 + 0x4c * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF;
    writew(u16regdat | 0x8100, addr);

    //+++wriu 0x00141f43 0x01
    addr      = (REG_ADDR_BASE_UPLL1 + 0x42 * 2);
    u16regdat = readw(addr);
    u16regdat &= 0xFF;
    writew(u16regdat | 0x0100, addr);

    // SATA
    addr = (REG_ADDR_BASE_SATA_MAC + 0x00 * 4);
    writew(0, addr);
    addr = (REG_ADDR_BASE_CLKGEN + 0x6e * 4);
    writew(0, addr);
    addr = (REG_ADDR_BASE_CLKGEN + 0x6c * 4);
    writew(0, addr);
    addr = (REG_ADDR_BASE_CLKGEN + 0x46 * 4);
    writew(0, addr);

    addr      = (REG_ADDR_BASE_SATA_MAC2 + 0x00 * 4);
    u16regdat = readw(addr);
    u16regdat &= ~(BIT(12));
    writew(u16regdat, addr);

    addr      = (REG_ADDR_BASE_SATA_PHY2 + 0x14 * 4);
    u16regdat = readw(addr);
    u16regdat |= BIT(0);
    writew(u16regdat, addr);

    addr      = (REG_ADDR_BASE_SATA_PHY2 + 0x14 * 4);
    u16regdat = readw(addr);
    u16regdat |= BIT(12);
    writew(u16regdat, addr);

    addr      = (REG_ADDR_BASE_SATA_PHY2 + 0x15 * 4);
    u16regdat = readw(addr);
    u16regdat |= BIT(0) | BIT(1);
    writew(u16regdat, addr);

    addr      = (REG_ADDR_BASE_SATA_PHY2 + 0x16 * 4);
    u16regdat = readw(addr);
    u16regdat |= BIT(10) | BIT(11);
    writew(u16regdat, addr);

    addr      = (REG_ADDR_BASE_SATA_PHY2 + 0x31 * 4);
    u16regdat = readw(addr);
    u16regdat |= BIT(1) | BIT(2);
    writew(u16regdat, addr);
}

#endif /* _HAL_PCIEIF_H */
