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
#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "list.h"
#include "mi_ipu.h"
#include "mi_ipu_datatype.h"
#include "st_common_venc.h"
#include "algo_hseg_api.h"
#include "st_common_rtsp_video.h"

#define ipu_path "/config/dla/ipu_firmware.bin"
#define model_path "./resource/input/hseg_y36.img"
#define CHANGE_NUM 4
#define PIPELINE_FPS 30

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

typedef struct ST_BGB_Node
{
    struct list_head  list;
    MI_SYS_BufInfo_t  stSclBufInfoOri;
    MI_SYS_BUF_HANDLE hSclHandleOri;
    MI_SYS_BufInfo_t  stSclBufInfoBg;
    MI_SYS_BUF_HANDLE hSclHandleBg;
    MI_SYS_BufInfo_t  stSclBufInfoDupBg;
    MI_SYS_BUF_HANDLE hSclHandleDupBg;
} ST_BGB_Node;

MI_U8 g_u8SensorIndex;
char  g_IqBinPath[128];
MI_U8 g_u8Duration = 0;

MI_U8 g_IveBgbDegree[CHANGE_NUM][2];

LIST_HEAD(g_HeadListIve);
LIST_HEAD(g_HeadListDla);

pthread_t g_pThreadBgbScl;
MI_BOOL   g_bThreadExitScl = TRUE;
pthread_t g_pThreadBgbDla;
MI_BOOL   g_bThreadExitDla = TRUE;
pthread_t g_pThreadBgbIve;
MI_BOOL   g_bThreadExitIve = TRUE;

pthread_mutex_t ListMutexIve = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ListMutexDla = PTHREAD_MUTEX_INITIALIZER;

void *g_hIpuHandle = 0;

static ST_SensorSize_t g_stSensorSize;

static MI_S32 STUB_IveBgbDegreeInit()
{
    for (int i = 0; i < CHANGE_NUM; i++)
    {
        g_IveBgbDegree[i][0] = 0;
        g_IveBgbDegree[i][1] = 16 / CHANGE_NUM * (i + 1) - 1;
    }
    g_IveBgbDegree[0][1] = 1;
    g_IveBgbDegree[1][1] = 5;

    return MI_SUCCESS;
} // CHANGE_NUM = 4 *********** {(0,1),(0,5),(0,11),(0,15)}

static MI_S32 STUB_BaseModuleInit()
{
    MI_SNR_PlaneInfo_t stPlaneInfo;
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;

    MI_U32 u32SnrPadId   = 0;
    MI_U8  u8SensorRes   = g_u8SensorIndex;
    MI_U32 u32VifGroupId = 0;
    MI_U32 u32VifDevId   = 0;
    MI_U32 u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId   = 0;
    MI_U32 u32SclChnId   = 0;
    MI_U32 u32SclPortId0 = 0;
    MI_U32 u32SclPortId1 = 1;

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
    g_stSensorSize.u16Width  = stPlaneInfo.stCapRect.u16Width;
    g_stSensorSize.u16Height = stPlaneInfo.stCapRect.u16Height;

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
    stIspChnParam.eRot = E_MI_SYS_ROTATE_180;
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
    u32SrcFrmrate          = PIPELINE_FPS;
    u32DstFrmrate          = PIPELINE_FPS;
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step5 :init scl
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

    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam); // E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1;

    /**********scl port0*********/
    stSclOutPortParam.stSCLOutputSize.u16Width  = g_stSensorSize.u16Width;
    stSclOutPortParam.stSCLOutputSize.u16Height = g_stSensorSize.u16Height;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId0, &stSclOutPortParam));

    /**********scl port1*********/
    stSclOutPortParam.stSCLOutputSize.u16Width  = 640;
    stSclOutPortParam.stSCLOutputSize.u16Height = 360;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId1, &stSclOutPortParam));

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
    //*****************************//
    stDstChnPort.u32PortId = u32SclPortId0;
    u32SrcFrmrate          = PIPELINE_FPS; // 30
    u32DstFrmrate          = PIPELINE_FPS; // 30
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 0, 4));

    /************************************************
    step7 :init ipu
    *************************************************/
    InitHsegParam_t initParam;
    memcpy(initParam.ipu_firware_bin, ipu_path, strlen(ipu_path));
    memcpy(initParam.seg_model_path, model_path, strlen(model_path) + 1);

    STCHECKRESULT(ALGO_HSEG_CreateHandle(&g_hIpuHandle));
    STCHECKRESULT(ALGO_HSEG_Init(g_hIpuHandle, initParam));

    /************************************************
    step7 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;
    MI_VENC_RcParam_t           stRcParam;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;

    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);

    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = PIPELINE_FPS;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth        = g_stSensorSize.u16Width;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight       = g_stSensorSize.u16Height;
    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate =
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth * stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 1.3;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

    MI_VENC_GetRcParam(u32VencDevId, u32VencChnId, &stRcParam);

    stRcParam.stParamH265Cbr.u32MaxQp  = 30;
    stRcParam.stParamH265Cbr.u32MaxIQp = 30;

    MI_VENC_SetRcParam(u32VencDevId, u32VencChnId, &stRcParam);

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

    MI_U32 u32SclDevId   = 0;
    MI_U32 u32SclChnId   = 0;
    MI_U32 u32SclPortId0 = 0;
    MI_U32 u32SclPortId1 = 1;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    step1 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step2 :deinit ive and ipu
    *************************************************/
    ALGO_HSEG_DeInit(g_hIpuHandle);
    ALGO_HSEG_ReleaseHandle(g_hIpuHandle);

    /************************************************
    step3 :unbind isp->scl
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
    stDstChnPort.u32PortId = u32SclPortId0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step4 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId0));
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId1));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step5 :unbind vif->isp
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
    step6 :deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));
    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    step7 :deinit vif/sensor
    *************************************************/
    STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
    STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));
    STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
    STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId));

    /************************************************
    step8 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

void *ST_SCL_Task()
{
    MI_S32 s32Ret = MI_SUCCESS;

    MI_U8  u8SclDev  = 0x0;
    MI_S32 s32SclChn = 0;

    struct timeval tv;
    MI_S32         s32FdSclPort0 = -1;
    MI_S32         s32FdSclPort1 = -1;
    fd_set         ReadFdsSclPort0;
    fd_set         ReadFdsSclPort1;

    //*******scl port0*********
    MI_SYS_ChnPort_t  stSclChnPort0;
    MI_SYS_BufInfo_t  stSclBufInfo0;
    MI_SYS_BUF_HANDLE hSclBufHandle0;

    //*******scl port1*********
    MI_SYS_ChnPort_t  stSclChnPort1;
    MI_SYS_BufInfo_t  stSclBufInfo1;
    MI_SYS_BUF_HANDLE hSclBufHandle1;
    MI_SYS_BUF_HANDLE hSclBufHandleDup1;

    memset(&stSclChnPort0, 0x0, sizeof(MI_SYS_ChnPort_t));

    stSclChnPort0.eModId    = E_MI_MODULE_ID_SCL;
    stSclChnPort0.u32DevId  = u8SclDev;
    stSclChnPort0.u32ChnId  = s32SclChn;
    stSclChnPort0.u32PortId = 0x0;

    MI_SYS_SetChnOutputPortDepth(0, &stSclChnPort0, 4, 5);

    stSclChnPort1.eModId    = E_MI_MODULE_ID_SCL;
    stSclChnPort1.u32DevId  = u8SclDev;
    stSclChnPort1.u32ChnId  = s32SclChn;
    stSclChnPort1.u32PortId = 0X1;
    MI_SYS_SetChnOutputPortDepth(0, &stSclChnPort1, 4, 5);

    if (MI_SUCCESS != (s32Ret = MI_SYS_GetFd(&stSclChnPort0, &s32FdSclPort0)))
    {
        printf("GET scl Fd Failed , scl Chn = %d\n", s32SclChn);
        return NULL;
    }
    printf("Get scl Fd = %d, scl Chn = %d\n ", s32FdSclPort0, s32SclChn);
    if (MI_SUCCESS != (s32Ret = MI_SYS_GetFd(&stSclChnPort1, &s32FdSclPort1)))
    {
        printf("GET scl Fd Failed , scl Chn = %d\n", s32SclChn);
        return NULL;
    }
    printf("Get scl Fd = %d, scl Chn = %d\n ", s32FdSclPort1, s32SclChn);

    ///*******g_bThreadExitScl*********//
    while (g_bThreadExitScl == TRUE)
    {
        ST_BGB_Node *ListSclToDla;
        ListSclToDla = (ST_BGB_Node *)malloc(sizeof(ST_BGB_Node));
        ST_BGB_Node *ListIpuToIve;
        ListIpuToIve = (ST_BGB_Node *)malloc(sizeof(ST_BGB_Node));

        FD_ZERO(&ReadFdsSclPort0);
        FD_SET(s32FdSclPort0, &ReadFdsSclPort0);

        tv.tv_sec = 2;

        s32Ret = select(s32FdSclPort0 + 1, &ReadFdsSclPort0, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("ive_bgb_Task%d select failed\n", s32SclChn);
            sleep(1);
            break;
        }
        else if (0 == s32Ret)
        {
            continue;
        }
        else
        {
            memset(&stSclBufInfo0, 0x0, sizeof(MI_SYS_BufInfo_t));
            if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stSclChnPort0, &stSclBufInfo0, &hSclBufHandle0))
            {
                memcpy(&ListIpuToIve->stSclBufInfoOri, &stSclBufInfo0, sizeof(MI_SYS_BufInfo_t));
                ListIpuToIve->hSclHandleOri = hSclBufHandle0;
            }
        }
        FD_ZERO(&ReadFdsSclPort1);
        FD_SET(s32FdSclPort1, &ReadFdsSclPort1);

        s32Ret = select(s32FdSclPort1 + 1, &ReadFdsSclPort1, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("ive_bgb_Task%d select failed\n", s32SclChn);
            sleep(1);
            MI_SYS_ChnOutputPortPutBuf(hSclBufHandle0);
            break;
        }
        else if (0 == s32Ret)
        {
            printf("ive_bgb_Task%d select timeout\n", s32SclChn);
            MI_SYS_ChnOutputPortPutBuf(hSclBufHandle0);
            break;
        }
        else
        {
            memset(&stSclBufInfo0, 0x0, sizeof(MI_SYS_BufInfo_t));
            if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stSclChnPort1, &stSclBufInfo1, &hSclBufHandle1))
            {
                memcpy(&ListIpuToIve->stSclBufInfoBg, &stSclBufInfo1, sizeof(MI_SYS_BufInfo_t));
                ListIpuToIve->hSclHandleBg = hSclBufHandle1;

                pthread_mutex_lock(&ListMutexIve);
                list_add_tail(&ListIpuToIve->list, &g_HeadListIve);
                pthread_mutex_unlock(&ListMutexIve);

                if (MI_SUCCESS == MI_SYS_DupBuf(hSclBufHandle1, &hSclBufHandleDup1))
                {
                    memcpy(&ListSclToDla->stSclBufInfoDupBg, &stSclBufInfo1, sizeof(MI_SYS_BufInfo_t));
                    ListSclToDla->hSclHandleDupBg = hSclBufHandleDup1;

                    pthread_mutex_lock(&ListMutexDla);
                    list_add_tail(&ListSclToDla->list, &g_HeadListDla);
                    pthread_mutex_unlock(&ListMutexDla);
                }
            }
            else
            {
                printf("get 4k ok, get 640x360 not ok \n");
                MI_SYS_ChnOutputPortPutBuf(hSclBufHandle0);
                continue;
            }
        }
    }
    printf("g_bThreadExitScl == end!!!\n ");
    MI_SYS_CloseFd(s32FdSclPort1);
    FD_ZERO(&ReadFdsSclPort1);
    MI_SYS_CloseFd(s32FdSclPort0);
    FD_ZERO(&ReadFdsSclPort0);

    return NULL;
}

void *ST_DLA_Task()
{
    MI_SYS_BUF_HANDLE   hDlaBufHandle;
    AlgoHsegInputInfo_t stDlaInputBufInfo;
    MI_BOOL             bGetBufListDla = 0;
    while (g_bThreadExitDla == TRUE)
    {
        bGetBufListDla = 0;
        pthread_mutex_lock(&ListMutexDla);
        if (!(list_empty(&g_HeadListDla)))
        {
            ST_BGB_Node *DlaReadListNode;
            DlaReadListNode = list_first_entry(&g_HeadListDla, ST_BGB_Node, list);

            stDlaInputBufInfo.bufsize = 640 * 360 * 3 / 2;
            ;
            stDlaInputBufInfo.pt_tensor_data[0]  = DlaReadListNode->stSclBufInfoDupBg.stFrameData.pVirAddr[0];
            stDlaInputBufInfo.phy_tensor_addr[0] = DlaReadListNode->stSclBufInfoDupBg.stFrameData.phyAddr[0];
            hDlaBufHandle                        = DlaReadListNode->hSclHandleDupBg;

            bGetBufListDla = 1;
            list_del(&(DlaReadListNode->list));
            free(DlaReadListNode);
        }
        pthread_mutex_unlock(&ListMutexDla);
        if (bGetBufListDla)
        {
            if (MI_SUCCESS != ALGO_HSEG_SendInput(g_hIpuHandle, &stDlaInputBufInfo))
            {
                ST_ERR("Segment failed \n");
            }
            MI_SYS_ChnOutputPortPutBuf(hDlaBufHandle);
        }
    }
    return NULL;
}

void *ST_IVE_Task()
{
    MI_S32 s32Ret      = MI_SUCCESS;
    MI_U8  u8Cnt       = 0;
    MI_U8  u8BgbOption = 0;

    MI_SYS_ChnPort_t  stVencChnPort;
    MI_SYS_BufInfo_t  stVencBufInfo;
    MI_SYS_BUF_HANDLE hVencBufHandle;
    MI_SYS_BufConf_t  stVencBufConf;
    MI_U32            SrcBufSizeOri = g_stSensorSize.u16Width * g_stSensorSize.u16Height;

    stVencChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stVencChnPort.u32DevId  = 0;
    stVencChnPort.u32ChnId  = 0x0;
    stVencChnPort.u32PortId = 0x0;

    ALGO_HsegBgBlurCtrl_t stBgbCtrl;
    stBgbCtrl.bgblur_mode    = E_ALOG_BGBLUR_MODE_BLUR;
    stBgbCtrl.mask_thredhold = 127;
    stBgbCtrl.blur_level     = 0;
    stBgbCtrl.scaling_stage  = 8;
    stBgbCtrl.maskOp         = E_ALGO_BGB_MASK_OP_DILATE;

    AlgoHsegInputInfo_t stDst;
    stDst.data_type = E_ALOG_YUV420SP;
    stDst.width     = g_stSensorSize.u16Width;
    stDst.height    = g_stSensorSize.u16Height;
    MI_SYS_MMA_Alloc(0, NULL, SrcBufSizeOri, &stDst.phy_tensor_addr[0]);
    MI_SYS_Mmap(stDst.phy_tensor_addr[0], SrcBufSizeOri, &stDst.pt_tensor_data[0], TRUE);
    MI_SYS_MMA_Alloc(0, NULL, SrcBufSizeOri / 2, &stDst.phy_tensor_addr[1]);
    MI_SYS_Mmap(stDst.phy_tensor_addr[1], SrcBufSizeOri / 2, &stDst.pt_tensor_data[1], TRUE);

    MI_SYS_BUF_HANDLE   hIveBufHandleOri;
    AlgoHsegInputInfo_t stSrcOri;
    stSrcOri.width  = g_stSensorSize.u16Width;
    stSrcOri.height = g_stSensorSize.u16Height;

    MI_SYS_BUF_HANDLE   hIveBufHandleRep;
    AlgoHsegInputInfo_t stSrcRep;
    stSrcRep.width  = 640;
    stSrcRep.height = 360;

    MI_BOOL bResultFlag    = 0;
    MI_BOOL bGetBufListIve = 0;

    ///*******g_bThreadExitIve*********//
    while (g_bThreadExitIve == TRUE)
    {
        if (g_u8Duration >= 0)
        {
            if (g_u8Duration > 0)
            {
                if (u8Cnt > g_u8Duration * PIPELINE_FPS)
                {
                    u8Cnt = 0;
                    u8BgbOption++;
                }
                if (u8BgbOption == CHANGE_NUM)
                {
                    u8BgbOption = 0;
                }
                stBgbCtrl.blur_level    = g_IveBgbDegree[u8BgbOption][0];
                stBgbCtrl.scaling_stage = g_IveBgbDegree[u8BgbOption][1];
            }
            bGetBufListIve = 0;
            memset(&stVencBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
            pthread_mutex_lock(&ListMutexIve);
            if (!(list_empty(&g_HeadListIve)))
            {
                ST_BGB_Node *IveReadListNode;
                IveReadListNode = list_first_entry(&g_HeadListIve, ST_BGB_Node, list);

                stSrcOri.phy_tensor_addr[0] = IveReadListNode->stSclBufInfoOri.stFrameData.phyAddr[0];
                stSrcOri.phy_tensor_addr[1] = IveReadListNode->stSclBufInfoOri.stFrameData.phyAddr[1];
                stSrcOri.pt_tensor_data[0]  = IveReadListNode->stSclBufInfoOri.stFrameData.pVirAddr[0];
                stSrcOri.pt_tensor_data[1]  = IveReadListNode->stSclBufInfoOri.stFrameData.pVirAddr[1];
                stSrcOri.data_type          = E_ALOG_YUV420SP;
                hIveBufHandleOri            = IveReadListNode->hSclHandleOri;

                stSrcRep.phy_tensor_addr[0] = IveReadListNode->stSclBufInfoBg.stFrameData.phyAddr[0];
                stSrcRep.phy_tensor_addr[1] = IveReadListNode->stSclBufInfoBg.stFrameData.phyAddr[1];
                stSrcRep.pt_tensor_data[0]  = IveReadListNode->stSclBufInfoBg.stFrameData.pVirAddr[0];
                stSrcRep.pt_tensor_data[1]  = IveReadListNode->stSclBufInfoBg.stFrameData.pVirAddr[1];
                stSrcRep.data_type          = E_ALOG_YUV420SP;
                hIveBufHandleRep            = IveReadListNode->hSclHandleBg;

                stVencBufConf.eBufType                  = IveReadListNode->stSclBufInfoOri.eBufType;
                stVencBufConf.u64TargetPts              = IveReadListNode->stSclBufInfoOri.u64Pts;
                stVencBufConf.stFrameCfg.eFrameScanMode = IveReadListNode->stSclBufInfoOri.stFrameData.eFrameScanMode;
                stVencBufConf.stFrameCfg.eFormat        = IveReadListNode->stSclBufInfoOri.stFrameData.ePixelFormat;
                stVencBufConf.stFrameCfg.u16Width       = IveReadListNode->stSclBufInfoOri.stFrameData.u16Width;
                stVencBufConf.stFrameCfg.u16Height      = IveReadListNode->stSclBufInfoOri.stFrameData.u16Height;
                bGetBufListIve                          = 1;

                list_del(&(IveReadListNode->list));
                free(IveReadListNode);
            }
            pthread_mutex_unlock(&ListMutexIve);
            if (bGetBufListIve)
            {
                if (MI_SUCCESS
                    == MI_SYS_ChnInputPortGetBuf(&stVencChnPort, &stVencBufConf, &stVencBufInfo, &hVencBufHandle, 0))
                {
                    stDst.phy_tensor_addr[0] = stVencBufInfo.stFrameData.phyAddr[0];
                    stDst.phy_tensor_addr[1] = stVencBufInfo.stFrameData.phyAddr[1];

                    do
                    {
                        s32Ret = ALGO_HSEG_SegmentAndBlurBackgroud(g_hIpuHandle, &stSrcOri, &stSrcRep, &stDst,
                                                                   stBgbCtrl, &bResultFlag);
                        if (s32Ret != 0 && s32Ret != -1)
                        {
                            ST_ERR("Segment failed \n");
                        }
                        else
                        {
                            u8Cnt++;
                        }
                    } while (s32Ret == -1);

                    MI_SYS_ChnInputPortPutBuf(hVencBufHandle, &stVencBufInfo, FALSE);
                }
                MI_SYS_ChnOutputPortPutBuf(hIveBufHandleOri);
                MI_SYS_ChnOutputPortPutBuf(hIveBufHandleRep);
            }
        }
    }
    MI_SYS_Munmap((void *)stDst.pt_tensor_data[0], SrcBufSizeOri);
    MI_SYS_MMA_Free(0, stDst.phy_tensor_addr[0]);
    MI_SYS_Munmap((void *)stDst.pt_tensor_data[1], SrcBufSizeOri / 2);
    MI_SYS_MMA_Free(0, stDst.phy_tensor_addr[1]);
    printf("g_bThreadExitIve == end!!!\n ");

    return NULL;
}

static MI_S32 ST_BGB_Pipeline_Preview()
{
    MI_U32             u32IspDevId  = 0;
    MI_U32             u32IspChnId  = 0;
    MI_U32             u32SensorPad = 0;
    char               IqApiBinFilePath[128];
    MI_SNR_PlaneInfo_t stPlaneInfo;
    MI_U32             u32VencDevId = 0;
    MI_U32             u32VencChnId = 0;

    STCHECKRESULT(STUB_BaseModuleInit());

    if (strlen(g_IqBinPath) == 0)
    {
        MI_SNR_GetPlaneInfo(u32SensorPad, 0, &stPlaneInfo);
        sprintf(IqApiBinFilePath, "/config/iqfile/%s_api.bin", stPlaneInfo.s8SensorName);
    }
    else
    {
        strcpy(IqApiBinFilePath, g_IqBinPath);
    }
    ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, IqApiBinFilePath);

    pthread_create(&g_pThreadBgbScl, NULL, ST_SCL_Task, NULL);
    pthread_create(&g_pThreadBgbDla, NULL, ST_DLA_Task, NULL);
    pthread_create(&g_pThreadBgbIve, NULL, ST_IVE_Task, NULL);

    ST_VideoStreamInfo_t stStreamInfo;
    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32VencChnId;
    stStreamInfo.u32Width     = g_stSensorSize.u16Width;
    stStreamInfo.u32Height    = g_stSensorSize.u16Height;
    stStreamInfo.u32FrameRate = PIPELINE_FPS;
    stStreamInfo.rtspIndex    = 0;

    // start rtsp
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    ST_Common_Pause();

    g_bThreadExitScl = FALSE;
    pthread_join(g_pThreadBgbScl, NULL);

    while (!(list_empty(&g_HeadListDla)))
    {
        continue;
    }
    g_bThreadExitDla = FALSE;
    pthread_join(g_pThreadBgbDla, NULL);

    while (!(list_empty(&g_HeadListIve)))
    {
        continue;
    }
    g_bThreadExitIve = FALSE;
    pthread_join(g_pThreadBgbIve, NULL);

    usleep(1 * 1000 * 1000);

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);
    STCHECKRESULT(STUB_BaseModuleDeInit());

    return MI_SUCCESS;
}

void ST_Ive_Usage(void)
{
    printf("Usage:./prog_ive_bgb_pipeline ) single sensor bgb pipeline vif->isp->scl->ipu->ive->venc->rtsp\n");
    printf("Usage:./prog_ive_bgb_pipeline index x) set sensor Resindex,index 7 -> 4K@30\n");
    printf("Usage:./prog_ive_bgb_pipeline time  x) set the duration of blur level switching");
}

MI_S32 ST_Ive_GetCmdlineParam(int argc, char **argv)
{
    g_u8SensorIndex = 0xFF; // default user input
    memset(g_IqBinPath, 0, 128);
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            g_u8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "iqbin"))
        {
            strcpy(g_IqBinPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "time"))
        {
            g_u8Duration = atoi(argv[i + 1]);
        }
    }

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    if (argc < 1)
    {
        ST_Ive_Usage();
        return -1;
    }
    STUB_IveBgbDegreeInit();
    ST_Ive_GetCmdlineParam(argc, argv);

    ST_BGB_Pipeline_Preview();

    memset(&g_stSensorSize, 0x00, sizeof(ST_SensorSize_t));
    return 0;
}