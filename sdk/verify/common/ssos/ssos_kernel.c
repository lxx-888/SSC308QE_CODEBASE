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
#include "ssos_io.h"
#include "cam_sysfs.h"
#include "cam_device_wrapper.h"

static int __init _SSOS_KERNEL_Init(void)
{
    SSOS_IO_Printf("SSOS kernel init\n");
    return 0;
}
static void __exit _SSOS_KERNEL_Exit(void)
{
    SSOS_IO_Printf("SSOS kernel exit\n");
}

module_init(_SSOS_KERNEL_Init);
module_exit(_SSOS_KERNEL_Exit);

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("pedro.peng@sigmastar.com.cn");
MODULE_DESCRIPTION("SSOS kernel");
