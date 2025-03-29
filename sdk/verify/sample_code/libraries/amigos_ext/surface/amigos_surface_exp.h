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

#ifndef __AMIGOS_SURFACE_EXP_H__
#define __AMIGOS_SURFACE_EXP_H__

#include "amigos_surface_base.h"
#include "ss_packet.h"

class AmigosSurfaceExp : public AmigosSurfaceBase
{
public:
    enum ExpInMode
    {
        E_EXP_IN_MODE_DIRECT,
        E_EXP_IN_MODE_HAND_SHAKE,
    };
    struct ExpInInfo
    {
        ExpInMode    mode;
        unsigned int socketPort;
        std::string  urlSuffix;
        unsigned int workId;
        ExpInInfo() : socketPort(0), workId(0) {}
    };

    struct ExpOutInfo
    {
        std::string  url;
        unsigned int workId;
        std::string  portType;
        std::string  fmt;
        unsigned int width;
        unsigned int height;
        std::string channel;
        unsigned int samplerate;
        ExpOutInfo() : workId(0), width(0), height(0), samplerate(0) {}
    };

public:
    explicit AmigosSurfaceExp(const std::string &strInSection);
    ~AmigosSurfaceExp() override;

protected:
    std::map<unsigned int, ExpInInfo>  mapExpInInfo;
    std::map<unsigned int, ExpOutInfo> mapExpOutInfo;

private:
    void _LoadDb() override;
    void _UnloadDb() override;
};
#endif //__AMIGOS_SURFACE_EXP_H__
