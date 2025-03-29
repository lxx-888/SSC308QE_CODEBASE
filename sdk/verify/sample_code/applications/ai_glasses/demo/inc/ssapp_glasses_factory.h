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
#ifndef __SSAPP_GLASSES_FACTORY_H
#define __SSAPP_GLASSES_FACTORY_H

#include "list.h"
#include "mi_common_datatype.h"
#include "st_common_ai_glasses_protrcol.h"

typedef struct SSAPP_GLASSES_FACTORY_WorkSpace_s
{
    struct list_head   workList;
    ST_Protrcol_Task_t stProtcolTask;
    MI_S32 (*workerFun)(struct SSAPP_GLASSES_FACTORY_WorkSpace_s* pstFactoryWorkSpace);
    MI_S32 (*workerCleanFun)(struct SSAPP_GLASSES_FACTORY_WorkSpace_s* pstFactoryWorkSpace);
    BOOL    bNeedWaitDone;
    BOOL    bWorkHadDone;
    BOOL    bNoReplayMcuDone; // we no need replay when got E_TASK_NONE form mcu
    MI_VIRT userPointer;
} SSAPP_GLASSES_FACTORY_WorkSpace_t;

typedef struct SSAPP_GLASSES_FACTORY_DebugParam_s
{
    BOOL bDisableSuspend;
    BOOL bDisableHeartBeat;
    BOOL bDisbaleSignal;
    BOOL bWaitDoingTimeout;
    BOOL bDmaSupport;
    BOOL bDebugMode;

} SSAPP_GLASSES_FACTORY_DebugParam_t;

MI_S32 SSAPP_GLASSES_FACTORY_Init(void);
MI_S32 SSAPP_GLASSES_FACTORY_DeInit(void);
void   SSAPP_GLASSES_FACTORY_WakeUpMainByResume(void);
MI_S32 SSAPP_GLASSES_FACTORY_WorkDoneWakeUpMain(void);
MI_S32 SSAPP_GLASSES_FACTORY_HandleTask(ST_Protrcol_Task_t* pstProtcolTask);
MI_S32 SSAPP_GLASSES_FACTORY_SendCmdToMcuUart(SS_CMD_TYPE_e eCmdType, SS_TASK_e eTaskCmd, BOOL isReject,
                                              SS_STATE_e eSocState, MI_U16 u16UserDefine);
#endif
