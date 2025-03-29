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

class AmigosSurfaceVdf : public AmigosSurfaceBase
{
public:
    struct Point
    {
        unsigned int  u32x;
        unsigned int  u32y;
        Point() : u32x(0), u32y(0)
        {}
    }; //定义点坐标

    struct VgLine
    {
        Point px;   //point x
        Point py;   //point y
        Point pdx;  //point direction x
        Point pdy;  //point direction y
    };

    struct MdAttr
    {
        unsigned int u32Sensitivity;
        unsigned int u32Thr;
        unsigned int u32ObjNumMax;
        unsigned int u32LearnRate;
        unsigned int u32PointNum;
        Point        stPnt[8];
        MdAttr()
        {
            Clear();
        }
        void Clear()
        {
            u32Sensitivity = 0;
            u32Thr = 0;
            u32ObjNumMax = 0;
            u32LearnRate = 0;
            u32PointNum = 0;
            for (int i = 0; i < 8; i++)
            {
                stPnt[i].u32x = 0;
                stPnt[i].u32y = 0;
            }
        }
    };

    struct OdAttr
    {
        unsigned int u32PointNum;
        Point        stPnt[8];
        OdAttr()
        {
            Clear();
        }
        void Clear()
        {
            u32PointNum = 0;
            for (int i = 0; i < 8; i++)
            {
                stPnt[i].u32x = 0;
                stPnt[i].u32y = 0;
            }
        }
    };

    struct VgAttr
    {
        unsigned int   u32LineNum;
        VgLine         stLine[4]; //gate for 4 line
        Point          stPt[4];  //inva 4 points  for rect
        VgAttr() : u32LineNum(0)
        {
            Clear();
        }
        void Clear()
        {
            u32LineNum = 0;
            for (int i = 0; i < 4; i++)
            {
                stLine[i].px.u32x = 0;
                stLine[i].px.u32y = 0;
                stLine[i].pdx.u32x = 0;
                stLine[i].pdx.u32y = 0;
                stLine[i].py.u32x = 0;
                stLine[i].py.u32y = 0;
                stLine[i].pdy.u32x = 0;
                stLine[i].pdy.u32y = 0;
                stPt[i].u32x = 0;
                stPt[i].u32y = 0;
            }
        }
    };

    struct VdfInfo
    {
        std::string strVdfMode;   //"md", "od", "vg"
        std::string strAlgMode; //md: 0x0 == fg, 1 == sad, 2 == framediff; vg: 0 == gate, 1 == Reg
        std::string strMdMbMode;  //0 == 4x4, 1 == 8x8, 2 == 16x16
        std::string strMdSadOutMode; // 0 == 8bit, 1 == 16bit
        std::string strOdWindows;      //suport 1x1, 2x2, 3x3
        std::string strSensitivity;    //od: 0:low, 1：middle, 2：hight, vg: 0 == min, 1 == low, 2 == middle, 3 == high, 4 == max
        std::string strMotionSensitivity; //od 0 == min, 1 == low, 2 == middle, 3 == high, 4 == max
        std::string strVgRegionDir;  //Region direction;
        std::string strVgSpecDirState;
        union
        {
            MdAttr stMdAttr;
            OdAttr stOdAttr;
            VgAttr stVgAttr;
        };
    public:
        VdfInfo()
        {
            stMdAttr.Clear();
            stOdAttr.Clear();
            stVgAttr.Clear();
        }
        void Clear()
        {
            VdfInfo tmp;
            std::swap(*this, tmp);
        }
    };    //vdf的属性结构体

    explicit AmigosSurfaceVdf(const std::string &strInSection);
    virtual ~AmigosSurfaceVdf();

    const VdfInfo& GetAttrInfo()
    {
        return stVdfInfo;
    };
    void SetInfo(const VdfInfo &info)
    {
        stVdfInfo = info;
    };

protected:
    VdfInfo stVdfInfo;
private:
    void _LoadDb();
    void _UnloadDb();
};
