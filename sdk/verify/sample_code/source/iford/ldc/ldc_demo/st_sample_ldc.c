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

#include "mi_sys.h"

#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_ldc.h"
#include "st_common_rtsp_video.h"

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

static MI_U8           gu8SensorIndex;
static ST_SensorSize_t gstSensorSize;
static char            IqBinPath[128];

static MI_S32 STUB_BaseModuleInit(MI_U8 u8SensorResTndex)
{
    MI_SNR_PlaneInfo_t stPlaneInfo;
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;
    MI_U32             u32SnrPadId   = 0;
    MI_U8              u8SensorRes   = u8SensorResTndex;
    MI_U32             u32VifGroupId = 0;
    MI_U32             u32VifDevId   = 0;
    MI_U32             u32VifPortId  = 0;
    MI_S32             s32Ret        = -1;

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

    ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
    STCHECKRESULT(ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

    ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
    stIspOutPortParam.stCropRect.u16Width  = gstSensorSize.u16Width;
    stIspOutPortParam.stCropRect.u16Height = gstSensorSize.u16Height;
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
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step5 :init ldc
    *************************************************/
    MI_LDC_DevAttr_t        stLdcDevAttr;
    MI_LDC_ChnAttr_t        stLdcChnAttr;
    MI_LDC_ChnLDCAttr_t     stLdcModeChnAttr;
    MI_LDC_InputPortAttr_t  stInputPortAttr;
    MI_LDC_OutputPortAttr_t stOutputPortAttr;
    FILE *                  file_fd       = NULL;
    char                    FilePath[128] = {0};

    ST_Common_GetLdcDefaultDevAttr(&stLdcDevAttr);
    STCHECKRESULT(ST_Common_LdcCreateDevice(u32LdcDevId, &stLdcDevAttr));
    ST_Common_GetLdcDefaultChnAttr(&stLdcChnAttr);
    ST_Common_GetLdcDefaultLdcModeChnAttr(&stLdcModeChnAttr);
    stLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Width  = gstSensorSize.u16Width;
    stLdcModeChnAttr.stRegionAttr[0].stOutRect.u16Height = gstSensorSize.u16Height;
    ST_Common_GetLdcDefaultLdcInputPortAttr(&stInputPortAttr);
    stInputPortAttr.u16Width  = gstSensorSize.u16Width;
    stInputPortAttr.u16Height = gstSensorSize.u16Height;
    ST_Common_GetLdcDefaultLdcOutputPortAttr(&stOutputPortAttr);
    stOutputPortAttr.u16Width  = gstSensorSize.u16Width;
    stOutputPortAttr.u16Height = gstSensorSize.u16Height;

    sprintf(FilePath, "./resource/input/CalibPoly_new.bin");
    s32Ret = ST_Common_OpenSourceFile(FilePath, &file_fd);
    if (s32Ret != 0)
    {
        printf("open source file %s failed\n", FilePath);
        return s32Ret;
    }

    ST_Common_GetSourceFileSize(file_fd, &stLdcModeChnAttr.stCalibInfo.u32CalibPolyBinSize);
    stLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr = malloc(stLdcModeChnAttr.stCalibInfo.u32CalibPolyBinSize);
    if (NULL == stLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr)
    {
        printf("[%s]:%d malloc failed \n", __FUNCTION__, __LINE__);
    }

    fseek(file_fd, 0, SEEK_SET);
    fread((char *)stLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr, 1, stLdcModeChnAttr.stCalibInfo.u32CalibPolyBinSize,
          file_fd);

    ST_Common_CloseSourceFile(&file_fd);
    s32Ret = ST_Common_LdcStartLdcModeChn(u32LdcDevId, u32LdcChnId, &stLdcChnAttr, &stLdcModeChnAttr, &stInputPortAttr,
                                          &stOutputPortAttr);
    if (s32Ret != 0)
    {
        free(stLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr);
        return s32Ret;
    }
    free(stLdcModeChnAttr.stCalibInfo.pCalibPolyBinAddr);
    /************************************************
    step6 :bind isp->ldc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stDstChnPort.u32DevId  = u32LdcDevId;
    stDstChnPort.u32ChnId  = u32LdcChnId;
    stDstChnPort.u32PortId = u32LdcPortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_HW_AUTOSYNC;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step7 :init scl
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

    memset(&stOutputPortAttr, 0x00, sizeof(MI_LDC_OutputPortAttr_t));
    STCHECKRESULT(MI_LDC_GetOutputPortAttr(u32LdcDevId, u32LdcChnId, &stOutputPortAttr));
    stSclOutPortParam.stSCLOutputSize.u16Width  = stOutputPortAttr.u16Width;
    stSclOutPortParam.stSCLOutputSize.u16Height = stOutputPortAttr.u16Height;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));

    /************************************************
    step8 :bind ldc->scl
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
    eBindType              = E_MI_SYS_BIND_TYPE_HW_AUTOSYNC;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 0, 4));

    /************************************************
    step9 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;

    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = stOutputPortAttr.u16Width;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = stOutputPortAttr.u16Height;
    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate =
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth * stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 1.3;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

    /************************************************
    step10 :bind scl->venc
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

    /************************************************
    step1 :unbind scl->venc
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
    step2 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));

    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step3 :unbind ldc->scl
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
    step4 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));

    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step5 :unbind isp->ldc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stDstChnPort.u32DevId  = u32LdcDevId;
    stDstChnPort.u32ChnId  = u32LdcChnId;
    stDstChnPort.u32PortId = u32LdcPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step6 :deinit ldc
    *************************************************/
    STCHECKRESULT(ST_Common_LdcStopChn(u32LdcDevId, u32LdcChnId));
    STCHECKRESULT(ST_Common_LdcDestroyDevice(u32LdcDevId));
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
    stDstChnPort.u32PortId = 0;
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

void ST_Ldc_Mode_Usage(void)
{
    printf("Usage:./prog_ldc_ldc_demo)  vif->isp->ldc->scl->venc->rtsp\n");
    printf("Usage:./prog_ldc_ldc_demo index x iqbin xxx.bin)set sensor Resindex and iqbin\n");
}

MI_S32 ST_Ldc_GetCmdlineParam(int argc, char **argv)
{
    gu8SensorIndex = 0xFF; // default user input
    memset(IqBinPath, 0, 128);
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gu8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "iqbin"))
        {
            strcpy(IqBinPath, argv[i + 1]);
        }
    }
    return MI_SUCCESS;
}

static MI_S32 ST_LdcModePipeline_Preview()
{
    MI_U32               u32VencDevId = 0;
    MI_U32               u32VencChnId = 0;
    MI_U32               u32IspDevId  = 0;
    MI_U32               u32IspChnId  = 0;
    MI_U32               u32SensorPad = 0;
    char                 IqApiBinFilePath[128];
    ST_VideoStreamInfo_t stStreamInfo;
    MI_SNR_PlaneInfo_t   stPlaneInfo;

    STCHECKRESULT(STUB_BaseModuleInit(gu8SensorIndex));

    if (strlen(IqBinPath) == 0)
    {
        MI_SNR_GetPlaneInfo(u32SensorPad, 0, &stPlaneInfo);
        sprintf(IqApiBinFilePath, "/config/iqfile/%s_api.bin", stPlaneInfo.s8SensorName);
    }
    else
    {
        strcpy(IqApiBinFilePath, IqBinPath);
    }

    ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, IqApiBinFilePath);

    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32VencChnId;
    stStreamInfo.u32Width     = 1920;
    stStreamInfo.u32Height    = 1080;
    stStreamInfo.u32FrameRate = 20;
    stStreamInfo.rtspIndex    = 0;
    // start rtsp
    STCHECKRESULT(ST_Common_RtspServerStartVideo(&stStreamInfo));

    ST_Common_Pause();

    // stop rtsp
    STCHECKRESULT(ST_Common_RtspServerStopVideo(&stStreamInfo));

    STCHECKRESULT(STUB_BaseModuleDeInit());

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    if (argc > 5)
    {
        ST_Ldc_Mode_Usage();
        return -1;
    }

    ST_Ldc_GetCmdlineParam(argc, argv);

    STCHECKRESULT(ST_LdcModePipeline_Preview());
    memset(&gstSensorSize, 0, sizeof(ST_SensorSize_t));
    return 0;
}