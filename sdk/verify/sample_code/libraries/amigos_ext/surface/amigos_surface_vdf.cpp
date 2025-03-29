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

#include "amigos_surface_vdf.h"
#include <sstream>
#include <iostream>
#include <string.h>

AmigosSurfaceVdf::AmigosSurfaceVdf(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, true, false)
{
}

AmigosSurfaceVdf::~AmigosSurfaceVdf()
{
}

void AmigosSurfaceVdf::_LoadDb()
{
    unsigned int   u32Num = 0;
    const char* pChModeType = "MODE_TYPE_PARAM";
    const char* pChVgModeType = "VG_ALGO_TYPE";
    stVdfInfo.strVdfMode    = this->pDb->GetMod<std::string>("MOD_TYPE");
    if (0 == stVdfInfo.strVdfMode.compare("md"))
    {
        stVdfInfo.strAlgMode = this->pDb->GetMod<std::string>(pChModeType, "ALG_MODE");
        stVdfInfo.strMdMbMode = this->pDb->GetMod<std::string>(pChModeType, "MB_MODE");
        stVdfInfo.strMdSadOutMode = this->pDb->GetMod<std::string>(pChModeType, "SAD_MODE");
        stVdfInfo.stMdAttr.u32Sensitivity = this->pDb->GetMod<int>(pChModeType, "ALG_SENS");
        stVdfInfo.stMdAttr.u32ObjNumMax = this->pDb->GetMod<int>(pChModeType, "OBJ_MAXNUM");
        stVdfInfo.stMdAttr.u32Thr = this->pDb->GetMod<int>(pChModeType, "VDF_ALGO_MODE", "MD_THR");
        stVdfInfo.stMdAttr.u32LearnRate = this->pDb->GetMod<int>(pChModeType, "VDF_ALGO_MODE", "LEARN_RATE");
        if (4 != this->pDb->GetMod<int>(pChModeType, "POINT_NUM"))
        {
            AMIGOS_ERR("md point num is only support 4 point, please check!\n");
            return;
        }
        for (u32Num = 0; u32Num  < 4; u32Num++)
        {
            std::stringstream ss;
            ss << "POINT_" << u32Num;
            stVdfInfo.stMdAttr.stPnt[u32Num].u32x  = pDb->GetMod<unsigned int>(pChModeType, ss.str().c_str(), "PX");
            stVdfInfo.stMdAttr.stPnt[u32Num].u32y  = pDb->GetMod<unsigned int>(pChModeType, ss.str().c_str(), "PY");
        }
        stVdfInfo.stMdAttr.u32PointNum = 4;
    }
    else if (0 == stVdfInfo.strVdfMode.compare("od"))
    {
        stVdfInfo.strMotionSensitivity = this->pDb->GetMod<std::string>(pChModeType, "MOT_SENS");
        stVdfInfo.strSensitivity = this->pDb->GetMod<std::string>(pChModeType, "ALG_SENS");
        stVdfInfo.strOdWindows = this->pDb->GetMod<std::string>(pChModeType, "OD_WINDOW");
        if (4 != this->pDb->GetMod<int>(pChModeType, "POINT_NUM"))
        {
            AMIGOS_ERR("od point num is only support 4 point, please check!\n");
            return;
        }
        for (u32Num = 0; u32Num  < 4; u32Num++)
        {
            std::stringstream ss;
            ss << "POINT_" << u32Num;
            stVdfInfo.stOdAttr.stPnt[u32Num].u32x  = pDb->GetMod<unsigned int>(pChModeType, ss.str().c_str(), "PX");
            stVdfInfo.stOdAttr.stPnt[u32Num].u32y  = pDb->GetMod<unsigned int>(pChModeType, ss.str().c_str(), "PY");
        }
        stVdfInfo.stOdAttr.u32PointNum = 4;
    }
    else if (0 == stVdfInfo.strVdfMode.compare("vg"))
    {
        stVdfInfo.strAlgMode = this->pDb->GetMod<std::string>(pChModeType, "ALG_MODE");
        stVdfInfo.strSensitivity = this->pDb->GetMod<std::string>(pChModeType, "ALG_SENS");
        if (0 == stVdfInfo.strAlgMode.compare("gate"))
        {
            stVdfInfo.stVgAttr.u32LineNum = this->pDb->GetMod<int>(pChModeType, pChVgModeType, "LINE_COUNT");
            stVdfInfo.stVgAttr.u32LineNum = stVdfInfo.stVgAttr.u32LineNum > 4 ? 4 : stVdfInfo.stVgAttr.u32LineNum;
            for (u32Num = 0; u32Num < stVdfInfo.stVgAttr.u32LineNum; u32Num++)
            {
                std::stringstream ss;
                ss << "LINE_" << u32Num;
                stVdfInfo.stVgAttr.stLine[u32Num].px.u32x  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PX_X");
                stVdfInfo.stVgAttr.stLine[u32Num].px.u32y  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PX_Y");
                stVdfInfo.stVgAttr.stLine[u32Num].py.u32x  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PY_X");
                stVdfInfo.stVgAttr.stLine[u32Num].py.u32y  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PY_Y");
                stVdfInfo.stVgAttr.stLine[u32Num].pdx.u32x  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PDX_X");
                stVdfInfo.stVgAttr.stLine[u32Num].pdx.u32y  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PDX_Y");
                stVdfInfo.stVgAttr.stLine[u32Num].pdy.u32x  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PDY_X");
                stVdfInfo.stVgAttr.stLine[u32Num].pdy.u32y  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PDY_Y");
            }
        }
        else if (0 == stVdfInfo.strAlgMode.compare("reg"))
        {
            stVdfInfo.strVgRegionDir = this->pDb->GetMod<std::string>(pChModeType, pChVgModeType, "REG_DIR");
            stVdfInfo.strVgSpecDirState = this->pDb->GetMod<std::string>(pChModeType, pChVgModeType, "SPEC_DIR");
            if (4 != this->pDb->GetMod<int>(pChModeType, pChVgModeType, "POINT_NUM"))
            {
                AMIGOS_ERR("vg reg point num is only support 4 point, please check!\n");
                return;
            }
            for (u32Num = 0; u32Num  < 4; u32Num++)
            {
                std::stringstream ss;
                ss << "POINT_" << u32Num;
                stVdfInfo.stVgAttr.stPt[u32Num].u32x  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PX");
                stVdfInfo.stVgAttr.stPt[u32Num].u32y  = pDb->GetMod<unsigned int>(pChModeType, pChVgModeType, ss.str().c_str(), "PY");
            }
            stVdfInfo.stVgAttr.u32LineNum = 4;
        }
    }
    else
    {
        AMIGOS_ERR("RC Mode error!\n");
        return;
    }
}

void AmigosSurfaceVdf::_UnloadDb()
{
    stVdfInfo.Clear();
}