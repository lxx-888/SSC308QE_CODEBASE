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
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_dpu.h"
#include "mi_sys_datatype.h"
#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_ldc.h"
#include "st_common_font.h"
#include "st_common_rgn.h"
#include "st_common_rtsp_video.h"
#include "mi_iqserver.h"

typedef struct ST_LdcInputParam_s
{
    MI_U8  u8SensorIndex;
    MI_U8  u8IqBinPath[128];
    MI_U8  u8CalibPolyBinPath[128];
    MI_S32 s32Mode;
} ST_LdcInputParam_t;

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

static ST_LdcInputParam_t gstLdcInputParm;
static ST_SensorSize_t    gstSensorSize;
static ST_SensorSize_t    gstPreviewSize;

static MI_RGN_HANDLE         hPointRegion = 1;
static MI_RGN_ChnPortParam_t gstRgnOsdChnPortParam;

static MI_LDC_ChnLDCAttr_t gstLdcModeChnAttr;
static MI_S32              gs32StepWS = 10;
static MI_S32              gs32StepAD = 10;
static MI_S32              gs32StepJK = 10;
static MI_S32              gs32StepRT = 10;

#define CENTERXOFFSET 253
#define CENTERYOFFSET 131
#define FISHEYERADIUS 1458

MI_S32 ST_DrawMappingPoint(MI_LDC_Point_t *pstPoints, MI_U32 u32PointCnt)
{
    MI_S32              s32Ret   = -1;
    MI_U16              u16SocId = 0;
    MI_RGN_CanvasInfo_t stRgnCanvasInfo;
    MI_LDC_Point_t *    pstDrawPoints = NULL;

    MI_S32 s32RowThickness    = 6;
    MI_S32 s32ColumnThickness = 6;

    MI_S32 s32ReMapPointX = 0;
    MI_S32 s32ReMapPointY = 0;

    memset(&stRgnCanvasInfo, 0, sizeof(MI_RGN_CanvasInfo_t));
    s32Ret = MI_RGN_GetCanvasInfo(u16SocId, hPointRegion, &stRgnCanvasInfo);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("MI_RGN_GetCanvasInfo failed\n");
        return s32Ret;
    }
    memset((char *)stRgnCanvasInfo.virtAddr, 0x00, stRgnCanvasInfo.u32Stride * stRgnCanvasInfo.stSize.u32Height);

    for (int i = 0; i < u32PointCnt; i++)
    {
        pstDrawPoints = (MI_LDC_Point_t *)pstPoints + i;

        if ((pstDrawPoints->s16Y < 0) || (pstDrawPoints->s16X < 0))
        {
            continue;
        }

        pstDrawPoints->s16X = pstDrawPoints->s16X / 2;
        pstDrawPoints->s16Y = pstDrawPoints->s16Y / 2;

        for (int j = 0; j < s32RowThickness; j++)
        {
            for (int k = 0; k < s32ColumnThickness; k++)
            {
                s32ReMapPointY =
                    MIN((int)(gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height / 2), (pstDrawPoints->s16Y + k));
                s32ReMapPointX =
                    MIN((int)(gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width / 2), (pstDrawPoints->s16X + j));

                memset((char *)stRgnCanvasInfo.virtAddr + s32ReMapPointY * stRgnCanvasInfo.u32Stride
                           + s32ReMapPointX * 2,
                       0x00, 1);
                memset((char *)stRgnCanvasInfo.virtAddr + s32ReMapPointY * stRgnCanvasInfo.u32Stride
                           + s32ReMapPointX * 2 + 1,
                       0xFF, 1);
            }
        }
    }

    s32Ret = MI_RGN_UpdateCanvas(u16SocId, hPointRegion);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("MI_RGN_UpdateCanvas failed\n");
        return s32Ret;
    }

    return MI_SUCCESS;
}

MI_S32 ST_ChangeLdcAttrPmode(char *pu8KeyString)
{
    MI_U32 u32LdcDevId = 0;
    MI_U32 u32LdcChnId = 0;
    MI_S32 s32Ret      = -1;

    char *pu8Tmp = NULL;

    strtok(pu8KeyString, " "); // Filter the first words
    pu8Tmp = strtok(NULL, " ");

    if (*pu8KeyString == 'w')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepWS = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomV =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomV + gs32StepWS;
        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt + gs32StepWS;
    }
    else if (*pu8KeyString == 's')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepWS = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomV =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomV - gs32StepWS;
        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt - gs32StepWS;
    }
    else if (*pu8KeyString == 'a')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepAD = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH - gs32StepAD;
        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan - gs32StepAD;
    }
    else if (*pu8KeyString == 'd')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepAD = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH + gs32StepAD;
        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan + gs32StepAD;
    }
    else
    {
        printf("press q to exit\n");
        printf("press 'w' 's' 'a' 'd' for move\n");
    }

    s32Ret = MI_LDC_SetChnLDCAttr(u32LdcDevId, u32LdcChnId, &gstLdcModeChnAttr);

    if (s32Ret != 0)
    {
        printf("Maybe LDC ChnAttr Over The Range Current\n");
        printf("s32ZoomH = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH);
        printf("s32ZoomV = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomV);
        printf("s32Pan = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan);
        printf("s32Tilt = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt);
        return s32Ret;
    }

    return MI_SUCCESS;
}

MI_S32 ST_ChangeLdcAttrRmode(char *pu8KeyString)
{
    MI_U32 u32LdcDevId = 0;
    MI_U32 u32LdcChnId = 0;
    MI_S32 s32Ret      = -1;

    char *pu8Tmp = NULL;

    strtok(pu8KeyString, " "); // Filter the first words
    pu8Tmp = strtok(NULL, " ");

    if (*pu8KeyString == 'w')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepWS = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt - gs32StepWS;
    }
    else if (*pu8KeyString == 's')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepWS = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt + gs32StepWS;
    }
    else if (*pu8KeyString == 'a')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepAD = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan + gs32StepAD;
    }
    else if (*pu8KeyString == 'd')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepAD = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan - gs32StepAD;
    }
    else if (*pu8KeyString == 'j')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepJK = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH + gs32StepJK;
    }
    else if (*pu8KeyString == 'k')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepJK = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH - gs32StepJK;
    }
    else if (*pu8KeyString == 'r')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepRT = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Rotate =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Rotate + gs32StepRT;
    }
    else if (*pu8KeyString == 't')
    {
        if (pu8Tmp != NULL)
        {
            gs32StepRT = MAX(1, atoi(pu8Tmp));
        }

        gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Rotate =
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Rotate - gs32StepRT;
    }
    else
    {
        printf("press q to exit\n");
        printf("press 'w' 's' 'a' 'd' for move\n");
        printf("press 'j' 'k' for zoom\n");
        printf("press 'r' 't' for rotate\n");
    }

    s32Ret = MI_LDC_SetChnLDCAttr(u32LdcDevId, u32LdcChnId, &gstLdcModeChnAttr);
    if (s32Ret != 0)
    {
        printf("Maybe LDC ChnAttr Over The Range Current\n");
        printf("s32ZoomH = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH);
        printf("s32Pan = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan);
        printf("s32Tilt = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt);
        printf("s32Rotate = %d\n", gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Rotate);
        return s32Ret;
    }

    return MI_SUCCESS;
}

void *ST_WaitQThread(void *args)
{
    MI_S32          s32Ret            = -1;
    char            au8KeyString[256] = {0};
    MI_U32          u32LdcDevId       = 0;
    MI_U32          u32LdcChnId       = 0;
    MI_U32          u32PointCnt       = 0;
    MI_LDC_Point_t *pstPoints;
    MI_S32 (*ST_ChangeLdcAttrMode)(char *pu8KeyString);

    switch (gstLdcInputParm.s32Mode)
    {
        case 0:
            ST_ChangeLdcAttrMode = ST_ChangeLdcAttrPmode;
            break;
        case 1:
            ST_ChangeLdcAttrMode = ST_ChangeLdcAttrRmode;
            break;
        default:
            ST_ChangeLdcAttrMode = ST_ChangeLdcAttrRmode;
            break;
    }

    do
    {
        s32Ret = ST_ChangeLdcAttrMode(au8KeyString);
        if (s32Ret != MI_SUCCESS)
        {
            continue;
        }

        s32Ret = MI_LDC_GetRegionBorderMappedPointCnt(u32LdcDevId, u32LdcChnId, 1, &u32PointCnt);
        if (s32Ret != MI_SUCCESS)
        {
            ST_ERR("MI_LDC_GetRegionBorderMappedPointCnt failed\n");
            return NULL;
        }

        pstPoints = malloc(u32PointCnt * sizeof(MI_LDC_Point_t));

        s32Ret = MI_LDC_GetRegionBorderMappedPoints(u32LdcDevId, u32LdcChnId, 1, pstPoints);
        if (s32Ret != MI_SUCCESS)
        {
            ST_ERR("MI_LDC_GetRegionBorderMappedPoints failed\n");
            free(pstPoints);
            return NULL;
        }

        s32Ret = ST_DrawMappingPoint(pstPoints, u32PointCnt);
        if (s32Ret != MI_SUCCESS)
        {
            ST_ERR("ST_DrawMappingPoint failed\n");
            free(pstPoints);
            return NULL;
        }

        free(pstPoints);
    } while (*(fgets(au8KeyString, 256, stdin)) != 'q');

    return NULL;
}

static MI_S32 STUB_BaseModuleInit(MI_U8 u8SensorResTndex)
{
    MI_SYS_ChnPort_t  stSrcChnPort;
    MI_SYS_ChnPort_t  stDstChnPort;
    MI_U32            u32SrcFrmrate;
    MI_U32            u32DstFrmrate;
    MI_SYS_BindType_e eBindType;
    MI_U32            u32BindParam;
    MI_S32            s32Ret      = -1;
    MI_U32            u32SnrPadId = 0;
    MI_U8             u8SensorRes = u8SensorResTndex;

    MI_U32             u32VifGroupId = 0;
    MI_U32             u32VifDevId   = 0;
    MI_U32             u32VifPortId  = 0;
    MI_SNR_PlaneInfo_t stPlaneInfo;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 1;

    MI_U32 u32LdcDevId  = 0;
    MI_U32 u32LdcChnId  = 0;
    MI_U32 u32LdcPortId = 0;

    MI_U32 u32SclDevId  = 1;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    /************************************************
    init sensor/vif
    *************************************************/
    MI_VIF_GroupAttr_t      stVifGroupAttr;
    MI_VIF_DevAttr_t        stVifDevAttr;
    MI_VIF_OutputPortAttr_t stVifPortAttr;
    MI_U32                  u32SensorBindId;

    STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SensorRes, 0xFF));

    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId, 0, &stPlaneInfo)); // get sensor size
    gstSensorSize.u16Width  = stPlaneInfo.stCapRect.u16Width;
    gstSensorSize.u16Height = stPlaneInfo.stCapRect.u16Height;

    ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
    u32SensorBindId                   = E_MI_ISP_SENSOR0;
    stVifGroupAttr.u32GroupStitchMask = ST_Common_VifGroupStitchMaskBySnrMask(u32SensorBindId);
    stVifGroupAttr.eHDRType           = E_MI_VIF_HDR_TYPE_OFF;
    STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));

    ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
    STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

    ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);
    STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId, u32VifPortId, &stVifPortAttr));

    /************************************************
    init isp
    *************************************************/
    MI_ISP_DevAttr_t      stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t     stIspChnParam;
    MI_SYS_WindowRect_t   stIspInputCrop;
    MI_ISP_OutPortParam_t stIspOutPortParam;

    ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
    STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));
    ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
    memset(&stIspChnParam, 0x00, sizeof(MI_ISP_ChnParam_t));
    stIspChnAttr.u32SensorBindId = u32SensorBindId;
    stIspChnAttr.u32Sync3AType   = E_MI_ISP_SYNC3A_AE | E_MI_ISP_SYNC3A_AWB | E_MI_ISP_SYNC3A_IQ;

    stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL2;
    stIspChnParam.eHDRType   = E_MI_ISP_HDR_TYPE_OFF;

    STCHECKRESULT(ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

    ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
    STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, u32IspPortId, &stIspOutPortParam));

    /************************************************
    bind vif->isp
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId  = u32VifDevId;
    stSrcChnPort.u32ChnId  = 0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId  = u32IspDevId;
    stDstChnPort.u32ChnId  = u32IspChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    init ldc
    *************************************************/
    MI_LDC_DevAttr_t        stLdcDevAttr;
    MI_LDC_ChnAttr_t        stLdcChnAttr;
    MI_LDC_InputPortAttr_t  stInputPortAttr;
    MI_LDC_OutputPortAttr_t stOutputPortAttr;
    FILE *                  file_fd       = NULL;
    char                    FilePath[128] = {0};

    ST_Common_GetLdcDefaultDevAttr(&stLdcDevAttr);
    STCHECKRESULT(ST_Common_LdcCreateDevice(u32LdcDevId, &stLdcDevAttr));

    ST_Common_GetLdcDefaultChnAttr(&stLdcChnAttr);
    stLdcChnAttr.eWorkMode      = MI_LDC_WORKMODE_LDC;
    stLdcChnAttr.eInputBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;

    ST_Common_GetLdcDefaultLdcModeChnAttr(&gstLdcModeChnAttr);
    gstLdcModeChnAttr.eMountMode                   = MI_LDC_DESKTOP_MOUNT;
    gstLdcModeChnAttr.stCalibInfo.s32CenterXOffset = CENTERXOFFSET;
    gstLdcModeChnAttr.stCalibInfo.s32CenterYOffset = CENTERYOFFSET;
    gstLdcModeChnAttr.stCalibInfo.s32FisheyeRadius = FISHEYERADIUS;

    ST_Common_GetLdcDefaultLdcInputPortAttr(&stInputPortAttr);
    stInputPortAttr.u16Width  = gstSensorSize.u16Width;
    stInputPortAttr.u16Height = gstSensorSize.u16Height;

    ST_Common_GetLdcDefaultLdcOutputPortAttr(&stOutputPortAttr);

    switch (gstLdcInputParm.s32Mode)
    {
        case 0:
            gstLdcModeChnAttr.u32RegionNum = 2;

            gstLdcModeChnAttr.stRegionAttr[0].eRegionMode         = MI_LDC_REGION_NO_TRANSFORMATION;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16X      = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Y      = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width  = ALIGN_UP((int)(gstSensorSize.u16Width / 3), (16));
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height = ALIGN_UP((int)(gstSensorSize.u16Height / 3), (2));

            gstLdcModeChnAttr.stRegionAttr[1].eRegionMode                = MI_LDC_REGION_360_PANORAMA;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH      = 300;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan        = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomV      = gstSensorSize.u16Height / 3;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt       = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32InRadius   = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32OutRadius  = FISHEYERADIUS - 20;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32FocalRatio = 10000;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.eCropMode     = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16X             = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Y =
                gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height + 16;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Width  = ALIGN_UP((gstSensorSize.u16Height * 2), (16));
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Height = ALIGN_UP((int)(gstSensorSize.u16Height / 3), (2));
            gstLdcModeChnAttr.stRegionAttr[1].u8Map2RegionId      = 0;

            stOutputPortAttr.u16Width = ALIGN_UP((gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Width), (16));
            stOutputPortAttr.u16Height =
                ALIGN_UP((gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height * 2 + 16), (2));

            break;

        case 1:
            gstLdcModeChnAttr.u32RegionNum = 2;

            gstLdcModeChnAttr.stRegionAttr[0].eRegionMode         = MI_LDC_REGION_NO_TRANSFORMATION;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16X      = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Y      = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height = ALIGN_UP((int)(gstSensorSize.u16Height / 1.5), (2));
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width  = ALIGN_UP((int)(gstSensorSize.u16Width / 1.5), (16));

            gstLdcModeChnAttr.stRegionAttr[1].eRegionMode = MI_LDC_REGION_NORMAL;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16X =
                gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width + 16;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Y      = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Height = ALIGN_UP((int)(gstSensorSize.u16Height / 1.5), (2));
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Width = ALIGN_UP((int)(gstSensorSize.u16Height / 1.5), (16));
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH  = 100;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan    = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt   = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Rotate = 0;
            gstLdcModeChnAttr.stRegionAttr[1].u8Map2RegionId         = 0;

            stOutputPortAttr.u16Height = ALIGN_UP((gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height), (2));
            stOutputPortAttr.u16Width  = ALIGN_UP((gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width
                                                  + gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height + 16),
                                                 (16));

            break;

        default:
            gstLdcModeChnAttr.u32RegionNum = 2;

            gstLdcModeChnAttr.stRegionAttr[0].eRegionMode                = MI_LDC_REGION_360_PANORAMA;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.s32ZoomH      = 360;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.s32Pan        = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.s32ZoomV      = gstSensorSize.u16Height / 3;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.s32Tilt       = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.s32InRadius   = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.s32OutRadius  = FISHEYERADIUS - 20;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.s32FocalRatio = 10000;
            gstLdcModeChnAttr.stRegionAttr[0].stRegionPara.eCropMode     = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16X             = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Y             = 0;
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height = ALIGN_UP((int)(gstSensorSize.u16Height / 3), (2));
            gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width  = ALIGN_UP((gstSensorSize.u16Height * 2), (16));

            gstLdcModeChnAttr.stRegionAttr[1].eRegionMode    = MI_LDC_REGION_NORMAL;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16X = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Y =
                gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height + 16;
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Height = ALIGN_UP((int)(gstSensorSize.u16Height / 1.5), (2));
            gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Width = ALIGN_UP((int)(gstSensorSize.u16Height / 1.5), (16));
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32ZoomH  = 100;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Pan    = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Tilt   = 0;
            gstLdcModeChnAttr.stRegionAttr[1].stRegionPara.s32Rotate = 0;
            gstLdcModeChnAttr.stRegionAttr[1].u8Map2RegionId         = 0;

            stOutputPortAttr.u16Height = ALIGN_UP((gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height
                                                   + gstLdcModeChnAttr.stRegionAttr[1].stOutRect.u16Height + 16),
                                                  (2));
            stOutputPortAttr.u16Width = ALIGN_UP((gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width), (16));

            break;
    }

    gstPreviewSize.u16Height = ALIGN_UP((int)(stOutputPortAttr.u16Height / 2), (16));
    gstPreviewSize.u16Width  = ALIGN_UP((int)(stOutputPortAttr.u16Width / 2), (16));

    sprintf(FilePath, (char *)gstLdcInputParm.u8CalibPolyBinPath);
    s32Ret = ST_Common_OpenSourceFile(FilePath, &file_fd);
    if (s32Ret != 0)
    {
        printf("open source file %s failed\n", FilePath);
        return s32Ret;
    }

    ST_Common_GetSourceFileSize(file_fd, &gstLdcModeChnAttr.stCalibInfo.u32CalibPolyBinSize);
    gstLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr = malloc(gstLdcModeChnAttr.stCalibInfo.u32CalibPolyBinSize);
    if (NULL == gstLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr)
    {
        printf("[%s]:%d malloc failed \n", __FUNCTION__, __LINE__);
        ST_Common_CloseSourceFile(&file_fd);
        return -1;
    }

    fseek(file_fd, 0, SEEK_SET);
    fread((char *)gstLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr, 1, gstLdcModeChnAttr.stCalibInfo.u32CalibPolyBinSize,
          file_fd);

    ST_Common_CloseSourceFile(&file_fd);

    // Set LDCModeChn
    s32Ret = ST_Common_LdcStartLdcModeChn(u32LdcDevId, u32LdcChnId, &stLdcChnAttr, &gstLdcModeChnAttr, &stInputPortAttr,
                                          &stOutputPortAttr);
    if (s32Ret != 0)
    {
        free(gstLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr);
        return s32Ret;
    }

    free(gstLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr);
    gstLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr = NULL;

    /************************************************
    bind isp->ldc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = 1;
    stDstChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stDstChnPort.u32DevId  = u32LdcDevId;
    stDstChnPort.u32ChnId  = u32LdcChnId;
    stDstChnPort.u32PortId = u32LdcPortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    stDstChnPort.u32ChnId = u32LdcChnId;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);

    stSclOutPortParam.stSCLOutputSize.u16Width  = gstPreviewSize.u16Width;
    stSclOutPortParam.stSCLOutputSize.u16Height = gstPreviewSize.u16Height;

    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));

    /************************************************
    bind ldc->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stSrcChnPort.u32DevId  = u32LdcDevId;
    stSrcChnPort.u32ChnId  = u32LdcChnId;
    stSrcChnPort.u32PortId = u32LdcPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = u32SclDevId;
    stDstChnPort.u32ChnId  = u32SclChnId;
    stDstChnPort.u32PortId = u32SclPortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;

    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);

    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = gstPreviewSize.u16Width;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = gstPreviewSize.u16Height;
    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate =
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth * stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 1.3;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

    /************************************************
    bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId;
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
    init rgn
    *************************************************/
    MI_U16                u16SocId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRgnOsdAttr;
    MI_RGN_ChnPort_t      stRgnChnPort;

    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    ST_Common_GetRgnDefaultCreateAttr(&stRgnOsdAttr);
    stRgnOsdAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB4444;
    stRgnOsdAttr.stOsdInitParam.stSize.u32Height =
        ALIGN_UP((int)(gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height / 2), (16));
    stRgnOsdAttr.stOsdInitParam.stSize.u32Width =
        ALIGN_UP((int)(gstLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width / 2), (16));
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hPointRegion, &stRgnOsdAttr));

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    ST_Common_GetRgnDefaultOsdAttr(&gstRgnOsdChnPortParam);
    gstRgnOsdChnPortParam.u32Layer                  = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.u8PaletteIdx = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32X = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32Y = 0;
    gstRgnOsdChnPortParam.bShow                     = TRUE;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hPointRegion, &stRgnChnPort, &gstRgnOsdChnPortParam));

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit()
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    MI_U32 u32SnrPadId   = 0;
    MI_U32 u32VifGroupId = 0;
    MI_U32 u32VifDevId   = 0;
    MI_U32 u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 1;

    MI_U32 u32LdcDevId  = 0;
    MI_U32 u32LdcChnId  = 0;
    MI_U32 u32LdcPortId = 0;

    MI_U32 u32SclDevId  = 1;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    MI_U16           u16SocId = 0;
    MI_RGN_ChnPort_t stRgnChnPort;

    /************************************************
    detach rgn->venc
    *************************************************/
    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    /************************************************
    deinit rgn
    *************************************************/
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hPointRegion, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hPointRegion));
    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    /************************************************
    unbind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));

    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    unbind ldc->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stSrcChnPort.u32DevId  = u32LdcDevId;
    stSrcChnPort.u32ChnId  = u32LdcChnId;
    stSrcChnPort.u32PortId = u32LdcPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = u32SclDevId;
    stDstChnPort.u32ChnId  = u32SclChnId;
    stDstChnPort.u32PortId = u32SclPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));

    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    unbind isp->ldc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stDstChnPort.u32DevId  = u32LdcDevId;
    stDstChnPort.u32ChnId  = u32LdcChnId;
    stDstChnPort.u32PortId = u32LdcPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    deinit ldc
    *************************************************/
    STCHECKRESULT(ST_Common_LdcStopChn(u32LdcDevId, u32LdcChnId));
    STCHECKRESULT(ST_Common_LdcDestroyDevice(u32LdcDevId));

    /************************************************
    unbind vif->isp
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
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));

    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    deinit vif/sensor
    *************************************************/
    STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
    STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));
    STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
    STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId));

    /************************************************
    sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

void ST_Drag_Mode_Usage(char **argv)
{
    printf("Usage:%s index x iqbin xxx.bin mode x\n", argv[0]);
    printf("mode 0: P->O (360 degrees panoramic correction -> Uncorrected original image)\n");
    printf("mode 1: R->O (normal correction -> Uncorrected original image)\n");
    printf("mode 2: R->P (normal correction -> 360 degrees panoramic corrected image)\n");
}

MI_S32 ST_Ldc_GetCmdlineParam(int argc, char **argv)
{
    gstLdcInputParm.u8SensorIndex = 0xFF; // default user input
    gstLdcInputParm.s32Mode       = 2;
    strcpy((char *)gstLdcInputParm.u8CalibPolyBinPath, "./resource/input/ldc/drag/CalibPoly_new.bin");

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gstLdcInputParm.u8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "iqbin"))
        {
            strcpy((char *)gstLdcInputParm.u8IqBinPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "mode"))
        {
            gstLdcInputParm.s32Mode = atoi(argv[i + 1]);
        }
    }

    return MI_SUCCESS;
}

static MI_S32 ST_Pipeline_Preview()
{
    pthread_t            pGetWaitQTheead;
    MI_U32               u32VencDevId = 0;
    MI_U32               u32VencChnId = 0;
    MI_U32               u32IspDevId  = 0;
    MI_U32               u32IspChnId  = 0;
    MI_U32               u32SensorPad = 0;
    char                 IqApiBinFilePath[128];
    ST_VideoStreamInfo_t stStreamInfo;
    MI_SNR_PlaneInfo_t   stPlaneInfo;

    STCHECKRESULT(STUB_BaseModuleInit(gstLdcInputParm.u8SensorIndex));

    if (strlen((char *)gstLdcInputParm.u8IqBinPath) == 0)
    {
        MI_SNR_GetPlaneInfo(u32SensorPad, 0, &stPlaneInfo);
        sprintf(IqApiBinFilePath, "/config/iqfile/%s_api.bin", stPlaneInfo.s8SensorName);
    }
    else
    {
        strcpy(IqApiBinFilePath, (char *)gstLdcInputParm.u8IqBinPath);
    }
    ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, IqApiBinFilePath);

    pthread_create(&pGetWaitQTheead, NULL, ST_WaitQThread, NULL);

    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32VencChnId;
    stStreamInfo.u32Width     = gstPreviewSize.u16Width;
    stStreamInfo.u32Height    = gstPreviewSize.u16Height;
    stStreamInfo.u32FrameRate = 25;
    stStreamInfo.rtspIndex    = 0;
    // start rtsp
    STCHECKRESULT(ST_Common_RtspServerStartVideo(&stStreamInfo));

    pthread_join(pGetWaitQTheead, NULL);

    // stop rtsp
    STCHECKRESULT(ST_Common_RtspServerStopVideo(&stStreamInfo));
    STCHECKRESULT(STUB_BaseModuleDeInit());

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    if (argc < 2)
    {
        ST_Drag_Mode_Usage(argv);
        return -1;
    }

    STCHECKRESULT(ST_Ldc_GetCmdlineParam(argc, argv));

    STCHECKRESULT(ST_Pipeline_Preview());

    memset(&gstLdcInputParm, 0, sizeof(ST_LdcInputParam_t));

    return 0;
}