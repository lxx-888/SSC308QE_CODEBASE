/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef __PARENA_H__
#define __PARENA_H__

#include "arena.h"

/*
 * parena - preload arena
 * ---
 *  parena add a pstTag for each block of arena for serialize and deserialize
 */

/*
 * parena_tag - parena pstTag structure
 *
 *           ┌─────────────┐
 *           │u32Size      ├────────────────────┐
 *           ├─────────────┤                    │
 *           │u32BaseOffset├────────────────┐   │
 *           ├─────────────┤                │   │
 *       ┌───┤u32BaseSize  │                │   │
 *       │   ├─────────────┼───────────┐◄───│───┤
 *       │   │pData        │           │    │   │
 *       │   └─────────────┤           │    │   │
 *       │                 │           │    │   │
 *       ├────────────────►├───────────┤◄───┘   │
 *       │                 │   base    │        │
 *       │                 │           │        │
 *       │                 │           │        │
 *       └────────────────►├───────────┤        │
 *                         │           │        │
 *                         └───────────┘◄───────┘
 *
 */
typedef struct PARENA_Tag_s
{
    unsigned int u32Magic;      /* Magic number */
    unsigned int u32Size;       /* Size of block */
    unsigned int u32BaseOffset; /* Start offset of base struct in struct */
    unsigned int u32BaseSize;   /* Base struct size */
    unsigned int u32Offset;     /* Arena pool base */
    char         pData[0];      /* Pointer to data */
} PARENA_Tag_t;

#define __PARENA_MAGIC_NUM__ 0x88860611

#define PARENA_ROOT_COUNT_MAX 64
#define PARENA_ROOT_COUNT_END (PARENA_ROOT_COUNT_MAX + 1)

#define PARENA_RELATIVE_OFFSET(_ptr, _dst) ((void *)(_dst) - (void *)(_ptr))

#define PARENA_RELATIVE_ADDRESS(_ptr, _off) ((void *)(_ptr) + (_off))

/* Alloc Arena menory */
#define PARENA_GET(_arena, _type, _base_member, _base_type)                                               \
    (                                                                                                     \
        {                                                                                                 \
            PARENA_Tag_t *_tag = (PARENA_Tag_t *)ARENA_Get(_arena, sizeof(PARENA_Tag_t) + sizeof(_type)); \
            if (_tag)                                                                                     \
            {                                                                                             \
                _tag->u32Magic      = __PARENA_MAGIC_NUM__;                                               \
                _tag->u32Size       = sizeof(_type);                                                      \
                _tag->u32BaseOffset = ((size_t) & ((_type *)0)->_base_member);                            \
                _tag->u32BaseSize   = sizeof(_base_type);                                                 \
                _tag->u32Offset     = ARENA_Offset(_arena, _tag);                                         \
                PTREE_DBG("SIZE: %d", _tag->u32Size);                                                     \
            }                                                                                             \
            _tag;                                                                                         \
        })

/* Shift Tag --> Base */
#define PARENA_USE_BASE(_tag)                                  \
    (                                                          \
        {                                                      \
            if ((_tag)->u32Magic != __PARENA_MAGIC_NUM__)      \
            {                                                  \
                PTREE_ERR("Tag u32Magic number match error!"); \
            }                                                  \
            (_tag)->pData + (_tag)->u32BaseOffset;             \
        })

/* Shift Current Tag --> Next Tag*/
#define PARENA_USE_NEXT(_tag)                                              \
    (                                                                      \
        {                                                                  \
            PARENA_Tag_t *_tag_next = NULL;                                \
            if ((_tag)->u32Magic != __PARENA_MAGIC_NUM__)                  \
            {                                                              \
                PTREE_ERR("Tag u32Magic number match error!");             \
            }                                                              \
            _tag_next = (PARENA_Tag_t *)((_tag)->pData + (_tag)->u32Size); \
            if ((_tag_next)->u32Magic != __PARENA_MAGIC_NUM__)             \
            {                                                              \
                PTREE_ERR("NextTag u32Magic number match error!");         \
            }                                                              \
            _tag_next;                                                     \
        })

#endif /* __PARENA_H__ */
