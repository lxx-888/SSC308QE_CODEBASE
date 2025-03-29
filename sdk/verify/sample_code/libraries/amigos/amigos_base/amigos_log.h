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

#ifndef __AMIGOS_LOG_H__
#define __AMIGOS_LOG_H__

#include "ss_log.h"

#define AMIGOS_LOG( __fmt, ...) do {            \
        amilog.Out.print(__fmt, ##__VA_ARGS__); \
    } while(0)

#define _AMIGOS_LOG(__sslog, __fmt, ...) do { \
        __sslog << CODE_TRACE;                \
        __sslog.print(__fmt, ##__VA_ARGS__);  \
        __sslog << PRINT_COLOR_END;           \
    } while(0)

#define AMIGOS_INFO(_fmt, ...) do {               \
        ss_log &_sslog = amilog.Info();           \
        _AMIGOS_LOG(_sslog, _fmt, ##__VA_ARGS__); \
    } while(0)

#define AMIGOS_DBG(_fmt, ...) do {                \
        ss_log &_sslog = amilog.Debug();          \
        _AMIGOS_LOG(_sslog, _fmt, ##__VA_ARGS__); \
    } while(0)

#define AMIGOS_WRN(_fmt, ...) do {                \
        ss_log &_sslog = amilog.Warn();           \
        _AMIGOS_LOG(_sslog, _fmt, ##__VA_ARGS__); \
    } while(0)

#define AMIGOS_ERR(_fmt, ...) do {                \
        ss_log &_sslog = amilog.Error();          \
        _AMIGOS_LOG(_sslog, _fmt, ##__VA_ARGS__); \
    } while(0)

#define AMILOG_INFO (amilog.Info()  << CODE_TRACE)
#define AMILOG_DBG  (amilog.Debug() << CODE_TRACE)
#define AMILOG_WRN  (amilog.Warn()  << CODE_TRACE)
#define AMILOG_ERR  (amilog.Error() << CODE_TRACE)
#define AMILOG      (amilog.Out)

#define AMIGOS_MEASURE_TIME(__exec_func) do {           \
        ss_log_result res(amilog.Info(), #__exec_func); \
        __exec_func;                                    \
    } while (0)

class AmigosLog
{
    public:
        AmigosLog(enum print_lv lv = PRINT_LV_TRACE);
        virtual ~AmigosLog();
        ss_log &Info();
        ss_log &Debug();
        ss_log &Warn();
        ss_log &Error();
        ss_log Out;
};
extern AmigosLog amilog;

#endif
