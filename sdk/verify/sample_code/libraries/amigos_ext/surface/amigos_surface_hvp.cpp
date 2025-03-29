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

#include "amigos_surface_hvp.h"

AmigosSurfaceHvp::AmigosSurfaceHvp(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, true) {}

AmigosSurfaceHvp::~AmigosSurfaceHvp() {}

void AmigosSurfaceHvp::_LoadDb()
{
    stHvpConf.FrcMode = pDb->GetMod<unsigned int>("FRC_MODE");
    if(stHvpConf.FrcMode == 0)
    {
        //FBL mode
    }
    else if(stHvpConf.FrcMode == 1 || stHvpConf.FrcMode == 2)
    {
        stHvpConf.DMAColor = pDb->GetMod<unsigned int>("DMA_COLOR");
        stHvpConf.BufMaxCnt = pDb->GetMod<unsigned int>("BUF_CNT");
        stHvpConf.BufMaxHeight = pDb->GetMod<unsigned int>("MAX_BUF_H");
        stHvpConf.BufMaxWidth = pDb->GetMod<unsigned int>("MAX_BUF_W");
        stHvpConf.CompressMode = pDb->GetMod<unsigned int>("COMPRESS_MODE");
        stHvpConf.FieldType = pDb->GetMod<unsigned int>("FIELD_TYPE");
    }
    else
    {
        AMIGOS_ERR("frc mode err\n");
    }
    stHvpConf.ColorDepth = pDb->GetMod<unsigned int>("COLOR_DEPTH");
    stHvpConf.InputColor = pDb->GetMod<unsigned int>("IN_COLOR");

    for (auto itMapHvpOut = mapModOutputInfo.begin(); itMapHvpOut != mapModOutputInfo.end(); itMapHvpOut++)
    {
        stHvpConf.OutWidth = pDb->GetOut<unsigned int>(itMapHvpOut->second.curLoopId, "VID_W");
        stHvpConf.OutHeight = pDb->GetOut<unsigned int>(itMapHvpOut->second.curLoopId, "VID_H");
        stHvpConf.DestColor = pDb->GetOut<unsigned int>(itMapHvpOut->second.curLoopId, "DST_COLOR");
        stHvpConf.Fpsx100 = pDb->GetOut<unsigned int>(itMapHvpOut->second.curLoopId, "FPS");
    }
}

void AmigosSurfaceHvp::_UnloadDb()
{
    stHvpConf.Clear();
}
