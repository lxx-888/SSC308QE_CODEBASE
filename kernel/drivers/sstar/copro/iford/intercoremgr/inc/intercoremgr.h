/*
 * intercoremgr.h - Sigmastar
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

#ifndef __INTERCOREMGR_H__
#define __INTERCOREMGR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char  u8;
typedef signed char    s8;
typedef unsigned short u16;
typedef signed short   s16;
typedef unsigned int   u32;
typedef signed int     s32;

typedef enum
{
    E_COREMGR_RET_OK            = 0,
    E_COREMGR_RET_FAIL          = 1,
    E_COREMGR_RET_INVAILD_PARAM = 2,
    E_COREMGR_RET_NOMEM         = 3,
    E_COREMGR_RET_TIMEOUT       = 4,
} SS_InterCoreMgr_Ret_e;

typedef enum
{
    E_COREMGR_CPU_ARM = 0,
    E_COREMGR_CPU_RISCV,
    E_COREMGR_CPU_CM4,
    E_COREMGR_CPU_MAX,
} SS_InterCoreMgr_CPU_e;

typedef struct SS_InterCoreMgr_ScanInfo_s
{
    SS_InterCoreMgr_CPU_e scanCPU;
    u16                   hbPeriodInMs;
    u32                   timestampInMs;
} SS_InterCoreMgr_ScanInfo_t;

typedef struct SS_InterCoreMgr_ScanAckInfo_s
{
    SS_InterCoreMgr_CPU_e ackCPU;
    u16                   hbPeriodInMs;
    u32                   timestampInMs;
} SS_InterCoreMgr_ScanAckInfo_t;

typedef enum
{
    E_COREMGR_DEFAULT = 0,
    E_COREMGR_SCAN,
    E_COREMGR_SCAN_ACK,
    E_COREMGR_HB,
    E_COREMGR_HB_STOP,
    E_COREMGR_STATE_QUERY,
    E_COREMGR_STATE_NOTIFY,
    E_COREMGR_MAX,
} SS_InterCoreMgr_Action_e;

typedef struct SS_InterCoreMgr_HbInfo_s
{
    SS_InterCoreMgr_CPU_e hbCPU;
    u32                   hbSequenceNum;
    u32                   timestampInMs;
} SS_InterCoreMgr_HbInfo_t;

typedef enum
{
    E_COREMGR_STATE_Normal = 0,
    E_COREMGR_STATE_STR,
    E_COREMGR_STATE_CHARGE,
    E_COREMGR_STATE_MAX,
} SS_InterCoreMgr_State_e;

typedef struct SS_COREMGR_stateInfo_s
{
    SS_InterCoreMgr_CPU_e   notifyCpu;
    SS_InterCoreMgr_State_e newState;
    u32                     timestampInMs;
} SS_InterCoreMgr_stateInfo_t;

typedef struct SS_InterCoreMgr_queryStateInfo_s
{
    SS_InterCoreMgr_CPU_e queryCpu;
    u32                   timestampInMs;
} SS_InterCoreMgr_queryStateInfo_t;

typedef void (*ReceiverCallback)(SS_InterCoreMgr_Action_e eAction, void *data);

s32 SS_InterCoreMgr_Comm_Init(ReceiverCallback pRecvCb);
s32 SS_InterCoreMgr_Comm_DeInit(void);
s32 SS_InterCoreMgr_Scan(SS_InterCoreMgr_CPU_e peerCpu, u32 scanTimeoutInMs, SS_InterCoreMgr_ScanInfo_t *pScanInfo);
s32 SS_InterCoreMgr_ScanAck(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_ScanAckInfo_t *pScanAckInfo);
s32 SS_InterCoreMgr_Heartbeat_Tick(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo);
s32 SS_InterCoreMgr_Heartbeat_Stop(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo);
s32 SS_InterCoreMgr_Notify_State(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_stateInfo_t *pStateInfo);
s32 SS_InterCoreMgr_Query_State(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_queryStateInfo_t *pQueryStateInfo);

#ifdef __cplusplus
}
#endif

#endif
