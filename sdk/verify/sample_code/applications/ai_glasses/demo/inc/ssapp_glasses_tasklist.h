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

#ifndef __SSAPP_GLASSES_TASKLIST_H
#define __SSAPP_GLASSES_TASKLIST_H

#include "mi_common_datatype.h"
#include "st_common_ai_glasses_protrcol.h"
#include "ssapp_glasses_factory.h"

MI_S32 SSAPP_GLASSES_TASKLIST_Init(void);

void SSAPP_GLASSES_TASKLIST_AddToDoingWorkList(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace);
void SSAPP_GLASSES_TASKLIST_AddToPendingWorkList(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace);
void SSAPP_GLASSES_TASKLIST_DelList(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace);

SSAPP_GLASSES_FACTORY_WorkSpace_t *SSAPP_GLASSES_TASKLIST_GetPendingWork(void);
SSAPP_GLASSES_FACTORY_WorkSpace_t *SSAPP_GLASSES_TASKLIST_GetDoingWork(void);

BOOL SSAPP_GLASSES_TASKLIST_WorkListIsEmpty(void);
BOOL SSAPP_GLASSES_TASKLIST_PendingWorkListIsEmpty(void);
#endif
