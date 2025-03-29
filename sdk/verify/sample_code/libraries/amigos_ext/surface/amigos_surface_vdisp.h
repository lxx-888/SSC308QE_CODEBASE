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

#ifndef __AMIGOS_SURFACE_VDISP_H__
#define __AMIGOS_SURFACE_VDISP_H__

#include "amigos_surface_base.h"

class AmigosSurfaceVdisp : public AmigosSurfaceBase
{
public:
    struct VdispInputInfo
    {
        int intPortId;
        int intChnId;
        int intFreeRun;
        int intVdispInX;
        int intVdispInY;
        int intVdispInWidth;
        int intVdispInHeight;
        VdispInputInfo()
        {
            intPortId        = 0;
            intChnId         = 0;
            intFreeRun       = 0;
            intVdispInX      = 0;
            intVdispInY      = 0;
            intVdispInWidth  = 0;
            intVdispInHeight = 0;
        }
    };
    struct VdispOutputInfo
    {
        int intPortId;
        int intVdispOutWidth;
        int intVdispOutHeight;
        int intVdispOutPts;
        int intVdispOutFrameRate;
        std::string strVdispOutFormat;
        std::string strVdispOutBkColor;
        VdispOutputInfo()
        {
            intPortId            = 0;
            intVdispOutWidth     = 0;
            intVdispOutHeight    = 0;
            intVdispOutPts       = 0;
            intVdispOutFrameRate = 0;
        }
    };
    explicit AmigosSurfaceVdisp(const std::string &strInSection);
    virtual ~AmigosSurfaceVdisp();
    void GetInfo(std::vector<VdispInputInfo> &in, std::vector<VdispOutputInfo> &out) const;
    void SetInfo(const std::vector<VdispInputInfo> &in, const std::vector<VdispOutputInfo> &out);
protected:
    std::vector<VdispInputInfo>  vVdispInputInfo;
    std::vector<VdispOutputInfo> vVdispOutputInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_VDISP_H__
