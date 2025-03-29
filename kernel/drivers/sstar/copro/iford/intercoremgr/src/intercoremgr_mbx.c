/*
 * intercoremgr_mbx.c - Sigmastar
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

#include "intercoremgr_mbx.h"
#include "cam_os_wrapper.h"
#include "ss_mbx.h"

#ifdef CAM_OS_RTK
#define CMR_MBX_DIRE     E_SS_MBX_DIRECT_CM4_TO_ARM
#define CMR_MBX_CPU      E_COREMGR_CPU_CM4
#define CMR_MBX_PEER_CPU E_COREMGR_CPU_ARM
#elif defined(CAM_OS_LINUX_USER)
#define CMR_MBX_DIRE     E_SS_MBX_DIRECT_ARM_TO_CM4
#define CMR_MBX_CPU      E_COREMGR_CPU_ARM
#define CMR_MBX_PEER_CPU E_COREMGR_CPU_CM4
#endif

#define CMR_MBX_DEF_MS 1000
#define CMR_DEBUG      1

static CamOsThread gMbxRecvTid = 0;
static u8          gbExit      = 0;

typedef union
{
    SS_InterCoreMgr_ScanInfo_t       scanInfo;
    SS_InterCoreMgr_ScanAckInfo_t    scanAckInfo;
    SS_InterCoreMgr_HbInfo_t         hbInfo;
    SS_InterCoreMgr_stateInfo_t      stateInfo;
    SS_InterCoreMgr_queryStateInfo_t queryStateInfo;
} Cmr_Mbx_Cb_u;

// [in]:eAction, arg. [out]:pstMsg.
void _InterCoreMgr_Mbx_Pack(SS_InterCoreMgr_Action_e eAction, void *arg, SS_Mbx_Msg_t *pstMsg)
{
    switch (eAction)
    {
        case E_COREMGR_SCAN:
        {
            SS_InterCoreMgr_ScanInfo_t *pScanInfo = (SS_InterCoreMgr_ScanInfo_t *)arg;

            pstMsg->u8MsgClass       = E_CMR_MBX_CLASS_HB;
            pstMsg->eDirect          = CMR_MBX_DIRE;
            pstMsg->u8ParameterCount = 4;
            pstMsg->u16Parameters[0] = E_CMR_MBX_HB_SCAN;
            pstMsg->u16Parameters[1] = pScanInfo->hbPeriodInMs;
            pstMsg->u16Parameters[2] = pScanInfo->timestampInMs & 0xFFFF;
            pstMsg->u16Parameters[3] = pScanInfo->timestampInMs >> 16;
            break;
        }
        case E_COREMGR_SCAN_ACK:
        {
            SS_InterCoreMgr_ScanAckInfo_t *pScanAckInfo = (SS_InterCoreMgr_ScanAckInfo_t *)arg;

            pstMsg->u8MsgClass       = E_CMR_MBX_CLASS_HB;
            pstMsg->eDirect          = CMR_MBX_DIRE;
            pstMsg->u8ParameterCount = 4;
            pstMsg->u16Parameters[0] = E_CMR_MBX_HB_SCAN_ACK;
            pstMsg->u16Parameters[1] = pScanAckInfo->hbPeriodInMs;
            pstMsg->u16Parameters[2] = pScanAckInfo->timestampInMs & 0xFFFF;
            pstMsg->u16Parameters[3] = pScanAckInfo->timestampInMs >> 16;
            break;
        }
        case E_COREMGR_HB:
        {
            SS_InterCoreMgr_HbInfo_t *pHbInfo = (SS_InterCoreMgr_HbInfo_t *)arg;

            pstMsg->u8MsgClass       = E_CMR_MBX_CLASS_HB;
            pstMsg->eDirect          = CMR_MBX_DIRE;
            pstMsg->u8ParameterCount = 5;
            pstMsg->u16Parameters[0] = E_CMR_MBX_HB_TICK;
            pstMsg->u16Parameters[1] = pHbInfo->hbSequenceNum & 0xFFFF;
            pstMsg->u16Parameters[2] = pHbInfo->hbSequenceNum >> 16;
            pstMsg->u16Parameters[3] = pHbInfo->timestampInMs & 0xFFFF;
            pstMsg->u16Parameters[4] = pHbInfo->timestampInMs >> 16;
            break;
        }
        case E_COREMGR_HB_STOP:
        {
            SS_InterCoreMgr_HbInfo_t *pHbInfo = (SS_InterCoreMgr_HbInfo_t *)arg;

            pstMsg->u8MsgClass       = E_CMR_MBX_CLASS_HB;
            pstMsg->eDirect          = CMR_MBX_DIRE;
            pstMsg->u8ParameterCount = 5;
            pstMsg->u16Parameters[0] = E_CMR_MBX_HB_STOP;
            pstMsg->u16Parameters[1] = pHbInfo->hbSequenceNum & 0xFFFF;
            pstMsg->u16Parameters[2] = pHbInfo->hbSequenceNum >> 16;
            pstMsg->u16Parameters[3] = pHbInfo->timestampInMs & 0xFFFF;
            pstMsg->u16Parameters[4] = pHbInfo->timestampInMs >> 16;
            break;
        }
        case E_COREMGR_STATE_QUERY:
        {
            SS_InterCoreMgr_queryStateInfo_t *pQueryStateInfo = (SS_InterCoreMgr_queryStateInfo_t *)arg;

            pstMsg->u8MsgClass       = E_CMR_MBX_CLASS_STATE;
            pstMsg->eDirect          = CMR_MBX_DIRE;
            pstMsg->u8ParameterCount = 3;
            pstMsg->u16Parameters[0] = E_CMR_MBX_STATE_QUERY;
            pstMsg->u16Parameters[1] = pQueryStateInfo->timestampInMs & 0xFFFF;
            pstMsg->u16Parameters[2] = pQueryStateInfo->timestampInMs >> 16;
            break;
        }
        case E_COREMGR_STATE_NOTIFY:
        {
            SS_InterCoreMgr_stateInfo_t *pStateInfo = (SS_InterCoreMgr_stateInfo_t *)arg;

            pstMsg->u8MsgClass       = E_CMR_MBX_CLASS_STATE;
            pstMsg->eDirect          = CMR_MBX_DIRE;
            pstMsg->u8ParameterCount = 4;
            pstMsg->u16Parameters[0] = E_CMR_MBX_STATE_NOTIFY;
            pstMsg->u16Parameters[1] = pStateInfo->newState & 0xFFFF;
            pstMsg->u16Parameters[2] = pStateInfo->timestampInMs & 0xFFFF;
            pstMsg->u16Parameters[3] = pStateInfo->timestampInMs >> 16;
            break;
        }
        default:
        {
            CamOsPrintf("%s[%d] not support this action:%d.\n", __FILE__, __LINE__, eAction);
        }
    }
}

// [in]:pstMsg. [out]:peAction, arg.
void _InterCoreMgr_Mbx_UnPack(SS_Mbx_Msg_t *pstMsg, SS_InterCoreMgr_Action_e *peAction, void *arg)
{
    if (pstMsg->u8MsgClass == E_CMR_MBX_CLASS_HB)
    {
        switch (pstMsg->u16Parameters[0])
        {
            case E_CMR_MBX_HB_SCAN:
            {
                SS_InterCoreMgr_ScanInfo_t *pScanInfo = (SS_InterCoreMgr_ScanInfo_t *)arg;

                *peAction                = E_COREMGR_SCAN;
                pScanInfo->scanCPU       = CMR_MBX_CPU;
                pScanInfo->hbPeriodInMs  = pstMsg->u16Parameters[1];
                pScanInfo->timestampInMs = pstMsg->u16Parameters[2] + (pstMsg->u16Parameters[3] << 16);
                break;
            }
            case E_CMR_MBX_HB_SCAN_ACK:
            {
                SS_InterCoreMgr_ScanAckInfo_t *pScanAckInfo = (SS_InterCoreMgr_ScanAckInfo_t *)arg;

                *peAction                   = E_COREMGR_SCAN_ACK;
                pScanAckInfo->ackCPU        = CMR_MBX_CPU;
                pScanAckInfo->hbPeriodInMs  = pstMsg->u16Parameters[1];
                pScanAckInfo->timestampInMs = pstMsg->u16Parameters[2] + (pstMsg->u16Parameters[3] << 16);
                break;
            }
            case E_CMR_MBX_HB_TICK:
            {
                SS_InterCoreMgr_HbInfo_t *pHbInfo = (SS_InterCoreMgr_HbInfo_t *)arg;

                *peAction              = E_COREMGR_HB;
                pHbInfo->hbCPU         = CMR_MBX_CPU;
                pHbInfo->hbSequenceNum = pstMsg->u16Parameters[1] + (pstMsg->u16Parameters[2] << 16);
                pHbInfo->timestampInMs = pstMsg->u16Parameters[3] + (pstMsg->u16Parameters[4] << 16);
                break;
            }
            case E_CMR_MBX_HB_STOP:
            {
                SS_InterCoreMgr_HbInfo_t *pHbInfo = (SS_InterCoreMgr_HbInfo_t *)arg;

                *peAction              = E_COREMGR_HB_STOP;
                pHbInfo->hbCPU         = CMR_MBX_CPU;
                pHbInfo->hbSequenceNum = pstMsg->u16Parameters[1] + (pstMsg->u16Parameters[2] << 16);
                pHbInfo->timestampInMs = pstMsg->u16Parameters[3] + (pstMsg->u16Parameters[4] << 16);
                break;
            }
            default:
            {
                CamOsPrintf("%s[%d] undefined coremgr mbx heartbeta type:%d.\n", __FILE__, __LINE__,
                            pstMsg->u16Parameters[0]);
            }
        }
    }
    else if (pstMsg->u8MsgClass == E_CMR_MBX_CLASS_STATE)
    {
        switch (pstMsg->u16Parameters[0])
        {
            case E_CMR_MBX_STATE_NOTIFY:
            {
                SS_InterCoreMgr_stateInfo_t *pStateInfo = (SS_InterCoreMgr_stateInfo_t *)arg;

                *peAction                 = E_COREMGR_STATE_NOTIFY;
                pStateInfo->notifyCpu     = CMR_MBX_CPU;
                pStateInfo->newState      = pstMsg->u16Parameters[1];
                pStateInfo->timestampInMs = pstMsg->u16Parameters[2] + (pstMsg->u16Parameters[3] << 16);
                break;
            }
            case E_CMR_MBX_STATE_QUERY:
            {
                SS_InterCoreMgr_queryStateInfo_t *pQueryStateInfo = (SS_InterCoreMgr_queryStateInfo_t *)arg;

                *peAction                      = E_COREMGR_STATE_QUERY;
                pQueryStateInfo->queryCpu      = CMR_MBX_CPU;
                pQueryStateInfo->timestampInMs = pstMsg->u16Parameters[1] + (pstMsg->u16Parameters[2] << 16);
                break;
            }
            default:
            {
                CamOsPrintf("%s[%d] undefined coremgr mbx state option:%d.\n", __FILE__, __LINE__,
                            pstMsg->u16Parameters[0]);
            }
        }
    }
    else
    {
        CamOsPrintf("%s[%d] undefined coremgr mbx class:%d.\n", __FILE__, __LINE__, pstMsg->u8MsgClass);
    }
}

void *_InterCoreMgr_Mbx_Receiver(void *arg)
{
    u8 class                           = 0;
    SS_Mbx_Msg_t             stMsg     = {0};
    Cmr_Mbx_Cb_u             uRecvCbSt = {0};
    SS_InterCoreMgr_Action_e eAction   = E_COREMGR_MAX;
    ReceiverCallback         pRecvCb   = (ReceiverCallback)arg;

#ifdef CAM_OS_RTK
#elif defined(CAM_OS_LINUX_USER)
    while (!gbExit)
    {
        for (class = CMR_MBX_CLASS_BASE; class < E_CMR_MBX_CLASS_MAX; class ++)
        {
            if (0 == SS_Mailbox_RecvMsg(class, &stMsg, 0))
            {
                _InterCoreMgr_Mbx_UnPack(&stMsg, &eAction, &uRecvCbSt);
                pRecvCb(eAction, &uRecvCbSt);
            }
        }

        CamOsMsSleep(10);
    }
#endif

    return (void *)0;
}

s32 InterCoreMgr_Mbx_Init(ReceiverCallback pRecvCb)
{
    u8 class = 0;
    s32 ret  = 0;

    ret = SS_Mailbox_Init();
    if (ret)
        return ret;

    for (class = CMR_MBX_CLASS_BASE; class < E_CMR_MBX_CLASS_MAX; class ++)
        ret |= SS_Mailbox_Enable(class);

    if (ret)
    {
        SS_Mailbox_Deinit();
        return ret;
    }

    gbExit = 0;
    CamOsThreadCreate(&gMbxRecvTid, NULL, _InterCoreMgr_Mbx_Receiver, pRecvCb);

    return ret;
}

s32 InterCoreMgr_Mbx_Deinit(void)
{
    u8 class = 0;
    s32 ret  = 0;

    gbExit = 1;
    CamOsThreadJoin(gMbxRecvTid);

    for (class = CMR_MBX_CLASS_BASE; class < E_CMR_MBX_CLASS_MAX; class ++)
        ret |= SS_Mailbox_Disable(class);

    ret |= SS_Mailbox_Deinit();

    return ret;
}

s32 InterCoreMgr_Mbx_Scan(SS_InterCoreMgr_CPU_e peerCpu, u32 scanTimeoutInMs, SS_InterCoreMgr_ScanInfo_t *pScanInfo)
{
    SS_Mbx_Msg_t stMsg = {0};

    _InterCoreMgr_Mbx_Pack(E_COREMGR_SCAN, pScanInfo, &stMsg);
    return SS_Mailbox_SendMsg(&stMsg, scanTimeoutInMs);
}

s32 InterCoreMgr_Mbx_ScanAck(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_ScanAckInfo_t *pScanAckInfo)
{
    SS_Mbx_Msg_t stMsg = {0};

    _InterCoreMgr_Mbx_Pack(E_COREMGR_SCAN_ACK, pScanAckInfo, &stMsg);
    return SS_Mailbox_SendMsg(&stMsg, CMR_MBX_DEF_MS);
}

s32 InterCoreMgr_Mbx_HbTick(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo)
{
    SS_Mbx_Msg_t stMsg = {0};

    _InterCoreMgr_Mbx_Pack(E_COREMGR_HB, pHbInfo, &stMsg);
    return SS_Mailbox_SendMsg(&stMsg, CMR_MBX_DEF_MS);
}

s32 InterCoreMgr_Mbx_HbStop(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_HbInfo_t *pHbInfo)
{
    SS_Mbx_Msg_t stMsg = {0};

    _InterCoreMgr_Mbx_Pack(E_COREMGR_HB_STOP, pHbInfo, &stMsg);
    return SS_Mailbox_SendMsg(&stMsg, CMR_MBX_DEF_MS);
}

s32 InterCoreMgr_Mbx_NotifyState(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_stateInfo_t *pStateInfo)
{
    SS_Mbx_Msg_t stMsg = {0};

    _InterCoreMgr_Mbx_Pack(E_COREMGR_STATE_NOTIFY, pStateInfo, &stMsg);
    return SS_Mailbox_SendMsg(&stMsg, CMR_MBX_DEF_MS);
}

s32 InterCoreMgr_Mbx_QueryState(SS_InterCoreMgr_CPU_e peerCpu, SS_InterCoreMgr_queryStateInfo_t *pQueryStateInfo)
{
    SS_Mbx_Msg_t stMsg = {0};

    _InterCoreMgr_Mbx_Pack(E_COREMGR_STATE_QUERY, pQueryStateInfo, &stMsg);
    return SS_Mailbox_SendMsg(&stMsg, CMR_MBX_DEF_MS);
}
