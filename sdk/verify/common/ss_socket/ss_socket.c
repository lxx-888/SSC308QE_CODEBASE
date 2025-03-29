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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "ss_socket.h"

#define LISTENQN       1
static MI_U32 SleepTime = 1000;

MI_S32 ss_socket_set_nodelay(MI_S32 Sock)
{
    MI_U32 opt = 1;

    if (Sock < 0)
    {
        return MST_FAIL;
    }

    return setsockopt(Sock,IPPROTO_TCP,TCP_NODELAY,(char *)&opt,sizeof(opt));
}

MI_S32 ss_socket_set_keepalive(MI_S32 Sock)
{
    MI_U32 opt = 1;

    if (Sock < 0)
    {
        return MST_FAIL;
    }

    return setsockopt(Sock, SOL_SOCKET, SO_KEEPALIVE,(char *)&opt,sizeof(opt));
}

MI_S32 ss_socket_set_reuseaddr(MI_S32 Sock, MI_U32 ReuseAddr)
{
    if (Sock < 0)
    {
        return MST_FAIL;
    }

    return setsockopt(Sock,SOL_SOCKET,SO_REUSEADDR,(void *)&ReuseAddr, sizeof(MI_U32));
}

MI_S32 ss_socket_set_sendtimeout(MI_S32 Sock, MI_U32 SendTimeO)
{
    struct timeval sendTo;

    if (Sock < 0)
    {
        return MST_FAIL;
    }

    sendTo.tv_sec  = SendTimeO / 1000;
    sendTo.tv_usec = (SendTimeO % 1000) * 1000;

    return setsockopt(Sock,SOL_SOCKET,SO_SNDTIMEO,(void  *)&sendTo,sizeof(sendTo));
}

MI_S32 ss_socket_set_recvtimeout(MI_S32 Sock, MI_U32 RecvTimeO)
{
    struct timeval rcvTo;
    if (Sock < 0)
    {
        return MST_FAIL;
    }

    rcvTo.tv_sec  = RecvTimeO / 1000;
    rcvTo.tv_usec = (RecvTimeO % 1000) * 1000;

    return setsockopt(Sock,SOL_SOCKET,SO_RCVTIMEO,(void  *)&rcvTo,sizeof(rcvTo));
}

MI_S32 ss_socket_set_sendbuf(MI_S32 Sock, MI_U32 SendBuf)
{
    if (Sock < 0)
    {
        return MST_FAIL;
    }

    return setsockopt(Sock,SOL_SOCKET,SO_SNDBUF,(void  *)&SendBuf,sizeof(SendBuf));
}

MI_S32 ss_socket_set_recvbuf(MI_S32 Sock, MI_U32 RcvBuf)
{
    if (Sock < 0)
    {
        return MST_FAIL;
    }

    return setsockopt(Sock,SOL_SOCKET,SO_RCVBUF,(void  *)&RcvBuf,sizeof(RcvBuf));
}

MI_S32 ss_socket_set_noblock(MI_S32 Sock)
{
    int bNoBlock = 1;

    if (Sock < 0)
    {
        return MST_FAIL;
    }

    return ioctl(Sock, FIONBIO, &bNoBlock);
}

MI_S32 ss_socket_set_attr(MI_S32 Sock, MI_U32 SndTimeO, MI_U32 RcvTimeO, MI_U32 SndBuf, MI_U32 RcvBuf)
{
    MI_S32 ret = MST_OK;
    if (Sock < 0)
    {
        return MST_FAIL;
    }

    if (ss_socket_set_keepalive(Sock) != 0)
    {
        return MST_FAIL;
    }

    if (SndTimeO != 0 && ss_socket_set_sendtimeout(Sock,SndTimeO) < 0)
    {
        ret = MST_FAIL;
    }

    if (RcvTimeO != 0 && ss_socket_set_recvtimeout(Sock,RcvTimeO) < 0)
    {
        ret = MST_FAIL;
    }

    if (SndBuf != 0 && ss_socket_set_sendbuf(Sock,SndBuf) < 0)
    {
        ret = MST_FAIL;
    }

    if (RcvBuf != 0 && ss_socket_set_recvbuf(Sock,RcvBuf) < 0)
    {
        ret = MST_FAIL;
    }

    return ret;
}

MI_S32 ss_socket_listern(MI_U32 Ip, MI_U32 Port)
{
    MI_S32    ListenSock = -1;
    struct sockaddr_in SockAddr;

    ListenSock = socket(AF_INET,SOCK_STREAM, 0);
    if(ListenSock < 0)
    {
        ss_socket_close(ListenSock);
        printf("Create socket failed. errno = %d\n", errno);
        return MST_FAIL;
    }

    memset(&SockAddr, 0, sizeof(SockAddr));
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(Port);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ss_socket_set_reuseaddr(ListenSock, 1);

    if(bind(ListenSock,(struct sockaddr *)&SockAddr,sizeof(SockAddr)) < 0)
    {
        ss_socket_close(ListenSock);
        printf("Bind failed. errno = %d\n", errno);
        return MST_FAIL;
    }

    if(listen (ListenSock, LISTENQN) < 0)
    {
        ss_socket_close(ListenSock);
        printf("Listen failed. errno = %d\n", errno);
        return MST_FAIL;
    }

    return ListenSock;

}

MI_S32 ss_socket_connect(const char *hostname, const char *serv)
{
    MI_S32 sockfd, n;

    struct addrinfo hints, *res, *ressave;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(hostname, serv, &hints, &res)) != 0)
    {
        printf("Tcp connect failed for %s:%s, %s\n", hostname, serv, gai_strerror(n));
        return MST_FAIL;
    }

    ressave = res;

    do{
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd < 0)
        {
            close(sockfd);
            continue;
        }

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
        {
            break;
        }
        close(sockfd);
    }while((res = res->ai_next) != NULL);

    if(res == NULL)
    {
        printf("Tcp connect failed for %s:%s\n", hostname, serv);
        freeaddrinfo(ressave);
        return MST_FAIL;
    }

    freeaddrinfo(ressave);
    return sockfd;
}

MI_S32 ss_socket_create(MI_U32 Ip, MI_U32 Port)
{
    MI_S32 Sock = -1;

    if ((Sock = ss_socket_listern(Ip, Port)) >= 0)
    {
        ss_socket_set_nodelay(Sock);
        ss_socket_set_attr(Sock, 0, 0, 4096, 4096);
        SleepTime = 1000;
    }

    return Sock;
}

MI_S32 ss_socket_close(MI_S32 sock)
{
    if (sock >= 0)
    {
        close(sock);
        /* sock = -1; */
        return MST_OK;
    }

    return MST_FAIL;
}

MI_S32 ss_socket_select(MI_S32 *sockfdArray, MI_S32 sockfdNum, MI_U32 sockfdType, MI_U32 timeout)
{
    MI_S32 maxfd = 0;
    MI_S32 index = 0;
    MI_S32 ret = 0;

    fd_set read_fd,   *p_read_fd  = NULL;
    fd_set write_fd,  *p_write_fd = NULL;
    fd_set except_fd, *p_except_fd = NULL;

    struct timeval timeO, *p_time_out = NULL;

    if (sockfdArray == NULL || sockfdNum <= 0)
    {
        return MST_FAIL;
    }

    if (timeout > 0)
    {
        timeO.tv_sec = timeout / 1000;
        timeO.tv_usec= (timeout % 1000)*1000;
        p_time_out = &timeO;
    }

    if (sockfdType & FD_SOCK_READ)
    {
        p_read_fd = &read_fd;
        FD_ZERO(p_read_fd);
    }

    if (sockfdType & FD_SOCK_WRITE)
    {
        p_write_fd = &write_fd;
        FD_ZERO(p_write_fd);
    }

    if (sockfdType & FD_SOCK_EXCEPT)
    {
        p_except_fd = &except_fd;
        FD_ZERO(p_except_fd);
    }

    for (index = 0; index < sockfdNum; ++index)
    {
        if(sockfdArray[index] <= 0)
            continue;

        maxfd = maxfd > sockfdArray[index] ? maxfd : sockfdArray[index];

        if (p_read_fd)
            FD_SET(sockfdArray[index], p_read_fd);
        if (p_write_fd)
            FD_SET(sockfdArray[index], p_write_fd);
        if (p_except_fd)
            FD_SET(sockfdArray[index], p_except_fd);
    }

    if (maxfd <= 0)
    {
        printf("[%s][%s] maxfd=%d \n", __FILE__,__func__,maxfd);
        return MST_FAIL;
    }

    maxfd += 1;

    while (1)
    {
        ret = select(maxfd, p_read_fd, p_write_fd, p_except_fd, p_time_out);

        if (ret < 0 && errno == EINTR)
            continue;
        else if (ret < 0)
            return MST_FAIL;
        else if (ret == 0)
            return ret;

        for (index = 0; index < sockfdNum; ++index)
        {
            if (sockfdArray[index] < 0)  //socket error
                continue;

            if (p_read_fd)
            {
                if (FD_ISSET(sockfdArray[index], p_read_fd))
                    return sockfdArray[index] | FD_SOCK_READ_SET;
            }
            else if (p_write_fd)
            {
                if (FD_ISSET(sockfdArray[index], p_write_fd))
                    return sockfdArray[index] | FD_SOCK_WRITE_SET;
            }
            else if (p_except_fd)
            {
                if (FD_ISSET(sockfdArray[index], p_except_fd))
                    return sockfdArray[index] | FD_SOCK_EXCEPT_SET;
            }
        }

        return MST_OK;
    }

    return MST_FAIL;
}

MI_S32 ss_socket_accept(MI_S32 Sock, struct sockaddr *pSockAddr, MI_S32 *pSockAddrLen)
{
    MI_S32 ret = MST_FAIL;

again:
    ret = accept(Sock, pSockAddr, (socklen_t*)pSockAddrLen);
    if ( ret < 0 && errno == ECONNABORTED)
    {
        goto again;
    }

    return ret;
}

MI_S32 ss_socket_receive(MI_S32 Sock, char *pRcvBuf, MI_S32 BufSize, MI_S32 RcvSize)
{
    MI_S32    ret = MST_FAIL;
    MI_U32 TryTime  = 0;
    MI_U32 TotalSize = 0;
    MI_U32 Received = 0;

    if (Sock <= 0 || pRcvBuf == NULL || BufSize <= 0)
    {
        return ret;
    }

    /* TotalSize = (RcvSize < BufSize)?RcvSize:BufSize; */
    TotalSize = (RcvSize <= 0)?BufSize:RcvSize;

    while(Received < TotalSize)
    {
        ret = recv(Sock, pRcvBuf + Received, TotalSize - Received, 0);
        if(ret == 0)
        {
            return MST_FAIL;
        }
        else if(ret < 0)
        {
            if(ECONNRESET == errno)
            {
                return MST_FAIL;
            }
            else if(  errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
            {
                if(TryTime++ < MAX_NOBLOCK_RECV_TIME)
                {
                    usleep(10000);
                    continue;
                }
                break;
            }
            return MST_FAIL;
        }

        TryTime = 0;
        Received += ret;

        if (RcvSize <= 0)
        {
            break;
        }
    }

    return Received;
}

MI_S32 ss_socket_send(MI_S32 Sock, char *SndBuf, MI_S32 Size)
{
    MI_S32 Sended = 0;
    MI_U32 TryTime = 0;
    MI_U32 TotalSize = Size;

    if (Sock < 0 || SndBuf == NULL || Size <= 0)
    {
        return 0;
    }

    while(TotalSize > 0)
    {
        Sended = send(Sock,SndBuf,TotalSize,0);
        if(Sended == 0)
        {
            return MST_FAIL;
        }
        else if(Sended < 0)
        {
            if(errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
            {
                if(TryTime++ < MAX_NOBLOCK_SEND_TIME)
                {
                    usleep(SleepTime);
                    continue;
                }
                break;
            }
            printf("send data error[%d]:%s\n", errno, strerror(errno));
            return MST_FAIL;
        }

        TryTime = 0;
        SndBuf += Sended;
        TotalSize -= Sended;
    }

    return Size - TotalSize;
}

