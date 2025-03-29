/*
 * drv_pcie_edma.h - Sigmastar
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
#ifndef _DRV_PCIE_EDMA_H_
#define _DRV_PCIE_EDMA_H_

/*
 * dma registers
 */
#define DMA_CTRL_OFF                     0x08
#define EDMA_WRITE_CH_CNT_MASK           (0xF)
#define EDMA_WRITE_CH_CNT_SHIFT          (0)
#define EDMA_READ_CH_CNT_MASK            (0xF0000)
#define EDMA_READ_CH_CNT_SHIFT           (16)
#define DMA_WRITE_ENGINE_EN_OFF          0x0C
#define EDMA_ENGINE_EN                   (0x01)
#define DMA_WRITE_DOORBELL_OFF           0x10
#define DMA_READ_ENGINE_EN_OFF           0x2C
#define DMA_READ_DOORBELL_OFF            0x30
#define DMA_WRITE_INT_STATUS_OFF         0x4C
#define DMA_WRITE_INT_MASK_OFF           0x54
#define DMA_WRITE_INT_CLEAR_OFF          0x58
#define DMA_WRITE_ERR_STATUS_OFF         0x5C
#define EDMA_DONE_INT_MASK               (0xFF)
#define EDMA_DONE_INT_SHIFT              (0)
#define EDMA_DONE_INT_CHi(ch)            (1 << ch)
#define EDMA_ABORT_INT_MASK              (0xFF0000)
#define EDMA_ABORT_INT_SHIFT             (16)
#define EDMA_ABORT_INT_CHi(ch)           (1 << (ch + 16))
#define DMA_WRITE_LINKED_LIST_ERR_EN_OFF 0x90
#define EDMA_CHi_LLRAIE(ch)              (1 << ch)
#define EDMA_CHi_LLLAIE(ch)              (1 << (ch + 16))
#define DMA_READ_INT_STATUS_OFF          0xA0
#define DMA_READ_INT_MASK_OFF            0xA8
#define DMA_READ_INT_CLEAR_OFF           0xAC
#define DMA_READ_ERR_STATUS_LOW_OFF      0xB4
#define DMA_READ_ERR_STATUS_HIGH_OFF     0xB8
#define DMA_READ_LINKED_LIST_ERR_EN_OFF  0xC4
#define DMA_WRITE_CHi_PWR_EN_OFF(ch)     (0x128 + (ch << 2))
#define DMA_READ_CHi_PWR_EN_OFF(ch)      (0x168 + (ch << 2))
#define EDMA_PWR_EN                      (0x01)
#define DMA_CH_CONTROL1_OFF_WRCH_i(ch)   (0x200 + (ch * 0x200))
#define DMA_LLP_LOW_OFF_WRCH_i(ch)       (0x21C + (ch * 0x200))
#define DMA_LLP_HIGH_OFF_WRCH_i(ch)      (0x220 + (ch * 0x200))
#define DMA_CH_CONTROL1_OFF_RDCH_i(ch)   (0x300 + (ch * 0x200))
#define DMA_LLP_LOW_OFF_RDCH_i(ch)       (0x31C + (ch * 0x200))
#define DMA_LLP_HIGH_OFF_RDCH_i(ch)      (0x320 + (ch * 0x200))
/* DMA_CH_CONTROL1 */
#define EDMA_CH_CTRL1_CCS (0x100) // Consume Cycle State
#define EDMA_CH_CTRL1_LLE (0x200) // Linked List Enable

/* Link structure size: 7 data elements + 1 link element */
#define EDMA_LL_SIZE (8)

/*
 * data structure
 */
typedef enum
{
    EDMA_DIR_WR,
    EDMA_DIR_RD,
} EDMA_DIR;

/* eDMA LL element */
typedef struct
{
    u32 cb : 1;  /* Cycle Bit */
    u32 tcb : 1; /* Toggle Cycle Bit */
    u32 llp : 1; /* Load Link Pointer */
    u32 lie : 1; /* Local Interrupt Enable */
    u32 rie : 1; /* Remote Interrupt Enable */
    u32 resv_5 : 27;
    u32 xfer_size; /* transfer size */
    u32 sar_l;     /* source address low 32-bit */
    u32 sar_h;     /* source address high 32-bit */
    u32 dar_l;     /* destination address low 32-bit */
    u32 dar_h;     /* destination address high 32-bit */
} EDMA_LL_ELMT;

typedef enum
{
    EDMA_CH_IDLE = 0, /* channel is idle */
    EDMA_CH_PENDING,  /* reqeusts are pending to process */
    EDMA_CH_BUSY,     /* channel in busy */
} EDMA_CH_STATE;

typedef enum
{
    EDMA_XFER_DONE = 0, /* transfer done */
    EDMA_XFER_ABORT,    /* transfer abort */
} EDMA_XFER_STATE;

typedef void (*EDMA_CALLBACK)(EDMA_XFER_STATE, void *);

/* eDMA channel */
typedef struct
{
    u32           index;   /* index of channel */
    u32           cnt;     /* number of data elements in LL structure */
    EDMA_DIR      dir;     /* direction of channel */
    EDMA_LL_ELMT *ll_virt; /* virtual addr of LL structure */
    phys_addr_t   ll_phys; /* physical addr of LL structure */
    EDMA_CH_STATE state;   /* channel state */
                           /* callback */
    EDMA_CALLBACK callback;
    void *        cb_data;
} PCIE_EDMA_CH;

typedef struct
{
    PCIE_EDMA_CH wr_ch; /* here just provide one write channel */
    PCIE_EDMA_CH rd_ch; /* here just provide one read channel */
    u8           id;
} PCIE_EDMA;

int Drv_PCIE_DmaInit(PCIE_EDMA *edma);
int Drv_PCIE_DmaSubmit(PCIE_EDMA *edma, u64 sar, u64 dar, u32 size, EDMA_DIR dir);
int Drv_PCIE_DmaIssuePending(PCIE_EDMA *edma, EDMA_DIR dir, EDMA_CALLBACK cb, void *data);

#endif //_DRV_PCIE_EDMA_H_