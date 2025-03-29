/*
 * drv_ddrfreq.h - Sigmastar
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

#ifndef __DRV_DDRFREQ_H__
#define __DRV_DDRFREQ_H__

typedef enum
{
    MIU_NOTIFY_DQ_START = 0x0001,
    MIU_NOTIFY_DQ_STOP,
} MIU_NOTIFY_EVENT;

int devfreq_notifier_register_nb(struct notifier_block *nb);
int devfreq_notifier_unregister_nb(struct notifier_block *nb);

#endif // #ifndef __DRV_DDRFREQ_H__