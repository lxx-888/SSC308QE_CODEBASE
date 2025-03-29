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

#ifndef __AMIGOS_SURFACE_VIF_H__
#define __AMIGOS_SURFACE_VIF_H__

#include "amigos_surface_base.h"

class AmigosSurfaceVif : public AmigosSurfaceBase
{
public:
    struct VifInfo
    {
        int          intSensorId;
        int          intHdrType;
        int          intHdrExposureMask;
        int          intWorkMode;
        unsigned int uintStitchMask;
    public:
        VifInfo()
        {
            Clear();
        }
        void Clear()
        {
            intSensorId    = 0;
            intHdrType     = 0;
            intHdrExposureMask = 0;
            intWorkMode    = 0;
            uintStitchMask = 0;
        }
    };
    struct VifOutInfo
    {
        int intCropX;
        int intCropY;
        int intCropW;
        int intCropH;
        int intWidth;
        int intHeight;
        int intIsUseSnrFmt;
        int intCompressMode;
        std::string strOutType;
        std::string strOutFmt;
        std::string strBayerId;
        std::string strPrecision;
        VifOutInfo()
            : intCropX(0), intCropY(0), intCropW(0), intCropH(0),
              intWidth(0), intHeight(0), intIsUseSnrFmt(0), intCompressMode(0)
        {}
    };
    explicit AmigosSurfaceVif(const std::string &strInSection);
    virtual ~AmigosSurfaceVif();
    void GetInfo(VifInfo &info, std::map<unsigned int, VifOutInfo> &out) const;
    void SetInfo(const VifInfo &info, const std::map<unsigned int, VifOutInfo> &out);
protected:
    VifInfo                            stVifInfo;
    std::map<unsigned int, VifOutInfo> mapVifOutInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_VIF_H__
