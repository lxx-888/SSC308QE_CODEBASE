/*
 * iic_os.h- Sigmastar
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

#ifndef __IIC_OS__
#define __IIC_OS__

#include <io.h>
#include <stdlib.h>
#include <cpu_func.h>
#include <sstar_types.h>
#include <linux/delay.h>

#ifdef CONFIG_ARM64
#define HAL_I2C_READ_WORD(_reg)        (*(volatile u16 *)(u64)(_reg))
#define HAL_I2C_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u64)(_reg)) = (u16)(_val))
#define HAL_I2C_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u64)(_reg)) = ((*(volatile u16 *)(u64)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))

#define HAL_I2C_WRITE_BYTE(_reg, _val) ((*(volatile unsigned char *)((u64)_reg)) = (u8)(_val))
#define HAL_I2C_READ_BYTE(_reg)        (*(volatile unsigned char *)((u64)_reg))
#else
#define HAL_I2C_READ_WORD(_reg)        (*(volatile u16 *)(u32)(_reg))
#define HAL_I2C_WRITE_WORD(_reg, _val) ((*(volatile u16 *)(u32)(_reg)) = (u16)(_val))
#define HAL_I2C_WRITE_WORD_MASK(_reg, _val, _mask) \
    ((*(volatile u16 *)(u32)(_reg)) = ((*(volatile u16 *)(u32)(_reg)) & ~(_mask)) | ((u16)(_val) & (_mask)))

#define HAL_I2C_WRITE_BYTE(_reg, _val) ((*(volatile unsigned char *)((u32)_reg)) = (u8)(_val))
#define HAL_I2C_READ_BYTE(_reg)        (*(volatile unsigned char *)((u32)_reg))
#endif

#define CONFIG_SSTAR_PM_DMA          1
#define HAL_I2C_PM_RAM_ACCESS_VAL    0x1
#define HAL_I2C_PM_RAM_ACCESS_MASK   0x3
#define HAL_I2C_PM_RAM_ACCESS_BANK   0x1E
#define HAL_I2C_PM_RAM_ACCESS_OFFSET 0x5E
#define BASE_REG_RIU_PA              0x1F000000

#define I2C_DELAY_N_US(_x) \
    do                     \
    {                      \
        udelay(_x);        \
    } while (0)

#define dmsg_i2c_halerr(fmt, ...)           \
    do                                      \
    {                                       \
        printf("err: " fmt, ##__VA_ARGS__); \
    } while (0)

//#define HAL_I2C_DMSG_ENABLE
#ifdef HAL_I2C_DMSG_ENABLE
#define dmsg_i2c_halwarn(fmt, ...)           \
    do                                       \
    {                                        \
        printf("debug " fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define dmsg_i2c_halwarn(fmt, ...)
#endif

#define HAL_I2C_CACHE_START(_addr_) (_addr_ & ~(CONFIG_SYS_CACHELINE_SIZE - 1))
#define HAL_I2C_CACHE_STOP(_addr_)                                                   \
    ((_addr_ & (CONFIG_SYS_CACHELINE_SIZE - 1))                                      \
         ? ((_addr_ + CONFIG_SYS_CACHELINE_SIZE) & ~(CONFIG_SYS_CACHELINE_SIZE - 1)) \
         : _addr_)

#define hal_i2c_cache_flush(_ptr_, _size_)                                                              \
    do                                                                                                  \
    {                                                                                                   \
        flush_dcache_range(HAL_I2C_CACHE_START((u32)_ptr_), HAL_I2C_CACHE_STOP((u32)(_ptr_ + _size_))); \
    } while (0)

#define hal_i2c_cache_invalidate(_ptr_, _size_)                                                              \
    do                                                                                                       \
    {                                                                                                        \
        invalidate_dcache_range(HAL_I2C_CACHE_START((u32)_ptr_), HAL_I2C_CACHE_STOP((u32)(_ptr_ + _size_))); \
    } while (0)

#endif
