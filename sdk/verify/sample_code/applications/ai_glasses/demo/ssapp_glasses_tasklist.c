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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "list.h"
#include "ssapp_glasses_tasklist.h"

struct list_head g_stPendingWorkList = {0};
struct list_head g_stDoingWorkList   = {0};

MI_S32 SSAPP_GLASSES_TASKLIST_Init(void)
{
    INIT_LIST_HEAD(&g_stPendingWorkList);
    INIT_LIST_HEAD(&g_stDoingWorkList);

    return MI_SUCCESS;
}

void SSAPP_GLASSES_TASKLIST_AddToDoingWorkList(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace)
{
    if (!pstFactoryWorkSpace)
    {
        printf("add doing pstFactoryWorkSpace is NULL\n");
        return;
    }
    list_del(&pstFactoryWorkSpace->workList);
    list_add_tail(&pstFactoryWorkSpace->workList, &g_stDoingWorkList);
}

void SSAPP_GLASSES_TASKLIST_DelList(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace)
{
    if (!pstFactoryWorkSpace)
    {
        printf("del list pstFactoryWorkSpace is NULL\n");
        return;
    }
    list_del(&pstFactoryWorkSpace->workList);
}

void SSAPP_GLASSES_TASKLIST_AddToPendingWorkList(SSAPP_GLASSES_FACTORY_WorkSpace_t *pstFactoryWorkSpace)
{
    if (!pstFactoryWorkSpace)
    {
        printf("add pending pstFactoryWorkSpace is NULL\n");
        return;
    }
    list_add_tail(&pstFactoryWorkSpace->workList, &g_stPendingWorkList);
}

SSAPP_GLASSES_FACTORY_WorkSpace_t *SSAPP_GLASSES_TASKLIST_GetPendingWork(void)
{
    if (list_empty(&g_stPendingWorkList))
    {
        return NULL;
    }
    return list_first_entry(&g_stPendingWorkList, SSAPP_GLASSES_FACTORY_WorkSpace_t, workList);
}

SSAPP_GLASSES_FACTORY_WorkSpace_t *SSAPP_GLASSES_TASKLIST_GetDoingWork(void)
{
    if (list_empty(&g_stDoingWorkList))
    {
        return NULL;
    }
    return list_first_entry(&g_stDoingWorkList, SSAPP_GLASSES_FACTORY_WorkSpace_t, workList);
}

BOOL SSAPP_GLASSES_TASKLIST_WorkListIsEmpty(void)
{
    return list_empty(&g_stDoingWorkList) && list_empty(&g_stPendingWorkList);
}

BOOL SSAPP_GLASSES_TASKLIST_PendingWorkListIsEmpty(void)
{
    return list_empty(&g_stPendingWorkList);
}
