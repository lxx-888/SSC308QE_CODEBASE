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
#include <mi_sys.h>
#include <linux/module.h>

MI_S32 MI_SYS_GetChnOutputPortDepth(MI_U16 u16SocId, MI_SYS_ChnPort_t *pstChnPort, MI_U32 *pu32UserFrameDepth,
                                    MI_U32 *pu32BufQueueDepth);
#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_SYS_Init);
EXPORT_SYMBOL(MI_SYS_Exit);
EXPORT_SYMBOL(MI_SYS_BindChnPort);
EXPORT_SYMBOL(MI_SYS_UnBindChnPort);
EXPORT_SYMBOL(MI_SYS_GetBindbyDest);
EXPORT_SYMBOL(MI_SYS_GetVersion);
EXPORT_SYMBOL(MI_SYS_GetCurPts);
EXPORT_SYMBOL(MI_SYS_InitPtsBase);
EXPORT_SYMBOL(MI_SYS_SyncPts);
EXPORT_SYMBOL(MI_SYS_Mmap);
EXPORT_SYMBOL(MI_SYS_Munmap);
EXPORT_SYMBOL(MI_SYS_Va2Pa);
EXPORT_SYMBOL(MI_SYS_ChnInputPortGetBuf);
EXPORT_SYMBOL(MI_SYS_ChnInputPortPutBuf);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortGetBuf);
EXPORT_SYMBOL(MI_SYS_ChnOutputPortPutBuf);
EXPORT_SYMBOL(MI_SYS_SetChnOutputPortDepth);
EXPORT_SYMBOL(MI_SYS_GetChnOutputPortDepth);
EXPORT_SYMBOL(MI_SYS_GetFd);
EXPORT_SYMBOL(MI_SYS_CloseFd);
EXPORT_SYMBOL(MI_SYS_ChnPortInjectBuf);
EXPORT_SYMBOL(MI_SYS_MMA_Alloc);
EXPORT_SYMBOL(MI_SYS_MMA_Free);
EXPORT_SYMBOL(MI_SYS_FlushInvCache);
EXPORT_SYMBOL(MI_SYS_ReadUuid);
EXPORT_SYMBOL(MI_SYS_ConfigPrivateMMAPool);
EXPORT_SYMBOL(MI_SYS_MemsetPa);
EXPORT_SYMBOL(MI_SYS_MemcpyPa);
EXPORT_SYMBOL(MI_SYS_BufFillPa);
EXPORT_SYMBOL(MI_SYS_BufBlitPa);
EXPORT_SYMBOL(MI_SYS_SetChnOutputPortUserFrc);
EXPORT_SYMBOL(MI_SYS_SetChnOutputPortBufExtConf);
EXPORT_SYMBOL(MI_SYS_PrivateDevChnHeapAlloc);
EXPORT_SYMBOL(MI_SYS_PrivateDevChnHeapFree);
#endif

