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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>

#include "list.h"
#include "ss_thread.h"
#include "ss_exp.h"

#define SEL_WAIT_TIMEOUT_MS 500
#define SEL_WAIT_TIMEOUT_CHECK_CNT 30
#define SEL_TRANSFER_RETRY_CHECK_CNT 300
#define MAX_CONNECTIONS_KEY 32
#define MAX_URL_LENGTH 64
#define MAX_SINK_SYNC_INFO_STRING_LENGTH 128
#define DEBUG_TRANSFER_SPEED 0

#define DBG(_fmt, ...) printf("[DBG][%s][%d]: " _fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ERR(_fmt, ...) printf("[ERR][%s][%d]: " _fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

typedef enum
{
    E_SS_EXP_TRANS_BY_HEAD,
    E_SS_EXP_TRANS_NO_HEAD,
    E_SS_EXP_TRANS_HEAD_DATA,
    E_SS_EXP_TRANS_DATA,
    E_SS_EXP_TRANS_CLOSE
} SS_EXP_TransState_e;

typedef struct SS_EXP_WorkInfoListData_s
{
    struct list_head  stWorkInfoList;
    void *            privateData;
    SS_EXP_WorkInfo_t stWorkInfo;
} SS_EXP_WorkInfoListData_t;

typedef struct SS_EXP_SourceWorkInfo_s
{
    struct list_head stWorkHashList;
    struct list_head stWorkInfoList;
    char             pUrl[MAX_URL_LENGTH];
} SS_EXP_SourceWorkInfo_t;

typedef struct SS_EXP_DataPackageHead_s
{
    /* To do ... CRC Check */
    unsigned int dataSize;
} SS_EXP_DataPackageHead_t;

typedef struct SS_EXP_DataRecvInfo_s
{
    SS_EXP_DataPackageHead_t headData;
    char *                   recvData;
    unsigned int             loopSize;
    unsigned int             timeOutLoopCnt;
} SS_EXP_DataRecvInfo_t;

#if DEBUG_TRANSFER_SPEED
typedef struct SS_EXP_TransMessure_s
{
    unsigned int       transDataSize;
    unsigned int       transAccessCnt;
    unsigned long long messureStartTime;
} SS_EXP_TransMessure_t;
#endif

typedef struct SS_EXP_ConnectDesc_s
{
    struct list_head       stDescList;
    SS_EXP_TransferActCb_t stTransCb;
    SS_EXP_TransState_e    enTransState;
#if DEBUG_TRANSFER_SPEED
    SS_EXP_TransMessure_t  transMessure;
#endif
    SS_EXP_DataRecvInfo_t recvInfo;
    int                   intClientFd;
    void *                cmdThreadHandle;
    void *                dataThreadHandle;
    void *                pUsrData;
} SS_EXP_ConnectDesc_t;

typedef struct SS_EXP_ConnectList_s
{
    struct list_head stList;
    struct list_head stDescListW;
    struct list_head stDescListR;
    char             pSyncString[MAX_SINK_SYNC_INFO_STRING_LENGTH];
} SS_EXP_ConnectList_t;

typedef enum
{
    E_SS_EXP_CONNECT_OPEN,
    E_SS_EXP_CONNECT_BREAK,
    E_SS_EXP_CONNECT_CLOSE
} SS_EXP_ConnectState_e;

typedef struct SS_EXP_ConnectCmd_s
{
    SS_EXP_ConnectState_e enState;
    void *                pCmdData;
} SS_EXP_ConnectCmd_t;

typedef struct SS_EXP_SinkInstance_s
{
    struct list_head       stDescListW;
    struct list_head       stDescListR;
    SS_EXP_TransferActCb_t stTransferAct;
    int                    clientFd;
    unsigned char          bIdle;
    char                   addr[64];
    char                   url[MAX_URL_LENGTH];
    char                   macAddr[32];
    char                   ip[16];
    unsigned int           uintPort;
    void *                 cmdThreadHandle;
    void *                 privateData;
} SS_EXP_SinkInstance_t;

typedef struct SS_EXP_SourceInstance_s
{
    struct list_head       connectListHead;
    SS_EXP_TransferActCb_t stTransferAct;
    struct list_head       sourceWorkHash[MAX_CONNECTIONS_KEY];
    unsigned char          bWithNoSync;
    int                    socketFd;
    unsigned int           uintPort;
    void *                 cmdThreadHandle;
    void *                 privateData;
    struct sockaddr_in     stSockaddrIn;
} SS_EXP_SourceInstance_t;

static unsigned int _SS_EXP_WorkInfoHash(const char *url)
{
    unsigned int hash = 0;
    while (*url)
    {
        hash += *url++;
    }
    return hash % MAX_CONNECTIONS_KEY;
}

static struct list_head *_SS_EXP_GetSourceWorkInfoHead(SS_EXP_SourceInstance_t *pInstance, const char *pUrl)
{
    SS_EXP_SourceWorkInfo_t *pos = NULL;
    unsigned int key             = _SS_EXP_WorkInfoHash(pUrl);

    list_for_each_entry(pos, &pInstance->sourceWorkHash[key], stWorkHashList)
    {
        if (!strcmp(pUrl, pos->pUrl))
        {
            return &pos->stWorkInfoList;
        }
    }
    return NULL;
}
static int _SS_EXP_TcpSourceInit(unsigned int uintPort, struct sockaddr_in *pstSockaddrIn)
{
    int on = 1;
    int intServerLen = 0;
    int intSrvFd;
    int fdflags = 0;

    ASSERT(pstSockaddrIn);
    intSrvFd = socket(AF_INET, SOCK_STREAM, 0);
    if (intSrvFd == -1)
    {
        perror("socket");
        return -1;
    }
    if (-1 == setsockopt(intSrvFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        perror("setsockopt");
        close(intSrvFd);

        return -1;
    }

    if (-1 == setsockopt(intSrvFd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)))
    {
        perror("setsockopt");
        close(intSrvFd);

        return -1;
    }
    fdflags = fcntl(intSrvFd, F_GETFL, 0);
    fcntl(intSrvFd, F_SETFL, fdflags | O_NONBLOCK);
    memset(pstSockaddrIn, 0, sizeof(struct sockaddr_in));
    pstSockaddrIn->sin_family = AF_INET;
    pstSockaddrIn->sin_addr.s_addr = htonl(INADDR_ANY);
    pstSockaddrIn->sin_port = htons(uintPort);
    intServerLen = sizeof(struct sockaddr_in);
    if (-1 == bind(intSrvFd, (struct sockaddr *)pstSockaddrIn, intServerLen))
    {
        perror("bind");
        close(intSrvFd);

        return -1;
    }
    if (-1 == listen(intSrvFd, 5))
    {
        perror("listen");
        close(intSrvFd);

        return -1;
    }

    return intSrvFd;
}
static int _SS_EXP_TcpSourceDeinit(int intFd)
{
    close(intFd);

    return 0;
}
static int _SS_EXP_TcpSourceAccept(int intFd, struct sockaddr_in *pstSockaddrIn)
{
    int intClientLen = 0;
    int intClientFd = -1;
    int fdflags = 0;
    struct timeval tv;
    fd_set fdsr;
    int ret = 0;
    int on = 1;

    ASSERT(pstSockaddrIn);
    FD_ZERO(&fdsr);
    FD_SET(intFd, &fdsr);

    tv.tv_sec = 0;
    tv.tv_usec = 1000 * SEL_WAIT_TIMEOUT_MS;
    ret = select(intFd + 1, &fdsr, NULL, NULL, &tv);
    if (ret == 1)
    {
        intClientLen = sizeof(struct sockaddr_in);
        intClientFd = accept(intFd, (struct sockaddr *)pstSockaddrIn, (socklen_t *)&intClientLen);
        if (intClientFd == -1)
        {
            perror("accept");
            return -1;
        }
        if (-1 == setsockopt(intClientFd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)))
        {
            close(intClientFd);
            perror("setsockopt");
            return -1;
        }
        fdflags = fcntl(intFd, F_GETFL, 0);
        fcntl(intClientFd, F_SETFL, fdflags | O_NONBLOCK);
    }

    return intClientFd;
}
static int _SS_EXP_TcpSinkInit(void)
{
    int socketFd;

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1)
    {
        perror("socket");
    }

    return socketFd;
}
static int _SS_EXP_TcpSinkConnect(int intFd, const char *pServerIp, unsigned int uintPort)
{
    struct sockaddr_in stAddress;
    int intResult;
    int len;
    int fdflags = 0;
    int on = 1;

    memset(&stAddress, 0, sizeof(struct sockaddr_in));
    stAddress.sin_family = AF_INET;
    stAddress.sin_port = htons(uintPort);
    if (inet_pton(AF_INET, pServerIp, &stAddress.sin_addr) <= 0)
    {
        ERR("inet_pton error for %s\n", pServerIp);
        return -1;
    }
    if (-1 == setsockopt(intFd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)))
    {
        perror("setsockopt");
        ERR("setsockopt error for %s\n", pServerIp);
        return -1;
    }
    fdflags = fcntl(intFd, F_GETFL, 0);
    fcntl(intFd, F_SETFL, fdflags | O_NONBLOCK);
    len = sizeof(struct sockaddr_in);
    intResult = connect(intFd, (struct sockaddr *)&stAddress, len);
    if (intResult != 0)
    {
        if (errno == EINPROGRESS)
        {
            unsigned int timeOutCnt = 0;
            unsigned char bEndLoop = 0;
            do
            {
                struct timeval tv;
                fd_set fdsr, fdsw;

                FD_ZERO(&fdsr);
                FD_SET(intFd, &fdsr);
                FD_ZERO(&fdsw);
                FD_SET(intFd, &fdsw);

                tv.tv_sec = 0;
                tv.tv_usec = 1000 * SEL_WAIT_TIMEOUT_MS;
                intResult = select(intFd + 1, &fdsr, &fdsw, NULL, &tv);
                switch (intResult)
                {
                    case 0:
                        timeOutCnt++;
                        break;
                    case 1:
                        if (FD_ISSET(intFd, &fdsw))
                        {
                            DBG("Connect success!\n");
                            bEndLoop = 1;
                        }
                        break;
                    case 2:
                        {
                            int err = 0;
                            int errlen = sizeof(err);

                            if (getsockopt(intFd, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&errlen) == -1)
                            {
                                fprintf(stderr, "getsockopt(SO_ERROR): %s", strerror(errno));
                            }
                            if (err)
                            {
                                errno = err;
                                fprintf(stderr, "connect error:%s\n", strerror(errno));

                            }
                        }
                        bEndLoop = 1;
                        break;
                    default:
                        perror("connect select");
                        break;
                }
            } while (timeOutCnt < SEL_WAIT_TIMEOUT_CHECK_CNT && !bEndLoop);
        }
    }

    return intResult;
}
static int _SS_EXP_TcpDeinit(int intFd)
{
    close(intFd);

    return 0;
}

static int _SS_EXP_TcpTimeWaitData(int intFd, char *pTransData, unsigned int u32DataSize, unsigned int waitMs)
{
    int retVal = 0;
    struct timeval tv;
    fd_set fdsr;

    FD_ZERO(&fdsr);
    FD_SET(intFd, &fdsr);
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * waitMs;
    retVal = select(intFd + 1, &fdsr, NULL, NULL, &tv);
    if (retVal < 0)
    {
        perror("Error accepting connection");
        return retVal;
    }
    if (!FD_ISSET(intFd, &fdsr))
    {
        return -1;
    }
    retVal = recv(intFd, pTransData, u32DataSize, 0);
    if (retVal == -1 && (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK))
    {
        return 0;
    }
    return retVal;
}

static int _SS_EXP_TcpWaitData(int intFd, char *pTransData, unsigned int u32DataSize)
{
    int retVal = 0;
    int intTotalSize = 0;
    unsigned int timeOutCnt = 0;
    struct timeval tv;
    fd_set fdsr;

    tv.tv_sec = 0;
    tv.tv_usec = 1000 * SEL_WAIT_TIMEOUT_MS;
    do
    {
        FD_ZERO(&fdsr);
        FD_SET(intFd, &fdsr);
        retVal = select(intFd + 1, &fdsr, NULL, NULL, &tv);
        if (retVal < 0)
        {
            perror("Error accepting connection");
            return retVal;
        }
        if (!FD_ISSET(intFd, &fdsr))
        {
            timeOutCnt++;
            if (timeOutCnt < SEL_TRANSFER_RETRY_CHECK_CNT)
            {
                usleep(1000 * 10);
                continue;
            }
            DBG("Transfer timeout fd %d!\n", intFd);
            return -1;
        }
        timeOutCnt = 0;
        retVal = recv(intFd, pTransData + intTotalSize, u32DataSize - intTotalSize, 0);
        if (retVal == -1 && (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK))
        {
            DBG("SEND DATA %d\n", errno);
            continue;
        }
        if (retVal <= 0)
        {
            DBG("RECV Socket broke!\n");
            return -1;
        }
        intTotalSize += retVal;
    } while (intTotalSize < u32DataSize);

    return intTotalSize;
}
static int _SS_EXP_TcpTransfer(int intFd, char *pTransData, unsigned int u32DataSize)
{
    struct timeval tv;
    fd_set fdsr;
    int intTotalSize = 0;
    unsigned int timeOutCnt = 0;
    int retVal = 0;

    tv.tv_sec = 0;
    tv.tv_usec = 1000 * SEL_WAIT_TIMEOUT_MS;
    do
    {
        FD_ZERO(&fdsr);
        FD_SET(intFd, &fdsr);
        retVal = select(intFd + 1, NULL, &fdsr, NULL, &tv);
        if (retVal < 0)
        {
            perror("Error accepting connection");
            return retVal;
        }
        if (!FD_ISSET(intFd, &fdsr))
        {
            timeOutCnt++;
            if (timeOutCnt < SEL_TRANSFER_RETRY_CHECK_CNT)
            {
                usleep(1000 * 10);
                continue;
            }
            DBG("Transfer timeout fd %d!\n", intFd);
            return -1;
        }
        timeOutCnt = 0;
        retVal = send(intFd, pTransData + intTotalSize, u32DataSize - intTotalSize, 0);
        if (retVal == -1 && (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK))
        {
            continue;
        }
        if (retVal <= 0)
        {
            DBG("SEND Socket broke!\n");
            return -1;
        }
        intTotalSize += retVal;
    } while (intTotalSize < u32DataSize);

    return intTotalSize;
}
static void _SS_EXP_GenRandomMacAddress(char* macAddress)
{
    srand(time(NULL));
    for (int i = 0; i < 6; i++)
    {
        int randomByte = rand() % 256;
        sprintf(macAddress + i * 3, "%02X", randomByte);
        if (i < 5)
        {
            sprintf(macAddress + i * 3 + 2, ":");
        }
    }
}
static int _SS_EXP_GetMyMacAddr(char *pstrMacAddr, unsigned int uintLen)
{
    struct ifreq ifreq;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }
    strcpy(ifreq.ifr_name, "eth0");
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
        strcpy(ifreq.ifr_name, "wlan0");
        if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
        {
            DBG("Can not get mac address, using a random one!\n");
            _SS_EXP_GenRandomMacAddress(pstrMacAddr);
        }
    }
    close(sock);
    return snprintf(pstrMacAddr, uintLen, "%02X:%02X:%02X:%02X:%02X:%02X",
                    (unsigned char)ifreq.ifr_hwaddr.sa_data[0], (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
                    (unsigned char)ifreq.ifr_hwaddr.sa_data[2], (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
                    (unsigned char)ifreq.ifr_hwaddr.sa_data[4], (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
}
static int _SS_EXP_CmdSend(void *threadHandle, SS_EXP_ConnectState_e state, void *data)
{
    struct ss_thread_user_data sendData;
    SS_EXP_ConnectCmd_t connectCmd;

    connectCmd.enState  = state;
    connectCmd.pCmdData = data;
    sendData.data       = &connectCmd;
    sendData.size       = sizeof(SS_EXP_ConnectCmd_t);
    sendData.real_size  = 0;
    return ss_thread_send(threadHandle, &sendData);
}
static struct list_head *_SS_EXP_SourceListFind(struct list_head *pListData, void *pKey)
{
    char *pKeyString = NULL;
    SS_EXP_ConnectList_t *pConnectList = NULL;

    pConnectList = list_entry(pListData, SS_EXP_ConnectList_t, stList);
    pKeyString = pKey;
    if (!strcmp(pConnectList->pSyncString, pKeyString))
    {
        return pListData;
    }

    return pListData->next;
}
static void *_SS_EXP_MainWrite(struct ss_thread_buffer *pstBuf)
{
    int ret = 0;
    int i = 0;
    SS_EXP_ConnectDesc_t *pstConnectDesc = NULL;
    SS_EXP_TransState_e enTransState = E_SS_EXP_TRANS_BY_HEAD;
    SS_EXP_DataPackageHead_t packageHead;
    SS_EXP_TransferObject_t transferObj;

    pstConnectDesc = (SS_EXP_ConnectDesc_t *)pstBuf->buf;
    enTransState = pstConnectDesc->enTransState;

    packageHead.dataSize = 0;
    memset(&transferObj, 0, sizeof(SS_EXP_TransferObject_t));
    while (1)
    {
        if (enTransState == E_SS_EXP_TRANS_BY_HEAD)
        {
            pstConnectDesc->stTransCb.fpSsExpTransfer(pstConnectDesc->pUsrData, &transferObj);
            if (transferObj.packetCount > SS_EXP_PACKET_SLICE_CNT)
            {
                ERR("Packet count error!\n");
                break;
            }
            for (i = 0; i < transferObj.packetCount; i++)
            {
                packageHead.dataSize += transferObj.packetArray[i].packetSize;
            }
            enTransState = E_SS_EXP_TRANS_HEAD_DATA;
            continue;
        }
        if (enTransState == E_SS_EXP_TRANS_NO_HEAD)
        {
            pstConnectDesc->stTransCb.fpSsExpTransfer(pstConnectDesc->pUsrData, &transferObj);
            if (transferObj.packetCount > SS_EXP_PACKET_SLICE_CNT)
            {
                ERR("Packet count error!\n");
                break;
            }
            enTransState = E_SS_EXP_TRANS_DATA;
            continue;
        }
        if (enTransState == E_SS_EXP_TRANS_HEAD_DATA)
        {
            ret = _SS_EXP_TcpTransfer(pstConnectDesc->intClientFd, (char *)&packageHead, sizeof(SS_EXP_DataPackageHead_t));
            if (ret == -1)
            {
                enTransState = E_SS_EXP_TRANS_CLOSE;
                pstConnectDesc->stTransCb.fpSsExpTransferDone(pstConnectDesc->pUsrData, &transferObj);
                continue;
            }
            enTransState = E_SS_EXP_TRANS_DATA;
#if DEBUG_TRANSFER_SPEED
            pstConnectDesc->transMessure.transDataSize += ret;
#endif
            continue;

        }
        if (enTransState == E_SS_EXP_TRANS_DATA)
        {
#if DEBUG_TRANSFER_SPEED
            struct timespec curr;
            unsigned long long currMs;
#endif
            if (transferObj.packetCount)
            {
                for (i = 0; i < transferObj.packetCount; i++)
                {
                    ret = _SS_EXP_TcpTransfer(pstConnectDesc->intClientFd, transferObj.packetArray[i].packetData,
                                             transferObj.packetArray[i].packetSize);
                    if (ret == -1)
                    {
                        enTransState = E_SS_EXP_TRANS_CLOSE;
                        pstConnectDesc->stTransCb.fpSsExpTransferDone(pstConnectDesc->pUsrData, &transferObj);
                        break;
                    }
#if DEBUG_TRANSFER_SPEED
                    pstConnectDesc->transMessure.transDataSize += ret;
#endif
                }
                if (i != transferObj.packetCount)
                {
                    continue;
                }
            }
#if DEBUG_TRANSFER_SPEED
            pstConnectDesc->transMessure.transAccessCnt++;
            clock_gettime(CLOCK_MONOTONIC, &curr);
            currMs = curr.tv_sec * 1000 + curr.tv_nsec / 1000000;
            if (pstConnectDesc->transMessure.messureStartTime + 5000 < currMs)
            {
                unsigned long long diff = currMs - pstConnectDesc->transMessure.messureStartTime;
                DBG("Access count %d, Transfer speed: %.2fkbps.\n", pstConnectDesc->transMessure.transAccessCnt,
                    (((float)pstConnectDesc->transMessure.transDataSize * 1000) / 1024) / diff);
                pstConnectDesc->transMessure.messureStartTime = currMs;
                pstConnectDesc->transMessure.transDataSize = 0;
                pstConnectDesc->transMessure.transAccessCnt = 0;
            }
#endif
            pstConnectDesc->stTransCb.fpSsExpTransferDone(pstConnectDesc->pUsrData, &transferObj);
            break;

        }
        if (enTransState == E_SS_EXP_TRANS_CLOSE)
        {
            ss_thread_stop(pstConnectDesc->dataThreadHandle); //stop myself
            _SS_EXP_CmdSend(pstConnectDesc->cmdThreadHandle, E_SS_EXP_CONNECT_BREAK, pstConnectDesc);
            DBG("WRITE BROKE! desc %p\n", pstConnectDesc);
            break;
        }
        ASSERT(0);
    }

    return NULL;
}
static void *_SS_EXP_MainRead(struct ss_thread_buffer *pstBuf)
{
    int ret = 0;
    SS_EXP_ConnectDesc_t *pstConnectDesc = NULL;

    pstConnectDesc = (SS_EXP_ConnectDesc_t *)pstBuf->buf;
    if (pstConnectDesc->recvInfo.loopSize < sizeof(SS_EXP_DataPackageHead_t))
    {
        ret = _SS_EXP_TcpTimeWaitData(pstConnectDesc->intClientFd,
                                  (char *)&pstConnectDesc->recvInfo.headData + pstConnectDesc->recvInfo.loopSize,
                                  sizeof(SS_EXP_DataPackageHead_t) - pstConnectDesc->recvInfo.loopSize, SEL_WAIT_TIMEOUT_MS);
        if (ret < 0)
        {
            if (pstConnectDesc->recvInfo.timeOutLoopCnt < SEL_WAIT_TIMEOUT_CHECK_CNT)
            {
                pstConnectDesc->recvInfo.timeOutLoopCnt++;
                return NULL;
            }
            goto CONNECT_BROKE;
        }
        pstConnectDesc->recvInfo.timeOutLoopCnt = 0;
        pstConnectDesc->recvInfo.loopSize += ret;
        if (pstConnectDesc->recvInfo.loopSize < sizeof(SS_EXP_DataPackageHead_t))
        {
            return NULL;
        }
        if (!pstConnectDesc->recvInfo.headData.dataSize)
        {
            /* Receive an empty header is userd for testing heart beat on the TCP connection. */
            pstConnectDesc->recvInfo.loopSize = 0;
            return NULL;
        }
        if (pstConnectDesc->stTransCb.fpSsExpRemoteRead && pstConnectDesc->stTransCb.fpSsExpRemoteReadAlloc)
        {
            pstConnectDesc->recvInfo.recvData = pstConnectDesc->stTransCb.fpSsExpRemoteReadAlloc(pstConnectDesc->pUsrData, pstConnectDesc->recvInfo.headData.dataSize);
        }
        if (!pstConnectDesc->recvInfo.recvData)
        {
            ERR("Alloc error!\n");
            goto CONNECT_BROKE;
        }
    }
    ASSERT(pstConnectDesc->recvInfo.recvData);
    ret = _SS_EXP_TcpTimeWaitData(pstConnectDesc->intClientFd, pstConnectDesc->recvInfo.recvData +
                                  pstConnectDesc->recvInfo.loopSize - sizeof(SS_EXP_DataPackageHead_t),
                                  pstConnectDesc->recvInfo.headData.dataSize - (pstConnectDesc->recvInfo.loopSize - sizeof(SS_EXP_DataPackageHead_t)),
                                  SEL_WAIT_TIMEOUT_MS);
    if (ret < 0)
    {
        if (pstConnectDesc->recvInfo.timeOutLoopCnt < SEL_WAIT_TIMEOUT_CHECK_CNT)
        {
            pstConnectDesc->recvInfo.timeOutLoopCnt++;
            return NULL;
        }
        goto CONNECT_BROKE;
    }
    pstConnectDesc->recvInfo.timeOutLoopCnt = 0;
    pstConnectDesc->recvInfo.loopSize += ret;
    if (pstConnectDesc->recvInfo.loopSize == sizeof(SS_EXP_DataPackageHead_t) + pstConnectDesc->recvInfo.headData.dataSize)
    {
        if (pstConnectDesc->stTransCb.fpSsExpRemoteRead && pstConnectDesc->stTransCb.fpSsExpRemoteReadFree)
        {
            pstConnectDesc->stTransCb.fpSsExpRemoteRead(pstConnectDesc->pUsrData, pstConnectDesc->recvInfo.recvData,
                                                        pstConnectDesc->recvInfo.headData.dataSize);
            pstConnectDesc->stTransCb.fpSsExpRemoteReadFree(pstConnectDesc->pUsrData, pstConnectDesc->recvInfo.recvData);
        }
        pstConnectDesc->recvInfo.recvData = NULL;
        pstConnectDesc->recvInfo.loopSize = 0;
    }
    return NULL;

CONNECT_BROKE:
    ss_thread_stop(pstConnectDesc->dataThreadHandle); //stop myself
    _SS_EXP_CmdSend(pstConnectDesc->cmdThreadHandle, E_SS_EXP_CONNECT_BREAK, pstConnectDesc);
    DBG("READ BROKE! desc %p\n", pstConnectDesc);
    return NULL;
}

static int _SS_EXP_SourceReaction(SS_EXP_SourceInstance_t *pInstance, int clientFd)
{
    struct list_head *pstSourceWorkInfoListHead = NULL;
    struct list_head *pstSrcConnectList = NULL;
    struct ss_thread_attr attr;
    SS_EXP_ConnectList_t *pstConnect = NULL;
    SS_EXP_ConnectDesc_t *pstConnectDesc = NULL;
    SS_EXP_WorkInfoListData_t *posWorkInfoList = NULL;
    SS_EXP_SourceWorkMode_e enSrcMode = E_SS_EXP_SRC_WORK_NONE;
    char pSyncString[MAX_SINK_SYNC_INFO_STRING_LENGTH + 1];
    char pUrl[MAX_URL_LENGTH];
    char pTargetMac[32];
    char pTargetPid[32];
    unsigned char bRead = 0;
    void *privateData = NULL;
    int ret = 0;

    if (!pInstance)
    {
        _SS_EXP_TcpDeinit(clientFd);
        ERR("%s: null pointer!\n", __FUNCTION__);
        return -1;
    }
    if (pInstance->bWithNoSync)
    {
        pstConnect = (SS_EXP_ConnectList_t *)malloc(sizeof(SS_EXP_ConnectList_t));
        ASSERT(pstConnect);
        memset(pstConnect, 0, sizeof(SS_EXP_ConnectList_t));
        snprintf(pstConnect->pSyncString, MAX_SINK_SYNC_INFO_STRING_LENGTH, "RAW_ONLY_CLIENT_%d", clientFd);
        snprintf(pUrl, MAX_URL_LENGTH, "CLIENT_%d", clientFd);
        list_add_tail(&pstConnect->stList, &pInstance->connectListHead);
        INIT_LIST_HEAD(&pstConnect->stDescListW);
        INIT_LIST_HEAD(&pstConnect->stDescListR);
        enSrcMode = E_SS_EXP_SRC_WRITE_ONLY;
        privateData = pInstance->privateData;
    }
    else
    {
        memset(pSyncString, 0, MAX_SINK_SYNC_INFO_STRING_LENGTH + 1);
        ret = _SS_EXP_TcpWaitData(clientFd, pSyncString, MAX_SINK_SYNC_INFO_STRING_LENGTH);
        if (ret < 0)
        {
            _SS_EXP_TcpDeinit(clientFd);
            ERR("TCP SRC get sync error, ret %d\n", ret);
            return -1;
        }
        memset(pTargetMac, 0, 32);
        memset(pTargetPid, 0, 32);
        //example: 33:EE:21:2E:8F:11/1234/main
        DBG("GET ID_STR: %s\n", pSyncString);
        sscanf(pSyncString, "%[^/]/%[^/]/%[^/]", pTargetMac, pTargetPid, pUrl);
        DBG("Mac is %s\n", pTargetMac);
        DBG("Pid is %s\n", pTargetPid);
        DBG("Url is %s\n", pUrl);
        pstSourceWorkInfoListHead = _SS_EXP_GetSourceWorkInfoHead(pInstance, pUrl);
        if (!pstSourceWorkInfoListHead || list_empty(pstSourceWorkInfoListHead))
        {
            SS_EXP_WorkInfo_t stEmptyWorkInfo;

            memset(&stEmptyWorkInfo, 0, sizeof(SS_EXP_WorkInfo_t));
            _SS_EXP_TcpTransfer(clientFd, (char *)&stEmptyWorkInfo, sizeof(SS_EXP_WorkInfo_t));
            _SS_EXP_TcpDeinit(clientFd);
            ERR("Can not find url->[%s] in work list.\n", pUrl);
            return -1;
        }
        pstSrcConnectList = list_find(&pInstance->connectListHead, pSyncString, _SS_EXP_SourceListFind);
        if (pstSrcConnectList == &pInstance->connectListHead)
        {
            pstConnect = (SS_EXP_ConnectList_t *)malloc(sizeof(SS_EXP_ConnectList_t));
            ASSERT(pstConnect);
            memset(pstConnect, 0, sizeof(SS_EXP_ConnectList_t));
            memcpy(pstConnect->pSyncString, pSyncString, MAX_SINK_SYNC_INFO_STRING_LENGTH);
            list_add_tail(&pstConnect->stList, &pInstance->connectListHead);
            INIT_LIST_HEAD(&pstConnect->stDescListW);
            INIT_LIST_HEAD(&pstConnect->stDescListR);
        }
        else
        {
            DBG("WORK CONNECT AGAIN!\n");
            pstConnect = list_entry(pstSrcConnectList, SS_EXP_ConnectList_t, stList);
        }
        pstConnectDesc = list_entry(&pstConnect->stDescListW, SS_EXP_ConnectDesc_t, stDescList);
        list_for_each_entry(posWorkInfoList, pstSourceWorkInfoListHead, stWorkInfoList)
        {
            /* It need loop all read/write work for sink. work_count=desc_count for each connection. */
            pstConnectDesc = list_first_entry(&pstConnectDesc->stDescList, SS_EXP_ConnectDesc_t, stDescList);
            if (&pstConnectDesc->stDescList == &pstConnect->stDescListW)
            {
                pstConnectDesc = list_first_entry(&pstConnect->stDescListR, SS_EXP_ConnectDesc_t, stDescList);
            }
            if (&pstConnectDesc->stDescList == &pstConnect->stDescListR)
            {
                break;
            }
        }
        if (&posWorkInfoList->stWorkInfoList == pstSourceWorkInfoListHead)
        {
            SS_EXP_WorkInfo_t stEmptyWorkInfo;

            /* Send empty work to sink to end the loop of client fd */
            memset(&stEmptyWorkInfo, 0, sizeof(SS_EXP_WorkInfo_t));
            _SS_EXP_TcpTransfer(clientFd, (char *)&stEmptyWorkInfo, sizeof(SS_EXP_WorkInfo_t));
            _SS_EXP_TcpDeinit(clientFd);
            DBG("SOURCE WORK SET!\n");
            return 0;
        }
        enSrcMode   = posWorkInfoList->stWorkInfo.enWorkMode;
        privateData = posWorkInfoList->privateData;
        _SS_EXP_TcpTransfer(clientFd, (char *)&posWorkInfoList->stWorkInfo, sizeof(SS_EXP_WorkInfo_t));
    }
    pstConnectDesc = (SS_EXP_ConnectDesc_t *)malloc(sizeof(SS_EXP_ConnectDesc_t));
    ASSERT(pstConnectDesc);
    memset(pstConnectDesc, 0, sizeof(SS_EXP_ConnectDesc_t));
    pstConnectDesc->intClientFd = clientFd;
    pstConnectDesc->stTransCb = pInstance->stTransferAct;
    pstConnectDesc->cmdThreadHandle = pInstance->cmdThreadHandle;
    memset(&attr, 0, sizeof(struct ss_thread_attr));
    switch (enSrcMode)
    {
    case E_SS_EXP_SRC_READ_ONLY:
        {
            attr.do_monitor = _SS_EXP_MainRead;
            bRead = 1;
        }
        break;
    case E_SS_EXP_SRC_WRITE_ONLY:
        {
            pstConnectDesc->enTransState = pInstance->bWithNoSync ? E_SS_EXP_TRANS_NO_HEAD : E_SS_EXP_TRANS_BY_HEAD;
            attr.do_monitor = _SS_EXP_MainWrite;
            bRead = 0;
        }
        break;
    default:
        ASSERT(0);
    }
    if (pstConnectDesc->stTransCb.fpSsExpOpen)
    {
        SS_EXP_WorkCfg_t workCfg;
        memset(&workCfg, 0, sizeof(SS_EXP_WorkCfg_t));
        ret = pstConnectDesc->stTransCb.fpSsExpOpen(pUrl, privateData, bRead, &workCfg);
        if (ret == -1)
        {
            ERR("Open user callback error.\n");
            free(pstConnectDesc);
            _SS_EXP_TcpDeinit(clientFd);
            return -1;
        }
        pstConnectDesc->pUsrData = workCfg.pUserData;
        attr.monitor_cycle_sec   = workCfg.cycleSec;
        attr.monitor_cycle_nsec  = workCfg.cycleNSec;
    }
    attr.in_buf.buf = (void *)pstConnectDesc;
    list_add_tail(&pstConnectDesc->stDescList, bRead ? &pstConnect->stDescListR : &pstConnect->stDescListW);
    pstConnectDesc->dataThreadHandle = ss_thread_open(&attr);
    ASSERT(pstConnectDesc->dataThreadHandle);
    ss_thread_start_monitor(pstConnectDesc->dataThreadHandle);
    DBG("Connection create successfully!\n");
    return 0;
}
static void *_SS_EXP_MonitorSource(struct ss_thread_buffer *pstBuf)
{
    SS_EXP_SourceInstance_t *pInstance = NULL;
    int clientFd = 0;

    pInstance = (SS_EXP_SourceInstance_t *)pstBuf->buf;
    clientFd = _SS_EXP_TcpSourceAccept(pInstance->socketFd, &pInstance->stSockaddrIn);
    if (clientFd != -1)
    {
        /* Tcp client connect.*/
        _SS_EXP_SourceReaction(pInstance, clientFd);
    }
    //DBG("Server is waiting for client connect...\n");
    return NULL;
}


static void _SS_EXP_DestructDesc(SS_EXP_ConnectDesc_t *desc)
{
    DBG("Destroy %p\n", desc);
    ss_thread_close(desc->dataThreadHandle);
    list_del(&desc->stDescList);
    if (desc->recvInfo.recvData && desc->stTransCb.fpSsExpRemoteReadFree)
    {
        desc->stTransCb.fpSsExpRemoteReadFree(desc->pUsrData, desc->recvInfo.recvData);
    }
    if (desc->stTransCb.fpSsExpClose)
    {
        desc->stTransCb.fpSsExpClose(desc->pUsrData);
    }
    _SS_EXP_TcpDeinit(desc->intClientFd);
    free(desc);
}

static unsigned char _SS_EXP_CheckAndDestructDesc(struct list_head *head, SS_EXP_ConnectDesc_t *desc)
{
    SS_EXP_ConnectDesc_t *posDesc = NULL, *posDescN = NULL;
    list_for_each_entry_safe(posDesc, posDescN, head, stDescList)
    {
        if (posDesc == desc)
        {
            _SS_EXP_DestructDesc(desc);
            return 1;
        }
    }
    return 0;
}

static void _SS_EXP_ClearConnectDesc(struct list_head *head)
{
    SS_EXP_ConnectDesc_t *posDesc = NULL, *posDescN = NULL;
    list_for_each_entry_safe(posDesc, posDescN, head, stDescList)
    {
        _SS_EXP_DestructDesc(posDesc);
    }
}

static void *_SS_EXP_CmdSource(struct ss_thread_buffer *pstBuf, struct ss_thread_user_data *pstData)
{
    SS_EXP_ConnectCmd_t *pstCmd;
    SS_EXP_ConnectList_t *pos = NULL, *posN = NULL;
    SS_EXP_SourceInstance_t *pInstance = NULL;

    pInstance = (SS_EXP_SourceInstance_t *)pstBuf->buf;
    pstCmd = (SS_EXP_ConnectCmd_t *)pstData->data;
    switch (pstCmd->enState)
    {
    case E_SS_EXP_CONNECT_OPEN:
    {
        INIT_LIST_HEAD(&pInstance->connectListHead);
        DBG("ss_exp_source_main_%x OPEN!\n", pInstance->socketFd);
    }
    break;
    case E_SS_EXP_CONNECT_BREAK:
    {
        SS_EXP_ConnectDesc_t *pstConnectDesc = NULL;

        if (list_empty(&pInstance->connectListHead))
        {
            /* Connection had beed closed before. */
            return NULL;
        }
        pstConnectDesc = (SS_EXP_ConnectDesc_t *)pstCmd->pCmdData;
        if (!pstConnectDesc)
        {
            ERR("connection desc null!\n");
            return NULL;
        }
        list_for_each_entry_safe(pos, posN, &pInstance->connectListHead, stList)
        {
            /*If read thread and write thread are broken at the same time, it need check and do destruction.*/
            if (!_SS_EXP_CheckAndDestructDesc(&pos->stDescListW, pstConnectDesc))
            {
                _SS_EXP_CheckAndDestructDesc(&pos->stDescListR, pstConnectDesc);
            }
            if (list_empty(&pos->stDescListW))
            {
                _SS_EXP_ClearConnectDesc(&pos->stDescListR);
            }
        }
        DBG("ss_exp_source_main_%x BROKE!\n", pInstance->socketFd);
    }
    break;
    case E_SS_EXP_CONNECT_CLOSE:
    {
        list_for_each_entry_safe(pos, posN, &pInstance->connectListHead, stList)
        {
            _SS_EXP_ClearConnectDesc(&pos->stDescListR);
            _SS_EXP_ClearConnectDesc(&pos->stDescListW);
            list_del(&pos->stList);
            free(pos);
        }
        DBG("ss_exp_source_main_%x BYE BYE!\n", pInstance->socketFd);
    }
    break;
    default:
        ERR("CMD %d\n", pstCmd->enState);
        break;
    }
    return NULL;
}

static int _SS_EXP_SinkReaction(SS_EXP_SinkInstance_t *pInstance)
{
    struct ss_thread_attr attr;
    SS_EXP_ConnectDesc_t *pstConnectDesc = NULL;
    SS_EXP_WorkInfo_t stWorkInfo;
    unsigned char bRead = 0;
    int ret = 0;

    if (!pInstance)
    {
        ERR("%s: null pointer!\n", __FUNCTION__);
        return -1;
    }
    ret = _SS_EXP_TcpWaitData(pInstance->clientFd, (char *)&stWorkInfo, sizeof(SS_EXP_WorkInfo_t));
    if (ret < 0)
    {
        ERR("Connect broke.\n");
        _SS_EXP_TcpDeinit(pInstance->clientFd);
        pInstance->clientFd = _SS_EXP_TcpSinkInit();
        return -1;
    }
    if (stWorkInfo.enWorkMode == E_SS_EXP_SRC_WORK_NONE)
    {
        DBG("All works mode is set!\n");
        _SS_EXP_TcpDeinit(pInstance->clientFd);
        pInstance->clientFd = -1;
        pInstance->bIdle = 1;
        return 0;
    }
    DBG("SINK CREATE WORK: mode %d\n", stWorkInfo.enWorkMode);
    pstConnectDesc = (SS_EXP_ConnectDesc_t *)malloc(sizeof(SS_EXP_ConnectDesc_t));
    ASSERT(pstConnectDesc);
    memset(pstConnectDesc, 0, sizeof(SS_EXP_ConnectDesc_t));
    pstConnectDesc->intClientFd = pInstance->clientFd;
    pstConnectDesc->stTransCb = pInstance->stTransferAct;
    pstConnectDesc->cmdThreadHandle = pInstance->cmdThreadHandle;
    memset(&attr, 0, sizeof(struct ss_thread_attr));
    switch (stWorkInfo.enWorkMode)
    {
    case E_SS_EXP_SRC_READ_ONLY:
        {
            attr.do_monitor = _SS_EXP_MainWrite;
            bRead = 0;
        }
        break;
    case E_SS_EXP_SRC_WRITE_ONLY:
        {
            pstConnectDesc->enTransState = E_SS_EXP_TRANS_HEAD_DATA;
            attr.do_monitor = _SS_EXP_MainRead;
            bRead = 1;
        }
        break;
    default:
        ASSERT(0);
    }
    if (pstConnectDesc->stTransCb.fpSsExpOpen)
    {
        SS_EXP_WorkCfg_t workCfg;
        memset(&workCfg, 0, sizeof(SS_EXP_WorkCfg_t));
        ret = pstConnectDesc->stTransCb.fpSsExpOpen(pInstance->addr, pInstance->privateData, bRead, &workCfg);
        if (ret == -1)
        {
            ERR("Open user callback error.\n");
            free(pstConnectDesc);
            return -1;
        }
        pstConnectDesc->pUsrData = workCfg.pUserData;
        attr.monitor_cycle_sec   = workCfg.cycleSec;
        attr.monitor_cycle_nsec  = workCfg.cycleNSec;
        if (pstConnectDesc->stTransCb.fpSsExpWorkMsg)
        {
            ret = pstConnectDesc->stTransCb.fpSsExpWorkMsg(pstConnectDesc->pUsrData, stWorkInfo.workMsg);
            if (ret == -1)
            {
                if (pstConnectDesc->stTransCb.fpSsExpClose)
                {
                    pstConnectDesc->stTransCb.fpSsExpClose(pInstance->privateData);
                }
                ERR("WORK Msg error!\n");
                free(pstConnectDesc);
                return -1;
            }
        }
    }
    attr.in_buf.buf = (void *)pstConnectDesc;
    pstConnectDesc->dataThreadHandle = ss_thread_open(&attr);
    ASSERT(pstConnectDesc->dataThreadHandle);
    ss_thread_start_monitor(pstConnectDesc->dataThreadHandle);
    list_add_tail(&pstConnectDesc->stDescList, bRead ? &pInstance->stDescListR: &pInstance->stDescListW);

    /* Go on getting work connection. */
    _SS_EXP_CmdSend(pInstance->cmdThreadHandle, E_SS_EXP_CONNECT_OPEN, NULL);
    return 0;
}
static void *_SS_EXP_MonitorSink(struct ss_thread_buffer *pstBuf)
{
    SS_EXP_SinkInstance_t *pInstance = NULL;
    int ret = -1;
    char pSyncString[MAX_SINK_SYNC_INFO_STRING_LENGTH];
    char pTargetPid[32];

    pInstance = (SS_EXP_SinkInstance_t *)pstBuf->buf;
    if (pInstance->bIdle)
    {
        usleep(1000 * 100);
        return NULL;
    }
    if (pInstance->clientFd == -1)
    {
        ERR("ClientFd error!!\n");
        usleep(1000 * 300);
        return NULL;
    }
    ret = _SS_EXP_TcpSinkConnect(pInstance->clientFd, pInstance->ip, pInstance->uintPort);
    if (ret != 1)
    {
        _SS_EXP_TcpDeinit(pInstance->clientFd);
        pInstance->clientFd = _SS_EXP_TcpSinkInit();
        DBG("SINK WAIT CONNECTION\n");
        usleep(1000 * 300);
        return NULL;
    }
    memset(pSyncString, 0, MAX_SINK_SYNC_INFO_STRING_LENGTH);
    memset(pTargetPid, 0, 32);
    snprintf(pTargetPid, 32, "%x%lx", getpid(), pthread_self());
    DBG("SINK: MAC %s\n", pInstance->macAddr);
    DBG("SINK: PID %s\n", pTargetPid);
    DBG("SINK: URL %s\n", pInstance->url);
    snprintf(pSyncString, MAX_SINK_SYNC_INFO_STRING_LENGTH, "%s/%s/%s", pInstance->macAddr, pTargetPid, pInstance->url);
    DBG("ID_STR: %s\n", pSyncString);
    ret = _SS_EXP_TcpTransfer(pInstance->clientFd, pSyncString, MAX_SINK_SYNC_INFO_STRING_LENGTH);
    if (ret != MAX_SINK_SYNC_INFO_STRING_LENGTH)
    {
        _SS_EXP_TcpDeinit(pInstance->clientFd);
        pInstance->clientFd = _SS_EXP_TcpSinkInit();
        return NULL;
    }
    _SS_EXP_SinkReaction(pInstance);
    return NULL;
}

static void *_SS_EXP_CmdSink(struct ss_thread_buffer *pstBuf, struct ss_thread_user_data *pstData)
{
    SS_EXP_SinkInstance_t *pInstance = NULL;
    SS_EXP_ConnectCmd_t *pstCmd;

    pInstance = (SS_EXP_SinkInstance_t *)pstBuf->buf;
    pstCmd = (SS_EXP_ConnectCmd_t *)pstData->data;

    switch (pstCmd->enState)
    {
    case E_SS_EXP_CONNECT_OPEN:
    {
        pInstance->clientFd = _SS_EXP_TcpSinkInit();
        if (pInstance->clientFd == -1)
        {
            ERR("Client fd return error!\n");
            return NULL;
        }
        DBG("ss_exp_sink_main_%p OPEN!\n", pInstance);
    }
    break;
    case E_SS_EXP_CONNECT_BREAK:
    {
        SS_EXP_ConnectDesc_t *pstConnectDesc = NULL;

        if (list_empty(&pInstance->stDescListW))
        {
            /* Connection had beed closed before. */
            return NULL;
        }
        pstConnectDesc = (SS_EXP_ConnectDesc_t *)pstCmd->pCmdData;
        if (!pstConnectDesc)
        {
            DBG("connection desc null!\n");
            return NULL;
        }
        _SS_EXP_DestructDesc(pstConnectDesc);
        if (list_empty(&pInstance->stDescListW))
        {
            _SS_EXP_ClearConnectDesc(&pInstance->stDescListR);
            pInstance->bIdle = 0;
            _SS_EXP_CmdSend(pInstance->cmdThreadHandle, E_SS_EXP_CONNECT_OPEN, NULL);
        }
        DBG("ss_exp_sink_main_%p BROKE!\n", pInstance);
    }
    break;
    case E_SS_EXP_CONNECT_CLOSE:
    {
        _SS_EXP_ClearConnectDesc(&pInstance->stDescListW);
        _SS_EXP_ClearConnectDesc(&pInstance->stDescListR);
        if (pInstance->clientFd != -1)
        {
            _SS_EXP_TcpDeinit(pInstance->clientFd);
            pInstance->clientFd = -1;
        }
        DBG("ss_exp_sink_main_%p BYE BYE!\n", pInstance);
    }
    break;
    default:
        DBG("ERROR CMD %d\n", pstCmd->enState);
        break;
    }
    return NULL;
}

static int _SS_EXP_ClearWorkInfo(SS_EXP_SourceInstance_t *pInstance)
{
    SS_EXP_SourceWorkInfo_t *pos  = NULL;
    SS_EXP_SourceWorkInfo_t *posN = NULL;
    int                         i = 0;

    for (i = 0; i < MAX_CONNECTIONS_KEY; i++)
    {
        list_for_each_entry_safe(pos, posN, &pInstance->sourceWorkHash[i], stWorkHashList)
        {
            if (!list_empty(&pos->stWorkInfoList))
            {
                ERR("ERROR: URL %s, Work resource did not clear", pos->pUrl);
                return -1;
            }
            list_del(&pos->stWorkHashList);
            free(pos);
        }
    }
    return 0;
}

WORK_HANDLE SS_EXP_SourceCreateWork(void *pIns, const char *pUrl, void *privateData, const SS_EXP_WorkInfo_t *pstWorkInfo)
{
    unsigned int key                             = 0;
    SS_EXP_SourceInstance_t *pInstance           = NULL;
    SS_EXP_WorkInfoListData_t *pWorkInfoListData = NULL;
    SS_EXP_SourceWorkInfo_t *pos                 = NULL;

    if (!pIns || !pUrl || !pstWorkInfo)
    {
        return NULL;
    }
    pInstance = (SS_EXP_SourceInstance_t *)pIns;
    if (pInstance->bWithNoSync)
    {
        ERR("ERROR: Not support work when no sync flag is 1.\n");
        return NULL;
    }
    if (strlen(pUrl) >= MAX_URL_LENGTH)
    {
        ERR("ERROR: String url [%s] is too long, pls make it be less than %d byte.\n", pUrl, MAX_URL_LENGTH);
        return NULL;
    }
    key = _SS_EXP_WorkInfoHash(pUrl);
    list_for_each_entry(pos, &pInstance->sourceWorkHash[key], stWorkHashList)
    {
        if (!strcmp(pUrl, pos->pUrl))
        {
            break;
        }
    }
    if (&pos->stWorkHashList == &pInstance->sourceWorkHash[key])
    {
        pos = (SS_EXP_SourceWorkInfo_t *)malloc(sizeof(SS_EXP_SourceWorkInfo_t));
        ASSERT(pos);
        INIT_LIST_HEAD(&pos->stWorkInfoList);
        snprintf(pos->pUrl, MAX_URL_LENGTH, "%s", pUrl);
        list_add_tail(&pos->stWorkHashList, &pInstance->sourceWorkHash[key]);
    }
    pWorkInfoListData = (SS_EXP_WorkInfoListData_t *)malloc(sizeof(SS_EXP_WorkInfoListData_t));
    ASSERT(pWorkInfoListData);
    pWorkInfoListData->privateData = privateData;
    memcpy(&pWorkInfoListData->stWorkInfo, pstWorkInfo, sizeof(SS_EXP_WorkInfo_t));
    list_add_tail(&pWorkInfoListData->stWorkInfoList, &pos->stWorkInfoList);
    DBG("%d: URL[%s] mode %d\n", key, pos->pUrl, pWorkInfoListData->stWorkInfo.enWorkMode);
    return (WORK_HANDLE)pWorkInfoListData;
}
int SS_EXP_SourceDestroyWork(WORK_HANDLE hWorkHandle)
{
    SS_EXP_WorkInfoListData_t *pWorkInfoListData = (SS_EXP_WorkInfoListData_t *)hWorkHandle;

    ASSERT(pWorkInfoListData);
    list_del(&pWorkInfoListData->stWorkInfoList);
    free(pWorkInfoListData);
    return 0;
}
void *SS_EXP_SourceInit(const SS_EXP_SourceAttr_t *sourceAttr, const SS_EXP_TransferActCb_t *pstActCb)
{
    SS_EXP_SourceInstance_t *pInstance = NULL;
    int                              i = 0;

    if (!pstActCb)
    {
        return NULL;
    }
    pInstance = (SS_EXP_SourceInstance_t *)malloc(sizeof(SS_EXP_SourceInstance_t));
    if (!pInstance)
    {
        return NULL;
    }
    memset(pInstance, 0, sizeof(SS_EXP_SourceInstance_t));
    memcpy(&pInstance->stTransferAct, pstActCb, sizeof(SS_EXP_TransferActCb_t));
    pInstance->uintPort = sourceAttr->uintPort;
    pInstance->bWithNoSync = sourceAttr->bWithNoSync;
    pInstance->privateData = sourceAttr->privateData;
    pInstance->socketFd = _SS_EXP_TcpSourceInit(pInstance->uintPort, &pInstance->stSockaddrIn);
    if (pInstance->socketFd == -1)
    {
        free(pInstance);
        return NULL;
    }
    for (i = 0; i < MAX_CONNECTIONS_KEY; i++)
    {
        INIT_LIST_HEAD(&pInstance->sourceWorkHash[i]);
    }
    return pInstance;
}
int SS_EXP_SourceStart(void *pIns)
{
    struct ss_thread_attr attr;
    SS_EXP_SourceInstance_t *pInstance = NULL;

    pInstance = (SS_EXP_SourceInstance_t *)pIns;
    if (!pInstance)
    {
        return -1;
    }
    memset(&attr, 0, sizeof(struct ss_thread_attr));
    attr.do_signal  = _SS_EXP_CmdSource;
    attr.do_monitor = _SS_EXP_MonitorSource;
    attr.in_buf.buf = pInstance;
    pInstance->cmdThreadHandle = ss_thread_open(&attr);
    if (!pInstance->cmdThreadHandle)
    {
        free(pInstance);
        return -1;
    }
    _SS_EXP_CmdSend(pInstance->cmdThreadHandle, E_SS_EXP_CONNECT_OPEN, NULL);
    return ss_thread_start_monitor(pInstance->cmdThreadHandle);
}
int SS_EXP_SourceStop(void *pIns)
{
    SS_EXP_SourceInstance_t *pInstance = NULL;

    pInstance = (SS_EXP_SourceInstance_t *)pIns;
    if (!pInstance)
    {
        return -1;
    }
    ss_thread_stop(pInstance->cmdThreadHandle);
    _SS_EXP_CmdSend(pInstance->cmdThreadHandle, E_SS_EXP_CONNECT_CLOSE, NULL);
    return ss_thread_close(pInstance->cmdThreadHandle);
}
int SS_EXP_SourceDeinit(void *pIns)
{
    SS_EXP_SourceInstance_t *pInstance = NULL;
    int ret                            = 0;

    pInstance = (SS_EXP_SourceInstance_t *)pIns;
    if (!pInstance)
    {
        return -1;
    }
    _SS_EXP_TcpSourceDeinit(pInstance->socketFd);
    ret = _SS_EXP_ClearWorkInfo(pInstance);
    if (ret != 0)
    {
        return ret;
    }
    free(pInstance);
    return 0;
}
void *SS_EXP_SinkInit(const SS_EXP_SinkAttr_t *sinkAttr, const SS_EXP_TransferActCb_t *pstActCb)
{
    struct ss_thread_attr attr;
    SS_EXP_SinkInstance_t *pInstance = NULL;
    char strPort[16];


    if (!pstActCb || !sinkAttr || !sinkAttr->pAddress)
    {
        return NULL;
    }
    pInstance = (SS_EXP_SinkInstance_t *)malloc(sizeof(SS_EXP_SinkInstance_t));
    ASSERT(pInstance);
    memset(pInstance, 0, sizeof(SS_EXP_SinkInstance_t));
    memset(strPort, 0, 16);
    if (3 != sscanf(sinkAttr->pAddress, "exp://%[^:]:%[^/]/%s", pInstance->ip, strPort, pInstance->url))
    {
        free(pInstance);
        DBG("addr error: exp://xxx.xxx.xxx.xxx:port/url");
        return NULL;
    }
    strncpy(pInstance->addr, sinkAttr->pAddress, 64);
    pInstance->uintPort = atoi(strPort);
    _SS_EXP_GetMyMacAddr(pInstance->macAddr, 32);
    DBG("YOUR CONNECT IP IS   : %s\n", pInstance->ip);
    DBG("YOUR CONNECT PORT IS : %d\n", pInstance->uintPort);
    DBG("YOUR CONNECT URL IS  : %s\n", pInstance->url);
    DBG("YOUR CONNECT MAC IS  : %s\n", pInstance->macAddr);
    INIT_LIST_HEAD(&pInstance->stDescListW);
    INIT_LIST_HEAD(&pInstance->stDescListR);
    memcpy(&pInstance->stTransferAct, pstActCb, sizeof(SS_EXP_TransferActCb_t));
    memset(&attr, 0, sizeof(struct ss_thread_attr));
    attr.do_signal  = _SS_EXP_CmdSink;
    attr.do_monitor = _SS_EXP_MonitorSink;
    attr.in_buf.buf = pInstance;
    pInstance->privateData = sinkAttr->privateData;
    pInstance->cmdThreadHandle = ss_thread_open(&attr);
    if (!pInstance->cmdThreadHandle)
    {
        free(pInstance);
        return NULL;
    }
    _SS_EXP_CmdSend(pInstance->cmdThreadHandle, E_SS_EXP_CONNECT_OPEN, NULL);
    ss_thread_start_monitor(pInstance->cmdThreadHandle);
    return pInstance;
}
int SS_EXP_SinkDeinit(void *pIns)
{
    SS_EXP_SinkInstance_t *pInstance = NULL;

    pInstance = (SS_EXP_SinkInstance_t *)pIns;
    if (!pInstance)
    {
        return -1;
    }
    ss_thread_stop(pInstance->cmdThreadHandle);
    _SS_EXP_CmdSend(pInstance->cmdThreadHandle, E_SS_EXP_CONNECT_CLOSE, NULL);
    ss_thread_close(pInstance->cmdThreadHandle);
    free(pInstance);
    return 0;
}
int SS_EXP_GetIpAddr(const char *interface, char *ipAddress, unsigned int addrLen)
{
    struct ifaddrs *addrs = NULL, *tmp = NULL;

    if (getifaddrs(&addrs) == -1)
    {
        perror("getifaddrs");
        return -1;
    }

    tmp = addrs;
    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            if (strcmp(tmp->ifa_name, interface) == 0)
            {
                snprintf(ipAddress, addrLen, "%s", inet_ntoa(pAddr->sin_addr));
                freeifaddrs(addrs);
                return 0;
            }
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(addrs);
    return -1;
}
