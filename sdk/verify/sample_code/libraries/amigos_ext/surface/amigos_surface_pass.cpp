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
#include "amigos_surface_pass.h"

AmigosSurfacePass::AmigosSurfacePass(const std::string &strInSection)
    : AmigosSurfaceBase(strInSection, false, false) {}

AmigosSurfacePass::~AmigosSurfacePass() {}

void AmigosSurfacePass::_LoadDb()
{
    for (auto itMapPassIn = this->mapModInputInfo.begin(); itMapPassIn != this->mapModInputInfo.end(); itMapPassIn++)
    {
        std::string sec = pDb->GetIn<std::string>(itMapPassIn->second.curLoopId, "TAIL");
        if (sec.empty())
        {
            AMILOG_ERR << "'TAIL' not config." << COLOR_ENDL;
            continue;
        }
        mapPassInInfo[itMapPassIn->first].secName   = sec;
        mapPassInInfo[itMapPassIn->first].secPortId = pDb->GetIn<unsigned int>(itMapPassIn->second.curLoopId, "TAIL_IN_ID");
    }
    for (auto itMapPassOut = this->mapModOutputInfo.begin(); itMapPassOut != this->mapModOutputInfo.end(); itMapPassOut++)
    {
        std::string sec = pDb->GetOut<std::string>(itMapPassOut->second.curLoopId, "HEAD");
        if (sec.empty())
        {
            AMILOG_ERR << "'HEAD' not config." << COLOR_ENDL;
            continue;
        }
        mapPassOutInfo[itMapPassOut->first].secName   = sec;
        mapPassOutInfo[itMapPassOut->first].secPortId = pDb->GetOut<unsigned int>(itMapPassOut->second.curLoopId, "HEAD_OUT_ID");
        std::set<std::string> setSecName;
        unsigned int subRootCnt = pDb->GetOut<unsigned int>(itMapPassOut->second.curLoopId, "ROOT_CNT");
        for (unsigned int i = 0; i < subRootCnt; i++)
        {
            std::stringstream ss;
            Members rootMember;
            ss << "ROOT_" << i;
            std::string strSubRootSec = pDb->GetOut<std::string>(itMapPassOut->second.curLoopId, ss.str().c_str(), "SEC_NAME");
            setSecName.clear();
            _TraverseMembers(strSubRootSec, rootMember, setSecName);
            if (rootMember.memberSection.size())
            {
                mapRootMembers[itMapPassOut->first].push_back(rootMember);
                if (rootMember.memberSection == sec)
                {
                    mapPassOutInfo[itMapPassOut->first].secRootIdx = i;
                }
            }
        }
    }
}

void AmigosSurfacePass::_UnloadDb()
{
    mapPassInInfo.clear();
    mapPassOutInfo.clear();
    mapRootMembers.clear();
}

void AmigosSurfacePass::_SignatureMembers(const std::string &sec, struct AmigosSurfacePass::Members &passMem)
{
    AmigosDatabase *pSec = this->GetDbIns()->GetFactory()->Create(sec);
    assert(pSec);
    std::string modStr = pSec->GetMod<std::string>("MOD");
    if (modStr.empty())
    {
        AMILOG_ERR << "SEC: " << sec << " Not found 'MOD' in json file." << COLOR_ENDL;
        this->GetDbIns()->GetFactory()->Destroy(pSec);
        return;
    }
    passMem.bOnlySignature = true;
    passMem.memberSection  = sec;
    passMem.memberModType  = modStr;
    this->GetDbIns()->GetFactory()->Destroy(pSec);
}

void AmigosSurfacePass::_TraverseMembers(const std::string &sec, struct AmigosSurfacePass::Members &passMem,
                                         std::set<std::string> &setSecName)
{
    AmigosDatabase *pSec = this->GetDbIns()->GetFactory()->Create(sec);
    assert(pSec);
    std::string modStr = pSec->GetMod<std::string>("MOD");
    if (modStr.empty())
    {
        AMILOG_ERR << "SEC: " << sec << " Not found 'MOD' in json file." << COLOR_ENDL;
        this->GetDbIns()->GetFactory()->Destroy(pSec);
        return;
    }
    passMem.memberSection = sec;
    passMem.memberModType = modStr;
    for (unsigned int uintLoopId = pSec->GetInLoopId(); uintLoopId != LOOP_ID_END;
         uintLoopId              = pSec->GetInLoopId(uintLoopId))
    {
        std::string strPrevRaw;
        strPrevRaw = pSec->GetIn<std::string>(uintLoopId, "PREV");
        if (strPrevRaw.size())
        {
            size_t findOffset = strPrevRaw.find_last_of(':');
            if (findOffset)
            {
                strPrevRaw.erase(findOffset, strPrevRaw.size() - findOffset);
                struct AmigosSurfacePass::Members tmpMember(uintLoopId);
                if (setSecName.find(strPrevRaw) == setSecName.end())
                {
                    // Go traverse in the first previous module.
                    _TraverseMembers(strPrevRaw, tmpMember, setSecName);
                    setSecName.insert(strPrevRaw);
                }
                else
                {
                    _SignatureMembers(strPrevRaw, tmpMember);
                }
                passMem.prevMembers.push_back(tmpMember);
            }
        }
    }
    this->GetDbIns()->GetFactory()->Destroy(pSec);
}
