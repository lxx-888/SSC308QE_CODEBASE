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

#ifndef __PTREE_MOD_PASS_H__
#define __PTREE_MOD_PASS_H__

#include "ptree_mod.h"

int PTREE_MOD_PASS_BuildDelayPass(PTREE_MOD_OutObj_t *modOut);
int PTREE_MOD_PASS_DeleteDelayPass(PTREE_MOD_OutObj_t *modOut);
int PTREE_MOD_PASS_InitDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx);
int PTREE_MOD_PASS_DeinitDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx);
int PTREE_MOD_PASS_BindDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx);
int PTREE_MOD_PASS_UnbindDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx);
int PTREE_MOD_PASS_StartDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx);
int PTREE_MOD_PASS_StopDelayPass(PTREE_MOD_OutObj_t *modOut, unsigned int rootIdx);

#endif //__PTREE_MOD_PASS_H__
