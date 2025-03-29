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

#ifndef __AMIGOS_SURFACE_INJECT_H__
#define __AMIGOS_SURFACE_INJECT_H__

#include "amigos_surface_base.h"

typedef struct stInjectOutInfo_s
{
    unsigned int uintBackGroudColor;
    unsigned int uintVideoFmt;
    unsigned int uintVideoWidth;
    unsigned int uintVideoHeight;
    unsigned int bEnableOsd;

    std::string  strInjectOsdSrcFile;
    unsigned int uintOsdDelay;
    unsigned int uintOsdColor;
    unsigned int uintOsdFmt;
    unsigned int uintOsdWidth;
    unsigned int uintOsdHeight;
    unsigned int uintOsdTargetPortId;
    unsigned int uintOsdTargetPortWid;
    unsigned int uintOsdTargetPortHei;
    unsigned int uintOsdShowFunction;
} stInjectOutInfo_t;

class AmigosSurfaceInject : public AmigosSurfaceBase
{
public:
    explicit AmigosSurfaceInject(const std::string &strInSection);
    virtual ~AmigosSurfaceInject();

protected:
    std::map<unsigned int, stInjectOutInfo_t> mapInjectOutInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_INJECT_H__
