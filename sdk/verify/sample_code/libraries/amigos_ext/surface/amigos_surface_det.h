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

#ifndef __AMIGOS_SURFACE_DET_H__
#define __AMIGOS_SURFACE_DET_H__

#include <string>
#include "amigos_surface_base.h"

class AmigosSurfaceDet : public AmigosSurfaceBase
{
public:
    struct DetInfo
    {
        std::string  strFwPath;
        std::string  strModelPath;
        std::string  strModelType;
        unsigned int intThresHold;
        DetInfo()
            : intThresHold(0)
        {}
        void Clear()
        {
            DetInfo tmp;
            *this = tmp;
        }
    };

public:
    explicit AmigosSurfaceDet(const std::string &strInSection);
    virtual ~AmigosSurfaceDet();
protected:
    DetInfo stDetInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};

#endif /* __AMIGOS_SURFACE_DET_H__ */
