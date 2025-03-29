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

#ifndef __SSOS_IO_H__
#define __SSOS_IO_H__

#include "ssos_def.h"
#include "ssos_io_platform.h"

#define SSOS_IO_O_RDONLY SSOS_IO_PLATFORM_O_RDONLY
#define SSOS_IO_O_WRONLY SSOS_IO_PLATFORM_O_WRONLY
#define SSOS_IO_O_RDWR   SSOS_IO_PLATFORM_O_RDWR
#define SSOS_IO_O_CREAT  SSOS_IO_PLATFORM_O_CREAT
#define SSOS_IO_O_TRUNC  SSOS_IO_PLATFORM_O_TRUNC
#define SSOS_IO_O_APPEND SSOS_IO_PLATFORM_O_APPEND

#define SSOS_IO_SEEK_SET SSOS_IO_PLATFORM_SEEK_SET /* seek relative to beginning of file */
#define SSOS_IO_SEEK_CUR SSOS_IO_PLATFORM_SEEK_CUR /* seek relative to current file position */
#define SSOS_IO_SEEK_END SSOS_IO_PLATFORM_SEEK_END /* seek relative to end of file */

typedef void *SSOS_IO_File_t;

#define SSOS_IO_Printf(fmt, ...) SSOS_IO_PLATFORM_PRINT(fmt, ##__VA_ARGS__) // NOLINT
#define SSOS_ASSERT(_x_)                                                                              \
    do                                                                                                \
    {                                                                                                 \
        if (!(_x_))                                                                                   \
        {                                                                                             \
            SSOS_IO_Printf("PTREE ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
            SSOS_IO_PANIC("PTREE BUG");                                                               \
        }                                                                                             \
    } while (0)

SSOS_IO_File_t SSOS_IO_FileOpen(const char *pathname, int flags, int mode);
int            SSOS_IO_FileClose(SSOS_IO_File_t file);
unsigned long  SSOS_IO_FileRead(SSOS_IO_File_t file, void *buf, unsigned long count);
unsigned long  SSOS_IO_FileWrite(SSOS_IO_File_t file, const void *buf, unsigned long count);
int            SSOS_IO_FileSeek(SSOS_IO_File_t file, long offset, int whence);
int            SSOS_IO_Atoi(const char *str);
long long      SSOS_IO_Atoll(const char *str);

#endif /* ifndef __SSOS_IO_H__ */
