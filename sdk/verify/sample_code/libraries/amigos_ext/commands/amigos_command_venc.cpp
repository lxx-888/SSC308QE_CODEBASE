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
#include "amigos_module_venc.h"
#include "mi_venc.h"
#include "ss_cmd_base.h"

#define ALIGN_16xUP(x)   (((x + 15) / 16) * 16)
#define ALIGN_32xUP(x)   (((x + 31) / 32) * 32)
#define ALIGN_64xUP(x)   (((x + 63) / 64) * 64)
#define ALIGN_16xDOWN(x) (x & 0xFFFFFFF0)
#define ALIGN_32xDOWN(x) (x & 0xFFFFFFE0)
#define ALIGN_64xDOWN(x) (x & 0xFFFFFFC0)

static void GetH264CurInfo(MI_VENC_ChnAttr_t GetChnAttr, int& Gop ,int& Fps)
{
    if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264CBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrH264Cbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum;
    }
    else if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264VBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrH264Vbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum;
    }
    else if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H264AVBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrH264Avbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum;
    }
    else
    {
        GetChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
        Gop = GetChnAttr.stRcAttr.stAttrH264FixQp.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum;
    }
    return;
}

static void GetH265CurInfo(MI_VENC_ChnAttr_t GetChnAttr, int& Gop ,int& Fps)
{
    if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265CBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrH265Cbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum;
    }
    else if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265VBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrH265Vbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum;
    }
    else if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_H265AVBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrH265Avbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum;
    }
    else
    {
        GetChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
        Gop = GetChnAttr.stRcAttr.stAttrH265FixQp.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum;
    }
    return;
}

static void GetAv1CurInfo(MI_VENC_ChnAttr_t GetChnAttr, int& Gop ,int& Fps)
{
    if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_AV1CBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrAv1Cbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum;
    }
    else if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_AV1VBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrAv1Vbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateNum;
    }
    else if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_AV1AVBR)
    {
        Gop = GetChnAttr.stRcAttr.stAttrAv1Avbr.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateNum;
    }
    else
    {
        GetChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_AV1FIXQP;
        Gop = GetChnAttr.stRcAttr.stAttrAv1FixQp.u32Gop;
        Fps = GetChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateNum;
    }
    return;
}

static void GetJpegCurInfo(MI_VENC_ChnAttr_t GetChnAttr, int& Fps)
{
    if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_MJPEGCBR)
    {
        Fps = GetChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum;
    }
    else if(GetChnAttr.stRcAttr.eRcMode == E_MI_VENC_RC_MODE_MJPEGVBR)
    {
        Fps = GetChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateNum;
    }
    else
    {
        GetChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
        Fps = GetChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum;
    }
    return;
}

static int SetFps(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev       = stInfo.devId;
    int chn       = stInfo.chnId;
    int frameRate = ss_cmd_atoi(in_strs[1].c_str());

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_RcMode_e  eRcModeType = E_MI_VENC_RC_MODE_MAX;
    MI_VENC_ModType_e enType      = E_MI_VENC_MODTYPE_VENC;
    MI_U32            tFnum = 0x0, tFden = 0x0;

    if (frameRate == 0x0)
    {
        ss_print(PRINT_LV_TRACE, "you can not set venc fps == 0. return err.\n");
        return 0;
    }
    else
    {
        tFnum = frameRate ;
        tFden = 1;
    }
    ss_print(PRINT_LV_TRACE, "the venc tFnum:%d, tFden:%d\n", tFnum, tFden);

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    MI_VENC_GetChnAttr((MI_VENC_DEV)dev, chn, &stChnAttr);

    eRcModeType = stChnAttr.stRcAttr.eRcMode;
    enType      = stChnAttr.stVeAttr.eType;
    ss_print(PRINT_LV_TRACE, "the enType is %d\n",enType);
    ss_print(PRINT_LV_TRACE, "the eRcModeType is %d\n",eRcModeType);

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
                ss_print(PRINT_LV_TRACE, "not init rcmode  type, eRcModeType:%d\n", eRcModeType);
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
                ss_print(PRINT_LV_TRACE, "not init rcmode  type, eRcModeType:%d\n", eRcModeType);
            }
        }
        break;

        case E_MI_VENC_MODTYPE_JPEGE:
        {
            if(E_MI_VENC_RC_MODE_MJPEGCBR == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateDen = tFden;

            }
            else if(E_MI_VENC_RC_MODE_MJPEGFIXQP == eRcModeType)
            {
                stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = tFnum;
                stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = tFden;
            }
            else
            {
               ss_print(PRINT_LV_TRACE,"rcMode:%d is not supported.\n", eRcModeType);
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
                ss_print(PRINT_LV_TRACE, "not init rcmode  type, eRcModeType:%d\n", eRcModeType);
            }
        }
        break;

        default:
            ss_print(PRINT_LV_TRACE, "not init video type, error video encoder type:%d\n", enType);
            break;
    }
    MI_VENC_SetChnAttr((MI_VENC_DEV)dev, chn, &stChnAttr);
    return 0;
}

static int SetGop(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int               dev = stInfo.devId;
    int               chn = stInfo.chnId;
    int               gop = ss_cmd_atoi(in_strs[1].c_str());
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_RcMode_e  eRcModeType = E_MI_VENC_RC_MODE_MAX;
    MI_VENC_ModType_e enType      = E_MI_VENC_MODTYPE_VENC;

    if (MI_SUCCESS != MI_VENC_GetChnAttr(dev, chn, &stChnAttr))
    {
        ss_print(PRINT_LV_TRACE, "venc:%d can not get gop\n", chn);
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
                ss_print(PRINT_LV_TRACE, "not init rcmode  type, eRcModeType:%d\n", eRcModeType);
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
                ss_print(PRINT_LV_TRACE, "not init rcmode type, eRcModeType:%d\n", eRcModeType);
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
                ss_print(PRINT_LV_TRACE, "not init rcmode  type, eRcModeType:%d\n", eRcModeType);
            }
        }
        break;

        default:
            ss_print(PRINT_LV_TRACE, "not init video type, error video encoder type:%d (E_MI_VENC_MODTYPE_JPEGE)\n",
                     enType);
            return 0;
    }

    if (MI_SUCCESS != MI_VENC_SetChnAttr(dev, chn, &stChnAttr))
    {
        ss_print(PRINT_LV_TRACE, "venc:%d can not set gop\n", chn);
        return 0;
    }

    ss_print(PRINT_LV_TRACE, "venc:%d set gop:%d ok!\n", chn, gop);
    return 0;
}

static int SetH264Cbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate   = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp     = ss_cmd_atoi(in_strs[2].c_str());
    int minQp     = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp    = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp    = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta = ss_cmd_atoi(in_strs[6].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH264CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H264CBR;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = GetFps;
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
    stRcParam.stParamH264Cbr.s32IPQPDelta = IPQPDelta;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetH264Vbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate   = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp     = ss_cmd_atoi(in_strs[2].c_str());
    int minQp     = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp    = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp    = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta = ss_cmd_atoi(in_strs[6].c_str());
    int ChangePos = ss_cmd_atoi(in_strs[7].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH264CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H264VBR;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH264Vbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH264Vbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH264VBR.u32MaxIQp    = maxIQp;
    stRcParam.stParamH264VBR.u32MinIQp    = minIQp;
    stRcParam.stParamH264VBR.s32IPQPDelta = IPQPDelta;
    stRcParam.stParamH264VBR.s32ChangePos = ChangePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetH264Fixqp(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    AmigosSurfaceVenc:: VencInfo stVencInfo;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int IQp = ss_cmd_atoi(in_strs[1].c_str());
    int PQp = ss_cmd_atoi(in_strs[2].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH264CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                          = E_MI_VENC_RC_MODE_H264FIXQP;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32IQp           = IQp;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32PQp           = PQp;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;

    // stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}

static int SetH264Avbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate              = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp                = ss_cmd_atoi(in_strs[2].c_str());
    int minQp                = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp               = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp               = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta            = ss_cmd_atoi(in_strs[6].c_str());
    int ChangePos            = ss_cmd_atoi(in_strs[7].c_str());
    int u32MinStillPercent   = ss_cmd_atoi(in_strs[8].c_str());
    int u32MotionSensitivity = ss_cmd_atoi(in_strs[9].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH264CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H264AVBR;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrH264Avbr.u32SrcFrmRateDen = 1;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH264Avbr.u32MaxIQp            = maxIQp;
    stRcParam.stParamH264Avbr.u32MinIQp            = minIQp;
    stRcParam.stParamH264Avbr.s32IPQPDelta         = IPQPDelta;
    stRcParam.stParamH264Avbr.s32ChangePos         = ChangePos;
    stRcParam.stParamH264Avbr.u32MinStillPercent   = u32MinStillPercent;

    stRcParam.stParamH264Avbr.u32MotionSensitivity = u32MotionSensitivity;
    ss_print(PRINT_LV_TRACE, "changepos:%d, percent:%d ,u32MotionSensitivity:%d.\n",ChangePos, u32MinStillPercent, u32MotionSensitivity);

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetH265Cbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate   = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp     = ss_cmd_atoi(in_strs[2].c_str());
    int minQp     = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp    = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp    = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta = ss_cmd_atoi(in_strs[6].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH265CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H265CBR;
    stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = GetFps;
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
    stRcParam.stParamH265Cbr.s32IPQPDelta = IPQPDelta;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetH265Vbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate   = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp     = ss_cmd_atoi(in_strs[2].c_str());
    int minQp     = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp    = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp    = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta = ss_cmd_atoi(in_strs[6].c_str());
    int ChangePos = ss_cmd_atoi(in_strs[7].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH265CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_H265VBR;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH265Vbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH265Vbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH265Vbr.u32MaxIQp    = maxIQp;
    stRcParam.stParamH265Vbr.u32MinIQp    = minIQp;
    stRcParam.stParamH265Vbr.s32IPQPDelta = IPQPDelta;
    stRcParam.stParamH265Vbr.s32ChangePos = ChangePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetH265Fixqp(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int IQp = ss_cmd_atoi(in_strs[1].c_str());
    int PQp = ss_cmd_atoi(in_strs[2].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH265CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                          = E_MI_VENC_RC_MODE_H265FIXQP;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32IQp           = IQp;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32PQp           = PQp;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}

static int SetH265Avbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate              = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp                = ss_cmd_atoi(in_strs[2].c_str());
    int minQp                = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp               = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp               = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta            = ss_cmd_atoi(in_strs[6].c_str());
    int ChangePos            = ss_cmd_atoi(in_strs[7].c_str());
    int u32MinStillPercent   = ss_cmd_atoi(in_strs[8].c_str());
    int u32MotionSensitivity = ss_cmd_atoi(in_strs[9].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetH265CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H265AVBR;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrH265Avbr.u32SrcFrmRateDen = 1;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamH265Avbr.u32MaxIQp            = maxIQp;
    stRcParam.stParamH265Avbr.u32MinIQp            = minIQp;
    stRcParam.stParamH265Avbr.s32IPQPDelta         = IPQPDelta;
    stRcParam.stParamH265Avbr.s32ChangePos         = ChangePos;
    stRcParam.stParamH265Avbr.u32MinStillPercent   = u32MinStillPercent;
    stRcParam.stParamH265Avbr.u32MotionSensitivity = u32MotionSensitivity;
    ss_print(PRINT_LV_TRACE, "changepos:%d, percent:%d,u32MotionSensitivity:%d.\n", ChangePos, u32MinStillPercent, u32MotionSensitivity);

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetJpegCbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate = ss_cmd_atoi(in_strs[1].c_str());
    int MaxQfactor = ss_cmd_atoi(in_strs[2].c_str());
    int MinQfactor = ss_cmd_atoi(in_strs[3].c_str());

    int GetFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;
    MI_VENC_RcParam_t stRcParam;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetJpegCurInfo(stChnAttrGet, GetFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_MJPEGCBR;
    stChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateDen = 1;

    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamMjpegCbr.u32MaxQfactor = MaxQfactor;
    stRcParam.stParamMjpegCbr.u32MinQfactor = MinQfactor;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetJpegVbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate = ss_cmd_atoi(in_strs[1].c_str());
    int MaxQfactor = ss_cmd_atoi(in_strs[2].c_str());
    int MinQfactor = ss_cmd_atoi(in_strs[3].c_str());
    int ChangePos = ss_cmd_atoi(in_strs[4].c_str());

    int GetFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;
    MI_VENC_RcParam_t stRcParam;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetJpegCurInfo(stChnAttrGet, GetFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_MJPEGVBR;
    stChnAttr.stRcAttr.stAttrMjpegVbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrMjpegVbr.u32SrcFrmRateDen = 1;

    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamMjpegVbr.u32MaxQfactor = MaxQfactor;
    stRcParam.stParamMjpegVbr.u32MinQfactor = MinQfactor;
    stRcParam.stParamMjpegVbr.s32ChangePos  = ChangePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetJpegFixqp(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int Qfactor = ss_cmd_atoi(in_strs[1].c_str());

    int GetFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetJpegCurInfo(stChnAttrGet, GetFps);

    stChnAttr.stRcAttr.eRcMode                           = E_MI_VENC_RC_MODE_MJPEGFIXQP;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor       = Qfactor;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;

    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}


static int SetAv1Cbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate   = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp     = ss_cmd_atoi(in_strs[2].c_str());
    int minQp     = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp    = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp    = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta = ss_cmd_atoi(in_strs[6].c_str());

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    int GetGop = 0;
    int GetFps = 0;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetAv1CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_AV1CBR;
    stChnAttr.stRcAttr.stAttrAv1Cbr.u32BitRate       = bitrate;
    stChnAttr.stRcAttr.stAttrAv1Cbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum = GetFps;
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
    stRcParam.stParamAv1Cbr.s32IPQPDelta = IPQPDelta;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetAv1Vbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate   = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp     = ss_cmd_atoi(in_strs[2].c_str());
    int minQp     = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp    = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp    = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta = ss_cmd_atoi(in_strs[6].c_str());
    int ChangePos = ss_cmd_atoi(in_strs[7].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetAv1CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_AV1VBR;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrAv1Vbr.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrAv1Vbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrAv1Vbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamAv1Vbr.u32MaxIQp    = maxIQp;
    stRcParam.stParamAv1Vbr.u32MinIQp    = minIQp;
    stRcParam.stParamAv1Vbr.s32IPQPDelta = IPQPDelta;
    stRcParam.stParamAv1Vbr.s32ChangePos = ChangePos;

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetAv1Fixqp(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int IQp = ss_cmd_atoi(in_strs[1].c_str());
    int PQp = ss_cmd_atoi(in_strs[2].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetAv1CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                          = E_MI_VENC_RC_MODE_AV1FIXQP;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32IQp           = IQp;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32PQp           = PQp;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrAv1FixQp.u32SrcFrmRateDen = 1;
    // stChnAttr.stRcAttr.stAttrAv1Cbr.u32StatTime = 0;
    // stChnAttr.stRcAttr.stAttrAv1Cbr.u32FluctuateLevel = 0;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}

static int SetAv1Avbr(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    int bitrate              = ss_cmd_atoi(in_strs[1].c_str());
    int maxQp                = ss_cmd_atoi(in_strs[2].c_str());
    int minQp                = ss_cmd_atoi(in_strs[3].c_str());
    int maxIQp               = ss_cmd_atoi(in_strs[4].c_str());
    int minIQp               = ss_cmd_atoi(in_strs[5].c_str());
    int IPQPDelta            = ss_cmd_atoi(in_strs[6].c_str());
    int ChangePos            = ss_cmd_atoi(in_strs[7].c_str());
    int u32MinStillPercent   = ss_cmd_atoi(in_strs[8].c_str());
    int u32MotionSensitivity = ss_cmd_atoi(in_strs[9].c_str());

    int GetGop = 0;
    int GetFps = 0;

    MI_VENC_RcParam_t stRcParam;
    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_ChnAttr_t stChnAttrGet;

    memset(&stChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));
    memset(&stChnAttrGet, 0x00, sizeof(MI_VENC_ChnAttr_t));

    MI_VENC_GetChnAttr(dev, chn, &stChnAttrGet);
    memcpy(&stChnAttr.stVeAttr, &stChnAttrGet.stVeAttr, sizeof(MI_VENC_Attr_t));

    GetAv1CurInfo(stChnAttrGet, GetGop, GetFps);

    stChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_AV1AVBR;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxBitRate    = bitrate;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32MaxQp         = maxQp;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32MinQp         = minQp;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32Gop           = GetGop;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateNum = GetFps;
    stChnAttr.stRcAttr.stAttrAv1Avbr.u32SrcFrmRateDen = 1;
    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    memset(&stRcParam, 0x00, sizeof(MI_VENC_RcParam_t));
    MI_VENC_GetRcParam(dev, chn, &stRcParam);

    stRcParam.stParamAv1Avbr.u32MaxIQp            = maxIQp;
    stRcParam.stParamAv1Avbr.u32MinIQp            = minIQp;
    stRcParam.stParamAv1Avbr.s32IPQPDelta         = IPQPDelta;
    stRcParam.stParamAv1Avbr.s32ChangePos         = ChangePos;
    stRcParam.stParamAv1Avbr.u32MinStillPercent   = u32MinStillPercent;
    stRcParam.stParamAv1Avbr.u32MotionSensitivity = u32MotionSensitivity;
    ss_print(PRINT_LV_TRACE, "changepos:%d, percent:%d,u32MotionSensitivity:%d.\n",ChangePos, u32MinStillPercent, u32MotionSensitivity);

    MI_VENC_SetRcParam(dev, chn, &stRcParam);

    return 0;
}

static int SetAv1SwitchFrame(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    MI_BOOL bEnableSwitchFrame   = ss_cmd_atoi(in_strs[1].c_str());

    MI_VENC_ChnAttr_t stChnAttr;
    MI_VENC_GetChnAttr(dev, chn, &stChnAttr);

    stChnAttr.stVeAttr.stAttrAv1.bEnableSwitchFrame = bEnableSwitchFrame;

    MI_VENC_SetChnAttr(dev, chn, &stChnAttr);

    return 0;
}


static int SetRefParam(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int  dev         = stInfo.devId;
    int  chn         = stInfo.chnId;
    int  Base        = ss_cmd_atoi(in_strs[1].c_str());
    int  Enhance     = ss_cmd_atoi(in_strs[2].c_str());
    bool bEnablePred = ss_cmd_atoi(in_strs[3].c_str());

    MI_S32             s32Ret = 0;
    MI_VENC_ParamRef_t stRefParam;
    s32Ret = MI_VENC_GetRefParam(dev, chn, &stRefParam);
    if (MI_SUCCESS != s32Ret)
    {
        ss_print(PRINT_LV_TRACE, "MI_VENC_GetRefParam err0x%x\n", s32Ret);
        return 0;
    }

    stRefParam.u32Base     = Base;
    stRefParam.u32Enhance  = Enhance;
    stRefParam.bEnablePred = bEnablePred;
    s32Ret                 = MI_VENC_SetRefParam(dev, chn, &stRefParam);
    if (MI_SUCCESS != s32Ret)
    {
        ss_print(PRINT_LV_TRACE, "MI_VENC_SetRefParam err0x%x\n", s32Ret);
        return 0;
    }

    return 0;
}

static int SetRoiParam(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int  dev       = stInfo.devId;
    int  chn       = stInfo.chnId;
    int  index     = ss_cmd_atoi(in_strs[1].c_str()); // index
    bool bEnable   = ss_cmd_atoi(in_strs[2].c_str()); // bEnable
    int  bAbsQp    = ss_cmd_atoi(in_strs[3].c_str()); // bAbsQp
    int  s32Qp     = ss_cmd_atoi(in_strs[4].c_str()); // s32Qp
    int  u32Left   = ss_cmd_atoi(in_strs[5].c_str()); // u32Left
    int  u32Top    = ss_cmd_atoi(in_strs[6].c_str()); // u32Top
    int  u32Width  = ss_cmd_atoi(in_strs[7].c_str()); // u32Width
    int  u32Height = ss_cmd_atoi(in_strs[8].c_str()); // u32Height

    MI_VENC_RoiCfg_t  stRoiCfg;
    MI_VENC_ChnAttr_t stChnAttr;

    memset(&stRoiCfg, 0, sizeof(MI_VENC_RoiCfg_t));
    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    MI_VENC_GetRoiCfg(dev, chn, index, &stRoiCfg);
    MI_VENC_GetChnAttr(dev, chn, &stChnAttr);

    ss_print(PRINT_LV_TRACE, "old: ch:%d, encodetype:%d, index:%d, enable:%d, bAbsQp:%d, Qp:%d X:%d,Y:%d,w:%d,h:%d !\n",
             chn, stChnAttr.stVeAttr.eType, stRoiCfg.u32Index, stRoiCfg.bEnable, stRoiCfg.bAbsQp, stRoiCfg.s32Qp,
             stRoiCfg.stRect.u32Left, stRoiCfg.stRect.u32Top, stRoiCfg.stRect.u32Width, stRoiCfg.stRect.u32Height);

    stRoiCfg.u32Index = index;
    stRoiCfg.bEnable  = bEnable;
    stRoiCfg.bAbsQp   = bAbsQp;
    stRoiCfg.s32Qp    = s32Qp;

    if (stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        stRoiCfg.stRect.u32Left   = ALIGN_16xUP(u32Left);
        stRoiCfg.stRect.u32Top    = ALIGN_16xUP(u32Top);
        stRoiCfg.stRect.u32Width  = ALIGN_16xDOWN((u32Width));
        stRoiCfg.stRect.u32Height = ALIGN_16xDOWN((u32Height));
    }
    else if (stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        stRoiCfg.stRect.u32Left   = ALIGN_32xUP(u32Left);
        stRoiCfg.stRect.u32Top    = ALIGN_32xUP(u32Top);
        stRoiCfg.stRect.u32Width  = ALIGN_32xDOWN((u32Width));
        stRoiCfg.stRect.u32Height = ALIGN_32xDOWN((u32Height));
    }
    else if (stChnAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_AV1)
    {
        stRoiCfg.stRect.u32Left   = ALIGN_64xUP(u32Left);
        stRoiCfg.stRect.u32Top    = ALIGN_64xUP(u32Top);
        stRoiCfg.stRect.u32Width  = ALIGN_64xDOWN((u32Width));
        stRoiCfg.stRect.u32Height = ALIGN_64xDOWN((u32Height));
    }
    else
    {
        ss_print(PRINT_LV_TRACE, "only support H264 && H265 &&Av1, so setfail, current coderType: %d\n",
                 stChnAttr.stVeAttr.eType);
        return -1;
    }

    ss_print(PRINT_LV_TRACE, "set: ch:%d, encodetype:%d, index:%d, enable:%d, bAbsQp:%d, Qp:%d X:%d,Y:%d,w:%d,h:%d !\n",
             chn, stChnAttr.stVeAttr.eType, stRoiCfg.u32Index, stRoiCfg.bEnable, stRoiCfg.bAbsQp, stRoiCfg.s32Qp,
             stRoiCfg.stRect.u32Left, stRoiCfg.stRect.u32Top, stRoiCfg.stRect.u32Width, stRoiCfg.stRect.u32Height);
    MI_VENC_SetRoiCfg(dev, chn, &stRoiCfg);

    return 0;
}

static int RequestIdr(vector<string> &in_strs)
{
    AmigosModuleVenc          *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int  dev      = stInfo.devId;
    int  chn      = stInfo.chnId;
    bool bInstant = ss_cmd_atoi(in_strs[1].c_str());

    MI_S32 s32Ret;
    bInstant = TRUE;
    s32Ret   = MI_VENC_RequestIdr(dev, chn, bInstant);
    if (MI_SUCCESS != s32Ret)
    {
        ss_print(PRINT_LV_TRACE, "MI_VENC_RequestIDR err0x%x\n", s32Ret);
        return 0;
    }

    return 0;
}

static int SetSuperFrame(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;

    MI_U32 i_value = ss_cmd_atoi(in_strs[1].c_str()); // isuper
    MI_U32 p_value = ss_cmd_atoi(in_strs[2].c_str()); // psuper
    MI_U32 mode    = ss_cmd_atoi(in_strs[3].c_str()); // supermode

    MI_S32                  s32Ret;
    MI_VENC_SuperFrameCfg_t stSuperFrmParam;

    memset(&stSuperFrmParam, 0, sizeof(MI_VENC_SuperFrameCfg_t));
    s32Ret = MI_VENC_GetSuperFrameCfg(dev, chn, &stSuperFrmParam);
    ss_print(PRINT_LV_TRACE,
             "venChId=%d, (old)SuperIFrmBitsThr=%d, SuperPFrmBitsThr=%d, SuperBFrmBitsThr=%d,mode = %d\n", chn,
             stSuperFrmParam.u32SuperIFrmBitsThr, stSuperFrmParam.u32SuperPFrmBitsThr,
             stSuperFrmParam.u32SuperBFrmBitsThr, stSuperFrmParam.eSuperFrmMode);

    if (i_value >= 0)
    {
        stSuperFrmParam.u32SuperIFrmBitsThr = i_value;
    }
    if (p_value >= 0)
    {
        stSuperFrmParam.u32SuperPFrmBitsThr = p_value;
    }
    stSuperFrmParam.eSuperFrmMode = (MI_VENC_SuperFrmMode_e)mode;

    if (MI_SUCCESS != s32Ret)
    {
        ss_print(PRINT_LV_TRACE, "MI_VENC_GetSuperFrameCfg error, %X\n", s32Ret);
        return 0;
    }

    ss_print(PRINT_LV_TRACE,
             "venChId=%d, (set)SuperIFrmBitsThr=%d, SuperPFrmBitsThr=%d, SuperBFrmBitsThr=%d,mode = %d\n", chn,
             stSuperFrmParam.u32SuperIFrmBitsThr, stSuperFrmParam.u32SuperPFrmBitsThr,
             stSuperFrmParam.u32SuperBFrmBitsThr, stSuperFrmParam.eSuperFrmMode);

    s32Ret = MI_VENC_SetSuperFrameCfg(dev, chn, &stSuperFrmParam);
    if (MI_SUCCESS != s32Ret)
    {
        ss_print(PRINT_LV_TRACE, "MI_VENC_SetSuperFrameCfg err0x%x\n", s32Ret);
        return 0;
    }
    return 0;
}

static int SetSei(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int            dev         = stInfo.devId;
    int            chn         = stInfo.chnId;
    unsigned char *au8UserData = NULL;
    au8UserData                = (unsigned char *)in_strs[1].c_str();

    MI_S32 s32Ret;
    s32Ret = MI_VENC_InsertUserData(dev, chn, au8UserData, sizeof(au8UserData));
    if (MI_SUCCESS != s32Ret)
    {
        ss_print(PRINT_LV_TRACE, "MI_VENC_InsertUserData err0x%x\n", s32Ret);
        return 0;
    }

    return 0;
}

static int SetStreamWidthHight(vector<string> &in_strs)
{
    AmigosModuleVenc *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleVenc, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int            dev         = stInfo.devId;
    int            chn         = stInfo.chnId;

    MI_U32 width = ss_cmd_atoi(in_strs[1].c_str());
    MI_U32 height = ss_cmd_atoi(in_strs[2].c_str());

    MI_VENC_ChnAttr_t stAttr;
    memset(&stAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    if (MI_SUCCESS != MI_VENC_GetChnAttr(dev, chn, &stAttr))
    {
        ss_print("MI_VENC_GetChnAttr Failed\n");
        return 0;
    }
    if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H264E)
    {
        stAttr.stVeAttr.stAttrH264e.u32PicWidth = width;
        stAttr.stVeAttr.stAttrH264e.u32PicHeight = height;
        MI_VENC_RequestIdr(dev, chn, TRUE);
    }
    else if(stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_H265E)
    {
        stAttr.stVeAttr.stAttrH265e.u32PicWidth = width;
        stAttr.stVeAttr.stAttrH265e.u32PicHeight = height;
        MI_VENC_RequestIdr(dev, chn, TRUE);
    }
    else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        stAttr.stVeAttr.stAttrJpeg.u32PicWidth = width;
        stAttr.stVeAttr.stAttrJpeg.u32PicHeight = height;
    }
    else if (stAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_AV1)
    {
        stAttr.stVeAttr.stAttrAv1.u32PicWidth = width;
        stAttr.stVeAttr.stAttrAv1.u32PicHeight = height;
        MI_VENC_RequestIdr(dev, chn, TRUE);
    }
    else
    {
        ss_print("Enc type %d is not support\n", stAttr.stVeAttr.eType);
        return 0;
    }
    if (MI_SUCCESS != MI_VENC_SetChnAttr(dev, chn, &stAttr))
    {
        ss_print("MI_VENC_SetChnAttr Failed\n");
        return 0;
    }
    return 0;
}

MOD_CMDS(AmiCmdVenc)
{
    ADD_CMD("set_fps", SetFps, 1);
    ADD_CMD_HELP("set_fps", "[fps]", "set fps");

    ADD_CMD("set_gop", SetGop, 1);
    ADD_CMD_HELP("set_gop", "[gop] ", "set gop value");

    ADD_CMD("request_idr", RequestIdr, 1);
    ADD_CMD_HELP("request_idr", "[bInstant]",
                 "request idr frame",
                 "[bInstant]: enable request");

    ADD_CMD("set_sei", SetSei, 1);
    ADD_CMD_HELP("set_sei", "[userdata] ",
                 "Supplemental Enhancement information",
                 "[userdata]: user data");

    ADD_CMD("set_superframe", SetSuperFrame, 3);
    ADD_CMD_HELP("set_superframe","[ibps] [pbps] [mode]",
                 "set super frame",
                 "[ibps]: I-frame super frame bps",
                 "[pbps]: P-frame super frame bps",
                 "[mode]: super frame mode (0->NONE, 1->DISCARD, 2->REENCODE)");

    ADD_CMD("set_refparam", SetRefParam, 3);
    ADD_CMD_HELP("set_refparam", "[base] [enhance] [bEnablePred] ",
                 "set ref param",
                 "[base]: base cycle ",
                 "[enhance]: enhance cycle",
                 "[bEnablePred]: bEnablePred = 0 --> base ref IDR frame",
                 "mode: normalp、LTR_I、LTR_VI、TSVC-2、TSVC-3",
                 "example:"
                 "normalp:  1 0 0",
                 "LTR_I:  1 29 0",
                 "LTR_VI:  1 29 1",
                 "TSVC-2:  1 1 1",
                 "TSVC-3:  1 3 1");

    ADD_CMD("set_roiparam", SetRoiParam, 8);
    ADD_CMD_HELP("set_roiparam", "[Index] [enable] [bAbsQp] [Qp value] [Left] [top] [Width] [Height] ",
                 "example: set_roiparam 0 1 0 20 10 20 200 200",
                 "[Index]: [0,7]. roi Index",
                 "[enable: [0,1]. roi enable",
                 "[bAbsQp]: [0,1]. roi bAbsQp [Absolutely qp:1, Relative qp:0]",
                 "[Qp value]: roi Qp value",
                 "[Left]: roi Rect Left [H264 n*16] [H265 n*32]",
                 "[top]: roi Rect top [H264 n*16] [H265 n*32]",
                 "[Width]: roi Rect Width [H264 n*16] [H265 n*32]",
                 "[Height]: roi Rect Height[H264 n*16][H265 n*32]");

    // set h264 rcattr
    ADD_CMD("set_h264_cbr", SetH264Cbr, 6);
    ADD_CMD_HELP("set_h264_cbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta]",
                 "set rcattr 264 cbr mode param",
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]");

    ADD_CMD("set_h264_vbr", SetH264Vbr, 7);
    ADD_CMD_HELP("set_h264_vbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta] [ChangePos]",
                 "set rcattr h265 vbr mode param"
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]",
                 "[ChangePos]: means ChangePos, range:[50,100]");

    ADD_CMD("set_h264_fixqp", SetH264Fixqp, 2);
    ADD_CMD_HELP("set_h264_fixqp", "[IQP] [PQP]",
                 "set rcattr h264 fixqp mode param",
                 "[IQP]: means IQP, range:[12,48]",
                 "[PQP]: means PQP, range:[12,48]");

    ADD_CMD("set_h264_avbr", SetH264Avbr, 9);
    ADD_CMD_HELP("set_h264_avbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta] [ChangePos] [u32MinStillPercent] [u32MotionSensitivity]",
                 "set rcattr h264 avbr mode param",
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]",
                 "[ChangePos]: means ChangePos, range:[50,100]",
                 "[u32MinStillPercent]: means u32MinStillPercent, range:[5,100]",
                 "[u32MotionSensitivity]: means u32MotionSensitivity, range:[0,100]");

    // set h265 rcattr
    ADD_CMD("set_h265_cbr", SetH265Cbr, 6);
    ADD_CMD_HELP("set_h265_cbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta]",
                 "set rcattr h265 cbr mode param"
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]");

    ADD_CMD("set_h265_vbr", SetH265Vbr, 7);
    ADD_CMD_HELP("set_h265_vbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta] [ChangePos]",
                 "set rcattr h265 cbr mode param"
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]",
                 "[ChangePos]: means ChangePos, range:[50,100]");

    ADD_CMD("set_h265_fixqp", SetH265Fixqp, 2);
    ADD_CMD_HELP("set_h265_fixqp", "[IQP] [PQP]",
                 "set rcattr h265 fixqp mode param",
                 "[IQP]: means IQP, range:[12,48]",
                 "[PQP]: means PQP, range:[12,48]");

    ADD_CMD("set_h265_avbr", SetH265Avbr, 9);
    ADD_CMD_HELP("set_h265_avbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta] [ChangePos] [u32MinStillPercent] [u32MotionSensitivity]",
                 "set rcattr h265 avbr mode param",
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]",
                 "[ChangePos]: means ChangePos, range:[50,100]",
                 "[u32MinStillPercent]: means u32MinStillPercent, range:[5,100]",
                 "[u32MotionSensitivity]: means u32MotionSensitivity, range:[0,100]");

    // set jpd rcattr
    ADD_CMD("set_jpeg_cbr", SetJpegCbr, 3);
    ADD_CMD_HELP("set_jpeg_cbr", "[bitrate] [MaxQfactor] [MinQfactor]",
                "set rcattr jpeg cbr mode param"
                 "[bitrate]: range:[2000,102400000]",
                 "[MaxQfactor]: means MaxQfactor, range:[1,90]",
                 "[MinQfactor]: means MinQfactor, range:[1,90]" );

    ADD_CMD("set_jpeg_vbr", SetJpegVbr, 4);
    ADD_CMD_HELP("set_jpeg_vbr", "[bitrate] [MaxQfactor] [MinQfactor] [ChangePos]",
                "set rcattr jpeg vbr mode param"
                 "[bitrate]: range:[2000,102400000]",
                 "[MaxQfactor]: means MaxQfactor, range:[1,90]",
                 "[MinQfactor]: means MinQfactor, range:[1,90]",
                 "[ChangePos]: means ChangePos, range:[50,100]" );

    ADD_CMD("set_jpeg_fixqp", SetJpegFixqp, 1);
    ADD_CMD_HELP("set_jpeg_fixqp", "[Qfactor]",
                 "set rcattr jpeg fixqp mode param"
                 "[Qfactor]: means Qfactor, range:[20,90]");

    // set av1 rcattr
    ADD_CMD("set_av1_cbr", SetAv1Cbr, 6);
    ADD_CMD_HELP("set_av1_cbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta]",
                 "set rcattr av1 cbr mode param",
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]");

    ADD_CMD("set_av1_vbr", SetAv1Vbr, 7);
    ADD_CMD_HELP("set_av1_vbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta] [ChangePos]",
                 "set rcattr h265 vbr mode param"
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]",
                 "[ChangePos]: means ChangePos, range:[50,100]");

    ADD_CMD("set_av1_fixqp", SetAv1Fixqp, 2);
    ADD_CMD_HELP("set_av1_fixqp", "[IQP] [PQP]",
                 "set rcattr av1 fixqp mode param",
                 "[IQP]: means IQP, range:[12,48]",
                 "[PQP]: means PQP, range:[12,48]");

    ADD_CMD("set_av1_avbr", SetAv1Avbr, 9);
    ADD_CMD_HELP("set_av1_avbr", "[bitrate] [maxQp] [minQp] [maxIQp] [minIQp] [IPQPDelta] [ChangePos] [u32MinStillPercent] [u32MotionSensitivity]",
                 "set rcattr av1 avbr mode param",
                 "[bitrate]: range:[2000,102400000]",
                 "[maxQp]: means maxQp, range:[12,48]",
                 "[minQp]: means minQp, range:[12,48]",
                 "[maxIQp]: means maxIQp, range:[12,48]",
                 "[minIQp]: means minIQp, range:[12,48]",
                 "[IPQPDelta]: means IPQPDelta, range:[-12,12]",
                 "[ChangePos]: means ChangePos, range:[50,100]",
                 "[u32MinStillPercent]: means u32MinStillPercent, range:[5,100]",
                 "[u32MotionSensitivity]: means u32MotionSensitivity, range:[0,100]");

    ADD_CMD("set_av1_switch_frame", SetAv1SwitchFrame, 1);
    ADD_CMD_HELP("set_av1_switch_frame", "bEnableSwitchFrame",
                 "set av1 s-frame function ",
                 "[bEnableSwitchFrame]: 0:close av1 s-frame ,1:open av1 s-frame");

    ADD_CMD("set_stream_width_hight", SetStreamWidthHight, 2);
    ADD_CMD_HELP("set_stream_width_hight", "[Width] [Hight]",
                 "set stream width hight",
                 "[Width]: means stream width",
                 "[Hight]: means stream hight");
}