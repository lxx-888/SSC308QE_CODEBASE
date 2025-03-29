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

#include "amigos_surface_ive.h"

AmigosSurfaceIve::AmigosSurfaceIve(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfaceIve::~AmigosSurfaceIve() {}

void AmigosSurfaceIve::_LoadDb()
{
    const static char* pModParam = "IVE_MODE_PARAM";
    stIveInfo.strIveType  = this->pDb->GetMod<std::string>("ALGO_TYPE");
    if ("bgblur" == stIveInfo.strIveType)
    {
        stIveInfo.strIveMode  = this->pDb->GetMod<std::string>(pModParam, "IVE_MODE");
        stIveInfo.strYMaskPath  = this->pDb->GetMod<std::string>(pModParam, "YMASK_PATH");
        stIveInfo.strUvMaskPath  = this->pDb->GetMod<std::string>(pModParam, "UVMASK_PATH");
        stIveInfo.strMaskOp  = this->pDb->GetMod<std::string> (pModParam, "MASK_OP");
        stIveInfo.uiMaskThr = this->pDb->GetMod<unsigned int>(pModParam, "MASK_THR");
        stIveInfo.uiBlurLv = this->pDb->GetMod<unsigned int>(pModParam, "BLUR_LV");
        stIveInfo.uiScalingStage = this->pDb->GetMod<unsigned int>(pModParam, "SCAL_STAGE");
        stIveInfo.uiSaturationLv = this->pDb->GetMod<unsigned int>(pModParam, "SATURATION_LV");
        stIveInfo.uiMosaicSize = this->pDb->GetMod<unsigned int>(pModParam, "MOSAIC_SIZE");
        AMIGOS_INFO("_LoadDb(),  mode: %s, thr: %d, lv: %d, scal: %d, SaturationLv: %d, MosaicSize: %d, Ymaskpath: %s, uvmask path: %s\n",
                    stIveInfo.strIveMode.c_str(), stIveInfo.uiMaskThr, stIveInfo.uiBlurLv,
                    stIveInfo.uiScalingStage, stIveInfo.uiSaturationLv, stIveInfo.uiMosaicSize, stIveInfo.strYMaskPath.c_str(), stIveInfo.strUvMaskPath.c_str());
        return;
    }
    AMIGOS_ERR("_LoadDb() ive type %s\n", stIveInfo.strIveType.c_str());
}

void AmigosSurfaceIve::_UnloadDb()
{
    stIveInfo.Clear();
}