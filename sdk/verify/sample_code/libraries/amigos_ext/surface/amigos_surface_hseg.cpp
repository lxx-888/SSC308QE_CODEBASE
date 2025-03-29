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

#include "amigos_surface_hseg.h"
#include <sstream>
#include <iostream>
#include <string.h>

AmigosSurfaceHseg::AmigosSurfaceHseg(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false)
{
}

AmigosSurfaceHseg::~AmigosSurfaceHseg()
{
}

void AmigosSurfaceHseg::_LoadDb()
{
    stHsegInfo.strHsegMode  = this->pDb->GetMod<std::string> ("HSEG_MODE");
    stHsegInfo.strIpuPath  = this->pDb->GetMod<std::string> ("IPU_PATH");
    stHsegInfo.strModelPath  = this->pDb->GetMod<std::string> ("MODEL_PATH");
    stHsegInfo.strMaskOp  = this->pDb->GetMod<std::string> ("MASK_OP");
    stHsegInfo.uiMaskThr = this->pDb->GetMod<unsigned int> ("MASK_THR");
    stHsegInfo.uiBlurLv = this->pDb->GetMod<unsigned int> ("BLUR_LV");
    stHsegInfo.uiScalingStage = this->pDb->GetMod<unsigned int> ("SCAL_STAGE");
}

void AmigosSurfaceHseg::_UnloadDb()
{
    stHsegInfo.Clear();
}
