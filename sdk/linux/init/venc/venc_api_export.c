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
#include <linux/module.h>
#include "mi_venc.h"
#include "mi_venc_datatype.h"

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_VENC_CreateDev);
EXPORT_SYMBOL(MI_VENC_DestroyDev);
EXPORT_SYMBOL(MI_VENC_CreateChn);
EXPORT_SYMBOL(MI_VENC_DestroyChn);
EXPORT_SYMBOL(MI_VENC_ResetChn);
EXPORT_SYMBOL(MI_VENC_StartRecvPic);
EXPORT_SYMBOL(MI_VENC_StartRecvPicEx);
EXPORT_SYMBOL(MI_VENC_StopRecvPic);
EXPORT_SYMBOL(MI_VENC_Query);
EXPORT_SYMBOL(MI_VENC_SetChnAttr);
EXPORT_SYMBOL(MI_VENC_GetChnAttr);
EXPORT_SYMBOL(MI_VENC_GetStream);
EXPORT_SYMBOL(MI_VENC_ReleaseStream);
EXPORT_SYMBOL(MI_VENC_InsertUserData);
EXPORT_SYMBOL(MI_VENC_SetMaxStreamCnt);
EXPORT_SYMBOL(MI_VENC_GetMaxStreamCnt);
EXPORT_SYMBOL(MI_VENC_RequestIdr);
EXPORT_SYMBOL(MI_VENC_GetFd);
EXPORT_SYMBOL(MI_VENC_CloseFd);
EXPORT_SYMBOL(MI_VENC_SetRoiCfg);
EXPORT_SYMBOL(MI_VENC_GetRoiCfg);
EXPORT_SYMBOL(MI_VENC_SetRoiBgFrameRate);
EXPORT_SYMBOL(MI_VENC_GetRoiBgFrameRate);
EXPORT_SYMBOL(MI_VENC_SetH264SliceSplit);
EXPORT_SYMBOL(MI_VENC_GetH264SliceSplit);
EXPORT_SYMBOL(MI_VENC_SetH264Trans);
EXPORT_SYMBOL(MI_VENC_GetH264Trans);
EXPORT_SYMBOL(MI_VENC_SetH264Entropy);
EXPORT_SYMBOL(MI_VENC_GetH264Entropy);
EXPORT_SYMBOL(MI_VENC_SetH264Dblk);
EXPORT_SYMBOL(MI_VENC_GetH264Dblk);
EXPORT_SYMBOL(MI_VENC_SetH264Vui);
EXPORT_SYMBOL(MI_VENC_GetH264Vui);
EXPORT_SYMBOL(MI_VENC_SetH265SliceSplit);
EXPORT_SYMBOL(MI_VENC_GetH265SliceSplit);
EXPORT_SYMBOL(MI_VENC_SetH265Trans);
EXPORT_SYMBOL(MI_VENC_GetH265Trans);
EXPORT_SYMBOL(MI_VENC_SetH265Dblk);
EXPORT_SYMBOL(MI_VENC_GetH265Dblk);
EXPORT_SYMBOL(MI_VENC_SetH265Vui);
EXPORT_SYMBOL(MI_VENC_GetH265Vui);
EXPORT_SYMBOL(MI_VENC_SetJpegParam);
EXPORT_SYMBOL(MI_VENC_GetJpegParam);
EXPORT_SYMBOL(MI_VENC_SetRcParam);
EXPORT_SYMBOL(MI_VENC_GetRcParam);
EXPORT_SYMBOL(MI_VENC_SetRefParam);
EXPORT_SYMBOL(MI_VENC_GetRefParam);
EXPORT_SYMBOL(MI_VENC_SetCrop);
EXPORT_SYMBOL(MI_VENC_GetCrop);
EXPORT_SYMBOL(MI_VENC_SetFrameLostStrategy);
EXPORT_SYMBOL(MI_VENC_GetFrameLostStrategy);
EXPORT_SYMBOL(MI_VENC_SetSuperFrameCfg);
EXPORT_SYMBOL(MI_VENC_GetSuperFrameCfg);
EXPORT_SYMBOL(MI_VENC_SetRcPriority);
EXPORT_SYMBOL(MI_VENC_GetRcPriority);
EXPORT_SYMBOL(MI_VENC_SetInputSourceConfig);
EXPORT_SYMBOL(MI_VENC_AllocCustomMap);
EXPORT_SYMBOL(MI_VENC_ApplyCustomMap);
EXPORT_SYMBOL(MI_VENC_GetLastHistoStaticInfo);
EXPORT_SYMBOL(MI_VENC_ReleaseHistoStaticInfo);
EXPORT_SYMBOL(MI_VENC_SetAdvCustRcAttr);
EXPORT_SYMBOL(MI_VENC_SetSmartDetInfo);
EXPORT_SYMBOL(MI_VENC_SetIntraRefresh);
EXPORT_SYMBOL(MI_VENC_GetIntraRefresh);
#endif

