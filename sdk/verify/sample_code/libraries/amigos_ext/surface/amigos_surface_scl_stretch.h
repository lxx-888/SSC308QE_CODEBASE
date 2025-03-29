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

#ifndef __AMIGOS_SURFACE_SCL_STRETCH_H__
#define __AMIGOS_SURFACE_SCL_STRETCH_H__

#include "amigos_surface_base.h"

class AmigosSurfaceSclStretch : public AmigosSurfaceBase
{
public:
    struct SclStretchInfo
    {
        unsigned int uintHwPortMode;
        unsigned int uintOsdEn;
        SclStretchInfo()
        {
            Clear();
        }
        void Clear()
        {
            uintHwPortMode = 0;
            uintOsdEn      = 0;
        }
    };
    struct SclStretchInInfo
    {
        unsigned int uintCropX;
        unsigned int uintCropY;
        unsigned int uintCropW;
        unsigned int uintCropH;
        SclStretchInInfo()
        {
            uintCropX = 0;
            uintCropY = 0;
            uintCropW = 0;
            uintCropH = 0;
        }
    };
    struct SclStretchOutInfo
    {
        unsigned int uintRowNum;
        unsigned int uintColNum;
        std::string  strOutFmt;
        SclStretchOutInfo()
        {
            uintRowNum = 0;
            uintColNum = 0;
        }
    };

public:
    explicit AmigosSurfaceSclStretch(const std::string &strInSection);
    virtual ~AmigosSurfaceSclStretch();

protected:
    SclStretchInfo stSclStretchInfo;
    std::map<unsigned int, SclStretchInInfo>  mapSclStretchIn;
    std::map<unsigned int, SclStretchOutInfo> mapSclStretchOut;

private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_SCL_STRETCH_H__
