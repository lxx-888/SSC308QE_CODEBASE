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
#include "ssos_mem.h"
#ifdef CAM_OS_RTK
#include "sys_mem.h"
#endif

void *SSOS_MEM_Alloc(unsigned long size)
{
    return CamOsMemAlloc(size);
}
SSOS_EXPORT_SYMBOL(SSOS_MEM_Alloc);

void SSOS_MEM_Free(void *ptr)
{
    CamOsMemRelease(ptr);
}
SSOS_EXPORT_SYMBOL(SSOS_MEM_Free);

void *SSOS_MEM_AlignAlloc(unsigned long size, unsigned char align)
{
#ifdef CAM_OS_RTK
    return SysMemAllocExt(size, align);
#else
    (void)align;
    return CamOsMemAlloc(size);
#endif
}
SSOS_EXPORT_SYMBOL(SSOS_MEM_AlignAlloc);

void SSOS_MEM_AlignFree(void *ptr)
{
#ifdef CAM_OS_RTK
    SysMemFree(ptr);
#else
    CamOsMemRelease(ptr);
#endif
}
SSOS_EXPORT_SYMBOL(SSOS_MEM_AlignFree);
