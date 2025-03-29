/* SigmaStar trade secret */
/* Copyright (c) [2021~2022] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _MI_NIR_DATATYPE_H_
#define _MI_NIR_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum MI_NIR_RET_e
    {
        E_MI_NIR_SUCCESS,
        E_MI_ERR_NIR_NOT_INIT,
        E_MI_ERR_NIR_DEV_INVALID,
        E_MI_ERR_NIR_NORESOURCE,
        E_MI_ERR_NIR_CHN_INVALID,
        E_MI_ERR_NIR_TIMEOUT,
        E_MI_ERR_NIR_NOMEM,      /*NO BUFF*/
        E_MI_ERR_NIR_NOTSUPPORT, /*NO SUPPORT*/
        E_MI_ERR_NIR_BUSY,       /*CHN BUSY*/
        E_MI_ERR_NIR_NOTREADY,   /*CHN Not Ready*/
        E_MI_ERR_NIR_NULL_PTR,
        E_MI_ERR_NIR_DEV_NOT_CREATE,
        E_MI_ERR_NIR_CHN_NOT_CREATE,
    } MI_NIR_RET;

    typedef MI_U32 MI_NIR_DEV;
    typedef MI_U32 MI_NIR_CHANNEL;

    typedef struct MI_NIR_DevAttr_s
    {
        MI_U32 u32Reserved;
    } MI_NIR_DevAttr_t;

    typedef enum
    {
        E_MI_NIR_MODE_NORMAL,
        E_MI_NIR_MODE_BYPASS_VIS,
        E_MI_NIR_MODE_BYPASS_NIR,
        E_MI_NIR_MODE_INVALID,
    } MI_NIR_Mode_e;

    typedef struct MI_NIR_ChannelAttr_s
    {
        MI_NIR_Mode_e eMode;
    } MI_NIR_ChannelAttr_t;

    typedef struct MI_NIR_ChnParam_s
    {
        MI_NIR_Mode_e eMode;
    } MI_NIR_ChnParam_t;

#define MI_NIR_OK                 (0)
#define MI_NIR_ERR                (-1)
#define MI_ERR_NIR_MOD_NOT_INIT   MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_NOT_INIT)
#define MI_ERR_NIR_DEV_INVALID    MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_DEV_INVALID)
#define MI_ERR_NIR_NORESOURCE     MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_NORESOURCE)
#define MI_ERR_NIR_CHN_INVALID    MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_CHN_INVALID)
#define MI_ERR_NIR_TIMEOUT        MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_TIMEOUT)
#define MI_ERR_NIR_NOMEM          MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_NOMEM)
#define MI_ERR_NIR_NOTSUPPORT     MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_NOTSUPPORT)
#define MI_ERR_NIR_BUSY           MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_BUSY)
#define MI_ERR_NIR_NOTREADY       MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_NOTREADY)
#define MI_ERR_NIR_NULL_PTR       MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_NULL_PTR)
#define MI_ERR_NIR_DEV_NOT_CREATE MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_DEV_NOT_CREATE)
#define MI_ERR_NIR_CHN_NOT_CREATE MI_DEF_ERR(E_MI_MODULE_ID_NIR, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NIR_CHN_NOT_CREATE)

#ifdef __cplusplus
}
#endif
#endif
