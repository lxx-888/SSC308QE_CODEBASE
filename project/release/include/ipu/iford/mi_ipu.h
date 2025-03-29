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

#ifndef _MI_IPU_H_
#define _MI_IPU_H_

#include "mi_ipu_datatype.h"


#define IPU_MAJOR_VERSION 1
#define IPU_SUB_VERSION 0
#define MACRO_TO_STR(macro) #macro
#define IPU_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_ipu_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_ipu_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_ipu_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_IPU_API_VERSION IPU_VERSION_STR(IPU_MAJOR_VERSION,IPU_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

MI_S32 MI_IPU_CreateDevice(MI_IPU_DevAttr_t *pstIPUDevAttr, SerializedReadFunc pReadFunc, char *pReadCtx, MI_U32 FWSize);
MI_S32 MI_IPU_DestroyDevice(void);
MI_S32 MI_IPU_DestroyDeviceExt(MI_IPU_DevAttr_t *pstIPUDevAttr);
MI_S32 MI_IPU_CreateCHN(MI_IPU_CHN *ptChnId, MI_IPUChnAttr_t *pstChnAttr, SerializedReadFunc pReadFunc, char *pReadCtx);
MI_S32 MI_IPU_DestroyCHN(MI_IPU_CHN u32ChnId);
MI_S32 MI_IPU_GetInOutTensorDesc(MI_IPU_CHN u32ChnId, MI_IPU_SubNet_InputOutputDesc_t *pstDesc);
MI_S32 MI_IPU_GetInputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_PutInputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_GetOutputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_PutOutputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_Invoke(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector, MI_IPU_TensorVector_t *pstOutputTensorVector);
MI_S32 MI_IPU_GetInputTensors2(MI_IPU_CHN u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam);
MI_S32 MI_IPU_PutInputTensors2(MI_IPU_CHN u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam);
MI_S32 MI_IPU_GetOutputTensors2(MI_IPU_CHN u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam);
MI_S32 MI_IPU_PutOutputTensors2(MI_IPU_CHN u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam);
MI_S32 MI_IPU_Invoke2(MI_IPU_CHN u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam, MI_IPU_RuntimeInfo_t *pstRuntimeInfo);
MI_S32 MI_IPU_Invoke2Custom(MI_IPU_CHN u32ChnId, MI_IPU_BatchInvokeParam_t *pstInvokeParam, MI_IPU_RuntimeInfo_t *pstRuntimeInfo);
MI_S32 MI_IPU_GetOfflineModeStaticInfo(SerializedReadFunc pReadFunc, char *pReadCtx, MI_IPU_OfflineModelStaticInfo_t *pStaticInfo);
MI_S32 MI_IPU_CancelInvoke(MI_U32 u32ThreadId, MI_IPU_CHN u32ChnId);

/*
    please make sure to do MI_SYS_FlushInvCache to flush model cache to DRAM before call MI_IPU_CreateCHNWithUserMem
*/
MI_S32 MI_IPU_CreateCHNWithUserMem(MI_IPU_CHN *ptChnId, MI_IPUChnAttr_t *pstChnAttr, MI_PHY u64ModelPA);
MI_S32 MI_IPU_DupCHN(MI_IPU_CHN u32ChnId);


#ifdef __cplusplus
}
#endif

#endif
