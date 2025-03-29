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

#ifndef __AMIGOS_SURFACE_AI_H__
#define __AMIGOS_SURFACE_AI_H__

#include <map>
#include <string>
#include <utility>
#include "amigos_surface_base.h"
#include "mi_ai_datatype.h"
#include "amigos_surface_aio.h"

class AmigosSurfaceAi : public AmigosSurfaceBase
{
public:
    struct AiInfo
    {
        std::string                strFormat;
        std::string                strSoundMode;
        std::map<std::string, int> mapInterface;
        unsigned int               uintSampleRate;
        unsigned int               uintWorkMode;
        unsigned int               uintPeriodSize;
        int                        intVolume;
        int                        intInterleaved;
        Audio_I2S_Config           stAiI2sInfo;

    public:
        AiInfo()
        {
            Clear();
        }
        void Clear()
        {
            strFormat.clear();
            strSoundMode.clear();
            mapInterface = {
                {"ADC_AB", 0},   {"ADC_CD", 0},   {"DMIC_A_01", 0}, {"DMIC_A_23", 0}, {"DMIC_A_45", 0},
                {"I2S_A_01", 0}, {"I2S_A_23", 0}, {"I2S_A_45", 0},  {"I2S_A_67", 0},  {"I2S_A_89", 0},
                {"I2S_A_ab", 0}, {"I2S_A_cd", 0}, {"I2S_A_ef", 0},  {"I2S_B_01", 0},  {"I2S_B_23", 0},
                {"I2S_B_45", 0}, {"I2S_B_67", 0}, {"I2S_B_89", 0},  {"I2S_B_ab", 0},  {"I2S_B_cd", 0},
                {"I2S_B_ef", 0}, {"I2S_C_01", 0}, {"I2S_C_23", 0},  {"I2S_C_45", 0},  {"I2S_C_67", 0},
                {"I2S_C_89", 0}, {"I2S_C_ab", 0}, {"I2S_C_cd", 0},  {"I2S_C_ef", 0},  {"I2S_D_01", 0},
                {"I2S_D_23", 0}, {"I2S_D_45", 0}, {"I2S_D_67", 0},  {"I2S_D_89", 0},  {"I2S_D_ab", 0},
                {"I2S_D_cd", 0}, {"I2S_D_ef", 0}, {"ECHO_A", 0},    {"HDMI_A", 0}
            };
            uintSampleRate = 0;
            uintWorkMode   = 0;
            uintPeriodSize = 0;
            intVolume      = 0;
            intInterleaved = 0;
            stAiI2sInfo.Mode.clear();
            stAiI2sInfo.Format.clear();
            stAiI2sInfo.Mclk.clear();
            stAiI2sInfo.BitWidth.clear();
            stAiI2sInfo.SyncClock = false;
            stAiI2sInfo.TdmSlots  = 0;
        }
    };

    explicit AmigosSurfaceAi(const std::string &strInSection);
    virtual ~AmigosSurfaceAi();
    void GetInfo(AiInfo &info) const;
    void SetInfo(const AiInfo &info);

protected:
    AiInfo stAiInfo;

private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};

#endif //__AMIGOS_SURFACE_AI_H__
