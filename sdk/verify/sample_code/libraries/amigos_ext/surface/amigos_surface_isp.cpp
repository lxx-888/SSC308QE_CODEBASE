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
#include "amigos_surface_isp.h"

AmigosSurfaceIsp::~AmigosSurfaceIsp() {}

AmigosSurfaceIsp::AmigosSurfaceIsp(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, true) {}

void AmigosSurfaceIsp::_LoadDb()
{
    char       strTableName[30];
    IspInInfo  stIspInInfo;
    IspOutInfo stIspOutInfo;

    stIspInfo.uintHdrType    = this->pDb->GetMod<unsigned int>("HDR_TYPE");
    stIspInfo.uintHdrFusionType    = this->pDb->GetMod<unsigned int>("HDR_FUSION_TYPE");
    stIspInfo.uintHdrExposureMask    = this->pDb->GetMod<unsigned int>("HDR_EXPOSURE_MASK");
    stIspInfo.uintRotation   = this->pDb->GetMod<unsigned int>("ROT");
    stIspInfo.uint3dNrLevel  = this->pDb->GetMod<unsigned int>("3DNR_LV");
    stIspInfo.uintSensorId   = this->pDb->GetMod<unsigned int>("SNR_MASK");
    stIspInfo.uintSnrMaskNum = this->pDb->GetMod<unsigned int>("SNR_MASK_NUM");
    stIspInfo.uintSync3AType = this->pDb->GetMod<unsigned int>("SYNC_3A_TYPE");
    stIspInfo.uintStitchMask = this->pDb->GetMod<unsigned int>("STITCH_MASK");
    stIspInfo.uintIsMirror   = this->pDb->GetMod<unsigned int>("IS_MIRROR");
    stIspInfo.uintIsFlip     = this->pDb->GetMod<unsigned int>("IS_FLIP");
    stIspInfo.uintZoomEn     = this->pDb->GetMod<unsigned int>("ZOOM_EN");
    stIspInfo.uintAibnrEn    = this->pDb->GetMod<unsigned int>("AIBNR_EN");
    stIspInfo.uintCustIqEn   = this->pDb->GetMod<unsigned int>("CUST_IQ_EN");
    stIspInfo.apiFileName    = this->pDb->GetMod<std::string>("IQ_API_FILE");
    stIspInfo.uintSubChnIqEn   = this->pDb->GetMod<unsigned int>("SUB_CHN_IQ_EN");
    if(stIspInfo.uintAibnrEn)
    {
        stIspInfo.aiBnrParam.strBnrSrcType    = this->pDb->GetMod<std::string>("BNR_PARAM", "BNR_SRC_TYPE");
        stIspInfo.aiBnrParam.bnrModelFileName = this->pDb->GetMod<std::string>("BNR_PARAM", "BNR_MODEL_FILE");
    }
    stIspInfo.uintIspLdcEn   = this->pDb->GetMod<unsigned int>("ISP_LDC_EN");
    stIspInfo.uintMutichnEn      = this->pDb->GetMod<unsigned int>("MUTI_CHN_EN");
    stIspInfo.uintIspOverlapEn      = this->pDb->GetMod<unsigned int>("OVER_LAP_EN");
    if(stIspInfo.uintZoomEn)
    {
        stIspInfo.ZoomParam.FromEntryIndex = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", "FROM_ENTRY_INDEX");
        stIspInfo.ZoomParam.ToEntryIndex   = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", "TO_ENTRY_INDEX");
        stIspInfo.ZoomParam.TableNum       = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", "ZOOM_TABLE_NUM");
        if(stIspInfo.ZoomParam.TableNum == (unsigned int)-1 || stIspInfo.ZoomParam.TableNum > ZOOM_TABLE_MAX_NUM)  //ZOOM_MAX_TABLE_NUM
        {
            AMIGOS_ERR("ZoomTableNum error!\n");
            return;
        }
        for(unsigned int i = 0;i<stIspInfo.ZoomParam.TableNum;i++)
        {
            snprintf(strTableName, 30, "TABLE_%d", i);
            stIspInfo.ZoomParam.Table[i].SnrId  = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", strTableName,"SNR_ID");
            stIspInfo.ZoomParam.Table[i].TableX = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", strTableName,"TABLE_X");
            stIspInfo.ZoomParam.Table[i].TableY = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", strTableName,"TABLE_Y");
            stIspInfo.ZoomParam.Table[i].TableW = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", strTableName,"TABLE_W");
            stIspInfo.ZoomParam.Table[i].TableH = this->pDb->GetMod<unsigned int>("ZOOM_PARAM", strTableName,"TABLE_H");
        }
    }
    if(stIspInfo.uintCustIqEn)
    {
        stIspInfo.CustIqParam.Revision          = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "REVISION");
        stIspInfo.CustIqParam.SnrEarlyFps       = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "FPS");
        stIspInfo.CustIqParam.SnrEarlyFlicker   = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "FLICKER");
        stIspInfo.CustIqParam.SnrEarlyShutter   = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "SHUTTER");
        stIspInfo.CustIqParam.SnrEarlyGainX1024 = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "GAIN_X1024");
        stIspInfo.CustIqParam.SnrEarlyDGain     = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "DGAIN");
        stIspInfo.CustIqParam.SnrEarlyAwbRGain  = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "AWB_RGAIN");
        stIspInfo.CustIqParam.SnrEarlyAwbGGain  = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "AWB_GGAIN");
        stIspInfo.CustIqParam.SnrEarlyAwbBGain  = this->pDb->GetMod<unsigned int>("CUST_IQ_PARAM", "AWB_BGAIN");
    }
    if(stIspInfo.uintSubChnIqEn)
    {
        stIspInfo.SubChnIqParam.dev         = this->pDb->GetMod<unsigned int>("SUB_CHN_IQ_PARAM", "DEV");
        stIspInfo.SubChnIqParam.chn         = this->pDb->GetMod<unsigned int>("SUB_CHN_IQ_PARAM", "CHN");
        stIspInfo.SubChnIqParam.apiFileName = this->pDb->GetMod<std::string>("SUB_CHN_IQ_PARAM", "API_FILE");
    }
    if(stIspInfo.uintIspLdcEn)
    {
        stIspInfo.IspLdcParam.CenterX     = this->pDb->GetMod<unsigned int>("ISP_LDC_PARAM", "CENTER_X");
        stIspInfo.IspLdcParam.CenterY     = this->pDb->GetMod<unsigned int>("ISP_LDC_PARAM", "CENTER_Y");
        stIspInfo.IspLdcParam.Alpha       = this->pDb->GetMod<unsigned int>("ISP_LDC_PARAM", "ALPHA");
        stIspInfo.IspLdcParam.Beta        = this->pDb->GetMod<unsigned int>("ISP_LDC_PARAM", "BETA");
        stIspInfo.IspLdcParam.CropL       = this->pDb->GetMod<unsigned int>("ISP_LDC_PARAM", "CROP_L");
        stIspInfo.IspLdcParam.CropR       = this->pDb->GetMod<unsigned int>("ISP_LDC_PARAM", "CROP_R");
    }
    if(stIspInfo.uintIspOverlapEn)
    {
        stIspInfo.OverLapParam.strOverlap = this->pDb->GetMod<std::string>("OVER_LAP_PARAM", "OVER_LAP_TYPE");
    }
    for (auto itMapIspIn = this->mapModInputInfo.begin(); itMapIspIn != this->mapModInputInfo.end(); itMapIspIn++)
    {
        stIspInInfo.uintIspInCropX  = this->pDb->GetIn<unsigned int>(itMapIspIn->second.curLoopId, "VID_CROP_X");
        stIspInInfo.uintIspInCropY  = this->pDb->GetIn<unsigned int>(itMapIspIn->second.curLoopId, "VID_CROP_Y");
        stIspInInfo.uintIspInWidth  = this->pDb->GetIn<unsigned int>(itMapIspIn->second.curLoopId, "VID_CROP_W");
        stIspInInfo.uintIspInHeight = this->pDb->GetIn<unsigned int>(itMapIspIn->second.curLoopId, "VID_CROP_H");
        mapIspIn[itMapIspIn->first] = stIspInInfo;
    }
    for (auto itMapIspOut = this->mapModOutputInfo.begin(); itMapIspOut != this->mapModOutputInfo.end(); itMapIspOut++)
    {
        stIspOutInfo.uintIspOutCropX  = this->pDb->GetOut<unsigned int>(itMapIspOut->second.curLoopId, "VID_CROP_X");
        stIspOutInfo.uintIspOutCropY  = this->pDb->GetOut<unsigned int>(itMapIspOut->second.curLoopId, "VID_CROP_Y");
        stIspOutInfo.uintIspOutCropW  = this->pDb->GetOut<unsigned int>(itMapIspOut->second.curLoopId, "VID_CROP_W");
        stIspOutInfo.uintIspOutCropH  = this->pDb->GetOut<unsigned int>(itMapIspOut->second.curLoopId, "VID_CROP_H");
        stIspOutInfo.uintCompressMode = this->pDb->GetOut<unsigned int>(itMapIspOut->second.curLoopId, "COMPRESS_MODE");
        stIspOutInfo.uintBufLayout    = this->pDb->GetOut<unsigned int>(itMapIspOut->second.curLoopId, "BUF_LAYOUT");
        stIspOutInfo.strOutType       = this->pDb->GetOut<std::string>(itMapIspOut->second.curLoopId, "OUT_TYPE");
        stIspOutInfo.strOutFmt        = this->pDb->GetOut<std::string>(itMapIspOut->second.curLoopId, "OUT_FMT");
        mapIspOut[itMapIspOut->first] = stIspOutInfo;
    }
}

void AmigosSurfaceIsp::_UnloadDb()
{
    stIspInfo.Clear();
}

void AmigosSurfaceIsp::GetInfo(IspInfo &info) const
{
    info = this->stIspInfo;
}

void AmigosSurfaceIsp::SetInfo(const IspInfo &info)
{
    this->stIspInfo = info;
}

void AmigosSurfaceIsp::GetInInfo(unsigned int portId, IspInInfo &stIn) const
{
    auto it = this->mapIspIn.find(portId);
    if (it == this->mapIspIn.end())
    {
        return;
    }
    stIn = it->second;
}

void AmigosSurfaceIsp::SetInInfo(unsigned int portId, const IspInInfo &stIn)
{
    auto it = this->mapIspIn.find(portId);
    if (it == this->mapIspIn.end())
    {
        return;
    }
    it->second = stIn;
}

void AmigosSurfaceIsp::GetOutInfo(unsigned int portId, IspOutInfo &stOut) const
{
    auto it = this->mapIspOut.find(portId);
    if (it == this->mapIspOut.end())
    {
        return;
    }
    stOut = it->second;
}

void AmigosSurfaceIsp::SetOutInfo(unsigned int portId, const IspOutInfo &stOut)
{
    auto it = this->mapIspOut.find(portId);
    if (it == this->mapIspOut.end())
    {
        return;
    }
    it->second = stOut;
}

