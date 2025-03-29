/*
 * ss_mbx_ioctl.h- Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
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

#ifndef _SS_MBX_IOCTL_H
#define _SS_MBX_IOCTL_H

#include <linux/ioctl.h>
#include "ss_mbx.h"

#define MBX_IOC_MAGIC     'M'
#define MBX_IOC_DEVICNAME "ss_mbx"
#define MBX_IOC_DEV_COUNT 1

typedef enum
{
    E_MBX_IOC_CMD_ENABLE_CLASS = 0,
    E_MBX_IOC_CMD_DISABLE_CLASS,
    E_MBX_IOC_CMD_SEND_MSG,
    E_MBX_IOC_CMD_RECV_MSG,
    E_MBX_IOC_CMD_MAX,
} MBX_IOC_CMD_TYPE_e;

typedef struct SS_Mbx_SendMsg_s
{
    SS_Mbx_Msg_t stMbxMsg;
} SS_Mbx_SendMsg_t;

typedef struct SS_Mbx_RecvMsg_s
{
    u8           u8MsgClass;
    SS_Mbx_Msg_t stMbxMsg;
    s32          s32WaitMs;
} SS_Mbx_RecvMsg_t;

#define MBX_IOC_CMD_ENABLE_CLASS  _IOW(MBX_IOC_MAGIC, E_MBX_IOC_CMD_ENABLE_CLASS, u8)
#define MBX_IOC_CMD_DISABLE_CLASS _IOW(MBX_IOC_MAGIC, E_MBX_IOC_CMD_DISABLE_CLASS, u8)
#define MBX_IOC_CMD_SEND_MSG      _IOW(MBX_IOC_MAGIC, E_MBX_IOC_CMD_SEND_MSG, SS_Mbx_SendMsg_t)
#define MBX_IOC_CMD_RECV_MSG      _IOR(MBX_IOC_MAGIC, E_MBX_IOC_CMD_RECV_MSG, SS_Mbx_RecvMsg_t)

#endif
