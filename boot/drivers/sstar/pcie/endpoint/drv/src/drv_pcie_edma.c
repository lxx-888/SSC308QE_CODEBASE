/*
 * drv_pcie_edma.c - Sigmastar
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
#include <linux/types.h>
#include <linux/errno.h>
#include <malloc.h>
#include <cpu_func.h>

#include "asm/arch/mach/platform.h"
#include "drv_pcieif.h"
#include "hal_pcie.h"
#include "drv_pcie.h"
#include "drv_pcie_epc.h"

//=================================================================================================
// Macro & Define
//=================================================================================================

//=================================================================================================
// Local Variable
//=================================================================================================

//=================================================================================================
// Private Fucntions
//=================================================================================================
static void _DmaGetDoneIntrSt(u8 id, u32 *write, u32 *read)
{
    *write = (_pcie_readl_dma(id, DMA_WRITE_INT_STATUS_OFF) & EDMA_DONE_INT_MASK) >> EDMA_DONE_INT_SHIFT;
    *read  = (_pcie_readl_dma(id, DMA_READ_INT_STATUS_OFF) & EDMA_DONE_INT_MASK) >> EDMA_DONE_INT_SHIFT;
}

static void _DmaGetAbortIntrSt(u8 id, u32 *write, u32 *read)
{
    *write = (_pcie_readl_dma(id, DMA_WRITE_INT_STATUS_OFF) & EDMA_ABORT_INT_MASK) >> EDMA_ABORT_INT_SHIFT;
    *read  = (_pcie_readl_dma(id, DMA_READ_INT_STATUS_OFF) & EDMA_ABORT_INT_MASK) >> EDMA_ABORT_INT_SHIFT;
}

static void _DmaClrDoneIntrSt(u8 id, u32 write, u32 read)
{
    if (write)
        _pcie_writel_dma(id, DMA_WRITE_INT_CLEAR_OFF, (write << EDMA_DONE_INT_SHIFT) & EDMA_DONE_INT_MASK);
    if (read)
        _pcie_writel_dma(id, DMA_READ_INT_CLEAR_OFF, (read << EDMA_DONE_INT_SHIFT) & EDMA_DONE_INT_MASK);
}

static void _DmaClrAbortIntrSt(u8 id, u32 write, u32 read)
{
    if (write)
        _pcie_writel_dma(id, DMA_WRITE_INT_CLEAR_OFF, (write << EDMA_ABORT_INT_SHIFT) & EDMA_ABORT_INT_MASK);
    if (read)
        _pcie_writel_dma(id, DMA_READ_INT_CLEAR_OFF, (read << EDMA_ABORT_INT_SHIFT) & EDMA_ABORT_INT_MASK);
}

static void _Drv_PCIE_DmaIrqHandler(void *arg)
{
    PCIE_EDMA *   edma = arg;
    PCIE_EDMA_CH *ch;
    u32           wr_done, wr_abort, rd_done, rd_abort;

    _DmaGetDoneIntrSt(edma->id, &wr_done, &rd_done);
    _DmaGetAbortIntrSt(edma->id, &wr_abort, &rd_abort);
    if (!wr_done && !wr_abort && !rd_done && !rd_abort)
        return;

    PCIE_DBG("done W_x%x R_x%x, abort W_x%x R_x%x\r\n", wr_done, rd_done, wr_abort, rd_abort);

    /* NOTE: only one channel used for read, and one for write */
    ch        = &edma->wr_ch;
    ch->cnt   = 0;
    ch->state = EDMA_CH_IDLE;
    if (wr_done & (1 << 0))
        ch->callback(EDMA_XFER_DONE, ch->cb_data);
    else if (wr_abort & (1 << 0))
        ch->callback(EDMA_XFER_ABORT, ch->cb_data);

    ch        = &edma->rd_ch;
    ch->cnt   = 0;
    ch->state = EDMA_CH_IDLE;
    if (rd_done & (1 << 0))
        ch->callback(EDMA_XFER_DONE, ch->cb_data);
    else if (rd_abort & (1 << 0))
        ch->callback(EDMA_XFER_ABORT, ch->cb_data);

    _DmaClrDoneIntrSt(edma->id, wr_done, rd_done);
    _DmaClrAbortIntrSt(edma->id, wr_abort, rd_abort);
}

static int _Drv_PCIE_DmaInitCh(PCIE_EDMA_CH *ch, u32 idx, EDMA_DIR dir)
{
    void *addr;
    char *name = (dir == EDMA_DIR_WR) ? "edma_wr0" : "edma_rd0";

    name[7] = '0' + idx;
    addr    = malloc(sizeof(EDMA_LL_ELMT) * EDMA_LL_SIZE);
    if (!addr)
    {
        PCIE_ERR("edma ll alloc failed\r\n");
        return -ENOMEM;
    }
    ch->index   = idx;
    ch->cnt     = 0;
    ch->dir     = dir;
    ch->state   = EDMA_CH_IDLE;
    ch->ll_virt = addr;
    ch->ll_phys = (phys_addr_t)addr;

    return 0;
}

void _Drv_PCIE_DmaStartCh(PCIE_EDMA_CH *ch, u8 id)
{
    u32         val, idx;
    phys_addr_t ll_miu;

    idx    = ch->index;
    ll_miu = Drv_PCIE_Phys2Miu(ch->ll_phys);

    if (ch->dir == EDMA_DIR_WR)
    {
        /* Power enable */
        _pcie_writel_dma(id, DMA_WRITE_CHi_PWR_EN_OFF(idx), EDMA_PWR_EN);
        /* Enable engine */
        _pcie_writel_dma(id, DMA_WRITE_ENGINE_EN_OFF, EDMA_ENGINE_EN);
        /* Interrupt unmask - done, abort */
        val = _pcie_readl_dma(id, DMA_WRITE_INT_MASK_OFF);
        val &= ~(EDMA_DONE_INT_CHi(idx) | EDMA_ABORT_INT_CHi(idx));
        _pcie_writel_dma(id, DMA_WRITE_INT_MASK_OFF, val);
        /* Linked list error */
        val = _pcie_readl_dma(id, DMA_WRITE_LINKED_LIST_ERR_EN_OFF);
        _pcie_writel_dma(id, DMA_WRITE_LINKED_LIST_ERR_EN_OFF, val | EDMA_CHi_LLRAIE(idx) | EDMA_CHi_LLLAIE(idx));
        /* Channel control */
        _pcie_writel_dma(id, DMA_CH_CONTROL1_OFF_WRCH_i(idx), EDMA_CH_CTRL1_CCS | EDMA_CH_CTRL1_LLE);
        /* Linked list - low, high */
        _pcie_writel_dma(id, DMA_LLP_LOW_OFF_WRCH_i(idx), lower_32_bits(ll_miu));
        _pcie_writel_dma(id, DMA_LLP_HIGH_OFF_WRCH_i(idx), upper_32_bits(ll_miu));
        /* Doorbell */
        _pcie_writel_dma(id, DMA_WRITE_DOORBELL_OFF, idx);
    }
    else
    {
        /* Power enable */
        _pcie_writel_dma(id, DMA_READ_CHi_PWR_EN_OFF(idx), EDMA_PWR_EN);
        /* Enable engine */
        _pcie_writel_dma(id, DMA_READ_ENGINE_EN_OFF, EDMA_ENGINE_EN);
        /* Interrupt unmask - done, abort */
        val = _pcie_readl_dma(id, DMA_READ_INT_MASK_OFF);
        val &= ~(EDMA_DONE_INT_CHi(idx) | EDMA_ABORT_INT_CHi(idx));
        _pcie_writel_dma(id, DMA_READ_INT_MASK_OFF, val);
        /* Linked list error */
        val = _pcie_readl_dma(id, DMA_READ_LINKED_LIST_ERR_EN_OFF);
        _pcie_writel_dma(id, DMA_READ_LINKED_LIST_ERR_EN_OFF, val | EDMA_CHi_LLRAIE(idx) | EDMA_CHi_LLLAIE(idx));
        /* Channel control */
        _pcie_writel_dma(id, DMA_CH_CONTROL1_OFF_RDCH_i(idx), EDMA_CH_CTRL1_CCS | EDMA_CH_CTRL1_LLE);
        /* Linked list - low, high */
        _pcie_writel_dma(id, DMA_LLP_LOW_OFF_RDCH_i(idx), lower_32_bits(ll_miu));
        _pcie_writel_dma(id, DMA_LLP_HIGH_OFF_RDCH_i(idx), upper_32_bits(ll_miu));
        /* Doorbell */
        _pcie_writel_dma(id, DMA_READ_DOORBELL_OFF, idx);
    }
}

void _Drv_PCIE_DmaStop(u8 id)
{
    _pcie_writel_dma(id, DMA_WRITE_INT_MASK_OFF, EDMA_DONE_INT_MASK | EDMA_ABORT_INT_MASK);
    _pcie_writel_dma(id, DMA_READ_INT_MASK_OFF, EDMA_DONE_INT_MASK | EDMA_ABORT_INT_MASK);
    _pcie_writel_dma(id, DMA_WRITE_INT_CLEAR_OFF, EDMA_DONE_INT_MASK | EDMA_ABORT_INT_MASK);
    _pcie_writel_dma(id, DMA_READ_INT_CLEAR_OFF, EDMA_DONE_INT_MASK | EDMA_ABORT_INT_MASK);
    _pcie_writel_dma(id, DMA_WRITE_ENGINE_EN_OFF, 0);
    _pcie_writel_dma(id, DMA_READ_ENGINE_EN_OFF, 0);
}

//=================================================================================================
// Public Fucntions
//=================================================================================================
int Drv_PCIE_DmaInit(PCIE_EDMA *edma)
{
    PCIE_EP *ep = to_ep_from_edma(edma);

    edma->id = ep->port_id;

    _Drv_PCIE_DmaStop(edma->id);

    /* Initial the write channel */
    _Drv_PCIE_DmaInitCh(&edma->wr_ch, 0, EDMA_DIR_WR);
    /* Initial the read channel */
    _Drv_PCIE_DmaInitCh(&edma->rd_ch, 0, EDMA_DIR_RD);

    /* Register eDMA interrupts callback */
    if (Drv_PCIE_EpcRegisterIrqHandler(ep, EP_CB_EDMA, _Drv_PCIE_DmaIrqHandler, edma))
    {
        PCIE_ERR("edma request irq failed\r\n");
        return -EINVAL;
    }

    return 0;
}

int Drv_PCIE_DmaSubmit(PCIE_EDMA *edma, u64 sar, u64 dar, u32 size, EDMA_DIR dir)
{
    PCIE_EDMA_CH *ch = (dir == EDMA_DIR_WR) ? &edma->wr_ch : &edma->rd_ch;
    EDMA_LL_ELMT *elmt;
    int           ret = 0;

    if ((ch->cnt == 0) && (ch->state != EDMA_CH_IDLE))
    {
        PCIE_ERR("edma ch wrong state %d\r\n", ch->state);
        ret = -ENOSR;
        goto _exit_submit;
    }
    else if ((ch->cnt + 1) >= EDMA_LL_SIZE)
    {
        PCIE_ERR("edma too many requests\r\n");
        ret = -ENOSR;
        goto _exit_submit;
    }

    elmt = &ch->ll_virt[ch->cnt];
    memset(elmt, 0, sizeof(EDMA_LL_ELMT));
    elmt->cb        = 1;
    elmt->xfer_size = size;
    elmt->sar_l     = lower_32_bits(sar);
    elmt->sar_h     = upper_32_bits(sar);
    elmt->dar_l     = lower_32_bits(dar);
    elmt->dar_h     = upper_32_bits(dar);
    ch->cnt++;
    ch->state = EDMA_CH_PENDING;

_exit_submit:

    return ret;
}

int Drv_PCIE_DmaIssuePending(PCIE_EDMA *edma, EDMA_DIR dir, EDMA_CALLBACK cb, void *data)
{
    PCIE_EDMA_CH *ch = (dir == EDMA_DIR_WR) ? &edma->wr_ch : &edma->rd_ch;
    EDMA_LL_ELMT *elmt_data, *elmt_link;
    phys_addr_t   ll_miu;
    int           ret = 0;

    if ((ch->cnt == 0) || (ch->state != EDMA_CH_PENDING))
    {
        PCIE_ERR("edma ch wrong state %d\r\n", ch->state);
        return -EINVAL;
    }

    // Set local interrupt enable of the last one request
    elmt_data      = &ch->ll_virt[ch->cnt - 1];
    elmt_data->lie = 1;
    // link element
    ll_miu    = Drv_PCIE_Phys2Miu(ch->ll_phys);
    elmt_link = &ch->ll_virt[ch->cnt];
    memset(elmt_link, 0, sizeof(EDMA_LL_ELMT));
    elmt_link->tcb   = 1;
    elmt_link->llp   = 1;
    elmt_link->sar_l = lower_32_bits(ll_miu);
    elmt_link->sar_h = upper_32_bits(ll_miu);
    ch->callback     = cb;
    ch->cb_data      = data;
    ch->state        = EDMA_CH_BUSY;
    flush_cache((ulong)elmt_data, sizeof(EDMA_LL_ELMT) * 2);

    _Drv_PCIE_DmaStartCh(ch, edma->id);

    return ret;
}
