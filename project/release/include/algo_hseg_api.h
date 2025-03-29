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

#ifdef __cplusplus
extern "C" {
#endif
#define IPU_MAX_LENGTH (256)
#define MODEL_MAX_LENGTH (256)
#define MAX_INPUT_NUM (3)
typedef void* MI_HSEG_HANDLE;
typedef enum
{
    E_ALOG_BGBLUR_MODE_BLUR = 0x0,
    E_ALGO_BGBLUR_MODE_REPLACE = 0x1
}ALGO_HsegBgbMode_e;

typedef enum
{
    E_ALOG_YUV420SP = 0x0,
    E_ALGO_YUV422_YUYV = 0x1
}ALGO_ImageDataType_e;

typedef enum
{
    E_ALGO_BGB_MASK_OP_DILATE = 0x0,
    E_ALGO_BGB_MASK_OP_NONE = 0x1,
    E_ALGO_BGB_MASK_OP_ERODE = 0x2
}ALGO_BgBlurMaskOp_e;
typedef struct ALGO_HsegBgBlurCtrl_
{
    ALGO_HsegBgbMode_e bgblur_mode;
    MI_U8 mask_thredhold;
    MI_U8 blur_level; //[0-255]
    MI_U8 scaling_stage; //[1-15]
    ALGO_BgBlurMaskOp_e maskOp;
} ALGO_HsegBgBlurCtrl_t;
typedef struct AlgoHsegInputInfo_
{
    void* pt_tensor_data[MAX_INPUT_NUM];
    MI_PHY phy_tensor_addr[MAX_INPUT_NUM];//notice that this is miu bus addr,not cpu bus addr.
    MI_U32 bufsize;
    MI_U32 width;
    MI_U32 height;
    MI_S64 pts;
    ALGO_ImageDataType_e data_type;
}AlgoHsegInputInfo_t;

typedef struct InitHsegParam_
{
    MI_U8 ipu_firware_bin[IPU_MAX_LENGTH];
    MI_U8 seg_model_path[MODEL_MAX_LENGTH];
}InitHsegParam_t;

MI_S32 ALGO_HSEG_CreateHandle(MI_HSEG_HANDLE* handle);
MI_S32 ALGO_HSEG_Init(MI_HSEG_HANDLE handle, InitHsegParam_t initParam);
MI_S32 ALGO_HSEG_SendInput(MI_HSEG_HANDLE handle, AlgoHsegInputInfo_t* stModelBufInfo);
MI_S32 ALGO_HSEG_SegmentAndBlurBackgroud(MI_HSEG_HANDLE handle, AlgoHsegInputInfo_t* stSrcOri, AlgoHsegInputInfo_t* stSrcRepBg, AlgoHsegInputInfo_t* stDst, ALGO_HsegBgBlurCtrl_t ctrl,MI_BOOL* s32ResultFlag);
MI_S32 ALGO_HSEG_DeInit(MI_HSEG_HANDLE handle);
MI_S32 ALGO_HSEG_ReleaseHandle(MI_HSEG_HANDLE handle);
#ifdef __cplusplus
}
#endif
