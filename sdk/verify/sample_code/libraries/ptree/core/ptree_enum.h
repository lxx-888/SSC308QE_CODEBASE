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

#ifndef __PTREE_ENUM_H__
#define __PTREE_ENUM_H__

#include "ssos_io.h"

#define PTREE_ENUM_DECLARE(_enumType)                         \
    const char * EnumCastToStr##_enumType(unsigned int eVal); \
    unsigned int EnumCastFromStr##_enumType(const char *str);

#define PTREE_ENUM_DEFINE(_enumType, ...)                                                                  \
    static const struct                                                                                    \
    {                                                                                                      \
        unsigned int eVal;                                                                                 \
        const char * str;                                                                                  \
    } G_ENUM_STR_PAIR_##_enumType[] = {__VA_ARGS__};                                                       \
    const char *EnumCastToStr##_enumType(unsigned int eVal)                                                \
    {                                                                                                      \
        int i = 0;                                                                                         \
        for (i = 0; i < sizeof(G_ENUM_STR_PAIR_##_enumType) / sizeof(G_ENUM_STR_PAIR_##_enumType[0]); ++i) \
        {                                                                                                  \
            if (eVal == G_ENUM_STR_PAIR_##_enumType[i].eVal)                                               \
            {                                                                                              \
                return G_ENUM_STR_PAIR_##_enumType[i].str;                                                 \
            }                                                                                              \
        }                                                                                                  \
        return "NA";                                                                                       \
    }                                                                                                      \
    unsigned int EnumCastFromStr##_enumType(const char *str)                                               \
    {                                                                                                      \
        int i = 0;                                                                                         \
        if (!str)                                                                                          \
        {                                                                                                  \
            return G_ENUM_STR_PAIR_##_enumType[0].eVal;                                                    \
        }                                                                                                  \
        for (i = 0; i < sizeof(G_ENUM_STR_PAIR_##_enumType) / sizeof(G_ENUM_STR_PAIR_##_enumType[0]); ++i) \
        {                                                                                                  \
            if (0 == strcmp(G_ENUM_STR_PAIR_##_enumType[i].str, str))                                      \
            {                                                                                              \
                return G_ENUM_STR_PAIR_##_enumType[i].eVal;                                                \
            }                                                                                              \
        }                                                                                                  \
        return G_ENUM_STR_PAIR_##_enumType[0].eVal;                                                        \
    }

#define PTREE_ENUM_TO_STR(_enumType, _eVal)  EnumCastToStr##_enumType(_eVal)
#define PTREE_ENUM_FROM_STR(_enumType, _str) EnumCastFromStr##_enumType(_str)

#endif /* ifndef __PTREE_ENUM_H__ */
