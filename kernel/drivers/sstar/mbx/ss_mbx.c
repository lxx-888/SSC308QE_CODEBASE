/*
 * ss_mbx.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */

#ifdef __linux__
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "ss_mbx_ioctl.h"
#else
#include "ss_mbx_impl.h"
#endif
#include "ss_mbx.h"

#ifdef __linux__
#define SS_MBX_DEV_PATH ("/dev/" MBX_IOC_DEVICNAME)

typedef struct SS_Mbx_Res_s
{
    int fd;
} SS_Mbx_Res_t;

SS_Mbx_Res_t g_stMbxRes = {0};
#endif

int SS_Mailbox_Init(void)
{
#ifdef __linux__
    int fd = 0;

    if (g_stMbxRes.fd == 0)
    {
        fd = open(SS_MBX_DEV_PATH, O_WRONLY);
        if (fd < 0)
        {
            printf("open mbx dev %s error.\n", SS_MBX_DEV_PATH);
            return E_SS_MBX_RET_FAIL;
        }
        g_stMbxRes.fd = fd;
    }

    return E_SS_MBX_RET_OK;
#else
    return SS_Mailbox_IMPL_Init();
#endif
}

int SS_Mailbox_Deinit(void)
{
#ifdef __linux__
    int ret = E_SS_MBX_RET_OK;

    if (g_stMbxRes.fd == 0)
    {
        printf("mbx not init.\n");
        return E_SS_MBX_RET_NOT_INIT;
    }

    close(g_stMbxRes.fd);
    g_stMbxRes.fd = 0;

    return ret;
#else
    return SS_Mailbox_IMPL_Deinit();
#endif
}

int SS_Mailbox_Enable(u8 u8MsgClass)
{
#ifdef __linux__
    u8 class = u8MsgClass;

    if (g_stMbxRes.fd == 0)
    {
        printf("mbx not init.\n");
        return E_SS_MBX_RET_NOT_INIT;
    }

    return ioctl(g_stMbxRes.fd, MBX_IOC_CMD_ENABLE_CLASS, &class);
#else
    return SS_Mailbox_IMPL_Enable(u8MsgClass);
#endif
}

int SS_Mailbox_Disable(u8 u8MsgClass)
{
#ifdef __linux__
    u8 class = u8MsgClass;

    if (g_stMbxRes.fd == 0)
    {
        printf("mbx not init.\n");
        return E_SS_MBX_RET_NOT_INIT;
    }

    return ioctl(g_stMbxRes.fd, MBX_IOC_CMD_DISABLE_CLASS, &class);
#else
    return SS_Mailbox_IMPL_Disable(u8MsgClass);
#endif
}

int SS_Mailbox_SendMsg(SS_Mbx_Msg_t *pstMbxMsg)
{
#ifdef __linux__
    SS_Mbx_SendMsg_t stSendMsg = {};

    if (g_stMbxRes.fd == 0)
    {
        printf("mbx not init.\n");
        return E_SS_MBX_RET_NOT_INIT;
    }

    memcpy(&stSendMsg.stMbxMsg, pstMbxMsg, sizeof(SS_Mbx_Msg_t));
    return ioctl(g_stMbxRes.fd, MBX_IOC_CMD_SEND_MSG, &stSendMsg);
#else
    return SS_Mailbox_IMPL_SendMsg(pstMbxMsg);
#endif
}

int SS_Mailbox_RecvMsg(u8 u8MsgClass, SS_Mbx_Msg_t *pstMbxMsg, s32 s32WaitMs)
{
#ifdef __linux__
    SS_Mbx_RecvMsg_t stRecvMsg = {};
    int              ret       = 0;

    if (g_stMbxRes.fd == 0)
    {
        printf("mbx not init.\n");
        return E_SS_MBX_RET_NOT_INIT;
    }

    stRecvMsg.u8MsgClass = u8MsgClass;
    stRecvMsg.s32WaitMs  = s32WaitMs;

    ret = ioctl(g_stMbxRes.fd, MBX_IOC_CMD_RECV_MSG, &stRecvMsg);
    if (ret == 0)
    {
        memcpy(pstMbxMsg, &stRecvMsg.stMbxMsg, sizeof(SS_Mbx_Msg_t));
    }

    return ret;
#else
    return SS_Mailbox_IMPL_RecvMsg(u8MsgClass, pstMbxMsg, s32WaitMs);
#endif
}
