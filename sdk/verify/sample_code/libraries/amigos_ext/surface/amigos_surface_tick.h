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

#ifndef __AMIGOS_SURFACE_TICK_H__
#define __AMIGOS_SURFACE_TICK_H__

#include "amigos_surface_base.h"

class AmigosSurfaceTick : public AmigosSurfaceBase
{
public:
    struct TickInfo
    {
        long longTimerSec;
        long longTimerNSec;
        std::string strFrcType;
        std::string strMode;
    public:
        TickInfo()
            : longTimerSec(0), longTimerNSec(0)
        {
        }
        void Clear()
        {
            longTimerSec = 0;
            longTimerNSec = 0;
            strFrcType = "";
        }
    };
    AmigosSurfaceTick(const std::string &strInSection);
    virtual ~AmigosSurfaceTick();
    void GetInfo(TickInfo &info) const;
    void SetInfo(const TickInfo &info);
protected:
    TickInfo stTickInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_TICK_H__
