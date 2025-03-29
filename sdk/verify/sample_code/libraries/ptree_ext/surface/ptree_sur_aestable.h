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

#ifndef __PTREE_SUR_AESTABLE_H__
#define __PTREE_SUR_AESTABLE_H__

#include "ptree_sur.h"

enum PTREE_SUR_AESTABLE_RunMode_e
{
    E_PTREE_SUR_AESTABLE_RUN_MODE_SHOT,
    E_PTREE_SUR_AESTABLE_RUN_MODE_RECORD,
};

enum PTREE_SUR_AESTABLE_StartMode_e
{
    E_PTREE_SUR_AESTABLE_START_MODE_FORCE,
    E_PTREE_SUR_AESTABLE_START_MODE_AUTO,
};

typedef struct PTREE_SUR_AESTABLE_Info_s
{
    PTREE_SUR_Info_t base;
    unsigned int     runMode;
    unsigned int     startMode;
    unsigned int     captureCount;  /* only use for capture. */
    unsigned char    usingLowPower; /* only use for capture. */
    unsigned int     stableCount;
    unsigned char    debugMode;
} PTREE_SUR_AESTABLE_Info_t;

#endif //__PTREE_MOD_FILE_H__
