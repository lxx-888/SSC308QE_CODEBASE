/*
 * msb250x_udc.c- Sigmastar
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

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/dma-mapping.h>
#include <linux/bitfield.h>
#include <linux/phy/phy.h>
#include "msb250x_udc_reg.h"
#include "msb250x_udc.h"
#include "msb250x_gadget.h"
#include "msb250x_ep.h"
#include "msb250x_dma.h"
#include <linux/reset.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/pm_domain.h>
#include <linux/pm.h>

/* the module parameter */
#define DRIVER_DESC    "MSB250X USB Device Controller Gadget"
#define DRIVER_VERSION "03/11 2024"
#define DRIVER_AUTHOR  "sigmastar"

#define AUTONAK_COUNT 2

static const struct usb_ep_ops msb250x_ep_ops = {
    .enable        = msb250x_ep_enable,
    .disable       = msb250x_ep_disable,
    .alloc_request = msb250x_ep_alloc_request,
    .free_request  = msb250x_ep_free_request,
    .queue         = msb250x_ep_queue,
    .dequeue       = msb250x_ep_dequeue,
    .set_halt      = msb250x_ep_set_halt,
};

static const struct usb_gadget_ops msb250x_gadget_ops = {
    .get_frame       = msb250x_gadget_get_frame,
    .wakeup          = msb250x_gadget_wakeup,
    .set_selfpowered = msb250x_gadget_set_selfpowered,
    .pullup          = msb250x_gadget_pullup,
    .udc_start       = msb250x_gadget_udc_start,
    .match_ep        = msb250x_gadget_match_ep,
    .udc_stop        = msb250x_gadget_udc_stop,
};

static int msb250x_udc_write_riu(struct msb250x_ep *mep, struct msb250x_request *mreq, u32 size)
{
    struct usb_ep *     ep  = &mep->ep;
    struct msb250x_udc *udc = mep->udc;
    u8                  ep_num;

    ep_num = IS_ERR_OR_NULL(ep->desc) ? 0 : usb_endpoint_num(ep->desc);

    if (size)
    {
        ms_writesb((mreq->req.buf + mreq->req.actual), OTG0_EP_FIFO_ACCESS_L(udc->otg_base, ep_num), size);
        mreq->req.actual += size;
    }
    /* last packet is often short (sometimes a zlp) */
    if (size != ep->maxpacket || mreq->req.length == mreq->req.actual)
    {
        list_del_init(&mreq->queue);
        msb250x_request_done(mep, mreq, 0);
        return 1;
    }

    return 0;
}

static int msb250x_udc_read_riu(struct msb250x_ep *mep, struct msb250x_request *mreq, u32 size)
{
    u8                  ep_num;
    int                 is_last = 0;
    struct usb_ep *     ep      = &mep->ep;
    struct msb250x_udc *udc     = mep->udc;

    if (mreq->req.length == 0)
    {
        return 1;
    }

    ep_num = IS_ERR_OR_NULL(ep->desc) ? 0 : usb_endpoint_num(ep->desc);

    if (size)
    {
        ms_readsb((mreq->req.buf + mreq->req.actual), OTG0_EP_FIFO_ACCESS_L(udc->otg_base, ep_num), size);
        mreq->req.actual += size;
        if (size != mep->ep.maxpacket || mreq->req.zero)
        {
            is_last = 1;
        }
    }

    if (mreq->req.length == mreq->req.actual)
    {
        is_last = 1;
    }

    if (is_last)
    {
        list_del_init(&mreq->queue);
        msb250x_request_done(mep, mreq, 0);
    }

    return is_last;
}

int msb250x_udc_get_autoNAK_cfg(struct msb250x_udc *udc, u8 ep_num)
{
    int cfg        = 0;
    u8  ep_bulkout = 0;

    for (cfg = 0; cfg < AUTONAK_COUNT; cfg++)
    {
        switch (cfg)
        {
            case 0:
                ep_bulkout = (ms_readb(MSB250X_OTG0_AUTONAK0_EP_BULKOUT(udc->otg_base)) & 0x0f);
                break;
            case 1:
                ep_bulkout = (ms_readb(MSB250X_OTG0_AUTONAK1_EP_BULKOUT(udc->otg_base)) & 0x0f);
                break;
            case 2:
                ep_bulkout = (ms_readb(MSB250X_OTG0_AUTONAK2_EP_BULKOUT(udc->otg_base)) & 0xf0);
                ep_bulkout = ep_bulkout >> 4;
                break;
        }

        if (0 == ep_bulkout)
        {
            return (cfg + 1);
        }
    }

    return 0;
}

int msb250x_udc_release_autoNAK_cfg(struct msb250x_udc *udc, u8 cfg)
{
    cfg--;

    switch (cfg)
    {
        case 0:
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK0_EP_BULKOUT(udc->otg_base)) & 0xf0),
                      MSB250X_OTG0_AUTONAK0_EP_BULKOUT(udc->otg_base));
            ms_writew(0x0000, MSB250X_OTG0_USB_CFG5_L(udc->otg_base));
            break;
        case 1:
            ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_H(udc->otg_base)) & 0x80, MSB250X_OTG0_USB_CFG0_H(udc->otg_base));
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT(udc->otg_base)) & 0xE000),
                      MSB250X_OTG0_AUTONAK1_RX_PKT_CNT(udc->otg_base));
            break;
        case 2:
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK2_EP_BULKOUT(udc->otg_base)) & 0x0f),
                      MSB250X_OTG0_AUTONAK2_EP_BULKOUT(udc->otg_base));
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base))
                       & ~(MSB250X_OTG0_AUTONAK2_EN | MSB250X_OTG0_AUTONAK2_OK2Rcv | MSB250X_OTG0_AUTONAK2_AllowAck)),
                      MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base));
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT(udc->otg_base)) & 0xE000),
                      MSB250X_OTG0_AUTONAK2_RX_PKT_CNT(udc->otg_base));
            break;
    }

    return 0;
}

int msb250x_udc_init_autoNAK_cfg(struct msb250x_udc *udc)
{
    int cfg = 0;

    for (cfg = 0; cfg < AUTONAK_COUNT; cfg++)
    {
        msb250x_udc_release_autoNAK_cfg(udc, (cfg + 1));
    }

    return 0;
}

void msb250x_udc_enable_autoNAK(struct msb250x_udc *udc, u8 ep_num, u8 cfg)
{
    cfg--;

    switch (cfg)
    {
        case 0:
            ep_num &= 0x0f;
            ms_writeb(ep_num, MSB250X_OTG0_AUTONAK0_EP_BULKOUT(udc->otg_base));
            {
                u16 ctrl = 0;
                ctrl |= MSB250X_OTG0_AUTONAK0_EN;
                ms_writew(ctrl, MSB250X_OTG0_AUTONAK0_CTRL(udc->otg_base));
            }
            break;
        case 1:
            ep_num &= 0x0f;
            ms_writeb(MSB250X_OTG0_AUTONAK1_EN | ep_num, MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base));
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base))
                       & ~(MSB250X_OTG0_AUTONAK1_OK2Rcv | MSB250X_OTG0_AUTONAK1_AllowAck)),
                      MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base));
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT(udc->otg_base)) & 0xE000),
                      MSB250X_OTG0_AUTONAK1_RX_PKT_CNT(udc->otg_base));
            break;
        case 2:
            ep_num = ep_num << 4;
            ep_num &= 0xf0;
            ms_writew(MSB250X_OTG0_AUTONAK2_EN | (u16)ep_num, MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base));
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base))
                       & ~(MSB250X_OTG0_AUTONAK2_OK2Rcv | (u16)MSB250X_OTG0_AUTONAK2_AllowAck)),
                      MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base));
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT(udc->otg_base)) & 0xE000),
                      MSB250X_OTG0_AUTONAK2_RX_PKT_CNT(udc->otg_base));
            break;
    }
}

void msb250x_udc_ok2rcv_for_packets(struct msb250x_udc *udc, u8 cfg, u16 pkt_num)
{
    u16 ctrl = 0;
    cfg--;

    switch (cfg)
    {
        case 0:
            ctrl |= MSB250X_OTG0_AUTONAK0_EN;
            ctrl |= (pkt_num & ~0xE000);

            if (0 < pkt_num)
            {
                ctrl |= MSB250X_OTG0_AUTONAK0_OK2Rcv;
            }

            ms_writew(ctrl, MSB250X_OTG0_AUTONAK0_CTRL(udc->otg_base));
            break;
        case 1:
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT(udc->otg_base)) & ~0xE000) | pkt_num,
                      MSB250X_OTG0_AUTONAK1_RX_PKT_CNT(udc->otg_base));
            ctrl = ms_readb(MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base));

            if (0 < pkt_num)
            {
                ctrl |= MSB250X_OTG0_AUTONAK1_OK2Rcv;
            }

            ms_writeb(ctrl & 0xFF, MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base));
            break;
        case 2:
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT(udc->otg_base)) & ~0xE000) | pkt_num,
                      MSB250X_OTG0_AUTONAK2_RX_PKT_CNT(udc->otg_base));
            ctrl = ms_readw(MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base));

            if (0 < pkt_num)
            {
                ctrl |= MSB250X_OTG0_AUTONAK2_OK2Rcv;
            }

            ms_writew(ctrl, MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base));
            break;
    }
}

void msb250x_udc_allowAck(struct msb250x_udc *udc, u8 cfg)
{
    cfg--;

    switch (cfg)
    {
        case 0:
            ms_writew(ms_readw(MSB250X_OTG0_AUTONAK0_CTRL(udc->otg_base)), MSB250X_OTG0_AUTONAK0_CTRL(udc->otg_base));
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK0_CTRL(udc->otg_base)) | MSB250X_OTG0_AUTONAK0_AllowAck),
                      MSB250X_OTG0_AUTONAK0_CTRL(udc->otg_base));
            break;
        case 1:
            ms_writeb(ms_readb(MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base)), MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base));
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base)) | MSB250X_OTG0_AUTONAK1_AllowAck),
                      MSB250X_OTG0_AUTONAK1_CTRL(udc->otg_base));
            break;
        case 2:
            ms_writeb(ms_readw(MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base)), MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base));
            ms_writeb((ms_readw(MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base)) | MSB250X_OTG0_AUTONAK2_AllowAck),
                      MSB250X_OTG0_AUTONAK2_CTRL(udc->otg_base));
            break;
    }
}

struct msb250x_request *msb250x_request_handler(struct msb250x_ep *mep, struct msb250x_request *mreq)
{
    u32                     bytes;
    u16                     pkt_num    = 0;
    u8                      ep_num     = 0;
    u8                      csr        = 0;
    u8                      intrusb    = 0;
    u8                      is_last    = 0;
    struct msb250x_udc *    udc        = mep->udc;
    struct usb_ep *         ep         = &mep->ep;
    struct otg0_ep0_csr_l * csr_l      = NULL;
    struct otg0_ep_rxcsr_l *rxcsr_l    = NULL;
    struct otg0_ep_txcsr_l *txcsr_l    = NULL;
    struct otg0_usb_intr *  st_intrusb = (struct otg0_usb_intr *)&intrusb;
    struct device *         dev        = &udc->pdev->dev;

    if (0 == mep->halted)
    {
        msb250x_set_ep_halt(mep, 1);
        intrusb = ms_readb(MSB250X_OTG0_INTRUSB_REG(udc->otg_base));

        if (1 == st_intrusb->bReset)
        {
            return mreq;
        }
        bytes = mreq->req.length - mreq->req.actual;
        if (IS_ERR_OR_NULL(ep->desc))
        {
            csr   = ms_readb(MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
            csr_l = (struct otg0_ep0_csr_l *)&csr;

            switch (mep->udc->ep0state)
            {
                case EP0_IDLE:
                    if (0 < udc->delay_status)
                    {
                        msb250x_ep0_clear_opr(udc);
                        udc->delay_status--;
                        is_last = 1;
                    }

                    break;
                case EP0_IN_DATA_PHASE:
                    if (0 == csr_l->bTxPktRdy)
                    {
                        bytes = min((u16)ep->maxpacket, (u16)bytes);

                        is_last = msb250x_udc_write_riu(mep, mreq, bytes);
                        if (is_last && !mreq->req.zero)
                        {
                            msb250x_ep0_set_de_in(udc);
                            mep->udc->ep0state = EP0_IDLE;
                        }
                        else
                        {
                            msb250x_ep0_set_ipr(udc);
                        }
                    }
                    break;

                case EP0_OUT_DATA_PHASE:
                    if (1 == csr_l->bRxPktRdy)
                    {
                        u16 counts = ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(udc->otg_base, ep_num));

                        if (bytes < counts)
                        {
                            dev_warn(dev,
                                     "<USB_WARN>[UDC][%d] Bytes in fifo(0x%04x) is more than buffer size. "
                                     "buf/actual/length(0x%p/0x%04x/0x%04x)!\n",
                                     ep_num, counts, mreq->req.buf, mreq->req.actual, mreq->req.length);
                        }

                        bytes = min((u16)counts, (u16)bytes);

                        is_last = msb250x_udc_read_riu(mep, mreq, bytes);

                        if (is_last)
                        {
                            msb250x_ep0_set_de_out(udc);
                            mep->udc->ep0state = EP0_IDLE;
                        }
                        else
                        {
                            msb250x_ep0_clear_opr(udc);
                        }
                    }

                    break;

                default:
                    dev_err(dev, "<USB_ERR> EP0 Request Error !!\n");
            }

            msb250x_set_ep_halt(mep, 0);
        }
        else
        {
            ep_num = usb_endpoint_num(ep->desc);

            if (!usb_endpoint_xfer_bulk(ep->desc))
            {
                bytes = min((u16)(usb_endpoint_maxp_mult(ep->desc) * ep->maxpacket), (u16)bytes);
            }

            if (usb_endpoint_dir_in(ep->desc))
            {
                txcsr_l = (struct otg0_ep_txcsr_l *)&csr;
                csr     = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));

                if (0 == txcsr_l->bTxPktRdy)
                {
                    if (0 == udc->using_dma || 0 != msb250x_dma_setup_control(ep, mreq, bytes))
                    {
                        is_last = msb250x_udc_write_riu(mep, mreq, bytes);
                        ep_set_ipr(udc, ep_num);
                    }
                }
            }
            else if (usb_endpoint_dir_out(ep->desc))
            {
                csr     = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
                rxcsr_l = (struct otg0_ep_rxcsr_l *)&csr;
                if (usb_endpoint_xfer_bulk(ep->desc))
                {
                    if (0 == rxcsr_l->bRxPktRdy)
                    {
                        pkt_num = (bytes + (ep->maxpacket - 1)) / ep->maxpacket;

                        if (1 == mep->shortPkt)
                        {
                            udelay(125);
                        }

                        pkt_num = 1;
                        if (1 == pkt_num)
                        {
                            msb250x_udc_allowAck(udc, mep->autoNAK_cfg);
                            mep->shortPkt = 0;
                        }
                        else
                        {
                            bytes = min((u16)(pkt_num * ep->maxpacket), (u16)bytes);

                            msb250x_dma_setup_control(&mep->ep, mreq, bytes);
                            msb250x_udc_ok2rcv_for_packets(udc, mep->autoNAK_cfg, pkt_num);
                            mep->shortPkt = 1;
                        }
                    }
                }

                if (1 == rxcsr_l->bRxPktRdy)
                {
                    bytes = min((u16)bytes, (u16)ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(udc->otg_base, ep_num)));
                    if (0 == udc->using_dma || 0 != msb250x_dma_setup_control(&mep->ep, mreq, bytes))
                    {
                        is_last = msb250x_udc_read_riu(mep, mreq, bytes);
                        ep_set_opr(udc, ep_num);
                    }
                }
            }
        }
    }

    if (1 == is_last)
    {
        mreq = NULL;
        msb250x_set_ep_halt(mep, 0);
    }

    return mreq;
}

void msb250x_request_nuke(struct msb250x_udc *udc, struct msb250x_ep *mep, int status)
{
    struct msb250x_request *mreq, *r;

    /* Sanity check */
    if ((void *)&mep->queue == NULL)
        return;

    list_for_each_entry_safe(mreq, r, &mep->queue, queue)
    {
        list_del_init(&mreq->queue);
        msb250x_request_done(mep, mreq, status);
    }
}

void msb250x_request_continue(struct msb250x_ep *mep)
{
    struct msb250x_request *mreq = NULL;

    if (likely(!list_empty(&mep->queue)))
    {
        mreq = list_entry(mep->queue.next, struct msb250x_request, queue);
        msb250x_request_handler(mep, mreq);
    }
}

void msb250x_request_done(struct msb250x_ep *mep, struct msb250x_request *mreq, int status)
{
    struct msb250x_udc *udc    = mep->udc;
    struct device *     dev    = &udc->pdev->dev;
    unsigned            halted = mep->halted;

    if (mreq == NULL)
    {
        dev_err(dev, "<USB> EP[%d] REQ NULL\n", IS_ERR_OR_NULL(mep->ep.desc) ? 0 : usb_endpoint_num(mep->ep.desc));
        return;
    }

    if (!IS_ERR_OR_NULL(mep->ep.desc) && udc->using_dma)
    {
        msb250x_gadget_unmap_request(mep->gadget, &mreq->req, usb_endpoint_dir_in(mep->ep.desc));
    }

    if (likely(mreq->req.status == -EINPROGRESS))
        mreq->req.status = status;
    else
        status = mreq->req.status;

    msb250x_set_ep_halt(mep, 1);
    spin_unlock(&udc->lock);
    usb_gadget_giveback_request(&mep->ep, &mreq->req);
    spin_lock(&udc->lock);
    msb250x_set_ep_halt(mep, halted);
}

static irqreturn_t msb250x_udc_link_isr(struct msb250x_udc *udc)
{
    u8                     usb_status, power;
    struct otg0_usb_power *pst_power = (struct otg0_usb_power *)&power;
    struct otg0_usb_intr * usb_st    = (struct otg0_usb_intr *)&usb_status;
    struct device *        dev       = &udc->pdev->dev;

    power      = ms_readb(MSB250X_OTG0_PWR_REG(udc->otg_base));
    usb_status = ms_readb(MSB250X_OTG0_INTRUSB_REG(udc->otg_base));
    ms_writeb(usb_status, MSB250X_OTG0_INTRUSB_REG(udc->otg_base));

    /* RESET */
    if (usb_st->bReset)
    {
        /* two kind of reset :
         * - reset start -> pwr reg = 8
         * - reset end   -> pwr reg = 0
         **/
        dev_info(dev, "<USB>[LINK] Bus reset\n");
        if (udc->driver && udc->driver->disconnect && USB_STATE_DEFAULT <= udc->gadget.state)
        {
            spin_unlock(&udc->lock);
            udc->driver->disconnect(&udc->gadget);
            spin_lock(&udc->lock);
        }
        ms_writeb(0, MSB250X_OTG0_FADDR_REG(udc->otg_base));
        udc->address  = 0;
        udc->ep0state = EP0_IDLE;

        if (pst_power->bHSMode)
        {
            udc->gadget.speed = USB_SPEED_HIGH;
            dev_info(dev, "<USB>[LINK] High speed device\n");
        }
        else
        {
            udc->gadget.speed = USB_SPEED_FULL;
            dev_info(dev, "<USB>[LINK] Full speed device\n");
        }
        msb250x_udc_init_autoNAK_cfg(udc);
        usb_gadget_set_state(&udc->gadget, USB_STATE_DEFAULT);
    }
    /* RESUME */
    if (usb_st->bResume)
    {
        dev_info(dev, "<USB>[LINK] Resume\n");
        if (udc->gadget.speed != USB_SPEED_UNKNOWN && udc->driver && udc->driver->resume)
        {
            spin_unlock(&udc->lock);
            udc->driver->resume(&udc->gadget);
            spin_lock(&udc->lock);
        }
    }
    /* SUSPEND */
    if (usb_st->bSuspend)
    {
        dev_info(dev, "<USB>[LINK] Suspend\n");
        if (pst_power->bSuspendMode)
        {
            if (udc->gadget.speed != USB_SPEED_UNKNOWN && udc->driver && udc->driver->suspend)
            {
                dev_info(dev, "gadget->suspend\n");
                spin_unlock(&udc->lock);
                udc->driver->suspend(&udc->gadget);
                spin_lock(&udc->lock);
                udc->ep0state = EP0_IDLE;
                usb_gadget_set_state(&udc->gadget, USB_STATE_SUSPENDED);
            }
        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t msb250x_udc_ep_isr(struct msb250x_udc *udc)
{
    u8 ep_num;
    u8 usb_intr_rx = 0, usb_intr_tx = 0;

    usb_intr_rx = ms_readb(MSB250X_OTG0_INTRRX_REG(udc->otg_base));
    usb_intr_tx = ms_readb(MSB250X_OTG0_INTRTX_REG(udc->otg_base));
    ms_writeb(usb_intr_rx, MSB250X_OTG0_INTRRX_REG(udc->otg_base));
    ms_writeb(usb_intr_tx, MSB250X_OTG0_INTRTX_REG(udc->otg_base));

    if (usb_intr_tx & MSB250X_OTG0_INTR_EP(0))
    {
        msb250x_ep0_isr_handler(udc);
    }

    for (ep_num = 1; ep_num < udc->max_ep_num; ep_num++)
    {
        if ((usb_intr_rx | usb_intr_tx) & MSB250X_OTG0_INTR_EP(ep_num))
        {
            msb250x_ep_isr_handler(udc, &udc->mep[ep_num]);
        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t msb250x_udc_dma_isr(struct msb250x_udc *udc)
{
    u8 dma_ch = 0;
    u8 dma_intr;

    dma_intr = ms_readb(MSB250X_OTG0_DMA_INTR(udc->otg_base));
    ms_writeb(dma_intr, MSB250X_OTG0_DMA_INTR(udc->otg_base));

    if (dma_intr)
    {
        for (dma_ch = 0; dma_ch < udc->dma_chanel_num; dma_ch++)
        {
            if (dma_intr & (1 << dma_ch))
            {
                msb250x_dma_isr_handler((dma_ch + 1), udc);
            }
        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t msb250x_udc_isr(int irq, void *_dev)
{
    struct msb250x_udc *udc = _dev;
    unsigned long       flags;

    spin_lock_irqsave(&udc->lock, flags);
    msb250x_udc_dma_isr(udc);
    msb250x_udc_link_isr(udc);
    msb250x_udc_ep_isr(udc);
    spin_unlock_irqrestore(&udc->lock, flags);

    return IRQ_HANDLED;
}

int msb250x_udc_get_status(struct msb250x_udc *udc, struct usb_ctrlrequest *crq)
{
    u8 status = 0;
    u8 ep_num = crq->wIndex & 0x7F;
    u8 is_in  = crq->wIndex & USB_DIR_IN;

    switch (crq->bRequestType & USB_RECIP_MASK)
    {
        case USB_RECIP_INTERFACE:
            break;

        case USB_RECIP_DEVICE:
            status = udc->devstatus;
            break;

        case USB_RECIP_ENDPOINT:
            if (udc->max_ep_num < ep_num || crq->wLength > 2)
                return 1;

            if (ep_num == 0)
            {
                status = ms_readb(MSB250X_OTG0_EP0_CSR0_REG(udc->otg_base));
                status &= MSB250X_OTG0_CSR0_SENDSTALL;
            }
            else
            {
                if (is_in)
                {
                    status = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(udc->otg_base, ep_num));
                    status &= MSB250X_OTG0_TXCSR1_SENDSTALL;
                }
                else
                {
                    status = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(udc->otg_base, ep_num));
                    status &= MSB250X_OTG0_RXCSR1_SENDSTALL;
                }
            }

            status = status ? 1 : 0;
            break;

        default:
            return 1;
    }

    /* Seems to be needed to get it working. ouch :( */
    udelay(5);
    ms_writeb(status & 0xFF, OTG0_EP_FIFO_ACCESS_L(udc->otg_base, 0));
    ms_writeb(0, OTG0_EP_FIFO_ACCESS_L(udc->otg_base, 0));

    return 0;
}

static void msb250x_udc_init_eps(struct msb250x_udc *udc, u8 max_ep_num, u32 *ep_maxpkt_limit, u32 *ep_fifo_size,
                                 const char **ep_name)
{
    struct msb250x_ep *mep;
    u8                 idx;

    INIT_LIST_HEAD(&udc->gadget.ep_list);
    for (idx = 0; idx < max_ep_num; idx++)
    {
        mep          = &udc->mep[idx];
        mep->udc     = udc;
        mep->gadget  = &udc->gadget;
        mep->ep.name = ep_name[idx];
        usb_ep_set_maxpacket_limit(&mep->ep, ep_maxpkt_limit[idx]);
        mep->fifo_size       = ep_fifo_size[idx];
        mep->ep.ops          = &msb250x_ep_ops;
        mep->ep.caps.dir_in  = true;
        mep->ep.caps.dir_out = true;
        if (idx == 0)
        {
            mep->ep.caps.type_control = true;
            udc->gadget.ep0           = &mep->ep;
        }
        else
        {
            list_add_tail(&mep->ep.ep_list, &udc->gadget.ep_list);
            mep->ep.caps.type_iso  = true;
            mep->ep.caps.type_bulk = true;
            mep->ep.caps.type_int  = true;
        }

        INIT_LIST_HEAD(&mep->queue);
    }
}

void msb250x_udc_deinit_usb0(struct msb250x_udc *udc)
{
    u8                       ctrl_l;
    struct usbc0_rst_ctrl_l *pst_usbc0_rst_ctrl_l = (struct usbc0_rst_ctrl_l *)&ctrl_l;

    ms_writeb(0x40, GET_REG16_ADDR(udc->usb0_base, 1));
    ctrl_l                             = ms_readb(GET_REG16_ADDR(udc->usb0_base, 0));
    pst_usbc0_rst_ctrl_l->bOTG_RST     = 1;
    pst_usbc0_rst_ctrl_l->bREG_SUSPEND = 1;
    ms_writeb(ctrl_l, GET_REG16_ADDR(udc->usb0_base, 0));
    mdelay(1); // otg rst, mantis:3075
}

void msb250x_udc_deinit_utmi(struct msb250x_udc *udc)
{
    ms_writeb(0x00, MSB250X_OTG0_PWR_REG(udc->utmi_base));

    ms_writew(ms_readw(GET_REG16_ADDR(udc->utmi_base, 0x03)) | BIT0 | BIT1 | BIT8,
              GET_REG16_ADDR(udc->utmi_base, 0x03));
    ms_writew(ms_readw(GET_REG16_ADDR(udc->utmi_base, 0x08)) | BIT12, GET_REG16_ADDR(udc->utmi_base, 0x08));
    mdelay(1);
    ms_writew(ms_readw(GET_REG16_ADDR(udc->utmi_base, 0x03)) & (~(BIT0 | BIT1 | BIT8)),
              GET_REG16_ADDR(udc->utmi_base, 0x03));
    ms_writew(ms_readw(GET_REG16_ADDR(udc->utmi_base, 0x08)) & (~BIT12), GET_REG16_ADDR(udc->utmi_base, 0x08));
    ms_writew(0x7F03, GET_REG16_ADDR(udc->utmi_base, 0x0));
    mdelay(5);
}

void msb250x_udc_init_usb0(struct msb250x_udc *udc)
{
    u8                                 ctrl_l                     = 0;
    u8                                 eve_en_l                   = 0;
    struct usbc0_rst_ctrl_l *          pst_usbc0_rst_ctrl_l       = (struct usbc0_rst_ctrl_l *)&ctrl_l;
    struct usbc0_pwr_mng_eve_enable_l *pst_usbc0_pwr_mng_eve_en_l = (struct usbc0_pwr_mng_eve_enable_l *)&eve_en_l;
    struct device *                    dev                        = &udc->pdev->dev;

    ms_writeb(0x70, GET_REG16_ADDR(udc->usb0_base, 0x0A)); // Setting MIU0 segment
    ms_writeb((ms_readb(GET_REG16_ADDR(udc->usb0_base, 0x0C) + 1) | 0x1),
              GET_REG16_ADDR(udc->usb0_base, 0x0C) + 1); // Enable miu partition mechanism

    // Disable UHC and OTG controllers
    ms_writeb(((ms_readb(GET_REG16_ADDR(udc->usb0_base, 0x01)) & ~(BIT0 | BIT1)) | BIT1),
              GET_REG16_ADDR(udc->usb0_base, 0x01));

    ctrl_l                             = ms_readb(GET_REG16_ADDR(udc->usb0_base, 0));
    pst_usbc0_rst_ctrl_l->bOTG_RST     = 1;
    pst_usbc0_rst_ctrl_l->bREG_SUSPEND = 1;
    ms_writeb(ctrl_l, GET_REG16_ADDR(udc->usb0_base, 0));

    ctrl_l                                = ms_readb(GET_REG16_ADDR(udc->usb0_base, 0));
    pst_usbc0_rst_ctrl_l->bOTG_RST        = 0;
    pst_usbc0_rst_ctrl_l->bOTG_XIU_ENABLE = 1;
    ms_writeb(ctrl_l, GET_REG16_ADDR(udc->usb0_base, 0));
    mdelay(1); // otg rst, mantis:3075
    eve_en_l                                  = ms_readb(GET_REG16_ADDR(udc->usb0_base, 5));
    pst_usbc0_pwr_mng_eve_en_l->bRESUME_INTEN = 1;
    pst_usbc0_pwr_mng_eve_en_l->bRESET_INTEN  = 1;
    ms_writeb(eve_en_l, GET_REG16_ADDR(udc->usb0_base, 5));

    dev_dbg(dev, "<USB>[GADGET] Init USB controller\n");
}

void msb250x_udc_init_utmi(struct msb250x_udc *udc)
{
#ifdef CONFIG_USB_FPGA_VERIFICATION
    ms_writew((ms_readw(GET_REG16_ADDR(udc->chiptop_base, 0x73)) & 0x7fff),
              GET_REG16_ADDR(udc->chiptop_base, 0x73)); // bit15 = 0 pull low reset
    ms_writew((ms_readw(GET_REG16_ADDR(udc->chiptop_base, 0x73)) & 0xffef),
              GET_REG16_ADDR(udc->chiptop_base, 0x73)); // bit4 = 0 pull low stp
    ms_writew((ms_readw(GET_REG16_ADDR(udc->chiptop_base, 0x73)) & 0xefff),
              GET_REG16_ADDR(udc->chiptop_base, 0x73));         // bit12 = 0 enable data bus for riu
    ms_writew(0x6184, GET_REG16_ADDR(udc->chiptop_base, 0x74)); // command softreset
    ms_writew((ms_readw(GET_REG16_ADDR(udc->chiptop_base, 0x73)) & 0xfffe),
              GET_REG16_ADDR(udc->chiptop_base, 0x73)); // trigger command
    ms_writew((ms_readw(GET_REG16_ADDR(udc->chiptop_base, 0x73)) | ~(0xefff)),
              GET_REG16_ADDR(udc->chiptop_base, 0x73)); // bit12 = 1 disable data bus for riu
    ms_writew((ms_readw(GET_REG16_ADDR(udc->chiptop_base, 0x73)) & 0xfeff),
              GET_REG16_ADDR(udc->chiptop_base, 0x73)); // bit8 = 0 enable data bus for mac
#else
    ms_writew(0x0C2F, GET_REG16_ADDR(udc->utmi_base, 0x4));

    // Turn on UTMI if it was powered down
    if (0x0001 != ms_readw(GET_REG16_ADDR(udc->utmi_base, 0)))
    {
        ms_writew(0x0001, GET_REG16_ADDR(udc->utmi_base, 0)); // Turn all (including hs_current) use override mode
        mdelay(3);
    }

    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x3c)) | 0x01),
              GET_REG8_ADDR(udc->utmi_base, 0x3c)); // set CA_START as 1
    mdelay(10);
    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x3c)) & ~0x01),
              GET_REG16_ADDR(udc->utmi_base, 0x3c)); // release CA_START
    while (0 == (ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x3c)) & 0x02))
        ; // polling bit <1> (CA_END)

    // reg_tx_force_hs_current_enable
    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x06)) & 0x9F) | 0x40, GET_REG8_ADDR(udc->utmi_base, 0x06));
    // Disconnect window select
    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x03)) | 0x28), GET_REG8_ADDR(udc->utmi_base, 0x03));
    // Disconnect window select
    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x03)) & 0xef), GET_REG8_ADDR(udc->utmi_base, 0x03));
    // Disable improved CDR
    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x07)) & 0xfd), GET_REG8_ADDR(udc->utmi_base, 0x07));
    // UTMI RX anti-dead-loc, ISI effect improvement
    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x09)) | 0x81), GET_REG8_ADDR(udc->utmi_base, 0x09));
    // Chirp signal source select
    // ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x15)) | 0x20), GET_REG8_ADDR(udc->utmi_base, 0x15));
    // set reg_ck_inv_reserved[6] to solve timing problem
    ms_writeb((ms_readb(GET_REG8_ADDR(udc->utmi_base, 0x0b)) | 0x80), GET_REG8_ADDR(udc->utmi_base, 0x0b));
    // avoid glitch
    ms_writeb((ms_readb(GET_REG16_ADDR(udc->utmi_base, 0x02)) | 0x80), GET_REG16_ADDR(udc->utmi_base, 0x02));
#endif
}

void msb250x_udc_deinit_otg(struct msb250x_udc *udc)
{
    /* Disable all interrupts */
    ms_writeb(0x00, MSB250X_OTG0_INTRUSBE_REG(udc->otg_base));
    ms_writeb(0x00, MSB250X_OTG0_INTRTXE_REG(udc->otg_base));
    ms_writeb(0x00, MSB250X_OTG0_INTRRXE_REG(udc->otg_base));

    /* Clear the interrupt registers */
    /* All active interrupts will be cleared when this register is read */
    ms_writeb(ms_readb(MSB250X_OTG0_INTRUSB_REG(udc->otg_base)), MSB250X_OTG0_INTRUSB_REG(udc->otg_base));
    ms_writeb(ms_readb(MSB250X_OTG0_INTRTX_REG(udc->otg_base)), MSB250X_OTG0_INTRTX_REG(udc->otg_base));
    ms_writeb(ms_readb(MSB250X_OTG0_INTRRX_REG(udc->otg_base)), MSB250X_OTG0_INTRRX_REG(udc->otg_base));

    // clear addr
    ms_writeb(0, MSB250X_OTG0_FADDR_REG(udc->otg_base));

    // clear power
    ms_writeb(0, MSB250X_OTG0_PWR_REG(udc->otg_base));
}

void msb250x_udc_init_otg(struct msb250x_udc *udc)
{
    u8                      usb_cfg_h      = 0;
    u8                      power          = 0;
    u8                      usb_intr       = 0;
    u8                      usbe_intr      = 0;
    struct otg0_usb_cfg0_h *pst_usb_cfg0_h = (struct otg0_usb_cfg0_h *)&usb_cfg_h;
    struct otg0_usb_cfg6_h *pst_usb_cfg6_h = NULL;
    struct otg0_usb_power * pst_power      = (struct otg0_usb_power *)&power;
    struct otg0_usb_intr *  pst_intr_usb   = (struct otg0_usb_intr *)&usb_intr;
    struct otg0_usbe_intr * pst_intr_usbe  = (struct otg0_usbe_intr *)&usbe_intr;

    /* soft reset otg */
    ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_L(udc->otg_base)) & ~MSB250X_OTG0_CFG0_SRST_N,
              MSB250X_OTG0_USB_CFG0_L(udc->otg_base)); // low active
    ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_L(udc->otg_base)) | MSB250X_OTG0_CFG0_SRST_N,
              MSB250X_OTG0_USB_CFG0_L(udc->otg_base)); // default

    /* Enable DM pull down */
    usb_cfg_h                   = ms_readb(MSB250X_OTG0_USB_CFG0_H(udc->otg_base));
    pst_usb_cfg0_h->bDMPullDown = 1;
    ms_writeb(usb_cfg_h, MSB250X_OTG0_USB_CFG0_H(udc->otg_base));

    /* Set FAddr to 0 */
    ms_writeb(0, MSB250X_OTG0_FADDR_REG(udc->otg_base));

    pst_usb_cfg6_h                      = (struct otg0_usb_cfg6_h *)&usb_cfg_h;
    usb_cfg_h                           = ms_readb(MSB250X_OTG0_USB_CFG6_H(udc->otg_base));
    pst_usb_cfg6_h->bINT_WR_CLR_EN      = 1;
    pst_usb_cfg6_h->bBusOPFix           = 1;
    pst_usb_cfg6_h->bShortMode          = 1;
    pst_usb_cfg6_h->bREG_MI_WDFIFO_CTRL = 1;
    ms_writeb(usb_cfg_h, MSB250X_OTG0_USB_CFG6_H(udc->otg_base));

    power                      = ms_readb(MSB250X_OTG0_PWR_REG(udc->otg_base));
    pst_power->bSuspendMode    = 0;
    pst_power->bSoftConn       = 0;
    pst_power->bEnableSuspendM = 1; /* USB suspend function Enable  */
    if (udc->gadget.max_speed == USB_SPEED_HIGH)
    {
        pst_power->bHSEnab = 1;
    }
    else
    {
        pst_power->bHSEnab = 0;
    }
    ms_writeb(power, MSB250X_OTG0_PWR_REG(udc->otg_base));
    ms_writeb(0, MSB250X_OTG0_DEVCTL_REG(udc->otg_base));

    usb_intr            = 0xff;
    pst_intr_usb->bConn = 0;
    pst_intr_usb->bSOF  = 0;
    ms_writeb(usb_intr, MSB250X_OTG0_INTRUSB_REG(udc->otg_base));
    ms_readb(MSB250X_OTG0_INTRUSB_REG(udc->otg_base));
    usbe_intr               = ms_readb(MSB250X_OTG0_INTRUSBE_REG(udc->otg_base));
    pst_intr_usbe->bSuspend = 1;
    ms_writeb(usbe_intr, MSB250X_OTG0_INTRUSBE_REG(udc->otg_base));

    // Flush the next packet to be transmitted/ read from the endpoint 0 FIFO
    ms_writeb(0, MSB250X_OTG0_INDEX_REG(udc->otg_base));
    ms_writeb(0x1, MSB250X_OTG0_CSR0_FLSH_REG(udc->otg_base));
    ms_writeb(0x01, MSB250X_OTG0_INTRTXE_REG(udc->otg_base));
    ms_writeb(0x01, MSB250X_OTG0_INTRRXE_REG(udc->otg_base));
    ms_readb(MSB250X_OTG0_INTRTXE_REG(udc->otg_base));
}

#if 0
static ssize_t test_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    u8 testmode;

    if (!strncmp(buf, "test_j", 6))
    {
        testmode = USB_TEST_J;
    }
    else if (!strncmp(buf, "test_k", 6))
    {
        testmode = USB_TEST_K;
    }
    else if (!strncmp(buf, "test_se0_nak", 12))
    {
        testmode = USB_TEST_SE0_NAK;
    }
    else if (!strncmp(buf, "test_packet", 11))
    {
        testmode = USB_TEST_PACKET;
    }
    else
    {
        printk(KERN_ERR "Unsupported test mode %s\n", buf);
        return n;
    }

    msb250x_ep0_test_mode(testmode);
    return n;
}

static DEVICE_ATTR_WO(test_mode);
#endif

static int msb250x_udc_clk_enable(struct device *dev)
{
    int                 retval = 0;
    struct msb250x_udc *udc    = dev_get_drvdata(dev);

    retval        = devm_clk_bulk_get_all(dev, &udc->clks);
    udc->num_clks = (retval < 0) ? (0) : (retval);

    return clk_bulk_prepare_enable(udc->num_clks, udc->clks);
}

static void msb250x_udc_clk_disable(struct device *dev)
{
    struct msb250x_udc *udc = dev_get_drvdata(dev);

    clk_bulk_disable_unprepare(udc->num_clks, udc->clks);
}

static int msb250x_udc_probe(struct platform_device *pdev)
{
    struct device *       dev  = &pdev->dev;
    struct device_node *  node = dev->of_node;
    struct msb250x_udc *  udc;
    const char *          int_name;
    struct reset_control *reset;
    int                   ret;
    int                   i;
    u32                   ep_maxpkt_limit[16];
    u32                   ep_fifo_size[16];
    const char *          ep_name[16];

    udc = devm_kzalloc(&pdev->dev, sizeof(*udc), GFP_KERNEL);
    if (!udc)
        return -ENOMEM;
    /* zlp */
    udc->zlp_buf = devm_kzalloc(&pdev->dev, 64, GFP_KERNEL);
    if (!udc->zlp_buf)
    {
        return -ENOMEM;
    }

    udc->max_ep_num = device_property_string_array_count(dev, "ep_name");
    if (udc->max_ep_num < 0)
    {
        dev_err(dev, "can not read ep count\n");
        return -EINVAL;
    }

    ret = device_property_read_string_array(dev, "ep_name", ep_name, udc->max_ep_num);
    if (ret < 0)
    {
        dev_err(dev, "can not read ep_maxpkt_limit\n");
        return -EINVAL;
    }

    ret = device_property_read_u32_array(dev, "ep_maxpkt_limit", ep_maxpkt_limit, udc->max_ep_num);
    if (ret < 0)
    {
        dev_err(dev, "can not read ep_maxpkt_limit\n");
        return -EINVAL;
    }
    ret = device_property_read_u32_array(dev, "ep_fifo_size", ep_fifo_size, udc->max_ep_num);
    if (ret < 0)
    {
        dev_err(dev, "can not read ep_fifo_size\n");
        return -EINVAL;
    }
    ret = device_property_read_u32(dev, "dma_channel_num", &udc->dma_chanel_num);
    if (ret < 0)
    {
        dev_err(dev, "can not read dma_channel_num\n");
        return -EINVAL;
    }

    msb250x_udc_init_eps(udc, udc->max_ep_num, ep_maxpkt_limit, ep_fifo_size, ep_name);

    udc->gadget.dev.parent    = dev;
    udc->pdev                 = pdev;
    udc->gadget.ops           = &msb250x_gadget_ops;
    udc->gadget.max_speed     = usb_get_maximum_speed(dev);
    udc->gadget.name          = "msb250x_udc";
    udc->gadget.dev.init_name = "gadget";
    udc->gadget.speed         = USB_SPEED_UNKNOWN;
    udc->using_dma            = (!dma_set_mask(dev, DMA_BIT_MASK(64))) ? 1 : 0;
    platform_set_drvdata(pdev, udc);
    spin_lock_init(&udc->lock);

    /* phy address base */
    i              = of_property_match_string(node, "reg-names", "utmi");
    udc->utmi_base = devm_of_iomap(dev, node, i, NULL);
    if (IS_ERR(udc->utmi_base))
    {
        dev_err(dev, "utmi ioremap failed\n");
        return PTR_ERR(udc->utmi_base);
    }
    /* usb ctrl address base */
    i              = of_property_match_string(node, "reg-names", "usb0");
    udc->usb0_base = devm_of_iomap(dev, node, i, NULL);
    if (IS_ERR(udc->usb0_base))
    {
        dev_err(dev, "usb0 ioremap failed\n");
        return PTR_ERR(udc->usb0_base);
    }
    /* otg address base */
    i             = of_property_match_string(node, "reg-names", "otg");
    udc->otg_base = devm_of_iomap(dev, node, i, NULL);
    if (IS_ERR(udc->otg_base))
    {
        dev_err(dev, "otg ioremap failed\n");
        return PTR_ERR(udc->otg_base);
    }
#ifdef CONFIG_USB_FPGA_VERIFICATION
    /* chiptop address base */
    i                 = of_property_match_string(node, "reg-names", "chiptop");
    udc->chiptop_base = devm_of_iomap(dev, node, i, NULL);
    if (IS_ERR(udc->chiptop_base))
    {
        dev_err(dev, "chiptop ioremap failed\n");
        return PTR_ERR(udc->chiptop_base);
    }
#endif
    msb250x_udc_clk_enable(dev);

    udc->irq = platform_get_irq(pdev, 0);
    if (udc->irq < 0)
        return udc->irq;
    ret = device_property_read_string(dev, "interrupt-names", &int_name);
    if (ret < 0)
        int_name = NULL;
    ret = devm_request_irq(dev, udc->irq, msb250x_udc_isr, 0, int_name ? int_name : pdev->name, udc);
    if (ret < 0)
    {
        dev_err(dev, "request irq(%d) fail\n", udc->irq);
        return ret;
    }

    reset = devm_reset_control_array_get_optional_shared(dev);
    if (IS_ERR(reset))
        return PTR_ERR(reset);

    ret = reset_control_deassert(reset);
    if (ret)
        return ret;

#ifdef CONFIG_SSTAR_MSB250X_SWITCH_PORT1
    ms_writew(ms_readw(GET_REG16_ADDR(UTMI1_BASE_ADDR, 0x01)) | (BIT7), GET_REG16_ADDR(UTMI1_BASE_ADDR, 0x01));
    printk(KERN_INFO "<USB>[UDC] Switch to port 1\n");
#endif

    /* add gadget udc to udc class driver */
    ret = usb_add_gadget_udc(dev, &udc->gadget);
    if (ret)
    {
        dev_err(dev, "<USB>[UDC] Error in probe\n");
        goto assert_reset;
    }

    // device_create_file(&pdev->dev, &dev_attr_test_mode);

    usb_gadget_set_state(&udc->gadget, USB_STATE_POWERED);
    pm_runtime_enable(dev);

    dev_info(dev, "<USB>[UDC] probe completed\n");

    return ret;

assert_reset:
    reset_control_assert(reset);
    msb250x_udc_clk_disable(dev);

    return ret;
}

static int msb250x_udc_remove(struct platform_device *pdev)
{
    struct msb250x_udc *udc = platform_get_drvdata(pdev);

    pm_runtime_get_sync(&pdev->dev);

    // device_remove_file(&pdev->dev, &dev_attr_test_mode);
    usb_del_gadget_udc(&udc->gadget);
    msb250x_udc_deinit_otg(udc);
    msb250x_udc_deinit_utmi(udc);

    msb250x_udc_clk_disable(&pdev->dev);
    pm_runtime_disable(&pdev->dev);
    pm_runtime_put(&pdev->dev);

    platform_set_drvdata(pdev, NULL);

    dev_info(&pdev->dev, "<USB>[UDC] remove\n");

    return 0;
}

static int __maybe_unused msb250x_udc_suspend(struct device *dev)
{
    struct msb250x_udc *udc = dev_get_drvdata(dev);

    udc->gadget.speed = USB_SPEED_UNKNOWN;

    msb250x_udc_deinit_otg(udc);
    msb250x_udc_deinit_usb0(udc);
    msb250x_udc_deinit_utmi(udc);

    msb250x_udc_clk_disable(dev);

    return 0;
}

static int __maybe_unused msb250x_udc_resume(struct device *dev)
{
    struct msb250x_udc *udc = dev_get_drvdata(dev);

    msb250x_udc_clk_enable(dev);
    if (udc->driver)
    {
        // enable udc
        msb250x_udc_init_utmi(udc);
        msb250x_udc_init_usb0(udc);
        msb250x_udc_init_otg(udc);
        udc->gadget.ops->pullup(&udc->gadget, 1);
    }

    return 0;
}

static int __maybe_unused msb250x_udc_runtime_suspend(struct device *dev)
{
    printk(KERN_DEBUG "<USB> %s \n", __FUNCTION__);
    msb250x_udc_clk_disable(dev);
    return 0;
}

static int __maybe_unused msb250x_udc_runtime_resume(struct device *dev)
{
    printk(KERN_DEBUG "<USB> %s \n", __FUNCTION__);
    msb250x_udc_clk_enable(dev);
    return 0;
}

static const struct dev_pm_ops msb250x_udc_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(msb250x_udc_suspend, msb250x_udc_resume)
        SET_RUNTIME_PM_OPS(msb250x_udc_runtime_suspend, msb250x_udc_runtime_resume, NULL)};

static struct of_device_id msb250x_udc_of_device_ids[] = {
    {.compatible = "sstar,msb250x-udc"},
    {},
};

static struct platform_driver msb250x_udc_driver = {
    .driver =
        {
            .name           = "soc:msb250x-udc",
            .of_match_table = msb250x_udc_of_device_ids,
            .pm             = &msb250x_udc_pm_ops,
        },
    .probe  = msb250x_udc_probe,
    .remove = msb250x_udc_remove,
};

module_platform_driver(msb250x_udc_driver);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
