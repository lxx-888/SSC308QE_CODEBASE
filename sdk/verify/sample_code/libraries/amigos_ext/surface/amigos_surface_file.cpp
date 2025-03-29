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


#include "amigos_surface_file.h"

AmigosSurfaceFile::AmigosSurfaceFile(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceFile::~AmigosSurfaceFile() {}

void AmigosSurfaceFile::_LoadDb()
{
    FileOutInfo stFileOutput;
    FileInInfo  stFileInInfo;

    for (auto itMapFileIn = this->mapModInputInfo.begin(); itMapFileIn != this->mapModInputInfo.end(); itMapFileIn++)
    {
        stFileInInfo.fileName      = this->pDb->GetIn<std::string>(itMapFileIn->second.curLoopId, "FILE_WRITE_PATH");
        stFileInInfo.frameCntLimit = this->pDb->GetIn<int>(itMapFileIn->second.curLoopId, "FRAME_CNT_LIMIT");
        stFileInInfo.bHead         = this->pDb->GetIn<int>(itMapFileIn->second.curLoopId, "HEAD");
        mapInputWrFile[itMapFileIn->first] = stFileInInfo;
    }
    for (auto itMapFileOut = this->mapModOutputInfo.begin(); itMapFileOut != this->mapModOutputInfo.end();
         itMapFileOut++)
    {
        stFileOutput.fileName = this->pDb->GetOut<std::string>(itMapFileOut->second.curLoopId, "FILE_READ_PATH");
        stFileOutput.frameCntLimit = this->pDb->GetOut<int>(itMapFileOut->second.curLoopId, "FRAME_CNT_LIMIT");

        stFileOutput.strOutType = this->pDb->GetOut<std::string>(itMapFileOut->second.curLoopId, "OUT_TYPE");
        if (stFileOutput.strOutType == "raw" || stFileOutput.strOutType == "video")
        {
            if (stFileOutput.strOutType == "raw")
            {
                stFileOutput.intPlaneNum = this->pDb->GetOut<int>(itMapFileOut->second.curLoopId, "VID_PLANE_NUM");
            }
            stFileOutput.intFileOutWidth  = this->pDb->GetOut<int>(itMapFileOut->second.curLoopId, "VID_W");
            stFileOutput.intFileOutHeight = this->pDb->GetOut<int>(itMapFileOut->second.curLoopId, "VID_H");
        }
        stFileOutput.strOutFmt  = this->pDb->GetOut<std::string>(itMapFileOut->second.curLoopId, "OUT_FMT");
        if("bayer" == stFileOutput.strOutFmt)
        {
            stFileOutput.strBayerId  =
                this->pDb->GetOut<std::string>(itMapFileOut->second.curLoopId, "BAYER_PARAM" ,"BAYER_ID");
            stFileOutput.strPrecision  =
                this->pDb->GetOut<std::string>(itMapFileOut->second.curLoopId, "BAYER_PARAM" ,"PRECISION");
        }
        mapOutputRdFile[itMapFileOut->first] = stFileOutput;
    }
}

void AmigosSurfaceFile::_UnloadDb()
{
    mapInputWrFile.clear();
    mapOutputRdFile.clear();
}

bool AmigosSurfaceFile::GetFileInInfo(unsigned int portId, FileInInfo &stIn) const
{
    auto iter = this->mapInputWrFile.find(portId);
    if (iter == this->mapInputWrFile.end())
    {
        return false;
    }
    stIn = iter->second;
    return true;
}
void AmigosSurfaceFile::SetFileInInfo(unsigned int portId, const FileInInfo &stIn)
{
    auto iter = this->mapInputWrFile.find(portId);
    if (iter == this->mapInputWrFile.end())
    {
        return ;
    }
    iter->second = stIn;
}
bool AmigosSurfaceFile::GetFileOutInfo(unsigned int portId, FileOutInfo &stOut) const
{
    auto iter = this->mapOutputRdFile.find(portId);
    if (iter == this->mapOutputRdFile.end())
    {
        return false;
    }
    stOut = iter->second;
    return true;
}
void AmigosSurfaceFile::SetFileOutInfo(unsigned int portId, const FileOutInfo &stOut)
{
    auto iter = this->mapOutputRdFile.find(portId);
    if (iter == this->mapOutputRdFile.end())
    {
        return ;
    }
    iter->second = stOut;
}
