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
#include "ptree_sur_venc.h"
#include "ssos_list.h"
#include "mi_venc.h"
#include "ptree_maker.h"

#define ALIGN_16X_UP(x)   (((x + 15) / 16) * 16)
#define ALIGN_32X_UP(x)   (((x + 31) / 32) * 32)
#define ALIGN_64X_UP(x)   (((x + 63) / 64) * 64)
#define ALIGN_16X_DOWN(x) (x & 0xFFFFFFF0)
#define ALIGN_32X_DOWN(x) (x & 0xFFFFFFE0)
#define ALIGN_64X_DOWN(x) (x & 0xFFFFFFC0)

void PTREE_CMD_VENC_GetH264CurInfo(MI_VENC_ChnAttr_t *getChnAttr, int *pGop, int *pFps)
{
    if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264CBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrH264Cbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum;
    }
    else if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264VBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrH264Vbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum;
    }
    else if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264AVBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrH264Avbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum;
    }
    else
    {
        getChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
        *pGop                        = getChnAttr->stRcAttr.stAttrH264FixQp.u32Gop;
        *pFps                        = getChnAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum;
    }
}

void PTREE_CMD_VENC_GetH265CurInfo(MI_VENC_ChnAttr_t *getChnAttr, int *pGop, int *pFps)
{
    if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265CBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrH265Cbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum;
    }
    else if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265VBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrH265Vbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum;
    }
    else if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265AVBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrH265Avbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum;
    }
    else
    {
        getChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
        *pGop                        = getChnAttr->stRcAttr.stAttrH265FixQp.u32Gop;
        *pFps                        = getChnAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum;
    }
}

void PTREE_CMD_VENC_GetAv1CurInfo(MI_VENC_ChnAttr_t *getChnAttr, int *pGop, int *pFps)
{
    if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_AV1CBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrAv1Cbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum;
    }
    else if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_AV1VBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrAv1Vbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrAv1Vbr.u32SrcFrmRateNum;
    }
    else if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_AV1AVBR)
    {
        *pGop = getChnAttr->stRcAttr.stAttrAv1Avbr.u32Gop;
        *pFps = getChnAttr->stRcAttr.stAttrAv1Avbr.u32SrcFrmRateNum;
    }
    else
    {
        getChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_AV1FIXQP;
        *pGop                        = getChnAttr->stRcAttr.stAttrAv1FixQp.u32Gop;
        *pFps                        = getChnAttr->stRcAttr.stAttrAv1FixQp.u32SrcFrmRateNum;
    }
}

void PTREE_CMD_VENC_GetJpegCurInfo(MI_VENC_ChnAttr_t *getChnAttr, int *pFps)
{
    if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_MJPEGCBR)
    {
        *pFps = getChnAttr->stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum;
    }
    else if (getChnAttr->stRcAttr.eRcMode == E_MI_VENC_RC_MODE_MJPEGVBR)
    {
        *pFps = getChnAttr->stRcAttr.stAttrMjpegVbr.u32SrcFrmRateNum;
    }
    else
    {
        getChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
        *pFps                        = getChnAttr->stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum;
    }
}

int PTREE_CMD_VENC_SetRcFps(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev       = pstModObj->info->devId;
    int chn       = pstModObj->info->chnId;
    int frameRate = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_RcMode_e  eRcModeType = E_MI_VENC_RC_MODE_MAX;
    MI_VENC_ModType_e enType      = E_MI_VENC_MODTYPE_VENC;
    MI_U32            tFnum = 0x0, tFden = 0x0;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    if (frameRate == 0x0)
    {
        PTREE_DBG("you can not set venc fps == 0. return err.");
    }
    else
    {
        tFnum = frameRate;
        tFden = 1;
    }
    PTREE_DBG("the venc tFnum:%d, tFden:%d", tFnum, tFden);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    MI_VENC_GetChnAttr((MI_VENC_DEV)dev, chn, &stChnAttr);

    eRcModeType = stChnAttr.stRcAttr.eRcMode;
    enType      = stChnAttr.stVeAttr.eType;
    PTREE_DBG("the enType is %d", enType);
    PTREE_DBG("the eRcModeType is %d", eRcModeType);

    switch (enType)
    {
        case E_MI_VENC_MODTYPE_H264E:
        {
            if (E_MI_VENC_RC_MODE_H264CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_H264VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_H264FIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_H264AVBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateDen = tFden;
            }
            else
            {
                PTREE_DBG("not init rcmode  type, eRcModeType:%d", eRcModeType);
            }
        }
        break;

        case E_MI_VENC_MODTYPE_H265E:
        {
            if (E_MI_VENC_RC_MODE_H265CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_H265VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_H265FIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_H265AVBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateDen = tFden;
            }
            else
            {
                PTREE_DBG("not init rcmode  type, eRcModeType:%d", eRcModeType);
            }
        }
        break;

        case E_MI_VENC_MODTYPE_JPEGE:
        {
            if (E_MI_VENC_RC_MODE_MJPEGCBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_MJPEGFIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = tFden;
            }
            else
            {
                PTREE_DBG("rcMode:%d is not supported.", eRcModeType);
            }
        }
        break;

        case E_MI_VENC_MODTYPE_AV1:
        {
            if (E_MI_VENC_RC_MODE_AV1VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_AV1CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_AV1FIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateDen = tFden;
            }
            else if (E_MI_VENC_RC_MODE_AV1AVBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateDen = tFden;
            }
            else
            {
                PTREE_DBG("not init rcmode  type, eRcModeType:%d", eRcModeType);
            }
        }
        break;

        default:
            PTREE_DBG("not init video type, error video encoder type:%d", enType);
            break;
    }
    MI_VENC_SetChnAttr((MI_VENC_DEV)dev, chn, &stChnAttr);
    return 0;
}

int PTREE_CMD_VENC_SetGop(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int               dev = pstModObj->info->devId;
    int               chn = pstModObj->info->chnId;
    int               gop = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_RcMode_e  eRcModeType = E_MI_VENC_RC_MODE_MAX;
    MI_VENC_ModType_e enType      = E_MI_VENC_MODTYPE_VENC;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    if (MI_SUCCESS != MI_VENC_GetChnAttr(dev, chn, &stChnAttr))
    {
        PTREE_DBG("venc:%d can not get gop", chn);
        return 0;
    }

    eRcModeType = stChnAttr.stRcAttr.eRcMode;
    enType      = stChnAttr.stVeAttr.eType;
    switch (enType)
    {
        case E_MI_VENC_MODTYPE_H264E:
        {
            if (E_MI_VENC_RC_MODE_H264CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_H264VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_H264FIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_H264AVBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH264Avbr.u32Gop = gop;
            }
            else
            {
                PTREE_DBG("not init rcmode  type, eRcModeType:%d", eRcModeType);
            }
        }
        break;

        case E_MI_VENC_MODTYPE_H265E:
        {
            if (E_MI_VENC_RC_MODE_H265CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_H265VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_H265FIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_H265AVBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrH265Avbr.u32Gop = gop;
            }
            else
            {
                PTREE_DBG("not init rcmode type, eRcModeType:%d", eRcModeType);
            }
        }
        break;

        case E_MI_VENC_MODTYPE_AV1:
        {
            if (E_MI_VENC_RC_MODE_AV1CBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1Cbr.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_AV1VBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1Vbr.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_AV1FIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1FixQp.u32Gop = gop;
            }
            else if (E_MI_VENC_RC_MODE_AV1AVBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrAv1Avbr.u32Gop = gop;
            }
            else
            {
                PTREE_DBG("not init rcmode  type, eRcModeType:%d", eRcModeType);
            }
        }
        break;

        default:
            PTREE_DBG("not init video type, error video encoder type:%d (E_MI_VENC_MODTYPE_JPEGE)", enType);
            return 0;
    }

    if (MI_SUCCESS != MI_VENC_SetChnAttr(dev, chn, &stChnAttr))
    {
        PTREE_DBG("venc:%d can not set gop", chn);
        return 0;
    }

    PTREE_DBG("venc:%d set gop:%d ok!", chn, gop);
    return 0;
}

int PTREE_CMD_VENC_SetH264Cbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s", (const char *)pstCmd->cmdId,
              paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH264CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H264CBR;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH264Cbr.u32MaxQp     = maxQp;
    stRcParam.stParamH264Cbr.u32MinQp     = minQp;
    stRcParam.stParamH264Cbr.u32MaxIQp    = maxIQp;
    stRcParam.stParamH264Cbr.u32MinIQp    = minIQp;
    stRcParam.stParamH264Cbr.s32IPQPDelta = ipqpDelta;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetH264Vbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);
    int changePos = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s",
              (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH264CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H264VBR;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH264Vbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH264Vbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH264VBR.u32MaxIQp    = maxIQp;
    stRcParam.stParamH264VBR.u32MinIQp    = minIQp;
    stRcParam.stParamH264VBR.s32IPQPDelta = ipqpDelta;
    stRcParam.stParamH264VBR.s32ChangePos = changePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetH264Fixqp(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int iQp = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int pQp = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH264CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                          = E_MI_VENC_RC_MODE_H264FIXQP;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp           = iQp;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp           = pQp;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;

    // stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}

int PTREE_CMD_VENC_SetH264Avbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate              = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp                = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp                = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp               = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp               = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta            = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);
    int changePos            = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]);
    int u32MinStillPercent   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[7]);
    int u32MotionSensitivity = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[8]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s, p7->%s, p8->%s",
              (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6], (const char *)pstCmd->cmdPara[7],
              (const char *)pstCmd->cmdPara[8]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH264CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H264AVBR;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateDen = 1;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH264Avbr.u32MaxIQp          = maxIQp;
    stRcParam.stParamH264Avbr.u32MinIQp          = minIQp;
    stRcParam.stParamH264Avbr.s32IPQPDelta       = ipqpDelta;
    stRcParam.stParamH264Avbr.s32ChangePos       = changePos;
    stRcParam.stParamH264Avbr.u32MinStillPercent = u32MinStillPercent;

    stRcParam.stParamH264Avbr.u32MotionSensitivity = u32MotionSensitivity;
    PTREE_DBG("changepos:%d, percent:%d ,u32MotionSensitivity:%d.", changePos, u32MinStillPercent,
              u32MotionSensitivity);

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetH265Cbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s", (const char *)pstCmd->cmdId,
              paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5]);

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH265CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H265CBR;
    stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH265Cbr.u32MaxQp     = maxQp;
    stRcParam.stParamH265Cbr.u32MinQp     = minQp;
    stRcParam.stParamH265Cbr.u32MaxIQp    = maxIQp;
    stRcParam.stParamH265Cbr.u32MinIQp    = minIQp;
    stRcParam.stParamH265Cbr.s32IPQPDelta = ipqpDelta;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetH265Vbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);
    int changePos = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s",
              (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH265CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H265VBR;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH265Vbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH265Vbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH265Vbr.u32MaxIQp    = maxIQp;
    stRcParam.stParamH265Vbr.u32MinIQp    = minIQp;
    stRcParam.stParamH265Vbr.s32IPQPDelta = ipqpDelta;
    stRcParam.stParamH265Vbr.s32ChangePos = changePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetH265Fixqp(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int iQp = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int pQp = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH265CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                          = E_MI_VENC_RC_MODE_H265FIXQP;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32IQp           = iQp;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32PQp           = pQp;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}

int PTREE_CMD_VENC_SetH265Avbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate              = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp                = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp                = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp               = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp               = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta            = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);
    int changePos            = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]);
    int u32MinStillPercent   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[7]);
    int u32MotionSensitivity = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[8]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s, p7->%s, p8->%s",
              (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6], (const char *)pstCmd->cmdPara[7],
              (const char *)pstCmd->cmdPara[8]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetH265CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H265AVBR;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateDen = 1;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH265Avbr.u32MaxIQp            = maxIQp;
    stRcParam.stParamH265Avbr.u32MinIQp            = minIQp;
    stRcParam.stParamH265Avbr.s32IPQPDelta         = ipqpDelta;
    stRcParam.stParamH265Avbr.s32ChangePos         = changePos;
    stRcParam.stParamH265Avbr.u32MinStillPercent   = u32MinStillPercent;
    stRcParam.stParamH265Avbr.u32MotionSensitivity = u32MotionSensitivity;
    PTREE_DBG("changepos:%d, percent:%d,u32MotionSensitivity:%d.", changePos, u32MinStillPercent, u32MotionSensitivity);

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetJpegCbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQfactor = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQfactor = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);

    int getFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;
    MI_VENC_RcParam_t stRcParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetJpegCurInfo(&stChnAttrGet, &getFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_MJPEGCBR;
    stChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateDen = 1;

    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamMjpegCbr.u32MaxQfactor = maxQfactor;
    stRcParam.stParamMjpegCbr.u32MinQfactor = minQfactor;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetJpegVbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQfactor = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQfactor = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int changePos  = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);

    int getFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;
    MI_VENC_RcParam_t stRcParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2],
              (const char *)pstCmd->cmdPara[3]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetJpegCurInfo(&stChnAttrGet, &getFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_MJPEGVBR;
    stChnAttr.stRcAttr.stAttrMjpegVbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateDen = 1;

    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamMjpegVbr.u32MaxQfactor = maxQfactor;
    stRcParam.stParamMjpegVbr.u32MinQfactor = minQfactor;
    stRcParam.stParamMjpegVbr.s32ChangePos  = changePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetJpegFixqp(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int qfactor = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    int getFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetJpegCurInfo(&stChnAttrGet, &getFps);

    stChnAttr.stRcAttr.eRcMode                           = E_MI_VENC_RC_MODE_MJPEGFIXQP;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor       = qfactor;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;

    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}

int PTREE_CMD_VENC_SetAv1Cbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    int getGop = 0;
    int getFps = 0;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s", (const char *)pstCmd->cmdId,
              paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetAv1CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                       = E_MI_VENC_RC_MODE_AV1CBR;
    stChnAttr.stRcAttr.stAttrAv1Cbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrAv1Cbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrAv1Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrAv1Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamAv1Cbr.u32MaxQp     = maxQp;
    stRcParam.stParamAv1Cbr.u32MinQp     = minQp;
    stRcParam.stParamAv1Cbr.u32MaxIQp    = maxIQp;
    stRcParam.stParamAv1Cbr.u32MinIQp    = minIQp;
    stRcParam.stParamAv1Cbr.s32IPQPDelta = ipqpDelta;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetAv1Vbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);
    int changePos = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s",
              (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetAv1CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                       = E_MI_VENC_RC_MODE_AV1VBR;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrAv1Vbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrAv1Vbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamAv1Vbr.u32MaxIQp    = maxIQp;
    stRcParam.stParamAv1Vbr.u32MinIQp    = minIQp;
    stRcParam.stParamAv1Vbr.s32IPQPDelta = ipqpDelta;
    stRcParam.stParamAv1Vbr.s32ChangePos = changePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetAv1Fixqp(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int iQp = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int pQp = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetAv1CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_AV1FIXQP;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32IQp           = iQp;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32PQp           = pQp;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrAv1Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrAv1Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}

int PTREE_CMD_VENC_SetAv1Avbr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int dev = pstModObj->info->devId;
    int chn = pstModObj->info->chnId;

    int bitrate              = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int maxQp                = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    int minQp                = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    int maxIQp               = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]);
    int minIQp               = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]);
    int ipqpDelta            = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]);
    int changePos            = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]);
    int u32MinStillPercent   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[7]);
    int u32MotionSensitivity = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[8]);

    int getGop = 0;
    int getFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s, p7->%s, p8->%s",
              (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6], (const char *)pstCmd->cmdPara[7],
              (const char *)pstCmd->cmdPara[8]);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    PTREE_CMD_VENC_GetAv1CurInfo(&stChnAttrGet, &getGop, &getFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_AV1AVBR;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32Gop           = getGop;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateNum = getFps;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateDen = 1;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamAv1Avbr.u32MaxIQp            = maxIQp;
    stRcParam.stParamAv1Avbr.u32MinIQp            = minIQp;
    stRcParam.stParamAv1Avbr.s32IPQPDelta         = ipqpDelta;
    stRcParam.stParamAv1Avbr.s32ChangePos         = changePos;
    stRcParam.stParamAv1Avbr.u32MinStillPercent   = u32MinStillPercent;
    stRcParam.stParamAv1Avbr.u32MotionSensitivity = u32MotionSensitivity;
    PTREE_DBG("changepos:%d, percent:%d,u32MotionSensitivity:%d.", changePos, u32MinStillPercent, u32MotionSensitivity);

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

int PTREE_CMD_VENC_SetRefParam(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int                dev         = pstModObj->info->devId;
    int                chn         = pstModObj->info->chnId;
    int                base        = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);
    int                enhance     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]);
    MI_BOOL            bEnablePred = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]);
    MI_S32             s32Ret      = MI_SUCCESS;
    MI_VENC_ParamRef_t stRefParam;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s", (const char *)pstCmd->cmdId, paraCnt,
              (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1], (const char *)pstCmd->cmdPara[2]);

    s32Ret = MI_VENC_GetRefParam(dev, chn, &stRefParam);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_DBG("MI_VENC_GetRefParam err0x%x", s32Ret);
        return 0;
    }

    stRefParam.u32Base     = base;
    stRefParam.u32Enhance  = enhance;
    stRefParam.bEnablePred = bEnablePred;
    s32Ret                 = MI_VENC_SetRefParam(dev, chn, &stRefParam);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_DBG("MI_VENC_SetRefParam err0x%x", s32Ret);
        return 0;
    }

    return 0;
}

int PTREE_CMD_VENC_SetRoiParam(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int     dev       = pstModObj->info->devId;
    int     chn       = pstModObj->info->chnId;
    int     index     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]); // index
    MI_BOOL bEnable   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[1]); // bEnable
    int     bAbsQp    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[2]); // bAbsQp
    int     s32Qp     = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[3]); // s32Qp
    int     u32Left   = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[4]); // u32Left
    int     u32Top    = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[5]); // u32Top
    int     u32Width  = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[6]); // u32Width
    int     u32Height = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[7]); // u32Height

    MI_VENC_RoiCfg_t  stRoiCfg;
    MI_VENC_ChnAttr_t stChnAttr;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s, p1->%s, p2->%s, p3->%s, p4->%s, p5->%s, p6->%s, p7->%s",
              (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0], (const char *)pstCmd->cmdPara[1],
              (const char *)pstCmd->cmdPara[2], (const char *)pstCmd->cmdPara[3], (const char *)pstCmd->cmdPara[4],
              (const char *)pstCmd->cmdPara[5], (const char *)pstCmd->cmdPara[6], (const char *)pstCmd->cmdPara[7]);

    memset(&stRoiCfg, 0, sizeof(MI_VENC_RoiCfg_t));
    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    MI_VENC_GetRoiCfg(dev, chn, index, &stRoiCfg);
    MI_VENC_GetChnAttr(dev, chn, &stChnAttr);

    PTREE_DBG("old: ch:%d, encodetype:%d, index:%d, enable:%d, bAbsQp:%d, Qp:%d X:%d,Y:%d,w:%d,h:%d !", chn,
              stChnAttr.stVeAttr.eType, stRoiCfg.u32Index, stRoiCfg.bEnable, stRoiCfg.bAbsQp, stRoiCfg.s32Qp,
              stRoiCfg.stRect.u32Left, stRoiCfg.stRect.u32Top, stRoiCfg.stRect.u32Width, stRoiCfg.stRect.u32Height);

    stRoiCfg.u32Index = index;
    stRoiCfg.bEnable  = bEnable;
    stRoiCfg.bAbsQp   = bAbsQp;
    stRoiCfg.s32Qp    = s32Qp;

    if (stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        stRoiCfg.stRect.u32Left   = ALIGN_16X_UP(u32Left);
        stRoiCfg.stRect.u32Top    = ALIGN_16X_UP(u32Top);
        stRoiCfg.stRect.u32Width  = ALIGN_16X_DOWN((u32Width));
        stRoiCfg.stRect.u32Height = ALIGN_16X_DOWN((u32Height));
    }
    else if (stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        stRoiCfg.stRect.u32Left   = ALIGN_32X_UP(u32Left);
        stRoiCfg.stRect.u32Top    = ALIGN_32X_UP(u32Top);
        stRoiCfg.stRect.u32Width  = ALIGN_32X_DOWN((u32Width));
        stRoiCfg.stRect.u32Height = ALIGN_32X_DOWN((u32Height));
    }
    else if (stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_AV1)
    {
        stRoiCfg.stRect.u32Left   = ALIGN_64X_UP(u32Left);
        stRoiCfg.stRect.u32Top    = ALIGN_64X_UP(u32Top);
        stRoiCfg.stRect.u32Width  = ALIGN_64X_DOWN((u32Width));
        stRoiCfg.stRect.u32Height = ALIGN_64X_DOWN((u32Height));
    }
    else
    {
        PTREE_DBG("only support H264 && H265 &&Av1, so setfail, current coderType: %d", stChnAttr.stVeAttr.eType);
        return -1;
    }

    PTREE_DBG("set: ch:%d, encodetype:%d, index:%d, enable:%d, bAbsQp:%d, Qp:%d X:%d,Y:%d,w:%d,h:%d !", chn,
              stChnAttr.stVeAttr.eType, stRoiCfg.u32Index, stRoiCfg.bEnable, stRoiCfg.bAbsQp, stRoiCfg.s32Qp,
              stRoiCfg.stRect.u32Left, stRoiCfg.stRect.u32Top, stRoiCfg.stRect.u32Width, stRoiCfg.stRect.u32Height);
    MI_VENC_SetRoiCfg(dev, chn, &stRoiCfg);

    return 0;
}

int PTREE_CMD_VENC_RequestIdr(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    int     dev      = pstModObj->info->devId;
    int     chn      = pstModObj->info->chnId;
    MI_BOOL bInstant = SSOS_IO_Atoi((const char *)pstCmd->cmdPara[0]);

    MI_S32 s32Ret = MI_SUCCESS;
    bInstant      = TRUE;

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    s32Ret = MI_VENC_RequestIdr(dev, chn, bInstant);
    if (MI_SUCCESS != s32Ret)
    {
        PTREE_DBG("MI_VENC_RequestIDR err0x%x", s32Ret);
        return 0;
    }

    return 0;
}

int PTREE_CMD_VENC_ChangeEnType(PTREE_MOD_Obj_t *pstModObj, const PTREE_CMD_Args_t *pstCmd, unsigned int paraCnt)
{
    const char *enType = (const char *)pstCmd->cmdPara[0];

    PTREE_SUR_VENC_Info_t *pstVencInfo = NULL;
    pstVencInfo                        = CONTAINER_OF(pstModObj->info, PTREE_SUR_VENC_Info_t, base.base);

    PTREE_DBG("Mod %s, Sec %s, Dev %d, Chn %d, In %d, Out %d", pstModObj->info->typeName, pstModObj->info->sectionName,
              pstModObj->info->devId, pstModObj->info->chnId, pstModObj->info->inCnt, pstModObj->info->outCnt);
    PTREE_DBG("Cmd Str %s, ParaCnt %d, p0->%s", (const char *)pstCmd->cmdId, paraCnt, (const char *)pstCmd->cmdPara[0]);

    if (!strcmp(enType, "h264"))
    {
        pstVencInfo->pEncodeType = E_PTREE_PACKET_VIDEO_STREAM_H264;
    }
    else if (!strcmp(enType, "h265"))
    {
        pstVencInfo->pEncodeType = E_PTREE_PACKET_VIDEO_STREAM_H265;
    }
    else if (!strcmp(enType, "av1"))
    {
        pstVencInfo->pEncodeType = E_PTREE_PACKET_VIDEO_STREAM_AV1;
    }
    else if (!strcmp(enType, "jpeg"))
    {
        pstVencInfo->pEncodeType = E_PTREE_PACKET_VIDEO_STREAM_JPEG;
    }

    return 0;
}

PTREE_MAKER_CMD_INIT(VENC, PTREE_CMD_StrCompare, PTREE_CMD_StrHashVal,
                     {(unsigned long)"set_rc_fps", PTREE_CMD_VENC_SetRcFps, 1},
                     {(unsigned long)"set_gop", PTREE_CMD_VENC_SetGop, 1},
                     {(unsigned long)"set_h264_cbr", PTREE_CMD_VENC_SetH264Cbr, 6},
                     {(unsigned long)"set_h264_vbr", PTREE_CMD_VENC_SetH264Vbr, 7},
                     {(unsigned long)"set_h264_fixqp", PTREE_CMD_VENC_SetH264Fixqp, 2},
                     {(unsigned long)"set_h264_avbr", PTREE_CMD_VENC_SetH264Avbr, 9},
                     {(unsigned long)"set_h265_cbr", PTREE_CMD_VENC_SetH265Cbr, 6},
                     {(unsigned long)"set_h265_vbr", PTREE_CMD_VENC_SetH265Vbr, 7},
                     {(unsigned long)"set_h265_fixqp", PTREE_CMD_VENC_SetH265Fixqp, 2},
                     {(unsigned long)"set_h265_avbr", PTREE_CMD_VENC_SetH265Avbr, 9},
                     {(unsigned long)"set_jpeg_cbr", PTREE_CMD_VENC_SetJpegCbr, 3},
                     {(unsigned long)"set_jpeg_vbr", PTREE_CMD_VENC_SetJpegVbr, 4},
                     {(unsigned long)"set_jpeg_fixqp", PTREE_CMD_VENC_SetJpegFixqp, 1},
                     {(unsigned long)"set_av1_cbr", PTREE_CMD_VENC_SetAv1Cbr, 6},
                     {(unsigned long)"set_av1_vbr", PTREE_CMD_VENC_SetAv1Vbr, 7},
                     {(unsigned long)"set_av1_fixqp", PTREE_CMD_VENC_SetAv1Fixqp, 2},
                     {(unsigned long)"set_av1_avbr", PTREE_CMD_VENC_SetAv1Avbr, 9},
                     {(unsigned long)"set_refparam", PTREE_CMD_VENC_SetRefParam, 3},
                     {(unsigned long)"set_roiparam", PTREE_CMD_VENC_SetRoiParam, 8},
                     {(unsigned long)"request_idr", PTREE_CMD_VENC_RequestIdr, 1},
                     {(unsigned long)"change_entype", PTREE_CMD_VENC_ChangeEnType, 1})
