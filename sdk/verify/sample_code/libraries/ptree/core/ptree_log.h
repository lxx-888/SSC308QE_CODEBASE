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

/*
 * The full name of 'ptree' is 'Pipeline tree', which use the idea of 'Amigos'
 * for reference and auther is 'pedro.peng' from Sigmastar.
 */

#ifndef __PTREE_LOG_H__
#define __PTREE_LOG_H__

#include "ssos_io.h"

#define PTREE_DBG_EN 0
#define PTREE_WRN_EN 0
#define PTREE_LOG_EN 1
#define PTREE_ERR_EN 1

#define PTREE_PRINT(_title, _color, _fmt, ...)             \
    SSOS_IO_Printf(_color                                  \
                   "["_title                               \
                   "]\033[0m"                              \
                   "\033[36m[%s]\033[35m[%d]\033[0m"_color \
                   "> "_fmt                                \
                   "\033[0m"                               \
                   "\n",                                   \
                   __FUNCTION__, __LINE__, ##__VA_ARGS__)

#if (!PTREE_DBG_EN || !PTREE_WRN_EN || !PTREE_ERR_EN)
static inline void _PTREE_LOG_None(const char *fmt, ...)
{
    /*
     *
     * In order to avoid build error/warning about unused-variable, we have to define an inline functin
     * to do nothing, which will be optimized by the compiler.
     *
     */
}
#endif

#if PTREE_DBG_EN
#define PTREE_DBG(_fmt, ...) PTREE_PRINT("ptree_dbg", "\033[1;32m", _fmt, ##__VA_ARGS__)
#else
#define PTREE_DBG(_fmt, ...) _PTREE_LOG_None(_fmt, ##__VA_ARGS__)
#endif

#if PTREE_WRN_EN
#define PTREE_WRN(_fmt, ...) PTREE_PRINT("ptree_wrn", "\033[1;33m", _fmt, ##__VA_ARGS__)
#else
#define PTREE_WRN(_fmt, ...) _PTREE_LOG_None(_fmt, ##__VA_ARGS__)
#endif

#if PTREE_LOG_EN
#define PTREE_LOG(_fmt, ...) PTREE_PRINT("ptree_log", "\033[1;32m", _fmt, ##__VA_ARGS__)
#else
#define PTREE_LOG(_fmt, ...) _PTREE_LOG_None(_fmt, ##__VA_ARGS__)
#endif

#if PTREE_ERR_EN
#define PTREE_ERR(_fmt, ...) PTREE_PRINT("ptree_err", "\033[1;31m", _fmt, ##__VA_ARGS__)
#else
#define PTREE_ERR(_fmt, ...) _PTREE_LOG_None(_fmt, ##__VA_ARGS__)
#endif

#endif /* __PTREE_LOG_H__ */
