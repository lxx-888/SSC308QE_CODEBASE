/*
 * ehci-sstar.h - Sigmastar
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

#ifndef __UBOOTGLUE_USB_H__
#define __UBOOTGLUE_USB_H__

#include <asm/types.h>
#include <usb.h>

#define EHCFLAG_NONE      0x0
#define EHCFLAG_DPDM_SWAP 0x1
#define EHCFLAG_XHC_COMP  0x2

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS        0x0080
#define RH_CLEAR_FEATURE     0x0100
#define RH_SET_FEATURE       0x0300
#define RH_SET_ADDRESS       0x0500
#define RH_GET_DESCRIPTOR    0x0680
#define RH_SET_DESCRIPTOR    0x0700
#define RH_GET_CONFIGURATION 0x0880
#define RH_SET_CONFIGURATION 0x0900
#define RH_GET_STATE         0x0280
#define RH_GET_INTERFACE     0x0A80
#define RH_SET_INTERFACE     0x0B00
#define RH_SYNC_FRAME        0x0C80

struct xhc_comp
{
    u8        port_index;
    uintptr_t xhci_port_addr;
    uintptr_t u3top_base;
    uintptr_t u3phy_A_base;
    uintptr_t u3phy_D_base;
    uintptr_t xhci_base;
};

struct usb_hcd
{
    u32       port_index;
    uintptr_t utmi_base;
    uintptr_t ehc_base;
    uintptr_t usbc_base;
    uintptr_t bc_base;
    uintptr_t init_flag;

    int ever_inited;

    struct xhc_comp xhci;

    // struct usb_port_status rh_status;
    struct ehci_hcd *ehci;
    u8               bSpeed;
    u32              urb_status;
    u32              total_bytes;
    int              FirstBulkOut;
    int              FirstBulkIn;
    int              FirstIntIn;
    int              IntrIn_Complete;
};

struct ehci_phy_priv_data
{
    const char *label;
    u32         has_utmi2_bank : 1;
    u32         has_phy_trim_val : 1;
    u32         has_dm_dp_swap : 1;
};

#endif /* __UBOOTGLUE_USB_H__ */
