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

#ifndef __SSAPP_MIXER_PRELOAD__
#define __SSAPP_MIXER_PRELOAD__

#include "ptree.h"

unsigned long SSAPP_MIXER_PRELOAD_GetTimer(void);
int           SSAPP_MIXER_PRELOAD_PtreeTakeOff(PTREE_Config_t *pstConfig, int s32Argc, char **ppArgv);
int           SSAPP_MIXER_PRELOAD_PtreeDropDown(int s32DbChoice, PTREE_Config_t *pstConfig);
void          SSAPP_MIXER_PRELOAD_PipelineModuleCommandConsole(void);
void          SSAPP_MIXER_PRELOAD_PipelineControlConsole(void *pIns);

#endif //__SSAPP_MIXER_PRELOAD__
