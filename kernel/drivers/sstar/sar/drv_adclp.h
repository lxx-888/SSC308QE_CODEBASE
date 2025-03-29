/*
 * drv_adclp.h- Sigmastar
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
#ifndef __DRV_ADCLP_H__
#define __DRV_ADCLP_H__

struct adclp_bound
{
    unsigned short upper_bound;
    unsigned short lower_bound;
};

#define ADCLP_IOC_MAXNR 1

#define ADCLP_IOC_MAGIC        'a'
#define IOCTL_ADCLP_SET_BOUND  _IO(ADCLP_IOC_MAGIC, 0)
#define IOCTL_ADCLP_READ_VALUE _IO(ADCLP_IOC_MAGIC, 1)

#if defined(__KERNEL__)
typedef int (*adclp_cb_t)(u8 channel);

int sstar_adclp_enable(u8 channel, u8 enable);
int sstar_adclp_get_data(u8 channel, u16 *data);
int sstar_adclp_set_bound(u8 channel, u16 max, u16 min);
int sstar_adclp_register_callback(u8 channel, adclp_cb_t cb_t);
int sstar_adclp_unregister_callback(u8 channel, adclp_cb_t cb_t);
#endif

#endif
