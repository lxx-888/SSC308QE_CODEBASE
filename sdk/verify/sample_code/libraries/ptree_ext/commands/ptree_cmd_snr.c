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
#include "ptree_sur_snr.h"
#include "ssos_list.h"
#include "mi_sensor.h"
#include "ptree_maker.h"

int PTREE_CMD_SNR_StrCmdSetRes(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int res = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    MI_SNR_SetRes((MI_SNR_PADID)dev, res);
    PTREE_DBG("set pad: %d res: %d ", dev, res);
    return 0;
}

int PTREE_CMD_SNR_StrCmdSetFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int fps = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    MI_SNR_SetFps((MI_SNR_PADID)dev, fps);
    PTREE_DBG("set pad: %d fps: %d ", dev, fps);
    return 0;
}

int PTREE_CMD_SNR_StrCmdSetMirrorFlip(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev    = pstModObj->info->devId;
    int mirror = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int flip   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    MI_SNR_SetOrien(dev, mirror, flip);
    PTREE_DBG("set pad: %d, mirror: %d, flip: %d ", dev, mirror, flip);
    return 0;
}

int PTREE_CMD_SNR_StrCmdSetHdr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev     = pstModObj->info->devId;
    int                   hdr     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int                   res     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_SUR_SNR_Info_t *pstInfo = CONTAINER_OF(pstModObj->info, PTREE_SUR_SNR_Info_t, base.base);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    pstInfo->s32HdrType = hdr;
    if (-1 != res)
    {
        pstInfo->s32SensorRes = res;
    }

    PTREE_DBG("set pad: %d, hdr type: %d, res: %d ", dev, hdr, res);
    return 0;
}
PTREE_MAKER_CMD_INIT(SNR, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"snr_res", PTREE_CMD_SNR_StrCmdSetRes, 1},
                     {(unsigned long)"snr_fps", PTREE_CMD_SNR_StrCmdSetFps, 1},
                     {(unsigned long)"snr_mirror_flip", PTREE_CMD_SNR_StrCmdSetMirrorFlip, 2},
                     {(unsigned long)"snr_hdr", PTREE_CMD_SNR_StrCmdSetHdr, 2})
