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
#pragma once

#include "amigos_surface_base.h"

class AmigosSurfaceHseg : public AmigosSurfaceBase
{
public:

    struct HsegInfo
    {
        std::string strHsegMode; //"blur", "replace"
        std::string strIpuPath;
        std::string strModelPath;
        std::string strMaskOp;  //bgblur的预处理。NONE是不做，DILATE 做膨胀，ERODE 做腐蚀
        unsigned int uiMaskThr; //用于Mask源图像二值化的阈值参数，取值范围[0, 255]。
        unsigned int uiBlurLv;  //用于配置背景模糊等级，取值范围[0, 255]。
        unsigned int uiScalingStage;  //用于配置背景模糊缩放挡位，取值范围[1, 15]。
        HsegInfo() : uiMaskThr(0), uiBlurLv(0), uiScalingStage(0)
        {
        }
        void Clear()
        {
            HsegInfo tmp;
            std::swap(*this, tmp);
        }
    };
    explicit AmigosSurfaceHseg(const std::string &strInSection);
    virtual ~AmigosSurfaceHseg();


protected:
    struct HsegInfo  stHsegInfo;

private:
    void _LoadDb();
    void _UnloadDb();
};
