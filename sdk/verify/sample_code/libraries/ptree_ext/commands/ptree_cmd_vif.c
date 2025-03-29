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
#include "ptree_sur_vif.h"
#include "ssos_list.h"
#include "mi_vif.h"
#include "ptree_maker.h"
// #include "mi_ai_datatype.h"

int PTREE_CMD_VIF_StrCmdSetCompress(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                     dev  = pstModObj->info->devId;
    MI_SYS_CompressMode_e   mode = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    MI_VIF_OutputPortAttr_t stVifPortInfo;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    MI_VIF_GetOutputPortAttr((MI_VIF_DEV)dev, 0, &stVifPortInfo);
    stVifPortInfo.eCompressMode = mode;
    MI_VIF_SetOutputPortAttr((MI_VIF_DEV)dev, 0, &stVifPortInfo);

    PTREE_DBG("set dev: %d, compress: %d ", dev, mode);
    return 0;
}

int PTREE_CMD_VIF_StrCmdSetHdr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                   dev             = pstModObj->info->devId;
    int                   hdr             = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int                   hdrExposureMask = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    PTREE_SUR_VIF_Info_t *pstInfo         = CONTAINER_OF(pstModObj->info, PTREE_SUR_VIF_Info_t, base.base);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    pstInfo->s32HdrType         = hdr;
    pstInfo->s32HdrExposureMask = hdrExposureMask;

    PTREE_DBG("set dev: %d, hdr: %d, hdrExposureMask: %d", dev, hdr, hdrExposureMask);
    return 0;
}

PTREE_MAKER_CMD_INIT(VIF, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"vif_compress", PTREE_CMD_VIF_StrCmdSetCompress, 1},
                     {(unsigned long)"vif_hdr", PTREE_CMD_VIF_StrCmdSetHdr, 2})
