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
#ifndef __MI_PSPI_H__
#define __MI_PSPI_H__

#include "mi_common_datatype.h"
#include "mi_pspi_datatype.h"

#define PSPI_MAJOR_VERSION  2
#define PSPI_SUB_VERSION    0
#define MACRO_TO_STR(macro) #macro
#define PSPI_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                                \
        {                                                                                                            \
            char* tmp =                                                                                              \
                sub_version / 100  ? "mi_pspi_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_pspi_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_pspi_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                     \
        })
#define MI_PSPI_API_VERSION PSPI_VERSION_STR(PSPI_MAJOR_VERSION, PSPI_SUB_VERSION)

#ifdef __cplusplus
extern "C"
{
#endif

    MI_S32 MI_PSPI_CreateDevice(MI_PSPI_DEV stPspiDev, MI_PSPI_Param_t* pstPspiParam);
    MI_S32 MI_PSPI_DestroyDevice(MI_PSPI_DEV stPspiDev);
    MI_S32 MI_PSPI_Transfer(MI_PSPI_DEV stPspiDev, MI_PSPI_Msg_t* pstMsg);
    MI_S32 MI_PSPI_SetDevAttr(MI_PSPI_DEV stPspiDev, MI_PSPI_Param_t* pstPspiParam);
    MI_S32 MI_PSPI_SetOutputAttr(MI_PSPI_DEV stPspiDev, MI_PSPI_OutputAttr_t* stOutputAttr);
    MI_S32 MI_PSPI_Disable(MI_PSPI_DEV stPspiDev);
    MI_S32 MI_PSPI_Enable(MI_PSPI_DEV stPspiDev);

#ifdef __cplusplus
}
#endif

#endif //__MI_PSPI_H__
