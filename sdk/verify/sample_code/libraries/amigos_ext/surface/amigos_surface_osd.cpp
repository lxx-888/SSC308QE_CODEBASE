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

#include "amigos_surface_osd.h"

AmigosSurfaceOsd::~AmigosSurfaceOsd() {}

AmigosSurfaceOsd::AmigosSurfaceOsd(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

void AmigosSurfaceOsd::_LoadDb()
{
    unsigned int uintOsdCnt = 0;

    uintOsdCnt = this->pDb->GetMod<unsigned int>("OSD_CNT");
    for (unsigned int i = 0; i < uintOsdCnt; i++)
    {
        OsdInfo           stInfo;
        std::stringstream ss;
        ss << "OSD" << i;
        stInfo.osdFile   = this->pDb->GetMod<std::string>(ss.str().c_str(), "FILE_PATH");
        stInfo.osdFormat = this->pDb->GetMod<std::string>(ss.str().c_str(), "COLOR_FORMAT");
        stInfo.alphaMode = this->pDb->GetMod<std::string>(ss.str().c_str(), "ALPHA_MODE");
        stInfo.ucharConstantAlpha = 0xFF;
        stInfo.ucharAlpha0 = 0xFF;
        stInfo.ucharAlpha0 = 0xFF;
        if (stInfo.alphaMode == "constant")
        {
            stInfo.ucharConstantAlpha = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "CONSTANT_ALPHA");
        }
        else if (stInfo.osdFormat == "argb1555")
        {
            stInfo.ucharAlpha0 = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "ALPHA0");
            stInfo.ucharAlpha1 = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "ALPHA1");
        }
        stInfo.uintLayer     = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "LAYER");
        stInfo.uintOsdWidth  = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "WIDTH");
        stInfo.uintOsdHeight = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "HEIGHT");
        stInfo.uintOsdPosX   = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "POS_X");
        stInfo.uintOsdPosY   = this->pDb->GetMod<unsigned int>(ss.str().c_str(), "POS_Y");
        stInfo.pExtData      = NULL;
        if (!listOsdInfo.size())
        {
            listOsdInfo.push_back(stInfo);
            return;
        }
        for (auto it = listOsdInfo.begin(); it != listOsdInfo.end(); ++it)
        {
            if (it->uintLayer > stInfo.uintLayer)
            {
                listOsdInfo.insert(it, stInfo);
                break;
            }
        }
    }
}

void AmigosSurfaceOsd::_UnloadDb()
{
    listOsdInfo.clear();
}
