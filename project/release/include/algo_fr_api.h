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
#define FR_POINT_LEN 5
#define MODEL_MAX_LENGTH (256)
#define IPU_MAX_LENGTH (256)

#define DET_INPUT_W      480
#define DET_INPUT_H      288

#define ATTR_INPUT_W     224
#define ATTR_INPUT_H     224
#define FEATURE_LEN 512

typedef enum{
    FR_MODE_FACE_DET=0x01,
    FR_MODE_FACE_RECOG=0x02,
    FR_MODE_FACE_ATTR=0x04,
    FR_MODE_FACE_EMOTION=0x08,
    FR_MODE_FACE_QUANTILY=0x10,
}ALGO_FrMode_e;

typedef struct FaceAttr_t_
{
    MI_BOOL   eye_glass;
    MI_BOOL   male;
    MI_BOOL   mask;
    MI_S32    age;
    MI_BOOL   mustache;
    MI_S32    emotion;
}FaceAttr_t;

typedef struct FrPoint_
{
    MI_FLOAT x;
    MI_FLOAT y;
}FrPoint_t;
typedef struct FaceQuanlityAngle_
{
    MI_FLOAT pitch;
    MI_FLOAT yaw;
    MI_FLOAT roll;
    MI_FLOAT quanlity;
}FaceQuanlityAngle_t;
typedef struct DetectBox_
{
    MI_FLOAT x1;
    MI_FLOAT y1;
    MI_FLOAT x2;
    MI_FLOAT y2;
    MI_FLOAT score;
    FrPoint_t landmark[FR_POINT_LEN];
    MI_S32 face_id;
    MI_S32 befiltered;
    MI_FLOAT move;
}DetectBox_t;
typedef struct AlgoFrFilterParam_
{
    MI_S32   box_min_size;
    MI_FLOAT filter_angle_ratio;
    MI_FLOAT eye_distance;
    MI_FLOAT det_thredhold;
    MI_FLOAT attr_thredhold;
    MI_FLOAT motion_sensitive;   //[0.0-1.0]
}AlgoFrFilterParam_t;

typedef struct AlgoFaceQuanlityParam_
{
    MI_FLOAT thredhold;
}AlgoFaceQuanlityParam_t;

typedef struct ParamDet_
{
    MI_S32 datatype;
}ParamDet_t;

typedef struct AlgoFrInputInfo_
{
    void* pt_tensor_data;
    MI_PHY phy_tensor_addr;//notice that this is miu bus addr,not cpu bus addr.
    MI_U32 bufsize;
    MI_S64 pts;
}AlgoFrInputInfo_t;

typedef struct InitFrParam_
{
    MI_U8 ipu_firware_bin[IPU_MAX_LENGTH];
    MI_U8 det_model_path[MODEL_MAX_LENGTH];
    MI_U8 feature_model_path[MODEL_MAX_LENGTH];
    MI_U8 cos_model_path[MODEL_MAX_LENGTH];
    MI_U8 attr_model_path[MODEL_MAX_LENGTH];
    MI_U8 emo_model_path[MODEL_MAX_LENGTH];
    MI_U8 quanlity_model_path[MODEL_MAX_LENGTH];
    MI_FLOAT det_thredhold;
    MI_FLOAT attr_thredhold;
    MI_S32   box_min_size;
    MI_FLOAT filter_angle_ratio;
    MI_FLOAT eye_distance;
    MI_U8   fr_mode;
    MI_BOOL had_create_device;
}InitFrParam_t;
MI_S32 ALGO_FR_Init(InitFrParam_t initParam);
MI_S32 ALGO_FR_CreateHandle(MI_S64* detectorId);
MI_S32 ALGO_FR_HandleResetParam(MI_S64 detectorID, AlgoFrFilterParam_t param);
MI_S32 ALGO_FR_Detect(MI_S64 detectId, AlgoFrInputInfo_t *stBufInfo, MI_S32 width, MI_S32 height,ParamDet_t* params, DetectBox_t** detectOut, MI_S32* faceCount);
MI_S32 ALGO_FR_Attr(MI_S64 detectId, AlgoFrInputInfo_t *stBufInfo, FaceAttr_t *results);
MI_S32 ALGO_FR_FaceQuanlity(MI_S64 detectId, AlgoFrInputInfo_t* stBufInfo,DetectBox_t* box, AlgoFaceQuanlityParam_t param, FaceQuanlityAngle_t* angle);
MI_S32 ALGO_FR_Align(MI_U8* imageData, MI_S32 width, MI_S32 height, MI_S32 type, DetectBox_t detectOut, MI_U8* outData);
MI_S32 ALGO_FR_FeatureExtract(MI_S64 detectId, MI_U8* imageData, MI_S16* featureOut);
MI_S32 ALGO_FR_FeatureCompare(MI_S16* feature1, MI_S16* feature2, MI_S32 length, MI_FLOAT* simility);
MI_S32 ALGO_FR_BatchFeatureCompare(MI_S64 detectId, MI_S16* feature1, MI_S16* feature2, MI_S32 batch, MI_FLOAT* simility);
MI_S32 ALGO_FR_BatchFeatureCompareV2(MI_S64 detectId, AlgoFrInputInfo_t* feature1, AlgoFrInputInfo_t* batchFeature, MI_S32 batch, MI_FLOAT* simility);
MI_S32 ALGO_FR_ReleaseHandle(MI_S64 detectorId);
MI_S32 ALGO_FR_Cleanup();
#ifdef __cplusplus
}
#endif
