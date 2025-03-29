/*
 * sys_ioctl.h- Sigmastar
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

#ifndef _SYS_IOCTL_H_
#define _SYS_IOCTL_H_

typedef enum
{
    E_MI_SYS_CMD_INIT,
    E_MI_SYS_CMD_EXIT,
    E_MI_SYS_CMD_BIND_CHN_PORT,
    E_MI_SYS_CMD_BIND_CHN_PORT2,
    E_MI_SYS_CMD_UNBIND_CHN_PORT,
    E_MI_SYS_CMD_GET_BIND_BY_DEST,
    E_MI_SYS_CMD_GET_VERSION,
    E_MI_SYS_CMD_GET_CUR_PTS,
    E_MI_SYS_CMD_INIT_PTS_BASE,
    E_MI_SYS_CMD_SYNC_PTS,
    E_MI_SYS_CMD_MMAP,
    E_MI_SYS_CMD_MUNMAP,
    E_MI_SYS_CMD_VA2PA,
    E_MI_SYS_CMD_SET_REG,
    E_MI_SYS_CMD_GET_REG,
    E_MI_SYS_CMD_SET_CHN_MMA_CONF,
    E_MI_SYS_CMD_GET_CHN_MMA_CONF,
    E_MI_SYS_CMD_CHN_INPUT_PORT_GET_BUF,
    E_MI_SYS_CMD_CHN_INPUT_PORT_PUT_BUF,
    E_MI_SYS_CMD_CHN_OUTPUT_PORT_GET_BUF,
    E_MI_SYS_CMD_CHN_OUTPUT_PORT_PUT_BUF,
    E_MI_SYS_CMD_SET_CHN_OUTPUT_PORT_DEPTH,
    E_MI_SYS_CMD_GET_CHN_OUTPUT_PORT_DEPTH,
    E_MI_SYS_CMD_GET_POLL_PRIVATE_DATA,
    E_MI_SYS_CMD_CHN_PORT_INJECT_BUF,
    E_MI_SYS_MMA_ALLOC,
    E_MI_SYS_MMA_FREE,
    E_MI_SYS_FLUSH_INV_CACHE,
    E_MI_SYS_CMD_READ_UUID,
    E_MI_SYS_CONFIG_PRIVATE_MMA_HEAP,
    E_MI_SYS_MEMSET_PA,
    E_MI_SYS_MEMCPY_PA,
    E_MI_SYS_DMAMEMCPY,
    E_MI_SYS_BUF_FILL_PA,
    E_MI_SYS_BUF_BLIT_PA,
    E_MI_SYS_CMD_PRIV_DEV_CHN_HEAP_ALLOC,
    E_MI_SYS_CMD_PRIV_DEV_CHN_HEAP_FREE,
    E_MI_SYS_CMD_CHN_OUTPUTPORT_LOWLATENCY,
    E_MI_SYS_CMD_CHN_DUP_BUF,
    E_MI_SYS_CMD_CHN_SET_USER_PICTURE,
    E_MI_SYS_CMD_CHN_ENABLE_USER_PICTURE,
    E_MI_SYS_CMD_CHN_DISABLE_USER_PICTURE,
    E_MI_SYS_CMD_SET_CHN_OUTPUT_PORT_USER_FRC,
    E_MI_SYS_CMD_SET_OUTPUT_PORT_BUF_EXT_CONF,
    E_MI_SYS_CMD_SET_INPUT_PORT_FRC,
    E_MI_SYS_CMD_CREATE_CHN_INPUT_DMABUF_ALLOCATOR,
    E_MI_SYS_CMD_DESTROY_CHN_INPUT_DMABUF_ALLOCATOR,
    E_MI_SYS_CMD_CHN_INPUT_PORT_ENQUEUE_DMABUF,
    E_MI_SYS_CMD_CHN_INPUT_PORT_DEQUEUE_DMABUF,
    E_MI_SYS_CMD_CHN_INPUT_PORT_DROP_DMABUF,
    E_MI_SYS_CMD_CREATE_CHN_OUTPUT_DMABUF_ALLOCATOR,
    E_MI_SYS_CMD_DESTROY_CHN_OUTPUT_DMABUF_ALLOCATOR,
    E_MI_SYS_CMD_CHN_OUTPUT_PORT_ENQUEUE_DMABUF,
    E_MI_SYS_CMD_CHN_OUTPUT_PORT_DEQUEUE_DMABUF,
    E_MI_SYS_CMD_CHN_OUTPUT_PORT_DROP_DMABUF,
    E_MI_SYS_CMD_MBX_INIT,
    E_MI_SYS_CMD_MBX_DEINIT,
    E_MI_SYS_CMD_MBX_ENABLE,
    E_MI_SYS_CMD_MBX_DISABLE,
    E_MI_SYS_CMD_MBX_SENDMSG,
    E_MI_SYS_CMD_MBX_RECVMSG,
    E_MI_SYS_CMD_MAX,
} MI_SYS_Cmd_e;

#if 0  // No used. Just for code alignment.
typedef struct MI_SYS_ChnPortArg_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    MI_U32 u32BindParam;    // Keep this variable compatible with older apis.
    MI_SYS_BindAttr_t stBindAttr;

    MI_SYS_ChnPortFrcAttr_t stFrcAttr;
} MI_SYS_ChnPortArg_t;

typedef struct MI_SYS_ChnPortSWLowlatencyArg_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_BOOL bEnable;
    MI_U32 u32DelayMS;

} MI_SYS_ChnPortSWLowlatencyArg_t;
#endif //#if 0 //No used. Just for code alignment.

typedef struct MI_SYS_Mmap_s
{
    MI_BOOL  bCache;
    MI_U32   u32Size;
    MI_PTR64 p64VirtualAddress;
    MI_PHY   phyAddr;
} MI_SYS_Mmap_t;

#if 0  // No used. Just for code alignment.
typedef struct MI_SYS_Munmap_s
{
    MI_PTR64 p64VirtualAddress;
    MI_U32 u32Size;
} MI_SYS_Munmap_t;

typedef struct MI_SYS_VA_PA_s
{
    MI_PTR64 p64VirtualAddress;
    MI_PHY phyAddr;
} MI_SYS_VA2PA_t;

typedef struct MI_SYS_SetReg_s
{
    MI_U32 u32RegAddr;
    MI_U16 u16Value;
    MI_U16 u16Mask;
} MI_SYS_SetReg_t;

typedef struct MI_SYS_GetReg_s
{
    MI_U32 u32RegAddr;
    MI_U16 u16Value;
} MI_SYS_GetReg_t;

typedef struct MI_SYS_SetChnPortMMAConf_s
{
    MI_ModuleId_e eModId;
    MI_U32 u32DevId;
    MI_U32 u32ChnId;
    MI_U8 u8MMAHeapName[MAX_MMA_HEAP_NAME_LENGTH];
} MI_SYS_SetChnPortMMAConf_t;

typedef struct MI_SYS_GetChnPortMMAConf_s
{
    MI_ModuleId_e eModId;
    MI_U32 u32DevId;
    MI_U32 u32ChnId;
    MI_U8 u8Data[MAX_MMA_HEAP_NAME_LENGTH];
    MI_U32 u32Length;
} MI_SYS_GetChnPortMMAConf_t;

typedef struct MI_SYS_SetChnOutputFrcCtrl_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_U32 u32FrcCtrlNumerator;
    MI_U32 u32FrcCtrlDenominator;
} MI_SYS_SetChnOutputFrcCtrl_t;
#endif //#if 0 //No used. Just for code alignment.

typedef struct MI_SYS_ChnInputPortGetBuf_s
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufConf_t  stBufConf;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_BUF_HANDLE BufHandle;
    MI_S32            s32TimeOutMs;
    MI_U32            u32ExtraFlags;
} MI_SYS_ChnInputPortGetBuf_t;

typedef struct MI_SYS_ChnInputPortPutBuf_s
{
    MI_SYS_BUF_HANDLE BufHandle;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_BOOL           bDropBuf;
} MI_SYS_ChnInputPortPutBuf_t;

typedef struct MI_SYS_ChnOutputPortGetBuf_s
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_BUF_HANDLE BufHandle;
    MI_U32            u32TimeoutMs;
    MI_U32            u32ExtraFlags;
} MI_SYS_ChnOutputPortGetBuf_t;

typedef struct MI_SYS_SetChnOutputPortDepth_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_U32           u32UserFrameDepth;
    MI_U32           u32BufQueueDepth;
} MI_SYS_SetChnOutputPortDepth_t;

typedef struct MI_SYS_GetChnOutputPortDepth_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_U32           u32UserFrameDepth;
    MI_U32           u32BufQueueDepth;
} MI_SYS_GetChnOutputPortDepth_t;

typedef struct MI_SYS_ChnPortInjectBuf_s
{
    MI_SYS_BUF_HANDLE BufHandle;
    MI_SYS_ChnPort_t  stChnPort;
} MI_SYS_ChnPortInjectBuf_t;

#if 0  // No used. Just for code alignment.
typedef struct MI_SYS_Mma_Alloc_s
{
    char szMMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
    MI_PTR64 szBufName;
    unsigned int blkSize;
    MI_PHY PhyAddr;
}MI_SYS_Mma_Alloc_t;

typedef struct MI_SYS_Mma_Free_s
{
    MI_PHY phyAddr;
}MI_SYS_Mma_Free_t;
#endif //#if 0 //No used. Just for code alignment.

typedef struct MI_SYS_FlushInvCache_s
{
    MI_PTR64 p64VirtualAddress;
    MI_U64   length;
} MI_SYS_FlushInvCache_t;

#if 0 // No used. Just for code alignment.
typedef struct MI_SYS_GetPollData_s
{
    MI_SYS_ChnPort_t stChnPort;
#ifdef __KERNEL__
    MI_PTR64 p64PrivateData;
#else
    void *pPrivateData;
#endif
}
MI_SYS_GetPollData_t;

typedef struct MI_SYS_ChnPortFd_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_S32 s32Fd;
} MI_SYS_ChnPortFd_t;

typedef struct MI_SYS_ConfigPrivateHeap_s
{
    MI_SYS_InsidePrivatePoolType_e eInsidePrivatePoolType;
    MI_ModuleId_e eModule;
    MI_BOOL bCreate;
    MI_U32 u32DevId;
    MI_U32 u32ChnId;
    MI_U32 u32Port;
    MI_U32 u32PrivateHeapSize;
    MI_U16 u16MaxWidth;
    MI_U16 u16MaxHeight;
    MI_U16 u16RingLine;
    MI_U8 u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
}
MI_SYS_ConfigPrivateHeap_t;
typedef struct MI_SYS_MemsetPaPara_s
{
    MI_PHY phyPa;
    MI_U32 u32Val;
    MI_U32 u32Lenth;
}MI_SYS_MemsetPaPara_t;
typedef struct MI_SYS_MemcpyPaPara_s
{
    MI_PHY phyDst;
    MI_PHY phySrc;
    MI_U32 u32Lenth;
}MI_SYS_MemcpyPaPara_t;

typedef struct MI_SYS_DmaMemcpyPara_s
{
    MI_PHY phyDst;
    MI_PHY phySrc;
    MI_U32 u32Lenth;
    MI_SYS_DmaMemCpyDirect_e eDirect;
}MI_SYS_DmaMemcpyPara_t;

typedef struct MI_SYS_BufFillPaPara_s
{
    MI_SYS_FrameData_t stBuf;
    MI_U32 u32Val;
    MI_SYS_WindowRect_t stRect;
}MI_SYS_BufFillPaPara_t;
typedef struct MI_SYS_BufBlitPaPara_s
{
    MI_SYS_FrameData_t stDstBuf;
    MI_SYS_WindowRect_t stDstRect;
    MI_SYS_FrameData_t stSrcBuf;
    MI_SYS_WindowRect_t stSrcRect;
}MI_SYS_BufBlitPaPara_t;

typedef struct MI_SYS_PrivateDevChnHeapAlloc_s
{
    MI_U8 pu8BufName[MI_MAX_MMA_HEAP_LENGTH];
    MI_PTR64 szMmaName;
    MI_U32 u32BlkSize;
    MI_PHY phyAddr;
    MI_ModuleId_e eModule;
    MI_U32 u32DevId;
    MI_S32 s32ChnId;
    MI_BOOL bTailAlloc;
} MI_SYS_PrivateDevChnHeapAlloc_t;

typedef struct MI_SYS_PrivateDevChnHeapFree_s
{
    MI_PHY phyAddr;
    MI_ModuleId_e eModule;
    MI_U32 u32DevId;
    MI_S32 s32ChnId;
} MI_SYS_PrivateDevChnHeapFree_t;

typedef struct MI_SYS_ChnOutputPortLowLatency_s
{
    MI_SYS_ChnPort_t stChnPort;
    MI_BOOL bEnable;
    MI_U32 u32Param;
} MI_SYS_ChnOutputPortLowLatency_t;
#endif //#if 0 //No used. Just for code alignment.

typedef struct MI_SYS_ChnDupBufHandle_s
{
    MI_SYS_BUF_HANDLE srcBufHandle;
    MI_SYS_BUF_HANDLE dupTargetBufHandle;
} MI_SYS_ChnDupBufHandle_t;

#endif ///_SYS_IOCTL_H_
