/*
 * cam_os_util_hash.h- Sigmastar
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

#ifndef __CAM_OS_UTIL_HASH_H__
#define __CAM_OS_UTIL_HASH_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline __attribute__((const)) s32 _CAM_OS_ILOG2_U32(u32 n)
{
    return CAM_OS_FLS(n) - 1;
}

static inline __attribute__((const)) s32 _CAM_OS_ILOG2_U64(u64 n)
{
    return CAM_OS_FLS64(n) - 1;
}

#define _CAM_OS_ILOG2_1(n)  (((n) >= 2) ? 1 : 0)
#define _CAM_OS_ILOG2_2(n)  (((n) >= 1ULL << 2) ? (2 + _CAM_OS_ILOG2_1((n) >> 2)) : _CAM_OS_ILOG2_1(n))
#define _CAM_OS_ILOG2_4(n)  (((n) >= 1ULL << 4) ? (4 + _CAM_OS_ILOG2_2((n) >> 4)) : _CAM_OS_ILOG2_2(n))
#define _CAM_OS_ILOG2_8(n)  (((n) >= 1ULL << 8) ? (8 + _CAM_OS_ILOG2_4((n) >> 8)) : _CAM_OS_ILOG2_4(n))
#define _CAM_OS_ILOG2_16(n) (((n) >= 1ULL << 16) ? (16 + _CAM_OS_ILOG2_8((n) >> 16)) : _CAM_OS_ILOG2_8(n))
#define _CAM_OS_ILOG_N(n)   (((n) >= 1ULL << 32) ? (32 + _CAM_OS_ILOG2_16((n) >> 32)) : _CAM_OS_ILOG2_16(n))

#define CAM_OS_ILOG2(n) \
    (__builtin_constant_p(n) ? _CAM_OS_ILOG_N(n) : (sizeof(n) <= 4) ? _CAM_OS_ILOG2_U32(n) : _CAM_OS_ILOG2_U64(n))

#define CAM_OS_HASH_SIZE(name) (CAM_OS_ARRAY_SIZE(name))

#define CAM_OS_HASH_BITS(name) CAM_OS_ILOG2(CAM_OS_HASH_SIZE(name))

#define CAM_OS_DEFINE_HASHTABLE(name, bits) \
    struct CamOsHListHead_t name[1 << (bits)] = {[0 ...((1 << (bits)) - 1)] = CAM_OS_HLIST_HEAD_INIT}

static inline void _CAM_OS_HASH_INIT(struct CamOsHListHead_t *ht, unsigned int sz)
{
    u32 i;

    for (i = 0; i < sz; i++)
        CAM_OS_INIT_HLIST_HEAD(&ht[i]);
}

#define CAM_OS_HASH_INIT(hashtable) _CAM_OS_HASH_INIT(hashtable, CAM_OS_HASH_SIZE(hashtable))

#define CAM_OS_HASH_ADD(hashtable, node, key) \
    CAM_OS_HLIST_ADD_HEAD(node, &hashtable[CAM_OS_HASH_MIN(key, CAM_OS_HASH_BITS(hashtable))])

#define CAM_OS_HASH_FOR_EACH(name, bkt, obj, member)                                    \
    for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < CAM_OS_HASH_SIZE(name); (bkt)++) \
    CAM_OS_HLIST_FOR_EACH_ENTRY(obj, &name[bkt], member)

#define CAM_OS_HASH_FOR_EACH_POSSIBLE(name, obj, member, key) \
    CAM_OS_HLIST_FOR_EACH_ENTRY(obj, &name[CAM_OS_HASH_MIN(key, CAM_OS_HASH_BITS(name))], member)

#define CAM_OS_HASH_FOR_EACH_POSSIBLE_SAFE(name, obj, tmp, member, key) \
    CAM_OS_HLIST_FOR_EACH_ENTRY_SAFE(obj, tmp, &name[CAM_OS_HASH_MIN(key, CAM_OS_HASH_BITS(name))], member)

static inline void CAM_OS_HASH_DEL(struct CamOsHListNode_t *node)
{
    CAM_OS_HLIST_DEL_INIT(node);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_UTIL_HASH_H__
