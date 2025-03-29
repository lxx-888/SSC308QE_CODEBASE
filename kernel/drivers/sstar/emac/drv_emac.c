/*
 * drv_emac.c- Sigmastar
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
//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
//#include <linux/autoconf.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/clk-provider.h>

#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
#include <asm/prom.h>
#include <asm/mach/map.h>
#endif

#include "mdrv_types.h"
//#include "mst_platform.h"
//#include "mdrv_system.h"
//#include "chip_int.h"
#include "ms_msys.h"
#include "hal_emac.h"
#include "drv_emac.h"
#include "ms_platform.h"
#include "registers.h"

#ifdef CONFIG_EMAC_SUPPLY_RNG
#include <linux/input.h>
#include <random.h>
#include "hal_rng_reg.h"
#endif

// #include "mdrv_msys_io_st.h"
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/phy.h>
#include <linux/rtnetlink.h>

#include "gpio.h"
#ifdef CONFIG_SSTAR_PADMUX
#include "drv_padmux.h"
#include "drv_puse.h"
#endif

#include "cam_os_wrapper.h"
#include "drv_camclk_Api.h"
#ifdef CONFIG_SSTAR_GPIO
extern void sstar_gpio_set_low(U8 gpio_index);
extern void sstar_gpio_set_high(U8 gpio_index);
extern void sstar_gpio_pad_set(U8 gpio_index);
#else
#define sstar_gpio_set_low(x)
#define sstar_gpio_set_high(x)
#define sstar_gpio_pad_set(x)
#endif

/////////////////////////////////
// to be refined
/////////////////////////////////
#define TXD_NUM 0
// #define TXQ_NUM_SW      256
#define TXQ_NUM_SW 0

#define RX_DESC_API 0

#define CPU_AFFINITY 0

#if EXT_PHY_PATCH
#define IS_EXT_PHY(hemac) (0 == (hemac)->phyRIU)
#endif

#ifndef TX_DELAY_INT
#define TX_DELAY_INT 0
#endif

#ifndef TX_DELAY_INT_DEBUG
#define TX_DELAY_INT_DEBUG 0
#endif

#ifndef HW_FLOW_CONTROL
#define HW_FLOW_CONTROL 0
#endif

#ifndef ETHTOOL_DEBUG
#define ETHTOOL_DEBUG 0
#endif

#ifndef NETWORK_STORM_PROTECT
#define NETWORK_STORM_PROTECT 0
#endif

#ifndef PHY_REGISTER_DEBUG
#define PHY_REGISTER_DEBUG 0
#endif

#ifndef PHY_LED_CTL
#define PHY_LED_CTL 0
#endif

#ifndef PHY_LED_INVERT
#define PHY_LED_INVERT 0
#endif

#ifndef ICPLUS_STR_FIX
#define ICPLUS_STR_FIX 0
#endif

//--------------------------------------------------------------------------------------------------
//  helper definition
//--------------------------------------------------------------------------------------------------
#define CLR_BITS(a, bits) ((a) & (~(bits)))
#define SET_BITS(a, bits) ((a) | (bits))

#define PA2BUS(a) Chip_Phys_to_MIU(a)
#define BUS2PA(a) Chip_MIU_to_Phys(a)

/* Note: The flowing three interfaces are not available at 64-bit */
#define BUS2VIRT(a) phys_to_virt(BUS2PA((a)))
#define VIRT2BUS(a) PA2BUS(virt_to_phys((a)))

#define VIRT2PA(a) virt_to_phys((a))

#if RX_DESC_API
#define RX_DESC_MAKE(desc, bus, wrap)                                                         \
    {                                                                                         \
        if ((bus)&0x3)                                                                        \
            printk("[%s][%d] bad RX buffer address 0x%08x\n", __FUNCTION__, __LINE__, (bus)); \
        ((desc)->addr = (((bus) << 2) & 0xFFFFFFFC) | (wrap));                                \
    }
#define RX_ADDR_GET(desc) BUS2VIRT(((((desc)->addr) & 0xFFFFFFFC) >> 2))
#endif

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#if SSTAR_EMAC_NAPI
#define EMAC_NAPI_WEIGHT 32
#endif

#if defined(CONFIG_SS_EMAC_RX_DESC_NUM)
#define RX_DESC_NUM CONFIG_SS_EMAC_RX_DESC_NUM
#else
#define RX_DESC_NUM 0x100
#endif
#define RX_DESC_SIZE       (sizeof(struct rbf_t))
#define RX_DESC_QUEUE_SIZE (RX_DESC_NUM * RX_DESC_SIZE)

#define EMAC_PACKET_SIZE_MAX 0x600

#if EMAC_SG
#define FEATURES_EMAC_SG (NETIF_F_SG)
#else
#define FEATURES_EMAC_SG (0)
#endif

#if (EMAC_GSO)
#define FEATURES_EMAC_GSO ((NETIF_F_GSO) | (NETIF_F_GRO))
#else
#define FEATURES_EMAC_GSO (0)
#endif

#if RX_CHECKSUM
#define FEATURES_EMAC_CSUM_RX (NETIF_F_RXCSUM)
#else
#define FEATURES_EMAC_CSUM_RX (0)
#endif

#if TX_CHECKSUM
#define FEATURES_EMAC_CSUM_TX (NETIF_F_HW_CSUM)
#else
#define FEATURES_EMAC_CSUM_TX (0)
#endif

#define EMAC_FEATURES (FEATURES_EMAC_SG | FEATURES_EMAC_GSO | FEATURES_EMAC_CSUM_RX | FEATURES_EMAC_CSUM_TX)

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
// #define EMAC_RX_TMR         (0)
// #define EMAC_LINK_TMR       (1)
// #define EMAC_FLOW_CTL_TMR   (2)

// #define TIMER_EMAC_DYNAMIC_RX   (1)
// #define TIMER_EMAC_FLOW_TX      (2)

#define EMAC_CHECK_LINK_TIME (HZ)

#define IDX_CNT_INT_DONE  (0)
#define IDX_CNT_INT_RCOM  (1)
#define IDX_CNT_INT_RBNA  (2)
#define IDX_CNT_INT_TOVR  (3)
#define IDX_CNT_INT_TUND  (4)
#define IDX_CNT_INT_RTRY  (5)
#define IDX_CNT_INT_TBRE  (6)
#define IDX_CNT_INT_TCOM  (7)
#define IDX_CNT_INT_TIDLE (8)
#define IDX_CNT_INT_LINK  (9)
#define IDX_CNT_INT_ROVR  (10)
#define IDX_CNT_INT_HRESP (11)
#define IDX_CNT_JULIAN_D  (12)
#if TX_DELAY_INT
#define IDX_CNT_INT_TDLY (13)
#define IDX_CNT_INT_TDTO (14)
#endif

// #define RTL_8210                    (0x1CUL)

#define RX_THROUGHPUT_TEST 0
#define TX_THROUGHPUT_TEST 0

#ifdef CONFIG_MP_ETHERNET_SSTAR_ICMP_ENHANCE
#define PACKET_THRESHOLD  260
#define TXCOUNT_THRESHOLD 10
#endif

#if EMAC_FLOW_CONTROL_TX
#define MAC_CONTROL_TYPE        0x8808
#define MAC_CONTROL_OPCODE      0x0001
#define PAUSE_QUANTA_TIME_10M   ((1000000 * 10) / 500)
#define PAUSE_QUANTA_TIME_100M  ((1000000 * 100) / 500)
#define PAUSE_TIME_DIVISOR_10M  (PAUSE_QUANTA_TIME_10M / HZ)
#define PAUSE_TIME_DIVISOR_100M (PAUSE_QUANTA_TIME_100M / HZ)
#endif // #if EMAC_FLOW_CONTROL_TX

#if EMAC_PERFORMANCE
#define PERFORMANC_TIME_INTERVAL 1000
CamOsTimespec_t performance_start_time;
unsigned long   tx_total_bytes;
unsigned long   rx_total_bytes;
#endif

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
// u32 contiROVR = 0;
// u32 initstate= 0;
// u8 txidx =0;
// u32 txcount = 0;
// spinlock_t emac_lock;

// 0x78c9: link is down.
// static u32 phy_status_register = 0x78c9UL;

static dev_t    gEthDev;
static u8       _u8Minor = MINOR_EMAC_NUM;
struct sk_buff *pseudo_packet;

// ethtool tx/rx pause
static u8 _ethtool_tx_pause = TRUE;
static u8 _ethtool_rx_pause = TRUE;

static const char stats_strings[][ETH_GSTRING_LEN] = {
    "rx_packets         ", "rx_frame_errors    ", "rx_length_errors   ", "rx_crc_errors      ", "rx_fifo_errors     ",
    "rx_errors          ", "tx_packets         ", "tx_fifo_errors     ", "tx_carrier_errors  ", "tx_heartbeat_errors",
    "tx_window_errors   ", "tx_aborted_errors  ", "collisions         ",
};

#define EMAC_STATS_LEN ARRAY_SIZE(stats_strings)

#if TX_THROUGHPUT_TEST
unsigned char packet_content[] = {
    0xa4, 0xba, 0xdb, 0x95, 0x25, 0x29, 0x00, 0x30, 0x1b, 0xba, 0x02, 0xdb, 0x08, 0x00, 0x45, 0x00, 0x05, 0xda, 0x69,
    0x0a, 0x40, 0x00, 0x40, 0x11, 0xbe, 0x94, 0xac, 0x10, 0x5a, 0xe3, 0xac, 0x10, 0x5a, 0x70, 0x92, 0x7f, 0x13, 0x89,
    0x05, 0xc6, 0x0c, 0x5b, 0x00, 0x00, 0x03, 0x73, 0x00, 0x00, 0x00, 0x65, 0x00, 0x06, 0xe1, 0xef, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x13, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0xff, 0xff,
    0xfc, 0x18, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31,
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
#endif

static unsigned int max_rx_packet_count    = 0;
static unsigned int max_tx_packet_count    = 0;
static unsigned int min_tx_fifo_idle_count = 0xffff;
static unsigned int tx_bytes_per_timerbak  = 0;
static unsigned int tx_bytes_per_timer     = 0;
#if EMAC_DEBUG_RX
static unsigned int debug_level_rx   = 0;
static unsigned int rx_protocol_type = 0;
#endif
#if EMAC_DEBUG_TX
static unsigned int debug_level_tx   = 0;
static unsigned int tx_protocol_type = 0;
#endif
#if EMAC_PERFORMANCE
static bool performance_flag = false;
#endif

#if EMAC_LOCAL_TEST
static unsigned rx_checksum_info = 0;
static unsigned keep_link_up     = 0;
static unsigned rx_reveive_all   = 0;
#endif
// u32 RAM_ALLOC_SIZE=0;

// static unsigned int gu32GatingRxIrqTimes=0;

u64 rx_packet_cnt = 0;
#if SSTAR_EMAC_NAPI
int napi_enable_flag[2] = {0, 0};
#endif

#if TX_DELAY_INT
u32 tx_delay_pack_cnt       = 0;
int tx_delay_debug          = 0;
int tx_delay_enable         = 1;
int tx_delay_timeout_enable = 1;
int tx_delay_num            = 8;
int tx_delay_timeout        = 10000;
#endif

#if RX_DELAY_INT_DEBUG
int rx_delay_debug    = 0;
int rx_int_com_enable = 0;
int rx_delay_enable   = 1;
int rx_delay_num      = 8;
int rx_delay_cyc      = 0;
#endif

#if EMAC_LOOPBACK_DEBUG
static bool emac_loop = FALSE;
#endif

static struct timespec64 rx_time_last    = {0};
static int               rx_duration_max = 0;

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
// static struct timer_list EMAC_timer, hemac->timer_link;
// static struct timer_list hemac->timer_link;
#if RX_THROUGHPUT_TEST
static struct timer_list RX_timer;
#endif

//-------------------------------------------------------------------------------------------------
//  EMAC Function
//-------------------------------------------------------------------------------------------------
static u8   Dev_EMAC_ClkDisable(struct emac_handle *hemac);
static u8   Dev_EMAC_ClkEnable(struct emac_handle *hemac);
static int  Dev_EMAC_tx(struct sk_buff *skb, struct net_device *dev);
int         Dev_EMAC_SwReset(struct net_device *dev);
static void Dev_EMAC_dts(struct net_device *);
#if EMAC_FLOW_CONTROL_TX
static int _MDrv_EMAC_Pause_TX(struct net_device *emac_dev, struct sk_buff *skb, unsigned char *p_recv);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static void _Dev_EMAC_FlowTX_CB(struct timer_list *t);
#else
static void _Dev_EMAC_FlowTX_CB(unsigned long data);
#endif
#endif
#if REDUCE_CPU_FOR_RBNA
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static void _Dev_EMAC_IntRX_CB(struct timer_list *t);
#else
static void _Dev_EMAC_IntRX_CB(unsigned long data);
#endif
#endif // #if REDUCE_CPU_FOR_RBNA

// static void Dev_EMAC_timer_callback( unsigned long value );
// static void Dev_EMAC_timer_LinkStatus(unsigned long data);
static void _Dev_EMAC_Swing(void *hal, int param, u32 speed);

static void free_rx_skb(struct emac_handle *hemac)
{
    rx_desc_queue_t *rxinfo = &(hemac->rx_desc_queue);
    int              i      = 0;
    // unsigned long flags;

    if (NULL == rxinfo->skb_arr)
        return;

    // spin_lock_irqsave(&hemac->mutexRXD, flags);
    for (i = 0; i < rxinfo->num_desc; i++)
    {
        if (rxinfo->skb_arr[i])
            kfree_skb(rxinfo->skb_arr[i]);
    }
    // spin_unlock_irqrestore(&hemac->mutexRXD, flags);
}

// unsigned long oldTime;
// unsigned long PreLinkStatus;
#if SSTAR_EMAC_NAPI
static int Dev_EMAC_napi_poll(struct napi_struct *napi, int budget);
#endif

#ifdef CONFIG_SSTAR_EEE
static int Dev_EMAC_IS_TX_IDLE(void);
#endif // CONFIG_SSTAR_EEE

//#define PACKET_DUMP
#if defined(PACKET_DUMP)
/* you should define CONFIG_MSYS_KFILE_API in drivers/sstar/msys/ms_msys.c */
extern struct file *msys_kfile_open(const char *path, int flags, int rights);
extern void         msys_kfile_close(struct file *fp);
extern int msys_kfile_write(struct file *fp, unsigned long long offset, unsigned char *data, unsigned int size);

static int          txDumpCtrl         = 0;
static int          rxDumpCtrl         = 0;
static int          txDumpFileLength   = 0;
static int          rxDumpFileLength   = 0;
static char         txDumpFileName[32] = {0};
static char         rxDumpFileName[32] = {0};
static struct file *txDumpFile         = NULL;
static struct file *rxDumpFile         = NULL;

static ssize_t tx_dump_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    txDumpCtrl = simple_strtoul(buf, NULL, 10);
    if (1 == txDumpCtrl)
    {
        strcpy(txDumpFileName, "/tmp/tx_dump");
        txDumpFile = msys_kfile_open(txDumpFileName, O_RDWR | O_CREAT, 0644);
        if (NULL != txDumpFile)
        {
            txDumpFileLength = 0;
            printk(KERN_WARNING "success to open emac tx_dump file, '%s'...\n", txDumpFileName);
        }
        else
        {
            printk(KERN_WARNING "failed to open emac tx_dump file, '%s'!!\n", txDumpFileName);
        }
    }
    else if (0 == txDumpCtrl && txDumpFile != NULL)
    {
        msys_kfile_close(txDumpFile);
        txDumpFile = NULL;
    }
    return count;
}
static ssize_t tx_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", txDumpCtrl);
}
DEVICE_ATTR(tx_dump, 0644, tx_dump_show, tx_dump_store);

static ssize_t rx_dump_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    rxDumpCtrl = simple_strtoul(buf, NULL, 10);
    if (1 == rxDumpCtrl)
    {
        strcpy(rxDumpFileName, "/tmp/rx_dump");
        rxDumpFile = msys_kfile_open(rxDumpFileName, O_RDWR | O_CREAT, 0644);
        if (NULL != rxDumpFile)
        {
            rxDumpFileLength = 0;
            printk(KERN_WARNING "success to open emac rx_dump file, '%s'...\n", rxDumpFileName);
        }
        else
        {
            printk(KERN_WARNING "failed to open emac rx_dump file, '%s'!!\n", rxDumpFileName);
        }
    }
    else if (0 == rxDumpCtrl)
    {
        if (rxDumpFile != NULL)
        {
            msys_kfile_close(rxDumpFile);
            rxDumpFile = NULL;
        }
    }
    return count;
}
static ssize_t rx_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", rxDumpCtrl);
}
DEVICE_ATTR(rx_dump, 0644, rx_dump_show, rx_dump_store);
#endif

static unsigned long getCurMs(void)
{
    ktime_t time_nanosec;
    ktime_t curMs;

    time_nanosec = ktime_get();
    curMs        = ktime_to_ms(time_nanosec);
    return (unsigned long)curMs;
}

#if RX_THROUGHPUT_TEST
int         receive_bytes = 0;
static void RX_timer_callback(unsigned long value)
{
    int get_bytes = receive_bytes;
    int cur_speed;
    receive_bytes = 0;

    cur_speed = get_bytes * 8 / 20 / 1024;
    printk(" %dkbps", cur_speed);
    RX_timer.expires = jiffies + 20 * EMAC_CHECK_LINK_TIME;
    add_timer(&RX_timer);
}
#endif

//-------------------------------------------------------------------------------------------------
// skb_queue implementation
//-------------------------------------------------------------------------------------------------
#define SKBQ_SANITY 0
static int skb_queue_create(skb_queue *skb_q, int size, int size1)
{
    if ((NULL == skb_q) || (0 == size))
        return 0;
    // skb_q->size = size + 1;
    skb_q->size[0] = size + 1;
    skb_q->size[1] = size1 + 1;
    if (NULL == (skb_q->skb_info_arr = kzalloc(skb_q->size[1] * sizeof(skb_info), GFP_KERNEL)))
        return 0;
    skb_q->read = skb_q->write = skb_q->rw = 0;
    return 1;
}

static int skb_queue_reset(skb_queue *skb_q)
{
    int                 i;
    struct emac_handle *hemac = container_of(skb_q, struct emac_handle, skb_queue_tx);

    if (NULL == skb_q->skb_info_arr)
        return 0;

    for (i = 0; i < skb_q->size[1]; i++)
    {
        if (!skb_q->skb_info_arr[i].skb)
        {
            continue;
        }
        if (0xFFFFFFFF == (long)skb_q->skb_info_arr[i].skb)
        {
            void *p = BUS2VIRT(skb_q->skb_info_arr[i].skb_phys);
            kfree(p);
            continue;
        }
        dev_kfree_skb_any(skb_q->skb_info_arr[i].skb);
        hemac->skb_tx_free++;
    }
    memset(skb_q->skb_info_arr, 0, skb_q->size[1] * sizeof(skb_info));
    skb_q->read = skb_q->write = skb_q->rw = 0;
    return 1;
}

static int skb_queue_destroy(skb_queue *skb_q)
{
    skb_queue_reset(skb_q);
    kfree(skb_q->skb_info_arr);
    skb_q->skb_info_arr = NULL;
    skb_q->size[0] = skb_q->size[1] = 0;

    return 1;
}

#define QUEUE_USED(size, read, write) ((write) >= (read)) ? ((write) - (read)) : ((size) - (read) + (write))
#define QUEUE_FREE(size, read, write) ((write) >= (read)) ? ((size) - (write) + (read)-1) : ((read) - (write)-1)

inline static int skb_queue_used(skb_queue *skb_q, int idx)
{
#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
#endif
    if (2 == idx)
        return QUEUE_USED(skb_q->size[1], skb_q->rw, skb_q->write);
    return (0 == idx) ? QUEUE_USED(skb_q->size[1], skb_q->read, skb_q->rw)
                      : QUEUE_USED(skb_q->size[1], skb_q->read, skb_q->write);
}

inline static int skb_queue_free(skb_queue *skb_q, int idx)
{
#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
#endif
    // return skb_q->size[idx] - skb_queue_used(skb_q, idx) - 1;
    if (2 == idx)
        return QUEUE_FREE(skb_q->size[1], skb_q->rw, skb_q->write);
    return (0 == idx) ? QUEUE_FREE(skb_q->size[1], skb_q->read, skb_q->rw)
                      : QUEUE_FREE(skb_q->size[1], skb_q->read, skb_q->write);
}

inline static int skb_queue_remove(skb_queue *skb_q, struct sk_buff **pskb, dma_addr_t *pphys, int bSkbFree, int idx)
{
    skb_info *          pskb_info;
    int                 read;
    int                 len;
    struct emac_handle *hemac = container_of(skb_q, struct emac_handle, skb_queue_tx);

#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
    if (0 == skb_queue_used(skb_q, idx))
    {
        printk("[%s][%d] why\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    read      = skb_q->read;
    pskb_info = &(skb_q->skb_info_arr[read]);
#if SKBQ_SANITY
    if ((!pskb_info->skb) && (!pskb_info->skb_phys))
    {
        printk("[%s][%d] strange remove\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    len = pskb_info->skb_len;

    // printk("[%s][%d] (skb, addr, len) = (0x%08x, 0x%08x, %d)\n", __FUNCTION__, __LINE__, (int)pskb_info->skb,
    // VIRT2BUS(pskb_info->skb->data), len);

    hemac->skb_tx_free++;
    if (bSkbFree)
    {
        if (pskb_info->skb)
        {
            if (0xFFFFFFFF == (long)pskb_info->skb)
            {
                void *p = BUS2VIRT(pskb_info->skb_phys);
                kfree(p);
            }
            else
            {
                dev_kfree_skb_any(pskb_info->skb);
            }
            pskb_info->skb = NULL;
            // hemac->skb_tx_free++;
        }
    }
    else
    {
        *pskb  = pskb_info->skb;
        *pphys = pskb_info->skb_phys;
    }
    pskb_info->skb_phys = 0;
    pskb_info->skb_len  = 0;

    skb_q->read++;
    if (skb_q->read >= skb_q->size[1])
        skb_q->read -= skb_q->size[1];
    return len;
}

inline static int skb_queue_insert(skb_queue *skb_q, struct sk_buff *skb, dma_addr_t phys, int skb_len, int idx)
{
    skb_info *          pskb_info;
    int *               pwrite = NULL;
    struct emac_handle *hemac  = container_of(skb_q, struct emac_handle, skb_queue_tx);

#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
    if (0 == skb_queue_free(skb_q, idx))
    {
        printk("[%s][%d] why\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    pwrite    = (0 == idx) ? &skb_q->rw : &skb_q->write;
    pskb_info = &(skb_q->skb_info_arr[*pwrite]);
    // if ((pskb_info->used) || (pskb_info->skb))
    // if (pskb_info->used)
#if SKBQ_SANITY
    if ((pskb_info->skb) || (pskb_info->skb_phys))
    {
        printk("[%s][%d] strange insert\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    // if (skb)
    hemac->skb_tx_send++;
    // pskb_info->used = 1;
    pskb_info->skb_phys = phys;
    pskb_info->skb_len  = skb_len;
    pskb_info->skb      = skb;
    (*pwrite)++;
    if (*pwrite >= skb_q->size[1])
        (*pwrite) -= skb_q->size[1];
    return 1;
}

inline static int skb_queue_head_inc(skb_queue *skb_q, struct sk_buff **skb, dma_addr_t *pphys, int *plen, int idx)
{
    skb_info *pskb_info;
    int *     pwrite = NULL;
#if SKBQ_SANITY
    if (NULL == skb_q->skb_info_arr)
        return 0;
    if (0 == skb_queue_free(skb_q, idx))
    {
        printk("[%s][%d] why\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    *skb      = NULL;
    *pphys    = 0;
    pwrite    = (0 == idx) ? &skb_q->rw : &skb_q->write;
    pskb_info = &(skb_q->skb_info_arr[*pwrite]);
#if SKBQ_SANITY
    // if ((!pskb_info->skb) || (!pskb_info->skb_phys))
    if ((!pskb_info->skb_phys))
    {
        printk("[%s][%d] strange head inc\n", __FUNCTION__, __LINE__);
        return 0;
    }
#endif
    *skb   = pskb_info->skb;
    *pphys = pskb_info->skb_phys;
    *plen  = pskb_info->skb_len;
    (*pwrite)++;
    if (*pwrite >= skb_q->size[1])
        (*pwrite) -= skb_q->size[1];
    return 1;
}

inline static int skb_queue_full(skb_queue *skb_q, int idx)
{
    return (skb_queue_free(skb_q, idx)) ? 0 : 1;
}

//-------------------------------------------------------------------------------------------------
// Program the hardware MAC address from dev->dev_addr.
//-------------------------------------------------------------------------------------------------
void Dev_EMAC_update_mac_address(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 value;
    value = (dev->dev_addr[3] << 24) | (dev->dev_addr[2] << 16) | (dev->dev_addr[1] << 8) | (dev->dev_addr[0]);
    Hal_EMAC_Write_SA1L(hemac->hal, value);
    value = (dev->dev_addr[5] << 8) | (dev->dev_addr[4]);
    Hal_EMAC_Write_SA1H(hemac->hal, value);
}

//-------------------------------------------------------------------------------------------------
// Set the ethernet MAC address in dev->dev_addr
//-------------------------------------------------------------------------------------------------
static void Dev_EMAC_get_mac_address(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    char                addr[6];
    u32                 HiAddr, LoAddr;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = Hal_EMAC_get_SA1H_addr(hemac->hal);
    LoAddr = Hal_EMAC_get_SA1L_addr(hemac->hal);

    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr(addr))
    {
        memcpy(dev->dev_addr, &addr, 6);
        return;
    }
    // Check if bootloader set address in Specific-Address 2 //
    HiAddr  = Hal_EMAC_get_SA2H_addr(hemac->hal);
    LoAddr  = Hal_EMAC_get_SA2L_addr(hemac->hal);
    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr(addr))
    {
        memcpy(dev->dev_addr, &addr, 6);
        return;
    }
}

#ifdef URANUS_ETHER_ADDR_CONFIGURABLE
//-------------------------------------------------------------------------------------------------
// Store the new hardware address in dev->dev_addr, and update the MAC.
//-------------------------------------------------------------------------------------------------
static int Dev_EMAC_set_mac_address(struct net_device *dev, void *addr)
{
    struct sockaddr *   address = addr;
    struct emac_handle *hemac   = (struct emac_handle *)netdev_priv(dev);

    if (!is_valid_ether_addr(address->sa_data))
        return -EADDRNOTAVAIL;
    if (!(hemac->ep_flag & EP_FLAG_OPEND))
    {
        Dev_EMAC_ClkEnable(hemac);
    }
    spin_lock(&hemac->mutexPhy);
    memcpy(dev->dev_addr, address->sa_data, dev->addr_len);
    Dev_EMAC_update_mac_address(dev);
    spin_unlock(&hemac->mutexPhy);
    if (!(hemac->ep_flag & EP_FLAG_OPEND))
    {
        Dev_EMAC_ClkDisable(hemac);
    }
    return 0;
}
#endif

//-------------------------------------------------------------------------------------------------
// sstar Multicast hash rule
//-------------------------------------------------------------------------------------------------
// Hash_index[5] = da[5] ^ da[11] ^ da[17] ^ da[23] ^ da[29] ^ da[35] ^ da[41] ^ da[47]
// Hash_index[4] = da[4] ^ da[10] ^ da[16] ^ da[22] ^ da[28] ^ da[34] ^ da[40] ^ da[46]
// Hash_index[3] = da[3] ^ da[09] ^ da[15] ^ da[21] ^ da[27] ^ da[33] ^ da[39] ^ da[45]
// Hash_index[2] = da[2] ^ da[08] ^ da[14] ^ da[20] ^ da[26] ^ da[32] ^ da[38] ^ da[44]
// Hash_index[1] = da[1] ^ da[07] ^ da[13] ^ da[19] ^ da[25] ^ da[31] ^ da[37] ^ da[43]
// Hash_index[0] = da[0] ^ da[06] ^ da[12] ^ da[18] ^ da[24] ^ da[30] ^ da[36] ^ da[42]
//-------------------------------------------------------------------------------------------------

static void Dev_EMAC_sethashtable(struct net_device *dev, unsigned char *addr)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 mc_filter[2];
    u32                 uHashIdxBit;
    u32                 uHashValue;
    u32                 i;
    u32                 tmpcrc;
    u32                 uSubIdx;
    u64                 macaddr;
    u64                 mac[6];

    uHashValue = 0;
    macaddr    = 0;

    // Restore mac //
    for (i = 0; i < 6; i++)
    {
        mac[i] = (u64)addr[i];
    }

    // Truncate mac to u64 container //
    macaddr |= mac[0] | (mac[1] << 8) | (mac[2] << 16);
    macaddr |= (mac[3] << 24) | (mac[4] << 32) | (mac[5] << 40);

    // Caculate the hash value //
    for (uHashIdxBit = 0; uHashIdxBit < 6; uHashIdxBit++)
    {
        tmpcrc = (macaddr & (0x1UL << uHashIdxBit)) >> uHashIdxBit;
        for (i = 1; i < 8; i++)
        {
            uSubIdx = uHashIdxBit + (i * 6);
            tmpcrc  = tmpcrc ^ ((macaddr >> uSubIdx) & 0x1);
        }
        uHashValue |= (tmpcrc << uHashIdxBit);
    }

    mc_filter[0] = Hal_EMAC_ReadReg32(hemac->hal, REG_ETH_HSL);
    mc_filter[1] = Hal_EMAC_ReadReg32(hemac->hal, REG_ETH_HSH);

    // Set the corrsponding bit according to the hash value //
    if (uHashValue < 32)
    {
        mc_filter[0] |= (0x1UL << uHashValue);
        Hal_EMAC_WritReg32(hemac->hal, REG_ETH_HSL, mc_filter[0]);
    }
    else
    {
        mc_filter[1] |= (0x1UL << (uHashValue - 32));
        Hal_EMAC_WritReg32(hemac->hal, REG_ETH_HSH, mc_filter[1]);
    }
}

//-------------------------------------------------------------------------------------------------
// Enable/Disable promiscuous and multicast modes.
//-------------------------------------------------------------------------------------------------
static void Dev_EMAC_set_rx_mode(struct net_device *dev)
{
    struct emac_handle *   hemac = (struct emac_handle *)netdev_priv(dev);
    u32                    uRegVal;
    struct netdev_hw_addr *ha;

    uRegVal = Hal_EMAC_Read_CFG(hemac->hal);

    if (dev->flags & IFF_PROMISC)
    {
        // Enable promiscuous mode //
        uRegVal |= EMAC_CAF;
    }
    else if (dev->flags & (~IFF_PROMISC))
    {
        // Disable promiscuous mode //
        uRegVal &= ~EMAC_CAF;
    }
    Hal_EMAC_Write_CFG(hemac->hal, uRegVal);

    if (dev->flags & IFF_ALLMULTI)
    {
        // Enable all multicast mode //
        Hal_EMAC_update_HSH(hemac->hal, -1, -1);
        uRegVal |= EMAC_MTI;
    }
    else if (dev->flags & IFF_MULTICAST)
    {
        // Enable specific multicasts//
        Hal_EMAC_update_HSH(hemac->hal, 0, 0);
        netdev_for_each_mc_addr(ha, dev)
        {
            Dev_EMAC_sethashtable(dev, ha->addr);
        }
        uRegVal |= EMAC_MTI;
    }
    else if (dev->flags & ~(IFF_ALLMULTI | IFF_MULTICAST))
    {
        // Disable all multicast mode//
        Hal_EMAC_update_HSH(hemac->hal, 0, 0);
        uRegVal &= ~EMAC_MTI;
    }

    Hal_EMAC_Write_CFG(hemac->hal, uRegVal);
}

//-------------------------------------------------------------------------------------------------
// User-space ioctl interface.
//-------------------------------------------------------------------------------------------------
static int Dev_EMAC_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    switch (cmd)
    {
        case SIOCGMIIPHY:
        case SIOCGMIIREG:
        case SIOCSMIIREG:
            return phy_mii_ioctl(dev->phydev, rq, cmd);
        default:
            break;
    }
    return -EOPNOTSUPP;
}

//-------------------------------------------------------------------------------------------------
// Initialize and start the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void Dev_EMAC_start(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 uRegVal;

    // Enable Receive and Transmit //
    uRegVal = Hal_EMAC_Read_CTL(hemac->hal);
    uRegVal |= (EMAC_RE | EMAC_TE);
    Hal_EMAC_Write_CTL(hemac->hal, uRegVal);
}

//-------------------------------------------------------------------------------------------------
// Stop the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void Dev_EMAC_stop(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 uRegVal;

    // Disable Receive and Transmit //
    uRegVal = Hal_EMAC_Read_CTL(hemac->hal);
    uRegVal &= ~(EMAC_TE | EMAC_RE);
    Hal_EMAC_Write_CTL(hemac->hal, uRegVal);
}

//-------------------------------------------------------------------------------------------------
// Open the ethernet interface
//-------------------------------------------------------------------------------------------------
static int Dev_EMAC_open(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 uRegVal;
    // unsigned long flags;

    Dev_EMAC_ClkEnable(hemac);

#if SSTAR_EMAC_NAPI
    if (!strncmp(dev->name, "eth0", strlen("eth0")))
    {
        if (napi_enable_flag[0] == 0)
            napi_enable(&hemac->napi);
        napi_enable_flag[0] = 1;
    }
    else
    {
        if (napi_enable_flag[1] == 0)
            napi_enable(&hemac->napi);
        napi_enable_flag[1] = 1;
    }
#endif

    spin_lock(&hemac->mutexPhy);
    if (!is_valid_ether_addr(dev->dev_addr))
    {
        spin_unlock(&hemac->mutexPhy);
        return -EADDRNOTAVAIL;
    }
    spin_unlock(&hemac->mutexPhy);

#ifdef TX_SW_QUEUE
    _Dev_EMAC_tx_reset_TX_SW_QUEUE(dev);
#endif
    // ato  EMAC_SYS->PMC_PCER = 1 << EMAC_ID_EMAC;   //Re-enable Peripheral clock //
    // Hal_EMAC_Power_On_Clk(dev->dev);
    uRegVal = Hal_EMAC_Read_CTL(hemac->hal);
    uRegVal |= EMAC_CSR;
    Hal_EMAC_Write_CTL(hemac->hal, uRegVal);
    // Enable PHY interrupt //
    Hal_EMAC_enable_phyirq(hemac->hal);

    Hal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);
    hemac->gu32intrEnable = EMAC_INT_RBNA | EMAC_INT_TUND | EMAC_INT_RTRY | EMAC_INT_ROVR | EMAC_INT_HRESP;
#if !DYNAMIC_INT_TX
    hemac->gu32intrEnable |= EMAC_INT_TCOM;
#endif

#if TX_DELAY_INT_DEBUG
    if (tx_delay_debug)
    {
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY, tx_delay_enable);
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY_TIMEOUT, tx_delay_timeout_enable);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_MAX_CNT, tx_delay_num);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_TO_MAX_CNT, tx_delay_timeout);
    }
#endif

    hemac->gu32intrEnable |= EMAC_INT_RCOM;
    Hal_EMAC_IntEnable(hemac->hal, hemac->gu32intrEnable, 1);

    hemac->ep_flag |= EP_FLAG_OPEND;

    Dev_EMAC_start(dev);
    phy_start(dev->phydev);
    netif_start_queue(dev);

#ifdef CONFIG_EMAC_PHY_RESTART_AN
    if (hemac->phy_mode != PHY_INTERFACE_MODE_RMII)
        Hal_EMAC_Phy_Restart_An(hemac->hal);
#endif

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Close the interface
//-------------------------------------------------------------------------------------------------
static int Dev_EMAC_close(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    unsigned long       flags;

#if SSTAR_EMAC_NAPI
    if (!strncmp(dev->name, "eth0", strlen("eth0")))
    {
        if (napi_enable_flag[0] == 1)
            napi_disable(&hemac->napi);
        napi_enable_flag[0] = 0;
    }
    else
    {
        if (napi_enable_flag[1] == 1)
            napi_disable(&hemac->napi);
        napi_enable_flag[1] = 0;
    }
#endif

    spin_lock(&hemac->mutexPhy);
    // Disable Receiver and Transmitter //
    Dev_EMAC_stop(dev);

    // Disable PHY interrupt //
    Hal_EMAC_disable_phyirq(hemac->hal);
    spin_unlock(&hemac->mutexPhy);

    Hal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);
    netif_stop_queue(dev);
    netif_carrier_off(dev);
    phy_stop(dev->phydev);
    // del_timer(&hemac->timer_link);
    // Hal_EMAC_Power_Off_Clk(dev->dev);
    // hemac->ThisBCE.connected = 0;
    hemac->ep_flag &= (~EP_FLAG_OPEND);
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_reset(&(hemac->skb_queue_tx));
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    Dev_EMAC_ClkDisable(hemac);
    return 0;
}

static void _Dev_EMAC_ClkEnable(struct emac_handle *hemac, int en)
{
    int         nclk;
    int         i;
    struct clk *emac_clks;

    nclk = of_clk_get_parent_count(hemac->dev->of_node);

    for (i = 0; i < nclk; i++)
    {
        emac_clks = of_clk_get(hemac->dev->of_node, i);
        if (IS_ERR(emac_clks))
        {
            clk_put(emac_clks);
            continue;
        }
        if ((en) && (!hemac->bEmacClkOn))
        {
            clk_prepare_enable(emac_clks);
        }
        else if ((!en) && (hemac->bEmacClkOn))
        {
            clk_disable_unprepare(emac_clks);
        }
        clk_put(emac_clks);
    }
}

u8 Dev_EMAC_ClkDisable(struct emac_handle *hemac)
{
    _Dev_EMAC_ClkEnable(hemac, 0);
    Hal_EMAC_Power_Off_Clk(hemac->hal, NULL);
    hemac->bEmacClkOn = 0;
    return 1;
}

u8 Dev_EMAC_ClkEnable(struct emac_handle *hemac)
{
    _Dev_EMAC_ClkEnable(hemac, 1);
    Hal_EMAC_Power_On_Clk(hemac->hal, NULL);
    hemac->bEmacClkOn = 1;
    return 1;
}

//-------------------------------------------------------------------------------------------------
// Update the current statistics from the internal statistics registers.
//-------------------------------------------------------------------------------------------------
static struct net_device_stats *Dev_EMAC_stats(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    int                 ale, lenerr, seqe, lcol, ecol;

    // spin_lock_irq (hemac->lock);

    if (netif_running(dev) && hemac->bEmacClkOn)
    {
        hemac->stats.rx_packets += Hal_EMAC_Read_OK(hemac->hal); /* Good frames received */
        ale = Hal_EMAC_Read_ALE(hemac->hal);
        hemac->stats.rx_frame_errors += ale; /* Alignment errors */
        lenerr = Hal_EMAC_Read_ELR(hemac->hal);
        hemac->stats.rx_length_errors += lenerr; /* Excessive Length or Undersize Frame error */
        seqe = Hal_EMAC_Read_SEQE(hemac->hal);
        hemac->stats.rx_crc_errors += seqe; /* CRC error */
        hemac->stats.rx_fifo_errors += Hal_EMAC_Read_ROVR(hemac->hal);
        hemac->stats.rx_errors += ale + lenerr + seqe + Hal_EMAC_Read_SE(hemac->hal) + Hal_EMAC_Read_RJB(hemac->hal);
        hemac->stats.tx_packets += Hal_EMAC_Read_FRA(hemac->hal);           /* Frames successfully transmitted */
        hemac->stats.tx_fifo_errors += Hal_EMAC_Read_TUE(hemac->hal);       /* Transmit FIFO underruns */
        hemac->stats.tx_carrier_errors += Hal_EMAC_Read_CSE(hemac->hal);    /* Carrier Sense errors */
        hemac->stats.tx_heartbeat_errors += Hal_EMAC_Read_SQEE(hemac->hal); /* Heartbeat error */
        lcol = Hal_EMAC_Read_LCOL(hemac->hal);
        ecol = Hal_EMAC_Read_ECOL(hemac->hal);
        hemac->stats.tx_window_errors += lcol;  /* Late collisions */
        hemac->stats.tx_aborted_errors += ecol; /* 16 collisions */
        hemac->stats.collisions += Hal_EMAC_Read_SCOL(hemac->hal) + Hal_EMAC_Read_MCOL(hemac->hal) + lcol + ecol;
    }

    // spin_unlock_irq (hemac->lock);

    return &hemac->stats;
}

static int Dev_EMAC_TxReset(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 val   = Hal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;

    Hal_EMAC_Write_CTL(hemac->hal, (val & ~EMAC_TE));

    val = Hal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;
    EMAC_ERR("MAC0_CTL:0x%08x\n", val);
    // Hal_EMAC_Write_TCR(0);
    mdelay(1);
    Hal_EMAC_Write_CTL(hemac->hal, (Hal_EMAC_Read_CTL(hemac->hal) | EMAC_TE));
    val = Hal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;
    EMAC_ERR("MAC0_CTL:0x%08x\n", val);
    return 0;
}

#ifdef CONFIG_SSTAR_EEE
static int Dev_EMAC_IS_TX_IDLE(void)
{
    u32 check;
    u32 tsrval = 0;

    u8 avlfifo[8] = {0};
    u8 avlfifoidx;
    u8 avlfifoval = 0;

#ifdef TX_QUEUE_4
    for (check = 0; check < EMAC_CHECK_CNT; check++)
    {
        tsrval = Hal_EMAC_Read_TSR();

        avlfifo[0] = ((tsrval & EMAC_IDLETSR) != 0) ? 1 : 0;
        avlfifo[1] = ((tsrval & EMAC_BNQ) != 0) ? 1 : 0;
        avlfifo[2] = ((tsrval & EMAC_TBNQ) != 0) ? 1 : 0;
        avlfifo[3] = ((tsrval & EMAC_FBNQ) != 0) ? 1 : 0;
        avlfifo[4] = ((tsrval & EMAC_FIFO1IDLE) != 0) ? 1 : 0;
        avlfifo[5] = ((tsrval & EMAC_FIFO2IDLE) != 0) ? 1 : 0;
        avlfifo[6] = ((tsrval & EMAC_FIFO3IDLE) != 0) ? 1 : 0;
        avlfifo[7] = ((tsrval & EMAC_FIFO4IDLE) != 0) ? 1 : 0;

        avlfifoval = 0;

        for (avlfifoidx = 0; avlfifoidx < 8; avlfifoidx++)
        {
            avlfifoval += avlfifo[avlfifoidx];
        }

        if (avlfifoval == 8)
            return 1;
    }
#endif

    return 0;
}
#endif // CONFIG_SSTAR_EEE

#if EMAC_FLOW_CONTROL_RX
static void _MDrv_EMAC_PausePkt_Send(struct net_device *dev)
{
    unsigned long       flags;
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 val   = Hal_EMAC_Read_CTL(hemac->hal) & 0x000001FFUL;

    // Disable Rx
    Hal_EMAC_Write_CTL(hemac->hal, (val & ~EMAC_RE)); // why RX
    memcpy(&hemac->pu8PausePkt[6], dev->dev_addr, 6);
    Chip_Flush_Cache_Range((void *)hemac->pu8PausePkt, SKB_DATA_ALIGN(hemac->u8PausePktSize));
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    hemac->isPausePkt = 1;
    // skb_queue_insert(&(hemac->skb_queue_tx), NULL, VIRT2BUS(hemac->pu8PausePkt), hemac->u8PausePktSize, 1);
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    Hal_EMAC_Write_CTL(hemac->hal, (Hal_EMAC_Read_CTL(hemac->hal) | EMAC_RE)); // why RX
}
#endif // #if EMAC_FLOW_CONTROL_RX

//-------------------------------------------------------------------------------------------------
// Patch for losing small-size packet when running SMARTBIT
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_MP_ETHERNET_SSTAR_ICMP_ENHANCE
static void Dev_EMAC_Period_Retry(struct sk_buff *skb, struct net_device *dev)
{
    u32 xval;
    u32 uRegVal;

    xval = Hal_EMAC_ReadReg32(hemac->hal, REG_ETH_CFG);

    if ((skb->len <= PACKET_THRESHOLD) && !(xval & EMAC_SPD) && !(xval & EMAC_FD))
    {
        txcount++;
    }
    else
    {
        txcount = 0;
    }

    if (txcount > TXCOUNT_THRESHOLD)
    {
        uRegVal = Hal_EMAC_Read_CFG(hemac->hal);
        uRegVal |= 0x00001000UL;
        Hal_EMAC_Write_CFG(hemac->hal, uRegVal);
    }
    else
    {
        uRegVal = Hal_EMAC_Read_CFG(hemac->hal);
        uRegVal &= ~(0x00001000UL);
        Hal_EMAC_Write_CFG(hemac->hal, uRegVal);
    }
}
#endif

static int _Dev_EMAC_tx_pump(struct emac_handle *hemac, int bFree, int bPump)
{
    int             txUsedCnt;
    int             txUsedCntSW;
    int             i;
    unsigned long   flags;
    unsigned long   flags1;
    int             len;
    struct sk_buff *skb = NULL;
    dma_addr_t      skb_addr;
    int             nPkt;
    int             txFreeCnt;
    int             txPendCnt;
    int             ret = 0;

    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    if (bFree)
    {
        txUsedCnt = Hal_EMAC_TXQ_Used(hemac->hal);
        if (txUsedCnt > max_tx_packet_count)
        {
            max_tx_packet_count = txUsedCnt;
        }
        txUsedCntSW = skb_queue_used(&hemac->skb_queue_tx, 0);
        ret         = txUsedCntSW - txUsedCnt;
        for (i = txUsedCnt; i < txUsedCntSW; i++)
        {
            // Hal_EMAC_TXQ_Remove();
            len = skb_queue_remove(&hemac->skb_queue_tx, NULL, NULL, 1, 0);
            spin_lock_irqsave(&hemac->emac_data_done_lock, flags1);
            hemac->data_done += len;
            spin_unlock_irqrestore(&hemac->emac_data_done_lock, flags1);

            hemac->stats.tx_bytes += len;
            tx_bytes_per_timer += len;
        }
    }

    if (bPump)
    {
        int skb_len;
        txFreeCnt = skb_queue_free(&hemac->skb_queue_tx, 0);
        txPendCnt = skb_queue_used(&hemac->skb_queue_tx, 2);
        nPkt      = (txFreeCnt < txPendCnt) ? txFreeCnt : txPendCnt;
        for (i = 0; i < nPkt; i++)
        {
            skb_queue_head_inc(&hemac->skb_queue_tx, &skb, &skb_addr, &skb_len, 0);
            if (skb_addr)
            {
                Hal_EMAC_TXQ_Insert(hemac->hal, skb_addr, skb_len);
            }
        }
    }
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    return ret;
}

#if EMAC_DEBUG_TX || EMAC_DEBUG_RX || EMAC_PERFORMANCE
static void Dev_EMAC_debug_level(unsigned char *pkt, u32 pktlen, unsigned int debug_level, unsigned int protocol_type,
                                 unsigned char *p_type)
{
    int i;
    int protocol;
    if (pktlen >= 24)
    {
        if (pkt[12] == 8 && pkt[13] == 6)
        {
            if (protocol_type != 86 && protocol_type != 0)
                return;
            if (!strcmp(p_type, "trans"))
                printk(KERN_CONT "\nTransmit packet:\n    protocol: ARP\n");
            else
                printk(KERN_CONT "\nReceive packet:\n    protocol: ARP\n");
        }
        else
        {
            protocol = (int)pkt[23];
            switch (protocol)
            {
                case 1:
                    if (protocol_type != 1 && protocol_type != 0)
                        return;
                    if (!strcmp(p_type, "trans"))
                        printk(KERN_CONT "\nTransmit packet:\n    protocol: ICMP\n");
                    else
                        printk(KERN_CONT "\nReceive packet:\n    protocol: ICMP\n");
                    break;
                case 6:
                    if (protocol_type != 6 && protocol_type != 0)
                        return;
                    if (!strcmp(p_type, "trans"))
                        printk(KERN_CONT "\nTransmit packet:\n    protocol: TCP\n");
                    else
                        printk(KERN_CONT "\nReceive packet:\n    protocol: TCP\n");
                    break;
                case 17:
                    if (protocol_type != 17 && protocol_type != 0)
                        return;
                    if (!strcmp(p_type, "trans"))
                        printk(KERN_CONT "\nTransmit packet:\n    protocol: UDP\n");
                    else
                        printk(KERN_CONT "\nReceive packet:\n    protocol: UDP\n");
                    break;
                default:
                    if (protocol_type != 0)
                        return;
                    if (!strcmp(p_type, "trans"))
                        printk(KERN_CONT "\nTransmit packet:\n    protocol: ARP\n");
                    else
                        printk(KERN_CONT "\nReceive packet:\n    protocol: OTHERS(%d)\n", protocol);
            }
        }
    }

    if (debug_level >= 2)
    {
        printk(KERN_CONT "    packet length = %d", pktlen);
        for (i = 0; i < pktlen; i++)
        {
            if (i % 16 == 0)
                printk(KERN_CONT "\n00%x0: ", i / 16);
            printk(KERN_CONT "%02x ", pkt[i]);
            if (i == 13 && debug_level == 2)
                break;
        }
        printk("\n");
    }
}
#endif

#if EMAC_DEBUG_TX
static void Dev_EMAC_tx_debug(unsigned char *p_trans, u32 pktlen)
{
    if (debug_level_tx <= 0)
        return;
    Dev_EMAC_debug_level(p_trans, pktlen, debug_level_tx, tx_protocol_type, "trans");
}
#endif

static int Dev_EMAC_tx(struct sk_buff *skb, struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    dma_addr_t          skb_addr;
    // int txIdleCount=0;
    // int txIdleCntSW = 0;
    unsigned long flags;
    unsigned long flag1;
    int           ret = NETDEV_TX_OK;

    spin_lock_irqsave(&hemac->mutexNetIf, flag1);

#if TX_DELAY_INT_DEBUG
    if (tx_delay_debug)
    {
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY, tx_delay_enable);
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY_TIMEOUT, tx_delay_timeout_enable);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_MAX_CNT, tx_delay_num);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_TO_MAX_CNT, tx_delay_timeout);
    }
#endif

#if PHY_LED_CTL
    // if(hemac->led_orange!=-1 && hemac->led_green!=-1)
    if (hemac->led_orange != -1)
    {
        if (hemac->led_count++ > hemac->led_flick_speed)
        {
            sstar_gpio_set_low(hemac->led_orange);
            hemac->led_count = 0;
        }
    }
#endif

#if EMAC_DEBUG_TX
    Dev_EMAC_tx_debug(skb->data, skb->len);
#endif

#ifdef CONFIG_SSTAR_EEE
    Hal_EMAC_Disable_EEE_TX();
#endif

    if (netif_queue_stopped(dev))
    {
        // EMAC_ERR("netif_queue_stopped\n");
        ret = NETDEV_TX_BUSY;
        goto out_unlock;
    }
    if (!netif_carrier_ok(dev))
    {
        // EMAC_ERR("netif_carrier_off\n");
        ret = NETDEV_TX_BUSY;
        goto out_unlock;
    }
    if (skb->len > EMAC_MTU)
    {
        // EMAC_ERR("Something wrong (mtu, tx_len) = (%d, %d)\n", dev->mtu, skb->len);
        // ret = NETDEV_TX_BUSY;
        dev_kfree_skb_any(skb);
        dev->stats.tx_dropped++;
        goto out_unlock;
    }
#if defined(PACKET_DUMP)
    if (1 == txDumpCtrl && NULL != txDumpFile)
    {
        txDumpFileLength += msys_kfile_write(txDumpFile, txDumpFileLength, skb->data, skb->len);
    }
    else if (2 == txDumpCtrl && NULL != txDumpFile)
    {
        msys_kfile_close(txDumpFile);
        txDumpFile = NULL;
        printk(KERN_WARNING "close emac tx_dump file '%s', len=0x%08X...\n", txDumpFileName, txDumpFileLength);
    }
    else if (3 == txDumpCtrl)
    {
        // you can print packet data in this area, with skb->data, skb->len
    }
#endif
    // if buffer remains one space, notice upperr layer to block transmition.
    //  if (Hal_EMAC_TXQ_Full() || skb_queue_full(&(hemac->skb_queue_tx)))
    //  if (Hal_EMAC_TXQ_Full())
    if (skb_queue_full(&hemac->skb_queue_tx, 1))
    {
        netif_stop_queue(dev);
#if DYNAMIC_INT_TX
#if TX_DELAY_INT
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY, tx_delay_enable);
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY_TIMEOUT, tx_delay_timeout_enable);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_MAX_CNT, tx_delay_num);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_TO_MAX_CNT, tx_delay_timeout);
#else
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
#endif
#endif
        ret = NETDEV_TX_BUSY;
        goto out_unlock;
    }

#if EMAC_FLOW_CONTROL_RX
    if (_ethtool_rx_pause)
    {
        if (hemac->isPausePkt)
        {
            unsigned long flags;
            spin_lock_irqsave(&hemac->mutexTXQ, flags);
            skb_queue_insert(&(hemac->skb_queue_tx), NULL, VIRT2BUS(hemac->pu8PausePkt), hemac->u8PausePktSize, 1);
            hemac->isPausePkt = 0;
            spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
            ret = NETDEV_TX_BUSY;
            goto out_unlock;
        }
    }
#endif // #if EMAC_FLOW_CONTROL_RX

#if EMAC_SG
    {
        int i;
        int nr_frags = skb_shinfo(skb)->nr_frags;
        int len;

        // dma_unmap_single(NULL, VIRT2PA(start), EMAC_MTU, DMA_TO_DEVICE);
        if (nr_frags)
        {
            char *start = kmalloc(EMAC_PACKET_SIZE_MAX, GFP_ATOMIC);
            char *p     = start;

            if (!start)
            {
                ret = NETDEV_TX_BUSY;
                goto out_unlock;
            }

            memcpy(p, skb->data, skb_headlen(skb));
            p += skb_headlen(skb);
            len = skb_headlen(skb);
            for (i = 0; i < nr_frags; i++)
            {
                skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
                memcpy(p, skb_frag_address(frag), skb_frag_size(frag));
                p += skb_frag_size(frag);
                len += skb_frag_size(frag);
            }
            /*
                        if (len != skb->len)
                            printk("[%s][%d] strange ??? (len, skb->len) = (%d, %d)\n", __FUNCTION__, __LINE__, len,
               skb->len);
            */
            if (EMAC_SG_BUF_CACHE)
                Chip_Flush_Cache_Range((void *)start, EMAC_PACKET_SIZE_MAX);
            skb_addr = VIRT2BUS(start);
            spin_lock_irqsave(&hemac->mutexTXQ, flags);
            skb_queue_insert(&(hemac->skb_queue_tx), (struct sk_buff *)0xFFFFFFFF, (dma_addr_t)skb_addr, skb->len, 1);
            spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
            dev_kfree_skb_any(skb);
        }
        else
        {
            {
                struct sk_buff *skb_tmp = skb_clone(skb, GFP_ATOMIC);
                if (!skb_tmp)
                {
                    printk("[%s][%d] skb_clone fail\n", __FUNCTION__, __LINE__);
                    ret = NETDEV_TX_BUSY;
                    goto out_unlock;
                }
                dev_kfree_skb_any(skb);
                skb = skb_tmp;
            }
            skb_addr = VIRT2BUS(skb->data);
            spin_lock_irqsave(&hemac->mutexTXQ, flags);
            skb_queue_insert(&(hemac->skb_queue_tx), skb, skb_addr, skb->len, 1);
            spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
            Chip_Flush_Cache_Range((void *)(skb->head), SKB_DATA_ALIGN(skb->len + (skb->data - skb->head)));
        }
        if (nr_frags >= hemac->maxSG)
            hemac->maxSG = nr_frags + 1;
    }
#else // #if #if EMAC_SG

#if DYNAMIC_INT_TX_TIMER
    if ((DYNAMIC_INT_TX) && (0 == hemac->timerTxWdtPeriod))
#else
    if (DYNAMIC_INT_TX)
#endif
    {
        struct sk_buff *skb_tmp = skb_clone(skb, GFP_ATOMIC);
        if (!skb_tmp)
        {
            printk("[%s][%d] skb_clone fail\n", __FUNCTION__, __LINE__);
            ret = NETDEV_TX_BUSY;
            goto out_unlock;
        }
        dev_kfree_skb_any(skb);
        // kfree_skb(skb);
        skb = skb_tmp;
    }
    skb_addr = VIRT2BUS(skb->data);
    if (unlikely(0 == skb_addr))
    {
        dev_kfree_skb_any(skb);
        // kfree_skb(skb);
        printk(KERN_ERR "ERROR!![%s]%d\n", __FUNCTION__, __LINE__);
        dev->stats.tx_dropped++;
        goto out_unlock;
    }
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_insert(&(hemac->skb_queue_tx), skb, skb_addr, skb->len, 1);
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);

    Chip_Flush_Cache_Range((void *)(skb->head), SKB_DATA_ALIGN(skb->len + (skb->data - skb->head)));
#endif // #if #if EMAC_SG

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
    netif_trans_update(dev);
#else
    dev->trans_start = jiffies;
#endif
out_unlock:
{
    /*
            int bSkbFree = ((hemac->skb_tx_send & 0x0f) == 0) ? 1 : 0;
            _Dev_EMAC_tx_pump(hemac, bSkbFree, 1);
    */
    _Dev_EMAC_tx_pump(hemac, 1, 1);
}

    /*
    #if DYNAMIC_INT_TX
        // if ((hemac->skb_tx_send - hemac->skb_tx_free) > 32)
        // if ((hemac->skb_tx_send & 0x0f) == 0)
        // if ((hemac->skb_tx_send - hemac->skb_tx_free) >= DYNAMIC_INT_TX_TH)
        if ((hemac->skb_tx_send - hemac->skb_tx_free) >= 64)
        {
            Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
        }
    #endif
    */
    spin_unlock_irqrestore(&hemac->mutexNetIf, flag1);
    return ret;
}

#if EMAC_DEBUG_RX
static void Dev_EMAC_rx_debug(unsigned char *p_recv, u32 pktlen)
{
    if (debug_level_rx <= 0)
        return;
    Dev_EMAC_debug_level(p_recv, pktlen, debug_level_rx, rx_protocol_type, "recv");
}
#endif

//-------------------------------------------------------------------------------------------------
// Extract received frame from buffer descriptors and sent to upper layers.
// (Called from interrupt context)
// (Disable RX software discriptor)
//-------------------------------------------------------------------------------------------------
static int Dev_EMAC_rx(struct net_device *dev, int budget)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    unsigned char *     p_recv;
    u32                 pktlen;
    u32                 received = 0;
    struct sk_buff *    skb;
    struct sk_buff *    clean_skb;
    rx_desc_queue_t *   rxinfo         = &(hemac->rx_desc_queue);
    u32                 pointer_offset = 0;
    // unsigned long flags;

    if (0 == budget)
        return 0;

#if PHY_LED_CTL
    // if(hemac->led_orange!=-1 && hemac->led_green!=-1)
    if (hemac->led_orange != -1)
    {
        if (hemac->led_count++ > hemac->led_flick_speed)
        {
            sstar_gpio_set_low(hemac->led_orange);
            hemac->led_count = 0;
        }
    }
#endif

    // spin_lock_irqsave(&hemac->mutexRXD, flags);
    // If any Ownership bit is 1, frame received.
    do
    {
        char *        pData;
        struct rbf_t *desc = &(rxinfo->desc[rxinfo->idx]);
        pointer_offset     = rxinfo->idx * EMAC_PACKET_SIZE_MAX;

        // if(!((dlist->descriptors[hemac->rxBuffIndex].addr) & EMAC_DESC_DONE))
        if (!(desc->addr & EMAC_DESC_DONE))
        {
            break;
        }
        // p_recv = (char *) ((((dlist->descriptors[hemac->rxBuffIndex].addr) & 0xFFFFFFFFUL) + RAM_VA_PA_OFFSET +
        // MIU0_BUS_BASE) &~(EMAC_DESC_DONE | EMAC_DESC_WRAP));

#if RX_DESC_API
        p_recv = RX_ADDR_GET(desc);
#else
#if EXT_PHY_PATCH
        if (IS_EXT_PHY(hemac))
        {
            p_recv = hemac->pu8RXBuf + pointer_offset;
        }
        else
#endif
        {
            p_recv = BUS2VIRT(CLR_BITS(desc->addr, EMAC_DESC_DONE | EMAC_DESC_WRAP));
        }
#endif
        pktlen = desc->size & 0x7ffUL; /* Length of frame including FCS */

#if EMAC_DEBUG_RX
        Dev_EMAC_rx_debug(p_recv, pktlen);
#endif

#if RX_THROUGHPUT_TEST
        receive_bytes += pktlen;
#endif

        if (unlikely(((pktlen > EMAC_MTU) || (pktlen < 64))))
        {
            EMAC_ERR("drop packet!!(pktlen = %d)", pktlen);
            desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE);
            Chip_Flush_MIU_Pipe();
            rxinfo->idx++;
            if (rxinfo->idx >= rxinfo->num_desc)
                rxinfo->idx = 0;
            hemac->stats.rx_length_errors++;
            hemac->stats.rx_errors++;
            hemac->stats.rx_dropped++;
            continue;
        }

        if (unlikely(!(clean_skb = alloc_skb(EMAC_PACKET_SIZE_MAX, GFP_ATOMIC))))
        {
            // printk(KERN_ERR"Can't alloc skb.[%s]%d\n",__FUNCTION__,__LINE__);;
            goto jmp_rx_exit;
            // return -ENOMEM;
        }
        skb = rxinfo->skb_arr[rxinfo->idx];
#if EXT_PHY_PATCH
        if (IS_EXT_PHY(hemac))
        {
            memcpy(skb->data, p_recv, pktlen);
            pktlen -= 4; /* Remove FCS */
            pData = skb_put(skb, pktlen);
        }
        else
#endif
        {
            pktlen -= 4; /* Remove FCS */
            pData = skb_put(skb, pktlen);
            dma_unmap_single(hemac->dev, VIRT2PA(pData), pktlen, DMA_FROM_DEVICE);
        }

#if defined(PACKET_DUMP)
        if (1 == rxDumpCtrl && NULL != rxDumpFile)
        {
            rxDumpFileLength += msys_kfile_write(rxDumpFile, rxDumpFileLength, pData, pktlen);
        }
        else if (2 == rxDumpCtrl && NULL != rxDumpFile)
        {
            msys_kfile_close(rxDumpFile);
            rxDumpFile = NULL;
            printk(KERN_WARNING "close emac rx_dump file '%s', len=0x%08X...\n", rxDumpFileName, rxDumpFileLength);
        }
        else if (3 == rxDumpCtrl)
        {
            // you can print packet data in this area, with pData, pktlen
        }
#endif

        skb->dev = dev;
#if EMAC_LOOPBACK_DEBUG
        if (emac_loop == FALSE)
#endif
            skb->protocol = eth_type_trans(skb, dev);
            // skb->len = pktlen;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
        dev->last_rx = jiffies;
#endif
        hemac->stats.rx_bytes += pktlen;
#if EMAC_FLOW_CONTROL_TX
        if (_ethtool_tx_pause)
        {
            if (0 == Hal_EMAC_FlowControl_TX(hemac))
            {
                _MDrv_EMAC_Pause_TX(dev, skb, p_recv);
            }
        }
#endif // #if EMAC_FLOW_CONTROL_TX
#if RX_THROUGHPUT_TEST
        kfree_skb(skb);
#else

#if RX_CHECKSUM
        if (((desc->size & EMAC_DESC_TCP) || (desc->size & EMAC_DESC_UDP)) && (desc->size & EMAC_DESC_IP_CSUM)
            && (desc->size & EMAC_DESC_TCP_UDP_CSUM))
        {
            skb->ip_summed = CHECKSUM_UNNECESSARY;
#if EAMC_RX_CHECKSUM_DEBUG
            if (rx_checksum_info)
            {
                printk("checksum pass desc size %x TCP[%d] UDP[%d] IP[%d] TCP-UDP[%d]\n", desc->size,
                       desc->size & EMAC_DESC_TCP ? 1 : 0, desc->size & EMAC_DESC_UDP ? 1 : 0,
                       (desc->size & EMAC_DESC_IP_CSUM) ? 1 : 0, (desc->size & EMAC_DESC_TCP_UDP_CSUM) ? 1 : 0);
            }
#endif
        }
        else
        {
#if EAMC_RX_CHECKSUM_DEBUG
            if (rx_checksum_info)
            {
                printk("checksum err desc size %x TCP[%d] UDP[%d] IP[%d] TCP-UDP[%d]\n", desc->size,
                       desc->size & EMAC_DESC_TCP ? 1 : 0, desc->size & EMAC_DESC_UDP ? 1 : 0,
                       (desc->size & EMAC_DESC_IP_CSUM) ? 1 : 0, (desc->size & EMAC_DESC_TCP_UDP_CSUM) ? 1 : 0);
            }
#endif

            skb->ip_summed = CHECKSUM_NONE;
        }
#endif

#if EMAC_LOOPBACK_DEBUG
        if (emac_loop)
        {
            Dev_EMAC_tx(skb, dev);
        }
        else
#endif
        {
#ifdef ISR_BOTTOM_HALF
            netif_rx_ni(skb);
#elif SSTAR_EMAC_NAPI
#if (EMAC_GSO)
            napi_gro_receive(&hemac->napi, skb);
#else
            netif_receive_skb(skb);
#endif
#else
            netif_rx(skb);
#endif
        }

        received++;
#if HW_FLOW_CONTROL
        if (_ethtool_rx_pause)
        {
            Hal_EMAC_Flow_control_Rx_Done_Trig(hemac->hal);
        }
#endif

#endif /*RX_THROUGHPUT_TEST*/

        // if (dlist->descriptors[hemac->rxBuffIndex].size & EMAC_MULTICAST)
        if (desc->size & EMAC_MULTICAST)
        {
            hemac->stats.multicast++;
        }

        // fill clean_skb into RX descriptor
        rxinfo->skb_arr[rxinfo->idx] = clean_skb;

#if EXT_PHY_PATCH
        if (IS_EXT_PHY(hemac))
        {
            if (rxinfo->idx == (rxinfo->num_desc - 1))
                desc->addr = PA2BUS(rxinfo->skb_phys + pointer_offset) | EMAC_DESC_DONE | EMAC_DESC_WRAP;
            else
                desc->addr = PA2BUS(rxinfo->skb_phys + pointer_offset) | EMAC_DESC_DONE;
            desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE);
        }
        else
#endif
        {
#if RX_DESC_API
            RX_DESC_MAKE(desc, VIRT2BUS(skb->data), (rxinfo->idx == (rxinfo->num_desc - 1)) ? EMAC_DESC_WRAP : 0);
#else
            if (rxinfo->idx == (rxinfo->num_desc - 1))
                desc->addr = VIRT2BUS(clean_skb->data) | EMAC_DESC_DONE | EMAC_DESC_WRAP;
            else
                desc->addr = VIRT2BUS(clean_skb->data) | EMAC_DESC_DONE;
            desc->addr = CLR_BITS(desc->addr, EMAC_DESC_DONE);
#endif
            dma_map_single(hemac->dev, clean_skb->data, EMAC_PACKET_SIZE_MAX, DMA_FROM_DEVICE);
        }
        Chip_Flush_MIU_Pipe();

        rxinfo->idx++;
        if (rxinfo->idx >= rxinfo->num_desc)
            rxinfo->idx = 0;

#if SSTAR_EMAC_NAPI
        // if(received >= EMAC_NAPI_WEIGHT) {
        if (received >= budget)
        {
            break;
        }
#endif

    } while (1);

jmp_rx_exit:
    // spin_unlock_irqrestore(&hemac->mutexRXD, flags);
    if (received > max_rx_packet_count)
        max_rx_packet_count = received;
    rx_packet_cnt += received;
    return received;
}

//-------------------------------------------------------------------------------------------------
// MAC interrupt handler
//(Interrupt delay enable)
//-------------------------------------------------------------------------------------------------
static int RBNA_detailed = 0;

static u32 _MDrv_EMAC_ISR_RBNA(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;
    int                 empty = 0;
    int                 idx;
    rx_desc_queue_t *   rxinfo;

    if (0 == (intstatus & EMAC_INT_RBNA))
        return 0;

    hemac  = (struct emac_handle *)netdev_priv(dev);
    rxinfo = &(hemac->rx_desc_queue);

    hemac->stats.rx_missed_errors++;

    // write 1 clear
    Hal_EMAC_Write_RSR(hemac->hal, EMAC_BNA);
    if (RBNA_detailed > 0)
    {
        // u32 u32RBQP_Addr = Hal_EMAC_Read_RBQP(hemac->hal)- VIRT2BUS(rxinfo->desc);
        for (idx = 0; idx < rxinfo->num_desc; idx++)
        {
            // if(!((dlist->descriptors[idx].addr) & EMAC_DESC_DONE))
            if (!(rxinfo->desc[idx].addr & EMAC_DESC_DONE))
            {
                empty++;
            }
            else
            {
                printk(KERN_ERR "RBNA: [0x%X]\n", idx);
            }
        }
        // printk(KERN_ERR"RBNA: empty=0x%X, rxBuffIndex=0x%X, rx_missed_errors=%ld RBQP_offset=0x%x\n",empty,
        // rxinfo->idx, hemac->stats.rx_missed_errors,u32RBQP_Addr);
    }

    hemac->irq_count[IDX_CNT_INT_RBNA]++;
    // printk("RBNA\n");
#if REDUCE_CPU_FOR_RBNA
    {
        unsigned long flags;
        spin_lock_irqsave(&hemac->mutexIntRX, flags);
        if (0 == timer_pending(&hemac->timerIntRX))
        {
            Hal_EMAC_RX_ParamSet(hemac->hal, 0xff, 0xff);
            hemac->timerIntRX.expires = jiffies + HZ;
            add_timer(&hemac->timerIntRX);
        }
        spin_unlock_irqrestore(&hemac->mutexIntRX, flags);
    }
#endif // #if REDUCE_CPU_FOR_RBNA
    // gu32GatingRxIrqTimes = 1;
    return 1;
}

static u32 _MDrv_EMAC_ISR_TCOM(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;

    if (0 == (intstatus & EMAC_INT_TCOM))
        return 0;

    hemac = (struct emac_handle *)netdev_priv(dev);

#if PHY_LED_CTL
    // if(hemac->led_orange!=-1 && hemac->led_green!=-1)
    if (hemac->led_orange != -1)
    {
        if (hemac->led_count++ > hemac->led_flick_speed)
        {
            sstar_gpio_set_high(hemac->led_orange);
            hemac->led_count = 0;
        }
    }
#endif
    hemac->tx_irqcnt++;

    // The TCOM bit is set even if the transmission failed. //
    if (intstatus & (EMAC_INT_TUND | EMAC_INT_RTRY))
    {
        hemac->stats.tx_errors += 1;
        if (intstatus & EMAC_INT_TUND)
        {
            // write 1 clear
            //  Hal_EMAC_Write_TSR(EMAC_UND);

            // Reset TX engine
            Dev_EMAC_TxReset(dev);
            EMAC_ERR("Transmit TUND error, TX reset\n");
            hemac->irq_count[IDX_CNT_INT_TUND]++;
        }
        hemac->irq_count[IDX_CNT_INT_RTRY]++;
    }

#if TX_THROUGHPUT_TEST
    Dev_EMAC_tx(pseudo_packet, dev);
#endif

    hemac->irq_count[IDX_CNT_INT_TCOM]++;
    return 1;
}

#if TX_DELAY_INT
static u32 _MDrv_EMAC_ISR_TDLY(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;

    if (0 == (intstatus & EMAC_INT_TX_DELAY))
    {
        return 0;
    }

    hemac = (struct emac_handle *)netdev_priv(dev);

    hemac->irq_count[IDX_CNT_INT_TDLY]++;

#if !TX_DELAY_INT_DEBUG
    Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY, 0);
#endif
    Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY_TIMEOUT, 0);

    return 1;
}

static u32 _MDrv_EMAC_ISR_TDTO(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;

    if (0 == (intstatus & EMAC_INT_TX_DELAY_TIMEOUT))
    {
        return 0;
    }

    hemac = (struct emac_handle *)netdev_priv(dev);

    hemac->irq_count[IDX_CNT_INT_TDTO]++;

    tx_delay_pack_cnt = Hal_EMAC_PACKET_CNT(hemac->hal);

    Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY, 0);
    Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY_TIMEOUT, 0);

    return 1;
}
#endif

static u32 _MDrv_EMAC_ISR_DONE(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);

    if (0 == (intstatus & EMAC_INT_DONE))
        return 0;
    hemac->irq_count[IDX_CNT_INT_DONE]++;
    return 1;
}

static u32 _MDrv_EMAC_ISR_ROVR(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);

#if EMAC_FLOW_CONTROL_RX
#if EMAC_FLOW_CONTROL_RX_TEST
    {
        static int cnt = 0;
        cnt++;

        if (0 == (cnt & 0xF))
            _MDrv_EMAC_PausePkt_Send(dev);
        return 0;
    }
#endif
#endif

    if (0 == (intstatus & EMAC_INT_ROVR))
    {
        hemac->contiROVR = 0;
        return 0;
    }

    hemac->stats.rx_over_errors++;
    hemac->contiROVR++;
    // write 1 clear
    Hal_EMAC_Write_RSR(hemac->hal, EMAC_RSROVR);
#if EMAC_FLOW_CONTROL_RX
    if (_ethtool_rx_pause)
    {
        _MDrv_EMAC_PausePkt_Send(dev);
    }
#endif
    hemac->irq_count[IDX_CNT_INT_ROVR]++;
    return 0;
}

static u32 _MDrv_EMAC_ISR_RCOM(struct net_device *dev, u32 intstatus)
{
    struct emac_handle *hemac = NULL;

    hemac = (struct emac_handle *)netdev_priv(dev);

    if (intstatus & EMAC_INT_RCOM_DELAY)
    {
        hemac->irq_count[IDX_CNT_JULIAN_D]++;
    }
    else if (intstatus & EMAC_INT_RCOM)
    {
        intstatus |= EMAC_INT_RCOM_DELAY;
        hemac->irq_count[IDX_CNT_INT_RCOM]++;
    }

    // Receive complete //
    if (intstatus & EMAC_INT_RCOM_DELAY)
    {
        if ((0 == rx_time_last.tv_sec) && (0 == rx_time_last.tv_nsec))
        {
            ktime_get_real_ts64(&rx_time_last);
        }
        else
        {
            struct timespec64 ct;
            int               duration;

            ktime_get_real_ts64(&ct);
            duration        = (ct.tv_sec - rx_time_last.tv_sec) * 1000 + (ct.tv_nsec - rx_time_last.tv_nsec) / 1000000;
            rx_duration_max = (rx_duration_max < duration) ? duration : rx_duration_max;
            rx_time_last    = ct;
        }

#if PHY_LED_CTL
        // if(hemac->led_orange!=-1 && hemac->led_green!=-1)
        if (hemac->led_orange != -1)
        {
            if (hemac->led_count++ > hemac->led_flick_speed)
            {
                sstar_gpio_set_high(hemac->led_orange);
                hemac->led_count = 0;
            }
        }
#endif

#if SSTAR_EMAC_NAPI
        /* Receive packets are processed by poll routine. If not running start it now. */
        if (napi_schedule_prep(&hemac->napi))
        {
            // MDEV_EMAC_DISABLE_RX_REG();
            Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_RCOM, 0);
            __napi_schedule(&hemac->napi);
        }
        else
        {
            // printk("[%s][%d] NAPI RX cannot be scheduled\n", __FUNCTION__, __LINE__);
        }
#elif defined ISR_BOTTOM_HALF
        /*Triger rx_task*/
        schedule_work(&hemac->rx_task);
#else
        {
            unsigned long flags;
            spin_lock_irqsave(&hemac->mutexRXInt, flags);
            Dev_EMAC_rx(dev, 0x0FFFFFFF);
            spin_unlock_irqrestore(&hemac->mutexRXInt, flags);
        }
#endif
    }
    return intstatus;
}

irqreturn_t Dev_EMAC_interrupt(int irq, void *dev_id)
{
    struct net_device * dev       = (struct net_device *)dev_id;
    struct emac_handle *hemac     = (struct emac_handle *)netdev_priv(dev);
    u32                 intstatus = 0;

    hemac->irqcnt++;
    hemac->oldTime = getCurMs();
    _Dev_EMAC_tx_pump(hemac, 1, 0);
    while ((intstatus = Hal_EMAC_IntStatus(hemac->hal)))
    {
        _MDrv_EMAC_ISR_RBNA(dev, intstatus);
        _MDrv_EMAC_ISR_TCOM(dev, intstatus);
        _MDrv_EMAC_ISR_DONE(dev, intstatus);
        _MDrv_EMAC_ISR_ROVR(dev, intstatus);
        _MDrv_EMAC_ISR_RCOM(dev, intstatus);
#if TX_DELAY_INT
        _MDrv_EMAC_ISR_TDLY(dev, intstatus);
        _MDrv_EMAC_ISR_TDTO(dev, intstatus);
#endif
    }

    if (netif_queue_stopped(dev) && skb_queue_free(&hemac->skb_queue_tx, 0))
    {
#if DYNAMIC_INT_TX
#if TX_DELAY_INT
        if (tx_delay_debug == 0)
        {
            Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY, 0);
            Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY_TIMEOUT, 0);
        }
#else
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 0);
#endif
#endif
#if EMAC_FLOW_CONTROL_TX
        if (0 == timer_pending(&hemac->timerFlowTX))
            netif_wake_queue(dev);
#else // #if EMAC_FLOW_CONTROL_TX
        netif_wake_queue(dev);
#endif
    }

    return IRQ_HANDLED;
}

#if DYNAMIC_INT_RX
#define emac_time_elapse(start)                                                                     \
    (                                                                                               \
        {                                                                                           \
            unsigned long     delta;                                                                \
            struct timespec64 ct;                                                                   \
            ktime_get_real_ts64(&ct);                                                               \
            delta = (ct.tv_sec - (start).tv_sec) * 1000 + (ct.tv_nsec - (start).tv_nsec) / 1000000; \
            (delta);                                                                                \
        })
#endif // #if DYNAMIC_INT_RX

#if SSTAR_EMAC_NAPI
static int Dev_EMAC_napi_poll(struct napi_struct *napi, int budget)
{
    struct emac_handle *hemac      = container_of(napi, struct emac_handle, napi);
    struct net_device * dev        = hemac->netdev;
    int                 work_done  = 0;
    int                 budget_rmn = budget;
    rx_desc_queue_t *   rxinfo     = &(hemac->rx_desc_queue);
    struct rbf_t *      desc;
#if DYNAMIC_INT_RX
    unsigned long elapse  = 0;
    unsigned long packets = 0;
#endif // #if DYNAMIC_INT_RX

    // rx_poll_again:
    work_done = Dev_EMAC_rx(dev, budget_rmn);

    if (work_done)
    {
        // budget_rmn -= work_done;
        // goto rx_poll_again;
    }
#if DYNAMIC_INT_RX
    if (hemac->rx_stats_enable)
    {
        if (0xFFFFFFFF == hemac->rx_stats_packet)
        {
            ktime_get_real_ts64(&hemac->rx_stats_time);
            hemac->rx_stats_packet = 0;
        }
        hemac->rx_stats_packet += work_done;
        if ((elapse = emac_time_elapse(hemac->rx_stats_time)) >= 1000)
        {
            packets = hemac->rx_stats_packet;
            packets *= 1000;
            packets /= elapse;

#if REDUCE_CPU_FOR_RBNA
            {
                unsigned long flags;
                spin_lock_irqsave(&hemac->mutexIntRX, flags);
                if (0 == timer_pending(&hemac->timerIntRX))
                {
#endif // #if REDUCE_CPU_FOR_RBNA
       // printk("[%s][%d] packet for delay number (elapse, current, packet) = (%d, %d, %d)\n", __FUNCTION__, __LINE__,
       // (int)elapse, (int)hemac->rx_stats_packet, (int)packets); Hal_EMAC_RX_ParamSet(hemac->hal, packets/200 + 2,
       // 0xFFFFFFFF);
                    Hal_EMAC_RX_ParamSet(hemac->hal, packets / 200 + 1, 0xFFFFFFFF);
#if REDUCE_CPU_FOR_RBNA
                }
                spin_unlock_irqrestore(&hemac->mutexIntRX, flags);
            }
#endif // #if REDUCE_CPU_FOR_RBNA
            hemac->rx_stats_packet = 0;
            ktime_get_real_ts64(&hemac->rx_stats_time);
        }
    }
#endif

    /*
        if (work_done == budget_rmn)
            return budget;
    */

    napi_gro_flush(napi, false);

    /* If budget not fully consumed, exit the polling mode */
    if (work_done < budget)
    {
        napi_complete(napi);
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_RCOM, 1);
        desc = &(rxinfo->desc[rxinfo->idx]);
        if (desc->addr & EMAC_DESC_DONE)
        {
            /* If had new packet in,reschedule */
            Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_RCOM, 0);
            napi_reschedule(napi);
        }
    }
    return work_done;
}
#endif

#ifdef LAN_ESD_CARRIER_INTERRUPT
irqreturn_t Dev_EMAC_interrupt_cable_unplug(int irq, void *dev_instance)
{
    struct net_device * dev   = (struct net_device *)dev_instance;
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    unsigned long       flags;

    if (netif_carrier_ok(dev))
        netif_carrier_off(dev);
    if (!netif_queue_stopped(dev))
        netif_stop_queue(dev);
    hemac->ThisBCE.connected = 0;

#ifdef TX_SW_QUEUE
    _Dev_EMAC_tx_reset_TX_SW_QUEUE(dev);
#endif
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_reset(&(hemac->skb_queue_tx));
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);

#if PHY_LED_CTL
    // if (hemac->led_orange!=-1 && hemac->led_green!=-1)
    if (hemac->led_orange != -1)
    {
        sstar_gpio_set_low(hemac->led_orange);
    }
    if (hemac->led_green != -1)
    {
        sstar_gpio_set_low(hemac->led_green);
    }
#endif

    return IRQ_HANDLED;
}
#endif

//-------------------------------------------------------------------------------------------------
// EMAC Hardware register set
//-------------------------------------------------------------------------------------------------
void Dev_EMAC_HW_init(struct net_device *dev)
{
    struct emac_handle *hemac        = (struct emac_handle *)netdev_priv(dev);
    u32                 word_ETH_CTL = 0x00000000UL;
    // u32 word_ETH_CFG = 0x00000800UL;
    // u32 uNegPhyVal = 0;
    u32 idxRBQP = 0;
    // u32 RBQP_offset = 0;
    struct sk_buff *skb = NULL;
    // u32 RBQP_rx_skb_addr = 0;
    // unsigned long flags;

    // (20071026_CHARLES) Disable TX, RX and MDIO:   (If RX still enabled, the RX buffer will be overwrited)
    Hal_EMAC_Write_CTL(hemac->hal, word_ETH_CTL);
    Hal_EMAC_Write_BUFF(hemac->hal, 0x00000000UL);
    // Set MAC address ------------------------------------------------------
    Hal_EMAC_Write_SA1_MAC_Address(hemac->hal, hemac->sa[0][0], hemac->sa[0][1], hemac->sa[0][2], hemac->sa[0][3],
                                   hemac->sa[0][4], hemac->sa[0][5]);
    Hal_EMAC_Write_SA2_MAC_Address(hemac->hal, hemac->sa[1][0], hemac->sa[1][1], hemac->sa[1][2], hemac->sa[1][3],
                                   hemac->sa[1][4], hemac->sa[1][5]);
    Hal_EMAC_Write_SA3_MAC_Address(hemac->hal, hemac->sa[2][0], hemac->sa[2][1], hemac->sa[2][2], hemac->sa[2][3],
                                   hemac->sa[2][4], hemac->sa[2][5]);
    Hal_EMAC_Write_SA4_MAC_Address(hemac->hal, hemac->sa[3][0], hemac->sa[3][1], hemac->sa[3][2], hemac->sa[3][3],
                                   hemac->sa[3][4], hemac->sa[3][5]);

    // spin_lock_irqsave(&hemac->mutexRXD, flags);
    {
        struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(dev);
        rx_desc_queue_t *   rxinfo = &(hemac->rx_desc_queue);

        // Initialize Receive Buffer Descriptors
        memset(rxinfo->desc, 0x00, rxinfo->size_desc_queue);

        for (idxRBQP = 0; idxRBQP < rxinfo->num_desc; idxRBQP++)
        {
            struct rbf_t *desc = &(rxinfo->desc[idxRBQP]);

            if (!(skb = alloc_skb(EMAC_PACKET_SIZE_MAX, GFP_ATOMIC)))
            {
                // printk("%s %d: alloc skb failed!\n",__func__, __LINE__);
                panic("can't alloc skb");
            }
            rxinfo->skb_arr[idxRBQP] = skb;
#if RX_DESC_API
            RX_DESC_MAKE(desc, VIRT2BUS(skb->data), (idxRBQP < (rxinfo->num_desc - 1)) ? 0 : EMAC_DESC_WRAP);
#else
#if EXT_PHY_PATCH
            if (IS_EXT_PHY(hemac))
            {
                if (idxRBQP < (rxinfo->num_desc - 1))
                    desc->addr = PA2BUS(rxinfo->skb_phys + idxRBQP * EMAC_PACKET_SIZE_MAX);
                else
                    desc->addr = PA2BUS(rxinfo->skb_phys + idxRBQP * EMAC_PACKET_SIZE_MAX) | EMAC_DESC_WRAP;
            }
            else
#endif
            {
                if (idxRBQP < (rxinfo->num_desc - 1))
                    desc->addr = VIRT2BUS(skb->data);
                else
                    desc->addr = VIRT2BUS(skb->data) | EMAC_DESC_WRAP;
            }
#endif
        }
        Chip_Flush_MIU_Pipe();
        // Initialize "Receive Buffer Queue Pointer"
        Hal_EMAC_Write_RBQP(hemac->hal, PA2BUS(rxinfo->desc_phys));
    }
    // spin_unlock_irqrestore(&hemac->mutexRXD, flags);

    Hal_EMAC_HW_init(hemac->hal);
}

static void *Dev_EMAC_RX_Desc_Init(struct emac_handle *hemac, void *p)
{
    rx_desc_queue_t *rxinfo = &(hemac->rx_desc_queue);

    rxinfo->num_desc        = RX_DESC_NUM;
    rxinfo->size_desc_queue = RX_DESC_QUEUE_SIZE;
    rxinfo->desc            = (struct rbf_t *)p;

    // EMAC_DBG("alloRAM_VA_BASE=0x%zx alloRAM_PA_BASE=0x%zx\n  alloRAM_SIZE=0x%zx\n", (size_t) rxinfo->desc,(size_t)
    // rxinfo->descPhys,(size_t)rxinfo->size_desc_queue);
    BUG_ON(!rxinfo->desc);

    rxinfo->idx = 0;
    memset(rxinfo->desc, 0x00, rxinfo->size_desc_queue);
    Chip_Flush_MIU_Pipe();

    rxinfo->skb_arr = kzalloc(rxinfo->num_desc * sizeof(struct sk_buff *), GFP_KERNEL);
    BUG_ON(!rxinfo->skb_arr);

    EMAC_DBG("RAM_VA_BASE=0x%08lx\n", (unsigned long)rxinfo->desc);
    EMAC_DBG("RAM_PA_BASE=0x%08lx\n", (unsigned long)hemac->rx_desc_queue.desc_phys);
    // EMAC_DBG("RAM_VA_PA_OFFSET=0x%08llx\n", rxinfo->off_va_pa);
    EMAC_DBG("RBQP_BASE=0x%08lx size=0x%x\n", (unsigned long)PA2BUS(hemac->rx_desc_queue.desc_phys),
             rxinfo->size_desc_queue);

    return (void *)(((long)p) + RX_DESC_QUEUE_SIZE);
}

static void Dev_EMAC_RX_Desc_Free(struct emac_handle *hemac)
{
    rx_desc_queue_t *rxinfo = &(hemac->rx_desc_queue);

    if (rxinfo->skb_arr)
    {
        kfree(rxinfo->skb_arr);
        rxinfo->skb_arr = NULL;
    }

    rxinfo->desc            = NULL;
    rxinfo->idx             = 0;
    rxinfo->num_desc        = 0;
    rxinfo->size_desc_queue = 0;
}

static void Dev_EMAC_RX_Desc_Reset(struct emac_handle *hemac)
{
    rx_desc_queue_t *rxinfo = &(hemac->rx_desc_queue);

    rxinfo->idx = 0;
    memset(rxinfo->desc, 0x00, rxinfo->size_desc_queue);
    Chip_Flush_MIU_Pipe();
}

static void Dev_EMAC_MemFree(struct emac_handle *hemac)
{
    if (hemac->mem_info.length)
    {
        msys_release_dmem(&hemac->mem_info);
        memset(&hemac->mem_info, 0, sizeof(hemac->mem_info));
    }
}

static void *Dev_EMAC_MemAlloc(struct emac_handle *hemac, u32 size)
{
    int ret;

    hemac->mem_info.length = size;
    // strcpy(hemac->mem_info.name, "EMAC_BUFF");
    sprintf(hemac->mem_info.name, "%s_buff", hemac->name);
    if ((ret = msys_request_dmem(&hemac->mem_info)))
    {
        memset(&hemac->mem_info, 0, sizeof(hemac->mem_info));
        panic("unable to locate DMEM for EMAC alloRAM!! %d\n", size);
        return NULL;
    }
    return (void *)((size_t)hemac->mem_info.kvirt);
}

//-------------------------------------------------------------------------------------------------
// EMAC init Variable
//-------------------------------------------------------------------------------------------------
// extern phys_addr_t memblock_start_of_DRAM(void);
// extern phys_addr_t memblock_size_of_first_region(void);

static void *Dev_EMAC_VarInit(struct emac_handle *hemac)
{
    char  addr[6];
    u32   HiAddr, LoAddr;
    int   txd_len;
    void *p = NULL;

#if EMAC_FLOW_CONTROL_RX
    static u8 pause_pkt[]               = {// DA - multicast
                             0x01, 0x80, 0xC2, 0x00, 0x00, 0x01,
                             // SA
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             // Len-Type
                             0x88, 0x08,
                             // Ctrl code
                             0x00, 0x01,
                             // Ctrl para 8192
                             0x20, 0x00};
    int       pausePktSize              = sizeof(pause_pkt);
    static u8 ETH_PAUSE_FRAME_DA_MAC[6] = {0x01UL, 0x80UL, 0xC2UL, 0x00UL, 0x00UL, 0x01UL};
#else
    int pausePktSize = 0;
#endif // #if EMAC_FLOW_CONTROL_RX

    // TXD init
    txd_len = Hal_EMAC_TXD_Cfg(hemac->hal, hemac->txd_num);
#if EXT_PHY_PATCH
    if (IS_EXT_PHY(hemac))
    {
#if (EMAC_SG && EMAC_SG_BUF_CACHE)
        p = Dev_EMAC_MemAlloc(hemac,
                              RX_DESC_QUEUE_SIZE + (RX_DESC_NUM * EMAC_PACKET_SIZE_MAX) + txd_len + pausePktSize);
#else
        p = Dev_EMAC_MemAlloc(hemac,
                              RX_DESC_QUEUE_SIZE + (RX_DESC_NUM * EMAC_PACKET_SIZE_MAX) + txd_len + pausePktSize);
#endif
    }
    else
#endif // #if EXT_PHY_PATCH
    {
#if (EMAC_SG && EMAC_SG_BUF_CACHE)
        p = Dev_EMAC_MemAlloc(hemac, RX_DESC_QUEUE_SIZE + txd_len + pausePktSize);
#else
        p = Dev_EMAC_MemAlloc(hemac, RX_DESC_QUEUE_SIZE + txd_len + pausePktSize);
#endif
    }
    if (NULL == p)
    {
        printk("[%s][%d] alloc memory fail %ld\n", __FUNCTION__, __LINE__, (long)(RX_DESC_QUEUE_SIZE + txd_len));
        return NULL;
    }

    /*
    #if (EMAC_SG && EMAC_SG_BUF_CACHE)
        if (NULL == (hemac->pTxBuf = kmalloc(txBufSize, GFP_KERNEL)))
        {
            printk("[%s][%d] kmalloc fail %d\n", __FUNCTION__, __LINE__, txBufSize);
            Dev_EMAC_MemFree(hemac);
            return NULL;
        }
    #endif
    */
    if (txd_len)
    {
        Hal_EMAC_TXD_Buf(hemac->hal, p, VIRT2BUS(p), txd_len);
        p = (void *)(((size_t)p) + txd_len);
    }
    hemac->rx_desc_queue.desc_phys = hemac->mem_info.phys + txd_len;

#if EXT_PHY_PATCH
    {
        long start, end;
        start                         = (long)hemac->rx_desc_queue.desc_phys;
        p                             = Dev_EMAC_RX_Desc_Init(hemac, p);
        hemac->rx_desc_queue.skb_phys = hemac->rx_desc_queue.desc_phys + RX_DESC_QUEUE_SIZE;
        if (IS_EXT_PHY(hemac))
        {
            hemac->pu8RXBuf = (char *)p;
            p               = (void *)(hemac->pu8RXBuf + (RX_DESC_NUM * EMAC_PACKET_SIZE_MAX));
            end = (long)(hemac->rx_desc_queue.desc_phys + RX_DESC_QUEUE_SIZE + (RX_DESC_NUM * EMAC_PACKET_SIZE_MAX));
            Hal_EMAC_MIU_Protect_RX(hemac->hal, PA2BUS(start), PA2BUS(end));
        }
        else
        {
            hemac->pu8RXBuf = NULL;
        }
    }
#else
    p = Dev_EMAC_RX_Desc_Init(hemac, p);
#endif

#if EMAC_FLOW_CONTROL_RX
    hemac->isPausePkt     = 0;
    hemac->u8PausePktSize = pausePktSize;
    hemac->pu8PausePkt    = (pausePktSize) ? p : NULL;
    if (pausePktSize)
    {
        memcpy(hemac->pu8PausePkt, pause_pkt, pausePktSize);
    }
    p = (void *)(((char *)p) + pausePktSize);
#endif // #if EMAC_FLOW_CONTROL_RX

#if EMAC_SG
    /*
        if (NULL == hemac->pTxBuf)
            hemac->pTxBuf = (txBufSize) ? (char*)p : NULL;
        hemac->TxBufIdx = 0;
    */
    hemac->maxSG = 0;
#endif // #if EMAC_SG

    memset(hemac->sa, 0, sizeof(hemac->sa));

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = Hal_EMAC_get_SA1H_addr(hemac->hal);
    LoAddr = Hal_EMAC_get_SA1L_addr(hemac->hal);

    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr(addr))
    {
        memcpy(&hemac->sa[0][0], &addr, 6);
    }
    else
    {
        // Check if bootloader set address in Specific-Address 2 //
        HiAddr  = Hal_EMAC_get_SA2H_addr(hemac->hal);
        LoAddr  = Hal_EMAC_get_SA2L_addr(hemac->hal);
        addr[0] = (LoAddr & 0xffUL);
        addr[1] = (LoAddr & 0xff00UL) >> 8;
        addr[2] = (LoAddr & 0xff0000UL) >> 16;
        addr[3] = (LoAddr & 0xff000000UL) >> 24;
        addr[4] = (HiAddr & 0xffUL);
        addr[5] = (HiAddr & 0xff00UL) >> 8;

        if (is_valid_ether_addr(addr))
        {
            memcpy(&hemac->sa[0][0], &addr, 6);
        }
        else
        {
            memcpy(&hemac->sa[0][0], MY_MAC, 6);
        }
    }
#if EMAC_FLOW_CONTROL_RX
    memcpy(&hemac->sa[1][0], ETH_PAUSE_FRAME_DA_MAC, 6);
#endif
    return p;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void Dev_EMAC_netpoll(struct net_device *dev)
{
    unsigned long flags;

    local_irq_save(flags);
    Dev_EMAC_interrupt(dev->irq, dev);
    local_irq_restore(flags);
}
#endif

#if KERNEL_PHY
static void emac_phy_link_adjust(struct net_device *dev)
{
    int                 cam          = 0; // 0:No CAM, 1:Yes
    int                 rcv_bcast    = 1; // 0:No, 1:Yes
    int                 rlf          = 0;
    u32                 word_ETH_CFG = 0x00000800UL;
    struct emac_handle *hemac        = (struct emac_handle *)netdev_priv(dev);
    unsigned long       flag1;

    spin_lock_irqsave(&hemac->mutexNetIf, flag1);
    if (!hemac->bEthCfg)
    {
        // ETH_CFG Register -----------------------------------------------------
        // (20070808) IMPORTANT: REG_ETH_CFG:bit1(FD), 1:TX will halt running RX frame, 0:TX will not halt running RX
        // frame. If always set FD=0, no CRC error will occur. But throughput maybe need re-evaluate. IMPORTANT:
        // (20070809) NO_MANUAL_SET_DUPLEX : The real duplex is returned by "negotiation"
        word_ETH_CFG = 0x00000800UL; // Init: CLK = 0x2
        if (SPEED_100 == dev->phydev->speed)
            word_ETH_CFG |= 0x00000001UL;
        if (DUPLEX_FULL == dev->phydev->duplex)
            word_ETH_CFG |= 0x00000002UL;
        if (cam)
            word_ETH_CFG |= 0x00000200UL;
        if (0 == rcv_bcast)
            word_ETH_CFG |= 0x00000020UL;
        if (1 == rlf)
            word_ETH_CFG |= 0x00000100UL;

        Hal_EMAC_Write_CFG(hemac->hal, word_ETH_CFG);
        hemac->bEthCfg = 1;
    }

#if EMAC_LOCAL_TEST
    if (keep_link_up)
    {
        dev->phydev->link = 1;
    }
#endif

    if (dev->phydev->link)
    {
        Hal_EMAC_update_speed_duplex(hemac->hal, dev->phydev->speed, dev->phydev->duplex);
        netif_carrier_on(dev);
        netif_start_queue(dev);
#if PHY_LED_CTL
        if (hemac->led_green != -1)
        {
            sstar_gpio_set_high(hemac->led_green);
        }
#endif
        printk("[%s] EMAC Link Up \n", __FUNCTION__);
    }
    else
    {
        // unsigned long flags;

        if (!netif_queue_stopped(dev))
            netif_stop_queue(dev);
        if (netif_carrier_ok(dev))
            netif_carrier_off(dev);
            // spin_lock_irqsave(&hemac->mutexTXQ, flags);
            // skb_queue_reset(&(hemac->skb_queue_tx));
            // spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
#if PHY_LED_CTL
        if (hemac->led_green != -1)
        {
            sstar_gpio_set_low(hemac->led_green);
        }
#endif
        printk("[%s] EMAC Link Down \n", __FUNCTION__);
    }
    spin_unlock_irqrestore(&hemac->mutexNetIf, flag1);

#if 0
    printk("[%s][%d] adjust phy (link, speed, duplex) = (%d, %d, %d, %d)\n", __FUNCTION__, __LINE__,
        dev->phydev->link, dev->phydev->speed, dev->phydev->duplex, dev->phydev->autoneg);
#endif
}

static int emac_phy_connect(struct net_device *netdev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(netdev);
    struct device_node *np    = NULL;
    struct phy_device * phydev;
    __ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = {
        0,
    };

    np = of_parse_phandle(netdev->dev.of_node, "phy-handle", 0);
    if (!np && of_phy_is_fixed_link(netdev->dev.of_node))
        if (!of_phy_register_fixed_link(netdev->dev.of_node))
            np = of_node_get(netdev->dev.of_node);
    if (!np)
    {
        printk("[%s][%d] can not find phy-handle in dts\n", __FUNCTION__, __LINE__);
        return -ENODEV;
    }

    if (!(phydev = of_phy_connect(netdev, np, emac_phy_link_adjust, 0, hemac->phy_mode)))
    {
        printk("[%s][%d] could not connect to PHY\n", __FUNCTION__, __LINE__);
        goto jmp_err_connect;
    }

    phy_init_hw(phydev);

    printk("[%s][%d] connected mac %s to PHY at %s [uid=%08x, driver=%s]\n", __FUNCTION__, __LINE__, hemac->name,
           phydev_name(phydev), phydev->phy_id, phydev->drv->name);

    netdev->phydev->autoneg = AUTONEG_ENABLE;
    netdev->phydev->speed   = 0;
    netdev->phydev->duplex  = 0;

    // the speed has to be limited to 10 MBits/sec in FPGA
    {
        u32 max_speed = 0;
        if (!of_property_read_u32(netdev->dev.of_node, "max-speed", &max_speed))
        {
            switch (max_speed)
            {
                case 10:
                    phy_set_max_speed(phydev, SPEED_10);
                    break;
                case 100:
                    phy_set_max_speed(phydev, SPEED_100);
                    break;
                default:
                    break;
            }
        }
    }
#if (EMAC_FLOW_CONTROL_TX || EMAC_FLOW_CONTROL_RX)
    phy_set_max_speed(netdev->phydev, SPEED_100);
    phy_support_asym_pause(netdev->phydev);
#else
    phy_set_max_speed(netdev->phydev, SPEED_100);
#endif
    ethtool_convert_legacy_u32_to_link_mode(mask, ADVERTISED_Autoneg);
    linkmode_or(netdev->phydev->advertising, netdev->phydev->advertising, mask);

    if (0 > phy_start_aneg(netdev->phydev))
    {
        printk("[%s][%d] phy_start_aneg fail\n", __FUNCTION__, __LINE__);
    }
    of_node_put(np);
    return 0;

jmp_err_connect:
    if (of_phy_is_fixed_link(netdev->dev.of_node))
        of_phy_deregister_fixed_link(netdev->dev.of_node);
    of_node_put(np);
    printk("[%s][%d]: invalid phy\n", __FUNCTION__, __LINE__);
    return -EINVAL;
}
#endif

static int __init Dev_EMAC_ndo_init(struct net_device *dev)
{
#if KERNEL_PHY
    return emac_phy_connect(dev);
#endif
    return 0;
}

static void Dev_EMAC_ndo_uninit(struct net_device *dev)
{
#if KERNEL_PHY
    phy_disconnect(dev->phydev);
#endif
}

int Dev_EMAC_ndo_change_mtu(struct net_device *dev, int new_mtu)
{
    if ((new_mtu < 68) || (new_mtu > EMAC_MTU))
    {
        printk("[%s][%d] not support mtu size %d\n", __FUNCTION__, __LINE__, new_mtu);
        return -EINVAL;
    }
    printk("[%s][%d] change mtu size from %d to %d\n", __FUNCTION__, __LINE__, dev->mtu, new_mtu);
    dev->mtu = new_mtu;
    return 0;
}

static int sstar_emac_get_link_ksettings(struct net_device *ndev, struct ethtool_link_ksettings *cmd)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
    phy_ethtool_ksettings_get(ndev->phydev, cmd);
    return 0;
#else
    return phy_ethtool_ksettings_get(ndev->phydev, cmd);
#endif
}

static int sstar_emac_set_link_ksettings(struct net_device *ndev, const struct ethtool_link_ksettings *cmd)
{
#if PHY_FORCE_NEG_SET
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(ndev);

    if (hemac->phy_mode != PHY_INTERFACE_MODE_RMII)
    {
        Hal_EMAC_PHY_FORCE_MODE_REG_SET(hemac->hal, cmd);
    }
#endif
    return phy_ethtool_ksettings_set(ndev->phydev, cmd);
}

static int sstar_emac_nway_reset(struct net_device *dev)
{
    return genphy_restart_aneg(dev->phydev);
}

static u32 sstar_emac_get_link(struct net_device *dev)
{
    int err;

    err = genphy_update_link(dev->phydev);
    if (err)
        return ethtool_op_get_link(dev);
    return dev->phydev->link;
}

static void sstar_emac_get_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
    ASSERT_RTNL();

    wol->supported = 0;
    wol->wolopts   = 0;

    if (dev->phydev)
        phy_ethtool_get_wol(dev->phydev, wol);
}

static int sstar_emac_set_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    int                 ret   = -EOPNOTSUPP;

    if (!device_can_wakeup(hemac->sstar_class_emac_device))
        return ret;

    ASSERT_RTNL();

    if (dev->phydev)
        ret = phy_ethtool_set_wol(dev->phydev, wol);

    if (!ret)
        device_set_wakeup_enable(hemac->sstar_class_emac_device, !!wol->wolopts);

    return ret;
}

#if ETHTOOL_DEBUG
static void sstar_emac_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *stats, u64 *data)
{
    struct net_device_stats *emac_stats;

    emac_stats = Dev_EMAC_stats(dev);

    data[0]  = emac_stats->rx_packets;
    data[1]  = emac_stats->rx_frame_errors;
    data[2]  = emac_stats->rx_length_errors;
    data[3]  = emac_stats->rx_crc_errors;
    data[4]  = emac_stats->rx_fifo_errors;
    data[5]  = emac_stats->rx_errors;
    data[6]  = emac_stats->tx_packets;
    data[7]  = emac_stats->tx_fifo_errors;
    data[8]  = emac_stats->tx_carrier_errors;
    data[9]  = emac_stats->tx_heartbeat_errors;
    data[10] = emac_stats->tx_window_errors;
    data[11] = emac_stats->tx_aborted_errors;
    data[12] = emac_stats->collisions;
}

static void sstar_emac_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
    unsigned int i;

    switch (stringset)
    {
        case ETH_SS_STATS:
            for (i = 0; i < EMAC_STATS_LEN; i++)
            {
                strlcpy(data, stats_strings[i], ETH_GSTRING_LEN);
                data += ETH_GSTRING_LEN;
            }
            break;
        default:
            printk("[%s] emac get strings fail !\n", __func__);
            break;
    }
}

static int sstar_emac_get_sset_count(struct net_device *dev, int sset)
{
    switch (sset)
    {
        case ETH_SS_STATS:
            return ARRAY_SIZE(stats_strings);
        default:
            printk("[%s] emac get no set count !\n", __func__);
            return 0;
    }
}
#endif

#if ETHTOOL_DEBUG
static void sstar_emac_get_pause_param(struct net_device *ndev, struct ethtool_pauseparam *pauseparam)
{
    pauseparam->autoneg  = 0; // always off
    pauseparam->rx_pause = _ethtool_rx_pause;
    pauseparam->tx_pause = _ethtool_tx_pause;
}

static int sstar_emac_set_pause_param(struct net_device *ndev, struct ethtool_pauseparam *pauseparam)
{
    struct emac_handle *hemac = NULL;
    hemac                     = (struct emac_handle *)netdev_priv(ndev);

    _ethtool_rx_pause = pauseparam->rx_pause;
    _ethtool_tx_pause = pauseparam->tx_pause;

#if HW_FLOW_CONTROL
    if (_ethtool_tx_pause)
    {
        Hal_EMAC_TX_Flow_Ctrl_Enable(hemac->hal, TRUE);
    }
    else
    {
        Hal_EMAC_TX_Flow_Ctrl_Enable(hemac->hal, FALSE);
    }
    if (_ethtool_rx_pause)
    {
        Hal_EMAC_RX_Flow_Ctrl_Enable(hemac->hal, TRUE);
    }
    else
    {
        Hal_EMAC_RX_Flow_Ctrl_Enable(hemac->hal, FALSE);
    }
#endif

    return 0;
}
#endif

static int Dev_EMAC_set_features(struct net_device *netdev, netdev_features_t features)
{
    /*
    printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
            printk("[%s][%d] set features = 0x%08x\n", __FUNCTION__, __LINE__, (int)features);
    printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
    */
    netdev->features = features;
    return 0;
}

static netdev_features_t Dev_EMAC_fix_features(struct net_device *netdev, netdev_features_t features)
{
    /*
    printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
        printk("[%s][%d] fix features = 0x%08x 0x%08x \n", __FUNCTION__, __LINE__, (int)features, (int)(features &
    (EMAC_FEATURES))); printk("[%s][%d] ##################################\n", __FUNCTION__, __LINE__);
    */
    return (features & EMAC_FEATURES);
}

//-------------------------------------------------------------------------------------------------
// Initialize the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
static const struct net_device_ops sstar_lan_netdev_ops = {
    .ndo_init   = Dev_EMAC_ndo_init,
    .ndo_uninit = Dev_EMAC_ndo_uninit,
    // .ndo_tx_timeout         = Dev_EMAC_ndo_tx_timeout,
    .ndo_change_mtu      = Dev_EMAC_ndo_change_mtu,
    .ndo_open            = Dev_EMAC_open,
    .ndo_stop            = Dev_EMAC_close,
    .ndo_start_xmit      = Dev_EMAC_tx,
    .ndo_set_mac_address = Dev_EMAC_set_mac_address,
    .ndo_set_rx_mode     = Dev_EMAC_set_rx_mode,
    .ndo_do_ioctl        = Dev_EMAC_ioctl,
    .ndo_get_stats       = Dev_EMAC_stats,
    .ndo_set_features    = Dev_EMAC_set_features,
    .ndo_fix_features    = Dev_EMAC_fix_features,
#ifdef CONFIG_NET_POLL_CONTROLLER
    .ndo_poll_controller = Dev_EMAC_netpoll,
#endif

};

static const struct ethtool_ops sstar_emac_ethtool_ops = {
    .get_link_ksettings = sstar_emac_get_link_ksettings,
    .set_link_ksettings = sstar_emac_set_link_ksettings,
    //.get_drvinfo        = sstar_emac_get_drvinfo,
    //.get_msglevel       = sstar_emac_get_msglevel,
    //.set_msglevel       = sstar_emac_set_msglevel,
    .nway_reset = sstar_emac_nway_reset,
    .get_link   = sstar_emac_get_link,
    .get_wol    = sstar_emac_get_wol,
    .set_wol    = sstar_emac_set_wol,
#if ETHTOOL_DEBUG
    .get_strings       = sstar_emac_get_strings,
    .get_sset_count    = sstar_emac_get_sset_count,
    .get_ethtool_stats = sstar_emac_get_ethtool_stats,
#endif
//.get_rxnfc          = sstar_emac_get_rxnfc,
//.set_rxnfc          = sstar_emac_set_rxnfc,
#if ETHTOOL_DEBUG
    .get_pauseparam = sstar_emac_get_pause_param,
    .set_pauseparam = sstar_emac_set_pause_param,
#endif
};
#endif //#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)

static ssize_t dlist_info_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    u32                 input;
    int                 idx = 0;

    input = simple_strtoul(buf, NULL, 10);

    if (0 == input)
    {
        RBNA_detailed = 0;
    }
    else if (1 == input)
    {
        RBNA_detailed = 1;
    }
    else if (2 == input)
    {
        max_rx_packet_count    = 0;
        max_tx_packet_count    = 0;
        min_tx_fifo_idle_count = 0xffff;
    }
    else if (3 == input)
    {
        for (idx = 0; idx < sizeof(hemac->irq_count) / sizeof(u32); idx++)
            hemac->irq_count[idx] = 0;
    }

    return count;
}

static ssize_t dlist_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *              str          = buf;
    char *              end          = buf + PAGE_SIZE;
    int                 idx          = 0;
    struct net_device * netdev       = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac        = (struct emac_handle *)netdev_priv(netdev);
    rx_desc_queue_t *   rxinfo       = &(hemac->rx_desc_queue);
    int                 empty        = 0;
    int                 max          = rxinfo->num_desc;
    u32                 u32RBQP_Addr = 0;
    char                descriptor_maps[RX_DESC_NUM];

    for (idx = 0; idx < rxinfo->num_desc; idx++)
    {
        // if(!((dlist->descriptors[idx].addr) & EMAC_DESC_DONE))
        if (!((rxinfo->desc[idx].addr) & EMAC_DESC_DONE))
        {
            empty++;
            descriptor_maps[idx] = 1;
        }
        else
        {
            descriptor_maps[idx] = 0;
        }
    }
    // u32RBQP_Addr =( Hal_EMAC_Read_RBQP()-(RBQP_BASE - MIU0_BUS_BASE))/RBQP_HW_BYTES;
    u32RBQP_Addr = BUS2PA(Hal_EMAC_Read_RBQP(hemac->hal)) / RX_DESC_SIZE;
    str += scnprintf(str, end - str, "%s=0x%x\n", "RBQP_size", max);
    str += scnprintf(str, end - str, "empty=0x%x, hemac->rxBuffIndex=0x%x, u32RBQP_Addr=0x%x\n",
                     // empty, hemac->rxBuffIndex, u32RBQP_Addr);
                     empty, rxinfo->idx, u32RBQP_Addr);
#define CHANGE_LINE_LENG 0x20
    for (idx = 0; idx < rxinfo->num_desc; idx++)
    {
        if (idx % CHANGE_LINE_LENG == 0x0)
            str += scnprintf(str, end - str, "0x%03x: ", idx);

        str += scnprintf(str, end - str, "%d", descriptor_maps[idx]);

        if (idx % 0x10 == (0xF))
            str += scnprintf(str, end - str, " ");

        if (idx % CHANGE_LINE_LENG == (CHANGE_LINE_LENG - 1))
            str += scnprintf(str, end - str, "\n");
    }

    str += scnprintf(str, end - str, "%s=%d\n", "max_rx_packet_count", max_rx_packet_count);
    str += scnprintf(str, end - str, "%s=%d\n", "max_tx_packet_count", max_tx_packet_count);

    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_DONE", hemac->irq_count[IDX_CNT_INT_DONE]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_RCOM", hemac->irq_count[IDX_CNT_INT_RCOM]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_RBNA", hemac->irq_count[IDX_CNT_INT_RBNA]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TOVR", hemac->irq_count[IDX_CNT_INT_TOVR]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TUND", hemac->irq_count[IDX_CNT_INT_TUND]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_RTRY", hemac->irq_count[IDX_CNT_INT_RTRY]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TCOM", hemac->irq_count[IDX_CNT_INT_TCOM]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_ROVR", hemac->irq_count[IDX_CNT_INT_ROVR]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_JULIAN_D", hemac->irq_count[IDX_CNT_JULIAN_D]);
#if TX_DELAY_INT
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TDLY", hemac->irq_count[IDX_CNT_INT_TDLY]);
    str += scnprintf(str, end - str, "%s=%d\n", "IDX_CNT_INT_TDTO", hemac->irq_count[IDX_CNT_INT_TDTO]);
#endif
    str += scnprintf(str, end - str, "%s=%llu\n", "skb_tx_send", hemac->skb_tx_send);
    str += scnprintf(str, end - str, "%s=%llu\n", "skb_tx_free", hemac->skb_tx_free);
    str += scnprintf(str, end - str, "%s=%d\n", "rx_duration_max", rx_duration_max);
    str += scnprintf(str, end - str, "%s=%llu\n", "rx_packet_cnt", rx_packet_cnt);
#if TX_DELAY_INT
    str += scnprintf(str, end - str, "%s=%d\n", "tx_delay_pack_cnt", tx_delay_pack_cnt);
#endif

    {
        struct timespec64 ct;
        int               duration;
        u64               data_done_ct;
        unsigned long     flags;
        u32               txPkt_ct  = hemac->skb_tx_send;
        u32               txInt_ct  = hemac->irq_count[IDX_CNT_INT_TCOM];
        u32               txInt_dly = hemac->irq_count[IDX_CNT_INT_TDLY];
        u32               txInt_to  = hemac->irq_count[IDX_CNT_INT_TDTO];
        u32               rxInt_dly = hemac->irq_count[IDX_CNT_JULIAN_D];

        ktime_get_real_ts64(&ct);
        duration =
            (ct.tv_sec - hemac->data_time_last.tv_sec) * 1000 + (ct.tv_nsec - hemac->data_time_last.tv_nsec) / 1000000;
        spin_lock_irqsave(&hemac->emac_data_done_lock, flags);
        data_done_ct     = hemac->data_done;
        hemac->data_done = 0;
        spin_unlock_irqrestore(&hemac->emac_data_done_lock, flags);

        // tx_duration_max = (tx_duration_max < duration) ? duration : tx_duration_max;
        hemac->data_time_last = ct;
        str += scnprintf(str, end - str, "%s=%lld\n", "data_done", data_done_ct);
        str += scnprintf(str, end - str, "%s=%d\n", "data_duration", duration);
        do_div(data_done_ct, duration);
        str += scnprintf(str, end - str, "%s=%lld\n", "data_average", data_done_ct);
        str += scnprintf(str, end - str, "%s=%d\n", "tx_pkt (duration)", txPkt_ct - hemac->txPkt);
        str += scnprintf(str, end - str, "%s=%d\n", "tx_int (duration)", txInt_ct - hemac->txInt);
        str += scnprintf(str, end - str, "%s=%d\n", "tx_int_dly (duration)", txInt_dly - hemac->txIntDly);
        str += scnprintf(str, end - str, "%s=%d\n", "tx_int_to (duration)", txInt_to - hemac->txIntTo);
        str += scnprintf(str, end - str, "%s=%d\n", "rx_int_dly (duration)", rxInt_dly - hemac->rxIntDly);
        str += scnprintf(str, end - str, "%s=%llu\n", "rx_pkt (duration)", rx_packet_cnt - hemac->rxPkt);

        hemac->txPkt    = txPkt_ct;
        hemac->txInt    = txInt_ct;
        hemac->txIntDly = txInt_dly;
        hemac->txIntTo  = txInt_to;
        hemac->rxIntDly = rxInt_dly;
        hemac->rxPkt    = rx_packet_cnt;
    }
    str += scnprintf(str, end - str, "%s=%d\n", "Hal_EMAC_TXQ_Mode", Hal_EMAC_TXQ_Mode(hemac->hal));
#if EMAC_SG
    str += scnprintf(str, end - str, "%s=%d\n", "maxSG", hemac->maxSG);
#endif // #if #if EMAC_SG
    return (str - buf);
}
DEVICE_ATTR(dlist_info, 0644, dlist_info_show, dlist_info_store);

static ssize_t info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *              str    = buf;
    char *              end    = buf + PAGE_SIZE;
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    rx_desc_queue_t *   rxinfo = &(hemac->rx_desc_queue);
    int                 clk    = 0;

    // str += scnprintf(str, end - str, "%s %s\n", __DATE__, __TIME__);
    str += scnprintf(str, end - str, "RAM_ALLOC_SIZE=0x%08x\n", rxinfo->size_desc_queue);
    str += scnprintf(str, end - str, "RAM_VA_BASE=0x%08lx\n", (long)rxinfo->desc);
    str += scnprintf(str, end - str, "RAM_PA_BASE=0x%08lx\n", (long)VIRT2PA(rxinfo->desc));

#if SSTAR_EMAC_NAPI
    str += scnprintf(str, end - str, "NAPI enabled, NAPI_weight=%d\n", EMAC_NAPI_WEIGHT);
#endif
    str += scnprintf(str, end - str, "ZERO_COPY enabled\n");
#ifdef NEW_TX_QUEUE_INTERRUPT_THRESHOLD
    str += scnprintf(str, end - str, "NEW_TX_QUEUE_INTERRUPT_THRESHOLD enabled\n");
#endif

    clk = Hal_EMAC_ReadReg8(EMAC_RIU_REG_BASE, REG_BANK_CLKGEN0, 0x84);
    clk = clk & 0x0F;
    if (clk == 0)
        str += scnprintf(str, end - str, "Emac clock 144m\n");
    else if (clk == 4)
        str += scnprintf(str, end - str, "Emac clock 123m\n");
    else if (clk == 8)
        str += scnprintf(str, end - str, "Emac clock 86m\n");
    else if (clk == 12)
        str += scnprintf(str, end - str, "Emac clock 125m\n");
    else
        str += scnprintf(str, end - str, "Emac clock invalid\n");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}
DEVICE_ATTR(info, 0444, info_show, NULL);

// struct timeval proc_read_time;
static ssize_t tx_sw_queue_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device * netdev    = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac     = (struct emac_handle *)netdev_priv(netdev);
    char *              str       = buf;
    char *              end       = buf + PAGE_SIZE;
    int                 idleCount = 0;

    idleCount = Hal_EMAC_TXQ_Free(hemac->hal);
    str += scnprintf(str, end - str,
                     "netif_queue_stopped=%d \n idleCount=%d \n irqcnt=%d, tx_irqcnt=%d \n tx_bytes_per_timerbak=%d \n "
                     "min_tx_fifo_idle_count=%d \n",
                     netif_queue_stopped(netdev), idleCount, hemac->irqcnt, hemac->tx_irqcnt, tx_bytes_per_timerbak,
                     min_tx_fifo_idle_count);

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

DEVICE_ATTR(tx_sw_queue_info, 0444, tx_sw_queue_info_show, NULL);

static ssize_t reverse_led_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    u32                 input;

    input = simple_strtoul(buf, NULL, 10);
    Hal_EMAC_Set_Reverse_LED(hemac->hal, input);
    return count;
}
static ssize_t reverse_led_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    u8                  u8reg  = 0;
    u8reg                      = Hal_EMAC_Get_Reverse_LED(hemac->hal);
    return sprintf(buf, "%d\n", u8reg);
}
DEVICE_ATTR(reverse_led, 0644, reverse_led_show, reverse_led_store);

static ssize_t check_link_time_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    u32                 input;

    input                    = simple_strtoul(buf, NULL, 10);
    hemac->gu32CheckLinkTime = input;
    return count;
}
static ssize_t check_link_time_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);

    return sprintf(buf, "%d\n", hemac->gu32CheckLinkTime);
}
DEVICE_ATTR(check_link_time, 0644, check_link_time_show, check_link_time_store);

static ssize_t check_link_timedis_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                        size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    u32                 input;

    input                       = simple_strtoul(buf, NULL, 10);
    hemac->gu32CheckLinkTimeDis = input;
    return count;
}
static ssize_t check_link_timedis_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    return sprintf(buf, "%d\n", hemac->gu32CheckLinkTimeDis);
}
DEVICE_ATTR(check_link_timedis, 0644, check_link_timedis_show, check_link_timedis_store);

#if PHY_LED_CTL
static ssize_t sw_led_flick_speed_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                        size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    u32                 input;

    input                  = simple_strtoul(buf, NULL, 10);
    hemac->led_flick_speed = input;
    return count;
}
static ssize_t sw_led_flick_speed_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);

    return sprintf(buf, "LED flick speed, the smaller the faster\n%d\n", hemac->led_flick_speed);
}
DEVICE_ATTR(sw_led_flick_speed, 0644, sw_led_flick_speed_show, sw_led_flick_speed_store);
#endif

#if DYNAMIC_INT_TX_TIMER
#if DYNAMIC_INT_TX_TIMER_HR
static enum hrtimer_restart _Dev_EMAC_TxWdt_CB(struct hrtimer *timer)
{
    struct emac_handle *hemac = container_of(timer, struct emac_handle, timerTxWdt);

    if (!hemac->timerTxWdtPeriod)
        return HRTIMER_NORESTART;

    Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
    hrtimer_forward_now(&hemac->timerTxWdt, ns_to_ktime(hemac->timerTxWdtPeriod * 1000));
    return HRTIMER_RESTART;
}
#else
static void _Dev_EMAC_TxWdt_CB(unsigned long data)
{
    struct emac_handle *hemac = (struct emac_handle *)data;

    if (!hemac->timerTxWdtPeriod)
        return;

    Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TCOM, 1);
    hemac->timerTxWdt.expires = jiffies + ((hemac->timerTxWdtPeriod) * HZ) / 1000000;
    add_timer(&hemac->timerTxWdt);
}
#endif
#endif

static void _Dev_EMAC_Swing(void *hal, int param, u32 speed)
{
    u32 val;
    int tmp;

    // if default setting is 0 not enable swing
    if (param != 0)
    {
        // enable
        Hal_EMAC_read_phy(hal, 0, 0x02e, &val); // Bank: phy0_2e
        val = val | 0x1;
        Hal_EMAC_write_phy(hal, 0, 0x02e, val);
    }
    // swing
    Hal_EMAC_read_phy(hal, 0, 0x142, &val); // Bank: phy2_42

    if (speed == 10)
        tmp = (val & 0x1f);
    else
        tmp = ((val >> 8) & 0x1f);

    if (tmp & 0x10)
    {
        tmp = tmp & 0xf;
        tmp++;
        tmp = -tmp;
    }
    tmp += param;
    if (tmp > 15)
        tmp = 15;
    if (tmp < -16)
        tmp = -16;
    if (tmp < 0)
    {
        tmp = -tmp;
        tmp--;
        tmp = tmp | 0x10;
    }

    if (speed == 10)
    {
        val = (val & ~0x001f) | tmp;
    }
    else
    {
        tmp = tmp << 8;
        val = (val & ~0x1f00) | tmp;
    }

    Hal_EMAC_write_phy(hal, 0, 0x142, val);
    Hal_EMAC_read_phy(hal, 0, 0x142, &val); // Bank: phy2_42

    if (speed == 10)
        tmp = (val & 0x1f);
    else
        tmp = ((val >> 8) & 0x1f);
    printk("swing : 0x%02x \n", tmp);
}

static int _StrSplit(char **arr, char *str, char *del)
{
    char *cur   = str;
    char *token = NULL;
    int   cnt   = 0;

    token = strsep(&cur, del);
    while (token)
    {
        arr[cnt] = token;
        token    = strsep(&cur, del);
        cnt++;
    }
    return cnt;
}
static inline void _turndrv_store_help(void)
{
    printk(
        "Usage:\n\
    echo f10t    > turndrv => set max_speed to 10M \n\
    echo an      > turndrv => set link mode to autonegotiatingn\n");

#if DYNAMIC_INT_RX
    printk(
        "\
    echo dir_on  > turndrv => enable  Dynamic Rx Interrupt\n\
    echo dir_off > turndrv => disable Dynamic Rx Interrupt\n");
#endif

#if DYNAMIC_INT_TX_TIMER
    printk(
        "\
    echo tx_int    [time]  > turndrv => set Tx delay [time] Period\n");
#endif

    printk(
        "\
    echo swing_100 [gear]  > turndrv => swing 100M tx gear\n\
    echo swing_10  [gear]  > turndrv => swing 10M tx gear\n\
    echo rx_imp    [level] > turndrv => adjust rx impedance, range: -4 ~ 7\n\
    echo timing    [is_rise] [delay_time] [duty_cycle] [phase] > turndrv\n");
}

static void _Dev_EMAC_Timing_adjust(void *hal, u8 rise, u8 delay, u8 duty, u8 phase)
{
    u32 val;
    u8  base_offset = 0;

    Hal_EMAC_read_phy(hal, 0, 0x177, &val); // Bank: phy2_77
    if (rise)
    {
        base_offset = 8;
    }
    val &= ~(0xff << base_offset);
    val |= delay << base_offset;
    val |= duty << base_offset << 3;
    val |= phase << base_offset << 5;
    Hal_EMAC_write_phy(hal, 0, 0x177, val);
}

extern void Hal_EMAC_phy_trunMax(void *);
// extern void Hal_EMAC_trim_phy(void*);
static ssize_t turndrv_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    // u32 input;
    struct net_device * netdev             = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac              = (struct emac_handle *)netdev_priv(netdev);
    __ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = {
        0,
    };
    char *argv[50];
    int   param;
    char  del[]     = " ";
    int   buf_count = 0;

    buf_count = _StrSplit(argv, (char *)buf, del);

    if (!strncmp(buf, "max", strlen("max")))
    {
        Hal_EMAC_phy_trunMax(hemac->hal);
        printk("EMAC_phy_trunMax \n");
        return count;
    }

    if (!strncmp(buf, "f10t", strlen("10t")))
    {
        /*
                //force to set 10M on FPGA
                Hal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_ADVERTISE, 0x0061UL);
                mdelay(10);
                Hal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
        */
        phy_set_max_speed(netdev->phydev, SPEED_10);
        printk("SPEED_10 \n");
        return count;
    }
    else if (!strncmp(argv[0], "an", strlen("an")))
    {
        /*
                //force to set 10M on FPGA
                Hal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_ADVERTISE, 0x01e1UL);
                mdelay(10);
                Hal_EMAC_write_phy(hemac->hal, hemac->phyaddr, MII_BMCR, 0x1200UL);
        */
        ethtool_convert_legacy_u32_to_link_mode(mask, ADVERTISED_Autoneg);
        linkmode_or(netdev->phydev->advertising, netdev->phydev->supported, mask);
        phy_start_aneg(netdev->phydev);
        printk("phy_start_aneg \n");
        return count;
    }

#if DYNAMIC_INT_RX
    else if (!strncmp(argv[0], "dir_on", strlen("dir_on")))
    {
        hemac->rx_stats_enable = 1;
        printk("rx_stats_enable: %d \n", hemac->rx_stats_enable);
    }
    else if (!strncmp(argv[0], "dir_off", strlen("dir_off")))
    {
        hemac->rx_stats_enable = 0;
        printk("rx_stats_enable: %d \n", hemac->rx_stats_enable);
    }
#endif

    else if (buf_count < 2)
    {
        printk("Invalid argument!\n");
        _turndrv_store_help();
        return count;
    }

#if DYNAMIC_INT_TX_TIMER
    else if (strcmp(argv[0], "tx_int") == 0)
    {
        if (sscanf(argv[1], "%d", &param) == 1)
        {
#if DYNAMIC_INT_TX_TIMER_HR
            hemac->timerTxWdtPeriod = param;
            hrtimer_try_to_cancel(&hemac->timerTxWdt);
            hrtimer_start(&hemac->timerTxWdt, ns_to_ktime(hemac->timerTxWdtPeriod * 1000), HRTIMER_MODE_REL);
#else
            int           prev = hemac->timerTxWdtPeriod;
            unsigned long expires;

            hemac->timerTxWdtPeriod = param;
            expires                 = jiffies + ((hemac->timerTxWdtPeriod) * HZ) / 1000000;
            if (hemac->timerTxWdtPeriod)
            {
                if (!prev)
                {
                    hemac->timerTxWdt.expires = expires;
                    add_timer(&hemac->timerTxWdt);
                }
            }
#endif
            // printk("[%s][%d] timer %d\n", __FUNCTION__, __LINE__, hemac->timerTxWdtPeriod);
        }
    }
#endif

    else if (strcmp(argv[0], "swing_100") == 0)
    {
        if (sscanf(argv[1], "%d", &param) == 1)
            _Dev_EMAC_Swing(hemac->hal, param, 100);
    }
    else if (strcmp(argv[0], "swing_10") == 0)
    {
        if (sscanf(argv[1], "%d", &param) == 1)
            _Dev_EMAC_Swing(hemac->hal, param, 10);
    }
    else if (strcmp(argv[0], "rx_imp") == 0)
    {
        if (sscanf(argv[1], "%d", &param) == 1)
        {
            u32 val;
            int tmp = 0;

            if ((param > 7) || (param < -4))
            {
                printk("rx impedance out of range: -4 ~ 7 !!\n");
                return count;
            }
            else
            {
                printk("rx impedance level: %d\n", param);
            }

            // enable
            Hal_EMAC_read_phy(hemac->hal, 0, 0x130, &val); // Bank: phy2_30
            val = val | 0x4;
            Hal_EMAC_write_phy(hemac->hal, 0, 0x130, val);

            Hal_EMAC_read_phy(hemac->hal, 0, 0x134, &val); // Bank: phy2_34
            val = val | 0x8000;
            Hal_EMAC_write_phy(hemac->hal, 0, 0x134, val);

            Hal_EMAC_read_phy(hemac->hal, 0, 0x02e, &val); // Bank: phy0_2e
            val = val | 0x4;
            Hal_EMAC_write_phy(hemac->hal, 0, 0x02e, val);

            // rx impedance
            if (param >= 0)
                tmp = param;
            else
            {
                switch (param)
                {
                    case -1:
                        tmp = 0x8;
                        break;
                    case -2:
                        tmp = 0x9;
                        break;
                    case -3:
                        tmp = 0xA;
                        break;
                    case -4:
                        tmp = 0xB;
                        break;
                    default:
                        break;
                }
            }

            Hal_EMAC_read_phy(hemac->hal, 0, 0x130, &val); // Bank: phy2_30[10:7]
            val = val & ~(0xf << 7);
            val = val | tmp << 7;
            Hal_EMAC_write_phy(hemac->hal, 0, 0x130, val);

            Hal_EMAC_read_phy(hemac->hal, 0, 0x141, &val); // Bank: phy2_41[7:4]
            val = val & ~(0xf << 4);
            val = val | tmp << 4;
            Hal_EMAC_write_phy(hemac->hal, 0, 0x141, val);
        }
    }
    else if (strcmp(argv[0], "timing") == 0)
    {
        u8 rise = 0, delay = 0, duty = 0, phase = 0;
        if (buf_count < 5)
        {
            printk("Usage:\n echo timing [is_rise] [delay_time] [duty_cycle] [phase] > turndrv");
            return -EINVAL;
        }
        rise  = simple_strtoul(argv[1], NULL, 16);
        delay = simple_strtoul(argv[2], NULL, 16);
        duty  = simple_strtoul(argv[3], NULL, 16);
        phase = simple_strtoul(argv[4], NULL, 16);
        if (delay > 7)
        {
            delay = 7;
        }
        if (duty > 3)
        {
            duty = 3;
        }
        if (phase > 3)
        {
            phase = 3;
        }
        _Dev_EMAC_Timing_adjust(hemac->hal, rise, delay, duty, phase);
    }
    else
    {
        _turndrv_store_help();
        return count;
    }

    // input = simple_strtoul(buf, NULL, 10);
    return count;
}

static ssize_t turndrv_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "Usage:\n\
        echo f10t    > turndrv => set max_speed to 10M \n\
        echo an      > turndrv => set link mode to autonegotiatingn\n");

#if DYNAMIC_INT_RX
    str += scnprintf(str, end - str,
                     "\
        echo dir_on  > turndrv => enable  Dynamic Rx Interrupt\n\
        echo dir_off > turndrv => disable Dynamic Rx Interrupt\n");
#endif

#if DYNAMIC_INT_TX_TIMER
    str += scnprintf(str, end - str,
                     "\
        echo tx_int    [time]  > turndrv => set Tx delay [time] Period\n");
#endif

    str += scnprintf(str, end - str,
                     "\
        echo swing_100 [gear]  > turndrv => swing 100M tx gear\n\
        echo swing_10  [gear]  > turndrv => swing 10M tx gear\n\
        echo rx_imp    [level] > turndrv => adjust rx impedance, range: -4 ~ 7\n\
        echo timing    [is_rise] [delay_time] [duty_cycle] [phase] > turndrv\n");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

DEVICE_ATTR(turndrv, 0644, turndrv_show, turndrv_store);

#if PHY_REGISTER_DEBUG
static ssize_t phyStatusWR_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "phy read & write:\n\
    Usage:\n echo phy_r phyAddress > phyStatusWR\n echo phy_w phyAddress phyValue> phyStatusWR \
          \n echo phyE_r phyId phyAddress > phyStatusWR\n echo phyE_w phyId phyAddress phyValues > phyStatusWR");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t phyStatusWR_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    unsigned char       phyAddress;
    unsigned int        phyValue;
    unsigned int        phyId;
    char *              argv[50];
    char                del[]     = " ";
    int                 buf_count = 0;
    buf_count                     = _StrSplit(argv, (char *)buf, del);

    if (buf_count < 2)
    {
        printk("Usage:\n echo phy_r phyAddress > phyStatusWR\n echo phy_w phyAddress phyValue > phyStatusWR\n");
        return count;
    }
    phyAddress = simple_strtoul(argv[1], NULL, 16);
    phyId      = phyAddress;

    if (strcmp(argv[0], "phy_r") == 0)
    {
        phyValue = phy_read(netdev->phydev, phyAddress);
        printk("phy_r address[%x] value[%x]\n", phyAddress, phyValue);
    }
    else if (strcmp(argv[0], "phy_w") == 0)
    {
        phyValue = simple_strtoul(argv[2], NULL, 16);
        phy_write(netdev->phydev, phyAddress, phyValue);
        printk("phy_w address[%x] value[%x]\n", phyAddress, phyValue);
    }
    else if (strcmp(argv[0], "phyE_r") == 0)
    {
        phyAddress = simple_strtoul(argv[2], NULL, 16);
        Hal_EMAC_read_phy(hemac->hal, phyId, phyAddress, &phyValue);
        printk("phyE_r id[%x] address[%x] value[%x]\n", phyId, phyAddress, phyValue);
    }
    else if (strcmp(argv[0], "phyE_w") == 0)
    {
        phyAddress = simple_strtoul(argv[2], NULL, 16);
        phyValue   = simple_strtoul(argv[3], NULL, 16);
        Hal_EMAC_write_phy(hemac->hal, phyId, phyAddress, phyValue);
        printk("phyE_w id[%x] address[%x] value[%x]\n", phyId, phyAddress, phyValue);
    }

    return count;
}

DEVICE_ATTR(phyStatusWR, 0644, phyStatusWR_show, phyStatusWR_store);
#endif

#if NETWORK_STORM_PROTECT || NETWORK_STORM_PROTECT_DEBUG
#define TX_MAX_DEFAULT     400000
#define TX_CONSUME_LEVEL_1 40000
#define TX_CONSUME_LEVEL_2 20000
#define TX_CONSUME_LEVEL_3 10000
#define TX_CONSUME_LEVEL_4 5000
#define TX_CONSUME_LEVEL_5 2500
#endif

#if NETWORK_STORM_PROTECT_DEBUG
struct net_storm_protect
{
    unsigned int uni_cast_max;
    unsigned int uni_cast_consume;
    unsigned int uni_cast_token;
    unsigned int uni_cast_drom;
    unsigned int uni_cast_en;
    unsigned int multi_cast_max;
    unsigned int multi_cast_consume;
    unsigned int multi_cast_token;
    unsigned int multi_cast_drom;
    unsigned int multi_cast_en;
    unsigned int broad_cast_max;
    unsigned int broad_cast_consume;
    unsigned int broad_cast_token;
    unsigned int broad_cast_drom;
    unsigned int broad_cast_en;
};

static int _levet2consume(char level)
{
    int consume = 0;

    switch (level)
    {
        case 1:
            consume = TX_CONSUME_LEVEL_1;
            break;
        case 2:
            consume = TX_CONSUME_LEVEL_2;
            break;
        case 3:
            consume = TX_CONSUME_LEVEL_3;
            break;
        case 4:
            consume = TX_CONSUME_LEVEL_4;
            break;
        case 5:
            consume = TX_CONSUME_LEVEL_5;
            break;
        default:
            consume = 0;
    }
    return consume;
}

static ssize_t netStormProtect_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "netStormProtect unicast multicast broadcast :\n\
    Usage:\n \
    \r echo unicast   max consume en > netStormProtect\n\
    \r echo multicast max consume en > netStormProtect\n\
    \r echo broadcast max consume en > netStormProtect\n\
    \r echo preset unicast/multicast/broadcast level > netStormProtect\n\
    \r 100M MII\n\
    \r level 1 => 625  packet/s level 2 => 1250 packet/s level 3 => 2500 packet/s\n\
    \r level 4 => 5000 packet/s level 5 => 10000 packet/s\n\
    \r 100M RMII\n\
    \r level 1 => 1250  packet/s level 2 => 2500 packet/s level 3 => 5000 packet/s\n\
    \r level 4 => 10000 packet/s level 5 => 20000 packet/s\n");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t netStormProtect_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device *      netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *     hemac  = (struct emac_handle *)netdev_priv(netdev);
    struct net_storm_protect net_storm_proctect_var;
    char *                   argv[50];
    char                     del[]     = " ";
    int                      buf_count = 0;
    int                      level     = 1;
    buf_count                          = _StrSplit(argv, (char *)buf, del);

    if (strcmp(argv[0], "unicast") == 0)
    {
        net_storm_proctect_var.uni_cast_max     = simple_strtoul(argv[1], NULL, 10);
        net_storm_proctect_var.uni_cast_consume = simple_strtoul(argv[2], NULL, 10);
        net_storm_proctect_var.uni_cast_en      = simple_strtoul(argv[3], NULL, 10);
        Hal_EMAC_Netsp_Unicast_Setting(hemac->hal, net_storm_proctect_var.uni_cast_max,
                                       net_storm_proctect_var.uni_cast_consume, net_storm_proctect_var.uni_cast_en);
        printk("unicast setting => max[%d] consume[%d] en[%d]\n", net_storm_proctect_var.uni_cast_max,
               net_storm_proctect_var.uni_cast_consume, net_storm_proctect_var.uni_cast_en);
    }
    else if (strcmp(argv[0], "multicast") == 0)
    {
        net_storm_proctect_var.multi_cast_max     = simple_strtoul(argv[1], NULL, 10);
        net_storm_proctect_var.multi_cast_consume = simple_strtoul(argv[2], NULL, 10);
        net_storm_proctect_var.multi_cast_en      = simple_strtoul(argv[3], NULL, 10);
        Hal_EMAC_Netsp_Multicast_Setting(hemac->hal, net_storm_proctect_var.multi_cast_max,
                                         net_storm_proctect_var.multi_cast_consume,
                                         net_storm_proctect_var.multi_cast_en);
        printk("multicast setting => max[%d] consume[%d] en[%d]\n", net_storm_proctect_var.multi_cast_max,
               net_storm_proctect_var.multi_cast_consume, net_storm_proctect_var.multi_cast_en);
    }
    else if (strcmp(argv[0], "broadcast") == 0)
    {
        net_storm_proctect_var.broad_cast_max     = simple_strtoul(argv[1], NULL, 10);
        net_storm_proctect_var.broad_cast_consume = simple_strtoul(argv[2], NULL, 10);
        net_storm_proctect_var.broad_cast_en      = simple_strtoul(argv[3], NULL, 10);
        Hal_EMAC_Netsp_Broadcast_Setting(hemac->hal, net_storm_proctect_var.broad_cast_max,
                                         net_storm_proctect_var.broad_cast_consume,
                                         net_storm_proctect_var.broad_cast_en);
        printk("multicast setting => max[%d] consume[%d] en[%d]\n", net_storm_proctect_var.broad_cast_max,
               net_storm_proctect_var.broad_cast_consume, net_storm_proctect_var.broad_cast_en);
    }
    else if (strcmp(argv[0], "preset") == 0)
    {
        if (strcmp(argv[1], "unicast") == 0)
        {
            level = simple_strtoul(argv[2], NULL, 10);
            Hal_EMAC_Netsp_Unicast_Setting(hemac->hal, TX_MAX_DEFAULT, _levet2consume(level), 1);
            printk("unicast set level[%d]\n", level);
        }
        else if (strcmp(argv[1], "multicast") == 0)
        {
            level = simple_strtoul(argv[2], NULL, 10);
            Hal_EMAC_Netsp_Multicast_Setting(hemac->hal, TX_MAX_DEFAULT, _levet2consume(level), 1);
            printk("multicast set level[%d]\n", level);
        }
        else if (strcmp(argv[1], "broadcast") == 0)
        {
            level = simple_strtoul(argv[2], NULL, 10);
            Hal_EMAC_Netsp_Broadcast_Setting(hemac->hal, TX_MAX_DEFAULT, _levet2consume(level), 1);
            printk("broadcast set level[%d]\n", level);
        }
    }

    return count;
}

DEVICE_ATTR(netStormProtect, 0644, netStormProtect_show, netStormProtect_store);
#endif

#if HW_FLOW_CONTROL_DEBUG
static ssize_t flowControl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "flowControl:\n\
    Usage:\n \
    \r echo init iswait pauselength > flowControl => iswait pauselength\n\
    \r echo th pauselength > flowControl          => th\n\
    \r echo mac m0 m1 m2 m3 m4 m5 > flowControl   => updata flow control mac addr\n\
    \r echo rx 0/1 > flowControl                  => 0/1 disable/enable flowControl\n\
    \r echo tx sw 0/1 > flowControl               => 0/1/2  disable/enable sw flowControl\n\
    \r echo tx hw 0/1 > flowControl               => 0/1/2  disable/enable hw flowControl\n");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t flowControl_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    char *              argv[50];
    char                del[]     = " ";
    int                 buf_count = 0;
    int                 rx_flow_en;
    int                 tx_flow_en;
    int                 tx_flow_triger_times;
    int                 isWait;
    int                 pauseLength;
    int                 th;
    int                 i;
    char                m0, m1, m2, m3, m4, m5;
    buf_count = _StrSplit(argv, (char *)buf, del);

    if (strcmp(argv[0], "rx") == 0)
    {
        rx_flow_en        = simple_strtoul(argv[1], NULL, 10);
        _ethtool_tx_pause = rx_flow_en;
        Hal_EMAC_TX_Flow_Ctrl_Enable(hemac->hal, rx_flow_en);
    }
    else if (strcmp(argv[0], "tx") == 0)
    {
        if (strcmp(argv[1], "hw") == 0)
        {
            tx_flow_en        = simple_strtoul(argv[2], NULL, 10);
            _ethtool_rx_pause = tx_flow_en;
            Hal_EMAC_RX_Flow_Ctrl_Enable(hemac->hal, tx_flow_en);
        }
        else if (strcmp(argv[1], "sw") == 0)
        {
            tx_flow_triger_times = simple_strtoul(argv[2], NULL, 10);
            for (i = 0; i < tx_flow_triger_times; i++)
            {
                Hal_EMAC_TX_Flow_control_sw_triger(hemac->hal, 1);
                CamOsUsDelay(1);
                Hal_EMAC_TX_Flow_control_sw_triger(hemac->hal, 0);
            }
        }
    }
    else if (strcmp(argv[0], "init") == 0)
    {
        isWait      = simple_strtoul(argv[1], NULL, 10);
        pauseLength = simple_strtoul(argv[2], NULL, 10);
        if (pauseLength > 0xffff)
        {
            printk("pauseLength is too large\n");
            return count;
        }
        printk("is Wait[%d] pause time%d us\n", isWait, pauseLength * 512 / 10);
        Hal_EMAC_Flow_Control_Init(hemac->hal, isWait, pauseLength);
    }
    else if (strcmp(argv[0], "mac") == 0)
    {
        m0 = simple_strtoul(argv[1], NULL, 16);
        m1 = simple_strtoul(argv[2], NULL, 16);
        m2 = simple_strtoul(argv[3], NULL, 16);
        m3 = simple_strtoul(argv[4], NULL, 16);
        m4 = simple_strtoul(argv[5], NULL, 16);
        m5 = simple_strtoul(argv[6], NULL, 16);
        printk("Seting mac addrss to %x:%x:%x:%x:%x:%x\n", m0, m1, m2, m3, m4, m5);
#if HW_FLOW_CONTROL
        Hal_EMAC_Flow_Control_Mac_Address(hemac->hal, m0, m1, m2, m3, m4, m5);
#endif
    }
    else if (strcmp(argv[0], "th") == 0)
    {
        th = simple_strtoul(argv[1], NULL, 10);
        if (th > 0x3ff)
        {
            printk("th is too large\n");
            return count;
        }
        printk("th setting to %d\n", th);
        Hal_EMAC_Flow_control_Auto_Trig_th(hemac->hal, th);
    }
    return count;
}

DEVICE_ATTR(flowControl, 0644, flowControl_show, flowControl_store);
#endif

#if TX_DELAY_INT_DEBUG
static ssize_t tx_delay_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(
        str, end - str,
        "Usage:\ncat tx_delay\necho [tx_delay_debug] [tx_delay_enable] [tx_delay_timeout_enable] [tx_delay_num] "
        "[tx_delay_timeout] > rx_delay\n");

    str += scnprintf(str, end - str, "%s\n", "\ntx delay current setting:");
    str += scnprintf(str, end - str, "%s=%d\n", "tx_delay_debug", tx_delay_debug);
    str += scnprintf(str, end - str, "%s=%d\n", "tx_delay_enable", tx_delay_enable);
    str += scnprintf(str, end - str, "%s=%d\n", "tx_delay_timeout_enable", tx_delay_timeout_enable);
    str += scnprintf(str, end - str, "%s=%d\n", "tx_delay_num", tx_delay_num);
    str += scnprintf(str, end - str, "%s=%d\n", "tx_delay_timeout", tx_delay_timeout);

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t tx_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    char *              argv[5];
    char                del[]     = " ";
    int                 buf_count = 0;

    buf_count = _StrSplit(argv, (char *)buf, del);

    if (buf_count < 5)
    {
        printk(
            "\nUsage:\necho [tx_delay_debug] [tx_delay_enable] [tx_delay_timeout_enable] [tx_delay_num] "
            "[tx_delay_timeout] > "
            "rx_delay\n");
        return count;
    }

    tx_delay_debug          = simple_strtoul(argv[0], NULL, 10);
    tx_delay_enable         = simple_strtoul(argv[1], NULL, 10);
    tx_delay_timeout_enable = simple_strtoul(argv[2], NULL, 10);
    tx_delay_num            = simple_strtoul(argv[3], NULL, 10);
    tx_delay_timeout        = simple_strtoul(argv[4], NULL, 10);
    if (tx_delay_num == 0 || tx_delay_timeout == 0)
    {
        printk("tx delay num or tx delay timeout should not set to zero\n");
        return count;
    }

    if (tx_delay_debug)
    {
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY, tx_delay_enable);
        Hal_EMAC_IntEnable(hemac->hal, EMAC_INT_TX_DELAY_TIMEOUT, tx_delay_timeout_enable);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_MAX_CNT, tx_delay_num);
        Hal_EMAC_WritReg32(hemac->hal, REG_TXDELAY_TO_MAX_CNT, tx_delay_timeout);
    }

    return count;
}

DEVICE_ATTR(tx_delay, 0644, tx_delay_show, tx_delay_store);
#endif

#if RX_DELAY_INT_DEBUG
static ssize_t rx_delay_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "Usage:\ncat rx_delay\necho [rx_delay_debug] [rx_delay_num] "
                     "[rx_delay_cyc] > rx_delay\n");

    str += scnprintf(str, end - str, "%s\n", "\ntx delay current setting:");
    str += scnprintf(str, end - str, "%s=%d\n", "rx_delay_debug", rx_delay_debug);
    str += scnprintf(str, end - str, "%s=%d\n", "rx_delay_num", rx_delay_num);
    str += scnprintf(str, end - str, "%s=%d\n", "rx_delay_cyc", rx_delay_cyc);

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t rx_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    char *              argv[5];
    char                del[]     = " ";
    int                 buf_count = 0;

    buf_count = _StrSplit(argv, (char *)buf, del);

    if (buf_count < 3)
    {
        printk("\nUsage:\necho [rx_delay_debug] [rx_delay_num] [rx_delay_cyc] \n");
        return count;
    }
    rx_delay_debug = simple_strtoul(argv[0], NULL, 10);
    rx_delay_num   = simple_strtoul(argv[1], NULL, 10);
    rx_delay_cyc   = simple_strtoul(argv[2], NULL, 10);

    if (rx_delay_num == 0 || rx_delay_cyc == 0)
    {
        printk("rx delay num or rx delay cyc should not set to zero\n");
        return count;
    }

    if (rx_delay_debug)
    {
        Hal_EMAC_RX_ParamSet(hemac->hal, rx_delay_num, rx_delay_cyc);
    }

    return count;
}

DEVICE_ATTR(rx_delay, 0644, rx_delay_show, rx_delay_store);
#endif

#if VLAD_TAG_TEST_DEBUG
static ssize_t vlan_tag_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "vlan_tag:\n\
    Usage:\n \
    \r echo init en id > vlan_tag => en vlan_id\n\
    \r echo set id > vlan_tag          => update vlan id\n");

    if (str > buf)
        str--;

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t vlan_tag_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    char *              argv[5];
    char                del[]     = " ";
    int                 buf_count = 0;
    int                 en;
    int                 id;
    buf_count = _StrSplit(argv, (char *)buf, del);

    if (buf_count < 2)
    {
        printk(
            "\nUsage:\necho init [en] [id ] > vlan_tag "
            "echo set id > vlan_tag \n");
        return count;
    }
    if (strcmp(argv[0], "init") == 0)
    {
        en = simple_strtoul(argv[1], NULL, 10);
        id = simple_strtoul(argv[2], NULL, 10);
        Hal_EMAC_Vlan_Id_Init(hemac->hal, en, id);
        printk("en %d id %d\n", en, id);
    }
    else if (strcmp(argv[0], "set") == 0)
    {
        id = simple_strtoul(argv[1], NULL, 10);
        Hal_EMAC_Update_Vlan_Id(hemac->hal, id);
        printk("set id %d\n", id);
    }

    return count;
}

DEVICE_ATTR(vlan_tag, 0644, vlan_tag_show, vlan_tag_store);
#endif

#if EMAC_DEBUG_TX
static ssize_t debug_tx_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "set tx debug level:\n\
Usage:\n  echo debug_level protocol_type > debug_tx\n  debug_level could be one of {0, 1, 2 ,3}\n\
  protocol_type(optional) could be one of {tcp, udp, icmp ,arp}, like:\n  echo 1 tcp > debug_tx\n\
    0: default value, not print\n\
    1: print packet protocol\n\
    2: print packet link layer part\n\
    3: print all the contents of the package\n\
  (debug_level = %d  protocol_type = %d)\n",
                     debug_level_tx, tx_protocol_type);
    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t debug_tx_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int           buf_count = 0;
    unsigned char protocol[16];
    unsigned int  debug_level;
    char *        argv[50];
    char          del[] = " ";
    buf_count           = _StrSplit(argv, (char *)buf, del);

    if (buf_count == 1)
    {
        debug_level = simple_strtoul(argv[0], NULL, 16);
    }
    else if (buf_count == 2)
    {
        debug_level = simple_strtoul(argv[0], NULL, 16);
        sscanf(argv[1], "%s", protocol);
    }
    else
    {
        printk(
            "Usage:\n echo debug_level protocol_type(optional) > debug_tx\n debug_level could be one of {0, 1, 2 "
            ",3}\n");
        return count;
    }

    if (!strcasecmp(protocol, "icmp"))
    {
        tx_protocol_type = 1;
    }
    else if (!strcasecmp(protocol, "tcp"))
    {
        tx_protocol_type = 6;
    }
    else if (!strcasecmp(protocol, "udp"))
    {
        tx_protocol_type = 17;
    }
    else if (!strcasecmp(protocol, "arp"))
    {
        tx_protocol_type = 86;
    }
    else
    {
        tx_protocol_type = 0;
    }

    if (debug_level >= 0 && debug_level <= 3)
    {
        debug_level_tx = debug_level;
    }
    else
    {
        printk(
            "Usage:\n echo debug_level protocol_type(optional) > debug_tx\n debug_level could be one of {0, 1, 2 "
            ",3}\n");
    }

    return count;
}

DEVICE_ATTR(debug_tx, 0644, debug_tx_show, debug_tx_store);
#endif

#if EMAC_DEBUG_RX
static ssize_t debug_rx_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "set rx debug level:\n\
Usage:\n  echo debug_level protocol_type > debug_rx\n  debug_level could be one of {0, 1, 2 ,3}\n\
  protocol_type(optional) could be one of {tcp, udp, icmp ,arp}, like:\n  echo 1 tcp > debug_rx\n\
    0: default value, not print\n\
    1: print packet protocol\n\
    2: print packet link layer part\n\
    3: print all the contents of the package\n\
  (debug_level = %d  protocol_type = %d)\n",
                     debug_level_rx, rx_protocol_type);

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t debug_rx_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int           buf_count = 0;
    unsigned char protocol[16];
    unsigned int  debug_level;
    char *        argv[50];
    char          del[] = " ";
    buf_count           = _StrSplit(argv, (char *)buf, del);

    if (buf_count == 1)
    {
        debug_level = simple_strtoul(argv[0], NULL, 16);
    }
    else if (buf_count == 2)
    {
        debug_level = simple_strtoul(argv[0], NULL, 16);
        sscanf(argv[1], "%s", protocol);
    }
    else
    {
        printk(
            "Usage:\n echo debug_level protocol_type(optional) > debug_Rx\n debug_level could be one of {0, 1, 2 "
            ",3}\n");
        return count;
    }

    if (!strcasecmp(protocol, "icmp"))
    {
        rx_protocol_type = 1;
    }
    else if (!strcasecmp(protocol, "tcp"))
    {
        rx_protocol_type = 6;
    }
    else if (!strcasecmp(protocol, "udp"))
    {
        rx_protocol_type = 17;
    }
    else if (!strcasecmp(protocol, "arp"))
    {
        rx_protocol_type = 86;
    }
    else
    {
        rx_protocol_type = 0;
    }

    if (debug_level >= 0 && debug_level <= 3)
    {
        debug_level_rx = debug_level;
    }
    else
    {
        printk(
            "Usage:\n echo debug_level protocol_type(optional) > debug_rx\n debug_level could be one of {0, 1, 2 "
            ",3}\n");
    }

    return count;
}

DEVICE_ATTR(debug_rx, 0644, debug_rx_show, debug_rx_store);
#endif

#if EMAC_PERFORMANCE
static ssize_t performance_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "Usage:\n  echo performance_flag > performance\n  Print performance data every second when set "
                     "performance_flag to 1\n");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t performance_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(netdev);
    CamOsGetMonotonicTime(&performance_start_time);
    performance_flag = strncmp(buf, "1", strlen("1")) ? false : true;
    tx_total_bytes   = hemac->stats.tx_bytes;
    rx_total_bytes   = hemac->stats.rx_bytes;

    while (1)
    {
        if (!performance_flag)
            break;
        else
        {
            CamOsTimespec_t tCur;
            int             elapse;
            CamOsGetMonotonicTime(&tCur);
            elapse = (int)CamOsTimeDiff(&performance_start_time, &tCur, CAM_OS_TIME_DIFF_MS);

            if (PERFORMANC_TIME_INTERVAL <= elapse)
            {
                printk(" RX packets:%llu  RX rate:%luB/s \n TX packets:%llu  TX rate:%luB/s\n\n", rx_packet_cnt,
                       hemac->stats.rx_bytes - rx_total_bytes, hemac->skb_tx_send,
                       hemac->stats.tx_bytes - tx_total_bytes);
                tx_total_bytes         = hemac->stats.tx_bytes;
                rx_total_bytes         = hemac->stats.rx_bytes;
                performance_start_time = tCur;
            }
        }
    }

    return count;
}

DEVICE_ATTR(performance, 0644, performance_show, performance_store);
#endif

#if EMAC_LOCAL_TEST
static ssize_t emac_local_test_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "Usage:\n  echo rx_checksum 0/1 > local_test => enable/disble rx checksum info\n\
               echo keep_link_up 0/1 > local_test => enable/disable keep link up\n\
               echo rx_receive_all 0/1 > local_test => enable/disable promiscuous mode\n\
               echo unicast_hash 0/1 macaddr > local_test => enable/disable unicast_hash\n\
               eg: echo hash 1 00:83:27:00:00:01 > local_tst => can receive frame mac is 00 83 27 00 00 01 \n\
    ");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
    return 0;
}
#define MAC_ADDR_SIZE 6
static ssize_t emac_local_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct net_device * netdev    = (struct net_device *)dev_get_drvdata(dev);
    struct emac_handle *hemac     = (struct emac_handle *)netdev_priv(netdev);
    int                 buf_count = 0;
    char *              argv[50];
    char                del[] = " ";
    u32                 val;
    char *              mac_str[50];
    unsigned char       addr[MAC_ADDR_SIZE];
    int                 i;
    int                 en;
    buf_count = _StrSplit(argv, (char *)buf, del);

    if (strcmp(argv[0], "rx_checksum") == 0)
    {
        rx_checksum_info = simple_strtoul(argv[1], NULL, 10);
        printk("rx checksum info %d\n", rx_checksum_info);
    }
    else if (strcmp(argv[0], "keep_link_up") == 0)
    {
        keep_link_up = simple_strtoul(argv[1], NULL, 10);
        printk("keep link up %d\n", keep_link_up);
    }
    else if (strcmp(argv[0], "rx_receive_all") == 0)
    {
        val            = Hal_EMAC_Read_CFG(hemac->hal);
        rx_reveive_all = simple_strtoul(argv[1], NULL, 10);
        if (rx_reveive_all)
        {
            val |= EMAC_CAF;
            printk("Enable promiscuous mode");
        }
        else
        {
            val &= ~EMAC_CAF;
            printk("Disbale promiscuous mode");
        }
        Hal_EMAC_Write_CFG(hemac->hal, val);
    }
    else if (strcmp(argv[0], "hash") == 0)
    {
        en = simple_strtoul(argv[1], NULL, 10);
        if (en)
        {
            printk("en hash test\n");
            _StrSplit(mac_str, argv[2], ":");
            for (i = 0; i < MAC_ADDR_SIZE; i++)
            {
                addr[i] = simple_strtoul(*(mac_str + i), NULL, 16);
            }
            val = Hal_EMAC_Read_CFG(hemac->hal);
            Dev_EMAC_sethashtable(netdev, addr);
            val |= EMAC_UNI;
            Hal_EMAC_Write_CFG(hemac->hal, val);
        }
        else
        {
            printk("disable hash test\n");
            val = Hal_EMAC_Read_CFG(hemac->hal);
            Dev_EMAC_sethashtable(netdev, addr);
            val &= ~EMAC_UNI;
            Hal_EMAC_Write_CFG(hemac->hal, val);
        }
    }
    else if (strcmp(argv[0], "miu_protect_status") == 0)
    {
        val = Hal_EMAC_ReadReg32(hemac->hal, REG_EMAC_JULIAN_0100);
        printk("miu protect status %d\n", (val & 1 << 20) ? 1 : 0);
    }
    else if (strcmp(argv[0], "tx_wrong_fix") == 0)
    {
        u32 value;
        en = simple_strtoul(argv[1], NULL, 10);
        if (en)
        {
            printk("tx_wrong_fix en\n");
            value = Hal_EMAC_ReadReg32(hemac->hal, REG_EMAC_JULIAN_0138);
            value &= 0xfffffffD;
            value |= 1 << 1;
            Hal_EMAC_WritReg32(hemac->hal, REG_EMAC_JULIAN_0138, value);

            value = Hal_EMAC_ReadReg32(hemac->hal, RED_EMAC_JULIAN_01D2);
            value &= 0xfffffffE;
            value |= 1 << 0;
            Hal_EMAC_WritReg32(hemac->hal, RED_EMAC_JULIAN_01D2, value);
        }
        else
        {
            printk("tx_wrong_fix disable\n");
            value = Hal_EMAC_ReadReg32(hemac->hal, REG_EMAC_JULIAN_0138);
            value &= 0xfffffffD;
            value &= ~(1 << 1);
            Hal_EMAC_WritReg32(hemac->hal, REG_EMAC_JULIAN_0138, value);

            value = Hal_EMAC_ReadReg32(hemac->hal, RED_EMAC_JULIAN_01D2);
            value &= 0xfffffffE;
            value &= ~(1 << 0);
            Hal_EMAC_WritReg32(hemac->hal, RED_EMAC_JULIAN_01D2, value);
        }
    }
    return count;
}

DEVICE_ATTR(local_test, 0644, emac_local_test_show, emac_local_test_store);
#endif

#if EMAC_LOOPBACK_DEBUG
static ssize_t emacLoopback_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str,
                     "emacLoopback:\n\
    Usage:\n \
    \r echo 0 => disable RX received packets loopback in emac\n\
    \r echo 1 =>  enable RX received packets loopback in emac\n");

    if (str > buf)
        str--;

    str += scnprintf(str, end - str, "\n");

    return (str - buf);
}

static ssize_t emacLoopback_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    char *argv[5];
    char  del[]     = " ";
    int   buf_count = 0;
    int   input;
    buf_count = _StrSplit(argv, (char *)buf, del);

    if (buf_count == 1)
    {
        input = simple_strtoul(argv[0], NULL, 16);
        if (input == 0)
        {
            emac_loop = FALSE;
        }
        else if (input == 1)
        {
            emac_loop = TRUE;
        }
    }

    return count;
}

DEVICE_ATTR(emacLoopback, 0644, emacLoopback_show, emacLoopback_store);
#endif

static int Dev_EMAC_setup(struct net_device *dev)
{
    struct emac_handle *hemac;
    // dma_addr_t dmaaddr;
    void *        RetAddr;
    unsigned long flags;
#ifdef CONFIG_SSTAR_HW_TX_CHECKSUM
    u32 retval;
#endif

    hemac = (struct emac_handle *)netdev_priv(dev);

    if (hemac->bInit)
    // if (already_initialized)
    {
        printk("[%s][%d] %s has been initiated\n", __FUNCTION__, __LINE__, hemac->name);
        return FALSE;
    }
    if (hemac == NULL)
    {
        free_irq(dev->irq, dev);
        EMAC_ERR("hemac fail\n");
        return -ENOMEM;
    }

#if defined ISR_BOTTOM_HALF
    /*Init tx and rx tasks*/
    INIT_WORK(&hemac->rx_task, Dev_EMAC_bottom_rx_task);
#endif
#ifdef TX_SOFTWARE_QUEUE
    INIT_WORK(&hemac->tx_task, Dev_EMAC_bottom_tx_task);
#endif

    hemac->netdev = dev;

    // skb_queue_create(&(hemac->skb_queue_tx), Hal_EMAC_TXQ_Size(), 256);
    // skb_queue_create(&(hemac->skb_queue_tx), Hal_EMAC_TXQ_Size(), Hal_EMAC_TXQ_Size());
    // RetAddr = Dev_EMAC_VarInit();
    RetAddr = Dev_EMAC_VarInit(hemac);
    if (!RetAddr)
    {
        EMAC_ERR("Var init fail!!\n");
        return FALSE;
    }

    // dev->base_addr = (long) (EMAC_RIU_REG_BASE+REG_BANK_EMAC0*0x200); // seems useless

    // spin_lock_init(&hemac->mutexRXD);
    Dev_EMAC_HW_init(dev);

    // hemac->lock = &emac_lock;
    spin_lock_init(&hemac->mutexNetIf);
    spin_lock_init(&hemac->mutexPhy);
    spin_lock_init(&hemac->mutexTXQ);
#if (0 == SSTAR_EMAC_NAPI)
    spin_lock_init(&hemac->mutexRXInt);
#endif

    // spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_create(&(hemac->skb_queue_tx), Hal_EMAC_TXQ_Size(hemac->hal),
                     Hal_EMAC_TXQ_Size(hemac->hal) + hemac->txq_num_sw);
    // spin_unlock_irqrestore(&hemac->mutexTXQ, flags);

#if EMAC_FLOW_CONTROL_TX
    spin_lock_init(&hemac->mutexFlowTX);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    timer_setup(&hemac->timerFlowTX, _Dev_EMAC_FlowTX_CB, 0);
    hemac->timerFlowTX.expires = jiffies;
#else
    init_timer(&hemac->timerFlowTX);
    hemac->timerFlowTX.data     = (unsigned long)hemac;
    hemac->timerFlowTX.expires  = jiffies;
    hemac->timerFlowTX.function = _Dev_EMAC_FlowTX_CB;
#endif
    // hemac->isPauseTX = 0;
#endif

#if REDUCE_CPU_FOR_RBNA
    spin_lock_init(&hemac->mutexIntRX);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    timer_setup(&hemac->timerIntRX, _Dev_EMAC_IntRX_CB, 0);
    hemac->timerIntRX.expires = jiffies;
#else
    init_timer(&hemac->timerIntRX);
    hemac->timerIntRX.data     = (unsigned long)hemac;
    hemac->timerIntRX.expires  = jiffies;
    hemac->timerIntRX.function = _Dev_EMAC_IntRX_CB;
#endif
#endif

#if DYNAMIC_INT_RX
    // getnstimeofday(&hemac->rx_stats_time);
    hemac->rx_stats_packet = 0xFFFFFFFF;
    hemac->rx_stats_enable = 1;
#endif

    ether_setup(dev);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 28)
    dev->open               = Dev_EMAC_open;
    dev->stop               = Dev_EMAC_close;
    dev->hard_start_xmit    = Dev_EMAC_tx;
    dev->get_stats          = Dev_EMAC_stats;
    dev->set_multicast_list = Dev_EMAC_set_rx_mode;
    dev->do_ioctl           = Dev_EMAC_ioctl;
    dev->set_mac_address    = Dev_EMAC_set_mac_address;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
    dev->netdev_ops = &sstar_lan_netdev_ops;
#endif
    dev->tx_queue_len = EMAC_MAX_TX_QUEUE;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
    ////SET_ETHTOOL_OPS(dev, &ethtool_ops);
    // EMAC_TODO("set Ethtool_ops\n");
    //  netdev_set_default_ethtool_ops(dev, &ethtool_ops);
#endif
    spin_lock_irqsave(&hemac->mutexPhy, flags);
    Dev_EMAC_get_mac_address(dev);    // Get ethernet address and store it in dev->dev_addr //
    Dev_EMAC_update_mac_address(dev); // Program ethernet address into MAC //
    Hal_EMAC_enable_mdi(hemac->hal);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

    // already_initialized = 1;
    // hemac->bInit = 1;
#ifdef CONFIG_SSTAR_HW_TX_CHECKSUM
    dev->features |= NETIF_F_IP_CSUM;
#endif

    dev->features |= EMAC_FEATURES;
    dev->vlan_features |= EMAC_FEATURES;

#ifdef CONFIG_ARM64
    // Set the mask of DMA to support DRAM bigger than 4GB
    if (dma_set_mask_and_coherent(hemac->dev, DMA_BIT_MASK(64)))
    {
        EMAC_ERR("Failed to set DMA Mask\n");
        return -EPROBE_DEFER;
    }
#endif

    hemac->irqcnt    = 0;
    hemac->tx_irqcnt = 0;

    // printk("[%s][%d] (irq_emac, irq_lan) = (%d, %d)\n", __FUNCTION__, __LINE__, hemac->irq_emac, hemac->irq_lan);
    // printk("[%s][%d] %d\n", __FUNCTION__, __LINE__, dev->irq);
    dev->irq = hemac->irq_emac;
    // dev->irq = irq_of_parse_and_map(dev->dev.of_node, 0);
    if (!dev->irq)
    {
        EMAC_ERR("Get irq number0 error from DTS\n");
        return -EPROBE_DEFER;
    }

    // printk("[%s][%d] request irq for %d %d\n", __FUNCTION__, __LINE__, dev->irq, hemac->irq_emac);
    // Install the interrupt handler //
    // Notes: Modify linux/kernel/irq/manage.c  /* interrupt.h */
    if (request_irq(dev->irq, Dev_EMAC_interrupt, 0 /*SA_INTERRUPT*/, dev->name, dev))
    {
        EMAC_ERR("Request emac irq fail\n");
        return -EBUSY;
    }

    if (hemac->cpu_affinity)
    {
        irq_set_affinity_hint(dev->irq, cpu_online_mask);
    }

#ifdef LAN_ESD_CARRIER_INTERRUPT
    // val = irq_of_parse_and_map(dev->dev.of_node, 1);
    val = hemac->irq_lan;
    if (!val)
    {
        EMAC_ERR("Get lan irq number error from DTS\n");
        return -EPROBE_DEFER;
    }
    if (request_irq(val /*INT_FIQ_LAN_ESD+32*/, Dev_EMAC_interrupt_cable_unplug, 0 /*SA_INTERRUPT*/, dev->name, dev))
        return -EBUSY;
#endif
    // Determine current link speed //
    spin_lock_irqsave(&hemac->mutexPhy, flags);
    // Dev_EMAC_update_linkspeed (dev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

#if DYNAMIC_INT_TX_TIMER

#if DYNAMIC_INT_TX_TIMER_HR
    hrtimer_init(&hemac->timerTxWdt, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hemac->timerTxWdt.function = _Dev_EMAC_TxWdt_CB;
#else
    init_timer(&hemac->timerTxWdt);
    hemac->timerTxWdt.data = (unsigned long)hemac;
    // hemac->timerDummy.expires = jiffies + HZ/10;
    hemac->timerTxWdt.function = _Dev_EMAC_TxWdt_CB;
    hemac->timerTxWdtPeriod    = 0;
    // add_timer(&hemac->timerDummy);
#endif

#endif
    alloc_chrdev_region(&gEthDev, 0, MINOR_EMAC_NUM, "ETH");
    hemac->sstar_class_emac_device =
        device_create(msys_get_sysfs_class(), NULL, MKDEV(MAJOR(gEthDev), hemac->u8Minor), NULL, hemac->name);
    dev_set_drvdata(hemac->sstar_class_emac_device, (void *)dev);

    // Wake-Up On Lan supported
    device_set_wakeup_capable(hemac->sstar_class_emac_device, 1);

    device_create_file(hemac->sstar_class_emac_device, &dev_attr_tx_sw_queue_info);
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_dlist_info);
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_reverse_led);
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_check_link_time);
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_check_link_timedis);
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_info);
#if PHY_LED_CTL
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_sw_led_flick_speed);
#endif
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_turndrv);
#if PHY_REGISTER_DEBUG
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_phyStatusWR);
#endif
#if NETWORK_STORM_PROTECT_DEBUG
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_netStormProtect);
#endif
#if HW_FLOW_CONTROL_DEBUG
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_flowControl);
#endif
#if TX_DELAY_INT_DEBUG
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_tx_delay);
#endif
#if RX_DELAY_INT_DEBUG
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_rx_delay);
#endif
#if EMAC_DEBUG_RX
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_debug_rx);
#endif
#if EMAC_DEBUG_TX
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_debug_tx);
#endif
#if EMAC_PERFORMANCE
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_performance);
#endif
#if VLAD_TAG_TEST_DEBUG
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_vlan_tag);
#endif
#if EMAC_LOCAL_TEST
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_local_test);
#endif
#if defined(PACKET_DUMP)
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_tx_dump);
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_rx_dump);
#endif
#if EMAC_LOOPBACK_DEBUG
    device_create_file(hemac->sstar_class_emac_device, &dev_attr_emacLoopback);
#endif
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Restar the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------
int Dev_EMAC_SwReset(struct net_device *dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(dev);
    u32                 oldCFG, oldCTL;
    // u32 retval;
    unsigned long flags;

    spin_lock_irqsave(&hemac->mutexPhy, flags);
    Dev_EMAC_get_mac_address(dev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);
    oldCFG = Hal_EMAC_Read_CFG(hemac->hal);
    oldCTL = Hal_EMAC_Read_CTL(hemac->hal) & ~(EMAC_TE | EMAC_RE);

    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_reset(&(hemac->skb_queue_tx));
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    free_rx_skb(hemac); // @FIXME : how about RX descriptor
    netif_stop_queue(dev);

    /*
    retval = Hal_EMAC_Read_JULIAN_0100(hemac->hal);
    Hal_EMAC_Write_JULIAN_0100(hemac->hal, retval & 0x00000FFFUL);
    Hal_EMAC_Write_JULIAN_0100(hemac->hal, retval);
    */
    Hal_EMAC_Write_JULIAN_0100(hemac->hal, 1);

    Dev_EMAC_HW_init(dev);
    Hal_EMAC_Write_CFG(hemac->hal, oldCFG);
    Hal_EMAC_Write_CTL(hemac->hal, oldCTL);

    spin_lock_irqsave(&hemac->mutexPhy, flags);
    Hal_EMAC_enable_mdi(hemac->hal);
    Dev_EMAC_update_mac_address(dev); // Program ethernet address into MAC //
    // (void)Dev_EMAC_update_linkspeed (dev);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

    Hal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);
    Hal_EMAC_IntEnable(hemac->hal, hemac->gu32intrEnable, 1);

    Dev_EMAC_start(dev);
    Dev_EMAC_set_rx_mode(dev);
    netif_start_queue(dev);
    hemac->contiROVR = 0;
    EMAC_ERR("=> Take %lu ms to reset EMAC!\n", (getCurMs() - hemac->oldTime));
    return 0;
}

//-------------------------------------------------------------------------------------------------
// Detect MAC and PHY and perform initialization
//-------------------------------------------------------------------------------------------------
#if defined(CONFIG_OF)
static struct of_device_id sstaremac_of_device_ids[] = {
    {.compatible = "sstar-emac"},
    {},
};
#endif

static int Dev_EMAC_probe(struct net_device *dev)
{
    int detected;
    /* Read the PHY ID registers - try all addresses */
    detected = Dev_EMAC_setup(dev);
    return detected;
}

#if PHY_MDI_CTL
static void Dev_EMAC_mdix_work_handle(struct work_struct *work)
{
    static int          mdix_status;
    struct emac_handle *hemac = container_of(work, struct emac_handle, mdix_work.work);
    struct net_device * dev   = hemac->netdev;

    if (hemac->phy_mode != PHY_INTERFACE_MODE_RMII)
    {
        if ((!dev->phydev->autoneg) && (!dev->phydev->link))
        {
            Hal_EMAC_Phy_MDI_MDIX(hemac->hal, mdix_status);
            mdix_status = !mdix_status;
        }
    }

    schedule_delayed_work(&hemac->mdix_work, msecs_to_jiffies(INT_PHY_MDIX_TIMER_MS));
}
#endif

//-------------------------------------------------------------------------------------------------
// EMAC MACADDR Setup
//-------------------------------------------------------------------------------------------------

#ifndef MODULE

#define MACADDR_FORMAT "XX:XX:XX:XX:XX:XX"

static int __init macaddr_auto_config_setup(char *addrs)
{
    if (strlen(addrs) == strlen(MACADDR_FORMAT) && ':' == addrs[2] && ':' == addrs[5] && ':' == addrs[8]
        && ':' == addrs[11] && ':' == addrs[14])
    {
        addrs[2]  = '\0';
        addrs[5]  = '\0';
        addrs[8]  = '\0';
        addrs[11] = '\0';
        addrs[14] = '\0';

        MY_MAC[0] = (u8)simple_strtoul(&(addrs[0]), NULL, 16);
        MY_MAC[1] = (u8)simple_strtoul(&(addrs[3]), NULL, 16);
        MY_MAC[2] = (u8)simple_strtoul(&(addrs[6]), NULL, 16);
        MY_MAC[3] = (u8)simple_strtoul(&(addrs[9]), NULL, 16);
        MY_MAC[4] = (u8)simple_strtoul(&(addrs[12]), NULL, 16);
        MY_MAC[5] = (u8)simple_strtoul(&(addrs[15]), NULL, 16);

        /* set back to ':' or the environment variable would be destoried */ // REVIEW: this coding style is dangerous
        addrs[2]  = ':';
        addrs[5]  = ':';
        addrs[8]  = ':';
        addrs[11] = ':';
        addrs[14] = ':';
    }

    return 1;
}

__setup("macaddr=", macaddr_auto_config_setup);
#endif

#if KERNEL_PHY
static int Dev_EMAC_mii_write(struct mii_bus *bus, int phy_addr, int phy_reg, u16 val)
{
    struct emac_handle *hemac = (struct emac_handle *)bus->priv;
    int                 ret;

    ret = Hal_EMAC_write_phy(hemac->hal, phy_addr, phy_reg, (u32)val);
    return ret;
}

static int Dev_EMAC_mii_read(struct mii_bus *bus, int phy_addr, int phy_reg)
{
    u32                 val;
    struct emac_handle *hemac = (struct emac_handle *)bus->priv;
    int                 ret;

    ret = Hal_EMAC_read_phy(hemac->hal, phy_addr, phy_reg, &val);
    return (int)val;
}

static int Dev_EMAC_mii_init(struct net_device *emac_dev)
{
    struct emac_handle *hemac  = (struct emac_handle *)netdev_priv(emac_dev);
    struct device_node *mii_np = NULL;
    int                 ret    = 0;

    // the force internal mdio bus in FPGA
    if (1)
    {
        u32 mdio_path = 0;
        if (!of_property_read_u32(emac_dev->dev.of_node, "mdio_path", &mdio_path))
        {
            Hal_EMAC_mdio_path(hemac->hal, mdio_path);
        }
    }

    if (!(mii_np = of_get_child_by_name(emac_dev->dev.of_node, "mdio-bus")))
    {
        // printk("[%s][%d] no child node of mdio-bus is found\n", __FUNCTION__, __LINE__);
        return -1;
    }
    if (!of_device_is_available(mii_np))
    {
        // printk("[%s][%d] mii_np is unavailable\n", __FUNCTION__, __LINE__);
        ret = -2;
        goto jmp_err_put_node;
    }
    if (!(hemac->mii_bus = devm_mdiobus_alloc(hemac->dev)))
    {
        // printk("[%s][%d] devm_mdiobus_alloc fail\n", __FUNCTION__, __LINE__);
        ret = -3;
        goto jmp_err_put_node;
    }

    hemac->mii_bus->name   = "mdio";
    hemac->mii_bus->read   = Dev_EMAC_mii_read;
    hemac->mii_bus->write  = Dev_EMAC_mii_write;
    hemac->mii_bus->priv   = hemac;
    hemac->mii_bus->parent = hemac->dev;

    snprintf(hemac->mii_bus->id, MII_BUS_ID_SIZE, "%s@%s", mii_np->name, hemac->name);
    ret = of_mdiobus_register(hemac->mii_bus, mii_np);
jmp_err_put_node:
    of_node_put(mii_np);
    return ret;
}

static void Dev_EMAC_mii_uninit(struct net_device *emac_dev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(emac_dev);
    if (!hemac->mii_bus)
        return;
    mdiobus_unregister(hemac->mii_bus);
}

#endif

#if defined(CONFIG_SSTAR_PADMUX)
static void Dev_EMAC_PHYHW_reset(struct emac_handle *hemac)
{
    if (drv_padmux_active() && hemac->phy_mode == PHY_INTERFACE_MODE_RMII)
    {
        int gpio_no;
        if (PAD_UNKNOWN != (gpio_no = drv_padmux_getpad(MDRV_PUSE_ETH0_PHY_RESET)))
        {
            sstar_gpio_set_low(gpio_no);
            mdelay(CONFIG_EMAC0_PHY_RESET_HOLD_MS);
            sstar_gpio_set_high(gpio_no);
            mdelay(CONFIG_EMAC0_PHY_RESET_WAIT_READY_MS);
            printk("[%s] EMAC0 PHY RESET \n", __FUNCTION__);
        }
        if (PAD_UNKNOWN != (gpio_no = drv_padmux_getpad(MDRV_PUSE_ETH1_PHY_RESET)))
        {
            sstar_gpio_set_low(gpio_no);
            mdelay(CONFIG_EMAC1_PHY_RESET_HOLD_MS);
            sstar_gpio_set_high(gpio_no);
            mdelay(CONFIG_EMAC1_PHY_RESET_WAIT_READY_MS);
            printk("[%s] EMAC1 PHY RESET \n", __FUNCTION__);
        }
    }
}
#endif

static int Dev_EMAC_init(struct platform_device *pdev)
{
    struct emac_handle *hemac;
    int                 ret;
    struct net_device * emac_dev = NULL;
    unsigned long       flags;

    emac_dev = alloc_etherdev(sizeof(*hemac));
    hemac    = netdev_priv(emac_dev);
    // printk("[%s][%d] alloc netdev = 0x%08x\n", __FUNCTION__, __LINE__, (int)emac_dev);

    if (!emac_dev)
    {
        EMAC_ERR(KERN_ERR "No EMAC dev mem!\n");
        return -ENOMEM;
    }

    // led gpio
#if PHY_LED_CTL
    hemac->led_orange      = -1;
    hemac->led_green       = -1;
    hemac->led_count       = 0;
    hemac->led_flick_speed = 30;
#endif

#if PHY_LED_INVERT
    hemac->led_invert = 0;
#endif

    // phy address
    // hemac->phyaddr = 0xff;

    ////////////////////////////////////////////////////////////////////////////
    hemac->gu32CheckLinkTime    = HZ;
    hemac->gu32CheckLinkTimeDis = 100;
    hemac->gu32intrEnable       = 0;
    memset(hemac->irq_count, 0, sizeof(hemac->irq_count));

    hemac->skb_tx_send = 0;
    hemac->skb_tx_free = 0;
    hemac->data_done   = 0;
    memset(&hemac->data_time_last, 0, sizeof(hemac->data_time_last));
    spin_lock_init(&hemac->emac_data_done_lock);

    hemac->txPkt = 0;
    hemac->txInt = 0;

    hemac->phy_status_register = 0x78c9UL;

    hemac->initstate = 0;
    hemac->contiROVR = 0;

    hemac->oldTime       = 0;
    hemac->PreLinkStatus = 0;

    // hemac->phy_type = 0;
    hemac->irq_emac = 0;
    hemac->irq_lan  = 0;

    hemac->name    = NULL;
    hemac->bInit   = 0;
    hemac->bEthCfg = 0;

    hemac->u8Minor = _u8Minor;
    _u8Minor++;
#if KERNEL_PHY
    hemac->dev = &pdev->dev;
#endif

    ////////////////////////////////////////////////////////////////////////////

    emac_dev->dev.of_node = pdev->dev.of_node; // pass of_node to Dev_EMAC_setup()

    SET_NETDEV_DEV(emac_dev, hemac->dev);
    // emac_dev->ethtool_ops = &sstar_emac_ethtool_ops;
    netdev_set_default_ethtool_ops(emac_dev, &sstar_emac_ethtool_ops);

    Dev_EMAC_dts(emac_dev);
    hemac->hal = Hal_EMAC_Alloc(hemac->emacRIU, hemac->emacX32, hemac->phyRIU);
    Hal_EMAC_Pad(hemac->hal, hemac->pad_reg, hemac->pad_msk, hemac->pad_val);
    Hal_EMAC_PadLed(hemac->hal, hemac->pad_led_reg, hemac->pad_led_msk, hemac->pad_led_val);
    Hal_EMAC_PhyMode(hemac->hal, hemac->phy_mode);

#if defined(CONFIG_SSTAR_PADMUX)
    Dev_EMAC_PHYHW_reset(hemac);
#endif

#if TX_THROUGHPUT_TEST
    printk("==========TX_THROUGHPUT_TEST===============");
    pseudo_packet = alloc_skb(EMAC_PACKET_SIZE_MAX, GFP_ATOMIC);
    memcpy(pseudo_packet->data, (void *)packet_content, sizeof(packet_content));
    pseudo_packet->len = sizeof(packet_content);
#endif

#if RX_THROUGHPUT_TEST
    printk("==========RX_THROUGHPUT_TEST===============");
    init_timer(&RX_timer);

    RX_timer.data     = EMAC_RX_TMR;
    RX_timer.function = RX_timer_callback;
    RX_timer.expires  = jiffies + 20 * EMAC_CHECK_LINK_TIME;
    add_timer(&RX_timer);
#endif
    hemac->bEmacClkOn = 0;
    Dev_EMAC_ClkEnable(hemac);

#if PHY_LED_INVERT
    if (hemac->led_invert)
    {
        Hal_EMAC_Set_Invert_LED(hemac->hal);
    }
#endif

#if HW_FLOW_CONTROL
    if (_ethtool_rx_pause)
    {
        // when rx queue frame is more than RX_DESC_NUM hw will send a pause frame
        Hal_EMAC_Flow_Control_Init(hemac->hal, 1, 0xffff);               // send pause frame onece 1ms
        Hal_EMAC_RX_Flow_Ctrl_Enable(hemac->hal, 0);                     // disable hw triger
        Hal_EMAC_RX_Flow_Ctrl_Enable(hemac->hal, 1);                     // enable hw triger
        Hal_EMAC_Flow_control_Auto_Trig_th(hemac->hal, RX_DESC_NUM - 1); // set auto trig threshold
    }
    if (_ethtool_tx_pause)
    {
        Hal_EMAC_TX_Flow_Ctrl_Enable(hemac->hal,
                                     1); // when hw receive a pause frame hw will stop tx queue pause time
    }
#endif

#if NETWORK_STORM_PROTECT
    {
        Hal_EMAC_Netsp_Unicast_Setting(hemac->hal, TX_MAX_DEFAULT, TX_CONSUME_LEVEL_5, 1);
        Hal_EMAC_Netsp_Multicast_Setting(hemac->hal, TX_MAX_DEFAULT, TX_CONSUME_LEVEL_5, 1);
        Hal_EMAC_Netsp_Broadcast_Setting(hemac->hal, TX_MAX_DEFAULT, TX_CONSUME_LEVEL_5, 1);
    }
#endif
#if SSTAR_EMAC_NAPI
    netif_napi_add(emac_dev, &hemac->napi, Dev_EMAC_napi_poll, EMAC_NAPI_WEIGHT);
#endif

    emac_dev->netdev_ops = &sstar_lan_netdev_ops;
    if (Dev_EMAC_probe(emac_dev))
        goto end;

#if KERNEL_PHY
    Dev_EMAC_mii_init(emac_dev);
#endif

    if ((ret = register_netdev(emac_dev)))
        goto jmp_mii_exit;

    // printk( KERN_ERR "[EMAC]Init EMAC success! (add delay in reset)\n" );
    platform_set_drvdata(pdev, (void *)emac_dev);
    hemac->bInit = 1;
    return 0;

jmp_mii_exit:
#if KERNEL_PHY
    Dev_EMAC_mii_uninit(emac_dev);
#endif
end:
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_destroy(&(hemac->skb_queue_tx));
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);
    free_netdev(emac_dev);
    emac_dev         = NULL;
    hemac->initstate = ETHERNET_TEST_INIT_FAIL;
    EMAC_ERR(KERN_ERR "Init EMAC error!\n");
    return -1;
}

static void Dev_EMAC_dts(struct net_device *netdev)
{
    struct emac_handle *hemac = (struct emac_handle *)netdev_priv(netdev);
    phy_interface_t     phy_interface;
    // const char* tmp_str = NULL;
#if PHY_LED_CTL || PHY_LED_INVERT
    unsigned int led_data;
#endif
    unsigned int bus_mode;
    // struct resource *res;
    struct resource res;
    u32             val[3];

    hemac->irq_emac = irq_of_parse_and_map(netdev->dev.of_node, 0);
    hemac->irq_lan  = irq_of_parse_and_map(netdev->dev.of_node, 1);
    // printk("[%s][%d] (irq_emac, irq_lan) = (%d, %d)\n", __FUNCTION__, __LINE__, hemac->irq_emac, hemac->irq_lan);

    if (of_property_read_u32(netdev->dev.of_node, "cpu-affinity", &hemac->cpu_affinity))
    {
        hemac->cpu_affinity = CPU_AFFINITY; // default value
    }

    if (of_property_read_u32(netdev->dev.of_node, "txd_num", &hemac->txd_num))
    {
        hemac->txd_num = TXD_NUM; // default value
    }
    hemac->txd_num = (hemac->txd_num + 0xFF) & 0x100; // 256 alignment
    if (of_property_read_u32(netdev->dev.of_node, "txq_num_sw", &hemac->txq_num_sw))
    {
        hemac->txq_num_sw = TXQ_NUM_SW; // default value
    }

#if PHY_LED_CTL
    if (!of_property_read_u32(netdev->dev.of_node, "led-orange", &led_data))
    {
        hemac->led_orange = (unsigned char)led_data;
        // printk(KERN_ERR "[EMAC]Set emac_led_orange=%d\n",led_data);
    }

    if (!of_property_read_u32(netdev->dev.of_node, "led-green", &led_data))
    {
        hemac->led_green = (unsigned char)led_data;
        // printk(KERN_ERR "[EMAC]Set emac_led_green=%d\n",led_data);
    }
#endif

    if (!of_property_read_u32(netdev->dev.of_node, "bus-mode", &bus_mode))
    {
        hemac->bus_mode = (unsigned char)bus_mode;
        // printk(KERN_ERR "[EMAC]Set bus_mode=%d\n",bus_mode);
    }
    else
    {
        hemac->bus_mode = 0;
    }

#if PHY_LED_INVERT
    if (!of_property_read_u32(netdev->dev.of_node, "led-invert", &led_data))
    {
        hemac->led_invert = (unsigned char)led_data;
        // printk(KERN_ERR "[EMAC]Set emac led-invert=%d\n",led_data);
    }
#endif

    // bank of emac & phy
    if (!of_address_to_resource(netdev->dev.of_node, 0, &res))
    {
        hemac->emacRIU = IO_ADDRESS(res.start);
        EMAC_DBG("[%s][%d] (emacRIU, start) = (0x%08lx, 0x%08lx)\n", __FUNCTION__, __LINE__, hemac->emacRIU,
                 (unsigned long)res.start);
    }
    else
    {
        hemac->emacRIU = 0x1F2A2000;
        hemac->emacRIU = IO_ADDRESS(hemac->emacRIU);
        EMAC_DBG("[%s][%d] (emacRIU) = (0x%08lx)\n", __FUNCTION__, __LINE__, hemac->emacRIU);
    }
    if (!of_address_to_resource(netdev->dev.of_node, 1, &res))
    {
        hemac->emacX32 = IO_ADDRESS(res.start);
        EMAC_DBG("[%s][%d] (emacX32, start) = (0x%08lx, 0x%08lx)\n", __FUNCTION__, __LINE__, hemac->emacX32,
                 (unsigned long)res.start);
    }
    else
    {
        hemac->emacX32 = 0x1F343C00;
        hemac->emacX32 = IO_ADDRESS(hemac->emacX32);
        EMAC_DBG("[%s][%d] (emacX32) = (0x%08lx)\n", __FUNCTION__, __LINE__, hemac->emacX32);
    }
    if (hemac->bus_mode == 2)
    {
        hemac->phyRIU = 0;
    }
    else
    {
        if (!of_address_to_resource(netdev->dev.of_node, 2, &res))
        {
            hemac->phyRIU = (res.start) ? IO_ADDRESS(res.start) : 0;
            // printk("[%s][%d] (phyRIU, start) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, hemac->phyRIU,
            // res.start);
        }
        else
        {
            hemac->phyRIU = IO_ADDRESS(hemac->phyRIU);
            // printk("[%s][%d] (phyRIU) = (0x%08x)\n", __FUNCTION__, __LINE__, hemac->phyRIU);
        }
    }

    val[0] = val[1] = val[2] = 0;
    if (hemac->bus_mode == 2)
    {
        of_property_read_u32_array(netdev->dev.of_node, "pad-rmii", val, 3);
    }
    else
    {
        of_property_read_u32_array(netdev->dev.of_node, "pad", val, 3);
    }
    hemac->pad_reg = IO_ADDRESS(val[0]);
    hemac->pad_msk = val[1];
    hemac->pad_val = val[2];

    val[0] = val[1] = val[2] = 0;
    of_property_read_u32_array(netdev->dev.of_node, "pad_led", val, 3);
    hemac->pad_led_reg = (val[0]) ? IO_ADDRESS(val[0]) : 0;
    hemac->pad_led_msk = val[1];
    hemac->pad_led_val = val[2];

    // printk("[%s][%d] pad (reg, msk, val) = (0x%08x 0x%08x 0x%08x)\n", __FUNCTION__, __LINE__, hemac->pad_reg,
    // hemac->pad_msk, hemac->pad_val);

    {
        // struct device_node* np = NULL;
        int phy_mode;

        hemac->phy_mode = PHY_INTERFACE_MODE_RMII;

        if (hemac->bus_mode != 2)
        {
            if (0 > (phy_mode = of_get_phy_mode(netdev->dev.of_node, &phy_interface)))
            {
                struct device_node *np = NULL;
                np                     = of_parse_phandle(netdev->dev.of_node, "phy-handle", 0);
                if ((np) && (0 <= (phy_mode = of_get_phy_mode(np, &phy_interface))))
                {
                    hemac->phy_mode = phy_mode;
                }
                if (np)
                    of_node_put(np);
            }
            else
            {
                hemac->phy_mode = phy_mode;
            }
        }
    }
    hemac->name = netdev->dev.of_node->name;
}

#if REDUCE_CPU_FOR_RBNA
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static void _Dev_EMAC_IntRX_CB(struct timer_list *t)
#else
static void _Dev_EMAC_IntRX_CB(unsigned long data)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    struct emac_handle *hemac = from_timer(hemac, t, timerIntRX);
#else
    struct emac_handle *hemac = (struct emac_handle *)data;
#endif
    unsigned long flags;

    spin_lock_irqsave(&hemac->mutexIntRX, flags);
    Hal_EMAC_RX_ParamSet(hemac->hal, 0x01, 0x10);
    // Hal_EMAC_RX_ParamRestore(hemac->hal);
    spin_unlock_irqrestore(&hemac->mutexIntRX, flags);
}
#endif // #if REDUCE_CPU_FOR_RBNA

#if EMAC_FLOW_CONTROL_TX
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static void _Dev_EMAC_FlowTX_CB(struct timer_list *t)
#else
static void _Dev_EMAC_FlowTX_CB(unsigned long data)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    struct emac_handle *hemac = from_timer(hemac, t, timerFlowTX);
#else
    struct emac_handle *hemac = (struct emac_handle *)data;
#endif
    unsigned long flags;

    spin_lock_irqsave(&hemac->mutexFlowTX, flags);
    netif_wake_queue(hemac->netdev);
    spin_unlock_irqrestore(&hemac->mutexFlowTX, flags);
}

static int _MDrv_EMAC_is_PausePkt(struct sk_buff *skb, unsigned char *p_recv)
{
    unsigned int mac_ctl_opcode = 0;

    if ((MAC_CONTROL_TYPE & 0xFF) != ((skb->protocol >> 8) & 0xFF))
        return 0;
    if (((MAC_CONTROL_TYPE >> 8) & 0xFF) != (skb->protocol & 0xFF))
        return 0;
    mac_ctl_opcode = (((*(p_recv + 14)) << 8) & 0xFF00) + ((*(p_recv + 15)) & 0xFF);
    return (MAC_CONTROL_OPCODE == mac_ctl_opcode) ? 1 : 0;
}

static int _MDrv_EMAC_Pause_TX(struct net_device *dev, struct sk_buff *skb, unsigned char *p_recv)
{
    struct emac_handle *hemac                 = (struct emac_handle *)netdev_priv(dev);
    int                 pause_time            = 0;
    int                 pause_time_to_jiffies = 0;
    unsigned long       flags;
    unsigned long       expires;

#if EMAC_FLOW_CONTROL_TX_TEST
    {
        static unsigned int cnt = 0;
        cnt++;
        if ((cnt & 0xF) == 0)
        {
            // printk("[%s][%d] pseudo pause packet\n", __FUNCTION__, __LINE__);
            pause_time = EMAC_FLOW_CONTROL_TX_TEST_TIME;
            goto jump_pause_tx_test;
        }
    }
#endif // #if EMAC_FLOW_CONTROL_TX_TEST

    if (0 == _MDrv_EMAC_is_PausePkt(skb, p_recv))
        return 0;

    pause_time = (((*(p_recv + 16)) << 8) & 0xFF00) + ((*(p_recv + 17)) & 0xFF);

#if EMAC_FLOW_CONTROL_TX_TEST
jump_pause_tx_test:
#endif // #if EMAC_FLOW_CONTROL_TX_TEST

    if (SPEED_100 == dev->phydev->speed)
    {
        pause_time_to_jiffies =
            (pause_time / PAUSE_TIME_DIVISOR_100M) + ((0 == (pause_time % PAUSE_TIME_DIVISOR_100M)) ? 0 : 1);
        expires = jiffies + pause_time_to_jiffies;
    }
    else if (SPEED_10 == dev->phydev->speed)
    {
        pause_time_to_jiffies =
            (pause_time / PAUSE_TIME_DIVISOR_10M) + ((0 == (pause_time % PAUSE_TIME_DIVISOR_10M)) ? 0 : 1);
        expires = jiffies + pause_time_to_jiffies;
    }
    else
    {
        printk("[%s][%d] Get emac speed error : %d\n", __FUNCTION__, __LINE__, (int)dev->phydev->speed);
        return 0;
    }
    spin_lock_irqsave(&hemac->mutexFlowTX, flags);

    // if (0 == hemac->isPauseTX)
    if (0 == timer_pending(&hemac->timerFlowTX))
    {
        netif_stop_queue(dev);
        hemac->timerFlowTX.expires = expires;
        // hemac->isPauseTX = 1;
        add_timer(&hemac->timerFlowTX);
    }
    else
    {
        mod_timer(&hemac->timerFlowTX, expires);
    }
    spin_unlock_irqrestore(&hemac->mutexFlowTX, flags);
    return 1;
}
#endif // #if EMAC_FLOW_CONTROL_TX

//-------------------------------------------------------------------------------------------------
// EMAC exit module
//-------------------------------------------------------------------------------------------------
static void Dev_EMAC_exit(struct platform_device *pdev)
{
    struct net_device * emac_dev = (struct net_device *)platform_get_drvdata(pdev);
    struct emac_handle *hemac    = (struct emac_handle *)netdev_priv(emac_dev);
    unsigned long       flags;

    if (!emac_dev)
        return;

#if EMAC_FLOW_CONTROL_TX
    del_timer(&hemac->timerFlowTX);
#endif // #if EMAC_FLOW_CONTROL_TX

#if REDUCE_CPU_FOR_RBNA
    del_timer(&hemac->timerIntRX);
#endif // #if REDUCE_CPU_FOR_RBNA
#if PHY_MDI_CTL
    cancel_delayed_work_sync(&hemac->mdix_work);
#endif
#if DYNAMIC_INT_TX_TIMER
#if DYNAMIC_INT_TX_TIMER_HR
    hrtimer_cancel(&hemac->timerTxWdt);
#else
    del_timer(&hemac->timerTxWdt);
#endif
#endif

#if KERNEL_PHY
    Dev_EMAC_mii_uninit(emac_dev);
#endif
    if (hemac->irq_emac)
    {
        if (hemac->cpu_affinity)
        {
            irq_set_affinity_hint(hemac->irq_emac, NULL);
        }
        free_irq(hemac->irq_emac, emac_dev);
    }
#ifdef LAN_ESD_CARRIER_INTERRUPT
    if (hemac->irq_lan)
        free_irq(hemac->irq_lan, emac_dev);
#endif
#if PHY_LED_CTL
    if (hemac->led_orange != -1)
    {
        sstar_gpio_set_low(hemac->led_orange);
        sstar_gpio_pad_clr(hemac->led_orange);
    }
    if (hemac->led_green != -1)
    {
        sstar_gpio_set_low(hemac->led_green);
        sstar_gpio_pad_clr(hemac->led_green);
    }
#endif
    if (hemac->sstar_class_emac_device)
    {
        device_destroy(msys_get_sysfs_class(), MKDEV(MAJOR(gEthDev), hemac->u8Minor));
        unregister_chrdev_region(gEthDev, MINOR_EMAC_NUM);
    }

    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_destroy(&(hemac->skb_queue_tx));
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);

    free_rx_skb(hemac);
    Dev_EMAC_RX_Desc_Free(hemac);
    Dev_EMAC_MemFree(hemac);

    unregister_netdev(emac_dev);
}

static int sstar_emac_drv_suspend(struct platform_device *dev, pm_message_t state)
{
    // struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct net_device * netdev = (struct net_device *)platform_get_drvdata(dev);
    struct emac_handle *hemac;
    unsigned long       flags;

    // printk(KERN_INFO "sstar_emac_drv_suspend\n");
    if (!netdev)
    {
        return -1;
    }

    hemac = (struct emac_handle *)netdev_priv(netdev);
    if (!(hemac->ep_flag & EP_FLAG_OPEND))
    {
        Dev_EMAC_ClkEnable(hemac);
    }
    hemac->ep_flag |= EP_FLAG_SUSPENDING;
    netif_stop_queue(netdev);

    disable_irq(netdev->irq);
    // del_timer(&hemac->timer_link);

    // corresponds with resume call Dev_EMAC_open
#if SSTAR_EMAC_NAPI
    if (!strncmp(dev->name, "eth0", strlen("eth0")))
    {
        if (napi_enable_flag[0] == 1)
            napi_disable(&hemac->napi);
        napi_enable_flag[0] = 0;
    }
    else
    {
        if (napi_enable_flag[1] == 1)
            napi_disable(&hemac->napi);
        napi_enable_flag[1] = 0;
    }
#endif

    // Disable Receiver and Transmitter //
    Dev_EMAC_stop(netdev);

#ifdef TX_SW_QUEUE
    // make sure that TX HW FIFO is empty
    while (TX_FIFO_SIZE != Hal_EMAC_TXQ_Free(hemac->hal))
        ;
#endif
    while (!Hal_EMAC_TXQ_Empty(hemac->hal))
    {
        msleep(1);
    }

    // Disable PHY interrupt //
    Hal_EMAC_disable_phyirq(hemac->hal);

    Hal_EMAC_IntEnable(hemac->hal, 0xFFFFFFFF, 0);

    // Dev_EMAC_SwReset(netdev);
    Dev_EMAC_ClkDisable(hemac);
#ifdef TX_SW_QUEUE
    _Dev_EMAC_tx_reset_TX_SW_QUEUE(netdev);
#endif
    spin_lock_irqsave(&hemac->mutexTXQ, flags);
    skb_queue_reset(&(hemac->skb_queue_tx));
    spin_unlock_irqrestore(&hemac->mutexTXQ, flags);

    free_rx_skb(hemac);

    if (phy_is_started(netdev->phydev))
    {
        phy_stop(netdev->phydev);
    }

    Dev_EMAC_RX_Desc_Reset(hemac);

    // phy_link_adjust
    hemac->bEthCfg = 0;

    return 0;
}

static int sstar_emac_drv_resume(struct platform_device *dev)
{
    // struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct net_device * netdev = (struct net_device *)platform_get_drvdata(dev);
    struct emac_handle *hemac;
    unsigned long       flags;

    // printk(KERN_INFO "sstar_emac_drv_resume\n");
    if (!netdev)
    {
        return -1;
    }
    hemac = (struct emac_handle *)netdev_priv(netdev);
#if defined(CONFIG_SSTAR_PADMUX)
    Dev_EMAC_PHYHW_reset(hemac);
#endif
    hemac->ep_flag &= ~EP_FLAG_SUSPENDING;
    Dev_EMAC_ClkEnable(hemac);

    // Hal_EMAC_Write_JULIAN_0100(hemac->hal, JULIAN_100_VAL);
    Hal_EMAC_Write_JULIAN_0100(hemac->hal, 0);

    spin_lock_irqsave(&hemac->mutexPhy, flags);
    /*
        if (0 > Dev_EMAC_ScanPhyAddr(netdev))
            return -1;

        Dev_EMAC_Patch_PHY(netdev);
    */
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);

    // hemac->ThisUVE.initedEMAC = 0;
    Dev_EMAC_HW_init(netdev);

#if ICPLUS_STR_FIX
    netdev->phydev->drv->config_init(netdev->phydev);
#endif

    spin_lock_irqsave(&hemac->mutexPhy, flags);
    Dev_EMAC_update_mac_address(netdev); // Program ethernet address into MAC //
    Hal_EMAC_enable_mdi(hemac->hal);
    spin_unlock_irqrestore(&hemac->mutexPhy, flags);
    enable_irq(netdev->irq);
    if (hemac->ep_flag & EP_FLAG_OPEND)
    {
        if (0 > Dev_EMAC_open(netdev))
        {
            printk(KERN_WARNING "Driver Emac: open failed after resume\n");
        }
        else
        {
            Dev_EMAC_set_rx_mode(netdev);
        }
    }
    else
    {
        Dev_EMAC_ClkDisable(hemac);
    }

    return 0;
}

static int sstar_emac_drv_probe(struct platform_device *pdev)
{
    int                 retval = 0;
    struct net_device * netdev;
    struct emac_handle *hemac;

    if (!(pdev->name) || strcmp(pdev->name, "Sstar-emac") || pdev->id != 0)
    {
        retval = -ENXIO;
    }

    if ((retval = Dev_EMAC_init(pdev)))
        return retval;

    netdev = (struct net_device *)platform_get_drvdata(pdev);
    hemac  = (struct emac_handle *)netdev_priv(netdev);

#if PHY_LED_CTL
    if (hemac->led_orange != -1)
    {
        sstar_gpio_pad_set(hemac->led_orange);
    }
    if (hemac->led_green != -1)
    {
        sstar_gpio_pad_set(hemac->led_green);
    }
#endif

#if PHY_MDI_CTL
    if (!strncmp(netdev->name, "eth0", strlen("eth0")))
    {
        INIT_DELAYED_WORK(&hemac->mdix_work, Dev_EMAC_mdix_work_handle);
        schedule_delayed_work(&hemac->mdix_work, msecs_to_jiffies(INT_PHY_MDIX_TIMER_MS));
    }
#endif
    Dev_EMAC_ClkDisable(hemac);

    return retval;
}

static int sstar_emac_drv_remove(struct platform_device *pdev)
{
    struct net_device * emac_dev = (struct net_device *)platform_get_drvdata(pdev);
    struct emac_handle *hemac    = (struct emac_handle *)netdev_priv(emac_dev);

    Dev_EMAC_ClkEnable(hemac);
    Dev_EMAC_exit(pdev);
    Dev_EMAC_ClkDisable(hemac);

    /* hal is still used at disabling clocks, so let's free it here */
    Hal_EMAC_Free(hemac->hal);
    if (emac_dev)
    {
        free_netdev(emac_dev);
    }

    platform_set_drvdata(pdev, NULL);
    return 0;
}

static struct platform_driver sstar_emac_driver = {.probe   = sstar_emac_drv_probe,
                                                   .remove  = sstar_emac_drv_remove,
                                                   .suspend = sstar_emac_drv_suspend,
                                                   .resume  = sstar_emac_drv_resume,

                                                   .driver = {
                                                       .name = "Sstar-emac",
#if defined(CONFIG_OF)
                                                       .of_match_table = sstaremac_of_device_ids,
#endif
                                                       .owner = THIS_MODULE,
                                                   }};

static int __init sstar_emac_drv_init_module(void)
{
    int retval = 0;

    retval = platform_driver_register(&sstar_emac_driver);
    if (retval)
    {
        // printk(KERN_INFO"sstar_emac_driver register failed...\n");
        return retval;
    }

    return retval;
}

static void __exit sstar_emac_drv_exit_module(void)
{
    platform_driver_unregister(&sstar_emac_driver);
    // emac_dev=NULL;
}

module_init(sstar_emac_drv_init_module);
module_exit(sstar_emac_drv_exit_module);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("EMAC Ethernet driver");
MODULE_LICENSE("GPL");
