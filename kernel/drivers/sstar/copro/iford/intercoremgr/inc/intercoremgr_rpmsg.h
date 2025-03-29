/*
 * intercoremgr_rpmsg.h - Sigmastar
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

#ifndef __INTERCOREMGR_RPMSG_H__
#define __INTERCOREMGR_RPMSG_H__

s32 InterCoreMgr_Rpmsg_Init(ReceiverCallback pRecvCb);
s32 InterCoreMgr_Rpmsg_Deinit(void);
s32 InterCoreMgr_Rpmsg_Scan(SS_InterCoreMgr_CPU_e peerCpu, u32 scanTimeoutInMs, SS_InterCoreMgr_ScanInfo_t *pScanInfo);
s32 InterCoreMgr_Rpmsg_ScanAck(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_ScanAckInfo_t *pScanAckInfo);
s32 InterCoreMgr_Rpmsg_HbTick(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo);
s32 InterCoreMgr_Rpmsg_HbStop(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo);
s32 InterCoreMgr_Rpmsg_NotifyState(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_stateInfo_t *pStateInfo);
s32 InterCoreMgr_Rpmsg_QueryState(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_queryStateInfo_t *pQueryStateInfo);

#endif
