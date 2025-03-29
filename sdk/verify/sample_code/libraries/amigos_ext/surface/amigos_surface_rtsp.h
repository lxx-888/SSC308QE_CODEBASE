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

#ifndef __AMIGOS_SURFACE_RTSP_H__
#define __AMIGOS_SURFACE_RTSP_H__

#include "amigos_surface_base.h"


class AmigosSurfaceRtsp : public AmigosSurfaceBase
{
public:
    struct RtspOutInfo
    {
        std::string portType;
        std::string url;
        std::string userName;
        std::string passwd;
        std::string fmt;
        unsigned int width;
        unsigned int height;
        std::string channel;
        unsigned int samplerate;
    };
    struct RtspInInfo
    {
        unsigned int depth;
        std::string  url;
    };

public:
    explicit AmigosSurfaceRtsp(const std::string &strInSection);
    virtual ~AmigosSurfaceRtsp();

    bool GetRtspInInfo(unsigned int, struct RtspInInfo &stIn) const;
    void SetRtspInInfo(unsigned int, const struct RtspInInfo &stIn);
    bool GetRtspOutInfo(unsigned int, struct RtspOutInfo &stOut) const;
    void SetRtspOutInfo(unsigned int, const struct RtspOutInfo &stOut);
    bool GetRtspOnvif(bool &bOpenOnvif) const;
    void SetRtspOnvif(const bool &bOpenOnvif);

protected:
    std::map<unsigned int, struct RtspInInfo>  mapRtspInInfo;
    std::map<unsigned int, struct RtspOutInfo> mapPortToCfg;
    unsigned char                       bOpenOnvif;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_RTSP_H__
