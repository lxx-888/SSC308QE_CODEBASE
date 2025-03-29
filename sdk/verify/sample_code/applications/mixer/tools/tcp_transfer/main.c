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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "list.h"
#include "ss_exp.h"

#define DBG(_fmt, ...) printf("[DBG][%s][%d]: " _fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

enum
{
    E_USING_STAGE_FILE_SERVER = 0x1,
    E_USING_STAGE_TALK_SERVER = 0x2,
    E_USING_STAGE_TALK_CLIENT = 0x4,
};

struct TalkMessageObj_s
{
    struct list_head msgList;
    char *message;
};

struct TalkServerObj_s
{
    struct list_head talkObjList;
    struct list_head msgList;
    char selfId[64];
    unsigned char bRead;
    unsigned char isExit;
};

static pthread_mutex_t g_srvTalkMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_srvTalkCond;
static LIST_HEAD(g_talkObjListHead);

static int _FileServerOpen(const char *url, void *privateData, unsigned char bRead, SS_EXP_WorkCfg_t *workCfg)
{
    char *filePath = (char *)privateData;
    struct FileServerDesc_s
    {
        int fd;
        char handleBuf[128];
    } *fileSrvDesc = NULL;
    if (!url || !filePath || !workCfg)
    {
        return -1;
    }
    fileSrvDesc = (struct FileServerDesc_s *)malloc(sizeof(struct FileServerDesc_s));
    if (!fileSrvDesc)
    {
        return -1;
    }
    DBG("Open file: %s url:%s->%p, bRead: %d\n", filePath, url, fileSrvDesc, bRead);
    fileSrvDesc->fd = open(filePath, O_RDONLY);
    if (fileSrvDesc->fd == -1)
    {
        printf("Open file %s error!\n", filePath);
        free(fileSrvDesc);
        return -1;
    }
    workCfg->pUserData = fileSrvDesc;
    return 0;
}
static int _FileServerClose(void *pUsrData)
{
    struct FileServerDesc_s
    {
        int fd;
        char handleBuf[128];
    } *fileSrvDesc = NULL;

    fileSrvDesc = (struct FileServerDesc_s *)pUsrData;
    DBG("Close %p fd %d\n", pUsrData, fileSrvDesc->fd);
    close(fileSrvDesc->fd);
    free(fileSrvDesc);
    return 0;
}
static int _FileServerTransfer(void *pUsrData, SS_EXP_TransferObject_t *transferObj)
{
    int readSize = 0;
    struct FileServerDesc_s
    {
        int fd;
        char handleBuf[128];
    } *fileSrvDesc = NULL;

    fileSrvDesc = (struct FileServerDesc_s *)pUsrData;
    if (!fileSrvDesc || !transferObj)
    {
        return -1;
    }
    readSize = read(fileSrvDesc->fd, fileSrvDesc->handleBuf, 128);
    if (readSize < 128)
    {
        lseek(fileSrvDesc->fd, 0, SEEK_SET);
    }
    transferObj->packetCount = 1;
    transferObj->packetArray[0].packetData = fileSrvDesc->handleBuf;
    transferObj->packetArray[0].packetSize = readSize;
    return 0;
}
static int _FileServerTransferDone(void *pUsrData, const SS_EXP_TransferObject_t *transferObj)
{
    return 0;
}
static int _FileServerRemoteRead(void *pUsrData, char *pTransData, unsigned int u32DataSize)
{
    DBG("Read buf user %p->%p\n", pUsrData, pTransData);
    return 0;
}

static void *_RemoteAlloc(void *pUserData, unsigned int size)
{
    return malloc(size);
}

static void _RemoteFree(void *pUserData, void *buf)
{
    free(buf);
}

static int _TalkOpen(const char *url, void *privateData, unsigned char bRead, SS_EXP_WorkCfg_t *workCfg)
{
    struct TalkServerObj_s *pos = NULL;
    struct TalkServerObj_s *srvObj = NULL;
    int loopId = 0;

    srvObj = (struct TalkServerObj_s *)malloc(sizeof(struct TalkServerObj_s));
    if (!srvObj)
    {
        printf("no buffer!\n");
        return -1;
    }
    memset(srvObj, 0, sizeof(struct TalkServerObj_s));
    snprintf(srvObj->selfId, 64, "%s_%d", url, loopId);
    srvObj->bRead = bRead;
    INIT_LIST_HEAD(&srvObj->msgList);
    pthread_mutex_lock(&g_srvTalkMutex);
    list_for_each_entry(pos, &g_talkObjListHead, talkObjList)
    {
        if (!strncmp(pos->selfId, url, strlen(url)))
        {
            snprintf(srvObj->selfId, 64, "%s_%d", url, ++loopId);
        }
    }
    list_add_tail(&srvObj->talkObjList, &g_talkObjListHead);
    pthread_mutex_unlock(&g_srvTalkMutex);
    DBG("Open %s, bRead %d\n", srvObj->selfId, bRead);
    workCfg->pUserData = (void *)srvObj;
    return 0;
}
static int _TalkClose(void *pUsrData)
{
    struct TalkServerObj_s *srvObj = (struct TalkServerObj_s *)pUsrData;
    struct TalkMessageObj_s *posMsg = NULL;
    struct TalkMessageObj_s *posMsgN = NULL;

    pthread_mutex_lock(&g_srvTalkMutex);
    list_del(&srvObj->talkObjList);
    list_for_each_entry_safe(posMsg, posMsgN, &srvObj->msgList, msgList)
    {
        list_del(&posMsg->msgList);
        free(posMsg->message);
        free(posMsg);
    }
    pthread_mutex_unlock(&g_srvTalkMutex);
    DBG("Close %s\n", srvObj->selfId);
    free(srvObj);
    return 0;
}
static int _TalkWorkMsg(void *pUsrData, const char workMsg[SS_EXP_WORK_INFO_MSG_SIZE])
{
    DBG("[Work Message From Server]:%s", workMsg);
    return 0;
}
static int _TalkTransfer(void *pUsrData, SS_EXP_TransferObject_t *transferObj)
{
    struct TalkServerObj_s *srvObj = (struct TalkServerObj_s *)pUsrData;
    struct TalkMessageObj_s *posMsg = NULL;
    struct TalkMessageObj_s *posMsgN = NULL;
    struct timespec waitTime;

    pthread_mutex_lock(&g_srvTalkMutex);
    if (srvObj->isExit)
    {
        list_for_each_entry_safe(posMsg, posMsgN, &srvObj->msgList, msgList)
        {
            list_del(&posMsg->msgList);
            free(posMsg->message);
            free(posMsg);
        }
        pthread_mutex_unlock(&g_srvTalkMutex);
        usleep(100 * 1000);
        return 0;
    }
    if (!list_empty(&srvObj->msgList))
    {
        posMsg = list_last_entry(&srvObj->msgList, struct TalkMessageObj_s, msgList);
        list_del(&posMsg->msgList);
        pthread_mutex_unlock(&g_srvTalkMutex);
        transferObj->packetCount = 1;
        transferObj->packetArray[0].packetData = posMsg->message;
        transferObj->packetArray[0].packetSize = strlen(posMsg->message) + 1;
        free(posMsg);
        return 0;
    }
    clock_gettime(CLOCK_MONOTONIC, &waitTime);
    waitTime.tv_nsec += 100000000; //100ms
    if (waitTime.tv_nsec > 1000000000)
    {
        waitTime.tv_nsec %= 1000000000;
        waitTime.tv_sec++;
    }
    pthread_cond_timedwait(&g_srvTalkCond, &g_srvTalkMutex, &waitTime);
    pthread_mutex_unlock(&g_srvTalkMutex);
    return 0;
}
static int _TalkTransferDone(void *pUsrData, const SS_EXP_TransferObject_t *transferObj)
{
    if (transferObj->packetArray[0].packetData)
    {
        free(transferObj->packetArray[0].packetData);
    }
    return 0;
}
static int _TalkRemoteRead(void *pUsrData, char *pTransData, unsigned int u32DataSize)
{
    struct TalkServerObj_s *srvObj = (struct TalkServerObj_s *)pUsrData;

    printf("FROM[%s]: %s\n", srvObj->selfId, pTransData);
    return 0;
}
static void _Communication(void)
{
    struct TalkServerObj_s *pos = NULL;
    struct TalkMessageObj_s *msg = NULL;
    int loopId = 0;
    int isExit = 0;
    char getVal[1024] = {0};

    while (!isExit)
    {
        printf("Press 'c' to chat with someone.\n");
        printf("Press 'q' to exit.\n");
        scanf("%s", getVal);
        if (getVal[0] != 'c' || getVal[1] != '\0')
        {
            if (getVal[0] == 'q' && getVal[1] == '\0')
            {
                isExit = 1;
            }
            continue;
        }
        if (list_empty(&g_talkObjListHead))
        {
            printf("There is no one to chat with.\n");
            continue;
        }
        printf("Who do you want to chat with:\n");
        printf("--------------------------------->\n");
        loopId = 0;
        pthread_mutex_lock(&g_srvTalkMutex);
        list_for_each_entry(pos, &g_talkObjListHead, talkObjList)
        {
            if (pos->bRead)
            {
                continue;
            }
            printf("%d : %s\n", loopId++, pos->selfId);
        }
        pthread_mutex_unlock(&g_srvTalkMutex);
        printf("<---------------------------------\n");
        scanf("%s", getVal);
        loopId = atoi(getVal);
        if (loopId == 0 && (getVal[0] != '0' || getVal[1] != 0))
        {
            continue;
        }
        pthread_mutex_lock(&g_srvTalkMutex);
        list_for_each_entry(pos, &g_talkObjListHead, talkObjList)
        {
            if (pos->bRead)
            {
                continue;
            }
            if (!loopId)
            {
                break;
            }
            loopId--;
        }
        pthread_mutex_unlock(&g_srvTalkMutex);
        if (&pos->talkObjList != &g_talkObjListHead)
        {
            printf("[Message to %s]:\n", pos->selfId);
            scanf("%s", getVal);
            msg = malloc(sizeof(struct TalkMessageObj_s));
            if (!msg)
            {
                continue;
            }
            msg->message = malloc(strlen(getVal) + 1);
            if (!msg->message)
            {
                free(msg);
                continue;
            }
            strcpy(msg->message, getVal);
            list_add_tail(&msg->msgList, &pos->msgList);
            pthread_mutex_lock(&g_srvTalkMutex);
            pthread_cond_broadcast(&g_srvTalkCond);
            pthread_mutex_unlock(&g_srvTalkMutex);
        }
    }
    pthread_mutex_lock(&g_srvTalkMutex);
    list_for_each_entry(pos, &g_talkObjListHead, talkObjList)
    {
        pos->isExit = 1;
    }
    pthread_cond_broadcast(&g_srvTalkCond);
    pthread_mutex_unlock(&g_srvTalkMutex);
}
static void _BrokenPipe(int num)
{
    printf("MASK BROKEN PIPE.\n");
}

static void _PrintHelp(const char *prog)
{
    printf("Program %s is a sample code of ss_exp within a file server and message talk function over TCP connection.\n", prog);
    printf("Build info : Commit %s, build by %s, date %s\n", GIT_COMMIT, BUILD_OWNER, BUILD_DATE);
    printf("Usage:\n");
    printf("%s --file-server|-s [file]   -----Create a file server with specified file path.\n", prog);
    printf("%s --talk-server|-t [url]    -----Create a chat server by config url name.\n", prog);
    printf("%s --talk-client|-c [addr]   -----Create a chat client to chat wich server, addr format: exp://xxx.xxx.xxx.xxx:port/url.\n", prog);
}

int main(int argc, char **argv)
{
    void *instance = NULL;
    int stage = 0;
    int result = 0;
    int optIndex = 0;
    int port = 12123;
    char url[64] = {0};
    char filePath[128] = {0};
    SS_EXP_TransferActCb_t transCb;
    pthread_condattr_t condAttr;
    struct option longOpt[] = {
        {"file-server", required_argument, 0, 's'},
        {"talk-server", required_argument, 0, 't'},
        {"talk-client", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    signal(SIGPIPE, _BrokenPipe);
    while ((result = getopt_long(argc, argv, "s:t:c:p:", longOpt, &optIndex)) != -1)
    {
        switch (result)
        {
        case 's':
            if (stage & (E_USING_STAGE_TALK_SERVER | E_USING_STAGE_TALK_CLIENT))
            {
                break;
            }
            snprintf(filePath, 128, "%s", optarg);
            printf("SERVER FILE: %s\n", filePath);
            stage = E_USING_STAGE_FILE_SERVER;
            break;
        case 'p':
            if (stage & E_USING_STAGE_TALK_CLIENT)
            {
                break;
            }
            port = atoi(optarg);
            break;
        case 't':
            if (stage & (E_USING_STAGE_FILE_SERVER | E_USING_STAGE_TALK_CLIENT))
            {
                break;
            }
            snprintf(url, 64, "%s", optarg);
            stage = E_USING_STAGE_TALK_SERVER;
            break;
        case 'c':
            if (stage & (E_USING_STAGE_FILE_SERVER | E_USING_STAGE_TALK_SERVER))
            {
                break;
            }
            snprintf(url, 64, "%s", optarg);
            stage = E_USING_STAGE_TALK_CLIENT;
            break;
        default:
            break;
        }
    }
    if (stage & E_USING_STAGE_FILE_SERVER)
    {
        SS_EXP_SourceAttr_t sourceAttr;
        char getVal = 0;

        transCb.fpSsExpOpen            = _FileServerOpen;
        transCb.fpSsExpClose           = _FileServerClose;
        transCb.fpSsExpTransfer        = _FileServerTransfer;
        transCb.fpSsExpTransferDone    = _FileServerTransferDone;
        transCb.fpSsExpRemoteRead      = _FileServerRemoteRead;
        transCb.fpSsExpRemoteReadAlloc = _RemoteAlloc;
        transCb.fpSsExpRemoteReadFree  = _RemoteFree;
        transCb.fpSsExpWorkMsg         = NULL;
        memset(&sourceAttr, 0, sizeof(SS_EXP_SourceAttr_t));
        sourceAttr.uintPort    = port;
        sourceAttr.bWithNoSync = 1;
        sourceAttr.privateData = (void *)filePath;
        instance = SS_EXP_SourceInit(&sourceAttr, &transCb);
        if (!instance)
        {
            printf("File server init error!\n");
            return -1;
        }
        SS_EXP_SourceStart(instance);
        printf("Create file server, PORT: %d\n", sourceAttr.uintPort);
        while (getVal != 'q')
        {
            printf("Press 'q' to exit!\n");
            getVal = getchar();
        }
        SS_EXP_SourceStop(instance);
        return SS_EXP_SourceDeinit(instance);
    }
    pthread_condattr_init(&condAttr);
    pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
    pthread_cond_init(&g_srvTalkCond, &condAttr);
    if (stage & E_USING_STAGE_TALK_SERVER)
    {
        int i = 0;
        SS_EXP_WorkInfo_t workInfo;
        SS_EXP_SourceAttr_t sourceAttr;
        WORK_HANDLE *readWork[10];
        WORK_HANDLE *writeWork[10];

        transCb.fpSsExpOpen            = _TalkOpen;
        transCb.fpSsExpClose           = _TalkClose;
        transCb.fpSsExpTransfer        = _TalkTransfer;
        transCb.fpSsExpTransferDone    = _TalkTransferDone;
        transCb.fpSsExpRemoteRead      = _TalkRemoteRead;
        transCb.fpSsExpRemoteReadAlloc = _RemoteAlloc;
        transCb.fpSsExpRemoteReadFree  = _RemoteFree;
        transCb.fpSsExpWorkMsg         = NULL;
        memset(&sourceAttr, 0, sizeof(SS_EXP_SourceAttr_t));
        sourceAttr.uintPort = port;
        instance = SS_EXP_SourceInit(&sourceAttr, &transCb);
        if (!instance)
        {
            printf("Talk server init error!\n");
            return -1;
        }
        memset(&workInfo, 0, sizeof(SS_EXP_WorkInfo_t));
        for (i = 0; i < 10; i++)
        {
            workInfo.enWorkMode = E_SS_EXP_SRC_READ_ONLY;
            snprintf(workInfo.workMsg, SS_EXP_WORK_INFO_MSG_SIZE, "Talk server: this is %d read only work\n", i);
            readWork[i] = SS_EXP_SourceCreateWork(instance, url, NULL, &workInfo);
            workInfo.enWorkMode = E_SS_EXP_SRC_WRITE_ONLY;
            snprintf(workInfo.workMsg, SS_EXP_WORK_INFO_MSG_SIZE, "Talk server: this is %d write only work\n", i);
            writeWork[i] = SS_EXP_SourceCreateWork(instance, url, NULL, &workInfo);
        }
        SS_EXP_SourceStart(instance);
        printf("Create talk server, URL:%s PORT: %d\n", url, sourceAttr.uintPort);
        _Communication();
        SS_EXP_SourceStop(instance);
        for (i = 0; i < 10; i++)
        {
            SS_EXP_SourceDestroyWork(writeWork[i]);
            SS_EXP_SourceDestroyWork(readWork[i]);
        }
        SS_EXP_SourceDeinit(instance);
        pthread_condattr_destroy(&condAttr);
        return 0;
    }
    if (stage & E_USING_STAGE_TALK_CLIENT)
    {
        SS_EXP_SinkAttr_t sinkAttr;

        transCb.fpSsExpOpen            = _TalkOpen;
        transCb.fpSsExpClose           = _TalkClose;
        transCb.fpSsExpWorkMsg         = _TalkWorkMsg;
        transCb.fpSsExpTransfer        = _TalkTransfer;
        transCb.fpSsExpTransferDone    = _TalkTransferDone;
        transCb.fpSsExpRemoteRead      = _TalkRemoteRead;
        transCb.fpSsExpRemoteReadAlloc = _RemoteAlloc;
        transCb.fpSsExpRemoteReadFree  = _RemoteFree;
        sinkAttr.pAddress              = url;
        instance = SS_EXP_SinkInit(&sinkAttr, &transCb);
        if (!instance)
        {
            printf("Talk client init error!\n");
            return -1;
        }
        printf("Create talk client, ADDR:%s\n", url);
        _Communication();
        SS_EXP_SinkDeinit(instance);
        pthread_condattr_destroy(&condAttr);
        return 0;
    }
    _PrintHelp(argv[0]);
    return 0;
}

