/*
 * voltage_request_init.h- Sigmastar
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
#ifndef __VOLTAGE_REQUEST_INIT_H
#define __VOLTAGE_REQUEST_INIT_H

enum idac_component
{
    IDAC_CP_VOLTAGE_STEP,
    IDAC_CP_STEP_CURRENT,
    IDAC_CP_SINK_SOURCE,
};

int idac_set_voltage(const char *name, int base_vol, int setvol);
int idac_get_voltage(const char *name, u32 base_vol, u32 *curvol);
int idac_set_gpio_analog_mode(int gpio);
int voltage_request_chip(const char *name);
int idac_get_max_vol_offset(const char *name);
int idac_init_one(const char *name);

#endif //__VOLTAGE_REQUEST_INIT_H
