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

#ifndef __AMIGOS_SURFACE_JPD_H__
#define __AMIGOS_SURFACE_JPD_H__

#include "amigos_surface_base.h"

class AmigosSurfaceJpd : public AmigosSurfaceBase
{
public:

    struct JpdInfo
    {
        unsigned int u32StreamBufSize;
        unsigned int u32MaxPicWidth;
        unsigned int u32MaxPicHeight;
        std::string strOutFmt;
    public:
        JpdInfo()
        {
            Clear();
        }
        void Clear()
        {
            u32StreamBufSize = 0;
            u32MaxPicWidth   = 0;
            u32MaxPicHeight  = 0;
            strOutFmt.clear();
        }
    };
    explicit AmigosSurfaceJpd(const std::string &strInSection);
    virtual ~AmigosSurfaceJpd();

protected:
    JpdInfo stJpdInfo;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_JPD_H__
