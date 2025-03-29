/*
 * sstar_clk.h- Sigmastar
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

#ifndef __SSTAR_CLK_H__
#define __SSTAR_CLK_H__

#include <linux/clk-provider.h>

typedef struct
{
    const char* name;
    ulong       rate;
} sstar_fixed_rate;

typedef struct
{
    const char*  name;
    const char*  parent;
    unsigned int mult;
    unsigned int div;
} sstar_fixed_factor;

typedef struct
{
    const char*        name;
    const char* const* parents;
    int                num_parents;
    ulong              reg;
    u8                 mux_shift;
    u8                 mux_width;
    u8                 gate_shift;
    unsigned long      flags;
} sstar_composite;

typedef enum
{
    SSTAR_CLK_TYPE_NONE = 0,
    SSTAR_CLK_TYPE_FIXED_RATE,
    SSTAR_CLK_TYPE_FIXED_FACTOR,
    SSTAR_CLK_TYPE_COMPOSITE,
    SSTAR_CLK_TYPE_MAX,
} sstar_clk_type;

typedef struct
{
    sstar_clk_type type;
    ulong          id;
    union
    {
        sstar_fixed_rate   fixed_rate;
        sstar_fixed_factor fixed_factor;
        sstar_composite    composite;
    } clk;
} sstar_clk_data;

#define SSTAR_CLK_FIXED_RATE(_id, _name, _rate)                                                         \
    {                                                                                                   \
        .type = SSTAR_CLK_TYPE_FIXED_RATE, .id = _id, .clk.fixed_rate = {.name = _name, .rate = _rate } \
    }

#define SSTAR_CLK_FIXED_FACTOR(_id, _name, _parent, _mult, _div) \
    {                                                            \
        .type = SSTAR_CLK_TYPE_FIXED_FACTOR, .id = _id,          \
        .clk.fixed_factor =                                      \
        {.name   = _name,                                        \
         .parent = _parent,                                      \
         .mult   = _mult,                                        \
         .div    = _div }                                           \
    }

#define SSTAR_CLK_COMPOSITE(_id, _name, _parents, _num_parents, _reg, _mux_shift, _mux_width, _gate_shift, _flags) \
    {                                                                                                              \
        .type = SSTAR_CLK_TYPE_COMPOSITE, .id = _id,                                                               \
        .clk.composite =                                                                                           \
        {.name        = _name,                                                                                     \
         .parents     = _parents,                                                                                  \
         .num_parents = _num_parents,                                                                              \
         .reg         = _reg,                                                                                      \
         .mux_shift   = _mux_shift,                                                                                \
         .mux_width   = _mux_width,                                                                                \
         .gate_shift  = _gate_shift,                                                                               \
         .flags       = _flags }                                                                                         \
    }

#endif /* __SSTAR_CLK_H__ */
