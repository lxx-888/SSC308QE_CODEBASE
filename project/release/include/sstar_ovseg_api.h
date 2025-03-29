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
#ifndef __SSTAR_OVSEG_API_H__
#define __SSTAR_OVSEG_API_H__

#ifdef __cplusplus
extern "C" {
#endif


#define OVSEG_MAX_STRLEN          (256)
#define OVSEG_MAX_DET_OBJECT      (100)



typedef struct
{
    char ipu_firmware_path[OVSEG_MAX_STRLEN];
    char det_model_path[OVSEG_MAX_STRLEN];
    char seg_model_path[OVSEG_MAX_STRLEN];
    char vocab_path[OVSEG_MAX_STRLEN];
}OvSegInitInfo_t;

typedef struct
{
    MI_U32 width;
    MI_U32 height;
    MI_IPU_ELEMENT_FORMAT format;
}OvSegInputAttr_t;


typedef struct
{
    MI_FLOAT det_threshold;
    MI_FLOAT seg_threshold;
}OvSegThreshold_t;


typedef struct
{
    MI_U32 x;
    MI_U32 y;
    MI_U32 width;
    MI_U32 height;
    MI_U32 class_id;
    MI_FLOAT score;
    MI_U64 pts;
}OvSegBox_t;


typedef struct
{
    void *p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
    MI_U64 pts;
}OvSegAlgo_Input_t;




MI_S32 ALGO_OVSEG_CreateHandle(void** handle);
MI_S32 ALGO_OVSEG_InitHandle(void* handle, OvSegInitInfo_t* init_info);
MI_S32 ALGO_OVSEG_GetInputAttr(void* handle, OvSegInputAttr_t* det_attr, OvSegInputAttr_t* seg_attr);
MI_S32 ALGO_OVSEG_SetThreshold(void* handle, OvSegThreshold_t threshold);
MI_S32 ALGO_OVSEG_RunDet(void* handle, const OvSegAlgo_Input_t *img_input, char* text, OvSegBox_t *bboxes, MI_S16* num_boxes);
MI_S32 ALGO_OVSEG_RunSeg(void* handle, const OvSegAlgo_Input_t *img_input, const OvSegBox_t *in_box, MI_U8* out_mask, MI_S16 out_mask_w, MI_S16 out_mask_h, MI_S16 out_mask_stride);
MI_S32 ALGO_OVSEG_DeInitHandle(void* handle);
MI_S32 ALGO_OVSEG_ReleaseHandle(void* handle);


#ifdef __cplusplus
}
#endif

#endif