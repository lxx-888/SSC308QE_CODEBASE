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

#ifndef __AMIGOS_SURFACE_LDC_H__
#define __AMIGOS_SURFACE_LDC_H__

#include "amigos_surface_base.h"

class AmigosSurfaceLdc : public AmigosSurfaceBase
{
public:

    struct LdcInInfo
    {
        unsigned int width;
        unsigned int height;
        LdcInInfo()
         : width(0), height(0)
        {}
    };

    struct LdcOutInfo
    {
        unsigned int width;
        unsigned int height;
        std::string strOutType;
        std::string strOutFmt;
        LdcOutInfo()
         : width(0), height(0)
        {}
    };

    struct stCalibInfo
    {
        std::string  path;
        void         *addr;
        unsigned int len;

        void clear()
        {
            path.clear();
            addr = nullptr;
            len = 0;
        }
    };

    struct OutRect
    {
        unsigned int x;
        unsigned int y;
        unsigned int width;
        unsigned int height;
    };

    struct stRegionPara
    {
        unsigned int CropMode;
        unsigned int Pan;
        unsigned int Tilt;
        unsigned int ZoomV;
        unsigned int ZoomH;
        unsigned int InRadius;
        unsigned int OutRadius;
        unsigned int FocalRatio;
        unsigned int DistortionRatio;
        unsigned int OutRot;
        unsigned int Rot;
    };

    struct stMap2BinPara
    {
        unsigned int grid;
        std::string  mapX;
        std::string  mapY;

        void clear()
        {
            grid = 0;
            mapX.clear();
            mapY.clear();
        }
    };

    struct stLDCRegion
    {
        unsigned int       RegionMode;
        struct OutRect   OutRect;
        struct stRegionPara Para;
        struct stMap2BinPara map2binPara;

        void clear()
        {
            RegionMode = 0;
            memset(&OutRect, 0, sizeof(OutRect));
            memset(&Para, 0, sizeof(stRegionPara));
            map2binPara.clear();
        }
    };

    struct stLdcLdcModeCfg
    {
        unsigned int       EnBgColor;
        unsigned int       BgColor;
        unsigned int       MountMode;
        unsigned int       RegionNum;
        unsigned int       Center_X_Off;
        unsigned int       Center_Y_Off;
        unsigned int       FisheyeRadius;
        struct stLDCRegion Region[12];

        void clear()
        {
            EnBgColor = 0;
            BgColor   = 0;
            MountMode = 0;
            RegionNum = 0;
            Center_X_Off = 0;
            Center_Y_Off = 0;
            FisheyeRadius = 0;
            for(int i = 0; i < 12; i++)
            {
                Region[i].clear();
            }
        }
    };

    struct stLdcLutModeCfg
    {
        unsigned int Width;
        unsigned int Height;
        std::string  Table_X;
        std::string  Table_Y;
        std::string  Table_Weight;

        void clear()
        {
            Width = 0;
            Height = 0;
            Table_X.clear();
            Table_Y.clear();
            Table_Weight.clear();
        }
    };

    struct stLdcDisModeCfg
    {
        unsigned int DisMode;
        unsigned int UserSliceNum;
        unsigned int FocalLengthX;
        unsigned int FocalLengthY;
        unsigned int SceneType;
        unsigned int MotionLevel;
        unsigned int CropRatio;
        std::string  RotationMatrix;

        void clear()
        {
            DisMode = 0;
            UserSliceNum = 0;
            FocalLengthX = 0;
            FocalLengthY = 0;
            SceneType = 0;
            MotionLevel = 0;
            CropRatio = 0;
            RotationMatrix.clear();
        }
    };

    struct stLdcPmfModeCfg
    {
        std::string PmfCoef;
    };

    struct stLdcStitchModeCfg
    {
        unsigned int ProjType;
        int          Distance;
    };

    struct stLdcNirModeCfg
    {
        int Distance;
    };

    struct stLdcDpuModeCfg
    {
        int Distance;
    };

    struct stLdcLdcHorizontalModeCfg
    {
        int DistortionRatio;
    };

    struct LdcInfo
    {
        std::string                      strWorkMode;
        struct stCalibInfo               CalibInfo;
        struct stLdcLdcModeCfg           LdcCfg;
        struct stLdcLutModeCfg           LutCfg;
        struct stLdcDisModeCfg           DisCfg;
        struct stLdcPmfModeCfg           PmfCfg;
        struct stLdcStitchModeCfg        StitchCfg;
        struct stLdcNirModeCfg           NirCfg;
        struct stLdcDpuModeCfg           DpuCfg;
        struct stLdcLdcHorizontalModeCfg LdcHorizontalCfg;
        LdcInfo()
        {
            Clear();
        }
        void Clear()
        {
            strWorkMode.clear();
            CalibInfo.clear();
            LdcCfg.clear();
            LutCfg.clear();
            DisCfg.clear();
            PmfCfg.PmfCoef.clear();
            memset(&StitchCfg, 0, sizeof(stLdcStitchModeCfg));
            memset(&NirCfg, 0, sizeof(stLdcNirModeCfg));
            memset(&DpuCfg, 0, sizeof(stLdcDpuModeCfg));
            memset(&LdcHorizontalCfg, 0, sizeof(stLdcLdcHorizontalModeCfg));
        }
    };

    explicit AmigosSurfaceLdc(const std::string &strInSection);
    virtual ~AmigosSurfaceLdc();
    void GetInfo(LdcInfo &info, std::map<unsigned int, LdcInInfo> &in,
             std::map<unsigned int, LdcOutInfo> &out) const;
    void SetInfo(const LdcInfo &info, const std::map<unsigned int, LdcInInfo> &in,
             const std::map<unsigned int,LdcOutInfo> &out);


protected:
    struct LdcInfo                     stLdcInfo;
    std::map<unsigned int, LdcInInfo>  mapLdcIn;
    std::map<unsigned int, LdcOutInfo> mapLdcOut;

private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_LDC_H__
