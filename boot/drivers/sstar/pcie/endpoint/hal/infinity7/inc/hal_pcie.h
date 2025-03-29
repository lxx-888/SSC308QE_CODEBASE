/*
 * hal_pcie.h - Sigmastar
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
#ifndef _HAL_PCIE__H_
#define _HAL_PCIE__H_

#include "asm/arch/mach/sstar_types.h"

/*
 * Design dependent parts
 */
#define HAL_PCIE_OB_ADDR(n) (0x1A000000 + (n * 0x01000000))

// Configure parameters
#define HAL_PCIE_OB_SPACE_SZ (0x01000000) // 16MB
#define HAL_PCIE_EP_PAGE_SZ  (0x00010000) // 64KB,   CX_ATU_MIN_REGION_SIZE
#define HAL_PCIE_IB_WIN_NUM  (2)          // 2 inbound windows,  CX_ATU_NUM_INBOUND_REGIONS
#define HAL_PCIE_OB_WIN_NUM  (2)          // 2 outbound windows, CX_ATU_NUM_OUTBOUND_REGIONS
#define HAL_PCIE_NUM_LANES   (2)          // 2 lanes

#define HAL_PCIE_BAR0_FIXSZ (0x100000) // 1MB,  BAR0_SIZING_SCHEME_0 = Fixed Mask (0)
#define HAL_PCIE_BAR1_FIXSZ (0x10000)  // 64KB, BAR1_SIZING_SCHEME_0 = Fixed Mask (0)
#define HAL_PCIE_BAR2_FIXSZ (0x100000) // 1MB,  BAR2_SIZING_SCHEME_0 = Fixed Mask (0)
#define HAL_PCIE_BAR3_FIXSZ (0x10000)  // 64KB, BAR3_SIZING_SCHEME_0 = Fixed Mask (0)
#define HAL_PCIE_BAR4_FIXSZ (0x100)    // 256,  BAR4_SIZING_SCHEME_0 = Fixed Mask (0)
#define HAL_PCIE_BAR5_FIXSZ (0x10000)  // 64KB, BAR5_SIZING_SCHEME_0 = Fixed Mask (0)

// Bank addresses
#define RIU_BASE_ADDR              (0x1F000000)
#define REG_ADDR_BASE_PCIE_DBI(n)  (RIU_BASE_ADDR + (0x1A8000 + (n * 0x800)) * 2)
#define REG_ADDR_BASE_PCIE_DBI2(n) (RIU_BASE_ADDR + (0x1A8000 + (n * 0x800)) * 2)
#define REG_ADDR_BASE_PCIE_PHY(n)  (RIU_BASE_ADDR + (0x163A00 + (n * 0x100)) * 2)
#define REG_ADDR_BASE_PCIE_MAC(n)  (RIU_BASE_ADDR + (0x163C00 + (n * 0x100)) * 2)

// Mac registers
#define REG_PCIE_DEVICE_TYPE  (0x02 << 2)
#define PCIE_DEVICE_EP        (0) // PCI Express Endpoint
#define PCIE_DEVICE_LEGACY_EP (1) // Legacy PCI Express Endpoint
#define PCIE_DEVICE_RC        (4) // Root port of PCIe Express Root Complex
#define REG_PCIE_IRQ_MASK     (0x06 << 2)
#define REG_PCIE_IRQ2_MASK    (0x14 << 2)
#define REG_PCIE_IRQ2_CLR     (0x16 << 2)
#define REG_PCIE_IRQ2_TYPE    (0x18 << 2)
#define REG_PCIE_IRQ2_STAT    (0x19 << 2)
#define IRQ2_INTR_MSI         (0x100)
#define IRQ2_INTR_ADDR_MATCH  (0x200)
#define REG_SLV_AWMISC_INFO   (0x20 << 2) // AXI: slave status count slv_awmisc_info[4:0] is the TLP type
#define REG_SLV_ARMISC_INFO   (0x28 << 2) // AXI: slave status count slv_armisc_info[4:0] is the TLP type
#define TLP_TYPE_MEM          (0)         //  5'b00000 => Memory
#define TLP_TYPE_IO           (2)         //  5'b00010 => I/O
#define TLP_TYPE0_CFG         (4)         //  5'b00100 => Type0 Configuration
#define TLP_TYPE1_CFG         (5)         //  5'b00101 => Type1 Configuration
#define REG_SLV_WSTRB_MASK    (0x2B << 2) // reg_slv_wstrb_mask
#define REG_DBI_ADDR0         (0x3D << 2) // reg_dbi_addr_20to12
#define REG_DBI_ADDR1         (0x3E << 2) // reg_dbi_addr_32to21
#define REG_PCIE_MAC_CTRL     (0x60 << 2) // PCIe MAC Control Registers
#define PCIE_LTSSM_EN         (1 << 4)    //  reg_app_ltssm_enable
#define REG_WR_MATCH_ADDR0    (0x7D << 2) // reg_pcie_mstwr_addr15to0_match
#define REG_WR_MATCH_ADDR1    (0x7E << 2) // reg_pcie_mstwr_addr31to16_match
#define REG_WR_MATCH_ADDR2    (0x7F << 2) // reg_pcie_mstwr_addr35to32_match

// Phy registers
#define REG_VREG_BYPASS     (0x05 << 2) // reg_phy_vreg_bypass: Built-in 3.3V Regulator Bypass Mode
#define VREG_BYPASS_ON      (1)
#define REG_SLV_READ_BYPASS (0x27 << 2) // reg_slv_read_bypass
#define SLC_READ_BYPASS_OFF (1 << 8)

#endif //_HAL_PCIE__H_
