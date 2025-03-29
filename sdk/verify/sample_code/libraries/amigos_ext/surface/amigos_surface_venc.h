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

#ifndef __AMIGOS_SURFACE_VENC_H__
#define __AMIGOS_SURFACE_VENC_H__

#include <string>
#include "amigos_surface_base.h"

class AmigosSurfaceVenc : public AmigosSurfaceBase
{
public:
    struct VencVbrConfig
    {
        int intGop;
        int intMinQp;
        int intMaxQp;
        int intBitRate;
        VencVbrConfig()
            : intGop(0), intMinQp(0), intMaxQp(0), intBitRate(0)
        {}
    };
    struct VencCbrConfig
    {
        int intGop;
        int intBitRate;
        VencCbrConfig()
            : intGop(0), intBitRate(0)
        {}
    };
    struct VencFixQpConfig
    {
        int intGop;
        int intIQp;
        int intPQp;
        int intQfactor;
        VencFixQpConfig()
            : intGop(0), intIQp(0), intPQp(0), intQfactor(0)
        {}
    };
    struct VencAvbrConfig
    {
        int intGop;
        int intMinQp;
        int intMaxQp;
        int intBitRate;
        VencAvbrConfig()
            : intGop(0), intMinQp(0), intMaxQp(0), intBitRate(0)
        {}
    };
    struct VencCvbrConfig
    {
        int intGop;
        int intMaxBitRate;
        int intShortTermStatsTime;
        int intLongTermStatsTime;
        int intLongTermMaxBitRate;
        int intLongTermMinBitRate;
        VencCvbrConfig()
            : intGop(0), intMaxBitRate(0), intShortTermStatsTime(0), intLongTermStatsTime(0), intLongTermMaxBitRate(0), intLongTermMinBitRate(0)
        {}
    };
    struct VencInfo
    {
        int intMaxWidth;
        int intMaxHeight;
        int intWidth;
        int intHeight;
        int intRcMode; // 0 cbr, 1 vbr, 2 vbr fixqp, 3 avbr
        int intRefLtr; // NormalP、LTR_I、LTR_VI、TSVC-2、TSVC-3
        int LtrEnable;
        int Base; //ref
        int Enhance; //ref
        int bEnablePred; //ref
        int RoiNum;
        unsigned char StreamCntEnable;
        int intMaxStreamCnt;
        union
        {
            VencVbrConfig   stVbrCfg;
            VencCbrConfig   stCbrCfg;
            VencFixQpConfig stFixQpCfg;
            VencAvbrConfig  stAvbrCfg;
            VencCvbrConfig  stCvbrCfg;
        };
        int intEncodeFps;
        int intMultiSlice;
        int intSliceRowCnt;
        bool yuvEnable;
        unsigned int yuvWidth;
        unsigned int yuvHeight;
        int DeBreathEnable;
        int Strength0;
        int Strength1;
        std::string strEncodeType;
        std::string strRcMode;
    public:
        VencInfo()
            : intMaxWidth(0), intMaxHeight(0), intWidth(0), intHeight(0),
              intRefLtr(0),LtrEnable(0), Base(0), Enhance(0),
              bEnablePred(0), RoiNum(0),StreamCntEnable(0),intMaxStreamCnt(0), intEncodeFps(0), intMultiSlice(0), intSliceRowCnt(0),
              yuvEnable(false), yuvWidth(0), yuvHeight(0),
              DeBreathEnable(0), Strength0(0), Strength1(0)
        {
        }
        void Clear()
        {
            strEncodeType.clear();
            strRcMode.clear();
            VencInfo tmp;
            *this = tmp;
        }
    };

    struct VencLayerInfo
    {
        unsigned char index;
        unsigned char bEnable;
        unsigned char bAbsQp;
        int           s32Qp;
        unsigned short u16RectX;
        unsigned short u16RectY;
        unsigned short u16RectWidth;
        unsigned short u16RectHeight;
        VencLayerInfo()
            : index(0), bEnable(0), bAbsQp(0),
              s32Qp(0), u16RectX(0), u16RectY(0), u16RectWidth(0), u16RectHeight(0)
        {}
    };
    explicit AmigosSurfaceVenc(const std::string &strInSection);
    virtual ~AmigosSurfaceVenc();
    void GetInfo(VencInfo &info) const;
    void SetInfo(const VencInfo &info);

protected:
    VencInfo stVencInfo;
    VencLayerInfo stVencRoi[15];
    struct VencDevInfo
    {
        unsigned int width;
        unsigned int height;
        unsigned int refCnt;
    };
    static std::map<unsigned int, VencDevInfo> mapDevInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_VENC_H__
