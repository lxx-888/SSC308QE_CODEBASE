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

#ifndef _SS_SOCKET_H_
#define _SS_SOCKET_H_

#include "mi_common_datatype.h"

#define MST_FAIL     -1
#define MST_OK        0

#define FD_SOCK_READ            0x01
#define FD_SOCK_WRITE           0x02
#define FD_SOCK_EXCEPT          0x04

#define FD_SOCK_READ_SET        0x10000
#define FD_SOCK_WRITE_SET       0x20000
#define FD_SOCK_EXCEPT_SET      0x40000

#define SOCK_SELECT_TIMEOUT     1000
#define MAX_NOBLOCK_RECV_TIME   500
#define MAX_NOBLOCK_SEND_TIME   4000

MI_S32 ss_socket_create(MI_U32 Ip, MI_U32 Port);
MI_S32 ss_socket_close(MI_S32 Sock);
MI_S32 ss_socket_select(MI_S32 *sockfdArray, MI_S32 sockfdNum, MI_U32 sockfdType, MI_U32 timeout);
MI_S32 ss_socket_accept(MI_S32 Sock, struct sockaddr *pSockAddr, MI_S32 *pSockAddrLen);
MI_S32 ss_socket_set_noblock(MI_S32 Sock);
MI_S32 ss_socket_set_attr(MI_S32 Sock, MI_U32 SndTimeO, MI_U32 RcvTimeO, MI_U32 SndBuf, MI_U32 RcvBuf);
MI_S32 ss_socket_receive(MI_S32 Sock, char *pRcvBuf, MI_S32 BufSize, MI_S32 RcvSize);
MI_S32 ss_socket_send(MI_S32 Sock, char *SndBuf, MI_S32 Size);
#endif

