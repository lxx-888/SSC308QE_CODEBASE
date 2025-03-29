/*
 * drv_emac.h- Sigmastar
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
#ifndef __DRV_EMAC_H_
#define __DRV_EMAC_H_

#define EMAC_DBG(fmt, args...)                            \
    {                                                     \
        printk(KERN_DEBUG "SSTAR_EMAC_DBG " fmt, ##args); \
    }
#define EMAC_ERR(fmt, args...)                          \
    {                                                   \
        printk(KERN_ERR "SSTAR_EMAC_ERR " fmt, ##args); \
    }
#define EMAC_INFO(fmt, args...)                           \
    {                                                     \
        printk(KERN_INFO "SSTAR_EMAC_INFO:" fmt, ##args); \
    }

#define MINOR_EMAC_NUM 1
#define MAJOR_EMAC_NUM 241

#define INT_PHY_MDIX_PATCH    0
#define INT_PHY_MDIX_TIMER_MS 3000

/////////////////////////////////
// to be refined
/////////////////////////////////

#define EMAC_SG           1
#define EMAC_SG_BUF_CACHE 1

#define EMAC_GSO 1

// #define DYNAMIC_INT_TX_TH       64
#define DYNAMIC_INT_TX          1
#define DYNAMIC_INT_TX_TIMER    0
#define DYNAMIC_INT_TX_TIMER_HR 0

#if RX_DELAY_INT_DEBUG
#define DYNAMIC_INT_RX      0
#define REDUCE_CPU_FOR_RBNA 0
#define SSTAR_EMAC_NAPI     0
#else
#define DYNAMIC_INT_RX      1
#define REDUCE_CPU_FOR_RBNA 1
#define SSTAR_EMAC_NAPI     1
#endif

#define EMAC_FLOW_CONTROL_RX      1
#define EMAC_FLOW_CONTROL_RX_TEST 0

#define EMAC_FLOW_CONTROL_TX           1
#define EMAC_FLOW_CONTROL_TX_TEST      0
#define EMAC_FLOW_CONTROL_TX_TEST_TIME 0x200

#define EMAC_LOOPBACK_DEBUG 0

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
#define EMAC_MTU (1524)

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
// #define DRV_EMAC_MAX_DEV                        0x1

//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------
/*
phys_addr_t     RAM_VA_BASE;                      //= 0x00000000;     // After init, RAM_ADDR_BASE = EMAC_ABSO_MEM_BASE
phys_addr_t     RAM_PA_BASE;
phys_addr_t     RAM_VA_PA_OFFSET;
phys_addr_t     RBQP_BASE;                          //= RX_BUFFER_SIZE;//0x00004000;     // IMPORTANT: lowest 13 bits as
zero.
*/

#define ETHERNET_TEST_NO_LINK          0x00000000UL
#define ETHERNET_TEST_AUTO_NEGOTIATION 0x00000001UL
#define ETHERNET_TEST_LINK_SUCCESS     0x00000002UL
#define ETHERNET_TEST_RESET_STATE      0x00000003UL
#define ETHERNET_TEST_SPEED_100M       0x00000004UL
#define ETHERNET_TEST_DUPLEX_FULL      0x00000008UL
#define ETHERNET_TEST_INIT_FAIL        0x00000010UL

u8 MY_DEV[16]                = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
u8 MY_MAC[6]                 = {0x00UL, 0x30UL, 0x1BUL, 0xBAUL, 0x02UL, 0xDBUL};
u8 PC_MAC[6]                 = {0x00UL, 0x1AUL, 0x4BUL, 0x5CUL, 0x39UL, 0xDFUL};
u8 ETH_PAUSE_FRAME_DA_MAC[6] = {0x01UL, 0x80UL, 0xC2UL, 0x00UL, 0x00UL, 0x01UL};

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
struct rbf_t
{
    u32 addr;
    u32 size;
} __attribute__((packed));

#define EP_FLAG_OPEND      0X00000001UL
#define EP_FLAG_SUSPENDING 0X00000002UL

typedef struct
{
    // u8 used;
    struct sk_buff* skb;      /* holds skb until xmit interrupt completes */
    dma_addr_t      skb_phys; /* phys addr from pci_map_single */
    int             skb_len;
} skb_info;

typedef struct
{
    skb_info* skb_info_arr;
    int       read;
    int       write;
    int       rw;
    int       size[2];
} skb_queue;

typedef struct
{
    // int off_va_pa;
    // skb_queue skb_queue_rx;
    struct rbf_t*    desc;
    dma_addr_t       desc_phys;
    struct sk_buff** skb_arr;
#if EXT_PHY_PATCH
    dma_addr_t skb_phys;
#endif
    int num_desc;
    int size_desc_queue;
    int idx;
} rx_desc_queue_t;

struct emac_handle
{
    struct net_device_stats stats;

    /* PHY */
    // unsigned long phy_type;             /* type of PHY (PHY_ID) */

    spinlock_t mutexNetIf;
    spinlock_t mutexTXQ;
    spinlock_t mutexPhy;

    /* Transmit */
    skb_queue skb_queue_tx;

    unsigned int irqcnt;
    unsigned int tx_irqcnt;

    /* Receive */
    // spinlock_t mutexRXD;
    rx_desc_queue_t rx_desc_queue;
#if EXT_PHY_PATCH
    char* pu8RXBuf;
#endif

    /* Suspend and resume */
    unsigned long ep_flag;

    struct net_device* netdev;

    struct device* sstar_class_emac_device;
#if SSTAR_EMAC_NAPI
    struct napi_struct napi;
#else
    spinlock_t mutexRXInt;
#endif
    MSYS_DMEM_INFO mem_info;

    u32 txd_num;
    u32 txq_num_sw;

    // mdix workqueue
    struct delayed_work mdix_work;

    // led gpio
    int led_orange;
    int led_green;
    int led_count;
    int led_flick_speed;
    int led_invert;

    // 0:Not support select 1:MII 2:RMII
    int bus_mode;

    // mac address
    u8 sa[4][6];

    // BasicConfigEMAC ThisBCE;
    // UtilityVarsEMAC ThisUVE;

    //
    unsigned int irq_emac;
    unsigned int irq_lan;

    //
    unsigned long emacRIU;
    unsigned long emacX32;
    unsigned long phyRIU;

    //
    unsigned long pad_reg;
    unsigned long pad_msk;
    unsigned long pad_val;

    u32 phy_mode;

    // led
    unsigned long pad_led_reg;
    unsigned long pad_led_msk;
    unsigned long pad_led_val;

    // hal handle
    void* hal;

    ////////////////
    u32 gu32CheckLinkTime;
    u32 gu32CheckLinkTimeDis;
    u32 gu32intrEnable;
    u32 irq_count[16];

    u64               skb_tx_send;
    u64               skb_tx_free;
    u64               data_done;
    struct timespec64 data_time_last;
    spinlock_t        emac_data_done_lock;

    u32 txPkt;
    u32 txInt;
    u32 txIntDly;
    u32 txIntTo;
    u32 rxIntDly;
    u32 rxPkt;

    u32 phy_status_register;

    u32           initstate;
    u32           contiROVR;
    unsigned long oldTime;
    unsigned long PreLinkStatus;

    //
    const char* name;
    u8          bInit;
    u8          bEthCfg;

    u8 u8Minor;

#if KERNEL_PHY
    /// phy separation
    struct mii_bus* mii_bus;
    struct device*  dev; // don't know its useness
#endif

#if EMAC_FLOW_CONTROL_TX
    // TX pause packet (TX flow control)
    spinlock_t        mutexFlowTX;
    struct timer_list timerFlowTX;
    // int                 isPauseTX;
#endif

#if EMAC_FLOW_CONTROL_RX
    // RX pause packet (TX flow control)
    u8* pu8PausePkt;
    u8  u8PausePktSize;
    u8  isPausePkt;
#endif

#if REDUCE_CPU_FOR_RBNA
    spinlock_t        mutexIntRX;
    struct timer_list timerIntRX;
#endif

#if DYNAMIC_INT_RX
    struct timespec64 rx_stats_time;
    u32               rx_stats_packet;
    u8                rx_stats_enable;
#endif

#if DYNAMIC_INT_TX_TIMER
#if DYNAMIC_INT_TX_TIMER_HR
    struct hrtimer timerTxWdt;
#else
    struct timer_list timerTxWdt;
#endif
    int timerTxWdtPeriod;
#endif

#if EMAC_SG
    // char*               pTxBuf;
    // int                 TxBufIdx;
    int maxSG;
#endif // #if EMAC_SG
#ifdef CONFIG_CAM_CLK
    void** pvclk;
    int    EmacParentCnt;
#endif
    bool bEmacClkOn;
    u32  cpu_affinity;
    bool resume_flag;
};

#endif
// -----------------------------------------------------------------------------
// Linux EMAC.h End
// -----------------------------------------------------------------------------
