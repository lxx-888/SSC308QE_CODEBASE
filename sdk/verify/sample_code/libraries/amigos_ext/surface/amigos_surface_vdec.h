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

#ifndef __AMIGOS_SURFACE_VDEC_H__
#define __AMIGOS_SURFACE_VDEC_H__

#include "amigos_surface_base.h"

class AmigosSurfaceVdec : public AmigosSurfaceBase
{
public:
    struct VdecInfo
    {
        int          dpBufMode;
        int          refFrameNum;
        unsigned int bitstreamSize;
        unsigned int uintBufWidth;
        unsigned int uintBufHeight;
    public:
        VdecInfo()
        {
            Clear();
        }
        void Clear()
        {
            dpBufMode     = 0;
            refFrameNum   = 0;
            bitstreamSize = 0;
            uintBufWidth  = 0;
            uintBufHeight = 0;
        }
    };
    struct VdecOutInfo
    {
        int          intPortId;
        unsigned int uintDecOutWidth;
        unsigned int uintDecOutHeight;
        int          bEnable;
        unsigned int uintVdecCropX;
        unsigned int uintVdecCropY;
        unsigned int uintVdecCropW;
        unsigned int uintVdecCropH;
        VdecOutInfo()
        {
            intPortId        = 0;
            uintDecOutWidth  = 0;
            uintDecOutHeight = 0;
            bEnable          = 0;
            uintVdecCropX    = 0;
            uintVdecCropY    = 0;
            uintVdecCropW    = 0;
            uintVdecCropH    = 0;
        }
    };
    explicit AmigosSurfaceVdec(const std::string &strInSection);
    virtual ~AmigosSurfaceVdec();
    void GetInfo(VdecInfo &info, std::vector<VdecOutInfo> &out)const;
    void SetInfo(const VdecInfo &info, const std::vector<VdecOutInfo> &out);

protected:
    VdecInfo                    stVdecInfo;
    std::vector<VdecOutInfo>    vVdecOutInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_VDEC_H__
