/*
 * mhal_gmac.h- Sigmastar
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

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define PA2BUS(a) ((a) - (CONFIG_SYS_SDRAM_BASE))

#undef writel
#undef readl
#define writel(v, reg)                                    \
    (                                                     \
        {                                                 \
            u32 __v = v;                                  \
            __iowmb();                                    \
            __arch_putl(__v, dwmac_to_sigma_rebase(reg)); \
            __v;                                          \
        })
#define readl(reg)                                             \
    (                                                          \
        {                                                      \
            u32 __v = __arch_getl(dwmac_to_sigma_rebase(reg)); \
            __iormb();                                         \
            __v;                                               \
        })

#undef clrbits_le32
#undef setbits_le32
#undef clrsetbits_le32

#define clrbits_le32(addr, clear)         clrbits(le32, dwmac_to_sigma_rebase(addr), clear)
#define setbits_le32(addr, set)           setbits(le32, dwmac_to_sigma_rebase(addr), set)
#define clrsetbits_le32(addr, clear, set) clrsetbits(le32, dwmac_to_sigma_rebase(addr), clear, set)

#define EQOS_MAC_MDIO_ADDRESS_CR_500_800 7

#define PHASE_CALB     0
#define DYN_PHASE_CALB 0
#define GMAC0_Base     0x1F32A000
#define GMAC1_Base     0x1F32B400

extern int Chip_Get_Revision(void);
#define GMAC_CHIP_REVISION_U01 0x01
#define NEED_CALB              ((Chip_Get_Revision() == GMAC_CHIP_REVISION_U01) ? 1 : 0)

#define txc_0_phase   0x0000
#define txc_90_phase  0x1800
#define txc_180_phase 0x800
#define txc_270_phase 0x1000

#define GMAC_CALB_MSK         (BIT(2) | BIT(3))
#define GMAC_RESV_SPEED_MSK   (BIT(14) | BIT(15))
#define GMAC_SEL_MSK          (GMAC_CALB_MSK | GMAC_RESV_SPEED_MSK)
#define GMAC_TX_PHASE_MSK     (BIT(11) | BIT(12))
#define GMAC_RGMII_RXCTL_MODE (BIT(15))

extern char gGmacEPF;
extern char gGmacDumpTx;
extern char gGmacDumpRx;
extern char gGmacLoopback;
extern int  gGmacLoopbackLen;
extern int  gGmacLoopbackSpeed;
//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
int   sstar_gamc_adjust_driving(int gmacid, int io, int gear);
int   sstar_gmac_probe(phys_addr_t reg, int interface, int mclk_freq, int mclk_mode, int reset_io);
int   sstar_gmac_start_clock(phys_addr_t reg);
int   sstar_gmac_stop_clock(phys_addr_t reg);
int   sstar_gmac_get_rate(phys_addr_t reg);
int   sstar_gmac_set_rate(phys_addr_t reg, int speed);
ulong dwmac_to_sigma_rebase(void __iomem *addr);
#if 0
void MHal_GMAC_ReadReg32( u32* addr );
u8 MHal_GMAC_ReadReg8( u32 bank, u32 reg );
void MHal_GMAC_WritReg8( u32 bank, u32 reg, u8 val );
#endif
int  sstar_gmac_set_tx_clk_pad_sel(phys_addr_t regs, int interface, int speed);
void sstar_gmac_dump_packet(char *packet, int length, int dir /*0 for TX, 1 for RX*/);
void sstar_gmac_loopback_test(void *dev, void *mii, int phyaddr, phys_addr_t regs, int pktlen, int speed);
#if DYN_PHASE_CALB
int  sstar_gmac_prepare_dyncalibrat(struct phy_device *phy, phys_addr_t regs, int interface, int speed);
void sstar_gmac_do_dyncalibrat(struct phy_device *phy, phys_addr_t regs, int interface, int speed);
#endif
