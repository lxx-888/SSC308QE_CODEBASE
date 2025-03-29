/*
 * drv_riu_dbg.h- Sigmastar
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

#ifndef _DRV_RIU_DBG_H_
#define _DRV_RIU_DBG_H_

struct riu_irq_info
{
    u8 bridge;
    u8 capture;
};

#define RIU_GET_IRQ_INFO _IOW('R', 0, struct riu_irq_info)

#endif
