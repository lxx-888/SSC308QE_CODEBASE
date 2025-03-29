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

#ifndef __AMIGOS_SURFACE_SWITCH_H__
#define __AMIGOS_SURFACE_SWITCH_H__

#include <map>
#include <string>
#include "amigos_surface_base.h"

class AmigosSurfaceSwitch : public AmigosSurfaceBase
{
public:
    explicit AmigosSurfaceSwitch(const std::string &strInSection);
    virtual ~AmigosSurfaceSwitch();
protected:
    std::map<unsigned int, int> mapSwitchInInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};

#endif /* __AMIGOS_SURFACE_SWITCH_H__ */
