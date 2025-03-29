/*
 * ss_mbx_impl.h- Sigmastar
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

#ifndef _SS_MBX_IMPL_H_
#define _SS_MBX_IMPL_H_

#include "ss_mbx.h"

//=============================================================================
// Description:
//      Initializes the mailbox.
// Parameters:
//      N/A
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_IMPL_Init(void);

//=============================================================================
// Description:
//      Deinitializes the mailbox.
// Parameters:
//      N/A
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================

int SS_Mailbox_IMPL_Deinit(void);

//=============================================================================
// Description:
//      Enable the mailbox class.
// Parameters:
//      [in]  msgClass: The mailbox class to enable.
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_IMPL_Enable(u8 u8MsgClass);

//=============================================================================
// Description:
//      Disable the mailbox class.
// Parameters:
//      [in]  u8MsgClass: The mailbox class to disable.
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_IMPL_Disable(u8 u8MsgClass);

//=============================================================================
// Description:
//      Disable the mailbox class.
// Parameters:
//      [in/out]  pstMbxMsg: The mailbox class message to send.
//      [in]  s32WaitMs: <0 is wait endlessly, >=0 is wait time.
// Return:
//      0 is returned if successful; otherwise, returns SS_Mbx_Ret_e.
//=============================================================================
int SS_Mailbox_IMPL_SendMsg(SS_Mbx_Msg_t *pstMbxMsg);

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
int SS_Mailbox_IMPL_RecvMsg(u8 u8MsgClass, SS_Mbx_Msg_t *pstMbxMsg, s32 s32WaitMs);

#endif
