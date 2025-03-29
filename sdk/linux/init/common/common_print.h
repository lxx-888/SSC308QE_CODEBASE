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
#ifndef __COMMON_PRINT_H__
#define __COMMON_PRINT_H__

#ifndef IS_PRINT_MATCH
#define IS_PRINT_MATCH MACRO_CAT(IS_PRINT_MATCH_, EXTRA_MODULE_NAME)
#endif
MI_BOOL IS_PRINT_MATCH(int level,const char* file,const char* func);

#ifndef L_MI_DBG_NONE
#define L_MI_DBG_NONE  0
#define L_MI_DBG_ERR   1
#define L_MI_DBG_WRN   2
#define L_MI_DBG_API   3
#define L_MI_DBG_KMSG  4
#define L_MI_DBG_INFO  5
#define L_MI_DBG_DEBUG 6
#define L_MI_DBG_TRACE 7
#define L_MI_DBG_ALL   8
#endif // !L_MI_DBG_NONE
/// ASCII color code
#define ASCII_COLOR_RED "\033[1;31m"
#define ASCII_COLOR_WHITE "\033[1;37m"
#define ASCII_COLOR_YELLOW "\033[1;33m"
#define ASCII_COLOR_BLUE "\033[1;36m"
#define ASCII_COLOR_GREEN "\033[1;32m"
#define ASCII_COLOR_END "\033[0m"
#ifndef DBG_ERR

#define DBG_ERR(fmt, args...)                                                                                     \
    ({                                                                                                            \
        do                                                                                                        \
        {                                                                                                         \
            if (IS_PRINT_MATCH(L_MI_DBG_ERR, __FILE__, __FUNCTION__))                                                                       \
            {                                                                                                     \
                CamOsPrintf(ASCII_COLOR_RED "[MI ERR ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, \
                                         __LINE__, ##args);                                                       \
            }                                                                                                     \
        } while (0);                                                                                              \
    })
#define DBG_WRN(fmt, args...)                                                                                        \
    ({                                                                                                               \
        do                                                                                                           \
        {                                                                                                            \
            if (IS_PRINT_MATCH(L_MI_DBG_WRN, __FILE__, __FUNCTION__))                                                                          \
            {                                                                                                        \
                CamOsPrintf(ASCII_COLOR_YELLOW "[MI WRN ]: %s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, \
                                         __LINE__, ##args);                                                          \
            }                                                                                                        \
        } while (0);                                                                                                 \
    })
#define DBG_INFO(fmt, args...)                                                                                     \
    ({                                                                                                             \
        do                                                                                                         \
        {                                                                                                          \
            if (IS_PRINT_MATCH(L_MI_DBG_INFO, __FILE__, __FUNCTION__))                                                                       \
            {                                                                                                      \
                CamOsPrintf(ASCII_COLOR_GREEN "[MI INFO]:%s[%d]: " fmt ASCII_COLOR_END, __FUNCTION__, \
                                         __LINE__, ##args);                                                        \
            }                                                                                                      \
        } while (0);                                                                                               \
    })

#endif // !DBG_ERR
#endif // !__COMMON_PRINT_H__
