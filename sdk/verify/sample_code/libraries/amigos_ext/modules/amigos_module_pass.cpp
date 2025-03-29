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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <map>
#include "amigos_module_init.h"
#include "ss_log.h"
#include "amigos_module_pass.h"

enum
{
    E_DELAY_PASS_CREATE = 0x1,
    E_DELAY_PASS_INIT = 0x2,
    E_DELAY_PASS_BIND = 0x4,
    E_DELAY_PASS_START = 0x8,
};

AmigosModulePass::AmigosModulePass(const std::string &strInSection)
    : AmigosSurfacePass(strInSection), AmigosModuleBase(this)
{
}
AmigosModulePass::~AmigosModulePass()
{
}
unsigned int AmigosModulePass::GetModId() const
{
    return uintExtModId;
}
unsigned int AmigosModulePass::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_NONE;
}
unsigned int AmigosModulePass::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_NONE;
}
void AmigosModulePass::_Init()
{
}
void AmigosModulePass::_Deinit()
{
    for (auto &it : mapPassMemObj)
    {
        if (!it.second.selfObj)
        {
            AMILOG_ERR << "Null pointer" << COLOR_ENDL;
            continue;
        }
        const AmigosSurfaceBase::ModInfo &stInfo = it.second.selfObj->GetSurface()->GetModInfo();
        AMILOG_ERR << "[DELAY-PASS] : current section: " << stInfo.sectionName << " resource isn't clear." << COLOR_ENDL;
    }
}
void AmigosModulePass::_Start()
{
}
void AmigosModulePass::_Stop()
{
}
bool AmigosModulePass::_IsDelayConnected(unsigned int uintInPort)
{
    // We redefine this virtual function to avoid some case that PASS has no output info after 'unlink' is
    // called in '_CreateDelayPass'.
    return true;
}
int AmigosModulePass::_ConnectedMembersInEdge(const struct Members &members)
{
    for (auto &iter : this->mapPassInInfo)
    {
        if (iter.second.secName == members.memberSection)
        {
            auto itIn = this->mapPortIn.find(iter.first);
            if (itIn != this->mapPortIn.end())
            {
                itIn->second.access();
            }
        }
    }
    for (auto &it : members.prevMembers)
    {
        this->_ConnectedMembersInEdge(it);
    }
    return 0;
}
int AmigosModulePass::_DisonnectedMembersInEdge(const struct Members &members)
{
    for (auto &iter : this->mapPassInInfo)
    {
        if (iter.second.secName == members.memberSection)
        {
            auto itIn = this->mapPortIn.find(iter.first);
            if (itIn != this->mapPortIn.end())
            {
                itIn->second.leave();
            }
        }
    }
    for (auto &it : members.prevMembers)
    {
        this->_DisonnectedMembersInEdge(it);
    }
    return 0;
}
int AmigosModulePass::_ConnectedTransfer(unsigned int outPortId)
{
    if (this->mapRootObj.find(outPortId) == this->mapRootObj.end())
    {
        // Internal pass haven't beed created, no need to transfer message to previous module.
        return 0;
    }
    auto it = this->mapPassOutInfo.find(outPortId);
    auto iter = this->mapRootMembers.find(outPortId);
    if (it == this->mapPassOutInfo.end() || iter == this->mapRootMembers.end()
        || iter->second.size() <= it->second.secRootIdx)
    {
        AMILOG_ERR << "Can not find the members in this out: " << outPortId << COLOR_ENDL;
        return -1;
    }
    return _ConnectedMembersInEdge(iter->second[it->second.secRootIdx]);
}
int AmigosModulePass::_DisconnectedTransfer(unsigned int outPortId)
{
    if (this->mapRootObj.find(outPortId) == this->mapRootObj.end())
    {
        // Internal pass haven't beed created, no need to transfer message to previous module.
        return 0;
    }
    auto it = this->mapPassOutInfo.find(outPortId);
    auto iter = this->mapRootMembers.find(outPortId);
    if (it == this->mapPassOutInfo.end() || iter == this->mapRootMembers.end()
        || iter->second.size() <= it->second.secRootIdx)
    {
        AMILOG_ERR << "Can not find the members in this out: " << outPortId << COLOR_ENDL;
        return -1;
    }
    return _DisonnectedMembersInEdge(iter->second[it->second.secRootIdx]);
}
struct AmigosModulePass::ObjNode &AmigosModulePass::_ImplementMembers(const struct Members &members)
{
    // Members to objects
    AmigosModuleBase *pObj = NULL;
    auto itInitMap = AmigosModuleInit::initMap.find(members.memberModType);
    if (itInitMap == AmigosModuleInit::initMap.end())
    {
        AMIGOS_ERR("MOD: %s,did not preload.\n", members.memberModType.c_str());
        assert(0);
    }
    struct AmigosModulePass::ObjNode &object  = mapPassMemObj[members.memberSection];
    if (object.objUseCnt)
    {
        pObj = object.selfObj;
        if (!members.bOnlySignature)
        {
            // If the member hadn't just looped from current root.
            object.objUseCnt++;
        }
    }
    else
    {
        AmigosDatabase *pSec = this->GetSurface()->GetDbIns()->GetFactory()->Create(members.memberSection);
        pObj = itInitMap->second(members.memberSection);
        assert(pObj);
        pObj->GetSurface()->SetDbIns(pSec);
        pObj->GetSurface()->LoadDb();
        object.selfObj     = pObj;
        object.objUseCnt   = 1;
        object.objInitCnt  = 0;
        object.objBindCnt  = 0;
        object.objStartCnt = 0;
    }
    AMILOG_INFO << "[DELAY-PASS-CREATE] : current section: " << members.memberSection
                << ", use-cnt :" <<  mapPassMemObj[members.memberSection].objUseCnt << COLOR_ENDL;
    for (auto &it : members.prevMembers)
    {
        struct AmigosModulePass::ObjNode &pPrevObj = _ImplementMembers(it);
        unsigned int curInputPort = pObj->GetSurface()->GetInPortIdFromLoopId(it.nextInLoopId);
        if (object.objUseCnt > 1)
        {
            // to judge current pass member has beed linked to previous or not.
            continue;
        }
        AmigosSurfaceBase::ModPortInInfo stInInfo;
        pObj->GetSurface()->GetPortInInfo(curInputPort, stInInfo);
        unsigned int prevOutPortId = pPrevObj.selfObj->GetSurface()->GetOutPortIdFromLoopId(stInInfo.stPrev.loopId);
        pObj->Link(curInputPort, prevOutPortId, pPrevObj.selfObj);
        AMILOG_INFO << "LINK: " << it.memberSection << ':' << prevOutPortId << "->" << members.memberSection
                    << ':' << curInputPort << COLOR_ENDL;
    }
    AMILOG_INFO << "IMPLEMENT: " << members.memberSection << " TYPE: " << members.memberModType << COLOR_ENDL;
    return object;
}
int AmigosModulePass::_GetModulesObjAll(std::vector<struct ObjNode *> &listObj, unsigned int outPortId)
{
    const std::vector<struct ObjNode *> &rootObj = mapRootObj[outPortId];

    for (auto &it : rootObj)
    {
        std::set<std::string> setSecName;
        if (!it || !it->selfObj)
        {
            AMILOG_ERR << "Null pointer" << COLOR_ENDL;
            return -1;
        }
        _TraverseModuleObj(listObj, it, setSecName);
    }
    return 0;
}
int AmigosModulePass::_GetModulesObjIdx(std::vector<struct ObjNode *> &listObj, unsigned int outPortId, unsigned int rootIdx)
{
    const std::vector<struct ObjNode *> &rootObj = mapRootObj[outPortId];

    if (rootIdx >= rootObj.size())
    {
        AMILOG_ERR << "ROOT IDX " << rootIdx << " Loop error!" << COLOR_ENDL;
        return -1;
    }
    struct ObjNode *loopObj = rootObj[rootIdx];
    if (!loopObj || !loopObj->selfObj)
    {
        AMILOG_ERR << "Null pointer" << COLOR_ENDL;
        return -1;
    }
    std::set<std::string> setSecName;
    _TraverseModuleObj(listObj, loopObj, setSecName);
    return 0;
}
void AmigosModulePass::_TraverseModuleObj(std::vector<struct ObjNode *> &listObj, struct ObjNode *loopObj, std::set<std::string> &setSecName)
{
    for (auto &itIn : loopObj->selfObj->mapPortIn)
    {
        AmigosModuleBase *pPrev = itIn.second.pPrev;
        if (!pPrev)
        {
            continue;
        }
        const AmigosSurfaceBase::ModInfo &stModInfo = pPrev->GetSurface()->GetModInfo();
        auto itPrevObj = mapPassMemObj.find(stModInfo.sectionName);
        if (itPrevObj == mapPassMemObj.end() || setSecName.find(stModInfo.sectionName) != setSecName.end())
        {
            // 1. If module on the edge of pass input, it make mapPassMemObj not be found.
            // 2. Previous module had been linked before.
            continue;
        }
        setSecName.insert(stModInfo.sectionName);
        _TraverseModuleObj(listObj, &itPrevObj->second, setSecName);
    }
    listObj.push_back(loopObj);
}
#define PRINT_PASS_RELINK(__forward_obj, __forward_in, __current_obj, __current_out, __replaced_obj, __replaced_out) \
    do                                                                                                               \
    {                                                                                                                \
        const AmigosSurfaceBase::ModInfo &__stInfo_forward = (__forward_obj)->GetSurface()->GetModInfo();            \
        AMILOG_INFO << " [Forward Section] : " << __stInfo_forward.sectionName << COLOR_ENDL;                        \
        AMILOG_INFO << " [PORT]            : " << (__forward_in) << COLOR_ENDL;                                      \
        const AmigosSurfaceBase::ModInfo &__stInfo_current = (__current_obj)->GetSurface()->GetModInfo();            \
        AMILOG_INFO << " [Curr Section]    : " << __stInfo_current.sectionName << COLOR_ENDL;                        \
        AMILOG_INFO << " [PORT]            : " << (__current_out)  << COLOR_ENDL;                                    \
        const AmigosSurfaceBase::ModInfo &__stInfo_replaced = (__replaced_obj)->GetSurface()->GetModInfo();          \
        AMILOG_INFO << " [Replace To]      : " << __stInfo_replaced.sectionName << COLOR_ENDL;                       \
        AMILOG_INFO << " [PORT]            : " << (__replaced_out) << COLOR_ENDL;                                    \
    } while (0)

int AmigosModulePass::_CreateDelayPass(unsigned int outPortId, unsigned int postInId, AmigosModuleBase *postIns)
{
    auto itPassOutInfo = mapPassOutInfo.find(outPortId);
    if (itPassOutInfo == mapPassOutInfo.end())
    {
        AMILOG_ERR << "Can not find pass out info, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    int ret = this->_DoCreateDelayPass(outPortId);
    if (ret != 0)
    {
        AMILOG_ERR << "Create delay-pass failed! PORT: " << outPortId << COLOR_ENDL;
        return -1;
    }
    auto itObj = mapPassMemObj.find(itPassOutInfo->second.secName);
    if (itObj == mapPassMemObj.end())
    {
        DeleteDelayPass(outPortId);
        AMILOG_ERR << "Not found object section : " << itPassOutInfo->second.secName << COLOR_ENDL;
        return -1;
    }
    AmigosModuleBase *passOutObj = itObj->second.selfObj;
    // Link the edge module in 'PASS' output.
    AMILOG_INFO << "Out link Replace current :"<< COLOR_ENDL;
    PRINT_PASS_RELINK(postIns, postInId, this, outPortId, passOutObj, itPassOutInfo->second.secPortId);
    postIns->Link(postInId, itPassOutInfo->second.secPortId, passOutObj);
    return 0;
}

int AmigosModulePass::_DoCreateDelayPass(unsigned int outPortId)
{
    auto iter = this->mapPassOutInfo.find(outPortId);
    if (iter == this->mapPassOutInfo.end())
    {
        AMILOG_ERR << "Not found output port id: " << outPortId << COLOR_ENDL;
        return -1;
    }
    if (mapRootObj.find(outPortId) != mapRootObj.end())
    {
        AMILOG_ERR << "Output port: " << outPortId << " create duplicated!" << COLOR_ENDL;
        return -1;
    }
    std::vector<struct ObjNode *> &rootObj = mapRootObj[outPortId];
    std::vector<int> &rootObjState         = mapRootObjState[outPortId];
    if (rootObjState.size())
    {
        AMILOG_ERR << "PASS-OUT " << outPortId << " Fail: Resource create again!" << COLOR_ENDL;
        return -1;
    }
    auto itRoot = mapRootMembers.find(outPortId);
    if (itRoot != mapRootMembers.end())
    {
        for (auto &itRootList : itRoot->second)
        {
            rootObj.push_back(&_ImplementMembers(itRootList));
            rootObjState.push_back(E_DELAY_PASS_CREATE);
        }
    }
    // Unlink the edge of 'PASS' input and link to the module of 'PASS' input.
    for (auto itPassInInfo = mapPassInInfo.begin(); itPassInInfo != mapPassInInfo.end(); itPassInInfo++)
    {
        auto itPassInObj = mapPassMemObj.find(itPassInInfo->second.secName);
        if (itPassInObj == mapPassMemObj.end())
        {
            continue;
        }
        auto itIn = mapPortIn.find(itPassInInfo->first);
        if (itIn == mapPortIn.end() || !itIn->second.pPrev)
        {
            // After other module had beed linked, it is normal that makes 'PASS' can not find input port.
            continue;
        }
        unsigned int preOutId   = itIn->second.prevOutPortId;
        AmigosModuleBase *pPrev = itIn->second.pPrev;
        pPrev->Stop();
        pPrev->StopOut(preOutId);
        AMILOG_INFO << "In link replace forward:"<< COLOR_ENDL;
        PRINT_PASS_RELINK(this, itPassInInfo->first, pPrev, preOutId, itPassInObj->second.selfObj, itPassInInfo->second.secPortId);
        this->StopIn(itPassInInfo->first);
        this->UnBindBlock(itPassInInfo->first);
        this->Unlink(itPassInInfo->first);
        //After it doing 'Unlink' 'itIn' is not avaliable, pls do not use it.
        itPassInObj->second.selfObj->Link(itPassInInfo->second.secPortId, preOutId, pPrev);
    }
    return 0;
}

int AmigosModulePass::_DestroyDelayPass(unsigned int outPortId, unsigned int postInId, AmigosModuleBase *postIns)
{
    auto itPassOutInfo = mapPassOutInfo.find(outPortId);
    if (itPassOutInfo == mapPassOutInfo.end())
    {
        AMILOG_ERR << "Can not find pass out info, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    auto itPassOutObj = mapPassMemObj.find(itPassOutInfo->second.secName);
    if (itPassOutObj == mapPassMemObj.end())
    {
        AMILOG_ERR << "Can not find pass out obj, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    auto itOut = itPassOutObj->second.selfObj->mapPortOut.find(itPassOutInfo->second.secPortId);
    if (itOut == itPassOutObj->second.selfObj->mapPortOut.end())
    {
        AMILOG_ERR << "Can not find output info, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    if (mapRootObj.empty())
    {
        AMILOG_ERR << "PASS-OUT " << outPortId << " Resouce didn't create." << COLOR_ENDL;
        return -1;
    }
    std::vector<struct ObjNode *> obj;
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    for (auto &it : obj)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = it->selfObj->GetSurface()->GetModInfo();
        if (it->objStartCnt >= it->objUseCnt || it->objBindCnt >= it->objUseCnt || it->objInitCnt >= it->objUseCnt)
        {
            AMILOG_ERR << "[DELAY-PASS-ERR] : current section: " << stInfo.sectionName
                       << ", should stop and unbind and deinit delay-pass first" <<  COLOR_ENDL;
            return -1;
        }
    }
    // Unlink the edge module in 'PASS' output.
    AMILOG_INFO << "Out unlink replace current:"<< COLOR_ENDL;
    PRINT_PASS_RELINK(postIns, postInId, itPassOutObj->second.selfObj, itPassOutInfo->second.secPortId, this, itPassOutInfo->first);
    postIns->Unlink(postInId);
    int ret = _DoDestroyDelayPass(obj);
    if (ret == 0)
    {
        mapRootObj.erase(outPortId);
        mapRootObjState.erase(outPortId);
    }
    return ret;
}
int AmigosModulePass::_DoDestroyDelayPass(const std::vector<struct ObjNode *> &obj)
{
    for (auto &it : obj)
    {
        if (it->objUseCnt > 1)
        {
            it->objUseCnt--;
            continue;
        }
        it->objUseCnt = 0;
    }

    // Unlink the edge module in 'PASS' input and link to 'PASS' of itself.
    for (auto itPassInInfo = mapPassInInfo.begin(); itPassInInfo != mapPassInInfo.end(); itPassInInfo++)
    {
        auto itPassInObj = mapPassMemObj.find(itPassInInfo->second.secName);
        if (itPassInObj == mapPassMemObj.end() || itPassInObj->second.objUseCnt > 0)
        {
            continue;
        }
        auto itIn = itPassInObj->second.selfObj->mapPortIn.find(itPassInInfo->second.secPortId);
        if (itIn == itPassInObj->second.selfObj->mapPortIn.end() || !itIn->second.pPrev)
        {
            AMILOG_ERR << "Not found pass port, id: " << itPassInInfo->second.secPortId<< COLOR_ENDL;
            continue;
        }
        unsigned int preOutId   = itIn->second.prevOutPortId;
        AmigosModuleBase *pPrev = itIn->second.pPrev;
        AMILOG_INFO << "In unlink replace forward:"<< COLOR_ENDL;
        PRINT_PASS_RELINK(itPassInObj->second.selfObj, itPassInInfo->second.secPortId, pPrev, preOutId, this, itPassInInfo->first);
        itPassInObj->second.selfObj->Unlink(itPassInInfo->second.secPortId);
        //After it doing 'Unlink' 'itIn' is not avaliable, pls do not use it.
        this->Link(itPassInInfo->first, preOutId, pPrev);
        this->BindBlock(itPassInInfo->first);
        this->StartIn(itPassInInfo->first);
        pPrev->StartOut(preOutId);
        pPrev->Start();
    }

    for (auto &it : mapPassMemObj)
    {
        if (!it.second.objUseCnt)
        {
            it.second.selfObj->Unlink();
            it.second.selfObj->GetSurface()->UnloadDb();
            AMILOG_INFO << "[DELAY-PASS-Unlink] : current section: " << it.first << COLOR_ENDL;
        }
    }
    auto itObj = mapPassMemObj.begin();
    while (itObj != mapPassMemObj.end())
    {
        auto itNext = itObj;
        itNext++;
        if (!itObj->second.objUseCnt)
        {
            AmigosDatabase *pSec = itObj->second.selfObj->GetSurface()->GetDbIns();
            delete itObj->second.selfObj;
            this->GetSurface()->GetDbIns()->GetFactory()->Destroy(pSec);
            AMILOG_INFO << "[DELAY-PASS-DESTROY] : current section: " << itObj->first << COLOR_ENDL;
            mapPassMemObj.erase(itObj);
        }
        itObj = itNext;
    }
    return 0;
}
int AmigosModulePass::BuildDelayPass(unsigned int outPortId)
{
    return this->_DoCreateDelayPass(outPortId);
}
int AmigosModulePass::DeleteDelayPass(unsigned int outPortId)
{
    std::vector<struct ObjNode *> obj;
    if (mapRootObj.empty())
    {
        AMILOG_ERR << "PASS-OUT " << outPortId << " Resouce didn't create." << COLOR_ENDL;
        return -1;
    }
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    for (auto &it : obj)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = it->selfObj->GetSurface()->GetModInfo();
        if (it->objStartCnt >= it->objUseCnt || it->objBindCnt >= it->objUseCnt || it->objInitCnt >= it->objUseCnt)
        {
            AMILOG_ERR << "[DELAY-PASS-ERR] : current section: " << stInfo.sectionName
                       << ", should stop and unbind and deinit delay-pass first" <<  COLOR_ENDL;
            return -1;
        }
    }
    int ret = _DoDestroyDelayPass(obj);
    if (ret == 0)
    {
        mapRootObj.erase(outPortId);
        mapRootObjState.erase(outPortId);
    }
    return ret;
}
int AmigosModulePass::InitDelayPass(unsigned int outPortId, unsigned int rootIdx)
{
    std::vector<struct ObjNode *> obj;
    if (!_CheckState(outPortId, rootIdx, E_DELAY_PASS_INIT))
    {
        return -1;
    }
    if (_GetModulesObjIdx(obj, outPortId, rootIdx) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoInitDelayPass(obj);
    if (ret == 0)
    {
        _MarkState(outPortId, rootIdx, E_DELAY_PASS_INIT);
    }
    return ret;
}
int AmigosModulePass::_InitDelayPass(unsigned int outPortId)
{
    std::vector<struct ObjNode *> obj;
    if (!_CheckState(outPortId, E_DELAY_PASS_INIT))
    {
        return -1;
    }
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoInitDelayPass(obj);
    if (ret == 0)
    {
        _MarkState(outPortId, E_DELAY_PASS_INIT);
    }
    return ret;
}
int AmigosModulePass::_DoInitDelayPass(const std::vector<struct ObjNode *> &obj)
{
    for (auto &it : obj)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = it->selfObj->GetSurface()->GetModInfo();
        if (it->objInitCnt > 0)
        {
            it->objInitCnt++;
            AMILOG_INFO << "[DELAY-PASS-SKIP] : current section: " << stInfo.sectionName << COLOR_ENDL;
            continue;
        }
        AMILOG_INFO << "[DELAY-PASS-INIT] : current section: " << stInfo.sectionName << COLOR_ENDL;
        it->selfObj->Init();
        it->objInitCnt = 1;
    }
    return 0;
}
int AmigosModulePass::DeinitDelayPass(unsigned int outPortId, unsigned int rootIdx)
{
    std::vector<struct ObjNode *> obj;
    if (_GetModulesObjIdx(obj, outPortId, rootIdx) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoDeinitDelayPass(obj);
    if (ret == 0)
    {
        _UnmarkState(outPortId, rootIdx, E_DELAY_PASS_INIT);
    }
    return ret;
}
int AmigosModulePass::_DeinitDelayPass(unsigned int outPortId)
{
    std::vector<struct ObjNode *> obj;
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoDeinitDelayPass(obj);
    if (ret == 0)
    {
        _UnmarkState(outPortId, E_DELAY_PASS_INIT);
    }
    return ret;
}
int AmigosModulePass::_DoDeinitDelayPass(const std::vector<struct ObjNode *> &obj)
{
    for (auto &it : obj)
    {
        if (it->objStartCnt >= it->objInitCnt || it->objBindCnt >= it->objInitCnt || it->objInitCnt == 0)
        {
            const AmigosSurfaceBase::ModInfo &stInfo = it->selfObj->GetSurface()->GetModInfo();
            AMILOG_ERR << "[DELAY-PASS-ERR] : current section: " << stInfo.sectionName
                       << ", Didn't stop or unbind delay-pass or delay-pass hadn't init before." <<  COLOR_ENDL;
            return -1;
        }
    }
    for (auto it = obj.rbegin(); it != obj.rend(); it++)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = (*it)->selfObj->GetSurface()->GetModInfo();
        if ((*it)->objInitCnt > 1)
        {
            AMILOG_INFO << "[DELAY-PASS-SKIP] : current section: " << stInfo.sectionName << COLOR_ENDL;
            (*it)->objInitCnt--;
            continue;
        }
        AMILOG_INFO << "[DELAY-PASS-DEINIT] : current section: " << stInfo.sectionName << COLOR_ENDL;
        (*it)->selfObj->Deinit();
        (*it)->objInitCnt = 0;
    }
    return 0;
}
int AmigosModulePass::BindDelayPass(unsigned int outPortId, unsigned int rootIdx)
{
    std::vector<struct ObjNode *> obj;
    if (!_CheckState(outPortId, rootIdx, E_DELAY_PASS_BIND))
    {
        return -1;
    }
    if (_GetModulesObjIdx(obj, outPortId, rootIdx) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoBindDelayPass(obj);
    if (ret == 0)
    {
        _MarkState(outPortId, rootIdx, E_DELAY_PASS_BIND);
    }
    return ret;
}
int AmigosModulePass::_BindDelayPass(unsigned int outPortId)
{
    std::vector<struct ObjNode *> obj;
    if (!_CheckState(outPortId, E_DELAY_PASS_BIND))
    {
        return -1;
    }
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoBindDelayPass(obj);
    if (ret == 0)
    {
        _MarkState(outPortId, E_DELAY_PASS_BIND);
    }
    return ret;
}
int AmigosModulePass::_DoBindDelayPass(const std::vector<struct ObjNode *> &obj)
{
    for (auto iter = obj.rbegin(); iter != obj.rend(); iter++)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = (*iter)->selfObj->GetSurface()->GetModInfo();
        if ((*iter)->objBindCnt > 0)
        {
            (*iter)->objBindCnt++;
            AMILOG_INFO << "[DELAY-PASS-SKIP] : current section: " << stInfo.sectionName << COLOR_ENDL;
            continue;
        }
        AMILOG_INFO << "[DELAY-PASS-BIND] : current section: " << stInfo.sectionName << COLOR_ENDL;
        (*iter)->selfObj->Bind();
        (*iter)->objBindCnt = 1;
    }
    return 0;
}
int AmigosModulePass::UnbindDelayPass(unsigned int outPortId, unsigned int rootIdx)
{
    std::vector<struct ObjNode *> obj;
    if (_GetModulesObjIdx(obj, outPortId, rootIdx) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoUnbindDelayPass(obj);
    if (ret == 0)
    {
        _UnmarkState(outPortId, rootIdx, E_DELAY_PASS_BIND);
    }
    return ret;
}
int AmigosModulePass::_UnbindDelayPass(unsigned int outPortId)
{
    std::vector<struct ObjNode *> obj;
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoUnbindDelayPass(obj);
    if (ret == 0)
    {
        _UnmarkState(outPortId, E_DELAY_PASS_BIND);
    }
    return ret;
}
int AmigosModulePass::_DoUnbindDelayPass(const std::vector<struct ObjNode *> &obj)
{
    for (auto &it : obj)
    {
        if (it->objBindCnt == 0)
        {
            const AmigosSurfaceBase::ModInfo &stInfo = it->selfObj->GetSurface()->GetModInfo();
            AMILOG_ERR << "[DELAY-PASS-UNBIND] : current section: " << stInfo.sectionName << ", did not bind." << COLOR_ENDL;
            return -1;
        }
    }
    for (auto &it : obj)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = it->selfObj->GetSurface()->GetModInfo();
        if (it->objBindCnt > 1)
        {
            AMILOG_INFO << "[DELAY-PASS-SKIP] : current section: " << stInfo.sectionName << COLOR_ENDL;
            it->objBindCnt--;
            continue;
        }
        AMILOG_INFO << "[DELAY-PASS-UNBIND] : current section: " << stInfo.sectionName << COLOR_ENDL;
        it->selfObj->UnBind();
        it->objBindCnt = 0;
    }
    return 0;
}
int AmigosModulePass::StartDelayPass(unsigned int outPortId, unsigned int rootIdx)
{
    std::vector<struct ObjNode *> obj;
    if (!_CheckState(outPortId, rootIdx, E_DELAY_PASS_START))
    {
        return -1;
    }
    if (_GetModulesObjIdx(obj, outPortId, rootIdx) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoStartDelayPass(obj);
    if (ret == 0)
    {
        _MarkState(outPortId, rootIdx, E_DELAY_PASS_START);
    }
    return ret;
}
int AmigosModulePass::_StartDelayPass(unsigned int outPortId)
{
    std::vector<struct ObjNode *> obj;
    auto itPassOutInfo = mapPassOutInfo.find(outPortId);
    if (itPassOutInfo == mapPassOutInfo.end())
    {
        AMILOG_ERR << "Can not find pass out info, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    auto it = mapPassMemObj.find(itPassOutInfo->second.secName);
    if (it == mapPassMemObj.end())
    {
        AMILOG_ERR << "Can not find pass out object, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    if (!_CheckState(outPortId, E_DELAY_PASS_START))
    {
        return -1;
    }
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    if (_DoStartDelayPass(obj) != 0)
    {
        AMILOG_ERR << "Output: " << outPortId << " Start error!" << COLOR_ENDL;
        return -1;
    }
    _MarkState(outPortId, E_DELAY_PASS_START);
    // It should just only start this out if module on the edge of pass output,
    // and if it was created by another pass before,
    //
    // Example:
    // |-------------|
    // |PASS|-----|--|--Out0(), Module XX in 'PASS' had beed created by trigger this out0.
    // |----| XX  |  |
    // |    |-----|--|--Out1(), After out0 triggered, module 'XX' just run '_StartOut' only.
    // |-------------|
    it->second.selfObj->StartOut(itPassOutInfo->second.secPortId);
    return 0;
}
int AmigosModulePass::_DoStartDelayPass(const std::vector<struct ObjNode *> &obj)
{
    for (auto it = obj.rbegin(); it != obj.rend(); it++)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = (*it)->selfObj->GetSurface()->GetModInfo();
        if ((*it)->objStartCnt > 0)
        {
            AMILOG_INFO << "[DELAY-PASS-SKIP] : current section: " << stInfo.sectionName << COLOR_ENDL;
            (*it)->objStartCnt++;
            continue;
        }
        (*it)->selfObj->Prepare();
        (*it)->selfObj->StartOut();
        (*it)->selfObj->StartIn();
        (*it)->selfObj->Start();
        (*it)->objStartCnt = 1;
        AMILOG_INFO << "[DELAY-PASS-START-IO] : section: " << stInfo.sectionName << COLOR_ENDL;
        for (auto &itIn : (*it)->selfObj->mapPortIn)
        {
            AmigosModuleBase *pPrev = itIn.second.pPrev;
            if (!pPrev)
            {
                continue;
            }
            const AmigosSurfaceBase::ModInfo &stModInfo = pPrev->GetSurface()->GetModInfo();
            auto itPrevObj = mapPassMemObj.find(stModInfo.sectionName);
            if (itPrevObj == mapPassMemObj.end())
            {
                // It make mapPassMemObj not be found, if module on the edge of pass input.
                //
                // Example:
                //       |-------------|
                // |---| |PASS|-----|--|--Out0(), Module XX in 'PASS' had beed created by trigger on out0.
                // |AA |-|----| XX  |  |
                // |---| |    |-----|--|--Out1(), After out0 had been triggered, module 'XX' just run '_StartOut' only.
                //       |-------------|
                AMILOG_INFO << "[DELAY-PASS-START-OUT] : section: " << stModInfo.sectionName
                            << " PORT: " << itIn.second.prevOutPortId << COLOR_ENDL;
                pPrev->StartOut(itIn.second.prevOutPortId);
                pPrev->Start();
            }
            else if (itPrevObj->second.objStartCnt > 0)
            {
                // It should just only start this out, if previous module which is inside 'PASS' had been created before.
                //
                // Example:
                // |-------------------|
                // |PASS               |
                // | |----|------------|--Out0(), Module AA in 'PASS' had beed created by trigger this out0.
                // | |    |            |
                // |-| AA |   |----|   |
                // | |    |---| BB |---|--Out1(), After out0 triggered, module 'AA' just run '_StartOut' only.
                // | |----|   |----|   |
                // |-------------------|
                AMILOG_INFO << "[DELAY-PASS-START-OUT] : section: " << stModInfo.sectionName
                            << " PORT: " << itIn.second.prevOutPortId << COLOR_ENDL;
                pPrev->StartOut(itIn.second.prevOutPortId);
            }
        }
    }
    return 0;
}
int AmigosModulePass::StopDelayPass(unsigned int outPortId, unsigned int rootIdx)
{
    std::vector<struct ObjNode *> obj;
    if (_GetModulesObjIdx(obj, outPortId, rootIdx) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    int ret = _DoStopDelayPass(obj);
    if (ret == 0)
    {
        _UnmarkState(outPortId, rootIdx, E_DELAY_PASS_START);
    }
    return ret;
}
int AmigosModulePass::_StopDelayPass(unsigned int outPortId)
{
    std::vector<struct ObjNode *> obj;
    if (_GetModulesObjAll(obj, outPortId) != 0 && !obj.size())
    {
        AMILOG_ERR << "Output: " << outPortId << " object error!" << COLOR_ENDL;
        return -1;
    }
    auto itPassOutInfo = mapPassOutInfo.find(outPortId);
    if (itPassOutInfo == mapPassOutInfo.end())
    {
        AMILOG_ERR << "Can not find pass out info, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    auto it = mapPassMemObj.find(itPassOutInfo->second.secName);
    if (it == mapPassMemObj.end())
    {
        AMILOG_ERR << "Can not find pass out object, port: " << outPortId  << COLOR_ENDL;
        return -1;
    }
    // Stop the module on the edge of pass output.
    it->second.selfObj->StopOut(itPassOutInfo->second.secPortId);
    int ret = _DoStopDelayPass(obj);
    if (ret == 0)
    {
        _UnmarkState(outPortId, E_DELAY_PASS_START);
    }
    return ret;
}
int AmigosModulePass::_DoStopDelayPass(const std::vector<struct ObjNode *> &obj)
{
    for (auto &it : obj)
    {
        if (it->objStartCnt == 0)
        {
            const AmigosSurfaceBase::ModInfo &stInfo = it->selfObj->GetSurface()->GetModInfo();
            AMILOG_ERR << "[DELAY-PASS-STOP-IO] : current section: " << stInfo.sectionName << ", did not start." << COLOR_ENDL;
            return -1;
        }
    }
    for (auto it= obj.rbegin(); it != obj.rend(); it++)
    {
        const AmigosSurfaceBase::ModInfo &stInfo = (*it)->selfObj->GetSurface()->GetModInfo();
        if ((*it)->objStartCnt > 1)
        {
            AMILOG_INFO << "[DELAY-PASS-SKIP] : current section: " << stInfo.sectionName << COLOR_ENDL;
            (*it)->objStartCnt--;
            continue;
        }
        (*it)->selfObj->Stop();
        (*it)->selfObj->StopIn();
        (*it)->selfObj->StopOut();
        (*it)->selfObj->Unprepare();
        for (auto &itIn : (*it)->selfObj->mapPortIn)
        {
            AmigosModuleBase *pPrev = itIn.second.pPrev;
            if (!pPrev)
            {
                continue;
            }
            const AmigosSurfaceBase::ModInfo &stModInfo = pPrev->GetSurface()->GetModInfo();
            auto itPrevObj = mapPassMemObj.find(stModInfo.sectionName);
            if (itPrevObj == mapPassMemObj.end())
            {
                // It make mapPassMemObj not be found, if module on the edge of pass input.
                pPrev->Stop();
                pPrev->StopOut(itIn.second.prevOutPortId);
                AMILOG_INFO << "[DELAY-PASS-STOP-OUT] : section: " << stModInfo.sectionName
                            << " PORT: " << itIn.second.prevOutPortId << COLOR_ENDL;
            }
            else if (itPrevObj->second.objStartCnt > 1)
            {
                // It should just only stop this out, if previous module which is inside 'PASS' had been created before.
                pPrev->StopOut(itIn.second.prevOutPortId);
                AMILOG_INFO << "[DELAY-PASS-STOP-OUT] : section: " << stModInfo.sectionName
                            << " PORT: " << itIn.second.prevOutPortId << COLOR_ENDL;
            }
        }
        (*it)->objStartCnt = 0;
        AMILOG_INFO << "[DELAY-PASS-STOP-IO] : section: " << stInfo.sectionName << COLOR_ENDL;
    }
    return 0;
}
std::set<AmigosModuleBase *> AmigosModulePass::GetAllObjectsInPass()
{
    std::set<AmigosModuleBase *> retVal;

    for (auto &it : mapPassMemObj)
    {
        retVal.insert(it.second.selfObj);
    }
    return retVal;
}
AmigosModuleBase * AmigosModulePass::GetObjectInPass(const std::string &sectionName)
{
    auto iter = mapPassMemObj.find(sectionName);
    if (iter == mapPassMemObj.end())
    {
        AMILOG_ERR << "Can not find section [" << sectionName << "] in PASS module" << COLOR_ENDL;
        return nullptr;
    }
    return iter->second.selfObj;
}

bool AmigosModulePass::_CheckState(unsigned int outPortId, unsigned int rootIdx, int targetSt)
{
    auto iter = this->mapRootObjState.find(outPortId);
    if (iter == this->mapRootObjState.end() || rootIdx >= iter->second.size())
    {
        AMILOG_ERR << "Output "<< outPortId << " rootIdx " << rootIdx << " resource didn't exist." << COLOR_ENDL;
        return false;
    }
    if (!(iter->second[rootIdx] & E_DELAY_PASS_CREATE))
    {
        AMILOG_ERR << "PASS-OUT "<< outPortId << " ROOT " << rootIdx << " didn't create." << COLOR_ENDL;
        return false;
    }
    if (!(iter->second[rootIdx] & E_DELAY_PASS_INIT)
        && (targetSt == E_DELAY_PASS_BIND || targetSt == E_DELAY_PASS_START))
    {
        AMILOG_ERR << "PASS-OUT "<< outPortId << " ROOT " << rootIdx << " must init first." << COLOR_ENDL;
        return false;
    }
    if (targetSt == E_DELAY_PASS_INIT && (iter->second[rootIdx] & targetSt))
    {
        AMILOG_ERR << "PASS-OUT "<< outPortId << " ROOT " << rootIdx << " init duplicated." << COLOR_ENDL;
        return false;
    }
    if (targetSt == E_DELAY_PASS_BIND && (iter->second[rootIdx] & targetSt))
    {
        AMILOG_ERR << "PASS-OUT "<< outPortId << " ROOT " << rootIdx << " bind duplicated." << COLOR_ENDL;
        return false;
    }
    if (targetSt == E_DELAY_PASS_START && (iter->second[rootIdx] & targetSt))
    {
        AMILOG_ERR << "PASS-OUT "<< outPortId << " ROOT " << rootIdx << " start duplicated." << COLOR_ENDL;
        return false;
    }
    return true;
}

bool AmigosModulePass::_CheckState(unsigned int outPortId, int targetSt)
{
    auto iter = this->mapRootObjState.find(outPortId);
    if (iter == this->mapRootObjState.end() || !iter->second.size())
    {
        AMILOG_ERR << "Output "<< outPortId << " resource didn't exist." << COLOR_ENDL;
        return false;
    }
    for (auto &it : iter->second)
    {
        if (!(it & E_DELAY_PASS_CREATE))
        {
            AMILOG_ERR << "PASS-OUT "<< outPortId << " didn't create." << COLOR_ENDL;
            return false;
        }
        if (!(it & E_DELAY_PASS_INIT)
            && (targetSt == E_DELAY_PASS_BIND || targetSt == E_DELAY_PASS_START))
        {
            AMILOG_ERR << "PASS-OUT "<< outPortId << " must init first." << COLOR_ENDL;
            return false;
        }
        if (targetSt == E_DELAY_PASS_INIT && (it & targetSt))
        {
            AMILOG_ERR << "PASS-OUT "<< outPortId << " init duplicated." << COLOR_ENDL;
            return false;
        }
        if (targetSt == E_DELAY_PASS_BIND && (it & targetSt))
        {
            AMILOG_ERR << "PASS-OUT "<< outPortId << " bind duplicated." << COLOR_ENDL;
            return false;
        }
        if (targetSt == E_DELAY_PASS_START && (it & targetSt))
        {
            AMILOG_ERR << "PASS-OUT "<< outPortId << " start duplicated." << COLOR_ENDL;
            return false;
        }
    }
    return true;
}

void AmigosModulePass::_MarkState(unsigned int outPortId, unsigned int rootIdx, int targetSt)
{
    auto iter = this->mapRootObjState.find(outPortId);
    if (iter == this->mapRootObjState.end() || rootIdx >= iter->second.size())
    {
        AMILOG_ERR << "Output "<< outPortId << " rootIdx " << rootIdx << " resource didn't exist." << COLOR_ENDL;
        return;
    }
    iter->second[rootIdx] |= targetSt;
}

void AmigosModulePass::_MarkState(unsigned int outPortId, int targetSt)
{
    auto iter = this->mapRootObjState.find(outPortId);
    if (iter == this->mapRootObjState.end() || !this->mapRootObjState.size())
    {
        AMILOG_ERR << "Output "<< outPortId << " resource didn't exist." << COLOR_ENDL;
        return;
    }
    for (auto &it : iter->second)
    {
        it |= targetSt;
    }
}

void AmigosModulePass::_UnmarkState(unsigned int outPortId, unsigned int rootIdx, int targetSt)
{
    auto iter = this->mapRootObjState.find(outPortId);
    if (iter == this->mapRootObjState.end() || rootIdx >= iter->second.size())
    {
        AMILOG_ERR << "Output "<< outPortId << " rootIdx " << rootIdx << " resource didn't exist." << COLOR_ENDL;
        return;
    }
    iter->second[rootIdx] &= (~(unsigned int)targetSt);
}

void AmigosModulePass::_UnmarkState(unsigned int outPortId, int targetSt)
{
    auto iter = this->mapRootObjState.find(outPortId);
    if (iter == this->mapRootObjState.end())
    {
        AMILOG_ERR << "Output "<< outPortId << " resource didn't exist." << COLOR_ENDL;
        return;
    }
    for (auto &it : iter->second)
    {
        it &= (~(unsigned int)targetSt);
    }
}

AMIGOS_MODULE_INIT("PASS", AmigosModulePass);
