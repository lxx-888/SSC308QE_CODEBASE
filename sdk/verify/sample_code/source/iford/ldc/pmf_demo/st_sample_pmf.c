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
#include "st_common_rtsp_video.h"
#include "mi_iqserver.h"
#include "matrix.h"

typedef enum { E_NONE = 0, E_MOVE, E_ROTATE, E_SCALE, E_CENTER_ROTATION, E_MIRROR } E_TransformType;

typedef struct ST_TransformParam_s
{
    E_TransformType eTransformType;
    MI_S32          move_x;
    MI_S32          move_y;
    MI_S16          rotate;
    MI_FLOAT        scale_x;
    MI_FLOAT        scale_y;
} ST_TransformParam_t;

typedef struct ST_LdcInputParam_s
{
    MI_U8               u8SensorIndex;
    MI_U8               u8IqBinPath[128];
    MI_BOOL             bWithDpu;
    MI_U8               TransformType;
    MI_S64              as64PMFCoef[LDC_PMFCOEF_NUM];
    Matrix *            pstMatrix;
    ST_TransformParam_t stTransformParam;
} ST_LdcInputParam_t;

static ST_LdcInputParam_t gstLdcInputParm;

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

static ST_SensorSize_t gstSensorSize;

void *ST_WaitQThread(void *args)
{
    char ch;
    printf("press q to exit\n");
    while ((ch = getchar()) != 'q')
    {
        printf("press q to exit\n");
        usleep(1 * 1000 * 1000);
    }

    return NULL;
}

static MI_S32 STUB_BaseModuleInit_PMF_Preview(MI_U8 u8SensorResTndex)
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

    MI_U32 u32LdcDevId    = 0;
    MI_U32 u32LdcPMFChnId = 0;
    MI_U32 u32LdcPortId   = 0;

    MI_U32 u32SclDevId  = 1;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    /************************************************
    step2 :init sensor/vif
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
    memset(&stIspChnParam, 0x00, sizeof(MI_ISP_ChnParam_t));
    stIspChnAttr.u32SensorBindId = u32SensorBindId;
    stIspChnAttr.u32Sync3AType   = E_MI_ISP_SYNC3A_AE | E_MI_ISP_SYNC3A_AWB | E_MI_ISP_SYNC3A_IQ;

    stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL2;
    stIspChnParam.eHDRType   = E_MI_ISP_HDR_TYPE_OFF;

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
    MI_LDC_ChnPMFAttr_t     stPMFModeChnAttr;
    MI_LDC_InputPortAttr_t  stInputPortAttr;
    MI_LDC_OutputPortAttr_t stOutputPortAttr;

    ST_Common_GetLdcDefaultDevAttr(&stLdcDevAttr);
    STCHECKRESULT(ST_Common_LdcCreateDevice(u32LdcDevId, &stLdcDevAttr));

    ST_Common_GetLdcDefaultChnAttr(&stLdcChnAttr);
    stLdcChnAttr.eWorkMode      = MI_LDC_WORKMODE_PMF;
    stLdcChnAttr.eInputBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;

    ST_Common_GetLdcDefaultPmfModeChnAttr(&stPMFModeChnAttr);
    stPMFModeChnAttr.as64PMFCoef[0] = gstLdcInputParm.as64PMFCoef[0];
    stPMFModeChnAttr.as64PMFCoef[1] = gstLdcInputParm.as64PMFCoef[1];
    stPMFModeChnAttr.as64PMFCoef[2] = gstLdcInputParm.as64PMFCoef[2];
    stPMFModeChnAttr.as64PMFCoef[3] = gstLdcInputParm.as64PMFCoef[3];
    stPMFModeChnAttr.as64PMFCoef[4] = gstLdcInputParm.as64PMFCoef[4];
    stPMFModeChnAttr.as64PMFCoef[5] = gstLdcInputParm.as64PMFCoef[5];
    stPMFModeChnAttr.as64PMFCoef[6] = gstLdcInputParm.as64PMFCoef[6];
    stPMFModeChnAttr.as64PMFCoef[7] = gstLdcInputParm.as64PMFCoef[7];
    stPMFModeChnAttr.as64PMFCoef[8] = gstLdcInputParm.as64PMFCoef[8];

    ST_Common_GetLdcDefaultLdcInputPortAttr(&stInputPortAttr);
    stInputPortAttr.u16Width  = gstSensorSize.u16Width;
    stInputPortAttr.u16Height = gstSensorSize.u16Height;

    ST_Common_GetLdcDefaultLdcOutputPortAttr(&stOutputPortAttr);
    stOutputPortAttr.u16Width  = gstSensorSize.u16Width;
    stOutputPortAttr.u16Height = gstSensorSize.u16Height;

    // Set PMFModeChn
    s32Ret = ST_Common_LdcStartPmfModeChn(u32LdcDevId, u32LdcPMFChnId, &stLdcChnAttr, &stPMFModeChnAttr,
                                          &stInputPortAttr, &stOutputPortAttr);
    if (s32Ret != 0)
    {
        printf("ST_Common_LdcStartPmfModeChn failed \n");
        return s32Ret;
    }

    /************************************************
    step6 :bind isp->ldc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = 1;
    stDstChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stDstChnPort.u32DevId  = u32LdcDevId;
    stDstChnPort.u32ChnId  = u32LdcPMFChnId;
    stDstChnPort.u32PortId = u32LdcPortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    stDstChnPort.u32ChnId = u32LdcPMFChnId;
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

    stSclOutPortParam.stSCLOutputSize.u16Width  = gstSensorSize.u16Width;
    stSclOutPortParam.stSCLOutputSize.u16Height = gstSensorSize.u16Height;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));

    /************************************************
    step8 :bind ldc->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stSrcChnPort.u32DevId  = u32LdcDevId;
    stSrcChnPort.u32ChnId  = u32LdcPMFChnId;
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
    step9 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;

    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);
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

    MI_U32 u32LdcDevId    = 0;
    MI_U32 u32LdcPMFChnId = 0;
    MI_U32 u32LdcPortId   = 0;

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
    stSrcChnPort.u32ChnId  = u32LdcPMFChnId;
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
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_LDC;
    stDstChnPort.u32DevId  = u32LdcDevId;
    stDstChnPort.u32ChnId  = u32LdcPMFChnId;
    stDstChnPort.u32PortId = u32LdcPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step6 :deinit ldc
    *************************************************/
    STCHECKRESULT(ST_Common_LdcStopChn(u32LdcDevId, u32LdcPMFChnId));
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

void ST_Stitch_Mode_Usage(char **argv)
{
    printf("Usage:%s index x iqbin xxx.bin [cmd][value]\n", argv[0]);
    printf("move:   move [move x] [move y]     such as: move 100,100\n");
    printf("rotate: rotate [angle]             such as: rotate 10\n");
    printf("scale:  scale [scale x] [scale y], such as: scale 0.6 1.2\n");
    printf("comb1:  Combining transformation   rotate 45 degrees in the center(Only applicable 1920x1080)\n");
    printf("comb2:  Combining transformation   Mirror image(Only applicable 1920x1080)\n");
}

void *ST_MatrixStructure()
{
    MATRIX_TYPE unit_matrix[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    switch (gstLdcInputParm.stTransformParam.eTransformType)
    {
        case E_MOVE:
            unit_matrix[0][2]         = gstLdcInputParm.stTransformParam.move_x;
            unit_matrix[1][2]         = gstLdcInputParm.stTransformParam.move_y;
            gstLdcInputParm.pstMatrix = Matrix_gen(3, 3, (MATRIX_TYPE *)unit_matrix);
            break;
        case E_ROTATE:
            unit_matrix[0][0]         = cos(gstLdcInputParm.stTransformParam.rotate * M_PI / 180.0);
            unit_matrix[0][1]         = -1 * sin(gstLdcInputParm.stTransformParam.rotate * M_PI / 180.0);
            unit_matrix[1][0]         = sin(gstLdcInputParm.stTransformParam.rotate * M_PI / 180.0);
            unit_matrix[1][1]         = cos(gstLdcInputParm.stTransformParam.rotate * M_PI / 180.0);
            gstLdcInputParm.pstMatrix = Matrix_gen(3, 3, (MATRIX_TYPE *)unit_matrix);
            break;
        case E_SCALE:
            unit_matrix[0][0]         = gstLdcInputParm.stTransformParam.scale_x;
            unit_matrix[1][1]         = gstLdcInputParm.stTransformParam.scale_y;
            gstLdcInputParm.pstMatrix = Matrix_gen(3, 3, (MATRIX_TYPE *)unit_matrix);
            break;
        case E_CENTER_ROTATION: {
            // Sets the matrix entry value
            MATRIX_TYPE _upper_left_corner_matrix[3][3] = {{1, 0, -(1920 / 2)}, {0, 1, -(1080 / 2)}, {0, 0, 1}};
            MATRIX_TYPE _reverse_clock_45[3][3]         = {{cos(45 * M_PI / 180.0), -1 * sin(45 * M_PI / 180.0), 0},
                                                   {sin(45 * M_PI / 180.0), cos(45 * M_PI / 180.0), 0},
                                                   {0, 0, 1}};
            MATRIX_TYPE _lower_right_corner[3][3] = {{1, 0, (1920 / 2)}, {0, 1, (1080 / 2)}, {0, 0, 1}};

            // Constructive matrix(step0 and step1 and step3)
            Matrix *upper_left_corner_matrix = Matrix_gen(3, 3, (MATRIX_TYPE *)_upper_left_corner_matrix);
            Matrix *reverse_clock_45         = Matrix_gen(3, 3, (MATRIX_TYPE *)_reverse_clock_45);
            Matrix *lower_right_corner       = Matrix_gen(3, 3, (MATRIX_TYPE *)_lower_right_corner);
            Matrix *intermediate_result;

            // Matrix multiplication(step2 · step1 · step0)
            intermediate_result       = M_mul(lower_right_corner, reverse_clock_45);
            gstLdcInputParm.pstMatrix = M_mul(intermediate_result, upper_left_corner_matrix);
            M_free(upper_left_corner_matrix);
            M_free(reverse_clock_45);
            M_free(lower_right_corner);
            M_free(intermediate_result);
            break;
        }
        case E_MIRROR: {
            // Sets the matrix entry value
            MATRIX_TYPE _mirror_scaling[3][3] = {{-1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
            MATRIX_TYPE _right_shift[3][3]    = {{1, 0, 1920}, {0, 1, 0}, {0, 0, 1}};

            // Constructive matrix(step0 and step1)
            Matrix *mirror_scaling = Matrix_gen(3, 3, (MATRIX_TYPE *)_mirror_scaling);
            Matrix *right_shift    = Matrix_gen(3, 3, (MATRIX_TYPE *)_right_shift);

            // Matrix multiplication(step1 · step0)
            gstLdcInputParm.pstMatrix = M_mul(right_shift, mirror_scaling);
            M_free(mirror_scaling);
            M_free(right_shift);
            break;
        }
        case E_NONE:
            return NULL;
    }

    printf("Original matrix:\n");
    M_print(gstLdcInputParm.pstMatrix);
    return NULL;
}

void *ST_MatrixAdaptive()
{
    // step0: Inverse
    gstLdcInputParm.pstMatrix = M_Inverse(gstLdcInputParm.pstMatrix);
    printf("Inverse matrix:\n");
    M_print(gstLdcInputParm.pstMatrix);

    // step1: Quantify
    gstLdcInputParm.pstMatrix = M_numul(gstLdcInputParm.pstMatrix, 1 / (gstLdcInputParm.pstMatrix->data[8]));
    printf("Quantify matrix:\n");
    M_print(gstLdcInputParm.pstMatrix);

    // step2: Fixed-point
    gstLdcInputParm.pstMatrix = M_numul(gstLdcInputParm.pstMatrix, pow(2, 25));
    printf("Fixed-point matrix:\n");
    M_print(gstLdcInputParm.pstMatrix);

    return NULL;
}

void *ST_MatrixSet()
{
    for (int i = 0; i < 9; i++)
    {
        gstLdcInputParm.as64PMFCoef[i] = (MI_S64)ceil(gstLdcInputParm.pstMatrix->data[i]);
    }

    printf("Set to API:\n");
    int i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            printf("%-17lld", gstLdcInputParm.as64PMFCoef[i * (3) + j]);
        }
        printf("\n");
    }
    printf("\n");

    return NULL;
}

void *ST_MatrixFree()
{
    M_free(gstLdcInputParm.pstMatrix);

    return NULL;
}

MI_S32 ST_Ldc_GetCmdlineParam(int argc, char **argv)
{
    gstLdcInputParm.u8SensorIndex = 0xFF; // default user input
    gstLdcInputParm.bWithDpu      = TRUE;

    gstLdcInputParm.stTransformParam.eTransformType = E_NONE;

    // Default translation: Perspective
    gstLdcInputParm.as64PMFCoef[0] = 32329500;
    gstLdcInputParm.as64PMFCoef[1] = -2936656;
    gstLdcInputParm.as64PMFCoef[2] = 548188658;
    gstLdcInputParm.as64PMFCoef[3] = 20045;
    gstLdcInputParm.as64PMFCoef[4] = 31289435;
    gstLdcInputParm.as64PMFCoef[5] = 219428231;
    gstLdcInputParm.as64PMFCoef[6] = -745;
    gstLdcInputParm.as64PMFCoef[7] = -3376;
    gstLdcInputParm.as64PMFCoef[8] = 33554432;

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
        else if (0 == strcmp(argv[i], "move"))
        {
            gstLdcInputParm.stTransformParam.eTransformType = E_MOVE;
            gstLdcInputParm.stTransformParam.move_x         = atoi(argv[i + 1]);
            gstLdcInputParm.stTransformParam.move_y         = atoi(argv[i + 2]);
            return MI_SUCCESS;
        }
        else if (0 == strcmp(argv[i], "rotate"))
        {
            gstLdcInputParm.stTransformParam.eTransformType = E_ROTATE;
            gstLdcInputParm.stTransformParam.rotate         = -1 * atoi(argv[i + 1]);
            if (gstLdcInputParm.stTransformParam.rotate > 360 || gstLdcInputParm.stTransformParam.rotate < -360)
            {
                printf("Range of rotation angles:[-360,360]");
                return -1;
            }
            return MI_SUCCESS;
        }
        else if (0 == strcmp(argv[i], "scale"))
        {
            gstLdcInputParm.stTransformParam.eTransformType = E_SCALE;
            gstLdcInputParm.stTransformParam.scale_x        = atof(argv[i + 1]);
            gstLdcInputParm.stTransformParam.scale_y        = atof(argv[i + 2]);
            if (gstLdcInputParm.stTransformParam.scale_x <= 0 || gstLdcInputParm.stTransformParam.scale_y <= 0)
            {
                printf("The scaling value must be greater than 0\n");
                return -1;
            }
            return MI_SUCCESS;
        }
        else if (0 == strcmp(argv[i], "comb1"))
        {
            gstLdcInputParm.stTransformParam.eTransformType = E_CENTER_ROTATION;
            return MI_SUCCESS;
        }
        else if (0 == strcmp(argv[i], "comb2"))
        {
            gstLdcInputParm.stTransformParam.eTransformType = E_MIRROR;
            return MI_SUCCESS;
        }
    }

    return MI_SUCCESS;
}

static MI_S32 ST_PMF_Pipeline_Preview()
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

    STCHECKRESULT(STUB_BaseModuleInit_PMF_Preview(gstLdcInputParm.u8SensorIndex));

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
    stStreamInfo.u32Width     = gstSensorSize.u16Width;
    stStreamInfo.u32Height    = gstSensorSize.u16Height;
    stStreamInfo.u32FrameRate = 20;
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
        ST_Stitch_Mode_Usage(argv);
        return -1;
    }

    STCHECKRESULT(ST_Ldc_GetCmdlineParam(argc, argv));

    // Generate the original matrix from the input parameters
    ST_MatrixStructure();

    // Adapt matrix to the SigmaStar Soc platform hardware
    ST_MatrixAdaptive();

    // Set matrix to LDC PMF API
    ST_MatrixSet();

    // Free matrix
    ST_MatrixFree();

    STCHECKRESULT(ST_PMF_Pipeline_Preview());

    memset(&gstLdcInputParm, 0, sizeof(ST_LdcInputParam_t));
    return 0;
}
