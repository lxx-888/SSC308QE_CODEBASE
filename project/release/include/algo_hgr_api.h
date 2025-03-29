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
typedef void* MI_HGR_HANDLE;
typedef struct HgrCropInfo_
{
    MI_S32 x;
    MI_S32 y;
    MI_S32 width;
    MI_S32 height;
}HgrCropInfo_t;
typedef struct HgrBbox_
{
    MI_S32 x;
    MI_S32 y;
    MI_S32 width;
    MI_S32 height;
    MI_FLOAT score;
    MI_S32 label;
}HgrBbox_t;
typedef struct HgrPoint_
{
    MI_FLOAT x;
    MI_FLOAT y;
}HgrPoint_t;
typedef struct AlgoHgrResult_
{
    MI_BOOL hg_ok;
    MI_BOOL hg_L;
    MI_BOOL hg_yes;
    MI_BOOL hg_stop;
    HgrBbox_t bbox;
}AlgoHgrResult_t;
typedef struct AlgoHgrInputInfo_
{
    void* pt_tensor_data;
    MI_PHY phy_tensor_addr;//notice that this is miu bus addr,not cpu bus addr.
    MI_U32 bufsize;
    MI_U32 width;
    MI_U32 height;
    MI_S64 pts;
}AlgoHgrInputInfo_t;
typedef struct AlgoHgrInitParam_
{
    MI_U8 ipu_firmware_path[IPU_MAX_LENGTH];           // ipu_firmware.bin path
    MI_U8 det_model[MODEL_MAX_LENGTH];                       // model path
    MI_U8 land_model[MODEL_MAX_LENGTH];                       // model path
    HgrCropInfo_t disp_area;
    MI_FLOAT thredhold;
}AlgoHgrInitParam_t;
MI_S32 ALGO_HGR_CreateHandle(MI_HGR_HANDLE* handle);
MI_S32 ALGO_HGR_HandleInit(MI_HGR_HANDLE handle, AlgoHgrInitParam_t* initParam);
MI_S32 ALGO_HGR_HandPersonDetect(MI_HGR_HANDLE handle, AlgoHgrInputInfo_t* stBufInfo, HgrBbox_t** detBox,MI_S32* count);
MI_S32 ALGO_HGR_HandPoseRecognition(MI_HGR_HANDLE handle,  AlgoHgrInputInfo_t* stBufInfo, HgrBbox_t detBox, HgrPoint_t** output, MI_S32* count, AlgoHgrResult_t* hgrResult);
MI_S32 ALGO_HGR_HandleDeinit(MI_HGR_HANDLE handle);
MI_S32 ALGO_HGR_ReleaseHandle(MI_HGR_HANDLE handle);
#ifdef __cplusplus
}
#endif