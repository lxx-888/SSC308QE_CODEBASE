/*
 * mi_common.h- Sigmastar
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
#ifndef _MI_COMMON_H_
#define _MI_COMMON_H_

#include "mi_common_datatype.h"

#define COMMON_MAJOR_VERSION 2
#define COMMON_SUB_VERSION   3
#define MACRO_TO_STR(macro)  #macro
#define COMMON_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                                  \
        {                                                                                                              \
            char *tmp =                                                                                                \
                sub_version / 100  ? "mi_common_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_common_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_common_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                       \
        })
#define MI_COMMON_API_VERSION COMMON_VERSION_STR(COMMON_MAJOR_VERSION, COMMON_SUB_VERSION)

#endif ///_MI_COMMON_H_
