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


#pragma once
#ifndef __SSTAR_OVCLS_API_H__
#define __SSTAR_OVCLS_API_H__

#ifdef __cplusplus
extern "C" {
#endif


#define OVCLS_MAX_STRLEN          (256)
#define OVCLS_MAX_ROI_NUM         (4)

typedef struct
{
    MI_U32    class_id;
    MI_FLOAT  score;
    char string[OVCLS_MAX_STRLEN];
}OvclsOutputInfo_t;

typedef struct
{
    MI_U32 language;                                //0: Chinese, 1: English
    char ipu_firmware_path[OVCLS_MAX_STRLEN];
    char img_model_path[OVCLS_MAX_STRLEN];
    char text_model_path[OVCLS_MAX_STRLEN];
    char vocab_path[OVCLS_MAX_STRLEN];
}OvclsInitInfo_t;

typedef struct
{
    MI_U32 width;
    MI_U32 height;
    MI_IPU_ELEMENT_FORMAT format;
}OvclsInputAttr_t;

typedef struct
{
    void *p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
}OvclsAlgo_Input_t;

MI_S32 ALGO_OVCLS_CreateHandle(void** handle);
MI_S32 ALGO_OVCLS_InitHandle(void* handle, OvclsInitInfo_t* init_info);
MI_S32 ALGO_OVCLS_GetInputAttr(void* handle, OvclsInputAttr_t* input_attr);
MI_S32 ALGO_OVCLS_SetThreshold(void* handle, MI_FLOAT threshold);

MI_S32 ALGO_OVCLS_CreateImageDatabase(void* handle, int img_num, char* img_database_path, char *img_label_path);
MI_S32 ALGO_OVCLS_CreateTextDatabase(void* handle, int text_num, char* text_database_path, char *text_label_path);
MI_S32 ALGO_OVCLS_AddImageToDatabase(void* handle, const OvclsAlgo_Input_t *img_input, const char *img_str);
MI_S32 ALGO_OVCLS_AddTextToDatabase(void* handle, const char *promopt_text, int language_type);
MI_S32 ALGO_OVCLS_SaveImageDatabase(void* handle, char* img_database_path, char *img_label_path);
MI_S32 ALGO_OVCLS_SaveTextDatabase(void* handle, char* text_database_path, char *text_label_path);

MI_S32 ALGO_OVCLS_SearchTextByImage(void* handle, const OvclsAlgo_Input_t *img_input, int language_type, OvclsOutputInfo_t *results);
MI_S32 ALGO_OVCLS_SearchImageByText(void* handle, const char *promopt_text, int language_type, OvclsOutputInfo_t *results);

MI_S32 ALGO_OVCLS_DeInitHandle(void* handle);
MI_S32 ALGO_OVCLS_ReleaseHandle(void* handle);


#ifdef __cplusplus
}
#endif

#endif