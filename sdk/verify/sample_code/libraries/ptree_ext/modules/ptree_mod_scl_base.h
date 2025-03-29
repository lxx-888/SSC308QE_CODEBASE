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

#ifndef __PTREE_MOD_SCL_BASE_H__
#define __PTREE_MOD_SCL_BASE_H__

#include "mi_scl_datatype.h"

#define PTREE_MOD_SCL_DEV_NUM (9)

int PTREE_MOD_SCL_BASE_CreateDevice(unsigned int devId, MI_SCL_DevAttr_t *sclDevAttr);
int PTREE_MOD_SCL_BASE_DestroyDevice(unsigned int devId);

#endif //__PTREE_MOD_SCL_BASE_H__
