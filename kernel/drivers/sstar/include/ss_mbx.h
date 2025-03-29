/*
 * ss_mbx.h- Sigmastar
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

///////////////////////////////////////////////////////////////////////////////
/// @file      ss_mbx.h
/// @brief     ss mailbox File for
///            1. RTK OS
///            2. Linux User Space
///////////////////////////////////////////////////////////////////////////////

#ifndef __SS_MBX_H__
#define __SS_MBX_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef NULL
#define NULL 0
#endif

#define SS_MBX_MAX_PARAM_SIZE 6
#define SS_MBX_USER_MAX_CLASS 200 // 0~199

typedef unsigned char  u8;
typedef signed char    s8;
typedef unsigned short u16;
typedef signed short   s16;
typedef unsigned int   u32;
typedef signed int     s32;

/* mbx direction */
typedef enum
{
    E_SS_MBX_DIRECT_CM4_TO_ARM,
    E_SS_MBX_DIRECT_ARM_TO_CM4,
    E_SS_MBX_DIRECT_MAX,
} SS_Mbx_Direct_e;

/// Mail Message Define
typedef struct SS_Mbx_Msg_s
{
    /// mailbox direction
    SS_Mbx_Direct_e eDirect;
    /// @ref MBX_Class
    u8 u8MsgClass;
    /// Parameter Count
    u8 u8ParameterCount;
    /// Parameters, Max Number @ref SS_MBX_MAX_PARAM_SIZE
    u16 u16Parameters[SS_MBX_MAX_PARAM_SIZE];
} SS_Mbx_Msg_t;

typedef enum
{
    E_SS_MBX_RET_OK            = 0,
    E_SS_MBX_RET_FAIL          = 1,
    E_SS_MBX_RET_NOT_INIT      = 2,
    E_SS_MBX_RET_NOT_ENABLE    = 3,
    E_SS_MBX_RET_INVAILD_PARAM = 4,
    E_SS_MBX_RET_TIME_OUT      = 5,
    E_SS_MBX_RET_NO_MEM        = 6,
    // send err code
    E_SS_MBX_RET_REMOTE_CLASS_NOT_ENABLE = 10,
    E_SS_MBX_RET_REMOTE_CLASS_QUEUE_FULL = 11,
    E_SS_MBX_RET_REMOTE_SYSTEM_NO_MEM    = 12,
    E_SS_MBX_RET_MAX,
} SS_Mbx_Ret_e;

//=============================================================================
// Description:
//      Initializes the mailbox.
// Parameters:
//      N/A
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_Init(void);

//=============================================================================
// Description:
//      Deinitializes the mailbox.
// Parameters:
//      N/A
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================

int SS_Mailbox_Deinit(void);

//=============================================================================
// Description:
//      Enable the mailbox class.
// Parameters:
//      [in]  msgClass: The mailbox class to enable.
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_Enable(u8 u8MsgClass);

//=============================================================================
// Description:
//      Disable the mailbox class.
// Parameters:
//      [in]  u8MsgClass: The mailbox class to disable.
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_Disable(u8 u8MsgClass);

//=============================================================================
// Description:
//      Disable the mailbox class.
// Parameters:
//      [in/out]  pstMbxMsg: The mailbox class message to send.
//      [in]  s32WaitMs: <0 is wait endlessly, >=0 is wait time.
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_SendMsg(SS_Mbx_Msg_t *pstMbxMsg);

//=============================================================================
// Description:
//      Disable the mailbox class.
// Parameters:
//      [in]  u8MsgClass: The mailbox class to recv.
//      [out]  pstMbxMsg: The mailbox message to recv.
//      [in]  s32WaitMs: <0 is wait endlessly, >=0 is wait time.
// Return:
//      0 is returned if successful; otherwise, returns SS_MbxRet_e.
//=============================================================================
int SS_Mailbox_RecvMsg(u8 u8MsgClass, SS_Mbx_Msg_t *pstMbxMsg, s32 s32WaitMs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SS_MBX_H__ */
