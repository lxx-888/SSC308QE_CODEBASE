/*
 * mi_sys_buf_mgr.h- Sigmastar
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

#ifndef _MI_SYS_BUFMGR_H_
#define _MI_SYS_BUFMGR_H_

struct MI_SYS_Buf_Ref_s;

typedef void (*OnBufRefRelFunc)(struct MI_SYS_Buf_Ref_s *pstBufRef, void *pCBData);

typedef struct MI_SYS_Buf_Ref_s
{
    unsigned int                      u32MagicNumber;
    struct CamOsListHead_t            list;
    struct MI_SYS_BufferAllocation_s *pstBufAllocation;
    OnBufRefRelFunc                   onRelCB;
    void *                            pCBData;
    MI_SYS_BufInfo_t                  bufinfo;
    MI_BOOL                           bInplaceProcess;
    MI_U32                            u8DupMultiPlaneIndex;
    MI_BOOL                           is_dumped;
    MI_U32                            u32ExtFlag;
    struct CamOsListHead_t            list_in_pass; // list head in dev pass ordered by time stampe
    struct CamOsListHead_t            list_in_port; // list head in port in FIFO way
    MI_U32                            u32ChannelId;
    union
    {
        MI_S64 s64FireTimeStampInNS; // the fire time stamp, MI_SYS_FIRE_IMMEDIATELY for fire immediately
    } uTimeStamp;
    MI_U32 u32InferFrameNumber;
    MI_U32 u32EnqueueInferPulseIndex;  // the infer pulse index when buffer eqnueued
    MI_S64 s64BufContentTimeStampInNS; // buffer generated time stamp for performance measurement
    MI_U32 u32TaskBufContentPendingInUS;

    struct MI_SYS_Buf_Ref_s *pstSrcBufRef; // pointer to source buffer reference
} MI_SYS_Buf_Ref_t;

#endif
