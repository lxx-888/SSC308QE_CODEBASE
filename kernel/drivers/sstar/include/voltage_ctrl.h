/*
 * voltage_ctrl.h- Sigmastar
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
#ifndef __VOLTAGE_CTRL_H
#define __VOLTAGE_CTRL_H

#include "voltage_ctrl_demander.h"

#ifdef CONFIG_SS_DUALOS
#define INTEROS_SC_L2R_CORE_VOLTAGE_SET 0xF1020000
#define INTEROS_SC_L2R_CORE_VOLTAGE_GET 0xF1020001
#endif

#define VOLTAGE_CORE_850  850
#define VOLTAGE_CORE_900  900
#define VOLTAGE_CORE_950  950
#define VOLTAGE_CORE_1000 1000

int                  set_core_voltage(const char *name, VOLTAGE_DEMANDER_E demander, u32 mV);
int                  get_core_voltage(const char *name, u32 *mV);
int                  get_core_lt_voltage(const char *name, int *mV);
struct voltage_ctrl *get_voltage_ctrl(const char *name);

#endif //__VOLTAGE_CTRL_H
