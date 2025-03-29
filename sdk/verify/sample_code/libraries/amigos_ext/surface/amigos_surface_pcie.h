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

#ifndef __AMIGOS_SURFACE_PCIE_H__
#define __AMIGOS_SURFACE_PCIE_H__

#include "amigos_surface_base.h"

class AmigosSurfacePcie : public AmigosSurfaceBase
{
public:
    struct PcieInfo
    {
        unsigned int uintDevType;
        unsigned int uintTgtId;
        unsigned int uintTgtDevId;
        unsigned int uintTgtChnId;
        unsigned int uintEsRingBuf;
    public:
        PcieInfo()
        {
            Clear();
        }
        void Clear()
        {
            uintDevType   = 0;
            uintTgtId     = 0;
            uintTgtDevId  = 0;
            uintTgtChnId  = 0;
            uintEsRingBuf = 0;
        }
    };
    explicit AmigosSurfacePcie(const std::string &strInSection);
    virtual ~AmigosSurfacePcie();
    void GetInfo(PcieInfo &info) const;
    void SetInfo(const PcieInfo &info);

protected:
    PcieInfo stPcieInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_PCIE_H__
