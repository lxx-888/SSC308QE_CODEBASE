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

#ifndef __AMIGOS_SURFACE_IVE_H__
#define __AMIGOS_SURFACE_IVE_H__

#include "amigos_surface_base.h"

class AmigosSurfaceIve : public AmigosSurfaceBase
{
public:
    struct IveInfo
    {
        std::string strIveType;   //"Bgblur“....
        std::string strIveMode; //"blur", "replace", "mosaic", "blur_mosaic"
        std::string strYMaskPath; //静态背景的Y分量路径
        std::string strUvMaskPath; //静态背景的uv分量路径
        std::string strMaskOp;  //bgblur的预处理。NONE是不做，DILATE 做膨胀，ERODE 做腐蚀
        unsigned int uiMaskThr; //用于Mask源图像二值化的阈值参数，取值范围[0, 255]。
        unsigned int uiBlurLv;  //用于配置背景模糊等级，取值范围[0, 255]。
        unsigned int uiScalingStage;  //用于配置背景模糊缩放挡位，取值范围[1, 15]。
        unsigned int uiSaturationLv;  //用于配置输出图像UV分量色彩饱和度，取值范围[0, 128], 仅 iFord 系列支持。
        unsigned int uiMosaicSize;  //用于配置背景马赛克粒度，取值范围[2, 4, 6, 8, 10], 仅 iFord 系列支持。
        IveInfo() : uiMaskThr(0), uiBlurLv(0), uiScalingStage(0), uiSaturationLv(0), uiMosaicSize(0)
        {
        }
        void Clear()
        {
            IveInfo tmp;
            std::swap(*this, tmp);
        }
    };
    explicit AmigosSurfaceIve(const std::string &strInSection);
    virtual ~AmigosSurfaceIve();
    void GetInfo(IveInfo &info) const
    {
        info = this->stIveInfo;
    }

    void SetInfo(const IveInfo &info)
    {
        this->stIveInfo = info;
    }
protected:
    IveInfo stIveInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};

#endif