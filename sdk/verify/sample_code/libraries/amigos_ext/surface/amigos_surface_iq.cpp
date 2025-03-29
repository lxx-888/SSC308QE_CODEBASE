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

#include "amigos_surface_iq.h"

AmigosSurfaceIq::AmigosSurfaceIq(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}
AmigosSurfaceIq::~AmigosSurfaceIq() {}

void AmigosSurfaceIq::_LoadDb()
{
    char caliCfgIdx[32];
    stIqInfo.intIqServer   = this->pDb->GetMod<int>("DO_OPEN_IQ_SRV");
    stIqInfo.intCus3a = this->pDb->GetMod<int>("CUS3A");
    stIqInfo.intCus3aAe = this->pDb->GetMod<int>("CUS3A_AE");
    stIqInfo.intCus3aAwb = this->pDb->GetMod<int>("CUS3A_AWB");
    stIqInfo.intCus3aAf = this->pDb->GetMod<int>("CUS3A_AF");
    stIqInfo.Cus3aType = this->pDb->GetMod<std::string>("CUS3A_TYPE");
    stIqInfo.intCus3aBlack = this->pDb->GetMod<int>("CUS3A_BLOCK");
    stIqInfo.intUsrKey = this->pDb->GetMod<int>("IQ_USR_KEY");
    stIqInfo.intCaliCfgCnt  = this->pDb->GetMod<int>("CALI_CFG_CNT");
    for (int i = 0; i < stIqInfo.intCaliCfgCnt; i++)
    {
        struct IqInfo::CaliCfg caliCfg;
        snprintf(caliCfgIdx, 32, "CALI_CFG_%d", i);
        caliCfg.caliItem  = this->pDb->GetMod<std::string>(caliCfgIdx, "IQ_CALI_ITEM");
        caliCfg.caliFileName  = this->pDb->GetMod<std::string>(caliCfgIdx, "IQ_CALI_FILE");
        if (caliCfg.caliItem.size() && caliCfg.caliFileName.size())
        {
            stIqInfo.vectCaliCfg.push_back(caliCfg);
        }
    }
}

void AmigosSurfaceIq::_UnloadDb()
{
    stIqInfo.Clear();
}

void AmigosSurfaceIq::GetInfo(IqInfo &info) const
{
    info = stIqInfo;
}
void AmigosSurfaceIq::SetInfo(const IqInfo &info)
{
    stIqInfo = info;
}

