/*
 * mi_sys_impl.h- Sigmastar
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

#ifndef _MI_SYS_IMPL_H_
#define _MI_SYS_IMPL_H_

typedef enum
{
    E_MI_SYS_IDR_BUF_TYPE_INPUT_PORT = 0,
    E_MI_SYS_IDR_BUF_TYPE_OUTPUT_PORT,
    E_MI_SYS_IDR_BUF_TYPE_MMAP_TO_USER_SPACE,
    E_MI_SYS_IDR_BUF_TYPE_USER_MMA_ALLOC,
    E_MI_SYS_IDR_BUF_TYPE_USER_PRIVATE_POOL_ALLOC,
} MI_SYS_IDR_BUF_TYPE_e;

typedef struct MI_SYS_UserPictureWotkThreadInfo_s
{
    CamOsThread              user_picture_thread_handle;
    MI_SYS_UserPictureInfo_t stUserPictureInfo;
    CamOsCondition_t         user_picture_thread_waitqueue;
    CamOsAtomic_t            bWakeUp;
    CamOsAtomic_t            bEnable;
    void *                   pHwTimerHandle;
} MI_SYS_UserPictureWotkThreadInfo_t;

typedef struct MI_SYS_BufHandleIdrData_s
{
    struct CamOsListHead_t list;
    MI_SYS_IDR_BUF_TYPE_e  eBufType;
    MI_SYS_Buf_Ref_t *     pstBufRef;
    union
    {
        MI_SYS_ChnPort_t  stChnPort;
        MI_SYS_DRV_HANDLE miSysDrvHandle;
    };
    MI_U32                              u32ExtraFlags;
    MI_S32                              s32Pid;
    MI_PHY                              phyAddr;
    MI_VIRTx2                           v2BufInfo;
    MI_SYS_BUF_HANDLE                   BufHandle;
    MI_SYS_UserPictureWotkThreadInfo_t *pstUserPictureWorkThreadInfo;
} MI_SYS_BufHandleIdrData_t;

#endif
