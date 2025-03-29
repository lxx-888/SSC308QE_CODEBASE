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

#include <errno.h>
#define SSOS_EXPORT_SYMBOL(__symbol)

#define SSOS_DEF_PLATFORM_OK       (0)
#define SSOS_DEF_PLATFORM_FAIL     (-1)
#define SSOS_DEF_PLATFORM_EINVAL   (-EINVAL)
#define SSOS_DEF_PLATFORM_ENOMEM   (-ENOMEM)
#define SSOS_DEF_PLATFORM_ETIMEOUT (-ETIMEDOUT)
#define SSOS_DEF_PLATFORM_EBUSY    (-EBUSY)

#endif /* ifndef __SSOS_DEF_PLATFORM__ */
