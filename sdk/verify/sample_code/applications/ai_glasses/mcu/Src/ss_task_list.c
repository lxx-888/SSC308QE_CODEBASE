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

#include <stdio.h>
#include "ss_task_list.h"

SS_TaskList_Head_t st_Pending_TaskList_t = {0};
SS_TaskList_Head_t st_Doing_TaskList_t   = {0};

uint8_t globalTaskId = 0;

void SS_TaskList_Init(void)
{
    INIT_LIST_HEAD(&(st_Pending_TaskList_t.list));
    INIT_LIST_HEAD(&(st_Doing_TaskList_t.list));
}

int _SS_TaskList_NewAdd_Task(SS_TASK_e eTask, struct list_head *pstHead, BOOL bTail)
{
    SS_TaskList_t *pstTask;
    BOOL           bEmpty = TRUE;

    bEmpty = list_empty(pstHead);
    if (!bEmpty)
    {
        SS_TaskList_t *pstCurTask  = NULL;
        BOOL           bNeedReturn = FALSE;

        list_for_each_entry(pstCurTask, pstHead, list)
        {
            if (pstCurTask)
            {
                if (pstCurTask->task == E_TASK_PHOTO && eTask == E_TASK_PHOTO)
                {
                    pstCurTask->photoCnt++;
                    bNeedReturn = TRUE;
                    break;
                }
                else if (pstCurTask->task == E_TASK_START_REC && eTask == E_TASK_START_REC)
                {
                    bNeedReturn = TRUE;
                    break;
                }
                else if (pstCurTask->task == E_TASK_TRANS && eTask == E_TASK_TRANS)
                {
                    bNeedReturn = TRUE;
                    break;
                }
            }
        }
        if (bNeedReturn)
        {
            return 0;
        }
    }

    pstTask = (SS_TaskList_t *)malloc(sizeof(SS_TaskList_t));
    if (!pstTask)
    {
        return -1;
    }
    memset(pstTask, 0x0, sizeof(SS_TaskList_t));
    pstTask->task   = eTask;
    pstTask->taskId = globalTaskId++;
    if (eTask == E_TASK_PHOTO)
    {
        pstTask->photoCnt = 1;
    }
    if (bTail)
    {
        list_add_tail(&(pstTask->list), pstHead);
    }
    else
    {
        list_add(&(pstTask->list), pstHead);
    }
    DEBUG_INFO("add task eTask:%d taskCnt:%d\r\n", eTask, globalTaskId);
    return 0;
}

int SS_TaskList_NewAdd_Pending_Task(SS_TASK_e eTask)
{
    return _SS_TaskList_NewAdd_Task(eTask, &(st_Pending_TaskList_t.list), TRUE);
}

int SS_TaskList_NewAdd_Doing_Task_Head(SS_TASK_e eTask)
{
    return _SS_TaskList_NewAdd_Task(eTask, &(st_Doing_TaskList_t.list), FALSE);
}

int SS_TaskList_Add_To_Doing_Task(SS_TaskList_t *pst_Task)
{
    list_del(&(pst_Task->list));
    list_add_tail(&(pst_Task->list), &(st_Doing_TaskList_t.list));
    return 0;
}

SS_TaskList_t *_SS_TaskList_Get_a_Task(SS_TaskList_t *pstTaskList)
{
    SS_TaskList_t *pst_Task = NULL;
    BOOL           bEmpty   = TRUE;

    bEmpty = list_empty(&(pstTaskList->list));
    if (!bEmpty)
    {
        pst_Task = list_first_entry(&(pstTaskList->list), SS_TaskList_t, list);
    }
    return pst_Task;
}

SS_TaskList_t *SS_TaskList_Get_a_Doning_Task(void)
{
    return _SS_TaskList_Get_a_Task(&st_Doing_TaskList_t);
}

SS_TaskList_t *SS_TaskList_Get_a_Pending_Task(void)
{
    return _SS_TaskList_Get_a_Task(&st_Pending_TaskList_t);
}

SS_TaskList_t *SS_TaskList_Get_a_Doning_Task_By_TaskType(SS_TASK_e eTask)
{
    SS_TaskList_t *pstTask     = NULL;
    SS_TaskList_t *pstCurTask  = NULL;
    BOOL           bNeedReturn = FALSE;

    list_for_each_entry(pstCurTask, &(st_Doing_TaskList_t.list), list)
    {
        if (pstCurTask)
        {
            if (pstCurTask->task == eTask)
            {
                pstTask = pstCurTask;
                break;
            }
        }
    }

    return pstTask;
}

SS_TaskList_t *SS_TaskList_Get_a_Doning_Task_By_TaskId(uint8_t taskId)
{
    SS_TaskList_t *pstTask     = NULL;
    SS_TaskList_t *pstCurTask  = NULL;
    BOOL           bNeedReturn = FALSE;

    list_for_each_entry(pstCurTask, &(st_Doing_TaskList_t.list), list)
    {
        if (pstCurTask)
        {
            if (pstCurTask->taskId == taskId)
            {
                pstTask = pstCurTask;
                break;
            }
        }
    }

    return pstTask;
}

int SS_TaskList_Deal_a_Pending_Task(void)
{
    SS_TaskList_t *pstPendingTask = NULL;
    pstPendingTask                = SS_TaskList_Get_a_Pending_Task();
    if (pstPendingTask)
    {
        SS_TaskList_Add_To_Doing_Task(pstPendingTask);
        return 0;
    }
    return -1;
}

int SS_TaskList_Free_a_Doing_Task(SS_TaskList_t *pst_Task)
{
    list_del(&(pst_Task->list));
    free(pst_Task);
    return 0;
}

int SS_TaskList_Task_Had_Done(uint8_t taskId)
{
    SS_TaskList_t *pstDoingTask = SS_TaskList_Get_a_Doning_Task_By_TaskId(taskId);
    if (pstDoingTask)
    {
        BOOL bEmpty = TRUE;
        SS_TaskList_Free_a_Doing_Task(pstDoingTask);
        bEmpty = list_empty(&(st_Doing_TaskList_t.list));
        if (bEmpty)
        {
            return 0;
        }
    }
    return -1;
}

int SS_TaskList_Is_Cmd_Doing(SS_TaskList_t **ppst_Task, SS_TASK_e eTask)
{
    *ppst_Task = SS_TaskList_Get_a_Doning_Task_By_TaskType(eTask);
    if (*ppst_Task)
    {
        return TRUE;
    }
    return FALSE;
}

int SS_TaskList_Is_Empty(void)
{
    return list_empty(&(st_Doing_TaskList_t.list)) && list_empty(&(st_Pending_TaskList_t.list));
}

volatile BOOL bCanAutoSend = FALSE;

int SS_TaskList_CheckCmd_Doing_Or_Replace_New_Cmd(SS_TASK_e eIsDoingTask, SS_TASK_e eNewAddTask)
{
    SS_TaskList_t *pstDoingTask = NULL;

    if (SS_TaskList_Is_Cmd_Doing(&pstDoingTask, eIsDoingTask))
    {
        SS_TaskList_Free_a_Doing_Task(pstDoingTask);
        SS_TaskList_NewAdd_Doing_Task_Head(eNewAddTask);
        pstDoingTask = SS_TaskList_Get_a_Doning_Task_By_TaskType(eNewAddTask);
        if (!pstDoingTask)
        {
            DEBUG_ERROR("CheckCmd pstDoingTask must can't be NULL\r\n");
            return -1;
        }
        else
        {
            bCanAutoSend = TRUE;
        }
    }
    else
    {
        if (!SS_TaskList_Is_Cmd_Doing(&pstDoingTask, eNewAddTask))
        {
            SS_TaskList_NewAdd_Pending_Task(eIsDoingTask);
        }
    }
    return 0;
}

void SS_TaskList_Auto_Send_Stop_Cmd_Task(void)
{
    SS_TaskList_t *pstDoingTask = NULL;
    if (bCanAutoSend)
    {
        pstDoingTask = SS_TaskList_Get_a_Doning_Task_By_TaskType(E_TASK_STOP_REC);
        if (!pstDoingTask)
        {
            pstDoingTask = SS_TaskList_Get_a_Doning_Task_By_TaskType(E_TASK_STOP_TRANS);
        }

        if (pstDoingTask && pstDoingTask->bHadSend == FALSE)
        {
            DEBUG_INFO("send task:%d by uart\r\n", pstDoingTask->task);
            SS_Factory_Send_Task_Protrcol_To_Uart(pstDoingTask);
            pstDoingTask->bHadSend == TRUE;
        }

        bCanAutoSend = FALSE;
    }
}
