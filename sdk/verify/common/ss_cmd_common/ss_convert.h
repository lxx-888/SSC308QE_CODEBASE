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
#ifndef _SS_CONVERT_H_
#define _SS_CONVERT_H_

#include <string>
#include <algorithm>

#define STR_ENUM(enum_type, prefix, ...)                                                \
    template<typename enum_type>                                                        \
    static inline std::string prefix##to_str(const enum_type &e)                        \
    {                                                                                   \
        std::string str;                                                                \
        static_assert(std::is_enum<enum_type>::value, #enum_type "must be an enum!");   \
        static const std::pair<enum_type, std::string> m[] = __VA_ARGS__;               \
        auto it = std::find_if(std::begin(m), std::end(m),                              \
                [e](const std::pair<enum_type, std::string>& estr_pair) -> bool         \
                {                                                                       \
                    return estr_pair.first == e;                                        \
                });                                                                     \
        str = ((it != std::end(m)) ? it : std::begin(m))->second;                       \
        return str;                                                                     \
    }                                                                                   \
    template<typename charp>                                                            \
    static inline enum_type prefix##from_str(charp &str)                                \
    {                                                                                   \
        enum_type e;                                                                    \
        static_assert(std::is_enum<enum_type>::value, #enum_type "must be an enum!");   \
        static const std::pair<enum_type, std::string> m[] = __VA_ARGS__;               \
        auto it = std::find_if(std::begin(m), std::end(m),                              \
                [&str](const std::pair<enum_type, std::string>& estr_pair)->bool        \
                {                                                                       \
                    return estr_pair.second == str;                                     \
                    });                                                                 \
        e = ((it != std::end(m)) ? it : std::begin(m))->first;                          \
        return e;                                                                       \
    }

#endif // _SS_CONVERT_H_
