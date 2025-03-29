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

#ifndef __AMIGOS_SURFACE_SNR9931_H__
#define __AMIGOS_SURFACE_SNR9931_H__

#include "amigos_surface_base.h"

class AmigosSurfaceSnr9931 : public AmigosSurfaceBase
{
public:
    struct Snr9931Info
    {
        int intSensorId;
        int intSensorRes;
        int intHdrType;
        int intADIndex;
    public:
        Snr9931Info()
        {
            Clear();
        }
        void Clear()
        {
            intSensorId  = 0;
            intSensorRes = 0;
            intHdrType   = 0;
            intADIndex   = 0;
        }
    };
    explicit AmigosSurfaceSnr9931(const std::string &strInSection);
    virtual ~AmigosSurfaceSnr9931();
    void GetInfo(std::map<int, Snr9931Info> &info) const;
    void SetInfo(const std::map<int, Snr9931Info> &info);
protected:
    std::map<int, Snr9931Info> mapSnr9931Info;
private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_SNR9931_H__
