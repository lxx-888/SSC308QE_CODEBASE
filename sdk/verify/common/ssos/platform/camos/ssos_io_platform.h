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

#ifndef __KERNEL__
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#if defined(__KERNEL__) || defined(CAM_OS_RTK)
#include "ms_platform.h"
#endif
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"

#define SSOS_IO_PLATFORM_O_RDONLY CAM_FS_O_RDONLY
#define SSOS_IO_PLATFORM_O_WRONLY CAM_FS_O_WRONLY
#define SSOS_IO_PLATFORM_O_RDWR   CAM_FS_O_RDWR
#define SSOS_IO_PLATFORM_O_CREAT  CAM_FS_O_CREAT
#define SSOS_IO_PLATFORM_O_TRUNC  CAM_FS_O_TRUNC
#define SSOS_IO_PLATFORM_O_APPEND CAM_FS_O_APPEND

#define SSOS_IO_PLATFORM_SEEK_SET CAM_FS_SEEK_SET /* seek relative to beginning of file */
#define SSOS_IO_PLATFORM_SEEK_CUR CAM_FS_SEEK_CUR /* seek relative to current file position */
#define SSOS_IO_PLATFORM_SEEK_END CAM_FS_SEEK_END /* seek relative to end of file */

#define SSOS_IO_PANIC(str)               CamOsPanic(str)
#define SSOS_IO_PLATFORM_PRINT(fmt, ...) CamOsPrintf(fmt, ##__VA_ARGS__)

#endif /* ifndef __SSOS_IO_PLATFORM_H__ */
