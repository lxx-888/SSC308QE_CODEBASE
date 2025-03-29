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

#include "mi_isp_cus3a_api.h"

#include "st_common_font.h"
#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_rgn.h"
#include "st_common_venc.h"
#include "st_common_dla_hgr.h"
#include "st_common_rtsp_video.h"

#define HGR_MAX_HANDS (32)
#define MAX_FRAME_HANDLE (32)

typedef struct ST_InputParam_s
{
    MI_U8 u8SensorIndex;
    char  au8DetectModelPath[MODEL_MAX_LENGTH];
    char  au8HandPoseModelPath[MODEL_MAX_LENGTH];
} ST_InputParam_t;

typedef struct ST_HandPoseThread_Attr_s
{
    pthread_t pGetHandPoseThread;
    MI_BOOL   bThreadExit;
    MI_S32    s32ThreadRetval;
} ST_HandPoseThread_Attr_t;

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

static ST_InputParam_t          gstHGRInputParm;
static ST_HandPoseThread_Attr_t gHandPoseThread_Attr;
static ST_SensorSize_t          gstSensorSize;

static MI_RGN_ChnPortParam_t        gstRgnChnPortParam;
static MI_RGN_ChnPortParam_t        gstRgnOsdChnPortParam;
static ST_Common_OsdDrawText_Attr_t gstDrawTextAttr;
MI_RGN_HANDLE                       hFrame[MAX_FRAME_HANDLE] = {0};
MI_RGN_HANDLE                       hOsd_OK                  = 100;
MI_RGN_HANDLE                       hOsd_L                   = 101;
MI_RGN_HANDLE                       hOsd_YES                 = 102;
MI_RGN_HANDLE                       hOsd_STOP                = 103;
MI_RGN_HANDLE                       hOsd_OTHERS              = 104;

MI_HGR_HANDLE hHgr;

void *ST_WaitQThread(void *args)
{
    char ch;
    printf("press q to exit\n");
    while ((ch = getchar()) != 'q')
    {
        printf("press q to exit\n");
        usleep(1 * 1000 * 1000);
    }
    gHandPoseThread_Attr.bThreadExit = TRUE;
    return NULL;
}

MI_S32 ST_Show_TipText()
{
    gstDrawTextAttr.u32X   = 0;
    gstDrawTextAttr.u32Y   = 0;
    gstDrawTextAttr.color  = COLOR_OF_RED_ARGB1555;
    gstDrawTextAttr.handle = hOsd_OK;
    strcpy(gstDrawTextAttr.text, "OK \n");
    OsdDrawTextCanvas(&gstDrawTextAttr);

    gstDrawTextAttr.color  = COLOR_OF_GREEN_ARGB1555;
    gstDrawTextAttr.handle = hOsd_L;
    strcpy(gstDrawTextAttr.text, "L \n");
    OsdDrawTextCanvas(&gstDrawTextAttr);

    gstDrawTextAttr.color  = COLOR_OF_BULE_ARGB1555;
    gstDrawTextAttr.handle = hOsd_YES;
    strcpy(gstDrawTextAttr.text, "yes \n");
    OsdDrawTextCanvas(&gstDrawTextAttr);

    gstDrawTextAttr.color  = 0xFFE0;
    gstDrawTextAttr.handle = hOsd_STOP;
    strcpy(gstDrawTextAttr.text, "stop \n");
    OsdDrawTextCanvas(&gstDrawTextAttr);

    gstDrawTextAttr.color  = 0xFFFC;
    gstDrawTextAttr.handle = hOsd_OTHERS;
    strcpy(gstDrawTextAttr.text, "others \n");
    OsdDrawTextCanvas(&gstDrawTextAttr);

    return MI_SUCCESS;
}

void *ST_HandPoseThread(void *args)
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stSourceBufInfo;
    MI_SYS_BufInfo_t  stScaledBufInfo;
    MI_SYS_BufInfo_t  stHandBufInfo;
    MI_SYS_BUF_HANDLE hSourceHandle;
    MI_SYS_BUF_HANDLE hScaledHandle;
    MI_U32            u32SclSourcePortId = 0;
    MI_U32            u32SclScaledPortId = 1;
    MI_U32            s32Ret             = 0;

    MI_U16           u16SocId = 0;
    MI_RGN_ChnPort_t stRgnChnPort;

    memset(&stChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId   = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = 0;
    stRgnChnPort.s32ChnId  = 0;
    stRgnChnPort.s32PortId = 0;

    AlgoHgrResult_t astHgrResult[HGR_MAX_HANDS] = {0};
    MI_S32          s32HandCount                = 0;
    HgrBbox_t *     pstHandbox                  = NULL;

    ST_Show_TipText();
    while (gHandPoseThread_Attr.bThreadExit != TRUE)
    {
        // Get HandPose Data
        stChnPort.u32PortId = u32SclSourcePortId;
        CHECK_DLA_RESULT(ST_Common_GetOutputBufInfo(stChnPort, &stSourceBufInfo, &hSourceHandle, NULL),
                         gHandPoseThread_Attr.s32ThreadRetval, EXIT);

        stChnPort.u32PortId = u32SclScaledPortId;
        CHECK_DLA_RESULT(ST_Common_GetOutputBufInfo(stChnPort, &stScaledBufInfo, &hScaledHandle, NULL),
                         gHandPoseThread_Attr.s32ThreadRetval, EXIT);

        CHECK_DLA_RESULT(ST_Common_HGR_DetectHands(&hHgr, &stScaledBufInfo, &s32HandCount, &pstHandbox),
                         gHandPoseThread_Attr.s32ThreadRetval, EXIT);

        // Draw Frame
        for (MI_U32 i = 0; i < MAX_FRAME_HANDLE; i++)
        {
            if (i < s32HandCount)
            {
                // crop hand buffer
                MI_PHY phyOutAllocAddr = 0;
                void * pVirOutaddr     = NULL;
                CHECK_DLA_RESULT(MI_SYS_MMA_Alloc(0, (MI_U8 *)"mma_heap_name0", NV12_256X256_SIZE, &phyOutAllocAddr),
                                 s32Ret, EXIT);
                s32Ret = MI_SYS_Mmap(phyOutAllocAddr, NV12_256X256_SIZE, &pVirOutaddr, FALSE);
                if (s32Ret != MI_SUCCESS)
                {
                    MI_SYS_MMA_Free(0, phyOutAllocAddr);

                    printf("MI_SYS_Mmap ERR\n");
                    gHandPoseThread_Attr.bThreadExit = TRUE;
                    return NULL;
                }

                MI_SCL_DirectBuf_t stSrcBuff;
                memset(&stSrcBuff, 0x0, sizeof(MI_SCL_DirectBuf_t));
                stSrcBuff.u32Width     = stSourceBufInfo.stFrameData.u16Width;
                stSrcBuff.u32Height    = stSourceBufInfo.stFrameData.u16Height;
                stSrcBuff.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                stSrcBuff.u32Stride[0] = stSourceBufInfo.stFrameData.u32Stride[0];
                stSrcBuff.u32Stride[1] = stSourceBufInfo.stFrameData.u32Stride[1];
                stSrcBuff.u32BuffSize  = stSourceBufInfo.stFrameData.u32BufSize;
                stSrcBuff.phyAddr[0]   = stSourceBufInfo.stFrameData.phyAddr[0];
                stSrcBuff.phyAddr[1]   = stSourceBufInfo.stFrameData.phyAddr[1];

                MI_SCL_DirectBuf_t stDestBuff;
                memset(&stDestBuff, 0x0, sizeof(MI_SCL_DirectBuf_t));
                stDestBuff.u32Width     = HGR_HANDPOSE_ACCEPT_WIDTH;
                stDestBuff.u32Height    = HGR_HANDPOSE_ACCEPT_HEIGHT;
                stDestBuff.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                stDestBuff.u32Stride[0] = HGR_HANDPOSE_ACCEPT_WIDTH;
                stDestBuff.u32Stride[1] = HGR_HANDPOSE_ACCEPT_WIDTH;
                stDestBuff.u32BuffSize  = NV12_256X256_SIZE;
                stDestBuff.phyAddr[0]   = phyOutAllocAddr;
                stDestBuff.phyAddr[1]   = phyOutAllocAddr + stDestBuff.u32Stride[0] * stDestBuff.u32Height;

                MI_SYS_WindowRect_t stOutputCrop;
                memset(&stOutputCrop, 0x0, sizeof(MI_SYS_WindowRect_t));
                stOutputCrop.u16X      = pstHandbox[i].x;
                stOutputCrop.u16Y      = pstHandbox[i].y;
                stOutputCrop.u16Width  = pstHandbox[i].width;
                stOutputCrop.u16Height = pstHandbox[i].height;

                s32Ret = MI_SCL_StretchBuf(&stSrcBuff, &stOutputCrop, &stDestBuff, E_MI_SCL_FILTER_TYPE_AUTO);
                if (s32Ret != MI_SUCCESS)
                {
                    MI_SYS_Munmap(pVirOutaddr, stDestBuff.u32BuffSize);
                    MI_SYS_MMA_Free(0, phyOutAllocAddr);

                    printf("MI_SCL_StretchBuf ERR\n");
                    gHandPoseThread_Attr.bThreadExit = TRUE;
                    return NULL;
                }

                HgrPoint_t *pstOutpoint               = NULL;
                MI_S32      ps32Pointcounts           = 0;
                stHandBufInfo.stFrameData.u32BufSize  = stDestBuff.u32BuffSize;
                stHandBufInfo.stFrameData.phyAddr[0]  = stDestBuff.phyAddr[0];
                stHandBufInfo.stFrameData.phyAddr[1]  = stDestBuff.phyAddr[1];
                stHandBufInfo.stFrameData.pVirAddr[0] = (void *)pVirOutaddr;
                stHandBufInfo.stFrameData.pVirAddr[1] = (void *)pVirOutaddr + 1;
                memset(astHgrResult, 0x0, sizeof(AlgoHgrResult_t) * HGR_MAX_HANDS);
                s32Ret = ST_Common_HGR_RecognizeHand(&hHgr, &stHandBufInfo, &pstHandbox[i], &pstOutpoint,
                                                     &ps32Pointcounts, &astHgrResult[i]);

                MI_SYS_Munmap(pVirOutaddr, stDestBuff.u32BuffSize);
                MI_SYS_MMA_Free(0, phyOutAllocAddr);

                if (s32Ret != MI_SUCCESS)
                {
                    printf("ST_Common_HGR_RecognizeHand ERR\n");
                    gHandPoseThread_Attr.bThreadExit = TRUE;
                    return NULL;
                }

                // show fram with rgn
                if (astHgrResult[i].hg_ok == TRUE)
                {
                    gstRgnChnPortParam.stFrameChnPort.u32Color = 0xff4c55;
                }
                else if (astHgrResult[i].hg_L == TRUE)
                {
                    gstRgnChnPortParam.stFrameChnPort.u32Color = 0x009500;
                }
                else if (astHgrResult[i].hg_yes == TRUE)
                {
                    gstRgnChnPortParam.stFrameChnPort.u32Color = 0x6b1dff;
                }
                else if (astHgrResult[i].hg_stop == TRUE)
                {
                    gstRgnChnPortParam.stFrameChnPort.u32Color = 0x95e200;
                }
                else
                {
                    gstRgnChnPortParam.stFrameChnPort.u32Color = 0x80ff80;
                }

                MI_S32 x = ((astHgrResult[i].bbox.x) * 8192) / gstSensorSize.u16Width;
                MI_S32 y = ((astHgrResult[i].bbox.y) * 8192) / gstSensorSize.u16Height;
                MI_S32 w = ((astHgrResult[i].bbox.width) * 8192) / gstSensorSize.u16Width;
                MI_S32 h = ((astHgrResult[i].bbox.height) * 8192) / gstSensorSize.u16Height;

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

            CHECK_DLA_RESULT(MI_RGN_SetDisplayAttr(u16SocId, hFrame[i], &stRgnChnPort, &gstRgnChnPortParam),
                             gHandPoseThread_Attr.s32ThreadRetval, EXIT);
        }

        CHECK_DLA_RESULT(ST_Common_PutOutputBufInfo(&hScaledHandle), gHandPoseThread_Attr.s32ThreadRetval, EXIT);
        CHECK_DLA_RESULT(ST_Common_PutOutputBufInfo(&hSourceHandle), gHandPoseThread_Attr.s32ThreadRetval, EXIT);
    }

EXIT:
    gHandPoseThread_Attr.bThreadExit = TRUE;
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
    MI_U8              u8SensorRes   = gstHGRInputParm.u8SensorIndex;
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
    step5 :init hgr
    *************************************************/
    AlgoHgrInitParam_t stInitParam;

    memset(&stInitParam, 0x0, sizeof(AlgoHgrInitParam_t));
    memcpy((char *)stInitParam.det_model, gstHGRInputParm.au8DetectModelPath, IPU_MAX_LENGTH);
    memcpy((char *)stInitParam.land_model, gstHGRInputParm.au8HandPoseModelPath, IPU_MAX_LENGTH);
    stInitParam.thredhold        = 0.5;
    stInitParam.disp_area.x      = 0;
    stInitParam.disp_area.y      = 0;
    stInitParam.disp_area.width  = gstSensorSize.u16Width;
    stInitParam.disp_area.height = gstSensorSize.u16Height;

    STCHECKRESULT(ST_Common_HGR_Init(&hHgr, &stInitParam));

    /************************************************
    step6 :init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutSourcePortParam;
    MI_SCL_OutPortParam_t stSclOutScaledPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    // for MI_SCL_StretchBuf
    MI_SCL_DEV       DevId = 1;
    MI_SCL_DevAttr_t stDevAttr;
    stDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2;
    STCHECKRESULT(ST_Common_SclCreateDevice(DevId, &stDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    ST_Common_GetSclDefaultPortAttr(&stSclOutSourcePortParam);
    ST_Common_GetSclDefaultPortAttr(&stSclOutScaledPortParam);

    stSclOutSourcePortParam.stSCLOutputSize.u16Width  = gstSensorSize.u16Width;
    stSclOutSourcePortParam.stSCLOutputSize.u16Height = gstSensorSize.u16Height;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclSourcePortId, &stSclOutSourcePortParam));

    stSclOutScaledPortParam.stSCLOutputSize.u16Width  = HGR_HANDS_ACCEPT_WIDTH;
    stSclOutScaledPortParam.stSCLOutputSize.u16Height = HGR_HANDS_ACCEPT_HEIGHT;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclScaledPortId, &stSclOutScaledPortParam));

    /************************************************
    step7 :bind isp->scl
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
    step10 :init rgn
    *************************************************/
    MI_U16                u16SocId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRegionAttr;
    MI_RGN_Attr_t         stRgnOsdAttr;

    MI_RGN_ChnPort_t stRgnChnPort;
    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    ST_Common_GetRgnDefaultFrameAttr(&gstRgnChnPortParam);
    gstRgnChnPortParam.bShow                      = FALSE;
    gstRgnChnPortParam.stFrameChnPort.u8Thickness = gstSensorSize.u16Height * 0.005;

    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    // OSD
    ST_Common_GetRgnDefaultCreateAttr(&stRgnOsdAttr);
    stRgnOsdAttr.stOsdInitParam.ePixelFmt        = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    stRgnOsdAttr.stOsdInitParam.stSize.u32Height = 100;
    stRgnOsdAttr.stOsdInitParam.stSize.u32Width  = 400;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hOsd_OK, &stRgnOsdAttr));
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hOsd_L, &stRgnOsdAttr));
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hOsd_YES, &stRgnOsdAttr));
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hOsd_STOP, &stRgnOsdAttr));
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hOsd_OTHERS, &stRgnOsdAttr));

    // Frame
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.eType = E_MI_RGN_TYPE_FRAME;
    for (MI_U32 i = 0; i < MAX_FRAME_HANDLE; i++)
    {
        hFrame[i] = i;
        STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hFrame[i], &stRegionAttr));

        /************************************************
        step11 :attch handle Frame to venc
        *************************************************/
        // Frame
        STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hFrame[i], &stRgnChnPort, &gstRgnChnPortParam));
    }

    // OSD
    ST_Common_GetRgnDefaultOsdAttr(&gstRgnOsdChnPortParam);
    gstRgnOsdChnPortParam.u32Layer                  = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.u8PaletteIdx = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32X = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32Y = 0;
    gstRgnOsdChnPortParam.bShow                     = TRUE;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hOsd_OK, &stRgnChnPort, &gstRgnOsdChnPortParam));
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32Y = 100;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hOsd_L, &stRgnChnPort, &gstRgnOsdChnPortParam));
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32Y = 200;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hOsd_YES, &stRgnChnPort, &gstRgnOsdChnPortParam));
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32Y = 300;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hOsd_STOP, &stRgnChnPort, &gstRgnOsdChnPortParam));
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.u32Y = 400;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hOsd_OTHERS, &stRgnChnPort, &gstRgnOsdChnPortParam));

    memset(&gstDrawTextAttr, 0x0, sizeof(ST_Common_OsdDrawText_Attr_t));
    gstDrawTextAttr.color     = COLOR_OF_RED_ARGB1555;
    gstDrawTextAttr.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    gstDrawTextAttr.eFontType = SS_FONT_32x32;
    gstDrawTextAttr.rot       = FONT_ROT_NONE;

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
        STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hFrame[i], &stRgnChnPort));
        STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hFrame[i]));
    }
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hOsd_OK, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hOsd_L, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hOsd_YES, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hOsd_STOP, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hOsd_OTHERS, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hOsd_OK));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hOsd_L));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hOsd_YES));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hOsd_STOP));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hOsd_OTHERS));

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

    // for MI_SCL_StretchBuf
    MI_SCL_DEV DevId = 1;
    STCHECKRESULT(ST_Common_SclDestroyDevice(DevId));
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step6 :deinit hgr
    *************************************************/
    STCHECKRESULT(ST_Common_HGR_DeInit(&hHgr));

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

static MI_S32 ST_HandPose_Preview()
{
    pthread_t            pGetWaitQTheead;
    MI_U32               u32VencDevId = 0;
    MI_U32               u32VencChnId = 0;
    ST_VideoStreamInfo_t stStreamInfo;

    STCHECKRESULT(STUB_BaseModuleInit());

    memset(&gHandPoseThread_Attr, 0, sizeof(ST_HandPoseThread_Attr_t));
    gHandPoseThread_Attr.bThreadExit     = FALSE;
    gHandPoseThread_Attr.s32ThreadRetval = -1;

    pthread_create(&pGetWaitQTheead, NULL, ST_WaitQThread, NULL);
    pthread_create(&gHandPoseThread_Attr.pGetHandPoseThread, NULL, ST_HandPoseThread, NULL);

    // start rtsp
    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32VencChnId;
    stStreamInfo.u32Width     = gstSensorSize.u16Width;
    stStreamInfo.u32Height    = gstSensorSize.u16Height;
    stStreamInfo.u32FrameRate = 30;
    stStreamInfo.rtspIndex    = 0;
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    while (FALSE == gHandPoseThread_Attr.bThreadExit)
    {
        usleep(1 * 1000 * 1000);
    }

    pthread_cancel(pGetWaitQTheead);
    pthread_join(gHandPoseThread_Attr.pGetHandPoseThread, NULL);

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    STCHECKRESULT(STUB_BaseModuleDeInit());

    if (MI_SUCCESS != gHandPoseThread_Attr.s32ThreadRetval)
    {
        return gHandPoseThread_Attr.s32ThreadRetval;
    }
    return MI_SUCCESS;
}

void ST_DET_Usage(int argc, char **argv)
{
    printf("Usage : %s index x\n", argv[0]);
    printf("index : Specify the sensor index\n");
}

MI_S32 ST_FR_GetCmdlineParam(int argc, char **argv)
{
    gstHGRInputParm.u8SensorIndex = 0xFF;

    strcpy(gstHGRInputParm.au8DetectModelPath, "resource/input/dla/handgesturerecognition/models/hgr_det_36y.img");
    strcpy(gstHGRInputParm.au8HandPoseModelPath, "resource/input/dla/handgesturerecognition/models/hgr_land_22y.img");

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gstHGRInputParm.u8SensorIndex = atoi(argv[i + 1]);
        }
    }

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    if (argc > 3)
    {
        ST_DET_Usage(argc, argv);
        return -1;
    }

    ST_FR_GetCmdlineParam(argc, argv);

    STCHECKRESULT(ST_HandPose_Preview());

    memset(&gstHGRInputParm, 0x00, sizeof(ST_InputParam_t));
    memset(&gstSensorSize, 0x00, sizeof(ST_SensorSize_t));
    return 0;
}
