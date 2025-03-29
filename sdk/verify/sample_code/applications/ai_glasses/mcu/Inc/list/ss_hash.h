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

#ifndef __SS_HASH_H__
#define __SS_HASH_H__

#include "list.h"
#include "array_init.h"
#define LIST_HEAD_ARRAY_ELEMENT(__num, __name) LIST_HEAD_INIT(__name[__num])
#define LIST_HEAD_ARRAY_RIGHT_VAL(__name, __num) {__ARRAY_INIT_##__num##_M_ONE__(LIST_HEAD_ARRAY_ELEMENT, __name)}
#define LIST_HEAD_ARRAY(__name, __num) struct list_head __name[__num] = LIST_HEAD_ARRAY_RIGHT_VAL(__name, __num)

static inline unsigned int ss_hash(const char *key, unsigned int hash_size)
{
    unsigned int hash = 0;
    while (*key)
    {
        hash += *key;
        key++;
    }
    return hash % hash_size;
}

#endif
