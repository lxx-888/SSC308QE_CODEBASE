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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "ssos_io.h"

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
    return atoi(str);
}

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
    return atoll(str);
}

SSOS_IO_File_t SSOS_IO_FileOpen(const char *pathname, int flags, int mode)
{
    int fd = open(pathname, flags, mode);
    return fd < 0 ? NULL : (SSOS_IO_File_t)(long)fd;
}
int SSOS_IO_FileClose(SSOS_IO_File_t file)
{
    return close((long unsigned int)file);
}
unsigned long SSOS_IO_FileRead(SSOS_IO_File_t file, void *buf, unsigned long count)
{
    return read((long unsigned int)file, buf, count);
}
unsigned long SSOS_IO_FileWrite(SSOS_IO_File_t file, const void *buf, unsigned long count)
{
    return write((long unsigned int)file, buf, count);
}
int SSOS_IO_FileSeek(SSOS_IO_File_t file, long offset, int whence)
{
    return lseek((long unsigned int)file, offset, whence);
}
