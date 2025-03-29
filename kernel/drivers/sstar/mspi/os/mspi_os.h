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

#include <ms_platform.h>
#include <cam_os_wrapper.h>
#include <mdrv_types.h>

#define HAL_MSPI_DEBUG 0
#if HAL_MSPI_DEBUG == 1
#define hal_mspi_dbg(fmt, arg...) CamOsPrintf(KERN_INFO fmt, ##arg)
#else
#define hal_mspi_dbg(fmt, arg...)
#endif
#define hal_mspi_err(fmt, arg...) CamOsPrintf(KERN_INFO fmt, ##arg)

#define hal_mspi_delay_us(__us) \
    do                          \
    {                           \
        CamOsUsDelay(__us);     \
    } while (0)

#define hal_mspi_virt_to_phys(_virt_) (u64) CamOsMemVirtToPhys(_virt_)
#define hal_mspi_phys_to_miu(_phys_)  (u32) CamOsMemPhysToMiu(_phys_)

#define hal_mspi_cache_line_size_align(_size_) ((_size_ + cache_line_size() - 1) & (~(cache_line_size() - 1)))
#define hal_mspi_cache_flush(_ptr_, _size_)    CamOsMemFlush((void *)_ptr_, (u32)hal_mspi_cache_line_size_align(_size_))
#define hal_mspi_cache_invalidate(_ptr_, _size_) \
    CamOsMemInvalidate((void *)_ptr_, (u32)hal_mspi_cache_line_size_align(_size_))

#endif
