// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * Portions based on U-Boot's rtl8169.c.
 */

/*
 * This driver supports the Synopsys Designware Ethernet QOS (Quality Of
 * Service) IP block. The IP supports multiple options for bus type, clocking/
 * reset structure, and feature list.
 *
 * The driver is written such that generic core logic is kept separate from
 * configuration-specific logic. Code that interacts with configuration-
 * specific resources is split out into separate functions to avoid polluting
 * common code. If/when this driver is enhanced to support multiple
 * configurations, the core code should be adapted to call all configuration-
 * specific functions through function pointers, with the definition of those
 * function pointers being supplied by struct udevice_id eqos_ids[]'s .data
 * field.
 *
 * The following configurations are currently supported:
 * tegra186:
 *    NVIDIA's Tegra186 chip. This configuration uses an AXI master/DMA bus, an
 *    AHB slave/register bus, contains the DMA, MTL, and MAC sub-blocks, and
 *    supports a single RGMII PHY. This configuration also has SW control over
 *    all clock and reset signals to the HW block.
 */

#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <eth_phy.h>
#ifdef CONFIG_ARCH_IMX8M
#include <asm/arch/clock.h>
#include <asm/mach-imx/sys_proto.h>
#endif
#include <linux/bitops.h>
#include <linux/delay.h>
#ifdef CONFIG_ARCH_SSTAR
#include "mhal_gmac.h"
#endif
/* Core registers */

#define EQOS_MAC_REGS_BASE 0x000
struct eqos_mac_regs {
	uint32_t configuration;				/* 0x000 */
	uint32_t unused_004[(0x070 - 0x004) / 4];	/* 0x004 */
	uint32_t q0_tx_flow_ctrl;			/* 0x070 */
	uint32_t unused_070[(0x090 - 0x074) / 4];	/* 0x074 */
	uint32_t rx_flow_ctrl;				/* 0x090 */
	uint32_t unused_094;				/* 0x094 */
	uint32_t txq_prty_map0;				/* 0x098 */
	uint32_t unused_09c;				/* 0x09c */
	uint32_t rxq_ctrl0;				/* 0x0a0 */
	uint32_t unused_0a4;				/* 0x0a4 */
	uint32_t rxq_ctrl2;				/* 0x0a8 */
	uint32_t unused_0ac[(0x0dc - 0x0ac) / 4];	/* 0x0ac */
	uint32_t us_tic_counter;			/* 0x0dc */
	uint32_t unused_0e0[(0x11c - 0x0e0) / 4];	/* 0x0e0 */
	uint32_t hw_feature0;				/* 0x11c */
	uint32_t hw_feature1;				/* 0x120 */
	uint32_t hw_feature2;				/* 0x124 */
	uint32_t unused_128[(0x200 - 0x128) / 4];	/* 0x128 */
	uint32_t mdio_address;				/* 0x200 */
	uint32_t mdio_data;				/* 0x204 */
	uint32_t unused_208[(0x300 - 0x208) / 4];	/* 0x208 */
	uint32_t address0_high;				/* 0x300 */
	uint32_t address0_low;				/* 0x304 */
};

#define EQOS_MAC_CONFIGURATION_GPSLCE			BIT(23)
#define EQOS_MAC_CONFIGURATION_CST			BIT(21)
#define EQOS_MAC_CONFIGURATION_ACS			BIT(20)
#define EQOS_MAC_CONFIGURATION_WD			BIT(19)
#define EQOS_MAC_CONFIGURATION_JD			BIT(17)
#define EQOS_MAC_CONFIGURATION_JE			BIT(16)
#define EQOS_MAC_CONFIGURATION_PS			BIT(15)
#define EQOS_MAC_CONFIGURATION_FES			BIT(14)
#define EQOS_MAC_CONFIGURATION_DM			BIT(13)
#define EQOS_MAC_CONFIGURATION_LM			BIT(12)
#define EQOS_MAC_CONFIGURATION_TE			BIT(1)
#define EQOS_MAC_CONFIGURATION_RE			BIT(0)

#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT		16
#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_MASK		0xffff
#define EQOS_MAC_Q0_TX_FLOW_CTRL_TFE			BIT(1)

#define EQOS_MAC_RX_FLOW_CTRL_RFE			BIT(0)

#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT		0
#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK		0xff

#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT			0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK			3
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_NOT_ENABLED		0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB		2
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV		1

#define EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT			0
#define EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK			0xff

#define EQOS_MAC_HW_FEATURE0_MMCSEL_SHIFT		8
#define EQOS_MAC_HW_FEATURE0_HDSEL_SHIFT		2
#define EQOS_MAC_HW_FEATURE0_GMIISEL_SHIFT		1
#define EQOS_MAC_HW_FEATURE0_MIISEL_SHIFT		0

#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT		6
#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK		0x1f
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT		0
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK		0x1f

#define EQOS_MAC_HW_FEATURE3_ASP_SHIFT			28
#define EQOS_MAC_HW_FEATURE3_ASP_MASK			0x3

#define EQOS_MAC_MDIO_ADDRESS_PA_SHIFT			21
#define EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT			16
#define EQOS_MAC_MDIO_ADDRESS_CR_SHIFT			8
#define EQOS_MAC_MDIO_ADDRESS_CR_20_35			2
#define EQOS_MAC_MDIO_ADDRESS_CR_250_300		5
#define EQOS_MAC_MDIO_ADDRESS_SKAP			BIT(4)
#define EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT			2
#define EQOS_MAC_MDIO_ADDRESS_GOC_READ			3
#define EQOS_MAC_MDIO_ADDRESS_GOC_WRITE			1
#define EQOS_MAC_MDIO_ADDRESS_C45E			BIT(1)
#define EQOS_MAC_MDIO_ADDRESS_GB			BIT(0)

#define EQOS_MAC_MDIO_DATA_GD_MASK			0xffff

#define EQOS_MTL_REGS_BASE 0xd00
struct eqos_mtl_regs {
	uint32_t txq0_operation_mode;			/* 0xd00 */
	uint32_t unused_d04;				/* 0xd04 */
	uint32_t txq0_debug;				/* 0xd08 */
	uint32_t unused_d0c[(0xd18 - 0xd0c) / 4];	/* 0xd0c */
	uint32_t txq0_quantum_weight;			/* 0xd18 */
	uint32_t unused_d1c[(0xd30 - 0xd1c) / 4];	/* 0xd1c */
	uint32_t rxq0_operation_mode;			/* 0xd30 */
	uint32_t unused_d34;				/* 0xd34 */
	uint32_t rxq0_debug;				/* 0xd38 */
};

#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT		16
#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK		0x1ff
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT	2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_MASK		3
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED	2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TSF		BIT(1)
#define EQOS_MTL_TXQ0_OPERATION_MODE_FTQ		BIT(0)

#define EQOS_MTL_TXQ0_DEBUG_TXQSTS			BIT(4)
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT		1
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK			3

#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT		20
#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK		0x3ff
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT		14
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK		0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT		8
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK		0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_EHFC		BIT(7)
#define EQOS_MTL_RXQ0_OPERATION_MODE_RSF		BIT(5)

#define EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT			16
#define EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK			0x7fff
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT		4
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK			3

#define EQOS_DMA_REGS_BASE 0x1000
struct eqos_dma_regs {
	uint32_t mode;					/* 0x1000 */
	uint32_t sysbus_mode;				/* 0x1004 */
	uint32_t unused_1008[(0x1100 - 0x1008) / 4];	/* 0x1008 */
	uint32_t ch0_control;				/* 0x1100 */
	uint32_t ch0_tx_control;			/* 0x1104 */
	uint32_t ch0_rx_control;			/* 0x1108 */
	uint32_t unused_110c;				/* 0x110c */
	uint32_t ch0_txdesc_list_haddress;		/* 0x1110 */
	uint32_t ch0_txdesc_list_address;		/* 0x1114 */
	uint32_t ch0_rxdesc_list_haddress;		/* 0x1118 */
	uint32_t ch0_rxdesc_list_address;		/* 0x111c */
	uint32_t ch0_txdesc_tail_pointer;		/* 0x1120 */
	uint32_t unused_1124;				/* 0x1124 */
	uint32_t ch0_rxdesc_tail_pointer;		/* 0x1128 */
	uint32_t ch0_txdesc_ring_length;		/* 0x112c */
	uint32_t ch0_rxdesc_ring_length;		/* 0x1130 */
};

#define EQOS_DMA_MODE_SWR				BIT(0)

#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT		16
#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_MASK		0xf
#define EQOS_DMA_SYSBUS_MODE_EAME			BIT(11)
#define EQOS_DMA_SYSBUS_MODE_BLEN16			BIT(3)
#define EQOS_DMA_SYSBUS_MODE_BLEN8			BIT(2)
#define EQOS_DMA_SYSBUS_MODE_BLEN4			BIT(1)

#define EQOS_DMA_CH0_CONTROL_DSL_SHIFT			18
#define EQOS_DMA_CH0_CONTROL_PBLX8			BIT(16)

#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT		16
#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK		0x3f
#define EQOS_DMA_CH0_TX_CONTROL_OSP			BIT(4)
#define EQOS_DMA_CH0_TX_CONTROL_ST			BIT(0)

#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT		16
#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK		0x3f
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT		1
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK		0x3fff
#define EQOS_DMA_CH0_RX_CONTROL_SR			BIT(0)

/* These registers are Tegra186-specific */
#define EQOS_TEGRA186_REGS_BASE 0x8800
struct eqos_tegra186_regs {
	uint32_t sdmemcomppadctrl;			/* 0x8800 */
	uint32_t auto_cal_config;			/* 0x8804 */
	uint32_t unused_8808;				/* 0x8808 */
	uint32_t auto_cal_status;			/* 0x880c */
};

#define EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD	BIT(31)

#define EQOS_AUTO_CAL_CONFIG_START			BIT(31)
#define EQOS_AUTO_CAL_CONFIG_ENABLE			BIT(29)

#define EQOS_AUTO_CAL_STATUS_ACTIVE			BIT(31)

/* Descriptors */
#define EQOS_DESCRIPTORS_TX	4
#ifdef CONFIG_ARCH_SSTAR
#define EQOS_DESCRIPTORS_RX	64
#else
#define EQOS_DESCRIPTORS_RX	4
#endif
#define EQOS_DESCRIPTORS_NUM	(EQOS_DESCRIPTORS_TX + EQOS_DESCRIPTORS_RX)
#define EQOS_BUFFER_ALIGN	ARCH_DMA_MINALIGN
#define EQOS_MAX_PACKET_SIZE	ALIGN(1568, ARCH_DMA_MINALIGN)
#define EQOS_RX_BUFFER_SIZE	(EQOS_DESCRIPTORS_RX * EQOS_MAX_PACKET_SIZE)

struct eqos_desc {
	u32 des0;
	u32 des1;
	u32 des2;
	u32 des3;
};

#define EQOS_DESC3_OWN		BIT(31)
#define EQOS_DESC3_FD		BIT(29)
#define EQOS_DESC3_LD		BIT(28)
#define EQOS_DESC3_BUF1V	BIT(24)

#define EQOS_AXI_WIDTH_32	4
#define EQOS_AXI_WIDTH_64	8
#define EQOS_AXI_WIDTH_128	16

struct eqos_config {
	bool reg_access_always_ok;
	int mdio_wait;
	int swr_wait;
	int config_mac;
	int config_mac_mdio;
	unsigned int axi_bus_width;
	phy_interface_t (*interface)(struct udevice *dev);
	struct eqos_ops *ops;
};

struct eqos_ops {
	void (*eqos_inval_desc)(void *desc);
	void (*eqos_flush_desc)(void *desc);
	void (*eqos_inval_buffer)(void *buf, size_t size);
	void (*eqos_flush_buffer)(void *buf, size_t size);
	int (*eqos_probe_resources)(struct udevice *dev);
	int (*eqos_remove_resources)(struct udevice *dev);
	int (*eqos_stop_resets)(struct udevice *dev);
	int (*eqos_start_resets)(struct udevice *dev);
	int (*eqos_stop_clks)(struct udevice *dev);
	int (*eqos_start_clks)(struct udevice *dev);
	int (*eqos_calibrate_pads)(struct udevice *dev);
	int (*eqos_disable_calibration)(struct udevice *dev);
	int (*eqos_set_tx_clk_speed)(struct udevice *dev);
	ulong (*eqos_get_tick_clk_rate)(struct udevice *dev);
};

struct eqos_priv {
	struct udevice *dev;
	const struct eqos_config *config;
	fdt_addr_t regs;
	struct eqos_mac_regs *mac_regs;
	struct eqos_mtl_regs *mtl_regs;
	struct eqos_dma_regs *dma_regs;
	struct eqos_tegra186_regs *tegra186_regs;
	struct reset_ctl reset_ctl;
	struct gpio_desc phy_reset_gpio;
	struct clk clk_master_bus;
	struct clk clk_rx;
	struct clk clk_ptp_ref;
	struct clk clk_tx;
	struct clk clk_ck;
	struct clk clk_slave_bus;
	struct mii_dev *mii;
	struct phy_device *phy;
	u32 max_speed;
	void *descs;
	int tx_desc_idx, rx_desc_idx;
	unsigned int desc_size;
	void *tx_dma_buf;
	void *rx_dma_buf;
	void *rx_pkt;
	bool started;
	bool reg_access_ok;
	bool clk_ck_enabled;
#ifdef CONFIG_ARCH_SSTAR
	struct clk mclk;
	u8 mclk_freq;
	u8 mclk_refmode;
	u8 reset_io;
#endif
};

/*
 * TX and RX descriptors are 16 bytes. This causes problems with the cache
 * maintenance on CPUs where the cache-line size exceeds the size of these
 * descriptors. What will happen is that when the driver receives a packet
 * it will be immediately requeued for the hardware to reuse. The CPU will
 * therefore need to flush the cache-line containing the descriptor, which
 * will cause all other descriptors in the same cache-line to be flushed
 * along with it. If one of those descriptors had been written to by the
 * device those changes (and the associated packet) will be lost.
 *
 * To work around this, we make use of non-cached memory if available. If
 * descriptors are mapped uncached there's no need to manually flush them
 * or invalidate them.
 *
 * Note that this only applies to descriptors. The packet data buffers do
 * not have the same constraints since they are 1536 bytes large, so they
 * are unlikely to share cache-lines.
 */
static void *eqos_alloc_descs(struct eqos_priv *eqos, unsigned int num)
{
	eqos->desc_size = ALIGN(sizeof(struct eqos_desc),
				(unsigned int)ARCH_DMA_MINALIGN);

	return memalign(eqos->desc_size, num * eqos->desc_size);
}

static void eqos_free_descs(void *descs)
{
	free(descs);
}

static struct eqos_desc *eqos_get_desc(struct eqos_priv *eqos,
				       unsigned int num, bool rx)
{
	return eqos->descs +
		((rx ? EQOS_DESCRIPTORS_TX : 0) + num) * eqos->desc_size;
}

static void eqos_inval_desc_generic(void *desc)
{
	unsigned long start = (unsigned long)desc;
	unsigned long end = ALIGN(start + sizeof(struct eqos_desc),
				  ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void eqos_flush_desc_generic(void *desc)
{
	unsigned long start = (unsigned long)desc;
	unsigned long end = ALIGN(start + sizeof(struct eqos_desc),
				  ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

static void eqos_inval_buffer_tegra186(void *buf, size_t size)
{
	unsigned long start = (unsigned long)buf & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + size, ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void eqos_inval_buffer_generic(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
				    ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void eqos_flush_buffer_tegra186(void *buf, size_t size)
{
	flush_cache((unsigned long)buf, size);
}

static void eqos_flush_buffer_generic(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
				    ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

static int eqos_mdio_wait_idle(struct eqos_priv *eqos)
{
	return wait_for_bit_le32(&eqos->mac_regs->mdio_address,
				 EQOS_MAC_MDIO_ADDRESS_GB, false,
				 1000000, true);
}

static int eqos_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			  int mdio_reg)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d):\n", __func__, eqos->dev, mdio_addr,
	      mdio_reg);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(eqos->config->config_mac_mdio <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_READ <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(eqos->config->mdio_wait);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_data);
	val &= EQOS_MAC_MDIO_DATA_GD_MASK;

	debug("%s: val=%x\n", __func__, val);

	return val;
}

static int eqos_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			   int mdio_reg, u16 mdio_val)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d, val=%x):\n", __func__, eqos->dev,
	      mdio_addr, mdio_reg, mdio_val);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	writel(mdio_val, &eqos->mac_regs->mdio_data);

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(eqos->config->config_mac_mdio <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_WRITE <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(eqos->config->mdio_wait);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	return 0;
}

static int eqos_start_clks_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_slave_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_slave_bus) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d", ret);
		goto err_disable_clk_slave_bus;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_ptp_ref);
	if (ret < 0) {
		pr_err("clk_enable(clk_ptp_ref) failed: %d", ret);
		goto err_disable_clk_rx;
	}

	ret = clk_set_rate(&eqos->clk_ptp_ref, 125 * 1000 * 1000);
	if (ret < 0) {
		pr_err("clk_set_rate(clk_ptp_ref) failed: %d", ret);
		goto err_disable_clk_ptp_ref;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_disable_clk_ptp_ref;
	}
#endif

	debug("%s: OK\n", __func__);
	return 0;

#ifdef CONFIG_CLK
err_disable_clk_ptp_ref:
	clk_disable(&eqos->clk_ptp_ref);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err_disable_clk_slave_bus:
	clk_disable(&eqos->clk_slave_bus);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
#endif
}

static int eqos_start_clks_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_disable_clk_rx;
	}

	if (clk_valid(&eqos->clk_ck) && !eqos->clk_ck_enabled) {
		ret = clk_enable(&eqos->clk_ck);
		if (ret < 0) {
			pr_err("clk_enable(clk_ck) failed: %d", ret);
			goto err_disable_clk_tx;
		}
		eqos->clk_ck_enabled = true;
	}
#endif

	debug("%s: OK\n", __func__);
	return 0;

#ifdef CONFIG_CLK
err_disable_clk_tx:
	clk_disable(&eqos->clk_tx);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
#endif
}

static int eqos_stop_clks_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_ptp_ref);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);
	clk_disable(&eqos->clk_slave_bus);
#endif

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_clks_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);
#endif

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_start_resets_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
	if (ret < 0) {
		pr_err("dm_gpio_set_value(phy_reset, assert) failed: %d", ret);
		return ret;
	}

	udelay(2);

	ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
	if (ret < 0) {
		pr_err("dm_gpio_set_value(phy_reset, deassert) failed: %d", ret);
		return ret;
	}

	ret = reset_assert(&eqos->reset_ctl);
	if (ret < 0) {
		pr_err("reset_assert() failed: %d", ret);
		return ret;
	}

	udelay(2);

	ret = reset_deassert(&eqos->reset_ctl);
	if (ret < 0) {
		pr_err("reset_deassert() failed: %d", ret);
		return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_resets_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	reset_assert(&eqos->reset_ctl);
	dm_gpio_set_value(&eqos->phy_reset_gpio, 1);

	return 0;
}

static int eqos_calibrate_pads_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->tegra186_regs->sdmemcomppadctrl,
		     EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD);

	udelay(1);

	setbits_le32(&eqos->tegra186_regs->auto_cal_config,
		     EQOS_AUTO_CAL_CONFIG_START | EQOS_AUTO_CAL_CONFIG_ENABLE);

	ret = wait_for_bit_le32(&eqos->tegra186_regs->auto_cal_status,
				EQOS_AUTO_CAL_STATUS_ACTIVE, true, 10, false);
	if (ret) {
		pr_err("calibrate didn't start");
		goto failed;
	}

	ret = wait_for_bit_le32(&eqos->tegra186_regs->auto_cal_status,
				EQOS_AUTO_CAL_STATUS_ACTIVE, false, 10, false);
	if (ret) {
		pr_err("calibrate didn't finish");
		goto failed;
	}

	ret = 0;

failed:
	clrbits_le32(&eqos->tegra186_regs->sdmemcomppadctrl,
		     EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD);

	debug("%s: returns %d\n", __func__, ret);

	return ret;
}

static int eqos_disable_calibration_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->tegra186_regs->auto_cal_config,
		     EQOS_AUTO_CAL_CONFIG_ENABLE);

	return 0;
}

static ulong eqos_get_tick_clk_rate_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_slave_bus);
#else
	return 0;
#endif
}

static ulong eqos_get_tick_clk_rate_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_master_bus);
#else
	return 0;
#endif
}

__weak u32 imx_get_eqos_csr_clk(void)
{
	return 100 * 1000000;
}
__weak int imx_eqos_txclk_set_rate(unsigned long rate)
{
	return 0;
}

static ulong eqos_get_tick_clk_rate_imx(struct udevice *dev)
{
	return imx_get_eqos_csr_clk();
}

static int eqos_set_full_duplex(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	return 0;
}

static int eqos_set_half_duplex(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	/* WAR: Flush TX queue when switching to half-duplex */
	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_FTQ);

	return 0;
}

static int eqos_set_gmii_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_100(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_10(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_FES, EQOS_MAC_CONFIGURATION_PS);

	return 0;
}

static int eqos_set_tx_clk_speed_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
	ulong rate;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	switch (eqos->phy->speed) {
	case SPEED_1000:
		rate = 125 * 1000 * 1000;
		break;
	case SPEED_100:
		rate = 25 * 1000 * 1000;
		break;
	case SPEED_10:
		rate = 2.5 * 1000 * 1000;
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}

	ret = clk_set_rate(&eqos->clk_tx, rate);
	if (ret < 0) {
		pr_err("clk_set_rate(tx_clk, %lu) failed: %d", rate, ret);
		return ret;
	}
#endif

	return 0;
}

static int eqos_set_tx_clk_speed_imx(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	ulong rate;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	switch (eqos->phy->speed) {
	case SPEED_1000:
		rate = 125 * 1000 * 1000;
		break;
	case SPEED_100:
		rate = 25 * 1000 * 1000;
		break;
	case SPEED_10:
		rate = 2.5 * 1000 * 1000;
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}

	ret = imx_eqos_txclk_set_rate(rate);
	if (ret < 0) {
		pr_err("imx (tx_clk, %lu) failed: %d", rate, ret);
		return ret;
	}

	return 0;
}

static int eqos_adjust_link(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	bool en_calibration;

	debug("%s(dev=%p):\n", __func__, dev);

	if (eqos->phy->duplex)
		ret = eqos_set_full_duplex(dev);
	else
		ret = eqos_set_half_duplex(dev);
	if (ret < 0) {
		pr_err("eqos_set_*_duplex() failed: %d", ret);
		return ret;
	}

	switch (eqos->phy->speed) {
	case SPEED_1000:
		en_calibration = true;
		ret = eqos_set_gmii_speed(dev);
		break;
	case SPEED_100:
		en_calibration = true;
		ret = eqos_set_mii_speed_100(dev);
		break;
	case SPEED_10:
		en_calibration = false;
		ret = eqos_set_mii_speed_10(dev);
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}
	if (ret < 0) {
		pr_err("eqos_set_*mii_speed*() failed: %d", ret);
		return ret;
	}

	if (en_calibration) {
		ret = eqos->config->ops->eqos_calibrate_pads(dev);
		if (ret < 0) {
			pr_err("eqos_calibrate_pads() failed: %d",
			       ret);
			return ret;
		}
	} else {
		ret = eqos->config->ops->eqos_disable_calibration(dev);
		if (ret < 0) {
			pr_err("eqos_disable_calibration() failed: %d",
			       ret);
			return ret;
		}
	}
	ret = eqos->config->ops->eqos_set_tx_clk_speed(dev);
	if (ret < 0) {
		pr_err("eqos_set_tx_clk_speed() failed: %d", ret);
		return ret;
	}

	return 0;
}

static int eqos_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct eqos_priv *eqos = dev_get_priv(dev);
	uint32_t val;

	/*
	 * This function may be called before start() or after stop(). At that
	 * time, on at least some configurations of the EQoS HW, all clocks to
	 * the EQoS HW block will be stopped, and a reset signal applied. If
	 * any register access is attempted in this state, bus timeouts or CPU
	 * hangs may occur. This check prevents that.
	 *
	 * A simple solution to this problem would be to not implement
	 * write_hwaddr(), since start() always writes the MAC address into HW
	 * anyway. However, it is desirable to implement write_hwaddr() to
	 * support the case of SW that runs subsequent to U-Boot which expects
	 * the MAC address to already be programmed into the EQoS registers,
	 * which must happen irrespective of whether the U-Boot user (or
	 * scripts) actually made use of the EQoS device, and hence
	 * irrespective of whether start() was ever called.
	 *
	 * Note that this requirement by subsequent SW is not valid for
	 * Tegra186, and is likely not valid for any non-PCI instantiation of
	 * the EQoS HW block. This function is implemented solely as
	 * future-proofing with the expectation the driver will eventually be
	 * ported to some system where the expectation above is true.
	 */
	if (!eqos->config->reg_access_always_ok && !eqos->reg_access_ok)
		return 0;

	/* Update the MAC address */
	val = (plat->enetaddr[5] << 8) |
		(plat->enetaddr[4]);
	writel(val, &eqos->mac_regs->address0_high);
	val = (plat->enetaddr[3] << 24) |
		(plat->enetaddr[2] << 16) |
		(plat->enetaddr[1] << 8) |
		(plat->enetaddr[0]);
	writel(val, &eqos->mac_regs->address0_low);

	return 0;
}

static int eqos_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);

#ifdef CONFIG_ARCH_IMX8M
	imx_get_mac_from_fuse(dev_seq(dev), pdata->enetaddr);
#endif
	return !is_valid_ethaddr(pdata->enetaddr);
}

static int eqos_start(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret, i;
	ulong rate;
	u32 val, tx_fifo_sz, rx_fifo_sz, tqs, rqs, pbl;
	ulong last_rx_desc;
	ulong desc_pad;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->tx_desc_idx = 0;
	eqos->rx_desc_idx = 0;

	ret = eqos->config->ops->eqos_start_clks(dev);
	if (ret < 0) {
		pr_err("eqos_start_clks() failed: %d", ret);
		goto err;
	}

	ret = eqos->config->ops->eqos_start_resets(dev);
	if (ret < 0) {
		pr_err("eqos_start_resets() failed: %d", ret);
		goto err_stop_clks;
	}

	udelay(10);

	eqos->reg_access_ok = true;

	ret = wait_for_bit_le32(&eqos->dma_regs->mode,
				EQOS_DMA_MODE_SWR, false,
				eqos->config->swr_wait, false);
	if (ret) {
		pr_err("EQOS_DMA_MODE_SWR stuck");
		goto err_stop_resets;
	}

	ret = eqos->config->ops->eqos_calibrate_pads(dev);
	if (ret < 0) {
		pr_err("eqos_calibrate_pads() failed: %d", ret);
		goto err_stop_resets;
	}
	rate = eqos->config->ops->eqos_get_tick_clk_rate(dev);

	val = (rate / 1000000) - 1;
	writel(val, &eqos->mac_regs->us_tic_counter);

	/*
	 * if PHY was already connected and configured,
	 * don't need to reconnect/reconfigure again
	 */
	if (!eqos->phy) {
		int addr = -1;
#ifdef CONFIG_DM_ETH_PHY
		addr = eth_phy_get_addr(dev);
#endif
#ifdef DWC_NET_PHYADDR
		addr = DWC_NET_PHYADDR;
#endif
		eqos->phy = phy_connect(eqos->mii, addr, dev,
					eqos->config->interface(dev));
		if (!eqos->phy) {
			pr_err("phy_connect() failed");
			goto err_stop_resets;
		}

		if (eqos->max_speed) {
			ret = phy_set_supported(eqos->phy, eqos->max_speed);
			if (ret) {
				pr_err("phy_set_supported() failed: %d", ret);
				goto err_shutdown_phy;
			}
		}

		ret = phy_config(eqos->phy);
		if (ret < 0) {
			pr_err("phy_config() failed: %d", ret);
			goto err_shutdown_phy;
		}
	}

	ret = phy_startup(eqos->phy);
	if (ret < 0) {
#ifdef CONFIG_ARCH_SSTAR
		if(gGmacLoopback)
			goto force_adjust_link;
#endif
		pr_err("phy_startup() failed: %d", ret);
		goto err_shutdown_phy;
	}

	if (!eqos->phy->link) {
		pr_err("No link");
		goto err_shutdown_phy;
	}

#ifdef CONFIG_ARCH_SSTAR
force_adjust_link:
	if(gGmacLoopback) {
		eqos->phy->speed = gGmacLoopbackSpeed;
		eqos->phy->duplex = 1;
	}
#endif

	ret = eqos_adjust_link(dev);
	if (ret < 0) {
		pr_err("eqos_adjust_link() failed: %d", ret);
		goto err_shutdown_phy;
	}

	/* Configure MTL */

	/* Enable Store and Forward mode for TX */
	/* Program Tx operating mode */
	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_TSF |
		     (EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED <<
		      EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT));

	/* Transmit Queue weight */
	writel(0x10, &eqos->mtl_regs->txq0_quantum_weight);

	/* Enable Store and Forward mode for RX, since no jumbo frame */
	setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
		     EQOS_MTL_RXQ0_OPERATION_MODE_RSF);

	/* Transmit/Receive queue fifo size; use all RAM for 1 queue */
	val = readl(&eqos->mac_regs->hw_feature1);
	tx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK;
	rx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

	/*
	 * r/tx_fifo_sz is encoded as log2(n / 128). Undo that by shifting.
	 * r/tqs is encoded as (n / 256) - 1.
	 */
	tqs = (128 << tx_fifo_sz) / 256 - 1;
	rqs = (128 << rx_fifo_sz) / 256 - 1;

	clrsetbits_le32(&eqos->mtl_regs->txq0_operation_mode,
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK <<
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT,
			tqs << EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
	clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK <<
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT,
			rqs << EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT);

	/* Flow control used only if each channel gets 4KB or more FIFO */
	if (rqs >= ((4096 / 256) - 1)) {
		u32 rfd, rfa;

		setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			     EQOS_MTL_RXQ0_OPERATION_MODE_EHFC);

		/*
		 * Set Threshold for Activating Flow Contol space for min 2
		 * frames ie, (1500 * 1) = 1500 bytes.
		 *
		 * Set Threshold for Deactivating Flow Contol for space of
		 * min 1 frame (frame size 1500bytes) in receive fifo
		 */
		if (rqs == ((4096 / 256) - 1)) {
			/*
			 * This violates the above formula because of FIFO size
			 * limit therefore overflow may occur inspite of this.
			 */
			rfd = 0x3;	/* Full-3K */
			rfa = 0x1;	/* Full-1.5K */
		} else if (rqs == ((8192 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0xa;	/* Full-6K */
		} else if (rqs == ((16384 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x12;	/* Full-10K */
		} else {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x1E;	/* Full-16K */
		}

		clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT),
				(rfd <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(rfa <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT));
	}

	/* Configure MAC */

	clrsetbits_le32(&eqos->mac_regs->rxq_ctrl0,
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
			eqos->config->config_mac <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

	/* Multicast and Broadcast Queue Enable */
	setbits_le32(&eqos->mac_regs->unused_0a4,
		     0x00100000);
	/* enable promise mode */
	setbits_le32(&eqos->mac_regs->unused_004[1],
		     0x1);

	/* Set TX flow control parameters */
	/* Set Pause Time */
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     0xffff << EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);
	/* Assign priority for TX flow control */
	clrbits_le32(&eqos->mac_regs->txq_prty_map0,
		     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK <<
		     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT);
	/* Assign priority for RX flow control */
	clrbits_le32(&eqos->mac_regs->rxq_ctrl2,
		     EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK <<
		     EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT);
	/* Enable flow control */
#if defined (CONFIG_ARCH_SSTAR) && defined (CONFIG_SSTAR_GMAC_DISABLE_TX_FCTL)
	clrbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     EQOS_MAC_Q0_TX_FLOW_CTRL_TFE);
#else
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     EQOS_MAC_Q0_TX_FLOW_CTRL_TFE);
#endif
	setbits_le32(&eqos->mac_regs->rx_flow_ctrl,
		     EQOS_MAC_RX_FLOW_CTRL_RFE);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_GPSLCE |
			EQOS_MAC_CONFIGURATION_WD |
			EQOS_MAC_CONFIGURATION_JD |
			EQOS_MAC_CONFIGURATION_JE,
			EQOS_MAC_CONFIGURATION_CST |
			EQOS_MAC_CONFIGURATION_ACS);

	eqos_write_hwaddr(dev);

	/* Configure DMA */

	/* Enable OSP mode */
	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_OSP);

	/* RX buffer size. Must be a multiple of bus width */
	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT,
			EQOS_MAX_PACKET_SIZE <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT);

	desc_pad = (eqos->desc_size - sizeof(struct eqos_desc)) /
		   eqos->config->axi_bus_width;

	setbits_le32(&eqos->dma_regs->ch0_control,
		     EQOS_DMA_CH0_CONTROL_PBLX8 |
		     (desc_pad << EQOS_DMA_CH0_CONTROL_DSL_SHIFT));

	/*
	 * Burst length must be < 1/2 FIFO size.
	 * FIFO size in tqs is encoded as (n / 256) - 1.
	 * Each burst is n * 8 (PBLX8) * 16 (AXI width) == 128 bytes.
	 * Half of n * 256 is n * 128, so pbl == tqs, modulo the -1.
	 */
	pbl = tqs + 1;
	if (pbl > 32)
		pbl = 32;
	clrsetbits_le32(&eqos->dma_regs->ch0_tx_control,
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK <<
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT,
			pbl << EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT,
			8 << EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT);

	/* DMA performance configuration */
	val = (2 << EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
		EQOS_DMA_SYSBUS_MODE_EAME | EQOS_DMA_SYSBUS_MODE_BLEN16 |
		EQOS_DMA_SYSBUS_MODE_BLEN8 | EQOS_DMA_SYSBUS_MODE_BLEN4;
	writel(val, &eqos->dma_regs->sysbus_mode);

	/* Set up descriptors */

	memset(eqos->descs, 0, eqos->desc_size * EQOS_DESCRIPTORS_NUM);

	for (i = 0; i < EQOS_DESCRIPTORS_TX; i++) {
		struct eqos_desc *tx_desc = eqos_get_desc(eqos, i, false);
		eqos->config->ops->eqos_flush_desc(tx_desc);
	}

	for (i = 0; i < EQOS_DESCRIPTORS_RX; i++) {
		struct eqos_desc *rx_desc = eqos_get_desc(eqos, i, true);
#ifdef CONFIG_ARCH_SSTAR
		rx_desc->des0 = PA2BUS((u32)(ulong)(eqos->rx_dma_buf +
					     (i * EQOS_MAX_PACKET_SIZE)));
#else
		rx_desc->des0 = (u32)(ulong)(eqos->rx_dma_buf +
					     (i * EQOS_MAX_PACKET_SIZE));
#endif
		rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
		mb();
		eqos->config->ops->eqos_flush_desc(rx_desc);
		eqos->config->ops->eqos_inval_buffer(eqos->rx_dma_buf +
						(i * EQOS_MAX_PACKET_SIZE),
						EQOS_MAX_PACKET_SIZE);
	}

	writel(0, &eqos->dma_regs->ch0_txdesc_list_haddress);
#ifdef CONFIG_ARCH_SSTAR
	writel(PA2BUS((ulong)eqos_get_desc(eqos, 0, false)),
		&eqos->dma_regs->ch0_txdesc_list_address);
#else
	writel((ulong)eqos_get_desc(eqos, 0, false),
		&eqos->dma_regs->ch0_txdesc_list_address);
#endif
	writel(EQOS_DESCRIPTORS_TX - 1,
	       &eqos->dma_regs->ch0_txdesc_ring_length);

	writel(0, &eqos->dma_regs->ch0_rxdesc_list_haddress);
#ifdef CONFIG_ARCH_SSTAR
	writel(PA2BUS((ulong)eqos_get_desc(eqos, 0, true)),
		&eqos->dma_regs->ch0_rxdesc_list_address);
#else
	writel((ulong)eqos_get_desc(eqos, 0, true),
		&eqos->dma_regs->ch0_rxdesc_list_address);
#endif
	writel(EQOS_DESCRIPTORS_RX - 1,
	       &eqos->dma_regs->ch0_rxdesc_ring_length);

#ifdef CONFIG_ARCH_SSTAR
	if(gGmacLoopback == 2)
		setbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_LM);

	clrbits_le32(&eqos->mtl_regs->rxq0_operation_mode,BIT(4));
#endif
	/* Enable everything */
	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);
	setbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);
	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	/* TX tail pointer not written until we need to TX a packet */
	/*
	 * Point RX tail pointer at last descriptor. Ideally, we'd point at the
	 * first descriptor, implying all descriptors were available. However,
	 * that's not distinguishable from none of the descriptors being
	 * available.
	 */
	last_rx_desc = (ulong)eqos_get_desc(eqos, EQOS_DESCRIPTORS_RX - 1, true);
#ifdef CONFIG_ARCH_SSTAR
	writel(PA2BUS(last_rx_desc), &eqos->dma_regs->ch0_rxdesc_tail_pointer);
#else
	writel(last_rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);
#endif
	eqos->started = true;
#ifdef CONFIG_ARCH_SSTAR
#if (DYN_PHASE_CALB)
	if(NEED_CALB)
		sstar_gmac_do_dyncalibrat(eqos->phy, eqos->regs, eqos->config->interface(dev), eqos->phy->speed);
#endif
	if(gGmacLoopback)
		sstar_gmac_loopback_test(dev, eqos->mii, 0, eqos->regs, gGmacLoopbackLen, gGmacLoopbackSpeed);
	if(gGmacEPF)
		setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,BIT(4));
#endif
	debug("%s: OK\n", __func__);
	return 0;

err_shutdown_phy:
	phy_shutdown(eqos->phy);
err_stop_resets:
	eqos->config->ops->eqos_stop_resets(dev);
err_stop_clks:
	eqos->config->ops->eqos_stop_clks(dev);
err:
	pr_err("FAILED: %d", ret);
	return ret;
}

static void eqos_stop(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int i;

	debug("%s(dev=%p):\n", __func__, dev);

	if (!eqos->started)
		return;
	eqos->started = false;
	eqos->reg_access_ok = false;

	/* Disable TX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);

	/* Wait for TX all packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->txq0_debug);
		u32 trcsts = (val >> EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
			EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK;
		u32 txqsts = val & EQOS_MTL_TXQ0_DEBUG_TXQSTS;
		if ((trcsts != 1) && (!txqsts))
			break;
	}

	/* Turn off MAC TX and RX */
	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	/* Wait for all RX packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->rxq0_debug);
		u32 prxq = (val >> EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK;
		u32 rxqsts = (val >> EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK;
		if ((!prxq) && (!rxqsts))
			break;
	}

	/* Turn off RX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);

	if (eqos->phy) {
		phy_shutdown(eqos->phy);
	}
	eqos->config->ops->eqos_stop_resets(dev);
	eqos->config->ops->eqos_stop_clks(dev);

	debug("%s: OK\n", __func__);
}

static int eqos_send(struct udevice *dev, void *packet, int length)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eqos_desc *tx_desc;
	int i;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);

	memcpy(eqos->tx_dma_buf, packet, length);
	eqos->config->ops->eqos_flush_buffer(eqos->tx_dma_buf, length);

	tx_desc = eqos_get_desc(eqos, eqos->tx_desc_idx, false);
	eqos->tx_desc_idx++;
	eqos->tx_desc_idx %= EQOS_DESCRIPTORS_TX;
#ifdef CONFIG_ARCH_SSTAR
	tx_desc->des0 = PA2BUS((ulong)eqos->tx_dma_buf);
#else
	tx_desc->des0 = (ulong)eqos->tx_dma_buf;
#endif
	tx_desc->des1 = 0;
	tx_desc->des2 = length;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	tx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_FD | EQOS_DESC3_LD | length;
	eqos->config->ops->eqos_flush_desc(tx_desc);
#ifdef CONFIG_ARCH_SSTAR
	writel(PA2BUS((ulong)eqos_get_desc(eqos, eqos->tx_desc_idx, false)),
		&eqos->dma_regs->ch0_txdesc_tail_pointer);
	if(gGmacDumpTx)
		sstar_gmac_dump_packet(packet, length, 0);
#else
	writel((ulong)eqos_get_desc(eqos, eqos->tx_desc_idx, false),
		&eqos->dma_regs->ch0_txdesc_tail_pointer);
#endif
	for (i = 0; i < 1000000; i++) {
		eqos->config->ops->eqos_inval_desc(tx_desc);
		if (!(readl(&tx_desc->des3) & EQOS_DESC3_OWN))
			return 0;
		udelay(1);
	}

	debug("%s: TX timeout\n", __func__);

	return -ETIMEDOUT;
}

static int eqos_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eqos_desc *rx_desc;
	int length;

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);

	rx_desc = eqos_get_desc(eqos, eqos->rx_desc_idx, true);
	eqos->config->ops->eqos_inval_desc(rx_desc);
	if (rx_desc->des3 & EQOS_DESC3_OWN) {
		debug("%s: RX packet not available\n", __func__);
		return -EAGAIN;
	}

	*packetp = eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	length = rx_desc->des3 & 0x7fff;
	debug("%s: *packetp=%p, length=%d\n", __func__, *packetp, length);

	eqos->config->ops->eqos_inval_buffer(*packetp, length);

#ifdef CONFIG_ARCH_SSTAR
	if(gGmacDumpRx)
		sstar_gmac_dump_packet(*packetp, length, 1);
#endif

	return length;
}

static int eqos_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	uchar *packet_expected;
	struct eqos_desc *rx_desc;

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	packet_expected = eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	if (packet != packet_expected) {
		debug("%s: Unexpected packet (expected %p)\n", __func__,
		      packet_expected);
		return -EINVAL;
	}

	eqos->config->ops->eqos_inval_buffer(packet, length);

	rx_desc = eqos_get_desc(eqos, eqos->rx_desc_idx, true);

	rx_desc->des0 = 0;
	mb();
	eqos->config->ops->eqos_flush_desc(rx_desc);
	eqos->config->ops->eqos_inval_buffer(packet, length);
#ifdef CONFIG_ARCH_SSTAR
	rx_desc->des0 = PA2BUS((u32)(ulong)packet);
#else
	rx_desc->des0 = (u32)(ulong)packet;
#endif
	rx_desc->des1 = 0;
	rx_desc->des2 = 0;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
	eqos->config->ops->eqos_flush_desc(rx_desc);
#ifdef CONFIG_ARCH_SSTAR
	writel(PA2BUS((ulong)rx_desc), &eqos->dma_regs->ch0_rxdesc_tail_pointer);
#else
	writel((ulong)rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);
#endif
	eqos->rx_desc_idx++;
	eqos->rx_desc_idx %= EQOS_DESCRIPTORS_RX;

	return 0;
}

static int eqos_probe_resources_core(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->descs = eqos_alloc_descs(eqos, EQOS_DESCRIPTORS_NUM);
	if (!eqos->descs) {
		debug("%s: eqos_alloc_descs() failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	eqos->tx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_MAX_PACKET_SIZE);
	if (!eqos->tx_dma_buf) {
		debug("%s: memalign(tx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_descs;
	}
	debug("%s: tx_dma_buf=%p\n", __func__, eqos->tx_dma_buf);

	eqos->rx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_RX_BUFFER_SIZE);
	if (!eqos->rx_dma_buf) {
		debug("%s: memalign(rx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_dma_buf;
	}
	debug("%s: rx_dma_buf=%p\n", __func__, eqos->rx_dma_buf);

	eqos->rx_pkt = malloc(EQOS_MAX_PACKET_SIZE);
	if (!eqos->rx_pkt) {
		debug("%s: malloc(rx_pkt) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_rx_dma_buf;
	}
	debug("%s: rx_pkt=%p\n", __func__, eqos->rx_pkt);

	eqos->config->ops->eqos_inval_buffer(eqos->rx_dma_buf,
			EQOS_MAX_PACKET_SIZE * EQOS_DESCRIPTORS_RX);

	debug("%s: OK\n", __func__);
	return 0;

err_free_rx_dma_buf:
	free(eqos->rx_dma_buf);
err_free_tx_dma_buf:
	free(eqos->tx_dma_buf);
err_free_descs:
	eqos_free_descs(eqos->descs);
err:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove_resources_core(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	free(eqos->rx_pkt);
	free(eqos->rx_dma_buf);
	free(eqos->tx_dma_buf);
	eqos_free_descs(eqos->descs);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_probe_resources_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = reset_get_by_name(dev, "eqos", &eqos->reset_ctl);
	if (ret) {
		pr_err("reset_get_by_name(rst) failed: %d", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "phy-reset-gpios", 0,
				   &eqos->phy_reset_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		pr_err("gpio_request_by_name(phy reset) failed: %d", ret);
		goto err_free_reset_eqos;
	}

	ret = clk_get_by_name(dev, "slave_bus", &eqos->clk_slave_bus);
	if (ret) {
		pr_err("clk_get_by_name(slave_bus) failed: %d", ret);
		goto err_free_gpio_phy_reset;
	}

	ret = clk_get_by_name(dev, "master_bus", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(master_bus) failed: %d", ret);
		goto err_free_clk_slave_bus;
	}

	ret = clk_get_by_name(dev, "rx", &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(rx) failed: %d", ret);
		goto err_free_clk_master_bus;
	}

	ret = clk_get_by_name(dev, "ptp_ref", &eqos->clk_ptp_ref);
	if (ret) {
		pr_err("clk_get_by_name(ptp_ref) failed: %d", ret);
		goto err_free_clk_rx;
		return ret;
	}

	ret = clk_get_by_name(dev, "tx", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		goto err_free_clk_ptp_ref;
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_clk_ptp_ref:
	clk_free(&eqos->clk_ptp_ref);
err_free_clk_rx:
	clk_free(&eqos->clk_rx);
err_free_clk_master_bus:
	clk_free(&eqos->clk_master_bus);
err_free_clk_slave_bus:
	clk_free(&eqos->clk_slave_bus);
err_free_gpio_phy_reset:
	dm_gpio_free(dev, &eqos->phy_reset_gpio);
err_free_reset_eqos:
	reset_free(&eqos->reset_ctl);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

/* board-specific Ethernet Interface initializations. */
__weak int board_interface_eth_init(struct udevice *dev,
				    phy_interface_t interface_type)
{
	return 0;
}

static int eqos_probe_resources_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	phy_interface_t interface;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	ret = board_interface_eth_init(dev, interface);
	if (ret)
		return -EINVAL;

	eqos->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(master_bus) failed: %d", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-rx", &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(rx) failed: %d", ret);
		goto err_free_clk_master_bus;
	}

	ret = clk_get_by_name(dev, "mac-clk-tx", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		goto err_free_clk_rx;
	}

	/*  Get ETH_CLK clocks (optional) */
	ret = clk_get_by_name(dev, "eth-ck", &eqos->clk_ck);
	if (ret)
		pr_warn("No phy clock provided %d", ret);

	debug("%s: OK\n", __func__);
	return 0;

err_free_clk_rx:
	clk_free(&eqos->clk_rx);
err_free_clk_master_bus:
	clk_free(&eqos->clk_master_bus);
err_probe:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static phy_interface_t eqos_get_interface_stm32(struct udevice *dev)
{
	const char *phy_mode;
	phy_interface_t interface = PHY_INTERFACE_MODE_NONE;

	debug("%s(dev=%p):\n", __func__, dev);

	phy_mode = dev_read_prop(dev, "phy-mode", NULL);
	if (phy_mode)
		interface = phy_get_interface_by_name(phy_mode);

	return interface;
}

static phy_interface_t eqos_get_interface_tegra186(struct udevice *dev)
{
	return PHY_INTERFACE_MODE_MII;
}

static int eqos_probe_resources_imx(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static phy_interface_t eqos_get_interface_imx(struct udevice *dev)
{
	const char *phy_mode;
	phy_interface_t interface = PHY_INTERFACE_MODE_NONE;

	debug("%s(dev=%p):\n", __func__, dev);

	phy_mode = dev_read_prop(dev, "phy-mode", NULL);
	if (phy_mode)
		interface = phy_get_interface_by_name(phy_mode);

	return interface;
}

static int eqos_remove_resources_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

#ifdef CONFIG_CLK
	clk_free(&eqos->clk_tx);
	clk_free(&eqos->clk_ptp_ref);
	clk_free(&eqos->clk_rx);
	clk_free(&eqos->clk_slave_bus);
	clk_free(&eqos->clk_master_bus);
#endif
	dm_gpio_free(dev, &eqos->phy_reset_gpio);
	reset_free(&eqos->reset_ctl);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_remove_resources_stm32(struct udevice *dev)
{
#if defined(CONFIG_ARCH_SSTAR) || defined(CONFIG_CLK)
	struct eqos_priv *eqos = dev_get_priv(dev);
#endif
#ifdef CONFIG_CLK
	debug("%s(dev=%p):\n", __func__, dev);

	clk_free(&eqos->clk_tx);
	clk_free(&eqos->clk_rx);
	clk_free(&eqos->clk_master_bus);
	if (clk_valid(&eqos->clk_ck))
		clk_free(&eqos->clk_ck);
#endif

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio))
		dm_gpio_free(dev, &eqos->phy_reset_gpio);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_probe(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->dev = dev;
	eqos->config = (void *)dev_get_driver_data(dev);

	eqos->regs = dev_read_addr(dev);
	if (eqos->regs == FDT_ADDR_T_NONE) {
		pr_err("dev_read_addr() failed");
		return -ENODEV;
	}
	eqos->mac_regs = (void *)(eqos->regs + EQOS_MAC_REGS_BASE);
	eqos->mtl_regs = (void *)(eqos->regs + EQOS_MTL_REGS_BASE);
	eqos->dma_regs = (void *)(eqos->regs + EQOS_DMA_REGS_BASE);
	eqos->tegra186_regs = (void *)(eqos->regs + EQOS_TEGRA186_REGS_BASE);

	ret = eqos_probe_resources_core(dev);
	if (ret < 0) {
		pr_err("eqos_probe_resources_core() failed: %d", ret);
		return ret;
	}

	ret = eqos->config->ops->eqos_probe_resources(dev);
	if (ret < 0) {
		pr_err("eqos_probe_resources() failed: %d", ret);
		goto err_remove_resources_core;
	}

#ifdef CONFIG_DM_ETH_PHY
	eqos->mii = eth_phy_get_mdio_bus(dev);
#endif
	if (!eqos->mii) {
		eqos->mii = mdio_alloc();
		if (!eqos->mii) {
			pr_err("mdio_alloc() failed");
			ret = -ENOMEM;
			goto err_remove_resources_tegra;
		}
		eqos->mii->read = eqos_mdio_read;
		eqos->mii->write = eqos_mdio_write;
		eqos->mii->priv = eqos;
		strcpy(eqos->mii->name, dev->name);

		ret = mdio_register(eqos->mii);
		if (ret < 0) {
			pr_err("mdio_register() failed: %d", ret);
			goto err_free_mdio;
		}
	}

#ifdef CONFIG_DM_ETH_PHY
	eth_phy_set_mdio_bus(dev, eqos->mii);
#endif

	debug("%s: OK\n", __func__);
	return 0;

err_free_mdio:
	mdio_free(eqos->mii);
err_remove_resources_tegra:
	eqos->config->ops->eqos_remove_resources(dev);
err_remove_resources_core:
	eqos_remove_resources_core(dev);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	mdio_unregister(eqos->mii);
	mdio_free(eqos->mii);
	eqos->config->ops->eqos_remove_resources(dev);

	eqos_probe_resources_core(dev);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_null_ops(struct udevice *dev)
{
	return 0;
}

#ifdef CONFIG_ARCH_SSTAR
static int eqos_start_clks_sstar_gmac(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
#if CONFIG_SSTAR_CLK
	int ret;

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_clk_tx;
	}
#else
	sstar_gmac_start_clock(eqos->regs);
#endif

	debug("%s: OK\n", __func__);
	return 0;

#if CONFIG_SSTAR_CLK
err_clk_tx:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
#endif
}

static int eqos_stop_clks_sstar_gmac(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

#if (DYN_PHASE_CALB)
	if(NEED_CALB)
		return 0;
#endif

#ifdef CONFIG_SSTAR_CLK
	debug("%s(dev=%p):\n", __func__, dev);
	clk_disable(&eqos->clk_tx);
#else
	sstar_gmac_stop_clock(eqos->regs);
#endif

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_start_resets_sstar_gmac(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int value;

	debug("%s(dev=%p):\n", __func__, dev);

	//SW DMA RESET
	value = readl(&eqos->dma_regs->mode);
	value |= EQOS_DMA_MODE_SWR;
	writel(value,&eqos->dma_regs->mode);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_resets_sstar_gmac(struct udevice *dev)
{
	//struct eqos_priv *eqos = dev_get_priv(dev);

	//dm_gpio_set_value(&eqos->phy_reset_gpio, 1);

	return 0;
}

static ulong eqos_get_tick_clk_rate_sstar(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
#ifdef CONFIG_SSTAR_CLK
	return clk_get_rate(&eqos->clk_tx);
#else
	return sstar_gmac_get_rate(eqos->regs);
#endif
}

static int eqos_set_tx_clk_speed_sstar_gmac(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	ulong rate;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	switch (eqos->phy->speed) {
	case SPEED_1000:
		rate = 125 * 1000 * 1000;
		break;
	case SPEED_100:
		rate = 25 * 1000 * 1000;
		break;
	case SPEED_10:
		rate = 2.5 * 1000 * 1000;
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}

#if DYN_PHASE_CALB
	if(NEED_CALB)
		sstar_gmac_prepare_dyncalibrat(eqos->phy, eqos->regs, eqos->config->interface(dev), eqos->phy->speed);
#endif
#ifdef CONFIG_SSTAR_CLK
	ret = clk_set_rate(&eqos->clk_tx, rate);
#else
	ret = sstar_gmac_set_rate(eqos->regs, eqos->phy->speed);
#endif
	if (ret < 0) {
		pr_err("clk_set_rate(tx_clk, %lu) failed: %d", rate, ret);
		return ret;
	}

	sstar_gmac_set_tx_clk_pad_sel(eqos->regs, eqos->config->interface(dev), eqos->phy->speed);
	return 0;
}

static int eqos_probe_resources_sstar_gmac(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	phy_interface_t interface;
	const u8 *p;
	char *s, *e;
	int i;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	eqos->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	eqos->mclk_freq = 0;
	p = dev_read_u8_array_ptr(dev, "mclk", 1);
	if(p)
		eqos->mclk_freq = *p;

	eqos->mclk_refmode = 0;
	p = dev_read_u8_array_ptr(dev, "mclk-refmode", 1);
	if(p)
		eqos->mclk_refmode = *p;

	eqos->reset_io = 0xFF;
	p = dev_read_u8_array_ptr(dev, "reset-io", 1);
	if(p)
		eqos->reset_io = *p;

	if(eqos->regs == GMAC0_Base)
	{
		s = env_get ("ethaddr");
		if (s)
		{
			for (i = 0; i < sizeof(pdata->enetaddr); ++i)
			{
				pdata->enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
				if (s)
				{
					s = (*e) ? e + 1 : e;
				}
			}
		}
	}
	else
	{
		s = env_get ("eth1addr");
		if (s)
		{
			for (i = 0; i < sizeof(pdata->enetaddr); ++i)
			{
				pdata->enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
				if (s)
				{
					s = (*e) ? e + 1 : e;
				}
			}
		}
	}

	eqos->reg_access_ok = true;

	eqos_write_hwaddr(dev);

#if CONFIG_SSTAR_CLK
	ret = clk_get_by_name(dev, "gmac-clk", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(gmac-clk) failed: %d", ret);
		goto err_free_clk_tx;
	}

	if(eqos->mclk_freq) {
		ret = clk_get_by_name(dev, "mclk", &eqos->mclk);
		if (ret) {
			pr_err("clk_get_by_name(mclk) failed: %d", ret);
			goto err_free_clk_tx;
		}

		ret = clk_enable(&eqos->mclk);
		if (ret < 0) {
			pr_err("clk_enable(mclk) failed: %d", ret);
			goto err_free_clk_tx;
		}

		ret = clk_set_rate(&eqos->mclk, eqos->mclk_freq * 1000000);

		if (ret < 0) {
			pr_err("clk_set_rate(mclk, %uMHz) failed: %d", eqos->mclk_freq, ret);
			goto err_free_clk_tx;
		}
	}
#endif

	ret = sstar_gmac_probe(eqos->regs, interface, eqos->mclk_freq, eqos->mclk_refmode, eqos->reset_io);
	if (ret)
		return -EINVAL;

	debug("%s: OK\n", __func__);
	return 0;

#if CONFIG_SSTAR_CLK
err_free_clk_tx:
	debug("%s: returns %d\n", __func__, ret);
	return ret;
#endif
}

static phy_interface_t eqos_get_interface_sstar_gmac(struct udevice *dev)
{
	const char *phy_mode;
	phy_interface_t interface = PHY_INTERFACE_MODE_NONE;

	debug("%s(dev=%p):\n", __func__, dev);

	phy_mode = dev_read_prop(dev, "phy-mode", NULL);
	if (phy_mode)
		interface = phy_get_interface_by_name(phy_mode);

	return interface;
}

static int eqos_remove_resources_sstar_gmac(struct udevice *dev)
{
#ifdef CONFIG_SSTAR_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
#endif

#if 0
	if (dm_gpio_is_valid(&eqos->phy_reset_gpio))
		dm_gpio_free(dev, &eqos->phy_reset_gpio);
#endif
#ifdef CONFIG_SSTAR_CLK
	clk_free(&eqos->clk_tx);
	if(eqos->mclk_freq)
		clk_free(&eqos->mclk);
#endif

	debug("%s: OK\n", __func__);
	return 0;
}
#endif

static const struct eth_ops eqos_ops = {
	.start = eqos_start,
	.stop = eqos_stop,
	.send = eqos_send,
	.recv = eqos_recv,
	.free_pkt = eqos_free_pkt,
	.write_hwaddr = eqos_write_hwaddr,
	.read_rom_hwaddr	= eqos_read_rom_hwaddr,
};

static struct eqos_ops eqos_tegra186_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_tegra186,
	.eqos_flush_buffer = eqos_flush_buffer_tegra186,
	.eqos_probe_resources = eqos_probe_resources_tegra186,
	.eqos_remove_resources = eqos_remove_resources_tegra186,
	.eqos_stop_resets = eqos_stop_resets_tegra186,
	.eqos_start_resets = eqos_start_resets_tegra186,
	.eqos_stop_clks = eqos_stop_clks_tegra186,
	.eqos_start_clks = eqos_start_clks_tegra186,
	.eqos_calibrate_pads = eqos_calibrate_pads_tegra186,
	.eqos_disable_calibration = eqos_disable_calibration_tegra186,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_tegra186,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_tegra186
};

static const struct eqos_config __maybe_unused eqos_tegra186_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 10,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_20_35,
	.axi_bus_width = EQOS_AXI_WIDTH_128,
	.interface = eqos_get_interface_tegra186,
	.ops = &eqos_tegra186_ops
};

static struct eqos_ops eqos_stm32_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_stm32,
	.eqos_remove_resources = eqos_remove_resources_stm32,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_null_ops,
	.eqos_stop_clks = eqos_stop_clks_stm32,
	.eqos_start_clks = eqos_start_clks_stm32,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_stm32
};

static const struct eqos_config __maybe_unused eqos_stm32_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = eqos_get_interface_stm32,
	.ops = &eqos_stm32_ops
};

static struct eqos_ops eqos_imx_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_imx,
	.eqos_remove_resources = eqos_null_ops,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_null_ops,
	.eqos_stop_clks = eqos_null_ops,
	.eqos_start_clks = eqos_null_ops,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_imx,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_imx
};

struct eqos_config __maybe_unused eqos_imx_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = eqos_get_interface_imx,
	.ops = &eqos_imx_ops
};

#ifdef CONFIG_ARCH_SSTAR
static struct eqos_ops eqos_sstar_gmac_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_sstar_gmac,
	.eqos_remove_resources = eqos_remove_resources_sstar_gmac,
	.eqos_stop_resets = eqos_stop_resets_sstar_gmac,
	.eqos_start_resets = eqos_start_resets_sstar_gmac,
	.eqos_stop_clks = eqos_stop_clks_sstar_gmac,
	.eqos_start_clks = eqos_start_clks_sstar_gmac,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_sstar_gmac,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_sstar
};

static const struct eqos_config __maybe_unused eqos_sstar_gmac_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 10,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_500_800,
	.axi_bus_width = EQOS_AXI_WIDTH_128,
	.interface = eqos_get_interface_sstar_gmac,
	.ops = &eqos_sstar_gmac_ops
};
#endif

static const struct udevice_id eqos_ids[] = {
#ifdef CONFIG_ARCH_SSTAR
	{
		.compatible = "sstar,gmac",
		.data = (ulong)&eqos_sstar_gmac_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_TEGRA186)
	{
		.compatible = "nvidia,tegra186-eqos",
		.data = (ulong)&eqos_tegra186_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_STM32)
	{
		.compatible = "st,stm32mp1-dwmac",
		.data = (ulong)&eqos_stm32_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_IMX)
	{
		.compatible = "fsl,imx-eqos",
		.data = (ulong)&eqos_imx_config
	},
#endif

	{ }
};

U_BOOT_DRIVER(eth_eqos) = {
	.name = "eth_eqos",
	.id = UCLASS_ETH,
	.of_match = of_match_ptr(eqos_ids),
	.probe = eqos_probe,
	.remove = eqos_remove,
	.ops = &eqos_ops,
	.priv_auto	= sizeof(struct eqos_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
