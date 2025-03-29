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

#ifndef __SSOS_IO_PLATFORM_H__
#define __SSOS_IO_PLATFORM_H__

#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#define SSOS_IO_PLATFORM_O_RDONLY O_RDONLY
#define SSOS_IO_PLATFORM_O_WRONLY O_WRONLY
#define SSOS_IO_PLATFORM_O_RDWR   O_RDWR
#define SSOS_IO_PLATFORM_O_CREAT  O_CREAT
#define SSOS_IO_PLATFORM_O_TRUNC  O_TRUNC
#define SSOS_IO_PLATFORM_O_APPEND O_APPEND

#define SSOS_IO_PLATFORM_SEEK_SET SEEK_SET /* seek relative to beginning of file */
#define SSOS_IO_PLATFORM_SEEK_CUR SEEK_CUR /* seek relative to current file position */
#define SSOS_IO_PLATFORM_SEEK_END SEEK_END /* seek relative to end of file */

#define SSOS_IO_PANIC(str)               printf("%s", str), assert(0)
#define SSOS_IO_PLATFORM_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)

#endif /* ifndef __SSOS_IO_PLATFORM_H__ */
