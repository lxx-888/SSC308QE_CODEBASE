/*
 * msb250x_udc.h - Sigmastar
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

#ifndef _MSB250X_UDC_H_
#define _MSB250X_UDC_H_

#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include <msb250x_reg.h>

#define AUTONAK_COUNT 3

enum ep0_state
{
    EP0_IDLE,
    EP0_IN_DATA_PHASE,
    EP0_OUT_DATA_PHASE,
    EP0_END_XFER,
    EP0_STALL,
};

struct msb250x_request
{
    struct usb_request req;
    struct list_head   queue;
} __attribute__((aligned(16)));

struct msb250x_ep
{
    struct usb_ep       ep;
    struct msb250x_udc *dev;

    const struct usb_endpoint_descriptor *desc;
    struct list_head                      queue;

    unsigned halted : 1;
    unsigned shortPkt : 1;
    unsigned ep_num;
    unsigned fifo_size;
    unsigned autoNAK_cfg;
} __attribute__((aligned(16)));

struct msb250x_udc
{
    struct usb_gadget         gadget;
    struct usb_gadget_driver *driver;

    u8 address;

    int                   ep0state;
    enum usb_device_state state;

    struct msb250x_ep ep[MSB250X_MAX_ENDPOINTS];

    unsigned req_pending : 1, req_std : 1, req_config : 1;
    unsigned delay_status : 1;
    int      devstatus;
} __attribute__((aligned(16)));

inline void ep0_idle(struct msb250x_udc *dev)
{
    dev->ep0state = EP0_IDLE;
}

inline void msb250x_ep0_clear_opr(void)
{
    sstar_writeb(MSB250X_OTG0_CSR0_SRXPKTRDY, MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_clear_sst(void)
{
    sstar_writeb(0x00, MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_clear_se(void)
{
    sstar_writeb(MSB250X_OTG0_CSR0_SSETUPEND, MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_set_ipr(void)
{
    sstar_writeb(MSB250X_OTG0_CSR0_TXPKTRDY, MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_set_de(void)
{
    sstar_writeb(MSB250X_OTG0_CSR0_DATAEND, MSB250X_OTG0_EP_TXCSR1_REG(0));
}

static inline void msb250x_ep0_clr_opr_set_ss(void)
{
    sstar_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_SENDSTALL), MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_set_ss(void)
{
    sstar_writeb(MSB250X_OTG0_CSR0_SENDSTALL, MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_set_de_out(void)
{
    sstar_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_DATAEND), MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_set_sse_out(void)
{
    sstar_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_SSETUPEND), MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep0_set_de_in(void)
{
    sstar_writeb((MSB250X_OTG0_CSR0_TXPKTRDY | MSB250X_OTG0_CSR0_DATAEND), MSB250X_OTG0_EP_TXCSR1_REG(0));
}

inline void msb250x_ep_set_ipr(u8 ep_num)
{
    sstar_writeb(sstar_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num)) | MSB250X_OTG0_TXCSR1_TXPKTRDY,
                 MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
}

inline void msb250x_ep_set_opr(u8 ep_num)
{
    sstar_writeb(sstar_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num)) & ~MSB250X_OTG0_RXCSR1_RXPKTRDY,
                 MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
}

inline void set_ep_halt(struct msb250x_ep *ep, int value)
{
    ep->halted = value;
}

#endif /* _MSB250X_UDC_H_ */
