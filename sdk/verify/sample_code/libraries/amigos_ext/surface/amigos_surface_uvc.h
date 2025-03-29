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

#ifndef __AMIGOS_SURFACE_UVC_H__
#define __AMIGOS_SURFACE_UVC_H__

#include "amigos_surface_base.h"

class AmigosSurfaceUvc : public AmigosSurfaceBase
{
public:
    struct UvcInfo
    {
        unsigned char ucMaxCnt;
        unsigned int  uintMaxPacket;
        unsigned char ucMult;
        unsigned char ucBurst;
        unsigned char ucCIntf;
        unsigned char ucSIntf;
        std::string   strMode;
        std::string   strType;
    public:
        UvcInfo()
        {
            Clear();
        }
        void Clear()
        {
            ucMaxCnt      = 0;
            uintMaxPacket = 0;
            ucMult        = 0;
            ucBurst       = 0;
            ucCIntf       = 0;
            ucSIntf       = 0;
            strMode.clear();
            strType.clear();
        }
    };
    explicit AmigosSurfaceUvc(const std::string &strInSection);
    virtual ~AmigosSurfaceUvc();
    void GetInfo(UvcInfo &info) const;
    void SetInfo(const UvcInfo &info);
protected:
    UvcInfo stUvcInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_UVC_H__
