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

#include <assert.h>
#include <elf.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "mi_common_datatype.h"
#include "st_common.h"
#include "st_common_font.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_rgn.h"
#include "st_common_dla_det.h"
#include "st_common_aov.h"

typedef struct ST_AovInputParam_s
{
    MI_U8             u8SensorIndex;
    MI_BOOL           bAutoTest;
    MI_BOOL           bDemonstrate;
    MI_BOOL           bStressTest;
    MI_BOOL           bAuidoEnable;
    MI_BOOL           bAuidoAoEnable;
    MI_BOOL           bRealTimeEnable;
    char              au8IqBinBrightPath[256];
    char              au8IqBinDarkPath[256];
    char              au8DetectModelPath[MAX_DET_STRLEN];
    char              au8DumpPath[256];
    char              au8AiFile[256];
    char              au8AoFile[256];
    char              auVencType[256];
    MI_VENC_ModType_e eVencType;
    MI_U32            u32MaxStreamBufferNum;
    MI_U32            u32WriteStreamTriggerNum;
    MI_BOOL           bEnableSWLightSensor;
    MI_BOOL           bEnableHWLightSensor;
    MI_BOOL           bEnableFastAE;
    MI_BOOL           bUsingMcu;
    MI_U32            u32SensorNum;
    MI_U32            u32RunTime;
} ST_AovInputParam_t;

typedef struct ST_AovThreadParam_s
{
    MI_BOOL bProcessExit;
    MI_BOOL bReadKeyBoardThreadExit;
    MI_BOOL bStreamProduceThreadExit;
    MI_BOOL bStreamConsumeThreadExit;
    MI_BOOL bStreamDetThreadExit;
    MI_BOOL bDetStateSwitchThreadExit;
    MI_BOOL bAudioThreadExit;
} ST_AovThreadParam_t;

static ST_Common_AovHandle_t       gstAovHandle;
static ST_Common_AovStreamHandle_t gstStreamHandle;
static ST_AovInputParam_t          gstAovInputParm;
static ST_AovThreadParam_t         gstAovThreadParm;

static pthread_cond_t  gCond_StateSwitchDoneForProduce  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_StateSwitchDoneForProduce = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_ProduceStart               = TRUE;

static pthread_cond_t  gCond_ProduceDone  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_ProduceDone = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_ProduceDone  = FALSE;

static pthread_cond_t  gCond_StateSwitchDoneForDet  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_StateSwitchDoneForDet = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_DetStart               = TRUE;

static pthread_cond_t  gCond_DetDone  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_DetDone = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_DetDone  = FALSE;

static pthread_cond_t  gCond_StateSwitchDoneForConsume  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_StateSwitchDoneForConsume = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_ConsumeStart               = FALSE;

static pthread_cond_t  gCond_ConsumeDone  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_ConsumeDone = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_ConsumeDone  = FALSE;

static MI_BOOL bFlag_ConsumeWaitBegin = FALSE;
static MI_BOOL bFlag_InConsume        = FALSE;

static pthread_cond_t  gCond_StateSwitchDoneForAudio  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_StateSwitchDoneForAudio = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_AudioStart               = FALSE;

static pthread_cond_t  gCond_AudioDone  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gMutex_AudioDone = PTHREAD_MUTEX_INITIALIZER;
static MI_BOOL         bFlag_AudioDone  = FALSE;

static MI_U32          g_u32AovStressSuspendCnt     = 0;
static MI_U32          g_u32AovStressBackupKmsgCnt  = 0;
static MI_BOOL         bFlag_AovStressOffSnr        = FALSE;
static ST_WakeupType_e gstAovStressForceNextWkType  = E_ST_WAKEUP_TIMER;

void ST_QuitHandler()
{
    printf("Enter ST_QuitHandler\n");

    gstAovThreadParm.bDetStateSwitchThreadExit = TRUE;
    while (TRUE == gstAovThreadParm.bDetStateSwitchThreadExit)
    {
        usleep(100 * 1000);
        continue;
    }

    gstAovThreadParm.bStreamProduceThreadExit = TRUE;
    pthread_mutex_lock(&gMutex_StateSwitchDoneForProduce);
    bFlag_ProduceStart = TRUE;
    pthread_cond_signal(&gCond_StateSwitchDoneForProduce);
    pthread_mutex_unlock(&gMutex_StateSwitchDoneForProduce);
    while (TRUE == gstAovThreadParm.bStreamProduceThreadExit)
    {
        usleep(100 * 1000);
        continue;
    }

    gstAovThreadParm.bStreamDetThreadExit = TRUE;
    pthread_mutex_lock(&gMutex_StateSwitchDoneForDet);
    bFlag_DetStart = TRUE;
    pthread_cond_signal(&gCond_StateSwitchDoneForDet);
    pthread_mutex_unlock(&gMutex_StateSwitchDoneForDet);
    while (TRUE == gstAovThreadParm.bStreamDetThreadExit)
    {
        usleep(100 * 1000);
        continue;
    }

    if (TRUE == gstAovHandle.stAovPipeAttr.stAudioParam.bEnable)
    {
        gstAovThreadParm.bAudioThreadExit                 = TRUE;
        gstAovHandle.stAovPipeAttr.stAudioParam.bAudioRun = FALSE;
        pthread_mutex_lock(&gMutex_StateSwitchDoneForAudio);
        bFlag_AudioStart = TRUE;
        pthread_cond_signal(&gCond_StateSwitchDoneForAudio);
        pthread_mutex_unlock(&gMutex_StateSwitchDoneForAudio);
        while (TRUE == gstAovThreadParm.bAudioThreadExit)
        {
            usleep(100 * 1000);
            continue;
        }
    }

    gstAovThreadParm.bStreamConsumeThreadExit = TRUE;
    pthread_mutex_lock(&gMutex_StateSwitchDoneForConsume);
    bFlag_ConsumeStart = TRUE;
    pthread_cond_signal(&gCond_StateSwitchDoneForConsume);
    pthread_mutex_unlock(&gMutex_StateSwitchDoneForConsume);
    while (TRUE == gstAovThreadParm.bStreamConsumeThreadExit)
    {
        usleep(100 * 1000);
        continue;
    }
    ST_Common_AovStreamConsume(&gstAovHandle, &gstStreamHandle);

    gstAovThreadParm.bReadKeyBoardThreadExit = TRUE;
    gstAovThreadParm.bProcessExit            = TRUE;
}

void *ST_ReadKeyBoardThread(void *args)
{
    struct timeval    stcurrent        = {0, 0};
    struct timeval    stbegin          = {0, 0};
    MI_U32 u32DiffTime                 = 0;
    gettimeofday(&stbegin, NULL);
    while (FALSE == gstAovThreadParm.bReadKeyBoardThreadExit)
    {
        if (gstAovInputParm.u32RunTime > 0)
        {
            gettimeofday(&stcurrent, NULL);
            u32DiffTime = ST_Common_CalcDiffTime_MS(&(stbegin), &stcurrent);
            if(u32DiffTime/1000 < gstAovInputParm.u32RunTime)
            {
               sleep(0.5);
               continue;
            }
            printf("app run time is end !\n");
            ST_QuitHandler();
        }
        else
        {
             fgets(gstAovHandle.stAutoTestParam.au8KeyBoardInput, 256, stdin);
             if ('q' == *gstAovHandle.stAutoTestParam.au8KeyBoardInput)
             {
                ST_QuitHandler();
             }
        }
    }

    printf("Exit ST_ReadKeyBoardThread\n");
    gstAovThreadParm.bReadKeyBoardThreadExit = FALSE;

    return NULL;
}

void *ST_StreamProduceThread(void *args)
{
    MI_S32 s32Ret = MI_SUCCESS;

    while (FALSE == gstAovThreadParm.bStreamProduceThreadExit)
    {
        pthread_mutex_lock(&gMutex_StateSwitchDoneForProduce);
        while (FALSE == bFlag_ProduceStart)
        {
            pthread_cond_wait(&gCond_StateSwitchDoneForProduce, &gMutex_StateSwitchDoneForProduce);
        }
        bFlag_ProduceStart = FALSE;
        pthread_mutex_unlock(&gMutex_StateSwitchDoneForProduce);
        if (FALSE == gstAovThreadParm.bStreamProduceThreadExit)
        {
            CHECK_AOV_RESULT(ST_Common_AovStreamProduce(&gstAovHandle, &gstStreamHandle), s32Ret, EXIT);

            pthread_mutex_lock(&gMutex_ProduceDone);
            bFlag_ProduceDone = TRUE;
            pthread_cond_signal(&gCond_ProduceDone);
            pthread_mutex_unlock(&gMutex_ProduceDone);
        }
    }

EXIT:
    printf("Exit ST_StreamProduceThread\n");
    gstAovThreadParm.bStreamProduceThreadExit = FALSE;

    return NULL;
}

void *ST_DetectThread(void *args)
{
    MI_S32 s32Ret = MI_SUCCESS;

    while (FALSE == gstAovThreadParm.bStreamDetThreadExit)
    {
        pthread_mutex_lock(&gMutex_StateSwitchDoneForDet);
        while (FALSE == bFlag_DetStart)
        {
            pthread_cond_wait(&gCond_StateSwitchDoneForDet, &gMutex_StateSwitchDoneForDet);
        }
        bFlag_DetStart = FALSE;
        pthread_mutex_unlock(&gMutex_StateSwitchDoneForDet);

        gstAovHandle.eLastDetResult = gstAovHandle.eCurrentDetResult;

        if (TRUE == gstAovHandle.stAovSWLightSensorParam.bCurrentRegardedStable)
        {
            CHECK_AOV_RESULT(ST_Common_AovDetect(&gstAovHandle), s32Ret, EXIT);
        }

        pthread_mutex_lock(&gMutex_DetDone);
        bFlag_DetDone = TRUE;
        pthread_cond_signal(&gCond_DetDone);
        pthread_mutex_unlock(&gMutex_DetDone);
    }

EXIT:
    printf("Exit ST_DetectThread\n");
    gstAovThreadParm.bStreamDetThreadExit = FALSE;

    return NULL;
}

void *ST_StreamConsumeThread(void *args)
{
    MI_S32 s32Ret = MI_SUCCESS;

    while (FALSE == gstAovThreadParm.bStreamConsumeThreadExit)
    {
        pthread_mutex_lock(&gMutex_StateSwitchDoneForConsume);
        while (FALSE == bFlag_ConsumeStart)
        {
            pthread_cond_wait(&gCond_StateSwitchDoneForConsume, &gMutex_StateSwitchDoneForConsume);
        }
        bFlag_InConsume    = TRUE;
        bFlag_ConsumeStart = FALSE;

        pthread_mutex_unlock(&gMutex_StateSwitchDoneForConsume);

        CHECK_AOV_RESULT(ST_Common_AovStreamConsume(&gstAovHandle, &gstStreamHandle), s32Ret, EXIT);

        pthread_mutex_lock(&gMutex_ConsumeDone);

        bFlag_ConsumeDone = TRUE;

        pthread_cond_signal(&gCond_ConsumeDone);
        pthread_mutex_unlock(&gMutex_ConsumeDone);
    }

EXIT:
    printf("Exit ST_StreamConsumeThread\n");
    gstAovThreadParm.bStreamConsumeThreadExit = FALSE;

    return NULL;
}

void *ST_AudioThread(void *args)
{
    MI_S32 s32Ret = MI_SUCCESS;

    while (FALSE == gstAovThreadParm.bAudioThreadExit)
    {
        pthread_mutex_lock(&gMutex_StateSwitchDoneForAudio);
        while (FALSE == bFlag_AudioStart)
        {
            pthread_cond_wait(&gCond_StateSwitchDoneForAudio, &gMutex_StateSwitchDoneForAudio);
        }
        pthread_mutex_unlock(&gMutex_StateSwitchDoneForAudio);
        printf("audio start .\n");

        CHECK_AOV_RESULT(ST_Common_AovDoAudio(&gstAovHandle), s32Ret, EXIT);

        pthread_mutex_lock(&gMutex_AudioDone);
        bFlag_AudioStart = FALSE;
        bFlag_AudioDone  = TRUE;
        pthread_cond_signal(&gCond_AudioDone);
        pthread_mutex_unlock(&gMutex_AudioDone);
        printf("audio stop .\n");
    }

EXIT:
    printf("Exit ST_AudioThread\n");
    gstAovThreadParm.bAudioThreadExit = FALSE;

    return NULL;
}

void *ST_ConsumeWaitThread(void *args)
{
    pthread_mutex_lock(&gMutex_ConsumeDone);
    while (FALSE == bFlag_ConsumeDone)
    {
        pthread_cond_wait(&gCond_ConsumeDone, &gMutex_ConsumeDone);
    }
    bFlag_ConsumeDone = FALSE;
    bFlag_InConsume   = FALSE;
    pthread_mutex_unlock(&gMutex_ConsumeDone);

    bFlag_ConsumeWaitBegin = FALSE;

    return NULL;
}

void *ST_StateSwitchThread(void *args)
{
    MI_S32            s32Ret           = MI_SUCCESS;
    struct timeval    stcurrent        = {0, 0};
    struct timeval    stbegin          = {0, 0};
    struct timeval    stPreview        = {0, 0};
    struct timeval    stResumeTimeDiff = {0, 0};
    MI_SYS_ChnPort_t  stSrcChnPort;
    ST_BatteryLevel_e eBatteryLevel;
    ST_RemoteStatus_e eRemoteStatus;
    MI_S32            s32DiffTime;
    pthread_t         pConsumeWaitThread;
    MI_BOOL           bThreadNeedFree = FALSE;
    MI_U32            u32StreamBuffersize =
        (gstAovHandle.stAovPipeAttr.u32MaxStreamBufferSize / gstAovHandle.stAovPipeAttr.u32MaxStreamBufferNum)
        * VENC_STREAM_POOL_BUFFER;

    struct timespec   abstime;
    int               ret;
    char              cmd[64];

    gettimeofday(&(gstAovHandle.stResumeTime), NULL);
    gettimeofday(&stPreview, NULL);

    while (FALSE == gstAovThreadParm.bDetStateSwitchThreadExit)
    {
        pthread_mutex_lock(&gMutex_ProduceDone);
        while (FALSE == bFlag_ProduceDone)
        {
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_nsec += STRESS_TIMEOUT_MS_COND_WAIT * 1000 * 1000;
            if (abstime.tv_nsec >= 1000000000)
            {
                //Overflow prevention
                abstime.tv_sec += abstime.tv_nsec / 1000000000;
                abstime.tv_nsec %= 1000000000;
            }
            ret = gstAovHandle.bStressTest ? /* Use pthread_cond_timedwait with a timeout mechanism for pressure testing */
                pthread_cond_timedwait(&gCond_ProduceDone, &gMutex_ProduceDone, &abstime) :
                pthread_cond_wait(&gCond_ProduceDone, &gMutex_ProduceDone);
            if (0 != ret)
            {
                switch (ret)
                {
                    case ETIMEDOUT:
                        printf("Condition wait timed out(%dms).[gCond_ProduceDone]\n", STRESS_TIMEOUT_MS_COND_WAIT);
                        break;
                    case EINVAL:
                        printf("invalid param![gCond_ProduceDone]\n");
                        break;
                    default:
                        printf("Error waiting for condition.[gCond_ProduceDone][ret=%d]\n", ret);
                }
                break;
            }
        }
        bFlag_ProduceDone = FALSE;
        pthread_mutex_unlock(&gMutex_ProduceDone);

        pthread_mutex_lock(&gMutex_DetDone);
        while (FALSE == bFlag_DetDone)
        {
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_nsec += STRESS_TIMEOUT_MS_COND_WAIT * 1000 * 1000;
            if (abstime.tv_nsec >= 1000000000)
            {
                abstime.tv_sec += abstime.tv_nsec / 1000000000;
                abstime.tv_nsec %= 1000000000;
            }
            ret = gstAovHandle.bStressTest ?
                pthread_cond_timedwait(&gCond_DetDone, &gMutex_DetDone, &abstime) :
                pthread_cond_wait(&gCond_DetDone, &gMutex_DetDone);
            if (0 != ret)
            {
                switch (ret)
                {
                    case ETIMEDOUT:
                        printf("Condition wait timed out(%dms).[gCond_DetDone]\n", STRESS_TIMEOUT_MS_COND_WAIT);
                        break;
                    case EINVAL:
                        printf("invalid param![gCond_DetDone]\n");
                        break;
                    default:
                        printf("Error waiting for condition.[gCond_DetDone][ret=%d]\n", ret);
                }
                break;
            }
        }
        bFlag_DetDone = FALSE;
        pthread_mutex_unlock(&gMutex_DetDone);

        if ((TRUE == gstAovHandle.bDemonstrate) && ('w' == *gstAovHandle.stAutoTestParam.au8KeyBoardInput))
        {
            printf("\033[32m===> Will write es file\n\033[0m");
            gstAovHandle.bDemonstrate = FALSE;
        }
        switch (gstAovHandle.eCurrentWakeupType)
        {
            case E_ST_WAKEUP_TIMER:
                if (gstAovHandle.bStressTest && gstAovStressForceNextWkType != E_ST_WAKEUP_TIMER && bFlag_AovStressOffSnr)
                {
                    gstAovHandle.eCurrentWakeupType = gstAovStressForceNextWkType;
                    continue;
                }

                if (E_ST_DET_UNDETECTED == gstAovHandle.eCurrentDetResult)
                {
                    CHECK_AOV_RESULT(ST_Common_AovSetLowFps(&gstAovHandle), s32Ret, EXIT);

                    // Run ST_ConsumeWaitThread
                    if ((TRUE == bFlag_InConsume) && (FALSE == bFlag_ConsumeWaitBegin))
                    {
                        int pthreadret         = 0;
                        bFlag_ConsumeWaitBegin = TRUE;
                        pthreadret             = pthread_create(&pConsumeWaitThread, NULL, ST_ConsumeWaitThread, NULL);
                        if (pthreadret != 0)
                        {
                            printf("[%d]error! pthreadret:%d\n", __LINE__, pthreadret);
                            bFlag_ConsumeWaitBegin = FALSE;
                        }
                        else
                        {
                            bThreadNeedFree = TRUE;
                        }
                    }

                    if ((TRUE == gstAovHandle.stAovPipeAttr.stAudioParam.bEnable) && (TRUE == bFlag_AudioStart))
                    {
                        gstAovHandle.stAovPipeAttr.stAudioParam.bAudioRun = FALSE;

                        pthread_mutex_lock(&gMutex_AudioDone);
                        while (FALSE == bFlag_AudioDone)
                        {
                            pthread_cond_wait(&gCond_AudioDone, &gMutex_AudioDone);
                        }
                        bFlag_AudioDone = FALSE;
                        pthread_mutex_unlock(&gMutex_AudioDone);
                    }

                    eBatteryLevel =
                        ST_Common_AovBatteryLevelCheck((MI_U8 *)gstAovHandle.stAutoTestParam.au8KeyBoardInput);
                    if (E_ST_BATTERYLEVEL_LOW == eBatteryLevel)
                    {
                        ST_Common_AovEnterLowPowerMode(&gstAovHandle);
                    }

                    gettimeofday(&stcurrent, NULL);
                    s32DiffTime = ST_Common_CalcDiffTime_MS(&stbegin, &stcurrent);

                    if ((TRUE == gstAovHandle.stAovSWLightSensorParam.bEnableSWLightSensor)
                        && (FALSE == gstAovHandle.stAovSWLightSensorParam.bCurrentRegardedStable))
                    {
                        printf("Continue to handle the next frame while AE unstable\n");

                        // Disable sleep mode
                        gstAovHandle.stSNRSleepParam.bSleepEnable = FALSE;
                        for (MI_U32 i = 0; i < gstAovHandle.stAovPipeAttr.u32SensorNum; i++)
                        {
                            MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                                        E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                                         &gstAovHandle.stSNRSleepParam);
                        }
                        // Change the port depth for high fps
                        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
                        stSrcChnPort.eModId    = gstAovHandle.stAovPipeAttr.stChnPortSclDetect[0].eModId;
                        stSrcChnPort.u32DevId  = gstAovHandle.stAovPipeAttr.stChnPortSclDetect[0].u32DevId;
                        stSrcChnPort.u32ChnId  = gstAovHandle.stAovPipeAttr.stChnPortSclDetect[0].u32ChnId;
                        stSrcChnPort.u32PortId = gstAovHandle.stAovPipeAttr.stChnPortSclDetect[0].u32PortId;
                        CHECK_AOV_RESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 2, 4), s32Ret, EXIT);
                    }
                    else
                    {
                        do
                        {
                            gettimeofday(&stResumeTimeDiff, NULL);
                            s32DiffTime = ST_Common_CalcDiffTime_MS(&(gstAovHandle.stResumeTime), &stResumeTimeDiff);

                            if ((s32DiffTime < (1000 / FPS_LOW)) || (TRUE == gstAovHandle.stFastAEParam.bIsDoFastAE))
                            {
                                if (FALSE == bFlag_ConsumeWaitBegin)
                                {
                                    if (bThreadNeedFree)
                                    {
                                        pthread_join(pConsumeWaitThread, NULL);
                                        bThreadNeedFree = FALSE;
                                    }

                                    if(gstAovHandle.bStressTest)
                                    {
                                        g_u32AovStressSuspendCnt < INT_MAX ? g_u32AovStressSuspendCnt++ : (g_u32AovStressSuspendCnt = 0);
                                        bFlag_AovStressOffSnr = FALSE;
                                        if (g_u32AovStressSuspendCnt % STRESS_TRIG_CMDQTIMEOUT_INTERVAL == 0)
                                        {
                                            for (MI_U32 j = 0; j < gstAovHandle.stAovPipeAttr.u32SensorNum; j++)
                                            {
                                                memset(cmd, 0x0, sizeof(cmd));
                                                snprintf(cmd, sizeof(cmd),
                                                    "echo offsnr %d %d %d > /proc/mi_modules/mi_vif/mi_vif%d",
                                                    gstAovHandle.stAovPipeAttr.stVifParam[j].u32VifDevId,
                                                    gstAovHandle.stAovPipeAttr.stVifParam[j].u32VifPortId,
                                                    TRUE,
                                                    gstAovHandle.stAovPipeAttr.stVifParam[j].u32VifDevId);
                                                ST_Common_AovStressExecSysCmd(cmd);
                                                printf("Current suspend cnt:%d\n",g_u32AovStressSuspendCnt);
                                                bFlag_AovStressOffSnr = TRUE;
                                                usleep(50 * 1000);
                                            }
                                        }
                                        g_u32AovStressBackupKmsgCnt++;
                                        if (g_u32AovStressBackupKmsgCnt >= STRESS_KMSG_BACKUP_RESUMECNT_CHK)
                                        {
                                            g_u32AovStressBackupKmsgCnt = 0;
                                            printf("Time to check if kmsg needs backup.\n");
                                            if (MI_SUCCESS == ST_Common_AovStressBackupKmsg(STRESS_PROCESS_NAME, STRESS_PROC_KMSG, STRESS_KMSG_FILE, STRESS_MOUNT_DIR))
                                            {
                                                printf("Backup kmsg to %s done!(timer)\n", STRESS_MOUNT_DIR);
                                            }
                                            usleep(50 * 1000);
                                        }
                                    }

                                    CHECK_AOV_RESULT(ST_Common_AovEnterSuspend(&gstAovHandle), s32Ret, EXIT);
                                    break;
                                }

                                usleep(1 * 1000);
                            }
                            else
                            {
                                if(gstAovHandle.bStressTest)
                                    break;//if dont break, it will keep trying to turn off the sleep mode of the sensor until it times out

                                printf("Continue to handle the next frame while something timeout\n");

                                for (MI_U32 i = 0; i < gstAovHandle.stAovPipeAttr.u32SensorNum; i++)
                                {
                                    // Disable sleep mode
                                    gstAovHandle.stSNRSleepParam.bSleepEnable = FALSE;
                                    MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                                        E_MI_VIF_CUSTCMD_SLEEPPARAM_SET,
                                                        sizeof(ST_Common_SNRSleepParam_t),
                                                        &gstAovHandle.stSNRSleepParam);

                                    // Configure sensor to enable sleep mode & consecutive frames cnt
                                    gstAovHandle.stSNRSleepParam.bSleepEnable           = TRUE;
                                    gstAovHandle.stSNRSleepParam.u32FrameCntBeforeSleep = 1;
                                    MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                                        E_MI_VIF_CUSTCMD_SLEEPPARAM_SET,
                                                        sizeof(ST_Common_SNRSleepParam_t),
                                                        &gstAovHandle.stSNRSleepParam);
                                }

                                break;
                            }
                        } while (1);
                    }

                    gstAovHandle.eLastWakeupType    = gstAovHandle.eCurrentWakeupType;
                    gstAovHandle.eCurrentWakeupType = ST_Common_AovWakeupCheck(&gstAovHandle);
                }
                else
                {
                    CHECK_AOV_RESULT(ST_Common_AovSetHighFps(&gstAovHandle), s32Ret, EXIT);
                }

                break;

            case E_ST_WAKEUP_PIR:
                if (gstAovInputParm.bUsingMcu)
                {
                    ST_Common_AovGpio_Set(GPIO_EVENT_DONE, "out", "1");
                }

                if (E_ST_DET_UNDETECTED == gstAovHandle.eCurrentDetResult)
                {
                    CHECK_AOV_RESULT(ST_Common_AovSetLowFps(&gstAovHandle), s32Ret, EXIT);

                    // Run ST_ConsumeWaitThread
                    if ((TRUE == bFlag_InConsume) && (FALSE == bFlag_ConsumeWaitBegin))
                    {
                        int pthreadret         = 0;
                        bFlag_ConsumeWaitBegin = TRUE;
                        pthreadret             = pthread_create(&pConsumeWaitThread, NULL, ST_ConsumeWaitThread, NULL);
                        if (pthreadret != 0)
                        {
                            printf("[%d]error! pthreadret:%d\n", __LINE__, pthreadret);
                            bFlag_ConsumeWaitBegin = FALSE;
                        }
                        else
                        {
                            bThreadNeedFree = TRUE;
                        }
                    }

                    if ((TRUE == gstAovHandle.stAovPipeAttr.stAudioParam.bEnable) && (TRUE == bFlag_AudioStart))
                    {
                        gstAovHandle.stAovPipeAttr.stAudioParam.bAudioRun = FALSE;

                        pthread_mutex_lock(&gMutex_AudioDone);
                        while (FALSE == bFlag_AudioDone)
                        {
                            pthread_cond_wait(&gCond_AudioDone, &gMutex_AudioDone);
                        }
                        bFlag_AudioDone = FALSE;
                        pthread_mutex_unlock(&gMutex_AudioDone);
                    }

                    eBatteryLevel =
                        ST_Common_AovBatteryLevelCheck((MI_U8 *)gstAovHandle.stAutoTestParam.au8KeyBoardInput);
                    if (E_ST_BATTERYLEVEL_LOW == eBatteryLevel)
                    {
                        ST_Common_AovEnterLowPowerMode(&gstAovHandle);
                    }

                    do
                    {
                        gettimeofday(&stResumeTimeDiff, NULL);
                        s32DiffTime = ST_Common_CalcDiffTime_MS(&(gstAovHandle.stResumeTime), &stResumeTimeDiff);

                        if ((s32DiffTime < (1000 / FPS_LOW)) || (TRUE == gstAovHandle.stFastAEParam.bIsDoFastAE))
                        {
                            if (FALSE == bFlag_ConsumeWaitBegin)
                            {
                                if (bThreadNeedFree)
                                {
                                    pthread_join(pConsumeWaitThread, NULL);
                                    bThreadNeedFree = FALSE;
                                }

                                CHECK_AOV_RESULT(ST_Common_AovEnterSuspend(&gstAovHandle), s32Ret, EXIT);
                                break;
                            }

                            usleep(1 * 1000);
                        }
                        else
                        {
                            printf("Continue to handle the next frame while something timeout\n");

                            for (MI_U32 i = 0; i < gstAovHandle.stAovPipeAttr.u32SensorNum; i++)
                            {
                                // Disable sleep mode
                                gstAovHandle.stSNRSleepParam.bSleepEnable = FALSE;
                                MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                                    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                                    &gstAovHandle.stSNRSleepParam);

                                // Configure sensor to enable sleep mode & consecutive frames cnt
                                gstAovHandle.stSNRSleepParam.bSleepEnable           = TRUE;
                                gstAovHandle.stSNRSleepParam.u32FrameCntBeforeSleep = 1;
                                MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                                    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                                    &gstAovHandle.stSNRSleepParam);
                            }

                            break;
                        }
                    } while (1);

                    if (gstAovInputParm.bUsingMcu)
                    {
                        ST_Common_AovGpio_Set(GPIO_EVENT_DONE, "out", "0");
                    }

                    gstAovHandle.eLastWakeupType    = gstAovHandle.eCurrentWakeupType;
                    gstAovHandle.eCurrentWakeupType = ST_Common_AovWakeupCheck(&gstAovHandle);
                }
                else
                {
                    CHECK_AOV_RESULT(ST_Common_AovSetHighFps(&gstAovHandle), s32Ret, EXIT);
                }

                break;

            case E_ST_WAKEUP_PREVIEW:
                if (gstAovInputParm.bUsingMcu)
                {
                    ST_Common_AovGpio_Set(GPIO_EVENT_DONE, "out", "1");
                }

                if(gstAovHandle.bStressTest && bFlag_AovStressOffSnr)
                {
                    //If sensor was previously turned off once, it remains off even after cmdq reset in preview mode; STR process needed to reinitialize.
                    printf("Reinit sensor via STR process due to stress test environment.(snr on/off effect in next resume)\n");
                    bFlag_AovStressOffSnr = FALSE;
                    gstAovStressForceNextWkType = E_ST_WAKEUP_PREVIEW;
                    usleep(50 * 1000);
                    CHECK_AOV_RESULT(ST_Common_AovSetLowFps(&gstAovHandle), s32Ret, EXIT);
                    CHECK_AOV_RESULT(ST_Common_AovEnterSuspend(&gstAovHandle), s32Ret, EXIT);
                }
                gstAovStressForceNextWkType = E_ST_WAKEUP_TIMER;

                eRemoteStatus = ST_Common_AovRemoteStatusCheck((MI_U8 *)gstAovHandle.stAutoTestParam.au8KeyBoardInput);

                if (E_ST_REMOTE_DISCONNECT == eRemoteStatus)
                {
                    CHECK_AOV_RESULT(ST_Common_AovSetLowFps(&gstAovHandle), s32Ret, EXIT);

                    // Run ST_ConsumeWaitThread
                    if ((TRUE == bFlag_InConsume) && (FALSE == bFlag_ConsumeWaitBegin))
                    {
                        int pthreadret         = 0;
                        bFlag_ConsumeWaitBegin = TRUE;
                        pthreadret             = pthread_create(&pConsumeWaitThread, NULL, ST_ConsumeWaitThread, NULL);
                        if (pthreadret != 0)
                        {
                            printf("[%d]error! pthreadret:%d\n", __LINE__, pthreadret);
                            bFlag_ConsumeWaitBegin = FALSE;
                        }
                        else
                        {
                            bThreadNeedFree = TRUE;
                        }
                    }

                    if ((TRUE == gstAovHandle.stAovPipeAttr.stAudioParam.bEnable) && (TRUE == bFlag_AudioStart))
                    {
                        gstAovHandle.stAovPipeAttr.stAudioParam.bAudioRun = FALSE;

                        pthread_mutex_lock(&gMutex_AudioDone);
                        while (FALSE == bFlag_AudioDone)
                        {
                            pthread_cond_wait(&gCond_AudioDone, &gMutex_AudioDone);
                        }
                        bFlag_AudioDone = FALSE;
                        pthread_mutex_unlock(&gMutex_AudioDone);
                    }

                    eBatteryLevel =
                        ST_Common_AovBatteryLevelCheck((MI_U8 *)gstAovHandle.stAutoTestParam.au8KeyBoardInput);
                    if (E_ST_BATTERYLEVEL_LOW == eBatteryLevel)
                    {
                        ST_Common_AovEnterLowPowerMode(&gstAovHandle);
                    }

                    do
                    {
                        gettimeofday(&stResumeTimeDiff, NULL);
                        s32DiffTime = ST_Common_CalcDiffTime_MS(&(gstAovHandle.stResumeTime), &stResumeTimeDiff);

                        if ((s32DiffTime < (1000 / FPS_LOW)) || (TRUE == gstAovHandle.stFastAEParam.bIsDoFastAE))
                        {
                            if (FALSE == bFlag_ConsumeWaitBegin)
                            {
                                if (bThreadNeedFree)
                                {
                                    pthread_join(pConsumeWaitThread, NULL);
                                    bThreadNeedFree = FALSE;
                                }

                                CHECK_AOV_RESULT(ST_Common_AovEnterSuspend(&gstAovHandle), s32Ret, EXIT);
                                break;
                            }

                            usleep(1 * 1000);
                        }
                        else
                        {
                            printf("Continue to handle the next frame while something timeout\n");

                            for (MI_U32 i = 0; i < gstAovHandle.stAovPipeAttr.u32SensorNum; i++)
                            {
                                // Disable sleep mode
                                gstAovHandle.stSNRSleepParam.bSleepEnable = FALSE;
                                MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                                    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                                    &gstAovHandle.stSNRSleepParam);

                                // Configure sensor to enable sleep mode & consecutive frames cnt
                                gstAovHandle.stSNRSleepParam.bSleepEnable           = TRUE;
                                gstAovHandle.stSNRSleepParam.u32FrameCntBeforeSleep = 1;
                                MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                                    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                                    &gstAovHandle.stSNRSleepParam);
                            }

                            break;
                        }
                    } while (1);

                    gstAovHandle.eLastWakeupType    = gstAovHandle.eCurrentWakeupType;
                    gstAovHandle.eCurrentWakeupType = ST_Common_AovWakeupCheck(&gstAovHandle);
                }
                else
                {
                    CHECK_AOV_RESULT(ST_Common_AovSetHighFps(&gstAovHandle), s32Ret, EXIT);
                }

                if (gstAovInputParm.bUsingMcu)
                {
                    ST_Common_AovGpio_Set(GPIO_EVENT_DONE, "out", "0");
                }

                if(gstAovHandle.bStressTest)
                {
                    gettimeofday(&stcurrent, NULL);
                    s32DiffTime = ST_Common_CalcDiffTime_MS(&stPreview, &stcurrent);
                    if (s32DiffTime >= 30*1000)
                    {
                        gettimeofday(&stPreview, NULL);
                        memset(cmd, 0x0, sizeof(cmd));
                        snprintf(cmd, sizeof(cmd), "echo set_cmdq_timeout 34 0 0 > /proc/mi_modules/mi_sys/mi_sys0");
                        ST_Common_AovStressExecSysCmd(cmd);
                        usleep(100 * 1000);
                        if (MI_SUCCESS == ST_Common_AovStressBackupKmsg(STRESS_PROCESS_NAME, STRESS_PROC_KMSG, STRESS_KMSG_FILE, STRESS_MOUNT_DIR))
                        {
                            printf("Backup kmsg to %s done!(preview)\n", STRESS_MOUNT_DIR);
                        }
                    }
                }

                break;

            default:
                ST_INFO("ST_Common_AovWakeupCheck fail\n");
                goto EXIT;
        }

        gettimeofday(&stbegin, NULL);
        gettimeofday(&(gstAovHandle.stResumeTime), NULL);

        CHECK_AOV_RESULT(__AovAttachTimestamp(&gstAovHandle), s32Ret, EXIT);

        if (E_ST_WAKEUP_TIMER == gstAovHandle.eCurrentWakeupType)
        {
            if ((gstStreamHandle.u32ListCnt >= gstAovHandle.stAovParam.u32WriteStreamTriggerNum)
                || ((gstStreamHandle.u32StreamSize + u32StreamBuffersize)
                    >= gstAovHandle.stAovPipeAttr.u32MaxStreamBufferSize))
            {
                printf("stream list cnt = %d, strem list buffer size = %d\n", gstStreamHandle.u32ListCnt,
                       gstStreamHandle.u32StreamSize);

                pthread_mutex_lock(&gMutex_StateSwitchDoneForConsume);
                bFlag_ConsumeStart = TRUE;
                pthread_cond_signal(&gCond_StateSwitchDoneForConsume);
                pthread_mutex_unlock(&gMutex_StateSwitchDoneForConsume);
            }
        }
        else
        {
            pthread_mutex_lock(&gMutex_StateSwitchDoneForConsume);
            bFlag_ConsumeStart = TRUE;
            pthread_cond_signal(&gCond_StateSwitchDoneForConsume);
            pthread_mutex_unlock(&gMutex_StateSwitchDoneForConsume);
        }

        pthread_mutex_lock(&gMutex_StateSwitchDoneForProduce);
        bFlag_ProduceStart = TRUE;
        pthread_cond_signal(&gCond_StateSwitchDoneForProduce);
        pthread_mutex_unlock(&gMutex_StateSwitchDoneForProduce);

        pthread_mutex_lock(&gMutex_StateSwitchDoneForDet);
        bFlag_DetStart = TRUE;
        pthread_cond_signal(&gCond_StateSwitchDoneForDet);
        pthread_mutex_unlock(&gMutex_StateSwitchDoneForDet);

        if ((TRUE == gstAovHandle.stAovPipeAttr.stAudioParam.bEnable)
            && (E_ST_FPS_HIGH == gstAovHandle.stAovPipeAttr.eCurrentFps) && (FALSE == bFlag_AudioStart)
            && (FALSE == gstAovHandle.stAutoTestParam.bAutoTest))
        {
            pthread_mutex_lock(&gMutex_StateSwitchDoneForAudio);
            bFlag_AudioStart                                  = TRUE;
            gstAovHandle.stAovPipeAttr.stAudioParam.bAudioRun = TRUE;
            pthread_cond_signal(&gCond_StateSwitchDoneForAudio);
            pthread_mutex_unlock(&gMutex_StateSwitchDoneForAudio);
        }

        if (TRUE == gstAovHandle.stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor)
        {
            ST_Common_AovISPAdjust_HWLightSensor(&gstAovHandle);
        }
    }

EXIT:
    printf("Exit ST_StateSwitchThread\n");
    gstAovThreadParm.bDetStateSwitchThreadExit = FALSE;
    sleep(1);
    ST_Common_SNRSleepParam_t stSNRSleepParam;
    stSNRSleepParam.bSleepEnable = FALSE;
    stSNRSleepParam.u32FrameCntBeforeSleep = 0;
    for (MI_U32 i = 0; i < gstAovHandle.stAovPipeAttr.u32SensorNum; i++)
    {
        MI_VIF_CustFunction(gstAovHandle.stAovPipeAttr.stVifParam[i].u32VifDevId,
                                    E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                                    &stSNRSleepParam);
    }
    return NULL;
}

static MI_S32 ST_AOV_Preview()
{
    pthread_t pStateSwitchThread;
    pthread_t pStreamProduceThread;
    pthread_t pStreamConsumeThread;
    pthread_t pDetectThread;
    pthread_t pReadKeyBoardThread;
    pthread_t pAudioThread;

    gstAovThreadParm.bProcessExit              = FALSE;
    gstAovThreadParm.bReadKeyBoardThreadExit   = FALSE;
    gstAovThreadParm.bStreamProduceThreadExit  = FALSE;
    gstAovThreadParm.bStreamConsumeThreadExit  = FALSE;
    gstAovThreadParm.bDetStateSwitchThreadExit = FALSE;
    gstAovThreadParm.bStreamDetThreadExit      = FALSE;
    gstAovThreadParm.bAudioThreadExit          = FALSE;

    gstAovHandle.stAovPipeAttr.u32SensorNum = gstAovInputParm.u32SensorNum;
    ST_Common_AovSetQos(&gstAovHandle);

    STCHECKRESULT(ST_Common_AovGetDefaultAttr(&gstAovHandle));
    gstAovHandle.stAovPipeAttr.stVifParam[0].u8SensorIndex = gstAovInputParm.u8SensorIndex;
    gstAovHandle.stAovPipeAttr.stVifParam[1].u8SensorIndex = gstAovInputParm.u8SensorIndex;
    gstAovHandle.stAutoTestParam.bAutoTest                 = gstAovInputParm.bAutoTest;
    gstAovHandle.bDemonstrate                              = gstAovInputParm.bDemonstrate;
    gstAovHandle.bStressTest                               = gstAovInputParm.bStressTest;
    gstAovHandle.stAovPipeAttr.stAudioParam.bEnable        = gstAovInputParm.bAuidoEnable;
    gstAovHandle.stAovPipeAttr.stAudioParam.bAoEnable      = gstAovInputParm.bAuidoAoEnable;
    gstAovHandle.stAovParam.u32WriteStreamTriggerNum       = gstAovInputParm.u32WriteStreamTriggerNum;
    gstAovHandle.stAovPipeAttr.u32MaxStreamBufferNum =
        gstAovHandle.stAovParam.u32WriteStreamTriggerNum + VENC_STREAM_POOL_BUFFER;
    gstAovHandle.stAovPipeAttr.eVencType = gstAovInputParm.eVencType;
    strcpy(gstAovHandle.stAovParam.au8StreamDumpPath, gstAovInputParm.au8DumpPath);
    if ((TRUE == gstAovInputParm.bAuidoEnable) && (TRUE == gstAovInputParm.bAuidoAoEnable))
    {
        strcpy(gstAovHandle.stAovPipeAttr.stAudioParam.au8AiFile, gstAovInputParm.au8AiFile);
        strcpy(gstAovHandle.stAovPipeAttr.stAudioParam.au8AoFile, gstAovInputParm.au8AoFile);
    }
    else if ((TRUE == gstAovInputParm.bAuidoEnable) && (FALSE == gstAovInputParm.bAuidoAoEnable))
    {
        strcpy(gstAovHandle.stAovPipeAttr.stAudioParam.au8AiFile, gstAovInputParm.au8AiFile);
    }
    strcpy(gstAovHandle.stAovParam.au8DetectModelPath, gstAovInputParm.au8DetectModelPath);
    strcpy(gstAovHandle.stAovPipeAttr.au8IqApiBinBrightPath, gstAovInputParm.au8IqBinBrightPath);
    strcpy(gstAovHandle.stAovPipeAttr.au8IqApiBinDarkPath, gstAovInputParm.au8IqBinDarkPath);

    if (TRUE == gstAovInputParm.bEnableSWLightSensor)
    {
        gstAovHandle.stAovSWLightSensorParam.bEnableSWLightSensor = TRUE;
    }
    if (TRUE == gstAovInputParm.bEnableHWLightSensor)
    {
        gstAovHandle.stLightMiscCtlParam.stAovHWLightSensorParam.bEnableHWLightSensor = TRUE;
    }
    if (TRUE == gstAovInputParm.bEnableFastAE)
    {
        gstAovHandle.stFastAEParam.bEnableFastAE = TRUE;
    }
    if (TRUE == gstAovInputParm.bUsingMcu)
    {
        gstAovHandle.bUsingMcu = gstAovInputParm.bUsingMcu;
    }
    if (TRUE == gstAovInputParm.bRealTimeEnable)
    {
        gstAovHandle.bRealTimeEnable = gstAovInputParm.bRealTimeEnable;
    }
    STCHECKRESULT(ST_Common_AovMcuGpioInit(&gstAovHandle));
    STCHECKRESULT(ST_Common_AovPipeInit(&gstAovHandle));
    STCHECKRESULT(ST_Common_AovPipeStart(&gstAovHandle));
    STCHECKRESULT(ST_Common_AovStreamCreate(&gstStreamHandle));

    pthread_create(&pReadKeyBoardThread, NULL, ST_ReadKeyBoardThread, NULL);
    pthread_create(&pStreamProduceThread, NULL, ST_StreamProduceThread, NULL);
    pthread_create(&pStreamConsumeThread, NULL, ST_StreamConsumeThread, NULL);
    pthread_create(&pDetectThread, NULL, ST_DetectThread, NULL);
    pthread_create(&pStateSwitchThread, NULL, ST_StateSwitchThread, NULL);
    if (gstAovHandle.stAovPipeAttr.stAudioParam.bEnable)
    {
        pthread_create(&pAudioThread, NULL, ST_AudioThread, NULL);
    }

    while (FALSE == gstAovThreadParm.bProcessExit)
    {
        sleep(1);
    }

    pthread_join(pStateSwitchThread, NULL);
    pthread_join(pStreamProduceThread, NULL);
    pthread_join(pStreamConsumeThread, NULL);
    pthread_join(pDetectThread, NULL);
    pthread_join(pReadKeyBoardThread, NULL);
    if (gstAovHandle.stAovPipeAttr.stAudioParam.bEnable)
    {
        pthread_join(pAudioThread, NULL);
    }

    STCHECKRESULT(ST_Common_AovPipeStop(&gstAovHandle));
    STCHECKRESULT(ST_Common_AovPipeDeInit(&gstAovHandle));

    return MI_SUCCESS;
}

MI_S32 ST_Aov_GetCmdlineParam(int argc, char **argv)
{
    gstAovInputParm.u8SensorIndex            = 0;
    gstAovInputParm.bAutoTest                = FALSE;
    gstAovInputParm.bDemonstrate             = FALSE;
    gstAovInputParm.bStressTest              = FALSE;
    gstAovInputParm.u32SensorNum             = 1;
    gstAovInputParm.u32WriteStreamTriggerNum = 10;
    gstAovInputParm.bAutoTest                = FALSE;
    gstAovInputParm.eVencType                = E_MI_VENC_MODTYPE_H264E;
    gstAovInputParm.bEnableSWLightSensor     = FALSE;
    gstAovInputParm.bEnableHWLightSensor     = FALSE;
    gstAovInputParm.bEnableFastAE            = FALSE;
    gstAovInputParm.u32RunTime               = 0;

#define ROFILES_PATH "/rofiles"
    if (access(ROFILES_PATH, R_OK) == 0)
    {
        sprintf(gstAovInputParm.au8DetectModelPath, "/rofiles/spdy48.img");
        sprintf(gstAovInputParm.au8IqBinBrightPath, "%s/sc4336P_api.bin", ROFILES_PATH);
        sprintf(gstAovInputParm.au8IqBinDarkPath, "%s/sc4336P_night_api.bin", ROFILES_PATH);
    }
    else
    {
        static const char *gDetModelPaths[] = {"/misc/spdy48.img", "/misc/dla/spdy48.img", "/config/dla/spdy48.img"};
        for (int i = 0; i < ARRAY_SIZE(gDetModelPaths); i++)
        {
            if (!access(gDetModelPaths[i], R_OK))
            {
                strcpy(gstAovInputParm.au8DetectModelPath, gDetModelPaths[i]);
                break;
            }
        }
        sprintf(gstAovInputParm.au8IqBinBrightPath, "/config/iqfile/sc4336P_api.bin");
        sprintf(gstAovInputParm.au8IqBinDarkPath, "/config/iqfile/sc4336P_night_api.bin");
    }
    strcpy(gstAovInputParm.au8DumpPath, ".");
    strcpy(gstAovInputParm.au8AiFile, ".");
    strcpy(gstAovInputParm.au8AoFile, "./ao.wav");

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gstAovInputParm.u8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "dual_sensor"))
        {
            gstAovInputParm.u32SensorNum = 2;
        }
        else if (0 == strcmp(argv[i], "iqbin_b"))
        {
            strcpy((char *)gstAovInputParm.au8IqBinBrightPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "iqbin_d"))
        {
            strcpy((char *)gstAovInputParm.au8IqBinDarkPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "model"))
        {
            strcpy(gstAovInputParm.au8DetectModelPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "dump"))
        {
            strcpy(gstAovInputParm.au8DumpPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "store"))
        {
            gstAovInputParm.u32WriteStreamTriggerNum = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "venc"))
        {
            strcpy(gstAovInputParm.auVencType, argv[i + 1]);

            if (0 == strcasecmp(gstAovInputParm.auVencType, "h265"))
            {
                gstAovInputParm.eVencType = E_MI_VENC_MODTYPE_H265E;
            }
            else
            {
                gstAovInputParm.eVencType = E_MI_VENC_MODTYPE_H264E;
            }
        }
        else if (0 == strcmp(argv[i], "aux_conv"))
        {
            if (0 == atoi(argv[i + 1]))
            {
                gstAovInputParm.bEnableHWLightSensor = TRUE;
                printf("Enable hardware light_sensor\n");
            }
            else if (1 == atoi(argv[i + 1]))
            {
                gstAovInputParm.bEnableSWLightSensor = TRUE;
                printf("Enable software light_sensor\n");
            }
            else
            {
                gstAovInputParm.bEnableFastAE = TRUE;
                printf("Enable sensor fastae\n");
            }
        }

        else if (0 == strcmp(argv[i], "-d"))
        {
            gstAovInputParm.bDemonstrate = TRUE;
        }
        else if (0 == strcmp(argv[i], "-ai"))
        {
            gstAovInputParm.bAuidoEnable   = TRUE;
            gstAovInputParm.bAuidoAoEnable = FALSE;
            strcpy(gstAovInputParm.au8AiFile, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "-aio"))
        {
            gstAovInputParm.bAuidoEnable   = TRUE;
            gstAovInputParm.bAuidoAoEnable = TRUE;
            strcpy(gstAovInputParm.au8AiFile, argv[i + 1]);
            strcpy(gstAovInputParm.au8AoFile, argv[i + 2]);
        }
        else if (0 == strcmp(argv[i], "-t"))
        {
            gstAovInputParm.bAutoTest = TRUE;
        }
        else if (0 == strcmp(argv[i], "-s"))
        {
            gstAovInputParm.bStressTest = TRUE;
        }
        else if (0 == strcmp(argv[i], "mcu"))
        {
            gstAovInputParm.bUsingMcu = TRUE;
        }
        else if (0 == strcmp(argv[i], "realtime"))
        {
            gstAovInputParm.bRealTimeEnable = TRUE;
        }
        else if(0 == strcmp(argv[i], "time"))
        {
            gstAovInputParm.u32RunTime = atoi(argv[i + 1]);
        }
        else if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "--help")))
        {
            printf("Options are: \n");
            printf("    index       : Specify sensor index\n");
            printf("    dual_sensor : Enable dual sensor\n");
            printf("    iqbin_b     : Specify the iqbin path used when the environment is bright\n");
            printf("    iqbin_d     : Specify the iqbin path used when the environment is dark\n");
            printf("    model       : Specify the network model path used for detection\n");
            printf("    dump        : Specify the path of the es file dump\n");
            printf("    store       : Specify how many es to store in memory before writing the file out\n");
            printf("    aux_conv: Specify hwlightsensor or swlightsensor or fast ae\n");
            printf("                  aux_conv 0 (specify hwlightsensor)\n");
            printf("                  aux_conv 1 (specify swlightsensor)\n");
            printf(
                "                  aux_conv 2 (specify fast ae (Need Sensor supports this feature, such as "
                "OS04D10))\n");
            printf("    -h --help   : Display option list\n");
            printf("    -ai         : Enable audio input and specify the path for audio input\n");
            printf("    -aio        : Enable audio input/output and specify the path for audio input/output\n");
            printf("    -t          : Just for auto test\n");
            printf("    mcu         : Using Mcu to manage soc power\n");
            printf("    realtime         : vif->isp use realtime mode bind\n");
            printf("    time         : app run time set(second)\n");
            exit(0);
        }
    }

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    ST_Aov_GetCmdlineParam(argc, argv);

    STCHECKRESULT(ST_AOV_Preview());

    return 0;
}
