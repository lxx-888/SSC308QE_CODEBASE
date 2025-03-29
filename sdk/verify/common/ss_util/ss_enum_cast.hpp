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

#ifndef __SS_ENUM_CAST_HPP__
#define __SS_ENUM_CAST_HPP__

#include <string>
#include <unordered_map>
#include <algorithm>

template <typename EnumType>
class ss_enum_cast
{
public:
    static std::string to_str(const EnumType &)      = delete;
    static EnumType    from_str(const std::string &) = delete;
};

#define SS_ENUM_CAST_STR(_EnumType, ...)                                                              \
    template <>                                                                                       \
    class ss_enum_cast<_EnumType>                                                                     \
    {                                                                                                 \
       public:                                                                                        \
        static std::string to_str(const _EnumType &e)                                                 \
        {                                                                                             \
            static const std::unordered_map<_EnumType, std::string> m = __VA_ARGS__;                  \
                                                                                                      \
            auto it = m.find(e);                                                                      \
            return it != m.end() ? it->second : "Unknown";                                            \
        }                                                                                             \
        static _EnumType from_str(const std::string &str)                                             \
        {                                                                                             \
            static const std::pair<_EnumType, std::string> m[] = __VA_ARGS__;                         \
                                                                                                      \
            auto it = std::find_if(std::begin(m), std::end(m),                                        \
                                   [&str](const std::pair<_EnumType, std::string> &estr_pair) -> bool \
                                   { return estr_pair.second == str; });                              \
            return it != std::end(m) ? it->first : std::begin(m)->first;                              \
        }                                                                                             \
    }

#endif /*__SS_ENUM_CAST_HPP__*/

