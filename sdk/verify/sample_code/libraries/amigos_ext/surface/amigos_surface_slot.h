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

#ifndef __AMIGOS_SURFACE_SLOT_H__
#define __AMIGOS_SURFACE_SLOT_H__

#include "amigos_surface_base.h"

class AmigosSurfaceSlot : public AmigosSurfaceBase
{
public:
    struct SlotInfo
    {
        unsigned int uintDstBindMod;
        SlotInfo()
        {
            Clear();
        }
        void Clear()
        {
            uintDstBindMod = 0;
        }
    };
    AmigosSurfaceSlot(const std::string &strInSection);
    virtual ~AmigosSurfaceSlot();

protected:
    SlotInfo stSlotInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_SLOT_H__
