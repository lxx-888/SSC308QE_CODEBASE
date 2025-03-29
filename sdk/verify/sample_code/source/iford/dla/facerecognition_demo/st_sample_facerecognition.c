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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "mi_sys.h"
#include "mi_isp_cus3a_api.h"

#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_rgn.h"
#include "st_common_venc.h"
#include "st_common_dla_fr.h"

#include "st_common_rtsp_video.h"

#define VYU444_RED 0xff4c55
#define VYU444_GREEN 0x009500
#define VYU444_BLUE 0x6b1dff
#define VYU444_YELLOW 0x95e200
#define VYU444_WHITE 0x80ff80

#define MAX_FRAME_HANDLE (32)

#define NV12_480X288_SIZE (207360)
#define NV12_1920X1080_SIZE (3110400)
#define ARGB8888_112X112_SIZE (50176)

typedef struct ST_FR_InputParam_s
{
    MI_U8  u8SensorIndex;
    MI_U8  u8CmdIndex;
    MI_S32 u32SecTimeout;
    MI_U8  au8DetectModelPath[MODEL_MAX_LENGTH];
    MI_U8  au8FeatureModelPath[MODEL_MAX_LENGTH];
    MI_U8  au8CosModelPath[MODEL_MAX_LENGTH];
    MI_U8  au8FaceFeatureDataPath[MODEL_MAX_LENGTH];
    MI_U8  au8FaceCropDataPath[MODEL_MAX_LENGTH];
} ST_FR_InputParam_t;

typedef struct ST_FR_FaceRecognizeThread_Attr_s
{
    pthread_t GetFaceDatathread;
    MI_BOOL   bThreadExit;
    MI_S32    s32ThreadRetval;
} ST_FR_FaceRecognizeThread_Attr_t;

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

static ST_FR_InputParam_t               gstFRInputParm;
static ST_SensorSize_t                  gstSensorSize;
static ST_FR_FaceRecognizeThread_Attr_t gstFaceRecognizeThread_Attr;
static MI_S64                           ghFr;

static MI_RGN_ChnPortParam_t gstRgnChnPortParam;
MI_RGN_HANDLE                ghFrame[MAX_FRAME_HANDLE] = {0};

void *ST_RegisterThread(void *args)
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stSourceBufInfo;
    MI_SYS_BufInfo_t  stScaledBufInfo;
    MI_SYS_BUF_HANDLE hSourceHandle;
    MI_SYS_BUF_HANDLE hScaledHandle;
    MI_U32            u32SclSourcePortId = 0;
    MI_U32            u32SclScaledPortId = 1;

    MI_U16           u16SocId = 0;
    MI_RGN_ChnPort_t stRgnChnPort;

    MI_S16 *     ps16FaceFeatureData = NULL;
    MI_U8 *      pu8FaceCropData     = NULL;
    DetectBox_t *pstDetectout        = NULL;
    MI_S32       s32FaceNum          = 0;
    ParamDet_t   stParams;
    MI_S32       s32FormatType;

    MI_U32         s32BaseSecTime = 0;
    struct timeval stCurStamp;

    gettimeofday(&stCurStamp, NULL);
    s32BaseSecTime = stCurStamp.tv_sec;

    memset(&stChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId   = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = 0;
    stRgnChnPort.s32ChnId  = 0;
    stRgnChnPort.s32PortId = 0;

    // 0: Detect the largest face has no face id
    // 1: Detect all faces has no face id
    // 2: Detect the largest face with face id
    // 3: Detect all faces with face id
    stParams.datatype = 3;

    s32FormatType = 1;

    ps16FaceFeatureData = malloc(sizeof(MI_S16) * FR_FEATURE_SIZE);
    if (!ps16FaceFeatureData)
    {
        printf("Malloc FaceFeatureData failed\n");
        gstFaceRecognizeThread_Attr.s32ThreadRetval = -1;
        return NULL;
    }

    pu8FaceCropData = malloc(sizeof(MI_U8) * ARGB8888_112X112_SIZE);
    if (!pu8FaceCropData)
    {
        free(ps16FaceFeatureData);
        printf("Malloc FaceCropData failed\n");
        gstFaceRecognizeThread_Attr.s32ThreadRetval = -1;
        return NULL;
    }

    while (gstFaceRecognizeThread_Attr.bThreadExit != TRUE)
    {
        stChnPort.u32PortId = u32SclSourcePortId;
        CHECK_DLA_RESULT(ST_Common_GetOutputBufInfo(stChnPort, &stSourceBufInfo, &hSourceHandle, NULL),
                         gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        stChnPort.u32PortId = u32SclScaledPortId;
        CHECK_DLA_RESULT(ST_Common_GetOutputBufInfo(stChnPort, &stScaledBufInfo, &hScaledHandle, NULL),
                         gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        // Do Face detect
        CHECK_DLA_RESULT(
            ST_Common_FR_Detect(&ghFr, &stSourceBufInfo, &stScaledBufInfo, stParams, &pstDetectout, &s32FaceNum),
            gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        if (1 == s32FaceNum)
        {
            CHECK_DLA_RESULT(ST_Common_FR_Align(&stSourceBufInfo, s32FormatType, pstDetectout[0], pu8FaceCropData),
                             gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

            CHECK_DLA_RESULT(ST_Common_FR_FeatureExtract(&ghFr, pu8FaceCropData, ps16FaceFeatureData),
                             gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

            // Draw frame
            gstRgnChnPortParam.stFrameChnPort.u32Color = VYU444_GREEN;

            MI_S32 x = ((pstDetectout[0].x1) * 8192) / gstSensorSize.u16Width;
            MI_S32 y = ((pstDetectout[0].y1) * 8192) / gstSensorSize.u16Height;
            MI_S32 w = ((pstDetectout[0].x2 - pstDetectout[0].x1) * 8192) / gstSensorSize.u16Width;
            MI_S32 h = ((pstDetectout[0].y2 - pstDetectout[0].y1) * 8192) / gstSensorSize.u16Height;

            gstRgnChnPortParam.stFrameChnPort.stRect.s32X      = x;
            gstRgnChnPortParam.stFrameChnPort.stRect.s32Y      = y;
            gstRgnChnPortParam.stFrameChnPort.stRect.u32Width  = w;
            gstRgnChnPortParam.stFrameChnPort.stRect.u32Height = h;

            gstRgnChnPortParam.bShow = TRUE;
        }
        else
        {
            gstRgnChnPortParam.bShow = FALSE;
        }

        CHECK_DLA_RESULT(MI_RGN_SetDisplayAttr(u16SocId, ghFrame[0], &stRgnChnPort, &gstRgnChnPortParam),
                         gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        CHECK_DLA_RESULT(ST_Common_PutOutputBufInfo(&hSourceHandle), gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);
        CHECK_DLA_RESULT(ST_Common_PutOutputBufInfo(&hScaledHandle), gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        if (0 < gstFRInputParm.u32SecTimeout)
        {
            gettimeofday(&stCurStamp, NULL);

            if (gstFRInputParm.u32SecTimeout < (MI_S32)stCurStamp.tv_sec - s32BaseSecTime)
            {
                break;
            }
        }
    }

    // Dump ARGB8888 Face Crop Data
    if (s32FaceNum == 1)
    {
        FILE * fpout;
        MI_U16 u16DumpNum = 1;

        CHECK_DLA_RESULT(ST_Common_WriteFile((char *)gstFRInputParm.au8FaceCropDataPath, &fpout, pu8FaceCropData,
                                             ARGB8888_112X112_SIZE, &u16DumpNum, TRUE),
                         gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        CHECK_DLA_RESULT(ST_Common_WriteFile((char *)gstFRInputParm.au8FaceFeatureDataPath, &fpout,
                                             (MI_U8 *)ps16FaceFeatureData, sizeof(MI_S16) * FR_FEATURE_SIZE,
                                             &u16DumpNum, TRUE),
                         gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);
    }
    else
    {
        ST_WARN("Face registration failed, no face was recognized\n");
        gstFaceRecognizeThread_Attr.s32ThreadRetval = -1;
        goto EXIT;
    }

EXIT:
    gstFaceRecognizeThread_Attr.bThreadExit = TRUE;
    free(ps16FaceFeatureData);
    free(pu8FaceCropData);
    return NULL;
}

void *ST_RecognizeThread(void *args)
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stSourceBufInfo;
    MI_SYS_BufInfo_t  stScaledBufInfo;
    MI_SYS_BUF_HANDLE hSourceHandle;
    MI_SYS_BUF_HANDLE hScaledHandle;
    MI_U32            u32SclSourcePortId = 0;
    MI_U32            u32SclScaledPortId = 1;

    MI_U16           u16SocId = 0;
    MI_RGN_ChnPort_t stRgnChnPort;

    MI_S16(*pps16FaceFeaturesData)[FR_FEATURE_SIZE];
    MI_S16 *     ps16FaceFeatureData = NULL;
    MI_U8 *      pu8FaceCropData     = NULL;
    DetectBox_t *pstDetectout        = NULL;
    MI_S32       s32FaceNum          = 0;
    MI_S32       s32MaxSimilarNum    = 0;
    ParamDet_t   stParams;
    MI_S32       s32FormatType;

    MI_U32         s32BaseSecTime = 0;
    struct timeval stCurStamp;
    gettimeofday(&stCurStamp, NULL);
    s32BaseSecTime = stCurStamp.tv_sec;

    memset(&stChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId   = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = 0;
    stRgnChnPort.s32ChnId  = 0;
    stRgnChnPort.s32PortId = 0;

    // 0: Detect the largest face has no face id
    // 1: Detect all faces has no face id
    // 2: Detect the largest face with face id
    // 3: Detect all faces with face id
    stParams.datatype = 1;

    s32FormatType = 1;

    ps16FaceFeatureData = malloc(sizeof(MI_S16) * FR_FEATURE_SIZE);
    if (!ps16FaceFeatureData)
    {
        printf("Malloc FaceFeatureData failed\n");
        gstFaceRecognizeThread_Attr.s32ThreadRetval = -1;
        return NULL;
    }

    pu8FaceCropData = malloc(sizeof(MI_U8) * ARGB8888_112X112_SIZE);
    if (!pu8FaceCropData)
    {
        free(ps16FaceFeatureData);
        printf("Malloc FaceCropData failed\n");
        gstFaceRecognizeThread_Attr.s32ThreadRetval = -1;
        return NULL;
    }

    FILE *fpin;
    fpin = fopen((char *)gstFRInputParm.au8FaceFeatureDataPath, "rb+");
    if (fpin == NULL)
    {
        printf("file %s open fail\n", (char *)gstFRInputParm.au8FaceFeatureDataPath);
        goto EXIT;
    }
    rewind(fpin);
    fread(ps16FaceFeatureData, sizeof(MI_S16), FR_FEATURE_SIZE, fpin);
    fclose(fpin);

    while (gstFaceRecognizeThread_Attr.bThreadExit != TRUE)
    {
        // Do FaceRecognize
        stChnPort.u32PortId = u32SclSourcePortId;
        CHECK_DLA_RESULT(ST_Common_GetOutputBufInfo(stChnPort, &stSourceBufInfo, &hSourceHandle, NULL),
                         gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        stChnPort.u32PortId = u32SclScaledPortId;
        CHECK_DLA_RESULT(ST_Common_GetOutputBufInfo(stChnPort, &stScaledBufInfo, &hScaledHandle, NULL),
                         gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        // Do Face detect
        CHECK_DLA_RESULT(
            ST_Common_FR_Detect(&ghFr, &stSourceBufInfo, &stScaledBufInfo, stParams, &pstDetectout, &s32FaceNum),
            gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        if (s32FaceNum > 0)
        {
            pps16FaceFeaturesData = malloc(sizeof(MI_S16[FR_FEATURE_SIZE]) * s32FaceNum);
            if (!pps16FaceFeaturesData)
            {
                printf("Malloc FaceFeaturesData failed\n");
                goto EXIT;
            }

            for (MI_U32 i = 0; i < s32FaceNum; i++)
            {
                gstFaceRecognizeThread_Attr.s32ThreadRetval =
                    ST_Common_FR_Align(&stSourceBufInfo, s32FormatType, pstDetectout[i], pu8FaceCropData);
                if (0 != gstFaceRecognizeThread_Attr.s32ThreadRetval)
                {
                    printf("ST_Common_FR_Align failed\n");
                    free(pps16FaceFeaturesData);
                    goto EXIT;
                }

                gstFaceRecognizeThread_Attr.s32ThreadRetval =
                    ST_Common_FR_FeatureExtract(&ghFr, pu8FaceCropData, pps16FaceFeaturesData[i]);
                if (0 != gstFaceRecognizeThread_Attr.s32ThreadRetval)
                {
                    printf("ST_Common_FR_FeatureExtract failed\n");
                    free(pps16FaceFeaturesData);
                    goto EXIT;
                }
            }

            gstFaceRecognizeThread_Attr.s32ThreadRetval =
                ST_Common_FR_CompareFeature(ps16FaceFeatureData, pps16FaceFeaturesData, s32FaceNum, &s32MaxSimilarNum);
            if (0 != gstFaceRecognizeThread_Attr.s32ThreadRetval)
            {
                printf("ST_Common_FR_CompareFeature failed\n");
                free(pps16FaceFeaturesData);
                goto EXIT;
            }

            free(pps16FaceFeaturesData);
        }

        // Draw Frame
        for (MI_U32 i = 0; i < MAX_FRAME_HANDLE; i++)
        {
            if (i < s32FaceNum)
            {
                gstRgnChnPortParam.stFrameChnPort.u32Color = VYU444_WHITE;
                if (i == s32MaxSimilarNum)
                {
                    gstRgnChnPortParam.stFrameChnPort.u32Color = VYU444_GREEN;
                }
                MI_S32 x = ((pstDetectout[i].x1) * 8192) / gstSensorSize.u16Width;
                MI_S32 y = ((pstDetectout[i].y1) * 8192) / gstSensorSize.u16Height;
                MI_S32 w = ((pstDetectout[i].x2 - pstDetectout[i].x1) * 8192) / gstSensorSize.u16Width;
                MI_S32 h = ((pstDetectout[i].y2 - pstDetectout[i].y1) * 8192) / gstSensorSize.u16Height;

                gstRgnChnPortParam.stFrameChnPort.stRect.s32X      = x;
                gstRgnChnPortParam.stFrameChnPort.stRect.s32Y      = y;
                gstRgnChnPortParam.stFrameChnPort.stRect.u32Width  = w;
                gstRgnChnPortParam.stFrameChnPort.stRect.u32Height = h;

                if ((0 == gstRgnChnPortParam.stFrameChnPort.stRect.u32Width)
                    && (0 == gstRgnChnPortParam.stFrameChnPort.stRect.u32Height))
                {
                    gstRgnChnPortParam.bShow = FALSE;
                }
                else
                {
                    gstRgnChnPortParam.bShow = TRUE;
                }
            }
            else
            {
                gstRgnChnPortParam.bShow = FALSE;
            }

            CHECK_DLA_RESULT(MI_RGN_SetDisplayAttr(u16SocId, ghFrame[i], &stRgnChnPort, &gstRgnChnPortParam),
                             gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);
        }

        CHECK_DLA_RESULT(ST_Common_PutOutputBufInfo(&hSourceHandle), gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);
        CHECK_DLA_RESULT(ST_Common_PutOutputBufInfo(&hScaledHandle), gstFaceRecognizeThread_Attr.s32ThreadRetval, EXIT);

        if (0 < gstFRInputParm.u32SecTimeout)
        {
            gettimeofday(&stCurStamp, NULL);

            if (gstFRInputParm.u32SecTimeout < (MI_S32)stCurStamp.tv_sec - s32BaseSecTime)
            {
                break;
            }
        }
    }

EXIT:
    gstFaceRecognizeThread_Attr.bThreadExit = TRUE;
    free(ps16FaceFeatureData);
    free(pu8FaceCropData);
    return NULL;
}

static MI_S32 STUB_BaseModuleInit()
{
    MI_SNR_PlaneInfo_t stPlaneInfo;
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;
    MI_U32             u32SnrPadId   = 0;
    MI_U8              u8SensorRes   = gstFRInputParm.u8SensorIndex;
    MI_U32             u32VifGroupId = 0;
    MI_U32             u32VifDevId   = 0;
    MI_U32             u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId        = 0;
    MI_U32 u32SclChnId        = 0;
    MI_U32 u32SclSourcePortId = 0;
    MI_U32 u32SclScaledPortId = 1;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_VIF_GroupAttr_t      stVifGroupAttr;
    MI_VIF_DevAttr_t        stVifDevAttr;
    MI_VIF_OutputPortAttr_t stVifPortAttr;
    /************************************************
    step2 :init sensor/vif
    *************************************************/
    STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SensorRes, 0xFF));

    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId, 0, &stPlaneInfo)); // get sensor size
    gstSensorSize.u16Width  = stPlaneInfo.stCapRect.u16Width;
    gstSensorSize.u16Height = stPlaneInfo.stCapRect.u16Height;

    ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
    STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));

    ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
    STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

    ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);
    STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId, u32VifPortId, &stVifPortAttr));

    /************************************************
    step3 :init isp
    *************************************************/

    MI_ISP_DevAttr_t      stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t     stIspChnParam;
    MI_SYS_WindowRect_t   stIspInputCrop;
    MI_ISP_OutPortParam_t stIspOutPortParam;

    ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
    STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));

    MI_U32 u32SensorBindId = 0;
    ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
    ST_Common_GetIspBindSensorIdByPad(0, &u32SensorBindId);
    stIspChnAttr.u32SensorBindId = u32SensorBindId;
    STCHECKRESULT(ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

    ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
    STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, u32IspPortId, &stIspOutPortParam));

    /************************************************
    step4 :bind vif->isp
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId  = u32VifDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32VifPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId  = u32IspDevId;
    stDstChnPort.u32ChnId  = u32IspChnId;
    stDstChnPort.u32PortId = u32IspPortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;

    eBindType = E_MI_SYS_BIND_TYPE_REALTIME;

    u32BindParam = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step5 :init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutSourcePortParam;
    MI_SCL_OutPortParam_t stSclOutScaledPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    ST_Common_GetSclDefaultPortAttr(&stSclOutSourcePortParam);
    ST_Common_GetSclDefaultPortAttr(&stSclOutScaledPortParam);

    stSclOutSourcePortParam.stSCLOutputSize.u16Width  = gstSensorSize.u16Width;
    stSclOutSourcePortParam.stSCLOutputSize.u16Height = gstSensorSize.u16Height;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclSourcePortId, &stSclOutSourcePortParam));

    stSclOutScaledPortParam.stSCLOutputSize.u16Width  = FR_ACCEPT_WIDTH;
    stSclOutScaledPortParam.stSCLOutputSize.u16Height = FR_ACCEPT_HEIGHT;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclScaledPortId, &stSclOutScaledPortParam));

    /************************************************
    step6 :bind isp->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = u32SclDevId;
    stDstChnPort.u32ChnId  = u32SclChnId;
    stDstChnPort.u32PortId = u32SclSourcePortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 2, 4));

    stDstChnPort.u32PortId = u32SclScaledPortId;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 2, 4));

    /************************************************
    step7 :init fr
    *************************************************/
    InitFrParam_t stInitParam;

    memset(&stInitParam, 0x0, sizeof(InitFrParam_t));
    memcpy(stInitParam.det_model_path, (char *)gstFRInputParm.au8DetectModelPath, MODEL_MAX_LENGTH);
    memcpy(stInitParam.feature_model_path, (char *)gstFRInputParm.au8FeatureModelPath, MODEL_MAX_LENGTH);
    memcpy(stInitParam.cos_model_path, (char *)gstFRInputParm.au8CosModelPath, MODEL_MAX_LENGTH);
    // empirical value
    stInitParam.box_min_size       = 5;
    stInitParam.det_thredhold      = 0.5;
    stInitParam.eye_distance       = 20;
    stInitParam.filter_angle_ratio = 0.2;
    stInitParam.fr_mode            = FR_MODE_FACE_RECOG | FR_MODE_FACE_DET;
    stInitParam.had_create_device  = false;

    STCHECKRESULT(ST_Common_FR_Init(&ghFr, &stInitParam));

    /************************************************
    step8 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;
    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId, 0, &stPlaneInfo)); // get sensor size

    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = gstSensorSize.u16Width;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = gstSensorSize.u16Height;
    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate =
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth * stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 1.3;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

    /************************************************
    step9 :bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclSourcePortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step9 :init rgn
    *************************************************/
    MI_U16                u16SocId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRegionAttr;

    MI_RGN_ChnPort_t stRgnChnPort;
    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    ST_Common_GetRgnDefaultFrameAttr(&gstRgnChnPortParam);
    gstRgnChnPortParam.bShow                      = FALSE;
    gstRgnChnPortParam.stFrameChnPort.u8Thickness = 4;

    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.eType = E_MI_RGN_TYPE_FRAME;
    for (MI_U32 i = 0; i < MAX_FRAME_HANDLE; i++)
    {
        ghFrame[i] = i;
        STCHECKRESULT(ST_Common_RgnCreate(u16SocId, ghFrame[i], &stRegionAttr));

        /************************************************
        step10 :attch handle Frame to venc
        *************************************************/
        STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, ghFrame[i], &stRgnChnPort, &gstRgnChnPortParam));
    }

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit()
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32           u32SnrPadId   = 0;
    MI_U32           u32VifGroupId = 0;
    MI_U32           u32VifDevId   = 0;
    MI_U32           u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId        = 0;
    MI_U32 u32SclChnId        = 0;
    MI_U32 u32SclSourcePortId = 0;
    MI_U32 u32SclScaledPortId = 1;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    MI_U16           u16SocId = 0;
    MI_RGN_ChnPort_t stRgnChnPort;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    for (MI_U32 i = 0; i < MAX_FRAME_HANDLE; i++)
    {
        /************************************************
        step1 :detach rgn->venc
        *************************************************/
        STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, ghFrame[i], &stRgnChnPort));
        STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, ghFrame[i]));
    }
    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    /************************************************
    step2 :unbind scl->venc
    *************************************************/

    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclSourcePortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step3 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));

    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step4 :unbind isp->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = u32SclDevId;
    stDstChnPort.u32ChnId  = u32SclChnId;
    stDstChnPort.u32PortId = u32SclSourcePortId;

    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step5 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclSourcePortId));
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclScaledPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));

    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step6 :deinit fr
    *************************************************/
    STCHECKRESULT(ST_Common_FR_DeInit(&ghFr));

    /************************************************
    step7 :unbind vif->isp
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId  = u32VifDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32VifPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId  = u32IspDevId;
    stDstChnPort.u32ChnId  = u32IspChnId;
    stDstChnPort.u32PortId = u32IspPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step8 :deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));

    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    step9 :deinit vif/sensor
    *************************************************/
    STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
    STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));
    STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
    STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId));

    /************************************************
    step10 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

static MI_S32 ST_FaceRecognition_Preview()
{
    MI_U8                u8SensorNum  = 1;
    MI_U32               u32VencDevId = 0;
    MI_U32               u32VencChnId = 0;
    ST_VideoStreamInfo_t stStreamInfo;

    STCHECKRESULT(STUB_BaseModuleInit(u8SensorNum));

    memset(&gstFaceRecognizeThread_Attr, 0, sizeof(ST_FR_FaceRecognizeThread_Attr_t));
    gstFaceRecognizeThread_Attr.bThreadExit     = FALSE;
    gstFaceRecognizeThread_Attr.s32ThreadRetval = 0;

    if (0 == gstFRInputParm.u8CmdIndex)
    {
        pthread_create(&gstFaceRecognizeThread_Attr.GetFaceDatathread, NULL, ST_RegisterThread,
                       (void *)&gstFaceRecognizeThread_Attr);
    }
    else if (1 == gstFRInputParm.u8CmdIndex)
    {
        pthread_create(&gstFaceRecognizeThread_Attr.GetFaceDatathread, NULL, ST_RecognizeThread,
                       (void *)&gstFaceRecognizeThread_Attr);
    }
    else
    {
        printf("the index is invaild!\n");
        return -1;
    }

    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32VencChnId;
    stStreamInfo.u32Width     = gstSensorSize.u16Width;
    stStreamInfo.u32Height    = gstSensorSize.u16Height;
    stStreamInfo.u32FrameRate = 30;
    stStreamInfo.rtspIndex    = 0;

    // start rtsp
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    if (0 < gstFRInputParm.u32SecTimeout)
    {
        while (FALSE == gstFaceRecognizeThread_Attr.bThreadExit)
        {
            usleep(1 * 1000 * 1000);
        }
    }
    else
    {
        char ch;
        printf("press q to exit\n");
        while ((ch = getchar()) != 'q')
        {
            printf("press q to exit\n");
            usleep(1 * 1000 * 1000);
        }
        gstFaceRecognizeThread_Attr.bThreadExit = TRUE;
    }

    pthread_join(gstFaceRecognizeThread_Attr.GetFaceDatathread, NULL);

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    STCHECKRESULT(STUB_BaseModuleDeInit(u8SensorNum));

    if (MI_SUCCESS != gstFaceRecognizeThread_Attr.s32ThreadRetval)
    {
        return -1;
    }
    return MI_SUCCESS;
}

void ST_FR_Usage(int argc, char **argv)
{
    printf("Usage:%s 0) Do face registration\n", argv[0]);
    printf("Usage:%s 1) Do face recognition\n", argv[0]);
    printf("Usage:%s x index x featuredata x cropdata x\n", argv[0]);
    printf("index        : Specify the sensor index\n");
    printf("featuredata  : Specify the feature data output file\n");
    printf("cropdata     : Specify the face image output file\n");
    printf("timeout      : Specify the working time of demo\n");
}

MI_S32 ST_GetCmdlineParam(int argc, char **argv)
{
    // default user input
    strcpy((char *)gstFRInputParm.au8DetectModelPath, "resource/input/dla/facerecognition/models/fr_det_y24s.img");
    strcpy((char *)gstFRInputParm.au8FeatureModelPath, "resource/input/dla/facerecognition/models/fr_feature_as.img");
    strcpy((char *)gstFRInputParm.au8CosModelPath, "resource/input/dla/facerecognition/models/fr_cos256.img");
    strcpy((char *)gstFRInputParm.au8FaceFeatureDataPath, "out/dla/facerecognition/FaceFeatureData");
    strcpy((char *)gstFRInputParm.au8FaceCropDataPath, "out/dla/facerecognition/FaceCrop.argb8888");

    gstFRInputParm.u8SensorIndex = 0xFF;
    gstFRInputParm.u8CmdIndex    = atoi(argv[1]);
    gstFRInputParm.u32SecTimeout = -1;

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gstFRInputParm.u8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "featuredata"))
        {
            strcpy((char *)gstFRInputParm.au8FaceFeatureDataPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "cropdata"))
        {
            strcpy((char *)gstFRInputParm.au8FaceCropDataPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "timeout"))
        {
            gstFRInputParm.u32SecTimeout = atoi(argv[i + 1]);
        }
    }

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    if (argc < 2)
    {
        ST_FR_Usage(argc, argv);
        return -1;
    }

    ST_GetCmdlineParam(argc, argv);
    STCHECKRESULT(ST_FaceRecognition_Preview());

    memset(&gstFRInputParm, 0x00, sizeof(ST_FR_InputParam_t));
    memset(&gstSensorSize, 0x00, sizeof(ST_SensorSize_t));
    return 0;
}
