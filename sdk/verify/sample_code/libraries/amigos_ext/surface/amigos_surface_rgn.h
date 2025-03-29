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

#ifndef __AMIGOS_SURFACE_RGN_H__
#define __AMIGOS_SURFACE_RGN_H__

#include <list>
#include <string>
#include "amigos_surface_base.h"

class AmigosSurfaceRgn : public AmigosSurfaceBase
{
public:
    struct CanvasInfo
    {
        unsigned int intShow;
        unsigned int intLayer;
        unsigned int intPosX;
        unsigned int intPosY;
        std::string  strAlphaType;
        unsigned int intAlphaVal;
        unsigned int intAlpha0;
        unsigned int intAlpha1;
        unsigned int intPaletteIdx;
        CanvasInfo()
            : intShow(0),
              intLayer(0),
              intPosX(0),
              intPosY(0),
              intAlphaVal(0),
              intAlpha0(0),
              intAlpha1(0),
              intPaletteIdx(0)
        {
        }
    };
    struct LineInfo
    {
        std::string strPixelFmt;
        unsigned int intColor;
        std::string strThickness;
        LineInfo() : intColor(0) {}
    };
    struct DotMatrixInfo
    {
        std::string strPixelFmt;
        unsigned int intColor;
        std::string strSize;
        DotMatrixInfo() : intColor(0) {}
    };
    struct TextInfo
    {
        std::string strPixelFmt;
        unsigned int intColor;
        std::string strFontSize;
        unsigned int intPosX;
        unsigned int intPosY;
        unsigned int intAreaW;
        unsigned int intAreaH;
        std::string strFontFile;
        TextInfo() : intColor(0), intPosX(0), intPosY(0), intAreaW(0), intAreaH(0) {}
    };
    struct CoverInfo
    {
        std::string strType;
        unsigned int intColor;
        std::string strBlockSize;
        CoverInfo() : intColor(0) {}
    };
    struct FrameInfo
    {
        unsigned int intColor;
        std::string strThickness;
        FrameInfo() : intColor(0) {}
    };
    struct RgnInputInfo
    {
        std::string   strMode;
        CanvasInfo    stCanvasInfo;
        LineInfo      stLineInfo;
        DotMatrixInfo stDotMatrixInfo;
        TextInfo      stTextInfo;
        CoverInfo     stCoverInfo;
        FrameInfo     stFrameInfo;
    };
    struct RgnModAttachInfo
    {
        std::string strMod;
        unsigned int intPort;
        unsigned int intIsInPort;
        unsigned int intTimingW;
        unsigned int intTimingH;
        RgnModAttachInfo() : intPort(0), intIsInPort(0), intTimingW(0), intTimingH(0) {}
    };
    struct RgnInfo
    {
        std::list<RgnModAttachInfo> lstAttach;
        unsigned int intColorInvertEn;
        std::string strColorInvertMode;
        unsigned int intColorInvertThresholdL;
        unsigned int intColorInvertThresholdH;
    public:
        RgnInfo()
            : intColorInvertEn(0), intColorInvertThresholdL(0), intColorInvertThresholdH(0)
        {
        }
        void Clear()
        {
            RgnInfo tmp;
            *this = tmp;
        }
    };

public:
    explicit AmigosSurfaceRgn(const std::string &strInSection);
    virtual ~AmigosSurfaceRgn();
    bool GetRgnInfo(RgnInfo &stInfo) const;
    void SetRgnInfo(const RgnInfo &stInfo);
    bool GetRgnInInfo(unsigned int portId, RgnInputInfo &stIn) const;
    void SetRgnInInfo(unsigned int portId, const RgnInputInfo &stIn);

protected:
    std::map<unsigned int, RgnInputInfo> mapRgnInputInfo;
    RgnInfo                              stRgnInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};

#endif /* __AMIGOS_SURFACE_RGN_H__ */
