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

#ifndef _MI_LDC_DATATYPE_H_
#define _MI_LDC_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LDC_MAXTRIX_NUM 9
#define LDC_PMFCOEF_NUM 9
#define LDC_MAX_REGION_NUM 12
#define LDC_MAX_CAMERA_NUM 10
#define LDC_MAX_PAIR_NUM   (LDC_MAX_CAMERA_NUM - 1)
#define MI_LDC_MAX_PLANE_NUM        MI_SYS_MAX_SUB_PLANE_CNT
#define MI_LDC_MAX_MAP_CNT          8
#define MI_LDC_MAX_PRIVATE_BUF_CNT  8
#define MI_LDC_IMU_AXIS_NUM 3

    typedef MI_U32 MI_LDC_DEV;
    typedef MI_U32 MI_LDC_CHN;

    typedef enum
    {
        MI_LDC_WORKMODE_LDC      = 0x01,
        MI_LDC_WORKMODE_LUT      = 0x02,
        MI_LDC_WORKMODE_DIS      = 0x04,
        MI_LDC_WORKMODE_PMF      = 0x08,
        MI_LDC_WORKMODE_STITCH   = 0x10,
        MI_LDC_WORKMODE_NIR      = 0x20,
        MI_LDC_WORKMODE_DPU      = 0x40,
        MI_LDC_WORKMODE_LDC_HORIZONTAL = 0x80,
        MI_LDC_WORKMODE_DIS_LDC        = (MI_LDC_WORKMODE_DIS | MI_LDC_WORKMODE_LDC),
        MI_LDC_WORKMODE_BUTT
    } MI_LDC_WorkMode_e;

    typedef struct MI_LDC_DevAttr_s
    {
        MI_U32 u32Reserved;
    } MI_LDC_DevAttr_t;

    typedef struct MI_LDC_ChnAttr_s
    {
        MI_LDC_WorkMode_e eWorkMode;
        MI_SYS_BindType_e eInputBindType;
    } MI_LDC_ChnAttr_t;

    typedef enum
    {
        MI_LDC_DESKTOP_MOUNT = 0x01,
        MI_LDC_CEILING_MOUNT = 0x02,
        MI_LDC_WALL_MOUNT    = 0x03,
        MI_LDC_MOUNT_BUTT
    } MI_LDC_MountMode_e;

    typedef struct MI_LDC_SensorCalibInfo_s
    {
        MI_S32 s32CenterXOffset;
        MI_S32 s32CenterYOffset;
        MI_S32 s32FisheyeRadius;

        void*  pCalibPolyBinAddr;
        MI_U32 u32CalibPolyBinSize;
    } MI_LDC_SensorCalibInfo_t;

    typedef enum
    {
        MI_LDC_REGION_360_PANORAMA      = 0x01,
        MI_LDC_REGION_180_PANORAMA      = 0x02,
        MI_LDC_REGION_NORMAL            = 0x03,
        MI_LDC_REGION_MAP2BIN           = 0x04,
        MI_LDC_REGION_NO_TRANSFORMATION = 0x05,
        MI_LDC_REGION_DOORBELL          = 0x06,
        MI_LDC_REGION_BUTT
    } MI_LDC_RegionMode_e;

    typedef enum
    {
        MI_LDC_REGION_CROP_NONE    = 0x00,
        MI_LDC_REGION_CROP_FILLING = 0x01,
        MI_LDC_REGION_CROP_STRETCH = 0x02,
        MI_LDC_REGION_CROP_BUTT
    } MI_LDC_RegionCropMode_e;

    typedef struct MI_LDC_RegionPara_s
    {
        MI_LDC_RegionCropMode_e eCropMode;
        MI_S32 s32Pan;
        MI_S32 s32Tilt;
        MI_S32 s32ZoomH;
        MI_S32 s32ZoomV;
        MI_S32 s32InRadius;
        MI_S32 s32OutRadius;
        MI_S32 s32FocalRatio; /* s32FocalRatio: value * 10000 */
        MI_S32 s32DistortionRatio;
        MI_S32 s32OutRotate;
        MI_S32 s32Rotate;
    } MI_LDC_RegionPara_t;

    typedef struct MI_LDC_DispMapInfo_s
    {
        MI_U32 u32Grid;

        void *pXmapAddr;
        void *pYmapAddr;
        MI_U32 u32XmapSize;
        MI_U32 u32YmapSize;
        MI_U32 u32XOffset;
        MI_U32 u32YOffset;
    } MI_LDC_DispMapInfo_t;

    typedef struct MI_LDC_RegionDoorbell_s
    {
        MI_S16 s16Fx;
        MI_S16 s16Fy;
        MI_S16 s16A;
        MI_S16 s16B;
        MI_S16 s16Scale;
    } MI_LDC_RegionDoorbell_t;

    typedef struct MI_LDC_RegionAttr_s
    {
        MI_LDC_RegionMode_e eRegionMode;
        MI_U8               u8Map2RegionId;
        union
        {
            MI_LDC_RegionPara_t     stRegionPara;
            MI_LDC_DispMapInfo_t    stRegionMapInfo;
            MI_LDC_RegionDoorbell_t stRegionDoorbellPara;
        };
        MI_SYS_WindowRect_t stOutRect;
    } MI_LDC_RegionAttr_t;

    typedef struct MI_LDC_ChnLDCAttr_s
    {
        MI_BOOL bBgColor;
        MI_U32 u32BgColor; /* the backgroup color, format rgb888 */

        MI_LDC_MountMode_e eMountMode;
        MI_LDC_SensorCalibInfo_t stCalibInfo;
        MI_U32 u32RegionNum;
        MI_LDC_RegionAttr_t stRegionAttr[LDC_MAX_REGION_NUM];
    } MI_LDC_ChnLDCAttr_t;

    typedef enum
    {
        MI_LDC_DIS_NONE   = 0x00,
        MI_LDC_DIS_GME_6DOF = 0x01,
        MI_LDC_DIS_GME_8DOF = 0x02,
        MI_LDC_DIS_GYRO   = 0x03,
        MI_LDC_DIS_CUST = 0x04,
        MI_LDC_DIS_BUTT,
    } MI_LDC_DISMode_e;

    typedef enum
    {
        MI_LDC_DIS_FIX_SCENE  = 0x00,
        MI_LDC_DIS_MOVE_SCENE = 0x01,
        MI_LDC_DIS_SCENE_BUTT,
    } MI_LDC_DISSceneType_e;

    typedef enum
    {
        MI_LDC_DIS_MOTION_LEVEL0 = 0x00,
        MI_LDC_DIS_MOTION_LEVEL1 = 0x01,
        MI_LDC_DIS_MOTION_BUTT,
    } MI_LDC_DISMotionLevel_e;

    typedef struct MI_LDC_IsMatrixInPararm_s
    {
        MI_LDC_DEV devId;
        MI_LDC_CHN chnId;
        MI_U64 u64FramePts;
    }MI_LDC_IsMatrixInParam_t;

    typedef struct MI_LDC_IsMatrixOutParam_s
    {
        MI_S32  as32Matrix[LDC_MAXTRIX_NUM];
    }MI_LDC_IsMatrixOutParam_t;

    typedef MI_S32 (*MI_LDC_CalIsMatrixCb_t )(const MI_LDC_IsMatrixInParam_t * const pstInParam, MI_LDC_IsMatrixOutParam_t * const pstOutParam);

    typedef struct MI_LDC_ImgDirection_s
    {
        MI_SYS_Rotate_e eRotate;
        MI_BOOL bMirror;
        MI_BOOL bFlip;
    }MI_LDC_ImgDirection_t;

    typedef struct MI_LDC_ChnDISAttr_s
    {
        MI_LDC_DISMode_e        eMode;
        MI_LDC_DISSceneType_e   eSceneType;
        MI_LDC_DISMotionLevel_e eMotionLevel;

        MI_S32  as32RotationMatrix[LDC_MAXTRIX_NUM];
        MI_U32  u32UserSliceNum;
        MI_U32  u32FocalLengthX; /* u32FocalLengthX: value * 10000 */
        MI_U32  u32FocalLengthY; /* u32FocalLengthY: value * 10000 */

        MI_U8    u8CropRatio;    /* range:[50, 98], range*0.01 */
        MI_BOOL  bBypass;

        MI_LDC_CalIsMatrixCb_t  pCalIsMatrixCb;
        MI_LDC_ImgDirection_t stSrcImgDirection;
    } MI_LDC_ChnDISAttr_t;

    typedef struct MI_LDC_ChnPMFAttr_s
    {
        MI_S64 as64PMFCoef[LDC_PMFCOEF_NUM];
    } MI_LDC_ChnPMFAttr_t;

    typedef struct MI_LDC_CalibConfig_s
    {
        void *pCalibCfgAddr;
        MI_U32 u32CalibCfgSize;
    } MI_LDC_CalibConfig_t;

    typedef struct MI_LDC_ChnLDCHorAttr_s
    {
        MI_BOOL bCropEn;
        MI_S32 s32DistortionRatio;

        MI_LDC_CalibConfig_t stCalCfg;
    } MI_LDC_ChnLDCHorAttr_t;

    typedef enum
    {
        MI_LDC_PROJECTION_RECTILINEAR   = 0x00,
        MI_LDC_PROJECTION_CYLINDRICAL   = 0x01,
        MI_LDC_PROJECTION_SPHERICAL     = 0x02,
        MI_LDC_PROJECTION_BUTT
    } MI_LDC_ProjectionMode_e;

    typedef struct MI_LDC_StitchDisparity_s
    {
        void *pMapData;
        MI_U32 u32MapSize;
    } MI_LDC_StitchDisparity_t;

    typedef struct MI_LDC_ChnStitchAttr_s
    {
        MI_LDC_ProjectionMode_e eProjType;
        MI_S32 s32Distance;
        MI_LDC_CalibConfig_t stCalCfg;
        MI_U8 u8PairNum;
        MI_LDC_StitchDisparity_t astDisparity[LDC_MAX_PAIR_NUM];
    } MI_LDC_ChnStitchAttr_t;

    typedef struct MI_LDC_PrivateBuff_s
    {
        void*  pVirAddr;
        MI_U32 u32BufSize;
    } MI_LDC_PrivateBuff_t;

    typedef struct MI_LDC_DispMapInfoEx_s
    {
        MI_LDC_DispMapInfo_t stMap;
        MI_SYS_WindowRect_t stInRect;
        MI_SYS_WindowRect_t stOutRect;
    } MI_LDC_DispMapInfoEx_t;

    typedef struct MI_LDC_ChnCustomAttr_s
    {
        MI_U8 u8PlaneNum;
        MI_U8 u8DispMapCntPerPlane[MI_LDC_MAX_PLANE_NUM];
        MI_LDC_DispMapInfoEx_t astDispMap[MI_LDC_MAX_PLANE_NUM][MI_LDC_MAX_MAP_CNT];
        MI_U8 u8PrivateBufCnt;
        MI_LDC_PrivateBuff_t astPrivateBuf[MI_LDC_MAX_PRIVATE_BUF_CNT];
    } MI_LDC_ChnCustomAttr_t;

    typedef struct MI_LDC_ChnNIRAttr_s
    {
        MI_S32 s32Distance;
        MI_LDC_CalibConfig_t stCalCfg;
    } MI_LDC_ChnNIRAttr_t;

    typedef struct MI_LDC_ChnDPUAttr_s
    {
        MI_S32 s32Distance;
        MI_LDC_CalibConfig_t stCalCfg;
    } MI_LDC_ChnDPUAttr_t;

    typedef struct MI_LDC_DirectBuf_s
    {
        MI_SYS_PixelFormat_e ePixelFormat;
        MI_U32               u32Width;
        MI_U32               u32Height;
        MI_U32               u32Stride[2];
        MI_PHY               phyAddr[2];
    } MI_LDC_DirectBuf_t;

    typedef struct MI_LDC_LutTaskAttr_s
    {
        struct
        {
            MI_LDC_DirectBuf_t stTableX;
            MI_LDC_DirectBuf_t stTableY;
            MI_LDC_DirectBuf_t stTableWeight;
        } stSrcBuf;

        MI_LDC_DirectBuf_t stDstBuf;
    } MI_LDC_LutTaskAttr_t;

    typedef enum
    {
        E_MI_LDC_ERR_DEV_CREATED = MI_LDC_INITIAL_ERROR_CODE, // dev has been created
        E_MI_LDC_ERR_DEV_NOT_CREATE,                          // dev not be created
        E_MI_LDC_ERR_DEV_NOT_DESTROY,                         // dev not be destroyed
        E_MI_LDC_ERR_CHN_CREATED,                             // chn has been created
        E_MI_LDC_ERR_CHN_NOT_CREATE,                          // chn not be created
        E_MI_LDC_ERR_CHN_NOT_STOP,                            // chn is still working
        E_MI_LDC_ERR_CHN_NOT_DESTROY,                         // chn not be destroyed
        E_MI_LDC_ERR_PORT_NOT_UNBIND,                         // port not unbind
    } MI_LDC_ErrCode_e;

    typedef struct MI_LDC_InputPortAttr_s
    {
        MI_U16               u16Width;
        MI_U16               u16Height;
    } MI_LDC_InputPortAttr_t;

    typedef struct MI_LDC_OutputPortAttr_s
    {
        MI_U16               u16Width;
        MI_U16               u16Height;
        MI_SYS_PixelFormat_e ePixelFmt;
    } MI_LDC_OutputPortAttr_t;

    typedef struct MI_LDC_Point_s
    {
        MI_S16 s16X;
        MI_S16 s16Y;
    } MI_LDC_Point_t;

    typedef enum
    {
        E_MI_LDC_DISPMAP_SRC = 0,
        E_MI_LDC_DISPMAP_DST,
        E_MI_LDC_DISPMAP_BUTT,
    } MI_LDC_DispMapType_e;

    typedef struct MI_LDC_DispMapConf_s
    {
        MI_U32 u32RegionIdx;
        MI_LDC_DispMapType_e eMapType;
    } MI_LDC_DispMapConf_t;

    typedef struct MI_LDC_DispMapSize_s
    {
        MI_U32 u32Width;
        MI_U32 u32Height;
    } MI_LDC_DispMapSize_t;

    typedef struct MI_LDC_BaseDrift_s
    {
        MI_BOOL bValid;
        MI_S32  as32BaseDrift[MI_LDC_IMU_AXIS_NUM];
    } MI_LDC_BaseDrift_t;

    typedef struct MI_LDC_Drift_s
    {
        MI_LDC_BaseDrift_t stBaseDrift;
    } MI_LDC_Drift_t;

    typedef struct MI_LDC_IMUDrift_s
    {
        MI_LDC_Drift_t stGyroDrift;
        MI_LDC_Drift_t stAccDrift;
    } MI_LDC_IMUDrift_t;

    typedef enum
    {
        E_MI_LDC_IMU_PART_GYRO = 0x01,
        E_MI_LDC_IMU_PART_ACC  = 0x02,
        E_MI_LDC_IMU_PART_BUTT = 0x0
    } MI_LDC_IMUPart_e;

#define MI_LDC_OK                   MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_INFO, MI_SUCCESS)
#define MI_ERR_LDC_ILLEGAL_PARAM    MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_LDC_NULL_PTR         MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
#define MI_ERR_LDC_NOMEM            MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM)
#define MI_ERR_LDC_BUSY             MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)
#define MI_ERR_LDC_FAIL             MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_FAILED)
#define MI_ERR_LDC_INVALID_DEVID    MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_LDC_INVALID_CHNID    MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID)
#define MI_ERR_LDC_NOT_SUPPORT      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_LDC_MOD_INITED       MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INITED)
#define MI_ERR_LDC_MOD_NOT_INIT     MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT)
#define MI_ERR_LDC_DEV_CREATED      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_CREATED)
#define MI_ERR_LDC_DEV_NOT_CREATE   MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_NOT_CREATE)
#define MI_ERR_LDC_DEV_NOT_DESTROY  MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_NOT_DESTROY)
#define MI_ERR_LDC_CHN_CREATED      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_CREATED)
#define MI_ERR_LDC_CHN_NOT_CREATE   MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_CREATE)
#define MI_ERR_LDC_CHN_NOT_STOP     MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_STOP)
#define MI_ERR_LDC_CHN_NOT_DESTROY  MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_DESTROY)
#define MI_ERR_LDC_PORT_NOT_DISABLE MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_DISABLE)
#define MI_ERR_LDC_PORT_NOT_UNBIND  MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_PORT_NOT_UNBIND)

#ifdef __cplusplus
}
#endif

#endif ///_MI_LDC_DATATYPE_H_
