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

#if defined(__KERNEL__) || defined(CAM_OS_RTK)
#include "ms_platform.h"
#endif
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "ssos_io.h"

SSOS_IO_File_t SSOS_IO_FileOpen(const char *pathname, int flags, int mode)
{
    SSOS_IO_File_t fd = NULL;
    return (CamFsOpen(&fd, pathname, flags, mode) == CAM_FS_OK && fd) ? fd : NULL;
}
SSOS_EXPORT_SYMBOL(SSOS_IO_FileOpen);

int SSOS_IO_FileClose(SSOS_IO_File_t file)
{
    return CamFsClose(file);
}
SSOS_EXPORT_SYMBOL(SSOS_IO_FileClose);

unsigned long SSOS_IO_FileRead(SSOS_IO_File_t file, void *buf, unsigned long count)
{
    return CamFsRead(file, buf, count);
}
SSOS_EXPORT_SYMBOL(SSOS_IO_FileRead);

unsigned long SSOS_IO_FileWrite(SSOS_IO_File_t file, const void *buf, unsigned long count)
{
    return CamFsWrite(file, buf, count);
}
SSOS_EXPORT_SYMBOL(SSOS_IO_FileWrite);

int SSOS_IO_FileSeek(SSOS_IO_File_t file, long offset, int whence)
{
    return CamFsSeek(file, offset, whence);
}
SSOS_EXPORT_SYMBOL(SSOS_IO_FileSeek);

int SSOS_IO_Atoi(const char *str)
{
    if (!str)
    {
        return -1;
    }
    if (strlen(str) >= 3 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        int ret = 0;
        sscanf(str, "%x", &ret);
        return ret;
    }
#if defined(__KERNEL__)
    return simple_strtol(str, NULL, 10);
#else
    return atoi(str);
#endif
}
SSOS_EXPORT_SYMBOL(SSOS_IO_Atoi);
long long SSOS_IO_Atoll(const char *str)
{
    if (!str)
    {
        return -1;
    }
    if (strlen(str) >= 3 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        long long ret = 0;
        sscanf(str, "%llx", &ret);
        return ret;
    }
#if defined(__KERNEL__)
    return simple_strtoll(str, NULL, 10);
#else
    return atoll(str);
#endif
}
SSOS_EXPORT_SYMBOL(SSOS_IO_Atoll);
