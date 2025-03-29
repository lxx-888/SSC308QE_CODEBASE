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

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "st_common_dla_det.h"
#include "st_common.h"

MI_S32 ST_Common_DET_Init(void** pHandle, DetectionInfo_t* pstInit_info)
{
    ST_CHECK_POINTER(pstInit_info);
    STCHECKRESULT(ALGO_DET_CreateHandle(pHandle));
    STCHECKRESULT(ALGO_DET_InitHandle(*pHandle, pstInit_info));
    return MI_SUCCESS;
}

MI_S32 ST_Common_DET_DeInit(void** pHandle)
{
    ST_CHECK_POINTER(pHandle);
    STCHECKRESULT(ALGO_DET_DeinitHandle(*pHandle));
    STCHECKRESULT(ALGO_DET_ReleaseHandle(*pHandle));
    return MI_SUCCESS;
}

MI_S32 ST_Common_DET_SetDefaultParam(void** pHandle)
{
    ST_CHECK_POINTER(pHandle);

    MI_S32 s32Tk_type = 0;
    MI_S32 s32Md_type = 0;
    bool   bStable    = true;

    STCHECKRESULT(ALGO_DET_SetTracker(*pHandle, s32Tk_type, s32Md_type));
    STCHECKRESULT(ALGO_DET_SetStableBox(*pHandle, bStable));

    return MI_SUCCESS;
}

MI_S32 ST_Common_DET_GetModelAttr(void** pHandle, InputAttr_t* pstInput_attr)
{
    ST_CHECK_POINTER(pHandle);
    ST_CHECK_POINTER(pstInput_attr);

    STCHECKRESULT(ALGO_DET_GetInputAttr(*pHandle, pstInput_attr));
    return MI_SUCCESS;
}

MI_S32 ST_Common_DET_GetTargetData(void** pHandle, MI_SYS_BufInfo_t* pstScaledBufInfo, Box_t astBboxes[MAX_DET_OBJECT],
                                   MI_S32* num_bboxes)
{
    ST_CHECK_POINTER(pHandle);
    ST_CHECK_POINTER(pstScaledBufInfo);
    ST_CHECK_POINTER(astBboxes);
    ST_CHECK_POINTER(num_bboxes);

    MI_S32       s32Ret = 0;
    ALGO_Input_t stDetInputBufInfo;
    stDetInputBufInfo.buf_size   = pstScaledBufInfo->stFrameData.u32BufSize;
    stDetInputBufInfo.p_vir_addr = pstScaledBufInfo->stFrameData.pVirAddr[0];
    stDetInputBufInfo.phy_addr   = pstScaledBufInfo->stFrameData.phyAddr[0];

    CHECK_DLA_RESULT(ALGO_DET_Run(*pHandle, &stDetInputBufInfo, astBboxes, num_bboxes), s32Ret, EXIT);

EXIT:
    return s32Ret;
}