/*
 * msb250x_ep.c- Sigmastar
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

#include <linux/delay.h>
#include "msb250x_udc_reg.h"
#include "msb250x_udc.h"
#include "msb250x_gadget.h"
#include "msb250x_dma.h"
#include <linux/platform_device.h>

void msb250x_ep0_clear_opr(struct msb250x_udc *udc)
{
    ms_writeb(MSB250X_OTG0_CSR0_SRXPKTRDY, MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_clear_sst(struct msb250x_udc *udc)
{
    ms_writeb(0x00, MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_clear_se(struct msb250x_udc *udc)
{
    ms_writeb(MSB250X_OTG0_CSR0_SSETUPEND, MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_set_ipr(struct msb250x_udc *udc)
{
    ms_writeb(MSB250X_OTG0_CSR0_TXPKTRDY, MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_set_de(struct msb250x_udc *udc)
{
    ms_writeb(MSB250X_OTG0_CSR0_DATAEND, MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_set_ss(struct msb250x_udc *udc)
{
    ms_writeb(MSB250X_OTG0_CSR0_SENDSTALL, MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_clr_opr_set_ss(struct msb250x_udc *udc)
{
    ms_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_SENDSTALL), MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_set_de_out(struct msb250x_udc *udc)
{
    ms_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_DATAEND), MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_set_sse_out(struct msb250x_udc *udc)
{
    ms_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_SSETUPEND), MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

void msb250x_ep0_set_de_in(struct msb250x_udc *udc)
{
    ms_writeb((MSB250X_OTG0_CSR0_TXPKTRDY | MSB250X_OTG0_CSR0_DATAEND), MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
}

int msb250x_ep_enable(struct usb_ep *ep, const struct usb_endpoint_descriptor *desc)
{
    struct msb250x_ep *    mep = to_msb250x_ep(ep);
    struct msb250x_udc *   udc = mep->udc;
    struct device *        dev = &udc->pdev->dev;
    u8                     ep_num;
    unsigned int           csr1, csr2;
    unsigned long          flags;
    u8                     power     = 0;
    struct otg0_usb_power *pst_power = (struct otg0_usb_power *)&power;

    if (!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN)
    {
        dev_err(dev, "<USB> %s ESHUTDOWN\n", __func__);
        return -ESHUTDOWN;
    }

    spin_lock_irqsave(&udc->lock, flags);
    ep->maxpacket = usb_endpoint_maxp(desc);
    ep->mult      = usb_endpoint_maxp_mult(desc);
    ep_num        = usb_endpoint_num(desc);

    mep->autoNAK_cfg = 0;

    /* set type, direction, address; reset fifo counters */
    if (usb_endpoint_dir_in(desc))
    {
        csr1 = MSB250X_OTG0_TXCSR1_FLUSHFIFO | MSB250X_OTG0_TXCSR1_CLRDATAOTG;
        csr2 = MSB250X_OTG0_TXCSR2_MODE;
        if (usb_endpoint_xfer_isoc(desc))
        {
            csr2 |= MSB250X_OTG0_TXCSR2_ISOC;
        }
        ms_writew(desc->wMaxPacketSize, MSB250X_OTG0_EP_TXMAP_L_REG(udc->otg_base, ep_num));
        mep->fifo_size = 1 << (ms_readb(MSB250X_OTG0_EP_FIFOSIZE_REG(udc->otg_base, ep_num)) & 0xf0 >> 4);
        ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
        /* double buffer need flush fifo twice */
        if ((ep->maxpacket) * 2 <= mep->fifo_size)
        {
            ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
        }

        ms_writeb(csr2, MSB250X_OTG0_EP_TXCSR2_REG(udc->otg_base, ep_num));

        /* enable irqs */
        ms_writeb((ms_readb(MSB250X_OTG0_INTRTXE_REG(udc->otg_base)) | MSB250X_OTG0_INTR_EP(ep_num)),
                  MSB250X_OTG0_INTRTXE_REG(udc->otg_base));
    }
    else
    {
        /* enable the enpoint direction as Rx */
        csr1 = MSB250X_OTG0_RXCSR1_FLUSHFIFO | MSB250X_OTG0_RXCSR1_CLRDATATOG;
        csr2 = 0;

        if (usb_endpoint_xfer_int(desc))
        {
            csr2 |= MSB250X_OTG0_RXCSR2_DISNYET;
        }
        if (usb_endpoint_xfer_isoc(desc))
        {
            csr2 |= MSB250X_OTG0_RXCSR2_ISOC;
        }
        if (usb_endpoint_xfer_bulk(desc))
        {
            mep->autoNAK_cfg = msb250x_udc_get_autoNAK_cfg(udc, ep_num);
            msb250x_udc_enable_autoNAK(udc, ep_num, mep->autoNAK_cfg);
        }

        ms_writew(desc->wMaxPacketSize, MSB250X_OTG0_EP_RXMAP_L_REG(udc->otg_base, ep_num));
        mep->fifo_size = 1 << (ms_readb(MSB250X_OTG0_EP_FIFOSIZE_REG(udc->otg_base, ep_num)) & 0x0f);
        ms_writeb(csr1, MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
        /* double buffer need flush fifo twice */
        if ((ep->maxpacket) * 2 <= mep->fifo_size)
        {
            ms_writeb(csr1, MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
        }

        ms_writeb(csr2, MSB250X_OTG0_EP_RXCSR2_REG(udc->otg_base, ep_num));

        /* enable irqs */
        ms_writeb((ms_readb(MSB250X_OTG0_INTRRXE_REG(udc->otg_base)) | MSB250X_OTG0_INTR_EP(ep_num)),
                  MSB250X_OTG0_INTRRXE_REG(udc->otg_base));
    }

    if (usb_endpoint_xfer_isoc(desc))
    {
        power = ms_readb(MSB250X_OTG0_PWR_REG(udc->otg_base));

        if (0 == pst_power->bISOUpdate)
        {
            pst_power->bISOUpdate = 1;
            ms_writeb(power, MSB250X_OTG0_PWR_REG(udc->otg_base));
        }
    }

    msb250x_set_ep_halt(mep, 0);
    mep->shortPkt = 0;
    dev_info(dev, "<USB>[EP][%d] Enable for %s %s with maxpkt/fifo(%d/%d)\r\n", ep_num,
             usb_endpoint_xfer_bulk(desc) ? "BULK" : (usb_endpoint_xfer_isoc(desc) ? "ISOC" : "INT"),
             usb_endpoint_dir_in(desc) ? "IN" : "OUT", ep->maxpacket, mep->fifo_size);

    spin_unlock_irqrestore(&udc->lock, flags);

    return 0;
}

int msb250x_ep_disable(struct usb_ep *ep)
{
    struct msb250x_ep * mep    = to_msb250x_ep(ep);
    struct msb250x_udc *udc    = mep->udc;
    struct device *     dev    = &udc->pdev->dev;
    u8                  ch     = 0;
    u8                  ep_num = 0;
    unsigned long       flags;

    spin_lock_irqsave(&udc->lock, flags);
    if (!ep || !ep->desc)
    {
        dev_err(dev, "<USB>[%s] not enabled\n", ep ? ep->name : NULL);
        spin_unlock_irqrestore(&udc->lock, flags);
        return -EINVAL;
    }

    msb250x_set_ep_halt(mep, 1);
    ep_num = usb_endpoint_num(ep->desc);
    if (0 < (ch = msb250x_dma_find_channel_by_ep(udc, ep_num)))
    {
        msb250x_dma_release_channel(udc, ch);
    }

    /* disable irqs */
    if (usb_endpoint_dir_in(ep->desc))
    {
        ms_writeb((ms_readb(MSB250X_OTG0_INTRTXE_REG(udc->otg_base)) & ~(MSB250X_OTG0_INTR_EP(ep_num))),
                  MSB250X_OTG0_INTRTXE_REG(udc->otg_base));
    }
    else
    {
        if (usb_endpoint_xfer_bulk(ep->desc))
        {
            msb250x_udc_release_autoNAK_cfg(udc, mep->autoNAK_cfg);
            mep->autoNAK_cfg = 0;
        }

        ms_writeb((ms_readb(MSB250X_OTG0_INTRRXE_REG(udc->otg_base)) & ~(MSB250X_OTG0_INTR_EP(ep_num))),
                  MSB250X_OTG0_INTRRXE_REG(udc->otg_base));
    }
    msb250x_request_nuke(udc, mep, -ESHUTDOWN);
    ep->desc = NULL;
    spin_unlock_irqrestore(&udc->lock, flags);

    dev_info(dev, "<USB>[EP%d] disabled\n", ep_num);

    return 0;
}

struct usb_request *msb250x_ep_alloc_request(struct usb_ep *ep, gfp_t gfp_flags)
{
    struct msb250x_request *mreq;
    struct msb250x_ep *     mep = to_msb250x_ep(ep);
    struct msb250x_udc *    udc = mep->udc;
    struct device *         dev = &udc->pdev->dev;

    mreq = kzalloc(sizeof(struct msb250x_request), gfp_flags);
    if (!mreq)
    {
        dev_err(dev, "alloc memory fail\n");
        return NULL;
    }

    INIT_LIST_HEAD(&mreq->queue);

    return &mreq->req;
}

void msb250x_ep_free_request(struct usb_ep *ep, struct usb_request *req)
{
    struct msb250x_request *mreq = to_msb250x_req(req);

    WARN_ON(!list_empty(&mreq->queue));
    kfree(mreq);
}

static void msb250x_zlp_complete(struct usb_ep *ep, struct usb_request *req)
{
    msb250x_ep_free_request(ep, req);
}

int __msb250x_ep_queue(struct usb_ep *ep, struct usb_request *req)
{
    struct msb250x_request *mreq = to_msb250x_req(req);
    struct msb250x_ep *     mep  = to_msb250x_ep(ep);
    struct msb250x_udc *    udc  = mep->udc;
    struct device *         dev  = &udc->pdev->dev;
    int                     ret;

    if (unlikely(!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN))
    {
        return -ESHUTDOWN;
    }

    if (req->length)
    {
        if (unlikely(!req || !req->complete || !req->buf || !list_empty(&mreq->queue)))
        {
            if (!req)
                dev_err(dev, "%s: 1 X X X\n", __FUNCTION__);
            else
            {
                dev_err(dev, "%s: 0 %01d %01d %01d\n", __FUNCTION__, !req->complete, !req->buf,
                        !list_empty(&mreq->queue));
            }

            return -EINVAL;
        }
    }

    req->status = -EINPROGRESS;
    req->actual = 0;

    if (!IS_ERR_OR_NULL(ep->desc) && udc->using_dma)
    {
        ret = msb250x_gadget_map_request(mep->gadget, req, usb_endpoint_dir_in(ep->desc));
        if (ret)
            return ret;
    }

    if (list_empty(&mep->queue))
    {
        mreq = msb250x_request_handler(mep, mreq);
    }

    if (mreq != NULL)
    {
        list_add_tail(&mreq->queue, &mep->queue);
    }

    return 0;
}

int msb250x_ep_queue(struct usb_ep *ep, struct usb_request *req, gfp_t gfp_flags)
{
    struct msb250x_ep * mep = to_msb250x_ep(ep);
    struct msb250x_udc *udc = NULL;
    unsigned long       flags;
    struct usb_request *zlp_req;
    int                 ret;

    udc = mep->udc;
    spin_lock_irqsave(&udc->lock, flags);

    ret = __msb250x_ep_queue(ep, req);

    if (ret == 0 && req && req->zero && req->length && (req->length % ep->maxpacket == 0))
    {
        zlp_req           = msb250x_ep_alloc_request(ep, gfp_flags);
        zlp_req->length   = 0;
        zlp_req->complete = msb250x_zlp_complete;
        zlp_req->buf      = udc->zlp_buf;
        ret               = __msb250x_ep_queue(ep, zlp_req);
    }

    spin_unlock_irqrestore(&udc->lock, flags);

    return ret;
}

int msb250x_ep_dequeue(struct usb_ep *ep, struct usb_request *req)
{
    struct msb250x_request *tmp, *mreq = to_msb250x_req(req);
    struct msb250x_ep *     mep = to_msb250x_ep(ep);
    struct msb250x_udc *    udc = mep->udc;
    unsigned long           flags;

    if (!udc->driver)
        return -ESHUTDOWN;

    spin_lock_irqsave(&udc->lock, flags);
    /* Pluck the descriptor from queue */
    list_for_each_entry(tmp, &mep->queue, queue)
    {
        if (tmp == mreq)
        {
            list_del_init(&mreq->queue);
            break;
        }
    }

    if (tmp != mreq)
    {
        spin_unlock_irqrestore(&udc->lock, flags);
        return -EINVAL;
    }
    msb250x_request_done(mep, mreq, -ECONNRESET);

    spin_unlock_irqrestore(&udc->lock, flags);

    return 0;
}

int msb250x_ep_set_halt(struct usb_ep *ep, int value)
{
    u8                      ep_num  = 0;
    struct msb250x_ep *     mep     = to_msb250x_ep(ep);
    struct msb250x_udc *    udc     = mep->udc;
    u8                      csr     = 0;
    struct otg0_ep_rxcsr_l *rxcsr_l = NULL;
    struct otg0_ep_txcsr_l *txcsr_l = NULL;

    msb250x_set_ep_halt(mep, value ? 1 : 0);

    if (IS_ERR_OR_NULL(ep->desc))
    {
        msb250x_ep0_clr_opr_set_ss(udc);
    }
    else
    {
        ep_num = usb_endpoint_num(ep->desc);

        if (usb_endpoint_dir_out(ep->desc))
        {
            csr     = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
            rxcsr_l = (struct otg0_ep_rxcsr_l *)&csr;

            if (value)
            {
                rxcsr_l->bSendStall = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
            }
            else
            {
                rxcsr_l->bSendStall  = 0;
                rxcsr_l->bClrDataTog = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));

                if (1 == rxcsr_l->bRxPktRdy)
                {
                    msb250x_request_continue(mep);
                }
            }
        }
        else
        {
            csr     = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
            txcsr_l = (struct otg0_ep_txcsr_l *)&csr;

            if (value)
            {
                txcsr_l->bSendStall = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
            }
            else
            {
                txcsr_l->bClrDataTog = 1;
                txcsr_l->bSendStall  = 0;
                txcsr_l->bFlushFIFO  = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
                if (0 == txcsr_l->bTxPktRdy)
                {
                    msb250x_request_continue(mep);
                }
            }
        }
    }

    return 0;
}

static void msb250x_ep0_test_mode(struct msb250x_udc *udc, u8 mode)
{
    u8 testPacket[53] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                         0xAA, 0xAA, 0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFE, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF, 0xEF, 0xF7,
                         0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0x7E};
    switch (mode)
    {
        case USB_TEST_J:
            printk("TEST_J mode\n");
            ms_writeb(MSB250X_OTG0_TESTMODE_TEST_J, MSB250X_OTG0_TESTMODE_REG(udc->otg_base));
            break;
        case USB_TEST_K:
            printk("TEST_K mode\n");
            ms_writeb(MSB250X_OTG0_TESTMODE_TEST_K, MSB250X_OTG0_TESTMODE_REG(udc->otg_base));
            break;
        case USB_TEST_SE0_NAK:
            printk("TEST_SE0_NAK mode\n");
            ms_writeb(MSB250X_OTG0_TESTMODE_TEST_SE0_NAK, MSB250X_OTG0_TESTMODE_REG(udc->otg_base));
            break;
        case USB_TEST_PACKET:
            printk("TEST_PACKET mode\n");
            ms_writeb(MSB250X_OTG0_TESTMODE_TEST_PACKET, MSB250X_OTG0_TESTMODE_REG(udc->otg_base));
            ms_writesb(testPacket, OTG0_EP_FIFO_ACCESS_L(udc->otg_base, 0), 53);
            msb250x_ep0_set_de_in(udc);
            break;
        default:
            printk("UNKNOWN TEST mode\n");
            break;
    }
}

static void msb250x_ep0_handle_idle(struct msb250x_udc *udc, struct msb250x_ep *mep)
{
    unsigned int           len     = 0;
    signed int             ret     = 0;
    u8                     address = 0;
    struct usb_ctrlrequest ctrl_req;
    struct device *        dev = &udc->pdev->dev;

    len = ms_readb(MSB250X_OTG0_EP0_COUNT0_REG(udc->otg_base));
    if (sizeof(struct usb_ctrlrequest) != len)
    {
        dev_err(dev, "setup begin: fifo READ ERROR wanted %zu bytes got %d. Stalling out...\n",
                sizeof(struct usb_ctrlrequest), len);
        msb250x_ep0_clr_opr_set_ss(udc);
        return;
    }

    ms_readsb(&ctrl_req, (void *)OTG0_EP_FIFO_ACCESS_L(udc->otg_base, 0), len);
#if 0
    dev_dbg(dev,
            "<USB>[EP][0][SETUP][Dir:%s, Type:%s, Recevier:%s]"
            "bRequestType/bRequest/wValue/wIndex/wlength(0x%02x/0x%02x/0x%04x/0x%04x/0x%04x)\r\n",
            (ctrl_req.bRequestType & USB_DIR_IN) ? "IN" : "OUT",
            ((ctrl_req.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) ? "standard"
            : ((ctrl_req.bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS)  ? "class"
                                                                           : "vendor",
            ((ctrl_req.bRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE)      ? "device"
            : ((ctrl_req.bRequestType & USB_RECIP_MASK) == USB_RECIP_INTERFACE) ? "interface"
            : ((ctrl_req.bRequestType & USB_RECIP_MASK) == USB_RECIP_ENDPOINT)  ? "endpoint"
                                                                                : "other",
            ctrl_req.bRequestType, ctrl_req.bRequest, le16_to_cpu(ctrl_req.wValue), le16_to_cpu(ctrl_req.wIndex),
            le16_to_cpu(ctrl_req.wLength));
#endif
    /* cope with automagic for some standard requests. */
    udc->req_std      = (ctrl_req.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD;
    udc->delay_status = 0;
    udc->req_pending  = 1;

    if (udc->req_std)
    {
        switch (ctrl_req.bRequest)
        {
            case USB_REQ_SET_CONFIGURATION:
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    udc->delay_status++;
                }
                break;

            case USB_REQ_SET_INTERFACE:
                if (ctrl_req.bRequestType == USB_RECIP_INTERFACE)
                {
                    udc->delay_status++;
                }
                break;

            case USB_REQ_SET_ADDRESS:
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    address      = ctrl_req.wValue & 0x7F;
                    udc->address = address;
                    ms_writeb(address, MSB250X_OTG0_FADDR_REG(udc->otg_base));
                    usb_gadget_set_state(&udc->gadget, USB_STATE_ADDRESS);
                    msb250x_ep0_clear_opr(udc);
                    return;
                }
                break;

            case USB_REQ_GET_STATUS:
                msb250x_ep0_clear_opr(udc);
                if (udc->req_std)
                {
                    if (0 == msb250x_udc_get_status(udc, &ctrl_req))
                    {
                        msb250x_ep0_set_de_in(udc);
                        return;
                    }
                }
                break;

            case USB_REQ_CLEAR_FEATURE:
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    if (ctrl_req.wValue == USB_DEVICE_REMOTE_WAKEUP)
                    {
                        udc->devstatus &= ~(1 << USB_DEVICE_REMOTE_WAKEUP);
                        msb250x_ep0_clear_opr(udc);
                    }
                }
                else if (ctrl_req.bRequestType == USB_RECIP_ENDPOINT)
                {
                    if (ctrl_req.wValue == USB_ENDPOINT_HALT)
                    {
                        msb250x_ep_set_halt(&udc->mep[ctrl_req.wIndex & 0x7f].ep, 0);
                        msb250x_ep0_clear_opr(udc);
                    }
                }
                return;
            case USB_REQ_SET_FEATURE:
                msb250x_ep0_clear_opr(udc);
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    if (ctrl_req.wValue == USB_DEVICE_TEST_MODE)
                    {
                        msb250x_ep0_test_mode(udc, ctrl_req.wIndex >> 8);
                    }
                    else if (ctrl_req.wValue == USB_DEVICE_REMOTE_WAKEUP)
                    {
                        udc->devstatus |= (1 << USB_DEVICE_REMOTE_WAKEUP);
                    }
                }
                else if (ctrl_req.bRequestType == USB_RECIP_ENDPOINT)
                {
                    msb250x_ep_set_halt(&udc->mep[ctrl_req.wIndex & 0x7f].ep, 1);
                    msb250x_ep0_clear_opr(udc);
                }
                return;
            default:
                msb250x_ep0_clear_opr(udc);
                break;
        }
    }
    else
    {
        msb250x_ep0_clear_opr(udc);
    }

    if (ctrl_req.bRequestType & USB_DIR_IN)
    {
        udc->ep0state = EP0_IN_DATA_PHASE;
    }
    else
    {
        udc->ep0state = EP0_OUT_DATA_PHASE;
    }

    if (0 == ctrl_req.wLength)
    {
        udc->ep0state = EP0_IDLE;
    }

    if (udc->driver && udc->driver->setup)
    {
        spin_unlock(&udc->lock);
        ret = udc->driver->setup(&udc->gadget, &ctrl_req);
        spin_lock(&udc->lock);
    }
    else
        ret = -EINVAL;

    if (ret < 0)
    {
        if (0 < udc->delay_status)
        {
            dev_err(dev, "<USB> config change %02x fail %d?\n", ctrl_req.bRequest, ret);
            return;
        }

        dev_warn(dev, "<USB> Request(0x%02x/0x%02x/0x%04x/0x%04x/0x%04x) setup failed(%d)\n", ctrl_req.bRequestType,
                 ctrl_req.bRequest, ctrl_req.wIndex, ctrl_req.wLength, ctrl_req.wValue, ret);

        msb250x_ep0_clr_opr_set_ss(udc);

        udc->ep0state = EP0_IDLE;
    }
    else if (udc->req_pending)
    {
        udc->req_pending = 0;
    }
}

void msb250x_ep0_isr_handler(struct msb250x_udc *udc)
{
    struct msb250x_ep *     mep  = &udc->mep[0];
    struct msb250x_request *mreq = NULL;
    struct device *         dev  = &udc->pdev->dev;
    u8                      ep0csr;

    ep0csr = ms_readb(MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
    /* clear stall status */
    if (ep0csr & MSB250X_OTG0_CSR0_SENTSTALL)
    {
        msb250x_request_nuke(udc, mep, -EPIPE);
        msb250x_ep0_clear_sst(udc);
        udc->ep0state = EP0_IDLE;
        msb250x_set_ep_halt(mep, 0);
        return;
    }

    /* clear setup end */
    if (ep0csr & MSB250X_OTG0_CSR0_SETUPEND)
    {
        msb250x_request_nuke(udc, mep, 0);
        msb250x_ep0_clear_se(udc);
        udc->ep0state = EP0_IDLE;
    }

    switch (udc->ep0state)
    {
        case EP0_IDLE:
            if (ep0csr & MSB250X_OTG0_CSR0_RXPKTRDY)
            {
                msb250x_ep0_handle_idle(udc, mep);
            }
            break;

        case EP0_IN_DATA_PHASE:  /* GET_DESCRIPTOR etc */
        case EP0_OUT_DATA_PHASE: /* SET_DESCRIPTOR etc */
            if (!list_empty(&mep->queue))
            {
                mreq = list_entry(mep->queue.next, struct msb250x_request, queue);
                msb250x_set_ep_halt(mep, 0);
                msb250x_request_handler(mep, mreq);
            }
            break;

        default:
            udc->ep0state = EP0_IDLE;
            dev_err(dev, "EP0 status ... what now?\n");
            break;
    }
}

void msb250x_ep_isr_handler(struct msb250x_udc *udc, struct msb250x_ep *mep)
{
    struct usb_ep *         ep   = &mep->ep;
    struct msb250x_request *mreq = NULL;
    u8                      ep_num;
    u8                      using_dma;
    unsigned int            counts      = 0;
    struct device *         dev         = &udc->pdev->dev;
    struct otg0_ep_rxcsr_h *pst_rxcsr_h = NULL;
    struct otg0_ep_rxcsr_l *pst_rxcsr_l = NULL;
    struct otg0_ep_txcsr_l *pst_txcsr_l = NULL;
    u8                      csr1 = 0, csr2 = 0;

    ep_num    = usb_endpoint_num(ep->desc);
    using_dma = msb250x_dma_find_channel_by_ep(udc, ep_num);

    if (likely(!list_empty(&mep->queue)))
    {
        mreq = list_entry(mep->queue.next, struct msb250x_request, queue);
    }

    if (usb_endpoint_dir_in(ep->desc))
    {
        csr1        = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
        pst_txcsr_l = (struct otg0_ep_txcsr_l *)&csr1;

        if (1 == pst_txcsr_l->bIncompTx)
        {
            dev_dbg(dev, "<USB>[ep][%d] Incomplete transfer.\n", ep_num);
        }

        if (1 == pst_txcsr_l->bSentStall)
        {
            pst_txcsr_l->bSentStall = 0;
            ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
            return;
        }

        if (!using_dma)
        {
            msb250x_set_ep_halt(mep, 0);
        }
    }
    else
    {
        csr1        = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
        csr2        = ms_readb(MSB250X_OTG0_EP_RXCSR2_REG(udc->otg_base, ep_num));
        pst_rxcsr_l = (struct otg0_ep_rxcsr_l *)&csr1;
        pst_rxcsr_h = (struct otg0_ep_rxcsr_h *)&csr2;

        if (1 == pst_rxcsr_l->bSentStall)
        {
            pst_rxcsr_l->bSentStall = 0;
            ms_writeb(csr1, MSB250X_OTG0_RXCSR1_REG(udc->otg_base));
            return;
        }

        if (mreq)
        {
            if (1 == pst_rxcsr_l->bRxPktRdy)
            {
                if (usb_endpoint_xfer_bulk(ep->desc))
                {
                    counts = ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(udc->otg_base, ep_num));
                    if (mep->shortPkt == 1)
                    {
                        msb250x_udc_ok2rcv_for_packets(udc, mep->autoNAK_cfg, 0);
                    }
                }
                if (using_dma)
                {
                    msb250x_dma_isr_handler(using_dma, udc); // meet short packet
                    msb250x_set_ep_halt(mep, 0);
                }
                else
                {
                    msb250x_set_ep_halt(mep, 0);
                    mreq = msb250x_request_handler(mep, mreq);
                }

                using_dma =
                    msb250x_dma_find_channel_by_ep(udc, ep_num); // need to refresh dma channel after handle request

                if (!using_dma)
                {
                    /*
                        1. zero length packet need to continue request again.
                        2. when request is completed by riu mode, it is necessary to continue request.
                    */
                    if (NULL == mreq || 0 == counts)
                    {
                        msb250x_set_ep_halt(mep, 0);
                    }
                }
            }
        }
    }

    msb250x_request_continue(mep);
}
