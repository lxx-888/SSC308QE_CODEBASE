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

#ifndef _SS_UVC_H_
#define _SS_UVC_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif

#include "ss_uvc_datatype.h"
#define CT_SCANNING_MODE (1)
extern int pthread_setname_np(pthread_t __target_thread,
        const char *__name);

int32_t SS_UVC_Init(const char *uvc_name, void *pdata, SS_UVC_Handle_h*);
int32_t SS_UVC_Uninit(SS_UVC_Handle_h);
int32_t SS_UVC_CreateDev(SS_UVC_Handle_h, const SS_UVC_ChnAttr_t*);
int32_t SS_UVC_DestroyDev(SS_UVC_Handle_h);
int32_t SS_UVC_StartDev(SS_UVC_Handle_h);
int32_t SS_UVC_StopDev(SS_UVC_Handle_h);
int SS_UVC_SetTraceLevel(int level);
void save_file(void *buf,uint32_t length,char type);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif //_SS_UVC_H_
