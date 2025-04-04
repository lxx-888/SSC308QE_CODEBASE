/*
 * mdrv_mma_heap.h- Sigmastar
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
#include "cam_os_wrapper.h"
#define MMA_HEAP_NAME_LENG 32

// enable max mma areas be a large value .
#define MAX_MMA_AREAS 30
#ifdef CONFIG_PHYS_ADDR_T_64BIT
typedef u64 phys_addr_t;
#else
typedef u32 phys_addr_t;
#endif
struct MMA_BootArgs_Config
{
    int           miu;                      // input :from bootargs or dts
    unsigned long size;                     // input :from bootargs or dts
    char          name[MMA_HEAP_NAME_LENG]; // input :from bootargs or dts
    phys_addr_t   max_start_offset_to_curr_bus_base;
    phys_addr_t   max_end_offset_to_curr_bus_base; // input:for vdec use.

    phys_addr_t reserved_start; // out: reserved_start
};
