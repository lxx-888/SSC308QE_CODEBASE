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

#ifndef __AMIGOS_SURFACE_IQ_H__
#define __AMIGOS_SURFACE_IQ_H__

#include <vector>
#include "amigos_surface_base.h"

class AmigosSurfaceIq : public AmigosSurfaceBase
{
public:
    struct IqInfo
    {
        bool intCus3a;
        bool intCus3aAe;
        bool intCus3aAwb;
        bool intCus3aAf;
        int intIqServer;
        int intCus3aBlack;
        int intUsrKey;
        int intCaliCfgCnt;
        std::string Cus3aType;
        struct CaliCfg
        {
            std::string caliItem;
            std::string caliFileName;
        };
        std::vector<struct CaliCfg> vectCaliCfg;
    public:
        IqInfo()
        {
            Clear();
        }
        void Clear()
        {
            intCus3a      = false;
            intCus3aAe    = false;
            intCus3aAwb   = false;
            intCus3aAf    = false;
            intCus3aBlack = 0;
            intUsrKey     = 0;
            Cus3aType.clear();
            vectCaliCfg.clear();
        }
    };

    explicit AmigosSurfaceIq(const std::string &strInSection);
    virtual ~AmigosSurfaceIq();
    void GetInfo(IqInfo &info)const;
    void SetInfo(const IqInfo &info);

protected:
    IqInfo stIqInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_IQ_H__
