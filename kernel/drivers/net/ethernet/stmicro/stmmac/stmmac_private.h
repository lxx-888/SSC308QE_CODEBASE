/*
 * stmmac_private.h- Sigmastar
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

#ifndef __STMMAC_PRIVATE_H__
#define __STMMAC_PRIVATE_H__

#if defined(CONFIG_ARCH_SSTAR)
#include "sstar_gmac_common.h"

#undef readl
#undef writel

extern stmmac_callback_handle_t g_callbackHander;

#define readl  (g_callbackHander.private_read)
#define writel (g_callbackHander.private_write)
#endif /* CONFIG_ARCH_SSTAR */

#endif /* __STMMAC_PRIVATE_H__ */
