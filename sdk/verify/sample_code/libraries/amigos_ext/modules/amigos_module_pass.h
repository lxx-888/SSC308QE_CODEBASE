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

#ifndef __AMIGOS_MODULE_PASS_H__
#define __AMIGOS_MODULE_PASS_H__

#include "amigos_module_base.h"
#include "amigos_surface_pass.h"

class AmigosModulePass: public AmigosSurfacePass, public AmigosModuleBase
{
public:
    explicit AmigosModulePass(const std::string &strInSection);
    ~AmigosModulePass() override;
    unsigned int GetModId() const override;
    unsigned int GetInputType(unsigned int port) const override;
    unsigned int GetOutputType(unsigned int port) const override;
    int BuildDelayPass(unsigned int outPortId);
    int DeleteDelayPass(unsigned int outPortId);
    int InitDelayPass(unsigned int outPortId, unsigned int rootIdx);
    int DeinitDelayPass(unsigned int outPortId, unsigned int rootIdx);
    int BindDelayPass(unsigned int outPortId, unsigned int rootIdx);
    int UnbindDelayPass(unsigned int outPortId, unsigned int rootIdx);
    int StartDelayPass(unsigned int outPortId, unsigned int rootIdx);
    int StopDelayPass(unsigned int outPortId, unsigned int rootIdx);
    std::set<AmigosModuleBase *> GetAllObjectsInPass();
    AmigosModuleBase *GetObjectInPass(const std::string &sectionName);
private:
    struct ObjNode
    {
        AmigosModuleBase *selfObj;
        unsigned int     objUseCnt;
        unsigned int     objBindCnt;
        unsigned int     objInitCnt;
        unsigned int     objStartCnt;
        ObjNode () : selfObj(0), objUseCnt(0), objBindCnt(0), objInitCnt(0), objStartCnt(0)
        {
        }
    };
    void _Init() override;
    void _Deinit() override;
    void _Start() override;
    void _Stop() override;
    int _ConnectedTransfer(unsigned int outPortId) override;
    int _DisconnectedTransfer(unsigned int outPortId) override;
    bool _IsDelayConnected(unsigned int uintInPort) override;
    int _CreateDelayPass(unsigned int outPortId, unsigned int postInId, AmigosModuleBase *postIns) override;
    int _DestroyDelayPass(unsigned int outPortId, unsigned int postInId, AmigosModuleBase *postIns) override;
    int _InitDelayPass(unsigned int outPortId) override;
    int _DeinitDelayPass(unsigned int outPortId) override;
    int _BindDelayPass(unsigned int outPortId) override;
    int _UnbindDelayPass(unsigned int outPortId) override;
    int _StartDelayPass(unsigned int outPortId) override;
    int _StopDelayPass(unsigned int outPortId) override;
    int _DoCreateDelayPass(unsigned int outPortId);
    int _DoDestroyDelayPass(const std::vector<struct ObjNode *> &obj);
    int _DoInitDelayPass(const std::vector<struct ObjNode *> &obj);
    int _DoDeinitDelayPass(const std::vector<struct ObjNode *> &obj);
    int _DoBindDelayPass(const std::vector<struct ObjNode *> &obj);
    int _DoUnbindDelayPass(const std::vector<struct ObjNode *> &obj);
    int _DoStartDelayPass(const std::vector<struct ObjNode *> &obj);
    int _DoStopDelayPass(const std::vector<struct ObjNode *> &obj);
    int _GetModulesObjAll(std::vector<struct ObjNode *> &listObj, unsigned int outPortId);
    int _GetModulesObjIdx(std::vector<struct ObjNode *> &listObj, unsigned int outPortId, unsigned int rootIdx);
    void _TraverseModuleObj(std::vector<struct ObjNode *> &listObj, struct ObjNode *loopObj, std::set<std::string> &setSecName);
    int _ConnectedMembersInEdge(const struct Members &members);
    int _DisonnectedMembersInEdge(const struct Members &members);
    struct AmigosModulePass::ObjNode &_ImplementMembers(const struct Members &members);
    bool _CheckState(unsigned int outPortId, unsigned int rootIdx, int targetSt);
    bool _CheckState(unsigned int outPortId, int targetSt);
    void _MarkState(unsigned int outPortId, int targetSt);
    void _MarkState(unsigned int outPortId, unsigned int rootIdx, int targetSt);
    void _UnmarkState(unsigned int outPortId, int targetSt);
    void _UnmarkState(unsigned int outPortId, unsigned int rootIdx, int targetSt);
    std::map<unsigned int, std::vector<struct ObjNode *>> mapRootObj;
    std::map<unsigned int, std::vector<int>> mapRootObjState;
    std::map<std::string, struct ObjNode> mapPassMemObj;
};
#endif

