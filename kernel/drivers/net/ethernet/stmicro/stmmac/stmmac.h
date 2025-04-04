/* SPDX-License-Identifier: GPL-2.0-only */
/*******************************************************************************
  Copyright (C) 2007-2009  STMicroelectronics Ltd


  Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
*******************************************************************************/

#ifndef __STMMAC_H__
#define __STMMAC_H__

#define STMMAC_RESOURCE_NAME   "stmmaceth"
#define DRV_MODULE_VERSION	"Jan_2016"

#include <linux/clk.h>
#include <linux/if_vlan.h>
#include <linux/stmmac.h>
#include <linux/phylink.h>
#include <linux/pci.h>
#include "common.h"
#include <linux/ptp_clock_kernel.h>
#include <linux/net_tstamp.h>
#include <linux/reset.h>
#include <net/page_pool.h>

#ifdef CONFIG_ARCH_SSTAR
#define GMAC_USING_MIU_MAPPING_API     1
#define SSTAR_GMAC_MAC_ADDR_NUM        32

#if GMAC_USING_MIU_MAPPING_API
#ifdef CONFIG_ARCH_SSTAR
extern u64 Chip_Phys_to_MIU(u64 phys);
extern u64 Chip_MIU_to_Phys(u64 miu);
#endif
#define PA2BUS(a) Chip_Phys_to_MIU(a)
#define BUS2PA(a) Chip_MIU_to_Phys(a)
#else
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
#define MIU_4G 0x100000000L
#define PA2BUS(a) ((a >= MIU_4G) ? ((a) - (MIU0_HIGH_BUS_BASE)) : ((a) - (MIU0_BUS_BASE)))
#define BUS2PA(a) ((a >= MIU_4G) ? ((a) + (MIU0_HIGH_BUS_BASE)) : ((a) + (MIU0_BUS_BASE)))
#else
#define PA2BUS(a) ((a) - (MIU0_BUS_BASE))
#define BUS2PA(a) ((a) + (MIU0_BUS_BASE))
#endif
#endif
#endif /* CONFIG_ARCH_SSTAR */

#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_RXIC)
#define SSTAR_SNPS_GMAC_RX_INT_HRTIMER 1
#if SSTAR_SNPS_GMAC_RX_INT_HRTIMER
	#define RX_TIMER_S					struct hrtimer
	#define RX_TIMER_INIT(tmr, fn)		{ hrtimer_init((tmr), CLOCK_MONOTONIC, HRTIMER_MODE_REL); (tmr)->function = fn; }
	// #define RX_TIMER_MOD(tmr, us)		hrtimer_forward_now((tmr), ns_to_ktime((us)*1000))
	#define RX_TIMER_MOD(tmr, us)		hrtimer_start((tmr), ns_to_ktime((us)*1000), HRTIMER_MODE_REL)
	#define RX_TIMER_ACTIVE(tmr)		hrtimer_active((tmr))
	#define RX_TIMER_DEL(tmr)			hrtimer_cancel((tmr))
	#define RX_TIMER_DEL_SYNC(tmr)		hrtimer_cancel((tmr))
#else
	#define RX_TIMER_S					struct timer_list
	#define RX_TIMER_INIT(tmr, fn) 		timer_setup((tmr), (fn), 0)
	#define RX_TIMER_MOD(tmr, us)		mod_timer((tmr), usecs_to_jiffies(us))
	#define RX_TIMER_ACTIVE(tmr)		timer_pending((tmr))
	#define RX_TIMER_DEL(tmr)			del_timer((tmr))
	#define RX_TIMER_DEL_SYNC(tmr)		del_timer_sync((tmr))
#endif

#define SSTAR_NAPI_WEIGHT_RX			128
#define SSTAR_NAPI_WEIGHT_TX			NAPI_POLL_WEIGHT
#endif /* CONFIG_ARCH_SSTAR && CONFIG_SSTAR_SNPS_GMAC_RXIC */

struct stmmac_resources {
	void __iomem *addr;
	const char *mac;
	int wol_irq;
	int lpi_irq;
	int irq;
};

struct stmmac_tx_info {
	dma_addr_t buf;
	bool map_as_page;
	unsigned len;
	bool last_segment;
	bool is_jumbo;
};

#define STMMAC_TBS_AVAIL	BIT(0)
#define STMMAC_TBS_EN		BIT(1)

/* Frequently used values are kept adjacent for cache effect */
struct stmmac_tx_queue {
	u32 tx_count_frames;
	int tbs;
	struct timer_list txtimer;
	u32 queue_index;
	struct stmmac_priv *priv_data;
	struct dma_extended_desc *dma_etx ____cacheline_aligned_in_smp;
	struct dma_edesc *dma_entx;
	struct dma_desc *dma_tx;
#if defined(CONFIG_ARCH_SSTAR) && defined (CONFIG_SSTAR_GMAC_BACKUP_DESC)
	struct dma_desc *dma_tx2;
#endif
	struct sk_buff **tx_skbuff;
	struct stmmac_tx_info *tx_skbuff_dma;
	unsigned int cur_tx;
	unsigned int dirty_tx;
	dma_addr_t dma_tx_phy;
#if defined(CONFIG_ARCH_SSTAR) && defined (CONFIG_SSTAR_GMAC_BACKUP_DESC)
	dma_addr_t dma_tx_phy2;
#endif
	u32 tx_tail_addr;
	u32 mss;
};

#if defined(CONFIG_ARCH_SSTAR) && !defined(CONFIG_SSTAR_SNPS_GMAC_RX_ZERO_COPY)
struct stmmac_rx_buffer {
	struct page *page;
	struct page *sec_page;
	dma_addr_t addr;
	dma_addr_t sec_addr;
};
#endif

struct stmmac_rx_queue {
	u32 rx_count_frames;
	u32 queue_index;
	struct page_pool *page_pool;
#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_RX_ZERO_COPY)
	struct sk_buff **rx_skbuff;
	dma_addr_t *rx_skbuff_dma;
#else
	struct stmmac_rx_buffer *buf_pool;
#endif
	struct stmmac_priv *priv_data;
	struct dma_extended_desc *dma_erx;
	struct dma_desc *dma_rx ____cacheline_aligned_in_smp;
#if defined(CONFIG_ARCH_SSTAR) && defined (CONFIG_SSTAR_GMAC_BACKUP_DESC)
	struct dma_desc *dma_rx2 ____cacheline_aligned_in_smp;
#endif
	unsigned int cur_rx;
	unsigned int dirty_rx;
	u32 rx_zeroc_thresh;
	dma_addr_t dma_rx_phy;
#if defined(CONFIG_ARCH_SSTAR) && defined (CONFIG_SSTAR_GMAC_BACKUP_DESC)
	dma_addr_t dma_rx_phy2;
#endif
	u32 rx_tail_addr;
	unsigned int state_saved;
	struct {
		struct sk_buff *skb;
		unsigned int len;
		unsigned int error;
	} state;
#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_RXIC)
	#define RXIC_MSK_BITS_MAX		(7)
	#define RXIC_MSK_GET(x)			((1 << (x)) - 1)
	#define RXIC_IOC_EN(i, msk)		(!((i) & (msk)))

	int rxic_msk_bit;
	unsigned int rxic_msk;
	RX_TIMER_S rxic_tmr;
	unsigned long rxic_tmr_pd_us;

	unsigned long jiffies_last;
	unsigned int rxic_pkt_cnt;
#endif
};

struct stmmac_channel {
	struct napi_struct rx_napi ____cacheline_aligned_in_smp;
	struct napi_struct tx_napi ____cacheline_aligned_in_smp;
	struct stmmac_priv *priv_data;
	spinlock_t lock;
	u32 index;
};

struct stmmac_tc_entry {
	bool in_use;
	bool in_hw;
	bool is_last;
	bool is_frag;
	void *frag_ptr;
	unsigned int table_pos;
	u32 handle;
	u32 prio;
	struct {
		u32 match_data;
		u32 match_en;
		u8 af:1;
		u8 rf:1;
		u8 im:1;
		u8 nc:1;
		u8 res1:4;
		u8 frame_offset;
		u8 ok_index;
		u8 dma_ch_no;
		u32 res2;
	} __packed val;
};

#define STMMAC_PPS_MAX		4
struct stmmac_pps_cfg {
	bool available;
	struct timespec64 start;
	struct timespec64 period;
};

struct stmmac_rss {
	int enable;
	u8 key[STMMAC_RSS_HASH_KEY_SIZE];
	u32 table[STMMAC_RSS_MAX_TABLE_SIZE];
};

#define STMMAC_FLOW_ACTION_DROP		BIT(0)
struct stmmac_flow_entry {
	unsigned long cookie;
	unsigned long action;
	u8 ip_proto;
	int in_use;
	int idx;
	int is_l4;
};

struct stmmac_priv {
	/* Frequently used values are kept adjacent for cache effect */
	u32 tx_coal_frames;
	u32 tx_coal_timer;
	u32 rx_coal_frames;

	int tx_coalesce;
	int hwts_tx_en;
	bool tx_path_in_lpi_mode;
	bool tso;
	int sph;
	u32 sarc_type;

	unsigned int dma_buf_sz;
	unsigned int rx_copybreak;
	u32 rx_riwt;
	int hwts_rx_en;

	void __iomem *ioaddr;
	struct net_device *dev;
	struct device *device;
	struct mac_device_info *hw;
	int (*hwif_quirks)(struct stmmac_priv *priv);
	struct mutex lock;

	/* RX Queue */
	struct stmmac_rx_queue rx_queue[MTL_MAX_RX_QUEUES];
	unsigned int dma_rx_size;
#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_CODING_OPTIMIZE)
	unsigned int dma_rx_size_msk;
#endif

	/* TX Queue */
	struct stmmac_tx_queue tx_queue[MTL_MAX_TX_QUEUES];
	unsigned int dma_tx_size;
#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_CODING_OPTIMIZE)
	unsigned int dma_tx_size_msk;
#endif

	/* Generic channel for NAPI */
	struct stmmac_channel channel[STMMAC_CH_MAX];

	int speed;
	unsigned int flow_ctrl;
	unsigned int pause;
	struct mii_bus *mii;
	int mii_irq[PHY_MAX_ADDR];

	struct phylink_config phylink_config;
	struct phylink *phylink;

	struct stmmac_extra_stats xstats ____cacheline_aligned_in_smp;
	struct stmmac_safety_stats sstats;
	struct plat_stmmacenet_data *plat;
	struct dma_features dma_cap;
	struct stmmac_counters mmc;
	int hw_cap_support;
	int synopsys_id;
	u32 msg_enable;
	int wolopts;
	int wol_irq;
	int clk_csr;
	struct timer_list eee_ctrl_timer;
	int lpi_irq;
	int eee_enabled;
	int eee_active;
	int tx_lpi_timer;
	int tx_lpi_enabled;
	int eee_tw_timer;
#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_EEE_REDUCTION)
	int eee_force_disable;
#endif
	unsigned int mode;
	unsigned int chain_mode;
	int extend_desc;
	struct hwtstamp_config tstamp_config;
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info ptp_clock_ops;
	unsigned int default_addend;
	u32 sub_second_inc;
	u32 systime_flags;
	u32 adv_ts;
	int use_riwt;
	int irq_wake;
	spinlock_t ptp_lock;
	void __iomem *mmcaddr;
	void __iomem *ptpaddr;
	unsigned long active_vlans[BITS_TO_LONGS(VLAN_N_VID)];

#ifdef CONFIG_DEBUG_FS
	struct dentry *dbgfs_dir;
#endif

	unsigned long state;
	struct workqueue_struct *wq;
	struct work_struct service_task;

	/* TC Handling */
	unsigned int tc_entries_max;
	unsigned int tc_off_max;
	struct stmmac_tc_entry *tc_entries;
	unsigned int flow_entries_max;
	struct stmmac_flow_entry *flow_entries;

	/* Pulse Per Second output */
	struct stmmac_pps_cfg pps[STMMAC_PPS_MAX];

	/* Receive Side Scaling */
	struct stmmac_rss rss;

#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_RXIC)
	int rxic_msk_bit_max;
	int rxic_msk_bit_max_cur;
#endif

#if defined(CONFIG_ARCH_SSTAR) && defined(CONFIG_SSTAR_SNPS_GMAC_STORM_PROTECT)
	struct timer_list storm_prct_tmr;
#endif

};

enum stmmac_state {
	STMMAC_DOWN,
	STMMAC_RESET_REQUESTED,
	STMMAC_RESETING,
	STMMAC_SERVICE_SCHED,
};

int stmmac_mdio_unregister(struct net_device *ndev);
int stmmac_mdio_register(struct net_device *ndev);
int stmmac_mdio_reset(struct mii_bus *mii);
void stmmac_set_ethtool_ops(struct net_device *netdev);

int stmmac_init_tstamp_counter(struct stmmac_priv *priv, u32 systime_flags);
void stmmac_ptp_register(struct stmmac_priv *priv);
void stmmac_ptp_unregister(struct stmmac_priv *priv);
int stmmac_resume(struct device *dev);
int stmmac_suspend(struct device *dev);
int stmmac_dvr_remove(struct device *dev);
int stmmac_dvr_probe(struct device *device,
		     struct plat_stmmacenet_data *plat_dat,
		     struct stmmac_resources *res);
void stmmac_disable_eee_mode(struct stmmac_priv *priv);
bool stmmac_eee_init(struct stmmac_priv *priv);
int stmmac_reinit_queues(struct net_device *dev, u32 rx_cnt, u32 tx_cnt);
int stmmac_reinit_ringparam(struct net_device *dev, u32 rx_size, u32 tx_size);
int stmmac_bus_clks_config(struct stmmac_priv *priv, bool enabled);

#if IS_ENABLED(CONFIG_STMMAC_SELFTESTS)
void stmmac_selftest_run(struct net_device *dev,
			 struct ethtool_test *etest, u64 *buf);
void stmmac_selftest_get_strings(struct stmmac_priv *priv, u8 *data);
int stmmac_selftest_get_count(struct stmmac_priv *priv);
#else
static inline void stmmac_selftest_run(struct net_device *dev,
				       struct ethtool_test *etest, u64 *buf)
{
	/* Not enabled */
}
static inline void stmmac_selftest_get_strings(struct stmmac_priv *priv,
					       u8 *data)
{
	/* Not enabled */
}
static inline int stmmac_selftest_get_count(struct stmmac_priv *priv)
{
	return -EOPNOTSUPP;
}
#endif /* CONFIG_STMMAC_SELFTESTS */

#endif /* __STMMAC_H__ */
