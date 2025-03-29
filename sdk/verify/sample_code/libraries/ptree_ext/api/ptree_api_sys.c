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

#include "mi_sys_datatype.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ptree_maker.h"
#include "ptree_sur_sys.h"
#include "ptree_api_sys.h"
#include "mi_sys.h"

static int _PTREE_API_SYS_SetOutputDepth(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                         unsigned int paraCnt)
{
    MI_SYS_ChnPort_t                  stChnPort;
    PTREE_API_SYS_OutputDepthParam_t *outDepthPara = NULL;
    PTREE_SUR_SYS_OutInfo_t *         sysOutInfo   = NULL;
    unsigned int                      port         = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->outCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModOut[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    if (pstModObj->status < E_PTREE_MOD_STATUS_ACTIVATE)
    {
        PTREE_ERR("Mod %s, Sec %s, Dev %d, Chn %d, Module status %d error", pstModObj->info->typeName,
                  pstModObj->info->sectionName, pstModObj->info->devId, pstModObj->info->chnId, pstModObj->status);
        return -1;
    }
    outDepthPara = (PTREE_API_SYS_OutputDepthParam_t *)pstCmd->cmdPara[1];
    if (!outDepthPara)
    {
        PTREE_ERR("output %d para is null!", port);
        return -1;
    }
    sysOutInfo             = CONTAINER_OF(pstModObj->arrModOut[port]->info, PTREE_SUR_SYS_OutInfo_t, base);
    sysOutInfo->depthEn    = outDepthPara->bEn;
    sysOutInfo->depthUser  = outDepthPara->user;
    sysOutInfo->depthTotal = outDepthPara->total;
    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId   = pstModObj->info->modId;
    stChnPort.u32DevId = pstModObj->info->devId;
    stChnPort.u32ChnId = pstModObj->info->chnId;
    return MI_SYS_SetChnOutputPortDepth(0, &stChnPort, outDepthPara->user, outDepthPara->total);
}

static int _PTREE_API_SYS_GetOutputDepth(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd,
                                         unsigned int paraCnt)
{
    PTREE_API_SYS_OutputDepthParam_t *outDepthPara = NULL;
    PTREE_SUR_SYS_OutInfo_t *         sysOutInfo   = NULL;
    unsigned int                      port         = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->outCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModOut[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    outDepthPara = (PTREE_API_SYS_OutputDepthParam_t *)pstCmd->cmdPara[1];
    if (!outDepthPara)
    {
        PTREE_ERR("output %d para is null!", port);
        return -1;
    }
    sysOutInfo          = CONTAINER_OF(pstModObj->arrModOut[port]->info, PTREE_SUR_SYS_OutInfo_t, base);
    outDepthPara->bEn   = sysOutInfo->depthEn;
    outDepthPara->user  = sysOutInfo->depthUser;
    outDepthPara->total = sysOutInfo->depthTotal;
    return 0;
}

static int _PTREE_API_SYS_SetOutputFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    MI_SYS_ChnPort_t                stChnPort;
    PTREE_SUR_SYS_OutInfo_t *       sysOutInfo    = NULL;
    PTREE_API_SYS_OutputFpsParam_t *sysOutFpsPara = NULL;
    unsigned int                    port          = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->outCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModOut[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    if (pstModObj->status < E_PTREE_MOD_STATUS_ACTIVATE)
    {
        PTREE_ERR("Mod %s, Sec %s, Dev %d, Chn %d, Module should init first", pstModObj->info->typeName,
                  pstModObj->info->sectionName, pstModObj->info->devId, pstModObj->info->chnId);
        return -1;
    }
    sysOutFpsPara = (PTREE_API_SYS_OutputFpsParam_t *)pstCmd->cmdPara[1];
    if (!sysOutFpsPara)
    {
        PTREE_ERR("output %d fps para is null!", port);
        return -1;
    }
    sysOutInfo           = CONTAINER_OF(pstModObj->arrModOut[port]->info, PTREE_SUR_SYS_OutInfo_t, base);
    sysOutInfo->base.fps = sysOutFpsPara->fps;
    sysOutInfo->userFrc  = sysOutFpsPara->userFrc;
    if (sysOutInfo->userFrc)
    {
        memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stChnPort.eModId   = pstModObj->info->modId;
        stChnPort.u32DevId = pstModObj->info->devId;
        stChnPort.u32ChnId = pstModObj->info->chnId;
        return MI_SYS_SetChnOutputPortUserFrc(&stChnPort, -1, sysOutInfo->base.fps);
    }
    return 0;
}

static int _PTREE_API_SYS_GetOutputFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_API_SYS_OutputFpsParam_t *sysOutFpsPara = NULL;
    PTREE_SUR_SYS_OutInfo_t *       sysOutInfo    = NULL;
    unsigned int                    port          = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->outCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModOut[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    sysOutFpsPara = (PTREE_API_SYS_OutputFpsParam_t *)pstCmd->cmdPara[1];
    if (!sysOutFpsPara)
    {
        PTREE_ERR("output %d para is null!", port);
        return -1;
    }
    sysOutInfo             = CONTAINER_OF(pstModObj->arrModOut[port]->info, PTREE_SUR_SYS_OutInfo_t, base);
    sysOutFpsPara->fps     = sysOutInfo->base.fps;
    sysOutFpsPara->userFrc = sysOutInfo->userFrc;
    return 0;
}

static int _PTREE_API_SYS_SetInputFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_SYS_InInfo_t *       sysInInfo    = NULL;
    PTREE_API_SYS_InputFpsParam_t *sysInFpsPara = NULL;
    unsigned int                   port         = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->inCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModIn[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    if (pstModObj->status < E_PTREE_MOD_STATUS_ACTIVATE)
    {
        PTREE_ERR("Mod %s, Sec %s, Dev %d, Chn %d, Module should init first", pstModObj->info->typeName,
                  pstModObj->info->sectionName, pstModObj->info->devId, pstModObj->info->chnId);
        return -1;
    }
    sysInFpsPara = (PTREE_API_SYS_InputFpsParam_t *)pstCmd->cmdPara[1];
    if (!sysInFpsPara)
    {
        PTREE_ERR("output %d fps para is null!", port);
        return -1;
    }
    sysInInfo           = CONTAINER_OF(pstModObj->arrModIn[port]->info, PTREE_SUR_SYS_InInfo_t, base);
    sysInInfo->base.fps = sysInFpsPara->fps;
    return 0;
}

static int _PTREE_API_SYS_GetInputFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_SYS_InInfo_t *       sysInInfo    = NULL;
    PTREE_API_SYS_InputFpsParam_t *sysInFpsPara = NULL;
    unsigned int                   port         = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->inCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModIn[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    sysInFpsPara = (PTREE_API_SYS_InputFpsParam_t *)pstCmd->cmdPara[1];
    if (!sysInFpsPara)
    {
        PTREE_ERR("output %d fps para is null!", port);
        return -1;
    }
    sysInInfo         = CONTAINER_OF(pstModObj->arrModIn[port]->info, PTREE_SUR_SYS_InInfo_t, base);
    sysInFpsPara->fps = sysInInfo->base.fps;
    return 0;
}

static int _PTREE_API_SYS_SetBindType(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_SYS_InInfo_t *sysInInfo = NULL;
    unsigned int            port      = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->inCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModIn[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    if (pstModObj->status < E_PTREE_MOD_STATUS_ACTIVATE)
    {
        PTREE_ERR("Mod %s, Sec %s, Dev %d, Chn %d, Module should init first", pstModObj->info->typeName,
                  pstModObj->info->sectionName, pstModObj->info->devId, pstModObj->info->chnId);
        return -1;
    }
    sysInInfo            = CONTAINER_OF(pstModObj->arrModIn[port]->info, PTREE_SUR_SYS_InInfo_t, base);
    sysInInfo->bindType  = (MI_SYS_BindType_e)pstCmd->cmdPara[1];
    sysInInfo->bindParam = (unsigned int)pstCmd->cmdPara[2];
    return 0;
}

static int _PTREE_API_SYS_GetBindType(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    PTREE_SUR_SYS_InInfo_t *sysInInfo = NULL;
    unsigned int            port      = 0;

    (void)paraCnt;
    port = (unsigned int)pstCmd->cmdPara[0];
    if (pstModObj->info->inCnt <= port)
    {
        PTREE_ERR("output port error! cnt %d", pstModObj->info->outCnt);
        return -1;
    }
    if (!pstModObj->arrModIn[port])
    {
        PTREE_ERR("output %d is null!", port);
        return -1;
    }
    if (!pstCmd->cmdPara[1])
    {
        PTREE_ERR("cmd para is null!");
        return -1;
    }
    sysInInfo = CONTAINER_OF(pstModObj->arrModIn[port]->info, PTREE_SUR_SYS_InInfo_t, base);
    *(MI_SYS_BindType_e *)pstCmd->cmdPara[1] = (MI_SYS_BindType_e)sysInInfo->bindType;
    return 0;
}
PTREE_MAKER_API_ADD(SYS, {E_PTREE_API_SYS_CMD_SET_OUTPUT_DEPTH, _PTREE_API_SYS_SetOutputDepth, 2},
                    {E_PTREE_API_SYS_CMD_GET_OUTPUT_DEPTH, _PTREE_API_SYS_GetOutputDepth, 2},
                    {E_PTREE_API_SYS_CMD_SET_OUTPUT_FPS, _PTREE_API_SYS_SetOutputFps, 2},
                    {E_PTREE_API_SYS_CMD_GET_OUTPUT_FPS, _PTREE_API_SYS_GetOutputFps, 2},
                    {E_PTREE_API_SYS_CMD_SET_INPUT_FPS, _PTREE_API_SYS_SetInputFps, 2},
                    {E_PTREE_API_SYS_CMD_GET_INPUT_FPS, _PTREE_API_SYS_GetInputFps, 2},
                    {E_PTREE_API_SYS_CMD_SET_INPUT_BINDTYPE, _PTREE_API_SYS_SetBindType, 3},
                    {E_PTREE_API_SYS_CMD_GET_INPUT_BINDTYPE, _PTREE_API_SYS_GetBindType, 2})
