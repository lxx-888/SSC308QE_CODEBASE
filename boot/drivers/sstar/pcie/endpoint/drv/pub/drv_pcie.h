/*
 * drv_pcie.h - Sigmastar
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
#ifndef _DRV_PCIE_H_
#define _DRV_PCIE_H_

#include <common.h>
#ifdef CONFIG_SSTAR_CLK
#include <clk.h>
#endif
#include <pci_ep.h>
#include "drv_pcie_edma.h"

typedef enum
{
    EP_CB_EDMA = 0,
    EP_CB_WRADDRMATCH,
    EP_CB_TYPE_MAX
} PCIE_EP_CB_TYPE;

typedef struct
{
    u64 msg_addr;
    u32 msg_data;
    u32 vector_ctrl;
} PCIE_MSIX_TABLE;

typedef struct
{
    phys_addr_t phys_base;
    u32         size;
    u32         offset;
    u32         page_size;
} PCIE_EPC_MEM;

struct pcie_epc;
struct pcie_epf;

struct pcie_epc
{
    struct pcie_epf *epf;
    PCIE_EPC_MEM     mem;
};

struct pcie_epf
{
    struct pcie_epc *     epc;
    struct pci_ep_header *header;
    struct pci_bar        bar[6];
    u8                    msi_interrupts;
    u8                    msix_interrupts;
};

typedef struct pcie_epf PCIE_EPF;
typedef struct pcie_epc PCIE_EPC;
typedef void (*EP_CALLBACK)(void *);

typedef struct
{
    PCIE_EPC    epc;
    PCIE_EDMA   edma;
    phys_addr_t outbound_phys[6];
    void *      msi_mem;
    phys_addr_t msi_mem_phys;
    u8          bar_to_atu[6];
    u8          msi_cap;  /* MSI capability offset */
    u8          msix_cap; /* MSI-X capability offset */
    u8          port_id;
    EP_CALLBACK callback[EP_CB_TYPE_MAX];
    void *      cb_data[EP_CB_TYPE_MAX];
    int         num_lanes;
    int         link_gen;
    int         clk_cnt;
    int         clk_src;
#define PCIE_CLK_SRC_EXT (0) /* use external clock source */
#define PCIE_CLK_SRC_INT (1) /* use internal clock source */
#ifdef CONFIG_SSTAR_CLK
    struct clk_bulk clks;
#endif
} PCIE_EP;

#ifndef lower_32_bits
#define lower_32_bits(n) ((u32)(n))
#endif
#ifndef upper_32_bits
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#endif

#ifndef container_of
#define container_of(ptr, type, member)                        \
    (                                                          \
        {                                                      \
            const typeof(((type *)0)->member) *__mptr = (ptr); \
            (type *)((char *)__mptr - offsetof(type, member)); \
        })
#endif

#define to_ep_from_epc(epc)   container_of((epc), PCIE_EP, epc)
#define to_ep_from_edma(edma) container_of((edma), PCIE_EP, edma)

#define EPC_MEM_ALIGN(x, a)     (((x) + (a - 1)) & ~(a - 1))
#define Drv_PCIE_Phys2Miu(addr) (addr - MIU0_START_ADDR)
#define Drv_PCIE_EpcBindEpf(epc, epf) \
    {                                 \
        epc->epf = epf;               \
        epf->epc = epc;               \
    }

/*
 * PCI configuration address space
 */

/* MSI-X registers */
#define PCI_MSIX_CAP_FLAGS    (2)
#define PCI_MSIX_TABLE_SIZE   (0x07F)      /* table size */
#define PCI_MSIX_FUNC_MASK    (0x4000)     /* function mask */
#define PCI_MSIX_ENABLE       (0x8000)     /* MSI-X enable */
#define PCI_MSIX_TABLE        (4)          /* MSI-X table */
#define PCI_MSIX_BIR          (0x7)        /* BAR indicator register field */
#define PCI_MSIX_TABLE_OFFSET (0xfffffff8) /* table offset */
#define PCI_MSIX_PBA          (8)          /* for PBA offset and PBA BIR value */

/* MSI-X table entry */
#define PCI_MSIX_ENTRY_SIZE        (16)
#define PCI_MSIX_ENTRY_ADDR_LOW    (0)
#define PCI_MSIX_ENTRY_ADDR_HIGH   (4)
#define PCI_MSIX_ENTRY_DATA        (8)
#define PCI_MSIX_ENTRY_VECTOR_MASK (1)

/* log print */
#define PCIE_VERBOSE(fmt, args...) // printf(fmt, ##args)
#define PCIE_DBG(fmt, args...)     // printf(fmt, ##args)
#define PCIE_INFO(fmt, args...)    // printf(fmt, ##args)
#define PCIE_NOTICE(fmt, args...)  printf(fmt, ##args)
#define PCIE_ERR(fmt, args...)     printf(fmt, ##args)

void  Drv_PCIE_EpcWriteHeader(PCIE_EPC *epc, struct pci_ep_header *hdr);
int   Drv_PCIE_EpcSetBar(PCIE_EPC *epc, struct pci_bar *epf_bar);
void *Drv_PCIE_EpcMemAlloc(PCIE_EPC *epc, u32 size, phys_addr_t *phys_addr);
int   Drv_PCIE_EpcMapAddr(PCIE_EPC *epc, phys_addr_t addr, u64 pci_addr, u32 size);
void  Drv_PCIE_EpcUnmapAddr(PCIE_EPC *epc, phys_addr_t addr);
int   Drv_PCIE_EpcSetMsi(PCIE_EPC *epc, u8 interrupts);
int   Drv_PCIE_EpcSetMsiX(PCIE_EPC *epc, u8 interrupts, enum pci_barno bir, u32 offset);
int   Drv_PCIE_EpcRaiseMsiIrq(PCIE_EPC *epc, u8 interrupt_num);
int   Drv_PCIE_EpcRaiseMsiXIrq(PCIE_EPC *epc, u8 interrupt_num);
int   Drv_PCIE_EpcRegisterIrqHandler(PCIE_EP *ep, PCIE_EP_CB_TYPE type, EP_CALLBACK cb, void *data);
void  Drv_PCIE_EpcSetWriteMatchAddr(PCIE_EPC *epc, void *addr);
int   Drv_PCIE_EpcHandleWriteAddrMatchIrq(PCIE_EP *ep);
void  Drv_PCIE_EpcIrqHandler(PCIE_EP *ep);
int   Drv_PCIE_EpcInit(PCIE_EP *ep);
void  Drv_PCIE_EpcResetBar(PCIE_EP *ep, enum pci_barno bar);
void  Drv_PCIE_EpcStart(PCIE_EPC *epc);

#endif //_DRV_PCIE_H_
