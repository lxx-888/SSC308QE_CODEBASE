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

#include "mi_common.h"
#include "mi_common_datatype.h"
#include "ssos_mem.h"
#include "ptree_log.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_sur_scl.h"
#include "ssos_list.h"
#include "mi_scl.h"
#include "ptree_maker.h"

int PTREE_CMD_SCL_StrCmdSetRot(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int               dev = pstModObj->info->devId;
    int               chn = pstModObj->info->chnId;
    int               rot = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    MI_SCL_ChnParam_t stSclChnParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    MI_SCL_GetChnParam((MI_SCL_DEV)dev, chn, &stSclChnParam);
    switch (rot)
    {
        case 90:
            stSclChnParam.eRot = E_MI_SYS_ROTATE_90;
            break;
        case 180:
            stSclChnParam.eRot = E_MI_SYS_ROTATE_180;
            break;
        case 270:
            stSclChnParam.eRot = E_MI_SYS_ROTATE_270;
            break;
        case 0:
            stSclChnParam.eRot = E_MI_SYS_ROTATE_NONE;
            break;
        default:
            stSclChnParam.eRot = E_MI_SYS_ROTATE_NONE;
    }
    MI_SCL_SetChnParam((MI_SCL_DEV)dev, chn, &stSclChnParam);

    PTREE_DBG("set scl dev: %d, chn: %d, rot :%d ", dev, chn, rot);
    return 0;
}

int PTREE_CMD_SCL_StrCmdSetMirrorFlip(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev    = pstModObj->info->devId;
    int                   chn    = pstModObj->info->chnId;
    int                   port   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int                   mirror = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int                   flip   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    MI_SCL_OutPortParam_t stSclOutputParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2]);

    MI_SCL_GetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    stSclOutputParam.bMirror = mirror;
    stSclOutputParam.bFlip   = flip;
    MI_SCL_SetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);

    PTREE_DBG("set scl dev: %d, chn: %d, port :%d, mirror:%d, flip:%d ", dev, chn, port, mirror, flip);
    return 0;
}

int PTREE_CMD_SCL_StrCmdSetOutputRes(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev  = pstModObj->info->devId;
    int                   chn  = pstModObj->info->chnId;
    int                   port = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int                   w    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int                   h    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    MI_SCL_OutPortParam_t stSclOutputParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2]);

    MI_SCL_GetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    stSclOutputParam.stSCLOutputSize.u16Width  = w;
    stSclOutputParam.stSCLOutputSize.u16Height = h;
    MI_SCL_SetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);

    PTREE_DBG("set scl dev: %d, chn: %d, port :%d, size(%d,%d) ", dev, chn, port, w, h);
    return 0;
}

int PTREE_CMD_SCL_StrCmdSetInputCrop(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                 dev = pstModObj->info->devId;
    int                 chn = pstModObj->info->chnId;
    int                 x   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int                 y   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int                 w   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int                 h   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    MI_SYS_WindowRect_t stCapRect;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2],
              (const char *)pstCmd->cmdPara[3]);

    MI_SCL_GetInputPortCrop((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, &stCapRect);
    stCapRect.u16X      = x;
    stCapRect.u16Y      = y;
    stCapRect.u16Width  = w;
    stCapRect.u16Height = h;
    MI_SCL_SetInputPortCrop((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, &stCapRect);

    PTREE_DBG("set scl dev: %d, chn: %d, crop(%d,%d,%d,%d) ", dev, chn, x, y, w, h);
    return 0;
}

int PTREE_CMD_SCL_StrCmdSetOutputCrop(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev  = pstModObj->info->devId;
    int                   chn  = pstModObj->info->chnId;
    int                   port = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int                   x    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int                   y    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int                   w    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int                   h    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    MI_SCL_OutPortParam_t stSclOutputParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2],
              (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4]);

    MI_SCL_GetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);
    stSclOutputParam.stSCLOutCropRect.u16X      = x;
    stSclOutputParam.stSCLOutCropRect.u16Y      = y;
    stSclOutputParam.stSCLOutCropRect.u16Width  = w;
    stSclOutputParam.stSCLOutCropRect.u16Height = h;
    MI_SCL_SetOutputPortParam((MI_SCL_DEV)dev, (MI_SCL_CHANNEL)chn, (MI_SCL_PORT)port, &stSclOutputParam);

    PTREE_DBG("set scl dev: %d, chn: %d, port :%d, crop(%d,%d,%d,%d) ", dev, chn, port, x, y, w, h);
    return 0;
}

PTREE_MAKER_CMD_INIT(SCL, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"scl_rot", PTREE_CMD_SCL_StrCmdSetRot, 1},
                     {(unsigned long)"scl_mirror_flip", PTREE_CMD_SCL_StrCmdSetMirrorFlip, 3},
                     {(unsigned long)"scl_output_res", PTREE_CMD_SCL_StrCmdSetOutputRes, 3},
                     {(unsigned long)"scl_input_crop", PTREE_CMD_SCL_StrCmdSetInputCrop, 4},
                     {(unsigned long)"scl_output_crop", PTREE_CMD_SCL_StrCmdSetOutputCrop, 5})
