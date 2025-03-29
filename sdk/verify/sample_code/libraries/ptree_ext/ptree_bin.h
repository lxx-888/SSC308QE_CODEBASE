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

#ifndef __PTREE_BIN_H__
#define __PTREE_BIN_H__
#ifndef __KERNEL__
#include <string.h>
#endif
typedef struct PTREE_BIN_Info_s
{
    const char * name;
    char *       data;
    unsigned int size;
} PTREE_BIN_Info_t;
#define PTREE_BIN_FILL_INFO(_name)                                                 \
    {                                                                              \
        .name = #_name, .data = auto_gen_##_name, .size = sizeof(auto_gen_##_name) \
    }
#include "resource/auto_gen_ptree_bin.h"
static inline const PTREE_BIN_Info_t *_PTREE_BIN_GetBinInfo(const char *name)
{
    int i = 0;
    for (i = 0; i < sizeof(G_BINARY_INFO_ARRAY) / sizeof(PTREE_BIN_Info_t); i++)
    {
        if (!strcmp(G_BINARY_INFO_ARRAY[i].name, name))
        {
            return &G_BINARY_INFO_ARRAY[i];
        }
    }
    if (i == 0)
    {
        /* return NULL, if no pipeline defined in current project. */
        return NULL;
    }
    /* Using default binary information if did not find anything. */
    return &G_BINARY_INFO_ARRAY[0];
}
#endif
