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

#ifndef __SS_TASK_LIST_H
#define __SS_TASK_LIST_H

#include "list/list.h"
#include "ss_datatype.h"

typedef struct SS_TaskList_Head_s
{
    struct list_head list;
    // CamOsTsem_t      worklist_semlock;
    // there is a semlock to control read/write list if run on system
} SS_TaskList_Head_t;

typedef struct SS_TaskList_s
{
    struct list_head list;
    SS_TASK_e        task;
    uint8_t          taskId;
    int              photoCnt;
    BOOL             bHadSend;

} SS_TaskList_t;

void           SS_TaskList_Init(void);
int            SS_TaskList_NewAdd_Pending_Task(SS_TASK_e eTask);
int            SS_TaskList_NewAdd_Doing_Task_Head(SS_TASK_e eTask); // stop recording use only
int            SS_TaskList_Free_a_Doing_Task(SS_TaskList_t* pst_Task);
int            SS_TaskList_Is_Cmd_Doing(SS_TaskList_t** ppst_Task, SS_TASK_e eTask);
int            SS_TaskList_Is_Empty(void);
int            SS_TaskList_Task_Had_Done(uint8_t taskId);
SS_TaskList_t* SS_TaskList_Get_a_Doning_Task(void);
void           SS_TaskList_Auto_Send_Stop_Cmd_Task(void);
#endif
