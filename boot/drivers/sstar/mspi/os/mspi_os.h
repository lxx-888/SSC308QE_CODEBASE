/*
 * mspi_os.h- Sigmastar
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

#ifndef __MSPI_OS__
#define __MSPI_OS__

#include <linux/io.h>
#include <common.h>
#include <platform.h>
#include <linux/delay.h>
#include <cpu_func.h>

#if !defined(BIT0) && !defined(BIT1)
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#endif

#define HAL_MSPI_DEBUG 0
#if HAL_MSPI_DEBUG == 1
#define hal_mspi_dbg(fmt, arg...) printf(KERN_INFO fmt, ##arg)
#else
#define hal_mspi_dbg(fmt, arg...)
#endif
#define hal_mspi_err(fmt, arg...) printf(KERN_INFO fmt, ##arg)

#define hal_mspi_delay_us(__us) \
    do                          \
    {                           \
        __udelay(__us);         \
    } while (0)

#define hal_mspi_virt_to_phys(_virt_) virt_to_phys(_virt_)
#define hal_mspi_phys_to_miu(_phys_)  (u32)(_phys_ - MIU0_START_ADDR)

#define MSPI_ALIGN_CACHELINE_LESS(_addr_) (_addr_ & ~(CONFIG_SYS_CACHELINE_SIZE - 1))
#define MSPI_ALIGN_CACHELINE_MORE(_addr_)       \
    ((_addr_ & (CONFIG_SYS_CACHELINE_SIZE - 1)) \
         ? _addr_                               \
         : ((_addr_ + CONFIG_SYS_CACHELINE_SIZE) & ~(CONFIG_SYS_CACHELINE_SIZE - 1)))

#define hal_mspi_cache_flush(_ptr_, _size_)                                                                          \
    do                                                                                                               \
    {                                                                                                                \
        flush_dcache_range(MSPI_ALIGN_CACHELINE_LESS((u32)_ptr_), MSPI_ALIGN_CACHELINE_MORE((u32)(_ptr_ + _size_))); \
    } while (0)

#define hal_mspi_cache_invalidate(_ptr_, _size_)                                   \
    do                                                                             \
    {                                                                              \
        invalidate_dcache_range(MSPI_ALIGN_CACHELINE_LESS((u32)_ptr_),             \
                                MSPI_ALIGN_CACHELINE_MORE((u32)(_ptr_ + _size_))); \
    } while (0)

#endif
