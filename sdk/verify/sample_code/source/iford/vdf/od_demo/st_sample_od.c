/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/prctl.h>
#include <errno.h>
#include <assert.h>
#include "mi_common_datatype.h"
#include "mi_scl.h"
#include "mi_vdf.h"
#include "mi_vdf_datatype.h"
#include "mi_sys_datatype.h"
#include "mi_sys.h"
#include "mi_md.h"
#include "mi_od.h"
#include "mi_vg.h"
#include "mi_rgn.h"
#include "st_common.h"
#include "st_common_vdf.h"
#include "st_common_scl.h"
#include "st_common_rgn.h"
#include "st_common_venc.h"
#include "st_common_font.h"

#define WINDOW_INDEX_COL_MAX        3
#define WINDOW_INDEX_ROW_MAX        3
#define RGN_FRAME_TOTAL_WIDTH       8192
#define RGN_FRAME_TOTAL_HEIGHT      8192
#define INPUT_WIDTH                 640
#define INPUT_HEIGHT                360
#define INPUT_PATH                  "./resource/input/od_test_640x360_nv12.yuv"
#define OUTPUT_CNT                  32

extern int pthread_setname_np(pthread_t thread, const char *name);
static ST_Common_InputFile_Attr_t g_stSclInputFileAttr;
static ST_Common_OutputFile_Attr_t g_stVencOutputFileAttr;

static MI_U8            g_u8CmdIndex = 0;
static MI_RGN_HANDLE    g_hRgnHandleCover[WINDOW_INDEX_COL_MAX * WINDOW_INDEX_ROW_MAX];
static MI_RGN_ChnPort_t g_stRgnChnPort;

static MI_VDF_CHANNEL   g_VdfOdChn = VDF_OD_CHN_NO;
static MI_BOOL          g_bOdExit = FALSE;
static MI_S32           g_pThreadOdState = -1;
static pthread_t        g_pThreadOd;

static MI_U16           g_u16ImageWidth = INPUT_WIDTH;
static MI_U16           g_u16ImageHeight = INPUT_HEIGHT;

static FILE             *g_pOutFile;
char                    g_strOdResult[50];
MI_U32                  g_u32DumpLogCnt = 0;

MI_S32                  g_s32VdfFD;

MI_S32 ST_Common_RgnInitFrameWindows_3x3(MI_RGN_ChnPort_t* stRgnChnPort)
{
    int i, j;
    MI_S32 s32Ret;
    MI_U32 x, y, u32HandleIndex;
    MI_RGN_Attr_t stRegionAttr;
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.eType = E_MI_RGN_TYPE_FRAME;

    MI_RGN_ChnPortParam_t stRgnChnPortParam;
    stRgnChnPortParam.bShow = 0;
    stRgnChnPortParam.u32Layer = 0;
    stRgnChnPortParam.stFrameChnPort.u8Thickness = 3;
    stRgnChnPortParam.stFrameChnPort.u32Color = 0XFF0000;

    stRgnChnPortParam.stFrameChnPort.stRect.u32Width  = RGN_FRAME_TOTAL_WIDTH / 3;
    stRgnChnPortParam.stFrameChnPort.stRect.u32Height = RGN_FRAME_TOTAL_HEIGHT / 3;
    for(i = 0; i < WINDOW_INDEX_COL_MAX; i++)
    {
        for(j = 0; j < WINDOW_INDEX_ROW_MAX; j++)
        {
            x = j * g_u16ImageWidth / 3;
            y = i * g_u16ImageHeight / 3;
            u32HandleIndex = 3 * i + j;
            g_hRgnHandleCover[u32HandleIndex] = u32HandleIndex;
            s32Ret = ST_Common_RgnCreate(0, g_hRgnHandleCover[u32HandleIndex], &stRegionAttr);
            if(s32Ret != MI_SUCCESS)
            {
                ST_ERR("Create Rgn failed \n");
                return -1;
            }
            stRgnChnPortParam.stFrameChnPort.stRect.s32X = x * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;
            stRgnChnPortParam.stFrameChnPort.stRect.s32Y = y * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
            STCHECKRESULT(ST_Common_RgnAttachChn(0, g_hRgnHandleCover[u32HandleIndex], stRgnChnPort, &stRgnChnPortParam));
        }
    }
    return MI_SUCCESS;
}

MI_S32 ODtoRECT(MI_U8 u32Chn, MI_OD_Result_t *pstOdResult)
{
    int i, j;
    MI_U32 u32HandleIndex;
    MI_S32 s32Ret;
    MI_RGN_ChnPortParam_t stTmpRgnChnPortParam;
    MI_RGN_ChnPort_t stTmpRgnChnPort;
    memset(&stTmpRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stTmpRgnChnPort.eModId = E_MI_MODULE_ID_VENC;
    stTmpRgnChnPort.s32DevId = 0;
    stTmpRgnChnPort.s32ChnId = 0;
    stTmpRgnChnPort.s32PortId = 0;

    if(g_u32DumpLogCnt < OUTPUT_CNT)
    {
        sprintf(g_strOdResult, "u8RgnAlarm%d:{", g_u32DumpLogCnt+1);
        fprintf(g_pOutFile, "%s", g_strOdResult);
    }
    for (i = 0; i < sizeof(pstOdResult->u8RgnAlarm) / sizeof(pstOdResult->u8RgnAlarm[0]); i++)
    {
        for (j = 0; j < sizeof(pstOdResult->u8RgnAlarm[0]) / sizeof(pstOdResult->u8RgnAlarm[0][0]); j++)
        {
            u32HandleIndex = 3 * i + j;
            g_hRgnHandleCover[u32HandleIndex] = u32HandleIndex;
            if(g_u32DumpLogCnt < OUTPUT_CNT)
            {
                sprintf(g_strOdResult, " %d, ", pstOdResult->u8RgnAlarm[i][j]);
                fprintf(g_pOutFile, "%s", g_strOdResult);
            }
            if(pstOdResult->u8RgnAlarm[i][j] == 1)
            {
                s32Ret = MI_RGN_GetDisplayAttr(0, g_hRgnHandleCover[u32HandleIndex], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_GetDisplayAttr err!!!\n");
                    return -1;
                }
                stTmpRgnChnPortParam.bShow = 1;
                s32Ret = MI_RGN_SetDisplayAttr(0, g_hRgnHandleCover[u32HandleIndex], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_SetDisplayAttr err!!!\n");
                    return -1;
                }
            }
            else
            {
                s32Ret = MI_RGN_GetDisplayAttr(0, g_hRgnHandleCover[u32HandleIndex], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_GetDisplayAttr err!!!\n");
                    return -1;
                }
                stTmpRgnChnPortParam.bShow = 0;
                s32Ret = MI_RGN_SetDisplayAttr(0, g_hRgnHandleCover[u32HandleIndex], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_SetDisplayAttr err!!!\n");
                    return -1;
                }
            }
        }
    }

    if(g_u32DumpLogCnt < OUTPUT_CNT)
    {
        sprintf(g_strOdResult, "}\n");
        fprintf(g_pOutFile, "%s", g_strOdResult);
    }
    g_u32DumpLogCnt++;

    return MI_SUCCESS;
}

void *ST_OD_Task(void *arg)
{
    MI_S32 s32Ret = 0;
    MI_S32 s32SelectRet = 0;

    MI_VDF_Result_t stVdfResult = {0};
    MI_U32 u32Chn = 0;
    MI_U8 u8VdfCount = 0;
    fd_set read_fds;

    struct timeval out_time;
    out_time.tv_sec = 5;

    FD_ZERO(&read_fds);
    FD_SET(g_s32VdfFD, &read_fds);

    printf("enter od task loop\n");
    while(!g_bOdExit)
    {
        memset(&stVdfResult, 0x00, sizeof(MI_VDF_Result_t));
        stVdfResult.enWorkMode = E_MI_VDF_WORK_MODE_OD;

        s32SelectRet = select(g_s32VdfFD + 1, &read_fds, NULL, NULL, &out_time);
        if(s32SelectRet == 0)
        {
            ST_ERR("OD TASK TIME OUT!\n");
            break;
        }

        s32Ret = MI_VDF_GetResult(g_VdfOdChn, &stVdfResult, 0);
        if((0 == s32Ret) && (1 == stVdfResult.stOdResult.u8Enable) && (u8VdfCount < OUTPUT_CNT))
        {
            s32Ret = ODtoRECT(u32Chn, &stVdfResult.stOdResult);
            if(s32Ret != 0)
            {
                ST_ERR("Rgn DisplayAttr err, ODtoRECT fail!\n");
            }
            u8VdfCount++;
            MI_VDF_PutResult(g_VdfOdChn, &stVdfResult);
        }

        if(u8VdfCount >= OUTPUT_CNT)
        {
            g_bOdExit = TRUE;
            break;
        }
    }
    memset(&stVdfResult, 0x00, sizeof(stVdfResult));
    ODtoRECT(u32Chn, &stVdfResult.stOdResult);

    return NULL;
}

static MI_S32 STUB_BaseModuleInit(void)
{
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;

    MI_U32 u32SclDevId = 1;
    MI_U32 u32SclChnId = 0;
    MI_U32 u32SclPortId0 = 0;
    MI_U32 u32SclPortId1 = 1;
    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    char *output_file = "out/vdf/od_result.txt";
    ST_Common_CheckMkdirOutFile(output_file);

    g_pOutFile = fopen(output_file, "w");
    if (g_pOutFile == NULL)
    {
        printf("Error: Can't open out/vdf/od_result.txt\n");
        return -1;
    }

    /************************************************
    step2 :init SCL
    *************************************************/
    MI_SCL_DevAttr_t stSclDevAttr;
    MI_SCL_ChannelAttr_t stSclChnAttr;
    MI_SCL_ChnParam_t stSclChnParam;
    MI_SYS_WindowRect_t stSclInputCrop;
    MI_SCL_OutPortParam_t  stSclOutPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);        //E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1;

    stSclOutPortParam.stSCLOutputSize.u16Width  = INPUT_WIDTH;
    stSclOutPortParam.stSCLOutputSize.u16Height = INPUT_HEIGHT;

    /**********scl port0*********/
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId0, &stSclOutPortParam));

    /**********scl port1*********/
    stSclOutPortParam.ePixelFormat  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId1, &stSclOutPortParam));

    /************************************************
    step2 :init VENC
    *************************************************/
    MI_VENC_DEV                 u32VencDevId = 0;
    MI_VENC_CHN                 u32VencChnId = 0;
    MI_VENC_ModType_e           eType    = E_MI_VENC_MODTYPE_H264E;
    MI_VENC_InitParam_t         stVencParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;

    STCHECKRESULT(ST_Common_Sys_Init());
    ST_Common_GetVencDefaultDevAttr(&stVencParam);

    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencParam));
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);

    stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth         = g_u16ImageWidth;
    stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight        = g_u16ImageHeight;

    stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 15;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 12;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 12;

    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

    /************************************************
    step8 :bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 15;
    u32DstFrmrate          = 15;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step3 :init RGN
    *************************************************/
    MI_U16 u16SocId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;

    memset(&g_stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    g_stRgnChnPort.eModId = E_MI_MODULE_ID_VENC;
    g_stRgnChnPort.s32DevId = u32VencDevId;
    g_stRgnChnPort.s32ChnId = u32VencChnId;
    g_stRgnChnPort.s32PortId = 0;

    ST_Common_Rgn_SetImageSize(g_u16ImageWidth, g_u16ImageHeight);

    STCHECKRESULT(ST_Common_GetRgnDefaultInitAttr(&stPaletteTable));
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    if(MI_SUCCESS != ST_Common_RgnInitFrameWindows_3x3(&g_stRgnChnPort))
    {
        ST_ERR("Init Rgn Frame Windows failed\n");
        return -1;
    }

    /************************************************
    step4 :init VDF_OD
    *************************************************/
    STCHECKRESULT(MI_VDF_Init());
    MI_VDF_ChnAttr_t stVdfChnAttr;

    ST_Common_GetVdfOdDefaultChnAttr(&stVdfChnAttr, g_u16ImageWidth, g_u16ImageHeight);
    stVdfChnAttr.stOdAttr.u8OdBufCnt = 16;
    if (0 != ST_Common_VdfCheckAlign((MI_U16)MDMB_MODE_BUTT, stVdfChnAttr.stOdAttr.stOdStaticParamsIn.inImgStride))
    {
        ST_ERR("Vdf Check Align Err!\n");
        return -1;
    }
    /************************************************
    step8 :bind scl->vdf
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId1;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VDF;
    stDstChnPort.u32DevId  = 0;
    stDstChnPort.u32ChnId  = 0;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 15;
    u32DstFrmrate          = 15;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;

    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 0, 8);

    STCHECKRESULT(MI_VDF_CreateChn(g_VdfOdChn, &stVdfChnAttr));
    STCHECKRESULT(MI_VDF_Run(E_MI_VDF_WORK_MODE_OD));

    STCHECKRESULT(MI_VDF_EnableSubWindow(g_VdfOdChn, 0, 0, 1));

    MI_SYS_GetFd(&stDstChnPort, &g_s32VdfFD);

    /************************************************
    step5 :get venc input port Yuv420sp
    *************************************************/
    g_stVencOutputFileAttr.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;
    g_stVencOutputFileAttr.stModuleInfo.u32DevId = u32VencDevId;
    g_stVencOutputFileAttr.stModuleInfo.u32ChnId = u32VencChnId;
    g_stVencOutputFileAttr.stModuleInfo.u32PortId = 0;
    g_stVencOutputFileAttr.bThreadExit = FALSE;
    sprintf(g_stVencOutputFileAttr.FilePath, "./out/vdf/od_demo_case%d", g_u8CmdIndex);
    g_stVencOutputFileAttr.u16DumpBuffNum = OUTPUT_CNT;

    ST_Common_CheckResult(&g_stVencOutputFileAttr);

    g_stSclInputFileAttr.stModuleInfo.eModId = E_MI_MODULE_ID_SCL;
    g_stSclInputFileAttr.stModuleInfo.u32DevId = u32SclDevId;
    g_stSclInputFileAttr.stModuleInfo.u32ChnId = u32SclChnId;
    g_stSclInputFileAttr.stModuleInfo.u32PortId = 0;
    g_stSclInputFileAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    g_stSclInputFileAttr.u32Width = INPUT_WIDTH;
    g_stSclInputFileAttr.u32Height = INPUT_HEIGHT;
    g_stSclInputFileAttr.bThreadExit = FALSE;
    g_stSclInputFileAttr.bInputOnce = TRUE;
    g_stSclInputFileAttr.u32Fps  = 15;
    sprintf(g_stSclInputFileAttr.InputFilePath, INPUT_PATH);

    pthread_create(&g_stSclInputFileAttr.pPutDatathread, NULL, ST_Common_PutInputDataThread, (void *)&g_stSclInputFileAttr);

    g_pThreadOdState = pthread_create(&g_pThreadOd, NULL, ST_OD_Task, NULL);
    if (0 == g_pThreadOdState)
    {
        pthread_setname_np(g_pThreadOd, "ST_OD_Task");
    }
    else
    {
        g_pThreadOd = -1;
        ST_ERR("ST_OD_Initial, create g_pthread_od=%ld is err[%d]!\n %s\n", g_pThreadOd, g_pThreadOdState,
                  strerror(g_pThreadOdState));
        return -1;
    }

    ST_Common_WaitDumpFinsh(&g_stVencOutputFileAttr);

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit(void)
{
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;

    MI_U32 u32SclDevId = 1;
    MI_U32 u32SclChnId = 0;
    MI_U32 u32SclPortId0 = 0;
    MI_U32 u32SclPortId1 = 1;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    step1 :deinit Yuv thread
    *************************************************/
    pthread_join(g_stSclInputFileAttr.pPutDatathread, NULL);
    pthread_join(g_pThreadOd, NULL);

    /************************************************
    step2 :deinit rgn
    *************************************************/
    MI_U8 WinIndex = WINDOW_INDEX_COL_MAX * WINDOW_INDEX_ROW_MAX;
    for(int Index = 0 ; Index < WinIndex; Index++ )
    {
        STCHECKRESULT(ST_Common_RgnDetachChn(0, g_hRgnHandleCover[Index], &g_stRgnChnPort));
        STCHECKRESULT(ST_Common_RgnDestroy(0, g_hRgnHandleCover[Index]));
    }
    STCHECKRESULT(ST_Common_RgnDeInit(0));

    /************************************************
    step3 :unbind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step4 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step5 :unbind scl->vdf
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId1;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VDF;
    stDstChnPort.u32DevId  = 0;
    stDstChnPort.u32ChnId  = 0;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));
    MI_SYS_CloseFd(g_s32VdfFD);

    /************************************************
    step6 :deinit vdf
    *************************************************/
    STCHECKRESULT(MI_VDF_EnableSubWindow(g_VdfOdChn, 0, 0, 0));
    STCHECKRESULT(MI_VDF_Stop(E_MI_VDF_WORK_MODE_OD));
    STCHECKRESULT(MI_VDF_DestroyChn(g_VdfOdChn));
    STCHECKRESULT(MI_VDF_Uninit());

    /************************************************
    step7 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId0));
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId1));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step8 :sys exit
    *************************************************/
    fclose(g_pOutFile);
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

void ST_Vdf_Usage(void)
{
    printf("Usage:./prog_vdf_od_demo) file -> scl -> venc(rgn attach cover Frame from vdf_od) -> file\n");
}

MI_S32 main(int argc, char **argv)
{
    ST_Vdf_Usage();
    STCHECKRESULT(STUB_BaseModuleInit());
    STCHECKRESULT(STUB_BaseModuleDeInit());

    return 0;
}
