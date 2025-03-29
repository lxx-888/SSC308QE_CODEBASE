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
#ifndef __SSTAR_CLASSIFY_API_H__
#define __SSTAR_CLASSIFY_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NUM_OUTPUS
#define NUM_OUTPUS 3
#endif

#ifndef MAX_CLS_STRLEN
#define MAX_CLS_STRLEN 256
#endif

typedef struct
{
    MI_U32    class_id;
    MI_FLOAT  score;
    MI_U64    pts;
}OutputInfo_t;

typedef struct
{
    char ipu_firmware_path[MAX_CLS_STRLEN]; // ipu_firmware.bin path
    char model[MAX_CLS_STRLEN];             // Classify model path
    MI_FLOAT threshold;                     // confidence
}InitInfo_t;

typedef struct
{
    MI_IPU_ELEMENT_FORMAT format;
    MI_U32                width;
    MI_U32                height;
}InputAttr_t;

typedef struct
{
    void*  p_vir_addr;
    MI_PHY phy_addr;
    MI_U32 buf_size;
    MI_U64 pts;
}ALGO_Input_t;

MI_S32 ALGO_CLS_CreateHandle(void** handle);
MI_S32 ALGO_CLS_InitHandle(void* handle, InitInfo_t* initInfo);
MI_S32 ALGO_CLS_GetInputAttr(void* handle, InputAttr_t* inputInfo);
MI_S32 ALGO_CLS_SetThreshold(void* handle, MI_FLOAT threshold);
MI_S32 ALGO_CLS_Run(void* handle, const ALGO_Input_t *algoBufInfo, OutputInfo_t results[NUM_OUTPUS], MI_S32* outputCount);
MI_S32 ALGO_CLS_DeInitHandle(void* handle);
MI_S32 ALGO_CLS_ReleaseHandle(void* handle);


#ifdef __cplusplus
}
#endif

#endif