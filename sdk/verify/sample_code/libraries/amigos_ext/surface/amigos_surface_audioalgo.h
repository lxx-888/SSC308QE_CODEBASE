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

#ifndef __AMIGOS_SURFACE_AODUIOALGO_H__
#define __AMIGOS_SURFACE_AODUIOALGO_H__

#include "amigos_surface_base.h"

class AmigosSurfaceAudioAlgo : public AmigosSurfaceBase
{
public:
    struct AlgoInfo
    {
        unsigned int uintSampleRate;
        std::string  strSoundMode;
        unsigned int uintPeriodSize;
        bool         bSyncMode;
        struct ApcInfo
        {
            bool onoff;
        } stApcInfo;
        struct AnrInfo
        {
            bool         onoff;
            unsigned int uintUserMode;
            unsigned int uintSmoothLevel;
            unsigned int uintSpeed;
        } stAnrInfo;
        struct EqInfo
        {
            bool         onoff;
            unsigned int uintUserMode;
        } stEqInfo;
        struct HpfInfo
        {
            bool         onoff;
            unsigned int uintUserMode;
            std::string  strCutoffFreq;
        } stHpfInfo;
        struct AgcInfo
        {
            bool         onoff;
            unsigned int uintUserMode;
            unsigned int uintDropGainMax;
            unsigned int uintAttackTime;
            unsigned int uintReleaseTime;
            int          intDropGainThreshold;
        } stAgcInfo;
        struct SedInfo
        {
            bool        onoff;
            std::string strModelPath;
            int         intDetectMode;
            int         intSmoothLength;
            float       fVadTheshold;
            float       fEventTheshold[2];
        } stSedInfo;
        struct SrcInfo
        {
            bool         onoff;
            unsigned int uintorder;
            unsigned int uintOutSampleRate;
        } stSrcInfo;
        AlgoInfo()
        {
            Clear();
        }
        void Clear()
        {
            uintSampleRate = 0;
            strSoundMode.clear();
            uintPeriodSize = 0;
            bSyncMode      = 0;
            memset(&stApcInfo, 0, sizeof(stApcInfo));
            memset(&stAnrInfo, 0, sizeof(stAnrInfo));
            memset(&stEqInfo, 0, sizeof(stEqInfo));
            stHpfInfo.uintUserMode = 0;
            stHpfInfo.strCutoffFreq.clear();
            memset(&stAgcInfo, 0, sizeof(stAgcInfo));
            stSedInfo.onoff = 0;
            stSedInfo.strModelPath.clear();
            stSedInfo.intDetectMode   = 0;
            stSedInfo.intSmoothLength = 0;
            stSedInfo.fVadTheshold    = 0;
            memset(stSedInfo.fEventTheshold, 0, sizeof(float) * 2);
            memset(&stSrcInfo, 0, sizeof(stSrcInfo));
        }
    };
    explicit AmigosSurfaceAudioAlgo(const std::string &strInSection);
    virtual ~AmigosSurfaceAudioAlgo();
    void GetInfo(AlgoInfo &info) const;
    void SetInfo(const AlgoInfo &info);

protected:
    AlgoInfo stAlgoInfo;

private:
    virtual void _LoadDb();
    virtual void _UnloadDb();
};

#endif
