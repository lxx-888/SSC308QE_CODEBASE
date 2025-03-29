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
#include "amigos_surface_vif.h"

AmigosSurfaceVif::AmigosSurfaceVif(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, true) {}

AmigosSurfaceVif::~AmigosSurfaceVif() {}

void AmigosSurfaceVif::_LoadDb()
{
    stVifInfo.intHdrType     = this->pDb->GetMod<int>("HDR_TYPE");
    stVifInfo.intHdrExposureMask    = this->pDb->GetMod<int>("HDR_EXPOSURE_MASK");
    stVifInfo.intSensorId    = this->pDb->GetMod<int>("SNR_ID");
    stVifInfo.intWorkMode    = this->pDb->GetMod<int>("WORK_MOD");
    stVifInfo.uintStitchMask = this->pDb->GetMod<unsigned int>("GROUP_STITCH_MASK");

    for (auto itMapOut = this->mapModOutputInfo.begin(); itMapOut != this->mapModOutputInfo.end(); itMapOut++)
    {
        mapVifOutInfo[itMapOut->first].intIsUseSnrFmt =
            this->pDb->GetOut<int>(itMapOut->second.curLoopId, "USE_SNR_FMT");
        mapVifOutInfo[itMapOut->first].intCompressMode =
            this->pDb->GetOut<int>(itMapOut->second.curLoopId, "COMPRESS_MODE");
        mapVifOutInfo[itMapOut->first].intCropX  =
            this->pDb->GetOut<unsigned int>(itMapOut->second.curLoopId, "VID_CROP_X");
        mapVifOutInfo[itMapOut->first].intCropY  =
            this->pDb->GetOut<unsigned int>(itMapOut->second.curLoopId, "VID_CROP_Y");
        mapVifOutInfo[itMapOut->first].intCropW  =
            this->pDb->GetOut<unsigned int>(itMapOut->second.curLoopId, "VID_CROP_W");
        mapVifOutInfo[itMapOut->first].intCropH  =
            this->pDb->GetOut<unsigned int>(itMapOut->second.curLoopId, "VID_CROP_H");
        if (!mapVifOutInfo[itMapOut->first].intIsUseSnrFmt)
        {
            mapVifOutInfo[itMapOut->first].intWidth =
                this->pDb->GetOut<int>(itMapOut->second.curLoopId, "VID_W");
            mapVifOutInfo[itMapOut->first].intHeight =
                this->pDb->GetOut<int>(itMapOut->second.curLoopId, "VID_H");
            mapVifOutInfo[itMapOut->first].strOutType =
                this->pDb->GetOut<std::string>(itMapOut->second.curLoopId, "OUT_TYPE");
            mapVifOutInfo[itMapOut->first].strOutFmt  =
                this->pDb->GetOut<std::string>(itMapOut->second.curLoopId, "OUT_FMT");
            if("bayer" == mapVifOutInfo[itMapOut->first].strOutFmt)
            {
                mapVifOutInfo[itMapOut->first].strBayerId  =
                    this->pDb->GetOut<std::string>(itMapOut->second.curLoopId, "BAYER_PARAM" ,"BAYER_ID");
                mapVifOutInfo[itMapOut->first].strPrecision  =
                    this->pDb->GetOut<std::string>(itMapOut->second.curLoopId, "BAYER_PARAM" ,"PRECISION");
            }
        }
    }
}

void AmigosSurfaceVif::_UnloadDb()
{
    mapVifOutInfo.clear();
    stVifInfo.Clear();
}

void AmigosSurfaceVif::GetInfo(VifInfo &info, std::map<unsigned int, VifOutInfo> &out) const
{
    info = stVifInfo;
    out  = mapVifOutInfo;
}

void AmigosSurfaceVif::SetInfo(const VifInfo &info, const std::map<unsigned int, VifOutInfo> &out)
{
    stVifInfo     = info;
    mapVifOutInfo = out;
}
