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
#ifndef _MI_DPU_DATATYPE_H_
#define _MI_DPU_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum MI_DPU_RET_e
    {
        E_MI_ERR_DPU_SUCCESS,
        E_MI_ERR_DPU_NOT_INIT,
        E_MI_ERR_DPU_DEV_INVALID,
        E_MI_ERR_DPU_CHN_INVALID,
        E_MI_ERR_DPU_TIMEOUT,
        E_MI_ERR_DPU_NOMEM,       /*NO BUFF*/
        E_MI_ERR_DPU_NOT_SUPPORT, /*NO SUPPORT*/
        E_MI_ERR_DPU_BUSY,        /*CHN BUSY*/
        E_MI_ERR_DPU_NOT_READY,
        E_MI_ERR_DPU_NULL_PTR,
    } MI_DPU_RET;

    typedef MI_U32 MI_DPU_DEV;
    typedef MI_U32 MI_DPU_CHANNEL;

    typedef struct MI_DPU_DevAttr_s
    {
        MI_U32 u32Reserved;
    } MI_DPU_DevAttr_t;

    typedef struct MI_DPU_ChnAttr_s
    {
        MI_U16 u16Width;
        MI_U16 u16Height;
        MI_U16 u16MinDisparity;
        MI_U16 u16MaxDisparity;
    } MI_DPU_ChnAttr_t;

    typedef struct MI_DPU_ChnParam_s
    {
        MI_U16 u16Width;
        MI_U16 u16Height;
        MI_U16 u16MinDisparity;
        MI_U16 u16MaxDisparity;
    } MI_DPU_ChnParam_t;

#define MI_DPU_RET_SUCCESS     (0)
#define MI_ERR_DPU_NOT_INIT    MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_NOT_INIT)
#define MI_ERR_DPU_DEV_INVALID MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_DEV_INVALID)
#define MI_ERR_DPU_CHN_INVALID MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_CHN_INVALID)
#define MI_ERR_DPU_TIMEOUT     MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_TIMEOUT)
#define MI_ERR_DPU_NOMEM       MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_NOMEM)
#define MI_ERR_DPU_NOT_SUPPORT MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_NOT_SUPPORT)
#define MI_ERR_DPU_BUSY        MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_BUSY)
#define MI_ERR_DPU_NOT_READY   MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_NOT_READY)
#define MI_ERR_DPU_NULL_PTR    MI_DEF_ERR(E_MI_MODULE_ID_DPU, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DPU_NULL_PTR)

#ifdef __cplusplus
}
#endif
#endif
