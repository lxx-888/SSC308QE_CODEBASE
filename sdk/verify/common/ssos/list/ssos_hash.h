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

#ifndef __PTHREE_HASH_H__
#define __PTHREE_HASH_H__

#include "array_init.h"
#include "ssos_list.h"
#define LIST_HEAD_ARRAY_ELEMENT(__num, __name) SSOS_LIST_HEAD_INIT(__name[__num])
#define LIST_HEAD_ARRAY_RIGHT_VAL(__name, __num)                        \
    {                                                                   \
        __ARRAY_INIT_##__num##_M_ONE__(LIST_HEAD_ARRAY_ELEMENT, __name) \
    }
#define LIST_HEAD_ARRAY(__name, __num) struct SSOS_LIST_Head_s __name[__num] = LIST_HEAD_ARRAY_RIGHT_VAL(__name, __num)

#define SSOS_HASH_INIT(__list_array)                                                         \
    do                                                                                       \
    {                                                                                        \
        unsigned int __i;                                                                    \
        for (__i = 0; __i < (sizeof(__list_array) / sizeof(struct SSOS_LIST_Head_s)); __i++) \
        {                                                                                    \
            SSOS_LIST_InitHead(&((__list_array)[__i]));                                      \
        }                                                                                    \
    } while (0)

static inline unsigned int SSOS_HASH_Val(const char *pKey, unsigned int u32HashSize) // NOLINT
{
    unsigned int u32HashVal = 0;
    while (*pKey)
    {
        u32HashVal += *pKey;
        pKey++;
    }
    return u32HashVal % u32HashSize;
}

#endif
