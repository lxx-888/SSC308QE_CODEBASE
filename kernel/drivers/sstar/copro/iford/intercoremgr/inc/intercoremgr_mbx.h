/*
 * intercoremgr_mbx.h - Sigmastar
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

#ifndef __INTERCOREMGR_MBX_H__
#define __INTERCOREMGR_MBX_H__

#define CMR_MBX_CLASS_BASE 0

typedef enum
{
    E_CMR_MBX_CLASS_HB = CMR_MBX_CLASS_BASE,
    E_CMR_MBX_CLASS_STATE,
    E_CMR_MBX_CLASS_MAX,
} InterCoreMgr_Mbx_Class_e;

typedef enum
{
    E_CMR_MBX_HB_SCAN = 0,
    E_CMR_MBX_HB_SCAN_ACK,
    E_CMR_MBX_HB_TICK,
    E_CMR_MBX_HB_STOP,
    E_CMR_MBX_HB_MAX,
} Cmr_Mbx_HB_Type_e;

typedef enum
{
    E_CMR_MBX_STATE_NOTIFY = 0,
    E_CMR_MBX_STATE_QUERY,
    E_CMR_MBX_STATE_MAX,
} Cmr_Mbx_State_Opt_e;

s32 InterCoreMgr_Mbx_Init(ReceiverCallback pRecvCb);
s32 InterCoreMgr_Mbx_Deinit(void);
s32 InterCoreMgr_Mbx_Scan(SS_InterCoreMgr_CPU_e peerCpu, u32 scanTimeoutInMs, SS_InterCoreMgr_ScanInfo_t *pScanInfo);
s32 InterCoreMgr_Mbx_ScanAck(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_ScanAckInfo_t *pScanAckInfo);
s32 InterCoreMgr_Mbx_HbTick(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo);
s32 InterCoreMgr_Mbx_HbStop(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo);
s32 InterCoreMgr_Mbx_NotifyState(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_stateInfo_t *pStateInfo);
s32 InterCoreMgr_Mbx_QueryState(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_queryStateInfo_t *pQueryStateInfo);

#endif
