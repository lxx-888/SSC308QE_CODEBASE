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

/*
 *   mi_isp.h
 *
 *   Created on: June 27, 2018
 *       Author: Jeffrey Chou
 */

#ifndef _MI_ISP_GENERAL_H_
#define _MI_ISP_GENERAL_H_

#include "cam_os_wrapper.h"
#include "mi_isp_datatype.h"
#include "mi_common.h"
#include "mi_common_macro.h"
#include "mi_isp_cus3a_api.h"
#include "mi_isp_hw_dep_datatype.h"
#include "mi_isp.h"
#include "mi_sys.h"
#ifdef CAM_OS_LINUX_USER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#ifdef CAM_OS_RTK
#include "sys_memmap.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define MI_ISP_MSG(fmt, args...) CamOsPrintf(fmt, ##args)
#define MI_ISP_DMSG(args...) \
    do                       \
    {                        \
    } while (0)
#define MI_ISP_EMSG(fmt, args...) CamOsPrintf(fmt, ##args)
#define MI_ISP_VMSG(args...) \
    do                       \
    {                        \
    } while (0)

#ifdef CAM_OS_RTK
#define MI_ISP_CALLOC(nNum, nSize) \
    (void *)MsPA2CVA(CamOsContiguousMemAlloc(CAM_OS_ALIGN_UP(nNum *nSize, CamOsGetCacheLineSize())))
#else
#define MI_ISP_CALLOC CamOsMemCalloc
#endif

#ifdef CAM_OS_RTK
#define MI_ISP_FREE(ptr) CamOsContiguousMemRelease(CamOsMemVirtToPhys(ptr))
#else
#define MI_ISP_FREE CamOsMemRelease
#endif

    MI_S32 MI_ISP_GENERAL_SetIspApiData(MI_ISP_IQApiHeader_t *pstIspDataHeader, void *pVirData);
    MI_S32 MI_ISP_GENERAL_GetIspApiData(MI_ISP_IQApiHeader_t *pstIspDataHeader, void *pVirData);

    extern MI_S32 MI_ISP_GetIQApiData(MI_ISP_IQApiHeader_t *pstIspDataHeader, void *pVirData);
    extern MI_S32 MI_ISP_SetIQApiData(MI_ISP_IQApiHeader_t *pstIspDataHeader, void *pVirData);

#define MI_GETAPI(DevId, Channel, APIFuncID, PARAM_t, PtrData)                                   \
    MI_S32               s32Ret = MI_ISP_OK;                                                     \
    MI_ISP_IQApiHeader_t stIspApiHeader;                                                         \
    if (sizeof(PARAM_t) == 0)                                                                    \
    {                                                                                            \
        MI_ISP_EMSG("[%s][%s] APIID:%d not support \n", __FILE__, __func__, APIFuncID);          \
        return MI_ERR_ISP_NOT_SUPPORT;                                                           \
    }                                                                                            \
    stIspApiHeader.u32HeadSize = sizeof(MI_ISP_IQApiHeader_t);                                   \
    stIspApiHeader.u32DataLen  = sizeof(PARAM_t);                                                \
    stIspApiHeader.u32CtrlID   = APIFuncID;                                                      \
    stIspApiHeader.u32DevId    = DevId;                                                          \
    stIspApiHeader.u32Channel  = Channel;                                                        \
    stIspApiHeader.s32Ret      = 0;                                                              \
    /*s32Ret = stIspApiHeader.s32Ret = MI_ISP_GetIQApiData(&stIspApiHeader, PtrData);*/          \
    s32Ret = stIspApiHeader.s32Ret = MI_ISP_GENERAL_GetIspApiData(&stIspApiHeader, PtrData);     \
    MI_ISP_DMSG("[%s] - (Channel,CtrlID, DataLenght, HeadSize) = (%d,%d,%d,%d)\n", __FUNCTION__, \
                stIspApiHeader.u32Channel, stIspApiHeader.u32CtrlID, stIspApiHeader.u32DataLen,  \
                stIspApiHeader.u32HeadSize);                                                     \
    return s32Ret;

#define MI_SETAPI(DevId, Channel, APIFuncID, PARAM_t, PtrData)                                   \
    MI_S32               s32Ret = MI_ISP_OK;                                                     \
    MI_ISP_IQApiHeader_t stIspApiHeader;                                                         \
    if (sizeof(PARAM_t) == 0)                                                                    \
    {                                                                                            \
        MI_ISP_EMSG("[%s][%s] APIID:%d not support \n", __FILE__, __func__, APIFuncID);          \
        return MI_ERR_ISP_NOT_SUPPORT;                                                           \
    }                                                                                            \
    stIspApiHeader.u32HeadSize = sizeof(MI_ISP_IQApiHeader_t);                                   \
    stIspApiHeader.u32DataLen  = sizeof(PARAM_t);                                                \
    stIspApiHeader.u32CtrlID   = APIFuncID;                                                      \
    stIspApiHeader.u32DevId    = DevId;                                                          \
    stIspApiHeader.u32Channel  = Channel;                                                        \
    stIspApiHeader.s32Ret      = 0;                                                              \
    /*s32Ret = stIspApiHeader.s32Ret = MI_ISP_SetIQApiData(&stIspApiHeader, PtrData);*/          \
    s32Ret = stIspApiHeader.s32Ret = MI_ISP_GENERAL_SetIspApiData(&stIspApiHeader, PtrData);     \
    MI_ISP_DMSG("[%s] - (Channel,CtrlID, DataLenght, HeadSize) = (%d,%d,%d,%d)\n", __FUNCTION__, \
                stIspApiHeader.u32Channel, stIspApiHeader.u32CtrlID, stIspApiHeader.u32DataLen,  \
                stIspApiHeader.u32HeadSize);                                                     \
    return s32Ret;

#define MI_CALI_SETAPI(DevId, Channel, APIFuncID, PARAM_t, DataSize, PtrData)                      \
    MI_ISP_IQApiHeader_t stIspApiHeader;                                                           \
    stIspApiHeader.u32HeadSize = sizeof(MI_ISP_IQApiHeader_t);                                     \
    stIspApiHeader.u32DataLen  = DataSize;                                                         \
    stIspApiHeader.u32CtrlID   = APIFuncID;                                                        \
    stIspApiHeader.u32DevId    = DevId;                                                            \
    stIspApiHeader.u32Channel  = Channel;                                                          \
    stIspApiHeader.s32Ret      = 0;                                                                \
    s32Ret = stIspApiHeader.s32Ret = MI_ISP_GENERAL_SetIspApiData(&stIspApiHeader, PtrData);       \
    MI_ISP_DMSG("[%s] - (Channel,CtrlID, DataLenght, HeadSize) = (%d,%d,%d,%d)\n", __FUNCTION__,   \
                stIspApiHeader->u32Channel, stIspApiHeader->u32CtrlID, stIspApiHeader->u32DataLen, \
                stIspApiHeader->u32HeadSize);

#define MI_ISP_SET(APIFunc, PARAM_t)                                               \
    if (ApiLen != sizeof(PARAM_t))                                                 \
    {                                                                              \
        MI_ISP_MSG("[%s][%s] APIID:%d error param \n", __FILE__, __func__, ApiId); \
        break;                                                                     \
    }                                                                              \
    ret = APIFunc(DevId, Channel, (PARAM_t *)pApiBuf);

#define MI_ISP_GET(APIFunc, PARAM_t) \
    *ApiLen = sizeof(PARAM_t);       \
    ret     = APIFunc(DevId, Channel, (PARAM_t *)pApiBuf);

#define MI_CALL_SETAPI(APIFunc, PARAM_t)                                              \
    MI_S32   s32Ret  = MI_ISP_OK;                                                     \
    PARAM_t *stParam = NULL;                                                          \
    if (sizeof(PARAM_t) == 0)                                                         \
    {                                                                                 \
        MI_ISP_EMSG("[%s][%s] not support \n", __FILE__, __func__);                   \
        return MI_ERR_ISP_NOT_SUPPORT;                                                \
    }                                                                                 \
    stParam = MI_ISP_CALLOC(1, sizeof(PARAM_t));                                      \
    if (!stParam)                                                                     \
    {                                                                                 \
        MI_ISP_MSG("[%s][%s] alloc %d error\n", __FILE__, __func__, sizeof(PARAM_t)); \
        return MI_ISP_FAILURE;                                                        \
    }                                                                                 \
    CamOsMemcpy(stParam, param_ary[0], sizeof(PARAM_t));                              \
    s32Ret = APIFunc(DevId, Channel, stParam);                                        \
    /*MI_ISP_DMSG("[%s][%s] param_num = %d\n", __FILE__, __FUNCTION__, param_num);*/  \
    UNUSED(param_num);                                                                \
    MI_ISP_FREE(stParam);                                                             \
    return s32Ret;

#define MI_RESET_API(APIFunc, PARAM_t)                                                \
    MI_S32   s32Ret  = MI_ISP_OK;                                                     \
    PARAM_t *stParam = MI_ISP_CALLOC(1, sizeof(PARAM_t));                             \
    if (!stParam)                                                                     \
    {                                                                                 \
        MI_ISP_MSG("[%s][%s] alloc %d error\n", __FILE__, __func__, sizeof(PARAM_t)); \
        return MI_ISP_FAILURE;                                                        \
    }                                                                                 \
    CamOsMemset(stParam, 0x00, sizeof(PARAM_t));                                      \
    s32Ret = APIFunc(DevId, Channel, stParam);                                        \
    UNUSED(s32Ret);                                                                   \
    MI_ISP_DMSG("[%s][%s] param_num = %d\n", __FILE__, __FUNCTION__, param_num);      \
    MI_ISP_FREE(stParam);

#ifdef __cplusplus
} // end of extern C
#endif

#endif //_MI_ISP_GENERAL_H_
