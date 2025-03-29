/* SigmaStar trade secret */
/* Copyright (c) [2022~2023] SigmaStar Technology.
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
#ifndef _MI_HDMIRX_H_
#define _MI_HDMIRX_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "mi_common.h"
#include "mi_hdmirx_datatype.h"

#define HDMI_RX_MAJOR_VERSION 1
#define HDMI_RX_SUB_VERSION   0
#define MACRO_TO_STR(macro)   #macro
#define HDMI_RX_VERSION_STR(major_version, sub_version)                                                                \
    (                                                                                                                  \
        {                                                                                                              \
            char *tmp =                                                                                                \
                sub_version / 100  ? "mi_hdmirx_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_hdmirx_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_hdmirx_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                       \
        })
#define MI_HDMI_RX_API_VERSION HDMI_RX_VERSION_STR(HDMI_RX_MAJOR_VERSION, HDMI_RX_SUB_VERSION)

    MI_S32 MI_HDMIRX_Init(void);
    MI_S32 MI_HDMIRX_DeInit(void);
    MI_S32 MI_HDMIRX_Connect(MI_HDMIRX_PortId_e ePortId);
    MI_S32 MI_HDMIRX_DisConnect(MI_HDMIRX_PortId_e ePortId);
    MI_S32 MI_HDMIRX_GetHdcpStatus(MI_HDMIRX_PortId_e ePortId, MI_BOOL *pbHdcpStatus);
    MI_S32 MI_HDMIRX_GetTimingInfo(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_TimingInfo_t *pstTimingInfo);
    MI_S32 MI_HDMIRX_GetSignalStatus(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_SigStatus_e *peStatus);
    MI_S32 MI_HDMIRX_UpdateEdid(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_Edid_t *pstEdid);
    MI_S32 MI_HDMIRX_LoadHdcp(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_Hdcp_t *pstHdcp);
    MI_S32 MI_HDMIRX_GetHDMILineDetStatus(MI_HDMIRX_PortId_e ePortId, MI_BOOL *pbConnected);
    MI_S32 MI_HDMIRX_SetEQAttr(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_EQAttr_t *pstEqAttr);
    MI_S32 MI_HDMIRX_GetEQAttr(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_EQAttr_t *pstEqAttr);
    MI_S32 MI_HDMIRX_SetDisplayMuteThreshold(MI_HDMIRX_PortId_e ePortId, MI_U8 u8Threshold);
    MI_S32 MI_HDMIRX_GetFd(MI_HDMIRX_PortId_e ePortId);
    MI_S32 MI_HDMIRX_CloseFd(MI_HDMIRX_PortId_e ePortId);
    //****************************************************************CEC
    MI_S32 MI_HDMIRX_CecEnable(MI_HDMIRX_PortId_e ePortId, MI_BOOL bEnable);
    MI_S32 MI_HDMIRX_CecGetPhyAddr(MI_HDMIRX_PortId_e ePortId, MI_U16 *pu16PhyAddr);
    MI_S32 MI_HDMIRX_CecSetLogicAddr(MI_HDMIRX_PortId_e ePortId, MI_U16 u16LogicAddr);
    MI_S32 MI_HDMIRX_CecClearLogicAddr(MI_HDMIRX_PortId_e ePortId);
    MI_S32 MI_HDMIRX_CecSendMessage(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_CecMessage_t *pstMsg, MI_U32 u32TimeoutMs,
                                    MI_U8 u8RetryCount);
    MI_S32 MI_HDMIRX_CecRegisterCallback(MI_HDMIRX_PortId_e ePortId, MI_HDMIRX_CEC_CALL_BACK pfnHdmiRxCecCallback,
                                         void *pCallBackArgs);
    MI_S32 MI_HDMIRX_CecUnRegisterCallback(MI_HDMIRX_PortId_e ePortId);
    //****************************************************************CEC

#ifdef __cplusplus
}
#endif

#endif ///_MI_HDMIRX_H_
