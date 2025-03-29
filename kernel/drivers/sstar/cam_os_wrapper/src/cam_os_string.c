/*
 * cam_os_string.c- Sigmastar
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

///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_string.c
/// @brief     Cam OS Wrapper Source File of string class for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#include "linux/string.h"
#include "linux/kernel.h"
#elif defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
#include "string.h"
#include "stdlib.h"
#endif

#include "cam_os_wrapper.h"

void *CamOsMemcpy(void *dest, const void *src, u32 num)
{
    return memcpy(dest, src, num);
}

void *CamOsMemmove(void *dest, const void *src, u32 num)
{
    return memmove(dest, src, num);
}

void *CamOsMemset(void *ptr, int value, u32 num)
{
    return memset(ptr, value, num);
}

s32 CamOsMemcmp(const void *ptr1, const void *ptr2, u32 num)
{
    return memcmp(ptr1, ptr2, num);
}

char *CamOsStrstr(const char *str1, const char *str2)
{
    return strstr(str1, str2);
}

char *CamOsStrncpy(char *dest, const char *src, u32 num)
{
    return strncpy(dest, src, num);
}

s32 CamOsStrncmp(const char *str1, const char *str2, u32 num)
{
    return strncmp(str1, str2, num);
}

s32 CamOsStrncasecmp(const char *str1, const char *str2, u32 num)
{
    return strncasecmp(str1, str2, num);
}

s32 CamOsStrlen(const char *str)
{
    return strlen(str);
}

char *CamOsStrcat(char *dest, const char *src)
{
    return strcat(dest, src);
}

char *CamOsStrsep(char **stringp, const char *delim)
{
    return strsep(stringp, delim);
}

char *CamOsStrchr(const char *str, int ch)
{
    return strchr(str, ch);
}

char *CamOsStrrchr(const char *str, int ch)
{
    return strrchr(str, ch);
}

long CamOsStrtol(const char *szStr, char **szEndptr, s32 nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtol(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtol(szStr, szEndptr, nBase);
#endif
}

unsigned long CamOsStrtoul(const char *szStr, char **szEndptr, s32 nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtoul(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtoul(szStr, szEndptr, nBase);
#endif
}

unsigned long long CamOsStrtoull(const char *szStr, char **szEndptr, s32 nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtoull(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtoull(szStr, szEndptr, nBase);
#endif
}
