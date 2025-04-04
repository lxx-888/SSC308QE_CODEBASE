/*
 * cam_os_util_bitmap.h- Sigmastar
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

#ifndef __CAM_OS_UTIL_BITMAP_H__
#define __CAM_OS_UTIL_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

///////////////////////////////////////////////////////////////////////////////////////

#define CAM_OS_BIT_MASK(nr)       (1UL << ((nr) % CAM_OS_BITS_PER_LONG))
#define CAM_OS_BIT_WORD(nr)       ((nr) / CAM_OS_BITS_PER_LONG)
#define CAM_OS_DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define CAM_OS_BITS_TO_LONGS(nr)  CAM_OS_DIV_ROUND_UP(nr, CAM_OS_BITS_PER_LONG)

#define CAM_OS_DECLARE_BITMAP(name, bits) unsigned long name[CAM_OS_BITS_TO_LONGS(bits)]

#define CAM_OS_BITMAP_CLEAR(name)             \
    do                                        \
    {                                         \
        CamOsMemset((name), 0, sizeof(name)); \
    } while (0)

unsigned long _CamOsFindFirstZeroBit(unsigned long *pAddr, unsigned long nSize, unsigned long nOffset);
#define CAM_OS_FIND_FIRST_ZERO_BIT(p, sz)    _CamOsFindFirstZeroBit(p, sz, 0)
#define CAM_OS_FIND_NEXT_ZERO_BIT(p, sz, of) _CamOsFindFirstZeroBit(p, sz, of)

static inline int CAM_OS_FFS(unsigned long x)
{
    return CAM_OS_FLS(x & -x);
}

static inline unsigned long _CAM_OS_FFS(unsigned long x)
{
    return CAM_OS_FFS(x) - 1;
}

#define CAM_OS_FFZ(x) _CAM_OS_FFS(~(x))

///////////////////////////////////////////////////////////////////////////////////////

static inline void CAM_OS_SET_BIT(s32 nr, volatile unsigned long *addr)
{
    unsigned long           ulMask = CAM_OS_BIT_MASK(nr);
    volatile unsigned long *p      = (addr) + CAM_OS_BIT_WORD(nr);

    __sync_fetch_and_or(p, ulMask);
}

static inline void CAM_OS_CLEAR_BIT(s32 nr, volatile unsigned long *addr)
{
    unsigned long           ulMask = CAM_OS_BIT_MASK(nr);
    volatile unsigned long *p      = (addr) + CAM_OS_BIT_WORD(nr);

    ulMask = ~ulMask;
    __sync_fetch_and_and(p, ulMask);
}

static inline void CAM_OS_CHANGE_BIT(s32 nr, volatile unsigned long *addr)
{
    unsigned long           ulMask = CAM_OS_BIT_MASK(nr);
    volatile unsigned long *p      = (addr) + CAM_OS_BIT_WORD(nr);

    __sync_fetch_and_xor(p, ulMask);
}

static inline s32 CAM_OS_TEST_AND_SET_BIT(s32 nr, volatile unsigned long *addr)
{
    unsigned long           ulMask = CAM_OS_BIT_MASK(nr);
    volatile unsigned long *p      = (addr) + CAM_OS_BIT_WORD(nr);
    volatile unsigned long  old    = 0;

    old = __sync_fetch_and_or(p, ulMask);
    return (old & ulMask) != 0;
}

static inline s32 CAM_OS_TEST_AND_CLEAR_BIT(s32 nr, volatile unsigned long *addr)
{
    unsigned long           ulMask        = CAM_OS_BIT_MASK(nr);
    unsigned long           ulMaskInverse = ~ulMask;
    volatile unsigned long *p             = (addr) + CAM_OS_BIT_WORD(nr);
    volatile unsigned long  old           = 0;

    old = __sync_fetch_and_and(p, ulMaskInverse);
    return (old & ulMask) != 0;
}

static inline s32 CAM_OS_TEST_AND_CHANGE_BIT(s32 nr, volatile unsigned long *addr)
{
    unsigned long           ulMask = CAM_OS_BIT_MASK(nr);
    volatile unsigned long *p      = (addr) + CAM_OS_BIT_WORD(nr);
    volatile unsigned long  old    = 0;

    old = __sync_fetch_and_xor(p, ulMask);
    return (old & ulMask) != 0;
}

static inline s32 CAM_OS_TEST_BIT(s32 nr, const volatile unsigned long *addr)
{
    return 1UL & (addr[CAM_OS_BIT_WORD(nr)] >> (nr & (CAM_OS_BITS_PER_LONG - 1)));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_UTIL_BITMAP_H__
