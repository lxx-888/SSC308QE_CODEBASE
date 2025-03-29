/*
 * sstar_usb3_phy_u3phy.h- Sigmastar
 *
 * Copyright (c) [2019~2021] SigmaStar Technology.
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

#ifndef __SSTAR_USB3_PHY_U3PHY_H__
#define __SSTAR_USB3_PHY_U3PHY_H__

#include <linux/usb/phy.h>
#include <linux/phy/phy.h>
#include <uapi/linux/usb/ch9.h>
#include <linux/usb/otg.h>

#define U3PHY_PORT_NUM 2

typedef enum
{
    EYE_DIAGRAM_MODE_HIGH_SPEED,
    EYE_DIAGRAM_MODE_FULL_SPEED,
    EYE_DIAGRAM_MODE_UNKNOWN
} EYE_DIAGRAM_MODE_E;

struct phy_data
{
    const char* label;
    /*Later chips default revision 2, legacy chips are revision 1.*/
    u32 revision;
};

struct sstar_phy_port
{
    struct phy*              phy;
    struct device*           dev;
    spinlock_t               lock;
    int                      num_clocks;
    struct clk_bulk_data*    clks;
    unsigned int             index;
    unsigned char            type;
    bool                     suspended;
    struct dentry*           root;
    struct debugfs_regset32* regset;
    void __iomem*            reg;
    enum usb_device_speed    speed;
    struct phy_data*         phy_data;

    bool          has_ed_switch;
    bool          ed_hs_switch_on;
    bool          ed_fs_switch_on;
    void __iomem* ed_bank1;
    void __iomem* ed_bank2;
    void __iomem* ed_bank3;
    void __iomem* ed_bank4;
};

struct sstar_u3phy
{
    struct device*        dev;
    struct sstar_phy_port u3phy_port[U3PHY_PORT_NUM];
    struct usb_phy        usb_phy;
    struct notifier_block usb3_typec_nb;
};

#endif
