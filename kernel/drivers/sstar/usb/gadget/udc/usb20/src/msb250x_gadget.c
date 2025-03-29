/*
 * msb250x_gadget.c- Sigmastar
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

#include <linux/dma-mapping.h>
#include "msb250x_udc_reg.h"
#include "msb250x_udc.h"
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#ifdef CONFIG_USB_MANUAL_MATCH_EP
unsigned int epnum;
unsigned int ep_number[6] = {1, 2, 1, 2, 3, 6};
module_param_array(ep_number, uint, &epnum, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ep_number, "1-6");
#endif

static bool use_performance_match_ep = false;
module_param(use_performance_match_ep, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(use_performance_match_ep, "Using performance matching endpoint patterns, default=0");

typedef unsigned long long ss_phys_addr_t;
typedef unsigned long long ss_miu_addr_t;
extern ss_miu_addr_t       Chip_Phys_to_MIU(ss_phys_addr_t phys);
extern ss_phys_addr_t      Chip_MIU_to_Phys(ss_miu_addr_t miu);

int msb250x_gadget_map_request(struct usb_gadget *gadget, struct usb_request *req, int is_in)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);
    struct device *     dev = &udc->pdev->dev;
    int                 ret;

    ret = usb_gadget_map_request(gadget, req, is_in);
    if (ret)
    {
        dev_err(dev, "usb gadget map request fail, ret:%d\n", ret);
        return ret;
    }

    req->dma = Chip_Phys_to_MIU(req->dma);

    return 0;
}

void msb250x_gadget_unmap_request(struct usb_gadget *gadget, struct usb_request *req, int is_in)
{
    req->dma = Chip_MIU_to_Phys(req->dma);
    usb_gadget_unmap_request(gadget, req, is_in);
}

void msb250x_gadget_pullup_i(struct msb250x_udc *udc, int is_on)
{
    u8                     power     = 0;
    struct otg0_usb_power *pst_power = (struct otg0_usb_power *)&power;
    struct device *        dev       = &udc->pdev->dev;

    if (is_on == 0)
    {
        msb250x_udc_deinit_utmi(udc);
        msb250x_udc_deinit_otg(udc);
    }
    else
    {
        msb250x_udc_init_utmi(udc);
        msb250x_udc_init_usb0(udc);
        mdelay(1);
        msb250x_udc_init_otg(udc);
    }
    power                = ms_readb(MSB250X_OTG0_PWR_REG(udc->otg_base));
    pst_power->bSoftConn = is_on;
    ms_writeb(power, MSB250X_OTG0_PWR_REG(udc->otg_base));

    dev_info(dev, "<USB>[GADGET] PULL_%s\n", (0 == is_on) ? "DOWN" : "UP");
}

int msb250x_gadget_get_frame(struct usb_gadget *gadget)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);

    return ms_readw(MSB250X_OTG0_FRAME_L_REG(udc->otg_base));
}

struct usb_ep *msb250x_gadget_match_ep(struct usb_gadget *gadget, struct usb_endpoint_descriptor *desc,
                                       struct usb_ss_ep_comp_descriptor *ep_comp)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);

#ifdef CONFIG_USB_MANUAL_MATCH_EP
    unsigned int ep_num;

    switch (usb_endpoint_type(desc))
    {
        case USB_ENDPOINT_XFER_ISOC:
            if (usb_endpoint_dir_in(desc))
            {
                ep_num = ep_number[0];
                printk("<USB>[GADGET] match isoc in ep\n");
                break;
            }
            else
            {
                ep_num = ep_number[1];
                printk("<USB>[GADGET] match isoc out ep\n");
                break;
            }
        case USB_ENDPOINT_XFER_BULK:
            if (usb_endpoint_dir_in(desc))
            {
                ep_num = ep_number[2];
                printk("<USB>[GADGET] match bulk in ep\n");
                break;
            }
            else
            {
                ep_num = ep_number[3];
                printk("<USB>[GADGET] match bulk out ep\n");
                break;
            }

        case USB_ENDPOINT_XFER_INT:
            if (usb_endpoint_dir_in(desc))
            {
                ep_num = ep_number[4];
                printk("<USB>[GADGET] match intr in ep\n");
                break;
            }
            else
            {
                ep_num = ep_number[5];
                printk("<USB>[GADGET] match intr out ep\n");
                break;
            }
        default:
            /* nothing */;
    }

    return &udc->mep[ep_num].ep;
#else
    int            maxpacket = usb_endpoint_maxp(desc);
    int            mult      = usb_endpoint_maxp_mult(desc);
    struct device *dev       = &udc->pdev->dev;
    int            index, select;
    struct usb_ep *ep      = NULL;
    int            matched = 0;

    /* if not specify maxpkt, default */
    if (!maxpacket)
    {
        switch (usb_endpoint_type(desc))
        {
            case USB_ENDPOINT_XFER_ISOC:
                maxpacket = 1024;
                break;
            case USB_ENDPOINT_XFER_BULK:
                maxpacket = 512;
                break;
            default:
                maxpacket = 64;
                break;
        }
    }
    /* try to match a ep that support double buffer */
    if (use_performance_match_ep
        && ((usb_endpoint_type(desc) == USB_ENDPOINT_XFER_ISOC) || (usb_endpoint_type(desc) == USB_ENDPOINT_XFER_BULK)))
    {
        for (index = 1, select = 1; index < udc->max_ep_num; index++)
        {
            if ((maxpacket > udc->mep[index].ep.maxpacket_limit) || (maxpacket * mult * 2 > udc->mep[index].fifo_size)
                || udc->mep[index].ep.claimed)
                continue;
            if ((udc->mep[select].ep.maxpacket_limit >= udc->mep[index].ep.maxpacket_limit)
                && (udc->mep[select].fifo_size >= udc->mep[index].fifo_size))
            {
                select  = index;
                matched = 1;
            }
        }
    }
    /* If no ep that supports doule buffer has been matched, match an ep normally. */
    if (!matched)
    {
        for (index = 1, select = 1; index < udc->max_ep_num; index++)
        {
            if ((maxpacket > udc->mep[index].ep.maxpacket_limit) || (maxpacket * mult > udc->mep[index].fifo_size)
                || udc->mep[index].ep.claimed)
                continue;
            if (udc->mep[select].ep.maxpacket_limit >= udc->mep[index].ep.maxpacket_limit)
            {
                select  = index;
                matched = 1;
            }
        }
    }

    if (matched)
    {
        ep = &udc->mep[select].ep;
        dev_dbg(dev, "<USB>[GADGET] match ep%d(mult:%d, maxpkt:%d)\n", select, mult, maxpacket);
    }
    else
    {
        dev_err(dev, "<USB>[GADGET] description can't match ep(mult:%d, maxpkt:%d)\n", mult, maxpacket);
    }
    return ep;
#endif
}

int msb250x_gadget_wakeup(struct usb_gadget *gadget)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);
    struct device *     dev = &udc->pdev->dev;
    u8                  power;

    if (udc->devstatus & (1 << USB_DEVICE_REMOTE_WAKEUP))
    {
        dev_dbg(dev, "triger remote wake up siginal\n");
        power = ms_readb(MSB250X_OTG0_PWR_REG(udc->otg_base));
        power |= MSB250X_OTG0_PWR_RESUME;
        ms_writeb(power, MSB250X_OTG0_PWR_REG(udc->otg_base));
        mdelay(10);
        power = ms_readb(MSB250X_OTG0_PWR_REG(udc->otg_base));
        power &= ~MSB250X_OTG0_PWR_RESUME;
        ms_writeb(power, MSB250X_OTG0_PWR_REG(udc->otg_base));
    }

    return 0;
}

int msb250x_gadget_set_selfpowered(struct usb_gadget *gadget, int is_on)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);

    gadget->is_selfpowered = (is_on != 0);

    if (is_on)
        udc->devstatus |= (1 << USB_DEVICE_SELF_POWERED);
    else
        udc->devstatus &= ~(1 << USB_DEVICE_SELF_POWERED);

    return 0;
}

int msb250x_gadget_pullup(struct usb_gadget *gadget, int is_on)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);
    // struct device *     dev = &udc->pdev->dev;
    u8                     power     = ms_readb(MSB250X_OTG0_PWR_REG(udc->otg_base));
    struct otg0_usb_power *pst_power = (struct otg0_usb_power *)&power;

    if (pst_power->bSoftConn != is_on)
    {
        msb250x_gadget_pullup_i(udc, is_on);
    }

    udc->soft_conn = is_on;

    return 0;
}

int msb250x_gadget_vbus_session(struct usb_gadget *gadget, int is_active)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);
    struct device *     dev = &udc->pdev->dev;

    dev_dbg(dev, "Entered %s\n", __FUNCTION__);

    return 0;
}

int msb250x_gadget_vbus_draw(struct usb_gadget *gadget, unsigned ma)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);
    struct device *     dev = &udc->pdev->dev;

    dev_dbg(dev, "Entered %s\n", __FUNCTION__);

    return 0;
}

int msb250x_gadget_udc_start(struct usb_gadget *gadget, struct usb_gadget_driver *driver)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);
    struct device *     dev = &udc->pdev->dev;

    udc->driver            = driver;
    udc->gadget.dev.driver = &driver->driver;

    pm_runtime_get_sync(dev);

    mdelay(1);

    dev_dbg(dev, "<USB>[UDC] Start\n");

    return 0;
}

int msb250x_gadget_udc_stop(struct usb_gadget *gadget)
{
    struct msb250x_udc *udc = to_msb250x_udc(gadget);
    struct device *     dev = &udc->pdev->dev;
    int                 i;

    for (i = 0; i < udc->max_ep_num; i++)
    {
        udc->mep[i].ep.desc = NULL;
    }
    udc->driver            = NULL;
    udc->gadget.dev.driver = NULL;
    udc->gadget.speed      = USB_SPEED_UNKNOWN;
    pm_runtime_put(dev);

    dev_dbg(dev, "<USB>[UDC] Stop\n");

    return 0;
}
