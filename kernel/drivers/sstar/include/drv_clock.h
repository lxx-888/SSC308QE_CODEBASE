/*
 * drv_clock.h- Sigmastar
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
#ifndef __DRV_CLOCK_H__
#define __DRV_CLOCK_H__

#include "cam_os_wrapper.h"

void *sstar_clk_get(void *dev, u16 index);
int   sstar_clk_prepare(void *clk_handler);
int   sstar_clk_enable(void *clk_handler);
int   sstar_clk_prepare_enable(void *clk_handler);
int   sstar_clk_is_enabled(void *clk_handler);
int   sstar_clk_set_rate(void *clk_handler, unsigned long rate);
u32   sstar_clk_get_rate(void *clk_handler);
u32   sstar_clk_round_rate(void *clk_handler, unsigned long rate);
int   sstar_clk_set_parent(void *clk_handler, void *clk_handler_parent);
void *sstar_clk_get_parent(void *clk_handler);
u32   sstar_clk_get_num_parents(void *clk_handler);
void *sstar_clk_get_parent_by_index(void *clk_handler, u16 index);
void  sstar_clk_unprepare(void *clk_handler);
void  sstar_clk_disable(void *clk_handler);
void  sstar_clk_disable_unprepare(void *clk_handler);
void  sstar_clk_put(void *clk_handler);
u32   sstar_clk_get_num_in_dev_node(void *dev);
int   sstar_clk_get_od_shift(void *clk_handler);
int   sstar_clk_get_dfs_config(void *clk_handler, u8 *numerator, u8 *denominator);
int   sstar_clk_set_max_rate(void *clk_handler, unsigned long rate);
int   sstar_clk_clr_dfs(void *clk_handler);

#endif
