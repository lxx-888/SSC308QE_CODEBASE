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

#ifndef _MI_DSP_H_
#define _MI_DSP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "mi_dsp_datatype.h"

#define DSP_MAJOR_VERSION   3
#define DSP_SUB_VERSION     2
#define MACRO_TO_STR(macro) #macro
#define DSP_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            char *tmp =                                                                                             \
                sub_version / 100  ? "mi_dsp_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_dsp_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_dsp_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                    \
        })
#define MI_DSP_API_VERSION DSP_VERSION_STR(DSP_MAJOR_VERSION, DSP_SUB_VERSION)

    //------------------------------------------------------------------------------
    /// @brief create a DSP channel.
    /// @param[out] pDivpChn: DSP channel ID.
    /// @param[in] pstAttr: Attribute of DSP channel.
    /// @return MI_SUCCESS: succeed in creating a channel.
    ///             MI_DSP_ERR_INVALID_PARAM: invalid input patamter.
    ///             MI_DSP_ERR_NULL_PTR: NULL poiter error.
    ///             MI_DSP_ERR_FAILED: Fail to create a channel.
    ///             MI_DSP_ERR_NO_RESOUCE: there is no resource.
    //------------------------------------------------------------------------------

    MI_S32 MI_DSP_CreateDev(MI_DSP_DEV_e eDspDev, MI_DSP_DevAttr_t *pstDspDevAttr, CUST_READ_FUNC pReadFunc, char *pCtx,
                            MI_U32 fwSize);

    MI_S32 MI_DSP_DestroyDev(MI_DSP_DEV_e eDspDev);

    MI_S32 MI_DSP_Invoke(MI_DSP_DEV_e eDspDev, MI_DSP_BufVector_t *pstInputBuf, MI_DSP_BufVector_t *pstOutputBuf,
                         MI_DSP_PARAM_t *pstParam, MI_DSP_PRI_e ePri, MI_U16 u16TimeOutMs);
    // NONBLOCK invoke fence=[1,MAX_INT]
    MI_S32 MI_DSP_InvokeAsync(MI_DSP_DEV_e eDspDev, MI_DSP_BufVector_t *pstInputBuf,          // NOLINT
                              MI_DSP_BufVector_t *pstOutputBuf,                               // NOLINT
                              MI_DSP_PARAM_t *pstParam, MI_DSP_PRI_e ePri, MI_U32 *u32Fence); // NOLINT

    MI_S32 MI_DSP_WaitDone(MI_DSP_DEV_e eDspDev, MI_DSP_PRI_e ePri, MI_U32 u32Fence, MI_U32 u32TimeOutMs); // NOLINT

#ifdef __cplusplus
}
#endif

#endif
