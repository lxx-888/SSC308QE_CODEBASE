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

#include "ssos_list.h"
#include "ssos_task.h"
#include "ssos_mem.h"
#include "ssos_io.h"
#include "ssos_thread.h"
#include "ssos_time.h"

#define ADD_TIME(_ts, _sec, _nsec)     \
    do                                 \
    {                                  \
        _ts->tvSec += _sec;            \
        _ts->tvNSec += _nsec;          \
        if (_ts->tvNSec > 1000000000)  \
        {                              \
            _ts->tvNSec %= 1000000000; \
            _ts->tvSec++;              \
        }                              \
    } while (0);

#define MUTEXCHECK(x)                                           \
    do                                                          \
    {                                                           \
        if (x != 0)                                             \
        {                                                       \
            SSOS_IO_Printf("%s <%d>:\n\t", __FILE__, __LINE__); \
        }                                                       \
    } while (0);

#ifndef PTH_RET_CHK
// Just Temp Solution
#define PTH_RET_CHK(_pf_)                                                          \
    (                                                                              \
        {                                                                          \
            int r = _pf_;                                                          \
            if (r != 0)                                                            \
                SSOS_IO_Printf("[PTASK] %s: %d: %s\n", __FILE__, __LINE__, #_pf_); \
            r;                                                                     \
        })
#endif
#define DEBUG(_fmt, ...) // SSOS_IO_Printf("[%s][%d][ssos_task][log]: " _fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define INFO(_fmt, ...)  // SSOS_IO_Printf("[%s][%d][ssos_task][info]: " _fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ERROR(_fmt, ...) SSOS_IO_Printf("[%s][%d][ssos_task][err]: " _fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define SSOS_TASK_HASH_SIZE 128

enum SSOS_TASK_Event_e
{
    E_SSOS_TASK_IDLE,
    E_SSOS_TASK_DO_USER_DATA,
    E_SSOS_TASK_DROP_USER_DATA,
    E_SSOS_TASK_DROP_USER_DATA_END,
    E_SSOS_TASK_EXIT,
    E_SSOS_TASK_START_MONITOR,
    E_SSOS_TASK_ONESHOT,
    E_SSOS_TASK_STOP,
    E_SSOS_TASK_CONFIG_TIMER,
};

typedef struct SSOS_TASK_DataNode_s
{
    enum SSOS_TASK_Event_e  threadEventn;   /* Thread status. */
    struct SSOS_LIST_Head_s threadDataList; /* Data list node. */
    SSOS_TASK_UserData_t    userData;       /* User data. */
} SSOS_TASK_DataNode_t;

typedef struct SSOS_TASK_Info_s
{
    SSOS_THREAD_Mutex_t  nodeMutex;
    SSOS_THREAD_Mutex_t  dataMutex;
    SSOS_THREAD_Cond_t   cond;
    SSOS_THREAD_Handle_t thread;
    int                  pid;
} SSOS_TASK_Info_t;

typedef struct SSOS_TASK_Node_s
{
    struct SSOS_LIST_Head_s threadDataList; /* 'ssos_thread' data event list head of SSOS_TASK_DataNode_t */
    struct SSOS_TASK_Attr_s attr;           /* User setting. */
    SSOS_TASK_Info_t        info;           /* Info of thread. */
    unsigned int            dataSize;       /* List data size count. */
    unsigned int            eventCount;     /* list count. */
} SSOS_TASK_Node_t;

static int _SSOS_TASK_PushEvent(SSOS_TASK_Node_t *node, SSOS_TASK_UserData_t *data, enum SSOS_TASK_Event_e status)
{
    SSOS_TASK_DataNode_t *dataNode = NULL;

    SSOS_ASSERT(node);
    dataNode = (SSOS_TASK_DataNode_t *)SSOS_MEM_Alloc(sizeof(SSOS_TASK_DataNode_t));
    SSOS_ASSERT(dataNode);
    INFO("Malloc Node 0x%lx\n", (unsigned long)dataNode);
    dataNode->userData.size     = 0;
    dataNode->userData.realSize = 0;
    if (data != NULL)
    {
        memcpy(&dataNode->userData, data, sizeof(SSOS_TASK_UserData_t));
    }
    dataNode->threadEventn = status;
    SSOS_THREAD_MutexLock(&node->info.dataMutex);
    SSOS_LIST_AddTail(&dataNode->threadDataList, &node->threadDataList);
    node->eventCount++;
    node->dataSize += dataNode->userData.realSize;
    INFO("[name:%s]: Real Size: %d Node: %d, Event %d\n", node->attr.threadAttr.name, dataNode->userData.realSize,
         node->dataSize, node->eventCount);
    if (status == E_SSOS_TASK_DO_USER_DATA)
    {
        SSOS_TASK_DataNode_t *dataNodeDrop    = NULL;
        SSOS_TASK_DataNode_t *dataNodeDropEnd = NULL;

        if (node->dataSize > node->attr.maxData && node->attr.isDropData)
        {
            ERROR("[thread %s]: Event data is over range %d[max: %d]! Drop data event!\n", node->attr.threadAttr.name,
                  node->dataSize, node->attr.maxData);
            dataNodeDrop = (SSOS_TASK_DataNode_t *)SSOS_MEM_Alloc(sizeof(SSOS_TASK_DataNode_t));
            SSOS_ASSERT(dataNodeDrop);
            dataNodeDrop->userData.data     = NULL;
            dataNodeDrop->userData.size     = 0;
            dataNodeDrop->userData.realSize = 0;
            dataNodeDrop->threadEventn      = E_SSOS_TASK_DROP_USER_DATA;
            dataNodeDropEnd                 = (SSOS_TASK_DataNode_t *)SSOS_MEM_Alloc(sizeof(SSOS_TASK_DataNode_t));
            SSOS_ASSERT(dataNodeDropEnd);
            dataNodeDropEnd->userData.size     = 0;
            dataNodeDropEnd->userData.realSize = 0;
            dataNodeDropEnd->userData.data     = NULL;
            dataNodeDropEnd->threadEventn      = E_SSOS_TASK_DROP_USER_DATA_END;
            SSOS_LIST_Add(&dataNodeDrop->threadDataList, &node->threadDataList);
            SSOS_LIST_AddTail(&dataNodeDropEnd->threadDataList, &node->threadDataList);
            node->eventCount += 2;
            SSOS_THREAD_MutexUnlock(&node->info.dataMutex);
            return -1;
        }
        if (node->eventCount > node->attr.maxEvent && node->attr.isDropEvent)
        {
            ERROR("[thread %s]: Event list is over range %d[max: %d]! Drop data event!\n", node->attr.threadAttr.name,
                  node->eventCount, node->attr.maxEvent);
            dataNodeDrop = (SSOS_TASK_DataNode_t *)SSOS_MEM_Alloc(sizeof(SSOS_TASK_DataNode_t));
            SSOS_ASSERT(dataNodeDrop);
            dataNodeDrop->userData.size     = 0;
            dataNodeDrop->userData.realSize = 0;
            dataNodeDrop->userData.data     = NULL;
            dataNodeDrop->threadEventn      = E_SSOS_TASK_DROP_USER_DATA;
            dataNodeDropEnd                 = (SSOS_TASK_DataNode_t *)SSOS_MEM_Alloc(sizeof(SSOS_TASK_DataNode_t));
            SSOS_ASSERT(dataNodeDropEnd);
            dataNodeDropEnd->userData.size     = 0;
            dataNodeDropEnd->userData.realSize = 0;
            dataNodeDropEnd->userData.data     = NULL;
            dataNodeDropEnd->threadEventn      = E_SSOS_TASK_DROP_USER_DATA_END;
            SSOS_LIST_Add(&dataNodeDrop->threadDataList, &node->threadDataList);
            SSOS_LIST_AddTail(&dataNodeDropEnd->threadDataList, &node->threadDataList);
            node->eventCount += 2;
            SSOS_THREAD_MutexUnlock(&node->info.dataMutex);
            return -1;
        }
    }
    SSOS_THREAD_MutexUnlock(&node->info.dataMutex);
    return 0;
}
static enum SSOS_TASK_Event_e _SSOS_TASK_PopEvent(SSOS_TASK_Node_t *node, SSOS_TASK_UserData_t *userData)
{
    SSOS_TASK_DataNode_t * dataNode = NULL;
    enum SSOS_TASK_Event_e event;

    SSOS_ASSERT(node);
    SSOS_ASSERT(userData);
    SSOS_THREAD_MutexLock(&node->info.dataMutex);
    if (SSOS_LIST_Empty(&node->threadDataList))
    {
        SSOS_THREAD_MutexUnlock(&node->info.dataMutex);
        return E_SSOS_TASK_IDLE;
    }
    dataNode = SSOS_LIST_FIRST_ENTRY(&node->threadDataList, SSOS_TASK_DataNode_t, threadDataList);
    SSOS_LIST_Del(&dataNode->threadDataList);
    node->eventCount--;
    node->dataSize -= dataNode->userData.realSize;
    SSOS_THREAD_MutexUnlock(&node->info.dataMutex);
    *userData = dataNode->userData;
    event     = dataNode->threadEventn;
    INFO("Free Node 0x%lx\n", (unsigned long)dataNode);
    SSOS_MEM_Free(dataNode);
    return event;
}
static void _SSOS_TASK_EventMonitor(SSOS_TASK_Node_t *node, SSOS_TIME_Spec_t *startTime, SSOS_TIME_Spec_t *stepTime,
                                    unsigned char *isRunOneShot, unsigned char *isRun)
{
    unsigned short         dropEventCount = 0;
    SSOS_TASK_UserData_t   userData;
    enum SSOS_TASK_Event_e event;

    while (1)
    {
        event = _SSOS_TASK_PopEvent(node, &userData);
        if (E_SSOS_TASK_IDLE == event)
        {
            break;
        }
        switch (event)
        {
            case E_SSOS_TASK_EXIT:
            {
                INFO("Exit thread id:[%p], name[%s]\n", node->info.thread, node->attr.threadAttr.name);
                *isRun           = 0;
                stepTime->tvSec  = 0;
                stepTime->tvNSec = 0;
            }
            break;
            case E_SSOS_TASK_STOP:
            {
                INFO("Stop thread id:[%p], name[%s]\n", node->info.thread, node->attr.threadAttr.name);
                stepTime->tvSec  = -1;
                stepTime->tvNSec = -1;
            }
            break;
            case E_SSOS_TASK_START_MONITOR:
            {
                stepTime->tvSec  = node->attr.monitorCycleSec;
                stepTime->tvNSec = node->attr.monitorCycleNsec;
                SSOS_TIME_Get(startTime);
                INFO("Start thread monitor id:[%p], name[%s], time[%lld,%lld]\n", node->info.thread,
                     node->attr.threadAttr.name, stepTime->tvSec, stepTime->tvNSec);
            }
            break;
            case E_SSOS_TASK_ONESHOT:
            {
                if (node->attr.doMonitor && !node->attr.monitorCycleSec && !node->attr.monitorCycleNsec)
                {
                    node->attr.doMonitor(&node->attr.inBuf);
                }
                else
                {
                    *isRunOneShot = 1;
                    SSOS_TIME_Get(startTime);
                    stepTime->tvSec  = node->attr.monitorCycleSec;
                    stepTime->tvNSec = node->attr.monitorCycleNsec;
                }
                INFO("One shot thread id::[%p], name[%s], time[%lld,%lld]\n", node->info.thread,
                     node->attr.threadAttr.name, stepTime->tvSec, stepTime->tvNSec);
            }
            break;
            case E_SSOS_TASK_CONFIG_TIMER:
            {
                stepTime->tvSec  = node->attr.monitorCycleSec;
                stepTime->tvNSec = node->attr.monitorCycleNsec;
                SSOS_TIME_Get(startTime);
                INFO("Config  thread id::[%p], name[%s], time[%lld,%lld]\n", node->info.thread,
                     node->attr.threadAttr.name, stepTime->tvSec, stepTime->tvNSec);
            }
            break;
            case E_SSOS_TASK_DO_USER_DATA:
            {
                if (dropEventCount)
                {
                    // ERROR("Do user data thread id:[%p], name[%s] Drop event!\n", node->info.thread,
                    // node->attr.name);
                    if (node->attr.doSignalDrop)
                    {
                        node->attr.doSignalDrop(&node->attr.inBuf, &userData);
                    }
                }
                else
                {
                    INFO("Do user data thread id:[%p], name[%s]\n", node->info.thread, node->attr.threadAttr.name);
                    if (node->attr.doSignal)
                    {
                        node->attr.doSignal(&node->attr.inBuf, &userData);
                    }
                }
                if (userData.size != 0)
                {
                    INFO("Free Node Data 0x%lx\n", (unsigned long)userData.data);
                    SSOS_MEM_Free(userData.data);
                    userData.data = NULL;
                }
            }
            break;
            case E_SSOS_TASK_DROP_USER_DATA:
            {
                dropEventCount++;
                ERROR("T[%s][%p] Drop event! cnt: %d\n", node->attr.threadAttr.name, node->info.thread, dropEventCount);
            }
            break;
            case E_SSOS_TASK_DROP_USER_DATA_END:
            {
                dropEventCount--;
                ERROR("T[%s][%p] Drop event! cnt: %d\n", node->attr.threadAttr.name, node->info.thread, dropEventCount);
            }
            break;
            default:
            {
                ERROR("T[%s][%p] Error ssos_thread thread event.id:[%d],", node->attr.threadAttr.name,
                      node->info.thread, event);
            }
            break;
        }
    }
}
static int _SSOS_TASK_EventWait(SSOS_TASK_Node_t *node, SSOS_TIME_Spec_t *timeStart, const SSOS_TIME_Spec_t *timeStep,
                                unsigned char isResetTimer)
{
    int condWaitRet = 0;

    if (!timeStep->tvSec && !timeStep->tvNSec)
    {
        return SSOS_DEF_ETIMEOUT;
    }
    SSOS_THREAD_MutexLock(&node->info.dataMutex);
    if (!SSOS_LIST_Empty(&node->threadDataList))
    {
        SSOS_THREAD_MutexUnlock(&node->info.dataMutex);
        return 0;
    }
    SSOS_THREAD_MutexUnlock(&node->info.dataMutex);
    switch (timeStep->tvSec)
    {
        case -1:
        {
            condWaitRet = SSOS_THREAD_COND_WAIT(&node->info.cond, !SSOS_LIST_Empty(&node->threadDataList));
        }
        break;
        default:
        {
            SSOS_TIME_Spec_t curTime;
            SSOS_TIME_Spec_t tmpDiffTime;
            if (isResetTimer)
            {
                SSOS_TIME_Get(timeStart);
            }
            ADD_TIME(timeStart, timeStep->tvSec, timeStep->tvNSec);
            SSOS_TIME_Get(&curTime);
            if (timeStart->tvSec < curTime.tvSec
                || (timeStart->tvSec == curTime.tvSec ? (timeStart->tvNSec < curTime.tvNSec) : 0))
            {
                /*Stop a while and then Stat Monitor.*/
                return SSOS_DEF_ETIMEOUT;
            }
            tmpDiffTime.tvSec = timeStart->tvSec - curTime.tvSec;
            if (curTime.tvNSec > timeStart->tvNSec)
            {
                tmpDiffTime.tvNSec = 1000000000 - (curTime.tvNSec - timeStart->tvNSec);
                tmpDiffTime.tvSec--;
            }
            else
            {
                tmpDiffTime.tvNSec = timeStart->tvNSec - curTime.tvNSec;
            }
            condWaitRet =
                SSOS_THREAD_COND_TIME_WAIT(&node->info.cond, !SSOS_LIST_Empty(&node->threadDataList), tmpDiffTime);
        }
        break;
    }
    return condWaitRet;
}
static void *_SSOS_TASK_Main(void *arg)
{
    int               condWaitRet  = 0;
    unsigned char     isRun        = 1;
    unsigned char     isRunOneShot = 0;
    SSOS_TIME_Spec_t  startTime    = {-1, -1};
    SSOS_TIME_Spec_t  stepTime     = {-1, -1};
    SSOS_TASK_Node_t *node         = (SSOS_TASK_Node_t *)arg;

    SSOS_ASSERT(node);
    node->info.pid = SSOS_THREAD_GetPid();
    INFO("THREAD Name: %s, PID: %d\n", node->attr->threadAttr.name, node->info.pid);
    SSOS_THREAD_SetName(node->info.thread, node->attr.threadAttr.name);
    while (1)
    {
        _SSOS_TASK_EventMonitor(node, &startTime, &stepTime, &isRunOneShot, &isRun);
        if (!isRun)
        {
            INFO("SSOS_TASK name [%s]: Bye bye~\n", node->attr.name);
            break;
        }
        if (node->attr.doMonitor && !stepTime.tvNSec && !stepTime.tvSec)
        {
            node->attr.doMonitor(&node->attr.inBuf);
            continue;
        }
        PTH_RET_CHK(SSOS_THREAD_MutexLock(&node->info.nodeMutex));
        if (node->attr.doMonitor && (condWaitRet == SSOS_DEF_PLATFORM_ETIMEOUT))
        {
            node->attr.doMonitor(&node->attr.inBuf);
            if (isRunOneShot)
            {
                stepTime.tvSec  = -1;
                stepTime.tvNSec = -1;
                isRunOneShot    = 0;
            }
        }
        PTH_RET_CHK(SSOS_THREAD_MutexUnlock(&node->info.nodeMutex));
        condWaitRet = _SSOS_TASK_EventWait(node, &startTime, &stepTime, node->attr.isResetTimer);
    }
    return NULL;
}
static void _SSOS_TASK_CondSignal(SSOS_TASK_Node_t *node)
{
    SSOS_THREAD_COND_WAKE(&node->info.cond);
}
void *SSOS_TASK_Open(struct SSOS_TASK_Attr_s *attr)
{
    SSOS_TASK_Node_t *node = NULL;
    const char *      str  = attr->threadAttr.name ? attr->threadAttr.name : "";

    node = (SSOS_TASK_Node_t *)SSOS_MEM_Alloc(sizeof(SSOS_TASK_Node_t));
    SSOS_ASSERT(node);
    memset(node, 0, sizeof(SSOS_TASK_Node_t));

    /*Init data event list*/
    SSOS_LIST_InitHead(&node->threadDataList);

    /*Copy Attr*/
    memcpy(&node->attr, attr, sizeof(struct SSOS_TASK_Attr_s));
    node->attr.threadAttr.name = SSOS_MEM_Alloc(strlen(str) + 1);
    SSOS_ASSERT(node->attr.threadAttr.name);
    strcpy(node->attr.threadAttr.name, str);

    /*Malloc 'ssos_thread' internal buffer size*/
    if (attr->inBuf.size != 0 && attr->inBuf.buf != NULL)
    {
        node->attr.inBuf.buf = SSOS_MEM_Alloc(attr->inBuf.size);
        SSOS_ASSERT(node->attr.inBuf.buf);
        memcpy(node->attr.inBuf.buf, attr->inBuf.buf, attr->inBuf.size);
    }
    /*Malloc ssos_thread info*/
    PTH_RET_CHK(SSOS_THREAD_MutexInit(&node->info.nodeMutex));
    PTH_RET_CHK(SSOS_THREAD_MutexInit(&node->info.dataMutex));
    SSOS_THREAD_COND_INIT(&node->info.cond);
    PTH_RET_CHK(SSOS_THREAD_Create(&(node->info.thread), &(node->attr.threadAttr), _SSOS_TASK_Main, (void *)node));

    return node;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_Open);

int SSOS_TASK_Close(void *handle)
{
    SSOS_TASK_Node_t *node = NULL;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    SSOS_ASSERT(node);
    _SSOS_TASK_PushEvent(node, NULL, E_SSOS_TASK_EXIT);
    SSOS_THREAD_COND_WAKE(&node->info.cond);
    PTH_RET_CHK(SSOS_THREAD_Join(node->info.thread));
    SSOS_ASSERT(SSOS_LIST_Empty(&node->threadDataList));
    SSOS_THREAD_COND_DEINIT(&node->info.cond);
    PTH_RET_CHK(SSOS_THREAD_MutexDeinit(&node->info.dataMutex));
    PTH_RET_CHK(SSOS_THREAD_MutexDeinit(&node->info.nodeMutex));
    DEBUG("Left data node size %d event count %d\n", node->dataSize, node->eventCount);
    if (node->attr.inBuf.buf && node->attr.inBuf.size != 0)
    {
        SSOS_MEM_Free(node->attr.inBuf.buf);
    }
    if (node->attr.threadAttr.name)
    {
        SSOS_MEM_Free(node->attr.threadAttr.name);
    }
    SSOS_MEM_Free(node);
    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_Close);

int SSOS_TASK_StartMonitor(void *handle)
{
    SSOS_TASK_Node_t *node = NULL;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    _SSOS_TASK_PushEvent(node, NULL, E_SSOS_TASK_START_MONITOR);
    _SSOS_TASK_CondSignal(node);

    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_StartMonitor);

int SSOS_TASK_OneShot(void *handle)
{
    SSOS_TASK_Node_t *node = NULL;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    _SSOS_TASK_PushEvent(node, NULL, E_SSOS_TASK_ONESHOT);
    _SSOS_TASK_CondSignal(node);

    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_OneShot);

int SSOS_TASK_ConfigTimer(void *handle, long timeOutSec, long timeOutNsec, unsigned char isResetTimer)
{
    SSOS_TASK_Node_t *node = NULL;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.pid == SSOS_THREAD_GetPid())
    {
        if (timeOutSec != 0)
        {
            node->attr.monitorCycleSec = timeOutSec;
        }
        if (timeOutNsec != 0)
        {
            node->attr.monitorCycleNsec = timeOutNsec;
        }
        node->attr.isResetTimer = isResetTimer;
    }
    else
    {
        PTH_RET_CHK(SSOS_THREAD_MutexLock(&node->info.nodeMutex));
        if (timeOutSec != 0)
        {
            node->attr.monitorCycleSec = timeOutSec;
        }
        if (timeOutNsec != 0)
        {
            node->attr.monitorCycleNsec = timeOutNsec;
        }
        node->attr.isResetTimer = isResetTimer;
        PTH_RET_CHK(SSOS_THREAD_MutexUnlock(&node->info.nodeMutex));
        _SSOS_TASK_PushEvent(node, NULL, E_SSOS_TASK_CONFIG_TIMER);
        _SSOS_TASK_CondSignal(node);
    }
    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_ConfigTimer);

int SSOS_TASK_Stop(void *handle)
{
    SSOS_TASK_Node_t *node = NULL;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    _SSOS_TASK_PushEvent(node, NULL, E_SSOS_TASK_STOP);
    _SSOS_TASK_CondSignal(node);
    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_Stop);

int SSOS_TASK_Send(void *handle, SSOS_TASK_UserData_t *userData)
{
    SSOS_TASK_Node_t *   node = NULL;
    SSOS_TASK_UserData_t userDataExp;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (userData && userData->size != 0)
    {
        userDataExp      = *userData;
        userDataExp.data = SSOS_MEM_Alloc(userData->size);
        SSOS_ASSERT(userDataExp.data);
        INFO("Malloc Node Data 0x%lx\n", (unsigned long)userData->data);
        memcpy(userDataExp.data, userData->data, userData->size);
        userData = &userDataExp;
    }
    _SSOS_TASK_PushEvent(node, userData, E_SSOS_TASK_DO_USER_DATA);
    _SSOS_TASK_CondSignal(node);

    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_Send);

int SSOS_TASK_SetBuffer(void *handle, void *data)
{
    SSOS_TASK_Node_t *node = NULL;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.pid == SSOS_THREAD_GetPid())
    {
        memcpy(node->attr.inBuf.buf, data, node->attr.inBuf.size);
    }
    else
    {
        PTH_RET_CHK(SSOS_THREAD_MutexLock(&node->info.nodeMutex));
        memcpy(node->attr.inBuf.buf, data, node->attr.inBuf.size);
        PTH_RET_CHK(SSOS_THREAD_MutexUnlock(&node->info.nodeMutex));
        _SSOS_TASK_CondSignal(node);
    }

    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_SetBuffer);

int SSOS_TASK_GetBuffer(void *handle, void *data)
{
    SSOS_TASK_Node_t *node = NULL;

    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.pid == SSOS_THREAD_GetPid())
    {
        memcpy(data, node->attr.inBuf.buf, node->attr.inBuf.size);
    }
    else
    {
        PTH_RET_CHK(SSOS_THREAD_MutexLock(&node->info.nodeMutex));
        memcpy(data, node->attr.inBuf.buf, node->attr.inBuf.size);
        PTH_RET_CHK(SSOS_THREAD_MutexUnlock(&node->info.nodeMutex));
        _SSOS_TASK_CondSignal(node);
    }

    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_GetBuffer);

int SSOS_TASK_SetPartBufData(void *handle, void *head, void *part, unsigned int size, void *inBuf)
{
    SSOS_TASK_Node_t *node   = NULL;
    unsigned int      offset = 0;

    if (head == NULL || part == NULL || size == 0)
    {
        ERROR("head or part or size is NULL!!!\n");
        return -1;
    }
    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.pid == SSOS_THREAD_GetPid())
    {
        offset = part - head;
        if (node->attr.inBuf.size != 0 && offset < node->attr.inBuf.size)
        {
            memcpy(node->attr.inBuf.buf + offset, inBuf, size);
        }
        else
        {
            ERROR("######your part of ssos_thread buf data addr is error!!!\n");
        }
    }
    else
    {
        PTH_RET_CHK(SSOS_THREAD_MutexLock(&node->info.nodeMutex));
        offset = part - head;
        if (node->attr.inBuf.size != 0 && offset < node->attr.inBuf.size)
        {
            memcpy(node->attr.inBuf.buf + offset, inBuf, size);
        }
        else
        {
            ERROR("######your part of ssos_thread buf data addr is error!!!\n");
        }
        PTH_RET_CHK(SSOS_THREAD_MutexUnlock(&node->info.nodeMutex));
        _SSOS_TASK_CondSignal(node);
    }
    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_SetPartBufData);

int SSOS_TASK_GetPartBufData(void *handle, void *head, void *part, unsigned int size, void *outBuf)
{
    SSOS_TASK_Node_t *node   = NULL;
    unsigned int      offset = 0;

    if (head == NULL || part == NULL || size == 0)
    {
        ERROR("head or part or size is NULL!!!\n");
        return -1;
    }
    node = (SSOS_TASK_Node_t *)handle;
    if (node == NULL)
    {
        return -1;
    }
    if (node->info.pid == SSOS_THREAD_GetPid())
    {
        offset = part - head;
        if (node->attr.inBuf.size != 0 && offset < node->attr.inBuf.size)
        {
            memcpy(outBuf, node->attr.inBuf.buf + offset, size);
        }
        else
        {
            ERROR("######Your part of ssos_thread buf data addr is error!!!\n");
        }
    }
    else
    {
        PTH_RET_CHK(SSOS_THREAD_MutexLock(&node->info.nodeMutex));
        offset = part - head;
        if (node->attr.inBuf.size != 0 && offset < node->attr.inBuf.size)
        {
            memcpy(outBuf, node->attr.inBuf.buf + offset, size);
        }
        else
        {
            ERROR("######Your part of ssos_thread buf data addr is error!!!\n");
        }
        PTH_RET_CHK(SSOS_THREAD_MutexUnlock(&node->info.nodeMutex));
        _SSOS_TASK_CondSignal(node);
    }
    return 0;
}
SSOS_EXPORT_SYMBOL(SSOS_TASK_GetPartBufData);
