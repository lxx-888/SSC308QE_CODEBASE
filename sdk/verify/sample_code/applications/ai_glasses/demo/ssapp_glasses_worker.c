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

#include "ssapp_glasses_worker.h"
#include "ssapp_glasses_tasklist.h"
#include "ssapp_glasses_wifi_trans.h"
#include "ssapp_glasses_misc.h"
#include "ssapp_glasses_media.h"

extern SSAPP_GLASSES_FACTORY_DebugParam_t g_stDebugParam;

MI_S32 SSAPP_GLASSES_WORKER_HandleUserPointer(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    UNUSED(pstFactoryWorkSpace);
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_NullStopTask(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    UNUSED(pstFactoryWorkSpace);
    // do nothing
    printf("communication or timing problem? please check it\n");
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_DealTaskNone(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (!pstFactoryWorkSpace)
    {
        printf("[%s] pstFactoryWorkSpace is NULL\n", __FUNCTION__);
    }
    pstFactoryWorkSpace->bNeedWaitDone    = FALSE;
    pstFactoryWorkSpace->bNoReplayMcuDone = TRUE;

    printf("there is no task from mcu, we can check and suspend!!\n");
    if (SSAPP_GLASSES_TASKLIST_PendingWorkListIsEmpty())
    {
        if (!g_stDebugParam.bDisableSuspend)
        {
            printf("going to suspend\n");
            // will send suspending state && go to str
            SSAPP_GLASSES_FACTORY_SendCmdToMcuUart(E_CMD_TYPE_ACK, E_TASK_HEARTBEAT, FALSE, E_STATE_SUSPENDING, 0xff);
            SSAPP_GLASSES_MISC_Suspend();
        }
        else
        {
            printf("sleep 10s replace str\n");
            sleep(10);
        }
    }
    else
    {
        printf("worklist not empty, why we got none tasks?\n");
        s32Ret = -1;
    }

    return s32Ret;
}

MI_S32 SSAPP_GLASSES_WORKER_DealTaskPhotoDestory(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    UNUSED(pstFactoryWorkSpace);
    printf("we are going to destory the photo pipe!!\n");
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_DealTaskPhoto(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    MI_U8 u8PhotoCnt = 1;
    if (!pstFactoryWorkSpace)
    {
        printf("[%s] pstFactoryWorkSpace is NULL\n", __FUNCTION__);
    }
    pstFactoryWorkSpace->bNeedWaitDone  = FALSE; // maybe no need wait if take photo is blocking before done
    pstFactoryWorkSpace->workerCleanFun = SSAPP_GLASSES_WORKER_DealTaskPhotoDestory;
    /* [0 - 7][8 - 15] */
    /* taskId photoCnt */
    u8PhotoCnt                                       = pstFactoryWorkSpace->stProtcolTask.u16UserDefine >> 0x8;
    pstFactoryWorkSpace->stProtcolTask.u16UserDefine = pstFactoryWorkSpace->stProtcolTask.u16UserDefine & 0xff;

    printf("we got a photo task, cnt:%d let's do it!!\n", u8PhotoCnt);
    SSAPP_GLASSES_MEDIA_TakePhoto(u8PhotoCnt);

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_DealTaskRecDestory(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    UNUSED(pstFactoryWorkSpace);
    printf("we are going to destory the recording pipe!!\n");
    SSAPP_GLASSES_MEDIA_StopRec();
    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_DealTaskStartRec(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    if (!pstFactoryWorkSpace)
    {
        printf("[%s] pstFactoryWorkSpace is NULL\n", __FUNCTION__);
    }
    pstFactoryWorkSpace->bNeedWaitDone  = TRUE;
    pstFactoryWorkSpace->workerCleanFun = SSAPP_GLASSES_WORKER_DealTaskRecDestory;

    // if photo pipe exist, we need destory it first
    /*     pstFactoryWorkSpace->userPointer = (MI_VIRT)ST_Glasses_Photo_Stop_Pipe_If_Exist_Thread(); */
    /* SSAPP_GLASSES_WORKER_HandleUserPointer(pstFactoryWorkSpace); */
    SSAPP_GLASSES_MEDIA_StartRec();

    printf("we got a recording task, let's do it!!\n");

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_DealTaskTransDestory(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    UNUSED(pstFactoryWorkSpace);
    printf("we are going to destory the trans pipe!!\n");
    SSAPP_GLASSES_WIFI_TRANS_Stop();
    /* SSAPP_GLASSES_WORKER_HandleUserPointer(pstFactoryWorkSpace); */

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_DealTaskStartTrans(SSAPP_GLASSES_FACTORY_WorkSpace_t* pstFactoryWorkSpace)
{
    if (!pstFactoryWorkSpace)
    {
        printf("[%s] pstFactoryWorkSpace is NULL\n", __FUNCTION__);
    }
    pstFactoryWorkSpace->bNeedWaitDone  = TRUE;
    pstFactoryWorkSpace->workerCleanFun = SSAPP_GLASSES_WORKER_DealTaskTransDestory;
    /* pstFactoryWorkSpace->userPointer    = (MI_VIRT)ST_Glasses_Photo_Stop_Pipe_If_Exist_Thread(); */

    printf("we got a Trans task, let's do it!!\n");
    SSAPP_GLASSES_WIFI_TRANS_Start();

    return MI_SUCCESS;
}

MI_S32 SSAPP_GLASSES_WORKER_Init(void)
{
    SSAPP_GLASSES_MISC_Init();
    // ai_glasses lib ptree init
    SSAPP_GLASSES_MEDIA_Init();

    SSAPP_GLASSES_WIFI_TRANS_Init();
    return MI_SUCCESS;
}
MI_S32 SSAPP_GLASSES_WORKER_DeInit(void)
{
    SSAPP_GLASSES_MEDIA_DeInit();

    SSAPP_GLASSES_MISC_DeInit();

    return MI_SUCCESS;
}
