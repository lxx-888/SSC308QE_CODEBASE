/*
 * sstar_usb2_phy_debugfs.h - Sigmastar
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

#include "sstar_usb2_phy.h"

#ifndef __PHY_SSTAR_U2PHY_DEBUGFS_H__
void sstar_u2phy_utmi_atop_v1_set(struct phy *phy);
void sstar_u2phy_utmi_atop_set(struct phy *phy);
int  sstar_u2phy_edswitch_creat(struct device_node *node, struct phy *phy);

#ifdef CONFIG_DEBUG_FS
void sstar_u2phy_utmi_debugfs_init(struct sstar_u2phy *priv);
void sstar_u2phy_utmi_debugfs_exit(void *data);
#else
static inline void sstar_u2phy_utmi_debugfs_init(struct sstar_u2phy *priv) {}
static inline void sstar_u2phy_utmi_debugfs_exit(void *data) {}
#endif
#endif
