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

#ifndef __AMIGOS_SURFACE_SCL_H__
#define __AMIGOS_SURFACE_SCL_H__

#include "amigos_surface_base.h"

class AmigosSurfaceScl : public AmigosSurfaceBase
{
public:
    struct SclInfo
    {
        unsigned int uintHwPortMode;
        unsigned int uintRotation;
    public:
        SclInfo()
        {
            Clear();
        }
        void Clear()
        {
            uintHwPortMode = 0;
            uintRotation   = 0;
        }
    };
    struct SclInInfo
    {
        unsigned int uintSclInCropX;
        unsigned int uintSclInCropY;
        unsigned int uintSclInWidth;
        unsigned int uintSclInHeight;
        SclInInfo()
        {
            uintSclInCropX  = 0;
            uintSclInCropY  = 0;
            uintSclInWidth  = 0;
            uintSclInHeight = 0;
        }
    };

    struct SclOutInfo
    {
        unsigned int uintIsMirror;
        unsigned int uintIsFlip;
        unsigned int uintSclOutCropX;
        unsigned int uintSclOutCropY;
        unsigned int uintSclOutCropW;
        unsigned int uintSclOutCropH;
        unsigned int uintSclOutWidth;
        unsigned int uintSclOutHeight;
        unsigned int uintCompressMode;
        std::string strOutType;
        std::string strOutFmt;
        SclOutInfo()
        {
            uintIsMirror     = 0;
            uintIsFlip       = 0;
            uintSclOutCropX  = 0;
            uintSclOutCropY  = 0;
            uintSclOutCropW  = 0;
            uintSclOutCropH  = 0;
            uintSclOutWidth  = 0;
            uintSclOutHeight = 0;
            uintCompressMode = 0;
        }
    };
    explicit AmigosSurfaceScl(const std::string &strInSection);
    virtual ~AmigosSurfaceScl();
    void GetInfo(SclInfo &info) const;
    void SetInfo(const SclInfo &info);
    void GetInInfo(unsigned int portId, SclInInfo &stIn) const;
    void SetInInfo(unsigned int portId, const SclInInfo &stIn);
    void GetOutInfo(unsigned int portId, SclOutInfo &stOut) const;
    void SetOutInfo(unsigned int portId, const SclOutInfo &stOut);

protected:
    SclInfo                            stSclInfo;
    std::map<unsigned int, SclInInfo>  mapSclIn;
    std::map<unsigned int, SclOutInfo> mapSclOut;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_SCL_H__
