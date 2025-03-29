/*
 * msb250x_dma.c- Sigmastar
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

/******************************************************************************
 * Include Files
 ******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/jiffies.h>
#include "msb250x_udc_reg.h"
#include "msb250x_udc.h"
#include "msb250x_gadget.h"

#define ENABLED_DMA_BYTES 512

static inline s8 msb250x_dma_get_channel(struct msb250x_udc* udc)
{
    int            ch;
    struct device* dev = &udc->pdev->dev;
    u16            ctrl;

    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*)&ctrl;

    for (ch = 1; ch <= udc->dma_chanel_num; ch++)
    {
        ctrl = ms_readw(MSB250X_OTG0_DMA_CNTL(udc->otg_base, ch));

        if (0 == dma_ctrl->bEnableDMA && 0 == dma_ctrl->bEndpointNumber)
        {
            return ch;
        }
    }

    dev_err(dev, "<USB>[DMA] No available channel!\n");

    return -1;
}

u8 msb250x_dma_find_channel_by_ep(struct msb250x_udc* udc, u8 ep_num)
{
    s8  ch   = 0;
    u16 ctrl = 0;

    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*)&ctrl;

    for (ch = 1; ch <= udc->dma_chanel_num; ch++)
    {
        ctrl = ms_readw(MSB250X_OTG0_DMA_CNTL(udc->otg_base, ch));

        if (ep_num == dma_ctrl->bEndpointNumber)
        {
            return ch;
        }
    }

    return 0;
}

void msb250x_dma_release_channel(struct msb250x_udc* udc, s8 ch)
{
    u8                           ep_num   = 0;
    u16                          ctrl     = 0;
    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*)&ctrl;
    struct device*               dev      = &udc->pdev->dev;

    ctrl = 0;
    ms_writew(ctrl, MSB250X_OTG0_DMA_CNTL(udc->otg_base, ch));
    ctrl = ms_readw(MSB250X_OTG0_DMA_CNTL(udc->otg_base, ch));

    if (0 != dma_ctrl->bEndpointNumber)
    {
        dev_err(dev, "<USB_ERR>[%d] DMA[%d] doesn't release!\n", ep_num, ch);
    }
}

int msb250x_dma_setup_control(struct usb_ep* ep, struct msb250x_request* mreq, u32 bytes)
{
    s8                  ch      = 0;
    u8                  csr1    = 0;
    u8                  csr2    = 0;
    u16                 control = 0;
    u8                  ep_num  = 0;
    u8                  dma_extend_addr;
    u64                 dma_addr;
    struct msb250x_ep*  mep = to_msb250x_ep(ep);
    struct msb250x_udc* udc = mep->udc;
    struct device*      dev = &udc->pdev->dev;

    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*)&control;
    struct otg0_ep_rxcsr_h*      rxcsr_h  = NULL;
    struct otg0_ep_rxcsr_l*      rxcsr_l  = NULL;
    struct otg0_ep_txcsr_h*      txcsr_h  = NULL;

    ep_num                     = usb_endpoint_num(ep->desc);
    dma_ctrl->bEndpointNumber  = ep_num;
    dma_ctrl->bEnableDMA       = 1;
    dma_ctrl->bInterruptEnable = 1;
    dma_ctrl->bRurstMode       = 3;
    dma_ctrl->bDirection       = usb_endpoint_dir_out(ep->desc) ? 0 : 1;

    if (usb_endpoint_xfer_bulk(ep->desc) && ep->maxpacket < bytes)
    {
        dma_ctrl->bDMAMode = 1;
    }

    if (usb_endpoint_dir_out(ep->desc)) // dma write
    {
        csr1 = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
        csr2 = ms_readb(MSB250X_OTG0_EP_RXCSR2_REG(udc->otg_base, ep_num));

        rxcsr_l = (struct otg0_ep_rxcsr_l*)&csr1;
        rxcsr_h = (struct otg0_ep_rxcsr_h*)&csr2;

        csr2 &= ~RXCSR2_MODE1;

        if (1 == dma_ctrl->bDMAMode)
        {
            csr2 |= RXCSR2_MODE1;
            rxcsr_h->bDisNyet = 1;
        }
    }
    else // dma read
    {
        txcsr_h = (struct otg0_ep_txcsr_h*)&csr2;
        csr2    = ms_readb(MSB250X_OTG0_EP_TXCSR2_REG(udc->otg_base, ep_num));

        csr2 &= ~TXCSR2_MODE1;

        if (1 == dma_ctrl->bDMAMode)
        {
            csr2 |= TXCSR2_MODE1;
        }
    }

    if (ENABLED_DMA_BYTES > bytes)
    {
        return -1;
    }

    if (0 >= (ch = msb250x_dma_get_channel(udc))) /* no free channel */
    {
        dev_err(dev, "<USB>[%d][DMA] Get DMA channel fail!\n", ep_num);
        return -1;
    }

    ms_writeb(csr2, ((usb_endpoint_dir_out(ep->desc)) ? MSB250X_OTG0_EP_RXCSR2_REG(udc->otg_base, ep_num)
                                                      : MSB250X_OTG0_EP_TXCSR2_REG(udc->otg_base, ep_num)));
#if 0
    ms_writelw((uintptr_t)(mreq->req.dma + mreq->req.actual), (u32 volatile*)MSB250X_OTG0_DMA_ADDR(udc->otg_base, ch));
    ms_writelw((uintptr_t)bytes, (u32 volatile*)MSB250X_OTG0_DMA_COUNT(udc->otg_base, ch));
#else
    dma_addr = mreq->req.dma + mreq->req.actual;
    ms_writelw((dma_addr & 0xFFFFFFFF), MSB250X_OTG0_DMA_ADDR(udc->otg_base, ch));
    if ((dma_addr >> 31) & 0x1)
        ms_writeb((ms_readb(udc->usb0_base) | 0x10), udc->usb0_base);
    else
        ms_writeb((ms_readb(udc->usb0_base) & 0xEF), udc->usb0_base);
    dma_extend_addr = (dma_addr >> 28) & 0xF0;
    ms_writeb(((ms_readb(GET_REG8_ADDR(udc->usb0_base, 0x01)) & (~(0xF0))) | dma_extend_addr),
              GET_REG8_ADDR(udc->usb0_base, 0x01));
    ms_writelw(bytes, MSB250X_OTG0_DMA_COUNT(udc->otg_base, ch));
#endif
    ms_writew(control, MSB250X_OTG0_DMA_CNTL(udc->otg_base, ch));

#if 0
    //if (usb_endpoint_dir_out(ep->ep.desc))
    printk(KERN_DEBUG "<USB>[%s][DMA][%d] %s mode/ctrl(0x%x/0x%04x) buff/bytes/actual/length(0x%p/0x%04x/0x%04x/0x%04x)\n",
                      _ep->name,
                      ch,
                      (usb_endpoint_dir_out(_ep->desc))? "RX" : "TX",
                      dma_ctrl->bDMAMode,
                      control,
                      req->req.buf,
                      bytes,
                      req->req.actual,
                      req->req.length);
#endif
    return 0;
}

void msb250x_dma_isr_handler(u8 ch, struct msb250x_udc* udc)
{
    int        is_last = 0;
    u32        bytesleft, bytesdone, control;
    u8         csr2 = 0, csr1 = 0;
    dma_addr_t dma_handle;
    u8         ep_num;
    u8         dma_extend_addr;
    u16        ep_maxpacket = 0;

    struct msb250x_ep*           mep;
    struct usb_ep*               ep;
    struct msb250x_request*      mreq     = NULL;
    struct device*               dev      = &udc->pdev->dev;
    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*)&control;
    struct otg0_ep_rxcsr_h*      rxcsr_h  = NULL;
    struct otg0_ep_rxcsr_l*      rxcsr_l  = NULL;
    //    struct ep_txcsr_h* txcsr_h = NULL;
    struct otg0_ep_txcsr_l* txcsr_l = NULL;

    control         = ms_readw(MSB250X_OTG0_DMA_CNTL(udc->otg_base, ch));
    dma_handle      = (dma_addr_t)ms_readlw((u32 volatile*)MSB250X_OTG0_DMA_ADDR(udc->otg_base, ch));
    dma_extend_addr = (ms_readb(GET_REG8_ADDR(udc->usb0_base, 0x01)) & 0xF0);
    dma_handle      = dma_handle | (u64)(dma_extend_addr) << 28;
    bytesleft       = ms_readlw((u32 volatile*)MSB250X_OTG0_DMA_COUNT(udc->otg_base, ch));
    ep_num          = dma_ctrl->bEndpointNumber;
    mep             = &udc->mep[ep_num];
    ep              = &mep->ep;
    msb250x_dma_release_channel(udc, ch);

    if (!ep->desc)
    {
        return;
    }

    /* release DMA channel */
    if (likely(!list_empty(&mep->queue)))
    {
        mreq = list_entry(mep->queue.next, struct msb250x_request, queue);
    }

    if (mreq)
    {
        bytesdone = dma_handle - (mreq->req.dma + mreq->req.actual);

        if (1 == dma_ctrl->bBusError)
        {
            dev_err(dev, "<USB_ERR>[DMA] Bus ERR!\n");
            return;
        }

        ep_maxpacket = ep->maxpacket;

        if (usb_endpoint_dir_in(ep->desc))
        {
            csr1    = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
            txcsr_l = (struct otg0_ep_txcsr_l*)&csr1;

            csr2 = ms_readb(MSB250X_OTG0_EP_TXCSR2_REG(udc->otg_base, ep_num));
            ms_writeb((csr2 & ~TXCSR2_MODE1), MSB250X_OTG0_EP_TXCSR2_REG(udc->otg_base, ep_num));

            if (0 == dma_ctrl->bDMAMode || 0 < (bytesdone % ep_maxpacket)) /* short packet || TX DMA mode0 */
            {
                txcsr_l->bTxPktRdy = 1;
                ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
            }
        }
        else
        {
            csr1    = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
            csr2    = ms_readb(MSB250X_OTG0_EP_RXCSR2_REG(udc->otg_base, ep_num));
            rxcsr_l = (struct otg0_ep_rxcsr_l*)&csr1;
            rxcsr_h = (struct otg0_ep_rxcsr_h*)&csr2;
            ms_writeb((csr2 & ~RXCSR2_MODE1), MSB250X_OTG0_EP_RXCSR2_REG(udc->otg_base, ep_num));
            if (0 == bytesleft)
            {
                if (usb_endpoint_xfer_bulk(ep->desc))
                {
                    if (1 == dma_ctrl->bDMAMode)
                    {
                        msb250x_udc_ok2rcv_for_packets(udc, mep->autoNAK_cfg, 0);
                        mep->shortPkt = 0;
                    }

                    while (0 == (ms_readb(MSB250X_OTG0_USB_CFG7_H(udc->otg_base)) & 0x80)) // last done bit
                    {
                        // printk(KERN_DEBUG "<USB>[DMA][%d] Last done bit.\n", ep_num);
                    }
                }

                if (0 == dma_ctrl->bDMAMode || 0 == rxcsr_h->bDMAReqMode)
                {
                    rxcsr_l->bRxPktRdy = 0;
                    ms_writeb(csr1, MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
                }
            }
        }

        mreq->req.actual += bytesdone;
        if (mreq->req.actual == mreq->req.length || 0 < (bytesdone % ep_maxpacket))
        {
            is_last = 1;
        }
#if 0
        if (!usb_endpoint_xfer_int(ep->desc) && !usb_endpoint_dir_in(ep->desc))
            if (usb_endpoint_dir_out(ep->desc))
                if (1 == dma_ctrl->bDMAMode && 0 < (bytesdone % ep->maxpacket))
                    dev_info(dev,
                             "<USB>[%s][DMA][%d] %s mode/%s/ctrl/bytesdone/bytesleft(0x%x/0x%x/0x%04x/0x%04x/0x%04x) "
                             "buff/%s/actual/length(0x%p/0x%04x/0x%04x/0x%04x)\n",
                             ep->name, ch, (usb_endpoint_dir_out(ep->desc)) ? "RX" : "TX",
                             (usb_endpoint_dir_out(ep->desc)) ? "bRxPktRdy" : "bTxPktRdy", dma_ctrl->bDMAMode,
                             (usb_endpoint_dir_out(ep->desc)) ? rxcsr_l->bRxPktRdy : txcsr_l->bTxPktRdy, control,
                             bytesdone, bytesleft, (usb_endpoint_dir_out(ep->desc)) ? "count" : "last", mreq->req.buf,
                             (usb_endpoint_dir_out(ep->desc))
                                 ? udc_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(udc->otg_base, ep_num))
                                 : is_last,
                             mreq->req.actual, mreq->req.length);
#endif
        if (1 == is_last)
        {
            list_del_init(&mreq->queue);
            msb250x_request_done(mep, mreq, 0);
        }
    }

    if (usb_endpoint_dir_out(mep->ep.desc) || !usb_endpoint_xfer_isoc(ep->desc))
    {
        msb250x_set_ep_halt(mep, 0);
        msb250x_request_continue(mep);
    }

    return;
}
