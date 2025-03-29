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
#include "amigos_surface_jpd.h"

AmigosSurfaceJpd::AmigosSurfaceJpd(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, true) {}

AmigosSurfaceJpd::~AmigosSurfaceJpd() {}

void AmigosSurfaceJpd::_LoadDb()
{
    stJpdInfo.u32StreamBufSize = this->pDb->GetMod<unsigned int>("STREAM_BUF_SIZE");
    stJpdInfo.u32MaxPicWidth   = this->pDb->GetMod<unsigned int>("MAX_VID_WIDTH");
    stJpdInfo.u32MaxPicHeight  = this->pDb->GetMod<unsigned int>("MAX_VID_HEIGHT");
    stJpdInfo.strOutFmt        = this->pDb->GetMod<std::string>("OUT_FMT");
}

void AmigosSurfaceJpd::_UnloadDb()
{
    stJpdInfo.Clear();
}
