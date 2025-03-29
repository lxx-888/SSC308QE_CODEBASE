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
typedef void* MI_HPOSE_HANDLE;
typedef struct HposeCropInfo_
{
    MI_S32 x;
    MI_S32 y;
    MI_S32 width;
    MI_S32 height;
}HposeCropInfo_t;
typedef struct HposeBbox_
{
    MI_S32 x;
    MI_S32 y;
    MI_S32 width;
    MI_S32 height;
    MI_FLOAT score;
    MI_S32 label;
    MI_S32 id;
}HposeBbox_t;
typedef struct HposePoint_
{
    MI_FLOAT x;
    MI_FLOAT y;
    MI_FLOAT score;
}HposePoint_t;
typedef struct FaceAngle_
{
    MI_FLOAT pitch;
    MI_FLOAT yaw;
    MI_FLOAT roll;
}FaceAngle_t;
typedef struct HposeHeadResult_
{
    MI_BOOL shake_head;
    MI_BOOL nod_head;
    HposeBbox_t box;
}HposeHeadResult_t;
typedef struct HposeBodyResult_
{
    MI_BOOL stand;
    MI_BOOL lie;
    MI_S32 status;
    HposeBbox_t box;
}HposeBodyResult_t;
typedef struct AlgoHposeInputInfo_
{
    void* pt_tensor_data;
    MI_PHY phy_tensor_addr;//notice that this is miu bus addr,not cpu bus addr.
    MI_U32 bufsize;
    MI_U32 width;
    MI_U32 height;
    MI_S64 pts;
}AlgoHposeInputInfo_t;
typedef struct AlgoHposeInitParam_
{
    MI_U8 ipu_firmware_path[IPU_MAX_LENGTH];           // ipu_firmware.bin path
    MI_U8 hdet_model[MODEL_MAX_LENGTH];                       // model path
    MI_U8 fdet_model[MODEL_MAX_LENGTH];
    MI_U8 angle_model[MODEL_MAX_LENGTH];
    MI_U8 pose_model[MODEL_MAX_LENGTH];
    HposeCropInfo_t disp_area;
    MI_FLOAT det_threshold;
    MI_FLOAT pitch_threshold;
    MI_FLOAT yaw_threshold;
}AlgoHposeInitParam_t;
MI_S32 ALGO_HPOSE_CreateHandle(MI_HPOSE_HANDLE* handle);
MI_S32 ALGO_HPOSE_HandleInit(MI_HPOSE_HANDLE handle, AlgoHposeInitParam_t* initParam);
MI_S32 ALGO_HPOSE_FacePersonDetect(MI_HPOSE_HANDLE handle,  AlgoHposeInputInfo_t*stBufInfo, HposeBbox_t** faceBox,MI_S32* faceCount, HposeBbox_t** bodyBox, MI_S32* bodyCount);
MI_S32 ALGO_HPOSE_HeadPoseRecognition(MI_HPOSE_HANDLE handle,  AlgoHposeInputInfo_t*stBufInfo, HposeBbox_t detBox, HposeHeadResult_t* result);
MI_S32 ALGO_HPOSE_BodyPoseRecognition(MI_HPOSE_HANDLE handle, AlgoHposeInputInfo_t* stBufInfo, HposeBbox_t detBox, HposeBodyResult_t* result);
MI_S32 ALGO_HPOSE_HandleDeinit(MI_HPOSE_HANDLE handle);
MI_S32 ALGO_HPOSE_ReleaseHandle(MI_HPOSE_HANDLE handle);
#ifdef __cplusplus
}
#endif