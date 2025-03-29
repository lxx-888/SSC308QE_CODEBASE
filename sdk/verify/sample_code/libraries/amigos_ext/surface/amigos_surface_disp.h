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

#ifndef __AMIGOS_SURFACE_DISP_H__
#define __AMIGOS_SURFACE_DISP_H__

#include "amigos_surface_base.h"

class AmigosSurfaceDisp : public AmigosSurfaceBase
{
public:
    struct DispInfo
    {
        std::string strDevType;     // 0: panel 1: hdmi: 2: vga 3: cvbs out
        std::string strPnlLinkType; // 0: mipi 11: ttl
        std::string strOutTiming;
        std::string strBackGroundColor;
    public:
        DispInfo()
        {
            Clear();
        }
        void Clear()
        {
            strDevType.clear();
            strPnlLinkType.clear();
            strOutTiming.clear();
            strBackGroundColor.clear();
        }
    };
    struct DispLayerInputPortInfo
    {
        unsigned int uintSrcWidth;
        unsigned int uintSrcHeight;
        unsigned int uintDstWidth;
        unsigned int uintDstHeight;
        unsigned int uintDstXpos;
        unsigned int uintDstYpos;
        unsigned int uintSysChn;
        unsigned int uintLayerId;
        unsigned int uintLayerPortId;
        DispLayerInputPortInfo()
            : uintSrcWidth(0), uintSrcHeight(0), uintDstWidth(0),
              uintDstHeight(0), uintDstXpos(0), uintDstYpos(0),
              uintSysChn(0), uintLayerId(0), uintLayerPortId(0)
        {}
    };
    struct DispLayerInfo
    {
        unsigned int uintId;
        unsigned int uintRot;
        unsigned int uintWidth;
        unsigned int uintHeight;
        unsigned int uintDispWidth;
        unsigned int uintDispHeight;
        unsigned int uintDispXpos;
        unsigned int uintDispYpos;
        DispLayerInfo()
            : uintId(0), uintRot(0), uintWidth(0),
              uintHeight(0), uintDispWidth(0), uintDispHeight(0),
              uintDispXpos(0), uintDispYpos(0)
        {}
    };
    explicit AmigosSurfaceDisp(const std::string &strInSection);
    virtual ~AmigosSurfaceDisp();
    void GetInfo(DispInfo &info, std::map<unsigned int, DispLayerInfo> &layerInfo)const;
    void SetInfo(const DispInfo &info, const std::map<unsigned int, DispLayerInfo> &layerInfo);

protected:
    DispInfo                                       stDispInfo;
    std::map<unsigned int, DispLayerInfo>          mapLayerInfo;
    std::map<unsigned int, DispLayerInputPortInfo> mapDispInInfo;

private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_DISP_H__
