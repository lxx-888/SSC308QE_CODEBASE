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
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_mod.h"
#include "ptree_cmd.h"
#include "ptree_sur_isp.h"
#include "ssos_list.h"
#include "mi_isp.h"
#include "ptree_maker.h"

int PTREE_CMD_ISP_StrCmdSetRot(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int               dev = pstModObj->info->devId;
    int               chn = pstModObj->info->chnId;
    int               rot = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    MI_ISP_ChnParam_t stChannelIspParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    MI_ISP_StopChannel(dev, chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    switch (rot)
    {
        case 90:
            stChannelIspParam.eRot = E_MI_SYS_ROTATE_90;
            break;
        case 180:
            stChannelIspParam.eRot = E_MI_SYS_ROTATE_180;
            break;
        case 270:
            stChannelIspParam.eRot = E_MI_SYS_ROTATE_270;
            break;
        case 0:
            stChannelIspParam.eRot = E_MI_SYS_ROTATE_NONE;
            break;
        default:
            stChannelIspParam.eRot = E_MI_SYS_ROTATE_NONE;
    }
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev, chn);

    PTREE_DBG("set isp dev: %d, chn: %d, rot :%d ", dev, chn, rot);
    return 0;
}

int PTREE_CMD_ISP_StrCmdSetMirrorFlip(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int               dev    = pstModObj->info->devId;
    int               chn    = pstModObj->info->chnId;
    int               mirror = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int               flip   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    MI_ISP_ChnParam_t stChannelIspParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    MI_ISP_StopChannel(dev, chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    stChannelIspParam.bMirror = mirror;
    stChannelIspParam.bFlip   = flip;
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev, chn);

    PTREE_DBG("set isp dev: %d, chn: %d, mirror :%d, flip: %d ", dev, chn, mirror, flip);
    return 0;
}

int PTREE_CMD_ISP_StrCmdSetHdr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int               dev             = pstModObj->info->devId;
    int               chn             = pstModObj->info->chnId;
    int               hdr             = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int               hdrFusion       = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int               hdrExposureMask = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    MI_ISP_ChnParam_t stChannelIspParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2-%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2]);

    MI_ISP_StopChannel(dev, chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    stChannelIspParam.eHDRType           = hdr;
    stChannelIspParam.eHDRFusionType     = hdrFusion;
    stChannelIspParam.u16HDRExposureMask = hdrExposureMask;
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev, chn);

    PTREE_DBG("set isp dev: %d, chn: %d, hdr :%d, fusion: %d, expo mask: %d", dev, chn, hdr, hdrFusion,
              hdrExposureMask);
    return 0;
}

int PTREE_CMD_ISP_StrCmdSet3dnr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int               dev       = pstModObj->info->devId;
    int               chn       = pstModObj->info->chnId;
    int               level3dnr = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    MI_ISP_ChnParam_t stChannelIspParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    MI_ISP_StopChannel(dev, chn);
    MI_ISP_GetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    stChannelIspParam.e3DNRLevel = (MI_ISP_3DNR_Level_e)level3dnr;
    MI_ISP_SetChnParam((MI_ISP_DEV)dev, chn, &stChannelIspParam);
    MI_ISP_StartChannel(dev, chn);

    PTREE_DBG("set isp dev: %d, chn: %d, 3dnr :%d ", dev, chn, level3dnr);
    return 0;
}

int PTREE_CMD_ISP_StrCmdSetInputCrop(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
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
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2-%s, p3->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2],
              (const char *)pstCmd->cmdPara[3]);

    stCapRect.u16X      = x;
    stCapRect.u16Y      = y;
    stCapRect.u16Width  = w;
    stCapRect.u16Height = h;
    MI_ISP_SetInputPortCrop((MI_ISP_DEV)dev, chn, &stCapRect);

    PTREE_DBG("set isp dev: %d, chn: %d, input crop(%d,%d,%d,%d) ", dev, chn, x, y, w, h);
    return 0;
}

int PTREE_CMD_ISP_StrCmdSetOutputCrop(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev  = pstModObj->info->devId;
    int                   chn  = pstModObj->info->chnId;
    int                   port = pstCmd->cmdPara[0];
    int                   x    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int                   y    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int                   w    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int                   h    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    MI_ISP_OutPortParam_t stIspOutputParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2-%s, p3->%s, p4->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2],
              (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4]);

    MI_ISP_GetOutputPortParam((MI_ISP_DEV)dev, (MI_ISP_CHANNEL)chn, port, &stIspOutputParam);
    stIspOutputParam.stCropRect.u16X      = x;
    stIspOutputParam.stCropRect.u16Y      = y;
    stIspOutputParam.stCropRect.u16Width  = w;
    stIspOutputParam.stCropRect.u16Height = h;
    MI_ISP_SetOutputPortParam((MI_ISP_DEV)dev, (MI_ISP_CHANNEL)chn, port, &stIspOutputParam);

    PTREE_DBG("set isp dev: %d, chn: %d, port:%d, output crop(%d,%d,%d,%d) ", dev, chn, port, x, y, w, h);
    return 0;
}
PTREE_MAKER_CMD_INIT(ISP, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"isp_rot", PTREE_CMD_ISP_StrCmdSetRot, 1},
                     {(unsigned long)"isp_mirror_flip", PTREE_CMD_ISP_StrCmdSetMirrorFlip, 2},
                     {(unsigned long)"isp_hdr", PTREE_CMD_ISP_StrCmdSetHdr, 3},
                     {(unsigned long)"isp_3dnr", PTREE_CMD_ISP_StrCmdSet3dnr, 1},
                     {(unsigned long)"isp_input_crop", PTREE_CMD_ISP_StrCmdSetInputCrop, 4},
                     {(unsigned long)"isp_output_crop", PTREE_CMD_ISP_StrCmdSetOutputCrop, 5})
