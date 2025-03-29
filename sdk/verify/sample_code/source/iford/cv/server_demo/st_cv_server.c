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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <errno.h>
#include "cjson.h"

#include "ss_socket.h"
#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_rtsp_video.h"
#include "mi_isp_iq.h"
#include "mi_iqserver.h"
#include "mi_isp_cus3a_api.h"

static MI_S32  g_hConnSock              = -1;
static MI_BOOL g_Exit                   = false;
static MI_BOOL isClientConnected        = FALSE;
static MI_BOOL isTcpListenThreadRunning = FALSE;
static MI_BOOL isParseCmdThreadRunning  = FALSE;

#define ST_CV_MAX_PIPE_LINE_NUM 8
#define ST_CV_TCP_LISTEN_PORT 9527
#define ST_CV_RTSP_LISTEN_PORT 12345

#define ST_CV_MAX_VIF_DEV_PERGROUP (4)
#define ST_CV_COMMON_PIPE_LINE_NUM 2
#define ST_CV_MAX_PAD_NUM 8
#define ST_CV_CMD_MIN_LEN 8
#define ST_CV_INI_DATA_BUF_LEN 4096

#define STR(R) #R
#define TOSTR(R) STR(R)
#define CJSON_PARSE_INT_VALUE(instance, entry, obj)                 \
    do                                                              \
    {                                                               \
        cJSON *jsParseObj = cJSON_GetObjectItem(entry, TOSTR(obj)); \
        if (jsParseObj == NULL)                                     \
        {                                                           \
            printf("invalid %s\n", TOSTR(obj));                     \
            return -1;                                              \
        }                                                           \
        instance = (typeof(instance))jsParseObj->valueint;          \
    } while (0)

#define CJSON_PARSE_STRING_VALUE(instance, entry, obj)                                  \
    do                                                                                  \
    {                                                                                   \
        cJSON *jsParseObj = cJSON_GetObjectItem(entry, TOSTR(obj));                     \
        if (jsParseObj == NULL)                                                         \
        {                                                                               \
            printf("invalid %s\n", TOSTR(obj));                                         \
            return -1;                                                                  \
        }                                                                               \
        memcpy(instance, jsParseObj->valuestring, strlen(jsParseObj->valuestring) + 1); \
    } while (0)

#define CHECK_RETURN(condExpr, retVal, format, ...) \
    do                                              \
    {                                               \
        if (condExpr)                               \
        {                                           \
            printf(format, ##__VA_ARGS__);          \
            return retVal;                          \
        }                                           \
    } while (0)

#define CHECK_GOTO(condExpr, gotoLbl, var, val, format, ...) \
    do                                                       \
    {                                                        \
        if (condExpr)                                        \
        {                                                    \
            printf(format, ##__VA_ARGS__);                   \
            var = val;                                       \
            goto gotoLbl;                                    \
        }                                                    \
    } while (0)

typedef enum { CV_STATUS_IDLE = 0, CV_STATUS_CREATE_PIPE } ST_CV_Status_e;

typedef enum {
    CMD_TYPE_BOARD_CFG,
    CMD_TYPE_CREATE_COMMON_PIPE,
    CMD_TYPE_CREATE_LDC_PIPE,
    CMD_TYPE_CREATE_NIR_PIPE,
    CMD_TYPE_CREATE_DPU_PIPE,
    CMD_TYPE_CREATE_STITCH_PIPE,
    CMD_TYPE_DESTROY_PIPE,
    CMD_TYPE_QUERY_URL,
    CMD_TYPE_CAP_PIC,
    CMD_TYPE_STOP_CAP,
    CMD_TYPE_MAX
} ST_CV_CmdType_e;

typedef enum { E_ST_CV_ISP_CFG = 0, E_ST_CV_SCL_CFG, E_ST_CV_VENC_CFG, E_ST_CV_MAX_CFG } ST_CV_CfgType_e;

typedef struct
{
    MI_U32 cmdType;
    MI_U32 reqRsp;
    MI_U32 status;
    MI_S32 dataLen;
} ST_CV_CmdHeader_t;

typedef struct
{
    char * dataBuf;
    MI_S32 bufSize;
    MI_U32 dataSize;
} ST_CV_CmdData_t;

typedef struct
{
    MI_U8        snrResIdx;
    MI_SNR_PADID padId;
    MI_VIF_GROUP GroupId;
    MI_VIF_PORT  vifPortId;
} ST_CV_VifCfg_t;

typedef struct
{
    MI_ISP_DEV          IspDevId;
    MI_ISP_CHANNEL      IspChnId;
    MI_ISP_PORT         IspOutPortId;
    MI_ISP_3DNR_Level_e Isp3DNRLevel;
    MI_U8               IspMultiChnEnable;
} ST_CV_IspCfg_t;

typedef struct
{
    MI_U8          SclSkip;
    MI_SCL_DEV     SclDevId;
    MI_SCL_CHANNEL SclChnId;
    MI_SCL_PORT    SclPortId;
    MI_U32         SclOutputW;
    MI_U32         SclOutputH;
} ST_CV_SclCfg_t;

typedef struct
{
    MI_VENC_DEV       VencDev;
    MI_VENC_CHN       VencChn;
    MI_U32            VencWidth;
    MI_U32            VencHeight;
    MI_U32            VencMaxWidth;
    MI_U32            VencMaxHeight;
    MI_VENC_ModType_e eType;
} ST_CV_VencCfg_t;

typedef struct ST_CV_IqCfg_s
{
    char stIqFile[128];
} ST_CV_IqCfg_t;

typedef struct ST_CV_PipeCfg_s
{
    ST_CV_IqCfg_t   stIqCfg;
    ST_CV_VifCfg_t  stVifCfg;
    ST_CV_IspCfg_t  stIspCfg;
    ST_CV_SclCfg_t  stSclCfg;
    ST_CV_VencCfg_t stVencCfg;
} ST_CV_PipeCfg_t;

typedef struct ST_CV_MultiPipeCfg_s
{
    MI_U32          u32PipeNum;
    ST_CV_PipeCfg_t stPipeLineCfg[ST_CV_MAX_PIPE_LINE_NUM];
} ST_CV_MultiPipeCfg_t;

typedef struct
{
    MI_U32 off;
    MI_U32 len;
    MI_U32 pipelineId;
} ST_CV_UrlInfo_t;

typedef struct
{
    MI_U32          urlNum;
    MI_U32          currentMode;
    ST_CV_UrlInfo_t urlInfo[ST_CV_MAX_PIPE_LINE_NUM];
} ST_CV_QueryUrlRsp_t;

typedef struct
{
    MI_U32 pipelineNum;
    MI_U32 captureMode;
    MI_U32 captruePos;
    MI_U32 pipelineIds[ST_CV_MAX_PIPE_LINE_NUM];
} ST_CV_CapRequest_t;

typedef struct
{
    MI_U32 pipelineId;
    MI_U32 off;
    MI_U32 len;
    MI_U32 w;
    MI_U32 h;
} ST_CV_PictureInfo_t;

typedef struct
{
    MI_U32              pipelineNum;
    MI_U32              captureCnt;
    MI_U32              boardStatus;
    MI_U32              imgFormat;
    ST_CV_PictureInfo_t imgCapInfo[ST_CV_MAX_PIPE_LINE_NUM];
    unsigned char       imgData[0];
} ST_CV_CapResInfo_t;

typedef struct
{
    ST_CV_CmdHeader_t * pstCmdHeader;
    ST_CV_CapResInfo_t *pstCmdData;
} ST_CV_CapData_t;

typedef struct ST_CV_SingleRtspUrl_s
{
    char url[128];
} ST_CV_SingleRtspUrl_t;

typedef struct ST_CV_MultiRtspUrl_s
{
    MI_U32                u32UrlNum;
    ST_CV_SingleRtspUrl_t stRtspInfo[ST_CV_MAX_PIPE_LINE_NUM];
} ST_CV_MultiRtspUrl_t;

typedef struct ST_CV_Param_s
{
    ST_CV_Status_e       eStatus;
    ST_CV_MultiPipeCfg_t stMultiPipeCfg;
} ST_CV_Param_t;

static ST_CV_Param_t        g_cvParam;
static ST_VideoStreamInfo_t stStreamInfo[8];

MI_S32 ST_CV_DeviceExist(ST_CV_CfgType_e type, MI_U32 end_idx, MI_U32 dev, ST_CV_MultiPipeCfg_t *pstMultiCfg)
{
    MI_U32 i   = 0;
    MI_S32 ret = 0;
    if (NULL == pstMultiCfg)
    {
        return ret;
    }

    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        if (i == end_idx)
        {
            break;
        }

        switch (type)
        {
            case E_ST_CV_ISP_CFG:
                if (dev == pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId)
                {
                    ret = 1;
                }
                break;
            case E_ST_CV_SCL_CFG:
                if (dev == pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId)
                {
                    ret = 1;
                }
                break;
            case E_ST_CV_VENC_CFG:
                if (dev == pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencDev)
                {
                    ret = 1;
                }
                break;
            default:
                break;
        }

        if (ret == 1)
        {
            break;
        }
    }

    return ret;
}

MI_S32 ST_CV_SendResponse(int connectSock, ST_CV_CmdHeader_t *header, ST_CV_CmdData_t *data)
{
    MI_S32 ret = 0;
    ret        = ss_socket_send(connectSock, (char *)header, sizeof(ST_CV_CmdHeader_t));
    if (ret != sizeof(ST_CV_CmdHeader_t))
    {
        printf("Send rsp heaer failed. errno = %d\n", errno);
        return -1;
    }

    if (data && data->dataSize > 0)
    {
        ret = ss_socket_send(connectSock, (char *)data->dataBuf, data->dataSize);
        if (ret != data->dataSize)
        {
            printf("Send rsp data failed. errno = %d\n", errno);
            return -1;
        }
    }

    return 0;
}

MI_S32 ST_CV_ParsePipeConfig(ST_CV_MultiPipeCfg_t *multiPipelineCfg)
{
    MI_S32           ret             = 0;
    MI_U32           u32Idx          = 0;
    MI_U32           u32JsonPipeNum  = 0;
    char *           jsonBuf         = NULL;
    char             fileName[256]   = {0};
    ST_CV_PipeCfg_t *singPipelineCfg = NULL;
    cJSON *          jsRoot = NULL, *jsObj = NULL, *jsSubObj = NULL;

    if (multiPipelineCfg->u32PipeNum > ST_CV_MAX_PIPE_LINE_NUM)
    {
        return -1;
    }

    snprintf(fileName, sizeof(fileName), "./%d_snr.json", multiPipelineCfg->u32PipeNum);
    jsonBuf = cJSON_FileOpen(fileName);
    if (NULL == jsonBuf)
    {
        printf("load cfg file failed!, file: %s \n", fileName);
        return -1;
    }

    jsRoot = cJSON_Parse(jsonBuf);
    CHECK_GOTO((!jsRoot), parse_fail, ret, -1, "Parse json node failed.\n");

    u32JsonPipeNum = cJSON_GetArraySize(jsRoot);
    CHECK_GOTO((u32JsonPipeNum < multiPipelineCfg->u32PipeNum), parse_fail, ret, -1, "Invalid json config.\n");

    for (u32Idx = 0; u32Idx < multiPipelineCfg->u32PipeNum; u32Idx++)
    {
        singPipelineCfg = &multiPipelineCfg->stPipeLineCfg[u32Idx];
        jsObj           = cJSON_GetArrayItem(jsRoot, u32Idx);
        CHECK_GOTO((!jsObj), parse_fail, ret, -1, "Get array item %d failed.\n", u32Idx);

        jsSubObj = cJSON_GetObjectItem(jsObj, "stVifCfg");
        CHECK_GOTO((!jsSubObj), parse_fail, ret, -1, "Get vif json node failed.\n");

        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVifCfg.snrResIdx, jsSubObj, snrResIdx);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVifCfg.padId, jsSubObj, padId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVifCfg.GroupId, jsSubObj, GroupId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVifCfg.vifPortId, jsSubObj, vifPortId);

        jsSubObj = cJSON_GetObjectItem(jsObj, "stIqCfg");
        if (jsSubObj != NULL)
        {
            CJSON_PARSE_STRING_VALUE(singPipelineCfg->stIqCfg.stIqFile, jsSubObj, IqFilePath);
        }

        jsSubObj = cJSON_GetObjectItem(jsObj, "stIspCfg");
        CHECK_GOTO((!jsSubObj), parse_fail, ret, -1, "Get isp json node failed.\n");
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stIspCfg.IspDevId, jsSubObj, IspDevId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stIspCfg.IspChnId, jsSubObj, IspChnId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stIspCfg.IspOutPortId, jsSubObj, IspOutPortId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stIspCfg.Isp3DNRLevel, jsSubObj, Isp3DNRLevel);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stIspCfg.IspMultiChnEnable, jsSubObj, IspMultiChn);

        jsSubObj = cJSON_GetObjectItem(jsObj, "stSclCfg");
        CHECK_GOTO((!jsSubObj), parse_fail, ret, -1, "Get scl json node failed.\n");

        CJSON_PARSE_INT_VALUE(singPipelineCfg->stSclCfg.SclSkip, jsSubObj, SclSkip);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stSclCfg.SclDevId, jsSubObj, SclDevId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stSclCfg.SclChnId, jsSubObj, SclChnId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stSclCfg.SclPortId, jsSubObj, SclOutPortId);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stSclCfg.SclOutputW, jsSubObj, SclOutPortW);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stSclCfg.SclOutputH, jsSubObj, SclOutPortH);

        jsSubObj = cJSON_GetObjectItem(jsObj, "stVencCfg");
        CHECK_GOTO((!jsSubObj), parse_fail, ret, -1, "Get venc json node failed.\n");
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVencCfg.VencDev, jsSubObj, VencDev);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVencCfg.VencChn, jsSubObj, VencChn);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVencCfg.eType, jsSubObj, eType);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVencCfg.VencWidth, jsSubObj, VencWidth);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVencCfg.VencHeight, jsSubObj, VencHeight);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVencCfg.VencMaxWidth, jsSubObj, VencMaxWidth);
        CJSON_PARSE_INT_VALUE(singPipelineCfg->stVencCfg.VencMaxHeight, jsSubObj, VencMaxHeight);
    }

parse_fail:
    if (jsonBuf != NULL)
    {
        free(jsonBuf);
        jsonBuf = NULL;
    }

    return ret;
}

MI_S32 ST_CV_BaseModuleInit(ST_CV_MultiPipeCfg_t *pstMultiCfg)
{
    MI_SNR_PlaneInfo_t stPlaneInfo;
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;
    int                i             = 0;
    MI_SNR_PADID       u32SnrPadId   = 0;
    MI_U8              u8SnrResId    = 0;
    MI_U32             u32VifGroupId = 0;
    MI_U32             u32VifDevId   = 0;
    MI_U32             u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;
    MI_U32 u8IspMultiChn = 0;

    MI_U8  u8SkipScl     = 0;
    MI_U32 u32SclDevId  = 0;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());
    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        MI_VIF_GroupAttr_t      stVifGroupAttr;
        MI_VIF_DevAttr_t        stVifDevAttr;
        MI_VIF_OutputPortAttr_t stVifPortAttr;
        /************************************************
        step2 :init sensor/vif
        *************************************************/
        u8SnrResId    = pstMultiCfg->stPipeLineCfg[i].stVifCfg.snrResIdx;
        u32SnrPadId   = pstMultiCfg->stPipeLineCfg[i].stVifCfg.padId;
        u32VifPortId  = pstMultiCfg->stPipeLineCfg[i].stVifCfg.vifPortId;
        u32VifGroupId = pstMultiCfg->stPipeLineCfg[i].stVifCfg.GroupId;
        u32VifDevId   = ST_CV_MAX_VIF_DEV_PERGROUP * u32VifGroupId;

        STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SnrResId, 0xFF));

        memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
        STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId, 0, &stPlaneInfo)); // get sensor size

        ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
        STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));

        ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
        STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

        ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);
        STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId, u32VifPortId, &stVifPortAttr));
    }
    /************************************************
    step3 :init isp
    *************************************************/

    MI_ISP_DevAttr_t      stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t     stIspChnParam;
    MI_SYS_WindowRect_t   stIspInputCrop;
    MI_ISP_OutPortParam_t stIspOutPortParam;

    ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        MI_U32 u32SensorBindId = 0;
        u32VifGroupId          = pstMultiCfg->stPipeLineCfg[i].stVifCfg.GroupId;
        u32VifDevId            = 0 + ST_CV_MAX_VIF_DEV_PERGROUP * u32VifGroupId;
        u32VifPortId           = pstMultiCfg->stPipeLineCfg[i].stVifCfg.vifPortId;
        u32IspDevId            = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId;
        u32IspChnId            = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId;
        u32IspPortId           = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspOutPortId;
        u8IspMultiChn          = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspMultiChnEnable;

        if (!ST_CV_DeviceExist(E_ST_CV_ISP_CFG, i, u32IspDevId, pstMultiCfg))
        {
            STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));
        }

        ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
        ST_Common_GetIspBindSensorIdByPad(pstMultiCfg->stPipeLineCfg[i].stVifCfg.padId, &u32SensorBindId);
        stIspChnAttr.u32SensorBindId = u32SensorBindId;
        stIspChnParam.e3DNRLevel     = pstMultiCfg->stPipeLineCfg[i].stIspCfg.Isp3DNRLevel;
        if (u8IspMultiChn)
        {
            STCHECKRESULT(ST_Common_IspStartMultiChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));
        }
        else
        {
            STCHECKRESULT(ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));
        }
        ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
        stIspOutPortParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
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
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));

        stDstChnPort.u32PortId = u32IspPortId;
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 1, 2));
    }
    /************************************************
    step5 :init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutPortParam;

    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        u8SkipScl    = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclSkip;
        u32SclDevId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId;
        u32SclChnId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclChnId;
        u32SclPortId = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclPortId;

        if (u8SkipScl)
        {
            continue;
        }

        ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
        if (u32SclDevId == MI_SCL_DEV_DIPR0)
        {
            stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2;
        }

        if (!ST_CV_DeviceExist(E_ST_CV_SCL_CFG, i, u32SclDevId, pstMultiCfg))
        {
            STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));
        }

        ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
        STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

        ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);

        stSclOutPortParam.stSCLOutputSize.u16Width  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclOutputW;
        stSclOutPortParam.stSCLOutputSize.u16Height = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclOutputH;
        STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));

        /************************************************
        step6 :bind isp->scl
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId;
        stSrcChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId;
        stSrcChnPort.u32PortId = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspOutPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = u32SclDevId;
        stDstChnPort.u32ChnId  = u32SclChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 30;
        u32DstFrmrate          = 30;
        eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));

        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 0, 4));
    }

    /************************************************
    step7 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencSourceCfg;


    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        u32VencDevId            = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencDev;
        u32VencChnId            = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencChn;
        MI_VENC_ModType_e eType = pstMultiCfg->stPipeLineCfg[i].stVencCfg.eType;
        ST_Common_GetVencDefaultDevAttr(&stVencInitParam);

        stVencInitParam.u32MaxWidth = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxWidth;
        stVencInitParam.u32MaxHeight = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxHeight;
        if (!ST_CV_DeviceExist(E_ST_CV_VENC_CFG, i, u32VencDevId, pstMultiCfg))
        {
            STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));
        }

        ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr, &stVencSourceCfg);
        switch (eType)
        {
            case E_MI_VENC_MODTYPE_H264E:
                stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth  = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencWidth;
                stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencHeight;
                stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxWidth;
                stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxHeight;
                break;
            case E_MI_VENC_MODTYPE_H265E:
                stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencWidth;
                stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencHeight;
                stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxWidth;
                stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxHeight;
                break;
            case E_MI_VENC_MODTYPE_JPEGE:
                stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencWidth;
                stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencHeight;
                stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxWidth;
                stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencMaxHeight;
                break;
             default:
                printf("venc type %d not support.\n", eType);
                break;
        }
        STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr, &stVencSourceCfg));

        /************************************************
        step8 :bind scl->venc
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));

        if (pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclSkip)
        {
            stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
            stSrcChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId;
            stSrcChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId;
            stSrcChnPort.u32PortId = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspOutPortId;
        }
        else
        {
            stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId;
            stSrcChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclChnId;
            stSrcChnPort.u32PortId = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclPortId;
        }

        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 30;
        u32DstFrmrate          = 30;
        eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));
    }

    return MI_SUCCESS;
}

MI_S32 ST_CV_BaseModuleDeint(ST_CV_MultiPipeCfg_t *pstMultiCfg)
{
    int              i        = 0;
    MI_VIF_DEV       vifDevId = 0;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    /************************************************
    step1 :unbind scl->venc
    *************************************************/
    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        if (pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclSkip)
        {
            stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
            stSrcChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId;
            stSrcChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId;
            stSrcChnPort.u32PortId = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspOutPortId;
        }
        else
        {
            stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId;
            stSrcChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclChnId;
            stSrcChnPort.u32PortId = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclPortId;
        }

        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencDev;
        stDstChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencChn;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step2 :deinit venc
        *************************************************/
        STCHECKRESULT(ST_Common_VencStopChn(pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencDev,
                                            pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencChn));
    }

    /************************************************
    step3 :unbind isp->scl and destroy venc device
    *************************************************/
    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        if (!ST_CV_DeviceExist(E_ST_CV_VENC_CFG, i, pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencDev, pstMultiCfg))
        {
            STCHECKRESULT(ST_Common_VencDestroyDev(pstMultiCfg->stPipeLineCfg[0].stVencCfg.VencDev));
        }

        if (pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclSkip)
        {
            continue;
        }
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId;
        stSrcChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId;
        stSrcChnPort.u32PortId = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspOutPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId;
        stDstChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclChnId;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step4 :deinit scl
        *************************************************/
        STCHECKRESULT(ST_Common_SclDisablePort(pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId,
                                               pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclChnId,
                                               pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclPortId));
        STCHECKRESULT(ST_Common_SclStopChn(pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId,
                                           pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclChnId));
    }

    /************************************************
    step5 :unbind vif->isp and destroy scl device
    *************************************************/
    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        if (!ST_CV_DeviceExist(E_ST_CV_SCL_CFG, i, pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId, pstMultiCfg))
        {
            STCHECKRESULT(ST_Common_SclDestroyDevice(pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclDevId));
        }

        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        vifDevId               = pstMultiCfg->stPipeLineCfg[i].stVifCfg.GroupId * ST_CV_MAX_VIF_DEV_PERGROUP;
        stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId  = vifDevId;
        stSrcChnPort.u32ChnId  = 0;
        stSrcChnPort.u32PortId = pstMultiCfg->stPipeLineCfg[i].stVifCfg.vifPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId;
        stDstChnPort.u32ChnId  = pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step6 :deinit isp
        *************************************************/
        STCHECKRESULT(ST_Common_IspDisablePort(pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId,
                                               pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId,
                                               pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspOutPortId));
        STCHECKRESULT(ST_Common_IspStopChn(pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId,
                                           pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId));
    }

    /************************************************
    step7 :deinit vif/sensor and destroy isp device
    *************************************************/
    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        if (!ST_CV_DeviceExist(E_ST_CV_ISP_CFG, i, pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId, pstMultiCfg))
        {
            STCHECKRESULT(ST_Common_IspDestroyDevice(pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId));
        }

        vifDevId = pstMultiCfg->stPipeLineCfg[i].stVifCfg.GroupId * ST_CV_MAX_VIF_DEV_PERGROUP;
        STCHECKRESULT(ST_Common_VifDisablePort(vifDevId, 0));
        STCHECKRESULT(ST_Common_VifDisableDev(vifDevId));
        STCHECKRESULT(ST_Common_VifDestroyDevGroup(pstMultiCfg->stPipeLineCfg[i].stVifCfg.GroupId));
        STCHECKRESULT(ST_Common_SensorDeInit(pstMultiCfg->stPipeLineCfg[i].stVifCfg.padId));
    }

    /************************************************
    step8 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return 0;
}

MI_S32 ST_CV_CreatePipeLine(ST_CV_Param_t *cvParam, ST_CV_CmdHeader_t *cmdHeader, ST_CV_CmdData_t *recvData)
{
    int                   i           = 0;
    MI_S32                ret         = 0;
    MI_S32                callRet     = 0;
    ST_CV_MultiPipeCfg_t *pstMultiCfg = NULL;

    cvParam->eStatus        = CV_STATUS_IDLE;
    pstMultiCfg             = &cvParam->stMultiPipeCfg;
    pstMultiCfg->u32PipeNum = *(MI_U32 *)(recvData->dataBuf);
    printf("pipe num:%d\n", pstMultiCfg->u32PipeNum);

    if (pstMultiCfg->u32PipeNum > ST_CV_MAX_PIPE_LINE_NUM)
    {
        printf("exceed max pipnum.\n");
        return -1;
    }

    callRet = ST_CV_ParsePipeConfig(pstMultiCfg);
    CHECK_RETURN((0 != callRet), -1, "Parse config failed.\n");

    callRet = ST_CV_BaseModuleInit(pstMultiCfg);
    CHECK_RETURN((0 != callRet), -1, "Create pipeline failed.\n");

    for (MI_U32 i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        memset(&stStreamInfo[i], 0x00, sizeof(ST_VideoStreamInfo_t));
        stStreamInfo[i].eType        = pstMultiCfg->stPipeLineCfg[i].stVencCfg.eType;
        stStreamInfo[i].VencDev      = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencDev;
        stStreamInfo[i].VencChn      = pstMultiCfg->stPipeLineCfg[i].stVencCfg.VencChn;
        stStreamInfo[i].u32Width     = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclOutputW;
        stStreamInfo[i].u32Height    = pstMultiCfg->stPipeLineCfg[i].stSclCfg.SclOutputH;
        stStreamInfo[i].u32FrameRate = 30;
        stStreamInfo[i].rtspIndex    = i;

        ST_Common_RtspServerStartVideo(&stStreamInfo[i]);
    }
    cvParam->eStatus = CV_STATUS_CREATE_PIPE;

    for (i = 0; i < pstMultiCfg->u32PipeNum; i++)
    {
        if (strlen(pstMultiCfg->stPipeLineCfg[i].stIqCfg.stIqFile) == 0)
        {
            continue;
        }

        ST_Common_IspSetIqBin(pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspDevId,
                              pstMultiCfg->stPipeLineCfg[i].stIspCfg.IspChnId,
                              pstMultiCfg->stPipeLineCfg[i].stIqCfg.stIqFile);
    }
    MI_IQSERVER_Open();

    return ret;
}

MI_S32 ST_CV_DestroyPipeLine(ST_CV_Param_t *cvParam, ST_CV_CmdHeader_t *cmdHeader)
{
    int               i   = 0;
    MI_S32            ret = 0;
    ST_CV_CmdHeader_t rspHeader;
    memset(&rspHeader, 0, sizeof(ST_CV_CmdHeader_t));

    MI_IQSERVER_Close();
    if (cvParam->eStatus == CV_STATUS_IDLE)
    {
        goto destroy_exit;
    }

    for (i = 0; i < cvParam->stMultiPipeCfg.u32PipeNum; i++)
    {
        ST_Common_RtspServerStopVideo(&stStreamInfo[i]);
    }
    ST_CV_BaseModuleDeint(&cvParam->stMultiPipeCfg);

destroy_exit:
    cvParam->eStatus = CV_STATUS_IDLE;

    return ret;
}

MI_S32 ST_CV_CheckBuffSize(ST_CV_CmdData_t *cmdData, MI_U32 dataSize)
{
    char *newBuf = NULL;
    if (cmdData == NULL || dataSize == 0)
    {
        return -1;
    }

    if (cmdData->bufSize < dataSize)
    {
        newBuf = (char *)malloc(dataSize);
        CHECK_RETURN((!newBuf), -1, "Malloc new buffer failed. errno = %d\n", errno);

        if (cmdData->dataBuf)
        {
            free(cmdData->dataBuf);
            cmdData->dataBuf = NULL;
        }

        cmdData->dataBuf  = newBuf;
        cmdData->bufSize  = dataSize;
        cmdData->dataSize = 0;
    }

    return 0;
}

MI_S32 ST_CV_QueryUrl(ST_CV_Param_t *cvParam, ST_CV_CmdHeader_t *cmdHeader, ST_CV_CmdData_t *sendData)
{
    MI_U32               i      = 0;
    MI_S32               s32Ret = 0;
    MI_U32               u32Rsz = 0;
    char *               url    = NULL;
    ST_CV_MultiRtspUrl_t stMultiRtspUrl;
    memset(&stMultiRtspUrl, 0x0, sizeof(ST_CV_MultiRtspUrl_t));

    stMultiRtspUrl.u32UrlNum = cvParam->stMultiPipeCfg.u32PipeNum;
    for (i = 0; i < stMultiRtspUrl.u32UrlNum; i++)
    {
        url = ST_Common_RtspServerGetUrl(i);
        if (url != NULL)
        {
            snprintf(stMultiRtspUrl.stRtspInfo[i].url, 128, "%s", url);
        }
    }

    u32Rsz = sizeof(ST_CV_QueryUrlRsp_t);
    for (i = 0; i < stMultiRtspUrl.u32UrlNum; i++)
    {
        u32Rsz += (strlen(stMultiRtspUrl.stRtspInfo[i].url) + 1);
    }

    ST_CV_CheckBuffSize(sendData, u32Rsz);

    if (!sendData->dataBuf)
    {
        printf("sendData->dataBuf is null\n");
        return -1;
    }
    memset(sendData->dataBuf, 0x0, u32Rsz);
    ST_CV_QueryUrlRsp_t *rsp = (ST_CV_QueryUrlRsp_t *)sendData->dataBuf;
    rsp->currentMode         = cvParam->eStatus;
    rsp->urlNum              = stMultiRtspUrl.u32UrlNum;
    u32Rsz                   = sizeof(ST_CV_QueryUrlRsp_t);

    for (i = 0; i < stMultiRtspUrl.u32UrlNum; i++)
    {
        rsp->urlInfo[i].len        = strlen(stMultiRtspUrl.stRtspInfo[i].url) + 1;
        rsp->urlInfo[i].off        = u32Rsz;
        rsp->urlInfo[i].pipelineId = i;
        strcpy((char *)sendData->dataBuf + u32Rsz, stMultiRtspUrl.stRtspInfo[i].url);

        printf("rsp URL is: %s\n", (char *)sendData->dataBuf + u32Rsz);

        u32Rsz += rsp->urlInfo[i].len;
    }
    sendData->dataSize = u32Rsz;

    return s32Ret;
}

int ST_CV_CapturePictures(ST_CV_Param_t *pstCvServer, ST_CV_CmdHeader_t *reqHeader, ST_CV_CmdData_t *reqData)
{
    int    i            = 0;
    MI_S32 ret          = 0;
    MI_U32 u32FailCnt   = 0;
    MI_U32 u32PipeIdx   = 0;
    MI_U32 u32ErrFlag   = 0;
    MI_U32 u32Offset    = 0;
    MI_U32 u32TotalSize = 0;

    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stBufInfo;

    ST_CV_PipeCfg_t *   pipeCfg   = NULL;
    ST_CV_CapRequest_t *capRquest = NULL;

    ST_CV_CapData_t   capRsp;
    ST_CV_CmdHeader_t stResHeader;
    memset(&capRsp, 0x0, sizeof(ST_CV_CapData_t));
    memset(&stResHeader, 0x0, sizeof(ST_CV_CmdHeader_t));

    if (!reqData->dataSize)
    {
        u32ErrFlag = 1;
        goto capture_exit;
    }

    capRquest = (ST_CV_CapRequest_t *)reqData->dataBuf;
    if (capRquest->pipelineNum > ST_CV_MAX_PIPE_LINE_NUM)
    {
        u32ErrFlag = 1;
        goto capture_exit;
    }

    for (i = 0; i < capRquest->pipelineNum; i++)
    {
        memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
        memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));

        u32PipeIdx = capRquest->pipelineIds[i];
        if (u32PipeIdx >= ST_CV_MAX_PIPE_LINE_NUM)
        {
            u32ErrFlag = 1;
            break;
        }
        pipeCfg = &pstCvServer->stMultiPipeCfg.stPipeLineCfg[u32PipeIdx];

        stChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stChnPort.u32DevId  = pipeCfg->stIspCfg.IspDevId;
        stChnPort.u32ChnId  = pipeCfg->stIspCfg.IspChnId;
        stChnPort.u32PortId = pipeCfg->stIspCfg.IspOutPortId;

        u32FailCnt = 3;
        while (u32FailCnt)
        {
            ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &hHandle);
            if (ret == MI_SUCCESS)
            {
                printf("success capture\n");
                break;
            }
            else
            {
                usleep(5000);
                u32FailCnt--;
            }
        }

        if (u32FailCnt <= 0)
        {
            printf("get picture fail, ret=%x\n", ret);
            u32ErrFlag = 1;
            MI_SYS_ChnOutputPortPutBuf(hHandle);
            break;
        }

        if (u32TotalSize == 0)
        {
            u32TotalSize      = u32Offset + capRquest->pipelineNum * stBufInfo.stFrameData.u32BufSize;
            if (u32TotalSize > 0){
                capRsp.pstCmdData = (ST_CV_CapResInfo_t *)malloc(u32TotalSize);
            }
            if (!capRsp.pstCmdData)
            {
                printf("Malloc failed.\n");
                u32ErrFlag = 1;
                break;
            }
        }

        memcpy((unsigned char *)capRsp.pstCmdData + u32Offset, stBufInfo.stFrameData.pVirAddr[0],
               stBufInfo.stFrameData.u32BufSize);
        printf("dump pipid is %d\n", u32PipeIdx);
        capRsp.pstCmdData->imgCapInfo[i].pipelineId = u32PipeIdx;
        capRsp.pstCmdData->imgCapInfo[i].w          = stBufInfo.stFrameData.u16Width;
        capRsp.pstCmdData->imgCapInfo[i].h          = stBufInfo.stFrameData.u16Height;
        capRsp.pstCmdData->imgCapInfo[i].off        = u32Offset;
        capRsp.pstCmdData->imgCapInfo[i].len        = stBufInfo.stFrameData.u32BufSize;
        u32Offset += stBufInfo.stFrameData.u32BufSize;

        printf(" vaddr: %p, pip: %d, w: %d, h: %d, off: %d, len: %d\n", capRsp.pstCmdData,
               capRsp.pstCmdData->imgCapInfo[i].pipelineId, capRsp.pstCmdData->imgCapInfo[i].w,
               capRsp.pstCmdData->imgCapInfo[i].h, capRsp.pstCmdData->imgCapInfo[i].off,
               capRsp.pstCmdData->imgCapInfo[i].len);
        MI_SYS_ChnOutputPortPutBuf(hHandle);
    }

capture_exit:
    if (u32ErrFlag || !capRsp.pstCmdData)
    {
        stResHeader.cmdType = CMD_TYPE_CAP_PIC;
        stResHeader.reqRsp  = 1;
        stResHeader.status  = 1;
        stResHeader.dataLen = 0;
        ss_socket_send(g_hConnSock, (char *)&stResHeader, sizeof(ST_CV_CmdHeader_t));
    }
    else
    {
        stResHeader.cmdType = CMD_TYPE_CAP_PIC;
        stResHeader.reqRsp  = 1;
        stResHeader.status  = 0;
        stResHeader.dataLen = u32TotalSize;

        capRsp.pstCmdData->pipelineNum = capRquest->pipelineNum;
        capRsp.pstCmdData->captureCnt  = 1;
        capRsp.pstCmdData->boardStatus = pstCvServer->eStatus;

        // send all pcitures
        ss_socket_send(g_hConnSock, (char *)&stResHeader, sizeof(ST_CV_CmdHeader_t));
        ss_socket_send(g_hConnSock, (char *)capRsp.pstCmdData, stResHeader.dataLen);
    }

    if (capRsp.pstCmdData != NULL)
    {
        free(capRsp.pstCmdData);
        capRsp.pstCmdData = NULL;
    }

    return ret;
}

MI_S32 ST_CV_ProcessCmd(ST_CV_CmdHeader_t *cmdHeader, ST_CV_CmdData_t *cmdData)
{
    MI_S32            ret = 0;
    ST_CV_CmdData_t   stResData;
    ST_CV_CmdHeader_t stResHeader;

    if (!cmdHeader || !cmdData)
    {
        printf("%s is null \n", !cmdHeader ? "cmdHeader" : "cmdData");
        return -1;
    }

    memset(&stResData, 0x0, sizeof(stResData));
    memset(&stResHeader, 0x0, sizeof(stResHeader));
    switch (cmdHeader->cmdType)
    {
        case CMD_TYPE_BOARD_CFG:
            break;
        case CMD_TYPE_CREATE_COMMON_PIPE:
            ret = ST_CV_CreatePipeLine(&g_cvParam, cmdHeader, cmdData);
            break;
        case CMD_TYPE_CAP_PIC:
            ret = ST_CV_CapturePictures(&g_cvParam, cmdHeader, cmdData);
            break;
        case CMD_TYPE_QUERY_URL:
            ret = ST_CV_QueryUrl(&g_cvParam, cmdHeader, &stResData);
            break;
        case CMD_TYPE_DESTROY_PIPE:
            ret = ST_CV_DestroyPipeLine(&g_cvParam, cmdHeader);
            break;
        default:
            printf("Unkown cmd: %d\n", cmdHeader->cmdType);
            break;
    }

    if (cmdHeader->cmdType != CMD_TYPE_CAP_PIC)
    {
        stResHeader.cmdType = cmdHeader->cmdType;
        stResHeader.dataLen = stResData.dataSize;
        stResHeader.reqRsp  = 1;
        stResHeader.status  = ret == 0 ? 0 : 1;

        printf("data size:%d\n", stResData.dataSize);
        ST_CV_SendResponse(g_hConnSock, &stResHeader, &stResData);
    }

    if (stResData.dataBuf != NULL)
    {
        free(stResData.dataBuf);
    }
    return ret;
}

void *ST_CV_ParserCmdThread(void *arg)
{
    ST_CV_CmdData_t   cmdData;
    ST_CV_CmdHeader_t cmdHead;
    MI_S32            s32CallRet     = 0;
    MI_S32            s32ConnectSock = 0;

    memset(&cmdData, 0x0, sizeof(cmdData));
    memset(&cmdHead, 0x0, sizeof(cmdHead));
    pthread_detach(pthread_self());

    s32ConnectSock = *((int *)arg);
    if (s32ConnectSock < 0)
    {
        goto exit;
    }

    cmdData.dataBuf = (char *)malloc(ST_CV_INI_DATA_BUF_LEN);
    if (cmdData.dataBuf == NULL)
    {
        printf("Malloc buffer failed. errno = %d\n", errno);
        goto exit;
    }

    memset(cmdData.dataBuf, 0x0, ST_CV_INI_DATA_BUF_LEN);
    cmdData.dataSize = 0;
    cmdData.bufSize  = ST_CV_INI_DATA_BUF_LEN;

    isParseCmdThreadRunning = TRUE;
    while (isParseCmdThreadRunning)
    {
        s32CallRet = ss_socket_select(&s32ConnectSock, 0x1, FD_SOCK_READ, SOCK_SELECT_TIMEOUT);
        if (-1 == s32CallRet)
        {
            printf("Select failed.\n");
            break;
        }
        else if (s32CallRet == 0 || (s32CallRet != (FD_SOCK_READ_SET | s32ConnectSock)))
        {
            continue;
        }

        cmdHead.dataLen   = 0;
        int expectRecvLen = sizeof(ST_CV_CmdHeader_t);
        s32CallRet        = ss_socket_receive(s32ConnectSock, (char *)&cmdHead, expectRecvLen, expectRecvLen);
        if (s32CallRet <= 0)
        {
            printf("Receive failed. errno = %d\n", errno);
            break;
        }
        else if (s32CallRet != expectRecvLen)
        {
            printf("sock ret(%d)!=len(%d)\n", s32CallRet, expectRecvLen);
            continue;
        }

        if (cmdHead.dataLen > 0)
        {
            s32CallRet = ST_CV_CheckBuffSize(&cmdData, cmdHead.dataLen);
            if (0 != s32CallRet)
            {
                printf("Adjust cmd data buffer size failed, new size = %d\n", cmdHead.dataLen);
                break;
            }

            if (cmdData.bufSize < cmdHead.dataLen)
            {
                break;
            }

            s32CallRet = ss_socket_receive(s32ConnectSock, (char *)cmdData.dataBuf, cmdData.bufSize, cmdHead.dataLen);
            if (s32CallRet != cmdHead.dataLen)
            {
                printf("Receive cmd data failed. expected = %d, received = %d, errno = %d\n", cmdHead.dataLen,
                       s32CallRet, errno);
                continue;
            }
        }

        cmdData.dataSize = cmdHead.dataLen;
        ST_CV_ProcessCmd(&cmdHead, &cmdData);
    }

exit:
    if (cmdData.dataBuf != NULL)
    {
        free(cmdData.dataBuf);
    }

    ss_socket_close(s32ConnectSock);
    isParseCmdThreadRunning = FALSE;
    isClientConnected       = FALSE;
    pthread_exit(0);
    return NULL;
}

void *ST_CV_ListenThread(void *arg)
{
    MI_S32 u32CallRet    = 0;
    MI_S32 s32AddrLen    = 0;
    MI_S32 s32ListenSock = -1;

    struct sockaddr_in ipAddr;
    pthread_detach(pthread_self());

    isTcpListenThreadRunning = TRUE;
    isClientConnected        = FALSE;

    while (isTcpListenThreadRunning)
    {
        if (s32ListenSock < 0)
        {
            s32ListenSock = ss_socket_create(INADDR_ANY, ST_CV_TCP_LISTEN_PORT);
            if (s32ListenSock < 0)
            {
                isTcpListenThreadRunning = FALSE;
                printf("Create listen port failed. errno = %d\n", errno);
                return NULL;
            }
            else
            {
                printf("Create listen port success. Listen port is: %d\n", ST_CV_TCP_LISTEN_PORT);
            }
        }

        u32CallRet = ss_socket_select(&s32ListenSock, 0x1, FD_SOCK_READ, SOCK_SELECT_TIMEOUT);
        if (u32CallRet == MST_FAIL)
        {
            printf("Select failed. errno = %d\n", errno);
            ss_socket_close(s32ListenSock);
            s32ListenSock = -1;
            continue;
        }
        else if (u32CallRet == 0 || (u32CallRet != (FD_SOCK_READ_SET | s32ListenSock)))
        {
            continue;
        }

        s32AddrLen  = sizeof(ipAddr);
        g_hConnSock = ss_socket_accept(s32ListenSock, (struct sockaddr *)&ipAddr, &s32AddrLen);
        if (g_hConnSock < 0)
        {
            printf("Accept failed, errno = %d\n", errno);
            ss_socket_close(s32ListenSock);
            s32ListenSock = -1;
            continue;
        }
        else
        {
            if (isClientConnected == FALSE)
            {
                isClientConnected = TRUE;
                printf("Create connection success.\n");
            }
            else
            {
                printf("Stitch server can only support one client!\n");
                ss_socket_close(g_hConnSock);
                g_hConnSock = -1;
                continue;
            }
        }

        pthread_t cmdParseThreadHandl = -1;
        u32CallRet                    = pthread_create(&cmdParseThreadHandl, NULL, ST_CV_ParserCmdThread, &g_hConnSock);
        if (0 != u32CallRet)
        {
            printf("Create tcp cmd parse thread failed, connected socket will be closed. callRet = %d\n", u32CallRet);
            ss_socket_close(g_hConnSock);
            cmdParseThreadHandl = -1;
        }
        else
        {
            printf("Create tcp cmd parse thread success.\n");
        }
    }

    isTcpListenThreadRunning = 0;
    ss_socket_close(s32ListenSock);

    pthread_exit(0);
    return NULL;
}

void ST_CV_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit normally\n");
        ST_CV_DestroyPipeLine(&g_cvParam, NULL);
        isParseCmdThreadRunning  = FALSE;
        isTcpListenThreadRunning = FALSE;
        g_Exit                   = true;
    }
}

int main()
{
    struct sigaction sigAction;
    sigAction.sa_handler = ST_CV_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);

    pthread_t threadHdl;
    pthread_create(&threadHdl, NULL, ST_CV_ListenThread, NULL);

    while (!g_Exit)
    {
        usleep(1000);
    }
}
