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

#ifndef __AMIGOS_SURFACE_PASS_H__
#define __AMIGOS_SURFACE_PASS_H__

#include <set>
#include <list>
#include <string>
#include "amigos_surface_base.h"

class AmigosSurfacePass : public AmigosSurfaceBase
{
public:
    AmigosSurfacePass(const std::string &strInSection);
    virtual ~AmigosSurfacePass();
protected:
    struct PassIoInfo
    {
        PassIoInfo()
            : secPortId(0), secRootIdx(-1)
        {}
        std::string secName;
        unsigned int secPortId;
        unsigned int secRootIdx;
    };
    struct Members
    {
        bool                      bOnlySignature;
        unsigned int              nextInLoopId;
        std::string               memberSection;
        std::string               memberModType;
        std::list<struct Members> prevMembers;
        Members()
            : bOnlySignature(false), nextInLoopId(-1)
        {
        }
        Members(unsigned int id)
            : bOnlySignature(false), nextInLoopId(id)
        {
        }
    };
    std::map<unsigned int, struct PassIoInfo>         mapPassInInfo;
    std::map<unsigned int, struct PassIoInfo>         mapPassOutInfo;
    std::map<unsigned int, std::vector<struct Members>> mapRootMembers;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
    void _TraverseMembers(const std::string &sec, struct AmigosSurfacePass::Members &passMem, std::set<std::string> &setSecName);
    void _SignatureMembers(const std::string &sec, struct AmigosSurfacePass::Members &passMem);
};
#endif //__AMIGOS_SURFACE_PASS_H__
