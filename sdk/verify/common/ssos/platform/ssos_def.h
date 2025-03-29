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

#ifndef __SSOS_DEF_H__
#define __SSOS_DEF_H__

#include "ssos_def_platform.h"

#define SSOS_DEF_OK       SSOS_DEF_PLATFORM_OK
#define SSOS_DEF_FAIL     SSOS_DEF_PLATFORM_FAIL
#define SSOS_DEF_EINVAL   SSOS_DEF_PLATFORM_EINVAL
#define SSOS_DEF_ENOMEM   SSOS_DEF_PLATFORM_ENOMEM
#define SSOS_DEF_ETIMEOUT SSOS_DEF_PLATFORM_ETIMEOUT
#define SSOS_DEF_EBUSY    SSOS_DEF_PLATFORM_EBUSY

#define SSOS_DEF_TRUE  (0 == 0)
#define SSOS_DEF_FALSE (0 == 1)

#endif /* ifndef __SSOS_DEF_H__ */
