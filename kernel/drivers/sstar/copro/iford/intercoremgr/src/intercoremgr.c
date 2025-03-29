/*
 * intercoremgr.c - Sigmastar
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

#include "intercoremgr.h"
#ifdef COREMGR_VIA_MBX
#include "intercoremgr_mbx.h"
#else
#include "intercoremgr_rpmsg.h"
#endif
s32 SS_InterCoreMgr_Comm_Init(ReceiverCallback pRecvCb)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_Init(pRecvCb);
#else
    return InterCoreMgr_Rpmsg_Init(pRecvCb);
#endif
}

s32 SS_InterCoreMgr_Comm_DeInit(void)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_Deinit();
#else
    return InterCoreMgr_Rpmsg_Deinit();
#endif
}

s32 SS_InterCoreMgr_Scan(SS_InterCoreMgr_CPU_e peerCpu, u32 scanTimeoutInMs, SS_InterCoreMgr_ScanInfo_t *pScanInfo)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_Scan(peerCpu, scanTimeoutInMs, pScanInfo);
#else
    return InterCoreMgr_Rpmsg_Scan(peerCpu, scanTimeoutInMs, pScanInfo);
#endif
}

s32 SS_InterCoreMgr_ScanAck(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_ScanAckInfo_t *pScanAckInfo)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_ScanAck(peerCpu, pScanAckInfo);
#else
    return InterCoreMgr_Rpmsg_ScanAck(peerCpu, pScanAckInfo);
#endif
}

s32 SS_InterCoreMgr_Heartbeat_Tick(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_HbTick(peerCpu, pHbInfo);
#else
    return InterCoreMgr_Rpmsg_HbTick(peerCpu, pHbInfo);
#endif
}

s32 SS_InterCoreMgr_Heartbeat_Stop(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_HbStop(peerCpu, pHbInfo);
#else
    return InterCoreMgr_Rpmsg_HbStop(peerCpu, pHbInfo);
#endif
}

s32 SS_InterCoreMgr_Notify_State(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_stateInfo_t *pStateInfo)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_NotifyState(peerCpu, pStateInfo);
#else
    return InterCoreMgr_Rpmsg_NotifyState(peerCpu, pStateInfo);
#endif
}

s32 SS_InterCoreMgr_Query_State(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_queryStateInfo_t *pQueryStateInfo)
{
#ifdef COREMGR_VIA_MBX
    return InterCoreMgr_Mbx_QueryState(peerCpu, pQueryStateInfo);
#else
    return InterCoreMgr_Rpmsg_QueryState(peerCpu, pQueryStateInfo);
#endif
}
