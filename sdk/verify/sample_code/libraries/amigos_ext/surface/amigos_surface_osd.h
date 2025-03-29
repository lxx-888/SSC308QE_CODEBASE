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

#ifndef __AMIGOS_SURFACE_OSD_H__
#define __AMIGOS_SURFACE_OSD_H__
#include <list>
#include <sstream>
#include "amigos_surface_base.h"

class AmigosSurfaceOsd : public AmigosSurfaceBase
{
public:
    struct OsdInfo
    {
        std::string osdFile;   // File path
        std::string osdFormat; // argb1555, argb4444, argb8888
        std::string alphaMode; // constant or pixel
        union
        {
            unsigned char ucharConstantAlpha;
            struct
            {
                unsigned char ucharAlpha0;
                unsigned char ucharAlpha1;
            };
        };
        unsigned int uintLayer;
        unsigned int uintOsdWidth;
        unsigned int uintOsdHeight;
        unsigned int uintOsdPosX;
        unsigned int uintOsdPosY;
        void        *pExtData;
    public:
        OsdInfo()
        {
            Clear();
        }
        void Clear()
        {
            osdFile.clear();
            osdFormat.clear();
            alphaMode.clear();
            ucharConstantAlpha = 0;
            ucharAlpha0        = 0;
            ucharAlpha1        = 0;
            uintLayer          = 0;
            uintOsdWidth       = 0;
            uintOsdHeight      = 0;
            uintOsdPosX        = 0;
            uintOsdPosY        = 0;
            pExtData           = NULL;
        }
    };
    explicit AmigosSurfaceOsd(const std::string &strInSection);
    virtual ~AmigosSurfaceOsd();

protected:
    std::list<OsdInfo> listOsdInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_UVC_H__
