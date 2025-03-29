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

#ifndef __SSOS_DEF_PLATFORM__
#define __SSOS_DEF_PLATFORM__

#if defined(__KERNEL__) || defined(CAM_OS_RTK)
#include "ms_platform.h"
#endif
#include "cam_os_wrapper.h"

#if defined(__KERNEL__)
#define SSOS_EXPORT_SYMBOL(__symbol) EXPORT_SYMBOL(__symbol)
#else
#define SSOS_EXPORT_SYMBOL(__symbol)
#endif

#define SSOS_DEF_PLATFORM_OK       CAM_OS_OK
#define SSOS_DEF_PLATFORM_FAIL     CAM_OS_FAIL
#define SSOS_DEF_PLATFORM_EINVAL   CAM_OS_PARAM_ERR
#define SSOS_DEF_PLATFORM_ENOMEM   CAM_OS_ALLOCMEM_FAIL
#define SSOS_DEF_PLATFORM_ETIMEOUT CAM_OS_TIMEOUT
#define SSOS_DEF_PLATFORM_EBUSY    CAM_OS_RESOURCE_BUSY

#endif /* ifndef __SSOS_DEF_PLATFORM__ */
