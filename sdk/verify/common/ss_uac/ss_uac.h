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
#ifndef _SS_UAC_H_
#define _SS_UAC_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif

#include "ss_uac_datatype.h"


int32_t SS_UAC_AllocStream(void *, SS_UAC_Handle_h*);

int32_t SS_UAC_FreeStream(SS_UAC_Handle_h);

int32_t SS_UAC_StartDev(SS_UAC_Handle_h);

int32_t SS_UAC_StoptDev(SS_UAC_Handle_h);

int32_t SS_UAC_SetTraceLevel(int);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif//_SS_UAC_H_
