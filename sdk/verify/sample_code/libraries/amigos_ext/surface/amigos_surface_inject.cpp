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
#include "amigos_surface_inject.h"
AmigosSurfaceInject::AmigosSurfaceInject(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
    for (auto itMapInjectOut = this->mapModOutputInfo.begin(); itMapInjectOut != this->mapModOutputInfo.end();
         itMapInjectOut++)
    {
        mapInjectOutInfo[itMapInjectOut->first].uintBackGroudColor =
            this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "BG_COLOR");
        mapInjectOutInfo[itMapInjectOut->first].uintVideoWidth =
            this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "VID_W");
        mapInjectOutInfo[itMapInjectOut->first].uintVideoHeight =
            this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "VID_H");
        mapInjectOutInfo[itMapInjectOut->first].uintVideoFmt =
            this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "VID_FMT");
        mapInjectOutInfo[itMapInjectOut->first].bEnableOsd =
            this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "IS_ENABLE_OSD");
        if (mapInjectOutInfo[itMapInjectOut->first].bEnableOsd)
        {
            mapInjectOutInfo[itMapInjectOut->first].strInjectOsdSrcFile =
                this->pDb->GetOut<std::string>(itMapInjectOut->second.curLoopId, "OSD_SRC_FILE");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdDelay =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_SHOW_DELAY");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdFmt =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_FMT");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdColor =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_COLOR");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdWidth =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_WIDTH");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdHeight =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_HEIGHT");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdShowFunction =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_SHOW_FUNCTION");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdTargetPortId =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_ATTACH_PORT");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdTargetPortWid =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_ATTACH_PORT_WIDTH");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdTargetPortHei =
                this->pDb->GetOut<unsigned int>(itMapInjectOut->second.curLoopId, "OSD_ATTACH_PORT_HEIGHT");
        }
    }
}
AmigosSurfaceInject::~AmigosSurfaceInject() {}
