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

#ifndef _LIGHT_MISC_CONTROL_H_
#define _LIGHT_MISC_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "light_misc_control_datatype.h"
#include "mi_vif_datatype.h"
#include "mi_vif.h"

    int Dev_Light_Misc_Device_GetFd(void);
    int Dev_Light_Misc_Device_CloseFd(int dev_fd);
    int Dev_Light_Misc_Device_Init(int dev_fd, int vifDevId);
    int Dev_Light_Misc_Device_DeInit(int dev_fd, int vifDevId);
    int Dev_Light_Misc_Device_Set_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t* pstAttr);
    int Dev_Light_Misc_Device_Get_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t* pstAttr);
    int Dev_Light_Misc_Device_Set_Ircut(int dev_fd, SSTAR_Switch_State_e eOpenState);
    int Dev_Light_Misc_Device_Get_LightSensor_Value(int dev_fd);
    int Dev_Light_Misc_Device_Get_TIGMode(int dev_fd);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_LIGHT_MISC_CONTROL_H_
