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

#ifndef __AMIGOS_SURFACE_AO_H__
#define __AMIGOS_SURFACE_AO_H__

#include <map>
#include <string>
#include <utility>
#include "amigos_surface_base.h"
#include "amigos_surface_aio.h"

class AmigosSurfaceAo : public AmigosSurfaceBase
{
public:
    struct AoInfo
    {
        std::string                strFormat;
        std::string                strSoundMode;
        std::string                strChannelMode;
        std::map<std::string, int> mapInterface;
        unsigned int               uintPeriodSize;
        unsigned int               uintSampleRate;
        int                        intVolume;
        bool                       bSyncMode;
        Audio_I2S_Config           stAoI2sInfo;
        AoInfo()
        {
            Clear();
        }
        void Clear()
        {
            strFormat.clear();
            strSoundMode.clear();
            strChannelMode.clear();
            mapInterface = {
                {"DAC_AB", 0}, {"DAC_CD", 0}, {"I2S_A", 0}, {"I2S_B", 0}, {"ECHO_A", 0}, {"HDMI_A", 0}
            };
            uintPeriodSize = 0;
            uintSampleRate = 0;
            intVolume      = 0;
            bSyncMode      = false;
            stAoI2sInfo.Mode.clear();
            stAoI2sInfo.Format.clear();
            stAoI2sInfo.Mclk.clear();
            stAoI2sInfo.BitWidth.clear();
            stAoI2sInfo.SyncClock = false;
            stAoI2sInfo.TdmSlots  = 0;
        }
    };
    explicit AmigosSurfaceAo(const std::string &strInSection);
    virtual ~AmigosSurfaceAo();

protected:
    AoInfo stAoInfo;

private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};
#endif //__AMIGOS_SURFACE_AO_H__
