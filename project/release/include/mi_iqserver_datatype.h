/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
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
#ifndef _MI_IQSERVER_DATATYPE_H_
#define _MI_IQSERVER_DATATYPE_H_

#include "mi_common.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef enum
{
    E_MI_IQSERVER_PROTOCOL_NETWORK = 0,
    E_MI_IQSERVER_PROTOCOL_UVC,
    E_MI_IQSERVER_PROTOCOL_UART,
    E_MI_IQSERVER_PROTOCOL_MAX
} MI_IQSERVER_ProtocolType_e;

typedef enum
{
    E_MI_IQSERVER_USB_CMD_LEN = 0,
    E_MI_IQSERVER_USB_CMD_TYPE,
    E_MI_IQSERVER_CMD_MAX
}MI_IQSERVER_CMD_TYPE_e;

typedef enum
{
    E_MI_ERR_IQSERVER_NOMEM,
    E_MI_ERR_IQSERVER_NOBUF,
    E_MI_ERR_IQSERVER_NULL_PTR,
    E_MI_ERR_IQSERVER_CREATE_THREAD,
    E_MI_ERR_IQSERVER_INVALID_IQPATH,
} MI_IQSERVER_ErrCode_e;

#define MI_IQSERVER_OK (0)
#define MI_ERR_IQSERVER_NOMEM          MI_DEF_ERR(E_MI_MODULE_ID_IQSERVER, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_IQSERVER_NOMEM)
#define MI_ERR_IQSERVER_NOBUF          MI_DEF_ERR(E_MI_MODULE_ID_IQSERVER, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_IQSERVER_NOBUF)
#define MI_ERR_IQSERVER_NULL_PTR       MI_DEF_ERR(E_MI_MODULE_ID_IQSERVER, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_IQSERVER_NULL_PTR)
#define MI_ERR_IQSERVER_CREATE_THREAD  MI_DEF_ERR(E_MI_MODULE_ID_IQSERVER, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_IQSERVER_CREATE_THREAD)
#define MI_ERR_IQSERVER_INVALID_IQPATH MI_DEF_ERR(E_MI_MODULE_ID_IQSERVER, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_IQSERVER_INVALID_IQPATH)

#ifdef __cplusplus
}
#endif

#endif
